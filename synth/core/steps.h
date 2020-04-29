#ifndef STEPS_included
#define STEPS_included

#include <cassert>
#include <iostream>

#include "synth/core/links.h"
#include "synth/core/resolver.h"

namespace step_util {

    typedef std::uint8_t index_type;
    typedef std::int8_t opt_index_type;

    constexpr static const index_type imax
        = std::numeric_limits<index_type>::max();
    constexpr static const opt_index_type oimax
        = std::numeric_limits<opt_index_type>::max();

    static_assert(MAX_CONTROLS <= imax &&
                  MAX_MODULES <= imax &&
                  MAX_PORTS <= imax,
                  "index_type too small");
    static_assert(MAX_PORTS <= oimax,
                  "opt_index_type too small");

    static inline Control *
    index_to_control(index_type index, const Resolver& res)
    {
      assert((size_t) index < res.controls().size());
      Control *ctl = res.controls()[index];
      assert(ctl);
      return ctl;
    }

    static inline Module *
    index_to_module(index_type index, const Resolver& res)
    {
      assert((size_t) index < res.modules().size());
      Module *mod = res.modules()[index];
      assert(mod);
      return mod;
    }

    static inline InputPort *
    index_to_inport(index_type index, const Resolver& res)
    {
        assert((size_t)index < res.ports().size());
        Port *port = res.ports()[index];
        assert(port && dynamic_cast<InputPort *>(port));
        return static_cast<InputPort *>(port);
    }

    static inline OutputPort *
    index_to_outport(opt_index_type index, const Resolver& res)
    {
        if (index < 0)
            return nullptr;
        assert((size_t)index < res.ports().size());
        Port *port = res.ports()[index];
        assert(port && dynamic_cast<OutputPort *>(port));
        return static_cast<OutputPort *>(port);
    }

};


// -- Prep Steps  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class ClearStep {

public:

    ClearStep() = default;
    ClearStep(size_t dest_port_index, SCALE_TYPE scale)
    : m_dest_port_index{step_util::index_type(dest_port_index)}, m_scale{scale}
    {}

    void prep(const Resolver& res) const
    {
        InputPort *dest = step_util::index_to_inport(m_dest_port_index, res);
        dest->clear(m_scale);
    }

    friend std::ostream&
    operator << (std::ostream& o, const ClearStep& s)
    {
        return o << "clear("
                 << size_t(s.m_dest_port_index)
                 << ", "
                 << s.m_scale
                 << ")";
    }

private:

    step_util::index_type m_dest_port_index;
    SCALE_TYPE            m_scale;

    friend class steps_unit_test;

};

class AliasStep {

public:

    AliasStep() = default;
    AliasStep(size_t dest_port_index, ssize_t src_port_index)
    : m_dest_port_index{step_util::index_type(dest_port_index)},
      m_src_port_index{step_util::opt_index_type(src_port_index)}
    {}

    void prep(const Resolver& res) const
    {
        InputPort *dest = step_util::index_to_inport(m_dest_port_index, res);
        OutputPort *src = step_util::index_to_outport(m_src_port_index, res);
        dest->alias(src->void_buf());
    }

    friend std::ostream&
    operator << (std::ostream& o, const AliasStep& s)
    {
        return o << "alias("
                 << size_t(s.m_dest_port_index)
                 << ", "
                 << ssize_t(s.m_src_port_index)
                 << ")";
    }

private:

    step_util::index_type     m_dest_port_index;
    step_util::opt_index_type m_src_port_index;

    friend class steps_unit_test;

};

class PrepStep {

public:

    enum class Tag {
        NONE,
        CLEAR,
        ALIAS,
    };

    PrepStep() : m_tag{Tag::NONE} {}
    PrepStep(const ClearStep& clear)
    : m_tag{Tag::CLEAR}, m_u{clear}
    {}
    PrepStep(const AliasStep& alias)
    : m_tag{Tag::ALIAS}, m_u{alias}
    {}

    Tag tag() const { return m_tag; }

    void prep(const Resolver& res) const
    {
        switch (m_tag) {

        case Tag::CLEAR:
            m_u.clear.prep(res);
            break;

        case Tag::ALIAS:
            m_u.alias.prep(res);
            break;

        default:
            assert(0 && "invalid prep type");
        }
    }

    friend std::ostream&
    operator << (std::ostream& o, const PrepStep& s)
    {
        switch (s.m_tag) {

        case Tag::NONE:
            return o << "none()";

        case Tag::CLEAR:
            return o << s.m_u.clear;

        case Tag::ALIAS:
            return o << s.m_u.alias;

        default:
            return o << "PrepStep[" << int(s.m_tag) << "]()";
        }
    }

private:

    Tag m_tag;
    union u {
        u() = default;
        u(const ClearStep& clear) : clear{clear} {}
        u(const AliasStep& alias) : alias{alias} {}
        ClearStep clear;
        AliasStep alias;
    } m_u;

    friend class steps_unit_test;

};


// -- Render Steps - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class ControlRenderStep {

public:

    ControlRenderStep() = default;
    ControlRenderStep(size_t ctl_index)
    : m_ctl_index{step_util::index_type(ctl_index)} {}

    render_action make_action(const Resolver& res) const
    {
        Control *ctl = step_util::index_to_control(m_ctl_index, res);
        return ctl->make_render_action();
    }

    friend std::ostream&
    operator << (std::ostream& o, const ControlRenderStep s)
    {
        return o << "crend(" << size_t(s.m_ctl_index) << ")";
    }

private:

    step_util::index_type m_ctl_index;

    friend class steps_unit_test;

};

class ModuleRenderStep {

public:

    ModuleRenderStep() = default;
    ModuleRenderStep(size_t mod_index)
    : m_mod_index{step_util::index_type(mod_index)}
    {}

    render_action make_action(const Resolver& res) const
    {
        Module *mod = step_util::index_to_module(m_mod_index, res);
        return mod->make_render_action();
    }

    friend std::ostream&
    operator << (std::ostream& o, const ModuleRenderStep s)
    {
        return o << "mrend(" << size_t(s.m_mod_index) << ")";
    }

private:

    step_util::index_type m_mod_index;

    friend class steps_unit_test;

};

class CopyStep {

public:

    CopyStep() = default;
    CopyStep(size_t      dest_port_index,
             ssize_t     src_port_index,
             ssize_t     ctl_port_index,
             const Link *link)
    : m_dest_port_index{step_util::index_type(dest_port_index)},
      m_src_port_index{step_util::opt_index_type(src_port_index)},
      m_ctl_port_index{step_util::opt_index_type(ctl_port_index)},
      m_link{link}
    {}

    render_action make_action(const Resolver& res) const
    {
        InputPort *dest = step_util::index_to_inport(m_dest_port_index, res);
        OutputPort *src = step_util::index_to_outport(m_src_port_index, res);
        OutputPort *ctl = step_util::index_to_outport(m_ctl_port_index, res);

        return m_link->make_copy_action(dest, src, ctl);
    }

    friend std::ostream&
    operator << (std::ostream& o, const CopyStep& s)
    {
        return o << "copy("
                 << size_t(s.m_dest_port_index)
                 << ", "
                 << ssize_t(s.m_src_port_index)
                 << ", "
                 << ssize_t(s.m_ctl_port_index)
                 << ")";
    }

private:

    step_util::index_type     m_dest_port_index;
    step_util::opt_index_type m_src_port_index;
    step_util::opt_index_type m_ctl_port_index;
    const Link               *m_link;

    friend class steps_unit_test;

};

class AddStep {

public:

    AddStep() = default;
    AddStep(size_t      dest_port_index,
            ssize_t     src_port_index,
            ssize_t     ctl_port_index,
            const Link *link)
    : m_dest_port_index{step_util::index_type(dest_port_index)},
      m_src_port_index{step_util::opt_index_type(src_port_index)},
      m_ctl_port_index{step_util::opt_index_type(ctl_port_index)},
      m_link{link}
    {}

    render_action make_action(const Resolver& res) const
    {
        InputPort *dest = step_util::index_to_inport(m_dest_port_index, res);
        OutputPort *src = step_util::index_to_outport(m_src_port_index, res);
        OutputPort *ctl = step_util::index_to_outport(m_ctl_port_index, res);

        return m_link->make_add_action(dest, src, ctl);

    }

    friend std::ostream&
    operator << (std::ostream& o, const AddStep& s)
    {
        return o << "add("
                 << size_t(s.m_dest_port_index)
                 << ", "
                 << ssize_t(s.m_src_port_index)
                 << ", "
                 << ssize_t(s.m_ctl_port_index)
                 << ")";
    }

private:

    step_util::index_type     m_dest_port_index;
    step_util::opt_index_type m_src_port_index;
    step_util::opt_index_type m_ctl_port_index;
    const Link               *m_link;

    friend class steps_unit_test;

};


class RenderStep {

public:

    enum class Tag {
        NONE,
        CONTROL_RENDER,
        MODULE_RENDER,
        COPY,
        ADD,
    };

    RenderStep() : m_tag{Tag::NONE} {}
    RenderStep(const ControlRenderStep& crend)
    : m_tag{Tag::CONTROL_RENDER}, m_u{crend}
    {}
    RenderStep(const ModuleRenderStep& mrend)
    : m_tag{Tag::MODULE_RENDER}, m_u{mrend}
    {}
    RenderStep(const CopyStep& copy)
    : m_tag{Tag::COPY}, m_u{copy}
    {}
    RenderStep(const AddStep& add)
    : m_tag{Tag::ADD}, m_u{add}
    {}

    Tag tag() const { return m_tag; }

    render_action make_action(const Resolver& res) const
    {
        switch (m_tag) {

        case Tag::CONTROL_RENDER:
            return m_u.crend.make_action(res);

        case Tag::MODULE_RENDER:
            return m_u.mrend.make_action(res);

        case Tag::COPY:
            return m_u.copy.make_action(res);

        case Tag::ADD:
            return m_u.add.make_action(res);

        default:
            assert(0 && "invalid process type");
            return [] (size_t) { abort(); };
        }
    }

    friend std::ostream&
    operator << (std::ostream& o, const RenderStep& s)
    {
        switch (s.m_tag) {

        case Tag::NONE:
            return o << "none()";

        case Tag::CONTROL_RENDER:
            return o << s.m_u.crend;

        case Tag::MODULE_RENDER:
            return o << s.m_u.mrend;

        case Tag::COPY:
            return o << s.m_u.copy;

        case Tag::ADD:
            return o << s.m_u.add;

        default:
            return o << "RenderStep[" << int(s.m_tag) << "]()";
        }
    }

private:

    Tag m_tag;
    union u {
        u() = default;
        u(const ControlRenderStep& crend) : crend(crend) {}
        u(const ModuleRenderStep& mrend) : mrend{mrend} {}
        u(const CopyStep& copy) : copy{copy} {}
        u(const AddStep& add) : add{add} {}
        ControlRenderStep crend;
        ModuleRenderStep mrend;
        CopyStep copy;
        AddStep add;
    } m_u;

    friend class steps_unit_test;

};

#endif /* !STEPS_included */
