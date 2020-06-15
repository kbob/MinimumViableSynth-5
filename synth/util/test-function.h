#include "function.h"

#include <sstream>
#include <string>

#include <cxxtest/TestSuite.h>

class function_unit_test : public CxxTest::TestSuite {

public:

    static class Logger {
    public:
        std::ostringstream ss;
        void clear() { ss.str(""); }
        std::string operator () () { return ss.str(); }
    } log;

    typedef std::string DATA;
    typedef std::string ARG1;
    typedef bool ARG2;
    typedef function<void()> FV0;
    typedef function<void(const ARG1&)> FV1;
    typedef function<void(const ARG1&, ARG2)> FV2;
    typedef function<int(const ARG1&, ARG2)> FI2;

    static void fv0(void *data)
    {
        auto s = static_cast<DATA *>(data);
        log.ss << "fv0(" << (s ? *s : "_") << ") ";
    }

    static void fv1(void *data, const ARG1& arg1)
    {
        auto s = static_cast<DATA *>(data);
        log.ss << "fv1("
               << (s ? *s : "_")
               << ", "
               << arg1
               << ") ";
    }

    static void fv2(void *data, const ARG1& arg1, ARG2 arg2)
    {
        auto s = static_cast<DATA *>(data);
        log.ss << std::boolalpha
               << "fv2("
               << (s ? *s : "_")
               << ", "
               << arg1
               << ", "
               << arg2
               << ") ";
    }

    static int fi2(void *data, const ARG1& arg1, ARG2 arg2)
    {
        auto s = static_cast<DATA *>(data);
        log.ss << std::boolalpha
               << "fi2("
               << (s ? *s : "_")
               << ", "
               << arg1
               << ", "
               << arg2
               << ") ";
        return arg2 ? +42 : -42;
    }

    class obj {

    public:

        obj(const char *p) : name{p} {}
        const char *name;

        void mv0()
        {
            log.ss << name << ".mv0() ";
        }

        void mv1(const ARG1& arg1)
        {
            log.ss << name << ".mv1(" << arg1 << ") ";
        }

        void mv2(const ARG1& arg1, ARG2 arg2)
        {
            log.ss << std::boolalpha
                   << name
                   << ".mv2("
                   << arg1
                   << ", "
                   << arg2
                   << ") ";
        }

        int mi2(const ARG1& arg1, ARG2 arg2)
        {
            log.ss << std::boolalpha
                   << name
                   << ".mi2("
                   << arg1
                   << ", "
                   << arg2
                   << ") ";
            return arg2 ? +24 : -24;
        }

        int mi2b(const ARG1& arg1, ARG2 arg2)
        {
            log.ss << std::boolalpha
                   << name
                   << ".mi2b("
                   << arg1
                   << ", "
                   << arg2
                   << ") ";
            return arg2 ? +123 : -321;
        }

    };

    void test_instantiate()
    {
        (void)function<void(int)>();
        // test_me();
    }

    void test_default_constructor()
    {
        FV0 f;
        TS_ASSERT(!bool(f));
        TS_ASSERT_EQUALS(f.m_fun, nullptr);
        TS_ASSERT_EQUALS(f.m_data, nullptr);
    }

    void test_constructor()
    {
        DATA d("d0");
        FV0 f(fv0, &d);
        TS_ASSERT(bool(f));
        TS_ASSERT_EQUALS(f.m_fun, fv0);
        TS_ASSERT_EQUALS(f.m_data, (void *)&d);
    }

    void test_copy_constructor()
    {
        FV0 f1;
        FV0 f1c(f1);
        TS_ASSERT(!bool(f1c));
        TS_ASSERT_EQUALS(f1c.m_fun, nullptr);
        TS_ASSERT_EQUALS(f1c.m_data, nullptr);

        DATA d2("d2");
        FV0 f2(fv0, &d2);
        FV0 f2c(f2);
        TS_ASSERT(bool(f2c));
        TS_ASSERT_EQUALS(f2c.m_fun, fv0);
        TS_ASSERT_EQUALS(f2c.m_data, (void *)&d2);
    }

    void test_assignment()
    {
        FV0 f1a;
        FV0 f1;
        f1a = f1;
        TS_ASSERT(!bool(f1a));
        TS_ASSERT_EQUALS(f1a.m_fun, nullptr);
        TS_ASSERT_EQUALS(f1a.m_data, nullptr);

        FV0 f2a;
        DATA d2("d2");
        FV0 f2(fv0, &d2);
        f2a = f2;
        TS_ASSERT(bool(f2a));
        TS_ASSERT_EQUALS(f2a.m_fun, fv0);
        TS_ASSERT_EQUALS(f2a.m_data, (void *)&d2);
    }

    void test_call()
    {
        log.clear();
        DATA d0("d0");
        FV0 f0(fv0, &d0);
        f0();
        TS_ASSERT_EQUALS(log(), "fv0(d0) ");

        log.clear();
        DATA d1("d1");
        FV1 f1(fv1, &d1);
        ARG1 a1("a1");
        f1(a1);
        TS_ASSERT_EQUALS(log(), "fv1(d1, a1) ");

        log.clear();
        DATA d2("d2");
        FV2 f2(fv2, &d2);
        // ARG1 a1("a1");
        ARG2 a2(true);
        f2(a1, a2);
        TS_ASSERT_EQUALS(log(), "fv2(d2, a1, true) ");
    }

    void test_return()
    {
        DATA d2("d2");
        FI2 f2(fi2, &d2);
        ARG1 a1("a1");

        log.clear();
        int r2t = f2(a1, true);
        TS_ASSERT_EQUALS(r2t, +42);
        TS_ASSERT_EQUALS(log(), "fi2(d2, a1, true) ");

        log.clear();
        int r2f = f2(a1, false);
        TS_ASSERT_EQUALS(r2f, -42);
        TS_ASSERT_EQUALS(log(), "fi2(d2, a1, false) ");
    }


    void test_relops()
    {
        DATA d0("d0");
        FV0 f0n;
        FV0 f0p(fv0, &d0);
        TS_ASSERT( (f0n == nullptr));
        TS_ASSERT(!(f0p == nullptr));
        TS_ASSERT(!(f0n != nullptr));
        TS_ASSERT( (f0p != nullptr));
        TS_ASSERT( (nullptr == f0n));
        TS_ASSERT(!(nullptr == f0p));
        TS_ASSERT(!(nullptr != f0n));
        TS_ASSERT( (nullptr != f0p));
    }

    void test_binding()
    {
        obj o1("obj1");
        obj o2("obj2");
        ARG1 a11("a11");
        ARG1 a12("a12");

        log.clear();
        FV0 fv01((FV0::binding<obj, &obj::mv0>(&o1)));
        FV0 fv02((FV0::binding<obj, &obj::mv0>(&o2)));
        fv01();
        fv02();
        TS_ASSERT_EQUALS(log(), "obj1.mv0() obj2.mv0() ");

        log.clear();
        FV1 fv11((FV1::binding<obj, &obj::mv1>(&o1)));
        FV1 fv12((FV1::binding<obj, &obj::mv1>(&o2)));
        fv11(a11);
        fv12(a12);
        TS_ASSERT_EQUALS(log(), "obj1.mv1(a11) obj2.mv1(a12) ");

        log.clear();
        FV2 fv21((FV2::binding<obj, &obj::mv2>(&o1)));
        FV2 fv22((FV2::binding<obj, &obj::mv2>(&o2)));
        fv21(a11, true);
        fv22(a12, false);
        TS_ASSERT_EQUALS(log(), "obj1.mv2(a11, true) obj2.mv2(a12, false) ");

        log.clear();
        FI2 fi21((FI2::binding<obj, &obj::mi2>(&o1)));
        FI2 fi22((FI2::binding<obj, &obj::mi2>(&o2)));
        int r1t = fi21(a11, true);
        int r2f = fi22(a12, false);
        TS_ASSERT_EQUALS(r1t, +24);
        TS_ASSERT_EQUALS(r2f, -24);
        TS_ASSERT_EQUALS(log(), "obj1.mi2(a11, true) obj2.mi2(a12, false) ");
    }

    void test_binding_two_methods()
    {
        obj o("obj");
        ARG1 a11("a11");
        ARG1 a12("a12");

        log.clear();
        FI2 fi2((FI2::binding<obj, &obj::mi2>(&o)));
        FI2 fi2b((FI2::binding<obj, &obj::mi2b>(&o)));
        int r1t = fi2(a11, true);
        int r2f = fi2b(a12, false);
        TS_ASSERT_EQUALS(r1t, +24);
        TS_ASSERT_EQUALS(r2f, -321);
        TS_ASSERT_EQUALS(log(), "obj.mi2(a11, true) obj.mi2b(a12, false) ");
    }

    void test_descriptor()
    {
        class new_obj {
        public:
            int mi2(const ARG1& a1, ARG2 a2)
            {
                log.ss << std::boolalpha
                       << "dmi2("
                       << a1
                       << ", "
                       << a2
                       <<") ";
                return 867-5309;
            }
            FI2 mi2_desc;
            new_obj()
            : mi2_desc{FI2::binding<new_obj, &new_obj::mi2>(this)}
            {}
        };

        log.clear();
        new_obj d;
        FI2 my_function = d.mi2_desc;
        int r = my_function("a1", true);
        TS_ASSERT_EQUALS(sizeof d.mi2_desc, 2 * sizeof (void *));
        TS_ASSERT_EQUALS(r, 867-5309);
        TS_ASSERT_EQUALS(log(), "dmi2(a1, true) ");
    }

    void test_sizeof()
    {
        obj o("obj");

        FV0 f0n;
        TS_ASSERT_EQUALS(sizeof f0n, 2 * sizeof (void *));

        DATA d0("d0");
        FV0 f0(fv0, &d0);
        TS_ASSERT_EQUALS(sizeof f0, 2 * sizeof (void *));

        DATA d1("d1");
        FV1 f1(fv1, &d1);
        TS_ASSERT_EQUALS(sizeof f1, 2 * sizeof (void *));

        DATA d2("d2");
        FV2 f2(fv2, &d2);
        TS_ASSERT_EQUALS(sizeof f2, 2 * sizeof (void *));

        FI2 f2i(fi2, &d2);
        TS_ASSERT_EQUALS(sizeof f2i, 2 * sizeof (void *));

        FV0 mv0((FV0::binding<obj, &obj::mv0>(&o)));
        TS_ASSERT_EQUALS(sizeof mv0, 2 * sizeof (void *));

        FV1 mv1((FV1::binding<obj, &obj::mv1>(&o)));
        TS_ASSERT_EQUALS(sizeof mv1, 2 * sizeof (void *));

        FV2 mv2((FV2::binding<obj, &obj::mv2>(&o)));
        TS_ASSERT_EQUALS(sizeof mv2, 2 * sizeof (void *));
    }

};

function_unit_test::Logger function_unit_test::log;
