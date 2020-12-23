#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include "CacheSim.h"
#include "attack.h"

int main(int argc, char* argv[])
{
    if(argc != 6){
        std::cout << "Usage: split testN param rate cpu" << std::endl;
        return 0;
    }
    
    uint32_t split = std::stoi(std::string(argv[1]));
    uint32_t testN = std::stoi(std::string(argv[2]));
    uint32_t param = std::stoi(std::string(argv[3]));
    double   rate = std::stoi(std::string(argv[4]));
    uint32_t cpu = std::stoi(std::string(argv[5]));

    std::vector<std::vector<uint32_t>> mm;
    std::list<uint32_t> eviction;
    std::vector<CacheSim*> dhr_cpu;
    // CacheSim* cc = new CacheSim(&mm);

    for(int i = 0; i < cpu; i ++){
        CacheSim* cc = new CacheSim();
        cc->reconf = false;
        cc->encry = true;
        dhr_cpu.push_back(cc);
    }

    //cc->reconf = true;
    //cc->encry  = true;
    //cc->param = param;
    //cc->rate = rate;


    uint64_t total_access = 0;
    double   total_succ   = 0;

    for(int l = 0; l < testN;l ++){
        std::cout << "round:" << l << std::endl;
 
        for(auto cpux:dhr_cpu){
            cpux->init_cfg();
            cpux->init_size();
            cpux->init_ac_time();
        }

        eviction.clear();    
        uint32_t target = gene_val();
        // evict_group(cc,eviction,10240,target,split);
        // evict_ppp(cc, eviction, 10240, target, ev_set);
        evict_ct(dhr_cpu, eviction, target,split);
        total_access += dhr_cpu[0]->access_time;

        for(auto cpux:dhr_cpu){
            cpux->init_ac_time();
        }
    std::cout << (total_access / testN) << std::endl;
    std::cout << total_succ << std::endl;
    }

    return 0;
}
