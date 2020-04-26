#include <cassert>
#include <functional>
#include <iostream>
#include <typeindex>
#include <utility>


// -- Debugging - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#include <cxxabi.h>

#define HERE (std::cout << __FILE__ << ':' << __LINE__ << std::endl)

// XXX Move this into the platform directory.
std::string demangle(const std::string& mangled)
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

typedef float SCALE_TYPE;
typedef std::function<void(size_t)> render_action;

class Port {
public:
    virtual ~Port() = default;
};

class InputPort : public Port {};
class OutputPort : public Port {};

template <class T>
class Input : public InputPort {
public:
    Input()
    : m_name{demangle(typeid(T).name())}
    {}
    std::string m_name;
};

template <class T>
class Output : public OutputPort {
public:
    Output()
    : m_name{demangle(typeid(T).name())}
    {}
    std::string m_name;
};

class Control {};

template<class C, class T>
class ControlType {
public:
    Output<T> out;
};

class VolumeControl : public ControlType<VolumeControl, float> {};

class Link {
public:
    template <class D, class S, class CT, class CE>
    Link(Input<D> *dest,
         Output<S> *src,
         ControlType<CT, CE> *ctl,
         SCALE_TYPE scale)
    : m_dest{dest}, m_src{src}, m_ctl{&ctl->out}
    {
        m_make_copy = [=] (InputPort *dest, OutputPort *src, OutputPort *ctl)
                     -> render_action {
            Input<D> *destD = dynamic_cast<Input<D> *>(dest);
            Output<S> *srcS = dynamic_cast<Output<S> *>(src);
            Output<CE> *ctlC = dynamic_cast<Output<CE> *>(ctl);
            assert(destD && srcS && ctlC);

            return [=] (size_t n) -> void {
                std::cout << destD->m_name
                          << " <- "
                          << srcS->m_name
                          << " * "
                          << ctlC->m_name
                          << " ^ "
                          << scale
                          << " ["
                          << n
                          << "]"
                          << std::endl;
            };
        };
    }
    template <class D, class S>
    Link(Input<D> *dest, Output<S> *src)
    : m_dest{dest}, m_src{src}, m_ctl{nullptr}
    {
        m_make_copy = [=] (InputPort *dest, OutputPort *src, OutputPort *)
                     -> render_action {
            Input<D> *destD = dynamic_cast<Input<D> *>(dest);
            Output<S> *srcS = dynamic_cast<Output<S> *>(src);
            assert(destD && srcS);

            return [=] (size_t n) -> void {
                std::cout << destD->m_name
                          <<" <- "
                          << srcS->m_name
                          << " ["
                          << n
                          << "]"
                          << std::endl;
            };
        };
    }

    template <class D>
    Link(Input<D> *dest, nullptr_t src)
    : m_dest{dest}, m_src{src}, m_ctl{nullptr}
    {
        m_make_copy = [=] (InputPort *dest, OutputPort *src, OutputPort *)
                     -> render_action {
            Input<D> *destD = dynamic_cast<Input<D> *>(dest);
            assert(src == nullptr);

            return [=] (size_t n) -> void {
                std::cout << destD->m_name
                          <<" <- "
                          << "nothing"
                          << " ["
                          << n
                          << "]"
                          << std::endl;
            };
        };
    }

    render_action
    make_copy_action(InputPort *dest, OutputPort *src, OutputPort *ctl)
    {
        return m_make_copy(dest, src, ctl);
    }

    InputPort *m_dest;
    OutputPort *m_src;
    OutputPort *m_ctl;
    std::function<render_action(InputPort *,
                                OutputPort *,
                                OutputPort *)> m_make_copy, m_make_add;
};

class Lynq {
public:
    InputPort *m_dest;
    OutputPort *m_src;
    OutputPort *m_ctl;

};

int main()
{
    Input<char> dest;
    Output<double> src;
    VolumeControl vol;
    Link link1(&dest, &src);
    Link link2(&dest, nullptr);
    Link link3(&dest, &src, &vol, 0.5f);


    Input<char> dest2;
    Output<double> src2;
    VolumeControl vol2;
    dest2.m_name += "2";
    src2.m_name += "2";
    vol2.out.m_name += "2";

    render_action a = link1.make_copy_action(&dest, &src, nullptr);
    a(32);

    auto a2 = link2.make_copy_action(&dest, nullptr, nullptr);
    a2(16);

    auto a3 = link3.make_copy_action(&dest2, &src2, &vol2.out);
    a3(8);

    std::cout << "sizeof (Lynq) = " << sizeof (Lynq) << std::endl;
    std::cout << "sizeof (Link) = " << sizeof (Link) << std::endl;
    std::cout << "sizeof link1.m_make_copy = "
              << sizeof link1.m_make_copy
              << std::endl;

    return 0;
}
