#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <syncstream>
#include <thread>

// A condition variable is synchronization primitive and an optimization over a busy-wait.

struct DataChunk
{
    int val = 0;
};

struct CVarExample
{
    static constexpr auto N = 10;

    void Produce()
    {
        for (int i = 1; i <= N; ++i)
        {
            DataChunk dataChunk{i};
            {
                std::lock_guard<std::mutex> lk{mut};
                shared_queue.emplace(dataChunk);
            }
            std::osyncstream{std::cout} << "Produced " << dataChunk.val << '\n';

            // Notify after releasing the lock -> if the waiting thread wakes immediately
            // it will not be blocked.
            cvar.notify_one();
        }
    }

    void Consume()
    {
        while (true)
        {
            // Uses unique_lock because cvar will unlock/lock in the process of waiting.
            std::unique_lock<std::mutex> lk{mut};

            // The condition inside the lambda will be checked only if lk is held by the thread.
            // This will happen when the cvar is notified and also can happen
            // spontaneously because of the OS (spurious wake).
            cvar.wait(lk, [&]() { return !shared_queue.empty(); });

            const auto dataChunk = shared_queue.front();
            shared_queue.pop();

            // As soon as we obtain the data there's no reason to hold the lock further.
            lk.unlock();

            std::osyncstream{std::cout} << "Consumer " << std::this_thread::get_id() << " : " << dataChunk.val << '\n';

            if (dataChunk.val == N)
                break;
        }
    }

    std::mutex mut;
    std::queue<DataChunk> shared_queue;
    std::condition_variable cvar;
};

int main()
{
    CVarExample example;

    // Note that without explicit std::launch::async the allowed policy can be async or deferred.
    auto producer = std::async(std::launch::async, [&]() { example.Produce(); });
    auto consumer = std::async(std::launch::async, [&]() { example.Consume(); });

    producer.get();
    consumer.get();

    return 0;
}