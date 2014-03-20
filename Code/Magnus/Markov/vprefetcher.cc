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
    std::vector<Addr> predictors;
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

void insert_pred_table(Addr index, Addr predictor) {
    
    /** 
        See if index is already in the predictor table. 
        If so, move in front                        
     **/
    for (int i = 0; i < pred_table.size(); i++) {

        if (pred_table[i].index_addr == index) {
            /** 
                See if the predictor is already predicted. 
                If so, move in front 
            **/
            int nof_preds = pred_table[i].predictors.size();
            for (int j = 0; j < nof_preds; j++) {
                if (pred_table[i].predictors[j] == predictor) {
                    Addr temp = pred_table[i].predictors[0];
                    pred_table[i].predictors[0] = pred_table[i].predictors[j];
                    pred_table[i].predictors[j] = temp;
                    break;
                }
                else if (j == nof_preds -1) {
                    pred_table[i].predictors.insert(pred_table[i].predictors.begin(), predictor);
                    if (nof_preds > MAX_NOF_PREDICTORS) {
                        pred_table[i].predictors.erase(nof_preds);
                    }
                }
            }
            Predictor_entry temp = pred_table[0];
            pred_table[0] = pred_table[i];
            pred_table[i] = temp;
                    

            break;

        }
        /** Was not already in table, insert it **/
        else if (i == pred_table.size() -1) {
            Predictor_entry new_entry;
            new_entry.index_addr = index;
            new_entry.predictors.insert(new_entry.predictors.begin(), predictor);
            pred_table.insert(pred_table.begin(), new_entry);
            if (pred_table.size() >= PRED_TABLE_MAX_SIZE) {
                pred_table.erase(PRED_TABLE_MAX_SIZE);
            } 
        }
    }
}

void prefetch_access(AccessStat stat)
{
    static Addr prev_mem_miss;

    if (pred_table.empty()) {
        insert_pred_table(stat.mem_addr, stat.mem_addr + BLOCK_SIZE);
        prev_mem_miss = stat.mem_addr;
    }
    else if (stat.miss) {
        insert_pred_table(prev_mem_miss, stat.mem_addr);
        prev_mem_miss = stat.mem_addr;
    }
    for (int i = 0; i < pred_table.size(); i++) {
        if (stat.mem_addr == pred_table[i].index_addr) {
            for (int j = 0; j < pred_table[i].predictors.size(); j++)
                if (!in_cache(pred_table[i].predictors[j]))
                    issue_prefetch(pred_table[i].predictors[j]);
        }
    }
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
