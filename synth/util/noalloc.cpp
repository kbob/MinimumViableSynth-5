#include "noalloc.h"

#include <iostream>
#include <vector>

int main()
{
    const size_t N = 5;
    typedef double E;
    typedef vector_pool<E, N> P;
    typedef std::vector<E, P> V;
    V v;
    v.reserve(N);
    try {
        for (size_t i = 0; i <= N; i++)
            v.push_back(i);
    } catch (std::length_error x){
        std::cerr << "length error: " << x.what() << std::endl;
    }

    return 0;
}
