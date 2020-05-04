#include "midi-controls.h"

#include <cxxtest/TestSuite.h>

using namespace midi;

class midi_controls_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)NoteFreqControl();
    }

};
