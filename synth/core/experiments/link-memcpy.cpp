#include <iostream>
#include <vector>

// #include "synth/core/links.h"


// -- Debugging - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#include <cxxabi.h>

#define HERE (std::cout << __FILE__ << ':' << __LINE__ << std::endl)

// XXX Move this into the platform directory.
std::string demangle(const std::string& mangled)
{
    int status;
    char *s = abi::__cxa_demangle(mangled.c_str(), 0, 0, &status);
    std::string demangled(s);
    std::free(s);
    return demangled;
}

template <typename T>
static std::string type_name(T& obj)
{
    const std::string& mangled = typeid(*&obj).name();
    return demangle(mangled);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


struct Port {};

template <class T>
struct Input : public Port { T member; };

struct Link {
    Link(Port *p) : p(p) {}
    virtual ~Link() = default;
    virtual void method() = 0;
    Port *p;
};

template <class T>
struct LinkType : public Link {
    LinkType(Input<T> *p) : Link(p) {}
    void method() override
    {
        std::cout << static_cast<Input<T> *>(p)->member;
    }
};

typedef LinkType<int> default_link;
typedef std::vector<default_link> V;

int main()
{
    Input<char> in;
    in.member = 'X';
    LinkType<char> link(&in);
    V vec;
    vec.push_back(default_link(nullptr));
    static_assert(sizeof vec.back() == sizeof link, "link size changed");
    memcpy((void *)&vec.back(), (void *)&link, sizeof link);
    vec.front().method();
    std::cout << std::endl;

    return 0;
}
