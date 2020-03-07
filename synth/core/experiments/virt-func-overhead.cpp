#include <array>
#include <iostream>

#include <ctime>

constexpr size_t OBJECT_COUNT = 100;
constexpr size_t REP_COUNT = 1000000000;

class Base {
public:
    virtual ~Base() {}
    virtual float vm() const = 0;
};

//extern void force_call();
#define force_call() ((void)0)

template <int n>
class Derived : public Base {
public:
    virtual float vm() const { force_call(); return n; }
    float m() const { force_call(); return n; }
};

template <size_t N>
float sum_virtual(std::array<Base *, N> objects)
{
    float sum = 0;
    for (int i = 0; i < REP_COUNT; i++) {
        float s0 = 0;
        for (size_t j = 0; j < objects.size(); j++)
            s0 += objects[j]->vm();
        sum += s0;
    }
    return sum;
}

template <class T>
float sum_generic(const T& objects)
{
    float sum = 0;
    for (int i = 0; i < REP_COUNT; i++) {
        float s0 = 0;
        s0 += std::get<0>(objects).m();
        s0 += std::get<1>(objects).m();
        s0 += std::get<2>(objects).m();
        s0 += std::get<3>(objects).m();
        s0 += std::get<4>(objects).m();
        s0 += std::get<5>(objects).m();
        s0 += std::get<6>(objects).m();
        s0 += std::get<7>(objects).m();
        s0 += std::get<8>(objects).m();
        s0 += std::get<9>(objects).m();
        s0 += std::get<10>(objects).m();
        s0 += std::get<11>(objects).m();
        s0 += std::get<12>(objects).m();
        s0 += std::get<13>(objects).m();
        s0 += std::get<14>(objects).m();
        s0 += std::get<15>(objects).m();
        s0 += std::get<16>(objects).m();
        s0 += std::get<17>(objects).m();
        s0 += std::get<18>(objects).m();
        s0 += std::get<19>(objects).m();
        s0 += std::get<20>(objects).m();
        s0 += std::get<21>(objects).m();
        s0 += std::get<22>(objects).m();
        s0 += std::get<23>(objects).m();
        s0 += std::get<24>(objects).m();
        s0 += std::get<25>(objects).m();
        s0 += std::get<26>(objects).m();
        s0 += std::get<27>(objects).m();
        s0 += std::get<28>(objects).m();
        s0 += std::get<29>(objects).m();
        s0 += std::get<30>(objects).m();
        s0 += std::get<31>(objects).m();
        s0 += std::get<32>(objects).m();
        s0 += std::get<33>(objects).m();
        s0 += std::get<34>(objects).m();
        s0 += std::get<35>(objects).m();
        s0 += std::get<36>(objects).m();
        s0 += std::get<37>(objects).m();
        s0 += std::get<38>(objects).m();
        s0 += std::get<39>(objects).m();
        s0 += std::get<40>(objects).m();
        s0 += std::get<41>(objects).m();
        s0 += std::get<42>(objects).m();
        s0 += std::get<43>(objects).m();
        s0 += std::get<44>(objects).m();
        s0 += std::get<45>(objects).m();
        s0 += std::get<46>(objects).m();
        s0 += std::get<47>(objects).m();
        s0 += std::get<48>(objects).m();
        s0 += std::get<49>(objects).m();
        s0 += std::get<50>(objects).m();
        s0 += std::get<51>(objects).m();
        s0 += std::get<52>(objects).m();
        s0 += std::get<53>(objects).m();
        s0 += std::get<54>(objects).m();
        s0 += std::get<55>(objects).m();
        s0 += std::get<56>(objects).m();
        s0 += std::get<57>(objects).m();
        s0 += std::get<58>(objects).m();
        s0 += std::get<59>(objects).m();
        s0 += std::get<60>(objects).m();
        s0 += std::get<61>(objects).m();
        s0 += std::get<62>(objects).m();
        s0 += std::get<63>(objects).m();
        s0 += std::get<64>(objects).m();
        s0 += std::get<65>(objects).m();
        s0 += std::get<66>(objects).m();
        s0 += std::get<67>(objects).m();
        s0 += std::get<68>(objects).m();
        s0 += std::get<69>(objects).m();
        s0 += std::get<70>(objects).m();
        s0 += std::get<71>(objects).m();
        s0 += std::get<72>(objects).m();
        s0 += std::get<73>(objects).m();
        s0 += std::get<74>(objects).m();
        s0 += std::get<75>(objects).m();
        s0 += std::get<76>(objects).m();
        s0 += std::get<77>(objects).m();
        s0 += std::get<78>(objects).m();
        s0 += std::get<79>(objects).m();
        s0 += std::get<80>(objects).m();
        s0 += std::get<81>(objects).m();
        s0 += std::get<82>(objects).m();
        s0 += std::get<83>(objects).m();
        s0 += std::get<84>(objects).m();
        s0 += std::get<85>(objects).m();
        s0 += std::get<86>(objects).m();
        s0 += std::get<87>(objects).m();
        s0 += std::get<88>(objects).m();
        s0 += std::get<89>(objects).m();
        s0 += std::get<90>(objects).m();
        s0 += std::get<91>(objects).m();
        s0 += std::get<92>(objects).m();
        s0 += std::get<93>(objects).m();
        s0 += std::get<94>(objects).m();
        s0 += std::get<95>(objects).m();
        s0 += std::get<96>(objects).m();
        s0 += std::get<97>(objects).m();
        s0 += std::get<98>(objects).m();
        s0 += std::get<99>(objects).m();
        sum += s0;
    }
    return sum;
}

class timer {
public:
    void start()
    {
        int err = clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_time);
        assert(err == 0);
    }
    void stop()
    {
        int err = clock_gettime(CLOCK_THREAD_CPUTIME_ID, &stop_time);
        assert(err == 0);
    }
    double duration()
    {
        double nsec = stop_time.tv_nsec - start_time.tv_nsec;
        double sec = stop_time.tv_sec - start_time.tv_sec;
        return sec + nsec / 1000000000.0;
    }
private:
    struct timespec start_time;
    struct timespec stop_time;
};

int main()
{
    std::array<Base *, OBJECT_COUNT> v_objects = {
        new Derived<0>(),
        new Derived<1>(),
        new Derived<2>(),
        new Derived<3>(),
        new Derived<4>(),
        new Derived<5>(),
        new Derived<6>(),
        new Derived<7>(),
        new Derived<8>(),
        new Derived<9>(),
        new Derived<10>(),
        new Derived<11>(),
        new Derived<12>(),
        new Derived<13>(),
        new Derived<14>(),
        new Derived<15>(),
        new Derived<16>(),
        new Derived<17>(),
        new Derived<18>(),
        new Derived<19>(),
        new Derived<20>(),
        new Derived<21>(),
        new Derived<22>(),
        new Derived<23>(),
        new Derived<24>(),
        new Derived<25>(),
        new Derived<26>(),
        new Derived<27>(),
        new Derived<28>(),
        new Derived<29>(),
        new Derived<30>(),
        new Derived<31>(),
        new Derived<32>(),
        new Derived<33>(),
        new Derived<34>(),
        new Derived<35>(),
        new Derived<36>(),
        new Derived<37>(),
        new Derived<38>(),
        new Derived<39>(),
        new Derived<40>(),
        new Derived<41>(),
        new Derived<42>(),
        new Derived<43>(),
        new Derived<44>(),
        new Derived<45>(),
        new Derived<46>(),
        new Derived<47>(),
        new Derived<48>(),
        new Derived<49>(),
        new Derived<50>(),
        new Derived<51>(),
        new Derived<52>(),
        new Derived<53>(),
        new Derived<54>(),
        new Derived<55>(),
        new Derived<56>(),
        new Derived<57>(),
        new Derived<58>(),
        new Derived<59>(),
        new Derived<60>(),
        new Derived<61>(),
        new Derived<62>(),
        new Derived<63>(),
        new Derived<64>(),
        new Derived<65>(),
        new Derived<66>(),
        new Derived<67>(),
        new Derived<68>(),
        new Derived<69>(),
        new Derived<70>(),
        new Derived<71>(),
        new Derived<72>(),
        new Derived<73>(),
        new Derived<74>(),
        new Derived<75>(),
        new Derived<76>(),
        new Derived<77>(),
        new Derived<78>(),
        new Derived<79>(),
        new Derived<80>(),
        new Derived<81>(),
        new Derived<82>(),
        new Derived<83>(),
        new Derived<84>(),
        new Derived<85>(),
        new Derived<86>(),
        new Derived<87>(),
        new Derived<88>(),
        new Derived<89>(),
        new Derived<90>(),
        new Derived<91>(),
        new Derived<92>(),
        new Derived<93>(),
        new Derived<94>(),
        new Derived<95>(),
        new Derived<96>(),
        new Derived<97>(),
        new Derived<98>(),
        new Derived<99>(),
    };
    auto g_objects = std::make_tuple(
        Derived<0>(),
        Derived<1>(),
        Derived<2>(),
        Derived<3>(),
        Derived<4>(),
        Derived<5>(),
        Derived<6>(),
        Derived<7>(),
        Derived<8>(),
        Derived<9>(),
        Derived<10>(),
        Derived<11>(),
        Derived<12>(),
        Derived<13>(),
        Derived<14>(),
        Derived<15>(),
        Derived<16>(),
        Derived<17>(),
        Derived<18>(),
        Derived<19>(),
        Derived<20>(),
        Derived<21>(),
        Derived<22>(),
        Derived<23>(),
        Derived<24>(),
        Derived<25>(),
        Derived<26>(),
        Derived<27>(),
        Derived<28>(),
        Derived<29>(),
        Derived<30>(),
        Derived<31>(),
        Derived<32>(),
        Derived<33>(),
        Derived<34>(),
        Derived<35>(),
        Derived<36>(),
        Derived<37>(),
        Derived<38>(),
        Derived<39>(),
        Derived<40>(),
        Derived<41>(),
        Derived<42>(),
        Derived<43>(),
        Derived<44>(),
        Derived<45>(),
        Derived<46>(),
        Derived<47>(),
        Derived<48>(),
        Derived<49>(),
        Derived<50>(),
        Derived<51>(),
        Derived<52>(),
        Derived<53>(),
        Derived<54>(),
        Derived<55>(),
        Derived<56>(),
        Derived<57>(),
        Derived<58>(),
        Derived<59>(),
        Derived<60>(),
        Derived<61>(),
        Derived<62>(),
        Derived<63>(),
        Derived<64>(),
        Derived<65>(),
        Derived<66>(),
        Derived<67>(),
        Derived<68>(),
        Derived<69>(),
        Derived<70>(),
        Derived<71>(),
        Derived<72>(),
        Derived<73>(),
        Derived<74>(),
        Derived<75>(),
        Derived<76>(),
        Derived<77>(),
        Derived<78>(),
        Derived<79>(),
        Derived<80>(),
        Derived<81>(),
        Derived<82>(),
        Derived<83>(),
        Derived<84>(),
        Derived<85>(),
        Derived<86>(),
        Derived<87>(),
        Derived<88>(),
        Derived<89>(),
        Derived<90>(),
        Derived<91>(),
        Derived<92>(),
        Derived<93>(),
        Derived<94>(),
        Derived<95>(),
        Derived<96>(),
        Derived<97>(),
        Derived<98>(),
        Derived<99>()
        );
    double v_delta_per = 0;
    double g_delta_per = 0;
    {
        timer tim;
        tim.start();
        float vsum = sum_virtual(v_objects);
        tim.stop();
        double delta = tim.duration();
        double delta_per = delta / OBJECT_COUNT / REP_COUNT;
        std::cout << "virtual sum = " << vsum << std::endl;
        std::cout << "time: " << delta << ", " << delta_per << std::endl;
        v_delta_per = delta_per;
    }
    {
        timer tim;
        tim.start();
        float gsum = sum_generic(g_objects);
        tim.stop();
        double delta = tim.duration();
        double delta_per = delta / OBJECT_COUNT / REP_COUNT;
        std::cout << "generic sum = " << gsum << std::endl;
        std::cout << "time: " << delta << ", " << delta_per << std::endl;
        g_delta_per = delta_per;
    }
    std::cout << "diff = " << v_delta_per - g_delta_per << std::endl;
    return 0;
}
