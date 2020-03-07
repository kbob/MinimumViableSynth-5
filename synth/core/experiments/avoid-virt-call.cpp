#include <iostream>

class Module {
public:
    virtual ~Module()
    {
        std::cout << "delete module " << typeid(this).name() << std::endl;
    }

    void do_render(State *state, size_t frame_count) const
    {
        do_render<t>
    }

};

template <class T>
class RenderDispatch {
public:
    struct State {};
    void do_render(State *state, size_t frame_count) const
    {
        auto derived = static_cast<T *>(this);
        auto derived_state = static_cast<typename T::State *>(state);
        derived->render(derived_state, frame_count);
    }
};

class Oscillator : public Module {
public:
};

class BLOscillator : public Oscillator, public RenderDispatch<BLOscillator> {
public:
    struct State {
        int note;
    };
    void render(State *state, int count) const
    {}
};

int main()
{
    Oscillator o;
    Module *m = &o;
    static_cast<RenderDispatch *>(m)->do_render();
    return 0;
}
