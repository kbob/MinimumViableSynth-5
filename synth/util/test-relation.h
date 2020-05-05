#include "relation.h"

#include <deque>
#include <stdexcept>
#include <vector>

#include <cxxtest/TestSuite.h>

class relation_unit_test : public CxxTest::TestSuite {

public:

    typedef float V1;
    typedef char V2;
    static const size_t N1 = 3;
    static const size_t N2 = 5;
    typedef std::vector<V1> C1;
    typedef std::deque<V2> C2;
    typedef Universe<C1, N1> U1;
    typedef Universe<C2, N2> U2;
    typedef Relation<U1, U2> R;

    const U1::referent ref1;
    const U2::referent ref2;
    const U1 u1;
    const U2 u2;
    R rel;

    relation_unit_test()
    : ref1{1.1f, 2.2f, 3.3f},
      ref2{'a', 'b', 'c', 'd', 'e'},
      u1{ref1},
      u2{ref2},
      rel{u1, u2}
    {
        rel.add(1.1f, 'a');
        rel.add(2.2f, 'b');
        rel.add(3.3f, 'c');
        rel.add(1.1f, 'd');
        rel.add(2.2f, 'e');

        // std::cout << "\n\n";
        // for (size_t i = 0; i < N1; i++)
        //     std::cout << u1[i] << ": " << rel.at(i) << "\n";
        // std::cout << std::endl;
    }

    void test_instantiation()
    {
        const U1::referent ref1{1.1f, 2.2f, 3.3f};
        const U1 u1{ref1};
        const U2::referent ref2{'a', 'b', 'c', 'd', 'e'};
        const U2 u2{ref2};
        (void)Relation<U1, U2>{u1, u2};
    }

    void test_containment()
    {
        TS_ASSERT(rel.contains(1.1f, 'a'));
        TS_ASSERT(!rel.contains(2.2f, 'c'));
        TS_ASSERT_THROWS(rel.contains(4.4f, 'a'), std::logic_error);
        TS_ASSERT_THROWS(rel.contains(3.3f, 'f'), std::logic_error);
    }

    void test_at()
    {
        TS_ASSERT_EQUALS(rel.at(0), u2.subset(0b01001));
        TS_ASSERT_EQUALS(rel.at(1), u2.subset(0b10010));
        TS_ASSERT_EQUALS(rel.at(2), u2.subset(0b00100));
        TS_ASSERT_THROWS(rel.at(3), std::logic_error);
    }

    void test_get()
    {
        TS_ASSERT_EQUALS(rel.get(1.1f), u2.subset(0b01001));
        TS_ASSERT_EQUALS(rel.get(2.2f), u2.subset(0b10010));
        TS_ASSERT_EQUALS(rel.get(3.3f), u2.subset(0b00100));
        TS_ASSERT_THROWS(rel.get(4.4f), std::logic_error);
    }

};
