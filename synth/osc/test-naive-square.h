#include "naive-square.h"

#include <cxxtest/TestSuite.h>

class naive_square_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)NaiveSquare();
    }

};
