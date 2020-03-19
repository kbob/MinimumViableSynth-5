#ifndef SIGNALGRAPH_included
#define SIGNALGRAPH_included

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "synth/util/span-map.h"
#include "synth/util/vec-map.h"

const size_t  MAX_MODULES        = 32;
const size_t  MAX_PORTS          = 32;
const double  DEFAULT_RANGE      =  1.0;
const double  DEFAULT_INTENSITY  =  1.0;
const bool    DEFAULT_ENABLEMENT = false;
typedef float DEFAULT_SAMPLE_TYPE;


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

    const class Module& module() const { return *m_module; }
    void module(const class Module& module) { m_module = &module; }

protected:

    // Abstract base class.  Must subclass to use.
    Port() {}

    virtual ~Port() {}

private:

    const class Module *m_module;

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

// `Controlled` is a mixin.  It marks a port as a control port.  (See above.)
class Controlled {

protected:

    // Abstract base class.  Must subclass to use.
    Controlled() {}

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
class ControlInput : public Input<ElementType>, public Controlled {

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
class ControlOutput : public Output<ElementType>, public Controlled {};


// -- Controls -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// XXX TBD

class Control {};


// -- Links -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// XXX write something
// XXX discuss src/dest, input/output, predecessor/successor.
// XXX a link is like a patch cord.

class Link {

    typedef std::tuple<OutputPort *, InputPort *, Control *> key_base;

public:

    // XXX use a better name than "Key".
    // typedef std::pair<OutputPort *, InputPort *> Key;
    // typedef std::tuple<OutputPort *, InputPort *, Control *> Key;
    class Key : public key_base {

    public:

        Key(OutputPort *src, InputPort *dest, Control *control = nullptr)
        : key_base(src, dest, control)
        {}

        OutputPort  *src() const { return std::get<0>(*this); }
        InputPort  *dest() const { return std::get<1>(*this); }
        Control *control() const { return std::get<2>(*this); }

    };

    // Link(OutputPort& src, InputPort& dest)
    // : m_src(&src),
    //   m_dest(&dest)
    // {}
    //
    virtual ~Link() {}

    Key key() const
    {
        // return std::make_tuple(m_src, m_dest, m_control);
        return Key(m_src, m_dest, m_control);
    }

    // XXX deprecate this.
    virtual bool is_active() const
    {
        return true;            // signal links are always active.
    }

protected:

    Link(OutputPort& src, InputPort& dest, Control *control = nullptr)
    : m_src(&src),
      m_dest(&dest),
      m_control(control)
    {
        m_control = &*m_control;
    }

private:

    OutputPort *m_src;
    InputPort *m_dest;
    Control *m_control;
};

class SimpleLink : public Link {

public:

    SimpleLink(OutputPort& src, InputPort& dest)
    : Link(src, dest)
    {}

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
    if (dynamic_cast<Controlled *>(&dest))
        return new ControlLink(src, dest);
    else
        return new SimpleLink(src, dest);
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
        p.module(*this);
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

std::string fqpn(const Port& p)
{
    return module_name(p.module()) + "." + port_name(p);
}


// -- Actions  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// XXX write something

class Action {

public:

    virtual ~Action() {}

    virtual void do_it() = 0;

    virtual std::string repr() const
    {
        return "Action";
    }

protected:

    Action() {}

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
    }

    virtual std::string repr() const
    {
        auto key = m_link->key();
        auto src = key.src();
        auto dest = key.dest();
        return "Copy(" + fqpn(*src) + ", " + fqpn(*dest) + ")";
    }

private:

    Link *m_link;

};

class Add : public Action {

public:

    Add(const Link *link)
    : m_link(link)
    {}

    virtual void do_it()
    {
        assert (false && "not implemented");
    }

    virtual std::string repr() const
    {
        auto key = m_link->key();
        return "Add(" + fqpn(*key.src()) + ", " + fqpn(*key.dest()) + ")";
    }

private:

    // const Module *m_src_mod;
    // const Module *m_dest_mod;
    const Link *m_link;

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

class Alias : public Action {

public:

    Alias(const Link *link)
    : m_link(link)
    {}

    virtual void do_it()
    {
        assert(false && "not implemented");
    }

    virtual std::string repr() const
    {
        auto key = m_link->key();
        return "Alias(" + fqpn(*key.src()) + ", " + fqpn(*key.dest()) + ")";
    }

private:

    const Link *m_link;

};


// -- Plan  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// XXX write something

class Plan {

public:

    typedef std::vector<std::unique_ptr<Action>> action_sequence;

    const action_sequence& prep() const
    {
        return m_prep;
    }

    const action_sequence& order() const
    {
        return m_order;
    }

    void push_back_prep(Action *action)
    {
        m_prep.emplace_back(action);
    }

    void push_back_order(Action *action)
    {
        m_prep.emplace_back(action);
    }


private:

    action_sequence m_prep;
    action_sequence m_order;

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

    // Create an anonymous link, add it to the graph, and
    // retain ownership.
    SignalGraph& connect(OutputPort& src, InputPort& dest)
    {
        auto link = make_link(src, dest);
        m_owned_links.emplace_back(link);
        return connection(link);
    }

    // Add a non-owned link to the graph.
    SignalGraph& connection(Link *link)
    {
        auto key = link->key();
        auto src = key.src();
        auto dest = key.dest();

        if (dynamic_cast<Controlled *>(dest)) {
            // can't have two links with the same key.
            assert(std::find_if(m_links.begin(),
                                m_links.end(),
                                [&] (Link *& l) {
                                    return l->key() == key;
                                }) == m_links.end());
        } else {
            // can't have two links to the same signal dest.
            assert(std::none_of(m_links.begin(),
                                m_links.end(),
                                [&] (Link *& l) {
                                    return l->key().dest() == dest;
                                }));
        }

        m_links.push_back(link);
        m_port_destinations[src].push_back(dest);
        m_port_sources[dest].push_back(src);
        return *this;
    }

    SignalGraph& disconnect(OutputPort& src,
                            InputPort& dest,
                            Control *control = nullptr)
    {
        auto key = Link::Key(&src, &dest, control);

        // Find in m_links;
        auto links_pos = std::find_if(m_links.begin(),
                                      m_links.end(),
                                      [&](Link *& p) {
                                          return p->key() == key;
                                      });
        assert(links_pos != m_links.end());
        Link *link = *links_pos;

        // Remove dest from m_port_destinations.
        auto& dests = m_port_destinations.at(&src);
        dests.erase(std::find(dests.begin(), dests.end(), &dest));

        // Remove src from m_port_sources.
        auto& sources = m_port_sources.at(&dest);
        sources.erase(std::find(sources.begin(), sources.end(), &src));

        // If link is in m_owned_links, remove it.
        auto owned_links_pos = std::find_if(m_owned_links.begin(),
                                            m_owned_links.end(),
                                            [&](std::unique_ptr<Link>& p) {
                                                return p.get() == link;
                                            });
        if (owned_links_pos != m_owned_links.end())
            m_owned_links.erase(owned_links_pos);

        // Remove from m_links.
        m_links.erase(links_pos);

        return *this;
    }

    // Plan new_plan()
    // {
    //     const size_t n_modules = m_modules.size();
    //     assert(n_modules <= MAX_MODULES);
    //     int32_t module_predecessor_mask = 0;
    //     span_map<Port *, Port *> port_sources;
    //     port_sources.reserve(MAX_PORTS);
    //     port_sources.values_reserve(MAX_PORTS);
    //
    //     for (auto it : self.link_map)
    // }

    // XXX actions allocated here are leaked.
    typedef std::vector<Action *> Order;
    Order plan()
    {
        typedef std::set<const Module *> ModuleSet;
        Order order;
        ModuleSet not_done(m_modules.begin(), m_modules.end());

        auto is_active = [&] (OutputPort *src, InputPort *dest) {
            auto match = [&] (Link *& l) {
                auto k = l->key();
                return k.src() == src && k.dest() == dest && l->is_active();
            };
            return std::any_of(m_links.begin(), m_links.end(), match);
        };

        auto is_ready = [&] (const Module *m) {
            for (auto dest : m_module_inputs[m]) {
                for (auto src : m_port_sources[dest]) {
                    if (is_active(src, dest)) {
                        if (not_done.find(&src->module()) != not_done.end()) {
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
                        if (is_active(src, dest))
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
                        auto match = [&] (Link *& l) {
                            auto k = l->key();
                            return k.src() == out && k.dest() == dest;
                        };
                        auto pos = std::find_if(m_links.begin(),
                                                m_links.end(),
                                                match);
                        assert(pos != m_links.end());
                        Link *link = *pos;
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
            // None of them are good.  I hate C++.
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

    void dump_maps() const
    {
        std::cout << "m_modules = [" << std::endl;
        for (auto i : m_modules)
            std::cout << "    " << module_name(*i) << "," << std::endl;
        std::cout << "]\n" << std::endl;

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
        for (auto link : m_links) {
            auto key = link->key();
            std::cout << "    "
                      << type_name(*link)
                      << "("
                      << fqpn(*key.src())
                      << ", "
                      << fqpn(*key.dest())
                      << "),"
                      << std::endl;
        }
        std::cout << "]\n" << std::endl;

        std::cout << "m_owned_links = [" << std::endl;
        for (auto it = m_owned_links.begin(); it != m_owned_links.end(); it++) {
            auto link = it->get();
            auto key = link->key();
            std::cout << "    "
                      << type_name(*link)
                      << "("
                      << fqpn(*key.src())
                      << ", "
                      << fqpn(*key.dest())
                      << "),"
                      << std::endl;
        }
        std::cout << "]\n" << std::endl;
    }

private:

    std::vector<const Module *> m_modules;
    std::map<const Module *, std::vector<InputPort *>> m_module_inputs;
    std::map<const Module *, std::vector<OutputPort *>> m_module_outputs;
    std::vector<Link *> m_links;
    std::vector<std::unique_ptr<Link>> m_owned_links;
    std::map<OutputPort *, std::vector<InputPort *>> m_port_destinations;
    std::map<InputPort *, std::vector<OutputPort *>> m_port_sources;

};

#endif /* !SIGNALGRAPH_included */
