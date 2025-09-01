#include <charconv>
#include <expected>
#include <iostream>
#include <optional>
#include <string_view>
#include <system_error>

using namespace std::literals;
constexpr auto WhiteSpaceChars = " \t\n\r\f\v"sv;

// C++17
constexpr int parseIntCxx17(std::string_view str)
{
    const auto start = str.find_first_not_of(WhiteSpaceChars);
    if (start == std::string_view::npos)
        return 0;

    int sign = 1;
    std::size_t index = start;
    if (str[start] == '-' || str[start] == '+')
    {
        sign = (str[start] == '-') ? -1 : 1;
        ++index;
    }

    int val = 0;
    for (; index < str.size(); ++index)
    {
        char ch = str[index];
        if (ch < '0' || ch > '9')
            break;

        // "123"
        // 0 * 10 + 1 = 1
        // 1 * 10 + 2 = 12
        // 12 * 10 + 3 = 123
        val = val * 10 + sign * (ch - '0');
    }

    return val;
}

// C++23
// std::optional is constexpr in C++23
constexpr std::optional<int> maybeParseIntCxx23(std::string_view str)
{
    auto start = str.find_first_not_of(WhiteSpaceChars);
    if (start == std::string_view::npos)
        return std::nullopt;

    // from_chars does not parse '+'
    if (str[start] == '+')
        ++start;

    int result = 0;
    // from_chars added since C++17, is constexpr since C++23
    const auto [ptr, err] = std::from_chars(str.data() + start, str.data() + str.size(), result);
    if (err == std::errc{})
        return result;

    return std::nullopt;
}

// C++23
// std::expected added in C++23
constexpr std::expected<int, std::errc> errorOrParseIntCxx23(std::string_view str)
{
    auto start = str.find_first_not_of(WhiteSpaceChars);
    if (start == std::string_view::npos)
        return std::unexpected{std::errc::invalid_argument};

    // from_chars does not parse '+'
    if (str[start] == '+')
        ++start;

    int result = 0;
    // from_chars added since C++17, is constexpr since C++23
    const auto [ptr, err] = std::from_chars(str.data() + start, str.data() + str.size(), result);
    if (err == std::errc{})
        return result;

    return std::unexpected{err};
}

int main()
{
    static_assert(parseIntCxx17("123") == 123);
    static_assert(parseIntCxx17("+123") == 123);
    static_assert(parseIntCxx17("-123") == -123);
    static_assert(parseIntCxx17("   456   ") == 456);
    static_assert(parseIntCxx17("abc123def") == 0);
    static_assert(parseIntCxx17("") == 0);
    static_assert(parseIntCxx17("abcdef") == 0);
    static_assert(parseIntCxx17("2147483647") == 2147483647);
    static_assert(parseIntCxx17("-2147483648") == -2147483648);

    static_assert(maybeParseIntCxx23("123") == 123);
    static_assert(maybeParseIntCxx23("+123") == 123);
    static_assert(maybeParseIntCxx23("-123") == -123);
    static_assert(maybeParseIntCxx23("   456   ") == 456);
    static_assert(!maybeParseIntCxx23("abc123def").has_value());
    static_assert(!maybeParseIntCxx23("").has_value());
    static_assert(!maybeParseIntCxx23("abcdef").has_value());
    static_assert(maybeParseIntCxx23("2147483647") == 2147483647);
    static_assert(maybeParseIntCxx23("-2147483648") == -2147483648);

    static_assert(errorOrParseIntCxx23("123") == 123);
    static_assert(errorOrParseIntCxx23("+123") == 123);
    static_assert(errorOrParseIntCxx23("-123") == -123);
    static_assert(errorOrParseIntCxx23("   456   ") == 456);
    static_assert(errorOrParseIntCxx23("abc123def").error() == std::errc::invalid_argument);
    static_assert(errorOrParseIntCxx23("").error() == std::errc::invalid_argument);
    static_assert(errorOrParseIntCxx23("abcdef").error() == std::errc::invalid_argument);
    static_assert(errorOrParseIntCxx23("2147483647") == 2147483647);
    static_assert(errorOrParseIntCxx23("-2147483648") == -2147483648);
    static_assert(errorOrParseIntCxx23("9999999999").error() == std::errc::result_out_of_range);

    std::cout << parseIntCxx17("9999999999") << '\n';

    return 0;
}