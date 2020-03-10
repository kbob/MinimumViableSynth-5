#ifndef SIGNALGRAPH_included
#define SIGNALGRAPH_included

#include <cstddef>
#include <iostream>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

#include <cxxabi.h>

const double  DEFAULT_RANGE      = 1.0;
const double  DEFAULT_INTENSITY  = 1.0;
const bool    DEFAULT_ENABLEMENT = false;
typedef float DEFAULT_SAMPLE_TYPE;

// -- Debugging - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

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

// -- Ports -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
// Ports are endpoints through which synth data passes.  They are
// attached to modules, and modules read and write them when they render
// signals.
//
// Ports are classified either as inputs or outputs.  Inputs provide
// data to a module, and outputs take data away.
//
// Ports also have types.  A type can be a language type like int,
// double, or stereo float, or something more abstract like MIDI note
// number.  (XXX types are not really defined yet.  I just know there
// will be many.
//
// Ports are also classified either as signal or control ports.  Signal
// ports transfer data without processing whenever they are connected.
// Control ports are more complex.  They do processing on their data,
// transforming it, scaling it, switching it on and off, and summing
// several outputs into one input.
//
// The intent is that audio data passes through signal ports, and
// "control voltage" data passes through control ports.  But the code
// doesn't enforce that distinction.  You can, *e.g.*, connect an
// audio-rate oscillator's output signal port to your filter's
// resonance control port.
//
// Control ports can also be attached to GUI controls or MIDI messages.
//
// XXX Someday I want to short-circuit unnecessary work for controls
// whose values are not changing.  If the user hasn't touched the knob,
// or the envelope is in its sustain phase, there's no point in filling
// buffer after buffer with the value 0.83376.  But that is a
// future optimization.

// `Port` is an abstract base class for all ports.
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

    // Abstract base class.  Must subclass to use.
    Port() {}
    virtual ~Port() {}

private:

    std::string m_name;

    // Ports cannot be copied, assigned, or moved.
    Port(const Port&) = delete;
    Port& operator = (const Port&) = delete;

};

std::string port_name(const Port& p)
{
    return p.name().empty() ? type_name(p) : p.name();
}

// `InputPort` is an abstract base class for input ports.
class InputPort : public Port {

protected:

    // Abstract base class.  Must subclass to use.
    InputPort() {}

};

// `OututPort` is an abstract base class for output ports.
class OutputPort : public Port {

protected:

    // Abstract base class.  Must subclass to use.
    OutputPort() {}

};

// `Control` is a mixin.  It marks a port as a control port.  (See above.)
class Control {

protected:

    // Abstract base class.  Must subclass to use.
    Control() {}

};

// `Input<T>` is a signal input port.
template <class ElementType = DEFAULT_SAMPLE_TYPE>
class Input : public InputPort {

public:

    ElementType operator [] (size_t i) const;

};

// `ControlInput<T>` is a control input port.
// A `ControlInput<T>` has an element type and a range.
// The range sets the maximum meaningful value for the input.
template <class ElementType = double>
class ControlInput : public Input<ElementType>, public Control {

public:

    ControlInput(const ElementType& range = DEFAULT_RANGE)
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

// `Output<T>` is a signal output port.
template <class ElementType = DEFAULT_SAMPLE_TYPE>
class Output : public OutputPort {

public:

    ElementType& operator [] (size_t i) const;

};

// `ControlOutput<T>` is a control output port.
// The ElementType can be used to imply a transform.
// XXX need to work out the transform details.
template <class ElementType = double>
class ControlOutput : public Output<ElementType>, public Control {};

// -- Links -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// XXX write something
// XXX discuss src/dest, input/output
// XXX a link is like a patch cord.

class Link {

public:

    // XXX use a better name than "Key".
    typedef std::pair<OutputPort *, InputPort *> Key;

    Link(OutputPort& src, InputPort& dest)
        : m_src(&src),
          m_dest(&dest)
    {}

    virtual ~Link() {}

    Key key() const
    {
        return std::make_pair(m_src, m_dest);
    }

    virtual bool is_active() const
    {
        return true;            // signal links are always active.
    }

private:

    OutputPort *m_src;
    InputPort *m_dest;

};

class ControlLink : public Link {

public:

    ControlLink(OutputPort& src,
                InputPort& dest,
                double intensity = DEFAULT_INTENSITY,
                bool enabled = DEFAULT_ENABLEMENT)
        : Link(src, dest),
          m_intensity(intensity),
          m_enabled(enabled)
    {}

    virtual bool is_active() const
    {
        return m_enabled && m_intensity != 0.0;
    }

    double intensity() const
    {
        return m_intensity;
    }

    bool enabled() const
    {
        return m_enabled;
    }

    void intensity(double intensity)
    {
        m_intensity = intensity;
    }

    void enabled(bool enabled)
    {
        m_enabled = enabled;
    }

private:

    double m_intensity;
    bool m_enabled;

};

Link *make_link(OutputPort& src, InputPort& dest)
{
    if (dynamic_cast<Control *>(&dest))
        return new ControlLink(src, dest);
    else
        return new Link(src, dest);
}

// -- Modules  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
// A module is a unit of audio processing.  It is very much analogous to
// a module in a modular synth: it accepts several continuous data
// streams, has several controls, and emits one or more continuous data
// streams.

//
// Data flows into and out of a module through ports (see above).
// The ports should be declared as public data members of the module.
//
// A module has a `render` member function.  The `render` function
// processes a block of samples.  It reads from its input ports
// and write to its output ports for every sample.
//
// A module may need to keep state around between renderings.  If so,
// it can define a `State` which derives from `Module::State` and
// define the member function `create_state` to initialze one.
//
// Note that a module does not have any other state -- there is one
// instance of the `Module` object, even though a polyphonic or
// multitimbral synth may have many modules for each note.
//
// A module's constructor

// `Module` is an abstract base class for modules.
class Module {

public:

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

    // XXX Should thie be `State`'s emplace constructor?
    virtual State *create_state() const { return nullptr; }

    // XXX what was I thinking?
    // virtual void init_note_state(State *) const {}

    virtual void render(State *, size_t frame_count) const = 0;

protected:

    // Abstract base class.  Must subclass to use.
    Module() {}
    virtual ~Module() {}

    template <typename... Types>
    void ports(Port& p, Types&... rest)
    {
        m_ports.push_back(&p);
        ports(rest...);
    }

    void ports() {}

private:

    std::string m_name;
    std::vector<Port *> m_ports;

    // Modules cannot be copied, assigned, or moved.
    Module(const Module&) = delete;
    Module& operator = (const Module&) = delete;

};

std::string module_name(const Module& m)
{
    return m.name().empty() ? type_name(m) : m.name();
}

// -- Actions  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// XXX write something

class Action {

public:

    virtual void do_it() = 0;

    virtual std::string repr() const
    {
        return "Action";
    }

protected:

    Action() {}
    virtual ~Action() {}

};

class Render : public Action {

public:

    Render(const Module *module)
        : m_module(module)
    {}

    virtual void do_it()
    {
        assert(false && "not implemented");
    }

    virtual std::string repr() const
    {
        return "Render(" + module_name(*m_module) + ")";
    }

private:

    const Module *m_module;

};

class Copy : public Action {

public:

    Copy(Link *link)
        : m_link(link)
    {}

    virtual void do_it()
    {
        assert(false && "not implemented");
        if (m_link)             // -Wunused-private-field
            ;
    }

    virtual std::string repr() const
    {
        auto src = m_link->key().first;
        auto dest = m_link->key().second;
        return "Copy(" + port_name(*src) + ", " + port_name(*dest) + ")";
    }

private:

    Link *m_link;

};

class Clear : public Action {

public:

    Clear(const Module *module, InputPort *port)
        : m_module(module),
          m_port(port)
    {}

    virtual void do_it()
    {
        assert(false && "not implemented");
        if (m_module || m_port) // -Wunused-private-field
            ;
    }

    virtual std::string repr() const
    {
        return "Clear(" +
               module_name(*m_module) +
               "." +
               port_name(*m_port) +
               ")";
    }

private:

    const Module *m_module;
    InputPort *m_port;

};

// -- SignalGraph -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// XXX write something

// XXX would this be better implemented as an adjacency matrix?
class SignalGraph {

public:

    SignalGraph& module(const Module& mod)
    {
        m_modules.push_back(&mod);
        for (auto p : mod.ports()) {
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

    // Create an anonymous like, add it to the graph, and
    // retain ownership.
    SignalGraph& connect(OutputPort& src, InputPort& dest)
    {
        auto link = make_link(src, dest);
        m_links.emplace_back(link);
        return connection(link);
    }

    // Add a non-owned link to the graph.
    SignalGraph& connection(Link *link)
    {
        auto key = link->key();
        auto src = key.first;
        auto dest = key.second;

        if (dynamic_cast<Control *>(dest)) {
            // can't have two links between the same src and control dest.
            assert(m_link_map.find(key) == m_link_map.end());
        } else {
            // can't have two links to the same signal dest.
            assert(!any_of(m_link_map.begin(),
                           m_link_map.end(),
                           [=] (std::pair<Link::Key, Link *> item) {
                               return item.first.second == dest;
                           }
                       ));
        }

        m_link_map[key] = link;
        m_port_destinations[src].push_back(dest);
        m_port_sources[dest].push_back(src);
        return *this;
    }

    SignalGraph& disconnect(OutputPort& src, InputPort& dest)
    {
        // Get link position in m_link_map.
        Link::Key key = std::make_pair(&src, &dest);
        auto link_map_pos = m_link_map.find(key);

        // Verify src and dest are connected.
        assert(link_map_pos != m_link_map.end());

        // Get the link.
        Link *link = link_map_pos->second;

        // Remove from m_link_map.
        m_link_map.erase(link_map_pos);

        // Remove dest from m_port_destinations.
        auto& dests = m_port_destinations.at(&src);
        dests.erase(std::find(dests.begin(), dests.end(), &dest));

        // Remove src from m_port_sources.
        auto& sources = m_port_sources.at(&dest);
        sources.erase(std::find(sources.begin(), sources.end(), &src));

        // If link is in m_links, remove it.
        auto links_pos = std::find_if(m_links.begin(),
                                      m_links.end(),
                                      [=](std::unique_ptr<Link>& p) {
                                          return p.get() == link;
                                      });
        if (links_pos != m_links.end())
            m_links.erase(links_pos);

        return *this;
    }

    // XXX actions allocated here are leaked.
    typedef std::vector<Action *> Order;
    Order plan()
    {
        typedef std::set<const Module *> ModuleSet;
        Order order;
        ModuleSet not_done(m_modules.begin(), m_modules.end());

        auto is_active = [&] (OutputPort *src, InputPort *dest) {
            auto key = std::make_pair(src, dest);
            return m_link_map.at(key)->is_active();
        };

        auto is_ready = [&] (const Module *m) {
            for (auto dest : m_module_inputs[m]) {
                for (auto src : m_port_sources[dest]) {
                    if (is_active(src, dest)) {
                        if (not_done.find(m_port_modules[src]) !=
                            not_done.end())
                        {
                            // module has an input that is connected
                            // through an active link to an output of
                            // a module that is not done.  Whew!
                            return false;
                        }
                    }
                }
            }
            return true;
        };

        while (not not_done.empty()) {
            ModuleSet ready;
            std::copy_if(not_done.begin(),
                         not_done.end(),
                         std::inserter(ready, ready.begin()),
                         is_ready);

            // {
            //     std::cout << "not_done = {";
            //     const char *sep = "";
            //     for (auto m : not_done) {
            //         std::cout << sep << module_name(*m);
            //         sep = " ";
            //     }
            //     std::cout << "}" << std::endl;
            // }

            {
                std::cout << "ready = {";
                const char *sep = "";
                for (auto m : ready) {
                    std::cout << sep << module_name(*m);
                    sep = " ";
                }
                std::cout << "}" << std::endl;
            }

            assert(!ready.empty() && "graph is cyclic");

            // Emit actions to clear all unconnected inputs to
            // ready modules.
            for (auto mod : ready) {
                for (auto dest : m_module_inputs[mod]) {
                    bool any_active = false;
                    for (auto src : m_port_sources[dest]) {
                        if (m_link_map[std::make_pair(src, dest)]->is_active())
                            any_active = true;
                    }
                    if (!any_active)
                        order.push_back(new Clear(mod, dest));
                }
            }

            // Emit actions to render all ready modules
            for (auto mod : ready) {
                order.push_back(new Render(mod));
            }

            // Emit actions to copy all ready modules' outputs
            // to control links.
            for (auto src_mod : ready) {
                for (auto out : m_module_outputs[src_mod]) {
                    for (auto dest : m_port_destinations[out]) {
                        auto link = m_link_map[std::make_pair(out, dest)];
                        if (dynamic_cast<ControlLink *>(link) &&
                            link->is_active())
                        {
                            order.push_back(new Copy(link));
                        }
                    }
                }
            }

//      //      //      //      //      //      //      //      //      //

            // The following three alteranatives all subtract `ready`
            // from `not_done` (set difference).  They attempt to be
            // equivalent to the Python statement,
            //
            //     not_done -= ready
            //
            // None of them are good.
#if 1
            // Runs but uses extra space.
            ModuleSet tmp;
            std::set_difference(not_done.begin(), not_done.end(),
                                ready.begin(), ready.end(),
                                std::inserter(tmp, tmp.begin()));
            std::swap(tmp, not_done);
#elif 0
            // Doesn't compile.
            auto new_end = std::remove_if(not_done.begin(), not_done.end(),
                                          [&] (const Module *m) -> bool {
                                              return ready.find(m) !=
                                                     ready.end();
                                          });
            not_done.erase(new_end, not_done.end());
#else
            // Segfaults.
            for (auto i = not_done.begin(), j = ready.begin();
                 i != not_done.end();
                 )
            {
                std::cout << "    not_done = [";
                const char *sep = "";
                for (auto m : not_done) {
                    std::cout << sep << module_name(*m);
                    sep = " ";
                }
                std::cout << "]" << std::endl;

                if (j != ready.end() && *i == *j) {
                    not_done.erase(i);
                    j++;
                } else {
                    i++;
                }
            }
#endif
        }
        return order;
    }

    // // XXX make private
    // bool is_active(OutputPort *src, InputPort *dest) const
    // {
    //     auto key = std::make_pair(src, dest);
    //     return m_link_map.at(key)->is_active();
    // }

    void dump_maps() const
    {
        std::cout << "m_modules = [" << std::endl;
        for (auto i : m_modules)
            std::cout << "    " << module_name(*i) << "," << std::endl;
        std::cout << "]\n" << std::endl;

        std::cout << "m_port_modules = {" << std::endl;
        for (auto i : m_port_modules)
            std::cout << "    "
                      << i.first->name()
                      << ": "
                      << module_name(*i.second)
                      << ","
                      << std::endl;
        std::cout << "}\n" << std::endl;

        std::cout << "m_module_inputs = {" << std::endl;
        for (auto mod : m_modules) {
            std::cout << "    " << module_name(*mod) << ": [";
            const char *sep = "";
            for (auto p : m_module_inputs.at(mod)) {
                std::cout << sep << port_name(*p);
                sep = ", ";
            }
            std::cout << "]," << std::endl;
        }
        std::cout << "}\n" << std::endl;

        std::cout << "m_module_outputs = {" << std::endl;
        for (auto mod : m_modules) {
            std::cout << "    " << module_name(*mod) << ": [";
            const char *sep = "";
            for (auto p : m_module_outputs.at(mod)) {
                std::cout << sep << port_name(*p);
                sep = ", ";
            }
            std::cout << "]," << std::endl;
        }
        std::cout << "}\n" << std::endl;

        std::cout << "m_port_destinations = {" << std::endl;
        for (auto i : m_port_destinations) {
            std::cout << "    " << fqpn(*i.first) << ": [";
            const char *sep = "";
            for (auto p : i.second) {
                std::cout << sep << fqpn(*p);
                sep = ", ";
            }
            std::cout << "]," << std::endl;
        }
        std::cout << "}\n" << std::endl;

        std::cout << "m_port_sources = {" << std::endl;
        for (auto i : m_port_sources) {
            std::cout << "    " << fqpn(*i.first) << ": [";
            const char *sep = "";
            for (auto p : i.second) {
                std::cout << sep << fqpn(*p);
                sep = ", ";
            }
            std::cout << "]," << std::endl;
        }
        std::cout << "}\n" << std::endl;

        std::cout << "m_links = [" << std::endl;
        for (auto it = m_links.begin(); it != m_links.end(); it++) {
            auto key = it->get()->key();
            std::cout << "    ("
                      << fqpn(*key.first)
                      << ", "
                      << fqpn(*key.second)
                      << "),"
                      << std::endl;
        }
        std::cout << "]\n" << std::endl;

        std::cout << "m_link_map = {" << std::endl;
        for (auto i : m_link_map) {
            std::cout << "    ("
                      << fqpn(*i.first.first)
                      << ", "
                      << fqpn(*i.first.second)
                      << "): "
                      << type_name(*i.second)
                      << ","
                      << std::endl;
        }
        std::cout << "}\n" << std::endl;
    }

private:

    std::string fqpn(const Port& port) const
    {
        return module_name(*m_port_modules.at(&port)) + "." + port_name(port);
    }

    // XXX should these be unordered maps?

    // XXX should `m_link_map` be a vector?  Vectors are slower but use
    // memory predictably.
    std::vector<const Module *> m_modules;
    std::map<const Port *, const Module *> m_port_modules;
    std::map<const Module *, std::vector<InputPort *>> m_module_inputs;
    std::map<const Module *, std::vector<OutputPort *>> m_module_outputs;
    std::vector<std::unique_ptr<Link>> m_links;
    std::map<Link::Key, Link *> m_link_map;
    std::map<OutputPort *, std::vector<InputPort *>> m_port_destinations;
    std::map<InputPort *, std::vector<OutputPort *>> m_port_sources;

};

#endif /* !SIGNALGRAPH_included */
