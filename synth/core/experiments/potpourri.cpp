// #include <cstddef>
#include <functional>
// #include <iostream>
// #include <type_traits>

// typedef double thing;
//
// std::function<thing(thing)> make_lambda(thing& it)
// {
//     return [&] (thing inc) {
//         std::cout << "\xce\xbb: *" << (void *)&it << " += " << inc << std::endl;
//         it += inc;
//         std::cout << "   => " << it << std::endl;
//         return it;
//     };
// }

template <class T>
constexpr std::function<T(const T&)> make_inc(T& it)
{
    return [&] (const T& inc) -> T {
        return it += inc;
    };
}

// template <class T>
// class C {
// public:
//     C() : member(0) {}
//     virtual ~C() = default;
//     T member;
// };
//
// void type_stuff()
// {
//     auto d = C<double>();
//     auto cd = C<const double>();
//     std::cout << std::boolalpha;
//     std::cout << "const vs mutable: "
//               << (typeid(d.member) == typeid(cd.member))
//               << std::endl;
//     std::cout << "classes: "
//               << (typeid(d) == typeid(cd))
//               << std::endl;
//     std::cout << std::endl;
// }

long fred = 35;
auto inc_fred = make_inc<long>(fred);

int main()
{
    inc_fred(2);
    return fred != 37;
    // std::cout << "sizeof (std::intptr_t() = "
    //           << sizeof (std::intptr_t)
    //           << std::endl;
    // std::cout << "sizeof inc_fred = " << sizeof inc_fred << std::endl;
    // std::cout << "&fred = " << (void *)&fred << std::endl;
    // std::cout << std::endl;
    // std::intptr_t *p =
    //     static_cast<std::intptr_t *>(static_cast<void *>(&inc_fred));
    // for (size_t i = 0; i < sizeof inc_fred / sizeof *p; i++)
    //     std::cout << i << ": " << std::hex << p[i] << std::endl;
    // std::cout << std::dec;
    // std::cout << std::endl;
    // type_stuff();
    // thing x = 1.0, y = 2.0;
    // std::cout << "&x = " << (void *)&x << std::endl;
    // std::cout << "&y = " << (void *)&y << std::endl;
    // auto ix0 = make_lambda(x);
    // auto ix1 = make_inc<thing>(x);
    // std::cout << "sizeof ix0 = " << sizeof ix0 << std::endl;
    // std::cout << "sizeof ix1 = " << sizeof ix1 << std::endl;
    // auto iy0 = make_lambda(y);
    // auto iy1 = make_lambda(y);
    // auto x1 = ix0(1.1);
    // auto x2 = ix1(0.3);
    // std::cout << "x1, x2 = " << x1 << ", " << x2 << std::endl;
    // return 0;
}
