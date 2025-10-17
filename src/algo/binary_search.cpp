#include <cassert>
#include <cstddef>
#include <vector>

template <template <typename> class Vec, typename T>
static bool binary_search(const Vec<T>& container, const T& value)
{
    int low = 0;
    int high = container.size() - 1; // can become -1

    // 1, 2, 3, 4, 5
    // l     m     h

    // 1, 2, 3, 4
    // l  m     h

    while (low <= high)
    {
        int mid = low + (high - low) / 2;
        if (container[mid] == value)
            return true;

        if (container[mid] < value)
            low = mid + 1; // target value could be on the right
        else
            high = mid - 1; // target value could be on the left
    }

    return false;
}

template <template <typename> class Vec, typename T>
static int lower_bound(const Vec<T>& container, const T& value)
{
    int low = 0;
    int high = container.size(); // past the end!

    while (low < high)
    {
        int mid = low + (high - low) / 2;
        if (container[mid] < value)
            low = mid + 1;
        else
            high = mid; // value <= cont[mid]
    }

    return low; // may be cont.size() [0..n]
}

template <template <typename> class Vec, typename T>
static int upper_bound(const Vec<T>& container, const T& value)
{
    int low = 0;
    int high = container.size(); // past the end!

    while (low < high)
    {
        int mid = low + (high - low) / 2;
        if (container[mid] <= value) // Comparison is no longer strict.
            low = mid + 1;
        else
            high = mid; // value < cont[mid]
    }

    return low; // may be container.size() [0..n]
}

int main()
{
    std::vector<int> empty;
    assert(!binary_search(empty, 123));

    std::vector<int> v1 = {1, 2, 3, 4, 10, 10, 10, 56, 66, 101};
    assert(binary_search(v1, 10));
    assert(binary_search(v1, 101));
    assert(binary_search(v1, 1));
    assert(!binary_search(v1, 65));
    assert(!binary_search(v1, -18));

    assert(lower_bound(v1, 102) == v1.size());
    assert(lower_bound(v1, 1) == 0);
    assert(lower_bound(v1, -1) == 0);
    assert(lower_bound(v1, 10) == 4);
    assert(lower_bound(v1, 56) == 7);
    assert(lower_bound(v1, 2) == 1);

    assert(upper_bound(v1, 102) == v1.size());
    assert(upper_bound(v1, 1) == 1);
    assert(upper_bound(v1, -1) == 0);
    assert(upper_bound(v1, 10) == 7);
    assert(upper_bound(v1, 56) == 8);
    assert(upper_bound(v1, 2) == 2);

    return 0;
}