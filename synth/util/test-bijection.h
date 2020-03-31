#include "bijection.h"

#include <cxxtest/TestSuite.h>

enum class Color {
    RED, GREEN, PURPLE, BLUE
};

class BijectionUnitTest : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)bijection<float, 5>();
    }

    void test_forward()
    {
        bijection<Color, 3> u;
        u.push_back(Color::GREEN);
        u.push_back(Color::RED);
        u.push_back(Color::BLUE);

        TS_ASSERT(u.at(0) == Color::GREEN);
        TS_ASSERT(u.at(1) == Color::RED);
        TS_ASSERT(u.at(2) == Color::BLUE);
        TS_ASSERT_THROWS(u.at(3), std::domain_error);
    }

    void test_reverse()
    {
        bijection<Color, 3> u;
        u.push_back(Color::GREEN);
        u.push_back(Color::RED);
        u.push_back(Color::BLUE);

        TS_ASSERT(u.at(Color::RED) == 1);
        TS_ASSERT(u.at(Color::GREEN) == 0);
        TS_ASSERT(u.at(Color::BLUE) == 2);
        TS_ASSERT_THROWS(u.at(Color::PURPLE), std::domain_error);
    }

};
