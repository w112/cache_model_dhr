#ifndef __ATTACK_H__
#define __ATTACK_H__

#include <iostream>
#include <cstring>
#include <list>
#include <random>
#include <chrono>
#include <unordered_set>

#include "CacheSim.h"

uint32_t gene_val();

// [57]
bool evict_group(CacheSim* cache, std::list<uint32_t>& candidate, uint32_t size, uint32_t target,uint32_t split);

// [97]
//void evict_ct(CacheSim* cache, std::list<uint32_t>& candidate, uint32_t target,uint32_t set_size);
void evict_ct(std::vector<CacheSim*> dhr_cpu, std::list<uint32_t>& candidate, uint32_t target,uint32_t set_size);

// [108]
bool evict_ppp(CacheSim* cc, std::list<uint32_t>& candidate, uint32_t can_size, uint32_t target,uint32_t set_size);


bool check(CacheSim* cc, std::list<uint32_t>& candidate, uint32_t target);

bool check_set_skew(CacheSim* cc, std::list<uint32_t>& eviction, uint32_t target);

bool check_set(CacheSim* cc,std::list<uint32_t>& eviction, uint32_t target);

#endif
