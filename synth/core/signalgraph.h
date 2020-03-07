#ifndef SIGNALGRAPH_included
#define SIGNALGRAPH_included

#include <cstddef>
#include <iostream>
#include <unordered_map>
#include <vector>

#include <cxxabi.h>

#define HERE (std::cout << __FILE__ << ':' << __LINE__ << std::endl)

// XXX Move this into the platform directory.
static std::string demangle(const char *abi_name)
{
    int status;
    char *s = abi::__cxa_demangle(abi_name, 0, 0, &status);
    std::string demangled(s);
    std::free(s);
    return demangled;
}

template <typename T>
static std::string type_name(T& obj)
{
    std::string name = typeid(*&obj).name();
    return demangle(name.c_str());
}

typedef float sample;

class Port {

public:

    Port() {}
    virtual ~Port() {}

    const std::string& name() const
    {
        return m_name;
    }

    Port& name(const std::string& name)
    {
        m_name = name;
        return *this;
    }

private:

    std::string m_name;

    Port(const Port&);
    Port& operator = (const Port&);

};

std::string port_name(const Port& p)
{
    return p.name().empty() ? type_name(p) : p.name();
}

// Let's redo the port hierarchy.
//
//    Port (abstract)
//      InputPort (abstract)
//        SignalInput
//        ControlInput
//      OutputPort (abstract)
//        SignalOutput
//        ControlOutput
//
// ... or "control" is an attribute?
// ... Does control mean lower sample rate or has a GUI or
// ... only runs when changing or what?

// Port subclasses have operator [] that know how to find the
// right value.  Should it also have iterators?  Sigh...

template <class ElementType = sample>
class Input : public Port {

public:

    ElementType operator [] (size_t i) const;

};

template <class ElementType = sample>
class Output : public Port {

public:

    ElementType& operator [] (size_t i) const;

};

// A control can be a modulation input, a MIDI CC, or a GUI parameter.
// Control will need ways to specify name, min, max, range, display,
// etc.
template <class ValueType = double>
class Control : public Port {

public:

    ValueType operator [] (size_t i) const;

};

// module has subclasses.
// module subclasses have inputs and outputs.
// module has way of looking up i's and o's.
// i's and o's are tied to their module somehow.

class Module {

public:

    Module() {}

    virtual ~Module()
    {
        std::cout << "delete module " << type_name(*this) << std::endl;
    }

    struct State {
        virtual ~State() {}
    };

    const std::vector<Port *>& ports() const
    {
        return m_ports;
    }

    virtual State *create_state() const { return nullptr; }
    virtual void init_note_state(State *) const {}

    virtual void render(State *, size_t frame_count) const = 0;

protected:

    template <typename... Types>
    void ports(Port& p, Types&... rest)
    {
        std::cout << "module "
                  << type_name(*this)
                  << " port "
                  << port_name(p)
                  << std::endl;
        m_ports.push_back(&p);
        ports(rest...);
    }

    void ports() {}

private:

    std::vector<Port *> m_ports;

    Module(const Module&);
    Module& operator = (const Module&);

};

class SignalGraph {

public:

    SignalGraph& module(const Module& mod)
    {
        std::cout << "add module " << type_name(mod) << std::endl;

        m_modules.push_back(&mod);
        for (auto p : mod.ports()) {
            // std::cout << "    port " << port_name(*p) << std::endl;
            std::cout << "    port "
                      << p->name()
                      << " "
                      << type_name(*p)
                      << std::endl;
        }
        return *this;
    }

    template <class OutputType, class InputType>
    SignalGraph& connect(const Output<OutputType>& src,
                         const Input<InputType>& dest)
    {
        std::cout << "connect "
                  << port_name(src)
                  << " to "
                  << port_name(dest)
                  << std::endl;
        for (auto i : m_links)
            if (i.src == &src && i.dest == &dest)
                assert(false && "ports are already connected");

        m_links.push_back(Link(&src, &dest));
        return *this;
    }

    template <class OutputType, class ControlType>
    SignalGraph& connect(const Output<OutputType>& src,
                         const Control<ControlType>& dest);

    template <class OutputType, class InputType>
    void disconnect(const Output<OutputType>&, const Input<InputType>&);

    template <class OutputType, class ControlType>
    void disconnect(const Output<OutputType>&, const Control<ControlType>&);

private:

    struct Link {

        Link(const Port *src, const Port *dest)
            : src(src), dest(dest)
        {}

        const Port *src;
        const Port *dest;
    };

    std::vector<const Module *> m_modules;
    // std::unordered_map<const Port *, const Module *> m_port_modules;
    std::vector<Link> m_links;

};

#endif /* !SIGNALGRAPH_included */
