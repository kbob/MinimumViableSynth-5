#ifndef FIXED_QUEUE_included
#define FIXED_QUEUE_included

#include <cassert>
#include <type_traits>
#include <utility>


// fixed_queue<T, N> is like std::queue<T, Container> with the following
// exceptions.
//
//   - maximum queue size is fixed to N elements.
//   - does not use underlying container.
//   - does not use allocator.
//   - `container_type` is not defined.
//   - just one constructor, no assignment.
//   - `swap` not implemented.


template <class T, size_t N>
class fixed_queue {

public:

    // Types
    typedef T        value_type;
    typedef T&       reference;
    typedef const T& const_reference;
    typedef size_t   size_type;

    // Constructors
    fixed_queue();
    fixed_queue(const fixed_queue&) = delete;

    // Destructor
    ~fixed_queue();

    // Assignment
    fixed_queue& operator = (const fixed_queue&) = delete;

    // Capacity
    bool empty() const;
    bool full() const;
    size_type size() const;
    size_type max_size() const;

    // Element Access
    reference front();
    const_reference front() const;
    reference back();
    const_reference back() const;

    // Modifiers
    void push(const value_type&);
    void push(value_type&&);
    template <class... Args>
        void emplace(Args&&...);
    void pop();

    // Relational Operators
    template <class T1, size_t N1>
    friend
    bool
    operator == (const fixed_queue<T1, N1>& a, const fixed_queue<T1, N1>& b);

    template <class T1, size_t N1>
    friend
    bool
    operator < (const fixed_queue<T1, N1>&, const fixed_queue<T1, N1>&);

private:

    typedef typename std::aligned_storage<sizeof(T), alignof(T)>::type pod_type;

    T *m_begin;
    T *m_end;
    pod_type m_store[N];

    T *inc(T *p)
    {
        T *store = reinterpret_cast<T *>(m_store);
        p++;
        if (p == store + N)
            p = store;
        return p;
    }

    const T *inc(const T *p) const
    {
        return const_cast<fixed_queue *>(this)->inc(const_cast<T *>(p));
    }

    T *push_pos()
    {
        assert(size() < N);
        T *store = reinterpret_cast<T *>(m_store);
        if (empty())
            m_begin = m_end = store;
        T *pos = m_end;
        m_end = inc(m_end);
        return pos;
    }

    friend class fixed_queue_unit_test;

};

// Relational Operators
template <class T, size_t N>
bool
operator != (const fixed_queue<T, N>&, const fixed_queue<T, N>&);

template <class T, size_t N>
bool
operator <= (const fixed_queue<T, N>&, const fixed_queue<T, N>&);

template <class T, size_t N>
bool
operator > (const fixed_queue<T, N>&, const fixed_queue<T, N>&);

template <class T, size_t N>
bool
operator >= (const fixed_queue<T, N>&, const fixed_queue<T, N>&);


// -- Constructors - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class T, size_t N>
inline
fixed_queue<T, N>::fixed_queue()
: m_begin{nullptr},
  m_end{nullptr}
{}


// -- Destructor  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class T, size_t N>
inline
fixed_queue<T, N>::~fixed_queue()
{
    while (!empty())
        pop();
}


// -- Capacity -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class T, size_t N>
inline
bool
fixed_queue<T, N>::empty() const
{
    return m_begin == nullptr;
}

template <class T, size_t N>
inline
bool
fixed_queue<T, N>::full() const
{
    return m_begin && m_begin == m_end;
}

template <class T, size_t N>
inline
typename fixed_queue<T, N>::size_type
fixed_queue<T, N>::size() const
{
    if (empty())
        return 0;
    size_t d = N + m_end - m_begin;
    if (d > N)
        d -= N;
    return d;
}

template <class T, size_t N>
inline
typename fixed_queue<T, N>::size_type
fixed_queue<T, N>::max_size() const
{
    return N;
}


// -- Element Access -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class T, size_t N>
inline
typename fixed_queue<T, N>::reference
fixed_queue<T, N>::front()
{
    assert(!empty());
    return *m_begin;
}

template <class T, size_t N>
inline
typename fixed_queue<T, N>::const_reference
fixed_queue<T, N>::front() const
{
    assert(!empty());
    return *m_begin;
}

template <class T, size_t N>
inline
typename fixed_queue<T, N>::reference
fixed_queue<T, N>::back()
{
    assert(!empty());
    if (m_end == reinterpret_cast<T *>(m_store))
        return m_end[N - 1];
    else
        return m_end[-1];
}

template <class T, size_t N>
inline
typename fixed_queue<T, N>::const_reference
fixed_queue<T, N>::back() const
{
    return const_cast<fixed_queue *>(this)->back();
}


// -- Modifiers - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class T, size_t N>
inline
void
fixed_queue<T, N>::push(const value_type& val)
{
    emplace(val);
}

template <class T, size_t N>
inline
void
fixed_queue<T, N>::push(value_type&& val)
{
    (void)new (push_pos()) T(std::move(val));
}

template <class T, size_t N>
template <class... Args>
inline
void
fixed_queue<T, N>::emplace(Args&&... args)
{
    (void)new (push_pos()) T(args...);
}

template <class T, size_t N>
inline
void
fixed_queue<T, N>::pop()
{
    assert(!empty());
    m_begin->~T();
    m_begin = inc(m_begin);
    if (m_begin == m_end)
        m_begin = m_end = nullptr;
}


// -- Relational Operators -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class T, size_t N>
inline
bool
operator == (const fixed_queue<T, N>& a, const fixed_queue<T, N>& b)
{
    size_t a_size = a.size();
    if (a_size != b.size())
        return false;

    if (a.empty())
        return true;

    const T *ap = a.m_begin;
    const T *bp = b.m_begin;
    for (size_t i = 0; i < a_size; i++) {
        if (*ap != *bp)
            return false;
        // ap++; bp++;
        ap = a.inc(ap);
        bp = b.inc(bp);
    }
    return true;
}

template <class T, size_t N>
inline
bool
operator != (const fixed_queue<T, N>& a, const fixed_queue<T, N>& b)
{
    return !(a == b);
}

template <class T, size_t N>
inline
bool
operator < (const fixed_queue<T, N>& a, const fixed_queue<T, N>& b)
{
    size_t a_empty = a.empty();
    size_t b_empty = b.empty();
    if (a_empty)
        return !b_empty;
    if (b_empty)
        return false;

    size_t a_size = a.size();
    size_t b_size = b.size();
    const T *ap = a.m_begin;
    const T *bp = b.m_begin;
    for (size_t i = 0; i < a_size && i < b_size; i++) {
        if (*ap < *bp)
            return true;
        if (*bp < *ap)
            return false;
        ap = a.inc(ap);
        bp = b.inc(bp);
    }
    return a_size < b_size;
}

template <class T, size_t N>
inline
bool
operator <= (const fixed_queue<T, N>& a, const fixed_queue<T, N>& b)
{
    return !(b < a);
}

template <class T, size_t N>
inline
bool
operator > (const fixed_queue<T, N>& a, const fixed_queue<T, N>& b)
{
    return b < a;
}

template <class T, size_t N>
inline
bool
operator >= (const fixed_queue<T, N>& a, const fixed_queue<T, N>& b)
{
    return !(a < b);
}

#endif /* !FIXED_QUEUE_included */
