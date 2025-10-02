
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <new>
#include <queue>
#include <syncstream>
#include <thread>
#include <utility>
#include <vector>

using namespace std::literals;

template <typename T>
class ThreadSafeQueue
{
public:
    [[nodiscard]] bool Empty() const
    {
        std::lock_guard<std::mutex> lk{mut};
        return queue.empty();
    }

    void Push(T val)
    {
        {
            std::lock_guard<std::mutex> lk{mut};
            queue.push(std::move(val));
        }
        cvar.notify_one();
    }

    void WaitAndPop(T& val)
    {
        std::unique_lock<std::mutex> lk{mut};
        cvar.wait(lk, [this]() { return !queue.empty(); });
        val = std::move(queue.front());
        queue.pop();
    }

    bool TryPop(T& val)
    {
        std::lock_guard<std::mutex> lk{mut};
        if (queue.empty())
            return false;

        val = std::move(queue.front());
        queue.pop();
        return true;
    }

private:
    mutable std::mutex mut;
    std::queue<T> queue;
    std::condition_variable cvar;
};

class ThreadPool
{
public:
    ThreadPool() : threadCount{std::thread::hardware_concurrency()}, done{false}
    {
        try
        {
            for (unsigned i = 0; i < threadCount; ++i)
                threads.emplace_back(&ThreadPool::DoWork, this);
        }
        catch (...)
        {
            Cleanup();
            throw;
        }
    }

    ~ThreadPool()
    {
        Cleanup();
    }

    [[nodiscard]] size_t Size() const
    {
        return threadCount;
    }

    template <typename CallableType>
    void Submit(CallableType callable)
    {
        workQueue.Push(std::function<void()>{callable});
    }

private:
    void Cleanup()
    {
        done = true;
        for (auto& t : threads)
            if (t.joinable())
                t.join();
    }

    void DoWork()
    {
        while (!done)
        {
            std::function<void()> task;
            if (workQueue.TryPop(task))
                task();
            else
                std::this_thread::yield();
        }
    }

    // Order matters:
    // - Destroy threads -> queue
    size_t threadCount;
    std::atomic_bool done;
    ThreadSafeQueue<std::function<void()>> workQueue;
    std::vector<std::thread> threads;
};

int main()
{
    ThreadPool threadPool;
    std::cout << "ThreadPool has " << threadPool.Size() << " threads\n";

    for (size_t i = 0; i < threadPool.Size(); ++i)
        threadPool.Submit(
            [i]() { std::osyncstream{std::cout} << std::this_thread::get_id() << " says " << i << '\n'; });

    std::this_thread::sleep_for(3s);

    return 0;
}