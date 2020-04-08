#include <functional>
#include <iostream>

using action = std::function<void()>;

class AbstractModule {

public:
    virtual ~AbstractModule() = default;
    virtual AbstractModule *clone() const = 0;
    virtual action make_render_action() = 0;

protected:
    AbstractModule() = default;
    AbstractModule(const AbstractModule&) = default;
    AbstractModule& operator = (const AbstractModule&) = default;

};

template <class T>
class Module : public AbstractModule {
public:
    AbstractModule *clone() const override
    { return new T(); }
    action make_render_action() override
    {
        return [this] () {
            static_cast<T *>(this)->render();
        };
    }

};

class Filter : public Module<Filter> {
    static int s_instance;
public:
    Filter()
    : m_instance(s_instance++) {}
    void render()
    {
        std::cout << "F render " << m_instance << std::endl;
    }
private:
    int m_instance;
};

int Filter::s_instance;

int main()
{
    Filter f;
    AbstractModule *amp = f.clone();
    action a = amp->make_render_action();

    f.render();
    a();
    f.render();
    a();

    return 0;
}
