#ifndef ACTIONS_included
#define ACTIONS_included

// typedef float SCALE_TYPE;
#include "synth/core/links.h"

// -- Prep Actions - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class ClearAction {
public:
    ClearAction() = default;
    ClearAction(uint8_t dest_port_index, SCALE_TYPE scale)
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

    friend class ActionsUnitTest;
};

class AliasAction {
public:
    AliasAction() = default;
    AliasAction(uint8_t dest_port_index, uint8_t src_port_index)
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

    friend class ActionsUnitTest;
};

enum class PrepActionType {
    NONE,
    CLEAR,
    ALIAS,
};

class PrepAction {
public:
    PrepAction() : m_tag{PrepActionType::NONE} {}
    PrepAction(const ClearAction& clear)
    : m_tag{PrepActionType::CLEAR}, m_u{clear}
    {}
    PrepAction(const AliasAction& alias)
    : m_tag{PrepActionType::ALIAS}, m_u{alias}
    {}
    PrepActionType type() const { return m_tag; }
    void do_it() const
    {
        switch (m_tag) {
        case PrepActionType::CLEAR:
            m_u.clear.do_it();
            break;
        case PrepActionType::ALIAS:
            m_u.alias.do_it();
            break;
        default:
            assert(0 && "invalid prep type");
        }
    }
private:
    PrepActionType m_tag;
    union u {
        u() = default;
        u(const ClearAction& clear) : clear{clear} {}
        u(const AliasAction& alias) : alias{alias} {}
        ClearAction clear;
        AliasAction alias;
    } m_u;

    friend class ActionsUnitTest;
};

// -- Run Actions -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// voice/global
// const/varying
// copy/add
// ctl? scale? src<type>? dest<type>
//
// -ctl -src -> const (move to prep)
// nonvarying ctl -src -> const


class CopyAction {
public:
    CopyAction() = default;
    CopyAction(uint8_t  dest_port_index,
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
    friend class ActionsUnitTest;
};

class AddAction {
public:
    AddAction() = default;
    AddAction(uint8_t  dest_port_index,
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

    friend class ActionsUnitTest;
};

class RenderAction {
    uint8_t m_mod_index;
    friend class ActionsUnitTest;
public:
    RenderAction() = default;
    RenderAction(uint8_t mod_index) : m_mod_index{mod_index} {}
    void do_it() const
    {
        std::cout << "render "
                  << int(m_mod_index)
                  << std::endl;
    }
};

enum class RunActionType {
    NONE,
    COPY,
    ADD,
    RENDER,
};

class RunAction {
    friend class ActionsUnitTest;
private:
    RunActionType m_tag;
    union u {
        u() = default;
        u(const CopyAction& copy) : copy{copy} {}
        u(const AddAction& add) : add{add} {}
        u(const RenderAction& render) : render{render} {}
        CopyAction copy;
        AddAction add;
        RenderAction render;
    } m_u;
public:
    RunAction() : m_tag{RunActionType::NONE} {}
    RunAction(const CopyAction& copy)
    : m_tag{RunActionType::COPY}, m_u{copy}
    {}
    RunAction(const AddAction& add)
    : m_tag{RunActionType::ADD}, m_u{add}
    {}
    RunAction(const RenderAction& render)
    : m_tag{RunActionType::RENDER}, m_u{render}
    {}
    RunActionType type() const { return m_tag; }
    void do_it() const
    {
        switch (m_tag) {
        case RunActionType::COPY:
            m_u.copy.do_it();
            break;
        case RunActionType::ADD:
            m_u.add.do_it();
            break;
        case RunActionType::RENDER:
            m_u.render.do_it();
            break;
        default:
            assert(0 && "invalid process type");
        }
    }
};

#endif /* !ACTIONS_included */
