#ifndef NOALLOC_included
#define NOALLOC_included

#include <cassert>
#include <memory>
#include <type_traits>

// vector_pool<T, N> is an allocator for vectors that can't
// grow beyond N elements.  Use it like this.
//
//     std::vector<MyType, vector_pool<MyType, 42>> my_vec;
//
// The storage is allocated inside the vector object, so there
// is no heap allocation.

template <class T, int N>
class vector_pool {

public:

    typedef T         value_type;
    typedef T        *pointer;
    typedef T&        reference;
    typedef const T  *const_pointer;
    typedef const T&  const_reference;
    typedef size_t    size_type;
    typedef ptrdiff_t difference_type;

    // default constructors, destructor, assignment.

    pointer address(reference x)             const noexcept { return &x; }
    const_pointer address(const_reference x) const noexcept { return &x; }

    pointer allocate(size_type n, std::allocator<void>::const_pointer hint = 0)
    {
        if (n > N) {
            throw std::length_error("fixed vector overflow");
        }
        *&hint = hint;          // -Wunused-parameter w/ NDEBUG
        assert(n <= N);
        assert(hint == 0 || hint == m_store);
        return reinterpret_cast<pointer>(m_store);
    }

    void deallocate(pointer, size_type) {}

    size_type max_size() const noexcept { return N; }

    template <class U, class ... Args>
    void construct(U *p, Args&&... args)
    {
        new (reinterpret_cast<void *>(p)) U(args...);
    }

    template <class U, class ... Args>
    void destroy(U *p)
    {
        reinterpret_cast<T *>(p)->~T();
    }

private:

    typedef typename std::aligned_storage<sizeof(T), alignof(T)>::type pod_type;
    pod_type m_store[N];

};

// fixed_vector<T, N> is a vector with static allocation for N elements.
template <class T, int N>
using fixed_vector = std::vector<T, vector_pool<T, N>>;

#endif /* !NOALLOC_included */
