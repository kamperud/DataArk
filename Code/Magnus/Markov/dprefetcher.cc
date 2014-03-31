/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "interface.hh"
#include <stdlib.h>
#include <vector>
#include <cmath>

struct Predictor_entry {
    Addr index_addr;
    std::vector<int> predictors;
};

// typedef struct {
//     Predictor_entry *first; /* Predicted mem_addr */
//     int nof_entries; /* Last time called */
//     int last_evicted;
// } Predictor_row;

#define PRED_TABLE_MAX_SIZE 16
#define MAX_NOF_PREDICTORS 3

static std::vector<Predictor_entry> pred_table;



void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */
    DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}

void insert_pred_table(Addr index, int predictor) {

    if (pred_table.size() == 0) {   
        Predictor_entry new_entry;
        new_entry.index_addr = index;
        new_entry.predictors.insert(new_entry.predictors.begin(), predictor);
        pred_table.insert(pred_table.begin(), new_entry);
        return; 
    }
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
                        pred_table[i].predictors.erase(pred_table[i].predictors.end()-1);
                    }
                }
            }

            /* Put the recently used index in front of the queue */
            Predictor_entry temp = pred_table[0];
            pred_table[0] = pred_table[i];
            pred_table[i] = temp;
                    

            return;

        }
        /** Was not already in table, insert it **/
        else if (i == (pred_table.size()-1)) {
            Predictor_entry new_entry;
            new_entry.index_addr = index;
            new_entry.predictors.insert(new_entry.predictors.begin(), predictor);
            pred_table.insert(pred_table.begin(), new_entry);
            if (pred_table.size() > PRED_TABLE_MAX_SIZE) {
                pred_table.erase(pred_table.end()-1);
            }
            return; 
        }
    }
}

void prefetch_access(AccessStat stat)
{
    static Addr prev_mem_addr;
    static Addr prev_pc;

    if (pred_table.empty()) {
        insert_pred_table(stat.pc, stat.mem_addr - prev_mem_addr);
    }
    else if (stat.miss) {
        insert_pred_table(prev_pc, stat.mem_addr - prev_mem_addr));
    }
    prev_mem_addr = stat.mem_addr;
    prev_pc = stat.pc;
    for (int i = 0; i < pred_table.size(); i++) {
        if (stat.pc == pred_table[i].index_addr) {
            for (int j = 0; j < pred_table[i].predictors.size(); j++) {
                int prefetch_addr = stat.mem_addr + pred_table[i].predictors[j];
                if (!in_cache(prefetch_addr) && !in_mshr_queue(prefetch_addr))
                    issue_prefetch(prefetch_addr);
            }
        }
    }
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}