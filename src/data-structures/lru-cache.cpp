#include <cassert>
#include <iostream>
#include <list>
#include <string>
#include <unordered_map>

using namespace std::literals;

template <typename K, typename V>
class LRUCache
{
public:
    LRUCache(std::size_t capacity) : capacity{capacity}
    {
    }

    [[nodiscard]] size_t Capacity() const
    {
        return capacity;
    }

    [[nodiscard]] size_t Size() const
    {
        return lru.size();
    }

    [[nodiscard]] bool Empty() const
    {
        return lru.empty();
    }

    void Put(K key, V value)
    {
        if (auto it = idx.find(key); it != idx.end())
        {
            // Update existing key
            it->second->second = std::move(value);
            Touch(it->second);
        }
        else
        {
            // Add key/value
            lru.emplace_front(std::move(key), std::move(value));
            idx.emplace(lru.front().first, lru.begin());

            // Maintain capacity
            EvictAsNeeded();
        }
    }

    V* Get(const K& key) // Touch
    {
        const auto it = idx.find(key);
        if (it == idx.end())
            return nullptr;

        Touch(it->second);
        return &it->second->second;
    }

    const V* Peek(const K& key) const // No Touch
    {
        const auto it = idx.find(key);
        return it != idx.end() ? &it->second->second : nullptr;
    }

private:
    using Node = std::pair<K, V>;
    using NodeIt = typename std::list<Node>::iterator;

    void Touch(NodeIt it)
    {
        // Moves node to the front
        if (it != lru.begin())
            lru.splice(lru.begin(), lru, it);
    }

    void EvictAsNeeded()
    {
        while (lru.size() > capacity)
        {
            const auto& key = lru.back().first;
            idx.erase(key);
            lru.pop_back();
        }
    }

    std::size_t capacity = 0;
    std::list<Node> lru; // MRU/Front <=========> LRU/Back
    std::unordered_map<K, NodeIt> idx;
};

int main()
{
    LRUCache<std::string, int> empty{5};
    assert(empty.Size() == 0);
    assert(empty.Empty());
    assert(empty.Capacity() == 5);

    const auto k1 = "hello"s;
    const auto k2 = "world"s;

    empty.Put(k1, 111);

    assert(*empty.Peek(k1) == 111);
    assert(*empty.Get(k1) == 111);

    empty.Put(k1, 222);
    assert(*empty.Peek(k1) == 222);
    assert(*empty.Get(k1) == 222);

    empty.Put(k2, 333);
    assert(*empty.Peek(k2) == 333);
    assert(*empty.Get(k2) == 333);

    return 0;
}