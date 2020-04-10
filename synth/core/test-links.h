#include "links.h"

#include <cxxtest/TestSuite.h>

enum class Color {};
enum class Shape {};
enum class Emotion {};
enum class Quality {};

class ConcreteControl : public ControlType<char> {};

class LinksUnitTest : public CxxTest::TestSuite {

public:

    void test_instantiate_DSC()
    {
        Input<Color> dest;
        Output<Shape> src;
        ConcreteControl ctl;
        auto link = make_link(&dest, &src, &ctl);

        TS_ASSERT(link.dest() == &dest);
        TS_ASSERT(link.src() == &src);
        TS_ASSERT(link.ctl() == &ctl.out);
        TS_ASSERT(link.scale() == 1.0f);
    }

    void test_instantiate_DSO()
    {
        Input<short> dest;
        Output<Quality> src;
        Output<Emotion> out;
        auto link = make_link(&dest, &src, &out);

        TS_ASSERT(link.dest() == &dest);
        TS_ASSERT(link.src() == &src);
        TS_ASSERT(link.ctl() == &out);
        TS_ASSERT(link.scale() == 1.0f);
    }

    void test_instantiate_DC()
    {
        Input<float> dest;
        ConcreteControl ctl;
        auto link = make_link(&dest, nullptr, &ctl);

        TS_ASSERT(link.dest() == &dest);
        TS_ASSERT(link.src() == nullptr);
        TS_ASSERT(link.ctl() == &ctl.out);
        TS_ASSERT(link.scale() == 1.0f);
    }

    void test_instantiate_DO()
    {
        Input<Shape> dest;
        Output<bool> out;
        auto link = make_link(&dest, nullptr, &out);

        TS_ASSERT(link.dest() == &dest);
        TS_ASSERT(link.src() == nullptr);
        TS_ASSERT(link.ctl() == &out);
        TS_ASSERT(link.scale() == 1.0f);
    }

    void test_instantiate_DS()
    {
        Input<float> dest;
        Output<double> src;
        auto link = make_link(&dest, &src, nullptr);

        TS_ASSERT(link.dest() == &dest);
        TS_ASSERT(link.src() == &src);
        TS_ASSERT(link.ctl() == nullptr);
        TS_ASSERT(link.scale() == 1.0f);
    }

    void test_instantiate_D()
    {
        Input<long> dest;
        auto link = make_link(&dest, nullptr, nullptr);

        TS_ASSERT(link.dest() == &dest);
        TS_ASSERT(link.src() == nullptr);
        TS_ASSERT(link.ctl() == nullptr);
        TS_ASSERT(link.scale() == 1.0f);
    }

    void test_instantiate_scale()
    {
        Input<Color> dest;
        Output<Shape> src;
        ConcreteControl ctl;
        Output<Emotion> out;
        auto l1 = make_link(&dest, &src,    &ctl,    0.1f);
        auto l2 = make_link(&dest, &src,    &out,    0.2f);
        auto l3 = make_link(&dest, nullptr, &ctl,    0.3f);
        auto l4 = make_link(&dest, nullptr, &out,    0.4f);
        auto l5 = make_link(&dest, &src,    nullptr, 0.5f);
        auto l6 = make_link(&dest, nullptr, nullptr, 0.6f);

        TS_ASSERT(l1.scale() == 0.1f);
        TS_ASSERT(l2.scale() == 0.2f);
        TS_ASSERT(l3.scale() == 0.3f);
        TS_ASSERT(l4.scale() == 0.4f);
        TS_ASSERT(l5.scale() == 0.5f);
        TS_ASSERT(l6.scale() == 0.6f);
    }

};
