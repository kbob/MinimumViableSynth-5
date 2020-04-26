#ifndef PORTS_included
#define PORTS_included

#include <cassert>
#include <string>
#include <typeindex>

#include "synth/core/config.h"


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

    class Ported *owner()
    {
        return m_owner;
    }

    void owner(class Ported& owner)
    {
        m_owner = &owner;
    }

    virtual std::type_index data_type() const = 0;

protected:

    // Abstract base class.  Must subclass to use.
    Port() : m_owner{nullptr} {}
    Port(const Port&) = default;
    Port& operator = (const Port&) = default;
    virtual ~Port() = default;

private:

    class Ported *m_owner;
    std::string m_name;

    friend class ports_unit_test;

};

// `InputPort` is an abstract base class for input ports.
class InputPort : public Port {

protected:

    // Abstract base class.  Must subclass to use.
    InputPort() = default;

};

// `OutputPort` is an abstract base class for output ports.
class OutputPort : public Port {

protected:

    // Abstract base class.  Must subclass to use.
    OutputPort() = default;

};

// `Input<T>` is a typed input port.
template <class ElementType = DEFAULT_SAMPLE_TYPE>
class Input : public InputPort {

public:

    Input()
    : m_data{m_buf}
    {}

    std::type_index data_type() const override { return typeid(ElementType); }

    void clear(const ElementType value)
    {
        m_data = m_buf;
        for (size_t i = 0; i < MAX_FRAMES; i++)
            m_buf[i] = value;
    }

    // Consumers read from m_data.
    // When this port is aliased to another, `m_data` points to the
    // other port's data.  When a port is not aliased, producers write
    // to `m_buf`, and `m_data` points there.
    void alias(const ElementType *data)
    {
        m_data = data ? data : m_buf;
    }

    ElementType operator [] (size_t i) const
    {
        assert(i < MAX_FRAMES);
        return m_data[i];
    }

    ElementType *buf() { return m_buf; }

private:

    const ElementType *m_data;
    ElementType m_buf[MAX_FRAMES];

    friend class ports_unit_test;

};

// `Output<T>` is a typed output port.
template <class ElementType = DEFAULT_SAMPLE_TYPE>
class Output : public OutputPort {

public:

    std::type_index data_type() const override { return typeid(ElementType); }

    ElementType& operator [] (size_t i)
    {
        assert(i < MAX_FRAMES);
        return m_buf[i];
    }

    const ElementType *buf() const { return m_buf; }

private:

    ElementType m_buf[MAX_FRAMES];

    friend class ports_unit_test;

};

#endif /* !PORTS_included */
