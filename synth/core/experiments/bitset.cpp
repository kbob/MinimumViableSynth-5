#include <array>
#include <bitset>

#define RRR __restrict
// #define RRR

// class C {
// public:
//
//     void method(size_t n) RRR
//     {
//         for (size_t i = 0; i < n; i++)
//             a[i] = b[i] * c[i];
//     }
//
//     double *__restrict a, *__restrict b, *__restrict c;
//
// };

void jeeter(double *RRR a, double *RRR b, double *RRR c)
{
    for (size_t i = 0; i < 256; i++)
        a[i] = b[i] * c[i];
}

// int foo(C& c)
// {
//     c.method(256);
//     return 0;
// }

std::bitset<1000003> my_bits;

int main()
{
    my_bits[12345] = true;
    return 0;
}
