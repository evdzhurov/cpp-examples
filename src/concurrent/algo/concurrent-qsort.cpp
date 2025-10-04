#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <future>
#include <iostream>
#include <iterator>
#include <syncstream>
#include <thread>
#include <vector>

template <typename Iter>
void QuickSort(Iter first, Iter last)
{
    if (std::distance(first, last) == 0)
        return;

    // 1) First is the partition value
    // 6, 2, 4, 3, 0, 1, 7, 9, 8, 5
    // p

    // 2) Partition [first, last) -> d is first ~Predicate ('x' >= pivot)
    // Note: by keeping the pivot outside the partition range we don't need a copy of the value for
    // comparisons.
    // 6, 2, 4, 3, 0, 1, 5, 9, 8, 7, E
    // p  f                 d        l

    const auto& pivot = *first;
    const Iter divide = std::partition(std::next(first), last, [&](const auto& x) { return x < pivot; });

    // 3) Swap pivot and the last element < pivot to keep the partition correct
    // divide is now prev(divide)
    // 5, 2, 4, 3, 0, 1, 6, 9, 8, 7, E
    // *  f              d           l

    std::iter_swap(first, std::prev(divide));

    // 4) Divide & Conquer
    QuickSort(first, std::prev(divide));
    QuickSort(divide, last);
}

// Pretty bad - spawns O(n) threads
template <typename Iter>
void ConcurrentQuickSort(Iter first, Iter last)
{
    std::osyncstream{std::cout} << std::this_thread::get_id() << '\n';

    if (std::distance(first, last) == 0)
        return;

    const auto& pivot = *first;
    const Iter divide = std::partition(std::next(first), last, [&](const auto& x) { return x < pivot; });
    std::iter_swap(first, std::prev(divide));

    auto fut = std::async(ConcurrentQuickSort<Iter>, first, std::prev(divide));
    ConcurrentQuickSort(divide, last);
    fut.get();
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "specify vector size to sort!\n";
        return 0;
    }

    const auto size = std::atoi(argv[1]);

    std::srand(1);
    auto data = std::vector<int>(size);
    std::fill(data.begin(), data.end(), std::rand());

    ConcurrentQuickSort(std::begin(data), std::end(data));
    assert(std::is_sorted(std::begin(data), std::end(data)));

    return 0;
}