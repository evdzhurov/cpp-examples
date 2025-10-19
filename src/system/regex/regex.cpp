#include <iomanip>
#include <iostream>
#include <regex>
#include <string>

int main()
{
    std::vector<std::string> ips = {
        "192.168.1.1",
        "192.168.300.1",
        "739.168.001.1",
        "135.168.01.1",
    };

    // std::regex ipv4_pattern_weak{R"(\b\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}\b)"};

    std::regex ipv4_pattern_strong{
        R"(\b(?:25[0-5]|2[0-4]\d|1\d{2}|[1-9]?\d)(?:\.(?:25[0-5]|2[0-4]\d|1\d{2}|[1-9]?\d)){3}\b)"};

    for (const auto& ip : ips)
    {
        std::cout << ip << '\n';
        if (std::regex_match(ip, ipv4_pattern_strong))
            std::cout << "Valid IP\n";
        else
            std::cout << "Invalid IP\n";
        std::cout << '\n';
    }

    std::regex valid_mac_patter{R"(\b[a-fA-F0-9]{2}([:-])(?:[a-fA-F0-9]{2}\1){4}[a-fA-F0-9]{2}\b)"};

    std::vector<std::string> valid_macs = {
        "00:1a:2b:3c:4d:5e", "AA:BB:CC:DD:EE:FF", "0a:Bb:Cc:0D:1e:2F", "00-1A-2B-3C-4D-5E", "ff:ff:ff:ff:ff:ff",
    };

    std::vector<std::string> invalid_macs = {
        "00:1A:2B:3C:4D",    "00:1A:2B:3C:4D:5E:6F", "00:1G:2H:3I:4J:5K",
        "00:1A-2B:3C:4D:5E", "00:1A:2B:3C:4D:5E:",   "0:1:2:3:4:5",
    };

    std::cout << "Valid macs:\n";
    for (const auto& valid_mac : valid_macs)
    {
        std::cout << std::setw(30) << std::left << valid_mac
                  << (std::regex_match(valid_mac, valid_mac_patter) ? "Valid!" : "Invalid!") << '\n';
    }

    std::cout << "\nInvalid macs:\n";
    for (const auto& invalid_mac : invalid_macs)
    {
        std::cout << std::setw(30) << std::left << invalid_mac
                  << (std::regex_match(invalid_mac, valid_mac_patter) ? "Valid!" : "Invalid!") << '\n';
    }
}