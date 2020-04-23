#ifndef STEPS_included
#define STEPS_included

#include "synth/core/links.h"


// -- Prep Steps  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class ClearStep {
public:
    ClearStep() = default;
    ClearStep(uint8_t dest_port_index, SCALE_TYPE scale)
    : m_dest_port_index{dest_port_index}, m_scale{scale}
    {}
    void do_it() const
    {
        std::cout << "clear "
                  << int(m_dest_port_index)
                  << " to "
                  << m_scale
                  << std::endl;
    }
private:
    uint8_t    m_dest_port_index;
    SCALE_TYPE m_scale;

    friend class steps_unit_test;
};

class AliasStep {
public:
    AliasStep() = default;
    AliasStep(uint8_t dest_port_index, uint8_t src_port_index)
    : m_dest_port_index{dest_port_index}, m_src_port_index{src_port_index}
    {}
    void do_it() const
    {
        std::cout << "alias "
                  << int(m_dest_port_index)
                  << " to "
                  << int(m_src_port_index)
                  << std::endl;
    }
private:
    uint8_t m_dest_port_index;
    uint8_t m_src_port_index;

    friend class steps_unit_test;
};

enum class PrepStepTag {
    NONE,
    CLEAR,
    ALIAS,
};

class PrepStep {
public:
    PrepStep() : m_tag{PrepStepTag::NONE} {}
    PrepStep(const ClearStep& clear)
    : m_tag{PrepStepTag::CLEAR}, m_u{clear}
    {}
    PrepStep(const AliasStep& alias)
    : m_tag{PrepStepTag::ALIAS}, m_u{alias}
    {}
    PrepStepTag tag() const { return m_tag; }
    void do_it() const
    {
        switch (m_tag) {
        case PrepStepTag::CLEAR:
            m_u.clear.do_it();
            break;
        case PrepStepTag::ALIAS:
            m_u.alias.do_it();
            break;
        default:
            assert(0 && "invalid prep type");
        }
    }
private:
    PrepStepTag m_tag;
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

class CopyStep {
public:
    CopyStep() = default;
    CopyStep(uint8_t  dest_port_index,
             uint8_t  src_port_index,
             uint8_t  ctl_port_index,
             Link    *link)
    : m_dest_port_index{dest_port_index},
      m_src_port_index{src_port_index},
      m_ctl_port_index{ctl_port_index},
      m_link{link}
    {}
    void do_it() const
    {
        std::cout << "copy "
                  << int(m_src_port_index)
                  << " and "
                  << int(m_ctl_port_index)
                  << " to "
                  << int(m_dest_port_index)
                  << " scaled "
                  << m_link->scale()
                  << std::endl;
    }
private:
    uint8_t  m_dest_port_index;
    uint8_t  m_src_port_index;
    uint8_t  m_ctl_port_index;
    Link    *m_link;
    friend class steps_unit_test;
};

class AddStep {
public:
    AddStep() = default;
    AddStep(uint8_t  dest_port_index,
            uint8_t  src_port_index,
            uint8_t  ctl_port_index,
            Link    *link)
    : m_dest_port_index{dest_port_index},
      m_src_port_index{src_port_index},
      m_ctl_port_index{ctl_port_index},
      m_link{link}
    {}
    void do_it() const
    {
        std::cout << "add "
                  << int(m_src_port_index)
                  << " and "
                  << int(m_ctl_port_index)
                  << " to "
                  << int(m_dest_port_index)
                  << " scaled "
                  << m_link->scale()
                  << std::endl;
    }
private:
    uint8_t  m_dest_port_index;
    uint8_t  m_src_port_index;
    uint8_t  m_ctl_port_index;
    Link    *m_link;

    friend class steps_unit_test;
};

class ModuleRenderStep {
    uint8_t m_mod_index;
    friend class steps_unit_test;
public:
    ModuleRenderStep() = default;
    ModuleRenderStep(uint8_t mod_index) : m_mod_index{mod_index} {}
    void do_it() const
    {
        std::cout << "mrend "
                  << int(m_mod_index)
                  << std::endl;
    }
};

enum class RenderStepTag {
    NONE,
    COPY,
    ADD,
    MODULE_RENDER,
};

class RenderStep {
    friend class steps_unit_test;
private:
    RenderStepTag m_tag;
    union u {
        u() = default;
        u(const CopyStep& copy) : copy{copy} {}
        u(const AddStep& add) : add{add} {}
        u(const ModuleRenderStep& mrend) : mrend{mrend} {}
        CopyStep copy;
        AddStep add;
        ModuleRenderStep mrend;
    } m_u;
public:
    RenderStep() : m_tag{RenderStepTag::NONE} {}
    RenderStep(const CopyStep& copy)
    : m_tag{RenderStepTag::COPY}, m_u{copy}
    {}
    RenderStep(const AddStep& add)
    : m_tag{RenderStepTag::ADD}, m_u{add}
    {}
    RenderStep(const ModuleRenderStep& mrend)
    : m_tag{RenderStepTag::MODULE_RENDER}, m_u{mrend}
    {}
    RenderStepTag tag() const { return m_tag; }
    void do_it() const
    {
        switch (m_tag) {
        case RenderStepTag::COPY:
            m_u.copy.do_it();
            break;
        case RenderStepTag::ADD:
            m_u.add.do_it();
            break;
        case RenderStepTag::MODULE_RENDER:
            m_u.mrend.do_it();
            break;
        default:
            assert(0 && "invalid process type");
        }
    }
};

#endif /* !STEPS_included */
