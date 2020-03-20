#include <array>
#include <exception>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <vector>

class stop_iteration : public std::exception {};

class range {

    class iter : public std::iterator_traits<std::input_iterator_tag> {
    public:
        iter(int i): m_i(i) {}
        iter() = default;
        iter(const iter&) = default;
        iter& operator = (const iter&) = default;
        // bool operator == (const iter& that) const { return m_i == that.m_i; }
        bool operator != (const iter& that) const { return m_i != that.m_i; }
        iter& operator ++ () { ++m_i; return *this; }
        int  operator *  () const {return m_i; }
    private:
        int m_i;
    };

public:

    range(int n) : m_stop(n) {}

    iter begin() { return iter(0); }
    iter end()   { return iter(m_stop); }

private:
    int m_stop;
};

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

class generator {

public:

    typedef std::function<int()> gf;

private:

    class iter : public std::iterator_traits<std::input_iterator_tag> {

    public:

        iter() : m_fun(nullptr), m_is_end(true), m_pos(0) {}
        iter(gf f, int count) : m_fun(f), m_is_end(false), m_pos(advance(count)) {}
        iter(const iter&) = default;

        bool operator == (const iter& that) {
            if (m_is_end or that.m_is_end)
                return m_is_end == that.m_is_end;
            return m_pos == that.m_pos;
        }
        bool operator != (const iter& that) { return !(*this == that); }
        int operator * () { return m_value; }
        const int& operator * () const { return m_value; }
        iter& operator ++ () { m_pos = advance(m_pos); return *this; }

        // ++ calls f and stores its value
        // * returns the value
        // iter has a counter for how many times it's been called.
        // iter has a boolean "is_env"
    private:
        int advance(int pos)
        {
            if (m_is_end)
                return pos;
            try {
                m_value = (m_fun)();
                return pos + 1;
            }
            catch (stop_iteration) {
                m_is_end = true;
                return pos;
            }
            return pos;
        }

        gf   m_fun;
        bool m_is_end;
        int  m_pos;
        int  m_value;
    };

public:

    generator(gf f) : m_fun(f) {}

    iter begin() { return iter(m_fun, 0); }
    iter end() { return iter(); }

public:

    gf m_fun;

};

int main()
{
    generator::gf asdf = [] () {
        int i = 0;
        return [=] () mutable {
            if (i >= 6)
                throw stop_iteration();
            return i++;
        };
    }();

    for (auto x : generator(asdf)) {
        std::cout << "x = " << x << std::endl;
    }

    std::array<uint16_t, 5> a = { 0, 1, 0x12F8, 0x8000, 0xFFFF };
    for (auto& word : a) {
        std::cout << std::hex << word << ": ";
        for (auto bit : set_bits<uint16_t>(word)) {
            std::cout << bit << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::dec << std::endl;

    auto rg5 = range(5);
    for (auto i : rg5) {
        std::cout << i << std::endl;
    }
    std::cout << "\nYo." << std::endl;
    return 0;
}
