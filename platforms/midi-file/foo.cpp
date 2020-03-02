#include <iostream>

#include "byte-span.h"

void printAddress(const int& v)
{
    std::cout << reinterpret_cast<const void *>(&v) << std::endl;
}

int x;

int getInt()
{
    return x;
}

int&& getRvalueInt()
{
    return std::move(x);
}

int main()
{
    printAddress(getInt());
    printAddress(x);
    printAddress(getRvalueInt());


    const uint8_t v[] = { '1', '2', '3', '4', '5' };

    for (auto  i : v)
        std::cout << i;
    std::cout << std::endl;

    const byte_span s(v, sizeof v);
    for (int i = 0; i < s.size(); i++)
        std::cout << s[i];
    std::cout << std::endl;

    for (auto i : s)
        std::cout << i;
    std::cout << std::endl;

    for (auto i : s.subspan(1, 3))
        std::cout << i;
    std::cout << std::endl;

    // decltype(nullptr) a_ptr;
    // a_ptr = nullptr;
    // void *vp = a_ptr;
    void *vp = nullptr;
    std::cout << vp << std::endl;
    std::cout << static_cast<const void *>(nullptr) << std::endl;
    std::cout << (const void *)nullptr << std::endl;

    auto is_little_endian = []() {
        std::uint16_t x = 0x0001;
        return *reinterpret_cast<bool *>(&x);
    };
    auto is_big_endian = []() {
        std::uint16_t x = 0x0100;
        return *reinterpret_cast<bool *>(&x);
    };
    std::cout << is_little_endian() << ' ' << is_big_endian() << std::endl;

    return 0;
}
