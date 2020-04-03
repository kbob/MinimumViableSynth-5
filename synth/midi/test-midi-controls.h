#include "midi-controls.h"

#include <cxxtest/TestSuite.h>

using namespace midi;

class MidiControlsUnitTest : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)NoteControl();
    }

};
