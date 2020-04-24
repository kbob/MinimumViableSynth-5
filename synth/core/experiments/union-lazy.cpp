#include <cassert>
#include <iostream>
#include <sstream>
#include <string>


// -- Debugging - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#include <cxxabi.h>

#define HERE (std::cout << __FILE__ << ':' << __LINE__ << std::endl)

// XXX Move this into the platform directory.
static std::string demangle(const std::string& mangled)
{
    int status;
    char *s = abi::__cxa_demangle(mangled.c_str(), 0, 0, &status);
    std::string demangled(s);
    std::free(s);
    return demangled;
}

template <typename T>
static std::string type_name(T& obj)
{
    const std::string& mangled = typeid(*&obj).name();
    return demangle(mangled);
}

template <class T, typename... Types>
std::string arglist_types(T first, Types&... rest)
{
    return type_name(first) + ", " + arglist_types(rest...);
}

template <class T>
std::string arglist_types(T last)
{
    return type_name(last);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


std::string haddr(const void *p)
{
    std::stringstream stream;
    stream << "..."
           << std::hex
           << (reinterpret_cast<uintptr_t>(p) & 0xFFF);
    return stream.str();
}

template <class T>
class deferred {

public:

    deferred()
    : m_initialized{false}
    {
        std::cout << "   deferred: default constructor at "
                  << haddr(this)
                  << "\n";
    }

    deferred(const deferred& that)
    : m_initialized{that.m_initialized}
    {
        std::cout << "    deferred: copy "
                  << haddr((void *)&that)
                  << " to "
                  << haddr(this)
                  << "\n";
        if (that.m_initialized)
            new (&m_holder.member) T{that.m_holder.member};
    }

    ~deferred()
    {
        if (m_initialized) m_holder.member.~T();
    }

    template <typename... A>
    void construct(A... args)
    {
        std::cout << "    begin deferred construct at " << haddr(this) << "\n";
        std::cout << "        type is " << type_name(m_holder.member) << "\n";
        std::cout << "        args are (" << arglist_types(args...) << ")\n";
        assert(!m_initialized);
        new (&m_holder.member) T{args...};
        m_initialized = true;
        std::cout << "    end deferred construct\n";
    }

    const T& operator * () const
    {
        assert(m_initialized);
        return m_holder.member;
    }

    const T *operator -> () const
    {
        assert(m_initialized);
        return &m_holder.member;
    }

    T& operator * ()
    {
        assert(m_initialized);
        return m_holder.member;
    }

    T *operator -> ()
    {
        assert(m_initialized);
        return &m_holder.member;
    }

private:

    bool m_initialized;
    union holder {
        holder() {}
        holder(const holder&) {}
        ~holder() {}
        T member;
    } m_holder;

};

template <class T>
class U {

public:

    U(const T& x)
    {
        std::cout << "        U constructor at " << haddr(this) << std::endl;
        data[0] = x;
    }

    U(const U& that)
    {
        std::cout << "        U copy from "
                  << haddr(&that)
                  << " to "
                  << haddr(this)
                  << std::endl;
        data[0] = that.data[0];
    }

   ~U()
    {
        std::cout << "        U destructor at " << haddr(this) << std::endl;
    }

   T data[100];

};

class C {
public:
    void finalize() { m_deferred_u.construct('a'); }
    char get_data() { return m_deferred_u->data[0]; }
private:
    deferred<U<char>> m_deferred_u;
};

// class XXXC {
//
// public:
//
//     XXXC()
//     : flag{false}
//     {
//         std::cout << "C constructor" << std::endl;
//     }
//
//     ~XXXC()
//     {
//         if (flag)
//             u_holder.member.~U();
//         std::cout << "C destructor" << std::endl;
//    }
//
//    void finalize()
//    {
//        new (&u_holder.member) U<char>('a');
//        flag = true;
//    }
//
// private:
//
//     template <class T>
//     union holder {
//         holder() {}
//         ~holder() {}
//         U<T> member;
//     };
//
//     bool flag;
//     holder<char> u_holder;
//
// };

int main()
{
    C c;
    std::cout << "main: c is constructed at " << haddr(&c) << "\n";
    C c1{c};
    std::cout << "main: c1 is constructed at " << haddr(&c1) << "\n";
    c.finalize();
    std::cout << "main: c is finalized\n";
    C c2{c};
    std::cout << "main: c2 is constructed at " << haddr(&c2) << "\n";
    char d = c.get_data();
    std::cout << "main: got data " << d << std::endl;
    std::cout << "main: returning\n";
    return 0;
}
