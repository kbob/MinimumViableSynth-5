#include "patch.h"

#include <cxxtest/TestSuite.h>


class patch_unit_test : public CxxTest::TestSuite {

public:

    typedef float D, S, CE;
    class CT : public ControlType<CT, CE> {
    public:
        void render(size_t) {}
    };
    Input<D> dest;
    Output<D> src;
    CT ctl;

    void test_instantiate()
    {
        (void)Patch();
    }

    void test_connect_DSCS()
    {
        auto p = Patch()
                .connect(dest, src, ctl, 0.5f)
                ;
        auto links = p.links();
        TS_ASSERT_EQUALS(links.size(), 1);
        auto& link = links.at(0);
        TS_ASSERT_EQUALS(link.dest(), &dest);
        TS_ASSERT_EQUALS(link.src(), &src);
        TS_ASSERT_EQUALS(link.ctl(), &ctl.out);
        TS_ASSERT_EQUALS(link.scale(), 0.5f);
    }

    void test_connect_DSC()
    {
        auto p = Patch()
                .connect(dest, src, ctl)
                ;
        auto links = p.links();
        TS_ASSERT_EQUALS(links.size(), 1);
        auto& link = links.front();
        TS_ASSERT_EQUALS(link.dest(), &dest);
        TS_ASSERT_EQUALS(link.src(), &src);
        TS_ASSERT_EQUALS(link.ctl(), &ctl.out);
        TS_ASSERT_EQUALS(link.scale(), 1);
    }

    void test_connect_DSPS()
    {
        auto p = Patch()
                .connect(dest, src, ctl.out, 0.5f)
                ;
        auto links = p.links();
        TS_ASSERT_EQUALS(links.size(), 1);
        auto& link = links.front();
        TS_ASSERT_EQUALS(link.dest(), &dest);
        TS_ASSERT_EQUALS(link.src(), &src);
        TS_ASSERT_EQUALS(link.ctl(), &ctl.out);
        TS_ASSERT_EQUALS(link.scale(), 0.5f);
    }

    void test_connect_DSP()
    {
        auto p = Patch()
                .connect(dest, src, ctl.out)
                ;
        auto links = p.links();
        TS_ASSERT_EQUALS(links.size(), 1);
        auto& link = links.front();
        TS_ASSERT_EQUALS(link.dest(), &dest);
        TS_ASSERT_EQUALS(link.src(), &src);
        TS_ASSERT_EQUALS(link.ctl(), &ctl.out);
        TS_ASSERT_EQUALS(link.scale(), 1);
    }

    void test_connect_DSS()
    {
        auto p = Patch()
                .connect(dest, src, 0.5f)
                ;
        auto links = p.links();
        TS_ASSERT_EQUALS(links.size(), 1);
        auto& link = links.at(0);
        TS_ASSERT_EQUALS(link.dest(), &dest);
        TS_ASSERT_EQUALS(link.src(), &src);
        TS_ASSERT_EQUALS(link.ctl(), nullptr);
        TS_ASSERT_EQUALS(link.scale(), 0.5f);
    }

    void test_connect_DSrc()
    {
        auto p = Patch()
                .connect(dest, src)
                ;
        auto links = p.links();
        TS_ASSERT_EQUALS(links.size(), 1);
        auto& link = links.front();
        TS_ASSERT_EQUALS(link.dest(), &dest);
        TS_ASSERT_EQUALS(link.src(), &src);
        TS_ASSERT_EQUALS(link.ctl(), nullptr);
        TS_ASSERT_EQUALS(link.scale(), 1);
    }

    void test_connect_DCS()
    {
        auto p = Patch()
                .connect(dest, ctl, 0.5f)
                ;
        auto links = p.links();
        TS_ASSERT_EQUALS(links.size(), 1);
        auto& link = links.at(0);
        TS_ASSERT_EQUALS(link.dest(), &dest);
        TS_ASSERT_EQUALS(link.src(), nullptr);
        TS_ASSERT_EQUALS(link.ctl(), &ctl.out);
        TS_ASSERT_EQUALS(link.scale(), 0.5f);
    }

    void test_connect_DC()
    {
        auto p = Patch()
                .connect(dest, ctl)
                ;
        auto links = p.links();
        TS_ASSERT_EQUALS(links.size(), 1);
        auto& link = links.front();
        TS_ASSERT_EQUALS(link.dest(), &dest);
        TS_ASSERT_EQUALS(link.src(), nullptr);
        TS_ASSERT_EQUALS(link.ctl(), &ctl.out);
        TS_ASSERT_EQUALS(link.scale(), 1);
    }

    void test_connect_DScale()
    {
        auto p = Patch()
                .connect(dest, 0.5f)
                ;
        auto links = p.links();
        TS_ASSERT_EQUALS(links.size(), 1);
        auto& link = links.at(0);
        TS_ASSERT_EQUALS(link.dest(), &dest);
        TS_ASSERT_EQUALS(link.src(), nullptr);
        TS_ASSERT_EQUALS(link.ctl(), nullptr);
        TS_ASSERT_EQUALS(link.scale(), 0.5f);
    }

    void test_connect_D()
    {
        auto p = Patch()
                .connect(dest)
                ;
        auto links = p.links();
        TS_ASSERT_EQUALS(links.size(), 1);
        auto& link = links.front();
        TS_ASSERT_EQUALS(link.dest(), &dest);
        TS_ASSERT_EQUALS(link.src(), nullptr);
        TS_ASSERT_EQUALS(link.ctl(), nullptr);
        TS_ASSERT_EQUALS(link.scale(), 1);
    }

};
