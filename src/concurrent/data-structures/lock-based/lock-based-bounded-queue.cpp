#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <syncstream>
#include <thread>

using namespace std::literals;

template <typename T>
class LockBasedBoundedQueue
{
public:
    LockBasedBoundedQueue(size_t capacity) : capacity{std::max<size_t>(1, capacity)} {};

    void Close()
    {
        {
            std::lock_guard<std::mutex> lk{mut};
            if (closed)
                return;
            closed = true;
        }

        cv_not_empty.notify_all();
        cv_not_full.notify_all();
    }

    template <typename... Args>
    bool WaitAndEmplace(Args&&... args)
    {
        {
            std::unique_lock<std::mutex> lk{mut};
            cv_not_full.wait(lk, [&]() { return queue.size() < capacity || closed; });
            if (closed)
                return false;

            queue.emplace(std::forward<Args>(args)...);
        }

        cv_not_empty.notify_one();

        return true;
    }

    std::optional<T> WaitAndPop()
    {
        std::optional<T> res;

        {
            std::unique_lock<std::mutex> lk{mut};
            cv_not_empty.wait(lk, [&]() { return !queue.empty() || closed; });
            if (queue.empty())
                return std::nullopt;
            res = std::move(queue.front());
            queue.pop();
        }

        cv_not_full.notify_one();

        return res;
    }

private:
    bool closed = false;
    size_t capacity;
    std::queue<T> queue;
    std::mutex mut;
    std::condition_variable cv_not_empty;
    std::condition_variable cv_not_full;
};

int main()
{
    LockBasedBoundedQueue<int> Q1{5};

    const auto doProduce = [&]() {
        const auto tid = std::this_thread::get_id();
        for (int i = 1; i <= 50; ++i)
        {
            std::osyncstream{std::cout} << tid << " pushing " << i << "...\n";
            if (!Q1.WaitAndEmplace(i))
            {
                std::osyncstream{std::cout} << tid << " can't push anymore!\n";
                return;
            }
        }

        std::osyncstream{std::cout} << tid << " closing...\n";
        Q1.Close();
    };

    const auto doConsume = [&]() {
        const auto tid = std::this_thread::get_id();
        while (true)
        {
            auto val = Q1.WaitAndPop();
            if (!val)
            {
                std::osyncstream{std::cout} << tid << " Empty value sentinel!\n";
                break;
            }

            std::osyncstream{std::cout} << tid << " WaitAndPop -> " << *val << '\n';
            std::this_thread::sleep_for(50ms);
        }
    };

    const size_t N_PRODUCERS = 5;
    std::vector<std::future<void>> producers;
    producers.reserve(N_PRODUCERS);
    for (int i = 0; i < N_PRODUCERS; ++i)
        producers.emplace_back(std::async(std::launch::async, doProduce));

    const size_t N_CONSUMERS = 15;
    std::vector<std::future<void>> consumers;
    consumers.reserve(N_CONSUMERS);
    for (int i = 0; i < N_CONSUMERS; ++i)
        consumers.emplace_back(std::async(std::launch::async, doConsume));

    return 0;
}