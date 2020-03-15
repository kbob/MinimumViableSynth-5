#include "synth/util/heap-map.h"

#include <cxxtest/TestSuite.h>

class heap_map_test : public CxxTest::TestSuite {

public:

    heap_map<int, const char *> *hmp;

    void setUp()
    {
        hmp = new heap_map<int, const char *>();
    }

    void tearDown()
    {
        delete hmp;
    }

    void testEmpty()
    {
        TS_ASSERT(hmp->size() == 0);
        TS_ASSERT(hmp->empty() == true);
        TS_ASSERT(hmp->begin() == hmp->end());
        TS_ASSERT(hmp->cbegin() == hmp->cend());
        TS_ASSERT(hmp->rbegin() == hmp->rend());
        TS_ASSERT(hmp->crbegin() == hmp->crend());
    }

    void testInsert()
    {
        (*hmp)[13] = "m";
        TS_ASSERT(hmp->size() == 1);
        TS_ASSERT(hmp->empty() == false);
        TS_ASSERT(!strcmp((*hmp)[13], "m"));
    }

    void testCapacity()
    {
        (*hmp)[1] = "a";
        (*hmp)[2] = "b";
        (*hmp)[13] = "m";
        TS_ASSERT(hmp->size() == 3);
        TS_ASSERT(hmp->capacity() >= 3);
        hmp->reserve(100);
        TS_ASSERT(hmp->capacity() == 100);
        hmp->shrink_to_fit();
        TS_ASSERT(hmp->capacity() == 3);
    }

    void testIter()
    {
        (*hmp)[13] = "m";
        (*hmp)[2] = "b";
        TS_ASSERT(hmp->size() == 2);
        auto a = hmp->cbegin();
        auto e = hmp->cend();
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
        TS_ASSERT(a == hmp->cbegin());
        TS_ASSERT(a->first == 13);

        auto ra = hmp->crbegin();
        auto re = hmp->crend();
        TS_ASSERT(ra->first == 2);
        ra++;
        TS_ASSERT(ra->first == 13);
        ++ra;
        TS_ASSERT(ra == re);
        --re;
    }

    void testFinalize()
    {
        (*hmp)[13] = "m";
        (*hmp)[2] = "b";
        hmp->finalize();

        auto vc = hmp->value_comp();
        auto kc = hmp->key_comp();
        TS_ASSERT(kc(hmp->begin()->first, hmp->begin()->first) == 0);
        TS_ASSERT(vc(*hmp->begin(), *hmp->begin()) == 0);
        hmp->find(13);
        hmp->at(13);
        TS_ASSERT(!strcmp(hmp->at(13), "m"));
    }

    void testConst()
    {
        (*hmp)[13] = "m";
        (*hmp)[2] = "b";
        hmp->finalize();

        const heap_map<int, const char *>& hmr = *hmp;

        auto vc = hmr.value_comp();
        auto kc = hmr.key_comp();
        TS_ASSERT(kc(hmp->begin()->first, hmp->begin()->first) == 0);
        TS_ASSERT(vc(*hmp->begin(), *hmp->begin()) == 0);

        TS_ASSERT(hmr.find(2) == hmr.cbegin());
        TS_ASSERT(!strcmp(hmr.at(2), "b"));

        TS_ASSERT(hmr.count(2) == 1);
        TS_ASSERT(hmr.count(3) == 0);

    }

};
