#include "midi-controls.h"

#include <cxxtest/TestSuite.h>

class midi_controls_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)midi::NoteFreqControl();
    }

};
