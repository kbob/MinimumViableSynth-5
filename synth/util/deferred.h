#ifndef DEFERRED_included
#define DEFERRED_included

#include <cassert>

// deferred<T> is a wrappers that defers an object's construction.
//
// Example:
//
//      struct C {
//          deferred<obj> lazy_obj;
//          void sometime_later()
//          {
//              lazy_obj.construct(args);
//              lazy_obj->method();
//          }
//      };
//
// The inner object is destructed iff it had been constructed.
//
// Copying does the right thing: only invokes the object's copy
// constructor if the source object has been constructed.
//
// The assignment operators and move constructor are unexplored.
// I don't know whether they compile, nor whether they work.

template <class T>
class deferred {

public:

    typedef T value_type;

    deferred()
    : m_constructed{false}
    {}

    deferred(const deferred& that)
    : m_constructed{that.m_constructed}
    {
        if (that.m_constructed)
            new (&m_holder.member) T{that.m_holder.member};
    }

    ~deferred()
    {
        if (m_constructed)
            m_holder.member.~T();
    }

    bool is_constructed() const { return m_constructed; }

    template <typename... A>
    void construct(const A&... args)
    {
        assert(!m_constructed);
        new (&m_holder.member) T{args...};
        m_constructed = true;
    }

    const T& operator * () const
    {
        assert(m_constructed);
        return m_holder.member;
    }

    const T *operator -> () const
    {
        assert(m_constructed);
        return &m_holder.member;
    }

    T& operator * ()
    {
        assert(m_constructed);
        return m_holder.member;
    }

    T *operator -> ()
    {
        assert(m_constructed);
        return &m_holder.member;
    }

private:

    bool m_constructed;
    union holder {
        holder() {}
        holder(const holder&) {}
        ~holder() {}
        T member;
    } m_holder;

};

#endif /* !DEFERRED_included */
