#include <concepts>
#include <iostream>
#include <stdexcept>
#include <vector>

template <typename T>
class MaxHeap
{
public:
    MaxHeap() : slots(1)
    {
    }

    MaxHeap(std::vector<T> data) : slots(data.size() + 1)
    {
        for (int i = 1; i < slots.size(); ++i)
            slots[i] = std::move(data[i - 1]);
        BuildHeap();
    }

    [[nodiscard]] bool Empty() const
    {
        return slots.size() < 2;
    }

    [[nodiscard]] const T& Top() const
    {
        if (Empty())
            throw std::runtime_error("Top: empty heap!");

        return slots[1];
    }

    void Pop()
    {
        if (Empty())
            throw std::runtime_error("Pop: empty heap!");

        using std::swap;
        swap(slots[1], slots.back());
        slots.pop_back();

        MaxHeapify(1);
    }

    void Push(T val)
    {
        slots.emplace_back(std::move(val));
        SiftUp(slots.size() - 1);
    }

private:
    void MaxHeapify(size_t idx)
    {
        const size_t left = idx * 2;
        const size_t right = (idx * 2) + 1;

        size_t largest = idx;

        if (left < slots.size() && slots[left] > slots[largest])
            largest = left;

        if (right < slots.size() && slots[right] > slots[largest])
            largest = right;

        if (idx != largest)
        {
            using std::swap;
            swap(slots[largest], slots[idx]);
            MaxHeapify(largest);
        }
    }

    void SiftUp(size_t idx)
    {
        while (idx > 1)
        {
            const size_t parent = idx / 2;
            if (slots[parent] < slots[idx])
            {
                using std::swap;
                swap(slots[parent], slots[idx]);
                idx = parent;
            }
            else
            {
                break;
            }
        }
    }

    void BuildHeap()
    {
        for (int i = ((slots.size() - 1) / 2); i >= 1; --i)
            MaxHeapify(i);
    }

    std::vector<T> slots;
};

int main()
{
    MaxHeap<int> h1;
    h1.Push(1);
    h1.Push(5);
    h1.Push(7);
    h1.Push(3);

    while (!h1.Empty())
    {
        std::cout << h1.Top() << '\n';
        h1.Pop();
    }

    MaxHeap<int> h2{{2, -3, 1, 4, 99, 15}};
    std::cout << "\nAfter build heap:\n";
    while (!h2.Empty())
    {
        std::cout << h2.Top() << '\n';
        h2.Pop();
    }

    return 0;
}