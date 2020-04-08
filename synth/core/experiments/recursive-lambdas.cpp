#include <cstdint>
#include <iostream>

void print_addrs(void *a[], size_t count)
{
    for (size_t i = 0; i < count; i++)
        std::cout << "  " << i << ": " << a[i] << std::endl;
}

// iterative factorial, no lambda
uint64_t f0(uint64_t n)
{
    uint64_t p = 1;
    for (uint64_t i = 1; i <= n; i++)
        p *= i;
    return p;
}

// factorial using recursive lambda
uint64_t f1(uint64_t n)
{
    std::function<uint64_t(uint64_t)> f = [&] (uint64_t n) {
        return n ? n * f(n - 1) : 1;
    };
    return f(n);
}

// factorial using tail-recursive lambda
// logs var address to check tail call optimization.  (Does not optimize.)
uint64_t f2(uint64_t n)
{
    void *a[n + 1];
    std::function<uint64_t(uint64_t, uint64_t)>
        f = [&] (uint64_t n, uint64_t p) {
            a[n] = static_cast<void *>(&p);
            if (!n)
                return p;
            return f(n - 1, n * p);
        };
    auto prod = f(n, 1);
    print_addrs(a, n + 1);
    return prod;
}

#if __cplusplus >= 201402L

// C++14 - no std::function, no free variables
uint64_t f3(uint64_t n)
{
    auto f = [] (auto&& self, uint64_t n) -> uint64_t {
        return n ? n * self(self, n - 1) : 1;
    };
    return f(f, n);
}

// C++14 - no std::function, no free variables, tail-recursive
// logs var address to check tail call optimization.  (Does not optimize.)
uint64_t f4(uint64_t n)
{
    void *a[n + 1];
    auto f = [] (auto&& self, uint64_t n, uint64_t p, void **a) -> uint64_t {
        *a = static_cast<void *>(&n);
        return n ? self(self, n - 1, n * p, a + 1) : p;
    };
    auto prod = f(f, n, 1, a);
    print_addrs(a, n + 1);
    return prod;
}

// C++14 - no std::function, no free variables, tail-recursive, no logging.
// Apple clang-1100.0.33.17 actually optimizes away the tail call of this one.
uint64_t f5(uint64_t n)
{
    auto f = [] (auto&& self, uint64_t n, uint64_t p) -> uint64_t {
        return n ? self(self, n - 1, n * p) : p;
    };
    auto prod = f(f, n, 1);
    return prod;
}

#endif

int main()
{
    const uint64_t n = 7;
    uint64_t p0 = f0(n);
    std::cout << n << "! = " << p0 << std::endl;
    uint64_t p1 = f1(n);
    std::cout << n << "! = " << p1 << std::endl;
    uint64_t p2 = f2(n);
    std::cout << n << "! = " << p2 << std::endl;
#if __cplusplus >= 201402L
    uint64_t p3 = f3(n);
    std::cout << n << "! = " << p3 << std::endl;
    uint64_t p4 = f4(n);
    std::cout << n << "! = " << p4 << std::endl;
    uint64_t p5 = f5(n);
    std::cout << n << "! = " << p5 << std::endl;
#endif
    return 0;
}
