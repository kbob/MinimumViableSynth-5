#include <iostream>

// my_thing.h:

template <class Mock = void>
class my_thing;

template <>
class my_thing<void> {
public:
    void public_method() { std::cout << "  public method\n"; }
    void other_method() { std::cout << "  other method\n"; }
};

// some_unit_test.h:

class some_unit_test;

template <>
class my_thing<some_unit_test> : public my_thing<void> {
public:
    void public_method() { std::cout << "  mock method\n"; }
};

class some_unit_test {
public:
    void test_something()
    {
        my_thing<some_unit_test> mock_thing;
        mock_thing.public_method();
        mock_thing.other_method();
    }
};

// driver.cpp:

int main()
{
    std::cout << "testing\n";
    some_unit_test t;
    t.test_something();

    std::cout << "\nrunning\n";
    my_thing<> thing;
    thing.public_method();
    thing.other_method();
    return 0;
}
