#ifndef SET_BITS_included
#define SET_BITS_included

#include <iterator>
#include <limits>

template <class Int>
class set_bits {

    static const unsigned max = std::numeric_limits<Int>::digits;

    class iter : public std::iterator_traits<std::input_iterator_tag> {
    public:
        iter(unsigned bit, Int word) : m_word(word), m_bit(advance(bit)) {}
        iter()                                      = default;
        iter(const iter&)                           = default;
        iter& operator = (const iter&)              = default;
        bool operator == (const iter& o) const      { return m_bit == o.m_bit; }
        bool operator != (const iter& o) const      { return m_bit != o.m_bit; }
        iter& operator ++ ()       { m_bit = advance(m_bit + 1); return *this; }
        iter& operator ++ (int)           { iter i = *this; ++*this; return i; }
        Int operator * () const                           { return 1 << m_bit; }

    private:

        unsigned advance(unsigned bit)
        {
            while (bit < max && !(m_word & 1 << bit)) {
                bit++;
            }
            return bit;
        }

        Int m_word;
        unsigned m_bit;

    };

public:

    set_bits(Int n) : m_word(n) {}

    iter begin() { return iter(0, m_word); }
    iter end() { return iter(max, m_word); }

private:

    Int m_word;

};

#endif /* !SET_BITS_included */
