#pragma once
#ifndef HCONE_HPP_INCLUDED
#define HCONE_HPP_INCLUDED
#include "./HC-1/HC-1.hpp"
#include "./HC-2/HC-2.hpp"
#include "./HC-3/HC-3.hpp"

namespace pachde {

struct HcOne
{
    struct Internal;
    Internal * my;

    static HcOne* get();
    void registerHc1(Hc1Module * module);
    void unregisterHc1(Hc1Module * module);
    int Hc1count();
    Hc1Module* getSoleHc1();
    Hc1Module* getHc1(std::function<bool(Hc1Module* const&)> pred);
    Hc1Module* getHc1(int64_t id);

private:
    HcOne();
};

}
#endif