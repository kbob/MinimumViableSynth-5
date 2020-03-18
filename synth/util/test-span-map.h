#include "span-map.h"

#include <cxxtest/TestSuite.h>

#include <iterator>

class span_map_test : public CxxTest::TestSuite {

public:

    typedef char K;
    typedef float V;
    typedef span_map<K, V> SM;
    typedef mapped_span<K, V> MS;

    void testTypes()
    {
        SM sf;
        sf.finalize();

        SM::key_type k;
        SM::mapped_type m;
        SM::value_type v;
        SM::key_compare kc;
        SM::value_compare vc = sf.value_comp();
        SM::reference r = v;
        SM::const_reference cr = v;
        SM::pointer p;
        SM::const_pointer cp;
        SM::iterator i;
        SM::const_iterator ci;
        SM::reverse_iterator ri;
        SM::const_reverse_iterator cri;
        SM::difference_type d;
        SM::size_type s;

        // attempt to use all those variables.
        k = v.first;
        k = *&k;
        kc = *&kc;
        vc = *&vc;
        k = r.first;
        k = cr.first;
        p = &v;
        cp = &v;
        d = 0;
        s = 0;
    }

    void testEmpty()
    {
        SM s;
        TS_ASSERT(s.size() == 0);
        TS_ASSERT(s.value_size() == 0);
        TS_ASSERT(s.empty());
        TS_ASSERT(s.begin() == s.end());
        TS_ASSERT(s.cbegin() == s.cend());
        TS_ASSERT(s.rbegin() == s.rend());
        TS_ASSERT(s.crbegin() == s.crend());
    }

    void testConstEmpty()
    {
        const SM cs;
        TS_ASSERT(cs.size() == 0);
        TS_ASSERT(cs.value_size() == 0);
        TS_ASSERT(cs.empty());
        TS_ASSERT(cs.begin() == cs.end());
        TS_ASSERT(cs.cbegin() == cs.cend());
        TS_ASSERT(cs.rbegin() == cs.rend());
        TS_ASSERT(cs.crbegin() == cs.crend());
    }

    void testInsert()
    {
        SM s;
        s['m'].push_back(1.13f);
        s['m'].push_back(2.13f);

        TS_ASSERT(s.size() == 1);
        TS_ASSERT(s.value_size() == 2);
        TS_ASSERT(s.empty() == false);

        auto& vr = s['m'];
        TS_ASSERT(!vr.empty());
        TS_ASSERT(vr.size() == 2);
        TS_ASSERT(vr[0] == 1.13f);
        TS_ASSERT(vr[1] == 2.13f);
    }

    void testInsertTwo()
    {
        SM s;
        s['m'].push_back(1.13f);
        s['m'].push_back(2.13f);
        s['b'].push_back(1.2f);

        TS_ASSERT(s.size() == 2);
        TS_ASSERT(s.value_size() == 3);
        TS_ASSERT(s['m'].size() == 2);
        TS_ASSERT(s['b'].size() == 1);
        TS_ASSERT(s['m'][0] == 1.13f);
        TS_ASSERT(s['m'][1] == 2.13f);
        TS_ASSERT(s['b'][0] == 1.2f);
    }

    void testCapacity()
    {
        SM s;
        s['m'].push_back(1.13f);
        s['m'].push_back(2.13f);
        s['b'].push_back(1.2f);

        TS_ASSERT(s.capacity() >= 2);
        TS_ASSERT(s.value_capacity() >= 3);
        TS_ASSERT(s.size() == 2);
        TS_ASSERT(s.value_size() == 3);
    }

    void testReserve()
    {
        SM s;
        s['m'].push_back(1.13f);
        s['m'].push_back(2.13f);
        s['b'].push_back(1.2f);
        s.reserve(100);
        s.values_reserve(200);

        TS_ASSERT(s.capacity() == 100);
        TS_ASSERT(s.value_capacity() == 200);
        TS_ASSERT(s.size() == 2);
        TS_ASSERT(s.value_size() == 3);
    }

    void testShrink()
    {
        SM s;
        s['m'].push_back(1.13f);
        s['m'].push_back(2.13f);
        s['b'].push_back(1.2f);

        s.shrink_to_fit();
        TS_ASSERT(s.size() == 2);
        TS_ASSERT(s.value_size() == 3);
        TS_ASSERT(s.capacity() == 2);
        TS_ASSERT(s.value_capacity() == 3);
    }

    void testConstiter()
    {
        SM s;
        s['m'].push_back(1.13f);
        s['m'].push_back(2.13f);
        s['b'].push_back(1.2f);

        SM::const_iterator b = s.cbegin();
        SM::const_iterator e = s.cend();
        TS_ASSERT(b == b);
        TS_ASSERT(e == e);
        TS_ASSERT(b != e);
        TS_ASSERT(e != b);
    }

    void testConstIter2()
    {
        SM s;
        s['m'].push_back(1.13f);
        s['m'].push_back(2.13f);
        s['b'].push_back(1.2f);

        SM::const_iterator a;   // use default constructor
        a = s.cbegin();         // use assignment
        SM::const_iterator a1 = a++;
        SM::const_iterator a2 = ++a;
        TS_ASSERT(a1 == s.cbegin());
        TS_ASSERT(a2 == s.cend());
    }

    void testConstIter3()
    {
        SM s;
        s['m'].push_back(1.13f);
        s['m'].push_back(2.13f);
        s['b'].push_back(1.2f);

        SM::const_iterator a = s.cbegin();
        auto e = *a;
        TS_ASSERT(e.first == 'm');
        TS_ASSERT(e.second.size() == 2);
        TS_ASSERT(e.second[0] == 1.13f);
        TS_ASSERT(e.second[1] == 2.13f);
        TS_ASSERT(a->first == 'm');
    }

    void testConstIter4()
    {
        SM s;
        s['m'].push_back(1.13f);
        s['m'].push_back(2.13f);
        s['b'].push_back(1.2f);

        auto a = s.end();
        auto a1 = --a;
        TS_ASSERT(a->first == 'b');
        TS_ASSERT(a->second.size() == 1);
        auto a2 = a--;
        TS_ASSERT(a->first == 'm');
        TS_ASSERT(a->second.size() == 2);
        TS_ASSERT(a1 == a2);
        TS_ASSERT(a1->first == 'b');
        TS_ASSERT(a1->second.size() == 1);
    }

    void testIter()
    {
        SM s;
        s['m'].push_back(1.13f);
        s['m'].push_back(2.13f);
        s['b'].push_back(1.2f);

        SM::iterator a = s.end();
        (--a)->second.push_back(2.2f);
        (a++)->second.push_back(3.2f);
        SM::const_iterator b = s.cbegin();
        b++;
        TS_ASSERT(b->second[2] == 3.2f);
        TS_ASSERT(a == s.end());
    }

    void testReverseIterator()
    {
        SM s;
        s['m'].push_back(1.13f);
        s['m'].push_back(2.13f);
        s['b'].push_back(1.2f);

        SM::const_reverse_iterator a = s.crbegin();
        TS_ASSERT(a->second[0] == 1.2f);
        a++;
        TS_ASSERT(a->second[0] == 1.13f);
        ++a;
        TS_ASSERT(a == s.crend());
    }

    void testKeyCompare()
    {
        SM s;
        s.finalize();
        const SM& cs = s;

        SM::key_compare kc = cs.key_comp();
        K m = 'm', b = 'b';
        TS_ASSERT(kc(m, m) == false);
        TS_ASSERT(kc(m, b) == false);
        TS_ASSERT(kc(b, m) == true);
    }

    void testValueCompare()
    {
        SM s;
        s['m'].push_back(1.13f);
        s['m'].push_back(2.13f);
        s['b'].push_back(1.2f);
        s.finalize();
        const SM& cs = s;

        SM::value_compare vc = cs.value_comp();
        auto a = cs.begin();
        auto b = *a++, m = *a;
        TS_ASSERT(vc(m, m) == false);
        TS_ASSERT(vc(m, b) == false);
        TS_ASSERT(vc(b, m) == true);
    }

    void testAt()
    {
        SM s;
        s['m'].push_back(1.13f);
        s['m'].push_back(2.13f);
        s['b'].push_back(1.2f);
        s.finalize();
        const SM& cs = s;

        TS_ASSERT(s.at('m')[0] == 1.13f);
        TS_ASSERT(cs.at('b')[0] == 1.2f);
    }

    void testFind()
    {
        SM s;
        s['m'].push_back(1.13f);
        s['m'].push_back(2.13f);
        s['b'].push_back(1.2f);
        s.finalize();
        const SM& cs = s;

        SM::iterator a = s.find('m');
        TS_ASSERT(a->first == 'm');

        SM::const_iterator ca = cs.find('b');
        TS_ASSERT(ca->first == 'b');
    }

    void testCount()
    {
        SM s;
        s['m'].push_back(1.13f);
        s['m'].push_back(2.13f);
        s['b'].push_back(1.2f);
        s.finalize();
        const SM& cs = s;

        TS_ASSERT(s.count('m') == 1);   // arguably wrong
        TS_ASSERT(cs.count('w') == 0);
    }

    void testSpanTypes()
    {
        MS::value_type v;
        MS::reference r = v;
        MS::const_reference cr = r;
        MS::pointer p;
        MS::const_pointer cp;
        MS::iterator i;
        MS::const_iterator ci;
        MS::reverse_iterator ri;
        MS::const_reverse_iterator cri;
        MS::difference_type d;
        MS::size_type s;

        v = cr;
        p = 0;
        cp = 0;
        d = 0;
        s = 0;
        // no assertions; just check that all the types compile.
    }

    void testSpanEmpty()
    {
        SM s;
        MS& m = s['m'];
        const MS& cm = m;

        TS_ASSERT(m.empty() == true);
        TS_ASSERT(cm.begin() == cm.end());
        TS_ASSERT(m.rbegin() == m.rend());
        TS_ASSERT(cm.rbegin() == cm.rend());
        TS_ASSERT(cm.cbegin() == cm.cend());
        TS_ASSERT(cm.crbegin() == cm.crend());
    }

    void testSpanSize()
    {
        SM s;
        MS& m = s['m'];
        m.push_back(1.13f);
        m.push_back(2.13f);

        TS_ASSERT(m.size() == 2);
    }

    void testSpanFrontBack()
    {
        SM s;
        MS& m = s['m'];
        m.push_back(1.13f);
        m.push_back(2.13f);

        const MS::value_type& f = m.front();
        const MS::value_type& b = m.back();

        TS_ASSERT(f == 1.13f);
        TS_ASSERT(b == 2.13f);
    }

    void testSpanConstIter()
    {
        SM s;
        MS& m = s['m'];
        m.push_back(1.13f);
        m.push_back(2.13f);

        MS::const_iterator b = m.cbegin();
        MS::const_iterator e = m.cend();
        TS_ASSERT(b == b);
        TS_ASSERT(e == e);
        TS_ASSERT(b != e);
        TS_ASSERT(e != b);
    }

    void testSpanConstIter2()
    {
        SM s;
        MS& m = s['m'];
        m.push_back(1.13f);
        m.push_back(2.13f);

        MS::const_iterator a;   // use default constructor
        a = m.cbegin();         // use assignment operator
        MS::const_iterator a1 = a++;
        MS::const_iterator a2 = ++a;
        TS_ASSERT(a1 == m.cbegin());
        TS_ASSERT(a2 == m.cend());
    }

    void testSpanConstIter3()
    {
        SM s;
        MS& m = s['m'];
        m.push_back(1.13f);
        m.push_back(2.13f);

        MS::const_iterator a = m.cbegin();
        TS_ASSERT(*a == 1.13f);
        const V& v = *a;
        const V *p = a.operator -> ();
        TS_ASSERT(p == &v);
        a++;
        TS_ASSERT(*a == 2.13f);
    }

    void testSpanConstIter4()
    {
        SM s;
        MS& m = s['m'];
        m.push_back(1.13f);
        m.push_back(2.13f);

        auto a = m.cend();
        auto a1 = --a;
        TS_ASSERT(*a == 2.13f);
        auto a2 = a--;
        TS_ASSERT(*a == 1.13f);
        TS_ASSERT(a1 == a2);
        TS_ASSERT(*a1 == 2.13f);
    }

    void testSpanIter()
    {
        SM s;
        MS& m = s['m'];
        m.push_back(1.13f);
        m.push_back(2.13f);

        MS::iterator a = m.begin();
        *++a = 3.13f;
        a = m.begin();
        *a++ = 4.13f;
        TS_ASSERT(m[0] == 4.13f);
        TS_ASSERT(m[1] == 3.13f);
    }

    void testSpanReverseIter()
    {
        SM s;
        MS& m = s['m'];
        m.push_back(1.13f);
        m.push_back(2.13f);

        MS::const_reverse_iterator a = m.crbegin();
        TS_ASSERT(*a == 2.13f);
        a++;
        TS_ASSERT(*a == 1.13f);
        ++a;
        TS_ASSERT(a == m.crend());
        a--;
        TS_ASSERT(*a == 1.13f);
        --a;
        TS_ASSERT(*a == 2.13f);
        TS_ASSERT(a == m.crbegin());
    }

};
