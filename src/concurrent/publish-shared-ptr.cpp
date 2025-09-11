#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <syncstream>
#include <thread>

// Source: https://accu.org/journals/overload/32/183/teodorescu/

using namespace std::literals;

template <typename T>
class SharedResource
{
public:
    void Publish(T&& doc)
    {
        std::lock_guard<std::mutex> lock{small_bottleneck};
        published_doc = std::make_shared<const T>(std::move(doc));
    }

    std::shared_ptr<const T> Get()
    {
        std::lock_guard<std::mutex> lock{small_bottleneck};
        return published_doc;
    }

private:
    std::mutex small_bottleneck;
    std::shared_ptr<const T> published_doc;
};

int main()
{
    SharedResource<int> shared_resource;
    const auto MaxVer = 10;

    auto producer = std::async([&]() {
        for (int i = 1; i <= MaxVer; ++i)
        {
            std::this_thread::sleep_for(1s);
            std::osyncstream{std::cout} << "Producer ver. " << i << '\n';
            shared_resource.Publish(std::move(i));
        }
    });

    auto consumer1 = std::async([&]() {
        while (true)
        {
            auto current = shared_resource.Get();

            std::this_thread::sleep_for(350ms);

            if (current)
            {
                std::osyncstream{std::cout} << "Consumer1 ver. " << *current << '\n';

                if (*current == MaxVer)
                    return;
            }
        }
    });

    auto consumer2 = std::async([&]() {
        while (true)
        {
            auto current = shared_resource.Get();

            std::this_thread::sleep_for(500ms);

            if (current)
            {
                std::osyncstream{std::cout} << "Consumer2 ver. " << *current << '\n';

                if (*current == MaxVer)
                    return;
            }
        }
    });

    producer.get();
    consumer1.get();
    consumer2.get();

    std::cout << "Done.\n";

    return 0;
}