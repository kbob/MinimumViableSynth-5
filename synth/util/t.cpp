#include <cassert>
#include <iostream>

// prep_action:
//     Clear   InputPort ip[j]
//             uint8_t
//     Alias   InputPort ip[i] to OutputPort op[j]
//             uint8_t, uint8_t
// process_action:
//     Copy    InputPort, OutputPort, ControlLink (pointer?)
//             uint8_t, uint8_t, ptr
//     Add     InputPort, OutputPort, ControlLink (pointer?)
//             uint8_t, uint8_t, ptr
//     Render  Module m[i]
//             uint8_t

// -- Prep Actions - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class clear_action {
    uint8_t m_in_port_index;
public:
    clear_action() = default;
    clear_action(uint8_t in_port_index)
    : m_in_port_index{in_port_index}
    {}
    void do_it() const
    {
        std::cout << "clear "
                  << int(m_in_port_index)
                  << std::endl;
    }
};

class alias_action {
    uint8_t m_out_port_index;
    uint8_t m_in_port_index;
public:
    alias_action() = default;
    alias_action(uint8_t out_port_index, uint8_t in_port_index)
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

enum class prep_action_type {
    CLEAR,
    ALIAS,
};

class prep_action {
    prep_action_type m_tag;
    union u {
        u() = default;
        u(const clear_action& clear) : clear(clear) {}
        u(const alias_action& alias) : alias(alias) {}
        clear_action clear;
        alias_action alias;
    } m_u;
public:
    prep_action(const clear_action& clear)
    : m_tag(prep_action_type::CLEAR), m_u(clear)
    {}
    prep_action(const alias_action& alias)
    : m_tag(prep_action_type::ALIAS), m_u(alias)
    {}
    void do_it() const
    {
        switch (m_tag) {
        case prep_action_type::CLEAR:
            m_u.clear.do_it();
            break;
        case prep_action_type::ALIAS:
            m_u.alias.do_it();
            break;
        default:
            assert(0 && "invalid prep type");
        }

    }
};

// -- Process Actions-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class copy_action {
    uint8_t m_out_port_index;
    uint8_t m_in_port_index;
    void   *m_control;
public:
    copy_action() = default;
    copy_action(uint8_t out_port_index, uint8_t in_port_index, void *control)
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

class add_action {
    uint8_t m_out_port_index;
    uint8_t m_in_port_index;
    void   *m_control;
public:
    add_action() = default;
    add_action(uint8_t out_port_index, uint8_t in_port_index, void *control)
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

class render_action {
    uint8_t m_mod_index;
public:
    render_action() = default;
    render_action(uint8_t mod_index) : m_mod_index(mod_index) {}
    void do_it() const
    {
        std::cout << "render "
                  << int(m_mod_index)
                  << std::endl;
    }
};

enum class process_action_type {
    COPY,
    ADD,
    RENDER,
};

class process_action {
public:
    process_action_type m_tag;
    union u {
        u() = default;
        u(const copy_action& copy) : copy(copy) {}
        u(const add_action& add) : add(add) {}
        u(const render_action& render) : render(render) {}
        copy_action copy;
        add_action add;
        render_action render;
    } m_u;
public:
    process_action(const copy_action& copy)
    : m_tag(process_action_type::COPY), m_u(copy)
    {}
    process_action(const add_action& add)
    : m_tag(process_action_type::ADD), m_u(add)
    {}
    process_action(const render_action& render)
    : m_tag(process_action_type::RENDER), m_u(render)
    {}
    void do_it() const
    {
        switch (m_tag) {
        case process_action_type::COPY:
            m_u.copy.do_it();
            break;
        case process_action_type::ADD:
            m_u.add.do_it();
            break;
        case process_action_type::RENDER:
            m_u.render.do_it();
            break;
        default:
            assert(0 && "invalid process type");
        }
    }
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

int main()
{
    std::cout << "clear   size = " << sizeof (clear_action) << std::endl;
    std::cout << "alias   size = " << sizeof (alias_action) << std::endl;
    std::cout << "prep    size = " << sizeof (prep_action) << std::endl;
    std::cout << "copy    size = " << sizeof (copy_action) << std::endl;
    std::cout << "add     size = " << sizeof (add_action) << std::endl;
    std::cout << "render  size = " << sizeof (render_action) << std::endl;
    std::cout << "process size = " << sizeof (process_action) << std::endl;

    clear_action clear = {3};
    alias_action alias = {4, 5};
    prep_action p1(clear);
    prep_action p2(alias);
    p1.do_it();
    p2.do_it();

    copy_action copy = { 6, 7, (void *)"a" };
    add_action add = { 8, 7, (void *)"b" };
    render_action render = {0};
    process_action p3(copy);
    process_action p4(add);
    process_action p5(render);
    p3.do_it();
    p4.do_it();
    p5.do_it();

    return 0;
}
