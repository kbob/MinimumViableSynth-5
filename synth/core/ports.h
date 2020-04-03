#ifndef PORTS_included
#define PORTS_included

#include <string>

typedef float DEFAULT_SAMPLE_TYPE;

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
// The concrete types are `Input<T>` and `Output<T>`.  `Port`,
// `InputPort`, and `OutputPort` are abstract bases.

// `Port` is an abstract base class for all ports.
class Port {

public:

    static const size_t MAX_FRAMES = 64;

    const std::string& name() const
    {
        return m_name;
    }

    Port& name(const std::string& name)
    {
        m_name = name;
        return *this;
    }

    const class Module *module() const
    {
        return m_module;
    }

    void module(const class Module& module)
    {
        m_module = &module;
    }

protected:

    // Abstract base class.  Must subclass to use.
    Port() : m_module(nullptr) {}
    Port(const Port&) = default;
    Port& operator = (const Port&) = delete;
    virtual ~Port() = default;

private:

    const class Module *m_module;

    std::string m_name;

};

// `InputPort` is an abstract base class for input ports.
class InputPort : public Port {

protected:

    // Abstract base class.  Must subclass to use.
    InputPort() {}

    // virtual void copy_in(size_t, const class Voice *) = 0;

};

// `OututPort` is an abstract base class for output ports.
class OutputPort : public Port {

protected:

    // Abstract base class.  Must subclass to use.
    OutputPort() {}

};

// `Input<T>` is a signal input port.
template <class ElementType = DEFAULT_SAMPLE_TYPE>
class Input : public InputPort {

public:

    ElementType operator [] (size_t i) const;
    ElementType *buf() { return m_buf; }

    // virtual void copy_in(size_t, ControlLink&, const class Voice& voice)
    // {
    //     link.copy<ElementType>(m_buf, frame_count, voice);
    // }

private:

    ElementType m_buf[MAX_FRAMES];
};

// `Output<T>` is a signal output port.
template <class ElementType = DEFAULT_SAMPLE_TYPE>
class Output : public OutputPort {

public:

    ElementType& operator [] (size_t i) const;

};


#endif /* !PORTS_included */
