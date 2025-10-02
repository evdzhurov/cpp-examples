
#include <atomic>
#include <cassert>
#include <csignal>
#include <iostream>
#include <thread>

using namespace std::literals;

std::atomic_bool signaled = false;

void sig_handler(int sig)
{
    signaled = true;
    std::cout << "Handled signal: " << sig << '\n';
}

int main()
{
    std::signal(SIGUSR1, sig_handler);

    while (!signaled)
    {
        std::cout << "Waiting for signal...\n";
        std::this_thread::sleep_for(1s);
    }

    return 0;
}