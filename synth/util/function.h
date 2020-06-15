#ifndef FUNCTION_included
#define FUNCTION_included

#include <cassert>
#include <cstddef>


// -- function -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
// function is a poor man's std::function<T>.  It wraps a callable whose
// first argument is `void *` user data.  The function should cast that
// into whatever reference it needs to find its state.
//
// function has an inner type, `binding`, which binds a pointer to
// member function to a `function`.  It is a lot like a Python bound
// method.  The usage is kind of ugly:
//
//    typedef function<result(arg_type)> f_type;
//    f_type f = f_type::binding<MyClass, &MyClass::mem_fun>(&my_instance);
//
// or, within a class definition:
//
//    struct Foo {
//        result some_method(arg_type arg) {}
//        f_type m_binding;
//        Foo() : m_binding{f_type::binding<Foo, &Foo::some_method>(this)} {}
//    };
//
// Using Clang 11.0, `function` uses two words of memory vs.
// `std::function<T>`'s six, and turns into a single indirect
// function call.

template <class F> class function;

template <class R, class ...Args>
class function<R(Args...)> {

public:

    using result_type   = R;
    using function_type = R(void *, Args...);

    function();
    function(const function&) = default;

    function(function_type *, void *);

    template <class T, R (T::*M)(Args...)>
    class binding;

    result_type
    operator () (Args...);

    operator bool () const;

    template <class R1, class ...A1>
    friend bool operator == (const function<R1(A1...)>&, std::nullptr_t);

    template <class R1, class ...A1>
    friend bool operator == (std::nullptr_t, const function<R1(A1...)>&);

    template <class R1, class ...A1>
    friend bool operator != (const function<R1(A1...)>&, std::nullptr_t);

    template <class R1, class ...A1>
    friend bool operator != (std::nullptr_t, const function<R1(A1...)>&);

private:

    function_type *m_fun;
    void          *m_data;

    friend class function_unit_test;

};


// -- function::binding -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class R, class ...Args>
template <class T, R (T::*M)(Args...)>
class
function<R(Args...)>::
binding
{

public:

    binding(T *obj)
    : m_obj{obj}
    {}

    operator function () const
    {
        return function(invoke_method, m_obj);
    }

private:

    static R invoke_method(void *obj, Args... args)
    {
        return (static_cast<T *>(obj)->*M)(args...);
    }

    T *m_obj;

};


// -- function implementation -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class R, class ...Args>
inline
function<R(Args...)>::
function()
: m_fun{nullptr},
  m_data{nullptr}
{}

template <class R, class ...Args>
inline
function<R(Args...)>::
function(function_type f, void *p)
: m_fun{f},
  m_data{p}
{}

template <class R, class ...Args>
inline R
function<R(Args...)>::
operator () (Args... args)
{
    assert(m_fun);
    return (*m_fun)(m_data, args...);
}

template <class R, class ...Args>
inline
function<R(Args...)>::
operator bool() const
{
    return m_fun != nullptr;
}


// -- relational operators -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class R, class ...Args>
inline bool
operator == (const function<R(Args...)>& a, std::nullptr_t)
{
    return a.m_fun == nullptr;
}

template <class R, class ...Args>
inline bool
operator == (std::nullptr_t, const function<R(Args...)>& b)
{
    return nullptr == b.m_fun;
}

template <class R, class ...Args>
inline bool
operator != (const function<R(Args...)>& a, std::nullptr_t)
{
    return a.m_fun != nullptr;
}

template <class R, class ...Args>
inline bool
operator != (std::nullptr_t, const function<R(Args...)>& b)
{
    return nullptr != b.m_fun;
}

#endif /* !FUNCTION_included */
