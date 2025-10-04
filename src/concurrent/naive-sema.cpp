#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

class NaiveSemaphore
{
public:
    NaiveSemaphore(unsigned int count = 0) : count{count}
    {
    }

    void Post()
    {
        {
            std::lock_guard<std::mutex> lk{mtx};
            ++count;
        }
        cv.notify_one();
    }

    void Wait()
    {
        std::unique_lock<std::mutex> lk{mtx};
        cv.wait(lk, [this]() { return count > 0; });
        --count;
    }

private:
    std::mutex mtx;
    std::condition_variable cv;
    unsigned int count = 0;
};

int main()
{
    using namespace std::literals;

    NaiveSemaphore sema1;

    const auto doWork = [&]() {
        const auto tid = std::this_thread::get_id();
        std::cout << "thread " << tid << " waiting..." << std::endl;

        sema1.Wait();
        std::cout << "thread " << tid << " entered the critical region" << std::endl;
        sema1.Post();
    };

    std::vector<std::thread> threads;
    threads.reserve(5);
    for (int i = 0; i < 5; ++i)
        threads.emplace_back(doWork);

    std::this_thread::sleep_for(1s);
    sema1.Post();

    for (auto& t : threads)
        t.join();

    return 0;
}