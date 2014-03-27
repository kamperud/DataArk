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
    Addr index_addr;
    std::vector<Addr> predictors;
};

// typedef struct {
//     Predictor_entry *first; /* Predicted mem_addr */
//     int nof_entries; /* Last time called */
//     int last_evicted;
// } Predictor_row;

#define PRED_TABLE_MAX_SIZE 2
#define MAX_NOF_PREDICTORS 2

static std::vector<Predictor_entry> pred_table;



void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */
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

int main () {

    Addr pc = 5;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 16; j++) {
            insert_pred_table(j, i);
            
        }
    }
    for (int j = 0; j < pred_table.size(); j++) {
        std::cout << "Index: " << pred_table[j].index_addr << "\n";
        for (int i = 0; i < pred_table[j].predictors.size(); i++) {
            std::cout << pred_table[j].predictors[i] << "\n";
        }
    }

    return 1;
}