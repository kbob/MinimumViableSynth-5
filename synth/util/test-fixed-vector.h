#include "fixed-vector.h"

#include <list>

#include <cxxtest/TestSuite.h>

class fixed_vector_unit_test : public CxxTest::TestSuite {

public:

    static std::stringstream& log()
    {
        static std::stringstream ss;
        return ss;
    }

    class logger {

    public:

        std::string haddr()
        {
            std::stringstream ss;
            ss << std::hex
               << "..."
               << (reinterpret_cast<uintptr_t>(this) & 0xFFF);
            return ss.str();
        }

        logger(int n = 0)
        : m(n)
        {
            log() << "c1(" << n << ") ";
        }

        logger(const logger& that)
        : m(that.m)
        {
            log() << "c4(" << m << ") ";
        }

        logger(logger&& that)
        : m(that.m)
        {
            log() << "c5(" << m << ") ";
        }

        logger& operator = (const logger& that)
        {
            log() << '=' << that.m << ' ';
            if (this != &that)
                m = that.m;
            return *this;
        }

        logger& operator = (logger&& that)
        {
            log() << "=&&" << that.m << ' ';
            if (this != &that)
                m = that.m;
            return *this;
        }

        ~logger()
        {
            log() << "~(" << m << ") ";
        }

        int m;
    };

    void test_instantiate()
    {
        (void)fixed_vector<int, 1>();
        (void)fixed_vector<logger, 1>();
    }

    void test_construct_default()
    {
        fixed_vector<int, 4> v;
        TS_ASSERT_EQUALS(v.size(), 0);
        TS_ASSERT_EQUALS(sizeof v, sizeof (int *) + 4 * sizeof (int));

        log().str("");
        {
            fixed_vector<logger, 4> vl;
        }
        TS_ASSERT_EQUALS(log().str(), "");
    }

    void test_construct_fill_a()
    {
        fixed_vector<int, 4> v(2);
        TS_ASSERT_EQUALS(v.size(), 2);

        log().str("");
        {
            fixed_vector<logger, 4> vl(2);
        }
        TS_ASSERT_EQUALS(log().str(), "c1(0) c1(0) ~(0) ~(0) ");
    }

    void test_construct_fill_b()
    {
        fixed_vector<int, 4> v(2, 3);
        TS_ASSERT_EQUALS(v.size(), 2);
        TS_ASSERT_EQUALS(v[0], 3);
        TS_ASSERT_EQUALS(v[1], 3);

        logger elem(3);
        log().str("");
        {
            fixed_vector<logger, 4> vl(2, elem);
        }
        TS_ASSERT_EQUALS(log().str(), "c4(3) c4(3) ~(3) ~(3) ");
    }

    void test_construct_range()
    {
        const char *p = "abc";
        fixed_vector<int, 4> v(p, p + 3);
        TS_ASSERT_EQUALS(v.size(), 3);
        TS_ASSERT_EQUALS(v[0], 'a');
        TS_ASSERT_EQUALS(v[1], 'b');
        TS_ASSERT_EQUALS(v[2], 'c');

        std::list<logger> seq{'a', 'b', 'c'};
        log().str("");
        {
            fixed_vector<logger, 4> vl(seq.begin(), seq.end());
        }
        TS_ASSERT_EQUALS(log().str(),
                         "c4(97) c4(98) c4(99) ~(99) ~(98) ~(97) ");
    }

    void test_construct_copy()
    {
        fixed_vector<int, 4> v0(2, 3);
        fixed_vector<int, 4> v(v0);
        TS_ASSERT_EQUALS(v.size(), 2);
        TS_ASSERT_EQUALS(v[0], 3);
        TS_ASSERT_EQUALS(v[1], 3);

        fixed_vector<logger, 4> vl0(2, 3);
        log().str("");
        {
            fixed_vector<logger, 4> vl(vl0);
        }
        TS_ASSERT_EQUALS(log().str(), "c4(3) c4(3) ~(3) ~(3) ");
    }

    void test_construct_move()
    {
        const char *p = "abc";
        fixed_vector<int, 4> v0(p, p + 3);
        fixed_vector<int, 4> v(std::move(v0));
        TS_ASSERT_EQUALS(v.size(), 3);
        TS_ASSERT_EQUALS(v[0], 'a');
        TS_ASSERT_EQUALS(v[1], 'b');
        TS_ASSERT_EQUALS(v[2], 'c');
        TS_ASSERT_EQUALS(v0.size(), 0);

        {
            fixed_vector<logger, 4> vl0(p, p + 3);
            log().str("");
            {
                fixed_vector<logger, 4> vl(std::move(vl0));
            }
            TS_ASSERT_EQUALS(log().str(),
                             "c5(97) c5(98) c5(99) ~(99) ~(98) ~(97) ");
            log().str("");
        }
        TS_ASSERT_EQUALS(log().str(), ""); // vl0 should have been emptied.
    }

    void test_construct_initializer_list()
    {
        fixed_vector<int, 4> v{7, 8, 9};
        TS_ASSERT_EQUALS(v.size(), 3);
        TS_ASSERT_EQUALS(v[0], 7);
        TS_ASSERT_EQUALS(v[1], 8);
        TS_ASSERT_EQUALS(v[2], 9);

        logger l7(7), l8(8), l9(9);
        log().str("");
        {
            fixed_vector<logger, 4> vl{l7, l8, l9};
        }
        TS_ASSERT_EQUALS(log().str(),
                         "c4(7) c4(8) c4(9) c4(7) c4(8) c4(9) "
                         "~(9) ~(8) ~(7) ~(9) ~(8) ~(7) ");
    }

    void test_assignment_copy()
    {
        fixed_vector<int, 4> v0{7, 8, 9};
        fixed_vector<int, 4> v(1, 3);
        v = v0;
        TS_ASSERT_EQUALS(v.size(), 3);
        TS_ASSERT_EQUALS(v[0], 7);
        TS_ASSERT_EQUALS(v[1], 8);
        TS_ASSERT_EQUALS(v[2], 9);

        fixed_vector<logger, 4> lv0{logger(7), logger(8), logger(9)};
        fixed_vector<logger, 4> lv(2, logger(3));
        log().str("");
        lv = lv0;
        TS_ASSERT_EQUALS(log().str(), "~(3) ~(3) c4(7) c4(8) c4(9) ");
    }

    void test_assignment_move()
    {
        fixed_vector<int, 4> v0{7, 8, 9};
        fixed_vector<int, 4> v(1, 3);
        v = std::move(v0);
        TS_ASSERT_EQUALS(v.size(), 3);
        TS_ASSERT_EQUALS(v[0], 7);
        TS_ASSERT_EQUALS(v[1], 8);
        TS_ASSERT_EQUALS(v[2], 9);

        fixed_vector<logger, 4> lv0{logger(7), logger(8), logger(9)};
        fixed_vector<logger, 4> lv(2, logger(3));
        log().str("");
        lv = std::move(lv0);
        TS_ASSERT_EQUALS(log().str(), "~(3) ~(3) c5(7) c5(8) c5(9) ");
    }

    void test_assignment_initializer_list()
    {
        fixed_vector<int, 4> v(2, 3);
        v = {7, 8, 9};
        TS_ASSERT_EQUALS(v.size(), 3);
        TS_ASSERT_EQUALS(v[0], 7);
        TS_ASSERT_EQUALS(v[1], 8);
        TS_ASSERT_EQUALS(v[2], 9);

        logger l7(7), l8(8), l9(9);
        fixed_vector<logger, 4> lv(2, logger(3));
        log().str("");
        lv = {l7, l8, l9};
        TS_ASSERT_EQUALS(log().str(),
                         "c4(7) c4(8) c4(9) "
                         "~(3) ~(3) "
                         "c4(7) c4(8) c4(9) "
                         "~(9) ~(8) ~(7) ");
    }

    void test_begin_end()
    {
        fixed_vector<int, 4> v{7, 8, 9};
        const fixed_vector<int, 4> cv{7, 8, 9};

        fixed_vector<int, 4>::iterator b = v.begin();
        auto e = v.end();
        TS_ASSERT_EQUALS(*b, 7);
        TS_ASSERT_EQUALS(*--e, 9);

        auto cb = cv.begin();
        auto ce = cv.end();
        TS_ASSERT_EQUALS(*cb, 7);
        TS_ASSERT_EQUALS(*--ce, 9);

        auto rb = v.rbegin();
        auto re = v.rend();
        TS_ASSERT_EQUALS(*rb, 9);
        TS_ASSERT_EQUALS(*--re, 7);

        auto crb = cv.rbegin();
        auto cre = cv.rend();
        TS_ASSERT_EQUALS(*crb, 9);
        TS_ASSERT_EQUALS(*--cre, 7);

        auto ccb = v.cbegin();
        auto cce = v.cend();
        TS_ASSERT_EQUALS(*ccb, 7);
        TS_ASSERT_EQUALS(*--cce, 9);

        auto ccrb = v.crbegin();
        auto ccre = v.crend();
        TS_ASSERT_EQUALS(*ccrb, 9);
        TS_ASSERT_EQUALS(*--ccre, 7);
    }

    void test_size()
    {
        fixed_vector<int, 4> v;
        TS_ASSERT_EQUALS(v.size(), 0);
        v.push_back(1);
        TS_ASSERT_EQUALS(v.size(), 1);
        v.push_back(2);
        TS_ASSERT_EQUALS(v.size(), 2);
        v.push_back(3);
        TS_ASSERT_EQUALS(v.size(), 3);
        v.push_back(4);
        TS_ASSERT_EQUALS(v.size(), 4);
    }

    void test_max_size()
    {
        fixed_vector<int, 4> v;
        TS_ASSERT_EQUALS(v.max_size(), 4);
    }

    void test_resize()
    {
        // simple resize
        fixed_vector<int, 4> v(3, 2);
        v.resize(1);
        TS_ASSERT_EQUALS(v.size(), 1);
        TS_ASSERT_EQUALS(v[0], 2);
        v.resize(4);
        TS_ASSERT_EQUALS(v.size(), 4);
        TS_ASSERT_EQUALS(v[0], 2);
        TS_ASSERT_EQUALS(v[1], 0);
        TS_ASSERT_EQUALS(v[2], 0);
        TS_ASSERT_EQUALS(v[3], 0);

        // resize with value
        fixed_vector<int, 4> v1(3, 2);
        v1.resize(1, 3);
        TS_ASSERT_EQUALS(v1.size(), 1);
        TS_ASSERT_EQUALS(v1[0], 2);
        v1.resize(4, 5);
        TS_ASSERT_EQUALS(v1.size(), 4);
        TS_ASSERT_EQUALS(v1[0], 2);
        TS_ASSERT_EQUALS(v1[1], 5);
        TS_ASSERT_EQUALS(v1[2], 5);
        TS_ASSERT_EQUALS(v1[3], 5);

        // resize logger
        fixed_vector<logger, 4> vl(3, 2);

        log().str("");
        vl.resize(1);
        TS_ASSERT_EQUALS(log().str(), "~(2) ~(2) ");

        log().str("");
        vl.resize(4);
        TS_ASSERT_EQUALS(log().str(), "c1(0) c1(0) c1(0) ");

        // resize logger with value
        fixed_vector<logger, 4> vl1(3, 2);
        logger l3(3), l5(5);

        log().str("");
        vl1.resize(1, l3);
        TS_ASSERT_EQUALS(log().str(), "~(2) ~(2) ");

        log().str("");
        vl1.resize(4, l5);
        TS_ASSERT_EQUALS(log().str(), "c4(5) c4(5) c4(5) ");
    }

    void test_capacity()
    {
        fixed_vector<int, 4> v;
        TS_ASSERT_EQUALS(v.capacity(), 4);
    }

    void test_empty()
    {
        fixed_vector<int, 4> ev;
        fixed_vector<int, 4> nev{1};
        TS_ASSERT(ev.empty());
        TS_ASSERT(!nev.empty());
    }

    void test_reserve()
    {
        fixed_vector<int, 4> v;
        v.reserve(0);
        v.reserve(4);
        TS_ASSERT_THROWS(v.reserve(5), std::length_error);
    }

    void test_shrink_to_fit()
    {
        fixed_vector<int, 4> v;
        v.shrink_to_fit();
        v = {7, 8, 9, 10};
        v.shrink_to_fit();
    }

    void test_subscript()
    {
        fixed_vector<int, 4> v{7, 8, 9};
        TS_ASSERT_EQUALS(v[0], 7);
        TS_ASSERT_EQUALS(v[1], 8);
        TS_ASSERT_EQUALS(v[2], 9);

        v[1] = 55;
        TS_ASSERT_EQUALS(v[0], 7);
        TS_ASSERT_EQUALS(v[1], 55);
        TS_ASSERT_EQUALS(v[2], 9);

        const fixed_vector<int, 4> cv{4, 5, 6};
        TS_ASSERT_EQUALS(cv[0], 4);
        TS_ASSERT_EQUALS(cv[1], 5);
        TS_ASSERT_EQUALS(cv[2], 6);
    }

    void test_at()
    {
        fixed_vector<int, 4> v{7, 8, 9};
        TS_ASSERT_EQUALS(v.at(0), 7);
        TS_ASSERT_EQUALS(v.at(1), 8);
        TS_ASSERT_EQUALS(v.at(2), 9);

        v.at(1) = 55;
        TS_ASSERT_EQUALS(v.at(0), 7);
        TS_ASSERT_EQUALS(v.at(1), 55);
        TS_ASSERT_EQUALS(v.at(2), 9);

        const fixed_vector<int, 4> cv{4, 5, 6};
        TS_ASSERT_EQUALS(cv.at(0), 4);
        TS_ASSERT_EQUALS(cv.at(1), 5);
        TS_ASSERT_EQUALS(cv.at(2), 6);
    }

    void test_front_back()
    {
        fixed_vector<int, 4> v{7, 8, 9};
        TS_ASSERT_EQUALS(v.front(), 7);
        TS_ASSERT_EQUALS(v.back(), 9);

        const fixed_vector<int, 4> cv{4, 5, 6};
        TS_ASSERT_EQUALS(cv.front(), 4);
        TS_ASSERT_EQUALS(cv.back(), 6);
    }

    void test_data()
    {
        fixed_vector<int, 4> v{7, 8, 9};
        TS_ASSERT_EQUALS(v.data()[0], 7);
        TS_ASSERT_EQUALS(v.data()[1], 8);

        const fixed_vector<int, 4> cv{4, 5, 6};
        TS_ASSERT_EQUALS(cv.data()[0], 4);
        TS_ASSERT_EQUALS(cv.data()[2], 6);
    }

    void test_assign_range()
    {
        const char *p = "abc";
        fixed_vector<int, 4> v(2, 3);
        v.assign(p, p + 3);
        TS_ASSERT_EQUALS(v.size(), 3);
        TS_ASSERT_EQUALS(v[0], 97);
        TS_ASSERT_EQUALS(v[1], 98);
        TS_ASSERT_EQUALS(v[2], 99);

        std::list<logger> seq{'a', 'b'};
        fixed_vector<logger, 4> vl(4, 3);
        log().str("");
        vl.assign(seq.begin(), seq.end());
        TS_ASSERT_EQUALS(log().str(), "~(3) ~(3) ~(3) ~(3) c4(97) c4(98) ");
    }

};
