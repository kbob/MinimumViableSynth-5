#ifndef LINK_included
#define LINK_included

#include <cassert>
#include <cstddef>
#include <functional>

#include "synth/core/action.h"
#include "synth/core/config.h"
#include "synth/core/controls.h"
#include "synth/core/ports.h"

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

    template <class D, class S, class C>
    Link(Input<D> *dest,
         Output<S> *src,
         Output<C> *ctl,
         SCALE_TYPE scale = DEFAULT_SCALE)
    : m_dest{dest}, m_src{src}, m_ctl{ctl}, m_scale{scale}
    {
        if (m_scale == DEFAULT_SCALE) {
            m_make_copy =
                [=] (InputPort *dest, OutputPort *src, OutputPort *ctl)
                    -> render_action {
                assert(dest && dynamic_cast<Input<D> *>(dest));
                assert(src && dynamic_cast<Output<S> *>(src));
                assert(ctl && dynamic_cast<Output<C> *>(ctl));
                auto dest_buf = static_cast<Input<D> *>(dest)->buf();
                auto src_buf = static_cast<Output<S> *>(src)->buf();
                auto ctl_buf = static_cast<Output<C> *>(ctl)->buf();
                return [=] (size_t frame_count) {
                    for (size_t i = 0; i < frame_count; i++)
                        dest_buf[i] = src_buf[i] * ctl_buf[i];
                };
            };
            m_make_add =
                [=] (InputPort *dest, OutputPort *src, OutputPort *ctl)
                    -> render_action {
                assert(dest && dynamic_cast<Input<D> *>(dest));
                assert(src && dynamic_cast<Output<S> *>(src));
                assert(ctl && dynamic_cast<Output<C> *>(ctl));
                auto dest_buf = static_cast<Input<D> *>(dest)->buf();
                auto src_buf = static_cast<Output<S> *>(src)->buf();
                auto ctl_buf = static_cast<Output<C> *>(ctl)->buf();
                return [=] (size_t frame_count) {
                    for (size_t i = 0; i < frame_count; i++)
                        dest_buf[i] += src_buf[i] * ctl_buf[i];
                };
            };
        } else {
            m_make_copy =
                [=] (InputPort *dest, OutputPort *src, OutputPort *ctl)
                    -> render_action {
                assert(dest && dynamic_cast<Input<D> *>(dest));
                assert(src && dynamic_cast<Output<S> *>(src));
                assert(ctl && dynamic_cast<Output<C> *>(ctl));
                auto dest_buf = static_cast<Input<D> *>(dest)->buf();
                auto src_buf = static_cast<Output<S> *>(src)->buf();
                auto ctl_buf = static_cast<Output<C> *>(ctl)->buf();
                auto scale = m_scale;
                return [=] (size_t frame_count) {
                    for (size_t i = 0; i < frame_count; i++)
                        dest_buf[i] = src_buf[i] * ctl_buf[i] * scale;
                };
            };
            m_make_add =
                [=] (InputPort *dest, OutputPort *src, OutputPort *ctl)
                    -> render_action {
                assert(dest && dynamic_cast<Input<D> *>(dest));
                assert(src && dynamic_cast<Output<S> *>(src));
                assert(ctl && dynamic_cast<Output<C> *>(ctl));
                auto dest_buf = static_cast<Input<D> *>(dest)->buf();
                auto src_buf = static_cast<Output<S> *>(src)->buf();
                auto ctl_buf = static_cast<Output<C> *>(ctl)->buf();
                auto scale = m_scale;
                return [=] (size_t frame_count) {
                    for (size_t i = 0; i < frame_count; i++)
                        dest_buf[i] += src_buf[i] * ctl_buf[i] * scale;
                };
            };
        }
    }

    template <class D, class S>
    Link(Input<D> *dest,
         Output<S> *src,
         std::nullptr_t,
         SCALE_TYPE scale = DEFAULT_SCALE)
    : m_dest{dest}, m_src{src}, m_ctl{nullptr}, m_scale{scale}
    {
        if (m_scale == DEFAULT_SCALE) {
            m_make_copy =
                [=] (InputPort *dest, OutputPort *src, OutputPort *)
                    -> render_action {
                assert(dest && dynamic_cast<Input<D> *>(dest));
                assert(src && dynamic_cast<Output<S> *>(src));
                auto dest_buf = static_cast<Input<D> *>(dest)->buf();
                auto src_buf = static_cast<Output<S> *>(src)->buf();
                return [=] (size_t frame_count) {
                    for (size_t i = 0; i < frame_count; i++)
                        dest_buf[i] = src_buf[i];
                };
            };
            m_make_add =
                [=] (InputPort *dest, OutputPort *src, OutputPort *)
                    -> render_action {
                assert(dest && dynamic_cast<Input<D> *>(dest));
                assert(src && dynamic_cast<Output<S> *>(src));
                auto dest_buf = static_cast<Input<D> *>(dest)->buf();
                auto src_buf = static_cast<Output<S> *>(src)->buf();
                return [=] (size_t frame_count) {
                    for (size_t i = 0; i < frame_count; i++)
                        dest_buf[i] += src_buf[i];
                };
            };
        } else {
            m_make_copy =
                [=] (InputPort *dest, OutputPort *src, OutputPort *)
                    -> render_action {
                assert(dest && dynamic_cast<Input<D> *>(dest));
                assert(src && dynamic_cast<Output<S> *>(src));
                auto dest_buf = static_cast<Input<D> *>(dest)->buf();
                auto src_buf = static_cast<Output<S> *>(src)->buf();
                auto scale = m_scale;
                return [=] (size_t frame_count) {
                    for (size_t i = 0; i < frame_count; i++)
                        dest_buf[i] = src_buf[i] * scale;
                };
            };
            m_make_add =
                [=] (InputPort *dest, OutputPort *src, OutputPort *)
                    -> render_action {
                assert(dest && dynamic_cast<Input<D> *>(dest));
                assert(src && dynamic_cast<Output<S> *>(src));
                auto dest_buf = static_cast<Input<D> *>(dest)->buf();
                auto src_buf = static_cast<Output<S> *>(src)->buf();
                auto scale = m_scale;
                return [=] (size_t frame_count) {
                    for (size_t i = 0; i < frame_count; i++)
                        dest_buf[i] += src_buf[i] * scale;
                };
            };
        }
    }

    template <class D, class C>
    Link(Input<D> *dest,
         std::nullptr_t,
         Output<C> *ctl,
         SCALE_TYPE scale = DEFAULT_SCALE)
         : m_dest{dest}, m_src{nullptr}, m_ctl{ctl}, m_scale{scale}
    {
        if (m_scale == DEFAULT_SCALE) {
            m_make_copy =
                [=] (InputPort *dest, OutputPort *, OutputPort *ctl)
                    -> render_action {
                assert(dest && dynamic_cast<Input<D> *>(dest));
                assert(ctl && dynamic_cast<Output<C> *>(ctl));
                auto dest_buf = static_cast<Input<D> *>(dest)->buf();
                auto ctl_buf = static_cast<Output<C> *>(ctl)->buf();
                return [=] (size_t frame_count) {
                    for (size_t i = 0; i < frame_count; i++)
                        dest_buf[i] = ctl_buf[i];
                };
            };
            m_make_add =
                [=] (InputPort *dest, OutputPort *, OutputPort *ctl)
                    -> render_action {
                assert(dest && dynamic_cast<Input<D> *>(dest));
                assert(ctl && dynamic_cast<Output<C> *>(ctl));
                auto dest_buf = static_cast<Input<D> *>(dest)->buf();
                auto ctl_buf = static_cast<Output<C> *>(ctl)->buf();
                return [=] (size_t frame_count) {
                    for (size_t i = 0; i < frame_count; i++)
                        dest_buf[i] += ctl_buf[i];
                };
            };
        } else {
            m_make_copy =
                [=] (InputPort *dest, OutputPort *, OutputPort *ctl)
                    -> render_action {
                assert(dest && dynamic_cast<Input<D> *>(dest));
                assert(ctl && dynamic_cast<Output<C> *>(ctl));
                auto dest_buf = static_cast<Input<D> *>(dest)->buf();
                auto ctl_buf = static_cast<Output<C> *>(ctl)->buf();
                auto scale = m_scale;
                return [=] (size_t frame_count) {
                    for (size_t i = 0; i < frame_count; i++)
                        dest_buf[i] = ctl_buf[i] * scale;
                };
            };
            m_make_add =
                [=] (InputPort *dest, OutputPort *, OutputPort *ctl)
                    -> render_action {
                assert(dest && dynamic_cast<Input<D> *>(dest));
                assert(ctl && dynamic_cast<Output<C> *>(ctl));
                auto dest_buf = static_cast<Input<D> *>(dest)->buf();
                auto ctl_buf = static_cast<Output<C> *>(ctl)->buf();
                auto scale = m_scale;
                return [=] (size_t frame_count) {
                    for (size_t i = 0; i < frame_count; i++)
                        dest_buf[i] += ctl_buf[i] * scale;
                };
            };
        }
    }

    template <class D>
    Link(Input<D> *dest,
         std::nullptr_t,
         std::nullptr_t,
         SCALE_TYPE scale = DEFAULT_SCALE)
         : m_dest{dest}, m_src{nullptr}, m_ctl{nullptr}, m_scale{scale}
    {
        m_make_copy = [=] (InputPort *dest, OutputPort *, OutputPort *)
                          -> render_action
        {
            assert(dest && dynamic_cast<Input<D> *>(dest));
            auto *dest_buf = static_cast<Input<D> *>(dest)->buf();
            return [=] (size_t frame_count) {
                for (size_t i = 0; i < frame_count; i++)
                    dest_buf[i] = scale;
            };
        };
        m_make_add = [=] (InputPort *dest, OutputPort *, OutputPort *)
                          -> render_action
        {
            assert(dest && dynamic_cast<Input<D> *>(dest));
            auto *dest_buf = static_cast<Input<D> *>(dest)->buf();
            return [=] (size_t frame_count) {
                for (size_t i = 0; i < frame_count; i++)
                    dest_buf[i] += scale;
            };
        };
    }

    bool operator == (const Link& that) const
    {
        return this == &that;
    }

    bool is_simple() const
    {
        return m_src &&
              !m_ctl &&
               m_src->data_type() == m_dest->data_type() &&
               m_scale == 1.0f;
    }

    bool is_ctl_simple() const
    {
        return m_ctl &&
              !m_src &&
               m_ctl->data_type() == m_dest->data_type() &&
               m_scale == 1.0f;
    }

    InputPort  *dest() const { return m_dest; }
    OutputPort  *src() const { return m_src; }
    OutputPort  *ctl() const { return m_ctl; }
    SCALE_TYPE scale() const { return m_scale; }

    render_action make_copy_action(InputPort *dest,
                                   OutputPort *src,
                                   OutputPort *ctl) const
    {
        return m_make_copy(dest, src, ctl);
    }

    render_action make_add_action(InputPort *dest,
                                  OutputPort *src,
                                  OutputPort *ctl) const
    {
        return m_make_add(dest, src, ctl);
    }

private:

    typedef
    std::function<render_action(InputPort *, OutputPort *, OutputPort *)>
    action_maker;

    InputPort   *m_dest;
    OutputPort  *m_src;
    OutputPort  *m_ctl;
    SCALE_TYPE   m_scale;
    action_maker m_make_copy;
    action_maker m_make_add;

    friend class link_unit_test;

};

#endif /* !LINK_included */
