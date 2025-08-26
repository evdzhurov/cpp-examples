#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <future>
#include <iostream>
#include <syncstream>
#include <vector>

// Source: https://brilliantsugar.github.io/posts/how-i-learned-to-stop-worrying-and-love-juggling-c++-atomics/

// Buggy single producer/single consumer triple buffer implementation
template <typename T>
class Trio
{
public:
    // Consumer calls to get a buffer with either stale data or latest comitted data.
    T& Read()
    {
        const auto dirtyBuffer = m_middleBuffer.load(std::memory_order_relaxed);
        if ((dirtyBuffer & kDirtyBit) == 1)
        {
            const auto cleanFrontBuffer = reinterpret_cast<std::uintptr_t>(m_frontBuffer);
            const auto prev = m_middleBuffer.exchange(cleanFrontBuffer, std::memory_order_acq_rel);
            m_frontBuffer = reinterpret_cast<T*>(prev & ~kDirtyBit); // NOLINT(performance-no-int-to-ptr)
        }

        return *m_frontBuffer;
    }

    // Producer side get the current back buffer to write.
    T& Write()
    {
        return *m_backBuffer;
    }

    // Producer commits written data and swaps the back/middle buffers.
    void Commit()
    {
        const auto dirtyBackBuffer = reinterpret_cast<uintptr_t>(m_backBuffer) | kDirtyBit;
        const auto prev = m_middleBuffer.exchange(dirtyBackBuffer, std::memory_order_acq_rel);
        m_backBuffer = reinterpret_cast<T*>(prev & ~kDirtyBit); // NOLINT(performance-no-int-to-ptr)
    }

private:
    static constexpr std::size_t kNoSharing = 64;
    static constexpr std::uintptr_t kDirtyBit = 0b1;

    struct alignas(kNoSharing) Buffer
    {
        T data{};
    };

    std::array<Buffer, 3> m_buffers;

    // Because we align on at least a 64-byte boundary to avoid
    // false sharing there are log2(64) = 6 'zero' bits in every address.
    std::atomic_uintptr_t m_middleBuffer{reinterpret_cast<std::uintptr_t>(&m_buffers[1].data)};
    alignas(kNoSharing) T* m_frontBuffer{&m_buffers[0].data}; // only consumer can access
    alignas(kNoSharing) T* m_backBuffer{&m_buffers[2].data};  // only producer can access
};

int main(int argc, char* argv[])
{
    Trio<int> sharedState;
    constexpr int kMaxNumber = 100;

    auto producer = std::async([&]() {
        for (int i = 1; i <= kMaxNumber; ++i)
        {
            std::osyncstream{std::cout} << " -> " << i << '\n';

            sharedState.Write() = i;
            sharedState.Commit();
        }
    });

    auto consumer = std::async([&]() {
        std::vector<int> history;

        int lastRead = 0;
        while (lastRead != kMaxNumber)
        {
            lastRead = sharedState.Read();
            if (std::ranges::find(history, lastRead) == history.end())
            {
                std::osyncstream{std::cout} << " <- " << lastRead << '\n';

                history.emplace_back(lastRead);
            }
        }
    });

    producer.get();
    consumer.get();

    return 0;
}