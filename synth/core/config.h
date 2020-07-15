#ifndef CORE_CONFIG_included
#define CORE_CONFIG_included

#include <cstdint>
#include <limits>
#include <typeindex>

#include "synth/core/sizes.h"
#include "synth/util/fixed-map.h"

class Control;
class Module;
class Synth;
class Timbre;
class Voice;


// -- Config - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
//  Config is a container for runtime configuration information.
//  When the `Synth` is finalized, the Config object is passed to all
//  its constituent Timbre, Voice, Module, and Control objects.
//  Objects may access whatever subsystems they need to configure
//  themselves.

class Config {

public:

    typedef std::uint32_t sample_rate_type;

    class Subsystem {
    public:
        virtual ~Subsystem() = default;

        virtual void pre_configure(Synth&) const {}
        virtual void post_configure(Synth&) const {}

        virtual void pre_configure(Timbre&) const {}
        virtual void post_configure(Timbre&) const {}

        virtual void pre_configure(Voice&) const {}
        virtual void post_configure(Voice&) const {}

        virtual void pre_configure(Control&) const {}
        virtual void post_configure(Control&) const {}

        virtual void pre_configure(Module&) const {}
        virtual void post_configure(Module&) const {}
    };

    Config()
    : m_sample_rate{NO_RATE}
    {}

    // N.B., `get` throws `std::out_of_range` if the subsystem is
    // not registered.
    template <class T>
    typename std::enable_if<std::is_base_of<Subsystem, T>::value, T>::type&
    get() const
    {
        Subsystem *p = m_subs.at(typeid(T));
        assert(dynamic_cast<T *>(p));
        return *static_cast<T *>(p);
    }

    void
    register_subsystem(Subsystem& sub)
    {
        m_subs[typeid(sub)] = &sub;
    }

    // helper because everybody needs the sample rate.
    sample_rate_type
    sample_rate() const
    {
        assert(m_sample_rate != NO_RATE);
        return m_sample_rate;
    }

    void
    set_sample_rate(sample_rate_type rate)
    {
        m_sample_rate = rate;
    }

    template <class Obj>
    void pre_configure(Obj& obj) const
    {
        for (auto& sub: m_subs)
            sub.second->pre_configure(obj);
    }

    template <class Obj>
    void post_configure(Obj& obj) const
    {
        for (auto& sub: m_subs)
            sub.second->post_configure(obj);
    }

private:

    typedef std::type_index key_type;

    static const sample_rate_type NO_RATE =
    std::numeric_limits<sample_rate_type>::max();

    sample_rate_type m_sample_rate;
    fixed_map<key_type, Subsystem *, MAX_SUBSYSTEMS> m_subs;

};

#endif /* !CORE_CONFIG_included */
