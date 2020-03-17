#ifndef SPAN_MAP_included
#define SPAN_MAP_included

#include <vector>

#include "synth/util/vec-map.h"

// span_map<K, V> -- map from K to spans of V.

template <class K, class V, class Comp = std::less<K>>
class span_map;

template <class K, class V, class Comp = std::less<K>>
class mapped_span {

    friend class span_map<K, V, Comp>;

    template <class Span, class Value>
    class iter_tmpl
    : public std::iterator<std::random_access_iterator_tag, Value> {

    public:

        // constructors, assignment, implicit destructor
        iter_tmpl() : m_span(nullptr), m_pos(0) {}
        iter_tmpl(Span *span, size_t pos) : m_span(span), m_pos(pos) {}

        // comparison
        bool operator == (const iter_tmpl& that) const
        {
            assert(m_span);
            assert(m_span == that.m_span);
            return m_pos == that.m_pos;
        }
        bool operator != (const iter_tmpl& that) const
        {
            return !(*this == that);
        }

        // element access
        Value& operator * () const
        {
            assert(m_span);
            assert(m_pos < m_span->size());
            return (*m_span)[m_pos];
        }
        Value *operator -> () const
        {
            return &**this;
        }

        // increment and decrement
        iter_tmpl& operator ++ ()
        {
            assert(m_span);
            assert(m_pos < m_span->size());
            m_pos++;
            return *this;
        }
        iter_tmpl operator ++ (int)
        {
            iter_tmpl prev = *this;
            (void)++*this;
            return prev;
        }
        iter_tmpl& operator -- ()
        {
            assert(m_span);
            assert(m_pos > 0);
            m_pos--;
            return *this;
        }
        iter_tmpl operator -- (int)
        {
            iter_tmpl prev = *this;
            (void)--*this;
            return prev;
        }

        // arithmetic
        iter_tmpl& operator += (ptrdiff_t n)
        {
            assert(m_span);
            assert(n >= 0 || m_pos >= static_cast<size_t>(-n));
            assert(m_pos + n <= m_span->size());
            m_pos += n;
            return *this;
        }
        iter_tmpl& operator -= (ptrdiff_t n)
        {
            *this += -n;
        }
        iter_tmpl operator + (ptrdiff_t n)
        {
            iter_tmpl it = *this;
            it += n;
            return it;
        }
        iter_tmpl operator - (ptrdiff_t n)
        {
            iter_tmpl it = *this;
            it -= n;
            return it;
        }

    private:

        Span *m_span;
        size_t m_pos;

    };

public:

    // member types
    typedef V value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type *pointer;
    typedef const value_type *const_pointer;
    typedef iter_tmpl<mapped_span, value_type> iterator;
    typedef iter_tmpl<const mapped_span, const value_type> const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;

    // constructors, assignment
    mapped_span() : m_map(nullptr), m_start(0), m_count(0) {}
    mapped_span(const mapped_span&)              = default;
    mapped_span& operator = (const mapped_span&) = default;

    // iterators
    iterator begin() { return iterator(this, 0); }
    iterator end() { return iterator(this, size()); }
    const_iterator begin() const { return const_iterator(this, 0); }
    const_iterator end() const { return const_iterator(this, size()); }
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }
    const_reverse_iterator crbegin() const { return rbegin(); }
    const_reverse_iterator crend() const { return rend(); }

    // capacity
    bool empty() const  { assert(m_map); return m_count == 0; }
    size_t size() const { assert(m_map); return m_count; }

    // element access
    reference operator [] (size_t index)
    {
        assert(m_map);
        assert(index < m_count);
        return m_map->m_values[m_start + index];
    }
    const_reference operator [] (size_t index) const
    {
        assert(m_map);
        assert(index < m_count);
        return m_map->m_values[m_start + index];
    }
    reference front()
    {
        return (*this)[0];
    }
    const_reference front() const
    {
        return (*this)[0];
    }
    reference back()
    {
        assert(m_count > 0);
        return (*this)[m_count - 1];
    }
    const_reference back() const
    {
        assert(m_count > 0);
        return (*this)[m_count - 1];
    }

    // modifiers
    void push_back(const V& v)
    {
        assert(m_map);
        // we only know how to push back onto the last span.
        assert(m_start + m_count == m_map->m_values.size());
        m_map->m_values.push_back(v);
        m_count++;
    }

private:

    span_map<K, V, Comp> *m_map;
    size_t m_start;
    size_t m_count;

};

template <class K, class V, class Comp>
class span_map : public vec_map<K, mapped_span<K, V, Comp>> {

    typedef vec_map<K, mapped_span<K, V, Comp>> super;

    friend class mapped_span<K, V, Comp>;

public:

    // XXX why aren't these typedefs inherited from the superclass?
    typedef typename super::key_type  key_type;
    typedef typename super::mapped_type mapped_type;

    // capacity
    size_t value_size() const     { return m_values.size(); }
    size_t value_capacity() const { return m_values.capacity(); }
    void values_reserve(size_t n) { m_values.reserve(n); }
    void shrink_to_fit()
    {
        super::shrink_to_fit();
        m_values.shrink_to_fit();
    }

    // element access
    mapped_type& operator [] (const key_type& key)
    {
        auto& it = super::operator [] (key);
        if (!it.m_map) {
            it.m_map = this;
            it.m_start = m_values.size();
        }
        return it;
    }

private:

    std::vector<V> m_values;

};

#endif /* !SPAN_MAP_included */
