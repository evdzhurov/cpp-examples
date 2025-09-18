#include <map>
#include <print>
#include <ranges>
#include <unordered_map>
#include <vector>

int main()
{
    // ====================================================================================
    // C++23
    // ====================================================================================
    // std::ranges::to (Create containers)
    {
        std::vector nums = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        std::unordered_map<int, std::string_view> num_to_text = {
            {1, "one"}, {2, "two"},   {3, "three"}, {4, "four"}, {5, "five"},
            {6, "six"}, {7, "seven"}, {8, "eight"}, {9, "nine"},
        };

        const auto even_nums =
            nums | std::views::filter([](int n) { return n % 2 == 0; }) | std::ranges::to<std::vector>();
        std::println("even numbers: {}", even_nums);

        const auto text_num_map = even_nums | std::views::transform([&num_to_text](int n) {
                                      return std::pair{n, num_to_text.contains(n) ? num_to_text[n] : "unknown"};
                                  }) |
                                  std::ranges::to<std::map>();

        std::println("even numbers text: {}", text_num_map);
    }

    return 0;
}