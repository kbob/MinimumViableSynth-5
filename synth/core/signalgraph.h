#ifndef SIGNALGRAPH_included
#define SIGNALGRAPH_included

#include <cstddef>
#include <iostream>
#include <map>
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

    // Port() {}
    // virtual ~Port() {}

    const std::string& name() const
    {
        return m_name;
    }

    Port& name(const std::string& name)
    {
        m_name = name;
        return *this;
    }

protected:

    Port() {}
    virtual ~Port() {}

private:

    std::string m_name;

    Port(const Port&);
    Port& operator = (const Port&);

};

std::string port_name(const Port& p)
{
    return p.name().empty() ? type_name(p) : p.name();
}

class InputPort : public Port {

protected:

    InputPort() {}

};

class OutputPort : public Port {

protected:

    OutputPort() {}

};

// Control is a mixin that indicates that a value change on a port
// requires recalculating its destination module's parameters.
//
// Controls are for sometimes-changing values like MIDI CCs or
// GUI parameters.
class Control {

protected:

    Control() {}

};

template <class ElementType = sample>
class Input : public InputPort {

public:

    ElementType operator [] (size_t i) const;

};

template <class ElementType = double>
class ControlInput : public Input<ElementType>, public Control {

public:

    ElementType operator [] (size_t i) const;

};

template <class ElementType = sample>
class Output : public OutputPort {

public:

    ElementType& operator [] (size_t i) const;

};

template <class ElementType = double>
class ControlOutput : public Output<ElementType>, public Control {

};

// module has subclasses.
// module subclasses have inputs and outputs.
// module has way of looking up i's and o's.
// i's and o's are tied to their module somehow.

class Module {

public:

    Module() {}
    virtual ~Module() {}

    void name(const std::string& name)
    {
        m_name = name;
    }

    const std::string& name() const
    {
        return m_name;
    }

    const std::vector<Port *>& ports() const
    {
        return m_ports;
    }


    struct State {
        virtual ~State() {}
    };

    virtual State *create_state() const { return nullptr; }
    virtual void init_note_state(State *) const {}

    virtual void render(State *, size_t frame_count) const = 0;

protected:

    template <typename... Types>
    void ports(Port& p, Types&... rest)
    {
        std::cout << "port "
                  << type_name(*this)
                  << "."
                  << port_name(p)
                  << ": "
                  << type_name(p)
                  << std::endl;
        // std::cout << "module "
        //           << type_name(*this)
        //           << " port "
        //           << port_name(p)
        //           << std::endl;
        m_ports.push_back(&p);
        ports(rest...);
    }

    void ports() {}

private:

    std::string m_name;
    std::vector<Port *> m_ports;

    Module(const Module&);
    Module& operator = (const Module&);

};

std::string module_name(const Module& m)
{
    return m.name().empty() ? type_name(m) : m.name();
}

class Plan {

public:

private:


};

class SignalGraph {

public:

    SignalGraph& module(const Module& mod)
    {
        std::cout << "add module " << module_name(mod) << std::endl;

        m_modules.push_back(&mod);
        for (auto p : mod.ports()) {
            std::cout << "    port " << port_name(*p) << std::endl;
            m_port_modules[p] = &mod;

            // Create map entries in case modules has no inputs or no outputs.
            m_module_inputs[&mod];
            m_module_outputs[&mod];

            if (auto ip = dynamic_cast<InputPort *>(p))
                m_module_inputs[&mod].push_back(ip);
            if (auto op = dynamic_cast<OutputPort *>(p))
                m_module_outputs[&mod].push_back(op);
        }
        return *this;
    }

    SignalGraph& connect(const OutputPort& src, const InputPort& dest)
    {
        std::cout << "connect "
                  << module_name(*m_port_modules[&src])
                  << "."
                  << port_name(src)
                  << " to "
                  << module_name(*m_port_modules[&dest])
                  << "."
                  << port_name(dest)
                  << std::endl;
        for (auto i : m_links)
            if (i.src == &src && i.dest == &dest)
                assert(false && "ports are already connected");

        m_links.push_back(Link(&src, &dest));
        return *this;
    }

    void disconnect(const OutputPort&, const InputPort&);

    void dump_maps() const
    {
        std::cout << "m_port_modules:" << std::endl;
        for (auto i : m_port_modules)
            std::cout << "  "
                      << i.first->name()
                      << ": "
                      << module_name(*i.second)
                      << std::endl;
        std::cout << std::endl;

        std::cout << "m_module_inputs:" << std::endl;
        for (auto mod : m_modules) {
            std::cout << "    "
                      << module_name(*mod)
                      << ": [";
            const char *sep = "";
            for (auto p : m_module_inputs.at(mod)) {
                std::cout << sep << port_name(*p);
                sep = ", ";
            }
            std::cout << "]" << std::endl;
        }
        std::cout << std::endl;

        std::cout << "m_module_outputs:" << std::endl;
        for (auto mod : m_modules) {
            std::cout << "    "
                      << module_name(*mod)
                      << ": [";
            const char *sep = "";
            for (auto p : m_module_outputs.at(mod)) {
                std::cout << sep << port_name(*p);
                sep = ", ";
            }
            std::cout << "]" << std::endl;
        }
        std::cout << std::endl;
    }

private:

    struct Link {

        Link(const Port *src, const Port *dest)
            : src(src), dest(dest)
        {}

        const Port *src;
        const Port *dest;
    };

    std::vector<const Module *> m_modules;
    std::map<const Port *, const Module *> m_port_modules;
    std::map<const Module *, std::vector<InputPort *>> m_module_inputs;
    std::map<const Module *, std::vector<OutputPort *>> m_module_outputs;
    std::vector<Link> m_links;

};

#endif /* !SIGNALGRAPH_included */
