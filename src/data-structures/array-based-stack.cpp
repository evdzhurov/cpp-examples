#include <cassert>
#include <cstddef>

template <typename T, size_t N = 1>
class ArrayBasedBoundedStack
{
    static_assert(N >= 1, "ArrayBasedBoundedStack must be at least 1!");

public:
    [[nodiscard]] size_t Capacity() const
    {
        return N;
    }

    [[nodiscard]] size_t Size() const
    {
        return top;
    }

    [[nodiscard]] bool Empty() const
    {
        return top == 0;
    }

    bool Push(T val)
    {
        if (top == Capacity())
            return false;

        data[++top] = std::move(val);
        return true;
    }

    bool Pop(T& val)
    {
        if (Empty())
            return false;

        val = std::move(data[top--]);
        return true;
    }

private:
    // Elements [1 : S.top]
    size_t top = 0;
    T data[N + 1] = {};
};

int main()
{
    ArrayBasedBoundedStack<int, 10> S1;
    assert(S1.Capacity() == 10);
    assert(S1.Empty());

    int elem = -1;
    assert(!S1.Pop(elem));

    assert(S1.Push(123));
    assert(S1.Size() == 1);
    assert(!S1.Empty());

    assert(S1.Push(256));
    assert(S1.Size() == 2);
    assert(!S1.Empty());

    assert(S1.Pop(elem));
    assert(elem == 256);

    assert(S1.Pop(elem));
    assert(elem == 123);

    assert(S1.Size() == 0);
    assert(S1.Empty());

    return 0;
}