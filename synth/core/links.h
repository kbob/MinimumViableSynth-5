#ifndef LINKS_included
#define LINKS_included

#include "synth/core/ports.h"

class Control;

const double  DEFAULT_SCALE =  1.0;
// typedef float DEFAULT_SAMPLE_TYPE;

// -- Links -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
// Modules' ports are connected by Links.  (Aka connections, but we
// call them links here.)  A link is like a patch cord -- it connects
// modules together via their ports.
//
// Simple links pass data through unmodified.  Control links can
// perform several modifications on their data or connect to outside
// sources such as MIDI messages or GUI controls.
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
                double scale = DEFAULT_SCALE)
    : Link(src, dest, control),
      m_scale(scale)
    {}
    ControlLink& scale(double scale)
    {
        m_scale = scale;
        return *this;
    }

    double scale() const
    {
        return m_scale;
    }

    // // We know src, dest, control, element type.
    // // XXX the conditional logic here should be folded into the action.
    // template <class ElementType>
    // void copy(size_t frame_count, ElementType *dest_buf, class Voice *voice)
    // {
    //     ElementType ctl[OutputPort::MAX_FRAMES];
    //     if (m_src && m_control) {
    //         const ElementType *src = m_src->get_buf();
    //         m_control->get(ctl, frame_count, voice);
    //         for (size_t i = 0; i < frame_count; i++)
    //             dest[i] = src[i] * ctl[i] * m_scale;
    //     } else if (m_src) {
    //         const ElementType *src = m_src->get_buf();
    //         for (size_t i = 0; i < frame_count; i++)
    //             dest[i] = src[i] * m_scale;
    //     } else if (m_control) {
    //         m_control->get(ctl, frame_count, voice);
    //         for (size_t i = 0; i < frame_count; i++)
    //             dest[i] = ctl[i] * m_scale;
    //     } else {
    //         for (size_t i = 0; i < frame_count; i++)
    //             dest[i] = m_scale;
    //     }
    // }

private:

    double m_scale;

};

#endif /* !LINKS_included */
