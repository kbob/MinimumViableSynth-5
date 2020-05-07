#ifndef UNIVERSE_included
#define UNIVERSE_included

#include <algorithm>
#include <bitset>
#include <cassert>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

template <class C, size_t N>
class Subset;


// -- Universe -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
//  Examples:
//      Universe<char, 3> u{"abc"};
//      u == u
//      u != u2
//      u.size()                // 3
//      u[0]                    // 'a' - undefined if out of range
//      u.at(1)                 // 'b' - throws if out of range
//      u.find('b')             // 1; -1 if not present
//      u.index('c')            // 2; throws if not present
//      ostream << u            // "{a b c}"
//
//      // Subset creation
//      std::string s{"bc"};
//      u.subset(0b101)         // {a c}
//      u.subset(s.begin(), s.end())
//                              // {b c}
//      u.none                  // {}
//      u.all                   // {a b c}

template <class C, size_t N>
class Universe {

public:

    typedef C referent;
    typedef typename C::value_type member_type;
    typedef Subset<referent, N> subset_type;
    typedef std::bitset<N> bits;
    static const size_t max_size = N;

    Universe(const referent& ref)
    : all{*this, (1 << ref.size()) - 1},
      none{*this},
      m_ref{ref}
    {}

    Universe(const referent& ref, size_t size)
    : all{*this, (1 << size) - 1},
      none{*this},
      m_ref{ref}
    {}

    // No two universes are equal.
    bool operator == (const Universe& that) const { return this == &that; }
    bool operator != (const Universe& that) const { return this != &that; }

    size_t size() const { return m_ref.size(); }

    ssize_t find(const member_type& m) const
    {
        auto pos = std::find(m_ref.begin(), m_ref.end(), m);
        if (pos == m_ref.end())
            return -1;
        return pos - m_ref.begin();
    }

    size_t index(const member_type& m) const
    {
        ssize_t pos = find(m);
        if (pos == -1)
            throw std::logic_error("universe");
        return pos;
    }

    const member_type& operator [] (size_t index) const
    {
        return m_ref[index];
    }

    const member_type& at(size_t index) const
    {
        return m_ref.at(index);
    }

    subset_type subset(const bits& b) const { return subset_type(*this, b); }

    template <class I>
    subset_type subset(I first, I last) const
    {
        return subset_type(*this, first, last);
    }

    friend std::ostream& operator << (std::ostream& o, const Universe& u)
    {
        o << '{';
        const char *sep = "";
        for (size_t i = 0; i < u.size(); i++) {
            o << sep << u[i];
            sep = " ";
        }
        return o << '}';
    }

    const subset_type all;
    const subset_type none;

private:

    const referent& m_ref;
    friend subset_type;

};


// -- Subset - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
// Subsets are created by Universes.
//
// Examples:
//      s = u.subset(0b101)     // s = {a c}
//      s == s2                 // set equality
//      s != s2                 // set inequality
//      s < s2                  // proper subset
//      s <= s2                 // subset
//      s >= s2                 // superset
//      s > s2                  // proper superset
//      // subsets can also be compared to std::bitset<> and to
//      // integers that coerce to bitsets.
//
//      // Membership
//      s.add('a')              // add 'a' to s1
//      s.remove('c')           // throws if 'c' not in s
//      s.discard('b')          // OK if 'b' not in s
//      s.contains('b')         // true iff s contains 'b'
//      s.test(2)               // true iff s contains 'c'
//      for (auto i: s.indices()) {} // iterate by index: 0, 2
//      for (auto v: s.members()) {}  // iterate by member: 'a', 'c'
//      ostream << s            // "{a c}"
//
//      // Operators
//      s1 | s2, s1 |= s2       // union
//      s1 & s2, s1 &= s2       // intersection
//      s1 ^ s2, s1 ^= s2       // set symmetric difference
//      s1 - s2, s1 -= s2       // set difference

template <class C, size_t N>
class Subset : public std::bitset<N> {

    typedef std::bitset<N> super;

public:
    typedef typename C::value_type member_type;

private:
    Subset(const Universe<C, N>& u)
    : m_universe{&u} {}

    Subset(const Universe<C, N>& u, const super& bits)
    : super{bits}, m_universe{&u} {}

    template <class I>
    Subset(const Universe<C, N>& u, I first, I last)
    : m_universe{&u}
    {
        auto b = m_universe->m_ref.begin();
        auto e = m_universe->m_ref.end();
        for (auto i = first; i != last; i++) {
            auto there = std::find(b, e, *i);
            auto j = there - b;
            super::set(j, true);
        }
    }

    class index_iter
    : public std::iterator<std::forward_iterator_tag, size_t>
    {
    public:
        index_iter() : m_sub{nullptr}, m_index{0} {}
        index_iter(const index_iter& that) = default;
        index_iter(const Subset *s, size_t index)
        : m_sub{s}, m_index{advance(index)} {}
        index_iter& operator = (const index_iter&) = default;

        bool operator == (const index_iter& that) const
        {
            assert(m_sub == that.m_sub);
            return m_index == that.m_index;
        }

        bool operator != (const index_iter& that) const
        {
            assert(m_sub == that.m_sub);
            return m_index != that.m_index;
        }

        size_t operator * () const
        {
            assert(m_sub);
            assert(m_index < m_sub->size());
            return m_index;
        }

        const size_t *operator -> () const
        {
            assert(m_sub);
            assert(m_index < m_sub->size());
            return &m_index;
        }

        index_iter& operator ++ ()
        {
            m_index = advance(m_index + 1);
            return *this;
        }

        index_iter operator ++ (int)
        {
            index_iter i = *this;
            ++*this;
            return i;
        }

    private:
        size_t advance(size_t index)
        {
            while (index < m_sub->size() && !m_sub->test(index))
                index++;
            return index;
        }

        const Subset *m_sub;
        size_t m_index;
        friend class Subset::member_iter;
    };

    class member_iter {
    public:
        member_iter() = default;
        member_iter(const member_iter&) = default;
        member_iter(const Subset *sub, size_t index) : m_iter(sub, index) {}
        member_iter& operator = (const member_iter&) = default;

        bool operator == (const member_iter& that) const
        {
            return m_iter == that.m_iter;
        }

        bool operator != (const member_iter& that) const
        {
            return m_iter != that.m_iter;
        }

        const member_type& operator * () const
        {
            assert(m_iter.m_sub);
            assert(m_iter.m_sub->m_universe);
            return m_iter.m_sub->m_universe->m_ref[*m_iter];
        }

        const member_type *operator -> () const
        {
            return &**this;
        }

        member_iter& operator ++ ()
        {
            ++m_iter;
            return *this;
        }

        member_iter operator ++ (int)
        {
            member_iter i = *this;
            ++*this;
            return i;
        }

    private:
        index_iter m_iter;
    };

    class index_iterable {
    public:
        index_iterable(const Subset *sub) : m_sub{sub} {}
        index_iter begin() const { return index_iter(m_sub, 0); }
        index_iter end() const { return index_iter(m_sub, m_sub->size()); }
    private:
        const Subset *m_sub;
    };

    class member_iterable {
    public:
        member_iterable(const Subset *sub) : m_sub{sub} {}
        member_iter begin() const { return member_iter(m_sub, 0); }
        member_iter end() const { return member_iter(m_sub, m_sub->size()); }
    private:
        const Subset *m_sub;
    };

public:

    Subset(const Subset&) = default;
    Subset& operator = (const Subset&) = default;

    friend bool operator == (const Subset& a, const Subset& b)
    {
        assert(a.m_universe == b.m_universe);
        return a.super::operator == (b);
    }

    friend bool operator != (const Subset& a, const Subset& b)
    {
        assert(a.m_universe == b.m_universe);
        return a.super::operator != (b);
    }

    friend bool operator < (const Subset& a, const Subset& b)
    {
        assert(a.m_universe == b.m_universe);
        return (a & ~b) == 0 && a != b;
    }

    friend bool operator <= (const Subset& a, const Subset& b)
    {
        assert(a.m_universe == b.m_universe);
        return (a & ~b) == 0;
    }

    friend bool operator >= (const Subset& a, const Subset& b)
    {
        assert(a.m_universe == b.m_universe);
        return (~a & b) == 0;
    }

    friend bool operator > (const Subset& a, const Subset& b)
    {
        assert(a.m_universe == b.m_universe);
        return (a & ~b) != 0 && (~a & b) == 0;
    }

    bool contains(const member_type& member) const
    {
        return (*this)[m_universe->index(member)];
    }

    Subset operator & (const Subset& that) const
    {
        assert(m_universe == that.m_universe);
        super prod = static_cast<super>(*this) & static_cast<super>(that);
        return Subset(*m_universe, prod);
    }

    Subset operator ^ (const Subset& that) const
    {
        assert(m_universe == that.m_universe);
        super sdif = static_cast<super>(*this) ^ static_cast<super>(that);
        return Subset(*m_universe, sdif);
    }

    Subset operator | (const Subset& that) const
    {
        assert(m_universe == that.m_universe);
        super sum = static_cast<super>(*this) | static_cast<super>(that);
        return Subset(*m_universe, sum);
    }

    Subset operator - (const Subset& that) const
    {
        assert(m_universe == that.m_universe);
        return Subset(*m_universe, *this & ~that);
    }

    Subset& operator -= (const Subset& that)
    {
        assert(m_universe == that.m_universe);
        *this &= ~that;
        return *this;
    }

    void add(const member_type& member)
    {
        super::set(m_universe->index(member));
    }

    void remove(const member_type& member)
    {
        ssize_t idx = m_universe->index(member);
        if (!super::test(idx))
            throw std::runtime_error("subset");
        super::reset(idx);
    }

    void discard(const member_type& member)
    {
        super::reset(m_universe->index(member));
    }

    const index_iterable indices() const
    {
        return index_iterable(this);
    }

    const member_iterable members() const
    {
        return member_iterable(this);
    }

    friend std::ostream& operator << (std::ostream& o, Subset s)
    {
        o << "{";
        const char *sep = "";
        for (const auto& v: s.members()) {
            o << sep << v;
            sep = " ";
        }
        return o << "}";
    }

private:

    const Universe<C, N> *m_universe;

    friend Universe<C, N>;
    friend class subset_unit_test;
};

template <size_t N>
inline bool operator < (const std::bitset<N>& a, const std::bitset<N>& b)
{
    return (a & ~b) == 0 && (~a & b) != 0;
}

template <size_t N>
inline bool operator <= (const std::bitset<N>& a, const std::bitset<N>& b)
{
    return (a & ~b) == 0;
}

template <size_t N>
inline bool operator >= (const std::bitset<N>& a, const std::bitset<N>& b)
{
    return (~a & b) == 0;
}

template <size_t N>
inline bool operator > (const std::bitset<N>& a, const std::bitset<N>& b)
{
    return (a & ~b) != 0 && (~a & b) == 0;
}

#endif /* !UNIVERSE_included */
