#include <barrier>
#include <cmath>
#include <cstdio>
#include <format>
#include <iostream>
#include <stop_token>
#include <syncstream>
#include <thread>
#include <vector>

using namespace std::literals;

int main()
{
    std::vector vals = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};

    // A barrier callback needs to be noexcept
    const auto printValues = [&vals]() noexcept {
        for (auto val : vals)
            std::cout << std::format(" {:<7.5}", val);
        std::cout << '\n';
    };

    std::cout << "Press Ctrl+D to stop.\n";
    printValues();

    // A barrier takes a counter initial value (signed) and a callback
    // Once the counter hits 0 it gets reset and the callback is called.
    std::barrier allDone{static_cast<int>(vals.size()), printValues};

    std::vector<std::jthread> threads;
    threads.reserve(vals.size());

    for (size_t i = 0; i < vals.size(); ++i)
    {
        threads.emplace_back([i, &vals, &allDone](std::stop_token st) {
            while (!st.stop_requested())
            {
                std::this_thread::sleep_for(1s);
                vals[i] = std::sqrt(vals[i]);
                allDone.arrive_and_wait();
            }

            allDone.arrive_and_drop(); // decrements the counter and the next starting value.
        });
    }

    while (std::getchar() != EOF)
    {
    }

    std::cout << "Requesting threads to stop...\n";
    for (auto& t : threads)
        t.request_stop();
    std::cout << "Done.\n";

    return 0;
}