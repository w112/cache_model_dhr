#include <iostream>
#include <vector>
#include <unordered_set>

#include "CacheSim.h"
#include "attack.h"

int main()
{
    std::vector<std::vector<uint32_t>> mm;
    CacheSim* cc = new CacheSim(&mm);

    cc->skew = true;
    cc->init_size();
    cc->init_cfg();
    cc->mod_rep(RANDOM);

    uint32_t split = 17;
    uint32_t size = 10240;

    uint32_t target = gene_val();
    std::list<uint32_t> eviction;
    //evict_group(cc,eviction,size,target,split);
    // evict_ct(cc, eviction, target);
    evict_ppp(cc, eviction, size, target);
    std::cout << "target = " << target << ";set = " << cc->get_set(target) << std::endl;
    for(auto ev:eviction)
        std::cout << ev <<" " << cc->get_set(ev)<< std::endl;

    return 0;
}
