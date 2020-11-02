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

void cand_init(CacheSim* cache, std::list<uint32_t>& candidate,uint32_t target, uint32_t size){
    gene_rand(candidate,size);
    while(!check(cache,candidate,target));
}

void evict_group(CacheSim* cc, std::list<uint32_t>& candidate, uint32_t size, uint32_t target,uint32_t split){
    uint32_t step;
    uint32_t loop;
    std::list<uint32_t> picked_set;

    cand_init(cc,candidate,target,size);

    while(candidate.size() > cc->get_way()){
        // cal the step 
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

// used for random policy cache
// conflict eviction
void evict_ct(CacheSim* cc, std::list<uint32_t>& candidate, uint32_t target){
    std::unordered_set<uint32_t> conflict;

    uint32_t val;
    cc->read(target);
    while(conflict.size() < cc->get_way()){
        val = gene_val();
        cc->read(val);
        if(access_high(cc,target)){
            conflict.insert(val);
        }
    }

    candidate = std::list<uint32_t>(conflict.begin(),conflict.end());
}

// used for ScatterCache
// ppt algorithm
void evict_ppt(CacheSim* cc, std::list<uint32_t>& candidate, uint32_t size, uint32_t target){
    std::list<uint32_t> space_set;

    while(candidate.size() < cc->get_way()){
        cand_init(cc,space_set,target,size);
        reduce_group(cc,space_set);

        cc->read(target);
        for(auto ev:space_set){
            if(access_high(cc,ev))
                candidate.push_back(ev);
        }
    }
}



