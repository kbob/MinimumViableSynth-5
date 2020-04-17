#ifndef LINKS_included
#define LINKS_included

#include <cstddef>

#include "synth/core/controls.h"
#include "synth/core/ports.h"

class Control;

const double  DEFAULT_SCALE =  1.0;
// typedef float DEFAULT_SAMPLE_TYPE;
typedef float SCALE_TYPE;

// -- Links -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
// XXX is any of this info current?
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

public:

    bool is_constant() const { return !m_src && !m_ctl; }
    bool is_simple() const
    {
        return m_src &&
              !m_ctl &&
               m_src->data_type() == m_dest->data_type() &&
               m_scale == 1.0f;
    }

    InputPort  *dest() const { return m_dest; }
    OutputPort  *src() const { return m_src; }
    OutputPort  *ctl() const { return m_ctl; }
    SCALE_TYPE scale() const { return m_scale; }

    virtual std::function<void(size_t)> make_copy_action() = 0;
    virtual std::function<void(size_t)> make_add_action() = 0;

protected:

    Link(InputPort *dest, OutputPort *src, OutputPort *ctl, SCALE_TYPE scale)
    : m_dest{dest}, m_src{src}, m_ctl{ctl}, m_scale{scale} {}
    virtual ~Link() = default;

    InputPort       *m_dest;
    OutputPort      *m_src;
    OutputPort      *m_ctl;
    const SCALE_TYPE m_scale;

};

template <class D, class S, class C>
class LinkType : public Link {

public:

    LinkType(Input<D> *dest, Output<S> *src, Output<C> *ctl, SCALE_TYPE scale)
    : Link(dest, src, ctl, scale) {}

    std::function<void(size_t)> make_copy_action() override
    {
        auto dest_buf = static_cast<Input<D> *>(m_dest)->buf();
        auto src_buf = m_src ? static_cast<Output<S> *>(m_src)->buf() : nullptr;
        auto ctl_buf = m_ctl ? static_cast<Output<C> *>(m_ctl)->buf() : nullptr;
        bool scaled = m_scale != 1.0f;
        float scale = m_scale;
        if (src_buf && ctl_buf && scaled)
            return [=] (size_t frame_count) {
                for (size_t i = 0; i < frame_count; i++)
                    dest_buf[i] = src_buf[i] * ctl_buf[i] * scale;
            };
        else if (src_buf && ctl_buf)
            return [=] (size_t frame_count) {
                for (size_t i = 0; i < frame_count; i++)
                    dest_buf[i] = src_buf[i] * ctl_buf[i];
            };
        else if (src_buf && scaled)
            return [=] (size_t frame_count) {
                for (size_t i = 0; i < frame_count; i++)
                    dest_buf[i] = src_buf[i] * ctl_buf[i];
            };
        else if (ctl_buf && scaled)
            return [=] (size_t frame_count) {
                for (size_t i = 0; i < frame_count; i++)
                    dest_buf[i] = ctl_buf[i] * scale;
            };
        else if (src_buf)
            return [=] (size_t frame_count) {
                for (size_t i = 0; i < frame_count; i++)
                    dest_buf[i] = src_buf[i];
            };
        else if (ctl_buf)
            return [=] (size_t frame_count) {
                for (size_t i = 0; i < frame_count; i++)
                    dest_buf[i] = ctl_buf[i];
            };
        else {
            // handle both scaled and unscaled cases here.
            return [=] (size_t frame_count) {
                for (size_t i = 0; i < frame_count; i++)
                    dest_buf[i] = scale;
            };
        }
    }

    std::function<void(size_t)> make_add_action() override
    {
        auto dest_buf = static_cast<Input<D> *>(m_dest)->buf();
        auto src_buf = m_src ? static_cast<Output<S> *>(m_src)->buf() : nullptr;
        auto ctl_buf = m_ctl ? static_cast<Output<C> *>(m_ctl)->buf() : nullptr;
        bool scaled = m_scale != 1.0f;
        float scale = m_scale;
        if (src_buf && ctl_buf && scaled)
            return [=] (size_t frame_count) {
                for (size_t i = 0; i < frame_count; i++)
                    dest_buf[i] += src_buf[i] * ctl_buf[i] * scale;
            };
        else if (src_buf && ctl_buf)
            return [=] (size_t frame_count) {
                for (size_t i = 0; i < frame_count; i++)
                    dest_buf[i] += src_buf[i] * ctl_buf[i];
            };
        else if (src_buf && scaled)
            return [=] (size_t frame_count) {
                for (size_t i = 0; i < frame_count; i++)
                    dest_buf[i] += src_buf[i] * ctl_buf[i];
            };
        else if (ctl_buf && scaled)
            return [=] (size_t frame_count) {
                for (size_t i = 0; i < frame_count; i++)
                    dest_buf[i] += ctl_buf[i] * scale;
            };
        else if (src_buf)
            return [=] (size_t frame_count) {
                for (size_t i = 0; i < frame_count; i++)
                    dest_buf[i] += src_buf[i];
            };
        else if (ctl_buf)
            return [=] (size_t frame_count) {
                for (size_t i = 0; i < frame_count; i++)
                    dest_buf[i] += ctl_buf[i];
            };
        else {
            // handle both scaled and unscaled cases here.
            return [=] (size_t frame_count) {
                for (size_t i = 0; i < frame_count; i++)
                    dest_buf[i] += scale;
            };
        }
    }

};

// There are six version of `make_link`.  The source may be present
// or null, and the control may be either a control, an output port,
// or null.
//
// When the control is a `Control`, we use its `out` member.
template <class D, class S, class C>
LinkType<D, S, C>
make_link(Input<D>       *dest,
          Output<S>      *src,
          ControlType<C> *ctl,
          SCALE_TYPE      scale = 1.0f)
{
    return LinkType<D, S, C>(dest, src, &ctl->out, scale);
}

template <class D, class S, class C>
LinkType<D, S, C>
make_link(Input<D>       *dest,
          Output<S>      *src,
          Output<C>      *ctl,
          SCALE_TYPE      scale = 1.0f)
{
    return LinkType<D, S, C>(dest, src, ctl, scale);
}

template <class D, class C>
LinkType<D, void, C>
make_link(Input<D>       *dest,
          std::nullptr_t,
          ControlType<C> *ctl,
          SCALE_TYPE      scale = 1.0f)
{
    return LinkType<D, void, C>(dest, nullptr, &ctl->out, scale);
}

template <class D, class C>
LinkType<D, void, C>
make_link(Input<D>       *dest,
          std::nullptr_t,
          Output<C>      *ctl,
          SCALE_TYPE      scale = 1.0f)
{
    return LinkType<D, void, C>(dest, nullptr, ctl, scale);
}

template <class D, class S>
LinkType<D, S, void>
make_link(Input<D>       *dest,
          Output<S>      *src,
          std::nullptr_t,
          SCALE_TYPE      scale = 1.0f)
{
    return LinkType<D, S, void>(dest, src, nullptr, scale);
}

template <class D>
LinkType<D, void, void>
make_link(Input<D>       *dest,
          std::nullptr_t,
          std::nullptr_t,
          SCALE_TYPE      scale = 1.0f)
{
    return LinkType<D, void, void>(dest, nullptr, nullptr, scale);
}

#endif /* !LINKS_included */
