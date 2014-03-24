/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "interface.hh"
#include <stdlib.h>
#include <vector>

struct Predictor_entry {
    Addr index_addr;
    std::vector<int64_t> predictors;
};

// typedef struct {
//     Predictor_entry *first; /* Predicted mem_addr */
//     int nof_entries; /* Last time called */
//     int last_evicted;
// } Predictor_row;

#define PRED_TABLE_MAX_SIZE 16
#define MAX_NOF_PREDICTORS 5

static std::vector<Predictor_entry> pred_table;



void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */
    DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}

void insert_pred_table(Addr index, int64_t predictor) {
    
    
}

void prefetch_access(AccessStat stat)
{
    
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
