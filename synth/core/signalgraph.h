#ifndef SIGNALGRAPH_included
#define SIGNALGRAPH_included

#include <algorithm>
#include <array>
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

#include "synth/util/noalloc.h"
#include "synth/util/set-bits.h"
#include "synth/util/span-map.h"
#include "synth/util/vec-map.h"

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
// will be many.)
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

    const class XXX_Module& module() const { return *m_module; }
    void module(const class XXX_Module& module) { m_module = &module; }

protected:

    // Abstract base class.  Must subclass to use.
    Port() {}
    Port(const Port&) = default;
    Port& operator = (const Port&) = delete;
    virtual ~Port() = default;

private:

    const class XXX_Module *m_module;

    std::string m_name;

};

inline std::string port_name(const Port& p)
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
class XXX_Module {

public:

    static const size_t MAX_PORTS = 8;
    typedef fixed_vector<Port *, MAX_PORTS> port_vector;

    void name(const std::string& name)
    {
        m_name = name;
    }

    const std::string& name() const
    {
        return m_name;
    }

    const port_vector& ports() const
    {
        return m_ports;
    }

    virtual void render(size_t frame_count) const = 0;

    virtual XXX_Module *clone() const = 0;

protected:

    // Abstract base class.  Must subclass to use.
    // Module() {}
    // Module(const Module&) = delete;
    // Module& operator = (const Module&) = delete;
    virtual ~XXX_Module() = default;

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
    //std::vector<Port *> m_ports;
    port_vector m_ports;

};

inline std::string module_name(const XXX_Module& m)
{
    return m.name().empty() ? type_name(m) : m.name();
}

inline std::string fqpn(const Port& p)
{
    return module_name(p.module()) + "." + port_name(p);
}


// -- Controls -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
// Controls are an abstraction for any input to a synth.  A control
// can be mapped to a MIDI event or a GUI parameter.  That is all
// I know at this point.
//
// XXX TBD

class Control {};


// -- Links -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
// Modules' ports are connected by Links.  (Aka connections, but we
// call them links here.)  A link is like a patch cord -- it connects
// modules together.
//
// Simple links pass data through unmodified.  Control links can
// perform several modifications on their data.  Control links can
// also be summed, several into a single input.
//
// Note the tricky terminology.  A module's input (`InputPort`) is
// a link's destination (`dest`), and a module's output (`OutputPort`)
// is a link's source (`src`).  Modules, meanwhile, use "predecessor"
// and "successor" to descrive their relationships.  The code tries hard
// to use those terms consistently:
//
//    Input - port, input to a module
//    Output - port, output from a module
//    Source - link, output from a module
//    Destination - link, input to a module
//    Predecessor - module, upstream from its successors
//    Successor - module, downstream
//
// Control links are inspired by the DLS level 2 specification's
// connection blocks.  Each control link has, in addition to its
// source and destination, an optional control, a scale, an input
// transform, and an output range.  This will probably evolve
// as more modules and synths are written.

class Link {

    typedef std::tuple<OutputPort *, InputPort *, Control *> key_base;

public:

    // XXX use a better name than "Key".
    class Key : public key_base {

    public:

        Key(OutputPort *src, InputPort *dest, Control *control = nullptr)
        : key_base(src, dest, control)
        {}

        OutputPort  *src() const { return std::get<0>(*this); }
        InputPort  *dest() const { return std::get<1>(*this); }
        Control *control() const { return std::get<2>(*this); }

    };

    virtual ~Link() = default;

    Key key() const
    {
        return Key(m_src, m_dest, m_control);
    }

protected:

    Link(OutputPort& src, InputPort& dest, Control *control = nullptr)
    : m_src(&src),
      m_dest(&dest),
      m_control(control)
    {}

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
                Control *control = nullptr,
                double intensity = DEFAULT_INTENSITY,
                bool enabled = DEFAULT_ENABLEMENT)
    : Link(src, dest, control),
      m_intensity(intensity),
      m_enabled(enabled)
    {}

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

inline Link *make_link(OutputPort& src, InputPort& dest)
{
    if (dynamic_cast<Controlled *>(&dest))
        return new ControlLink(src, dest);
    else
        return new SimpleLink(src, dest);
}


// -- Actions  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// XXX write something

class Action {

public:

    virtual ~Action() = default;

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

    Render(const XXX_Module *module)
    : m_module(module)
    {}

    virtual void do_it() override
    {
        assert(false && "not implemented");
    }

    virtual std::string repr() const override
    {
        return "Render(" + module_name(*m_module) + ")";
    }

private:

    const XXX_Module *m_module;

};

class Copy : public Action {

public:

    Copy(const Link *link)
    : m_link(link)
    {}

    virtual void do_it() override
    {
        assert(false && "not implemented");
    }

    virtual std::string repr() const override
    {
        auto key = m_link->key();
        auto src = key.src();
        auto dest = key.dest();
        return "Copy(" + fqpn(*src) + ", " + fqpn(*dest) + ")";
    }

private:

    const Link *m_link;

};

class Add : public Action {

public:

    Add(const Link *link)
    : m_link(link)
    {}

    virtual void do_it() override
    {
        assert (false && "not implemented");
    }

    virtual std::string repr() const override
    {
        auto key = m_link->key();
        return "Add(" + fqpn(*key.src()) + ", " + fqpn(*key.dest()) + ")";
    }

private:

    const Link *m_link;

};

class Clear : public Action {

public:

    Clear(InputPort *port)
    : m_port(port)
    {}

    virtual void do_it() override
    {
        assert(false && "not implemented");
    }

    virtual std::string repr() const override
    {
        return "Clear(" + fqpn(*m_port) + ")";
    }

private:

    InputPort *m_port;

};

class Alias : public Action {

public:

    Alias(const Link *link)
    : m_link(link)
    {}

    virtual void do_it() override
    {
        assert(false && "not implemented");
    }

    virtual std::string repr() const override
    {
        auto key = m_link->key();
        return "Alias(" + fqpn(*key.src()) + ", " + fqpn(*key.dest()) + ")";
    }

private:

    const Link *m_link;

};


// -- Plan  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// XXX write something

class XXX_Plan {

public:

    typedef std::vector<std::unique_ptr<Action>> action_sequence;

    XXX_Plan()                         = default;
    XXX_Plan(const XXX_Plan&)              = delete;
    XXX_Plan(XXX_Plan&&)                   = default;
    XXX_Plan& operator = (const XXX_Plan&) = delete;
    XXX_Plan& operator = (XXX_Plan&&)      = default;

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
        m_order.emplace_back(action);
    }

private:

    action_sequence m_prep;
    action_sequence m_order;

};

// -- SignalGraph -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// XXX write something

class SignalGraph {

public:

    // These can be increased by switching to uint64_t.
    // And the ports can be separated into separate input
    // port indices and output port indices.
    //
    // After that, it'll be necessary to write a multi-precision
    // bitmap class.  `std::vector<bool>` doesn't cut it, because
    // it does not have vector-level union/intersection/difference
    // operations.
    typedef std::uint32_t module_mask;
    typedef std::uint32_t port_mask;
    static const size_t MAX_MODULES = std::numeric_limits<module_mask>::digits;
    static const size_t MAX_PORTS = std::numeric_limits<port_mask>::digits;

    SignalGraph& module(const XXX_Module& mod)
    {
        assert(m_modules.size() < MAX_MODULES);
        m_modules.push_back(&mod);
        return *this;
    }

    // Create an anonymous link, add it to the graph, and
    // retain ownership.
    SignalGraph& connect(OutputPort& src, InputPort& dest)
    {
        auto link = make_link(src, dest);
        m_owned_links.emplace_back(link);
        return add_connection(link);
    }

    // Add a non-owned link to the graph.
    SignalGraph& add_connection(const Link *link)
    {
#ifndef NDEBUG
        auto key = link->key();
        auto src = key.src();
        auto dest = key.dest();

        // Verify source and dest modules are in m_modules.
        assert(module_index(&src->module()) != -1);
        assert(module_index(&dest->module()) != -1);

        if (dynamic_cast<Controlled *>(dest)) {
            // can't have two links with the same key.
            assert(std::find_if(m_links.begin(),
                                m_links.end(),
                                [&] (const Link *& l) {
                                    return l->key() == key;
                                }) == m_links.end());
        } else {
            // can't have two links to the same signal dest.
            assert(std::none_of(m_links.begin(),
                                m_links.end(),
                                [&] (const Link *& l) {
                                    return l->key().dest() == dest;
                                }));
        }
#endif
        m_links.push_back(link);
        return *this;
    }

    SignalGraph& disconnect(OutputPort& src,
                            InputPort& dest,
                            Control *control = nullptr)
    {
        auto key = Link::Key(&src, &dest, control);

        // Find in m_links.
        auto links_pos = std::find_if(m_links.begin(),
                                      m_links.end(),
                                      [&](const Link *& p) {
                                          return p->key() == key;
                                      });
        assert(links_pos != m_links.end());
        const Link *link = *links_pos;

        // If link is in m_owned_links, remove and delete it.
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

    XXX_Plan make_plan()
    {
        const size_t n_modules = m_modules.size();
        const size_t n_ports = count_ports();
        assert(n_modules <= MAX_MODULES);
        assert(n_ports <= MAX_PORTS);

        // `module_predecessors` is an adjacency matrix
        // for modules.   The i'th row or column corresponds
        // to m_modules[i].
        std::array<module_mask, MAX_MODULES> module_predecessors;
        module_predecessors.fill(0);

        // `ports` maps port indices to Ports.  port indices
        // are used in `port_sources`.
        std::array<const Port *, MAX_PORTS> ports;
        ports.fill(nullptr);
        size_t port_idx = 0;
        for (const auto *m : m_modules) {
            for (const auto *p : m->ports()) {
                assert(port_idx < MAX_PORTS);
                ports[port_idx++] = p;
            }
        }

        // `port_index(p)` maps Port pointers to port indices.
        auto port_index = [&] (const Port *p) -> ssize_t {
            for (size_t i = 0; i < n_ports; i++) {
                if (ports[i] == p)
                    return i;
            }
            return -1;
        };

        // `port_sources` is an adjacency matrix for ports.
        // When `port_sources[i] & 1 << j`, there is a link
        // from the i'th to the j'th port.
        std::array<port_mask, MAX_PORTS> port_sources;
        port_sources.fill(0);

        // iterate through the links and fill in both
        // `module_predecessors` and `port_sources`.
        for (const auto& link : m_links) {
            auto key = link->key();
            auto src = key.src();
            auto dest = key.dest();
            auto pred_index = module_index(&src->module());
            auto succ_index = module_index(&dest->module());
            assert(pred_index != -1 && succ_index != -1);
            module_predecessors[succ_index] |= 1 << pred_index;
            auto src_index = port_index(src);
            auto dest_index = port_index(dest);
            assert(src_index != -1 && dest_index != -1);
            port_sources[dest_index] |= 1 << src_index;
        }

        auto plan = XXX_Plan();

        // Load necessary prep work into `plan.prep`.
        // Unconnected ports are cleared; simply-connected
        // ports are aliased so there is no data copying.
        //
        //     for mod in modules:
        //         for dest in mod inputs:
        //             if no links to dest: gen Clear action
        //             if one simple link to dest: gen Alias action
        //
        for (const auto *mod : m_modules) {
            for (auto *port : mod->ports()) {
                InputPort *dest = dynamic_cast<InputPort *>(port);
                if (!dest)
                    continue;
                size_t link_count = 0;
                const Link *a_link = nullptr;
                for (const auto *link : m_links) {
                    if (link->key().dest() == dest) {
                        link_count++;
                        a_link = link;
                    }
                }
                if (link_count == 0) {
                    plan.push_back_prep(new Clear(dest));
                } else if (link_count == 1 &&
                           !dynamic_cast<const ControlLink *>(a_link)) {
                    plan.push_back_prep(new Alias(a_link));
                }
            }
        }

        // Generate the rendering actions.
        // Modules are rendered after the modules they depend on,
        // and after their inputs are computed.
        //
        //     done = {}
        //     while done != {all}:
        //         ready = {modules whose predecessors are done}
        //         create actions for ready modules
        //         done |= ready
        //
        module_mask done_mask = 0;
        const module_mask all_done_mask = (1 << n_modules) - 1;
        while (done_mask != all_done_mask) {

            // Collect all modules ready to process.
            module_mask ready_mask = 0;
            for (size_t i = 0; i < n_modules; i++) {
                if ((module_predecessors[i] & ~done_mask) == 0) {
                    ready_mask |= 1 << i;
                }
            }
            ready_mask &= ~done_mask;
            assert(ready_mask && "cycle in signal graph");

            // for mod in ready modules:
            //     for dest in mod inputs:
            //         for link in links to dest:
            //             copy first link; add other links
            //     render mod
            for (size_t i = 0; i < n_modules; i++) {
                if (!(ready_mask & 1 << i))
                    continue;
                const XXX_Module *mod = m_modules[i];
                for (auto *port : mod->ports()) {
                    InputPort *dest = dynamic_cast<InputPort *>(port);
                    if (!dest)
                        continue;
                    bool copied = false;
                    for (const auto *link : m_links) {
                        if (link->key().dest() != dest)
                            continue;
                        if (!dynamic_cast<const ControlLink *>(link))
                            continue;
                        Action *act;
                        if (!copied) {
                            act = new Copy(link);
                            copied = true;
                        } else {
                            act = new Add(link);
                        }
                        plan.push_back_order(act);
                    }
                }
                auto render = new Render(mod);
                plan.push_back_order(render);
            }

            // This set of modules is now done.
            done_mask |= ready_mask;
        }

        return plan;
    }

    void dump_maps() const
    {
        std::cout << "m_modules = [" << std::endl;
        for (const auto *mod : m_modules)
            std::cout << "    " << module_name(*mod) << "," << std::endl;
        std::cout << "]\n" << std::endl;

        std::cout << "m_links = [" << std::endl;
        for (const auto *link : m_links) {
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

    // total number of ports in the graph.
    size_t count_ports()
    {
        size_t count = 0;
        for (const auto *m : m_modules) {
            count += m->ports().size();
        }
        return count;
    }

    // map module pointer to index in m_modules
    // (and in the adjacency matrix).
    ssize_t module_index(const XXX_Module *m)
    {
        for (size_t i = 0; i < m_modules.size(); i++)
            if (m_modules.at(i) == m)
                return i;
        return -1;
    };

    // all modules (vertices) in the graph
    std::vector<const XXX_Module *> m_modules;

    // all links (edges) in the graph.
    std::vector<const Link *> m_links;

    // some links are anonymous and owned by the SignalGraph.
    // Store those here so they will be deleted with the graph.
    std::vector<std::unique_ptr<Link>> m_owned_links;

};

#endif /* !SIGNALGRAPH_included */
