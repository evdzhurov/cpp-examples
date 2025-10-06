#include <algorithm>
#include <cassert>
#include <cstddef>
#include <vector>

template <typename T>
class ArrayBasedBoundedQueue
{
public:
    ArrayBasedBoundedQueue(size_t capacity) : data(capacity + 1)
    {
    }

    [[nodiscard]] bool Empty() const
    {
        return head == tail;
    }

    [[nodiscard]] bool Full() const
    {
        return (tail + 1) % data.size() == head;
    }

    [[nodiscard]] size_t Size() const
    {
        return tail >= head ? tail - head : tail + data.size() - head;
    }

    bool Enqueue(T value)
    {
        if (Full())
            return false;

        data[tail++] = std::move(value);
        if (tail == data.size())
            tail = 0;

        return true;
    }

    bool EnqueueCycle(T value)
    {
        if (Full())
        {
            ++head;
            if (head == data.size())
                head = 0;
        }

        data[tail++] = std::move(value);
        if (tail == data.size())
            tail = 0;

        return true;
    }

    bool Dequeue(T& value)
    {
        if (Empty())
            return false;

        value = std::move(data[head++]);
        if (head == data.size())
            head = 0;

        return true;
    }

private:
    size_t head = 0;
    size_t tail = 0;
    std::vector<T> data;
};

int main()
{
    ArrayBasedBoundedQueue<int> Q1(3);
    assert(Q1.Empty());
    assert(!Q1.Full());
    assert(Q1.Size() == 0);
    // [?, ?, ?, ?] -> Empty
    //  h
    //  t

    Q1.Enqueue(1);
    Q1.Enqueue(2);
    Q1.Enqueue(3);
    assert(!Q1.Empty());
    assert(Q1.Full());
    assert(!Q1.Enqueue(4));
    assert(Q1.Size() == 3);
    // [1, 2, 3, ?] -> Full
    //  h
    //           t

    assert(Q1.EnqueueCycle(4));
    assert(Q1.Full());
    // [1, 2, 3, 4] -> Cycle One
    //     h
    //  t

    int val = -1;
    assert(Q1.Dequeue(val) && val == 2);
    assert(Q1.Dequeue(val) && val == 3);
    assert(Q1.Dequeue(val) && val == 4);
    assert(Q1.Empty());

    // Q1.Dequeue(val);
    // assert(val == 1);
    // assert(Q1.Size() == 2);

    // Q1.Dequeue(val);
    // assert(val == 2);
    // assert(Q1.Size() == 1);

    // Q1.Dequeue(val);
    // assert(val == 3);
    // assert(!Q1.Dequeue(val));
    // assert(Q1.Size() == 0);
    // // [1, 2, 3, ?] -> Empty
    // //           h
    // //           t

    // assert(Q1.Enqueue(4));
    // assert(Q1.Enqueue(5));
    // assert(Q1.Size() == 2);
    // // [5, 2, 3, 4]
    // //           h
    // //     t

    // assert(Q1.Dequeue(val) && val == 4);
    // assert(Q1.Dequeue(val) && val == 5);
    // assert(Q1.Size() == 0);
    // // [5, 2, 3, 4] -> Empty
    // //     h
    // //     t

    return 0;
}