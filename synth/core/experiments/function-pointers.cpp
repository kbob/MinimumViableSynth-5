#include <functional>
#include <iostream>
#include <string>



class SmallMessage {
public:
    SmallMessage(const std::string& s) : m_contents{s} {}
    std::string m_contents;
};

struct small_handler {
    typedef void handler_function(void *, const SmallMessage&);
    handler_function *m_fun;
    void *m_data;

    small_handler()
    : m_fun{nullptr},
      m_data{nullptr}
    {}

    small_handler(handler_function *f, void *data)
    : m_fun{f},
      m_data{data}
    {}

    void operator () (const SmallMessage& msg) const
    {
        (*m_fun)(m_data, msg);
    }
};

// It occurs to me that `method_wrapper` is what Python calls
// a bound method.  Might be a better name.
template <class T, void (T::*M)(const SmallMessage&)>
class method_wrapper {

public:

    method_wrapper(T *t)
    : handler{invoke_it, t}
    {}

    small_handler handler;

private:

    static void invoke_it(void *obj, const SmallMessage& msg)
    {
        (static_cast<T *>(obj)->*M)(msg);
    }

};

class Quikrete {

public:
    Quikrete(const std::string& name)
    : foo_wrapper{this},
      m_name{name}
    {}

    void handle_foo_message(const SmallMessage& msg)
    {
        std::cout << m_name << ": \"" << msg.m_contents << "\"\n";
    }

    method_wrapper<Quikrete, &Quikrete::handle_foo_message> foo_wrapper;

private:
    std::string m_name;

};

void xyzzy()
{
    Quikrete q("Q");

    small_handler foo;
    foo = q.foo_wrapper.handler;

    std::function<void(const SmallMessage&)> std_fun;
    std_fun = q.foo_wrapper.handler;

    SmallMessage msg("Goodbye");
    foo(msg);
    std_fun(SmallMessage("Standard Greeting"));
}

class Concrete {

public:

    Concrete(const std::string& name)
    : m_foo_msg_handler{foo_handler, this},
     m_name{name}
    {}
    virtual ~Concrete() = default;

    void handle_foo_message(const SmallMessage& msg)
    {
        std::cout << "Concrete "
                  << m_name
                  << ": message = \""
                  << msg.m_contents
                  << "\"\n";
    }

    small_handler get_foo_handler() { return m_foo_msg_handler; }

private:

    static void foo_handler(void *obj, const SmallMessage& msg)
    {
        static_cast<Concrete *>(obj)->handle_foo_message(msg);
    }

    small_handler m_foo_msg_handler;

    std::string m_name;
    friend int main();
};

int main()
{
    xyzzy();

    SmallMessage msg("Hello");
    (void)msg;

    Concrete co{"Barney"};

    auto handler = co.get_foo_handler();
    handler(msg);

    return 0;
}
