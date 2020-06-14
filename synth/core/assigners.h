#ifndef ASSIGNERS_included
#define ASSIGNERS_included

class Voice;

class Assigner {

public:

    virtual ~Assigner() = default;

    // returns nullptr if no voice is immediately available.
    virtual Voice *assign_idle_voice() = 0;
    virtual Voice *choose_voice_to_steal() = 0;

};

#endif /* !ASSIGNERS_included */
