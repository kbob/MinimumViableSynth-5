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

    const class Ported *owner() const
    {
        return m_owner;
    }

    void owner(const class Ported& owner)
    {
        m_owner = &owner;
    }

protected:

    // Abstract base class.  Must subclass to use.
    Port() : m_owner(nullptr) {}
    Port(const Port&) = default;
    Port& operator = (const Port&) = default;
    virtual ~Port() = default;

private:

    const class Ported *m_owner;

    std::string m_name;

};

// `InputPort` is an abstract base class for input ports.
class InputPort : public Port {

protected:

    // Abstract base class.  Must subclass to use.
    InputPort() = default;

};

// `OututPort` is an abstract base class for output ports.
class OutputPort : public Port {

protected:

    // Abstract base class.  Must subclass to use.
    OutputPort() = default;

};

// `Input<T>` is a signal input port.
template <class ElementType = DEFAULT_SAMPLE_TYPE>
class Input : public InputPort {

public:

    ElementType operator [] (size_t i) const;
    ElementType *buf() { return m_buf; }

private:

    ElementType m_buf[MAX_FRAMES];

};

// `Output<T>` is a signal output port.
template <class ElementType = DEFAULT_SAMPLE_TYPE>
class Output : public OutputPort {

public:

    ElementType& operator [] (size_t i) const;

};

// It is useful to define the null output type -- see links.h.
// But it is not useful to instantiate it.
template <>
class Output<void> : public OutputPort {
private:
    Output() = delete;
};

#endif /* !PORTS_included */
