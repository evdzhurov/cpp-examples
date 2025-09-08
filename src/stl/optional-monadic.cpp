#include <algorithm>
#include <cctype>
#include <iostream>
#include <optional>
#include <string>

// C++20 - std::format
// C++23 - monadic optional functions
// c++23 - println

using namespace std::literals;

std::optional<std::string> get_user_input()
{
    std::string input;
    std::cout << "Enter your name: ";
    std::getline(std::cin, input);
    if (input.empty())
        return std::nullopt;
    return input;
}

int main()
{
    const auto result = get_user_input()
                            .transform([](std::string name) -> std::string {
                                std::ranges::transform(name, name.begin(), ::toupper);
                                return name;
                            })
                            .transform([](const std::string& name) {
                                if (name == "ADMIN")
                                    return "Welcome, Admin!"s;
                                return std::format("Hello, {}!", name);
                            })
                            .value_or("No input provided."s);

    std::println("{}", result);

    return 0;
}