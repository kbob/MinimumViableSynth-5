#ifndef BYTE_SPAN_included
#define BYTE_SPAN_included

#ifdef __cplusplus

#include <cassert>
#include <cstddef>
#include <cstdint>

#include <iterator>

#define ALWAYS_CONST

class byte_span {

    template <class T> class iter_tmpl
    : public std::iterator<std::random_access_iterator_tag, T> {

    public:

        constexpr iter_tmpl() : m_span(0), m_offset(0) {}

#ifdef ALWAYS_CONST
        constexpr iter_tmpl(const byte_span& s, size_t o)
#else
        constexpr iter_tmpl(byte_span& s, size_t o)
#endif
        : m_span(&s),
          m_offset(o)
        {}

        constexpr bool operator == (const iter_tmpl<T>& that) const
        {
            return m_span == that.m_span && m_offset == that.m_offset;
        }

        constexpr bool operator != (const iter_tmpl<T>& that) const
        {
            return !(*this == that);
        }

        constexpr bool operator < (const iter_tmpl<T>& that) const
        {
            if (m_span != that.m_span) {
                const char *what = "unspecified byte_span iterator comparison";
                throw std::logic_error(what);
            }
            return m_offset < that.m_offset;
        }

        constexpr bool operator <= (const iter_tmpl<T>& that) const
        {
            return !(that < *this);
        }

        constexpr bool operator >= (const iter_tmpl<T>& that) const
        {
            return !(*this < that);
        }

        constexpr bool operator > (const iter_tmpl<T>& that) const
        {
            return that < *this;
        }

        constexpr iter_tmpl<T> operator ++ () { ++m_offset; return *this; }

        constexpr iter_tmpl<T> operator -- () { --m_offset; return *this; }

        constexpr iter_tmpl<T> operator ++ (int)
        {
            auto tmp = *this;
            m_offset++;
            return tmp;
        }

        constexpr iter_tmpl<T> operator -- (int)
        {
            auto tmp = *this;
            m_offset--;
            return tmp;
        }

        constexpr iter_tmpl<T> operator + (ptrdiff_t offset) const
        {
            return iter_tmpl<T>(*m_span, m_offset + offset);
        }

        constexpr iter_tmpl<T> operator - (ptrdiff_t offset) const
        {
            return iter_tmpl<T>(*m_span, m_offset - offset);
        }

        constexpr iter_tmpl<T>& operator += (ptrdiff_t offset)
        {
            m_offset += offset;
            return *this;
        }

        constexpr iter_tmpl<T>& operator -= (ptrdiff_t offset)
        {
            m_offset -= offset;
            return *this;
        }

        constexpr T& operator * () const { return (*m_span)[m_offset]; }

        constexpr T *operator -> () const { return &(*m_span)[m_offset]; }

        constexpr T& operator [] (ptrdiff_t n) const
        {
            return (*m_span)[m_offset + n];
        }

    private:

        const byte_span *m_span;
        std::size_t m_offset;

    };

public:

    typedef const uint8_t element_type;
    typedef const uint8_t value_type;
    typedef size_t index_type;
    typedef ptrdiff_t difference_type;
    typedef element_type *pointer;
    typedef const element_type *const_pointer;
    typedef const uint8_t& reference;
    typedef iter_tmpl<const uint8_t> iterator;
    typedef iter_tmpl<const uint8_t> const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    // more constructors
    constexpr byte_span(const std::uint8_t *p, std::size_t s)
    : m_ptr(p),
      m_size(s)
    {}

    constexpr size_t size() const { return m_size; }

    constexpr size_t size_bytes() const { return m_size * sizeof *m_ptr; }

    constexpr bool empty() const { return m_size == 0; }

#ifndef ALWAYS_CONST
    constexpr uint8_t& operator [] (size_t index)
    {
        if (index >= m_size)
            throw std::runtime_error("index out of range");
        return m_ptr[index];
    }
#endif

    constexpr const uint8_t& operator [] (size_t index) const
    {
        if (index >= m_size)
            throw std::runtime_error("index out of range");
        return m_ptr[index];
    }

    constexpr iterator begin() const
    {
        return iterator(*this, 0);
    }

    constexpr iterator end() const
    {
        return iterator(*this, size());
    }

    constexpr const_iterator cbegin() const
    {
        return const_iterator(*this, 0);
    }

    constexpr const_iterator cend() const
    {
        return const_iterator(*this, size());
    }

    constexpr reverse_iterator rbegin() const
    {
        return reverse_iterator(end());
    }

    constexpr reverse_iterator rend() const
    {
        return reverse_iterator(begin());
    }

    constexpr const_reverse_iterator crbegin() const
    {
        return const_reverse_iterator(cend());
    }

    constexpr const_reverse_iterator crend() const
    {
        return const_reverse_iterator(cbegin());
    }

    constexpr pointer data() const { return m_ptr; }

    constexpr void swap(byte_span& that)
    {
#ifdef ALWAYS_CONST
        const std::uint8_t *ptr = m_ptr;
#else
        std::uint8_t *ptr = m_ptr;
#endif
        std::size_t size = m_size;
        m_ptr = that.m_ptr;
        m_size = that.m_size;
        that.m_ptr = ptr;
        that.m_size = size;
    }

    constexpr byte_span first(size_t count) const
    {
        if (count > size())
            throw std::length_error("first exceeds byte_span");
        return byte_span(m_ptr, count);
    }

    constexpr byte_span last(size_t count) const
    {
        if (count > size())
            throw std::length_error("last exceeds byte_span");
        return byte_span(m_ptr + size() - count, count);
    }

    constexpr byte_span subspan(size_t offset, size_t count) const
    {
        if (offset + count > size())
            throw std::length_error("subspan exceeds byte_span");
        return byte_span(m_ptr + offset, count);
    }

private:

#ifdef ALWAYS_CONST
    const std::uint8_t *m_ptr;
#else
    std::uint8_t *m_ptr;
#endif
    std::size_t m_size;

    // void logic_error(const char *msg) const { abort(); }
    //
    // void runtime_error(const char *msg) const { abort(); }
};

#endif /* __cplusplus */

#endif /* !BYTE_SPAN_included */
