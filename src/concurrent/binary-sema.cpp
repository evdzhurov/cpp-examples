#include <iostream>
#include <semaphore>
#include <stop_token>
#include <syncstream>
#include <thread>

// binarys_semaphore is counting_semaphore<1>
// - can work as a mutex but releasing thread may not be the acquiring thread.
// - can be used to signal/notify a thread multiple times unlike a future.

using namespace std::literals;

int main()
{
    int shared_data = 0;

    std::binary_semaphore data_ready{0};
    std::binary_semaphore data_done{0};

    std::jthread process{[&](std::stop_token st) {
        const auto tid = std::this_thread::get_id();
        while (!st.stop_requested())
        {
            if (data_ready.try_acquire_for(1s))
            {
                int data = shared_data;
                std::osyncstream{std::cout} << '[' << tid << "] read " << data << '\n';
                std::this_thread::sleep_for(data * 500ms);
                std::osyncstream{std::cout} << '[' << tid << "] done\n";

                data_done.release();
            }
            else
            {
                std::osyncstream{std::cout} << '[' << tid << "] timeout\n";
            }
        }
    }};

    for (int i = 0; i < 10; ++i)
    {
        std::osyncstream{std::cout} << "[main] store " << i << '\n';
        shared_data = i;

        data_ready.release();

        data_done.acquire();
        std::osyncstream{std::cout} << "[main] processing done\n";
    }

    return 0;
}