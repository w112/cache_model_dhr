#include <iostream>
#include <cstdlib>
#include <random>
#include <chrono>
#include <vector>
#include <list>

#include "CacheSim.h"


int main()
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 g1 (seed);

    uint32_t a;
    std::vector<std::vector<uint32_t>> mm;

    CacheSim* cache = new CacheSim(&mm);
    cache->init_size();

    a = g1();
    cache->read(a);
    cache->sh_slice(a);

    cache->flush(a);
    cache->sh_slice(a);
    cache->read(a);
    cache->sh_slice(a);

    return 0;
}
