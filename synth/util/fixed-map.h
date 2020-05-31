#ifndef FIXED_MAP_included
#define FIXED_MAP_included

#include <functional>           // std::less
#include <initializer_list>     // std::initializer_list
#include <iterator>             // std::reverse_iterator
#include <stdexcept>            // std::out_of_range
#include <utility>              // std::move, std::pair

#include "synth/util/fixed-vector.h"


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


// -- fixed_map - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
// fixed_map<K, V, N, Comp> is like std::map<K, T, Compare, Alloc>
// with the following exceptions.
//
//   - does not allocate memory.
//   - insertion is O(n).  (lookup is O(log n) with better locality.)
//   - iterators are random access.
//   - does not use an allocator.
//   - constructors do not take comparator or allocator arguments.
//   - `allocator_type` is not defined.
//   - `swap` is not implemented.

template <class K, class V, size_t N, class Comp = std::less<K>>
class fixed_map {

public:

    // Member Types
    typedef K                                     key_type;
    typedef V                                     mapped_type;
    typedef std::pair<const K, V>                 value_type;
    typedef Comp                                  key_compare;
    class                                         value_compare;
    typedef value_type&                           reference;
    typedef const value_type&                     const_reference;
    typedef value_type                           *pointer;
    typedef const value_type                     *const_pointer;
    typedef value_type                           *iterator;
    typedef const value_type                     *const_iterator;
    typedef std::reverse_iterator<iterator>       reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef ptrdiff_t                             difference_type;
    typedef size_t                                size_type;

private:

    template <class P>
        using convertible =
            typename std::enable_if<
                std::is_constructible<
                    value_type,
                    P
                >::value,
                P
            >::type;

public:

    // Constructors
    explicit fixed_map();
    template <class InputIterator>
        fixed_map(InputIterator first, InputIterator last);
    fixed_map(const fixed_map&);
    fixed_map(fixed_map&&);
    fixed_map(std::initializer_list<value_type>);

    // Destructor
    ~fixed_map();

    // Assignment
    fixed_map& operator = (const fixed_map&);
    fixed_map& operator = (fixed_map&&);
    fixed_map& operator = (std::initializer_list<value_type>);

    // Iterators
    iterator               begin();
    const_iterator         begin()   const;
    iterator               end();
    const_iterator         end()     const;
    reverse_iterator       rbegin();
    const_reverse_iterator rbegin()  const;
    reverse_iterator       rend();
    const_reverse_iterator rend()    const;
    const_iterator         cbegin()  const;
    const_iterator         cend()    const;
    const_reverse_iterator crbegin() const;
    const_reverse_iterator crend()   const;

    // Capacity
    bool empty() const;
    size_type size() const;
    size_type max_size() const;

    // Element Access
    mapped_type& operator [] (const key_type&);
    mapped_type& operator [] (key_type&&);
    mapped_type& at(const key_type&);
    const mapped_type& at(const key_type&) const;

    // Modifiers
    std::pair<iterator, bool> insert(const value_type&);
    template <class P, class = convertible<P>>
        std::pair<iterator, bool> insert(P&&);
    iterator insert(const_iterator position, const value_type&);
    template <class P, class = convertible<P>>
        iterator insert(const_iterator position, P&&);
    template <class InputIterator>
        void insert(InputIterator first, InputIterator last);
    void insert(std::initializer_list<value_type>);

    iterator erase(const iterator position);
    size_type erase(const key_type&);
    iterator erase(const_iterator first, const_iterator last);

    // void swap(fixed_map&);   // Not implemented

    void clear();

    template <class... Args>
        std::pair<iterator, bool> emplace(Args&&...);
    template <class... Args>
        iterator emplace_hint(const_iterator position, Args&&...);

    // Observers
    key_compare key_comp() const;
    value_compare value_comp() const;

    // Operations
    iterator find(const key_type&);
    const_iterator find(const key_type&) const;

    size_type count(const key_type&) const;

    iterator lower_bound(const key_type&);
    const_iterator lower_bound(const key_type&) const;

    iterator upper_bound(const key_type&);
    const_iterator upper_bound(const key_type&) const;

    std::pair<const_iterator, const_iterator>
        equal_range(const key_type&) const;
    std::pair<iterator, iterator> equal_range(const key_type&);

    // Relational Operators
    template <class K1, class V1, size_t N1, class C1>
        friend
        bool
        operator == (const fixed_map<K1, V1, N1, C1>&,
                     const fixed_map<K1, V1, N1, C1>&);
    template <class K1, class V1, size_t N1, class C1>
        friend
        bool
        operator <  (const fixed_map<K1, V1, N1, C1>&,
                     const fixed_map<K1, V1, N1, C1>&);

    // Value Comparison Object
    class value_compare {
    protected:
        Comp m_comp;
        value_compare(Comp c) : m_comp(c) {}
    public:
        typedef bool result_type;
        typedef value_type first_argument_type;
        typedef value_type second_argument_type;
        bool operator () (const value_type& a, const value_type& b) const
        {
            return m_comp(a.first, b.first);
        }
        friend class fixed_map;
    };

private:

    fixed_vector<value_type, N> m_data;
    Comp m_comp;

};


// Relational Operators.

template <class K, class V, size_t N, class C>
bool
operator == (const fixed_map<K, V, N, C>&, const fixed_map<K, V, N, C>&);

template <class K, class V, size_t N, class C>
bool
operator != (const fixed_map<K, V, N, C>&, const fixed_map<K, V, N, C>&);

template <class K, class V, size_t N, class C>
bool
operator < (const fixed_map<K, V, N, C>&, const fixed_map<K, V, N, C>&);

template <class K, class V, size_t N, class C>
bool
operator <= (const fixed_map<K, V, N, C>&, const fixed_map<K, V, N, C>&);

template <class K, class V, size_t N, class C>
bool
operator > (const fixed_map<K, V, N, C>&, const fixed_map<K, V, N, C>&);

template <class K, class V, size_t N, class C>
bool
operator >= (const fixed_map<K, V, N, C>&, const fixed_map<K, V, N, C>&);


// -- Constructors - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class K, class V, size_t N, class C>
inline
fixed_map<K, V, N, C>::fixed_map()
{}

template <class K, class V, size_t N, class C>
template <class InputIterator>
inline
fixed_map<K, V, N, C>::fixed_map(InputIterator first, InputIterator last)
{
    insert(first, last);
}

template <class K, class V, size_t N, class C>
inline
fixed_map<K, V, N, C>::fixed_map(const fixed_map& that)
: m_data{that.m_data},
  m_comp{that.m_comp}
{}

template <class K, class V, size_t N, class C>
inline
fixed_map<K, V, N, C>::fixed_map(fixed_map&& that)
: m_data{std::move(that.m_data)},
  m_comp{std::move(that.m_comp)}
{}

template <class K, class V, size_t N, class C>
inline
fixed_map<K, V, N, C>::fixed_map(std::initializer_list<value_type> il)
{
    for (auto& val: il)
        insert(val);
}


// -- Destructor  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class K, class V, size_t N, class C>
inline
fixed_map<K, V, N, C>::~fixed_map()
{}


// -- Assignment  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class K, class V, size_t N, class C>
inline
fixed_map<K, V, N, C>&
fixed_map<K, V, N, C>::operator = (const fixed_map& that)
{
    if (this != &that) {
        m_data = that.m_data;
        m_comp = that.m_comp;
    }
    return *this;
}

template <class K, class V, size_t N, class C>
inline
fixed_map<K, V, N, C>&
fixed_map<K, V, N, C>::operator = (fixed_map&& that)
{
    if (this != &that) {
        m_data = std::move(that.m_data);
        m_comp = std::move(that.m_comp);
    }
    return *this;
}

template <class K, class V, size_t N, class C>
inline
fixed_map<K, V, N, C>&
fixed_map<K, V, N, C>::operator = (std::initializer_list<value_type> il)
{
    clear();
    for (auto& val: il)
        insert(val);
    return *this;
}


// -- Iterators - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::iterator
fixed_map<K, V, N, C>::begin()
{
    return m_data.begin();
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::const_iterator
fixed_map<K, V, N, C>::begin() const
{
    return m_data.begin();
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::iterator
fixed_map<K, V, N, C>::end()
{
    return m_data.end();
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::const_iterator
fixed_map<K, V, N, C>::end() const
{
    return m_data.end();
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::reverse_iterator
fixed_map<K, V, N, C>::rbegin()
{
    return m_data.rbegin();
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::const_reverse_iterator
fixed_map<K, V, N, C>::rbegin() const
{
    return m_data.rbegin();
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::reverse_iterator
fixed_map<K, V, N, C>::rend()
{
    return m_data.rend();
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::const_reverse_iterator
fixed_map<K, V, N, C>::rend() const
{
    return m_data.rend();
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::const_iterator
fixed_map<K, V, N, C>::cbegin() const
{
    return m_data.cbegin();
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::const_iterator
fixed_map<K, V, N, C>::cend() const
{
    return m_data.cend();
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::const_reverse_iterator
fixed_map<K, V, N, C>::crbegin() const
{
    return m_data.crbegin();
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::const_reverse_iterator
fixed_map<K, V, N, C>::crend() const
{
    return m_data.crend();
}


// -- Capacity -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class K, class V, size_t N, class C>
inline
bool
fixed_map<K, V, N, C>::empty() const
{
    return m_data.empty();
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::size_type
fixed_map<K, V, N, C>::size() const
{
    return m_data.size();
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::size_type
fixed_map<K, V, N, C>::max_size() const
{
    return m_data.max_size();
}


// -- Element Access -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::mapped_type&
fixed_map<K, V, N, C>::operator [] (const key_type& key)
{
    auto p = equal_range(key);
    if (p.first == p.second)
        insert(p.first, std::make_pair(key, mapped_type()));
    return p.first->second;
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::mapped_type&
fixed_map<K, V, N, C>::operator [] (key_type&& key)
{
    auto p = equal_range(key);
    if (p.first == p.second)
        insert(p.first,
               std::move(std::make_pair(std::move(key), mapped_type())));
    return p.first->second;
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::mapped_type&
fixed_map<K, V, N, C>::at(const key_type& key)
{
    auto p = equal_range(key);
    if (p.first == p.second)
        throw std::out_of_range("fixed_map");
    return p.first->second;
}

template <class K, class V, size_t N, class C>
inline
const typename fixed_map<K, V, N, C>::mapped_type&
fixed_map<K, V, N, C>::at(const key_type& key) const
{
    return const_cast<fixed_map *>(this)->at(key);
}


// -- Modifiers - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class K, class V, size_t N, class C>
inline
std::pair<typename fixed_map<K, V, N, C>::iterator, bool>
fixed_map<K, V, N, C>::insert(const value_type& val)
{
    auto p = equal_range(val.first);
    if (p.first == p.second) {
        m_data.insert(p.first, val);
        return std::make_pair(p.first, true);
    }
    return std::make_pair(p.first, false);
}

template <class K, class V, size_t N, class C>
template <class P, class>
inline
std::pair<typename fixed_map<K, V, N, C>::iterator, bool>
fixed_map<K, V, N, C>::insert(P&& val)
{
    auto p = equal_range(val.first);
    if (p.first == p.second) {
        m_data.insert(p.first, std::move(val));
        return std::make_pair(p.first, true);
    }
    return std::make_pair(p.first, false);
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::iterator
fixed_map<K, V, N, C>::insert(const_iterator pos, const value_type& val)
{
    if ((pos != end() && m_comp(pos->first, val.first)) ||
        (pos != begin() && m_comp(val.first, (pos - 1)->first)))
    {
        pos = equal_range(val.first).first;
    }
    if (pos->first != val.first)
        m_data.insert(pos, val);
    return const_cast<iterator>(pos);
}

template <class K, class V, size_t N, class C>
template <class P, class>
inline
typename fixed_map<K, V, N, C>::iterator
fixed_map<K, V, N, C>::insert(const_iterator pos, P&& val)
{
    value_type value(std::move(val));
    if ((pos != end() && m_comp(pos->first, value.first)) ||
        (pos != begin() && m_comp(value.first, (pos - 1)->first)))
    {
        pos = equal_range(value.first).first;
    }
    if (pos->first != value.first)
        m_data.emplace(pos, std::move(value));
    return const_cast<iterator>(pos);
}

template <class K, class V, size_t N, class C>
template <class InputIterator>
inline
void
fixed_map<K, V, N, C>::insert(InputIterator first, InputIterator last)
{
    for (InputIterator it = first; it != last; ++it)
        insert(*it);
}

template <class K, class V, size_t N, class C>
inline
void
fixed_map<K, V, N, C>::insert(std::initializer_list<value_type> il)
{
    for (auto& val: il)
        insert(val);
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::iterator
fixed_map<K, V, N, C>::erase(const iterator pos)
{
    assert(pos >= begin() && pos < end());
    m_data.erase(pos);
    return pos;
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::size_type
fixed_map<K, V, N, C>::erase(const key_type& key)
{
    auto p = equal_range(key);
    if (p.first == p.second)
        return 0;
    m_data.erase(p.first);
    return 1;
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::iterator
fixed_map<K, V, N, C>::erase(const_iterator first, const_iterator last)
{
    assert(begin() <= first && first <= last && last <= end());
    m_data.erase(first, last);
    return const_cast<iterator>(first);
}

// Not implemented.
// template <class K, class V, size_t N, class C>
// inline
// void
// fixed_map<K, V, N, C>::swap(fixed_map&)
// {}

template <class K, class V, size_t N, class C>
inline
void
fixed_map<K, V, N, C>::clear()
{
    m_data.clear();
}

template <class K, class V, size_t N, class C>
template <class... Args>
inline
std::pair<typename fixed_map<K, V, N, C>::iterator, bool>
fixed_map<K, V, N, C>::emplace(Args&&... args)
{
    value_type val(args...);
    auto p = equal_range(val.first);
    if (p.first == p.second) {
        m_data.emplace(p.first, std::move(val));
        return make_pair(p.first, true);
    }
    return make_pair(p.first, false);
}

template <class K, class V, size_t N, class C>
template <class... Args>
inline
typename fixed_map<K, V, N, C>::iterator
fixed_map<K, V, N, C>::emplace_hint(const_iterator pos, Args&&... args)
{
    value_type value(args...);
    if ((pos != end() && m_comp(pos->first, value.first)) ||
        (pos != begin() && m_comp(value.first, (pos - 1)->first)))
    {
        pos = equal_range(value.first).first;
    }
    if (pos->first != value.first)
        m_data.insert(pos, std::move(value));
    return const_cast<iterator>(pos);
}


// -- Observers - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::key_compare
fixed_map<K, V, N, C>::key_comp() const
{
    return m_comp;
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::value_compare
fixed_map<K, V, N, C>::value_comp() const
{
    return value_compare(m_comp);
}


// -- Operations  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::iterator
fixed_map<K, V, N, C>::find(const key_type& key)
{
    auto p = equal_range(key);
    if (p.first != p.second)
        return p.first;
    return end();
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::const_iterator
fixed_map<K, V, N, C>::find(const key_type& key) const
{
    return const_cast<fixed_map *>(this)->find(key);
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::size_type
fixed_map<K, V, N, C>::count(const key_type& key) const
{
    auto p = equal_range(key);
    return p.second - p.first;
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::iterator
fixed_map<K, V, N, C>::lower_bound(const key_type&  key)
{
    return equal_range(key).first;
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::const_iterator
fixed_map<K, V, N, C>::lower_bound(const key_type& key) const
{
    return const_cast<fixed_map *>(this)->lower_bound(key);
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::iterator
fixed_map<K, V, N, C>::upper_bound(const key_type& key)
{
    return equal_range(key).second;
}

template <class K, class V, size_t N, class C>
inline
typename fixed_map<K, V, N, C>::const_iterator
fixed_map<K, V, N, C>::upper_bound(const key_type& key) const
{
    return const_cast<fixed_map *>(this)->upper_bound(key);
}

template <class K, class V, size_t N, class C>
inline
std::pair<typename fixed_map<K, V, N, C>::const_iterator,
          typename fixed_map<K, V, N, C>::const_iterator>
fixed_map<K, V, N, C>::equal_range(const key_type& key) const
{
    return const_cast<fixed_map *>(this)->equal_range(key);
}

template <class K, class V, size_t N, class C>
inline
std::pair<typename fixed_map<K, V, N, C>::iterator,
          typename fixed_map<K, V, N, C>::iterator>
fixed_map<K, V, N, C>::equal_range(const key_type& key)
{
    auto low = m_data.begin(), high = m_data.end();
    while (low != high) {
        iterator mid = low + (high - low) / 2;
        if (m_comp(mid->first, key))
            low = mid + 1;
        else if (m_comp(key, mid->first))
            high = mid;
        else
            return make_pair(mid, mid + 1);
    }
    return make_pair(low, low);
}


// -- Relational Operators -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class K, class V, size_t N, class C>
inline
bool
operator == (const fixed_map<K, V, N, C>& a, const fixed_map<K, V, N, C>& b)
{
    return a.m_data == b.m_data;
}

template <class K, class V, size_t N, class C>
inline
bool
operator != (const fixed_map<K, V, N, C>& a, const fixed_map<K, V, N, C>& b)
{
    return !(a == b);
}

template <class K, class V, size_t N, class C>
inline
bool
operator < (const fixed_map<K, V, N, C>& a, const fixed_map<K, V, N, C>& b)
{
    return a.m_data < b.m_data;
}

template <class K, class V, size_t N, class C>
inline
bool
operator <= (const fixed_map<K, V, N, C>& a, const fixed_map<K, V, N, C>& b)
{
    return !(b < a);
}

template <class K, class V, size_t N, class C>
inline
bool
operator > (const fixed_map<K, V, N, C>& a, const fixed_map<K, V, N, C>& b)
{
    return b < a;
}

template <class K, class V, size_t N, class C>
inline
bool
operator >= (const fixed_map<K, V, N, C>& a, const fixed_map<K, V, N, C>& b)
{
    return !(a < b);
}


#endif /* !FIXED_MAP_included */
