#include "relation.h"

#include <cxxtest/TestSuite.h>

class relation_unit_test : public CxxTest::TestSuite {

public:

    typedef float V1;
    typedef char V2;
    static const size_t N1 = 3;
    static const size_t N2 = 5;
    typedef Universe<V1, N1> U1;
    typedef Universe<V2, N2> U2;
    typedef Relation<V1, V2, N1, N2> REL;

    const U1::referent ref1;
    const U2::referent ref2;
    const U1 u1;
    const U2 u2;
    REL rel;

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
        (void)Relation<V1, V2, N1, N2>{u1, u2};
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
        TS_ASSERT(rel.at(0) == u2.subset(0b01001));
        TS_ASSERT(rel.at(1) == u2.subset(0b10010));
        TS_ASSERT(rel.at(2) == u2.subset(0b00100));
        TS_ASSERT_THROWS(rel.at(3), std::logic_error);
    }

    void test_get()
    {
        TS_ASSERT(rel.get(1.1f) == u2.subset(0b01001));
        TS_ASSERT(rel.get(2.2f) == u2.subset(0b10010));
        TS_ASSERT(rel.get(3.3f) == u2.subset(0b00100));
        TS_ASSERT_THROWS(rel.get(4.4f), std::logic_error);
    }

};
