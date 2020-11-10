#include "attack.h"

uint32_t gene_val(){
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 g(seed);

    return g();
}

void gene_rand(std::list<uint32_t> &group, uint32_t size, uint32_t max = 0){
    // generate an set, has no same addr
    std::unordered_set<uint32_t> random_set;
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 g(seed);
    uint32_t value;

    group.clear();

    while(random_set.size() < size){
        if(max == 0)
            value = g();
        else
            value = g()%max;

        random_set.insert(random_set.begin(),value);
    }

    group = std::list<uint32_t>(random_set.begin(),random_set.end());
}


// true : addr is not in cache
bool access_high(CacheSim* cc, uint32_t addr){
    uint32_t delay = cc->delay;
    cc->read(addr);
    if(delay == cc->delay)
        return false;
    else
        return true;
}

bool check(CacheSim* cc, std::list<uint32_t>& candidate, uint32_t target){
    uint32_t latency = 0;

    // traverse to test if this candidate is evicted with target
    for(int i = 0; i < 10; i ++){
        cc->read(target);
    }

    for(auto ev:candidate){
        cc->read(ev);
    }
    latency = cc->delay;
    
    for(int i = 0; i < 10; i ++){
        cc->read(target);
    }

    if(cc->delay == latency){
        return false;
    }else{
        return true;
    }
}

void split_set(std::list<uint32_t>& candidate, std::list<uint32_t>& pick, uint32_t size){
    std::list<uint32_t> index;
    std::unordered_set<uint32_t> index_set;
    gene_rand(index,size,candidate.size());

    pick.clear();

    for(auto i:index)
        index_set.insert(i);

    auto it = candidate.begin();
    for(uint32_t i = 0; i < candidate.size();i ++){
        if(index_set.count(i)){
            pick.push_back(*it);
            it = candidate.erase(it);
        }else
            it++;
    }
}

// delete the self-conflict addr
void reduce_group(CacheSim* cc, std::list<uint32_t>& candidate){
    bool flag = true;
    while(flag){
        flag = false;
        // read all addr
        for(auto ev:candidate)
            cc->read(ev);
        auto it = candidate.begin();
        for(int i = 0; i < candidate.size();i ++){
            if(access_high(cc,*it)){
                flag = true;
                it = candidate.erase(it);
            }else{
                it++;
            }
        }
    }
}

bool cand_init(CacheSim* cache, std::list<uint32_t>& candidate,uint32_t target, uint32_t size){
    gene_rand(candidate,size);
    if(check(cache,candidate,target))
        return true;
    else
        return false;
}

bool evict_group(CacheSim* cc, std::list<uint32_t>& candidate, uint32_t size, uint32_t target,uint32_t split){
    uint32_t step;
    uint32_t loop;
    std::list<uint32_t> picked_set;
 
    uint32_t loop_init = 0;
    uint32_t loop_ev   = 0;
    uint32_t fail_time = 0;

    while(!cand_init(cc,candidate,target,size)){
        candidate.clear();
        loop_init++;
        std::cout << "loop_init = "<< loop_init << std::endl;
        if(loop_init > 100){
            std::cout << "loop_init = "<< loop_init << std::endl;
            std::cout << "failed" << std::endl;
            return false;
        }
    }

    while(candidate.size() > split){
        // cal the step 
        //std::cout << "candidate= " << candidate.size() << std::endl;
        loop_ev ++;
        std::cout << "loop_ev =" << loop_ev << std::endl;
        if(!check(cc,candidate,target)){
            fail_time++;
            if(fail_time > 100)
                break;
        }else{
            fail_time = 0;
            if(candidate.size() > 2*split){
                step = (candidate.size() + split - 1)/ split;
            }else{
                step = 1;
            }
            split_set(candidate,picked_set,step);
            if(!check(cc,candidate,target)){
                candidate.insert(candidate.end(),picked_set.begin(),picked_set.end());
            }

        }
    }

    return true;
}

// used for random policy cache
// conflict eviction
void evict_ct(CacheSim* cc, std::list<uint32_t>& candidate, uint32_t target,uint32_t set_size){
    std::unordered_set<uint32_t> conflict;

    uint32_t val;
    cc->read(target);
    while(conflict.size() < set_size){
        val = gene_val();
        while(val == target){
            val = gene_val();
        }

        cc->read(val);
        if(access_high(cc,target)){
            conflict.insert(val);
        }
    }

    candidate = std::list<uint32_t>(conflict.begin(),conflict.end());
}

// used for ScatterCache
// ppt algorithm
bool evict_ppp(CacheSim* cc, std::list<uint32_t>& candidate, uint32_t can_size, uint32_t target,uint32_t set_size){
    std::list<uint32_t> space_set;

    while(candidate.size() < set_size){
        
        uint32_t loop_init = 0;
        while(!cand_init(cc,space_set,target,can_size)){
            loop_init++;
            if(loop_init > 10000){
                std::cout << "failed" << std::endl;
                return false;
            }
        }
        reduce_group(cc,space_set);

        cc->read(target);
        for(auto ev:space_set){
            if(access_high(cc,ev))
                candidate.push_back(ev);
        }
    }
    return true;
}


// judge the total number is in eviciton set or not
bool check_set(CacheSim* cc,std::list<uint32_t>& eviction, uint32_t target){
    uint32_t set_val = cc->get_set(target);
    uint32_t actual_value = 0; 

    for(auto ev:eviction){
        if(set_val == cc->get_set(ev)){
            actual_value ++;
        }
    }

    if(actual_value < cc->get_way())
        return false;
    else
        return true;

}


bool check_set_skew(CacheSim* cc, std::list<uint32_t>& eviction, uint32_t target){

    std::unordered_set<uint32_t> skew_set;

    if(cc->skew){
        skew_set.clear();
        // attacker has no right to access this function, only use for test the successful ratio
        for(int i = 0; i < cc->get_way();i ++){
            skew_set.insert(cc->get_skew_set(target,i));
        }
        
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

    return true;
}


