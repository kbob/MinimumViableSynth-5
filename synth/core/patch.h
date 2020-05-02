#ifndef PATCH_included
#define PATCH_included

#include "synth/core/config.h"
#include "synth/core/link.h"
#include "synth/util/noalloc.h"


// -- Patch -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
// A Patch has:
//     a set of Links
//     values for all controls (not yet)

class Patch {

public:

    typedef fixed_vector<Link, MAX_LINKS> link_vector;

    const link_vector& links() const { return m_links; }

    // Many ways to connect.
    //     connect(dest, src, ctl, scale)
    //     connect(dest, src, ctl)
    //     connect(dest, src, port, scale)
    //     connect(dest, src, port)
    //     connect(dest, port, scale)
    //     connect(dest, port)
    //     connect(dest, ctl, scale)
    //     connect(dest, ctl)
    //     connect(dest, scale)
    //     connect(dest)
    template <class D, class S, class CT, class CE>
    Patch& connect(Input<D>& dest,
                   Output<S>& src,
                   ControlType<CT, CE>& ctl,
                   SCALE_TYPE scale = DEFAULT_SCALE)
    {
        m_links.emplace_back(&dest, &src, &ctl.out, scale);
        return *this;
    }

    template <class D, class S, class C>
    Patch& connect(Input<D>& dest,
                   Output<S>& src,
                   Output<C>& ctl,
                   SCALE_TYPE scale = DEFAULT_SCALE)
    {
        m_links.emplace_back(&dest, &src, &ctl, scale);
        return *this;
    }

    template <class D, class S>
    Patch& connect(Input<D>& dest,
                   Output<S>& src,
                   SCALE_TYPE scale = DEFAULT_SCALE)
    {
        m_links.emplace_back(&dest, &src, nullptr, scale);
        return *this;
    }

    template <class D, class CT, class CE>
    Patch& connect(Input<D>& dest,
                   ControlType<CT, CE>& ctl,
                   SCALE_TYPE scale = DEFAULT_SCALE)
    {
        m_links.emplace_back(&dest, nullptr, &ctl.out, scale);
        return *this;
    }

    template <class D>
    Patch& connect(Input<D>& dest,
                   SCALE_TYPE scale = DEFAULT_SCALE)
    {
        m_links.emplace_back(&dest, nullptr, nullptr, scale);
        return *this;
    }

    // XXX need disconnect methods?

private:

    link_vector m_links;

    friend class patch_unit_test;

};

#endif /* !PATCH_inclued */
