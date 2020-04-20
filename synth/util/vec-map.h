#ifndef VEC_MAP_included
#define VEC_MAP_included

#include <cassert>
#include <initializer_list>
#include <memory>
#include <utility>
#include <vector>

// // -- Debugging - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
// #include <cxxabi.h>
//
// // XXX Move this into the platform directory.
// static inline std::string demangle(const std::string& mangled)
// {
//     int status;
//     char *s = abi::__cxa_demangle(mangled.c_str(), 0, 0, &status);
//     std::string demangled(s);
//     std::free(s);
//     return demangled;
// }
//
// template <typename T>
// static inline std::string type_name(T& obj)
// {
//     const std::string& mangled = typeid(*&obj).name();
//     return demangle(mangled);
// }


template <class K, class V, class Comp = std::less<K>>
class vec_map {

    typedef std::pair<K, V> mut_value_type;

    template<class Map, class Value>
    class iter_tmpl
    : public std::iterator<std::bidirectional_iterator_tag, Value>
    {

    public:

        // default constructor
        iter_tmpl()
        : m_map(nullptr),
          m_pos(0)
        {}

        // copy constructor
        iter_tmpl(const iter_tmpl& that) = default;

        // constructor
        iter_tmpl(Map *m, typename Map::size_type pos)
        : m_map(m),
          m_pos(pos)
        {}

        // assignment
        iter_tmpl& operator = (const iter_tmpl& that) = default;

        // comparison
        bool operator == (const iter_tmpl& that) const
        {
            assert(m_map == that.m_map);
            return m_pos == that.m_pos;
        }
        bool operator != (const iter_tmpl& that) const
        {
            return !(*this == that);
        }

        // element access
        Value& operator *  () const
        {
            assert(m_map);
            // sigh. m_v has mutable keys, but we need to return
            // a value_type here where the key is const but the
            // value might not be.
            return *reinterpret_cast<Value *>(&m_map->m_v[m_pos]);
        }
        Value *operator -> () const
        {
            assert(m_map);
            return reinterpret_cast<Value *>(&m_map->m_v[m_pos]);
        }

        // increment and decrement
        iter_tmpl& operator ++ ()
        {
            assert(m_map);
            assert(m_pos < m_map->size());
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
            assert(m_map);
            assert(m_pos > 0);
            --m_pos;
            return *this;
        }
        iter_tmpl operator -- (int)
        {
            iter_tmpl prev = *this;
            (void)--*this;
            return prev;
        }

    private:

        Map *m_map;
        size_t m_pos;

    };

public:

    // member types
    typedef K                                           key_type;
    typedef V                                           mapped_type;
    typedef std::pair<const key_type, mapped_type>      value_type;
    typedef Comp                                        key_compare;
    typedef value_type&                                 reference;
    typedef const value_type&                           const_reference;
    typedef value_type *                                pointer;
    typedef const value_type *                          const_pointer;
    typedef iter_tmpl<vec_map, value_type>              iterator;
    typedef iter_tmpl<const vec_map, const value_type>  const_iterator;
    typedef std::reverse_iterator<iterator>             reverse_iterator;
    typedef std::reverse_iterator<const_iterator>       const_reverse_iterator;
    typedef ptrdiff_t                                   difference_type;
    typedef size_t                                      size_type;

    class value_compare
    : public std::binary_function<value_type, value_type, bool>
    {
        friend class vec_map;
    protected:
        key_compare comp;
        value_compare(key_compare c)
        : comp(c)
        {}
    public:
        bool operator () (const value_type& a, const value_type& b) const
        {
            return comp(a.first, b.first);
        }
    };

    // default constructor, no copy, no assign, implicit destructor
    vec_map()
    : m_finalized(false),
      m_v()
    {}
    vec_map(const vec_map&) = delete;
    vec_map& operator = (const vec_map&) = delete;

    // iterators
    iterator               begin()
    {
        return iterator(this, 0);
    }
    iterator               end()
    {
        return iterator(this, m_v.size());
    }
    const_iterator         begin()   const
    {
        return const_iterator(this, 0);
    }
    const_iterator         end()     const
    {
        return const_iterator(this, m_v.size());
    }
    reverse_iterator       rbegin()
    {
        return reverse_iterator(end());
    }
    reverse_iterator       rend()
    {
        return reverse_iterator(begin());
    }
    const_reverse_iterator rbegin()  const
    {
        return const_reverse_iterator(end());
    }
    const_reverse_iterator rend()    const
    {
        return const_reverse_iterator(begin());
    }
    const_iterator         cbegin()  const
    {
        return begin();
    }
    const_iterator         cend()    const
    {
        return end();
    }
    const_reverse_iterator crbegin() const
    {
        return rbegin();
    }
    const_reverse_iterator crend()   const
    {
        return rend();
    }

    // capacity
    bool empty() const
    {
        return m_v.empty();
    }
    size_type size() const
    {
        return m_v.size();
    }
    size_type max_size() const
    {
        return m_v.max_size();
    }
    size_t capacity() const
    {
        return m_v.capacity();
    }
    void reserve(size_type n)
    {
        m_v.reserve(n);
    }
    void shrink_to_fit()
    {
        m_v.shrink_to_fit();
    }

    // element access
    mapped_type& operator [] (const key_type& key)
    {
        auto match = [&](const value_type& v) {
            return v.first == key;
        };

        assert(!m_finalized);
        auto it = std::find_if(m_v.begin(), m_v.end(), match);
        if (it == m_v.end()) {
            m_v.push_back(value_type(key, mapped_type()));
            it = m_v.end() - 1;
        }
        return it->second;
    }
    mapped_type& at(const key_type& key)
    {
        auto pos = find(key);
        assert(pos != end());
        return pos->second;
    }
    const mapped_type& at(const key_type& key) const
    {
        auto pos = find(key);
        assert(pos != end());
        return pos->second;
    }

    // observers
    key_compare key_comp() const
    {
        assert(m_finalized);
        return key_compare();
    }
    value_compare value_comp() const
    {
        return value_compare(key_comp());
    }

    // operations
    iterator find(const key_type& key)
    {
        auto comp = [] (const mut_value_type& a, const mut_value_type& b) {
            return a.first < b.first;
        };
        auto k = mut_value_type(key, mapped_type());
        auto it = std::lower_bound(m_v.begin(), m_v.end(), k, comp);
        return iterator(this, it - m_v.begin());
    }
    const_iterator find(const key_type& key) const
    {
        auto comp = [] (const mut_value_type& a, const mut_value_type& b) {
            return a.first < b.first;
        };
        auto k = mut_value_type(key, mapped_type());
        auto it = std::lower_bound(m_v.begin(), m_v.end(), k, comp);
        return const_iterator(this, it - m_v.cbegin());
    }
    size_type count(const key_type& key) const
    {
        auto comp = [] (const mut_value_type& a, const mut_value_type& b) {
            return a.first < b.first;
        };
        assert(m_finalized);
        auto k = mut_value_type(key, mapped_type());
        auto low = std::lower_bound(m_v.begin(), m_v.end(), k, comp);
        auto high = std::upper_bound(m_v.begin(), m_v.end(), k, comp);
        return high - low;
    }

    // finalization
    bool finalized() const
    {
        return m_finalized;
    }
    void finalize()
    {
        m_finalized = true;
        std::sort(m_v.begin(), m_v.end(), value_comp());
    }

private:

    bool m_finalized;
    std::vector<mut_value_type> m_v;

};

#endif /* !VEC_MAP_included */
