#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <semaphore>
#include <stop_token>
#include <syncstream>
#include <thread>

using namespace std::literals;

int main()
{
    std::queue<char> values;
    std::mutex valuesMtx;

    for (int i = 0; i < 1000; ++i)
        values.push(static_cast<char>('a' + (i % ('z' - 'a' + 1))));

    // The maximum count is provided as a compile-time constant
    // to enable optimized implementations.
    constexpr int numThreads = 10;
    std::counting_semaphore<numThreads> enabled{0};

    std::vector<std::jthread> pool;
    pool.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i)
        pool.emplace_back([&enabled, &values, &valuesMtx](std::stop_token st) {
            while (!st.stop_requested())
            {
                // Request this thread to become one of the 'enabled' threads.
                // Initially blocked because the number of 'enabled' threads is 0
                // Can acquire when we gradually increase the 'enabled' count.
                enabled.acquire();

                const auto tid = std::this_thread::get_id();
                std::osyncstream{std::cout} << tid << " is enabled!\n";

                std::this_thread::sleep_for(500ms);

                const auto val = std::invoke([&values, &valuesMtx] {
                    std::lock_guard lk{valuesMtx};
                    char val = values.front();
                    values.pop();
                    return val;
                });

                std::osyncstream{std::cout} << tid << ' ' << val << '\n';

                // Remove thread from 'enabled'
                // Note that the same thread may acquire again after release
                // because it would not have to do a context switch.
                // For this reason scheduling can be unfair.
                enabled.release();
            }
        });

    std::osyncstream{std::cout} << "wait 2 seconds...\n";
    std::this_thread::sleep_for(2s);

    std::osyncstream{std::cout} << "enable 3 concurrent threads...\n";
    enabled.release(3);
    std::this_thread::sleep_for(2s);

    std::osyncstream{std::cout} << "enable 2 more concurrent threads...\n";
    enabled.release(2);
    std::this_thread::sleep_for(2s);

    std::osyncstream{std::cout} << "stop processing...\n";
    for (auto& t : pool)
        t.request_stop();

    return 0;
}