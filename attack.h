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
void evict_group(CacheSim* cache, std::list<uint32_t>& candidate, uint32_t size, uint32_t target,uint32_t split);

// [97]
void evict_ct(CacheSim* cache, std::list<uint32_t>& candidate, uint32_t target);

// [108]
void evict_ppp(CacheSim* cc, std::list<uint32_t>& candidate, uint32_t size, uint32_t target);

#endif
