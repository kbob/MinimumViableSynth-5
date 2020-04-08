#include <functional>
#include <iostream>
#include <memory>

template <class T>
struct data {
    data() : m_data(0) {}
    explicit data(const T& d)
    : m_data(d) {}
    public: T m_data;                                               // NOLINT
};

template <class T>
class Src  : public data<T> {
public:
    explicit Src(const T& d) : data<T>(d) {}
};

template <class T>
class Dest : public data<T> {};

template <class T>
class Ctl  : public data<T> {
public:
    explicit Ctl(const T& d) : data<T>(d) {}
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

typedef std::function<void()> action;                               // NOLINT
class CL {                                                          // NOLINT
public:
    virtual ~CL() = default;
    virtual action create_copy() = 0;                               // NOLINT
protected:
    CL() = default;
};

template <class S, class D, class C>
class TCL : public CL {
public:
    TCL(const Src<S> *s, Dest<D> *d, const Ctl<C> *c)
    : m_src(s), m_dest(d), m_ctl(c) {}
    action create_copy() override                                   // NOLINT
    {
        return [this] {
            m_dest->m_data = m_src->m_data * m_ctl->m_data;
        };
    }
private:
    const Src<S> *m_src;
    Dest<D> *m_dest;
    const Ctl<C> *m_ctl;
};

template <class S, class D, class C>
CL *make_cl(const Src<S> *s, Dest<D> *d, const Ctl<C> *c)           // NOLINT
{
    return new TCL<S, D, C>(s, d, c);                               // NOLINT
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

int main()                                                          // NOLINT
{
    Src<int> di(3);
    Dest<short> ds;                                                 // NOLINT
    Ctl<float> dc(4);
    std::unique_ptr<CL> clp(make_cl(&di, &ds, &dc));
    action a = clp->create_copy();

    std::cout << ds.m_data << std::endl;
    a();
    std::cout << ds.m_data << std::endl;

    return 0;
}
