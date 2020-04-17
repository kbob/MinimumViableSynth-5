#include "links.h"

#include <cxxtest/TestSuite.h>

class LinksUnitTest : public CxxTest::TestSuite {

public:

    typedef double D;
    typedef short S;
    typedef char C;

    class ConcreteControl : public ControlType<char> {};

    Input<D> dest;
    Output<S> src;
    Output<D> dsrc;
    ConcreteControl ctl;

    void test_instantiate_DSC()
    {
        auto link = make_link(&dest, &src, &ctl);

        TS_ASSERT(link.dest() == &dest);
        TS_ASSERT(link.src() == &src);
        TS_ASSERT(link.ctl() == &ctl.out);
        TS_ASSERT(link.scale() == 1.0f);
    }

    void test_instantiate_DSO()
    {
        auto link = make_link(&dest, &src, &ctl.out);

        TS_ASSERT(link.dest() == &dest);
        TS_ASSERT(link.src() == &src);
        TS_ASSERT(link.ctl() == &ctl.out);
        TS_ASSERT(link.scale() == 1.0f);
    }

    void test_instantiate_DC()
    {
        auto link = make_link(&dest, nullptr, &ctl);

        TS_ASSERT(link.dest() == &dest);
        TS_ASSERT(link.src() == nullptr);
        TS_ASSERT(link.ctl() == &ctl.out);
        TS_ASSERT(link.scale() == 1.0f);
    }

    void test_instantiate_DO()
    {
        auto link = make_link(&dest, nullptr, &ctl.out);

        TS_ASSERT(link.dest() == &dest);
        TS_ASSERT(link.src() == nullptr);
        TS_ASSERT(link.ctl() == &ctl.out);
        TS_ASSERT(link.scale() == 1.0f);
    }

    void test_instantiate_DS()
    {
        auto link = make_link(&dest, &src, nullptr);

        TS_ASSERT(link.dest() == &dest);
        TS_ASSERT(link.src() == &src);
        TS_ASSERT(link.ctl() == nullptr);
        TS_ASSERT(link.scale() == 1.0f);
    }

    void test_instantiate_D()
    {
        auto link = make_link(&dest, nullptr, nullptr);

        TS_ASSERT(link.dest() == &dest);
        TS_ASSERT(link.src() == nullptr);
        TS_ASSERT(link.ctl() == nullptr);
        TS_ASSERT(link.scale() == 1.0f);
    }

    void test_instantiate_scale()
    {
        auto l1 = make_link(&dest, &src,    &ctl,     0.1f);
        auto l2 = make_link(&dest, &src,    &ctl.out, 0.2f);
        auto l3 = make_link(&dest, nullptr, &ctl,     0.3f);
        auto l4 = make_link(&dest, nullptr, &ctl,     0.4f);
        auto l5 = make_link(&dest, &src,    nullptr,  0.5f);
        auto l6 = make_link(&dest, nullptr, nullptr,  0.6f);

        TS_ASSERT(l1.scale() == 0.1f);
        TS_ASSERT(l2.scale() == 0.2f);
        TS_ASSERT(l3.scale() == 0.3f);
        TS_ASSERT(l4.scale() == 0.4f);
        TS_ASSERT(l5.scale() == 0.5f);
        TS_ASSERT(l6.scale() == 0.6f);
    }

    void test_simple()
    {
        auto l1 = make_link(&dest, &dsrc,   nullptr, 1.0f);
        auto l2 = make_link(&dest, nullptr, nullptr, 1.0f);
        auto l3 = make_link(&dest, &src,    nullptr, 1.0f);
        auto l4 = make_link(&dest, &dsrc,   &dsrc,   1.0f);
        auto l5 = make_link(&dest, &dsrc,   nullptr, 0.5f);

        TS_ASSERT(l1.is_simple());
        TS_ASSERT(not l2.is_simple());
        TS_ASSERT(not l3.is_simple());
        TS_ASSERT(not l4.is_simple());
        TS_ASSERT(not l5.is_simple());
    }

};
