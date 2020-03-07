#include <iostream>
#include <utility>

class Container {

public:

    Container()
    {
        m_a[0] = 0;
        m_a[1] = 2;
        m_a[2] = 1;
        std::cout << "Container()" << std::endl;
    }
    Container(const Container&);
    Container(Container&&);
    Container& operator = (const Container&);
    Container& operator = (const Container&&);
    ~Container()
    {
        std::cout << "~Container()" << std::endl;
    }

private:

    int m_a[3];

};

Container foo()
{
    return Container();
}

Container bar()
{
    return foo();
}

class Object {

public:

    Object()
        : c(bar())
    {
        std::cout << "Object()" << std::endl;
    }

private:
    Container c;
};

Object foo1()
{
    return Object();
}

Object bar1()
{
    return foo1();
}

int main()
{
    Object o = bar1();
    return 0;
}
