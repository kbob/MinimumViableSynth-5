#include <iostream>

#include "byte-span.h"

int main()
{
    uint8_t v[] = { '1', '2', '3', '4', '5' };

    for (auto  i : v)
        std::cout << i;
    std::cout << std::endl;

    byte_span s(v, sizeof v);
    for (int i = 0; i < s.size(); i++)
        std::cout << s[i];
    std::cout << std::endl;

    for (auto i : s)
        std::cout << i;
    std::cout << std::endl;

    for (auto i : s.subspan(1, 3))
        std::cout << i;
    std::cout << std::endl;

    return 0;
}
