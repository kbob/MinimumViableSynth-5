#include <cstdint>
#include <iostream>

std::int32_t gcd(std::int32_t a, std::int32_t b)
{
    if (b == 0)
        return a;
    return gcd(b, a % b);
}

std::int64_t lcm(std::int32_t a, std::int32_t b)
{
    return a / gcd(a, b) * static_cast<std::int64_t>(b);
}

int main()
{
    // for (int a = 2; a < 10; a += 2)
    //     for (int b = 3; b < 12; b += 3)
    //         std::cout << "gcd("
    //                   << a
    //                   << ' '
    //                   << b
    //                   << ") = "
    //                   << gcd(a, b)
    //                   << '\n';

    std::cout << "User-preferred locale setting is " << std::locale("").name() << '\n';
    std::cout.imbue(std::locale(""));
    for (auto a: {44100, 48000, 96000, 192000})
        for (auto b: {1000, 1000000, 1048576})
            std::cout << "lcm("
                      << a
                      << ' '
                      << b
                      << ") = "
                      << lcm(a, b)
                      << '\n';
    return 0;
}
