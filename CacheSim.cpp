#include "CacheSim.h"
#define NUM_16 ((uint16_t)1<<16)

uint32_t get_rand_val(uint32_t val){
    srand((unsigned)time(NULL));
    return rand()%val;
}

uint32_t gene_key(){
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 g(seed);
    uint32_t key = g();
    return key;
}

// encryption function
uint32_t CacheSim::encrypt(uint32_t num,uint32_t key){
    // uint32_t offset = log2(nset);
    // return val ^ key ^ (val >> offset);
    uint32_t key_set[] = {key + 1,key + 2, key + 3, key + 4};

    uint16_t lower = (uint16_t)(num & (NUM_16 - 1));
    uint16_t upper = (uint16_t)(num >> 16);

    uint32_t v0 = lower;
    uint32_t v1 = upper;

    uint32_t sum = 0;
    uint32_t delta = 0x9E3779B9;

    uint32_t val;

    for(int i = 0; i < 32; i ++){
        v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key_set[sum & 3]);
        sum += delta;
        v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key_set[(sum >> 11) & 3]);
    }
    lower = v0;
    upper = v1;

    val = ((uint32_t) upper << 16) | lower;
    return val;
}


void CacheSim::sh_rp(){
    switch (replacer){
        case LRU: cout << "replacer:lru"<< endl;break;
        case FIFO:cout << "replacer:fifo" << endl;break;
        case RANDOM:cout <<"replacer:random"<<endl;break;
        default:  cout <<"replacer:lru" <<endl;break;
    }
}

void CacheSim::sh_cfg(){
    cout << "------configuration------"<< endl;
    cout << "nset:" << nset     << endl;
    cout << "nway:" << nway     << endl;
    cout << "nwidth"<< nwidth   << endl;
    sh_rp();
    cout << "-------------------------"<< endl;
}

void CacheSim::sh_slice(uint32_t addr){
    uint32_t _set;
    uint32_t _meta;
    if(skew){
        for(int i = 0; i < nway; i ++){
            _set = get_skew_set(addr,i);
            _meta = get_meta(_set,i);
            std::cout << _meta << std::endl;
        }
    }else{
        _set = get_set(addr);
        for(int i = 0; i < nway; i++)
            std::cout << get_meta(_set,i)<< std::endl;
    }
}

void CacheSim::init_cfg(){
    this->nway = 16;
    this->nset = 1024;
    this->nwidth = 32;
    this->replacer = LRU;
    if(encry){
        key = gene_key();
    }
    if(skew){
        for(int i = 0; i < skew_p; i++)
            skew_fun.push_back(gene_key());
    }
}

void CacheSim::init_size(){
    (*meta).resize(nset);
    for(int i = 0; i < (*meta).size(); i++){
        (*meta)[i].resize(nway);
    }
}

void CacheSim::init_rpinfo(){
    for(int i = 0; i < nset; i++){
        for(int j = 0; j < nway; j++){
            free_map[i].insert(j);
            if(get_meta(i,j) != 0){
                free_map[i].erase(j);
                used_map[i].push_back(j);
            }
        }
    }

}

uint32_t CacheSim::get_index(uint32_t addr){
    uint32_t offset = log2(nwidth);
    return (addr >> offset);
}

uint32_t CacheSim::get_set(uint32_t addr){
    uint32_t offset = log2(nwidth);
    if(!encry)
        return ((addr >> offset) & (nset - 1));
    else
        return encrypt( (addr >> offset),key ) & (nset - 1);
}

uint32_t CacheSim::get_skew_set(uint32_t addr, uint32_t _way){
    // according the paration
    // return ( get_index(addr) ^ skew_fun[(_way % skew_p)] ) & (nset - 1);
    return encrypt( get_index(addr),skew_fun[(_way % skew_p)] ) & (nset - 1);
           
}

uint32_t CacheSim::get_tag(uint32_t addr){
    uint32_t offset = log2(nwidth*nset);
    return (addr >> offset);
}

uint32_t CacheSim::get_meta(uint32_t _set,uint32_t _way){
    return (*meta)[_set][_way];
}

void CacheSim::set_meta(uint32_t _set, uint32_t _way, uint32_t _meta){
    // when set the value, we assume that only if this value is not contain in cache,
    // there will be latency
    // however, there is no time to access cache
    delay += 1; 
    (*meta)[_set][_way] = _meta;
}

bool CacheSim::match(uint32_t addr, uint32_t m_meta){
    return get_tag(addr) == get_tag(m_meta);
}

bool CacheSim::hit(uint32_t addr,uint32_t *_set,uint32_t *_way){
    if(skew){
        uint32_t skew_set;
        uint32_t _meta;
        for(int i = 0; i < nway; i++){
            skew_set = get_skew_set(addr,i);
            _meta = get_meta(skew_set,i);
            if(match(_meta,addr)){
               *_way = i;
               *_set = skew_set;
               return true;
            }
        }
        return false;

    }else{
        *_set = get_set(addr);
        for(int i = 0; i < nway; i++){
            uint32_t _meta = get_meta(*_set,i);
            if(match(_meta,addr)){
                *_way = i;
                return true;
            }
        }
        return false;    
    }
}

void CacheSim::access(uint32_t _set,uint32_t _way){
    switch (replacer){
        case LRU:
            if(free_map[_set].count(_way)){
                free_map[_set].erase(_way);
                used_map[_set].push_back(_way);
            }else{
                used_map[_set].remove(_way);
                used_map[_set].push_back(_way);
            }
            break;
        case FIFO:
            if(free_map[_set].count(_way)){
                free_map[_set].erase(_way);
                used_map[_set].push_back(_way);
            }
            break;
        case RANDOM:
            if(free_map[_set].count(_way)){
                free_map[_set].erase(_way);
            }
            break;
    }
    access_time++;
    if(reconf){
        if(access_time > threshold){
            reconfiguration();
        }
    }
}

bool CacheSim::replace_skew(uint32_t addr, uint32_t *_set, uint32_t *_way){
    // for skew cache, we use random replacement policy
    uint32_t skew_set;
    uint32_t _meta;

    for(int i = 0; i < nway; i++){
        skew_set = get_skew_set(addr,i);
        _meta = get_meta(skew_set,i);
        if(_meta == 0){
            *_set = skew_set;
            *_way = i;
            return true;
        }
        // has no free space
    }
    *_way = get_rand_val(nway);
    *_set = get_skew_set(addr,*_way);
    return false;
}

uint32_t CacheSim::replace(uint32_t _set){
    if(!free_map.count(_set))
        for(uint32_t i = 0;i < nway;i ++) free_map[_set].insert(i);

    switch (replacer){
        case LRU:
        case FIFO:
            if(free_map[_set].size() > 0)
                return *(free_map[_set].begin());
            else
                return used_map[_set].front();
        case RANDOM:
            if(free_map[_set].size() > 0)
                return *(free_map[_set].begin());
            else
                return get_rand_val(nway);
    }
}

void CacheSim::flush(uint32_t addr){
    uint32_t _set;
    uint32_t _way;

    if(hit(addr,&_set,&_way)){
        set_meta(_set,_way,0);
        free_map[_set].insert(_way);
        used_map[_set].remove(_way);
    }
}

void CacheSim::read(uint32_t addr){
    uint32_t _set;
    uint32_t _way;

    if(!hit(addr,&_set,&_way)){
        if(skew){
            replace_skew(addr,&_set,&_way);
        }else{
            _set = get_set(addr);
            _way = replace(_set);
        }
        set_meta(_set,_way,addr);
    }
    access(_set,_way);
}

void CacheSim::reconfiguration(){
 
    uint32_t set_pool[]   = {64,128,256,512,1024,2048,4096};
    uint32_t way_pool[]   = {4,8,16,32,64};
    uint32_t width_pool[] = {16,32,64,128};
    RP     replace_pool[] = {LRU,FIFO,RANDOM};
           
    int size_set        = sizeof(set_pool)/sizeof(set_pool[0]);
    int size_way        = sizeof(way_pool)/sizeof(way_pool[0]);
    int size_width      = sizeof(width_pool)/sizeof(width_pool[0]);
    int size_replace    = sizeof(replace_pool)/sizeof(replace_pool[0]);


    if((param & SET) == SET){ mod_nset(set_pool[rand()%size_set]); }

    if((param & WAY) == WAY){ mod_nway(way_pool[rand()%size_way]); }
    
    if((param & WIDTH) == WIDTH){ mod_nwidth(width_pool[rand()%size_width]); }

    if((param & REPLACER) == REPLACER) { mod_rep(replace_pool[rand()%size_replace]); }
    // 
    if((param & ENCRYPT) == ENCRYPT){ key = gene_key(); }

    if((param & SKEW) == SKEW){ 
        for(int i = 0; i < skew_p; i++){
            skew_fun[i] = gene_key();
        }
    }
    if(param == 1 || param == 2 || param == 4){ init_size(); } 
    if(param == 8){ init_rpinfo(); }
}
