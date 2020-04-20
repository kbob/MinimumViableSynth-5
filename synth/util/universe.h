#ifndef UNIVERSE_included
#define UNIVERSE_included

#include <bitset>
#include <iostream>

#include "synth/util/noalloc.h"

template <class T, size_t N>
class Subset;


// -- Universe -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
//  Examples:
//      Universe<char, 3> u{"abc"};
//      u == u
//      u != u2
//      u.size()                // 3
//      u[0]                    // 'a'
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

template <class T, size_t N>
class Universe {

public:

    typedef const fixed_vector<T, N> referent;
    typedef Subset<T, N> Subset;
    typedef std::bitset<N> bits;

    Universe(referent& ref)
    : all{*this, (1 << ref.size()) - 1},
      none{*this},
      m_ref{ref}
    {}

    bool operator == (const Universe& that) const { return this == &that; }
    bool operator != (const Universe& that) const { return this != &that; }

    size_t size() const { return m_ref.size(); }

    ssize_t find(const T& m) const
    {
        auto pos = std::find(m_ref.begin(), m_ref.end(), m);
        if (pos == m_ref.end())
            return -1;
        return pos - m_ref.begin();
    }

    size_t index(const T& m) const
    {
        ssize_t pos = find(m);
        if (pos == -1)
            throw std::logic_error("universe");
        return pos;
    }

    const T& operator [] (size_t index) const { return m_ref.at(index); }

    Subset subset(const bits& b) const { return Subset(*this, b); }

    template <class I>
    Subset subset(I first, I last) const
    {
        return Subset(*this, first, last);
    }

    const Subset all;
    const Subset none;

private:

    referent& m_ref;
    friend Subset;

};

template <class T, size_t N>
std::ostream& operator << (std::ostream& o, const Universe<T, N>& u)
{
    o << "{";
    const char *sep = "";
    for (size_t i = 0; i < u.size(); i++) {
        o << sep << u[i];
        sep = " ";
    }
    o << "}";
    return o;
}

// -- Subset - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
// Subsets are created by Universes.
//
// Examples:
//      s1 = u.subset(0b101)    // s1 = {a c}
//      s1 == s2                // set equality
//      s1 != s2                // set inequality
//      s1 < s2                 // proper subset
//      s1 <= s2                // subset
//      s1 >= s2                // superset
//      s1 > s2                 // proper superset
//      // subsets can also be compared to std::bitset<> and to
//      // integers that coerce to bitsets.
//
//      // Membership
//      s1.add('a')             // add 'a' to s1
//      s1.contains('b')        // true iff s1 contains 'b'
//      s1.test(2)              // true iff s1 contains 'c'
//      for (auto i: s1.indices()) {} // iterate by index: 0, 2
//      for (auto v: s1.values()) {}  // iterate by value: 'a', 'c'
//      ostream << s1           // "{a c}"
//
//      // Operators
//      s1 | s2, s1 |= s2       // union
//      s1 & s2, s1 &= s2       // intersection
//      s1 ^ s2, s1 ^= s2       // set symmetric difference
//      s1 - s2, s1 -= s2       // set difference

template <class T, size_t N>
class Subset : public std::bitset<N> {

    typedef std::bitset<N> super;
    Subset(const Universe<T, N>& u)
    : /* indices{this}, values(this), */ m_universe{&u} {}

    Subset(const Universe<T, N>& u, const super& bits)
    : super{bits}, /* indices{this}, values(this), */ m_universe{&u} {}

    template <class I>
    Subset(const Universe<T, N>& u, I first, I last)
    : /* indices{this}, values(this), */ m_universe{&u}
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
        friend class Subset::value_iter;
    };

    class value_iter {
    public:
        value_iter() = default;
        value_iter(const value_iter&) = default;
        value_iter(const Subset *sub, size_t index) : m_iter(sub, index) {}
        value_iter& operator = (const value_iter&) = default;

        bool operator == (const value_iter& that) const
        {
            return m_iter == that.m_iter;
        }

        bool operator != (const value_iter& that) const
        {
            return m_iter != that.m_iter;
        }

        const T& operator * () const
        {
            assert(m_iter.m_sub);
            assert(m_iter.m_sub->m_universe);
            return m_iter.m_sub->m_universe->m_ref[*m_iter];
        }

        const T *operator -> () const
        {
            return &**this;
        }

        value_iter& operator ++ ()
        {
            ++m_iter;
            return *this;
        }

        value_iter operator ++ (int)
        {
            value_iter i = *this;
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

    class value_iterable {
    public:
        value_iterable(const Subset *sub) : m_sub{sub} {}
        value_iter begin() const { return value_iter(m_sub, 0); }
        value_iter end() const { return value_iter(m_sub, m_sub->size()); }
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

    bool contains(const T& member) const
    {
        return (*this)[m_universe->index(member)];
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

    void add(const T& value)
    {
        super::set(m_universe->index(value));
    }

    const index_iterable indices() const
    {
        return index_iterable(this);
    }

    const value_iterable values() const
    {
        return value_iterable(this);
    }

    friend std::ostream& operator << (std::ostream& o, Subset s)
    {
        o << "{";
        const char *sep = "";
        for (const auto& v: s.values()) {
            o << sep << v;
            sep = " ";
        }
        return o << "}";
    }

private:

    const Universe<T, N> *m_universe;

    friend Universe<T, N>;
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
