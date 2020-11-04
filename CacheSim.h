#ifndef __CacheSim__H__
#define __CacheSim__H__

#include <iostream>
#include <cstdlib> // random value
#include <ctime> // time seed
#include <cmath> // log2
#include <random>
#include <chrono>
#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>

// the parameter for different configuration
#define SET         1
#define WAY         2
#define WIDTH       4
#define REPLACER    8
#define ENCRYPT     16
#define SKEW        32

// show the output information or not
#define DEGUG 0

using std::cout;
using std::endl;


// define the candidate for different policy
typedef enum ReplacePolicy{ LRU, FIFO, RANDOM } RP;

class CacheSim{
private:
    std::vector<std::vector<uint32_t>> *meta; // store the data in cache
    uint32_t nway;
    uint32_t nset;
    uint32_t nwidth;
    RP replacer;

    // key for encryption
    uint32_t key = 0;
    // skew function for different way
    std::vector<uint32_t> skew_fun;

    uint32_t skew_p;
    // store information of replacement policy
    std::unordered_map<uint32_t,std::unordered_set<uint32_t>> free_map;
    std::unordered_map<uint32_t,std::list<uint32_t>> used_map;

public:
    // the policy for different cache  
    bool encry = false;
    bool skew  = false;
    bool reconf = false;
    
    // statistic value 
    uint64_t delay;
    uint64_t access_time;

    // dynamic condition for reconfiguration
    uint32_t threshold = 0;
    uint32_t param = 0;

    CacheSim(std::vector<std::vector<uint32_t>> *meta,uint32_t skew_p = 2, uint32_t nway = 16, uint32_t nset = 1024, uint32_t nwidth = 32,
             RP replacer = LRU)
        :meta(meta),nway(nway),nset(nset),nwidth(nwidth),replacer(replacer),skew_p(skew_p)
    {}

    // get the statistic value
    void init_delay()       {delay          = 0;}
    void init_access_time() {access_time    = 0;}
    uint64_t get_delay()    {return delay;}
    uint64_t get_access()   {return access_time;}

    void sh_cfg();
    void sh_rp();
    void sh_slice(uint32_t addr);

    void init_cfg();
    void init_size();
    void init_rpinfo();
    void init_ac_time(){access_time = 0;}

    uint32_t get_way(){return nway;}
    uint32_t get_index(uint32_t addr); // use for encrypted input
    uint32_t get_set(uint32_t addr);
    uint32_t get_skew_set(uint32_t addr,uint32_t _way);
    uint32_t get_tag(uint32_t addr);
    uint32_t get_meta(uint32_t _set,uint32_t _way);

    
    uint32_t encrypt(uint32_t val,uint32_t key);
    void set_meta(uint32_t _set, uint32_t _way, uint32_t _meta);

    bool hit(uint32_t addr, uint32_t *_set,uint32_t *_way);
    bool match(uint32_t addr,uint32_t _meta);
    void access(uint32_t _set,uint32_t _way);
    void read(uint32_t addr);
    void flush(uint32_t addr);
    
    uint32_t replace(uint32_t _set);
    bool replace_skew(uint32_t addr, uint32_t *_set, uint32_t *_way);

    // reconfiguration
    void mod_nset(uint32_t nset) {this->nset = nset;}
    void mod_nway(uint32_t nway) {this->nway = nway;}
    void mod_nwidth(uint32_t nwidth) {this->nwidth = nwidth;}
    void mod_rep(RP replacer) {this->replacer = replacer;}

    void reconfiguration();

    ~CacheSim(){}
};

#endif
