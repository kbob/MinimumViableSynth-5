#ifndef SIGNALGRAPH_included
#define SIGNALGRAPH_included

#include <cstddef>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>

#include <cxxabi.h>

const double DEFAULT_RANGE = 1.0;
const double DEFAULT_INTENSITY = 1.0;

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

typedef float sample;

class Port {

public:

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

    Port(const Port&) = delete;
    Port& operator = (const Port&) = delete;

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

// A non-control input can only be connected to one output.
template <class ElementType = sample>
class Input : public InputPort {

public:

    ElementType operator [] (size_t i) const;

};

template <class ElementType = double>
class ControlInput : public Input<ElementType>, public Control {

public:

    ControlInput(range = DEFAULT_RANGE)
        : m_range(range)
    {}

    double range() const
    {
        return m_range;
    }

    void range(double range)
    {
        m_range = range;
    }

    ElementType operator [] (size_t i) const;

private:

    double m_range;

};

template <class ElementType = sample>
class Output : public OutputPort {

public:

    ElementType& operator [] (size_t i) const;

};

// XXX need ElementTypes that imply transforms.
// E.g., MIDINote or BipolarDouble type.
template <class ElementType = double>
class ControlOutput : public Output<ElementType>, public Control {

};

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
        // std::cout << "port "
        //           << type_name(*this)
        //           << "."
        //           << port_name(p)
        //           << ": "
        //           << type_name(p)
        //           << std::endl;

        m_ports.push_back(&p);
        ports(rest...);
    }

    void ports() {}

private:

    std::string m_name;
    std::vector<Port *> m_ports;

    Module(const Module&) = delete;
    Module& operator = (const Module&) = delete;

};

std::string module_name(const Module& m)
{
    return m.name().empty() ? type_name(m) : m.name();
}

class SignalGraph {

public:

    SignalGraph& module(const Module& mod)
    {
        // std::cout << "add module " << module_name(mod) << std::endl;

        m_modules.push_back(&mod);
        for (auto p : mod.ports()) {
            // std::cout << "    port " << port_name(*p) << std::endl;
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

    // XXX In addition to src and dest, we need to specify
    //   Transform -- now part of ControlOutput
    //   Intensity -- default to 1.0
    //   Range     -- now part of ControlInput
    //   Enabled   -- default to false
    SignalGraph& connect(const OutputPort& src, const InputPort& dest)
    {
        // std::cout << "connect "
        //           << module_name(*m_port_modules[&src])
        //           << "."
        //           << port_name(src)
        //           << " to "
        //           << module_name(*m_port_modules[&dest])
        //           << "."
        //           << port_name(dest)
        //           << std::endl;

        for (auto i : m_links)
            if (i.src == &src && i.dest == &dest)
                assert(false && "ports are already connected");

        m_links.push_back(Link(&src, &dest));
        return *this;
    }

    SignalGraph& disconnect(const OutputPort& src, const InputPort& dest)
    {
        // std::cout << "disconnect "
        //           << module_name(*m_port_modules[&src])
        //           << "."
        //           << port_name(src)
        //           << " from "
        //           << module_name(*m_port_modules[&dest])
        //           << "."
        //           << port_name(dest)
        //           << std::endl;

        auto match = [&] (const Link& link) {
            return link.src == &src && link.dest == &dest;
        };
        auto it = std::find_if(m_links.begin(), m_links.end(), match);
        assert(it != m_links.end() && "ports are not connected");
        m_links.erase(it);
        return *this;
    }

    // Plan make_plan() const
    // {
    //     size_t undone_count = m_modules.size();
    //     std::vector<bool> done(m_modules.size(), false);
    //
    //     while (true) {
    //         auto it = std::find_if(done.begin(),
    //                                done.end(), false);
    //         if (it == done.end())
    //             break;
    //         size_t i = it - done.begin();
    //         if (is_ready(m_modules[i])) {
    //             plan.push_back(m_modules[i]);
    //         }
    //     }
    // }

    void dump_maps() const
    {
        std::cout << "m_port_modules:" << std::endl;
        for (auto i : m_port_modules)
            std::cout << "    "
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

        std::cout << "m_links:" << std::endl;
        for (auto link : m_links)
            std::cout << "    "
                      << module_name(*m_port_modules.at(link.src))
                      << "."
                      << port_name(*link.src)
                      << " -> "
                      << module_name(*m_port_modules.at(link.dest))
                      << "."
                      << port_name(*link.dest)
                      << std::endl;
        std::cout << std::endl;
    }

private:

    struct Link {

        // Link should have src, dest, transform, intensity, range, enabled bit.
        Link(const OutputPort *src, const InputPort *dest)
            : src(src),
              dest(dest),
              intensity(DEFAULT_INTENSITY),
              enabled(false)
        {}

        const OutputPort *src;
        const InputPort *dest;
        double intensity;
        bool enabled;
    };

    std::vector<const Module *> m_modules;
    std::map<const Port *, const Module *> m_port_modules;
    std::map<const Module *, std::vector<InputPort *>> m_module_inputs;
    std::map<const Module *, std::vector<OutputPort *>> m_module_outputs;
    std::vector<Link> m_links;

};

#endif /* !SIGNALGRAPH_included */
