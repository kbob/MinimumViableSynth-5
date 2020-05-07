#include "summer.h"

#include <cxxtest/TestSuite.h>

class summer_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)Summer<>();
    }

    void test_linkage()
    {
        Summer<> s;
        TS_ASSERT_EQUALS(&s.voice_side.m_parent, &s);
        TS_ASSERT_EQUALS(&s.timbre_side.m_voice_ports, &s.m_voice_ports);
        TS_ASSERT_EQUALS(s.m_voice_ports.size(), 1);
        TS_ASSERT_EQUALS(s.m_voice_ports[0], &s.voice_side.in);
    }

    void test_clone_timbre()
    {
        Summer<> s;
        Module *m = s.timbre_side.clone();
        Summer<>::TimbreSide *ts = dynamic_cast<Summer<>::TimbreSide *>(m);
        TS_ASSERT(ts);
        TS_ASSERT_EQUALS(&ts->m_voice_ports, &s.m_voice_ports);
        delete m;
    }

    void test_clone_voice()
    {
        Summer<> s;
        Module *m = s.voice_side.clone();
        Summer<>::VoiceSide *vs = dynamic_cast<Summer<>::VoiceSide *>(m);
        TS_ASSERT(vs);
        TS_ASSERT_EQUALS(&vs->m_parent, &s);
        TS_ASSERT_EQUALS(s.m_voice_ports.size(), 2);
        TS_ASSERT_EQUALS(s.m_voice_ports[1], &vs->in);
        delete m;
    }

    void test_clone_types()
    {
        double a_double;
        Summer<double> ds;
        Module *m = ds.voice_side.clone();
        auto dvs = dynamic_cast<Summer<double>::VoiceSide *>(m);
        TS_ASSERT(dvs);
        TS_ASSERT(typeid(dvs->in[0]) == typeid(a_double));

        m = ds.timbre_side.clone();
        auto dts = dynamic_cast<Summer<double>::TimbreSide *>(m);
        TS_ASSERT(dts);
        TS_ASSERT(typeid(dts->out[0]) == typeid(a_double));
    }

    void test_summing()
    {
        Timbre t(false);
        Summer<> s;
        auto vs = dynamic_cast<Summer<>::VoiceSide *>(s.voice_side.clone());
        t.add_module(&s.timbre_side);
        t.add_voice(0);
        t.add_voice(1);
        for (size_t i = 0; i < MAX_FRAMES; i++) {
            s.voice_side.in.buf()[i] = i + 1;
            vs->in.buf()[i] = i + 2;
        }
        s.timbre_side.render(MAX_FRAMES);
        for (size_t i = 0; i < MAX_FRAMES; i++)
            TS_ASSERT_EQUALS(s.timbre_side.out[i], 2 * i + 3);
    }

};
