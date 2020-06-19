#include <bitset>
#include <iostream>
#include <functional>
#include <limits>
#include <type_traits>


// -- Debugging - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#include <cxxabi.h>

#define HERE (std::cout << __FILE__ << ':' << __LINE__ << std::endl)

// XXX Move this into the platform directory.
static std::string demangle(const std::string& mangled)
{
    int status;
    char *s = abi::__cxa_demangle(mangled.c_str(), 0, 0, &status);
    std::string demangled(s);
    std::free(s);
    return demangled;
}

template <typename T>
static std::string type_name(T& obj)
{
    const std::string& mangled = typeid(*&obj).name();
    return demangle(mangled);
}

template <class T, typename... Types>
std::string arglist_types(T first, Types&... rest)
{
    return type_name(first) + ", " + arglist_types(rest...);
}

template <class T>
std::string arglist_types(T last)
{
    return type_name(last);
}


// template <size_t N> class mask;

// template <size_t N>
//     using mask =
//         typename std::enable_if<
//             N <= std::numeric_limits<std::uint8_t>::digits,
//             std::uint8_t
//         >::type;

// template <size_t N>
//     using mask =
//         typename std::enable_if<
//             !(N <= std::numeric_limits<std::uint8_t>::digits) &&
//             N <= std::numeric_limits<std::uint16_t>::digits,
//             std::uint16_t
//         >::type;


template <size_t N>
    using mask =
        typename std::conditional<
            N <= std::numeric_limits<std::uint8_t>::digits,
            std::uint8_t,
            typename std::conditional<
                N <= std::numeric_limits<std::uint16_t>::digits,
                std::uint16_t,
                // std::uint32_t
                typename std::conditional<
                    N <= std::numeric_limits<std::uint32_t>::digits,
                    std::uint32_t,
                    typename std::enable_if<
                        N <= std::numeric_limits<std::uint64_t>::digits,
                        std::uint64_t
                    >::type
                >::type
            >::type
        >::type;

int main()
{
    mask<0> m0;
    mask<1> m1;
    mask<8> m8;
    mask<9> m9;
    mask<16> m16;
    mask<17> m17;
    mask<32> m32;
    mask<33> m33;
    mask<64> m64;
    std::cout << " 0 : " << sizeof m0 << ' ' << type_name(m0) << '\n';
    std::cout << " 1 : " << sizeof m1 << ' ' << type_name(m1) << '\n';
    std::cout << " 8 : " << sizeof m8 << ' ' << type_name(m8) << '\n';
    std::cout << " 9 : " << sizeof m9 << ' ' << type_name(m9) << '\n';
    std::cout << "16 : " << sizeof m16 << ' ' << type_name(m16) << '\n';
    std::cout << "17 : " << sizeof m17 << ' ' << type_name(m17) << '\n';
    std::cout << "32 : " << sizeof m32 << ' ' << type_name(m32) << '\n';
    std::cout << "33 : " << sizeof m33 << ' ' << type_name(m33) << '\n';
    std::cout << "64 : " << sizeof m64 << ' ' << type_name(m64) << '\n';
    return 0;
}
