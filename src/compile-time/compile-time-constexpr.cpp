
constexpr int factorial(int n)
{
    return n == 0 ? 1 : n * factorial(n - 1);
}

static_assert(factorial(0) == 1);
static_assert(factorial(1) == 1);
static_assert(factorial(2) == 2);
static_assert(factorial(3) == 6);