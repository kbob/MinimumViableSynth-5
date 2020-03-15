#include "synth/util/vec-map.h"

#include <cxxtest/TestSuite.h>

class vec_map_test : public CxxTest::TestSuite {

public:

    typedef int K;
    typedef float V;
    typedef vec_map<K, V> VM;
    typedef std::pair<const K, V> P;

    void testEmpty()
    {
        VM vm;

        TS_ASSERT(vm.size() == 0);
        TS_ASSERT(vm.empty() == true);
        TS_ASSERT(vm.begin() == vm.end());
        TS_ASSERT(vm.cbegin() == vm.cend());
        TS_ASSERT(vm.rbegin() == vm.rend());
        TS_ASSERT(vm.crbegin() == vm.crend());
    }

    void testConstEmpty()
    {
        const VM cvm;

        TS_ASSERT(cvm.size() == 0);
        TS_ASSERT(cvm.empty() == true);
        TS_ASSERT(cvm.begin() == cvm.end());
        TS_ASSERT(cvm.cbegin() == cvm.cend());
        TS_ASSERT(cvm.rbegin() == cvm.rend());
        TS_ASSERT(cvm.crbegin() == cvm.crend());
    }

    void testInsert()
    {
        VM vm;
        vm[13] = 1.13f;

        TS_ASSERT(vm.size() == 1);
        TS_ASSERT(vm.empty() == false);
        TS_ASSERT(vm[13] == 1.13f);
    }

    void testCapacity()
    {
        VM vm;
        vm[1] = 1.1f;
        vm[2] = 1.2f;
        vm[13] = 1.13f;

        TS_ASSERT(vm.size() == 3);
        TS_ASSERT(vm.capacity() >= 3);
    }

    void testReserve()
    {
        VM vm;
        vm[1] = 1.1f;
        vm[2] = 1.2f;
        vm[13] = 1.13f;
        vm.reserve(100);

        TS_ASSERT(vm.capacity() == 100);
        TS_ASSERT(vm.size() == 3);
    }

    void testShrink()
    {
        VM vm;
        vm[1] = 1.1f;
        vm[2] = 1.2f;
        vm[13] = 1.13f;

        vm.shrink_to_fit();
        TS_ASSERT(vm.capacity() == 3);
        TS_ASSERT(vm.size() == 3);
    }

    void testConstIter()
    {
        VM vm;
        vm[13] = 1.13f;
        vm[2] = 1.2f;

        VM::const_iterator b = vm.cbegin();
        VM::const_iterator e = vm.cend();
        TS_ASSERT(b == b);
        TS_ASSERT(e == e);
        TS_ASSERT(b != e);
    }

    void testConstIter2()
    {
        VM vm;
        vm[13] = 1.13f;
        vm[2] = 1.2f;

        VM::const_iterator a;   // use default constructor
        a = vm.cbegin();        // use assignment
        VM::const_iterator a1 = a++;
        VM::const_iterator a2 = ++a;
        TS_ASSERT(a1 == vm.cbegin());
        TS_ASSERT(a2 == vm.cend());
    }

    void testConstIter3()
    {
        VM vm;
        vm[13] = 1.13f;
        vm[2] = 1.2f;

        VM::const_iterator a = vm.cbegin();
        TS_ASSERT(*a == P(13, 1.13f));
        TS_ASSERT(a->first == 13);
        TS_ASSERT(a->second == 1.13f);
        a++;
        TS_ASSERT(*a == P(2, 1.2f));
    }

    void testConstIter4()
    {
        VM vm;
        vm[13] = 1.13f;
        vm[2] = 1.2f;

        auto a = vm.cend();
        auto a1 = --a;
        TS_ASSERT(*a == P(2, 1.2f));
        auto a2 = a--;
        TS_ASSERT(*a == P(13, 1.13f));
        TS_ASSERT(a1 == a2);
        TS_ASSERT(a1->first == 2);
    }

    void testIter()
    {
        VM vm;
        vm[13] = 1.13f;
        vm[2] = 1.2f;

        VM::iterator a = vm.begin();
        (++a)->second = 2.2f;
        a = vm.begin();
        (a++)->second = 3.13f;
        TS_ASSERT(*vm.begin() == P(13, 3.13f));
        TS_ASSERT(*a == P(2, 2.2f));
    }

    void testReverseIter()
    {
        VM vm;
        vm[13] = 1.13f;
        vm[2] = 1.2f;

        VM::const_reverse_iterator a = vm.crbegin();
        TS_ASSERT(*a == P(2, 1.2f));
        a++;
        TS_ASSERT(*a == P(13, 1.13f));
        ++a;
        TS_ASSERT(a == vm.crend());
    }

    void testKeyCompare()
    {
        VM vm;
        vm.finalize();
        const VM& cvm = vm;

        VM::key_compare kc = cvm.key_comp();
        K a = 13, b = 2;
        TS_ASSERT(kc(a, a) == false);
        TS_ASSERT(kc(a, b) == false);
        TS_ASSERT(kc(b, a) == true);
    }

    void testValueCompare()
    {
        VM vm;
        vm.finalize();
        const VM& cvm = vm;

        VM::value_compare vc = cvm.value_comp();
        P a(13, 1.13f), b(2, 1.13f), c(13, -2.31f);

        TS_ASSERT(vc(a, a) == false);
        TS_ASSERT(vc(a, b) == false);
        TS_ASSERT(vc(b, a) == true);
        TS_ASSERT(vc(a, c) == false);
        TS_ASSERT(vc(c, a) == false);
        TS_ASSERT(vc(b, c) == true);
        TS_ASSERT(vc(c, b) == false);
    }

    void testAt()
    {
        VM vm;
        vm[13] = 1.13f;
        vm[2] = 1.2f;
        vm.finalize();
        const VM& cvm = vm;

        TS_ASSERT(vm.at(13) == 1.13f);
        TS_ASSERT(cvm.at(2) == 1.2f);
    }

    void testFind()
    {
        VM vm;
        vm[13] = 1.13f;
        vm[2] = 1.2f;
        vm.finalize();
        const VM& cvm = vm;

        VM::iterator a = vm.find(13);
        TS_ASSERT(*a == P(13, 1.13f));

        VM::const_iterator ca = cvm.find(2);
        TS_ASSERT(*ca == P(2, 1.2f));
    }

    void testConst()
    {
        VM vm;
        vm[13] = 1.13f;
        vm[2] = 1.2f;
        vm.finalize();
        const VM& cvm = vm;

        TS_ASSERT(cvm.count(2) == 1);
        TS_ASSERT(cvm.count(3) == 0);
    }

};
