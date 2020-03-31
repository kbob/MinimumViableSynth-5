#ifndef ACTIONS_included
#define ACTIONS_included

// -- Prep Actions - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class ClearAction {
    uint8_t m_in_port_index;
public:
    ClearAction() = default;
    ClearAction(uint8_t in_port_index)
    : m_in_port_index{in_port_index}
    {}
    void do_it() const
    {
        std::cout << "clear "
                  << int(m_in_port_index)
                  << std::endl;
    }
};

class AliasAction {
    uint8_t m_out_port_index;
    uint8_t m_in_port_index;
public:
    AliasAction() = default;
    AliasAction(uint8_t out_port_index, uint8_t in_port_index)
    : m_out_port_index(out_port_index), m_in_port_index(in_port_index)
    {}
    void do_it() const
    {
        std::cout << "alias "
                  << int(m_in_port_index)
                  << " to "
                  << int(m_out_port_index)
                  << std::endl;
    }
};

enum class PrepActionType {
    CLEAR,
    ALIAS,
};

class PrepAction {
    PrepActionType m_tag;
    union u {
        u() = default;
        u(const ClearAction& clear) : clear(clear) {}
        u(const AliasAction& alias) : alias(alias) {}
        ClearAction clear;
        AliasAction alias;
    } m_u;
public:
    PrepAction(const ClearAction& clear)
    : m_tag(PrepActionType::CLEAR), m_u(clear)
    {}
    PrepAction(const AliasAction& alias)
    : m_tag(PrepActionType::ALIAS), m_u(alias)
    {}
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
};

// -- Process Actions - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CopyAction {
    uint8_t m_out_port_index;
    uint8_t m_in_port_index;
    void   *m_control;
public:
    CopyAction() = default;
    CopyAction(uint8_t out_port_index, uint8_t in_port_index, void *control)
    : m_out_port_index(out_port_index),
      m_in_port_index(in_port_index),
      m_control(control)
    {}
    void do_it() const
    {
        std::cout << "copy "
                  << int(m_out_port_index)
                  << " to "
                  << int(m_in_port_index)
                  << " using "
                  << (const char *)m_control
                  << std::endl;
    }
};

class AddAction {
    uint8_t m_out_port_index;
    uint8_t m_in_port_index;
    void   *m_control;
public:
    AddAction() = default;
    AddAction(uint8_t out_port_index, uint8_t in_port_index, void *control)
    : m_out_port_index(out_port_index),
      m_in_port_index(in_port_index),
      m_control(control)
    {}
    void do_it() const
    {
        std::cout << "add "
                  << int(m_out_port_index)
                  << " to "
                  << int(m_in_port_index)
                  << " using "
                  << (const char *)m_control
                  << std::endl;
    }
};

class RenderAction {
    uint8_t m_mod_index;
public:
    RenderAction() = default;
    RenderAction(uint8_t mod_index) : m_mod_index(mod_index) {}
    void do_it() const
    {
        std::cout << "render "
                  << int(m_mod_index)
                  << std::endl;
    }
};

enum class RunActionType {
    COPY,
    ADD,
    RENDER,
};

class RunAction {
public:
    RunActionType m_tag;
    union u {
        u() = default;
        u(const CopyAction& copy) : copy(copy) {}
        u(const AddAction& add) : add(add) {}
        u(const RenderAction& render) : render(render) {}
        CopyAction copy;
        AddAction add;
        RenderAction render;
    } m_u;
public:
    RunAction(const CopyAction& copy)
    : m_tag(RunActionType::COPY), m_u(copy)
    {}
    RunAction(const AddAction& add)
    : m_tag(RunActionType::ADD), m_u(add)
    {}
    RunAction(const RenderAction& render)
    : m_tag(RunActionType::RENDER), m_u(render)
    {}
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
