#include <condition_variable>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <syncstream>
#include <thread>
#include <vector>

using namespace std::literals;

template <typename T>
class ThreadSafeQueue
{
public:
    // Creates a dummy node that separates head/tail
    ThreadSafeQueue() : head{std::make_unique<Node>(T{})}, tail{head.get()}
    {
    }

    bool Empty()
    {
        std::lock_guard<std::mutex> lk{headMut};
        return head.get() == GetTail();
    }

    std::optional<T> TryPop()
    {
        auto oldHead = PopHead();
        return oldHead ? std::optional<T>{std::move(oldHead->data)} : std::nullopt;
    }

    void Push(T data)
    {
        std::unique_ptr<Node> newDummy = std::make_unique<Node>(std::move(data));

        std::lock_guard<std::mutex> lk{tailMut};

        tail->data = std::move(data);
        tail->next = std::move(newDummy);
        tail = tail->next.get();
    }

private:
    struct Node
    {
        Node(T data) : data{std::move(data)}
        {
        }

        T data;
        std::unique_ptr<Node> next;
    };

    Node* GetTail()
    {
        std::lock_guard<std::mutex> lk{tailMut};
        return tail;
    }

    std::unique_ptr<Node> PopHead()
    {
        std::lock_guard<std::mutex> lk{headMut};

        if (head.get() == GetTail()) // Head, Tail point to the dummy node, the list is empty
            return nullptr;

        auto oldHead = std::move(head);
        head = std::move(oldHead->next);
        return oldHead;
    }

    std::mutex headMut;
    std::unique_ptr<Node> head;

    std::mutex tailMut;
    Node* tail = nullptr;
};

int main()
{
    const int MAX_VAL = 10;
    ThreadSafeQueue<int> queue;

    std::vector<std::future<void>> futures;
    futures.reserve(20);

    futures.emplace_back(std::async(std::launch::async, [&queue]() {
        const auto tid = std::this_thread::get_id();

        while (true)
        {
            auto data = queue.TryPop();
            if (data)
            {
                std::osyncstream{std::cout} << tid << " consumed " << *data << '\n';
                if (*data == MAX_VAL)
                    break;
            }
            else
            {
                std::osyncstream{std::cout} << tid << " queue empty!\n";
            }

            std::this_thread::sleep_for(100ms);
        }
    }));

    futures.emplace_back(std::async(std::launch::async, [&queue]() {
        for (int i = 1; i <= MAX_VAL; ++i)
        {
            std::osyncstream{std::cout} << std::this_thread::get_id() << " producing " << i << '\n';
            queue.Push(i);
            std::this_thread::sleep_for(100ms);
        }
    }));

    return 0;
}