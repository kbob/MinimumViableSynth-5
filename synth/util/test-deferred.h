#include "deferred.h"

#include <sstream>

#include <cxxtest/TestSuite.h>

class Logged {

public:

    static std::stringstream logs;
    static size_t count;

    static void reset()
    {
        logs.str("");
        count = 0;
    }
    static std::string log()
    {
        return logs.str();
    }
    Logged()
    : id{count++}
    {
        logs << 'd' << id << ' ';
    }
    Logged(int arg)
    : id{count++}
    {
        logs << 'a' << id << '(' << arg << ") ";
    }
    Logged(float a1, const char *a2)
    : id{count++}
    {
        logs << 'b' << id << '(' << a1 << ", " << a2 << ") ";
    }
    Logged(const Logged& that)
    : id{count++}
    {
        logs << 'c' << id << "<-" << that.id << ' ';
    }

    ~Logged()
    {
        logs << '~' << id << ' ';
    }

    size_t method()
    {
        logs << 'm' << id << ' ';
        return id;
    }

    size_t method() const
    {
        logs << "mc" << id << ' ';
        return id;
    }

    size_t id;
};
std::stringstream Logged::logs;
size_t Logged::count;

class DeferredUnitTest : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)deferred<int>();
    }

    void test_value_type()
    {
        typedef deferred<std::pair<double, short>> D;
        D::value_type Dvt;
        std::pair<double, short> spds;
        TS_ASSERT(typeid(Dvt) == typeid(spds));
    }

    void test_unconstructed()
    {
        Logged::reset();
        {
            deferred<Logged> d;
        }
        TS_ASSERT_EQUALS(Logged::log(), "");
    }

    void test_default_constructor()
    {
        Logged::reset();
        {
            deferred<Logged> d;
            d.construct();
        }
        TS_ASSERT_EQUALS(Logged::log(), "d0 ~0 ");
    }

    void test_constructor_1()
    {
        Logged::reset();
        {
            deferred<Logged> d;
            d.construct(3);
        }
        TS_ASSERT_EQUALS(Logged::log(), "a0(3) ~0 ");
    }

    void test_constructor_2()
    {
        Logged::reset();
        {
            deferred<Logged> d;
            d.construct(3.14f, "foo");
        }
        TS_ASSERT_EQUALS(Logged::log(), "b0(3.14, foo) ~0 ");
    }

    void test_construct_multiple()
    {
        Logged::reset();
        {
            deferred<Logged> d0, d2;
            d0.construct();
            deferred<Logged> d1;
            d1.construct(3.14f, "foo");
            d2.construct(42);
        }
        TS_ASSERT_EQUALS(Logged::log(), "d0 b1(3.14, foo) a2(42) ~1 ~2 ~0 ");
    }

    void test_copy_unconstructed()
    {
        Logged::reset();
        {
            deferred<Logged> d0;
            deferred<Logged> d1(d0);
        }
        TS_ASSERT_EQUALS(Logged::log(), "");
    }

    void test_copy_constructed()
    {
        Logged::reset();
        {
            deferred<Logged> d0;
            d0.construct(42);
            deferred<Logged> d1(d0);
        }
        TS_ASSERT_EQUALS(Logged::log(), "a0(42) c1<-0 ~1 ~0 ");
    }

    void test_is_constructed()
    {
        deferred<int> d;
        TS_ASSERT_EQUALS(d.is_constructed(), false);
        d.construct(42);
        TS_ASSERT_EQUALS(d.is_constructed(), true);
    }

    void test_access()
    {
        Logged::reset();
        {
            deferred<Logged> d0;
            d0.construct();
            size_t id = (*d0).method();
            TS_ASSERT_EQUALS(id, 0);
            id = d0->method();
            TS_ASSERT_EQUALS(id, 0);
        }
        TS_ASSERT_EQUALS(Logged::log(), "d0 m0 m0 ~0 ");
    }

    void test_const_access()
    {
        Logged::reset();
        {
            deferred<Logged> d0;
            d0.construct();
            const deferred<Logged>& ref = d0;
            size_t id = (*ref).method();
            TS_ASSERT_EQUALS(id, 0);
            id = ref->method();
            TS_ASSERT_EQUALS(id, 0);
        }
        TS_ASSERT_EQUALS(Logged::log(), "d0 mc0 mc0 ~0 ");
    }

};
