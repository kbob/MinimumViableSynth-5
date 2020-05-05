#include "link.h"

#include <string>
#include <vector>

#include <cxxtest/TestSuite.h>

class link_unit_test : public CxxTest::TestSuite {

public:

    static const size_t N = 4;

    typedef double D;
    typedef short S;
    typedef char C;

    class ConcreteControl : public ControlType<ConcreteControl, C> {
    public:
        void render(size_t) {}
    };

    Input<D> dest, dest0;
    Output<S> src, src0;
    Output<D> dsrc;
    ConcreteControl ctl, ctl0;

    void load_data()
    {
        dest.clear(D(42));
        for (size_t i = 0; i < N; i++) {
            src[i] = i;
            dsrc[i] = i;
            ctl.out[i] = i;
        }
    }

    void test_size()
    {
        TS_TRACE("sizeof (Link) = " + std::to_string(sizeof (Link)));
    }

    void test_instantiate_DSC()
    {
        Link link{&dest, &src, &ctl.out};

        TS_ASSERT_EQUALS(link.dest(), &dest);
        TS_ASSERT_EQUALS(link.src(), &src);
        TS_ASSERT_EQUALS(link.ctl(), &ctl.out);
        TS_ASSERT_EQUALS(link.scale(), 1.0f);
    }

    void test_instantiate_DC()
    {
        Link link{&dest, nullptr, &ctl.out};

        TS_ASSERT_EQUALS(link.dest(), &dest);
        TS_ASSERT_EQUALS(link.src(), nullptr);
        TS_ASSERT_EQUALS(link.ctl(), &ctl.out);
        TS_ASSERT_EQUALS(link.scale(), 1.0f);
    }

    void test_instantiate_DS()
    {
        Link link{&dest, &src, nullptr};

        TS_ASSERT_EQUALS(link.dest(), &dest);
        TS_ASSERT_EQUALS(link.src(), &src);
        TS_ASSERT_EQUALS(link.ctl(), nullptr);
        TS_ASSERT_EQUALS(link.scale(), 1.0f);
    }

    void test_instantiate_D()
    {
        Link link{&dest, nullptr, nullptr};

        TS_ASSERT_EQUALS(link.dest(), &dest);
        TS_ASSERT_EQUALS(link.src(), nullptr);
        TS_ASSERT_EQUALS(link.ctl(), nullptr);
        TS_ASSERT_EQUALS(link.scale(), 1.0f);
    }

    void test_instantiate_scale()
    {
        Link l1{&dest, &src,    &ctl.out, 0.1f};
        Link l2{&dest, nullptr, &ctl.out, 0.2f};
        Link l3{&dest, &src,    nullptr,  0.3f};
        Link l4{&dest, nullptr, nullptr,  0.4f};

        TS_ASSERT_EQUALS(l1.scale(), 0.1f);
        TS_ASSERT_EQUALS(l2.scale(), 0.2f);
        TS_ASSERT_EQUALS(l3.scale(), 0.3f);
        TS_ASSERT_EQUALS(l4.scale(), 0.4f);
    }

    void test_copy()
    {
        Link link{&dest0, &src0, &ctl0.out, 0.5f};
        Link l2(link);
        TS_ASSERT_EQUALS(l2.dest(), &dest0);
        TS_ASSERT_EQUALS(l2.src(), &src0);
        TS_ASSERT_EQUALS(l2.ctl(), &ctl0.out);
        TS_ASSERT_EQUALS(l2.scale(), 0.5f);

        load_data();
        render_action copy = l2.make_copy_action(&dest, &src, &ctl.out);
        copy(2);
        std::vector<D> actual1{dest.buf(), dest.buf() + N};
        std::vector<D> expected1{0.0, 0.5, 42, 42};
        TS_ASSERT_EQUALS(actual1, expected1);
    }

    void test_assignment()
    {
        Input<double> other_dest;
        Link link{&dest0, &src0, &ctl0.out, 0.5f};
        Link l2(&other_dest, nullptr, nullptr);
        l2 = link;
        TS_ASSERT_EQUALS(l2.dest(), &dest0);
        TS_ASSERT_EQUALS(l2.src(), &src0);
        TS_ASSERT_EQUALS(l2.ctl(), &ctl0.out);
        TS_ASSERT_EQUALS(l2.scale(), 0.5f);

        load_data();
        render_action copy = l2.make_copy_action(&dest, &src, &ctl.out);
        copy(2);
        std::vector<D> actual1{dest.buf(), dest.buf() + N};
        std::vector<D> expected1{0.0, 0.5, 42, 42};
        TS_ASSERT_EQUALS(actual1, expected1);
    }

    void test_simple()
    {
        Link l1{&dest, &dsrc,   nullptr, 1.0f};
        Link l2{&dest, nullptr, nullptr, 1.0f};
        Link l3{&dest, &src,    nullptr, 1.0f};
        Link l4{&dest, &dsrc,   &dsrc,   1.0f};
        Link l5{&dest, &dsrc,   nullptr, 0.5f};

        TS_ASSERT(l1.is_simple());
        TS_ASSERT(not l2.is_simple());
        TS_ASSERT(not l3.is_simple());
        TS_ASSERT(not l4.is_simple());
        TS_ASSERT(not l5.is_simple());
    }

    void test_ctl_simple()
    {
        Link l1{&dest, nullptr, &dsrc,    1.0f};
        Link l2{&dest, nullptr, nullptr,  1.0f};
        Link l3{&dest, nullptr, &ctl.out, 1.0f};
        Link l4{&dest, &dsrc,   &dsrc,    1.0f};
        Link l5{&dest, nullptr, &dsrc,    0.5f};

        TS_ASSERT(l1.is_ctl_simple());
        TS_ASSERT(not l2.is_ctl_simple());
        TS_ASSERT(not l3.is_ctl_simple());
        TS_ASSERT(not l4.is_ctl_simple());
        TS_ASSERT(not l5.is_ctl_simple());
    }

    void test_copy_add_DSrcCScale()
    {
        load_data();
        Link link{&dest0, &src0, &ctl0.out, 0.5f};
        render_action copy = link.make_copy_action(&dest, &src, &ctl.out);
        render_action add = link.make_add_action(&dest, &src, &ctl.out);

        copy(2);
        std::vector<D> actual1{dest.buf(), dest.buf() + N};
        std::vector<D> expected1{0.0, 0.5, 42, 42};
        TS_ASSERT_EQUALS(actual1, expected1);
        add(3);
        std::vector<D> actual2{dest.buf(), dest.buf() + N};
        std::vector<D> expected2{0.0, 1.0, 44.0, 42};
        TS_ASSERT_EQUALS(actual2, expected2);
    }

    void test_copy_add_DSrcC()
    {
        load_data();
        Link link{&dest0, &src0, &ctl0.out};
        render_action copy = link.make_copy_action(&dest, &src, &ctl.out);
        render_action add = link.make_add_action(&dest, &src, &ctl.out);

        copy(2);
        std::vector<D> actual1{dest.buf(), dest.buf() + N};
        std::vector<D> expected1{0.0, 1, 42, 42};
        TS_ASSERT_EQUALS(actual1, expected1);
        add(3);
        std::vector<D> actual2{dest.buf(), dest.buf() + N};
        std::vector<D> expected2{0.0, 2.0, 46.0, 42};
        TS_ASSERT_EQUALS(actual2, expected2);
    }

    void test_copy_add_DCScale()
    {
        load_data();
        Link link{&dest0, nullptr, &ctl0.out, 0.5f};
        render_action copy = link.make_copy_action(&dest, nullptr, &ctl.out);
        render_action add = link.make_add_action(&dest, nullptr, &ctl.out);

        copy(2);
        std::vector<D> actual1{dest.buf(), dest.buf() + N};
        std::vector<D> expected1{0.0, 0.5, 42, 42};
        TS_ASSERT_EQUALS(actual1, expected1);
        add(3);
        std::vector<D> actual2{dest.buf(), dest.buf() + N};
        std::vector<D> expected2{0.0, 1.0, 43.0, 42};
        TS_ASSERT_EQUALS(actual2, expected2);
    }

    void test_copy_add_DC()
    {
        load_data();
        Link link{&dest0, nullptr, &ctl0.out};
        render_action copy = link.make_copy_action(&dest, nullptr, &ctl.out);
        render_action add = link.make_add_action(&dest, nullptr, &ctl.out);

        copy(2);
        std::vector<D> actual1{dest.buf(), dest.buf() + N};
        std::vector<D> expected1{0.0, 1, 42, 42};
        TS_ASSERT_EQUALS(actual1, expected1);
        add(3);
        std::vector<D> actual2{dest.buf(), dest.buf() + N};
        std::vector<D> expected2{0.0, 2.0, 44.0, 42};
        TS_ASSERT_EQUALS(actual2, expected2);
    }

    void test_copy_add_DSrc()
    {
        load_data();
        Link link{&dest0, &src0, nullptr, 1.0f};
        render_action copy = link.make_copy_action(&dest, &src, nullptr);
        render_action add = link.make_add_action(&dest, &src, nullptr);

        copy(2);
        std::vector<D> actual1{dest.buf(), dest.buf() + N};
        std::vector<D> expected1{0.0, 1, 42, 42};
        TS_ASSERT_EQUALS(actual1, expected1);
        add(3);
        std::vector<D> actual2{dest.buf(), dest.buf() + N};
        std::vector<D> expected2{0.0, 2.0, 44.0, 42};
        TS_ASSERT_EQUALS(actual2, expected2);
    }

    void test_copy_add_DSrcScale()
    {
        load_data();
        Link link{&dest0, &src0, nullptr, 0.5f};
        render_action copy = link.make_copy_action(&dest, &src, nullptr);
        render_action add = link.make_add_action(&dest, &src, nullptr);

        copy(2);
        std::vector<D> actual1{dest.buf(), dest.buf() + N};
        std::vector<D> expected1{0.0, 0.5, 42, 42};
        TS_ASSERT_EQUALS(actual1, expected1);
        add(3);
        std::vector<D> actual2{dest.buf(), dest.buf() + N};
        std::vector<D> expected2{0.0, 1.0, 43.0, 42};
        TS_ASSERT_EQUALS(actual2, expected2);
    }

    void test_copy_add_Dscale()
    {
        load_data();
        Link link{&dest0, nullptr, nullptr, 0.5f};
        render_action copy = link.make_copy_action(&dest, nullptr, nullptr);
        render_action add = link.make_add_action(&dest, nullptr, nullptr);

        copy(2);
        std::vector<D> actual1{dest.buf(), dest.buf() + N};
        std::vector<D> expected1{0.5, 0.5, 42, 42};
        TS_ASSERT_EQUALS(actual1, expected1);
        add(3);
        std::vector<D> actual2{dest.buf(), dest.buf() + N};
        std::vector<D> expected2{1.0, 1.0, 42.5, 42};
        TS_ASSERT_EQUALS(actual2, expected2);
    }

    void test_copy_add_D()
    {
        load_data();
        Link link{&dest0, nullptr, nullptr, 1};
        render_action copy = link.make_copy_action(&dest, nullptr, nullptr);
        render_action add = link.make_add_action(&dest, nullptr, nullptr);

        copy(2);
        std::vector<D> actual1{dest.buf(), dest.buf() + N};
        std::vector<D> expected1{1, 1, 42, 42};
        TS_ASSERT_EQUALS(actual1, expected1);
        add(3);
        std::vector<D> actual2{dest.buf(), dest.buf() + N};
        std::vector<D> expected2{2, 2, 43, 42};
        TS_ASSERT_EQUALS(actual2, expected2);
    }

};
