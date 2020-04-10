#include <cstddef>
#include <iostream>
#include <string>

// -- Debugging - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#include <cxxabi.h>

#define HERE (std::cout << __FILE__ << ':' << __LINE__ << std::endl)

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

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class D> class I {};
template <class S> class O {};
template <class C> class Ct {
public:
    O<C> out;
};

template <class D, class S, class C>
class L {
public:
    L(I<D> *d, O<S> *s, O<C> *c, float scale)
    : m_d(d), m_s(s), m_c(c), m_scale(scale) {}
private:
    I<D>  *m_d;
    O<S>  *m_s;
    O<C> *m_c;
    float  m_scale;
};

template <class D, class S, class C>
L<D, S, C> make_l(I<D> *d, O<S> *s, O<C> *c, float scale = 1.0f)
{
    return L<D, S, C>(d, s, c, scale);
}

template <class D, class S, class C>
L<D, S, C> make_l(I<D> *d, O<S> *s, Ct<C> *c, float scale = 1.0f)
{
    return L<D, S, C>(d, s, &c->out, scale);
}

template <class D, class C>
L<D, void, C> make_l(I<D> *d, std::nullptr_t, O<C> *c, float scale = 1.0f)
{
    return L<D, void, C>(d, nullptr, c, scale);
}

template <class D, class C>
L<D, void, C> make_l(I<D> *d, std::nullptr_t, Ct<C> *c, float scale = 1.0f)
{
    return L<D, void, C>(d, nullptr, &c->out, scale);
}

template <class D, class S>
L<D, S, void> make_l(I<D> *d, O<S> *s, std::nullptr_t, float scale = 1.0f)
{
    return L<D, S, void>(d, s, nullptr, scale);
}

template <class D>
L<D, void, void> make_l(I<D> *d,
                        std::nullptr_t,
                        std::nullptr_t,
                        float scale = 1.0f)
{
    return L<D, void, void>(d, nullptr, nullptr, scale);
}

int main()
{
    I<float> d;
    O<char> s;
    O<short> c;
    Ct<bool> ct;

    auto l7 = make_l(&d, &s, &c);
    auto l7s = make_l(&d, &s, &c, 0.7f);
    auto l7c = make_l(&d, &s, &ct);

    auto l5 = make_l(&d, nullptr, &c);
    auto l5s = make_l(&d, nullptr, &c, 0.5f);
    auto l5c = make_l(&d, nullptr, &ct);

    auto l3 = make_l(&d, &s, nullptr);
    auto l3s = make_l(&d, &s, nullptr, 0.3f);

    auto l1 = make_l(&d, nullptr, nullptr);
    auto l1s = make_l(&d, nullptr, nullptr, 0.1f);

    std::cout << "l7  type is " << type_name(l7) << std::endl;
    std::cout << "l7s type is " << type_name(l7s) << std::endl;
    std::cout << "l7c type is " << type_name(l7c) << std::endl;
    std::cout << "l5  type is " << type_name(l5) << std::endl;
    std::cout << "l5s type is " << type_name(l5s) << std::endl;
    std::cout << "l5c type is " << type_name(l5c) << std::endl;
    std::cout << "l3  type is " << type_name(l3) << std::endl;
    std::cout << "l3s type is " << type_name(l3s) << std::endl;
    std::cout << "l1  type is " << type_name(l1) << std::endl;
    std::cout << "l1s type is " << type_name(l1s) << std::endl;

    return 0;
}
