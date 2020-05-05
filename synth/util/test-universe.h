#include "universe.h"

#include <sstream>
#include <vector>

#include <cxxtest/TestSuite.h>

#include "synth/util/fixed-vector.h"

class universe_unit_test : public CxxTest::TestSuite {

public:

    enum class Turtle { Leo, Mich, Don, Raph, Splinter };
    // N.B., Splinter is not a turtle.

    typedef fixed_vector<Turtle, 4> V;
    typedef Universe<V, 4> U;
    typedef U::subset_type S;
    const V turtles = {Turtle::Leo, Turtle::Mich, Turtle::Don, Turtle::Raph};
    const U u{turtles};

    void test_instantiate()
    {
        (void)U(turtles);

        // auto u = U(turtles);
        // auto s = u.none;
        // auto i = s.indices();
        // auto& v = s.members();
        //
        // std::cout << "\n\nsizeof U = " << sizeof u << "\n";
        // std::cout << "sizeof S = " << sizeof s << "\n";
        // std::cout << "sizeof indices = " << sizeof i << "\n";
        // std::cout << "sizeof index_iter = " << sizeof i.begin() << "\n";
        // std::cout << "sizeof members = " << sizeof v << "\n";
        // std::cout << "sizeof member_iter = " << sizeof v.begin() << "\n";
        // std::cout << std::endl;
    }

    void test_equality()
    {
        const U u(turtles);
        const U u1(turtles);
        // const Universe<int, 3> u3{fixed_vector<int, 3>{42, 43, 45}};
        fixed_vector<int, 3> ref3{42, 43, 45};
        const Universe<fixed_vector<int, 3>, 3> u3{ref3};
        TS_ASSERT(u == u);
        TS_ASSERT(!(u == u1));
        TS_ASSERT(u != u1);
    }

    void test_size()
    {
        const U u{turtles};
        TS_ASSERT_EQUALS(u.size(), 4)

        typedef Universe<fixed_vector<Turtle, 17>, 17> U2;
        typedef U2::referent V2;
        const V2 turtles2(turtles.begin(), turtles.end());
        const U2 u2(turtles2);
        TS_ASSERT_EQUALS(u2.size(), 4);
    }

    void test_find()
    {
        const U u(turtles);
        TS_ASSERT_EQUALS(u.find(Turtle::Leo), 0);
        TS_ASSERT_EQUALS(u.find(Turtle::Mich), 1);
        TS_ASSERT_EQUALS(u.find(Turtle::Don), 2);
        TS_ASSERT_EQUALS(u.find(Turtle::Raph), 3);
        TS_ASSERT_EQUALS(u.find(Turtle::Splinter), -1);
    }

    void test_index()
    {
        const U u(turtles);
        TS_ASSERT_EQUALS(u.index(Turtle::Leo), 0);
        TS_ASSERT_EQUALS(u.index(Turtle::Mich), 1);
        TS_ASSERT_EQUALS(u.index(Turtle::Don), 2);
        TS_ASSERT_EQUALS(u.index(Turtle::Raph), 3);
        TS_ASSERT_THROWS(u.index(Turtle::Splinter), std::logic_error);
    }

    void test_subscript()
    {
        const U u(turtles);
        TS_ASSERT_EQUALS(u[0], Turtle::Leo);
        TS_ASSERT_EQUALS(u[1], Turtle::Mich);
        TS_ASSERT_EQUALS(u[2], Turtle::Don);
        TS_ASSERT_EQUALS(u[3], Turtle::Raph);
    }

    void test_at()
    {
        const U u(turtles);
        TS_ASSERT_EQUALS(u.at(0), Turtle::Leo);
        TS_ASSERT_EQUALS(u.at(1), Turtle::Mich);
        TS_ASSERT_EQUALS(u.at(2), Turtle::Don);
        TS_ASSERT_EQUALS(u.at(3), Turtle::Raph);
        TS_ASSERT_THROWS(u.at(4), std::out_of_range);
    }

    void test_subset_bits()
    {
        const U u(turtles);
        auto s = u.subset(U::bits(0b0101));
        TS_ASSERT(s[0] == true);
        TS_ASSERT(s[1] == false);
        TS_ASSERT(s[2] == true);
        TS_ASSERT(s[3] == false);
    }

    void test_subset_range()
    {
        const U u(turtles);
        std::vector<Turtle> some_turtles = {Turtle::Mich, Turtle::Raph};
        auto s = u.subset(some_turtles.begin(), some_turtles.end());
        TS_ASSERT(s[0] == false);
        TS_ASSERT(s[1] == true);
        TS_ASSERT(s[2] == false);
        TS_ASSERT(s[3] == true);
    }

    void test_all_none()
    {
        const U u(turtles);
        S all = u.all;
        S none = u.none;

        TS_ASSERT(all[0] == true);
        TS_ASSERT(all[1] == true);
        TS_ASSERT(all[2] == true);
        TS_ASSERT(all[3] == true);

        TS_ASSERT(none[0] == false);
        TS_ASSERT(none[1] == false);
        TS_ASSERT(none[2] == false);
        TS_ASSERT(none[3] == false);
    }

    void test_operator_insertion()
    {
        typedef fixed_vector<short, 4> V;
        V v{2, 3, 5, 7};
        Universe<fixed_vector<short, 4>, 4> u{v};
        std::ostringstream stream;
        stream << u;
        std::string str = stream.str();
        TS_ASSERT_EQUALS(stream.str(), "{2 3 5 7}")
    }

};

class subset_unit_test : public CxxTest::TestSuite {

public:

    enum class Turtle { Leo, Mich, Don, Raph, Splinter };
    // N.B., Splinter is not a turtle.

    friend std::ostream& operator << (std::ostream& o, const Turtle& turtle)
    {
        switch (turtle) {
            case Turtle::Leo:
                return o << "Leo";
            case Turtle::Mich:
                return o << "Mich";
            case Turtle::Don:
                return o << "Don";
            case Turtle::Raph:
                return o << "Raph";
            case Turtle::Splinter:
                return o << "Splinter";
            default:
                assert(false);
        }
    }

    typedef Universe<fixed_vector<Turtle, 4>, 4> U;
    typedef U::referent V;
    typedef U::subset_type S;
    const V turtles;

    U u;
    S ld, md, mdr;

    subset_unit_test()
    : turtles{Turtle::Leo, Turtle::Mich, Turtle::Don, Turtle::Raph},
      u{turtles},
      ld{u.subset(0b0101)},
      md{u.subset(0b0110)},
      mdr{u.subset(0b01110)}
    {}

    void test_instantiate()
    {
        const U u(turtles);
        S s(u.subset(U::bits(0b0101)));
        TS_ASSERT(s[0] == true);
        TS_ASSERT(s[1] == false);
        TS_ASSERT(s[2] == true);
        TS_ASSERT(s[3] == false);
    }

    void test_copy_assign()
    {
        S s(ld);

        TS_ASSERT(s[0] == true);
        TS_ASSERT(s[1] == false);
        TS_ASSERT(s[2] == true);
        TS_ASSERT(s[3] == false);

        s.m_universe = nullptr;
        s = md;

        TS_ASSERT(s[0] == false);
        TS_ASSERT(s[1] == true);
        TS_ASSERT(s[2] == true);
        TS_ASSERT(s[3] == false);
        TS_ASSERT(s.m_universe == &u);
    }

    void test_count_size()
    {
        TS_ASSERT(ld.count() == 2);
        TS_ASSERT(ld.size() == 4);
    }

    // void test_bool()
    // {
    //     TS_ASSERT(bool(ld) == true);
    //     TS_ASSERT(bool(u.none) == false);
    // }

    void test_eq()
    {
        S s = md;
        TS_ASSERT(s == md);
        TS_ASSERT(md == md);
        TS_ASSERT(!(md == ld));
        TS_ASSERT(s == 0b0110);
        TS_ASSERT(U::bits(0b0110) == s);
    }

    void test_ne()
    {
        S s = md;
        TS_ASSERT(s != ld);
        TS_ASSERT(!(s != s));
        TS_ASSERT(!(s != md));
        TS_ASSERT(s != 0b0101);
        TS_ASSERT(U::bits(0b0101) != s);
    }

    void test_lt()
    {
        TS_ASSERT(md < mdr);
        TS_ASSERT(!(ld < mdr));
        TS_ASSERT(md < U::bits(0b0111));
        TS_ASSERT(U::bits(0b0100) < md);
    }

    void test_le()
    {
        TS_ASSERT(md <= md);
        TS_ASSERT(md <= mdr);
        TS_ASSERT(!(ld <= mdr));
        TS_ASSERT(md <= U::bits(0b0110));
        TS_ASSERT(U::bits(0b0110) <= md);
    }

    void test_ge()
    {
        TS_ASSERT(md >= md);
        TS_ASSERT(mdr >= md);
        TS_ASSERT(!(mdr >= ld));
        TS_ASSERT(md >= U::bits(0b0110));
        TS_ASSERT(U::bits(0b0110) >= md);
    }

    void test_gt()
    {
        TS_ASSERT(!(md > md));
        TS_ASSERT(mdr > md);
        TS_ASSERT(!(mdr > ld));
        TS_ASSERT(mdr > U::bits(0b0110));
        TS_ASSERT(U::bits(0b0111) > md);
    }

    void test_all_any_none()
    {
        TS_ASSERT(u.none.none());
        TS_ASSERT(!u.none.any());
        TS_ASSERT(!u.none.all());
        TS_ASSERT(!ld.none());
        TS_ASSERT(ld.any());
        TS_ASSERT(!ld.all());
        TS_ASSERT(!u.all.none());
        TS_ASSERT(u.all.any());
        TS_ASSERT(u.all.all());
    }

    void test_contains()
    {
        TS_ASSERT(ld.contains(Turtle::Leo));
        TS_ASSERT(!ld.contains(Turtle::Mich));
        TS_ASSERT_THROWS(ld.contains(Turtle::Splinter), std::logic_error);
    }

    void test_and_xor_or()
    {
        TS_ASSERT_EQUALS((ld & mdr), U::bits(0b0100));
        TS_ASSERT_EQUALS((u.none & u.none), u.none);
        TS_ASSERT_EQUALS((u.none & ld), u.none);
        TS_ASSERT_EQUALS((u.none & u.all), u.none);
        TS_ASSERT_EQUALS((ld & u.all), ld)
        TS_ASSERT_EQUALS((ld & ld), ld);
        TS_ASSERT_EQUALS((u.all & u.all), u.all);

        TS_ASSERT_EQUALS((ld ^ mdr), U::bits(0b1011));
        TS_ASSERT_EQUALS((u.none ^ u.none), u.none);
        TS_ASSERT_EQUALS((u.none ^ ld), ld);
        TS_ASSERT_EQUALS((u.none ^ u.all), u.all);
        TS_ASSERT_EQUALS((ld ^ u.all), 0b1010);
        TS_ASSERT_EQUALS((ld ^ ld), u.none);
        TS_ASSERT_EQUALS((u.all ^ u.all), u.none);

        TS_ASSERT_EQUALS((ld | md), 0b0111);
        TS_ASSERT_EQUALS((u.none | u.none), u.none);
        TS_ASSERT_EQUALS((u.none | ld), ld);
        TS_ASSERT_EQUALS((u.none | u.all), u.all);
        TS_ASSERT_EQUALS((ld | ld), ld);
        TS_ASSERT_EQUALS((ld | u.all), u.all);
        TS_ASSERT_EQUALS((u.all | u.all), u.all);
    }

    void test_difference()
    {
        TS_ASSERT_EQUALS(mdr - ld, U::bits(0b1010));
        TS_ASSERT_EQUALS(u.none - u.all, u.none);
        TS_ASSERT_EQUALS(u.all - u.all, u.none);
        TS_ASSERT_EQUALS(u.all - u.none, u.all);
        TS_ASSERT_EQUALS(u.all - ld, U::bits(0b1010));
    }

    void test_op_types()
    {
        S diff = mdr - ld;
        TS_ASSERT(typeid(diff) == typeid(mdr - ld));
        TS_ASSERT_EQUALS(diff.m_universe, mdr.m_universe);

        S prod = ld & mdr;
        TS_ASSERT(typeid(prod) == typeid(ld & mdr));
        TS_ASSERT_EQUALS(prod.m_universe, ld.m_universe);

        S sdif = ld ^ mdr;
        TS_ASSERT(typeid(sdif) == typeid(ld ^ mdr));
        TS_ASSERT_EQUALS(prod.m_universe, ld.m_universe);

        S sum = ld | mdr;
        TS_ASSERT(typeid(sum) == typeid(ld | mdr));
        TS_ASSERT_EQUALS(prod.m_universe, ld.m_universe);
    }

    void test_assign_ops()
    {
        S s = ld;
        s &= md;
        TS_ASSERT_EQUALS(s, 0b0100);
        TS_ASSERT_EQUALS(s.m_universe, ld.m_universe);

        s = ld;
        s ^= md;
        TS_ASSERT_EQUALS(s, 0b0011);

        s = ld;
        s |= md;
        TS_ASSERT_EQUALS(s, 0b0111);

        s = ld;
        s -= md;
        TS_ASSERT_EQUALS(s, 0b0001);
    }

    void test_subscript()
    {
        TS_ASSERT(ld[0] == true);
        TS_ASSERT(ld[1] == false);
        TS_ASSERT(ld[2] == true);
        TS_ASSERT(ld[3] == false);
    }

    void test_test()
    {
        TS_ASSERT_EQUALS(ld.test(0), true);
        TS_ASSERT_EQUALS(ld.test(1), false);
        TS_ASSERT_EQUALS(ld.test(2), true);
        TS_ASSERT_EQUALS(ld.test(3), false);
        TS_ASSERT_THROWS(ld.test(4), std::out_of_range);
    }

    void test_set()
    {
        S s = md;
        s.set(3);
        TS_ASSERT_EQUALS(s, mdr);

        s = u.none;
        for (size_t i = 0; i < 4; i++)
            s.set(i);
        TS_ASSERT_EQUALS(s, u.all);
    }

    void test_add()
    {
        S s = md;
        s.add(Turtle::Raph);
        TS_ASSERT_EQUALS(s, mdr);

        s = u.none;
        for (auto t: turtles)
            s.add(t);
        TS_ASSERT_EQUALS(s, u.all);
    }

    void test_index_iterator()
    {
        auto b = ld.indices().begin();
        auto e = ld.indices().end();
        TS_ASSERT(b == b);
        TS_ASSERT(e == e);
        TS_ASSERT(b != e);

        S::index_iter i;        // use default constructor
        i = b;                  // use assignment
        TS_ASSERT(*i == 0);
        TS_ASSERT(*i.operator -> () == 0);
        auto i1 = i++;
        TS_ASSERT(*i == 2);
        TS_ASSERT(*i.operator -> () == 2);
        TS_ASSERT(i1 == b);
        auto i2(++i);           // use copy constructor
        TS_ASSERT(i == e);
        TS_ASSERT(i2 == e);

        size_t j = 0;
        for (size_t it: ld.indices()) {
            TS_ASSERT(j < 2);
            if (j == 0)
                TS_ASSERT(it == 0);
            if (j == 1)
                TS_ASSERT(it == 2);
            j++;
        }
    }

    void test_member_iterator()
    {
        auto b = ld.members().begin();
        auto e = ld.members().end();
        TS_ASSERT(b == b);
        TS_ASSERT(e == e);
        TS_ASSERT(b != e);

        S::member_iter i;        // use default constructor
        i = b;                  // use assignment
        TS_ASSERT(*i == Turtle::Leo);
        TS_ASSERT(*i.operator -> () == Turtle::Leo);
        auto i1 = i++;
        TS_ASSERT(*i == Turtle::Don);
        TS_ASSERT(*i.operator -> () == Turtle::Don);
        TS_ASSERT(i1 == b);
        auto i2(++i);           // use copy constructor
        TS_ASSERT(i == e);
        TS_ASSERT(i2 == e);

        size_t j = 0;
        for (Turtle it: ld.members()) {
            TS_ASSERT(j < 2);
            if (j == 0)
                TS_ASSERT(it == Turtle::Leo);
            if (j == 1)
                TS_ASSERT(it == Turtle::Don);
            j++;
        }
    }

    void test_ostream_inserter()
    {
        std::ostringstream stream;
        stream << ld;
        TS_ASSERT_EQUALS(stream.str(), "{Leo Don}");

        stream.str("");
        stream << u.none;
        TS_ASSERT_EQUALS(stream.str(), "{}");

        stream.str("");
        stream << u.all;
        TS_ASSERT_EQUALS(stream.str(), "{Leo Mich Don Raph}");
    }

};
