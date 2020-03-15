#include "synth/util/vec-map.h"

#include <cxxtest/TestSuite.h>

class vec_map_test : public CxxTest::TestSuite {

public:

    vec_map<int, const char *> *vmp;

    void setUp()
    {
        vmp = new vec_map<int, const char *>();
    }

    void tearDown()
    {
        delete vmp;
    }

    void testEmpty()
    {
        TS_ASSERT(vmp->size() == 0);
        TS_ASSERT(vmp->empty() == true);
        TS_ASSERT(vmp->begin() == vmp->end());
        TS_ASSERT(vmp->cbegin() == vmp->cend());
        TS_ASSERT(vmp->rbegin() == vmp->rend());
        TS_ASSERT(vmp->crbegin() == vmp->crend());
    }

    void testInsert()
    {
        (*vmp)[13] = "m";
        TS_ASSERT(vmp->size() == 1);
        TS_ASSERT(vmp->empty() == false);
        TS_ASSERT(!strcmp((*vmp)[13], "m"));
    }

    void testCapacity()
    {
        (*vmp)[1] = "a";
        (*vmp)[2] = "b";
        (*vmp)[13] = "m";
        TS_ASSERT(vmp->size() == 3);
        TS_ASSERT(vmp->capacity() >= 3);
        vmp->reserve(100);
        TS_ASSERT(vmp->capacity() == 100);
        vmp->shrink_to_fit();
        TS_ASSERT(vmp->capacity() == 3);
    }

    void testIter()
    {
        (*vmp)[13] = "m";
        (*vmp)[2] = "b";
        TS_ASSERT(vmp->size() == 2);
        auto a = vmp->cbegin();
        auto e = vmp->cend();
        TS_ASSERT(a != e);
        TS_ASSERT((*a).first == 13);
        TS_ASSERT(a->first == 13);
        TS_ASSERT(!strcmp(a->second, "m"));
        ++a;
        TS_ASSERT(a != e);
        TS_ASSERT(a->first == 2);
        TS_ASSERT(!strcmp(a->second, "b"));
        a++;
        TS_ASSERT(a == e);
        --a;
        TS_ASSERT(a != e);
        TS_ASSERT(a->first == 2);
        a--;
        TS_ASSERT(a == vmp->cbegin());
        TS_ASSERT(a->first == 13);

        auto ra = vmp->crbegin();
        auto re = vmp->crend();
        TS_ASSERT(ra->first == 2);
        ra++;
        TS_ASSERT(ra->first == 13);
        ++ra;
        TS_ASSERT(ra == re);
        --re;
    }

    void testFinalize()
    {
        (*vmp)[13] = "m";
        (*vmp)[2] = "b";
        vmp->finalize();

        auto vc = vmp->value_comp();
        auto kc = vmp->key_comp();
        TS_ASSERT(kc(vmp->begin()->first, vmp->begin()->first) == 0);
        TS_ASSERT(vc(*vmp->begin(), *vmp->begin()) == 0);
        vmp->find(13);
        vmp->at(13);
        TS_ASSERT(!strcmp(vmp->at(13), "m"));
    }

    void testConst()
    {
        (*vmp)[13] = "m";
        (*vmp)[2] = "b";
        vmp->finalize();

        const vec_map<int, const char *>& vmr = *vmp;

        auto vc = vmr.value_comp();
        auto kc = vmr.key_comp();
        TS_ASSERT(kc(vmp->begin()->first, vmp->begin()->first) == 0);
        TS_ASSERT(vc(*vmp->begin(), *vmp->begin()) == 0);

        TS_ASSERT(vmr.find(2) == vmr.cbegin());
        TS_ASSERT(!strcmp(vmr.at(2), "b"));

        TS_ASSERT(vmr.count(2) == 1);
        TS_ASSERT(vmr.count(3) == 0);

    }

};
