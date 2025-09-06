#include <mdspan>
#include <print>
#include <vector>

// Source: https://www.cppstories.com/2025/cpp23_mdspan/
// Requires C++23 and libc++ for
// - std::mdspan
// - std::println

int main()
{
    // vec is the underlying data
    std::vector vec = {1, 2, 3, 4, 5, 3, 5, 6, 7, 8, 9};

    // With static 2D extents
    // - When the extents are known at compile time the mdspan object can be smaller.
    {
        std::mdspan<int, std::extents<size_t, 3, 3>> mat3x3{vec.data()};

        // NOLINTNEXTLINE(bugprone-sizeof-container, readability-static-accessed-through-instance)
        std::println("mat3x3 static extent | sizeof: {}, rank: {}, ext 0: {}, ext 1: {}", sizeof(mat3x3), mat3x3.rank(),
                     mat3x3.extent(0), mat3x3.extent(1));
    }

    // With dynamic 2D extents
    {
        std::mdspan<int, std::dextents<size_t, 2>> mat2x2{vec.data(), 2, 2};

        // NOLINTNEXTLINE(bugprone-sizeof-container, readability-static-accessed-through-instance)
        std::println("mat2x2 dyn extent |sizeof: {}, rank: {}, ext 0: {}, ext 1: {}", sizeof(mat2x2), mat2x2.rank(),
                     mat2x2.extent(0), mat2x2.extent(1));
    }

    // Empty span
    {
        std::mdspan<int, std::dextents<size_t, 1>> empty_span;
        std::println("is empty {}", empty_span.empty());
    }

    // Extents from variadic arguments
    {
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
        int arr[12] = {0};

        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
        std::mdspan<int, std::extents<size_t, 3, 4>> span{arr};
        std::println("w: {}, h: {}", span.extent(0), span.extent(1));

        const auto ex = span.extents();
        // NOLINTNEXTLINE(readability-static-accessed-through-instance)
        std::println("{}, {}", ex.rank(), ex.static_extent(1));

        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
        std::mdspan<int, std::dextents<size_t, 2>> dspan{arr, 3, 4};
        std::println("w: {}, h: {}", dspan.extent(0), dspan.extent(1));

        const auto ex2 = dspan.extents();
        // NOLINTNEXTLINE(readability-static-accessed-through-instance)
        std::println("{}, {}", ex2.rank(), ex2.static_extent(1));
    }

    // Extents from std::array
    {
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
        int arr[12] = {0};

        std::array<size_t, 2> my_extents = {3, 4};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
        std::mdspan<int, std::extents<size_t, 3, 4>> span{arr, my_extents};

        // * With CTAD (Class Template Argument Deduction)
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
        std::mdspan span_ctad{arr, my_extents};
    }

    // Layout
    {
        std::vector<int> vec = {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        };

        // * Lambda template parameters (C++20)
        const auto printMat = []<typename layout>(std::mdspan<int, std::dextents<size_t, 2>, layout> mat) {
            for (size_t i = 0; i < mat.extent(0); ++i)
            {
                for (size_t j = 0; j < mat.extent(1); ++j)
                    std::print("{} ", mat[i, j]);
                std::println();
            }
        };

        std::mdspan<int, std::dextents<size_t, 2>, std::layout_right> mat{vec.data(), 2, 3};
        std::println("right layout (row-major): ");
        printMat(mat);

        std::mdspan<int, std::dextents<size_t, 2>, std::layout_left> mat_left{vec.data(), 2, 3};
        std::println("left layout (column-major): ");
        printMat(mat_left);
    }

    return 0;
}