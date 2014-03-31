/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "interface.hh"
#include <stdlib.h>
#include <vector>
#include <iostream>

struct Predictor_entry {
    Addr pc;
    int64_t displacement;
};

#define GHB_SIZE 32
#define MAX_NOF_PREDICTORS 2

static std::vector<Predictor_entry> GHB;






void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */
    //DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}


void insert_GHB(Addr pc, int displacement) {
    
    Predictor_entry new_entry;
    new_entry.pc = pc;
    new_entry.displacement = displacement;

    GHB.insert(GHB.begin(), new_entry);
    if (GHB.size() > GHB_SIZE) {
        GHB.erase(GHB.end()-1);
    }
}

void prefetch_access(AccessStat stat)
{
    static Addr prev_mem;
    static Addr prev_pc;
    for (int i = GHB.size()-1; i >= 0; i--) {
        if (stat.pc == GHB[i].pc) {
            Addr next_addr = stat.mem_addr;
            for (int j = 1; j <= MAX_NOF_PREDICTORS; j++) {
                if (i - j < 0) break;
                Addr prefetch_addr = GHB[i - j].displacement + next_addr;
                //if (!in_cache(prefetch_addr) && !in_mshr_queue(prefetch_addr))
                        std::cout << "PC: " << stat.pc << ". Addr: " << prefetch_addr << "\n";
                next_addr = prefetch_addr;
            }
        }
    }

    if (GHB.empty()) {
        insert_GHB(stat.pc, BLOCK_SIZE);
    }
    else {
        int64_t temp= stat.mem_addr;
        int64_t temp2 = prev_mem;
        insert_GHB(prev_pc, temp - temp2);
    }
    prev_pc = stat.pc;
    prev_mem = stat.mem_addr;
    
}



void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}

int main () {

    Addr stride = 5;
    for (int i = 0; i < 32; i++) {
        AccessStat stat;
        stat.pc = i;        /* The address of the instruction that caused the access */
        stat.mem_addr = i;  /* The memory address that was requested */
        stat.time = 0;      /* The simulator time cycle when the request was sent */
        stat.miss = 1;       /* Was this demand access a cache hit (0) or miss (1)? */
        prefetch_access(stat);
            
    }
    for (int i = 0; i < 100; i++) {
        AccessStat stat;
        stat.pc = i;        /* The address of the instruction that caused the access */
        stat.mem_addr = i;  /* The memory address that was requested */
        stat.time = 0;      /* The simulator time cycle when the request was sent */
        stat.miss = 1;       /* Was this demand access a cache hit (0) or miss (1)? */
        prefetch_access(stat);
            
    }
    // for (int j = 0; j < GHB.size(); j++) {
    //     std::cout << "Index: " << GHB[j].pc << ". Displacement: " << GHB[j].displacement << "\n";
    // }
    // std::cout << "Size: " << sizeof(std::vector<Predictor_entry>) + (sizeof(Predictor_entry) * GHB.size()) << "\n";
    return 1;
}