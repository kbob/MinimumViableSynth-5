#include "fixed-map.h"

#include <cxxtest/TestSuite.h>

class fixed_map_unit_test : public CxxTest::TestSuite {

public:

    using K = short;
    using T = float;
    using V = std::pair<const K, T>;
    static const size_t N = 3;
    using M = fixed_map<K, T, N>;

    void test_instantiate()
    {
        (void)fixed_map<int, int, 4>();
    }

    void test_construct_empty()
    {
        M m;

        TS_ASSERT_EQUALS(m.size(), 0);
    }

    void test_construct_range()
    {
        V a[3] = {
            std::make_pair(4, 2.2f),
            std::make_pair(3, 1.1f),
            std::make_pair(5, 3.3f),
        };
        M m(a + 0, a + 3);

        TS_ASSERT_EQUALS(m.size(), 3);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(4), 2.2f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
    }

    void test_construct_copy()
    {
        V a[3] = {
            std::make_pair(3, 1.1f),
            std::make_pair(4, 2.2f),
            std::make_pair(5, 3.3f),
        };
        M m0(a + 0, a + 3);
        M m(m0);

        TS_ASSERT_EQUALS(m.size(), 3);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(4), 2.2f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
    }

    void test_construct_move()
    {
        V a[3] = {
            std::make_pair(3, 1.1f),
            std::make_pair(4, 2.2f),
            std::make_pair(5, 3.3f),
        };
        M m0(a + 0, a + 3);
        M m(std::move(m0));

        TS_ASSERT_EQUALS(m0.size(), 0);
        TS_ASSERT_EQUALS(m.size(), 3);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(4), 2.2f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
    }

    void test_construct_initializer_list()
    {
        M m{
            std::make_pair(3, 1.1f),
            std::make_pair(4, 2.2f),
            std::make_pair(5, 3.3f),
        };

        TS_ASSERT_EQUALS(m.size(), 3);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(4), 2.2f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
    }

    void test_copy_assignment()
    {
        M m;
        M m0{
            std::make_pair(3, 1.1f),
            std::make_pair(5, 3.3f),
            std::make_pair(4, 2.2f),
        };
        m = m0;

        TS_ASSERT_EQUALS(m.size(), 3);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(4), 2.2f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
    }

    void test_move_assignment()
    {
        M m;
        M m0{
            std::make_pair(3, 1.1f),
            std::make_pair(5, 3.3f),
            std::make_pair(4, 2.2f),
        };
        m = std::move(m0);

        TS_ASSERT_EQUALS(m0.size(), 0);
        TS_ASSERT_EQUALS(m.size(), 3);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(4), 2.2f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
    }

    void test_initializer_assignment()
    {
        M m{std::make_pair(6, 4.4f), std::make_pair(7, 5.5f)};
        m = {
            std::make_pair(3, 1.1f),
            std::make_pair(5, 3.3f),
            std::make_pair(4, 2.2f),
        };

        TS_ASSERT_EQUALS(m.size(), 3);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(4), 2.2f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
    }

    void test_begin_end()
    {
        M m{
            std::make_pair(3, 1.1f),
            std::make_pair(4, 2.2f),
            std::make_pair(5, 3.3f),
        };
        const M& cm(m);

        M::iterator b = m.begin();
        auto e = m.end();
        TS_ASSERT_EQUALS(b->first, 3);
        TS_ASSERT_EQUALS((--e)->second, 3.3f);

        auto cb = cm.begin();
        auto ce = cm.end();
        TS_ASSERT_EQUALS(cb->first, 3);
        TS_ASSERT_EQUALS((--ce)->second, 3.3f);

        auto rb = m.rbegin();
        auto re = m.rend();
        TS_ASSERT_EQUALS(rb->first, 5);
        TS_ASSERT_EQUALS((--re)->second, 1.1f);

        auto crb = cm.rbegin();
        auto cre = cm.rend();
        TS_ASSERT_EQUALS(crb->first, 5);
        TS_ASSERT_EQUALS((--cre)->second, 1.1f);

        auto ccb = m.cbegin();
        auto cce = m.cend();
        TS_ASSERT_EQUALS(ccb->first, 3);
        TS_ASSERT_EQUALS((--cce)->second, 3.3f);

        auto ccrb = m.crbegin();
        auto ccre = m.crend();
        TS_ASSERT_EQUALS(ccrb->first, 5);
        TS_ASSERT_EQUALS((--ccre)->second, 1.1f);
    }

    void test_empty()
    {
        M em;
        M nem{std::make_pair(3, 1.1f)};
        TS_ASSERT(em.empty());
        TS_ASSERT(!nem.empty());
    }

    void test_size()
    {
        M m;
        TS_ASSERT_EQUALS(m.size(), 0);
        m.insert(std::make_pair(3, 1.1f));
        TS_ASSERT_EQUALS(m.size(), 1);
        m.insert(std::make_pair(4, 2.2f));
        TS_ASSERT_EQUALS(m.size(), 2);
        m.insert(std::make_pair(5, 3.3f));
        TS_ASSERT_EQUALS(m.size(), 3);
    }

    void test_max_size()
    {
        M m;
        TS_ASSERT_EQUALS(m.max_size(), N);
    }

    void test_subscript()
    {
        M m{
            std::make_pair(3, 1.1f),
        };
        const short three = 3, four = 4, five = 5;
        TS_ASSERT_EQUALS(m[three], 1.1f);
        TS_ASSERT_EQUALS(m[4], 0.0f);
        TS_ASSERT_EQUALS(m[five], 0.0f);
        TS_ASSERT_EQUALS(m[3], 1.1f);
        TS_ASSERT_EQUALS(m[four], 0.0f);
        TS_ASSERT_EQUALS(m[5], 0.0f);
        m[four] = 2.2f;
        m[5] = 3.3f;
        TS_ASSERT_EQUALS(m[three], 1.1f);
        TS_ASSERT_EQUALS(m[four], 2.2f);
        TS_ASSERT_EQUALS(m[five], 3.3f);
        TS_ASSERT_EQUALS(m[3], 1.1f);
        TS_ASSERT_EQUALS(m[4], 2.2f);
        TS_ASSERT_EQUALS(m[5], 3.3f);
    }

    void test_at()
    {
        M m{
            std::make_pair(3, 1.1f),
            std::make_pair(4, 2.2f),
        };
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(4), 2.2f);
        TS_ASSERT_THROWS(m.at(5), std::out_of_range);

        m.at(4) = 6.2f;
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(4), 6.2f);
        TS_ASSERT_THROWS(m.at(5), std::out_of_range);

        const M cm{
            std::make_pair(3, 1.1f),
            std::make_pair(4, 2.2f),
        };
        TS_ASSERT_EQUALS(cm.at(3), 1.1f);
        TS_ASSERT_EQUALS(cm.at(4), 2.2f);
        TS_ASSERT_THROWS(cm.at(5), std::out_of_range);
    }

    void test_move_insert()
    {
        M m{
            std::make_pair(3, 1.1f),
        };
        auto p = std::make_pair(4, 2.2f);
        auto ret = m.insert(std::move(p));
        TS_ASSERT_EQUALS(m.size(), 2);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(4), 2.2f);
        TS_ASSERT_EQUALS(ret.first, m.begin() + 1);
        TS_ASSERT_EQUALS(ret.second, true);
    }

    void test_copy_hint_insert()
    {
        M m;
        const V v5(5, 3.3f);
        const V v4(4, 2.2f);
        const V v3(3, 1.1f);

        auto r5 = m.insert(m.begin(), v5);
        TS_ASSERT_EQUALS(m.size(), 1);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
        TS_ASSERT_EQUALS(r5, m.begin());

        auto r4 = m.insert(m.begin(), v4);
        TS_ASSERT_EQUALS(m.size(), 2);
        TS_ASSERT_EQUALS(m.at(4), 2.2f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
        TS_ASSERT_EQUALS(r4, m.begin());

        auto r3 = m.insert(m.end(), v3);
        TS_ASSERT_EQUALS(m.size(), 3);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(4), 2.2f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
        TS_ASSERT_EQUALS(r3, m.begin());

        auto r4_again = m.insert(m.begin(), v4);
        TS_ASSERT_EQUALS(m.size(), 3);
        TS_ASSERT_EQUALS(r4_again, m.begin() + 1);
    }

    void test_move_hint_insert()
    {
        M m;
        const V v5(5, 3.3f);
        const V v4(4, 2.2f);
        const V v3(3, 1.1f);

        auto r5 = m.insert(m.begin(), std::move(v5));
        TS_ASSERT_EQUALS(m.size(), 1);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
        TS_ASSERT_EQUALS(r5, m.begin());

        auto r4 = m.insert(m.begin(), v4);
        TS_ASSERT_EQUALS(m.size(), 2);
        TS_ASSERT_EQUALS(m.at(4), 2.2f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
        TS_ASSERT_EQUALS(r4, m.begin());

        auto r3 = m.insert(m.end(), v3);
        TS_ASSERT_EQUALS(m.size(), 3);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(4), 2.2f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
        TS_ASSERT_EQUALS(r3, m.begin());

        auto r4_again = m.insert(m.begin(), v4);
        TS_ASSERT_EQUALS(m.size(), 3);
        TS_ASSERT_EQUALS(r4_again, m.begin() + 1);
    }

    void test_range_insert()
    {
        M m;
        V a[] = {
            std::make_pair(3, 1.1f),
            std::make_pair(5, 3.3f),
            std::make_pair(4, 2.2f),
        };

        m.insert(a + 0, a + 3);
        TS_ASSERT_EQUALS(m.size(), 3);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(4), 2.2f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
    }

    void test_initializer_insert()
    {
        M m{
            std::make_pair(4, 2.2f),
        };
        m.insert({
            std::make_pair(5, 3.3f),
            std::make_pair(3, 1.1f),
        });
        TS_ASSERT_EQUALS(m.size(), 3);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(4), 2.2f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
    }

    void test_erase_by_position()
    {
        M m{
            std::make_pair(3, 1.1f),
            std::make_pair(5, 3.3f),
            std::make_pair(4, 2.2f),
        };
        auto r = m.erase(m.begin() + 1);
        TS_ASSERT_EQUALS(m.size(), 2);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
        TS_ASSERT_EQUALS(r, m.begin() + 1);
    }

    void test_erase_by_key()
    {
        M m{
            std::make_pair(3, 1.1f),
            std::make_pair(5, 3.3f),
            std::make_pair(4, 2.2f),
        };
        auto r = m.erase(4);
        TS_ASSERT_EQUALS(m.size(), 2);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
        TS_ASSERT_EQUALS(r, 1);

        auto r2 = m.erase(4);
        TS_ASSERT_EQUALS(m.size(), 2);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
        TS_ASSERT_EQUALS(r2, 0);
    }

    void test_erase_by_range()
    {
        M m{
            std::make_pair(3, 1.1f),
            std::make_pair(5, 3.3f),
            std::make_pair(4, 2.2f),
        };
        auto r = m.erase(m.begin() + 1, m.end() - 1);
        TS_ASSERT_EQUALS(m.size(), 2);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
        TS_ASSERT_EQUALS(r, m.begin() + 1);
    }

    void test_clear()
    {
        M m{
            std::make_pair(3, 1.1f),
            std::make_pair(5, 3.3f),
            std::make_pair(4, 2.2f),
        };
        m.clear();
        TS_ASSERT_EQUALS(m.size(), 0);
    }

    void test_emplace()
    {
        M m;
        auto r3 = m.emplace(3, 1.1f);
        TS_ASSERT_EQUALS(m.size(), 1);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(r3.first, m.begin());
        TS_ASSERT_EQUALS(r3.second, true);

        auto r5 = m.emplace(5, 3.3f);
        TS_ASSERT_EQUALS(m.size(), 2);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
        TS_ASSERT_EQUALS(r5.first, m.begin() + 1);
        TS_ASSERT_EQUALS(r5.second, true);

        auto r5a = m.emplace(5, 6.0f);
        TS_ASSERT_EQUALS(m.size(), 2);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
        TS_ASSERT_EQUALS(r5a.first, m.begin() + 1);
        TS_ASSERT_EQUALS(r5a.second, false);

        m.emplace(4, 2.2f);
        TS_ASSERT_EQUALS(m.size(), 3);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(4), 2.2f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
    }

    void test_emplace_hint()
    {
        M m;
        // const V v5(5, 3.3f);
        const V v4(4, 2.2f);
        const V v3(3, 1.1f);

        auto r5 = m.emplace_hint(m.begin(), 5, 3.3f);
        TS_ASSERT_EQUALS(m.size(), 1);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
        TS_ASSERT_EQUALS(r5, m.begin());

        auto r4 = m.emplace_hint(m.begin(), 4, 2.2f);
        TS_ASSERT_EQUALS(m.size(), 2);
        TS_ASSERT_EQUALS(m.at(4), 2.2f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
        TS_ASSERT_EQUALS(r4, m.begin());

        auto r3 = m.emplace_hint(m.end(), 3, 1.1f);
        TS_ASSERT_EQUALS(m.size(), 3);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(4), 2.2f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
        TS_ASSERT_EQUALS(r3, m.begin());

        auto r4_again = m.emplace_hint(m.begin(), 4, 6.7f);
        TS_ASSERT_EQUALS(m.size(), 3);
        TS_ASSERT_EQUALS(m.at(3), 1.1f);
        TS_ASSERT_EQUALS(m.at(4), 2.2f);
        TS_ASSERT_EQUALS(m.at(5), 3.3f);
        TS_ASSERT_EQUALS(r4_again, m.begin() + 1);
    }

    void test_key_comp()
    {
        M m;
        M::key_compare kc = m.key_comp();

        T a = 1.1f, b = 2.2f;
        TS_ASSERT(!kc(a, a));
        TS_ASSERT( kc(a, b));
        TS_ASSERT(!kc(b, a));
        TS_ASSERT(!kc(b, b));
    }

    void test_value_comp()
    {
        M m;
        M::value_compare vc = m.value_comp();
        V a(3, 2.2f), b(4, 1.1f);
        TS_ASSERT(!vc(a, a));
        TS_ASSERT( vc(a, b));
        TS_ASSERT(!vc(b, a));
        TS_ASSERT(!vc(b, b));
    }

    void test_find()
    {
        M m{
            std::make_pair(3, 1.1f),
            std::make_pair(5, 3.3f),
        };
        auto r2 = m.find(2);
        TS_ASSERT_EQUALS(r2, m.end());
        auto r3 = m.find(3);
        TS_ASSERT_EQUALS(r3, m.begin());
        auto r4 = m.find(4);
        TS_ASSERT_EQUALS(r4, m.end());
        auto r5 = m.find(5);
        TS_ASSERT_EQUALS(r5, m.begin() + 1);
        auto r6 = m.find(6);
        TS_ASSERT_EQUALS(r6, m.end());

        const M cm{
            std::make_pair(3, 1.1f),
            std::make_pair(5, 3.3f),
        };
        auto cr2 = cm.find(2);
        TS_ASSERT_EQUALS(cr2, cm.end());
        auto rc3 = cm.find(3);
        TS_ASSERT_EQUALS(rc3, cm.begin());
        auto rc4 = cm.find(4);
        TS_ASSERT_EQUALS(rc4, cm.end());
        auto rc5 = cm.find(5);
        TS_ASSERT_EQUALS(rc5, cm.begin() + 1);
        auto cr6 = cm.find(6);
        TS_ASSERT_EQUALS(cr6, cm.end());
    }

    void test_count()
    {
        const M cm{
            std::make_pair(3, 1.1f),
            std::make_pair(5, 3.3f),
        };
        auto rc2 = cm.count(2);
        TS_ASSERT_EQUALS(rc2, 0);
        auto rc3 = cm.count(3);
        TS_ASSERT_EQUALS(rc3, 1);
        auto rc4 = cm.count(4);
        TS_ASSERT_EQUALS(rc4, 0);
        auto rc5 = cm.count(5);
        TS_ASSERT_EQUALS(rc5, 1);
        auto rc6 = cm.count(6);
        TS_ASSERT_EQUALS(rc6, 0);
    }

    void test_lower_bound()
    {
        M m{
            std::make_pair(3, 1.1f),
            std::make_pair(5, 3.3f),
        };
        auto r2 = m.lower_bound(2);
        TS_ASSERT_EQUALS(r2, m.begin());
        auto r3 = m.lower_bound(3);
        TS_ASSERT_EQUALS(r3, m.begin());
        auto r4 = m.lower_bound(4);
        TS_ASSERT_EQUALS(r4, m.begin() + 1);
        auto r5 = m.lower_bound(5);
        TS_ASSERT_EQUALS(r5, m.begin() + 1);
        auto r6 = m.lower_bound(6);
        TS_ASSERT_EQUALS(r6, m.end());

        const M cm{
            std::make_pair(3, 1.1f),
            std::make_pair(5, 3.3f),
        };
        auto rc2 = cm.lower_bound(2);
        TS_ASSERT_EQUALS(rc2, cm.begin());
        auto rc3 = cm.lower_bound(3);
        TS_ASSERT_EQUALS(rc3, cm.begin());
        auto rc4 = cm.lower_bound(4);
        TS_ASSERT_EQUALS(rc4, cm.begin() + 1);
        auto rc5 = cm.lower_bound(5);
        TS_ASSERT_EQUALS(rc5, cm.begin() + 1);
        auto rc6 = cm.lower_bound(6);
        TS_ASSERT_EQUALS(rc6, cm.end());
    }

    void test_upper_bound()
    {
        M m{
            std::make_pair(3, 1.1f),
            std::make_pair(5, 3.3f),
        };
        auto r2 = m.upper_bound(2);
        TS_ASSERT_EQUALS(r2, m.begin());
        auto r3 = m.upper_bound(3);
        TS_ASSERT_EQUALS(r3, m.begin() + 1);
        auto r4 = m.upper_bound(4);
        TS_ASSERT_EQUALS(r4, m.begin() + 1);
        auto r5 = m.upper_bound(5);
        TS_ASSERT_EQUALS(r5, m.end());
        auto r6 = m.upper_bound(6);
        TS_ASSERT_EQUALS(r6, m.end());

        const M cm{
            std::make_pair(3, 1.1f),
            std::make_pair(5, 3.3f),
        };
        auto rc2 = cm.upper_bound(2);
        TS_ASSERT_EQUALS(rc2, cm.begin());
        auto rc3 = cm.upper_bound(3);
        TS_ASSERT_EQUALS(rc3, cm.begin() + 1);
        auto rc4 = cm.upper_bound(4);
        TS_ASSERT_EQUALS(rc4, cm.begin() + 1);
        auto rc5 = cm.upper_bound(5);
        TS_ASSERT_EQUALS(rc5, cm.end());
        auto rc6 = cm.upper_bound(6);
        TS_ASSERT_EQUALS(rc6, cm.end());
    }

    void test_equal_range()
    {
        M m{
            std::make_pair(3, 1.1f),
            std::make_pair(5, 3.3f),
        };
        auto r2 = m.equal_range(2);
        TS_ASSERT_EQUALS(r2.first,  m.begin());
        TS_ASSERT_EQUALS(r2.second, m.begin())
        auto r3 = m.equal_range(3);
        TS_ASSERT_EQUALS(r3.first,  m.begin());
        TS_ASSERT_EQUALS(r3.second, m.begin() + 1)
        auto r4 = m.equal_range(4);
        TS_ASSERT_EQUALS(r4.first,  m.begin() + 1);
        TS_ASSERT_EQUALS(r4.second, m.begin() + 1)
        auto r5 = m.equal_range(5);
        TS_ASSERT_EQUALS(r5.first,  m.begin() + 1);
        TS_ASSERT_EQUALS(r5.second, m.end())
        auto r6 = m.equal_range(6);
        TS_ASSERT_EQUALS(r6.first,  m.end());
        TS_ASSERT_EQUALS(r6.second, m.end())

        const M cm{
            std::make_pair(3, 1.1f),
            std::make_pair(5, 3.3f),
        };
        auto rc2 = cm.equal_range(2);
        TS_ASSERT_EQUALS(rc2.first,  cm.begin());
        TS_ASSERT_EQUALS(rc2.second, cm.begin())
        auto rc3 = cm.equal_range(3);
        TS_ASSERT_EQUALS(rc3.first,  cm.begin());
        TS_ASSERT_EQUALS(rc3.second, cm.begin() + 1)
        auto rc4 = cm.equal_range(4);
        TS_ASSERT_EQUALS(rc4.first,  cm.begin() + 1);
        TS_ASSERT_EQUALS(rc4.second, cm.begin() + 1)
        auto rc5 = cm.equal_range(5);
        TS_ASSERT_EQUALS(rc5.first,  cm.begin() + 1);
        TS_ASSERT_EQUALS(rc5.second, cm.end())
        auto rc6 = cm.equal_range(6);
        TS_ASSERT_EQUALS(rc6.first,  cm.end());
        TS_ASSERT_EQUALS(rc6.second, cm.end())
    }

    void test_equality()
    {
        M m0;
        M m1{
            std::make_pair(3, 1.1),
        };
        M m2{
            std::make_pair(3, 1.1),
            std::make_pair(4, 2.2),
        };
        M m3{
            std::make_pair(3, 2.2),
        };
        M m4{
            std::make_pair(4, 2.2),
        };

        TS_ASSERT( (m0 == m0));
        TS_ASSERT(!(m0 == m1));
        TS_ASSERT(!(m0 == m2));
        TS_ASSERT(!(m0 == m3));
        TS_ASSERT(!(m0 == m4));

        TS_ASSERT(!(m1 == m0));
        TS_ASSERT( (m1 == m1));
        TS_ASSERT(!(m1 == m2));
        TS_ASSERT(!(m1 == m3));
        TS_ASSERT(!(m1 == m4));

        TS_ASSERT(!(m2 == m0));
        TS_ASSERT(!(m2 == m1));
        TS_ASSERT( (m2 == m2));
        TS_ASSERT(!(m2 == m3));
        TS_ASSERT(!(m2 == m4));

        TS_ASSERT(!(m3 == m0));
        TS_ASSERT(!(m3 == m1));
        TS_ASSERT(!(m3 == m2));
        TS_ASSERT( (m3 == m3));
        TS_ASSERT(!(m3 == m4));

        TS_ASSERT(!(m4 == m0));
        TS_ASSERT(!(m4 == m1));
        TS_ASSERT(!(m4 == m2));
        TS_ASSERT(!(m4 == m3));
        TS_ASSERT( (m4 == m4));
    }

    void test_inequality()
    {
        M m0;
        M m1{
            std::make_pair(3, 1.1),
        };
        M m2{
            std::make_pair(3, 1.1),
            std::make_pair(4, 2.2),
        };
        M m3{
            std::make_pair(3, 2.2),
        };
        M m4{
            std::make_pair(4, 2.2),
        };

        TS_ASSERT(!(m0 != m0));
        TS_ASSERT( (m0 != m1));
        TS_ASSERT( (m0 != m2));
        TS_ASSERT( (m0 != m3));
        TS_ASSERT( (m0 != m4));

        TS_ASSERT( (m1 != m0));
        TS_ASSERT(!(m1 != m1));
        TS_ASSERT( (m1 != m2));
        TS_ASSERT( (m1 != m3));
        TS_ASSERT( (m1 != m4));

        TS_ASSERT( (m2 != m0));
        TS_ASSERT( (m2 != m1));
        TS_ASSERT(!(m2 != m2));
        TS_ASSERT( (m2 != m3));
        TS_ASSERT( (m2 != m4));

        TS_ASSERT( (m3 != m0));
        TS_ASSERT( (m3 != m1));
        TS_ASSERT( (m3 != m2));
        TS_ASSERT(!(m3 != m3));
        TS_ASSERT( (m3 != m4));

        TS_ASSERT( (m4 != m0));
        TS_ASSERT( (m4 != m1));
        TS_ASSERT( (m4 != m2));
        TS_ASSERT( (m4 != m3));
        TS_ASSERT(!(m4 != m4));
    }

    void test_less_than()
    {
        M m0;
        M m1{
            std::make_pair(3, 1.1),
        };
        M m2{
            std::make_pair(3, 1.1),
            std::make_pair(4, 2.2),
        };
        M m3{
            std::make_pair(3, 2.2),
        };
        M m4{
            std::make_pair(4, 2.2),
        };

        TS_ASSERT(!(m0 < m0));
        TS_ASSERT( (m0 < m1));
        TS_ASSERT( (m0 < m2));
        TS_ASSERT( (m0 < m3));
        TS_ASSERT( (m0 < m4));

        TS_ASSERT(!(m1 < m0));
        TS_ASSERT(!(m1 < m1));
        TS_ASSERT( (m1 < m2));
        TS_ASSERT( (m1 < m3));
        TS_ASSERT( (m1 < m4));

        TS_ASSERT(!(m2 < m0));
        TS_ASSERT(!(m2 < m1));
        TS_ASSERT(!(m2 < m2));
        TS_ASSERT( (m2 < m3));
        TS_ASSERT( (m2 < m4));

        TS_ASSERT(!(m3 < m0));
        TS_ASSERT(!(m3 < m1));
        TS_ASSERT(!(m3 < m2));
        TS_ASSERT(!(m3 < m3));
        TS_ASSERT( (m3 < m4));

        TS_ASSERT(!(m4 < m0));
        TS_ASSERT(!(m4 < m1));
        TS_ASSERT(!(m4 < m2));
        TS_ASSERT(!(m4 < m3));
        TS_ASSERT(!(m4 < m4));
    }

    void test_less_than_or_equal()
    {
        M m0;
        M m1{
            std::make_pair(3, 1.1),
        };
        M m2{
            std::make_pair(3, 1.1),
            std::make_pair(4, 2.2),
        };
        M m3{
            std::make_pair(3, 2.2),
        };
        M m4{
            std::make_pair(4, 2.2),
        };

        TS_ASSERT( (m0 <= m0));
        TS_ASSERT( (m0 <= m1));
        TS_ASSERT( (m0 <= m2));
        TS_ASSERT( (m0 <= m3));
        TS_ASSERT( (m0 <= m4));

        TS_ASSERT(!(m1 <= m0));
        TS_ASSERT( (m1 <= m1));
        TS_ASSERT( (m1 <= m2));
        TS_ASSERT( (m1 <= m3));
        TS_ASSERT( (m1 <= m4));

        TS_ASSERT(!(m2 <= m0));
        TS_ASSERT(!(m2 <= m1));
        TS_ASSERT( (m2 <= m2));
        TS_ASSERT( (m2 <= m3));
        TS_ASSERT( (m2 <= m4));

        TS_ASSERT(!(m3 <= m0));
        TS_ASSERT(!(m3 <= m1));
        TS_ASSERT(!(m3 <= m2));
        TS_ASSERT( (m3 <= m3));
        TS_ASSERT( (m3 <= m4));

        TS_ASSERT(!(m4 <= m0));
        TS_ASSERT(!(m4 <= m1));
        TS_ASSERT(!(m4 <= m2));
        TS_ASSERT(!(m4 <= m3));
        TS_ASSERT( (m4 <= m4));
    }

    void test_greater_than()
    {
        M m0;
        M m1{
            std::make_pair(3, 1.1),
        };
        M m2{
            std::make_pair(3, 1.1),
            std::make_pair(4, 2.2),
        };
        M m3{
            std::make_pair(3, 2.2),
        };
        M m4{
            std::make_pair(4, 2.2),
        };

        TS_ASSERT(!(m0 > m0));
        TS_ASSERT(!(m0 > m1));
        TS_ASSERT(!(m0 > m2));
        TS_ASSERT(!(m0 > m3));
        TS_ASSERT(!(m0 > m4));

        TS_ASSERT( (m1 > m0));
        TS_ASSERT(!(m1 > m1));
        TS_ASSERT(!(m1 > m2));
        TS_ASSERT(!(m1 > m3));
        TS_ASSERT(!(m1 > m4));

        TS_ASSERT( (m2 > m0));
        TS_ASSERT( (m2 > m1));
        TS_ASSERT(!(m2 > m2));
        TS_ASSERT(!(m2 > m3));
        TS_ASSERT(!(m2 > m4));

        TS_ASSERT( (m3 > m0));
        TS_ASSERT( (m3 > m1));
        TS_ASSERT( (m3 > m2));
        TS_ASSERT(!(m3 > m3));
        TS_ASSERT(!(m3 > m4));

        TS_ASSERT( (m4 > m0));
        TS_ASSERT( (m4 > m1));
        TS_ASSERT( (m4 > m2));
        TS_ASSERT( (m4 > m3));
        TS_ASSERT(!(m4 > m4));
    }

    void test_greater_than_or_equal()
    {
        M m0;
        M m1{
            std::make_pair(3, 1.1),
        };
        M m2{
            std::make_pair(3, 1.1),
            std::make_pair(4, 2.2),
        };
        M m3{
            std::make_pair(3, 2.2),
        };
        M m4{
            std::make_pair(4, 2.2),
        };

        TS_ASSERT( (m0 >= m0));
        TS_ASSERT(!(m0 >= m1));
        TS_ASSERT(!(m0 >= m2));
        TS_ASSERT(!(m0 >= m3));
        TS_ASSERT(!(m0 >= m4));

        TS_ASSERT( (m1 >= m0));
        TS_ASSERT( (m1 >= m1));
        TS_ASSERT(!(m1 >= m2));
        TS_ASSERT(!(m1 >= m3));
        TS_ASSERT(!(m1 >= m4));

        TS_ASSERT( (m2 >= m0));
        TS_ASSERT( (m2 >= m1));
        TS_ASSERT( (m2 >= m2));
        TS_ASSERT(!(m2 >= m3));
        TS_ASSERT(!(m2 >= m4));

        TS_ASSERT( (m3 >= m0));
        TS_ASSERT( (m3 >= m1));
        TS_ASSERT( (m3 >= m2));
        TS_ASSERT( (m3 >= m3));
        TS_ASSERT(!(m3 >= m4));

        TS_ASSERT( (m4 >= m0));
        TS_ASSERT( (m4 >= m1));
        TS_ASSERT( (m4 >= m2));
        TS_ASSERT( (m4 >= m3));
        TS_ASSERT( (m4 >= m4));
    }

};
