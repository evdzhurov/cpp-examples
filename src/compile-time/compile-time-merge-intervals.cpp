#include <utility>
#include <vector>

using Interval = std::pair<int, int>;

constexpr bool IntervalOverlap(const Interval& first, const Interval& second)
{
    const auto [s0, e0] = first;
    const auto [s1, e1] = second;

    // 1) Two possible states to NOT overlap (assuming first/second are not ordered)
    // S0---------E0  S1--------E1
    // S1---------E1  S0--------E0
    // S1 > E0 || S0 > E1

    // 2) DeMorgan's Law
    // !(S1 > E0 || S0 > E1)
    // S1 <= E0 && S0 <= E1

    // 3) Reorder End -> Start
    // E0 >= S1 && E1 >= S0
    return e0 >= s1 && e1 >= s0;
}

static_assert(!IntervalOverlap({0, 1}, {2, 3}));
static_assert(!IntervalOverlap({2, 3}, {0, 1}));
static_assert(IntervalOverlap({3, 6}, {6, 7}));
static_assert(IntervalOverlap({6, 7}, {3, 6}));

// constexpr vector requires C++20
constexpr std::vector<Interval> MergeSorted(const std::vector<Interval>& intervals)
{
    if (intervals.empty())
        return {};

    // Assume intervals are sorted.
    auto curr = intervals[0];
    std::vector<Interval> merged;
    for (size_t i = 1; i < intervals.size(); ++i)
    {
        if (IntervalOverlap(curr, intervals[i]))
        {
            curr.second = std::max(curr.second, intervals[i].second);
        }
        else
        {
            merged.push_back(curr);
            curr = intervals[i];
        }
    }

    merged.push_back(curr);

    return merged;
}

static_assert(MergeSorted({}) == std::vector<Interval>{});
static_assert(MergeSorted({{1, 2}}) == std::vector<Interval>{{1, 2}});
static_assert(MergeSorted({{1, 2}, {2, 3}}) == std::vector<Interval>{{1, 3}});
static_assert(MergeSorted({{1, 2}, {3, 5}}) == std::vector<Interval>{{1, 2}, {3, 5}});
static_assert(MergeSorted({{1, 5}, {4, 7}, {6, 10}}) == std::vector<Interval>{{1, 10}});