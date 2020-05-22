#include "fixed-queue.h"

#include <iostream>
#include <sstream>
#include <string>

#include <cxxtest/TestSuite.h>

class fixed_queue_unit_test : public CxxTest::TestSuite {

public:

    static std::ostringstream& log()
    {
        static std::ostringstream ss;
        return ss;
    }

    static void clear_log()
    {
        log().str("");
    }

    class logger {

    public:

        std::string haddr()
        {
            std::ostringstream ss;
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
            that.m = 0;
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
            if (this != &that) {
                m = that.m;
                that.m = 0;
            }
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
        (void)fixed_queue<int, 3>();
        (void)fixed_queue<logger, 3>();
    }

    void test_constructor()
    {
        fixed_queue<int, 3> q;
        TS_ASSERT(q.empty());

        clear_log();
        {
            fixed_queue<logger, 3> ql;
        }
        TS_ASSERT_EQUALS(log().str(), "");
    }

    void test_empty()
    {
        fixed_queue<int, 3> q;
        TS_ASSERT(q.empty());
        q.push(7);
        TS_ASSERT(!q.empty());
        q.pop();
        TS_ASSERT(q.empty());
    }

    void test_size()
    {
        fixed_queue<int, 3> q;
        TS_ASSERT_EQUALS(q.size(), 0);
        q.push(1);
        TS_ASSERT_EQUALS(q.size(), 1);
        q.push(2);
        TS_ASSERT_EQUALS(q.size(), 2);
        q.push(3);
        TS_ASSERT_EQUALS(q.size(), 3);
    }

    void test_max_size()
    {
        fixed_queue<int, 3> q;
        TS_ASSERT_EQUALS(q.max_size(), 3);
    }

    void test_front_back()
    {
        fixed_queue<int, 3> q;
        const fixed_queue<int, 3>& cq = q;
        q.push(1);
        TS_ASSERT_EQUALS(q.front(), 1);
        TS_ASSERT_EQUALS(cq.front(), 1);
        TS_ASSERT_EQUALS(q.back(), 1);
        TS_ASSERT_EQUALS(cq.back(), 1);
        q.push(2);
        q.push(3);
        TS_ASSERT_EQUALS(q.front(), 1);
        TS_ASSERT_EQUALS(cq.front(), 1);
        TS_ASSERT_EQUALS(q.back(), 3);
        TS_ASSERT_EQUALS(cq.back(), 3);
        q.pop();
        TS_ASSERT_EQUALS(q.front(), 2);
        TS_ASSERT_EQUALS(cq.front(), 2);
        TS_ASSERT_EQUALS(q.back(), 3);
        TS_ASSERT_EQUALS(cq.back(), 3);
        q.push(4);
        TS_ASSERT_EQUALS(q.front(), 2);
        TS_ASSERT_EQUALS(cq.front(), 2);
        TS_ASSERT_EQUALS(q.back(), 4);
        TS_ASSERT_EQUALS(cq.back(), 4);
    }

    void test_push_copy()
    {
        fixed_queue<int, 3> q;
        TS_ASSERT_EQUALS(q.size(), 0);
        q.push(1);
        TS_ASSERT_EQUALS(q.size(), 1);
        TS_ASSERT_EQUALS(q.front(), 1);
        TS_ASSERT_EQUALS(q.back(), 1);
        q.push(2);
        q.push(3);
        TS_ASSERT_EQUALS(q.size(), 3);
        TS_ASSERT_EQUALS(q.front(), 1);
        TS_ASSERT_EQUALS(q.back(), 3);
        q.pop();
        TS_ASSERT_EQUALS(q.size(), 2);
        TS_ASSERT_EQUALS(q.front(), 2);
        TS_ASSERT_EQUALS(q.back(), 3);
        q.push(4);
        TS_ASSERT_EQUALS(q.size(), 3);
        TS_ASSERT_EQUALS(q.front(), 2);
        TS_ASSERT_EQUALS(q.back(), 4);

        {
            logger l6(6);
            clear_log();
            {
                fixed_queue<logger, 3> ql;
                ql.push(l6);
                TS_ASSERT_EQUALS(log().str(), "c4(6) ");
                clear_log();
            }
            TS_ASSERT_EQUALS(log().str(), "~(6) ");
            clear_log();
        }
        TS_ASSERT_EQUALS(log().str(), "~(6) ");
    }

    void test_push_move()
    {
        fixed_queue<int, 3> q;
        int i1 = 1, i2 = 2;
        q.push(std::move(i1));
        q.push(std::move(i2));
        TS_ASSERT_EQUALS(q.size(), 2);
        TS_ASSERT_EQUALS(q.front(), 1);
        TS_ASSERT_EQUALS(q.back(), 2);

        {
            logger l6(6);
            {
                fixed_queue<logger, 3> q;
                clear_log();
                q.push(std::move(l6));
                TS_ASSERT_EQUALS(log().str(), "c5(6) ");
                clear_log();
            }
            TS_ASSERT_EQUALS(log().str(), "~(6) ");
            clear_log();
        }
        TS_ASSERT_EQUALS(log().str(), "~(0) ");
    }

    void test_emplace()
    {
        fixed_queue<int, 3> q;
        q.emplace('x');
        q.emplace();
        TS_ASSERT_EQUALS(q.size(), 2);
        TS_ASSERT_EQUALS(q.front(), 'x');
        TS_ASSERT_EQUALS(q.back(), 0);

        {
            fixed_queue<logger, 3> ql;
            clear_log();
            ql.emplace('x');
            ql.emplace();
            TS_ASSERT_EQUALS(log().str(), "c1(120) c1(0) ");
            clear_log();
        }
        TS_ASSERT_EQUALS(log().str(), "~(120) ~(0) ");
    }

    void test_pop()
    {
        fixed_queue<int, 3> q;
        q.push(1);
        q.push(2);
        TS_ASSERT_EQUALS(q.size(), 2);
        TS_ASSERT_EQUALS(q.front(), 1);
        TS_ASSERT_EQUALS(q.back(), 2);
        q.pop();
        TS_ASSERT_EQUALS(q.size(), 1);
        TS_ASSERT_EQUALS(q.front(), 2);
        TS_ASSERT_EQUALS(q.back(), 2);
        q.push(3);
        q.push(4);
        TS_ASSERT_EQUALS(q.size(), 3);
        TS_ASSERT_EQUALS(q.front(), 2);
        TS_ASSERT_EQUALS(q.back(), 4);
        q.pop();
        TS_ASSERT_EQUALS(q.size(), 2);
        TS_ASSERT_EQUALS(q.front(), 3);
        TS_ASSERT_EQUALS(q.back(), 4);
        q.pop();
        TS_ASSERT_EQUALS(q.size(), 1);
        TS_ASSERT_EQUALS(q.front(), 4);
        TS_ASSERT_EQUALS(q.back(), 4);

        {
            fixed_queue<logger, 3> ql;
            ql.push(1);
            clear_log();
            ql.pop();
            TS_ASSERT_EQUALS(log().str(), "~(1) ");
            clear_log();
        }
        TS_ASSERT_EQUALS(log().str(), "");
    }

    template <class T, size_t N>
    void rotate_in(fixed_queue<T, N>& q, size_t r, const std::vector<T>& values)
    {
        if (r) {
            q.push(0);
            for (size_t i = 1; i < r; i++) {
                q.push(0);
                q.pop();
            }
            TS_ASSERT_EQUALS(q.size(), 1);
        }
        for (auto& v: values)
            q.push(v);
        if (r) {
            TS_ASSERT_EQUALS(q.front(), 0);
            q.pop();
        }
        TS_ASSERT_EQUALS(q.front(), values.front());
        TS_ASSERT_EQUALS(q.back(), values.back());
    }

    // template <class T, size_t N>
    // std::string repr(const fixed_queue<T, N>& q)
    // {
    //     std::ostringstream ss;
    //     if (q.empty())
    //         ss << "(empty)";
    //     else {
    //         ss << '<';
    //         const char *sep = "";
    //         const T *p = q.m_begin;
    //         do {
    //             ss << sep << *p;
    //             sep = " ";
    //             p = q.inc(p);
    //         } while (p != q.m_end);
    //         ss << ']';
    //     }
    //     return ss.str();
    // }

    template <class T, size_t N>
    std::string repr(const fixed_queue<T, N>& q)
    {
        std::ostringstream ss;
        ss << '|';
        for (size_t i = 0; i < N; i++) {
            const T *p = reinterpret_cast<const T *>(q.m_store + i);
            if (i)
                ss << ' ';
            if (p == q.m_begin)
                ss << '<';
            ss << *p;
        }
        ss << '|';
        return ss.str();
    }

    void test_equality()
    {
        for (size_t r = 0; r < 4; r++) {
            // q0 = (empty)
            // q1 = <1]   (rotated)
            // q2 = <1 2] (rotated)
            // q3 = <1 3]
            // q4 = <1 2]
            fixed_queue<int, 3> q0, q1, q2, q3, q4;
            rotate_in(q1, r, {1});
            rotate_in(q2, r, {1, 2});
            q3.push(1);
            q3.push(3);
            q4.push(1);
            q4.push(2);

            TS_ASSERT( (q0 == q0));
            TS_ASSERT(!(q0 == q1));
            TS_ASSERT(!(q0 == q2));
            TS_ASSERT(!(q0 == q3));
            TS_ASSERT(!(q0 == q4));

            TS_ASSERT(!(q1 == q0));
            TS_ASSERT( (q1 == q1));
            TS_ASSERT(!(q1 == q2));
            TS_ASSERT(!(q1 == q3));
            TS_ASSERT(!(q1 == q4));

            TS_ASSERT(!(q2 == q0));
            TS_ASSERT(!(q2 == q1));
            TS_ASSERT( (q2 == q2));
            TS_ASSERT(!(q2 == q3));
            // TS_TRACE("r = " + std::to_string(r));
            // TS_TRACE("q2 = " + repr(q2) + ", q4 = " + repr(q4));
            TS_ASSERT( (q2 == q4));

            TS_ASSERT(!(q3 == q0));
            TS_ASSERT(!(q3 == q1));
            TS_ASSERT(!(q3 == q2));
            TS_ASSERT( (q3 == q3));
            TS_ASSERT(!(q3 == q4));

            TS_ASSERT(!(q4 == q0));
            TS_ASSERT(!(q4 == q1));
            TS_ASSERT( (q4 == q2));
            TS_ASSERT(!(q4 == q3));
            TS_ASSERT( (q4 == q4));
        }
    }

    void test_inequality()
    {
        for (size_t r = 0; r < 4; r++) {
            fixed_queue<int, 3> q0, q1, q2, q3, q4;
            rotate_in(q1, r, {1});
            rotate_in(q2, r, {1, 2});
            q3.push(1);
            q3.push(3);
            q4.push(1);
            q4.push(2);

            TS_ASSERT(!(q0 != q0));
            TS_ASSERT( (q0 != q1));
            TS_ASSERT( (q0 != q2));
            TS_ASSERT( (q0 != q3));
            TS_ASSERT( (q0 != q4));

            TS_ASSERT( (q1 != q0));
            TS_ASSERT(!(q1 != q1));
            TS_ASSERT( (q1 != q2));
            TS_ASSERT( (q1 != q3));
            TS_ASSERT( (q1 != q4));

            TS_ASSERT( (q2 != q0));
            TS_ASSERT( (q2 != q1));
            TS_ASSERT(!(q2 != q2));
            TS_ASSERT( (q2 != q3));
            // TS_TRACE("r = " + std::to_string(r));
            // TS_TRACE("q2 = " + repr(q2) + ", q4 = " + repr(q4));
            TS_ASSERT(!(q2 != q4));

            TS_ASSERT( (q3 != q0));
            TS_ASSERT( (q3 != q1));
            TS_ASSERT( (q3 != q2));
            TS_ASSERT(!(q3 != q3));
            TS_ASSERT( (q3 != q4));

            TS_ASSERT( (q4 != q0));
            TS_ASSERT( (q4 != q1));
            TS_ASSERT(!(q4 != q2));
            TS_ASSERT( (q4 != q3));
            TS_ASSERT(!(q4 != q4));
        }
    }

    void test_less_than()
    {
        for (size_t r = 0; r < 4; r++) {
            fixed_queue<int, 3> q0, q1, q2, q3, q4;
            rotate_in(q1, r, {1});
            rotate_in(q2, r, {1, 2});
            q3.push(1);
            q3.push(3);
            q4.push(1);
            q4.push(2);

            TS_ASSERT(!(q0 < q0));
            TS_ASSERT( (q0 < q1));
            TS_ASSERT( (q0 < q2));
            TS_ASSERT( (q0 < q3));
            TS_ASSERT( (q0 < q4));

            TS_ASSERT(!(q1 < q0));
            TS_ASSERT(!(q1 < q1));
            TS_ASSERT( (q1 < q2));
            TS_ASSERT( (q1 < q3));
            TS_ASSERT( (q1 < q4));

            TS_ASSERT(!(q2 < q0));
            TS_ASSERT(!(q2 < q1));
            TS_ASSERT(!(q2 < q2));
            TS_ASSERT( (q2 < q3));
            TS_ASSERT(!(q2 < q4));

            TS_ASSERT(!(q3 < q0));
            TS_ASSERT(!(q3 < q1));
            TS_ASSERT(!(q3 < q2));
            TS_ASSERT(!(q3 < q3));
            TS_ASSERT(!(q3 < q4));

            TS_ASSERT(!(q4 < q0));
            TS_ASSERT(!(q4 < q1));
            TS_ASSERT(!(q4 < q2));
            TS_ASSERT( (q4 < q3));
            TS_ASSERT(!(q4 < q4));
        }
    }

    void test_less_or_equal()
    {
        for (size_t r = 0; r < 4; r++) {
            fixed_queue<int, 3> q0, q1, q2, q3, q4;
            rotate_in(q1, r, {1});
            rotate_in(q2, r, {1, 2});
            q3.push(1);
            q3.push(3);
            q4.push(1);
            q4.push(2);

            TS_ASSERT( (q0 <= q0));
            TS_ASSERT( (q0 <= q1));
            TS_ASSERT( (q0 <= q2));
            TS_ASSERT( (q0 <= q3));
            TS_ASSERT( (q0 <= q4));

            TS_ASSERT(!(q1 <= q0));
            TS_ASSERT( (q1 <= q1));
            TS_ASSERT( (q1 <= q2));
            TS_ASSERT( (q1 <= q3));
            TS_ASSERT( (q1 <= q4));

            TS_ASSERT(!(q2 <= q0));
            TS_ASSERT(!(q2 <= q1));
            TS_ASSERT( (q2 <= q2));
            TS_ASSERT( (q2 <= q3));
            TS_ASSERT( (q2 <= q4));

            TS_ASSERT(!(q3 <= q0));
            TS_ASSERT(!(q3 <= q1));
            TS_ASSERT(!(q3 <= q2));
            TS_ASSERT( (q3 <= q3));
            TS_ASSERT(!(q3 <= q4));

            TS_ASSERT(!(q4 <= q0));
            TS_ASSERT(!(q4 <= q1));
            TS_ASSERT( (q4 <= q2));
            TS_ASSERT( (q4 <= q3));
            TS_ASSERT( (q4 <= q4));
        }
    }

    void test_greater_than()
    {
        for (size_t r = 0; r < 4; r++) {
            fixed_queue<int, 3> q0, q1, q2, q3, q4;
            rotate_in(q1, r, {1});
            rotate_in(q2, r, {1, 2});
            q3.push(1);
            q3.push(3);
            q4.push(1);
            q4.push(2);

            TS_ASSERT(!(q0 > q0));
            TS_ASSERT(!(q0 > q1));
            TS_ASSERT(!(q0 > q2));
            TS_ASSERT(!(q0 > q3));
            TS_ASSERT(!(q0 > q4));

            TS_ASSERT( (q1 > q0));
            TS_ASSERT(!(q1 > q1));
            TS_ASSERT(!(q1 > q2));
            TS_ASSERT(!(q1 > q3));
            TS_ASSERT(!(q1 > q4));

            TS_ASSERT( (q2 > q0));
            TS_ASSERT( (q2 > q1));
            TS_ASSERT(!(q2 > q2));
            TS_ASSERT(!(q2 > q3));
            TS_ASSERT(!(q2 > q4));

            TS_ASSERT( (q3 > q0));
            TS_ASSERT( (q3 > q1));
            TS_ASSERT( (q3 > q2));
            TS_ASSERT(!(q3 > q3));
            TS_ASSERT( (q3 > q4));

            TS_ASSERT( (q4 > q0));
            TS_ASSERT( (q4 > q1));
            TS_ASSERT(!(q4 > q2));
            TS_ASSERT(!(q4 > q3));
            TS_ASSERT(!(q4 > q4));
        }
    }

    void test_greater_equal()
    {
        for (size_t r = 0; r < 4; r++) {
            fixed_queue<int, 3> q0, q1, q2, q3, q4;
            rotate_in(q1, r, {1});
            rotate_in(q2, r, {1, 2});
            q3.push(1);
            q3.push(3);
            q4.push(1);
            q4.push(2);

            TS_ASSERT( (q0 >= q0));
            TS_ASSERT(!(q0 >= q1));
            TS_ASSERT(!(q0 >= q2));
            TS_ASSERT(!(q0 >= q3));
            TS_ASSERT(!(q0 >= q4));

            TS_ASSERT( (q1 >= q0));
            TS_ASSERT( (q1 >= q1));
            TS_ASSERT(!(q1 >= q2));
            TS_ASSERT(!(q1 >= q3));
            TS_ASSERT(!(q1 >= q4));

            TS_ASSERT( (q2 >= q0));
            TS_ASSERT( (q2 >= q1));
            TS_ASSERT( (q2 >= q2));
            TS_ASSERT(!(q2 >= q3));
            TS_ASSERT( (q2 >= q4));

            TS_ASSERT( (q3 >= q0));
            TS_ASSERT( (q3 >= q1));
            TS_ASSERT( (q3 >= q2));
            TS_ASSERT( (q3 >= q3));
            TS_ASSERT( (q3 >= q4));

            TS_ASSERT( (q4 >= q0));
            TS_ASSERT( (q4 >= q1));
            TS_ASSERT( (q4 >= q2));
            TS_ASSERT(!(q4 >= q3));
            TS_ASSERT( (q4 >= q4));
        }
    }

};
