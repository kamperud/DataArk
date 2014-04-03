/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "interface.hh"
#include <stdlib.h>
#include <vector>

struct Predictor_entry {
    Addr pc;
    int64_t displacement;
};

#define GHB_SIZE 32
#define MAX_NOF_PREDICTORS 8

static std::vector<Predictor_entry> GHB;






void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */
    DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}


void insert_GHB(Addr pc, int64_t displacement) {
    
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

    if (GHB.empty()) {
        insert_GHB(stat.pc, BLOCK_SIZE);
    }  
    else {
        insert_GHB(prev_pc, stat.mem_addr - prev_mem);
    }
    prev_pc = stat.pc;
    prev_mem = stat.mem_addr;
    
    if (stat.miss) {
        for (int i = GHB.size()-1; i >= 0; i--) {
            if (stat.pc == GHB[i].pc) {
                Addr next_addr = stat.mem_addr;
                for (int j = 1; j <= MAX_NOF_PREDICTORS; j++) {
                    if (i - j < 0) break;
                    Addr prefetch_addr = GHB[i - j].displacement + next_addr;
                    if (prefetch_addr >= 0 && prefetch_addr < MAX_PHYS_MEM_ADDR && !in_cache(prefetch_addr) && !in_mshr_queue(prefetch_addr))
                            issue_prefetch(prefetch_addr);
                    next_addr = prefetch_addr;
                }
            }
        }
    }
    
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
