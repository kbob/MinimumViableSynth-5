#ifndef FIXED_VECTOR_included
#define FIXED_VECTOR_included

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>


// A vector with fixed memory allocation.
//
// Differences from std::vector:
//
//    - template has a size parameter.
//    - template has no allocator parameter.
//    - constructores do not take an allocator arg.
//    - `allocator_type` is undefiend.
//    - `get_allocator` is undefined.
//    - move constructor moves elements instead of moving vector.
//    - `swap` and `std::swap` swap elements instead of whole vector.

template <class T, size_t N>
class fixed_vector {

    // Horrible template filter verifies I is a compatible input
    // iterator type.
    template <class I>
        using compat_input_iter =
            typename std::enable_if<
                std::is_base_of<
                    std::input_iterator_tag,
                    typename std::iterator_traits<I>::iterator_category
                >::value &&
                std::is_constructible<
                    T,
                    typename std::iterator_traits<I>::value_type
                >::value,
                I
            >::type;

public:

    // Types
    typedef T                                     value_type;
    typedef T&                                    reference;
    typedef const T&                              const_reference;
    typedef T                                    *pointer;
    typedef const T                              *const_pointer;
    typedef pointer                               iterator;
    typedef const_pointer                         const_iterator;
    typedef std::reverse_iterator<iterator>       reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef ptrdiff_t                             difference_type;
    typedef size_t                                size_type;

    // Constructors
    explicit fixed_vector();
    explicit fixed_vector(size_type);
    explicit fixed_vector(size_type, const value_type&);
    template <class InputIterator>
        explicit fixed_vector(InputIterator, compat_input_iter<InputIterator>);
    fixed_vector(const fixed_vector&);
    fixed_vector(fixed_vector&&);
    fixed_vector(std::initializer_list<value_type>);

    // Destructor
    ~fixed_vector();

    // Assignment
    fixed_vector& operator = (const fixed_vector&);
    fixed_vector& operator = (fixed_vector&&);
    fixed_vector& operator = (std::initializer_list<value_type>);

    // Iterators
    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    iterator end() noexcept;
    const_iterator end() const noexcept;
    reverse_iterator rbegin() noexcept;
    const_reverse_iterator rbegin() const noexcept;
    reverse_iterator rend() noexcept;
    const_reverse_iterator rend() const noexcept;
    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;
    const_reverse_iterator crbegin() const noexcept;
    const_reverse_iterator crend() const noexcept;

    // Capacity
    size_type size() const noexcept;
    size_type max_size() const noexcept;
    void resize(size_type);
    void resize(size_type, const value_type&);
    size_type capacity() const noexcept;
    bool empty() const noexcept;
    void reserve(size_type);
    void shrink_to_fit();

    // Element Access
    reference operator [] (size_type);
    const_reference operator [] (size_type) const;
    reference at(size_type);
    const_reference at(size_type) const;
    reference front();
    const_reference front() const;
    reference back();
    const_reference back() const;
    value_type *data() noexcept;
    const value_type *data() const noexcept;

    // Modifiers
    template <class InputIterator>
        void assign(InputIterator, compat_input_iter<InputIterator>);
    void assign(size_type, const value_type&);
    void assign(std::initializer_list<value_type>);
    void push_back(const value_type&);
    void push_back(value_type&&);
    void pop_back();
    iterator insert(const_iterator, const value_type&);
    iterator insert(const_iterator, size_type, const value_type&);
    template <class InputIterator>
        iterator insert(const_iterator,
                        InputIterator,
                        compat_input_iter<InputIterator>);
    iterator insert(const_iterator, value_type&&);
    iterator insert(const_iterator, std::initializer_list<value_type>);
    iterator erase(const_iterator);
    iterator erase(const_iterator, const_iterator);
    void swap(fixed_vector&);
    void clear() noexcept;
    template <class... Args>
        iterator emplace(const_iterator, Args&&...);
    template <class... Args>
        void emplace_back(Args&&...);

private:

    typedef typename std::aligned_storage<sizeof(T), alignof(T)>::type pod_type;

    pointer m_end;
    pod_type m_store[N];

    friend class fixed_vector_unit_test;

};

// Relational Operators
template <class T, size_t N>
bool operator == (const fixed_vector<T, N>&, const fixed_vector<T, N>&);

template <class T, size_t N>
bool operator != (const fixed_vector<T, N>&, const fixed_vector<T, N>&);

template <class T, size_t N>
bool operator < (const fixed_vector<T, N>&, const fixed_vector<T, N>&);

template <class T, size_t N>
bool operator <= (const fixed_vector<T, N>&, const fixed_vector<T, N>&);

template <class T, size_t N>
bool operator > (const fixed_vector<T, N>&, const fixed_vector<T, N>&);

template <class T, size_t N>
bool operator >= (const fixed_vector<T, N>&, const fixed_vector<T, N>&);


// -- Constructors - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// Constructors
template <class T, size_t N>
inline
fixed_vector<T, N>::fixed_vector()
: m_end{data()}
{}

template<class T, size_t N>
inline
fixed_vector<T, N>::fixed_vector(size_type n)
: m_end{data()}
{
    assert(n <= N);
    for (size_t i = 0; i < n; i++)
        emplace_back();
}

template<class T, size_t N>
inline
fixed_vector<T, N>::fixed_vector(size_type n, const value_type& val)
: m_end{data()}
{
    assert(n <= N);
    for (size_t i = 0; i < n; i++)
        push_back(val);
}

template <class T, size_t N>
template <class InputIterator>
inline
fixed_vector<T, N>::fixed_vector(
        InputIterator first,
        fixed_vector<T, N>::compat_input_iter<InputIterator> last
    )
: m_end{data()}
{
    for ( ; first != last; ++first)
         push_back(*first);
}

template <class T, size_t N>
inline
fixed_vector<T, N>::fixed_vector(const fixed_vector& that)
: m_end{data()}
{
    for (auto& item: that)
        push_back(item);
}

template <class T, size_t N>
inline
fixed_vector<T, N>::fixed_vector(fixed_vector&& that)
: m_end{data()}
{
    for (auto& item: that)
        (void) new (m_end++) T(std::move(item));
    that.m_end = that.data();
}

template <class T, size_t N>
inline
fixed_vector<T, N>::fixed_vector(std::initializer_list<value_type> il)
: m_end{data()}
{
    for (auto& item: il)
        emplace_back(item);
}


// -- Destructor  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class T, size_t N>
inline
fixed_vector<T, N>::~fixed_vector()
{
    clear();
}


// -- Assignment  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class T, size_t N>
inline
fixed_vector<T, N>&
fixed_vector<T, N>::operator = (const fixed_vector& that)
{
    if (this != &that) {
        clear();
        for (auto& item: that)
            push_back(item);
    }
    return *this;
}

template <class T, size_t N>
inline
fixed_vector<T, N>&
fixed_vector<T, N>::operator = (fixed_vector&& that)
{
    if (this != &that) {
        clear();
        for (auto& item: that)
            (void)new (m_end++) T(std::move(item));
    }
    return *this;
}

template <class T, size_t N>
inline
fixed_vector<T, N>&
fixed_vector<T, N>::operator = (std::initializer_list<value_type> il)
{
    assert(il.size() <= N);
    clear();
    for (auto& item: il)
        emplace_back(item);
    return *this;
}


// -- Iterators - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class T, size_t N>
inline
typename fixed_vector<T, N>::iterator
fixed_vector<T, N>::begin() noexcept
{
    return data();
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>:: const_iterator
fixed_vector<T, N>::begin() const noexcept
{
    return data();
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>:: iterator
fixed_vector<T, N>::end() noexcept
{
    return m_end;
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>:: const_iterator
fixed_vector<T, N>::end() const noexcept
{
    return m_end;
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>:: reverse_iterator
fixed_vector<T, N>::rbegin() noexcept
{
    return reverse_iterator(end());
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>:: const_reverse_iterator
fixed_vector<T, N>::rbegin() const noexcept
{
    return const_reverse_iterator(end());
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>:: reverse_iterator
fixed_vector<T, N>::rend() noexcept
{
    return reverse_iterator(begin());
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>:: const_reverse_iterator
fixed_vector<T, N>::rend() const noexcept
{
    return const_reverse_iterator(begin());
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>:: const_iterator
fixed_vector<T, N>::cbegin() const noexcept
{
    return data();
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>:: const_iterator
fixed_vector<T, N>::cend() const noexcept
{
    return m_end;
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>:: const_reverse_iterator
fixed_vector<T, N>::crbegin() const noexcept
{
    return const_reverse_iterator(cend());
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>:: const_reverse_iterator
fixed_vector<T, N>::crend() const noexcept
{
    return const_reverse_iterator(cbegin());
}


// -- Capacity -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class T, size_t N>
typename fixed_vector<T, N>::size_type
fixed_vector<T, N>::size() const noexcept
{
    return m_end - data();
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>::size_type
fixed_vector<T, N>::max_size() const noexcept
{
    return N;
}

template <class T, size_t N>
inline
void
fixed_vector<T, N>::resize(size_type n)
{
    if (n < size()) {
        do {
            pop_back();
        } while (n < size());
    } else {
        assert(n <= N);
        while (n > size())
            emplace_back();
    }
}

template <class T, size_t N>
inline
void
fixed_vector<T, N>::resize(size_type n, const value_type& val)
{
    if (n < size()) {
        do {
            pop_back();
        } while (n < size());
    } else {
        assert(n <= N);
        while (n > size())
            push_back(val);
    }
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>::size_type
fixed_vector<T, N>::capacity() const noexcept
{
    return N;
}

template <class T, size_t N>
inline
bool
fixed_vector<T, N>::empty() const noexcept
{
    return m_end == data();
}

template <class T, size_t N>
inline
void
fixed_vector<T, N>::reserve(size_type n)
{
    if (n > N)
        throw std::length_error("fixed_vector");
}

template <class T, size_t N>
inline
void
fixed_vector<T, N>::shrink_to_fit()
{}


// -- Element Access -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class T, size_t N>
inline
typename fixed_vector<T, N>::reference
fixed_vector<T, N>::operator [] (size_type i)
{
    assert(i < size());
    return data()[i];
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>::const_reference
fixed_vector<T, N>::operator [] (size_type i) const
{
    assert(i < size());
    return data()[i];
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>::reference
fixed_vector<T, N>::at(size_type i)
{
    if (i >= size())
        throw std::out_of_range("fixed_vector");
    return data()[i];
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>::const_reference
fixed_vector<T, N>::at(size_type i) const
{
    if (i >= size())
        throw std::out_of_range("fixed_vector");
    return data()[i];
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>::reference
fixed_vector<T, N>::front()
{
    assert(!empty());
    return *data();
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>::const_reference
fixed_vector<T, N>::front() const
{
    assert(!empty());
    return *data();
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>::reference
fixed_vector<T, N>::back()
{
    assert(!empty());
    return m_end[-1];
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>::const_reference
fixed_vector<T, N>::back() const
{
    assert(!empty());
    return m_end[-1];
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>::value_type *
fixed_vector<T, N>::data() noexcept
{
    return reinterpret_cast<value_type *>(m_store);
}

template <class T, size_t N>
inline
const typename fixed_vector<T, N>::value_type *
fixed_vector<T, N>::data() const noexcept
{
    return reinterpret_cast<const value_type *>(m_store);
}


// -- Modifiers - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class T, size_t N>
template <class InputIterator>
inline
void
fixed_vector<T, N>::assign(
        InputIterator first,
        compat_input_iter<InputIterator> last
    )
{
    clear();
    for ( ; first != last; ++first)
         push_back(*first);
}

template <class T, size_t N>
inline
void
fixed_vector<T, N>::assign(size_type n, const value_type& val)
{
    assert(n <= N);
    clear();
    for (size_t i = 0; i < n; i++)
        push_back(val);
}

template <class T, size_t N>
inline
void
fixed_vector<T, N>::assign(std::initializer_list<value_type> il)
{
    assert(il.size() <= N);
    clear();
    for (auto& item: il)
        emplace_back(item);
}

template <class T, size_t N>
inline
void
fixed_vector<T, N>::push_back(const value_type& val)
{
    emplace_back(val);
}

template <class T, size_t N>
inline
void
fixed_vector<T, N>::push_back(value_type&& val)
{
    assert(size() < N);
    (void)new (m_end++) T(std::move(val));
}

template <class T, size_t N>
inline
void
fixed_vector<T, N>::pop_back()
{
    assert(!empty());
    (--m_end)->~T();
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>::iterator
fixed_vector<T, N>::insert(const_iterator pos, const value_type& val)
{
    assert(size() + 1 <= N);
    assert(data() <= pos && pos <= m_end);
    T *to = m_end + 1;
    T *from = m_end++;
    while (from != pos)
        (void)new (--to) T(std::move(*--from));
    (void)new (from) T(val);
    return from;
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>::iterator
fixed_vector<T, N>::insert(const_iterator pos,
                           size_type n,
                           const value_type& val)
{
    assert(size() + n <= N);
    assert(data() <= pos && pos <= m_end);
    T *to = m_end + n;
    T *from = m_end;
    m_end += n;
    while (from != pos)
        (void)new (--to) T(std::move(*--from));
    for (size_t i = 0; i < n; i++)
        (void)new (from + i) T(val);
    return from;
}

template <class T, size_t N>
template <class InputIterator>
inline
typename fixed_vector<T, N>::iterator
fixed_vector<T, N>::insert(const_iterator pos,
                           InputIterator first,
                           compat_input_iter<InputIterator> last)
{
    size_t n = std::distance(first, last);
    assert(size() + n <= N);
    assert(data() <= pos && pos <= m_end);
    T *to = m_end + n;
    T *from = m_end;
    m_end += n;
    while (from != pos)
        (void)new (--to) T(std::move(*--from));
    for (to = from; first != last; first++)
        (void)new (to++) T(*first);
    return from;
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>::iterator
fixed_vector<T, N>::insert(const_iterator pos, value_type&& val)
{
    assert(size() + 1 <= N);
    assert(data() <= pos && pos <= m_end);
    T *to = m_end + 1;
    T *from = m_end++;
    while (from != pos)
        (void)new (--to) T(std::move(*--from));
    (void)new (from) T(std::move(val));
    return from;
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>::iterator
fixed_vector<T, N>::insert(const_iterator pos,
                           std::initializer_list<value_type> il)
{
    size_t n = il.size();
    assert(size() + n <= N);
    assert(data() <= pos && pos <= m_end);
    T *to = m_end + n;
    T *from = m_end;
    m_end += n;
    while (from != pos)
        (void)new (--to) T(std::move(*--from));
    to = from;
    for (auto& item: il)
        (void)new (to++) T(item);
    return from;
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>::iterator
fixed_vector<T, N>::erase(const_iterator pos)
{
    assert(size() >= 1);
    assert(data() <= pos && pos + 1 <= m_end);
    T *to = const_cast<T *>(pos);
    T *from = to + 1;
    to->~T();
    while (from != m_end)
        (void)new (to++) T(std::move(*from++));
    m_end -= 1;
    return const_cast<iterator>(pos);
}

template <class T, size_t N>
inline
typename fixed_vector<T, N>::iterator
fixed_vector<T, N>::erase(const_iterator first, const_iterator last)
{
    assert(data() <= first && first <= last && last <= m_end);
    T *first_mut = const_cast<T *>(first);
    for (T *p = first_mut; p != last; p++)
        p->~T();
    T *from = const_cast<T *>(last);
    T *to = first_mut;
    while (from != m_end)
        (void)new (to++) T(std::move(*from++));
    m_end = to;
    return first_mut;
}

template <class T, size_t N>
inline
void
fixed_vector<T, N>::swap(fixed_vector& that)
{
    std::swap(*this, that);
}

template <class T, size_t N>
inline
void
fixed_vector<T, N>::clear() noexcept
{
    while (!empty())
        pop_back();
}

template <class T, size_t N>
template <class... Args>
inline
typename fixed_vector<T, N>::iterator
fixed_vector<T, N>::emplace(const_iterator pos, Args&&... args)
{
    assert(size() + 1 <= N);
    assert(data() <= pos && pos <= m_end);
    T *to = m_end + 1;
    T *from = m_end++;
    while (from != pos)
        (void)new (--to) T(std::move(*--from));
    (void)new (from) T(args...);
    return from;
}

template <class T, size_t N>
template <class... Args>
inline void
fixed_vector<T, N>::emplace_back(Args&&... args)
{
    assert(size() < N);
    (void)new (m_end++) T(args...);
}


// -- Relational Operators -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class T, size_t N>
inline
bool
operator == (const fixed_vector<T, N>& a, const fixed_vector<T, N>& b)
{
    if (a.size() != b.size())
        return false;
    for (auto ai = a.begin(), bi = b.begin(); ai != a.end(); ai++, bi++)
        if (*ai != *bi)
            return false;
    return true;
}

template <class T, size_t N>
inline
bool
operator != (const fixed_vector<T, N>& a, const fixed_vector<T, N>& b)
{
    return !(a == b);
}

template <class T, size_t N>
inline
bool
operator < (const fixed_vector<T, N>& a, const fixed_vector<T, N>& b)
{
    for (auto ai = a.begin(), bi = b.begin();
         ai != a.end() && bi != b.end();
         ai++, bi++)
    {
        if (*ai < *bi)
            return true;
        if (*bi < *ai)
            return false;
    }
    return a.size() < b.size();
}

template <class T, size_t N>
inline
bool
operator <= (const fixed_vector<T, N>& a, const fixed_vector<T, N>& b)
{
    return !(b < a);
}

template <class T, size_t N>
inline
bool
operator > (const fixed_vector<T, N>& a, const fixed_vector<T, N>& b)
{
    return b < a;
}

template <class T, size_t N>
inline
bool
operator >= (const fixed_vector<T, N>& a, const fixed_vector<T, N>& b)
{
    return !(a < b);
}

#endif /* !FIXED_VECTOR_included */
