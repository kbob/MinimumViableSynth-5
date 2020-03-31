#include "noalloc.h"

#include <cxxtest/TestSuite.h>

#include <vector>

class vector_pool_test : public CxxTest::TestSuite {

public:

    typedef double E;
    static const size_t N = 5;
    typedef vector_pool<E, N> P;
    typedef std::vector<E, P> V;

    void test_assignnent()
    {
        V v = {2.0, 4.0, 6.0, 8.0, 10.0};
        TS_ASSERT(v.size() == 5);
        TS_ASSERT(v[0] == 2.0);
        TS_ASSERT(v[1] == 4.0);
        TS_ASSERT(v[2] == 6.0);
        TS_ASSERT(v[3] == 8.0);
        TS_ASSERT(v[4] == 10.0);
        TS_ASSERT_THROWS(v.push_back(12.0), std::length_error);
        TS_ASSERT(v[3] == 8.0);
    }

    void test_push_back()
    {
        V v;
        v.push_back(2.0);
        v.push_back(3.0);
        v.push_back(4.0);
        v.push_back(5.0);
        v.push_back(6.0);
        TS_ASSERT(v.size() == 5);
        TS_ASSERT(v.at(0) == 2.0);
        TS_ASSERT(v.at(1) == 3.0);
        TS_ASSERT(v.at(2) == 4.0);
        TS_ASSERT(v.at(3) == 5.0);
        TS_ASSERT(v.at(4) == 6.0);
    }

    void test_reserve()
    {
        V v;
        v.reserve(N);
        TS_ASSERT(v.capacity() == N);
        TS_ASSERT_THROWS(v.reserve(N + 1), std::length_error);
    }

    void test_emplace()
    {
        V v;
        v.emplace_back(123.456);
        TS_ASSERT(v.size() == 1);
        TS_ASSERT(v.at(0) == 123.456);
    }

    void test_address()
    {
        double d = 0.0;
        const double cd = 1.0;
        double *dp = P().address(d);
        const double *cdp = P().address(cd);
        TS_ASSERT(dp == &d);
        TS_ASSERT(cdp == &cd);
    }

    void test_construct_destroy()
    {
        double d = 4.4;
        float f = 14.14f;

        // Are we sure we know how std::allocator works?
        std::allocator<double> alloc;
        alloc.construct(&d, 5.5);
        TS_ASSERT(d == 5.5)
        alloc.destroy(&d);
        alloc.construct(&f, 15.15f);
        TS_ASSERT(f == 15.15f);
        alloc.destroy((double *)&f);
        TS_ASSERT(d == 5.5);
        TS_ASSERT(f == 15.15f);

        P pool;
        pool.construct(&d, 6.6);
        pool.construct(&f, 16.16f);
        TS_ASSERT(d == 6.6);
        TS_ASSERT(f == 16.16f);
        pool.destroy(&d);
        pool.destroy((double *)&f);
        TS_ASSERT(d == 6.6);
        TS_ASSERT(f == 16.16f);
    }

};
