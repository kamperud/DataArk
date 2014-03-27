/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "interface.hh"
#include <stdlib.h>
#include <vector>

struct Predictor_entry {
    Addr pc_addr;
    int displacement;
    Predictor_entry *next_entry;
};

struct Index_entry {
    Addr index;
    Addr displacement;
    Predictor_entry *ghb_entry;
};

class GHB {
    Predictor_entry *head;
    int current_index;
    int size;

public:
    GHB(int size);
    ~GHB();
    Predictor_entry* insert(Predictor_entry entry);

};

GHB::GHB (int size) {
    head = (Predictor_entry*)malloc(sizeof(Predictor_entry)*size));
    last_index = size-1;
    current_index = 0;
    
}
GHB::~GHB() {free(head);}

Predictor_entry* GHB::insert(Addr pc, int displacement, Predictor_entry *prev_entry) {
    if ()

    head[current_index] = entry;
    if (current_index == last_index) current_index = 0;
    else current_index++;
    return &head[current_index];

}
//     Predictor_entry *first; /* Predicted mem_addr */
//     int nof_entries; /* Last time called */
//     int last_evicted;
// } Predictor_row;

#define GHB_SIZE 64
#define MAX_NOF_PREDICTORS 2

static std::vector<Index_entry> index_table;






void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */
    DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}


void insert_pred_table(Addr index, Addr predictor) {

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
    static Addr prev_mem_miss;

    if (pred_table.empty()) {
        insert_pred_table(stat.pc, stat.mem_addr + BLOCK_SIZE);
    }
    else {
        insert_pred_table(prev_mem_miss, stat.mem_addr);
    }
    prev_mem_miss = stat.pc;
    for (int i = 0; i < pred_table.size(); i++) {
        if (stat.pc == pred_table[i].index_addr) {
            for (int j = 0; j < pred_table[i].predictors.size(); j++)
                if (!in_cache(pred_table[i].predictors[j]) && !in_mshr_queue(pred_table[i].predictors[j]))
                    issue_prefetch(pred_table[i].predictors[j]);
        }
    }
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
