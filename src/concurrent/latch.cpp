#include <array>
#include <iostream>
#include <latch>
#include <syncstream>
#include <thread>

using namespace std::literals;

int main()
{
    std::array tags{'0', '1', '2', '3', '4'};

    // Can be used once to synchronize multiple threads.
    std::latch allReady{2};
    std::latch allDone{tags.size()};

    std::jthread t1{[&] {
        allReady.arrive_and_wait(); // count_down() + wait()

        const auto tid = std::this_thread::get_id();
        std::osyncstream{std::cout} << '[' << tid << "] arrived and ready!\n";

        for (size_t i = 0; i < tags.size(); i += 2)
        {
            std::this_thread::sleep_for(100ms);
            std::osyncstream{std::cout} << '[' << tid << "] " << tags[i] << '\n';
            allDone.count_down();
        }
    }};

    std::jthread t2{[&] {
        allReady.arrive_and_wait();

        const auto tid = std::this_thread::get_id();
        std::osyncstream{std::cout} << '[' << tid << "] arrived and ready!\n";

        for (size_t i = 1; i < tags.size(); i += 2)
        {
            std::this_thread::sleep_for(100ms);
            std::osyncstream{std::cout} << '[' << tid << "] " << tags[i] << '\n';
            allDone.count_down();
        }
    }};

    std::cout << "\nwaiting until all tasks are done\n";
    allDone.wait();
    std::cout << "\nall tasks done\n";
}