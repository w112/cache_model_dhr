#include <iostream>
#include <vector>
#include <unordered_set>

#include "CacheSim.h"
#include "attack.h"

int main()
{
    std::unordered_set<uint32_t> skew_set;

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
    evict_ct(cc, eviction, target);
    // evict_ppp(cc, eviction, size, target);

    if(cc->skew){
        skew_set.clear();
        // attacker has no right to access this function, only use for test the successful ratio
        for(int i = 0; i < cc->get_way();i ++){
            skew_set.insert(cc->get_skew_set(target,i));
        }
        for(auto it:eviction)
            std::cout << "slice :" << it << std::endl;

        for(auto ev:eviction){
            for(int i = 0; i < cc->get_way();i++){
                uint32_t ss = cc->get_skew_set(ev,i);
                if(skew_set.count(ss) > 0){
                    std::cout << "S:" << ss << " A:" << ev << std::endl; 
                    break;
                }
            }
        }

    }else{
        std::cout << "target = " << target << ";set = " << cc->get_set(target) << std::endl;
        for(auto ev:eviction)
            std::cout << ev <<" " << cc->get_set(ev)<< std::endl;
    }
        return 0;
}
