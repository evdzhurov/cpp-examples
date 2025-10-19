#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <exception>
#include <iomanip>
#include <ios>
#include <iostream>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string_view>

using namespace std::literals;

constexpr unsigned ETH_IPV4 = 0x0800;
constexpr unsigned FIRST_COL_W = 20;

static const std::array<uint8_t, 14 + 20 + 20 + 27> SAMPLE_TCP_HTTP = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0x08, 0x00, 0x45, 0x00, 0x00,
    0x3A, 0x12, 0x34, 0x40, 0x00, 0x40, 0x06, 0x00, 0x00, 0xC0, 0xA8, 0x01, 0x64, 0x93, 0x18, 0x15, 0x2A,
    0xC0, 0x23, 0x00, 0x50, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x50, 0x18, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 'G',  'E',  'T',  ' ',  '/',  ' ',  'H',  'T',  'T',  'P',  '/',  '1',  '.',  '1',
    '\r', '\n', '\r', '\n', 'H',  'o',  's',  't',  ':',  ' ',  'x',  '\r', '\n'};

static const std::array<uint8_t, 14 + 20 + 8 + 12> SAMPLE_UDP_DNS = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBA, 0xAD, 0xF0, 0x0D, 0x12, 0x34, 0x08, 0x00, 0x45, 0x00, 0x00, 0x28,
    0xAB, 0xCD, 0x00, 0x00, 0x40, 0x11, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x01, 0x08, 0x08, 0x08, 0x08, 0xC0, 0x00,
    0x00, 0x35, 0x00, 0x14, 0x00, 0x00, 0x12, 0x34, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// ======================================= BUFFER VIEW ======================================= //
struct BufferView
{
    [[nodiscard]] uint16_t get_be16(size_t off) const
    {
        if (off + 2 > size)
            throw std::out_of_range("get_be16");

        return uint16_t{data[off]} << 8 | uint16_t{data[off + 1]};
    }

    [[nodiscard]] uint16_t get_be32(size_t off) const
    {
        if (off + 4 > size)
            throw std::out_of_range("get_be32");

        return uint16_t{data[off]} << 24 | uint16_t{data[off + 1]} << 16 | uint16_t{data[off + 2]} << 8 |
               uint16_t{data[off + 3]};
    }

    const uint8_t* data = nullptr;
    size_t size = 0;
};

// ======================================= ETHERNET ======================================= //
struct Ethernet_Header
{
    std::array<uint8_t, 6> dst{}; // Dest MAC address
    std::array<uint8_t, 6> src{}; // Src MAC address
    uint16_t ethertype{};         // Next protocol: IPv4, ARP, IPv6, etc.
};
static_assert(alignof(Ethernet_Header) == 2);
static_assert(sizeof(Ethernet_Header) == 14);

static std::optional<Ethernet_Header> parse_eth(const BufferView& buf, size_t& off)
{
    if (buf.size < sizeof(Ethernet_Header))
        return std::nullopt;

    Ethernet_Header eth{};
    std::memcpy(eth.dst.data(), buf.data, 6);     // bytes [0, 6)
    std::memcpy(eth.src.data(), buf.data + 6, 6); // bytes [6, 12)
    off += 12;

    // Note: big-endian encoding, byte 12 holds the most significant byte, so we shift it one byte to the left.
    eth.ethertype = buf.get_be16(off);
    off += 2;

    return eth;
}

// ======================================= IPv4 ======================================= //
struct IPv4_Header
{
    uint8_t version{}; // Version (IPV4=4)
    uint8_t ihl{};     // Internet Header Len (number of 32 bit words)
    uint8_t tos{};     // Type of service

    uint16_t total_len{}; // Entire packet size header + data
    uint16_t id{};
    uint16_t frag_off{}; // Flags (3 bits) + Fragment Offset (13 bits)
    uint8_t ttl{};       // Time to live (hop count)
    uint8_t proto{};     // Protocol (ICMP, IGMP, TCP, UDP, ENCAP, OSPF, SCTP)
    uint16_t checksum{};

    uint32_t src{}; // Source address
    uint32_t dst{}; // Dest address

    // Options: 0-320 bits (32 bits padded)
    size_t header_bytes = 0;
};

static std::optional<IPv4_Header> parse_ipv4(const BufferView& buf, size_t& off)
{
    if (off + 20 > buf.size)
        return std::nullopt;

    IPv4_Header ipv4_hdr;

    const auto ver_ihl = buf.data[off];

    ipv4_hdr.version = ver_ihl >> 4; // Get the high 4 bits
    ipv4_hdr.ihl = ver_ihl & 0x0F;   // Get the low 4 bits
    if (ipv4_hdr.version != 4 || ipv4_hdr.ihl < 5)
        return std::nullopt;

    ipv4_hdr.tos = buf.data[off + 1];
    ipv4_hdr.total_len = buf.get_be16(off + 2);
    ipv4_hdr.id = buf.get_be16(off + 4);
    ipv4_hdr.frag_off = buf.get_be16(off + 6);

    ipv4_hdr.ttl = buf.data[off + 8];
    ipv4_hdr.proto = buf.data[off + 9];

    ipv4_hdr.checksum = buf.get_be16(off + 10);
    ipv4_hdr.src = buf.get_be32(off + 12);
    ipv4_hdr.dst = buf.get_be32(off + 16);

    ipv4_hdr.header_bytes = ipv4_hdr.ihl * 4;
    if (off + ipv4_hdr.header_bytes > buf.size)
        return std::nullopt;

    off += ipv4_hdr.header_bytes;

    return ipv4_hdr;
}

// ======================================= UDP ======================================= //
// Portable way to pack a struct (not really needed for structs already packed)
// #pragma pack(push, 1)
struct /*__attribute__((packed))*/ UDP_Header // GCC/Clang-specific packing
{
    uint16_t sport{};    // Source Port
    uint16_t dport{};    // Dest Port
    uint16_t len{};      // Length
    uint16_t checksum{}; // Checksum
};
// #pragma pack(pop)

static_assert(alignof(UDP_Header) == 2);
static_assert(sizeof(UDP_Header) == 8);

UDP_Header parse_udp(const BufferView& buf, size_t& off)
{
    if (off + sizeof(UDP_Header) > buf.size)
        throw std::out_of_range("UDP Header insufficient buffer size!");

    UDP_Header hdr;
    hdr.sport = buf.get_be16(off);
    hdr.dport = buf.get_be16(off + 2);
    hdr.len = buf.get_be16(off + 4);
    hdr.checksum = buf.get_be16(off + 6);

    off += sizeof(UDP_Header);

    return hdr;
}

// ======================================= TCP ======================================= //
struct TCP_Header
{
    enum Flags : uint8_t
    {
        FIN = 0b1 << 0, // Last packet from sender
        SYN = 0b1 << 1, // Synchronize seq. numbers
        RST = 0b1 << 2, // Reset the connection
        PSH = 0b1 << 3, // Push buffered data to receiving application
        ACK = 0b1 << 4, // Ack field is significant
        URG = 0b1 << 5, // Urgent pointer is significant
        ECE = 0b1 << 6, // ECN
        CWR = 0b1 << 7, // Congestion window reduced
    };

    uint16_t sport{};      // Source Port
    uint16_t dport{};      // Dest Port
    uint32_t seq{};        // Seq. number
    uint32_t ack{};        // Ack number
    uint8_t data_offset{}; // data offset (4 bits) + reserved (4 bits)
    Flags flags{};
    uint16_t window{};
    uint16_t checksum{};
    uint16_t urg_ptr{};

    // size_t header_bytes{}; Options + padding
    size_t header_bytes = 0;
};

std::ostream& operator<<(std::ostream& os, TCP_Header::Flags flags)
{
    os << "CWR:" << ((flags & TCP_Header::CWR) > 0 ? 1 : 0) << ' ';
    os << "ECE:" << ((flags & TCP_Header::ECE) > 0 ? 1 : 0) << ' ';
    os << "URG:" << ((flags & TCP_Header::URG) > 0 ? 1 : 0) << ' ';
    os << "ACK:" << ((flags & TCP_Header::ACK) > 0 ? 1 : 0) << ' ';
    os << "PSH:" << ((flags & TCP_Header::PSH) > 0 ? 1 : 0) << ' ';
    os << "RST:" << ((flags & TCP_Header::RST) > 0 ? 1 : 0) << ' ';
    os << "SYN:" << ((flags & TCP_Header::SYN) > 0 ? 1 : 0) << ' ';
    os << "FIN:" << ((flags & TCP_Header::FIN) > 0 ? 1 : 0);

    return os;
}

TCP_Header parse_tcp(const BufferView& buf, size_t& off)
{
    if (off + sizeof(TCP_Header) > buf.size)
        throw std::out_of_range("TCP Header insufficient buffer size!");

    TCP_Header hdr;
    hdr.sport = buf.get_be16(off);
    hdr.dport = buf.get_be16(off + 2);
    hdr.seq = buf.get_be32(off + 4);
    hdr.ack = buf.get_be32(off + 8);

    hdr.data_offset = buf.data[off + 12] >> 4; // Read in the high 4-bits
    hdr.flags = static_cast<TCP_Header::Flags>(buf.data[off + 13]);

    hdr.window = buf.get_be16(off + 14);
    hdr.checksum = buf.get_be16(off + 16);
    hdr.urg_ptr = buf.get_be16(off + 18);

    hdr.header_bytes = hdr.data_offset * 4;

    if (off + hdr.header_bytes > buf.size)
        throw std::out_of_range("TCP header data offset outside buffer!");

    off += hdr.header_bytes;

    return hdr;
}

// ======================================= UTILS ======================================= //
static std::string mac_to_str(const std::array<uint8_t, 6>& addr)
{
    std::ostringstream os;
    os << std::hex << std::setfill('0');

    for (int i = 0; i < addr.size(); ++i)
    {
        if (i > 0)
            os << ':';
        os << std::setw(2) << (unsigned)addr[i]; // Not that we need to cast to an integer type otherwise the print will
                                                 // be broken (trying to print a char!)
    }

    return os.str();
}

static std::string ip_to_str(const uint32_t ip_addr)
{
    std::stringstream ss;

    ss << ((ip_addr >> 24) & 0xFF) << '.';
    ss << ((ip_addr >> 16) & 0xFF) << '.';
    ss << ((ip_addr >> 8) & 0xFF) << '.';
    ss << (ip_addr & 0xFF);

    return ss.str();
}

void parse_and_print(const uint8_t* data, size_t size)
{
#define COUT_FIRST_COL std::cout << std::setw(FIRST_COL_W) << std::left

    const auto buf_view = BufferView{data, size};

    size_t off = 0;
    const auto eth_hdr = parse_eth(buf_view, off);
    if (!eth_hdr)
    {
        std::cout << "Failed to parse Ethernet frame header!\n";
        return;
    }
    assert(off == sizeof(Ethernet_Header));

    std::cout << "[Ethernet Header]\n";
    COUT_FIRST_COL << "DstMac: " << mac_to_str(eth_hdr->dst) << '\n';
    COUT_FIRST_COL << "SrcMac: " << mac_to_str(eth_hdr->src) << '\n';
    COUT_FIRST_COL << "Type: " << std::showbase << std::hex << eth_hdr->ethertype << "\n";
    std::cout << std::noshowbase << std::dec;

    if (eth_hdr->ethertype != ETH_IPV4)
    {
        std::cout << "Not IPv4\n";
        return;
    }

    const auto ipv4_hdr = parse_ipv4(buf_view, off);
    if (!ipv4_hdr)
    {
        std::cout << "Failed to parse IPv4 header!\n";
        return;
    }

    std::cout << "\n[IPv4 Header]\n";
    COUT_FIRST_COL << "Src: " << ip_to_str(ipv4_hdr->src) << '\n';
    COUT_FIRST_COL << "Dst: " << ip_to_str(ipv4_hdr->dst) << '\n';
    COUT_FIRST_COL << "Proto: " << (unsigned)ipv4_hdr->proto << '\n';
    COUT_FIRST_COL << "Internet Hdr Len: " << (unsigned)ipv4_hdr->ihl << '\n';
    COUT_FIRST_COL << "Total Len: " << ipv4_hdr->total_len << '\n';

    if (ipv4_hdr->proto == 6)
    {
        const auto tcp_hdr = parse_tcp(buf_view, off);

        std::cout << "\n[TCP Header]\n";
        COUT_FIRST_COL << "Src Port:" << tcp_hdr.sport << '\n';
        COUT_FIRST_COL << "Dst Port:" << tcp_hdr.dport << '\n';
        COUT_FIRST_COL << "Seq:" << tcp_hdr.seq << '\n';
        COUT_FIRST_COL << "Ack:" << tcp_hdr.ack << '\n';
        COUT_FIRST_COL << "Flags:" << tcp_hdr.flags << '\n';
        COUT_FIRST_COL << "Window:" << tcp_hdr.window << '\n';
        COUT_FIRST_COL << "Checksum:" << tcp_hdr.checksum << '\n';
        COUT_FIRST_COL << "Urg. Ptr:" << tcp_hdr.urg_ptr << '\n';

        if (off < buf_view.size)
        {
            const char* payload = reinterpret_cast<const char*>(buf_view.data + off);
            size_t payload_size = buf_view.size - off;

            std::string_view payload_view{payload, payload_size};
            std::cout << "Payload:\n" << payload_view << '\n';
        }
    }
    else if (ipv4_hdr->proto == 17)
    {
        const auto udp_hdr = parse_udp(buf_view, off);

        std::cout << "\n[UDP Header]\n";
        COUT_FIRST_COL << "Src Port: " << udp_hdr.sport << '\n';
        COUT_FIRST_COL << "Dst Port: " << udp_hdr.dport << '\n';
        COUT_FIRST_COL << "Length: " << udp_hdr.len << '\n';
        COUT_FIRST_COL << "Checksum: " << udp_hdr.len << '\n';
    }
}

int main()
{
    try
    {
        std::cout << "=== Sample Frame 1: TCP + HTTP ===\n";
        parse_and_print(SAMPLE_TCP_HTTP.data(), SAMPLE_TCP_HTTP.size());

        std::cout << "\n=== Sample Frame 2: UDP (DNS-like) ===\n";
        parse_and_print(SAMPLE_UDP_DNS.data(), SAMPLE_UDP_DNS.size());
    }
    catch (const std::exception& ex)
    {
        std::cout << "Exception thrown: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}