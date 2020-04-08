#include "mod-vector.h"

#include <cxxtest/TestSuite.h>

class FooModule : public ModuleType<FooModule> {
public:
    FooModule()
    {
        in.name("in");
        out.name("out");
        ports(in, out);
    }
    Input<> in;
    Output<> out;
    void render(size_t) {}
};

class ModVectorUnitTest : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        mod_vector();
    }

    void test_populate()
    {
        FooModule foo1, foo2;
        mod_vector v;
        v.push_back(&foo1);
        v.push_back(&foo2);

        TS_ASSERT(v.size() == 2);
        TS_ASSERT(v.m_modules[0] == &foo1);
        TS_ASSERT(v.m_modules[1] == &foo2);
    }

    void test_iterate()
    {
        FooModule foo1, foo2;
        mod_vector v;
        v.push_back(&foo1);
        v.push_back(&foo2);

        mod_vector::const_iterator b = v.begin();
        mod_vector::const_iterator e = v.end();
        mod_vector::const_iterator i = b;
        TS_ASSERT(i != e);
        TS_ASSERT(*i == &foo1);
        i++;
        TS_ASSERT(i != e);
        TS_ASSERT(*i == &foo2);
        ++i;
        TS_ASSERT(i == e);
    }

    void test_access()
    {
        FooModule foo1, foo2;
        mod_vector v;
        v.push_back(&foo1);
        v.push_back(&foo2);

        TS_ASSERT(v.at(0) == &foo1);
        TS_ASSERT(v.at(1) == &foo2);
        TS_ASSERT_THROWS(v.at(2), std::out_of_range);
    }

    void test_index()
    {
        FooModule foo1, foo2, foo3;
        mod_vector v;
        v.push_back(&foo1);
        v.push_back(&foo2);

        TS_ASSERT(v.index(&foo1) == 0);
        TS_ASSERT(v.index(&foo2) == 1);
        TS_ASSERT_THROWS(v.index(&foo3), std::domain_error);
    }

    void test_ports()
    {
        FooModule foo1, foo2;
        mod_vector v;
        v.push_back(&foo1);
        v.push_back(&foo2);

        const mod_vector::port_vector p = v.ports();
        TS_ASSERT(p.at(0) == &foo1.in);
        TS_ASSERT(p.at(1) == &foo1.out);
        TS_ASSERT(p.at(2) == &foo2.in);
        TS_ASSERT(p.at(3) == &foo2.out);
        TS_ASSERT_THROWS(p.at(4), std::domain_error);
    }

};
