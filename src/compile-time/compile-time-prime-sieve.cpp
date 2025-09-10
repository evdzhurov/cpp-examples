#include <iostream>
#include <type_traits>

// Source: https://accu.org/journals/overload/32/183/wu/

// < C++17 metaprograming (see constexpr solution after)
struct Nil
{
};

template <typename Head, typename Tail = Nil>
struct List
{
};

// Prototype
template <template <typename> class Pred, typename List>
struct Filter;

// Main "overload"
// Check value and:
// 1) Include Head and continue filtering the tail
// 2) Discard Head and continue filtering the tail
template <template <typename> class Pred, typename Head, typename Tail>
struct Filter<Pred, List<Head, Tail>>
{
    using type = typename std::conditional<Pred<Head>::value, List<Head, typename Filter<Pred, Tail>::type>,
                                           typename Filter<Pred, Tail>::type>::type;
};

// Recursion terminates when we reach the Nil marker.
template <template <typename> class Pred>
struct Filter<Pred, Nil>
{
    using type = Nil;
};

// Algorithm
template <typename T>
struct PrimeSieve;

template <typename Head, typename Tail>
struct PrimeSieve<List<Head, Tail>>
{
    template <typename T>
    struct IsNotDivisible : std::integral_constant<bool, (T::value % Head::value) != 0>
    {
    };

    using type = List<Head, typename PrimeSieve<typename Filter<IsNotDivisible, Tail>::type>::type>;
};

template <>
struct PrimeSieve<Nil>
{
    using type = Nil;
};

// Range helper
template <int First, int Last>
struct Range
{
    using type = List<std::integral_constant<int, First>, typename Range<First + 1, Last>::type>;
};

template <int Last>
struct Range<Last, Last>
{
    using type = Nil;
};

// Generator
template <int N>
struct GeneratePrimes : PrimeSieve<typename Range<2, N + 1>::type>::type
{
};

// Output
template <typename Head, typename Tail>
void Output(List<Head, Tail>)
{
    std::cout << Head::value << ", ";
    Output(Tail());
}

inline void Output(Nil)
{
}

// C++17 constexpr solution
template <int N>
constexpr auto PrimeSieveFn()
{
    std::array<bool, N + 1> sieve{};
    for (int i = 2; i <= N; ++i)
        sieve[i] = true;

    for (int p = 2; p * p <= N; ++p)
        if (sieve[p])
            for (int i = p * p; i <= N; i += p)
                sieve[i] = false;

    return sieve;
}

template <size_t N>
constexpr size_t CountPrime(const std::array<bool, N>& sieve)
{
    size_t count = 0;
    for (size_t i = 2; i < sieve.size(); ++i)
        if (sieve[i])
            ++count;
    return count;
}

// C++20 force consteval
template <int N>
consteval auto GetPrimes()
{
    constexpr auto sieve = PrimeSieveFn<N>();
    std::array<int, CountPrime(sieve)> result; // Missing initialization but permitted in C++20
    for (size_t i = 2, j = 0; i < sieve.size(); ++i)
        if (sieve[i])
        {
            result[j] = i;
            ++j;
        }
    return result;
}

static_assert(CountPrime(PrimeSieveFn<1>()) == 0);
static_assert(CountPrime(PrimeSieveFn<2>()) == 1);
static_assert(CountPrime(PrimeSieveFn<3>()) == 2);
static_assert(CountPrime(PrimeSieveFn<5>()) == 3);
static_assert(CountPrime(PrimeSieveFn<6>()) == 3);
static_assert(CountPrime(PrimeSieveFn<7>()) == 4);

int main()
{
    const auto primes = GetPrimes<10>();
    for (int p : primes)
        std::cout << p << ", ";
    std::cout << '\n';

    return 0;
}