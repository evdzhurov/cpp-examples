#include <print>
#include <ranges>
#include <vector>

int main()
{
    // ====================================================================================
    // C++23
    // ====================================================================================
    // cartesian_product, enumerate, zip
    {
        const auto range1 = std::vector{1, 2, 3};
        const auto range2 = std::vector{'A', 'B', 'C'};

        const auto product = std::views::cartesian_product(range1, range2);
        std::println("Cartesian Product:");
        for (const auto& [a, b] : product)
            std::println("({}, {})", a, b);

        const auto enumerated = std::views::enumerate(range1);
        std::println("\nEnumerated Range:");
        for (const auto& [index, value] : enumerated)
            std::println("Index: {}, Value: {}", index, value);

        const auto zipped = std::views::zip(range1, range2);
        std::println("\nZipped Ranges:");
        for (const auto& [a, b] : zipped)
            std::println("({}, {})", a, b);

        std::println();
    }

    // chunk, slide, stride
    {
        const auto numbers = std::vector{1, 2, 3, 4, 5, 6, 7, 8, 9};

        // Chunk: divide the range into groups of 3
        const auto chunks = std::views::chunk(numbers, 3);
        std::println("Chunks of 3:");
        for (const auto& chunk : chunks)
        {
            for (int n : chunk)
                std::print("{} ", n);
            std::println();
        }
        std::println();

        // Slide: create overlapping subranges of size 3
        const auto sliding = std::views::slide(numbers, 3);
        std::println("Sliding Window of 3:");
        for (const auto& window : sliding)
        {
            for (int n : window)
                std::print("{} ", n);
            std::println();
        }
        std::println();

        // Stride: skip every 2 elements
        const auto strided = std::views::stride(numbers, 2);
        std::println("Strided Range (step 2):");
        for (int n : strided)
            std::print("{}", n);
        std::println();
    }

    return 0;
}