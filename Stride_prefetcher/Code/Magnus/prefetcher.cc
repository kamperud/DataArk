/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "interface.hh"
#include <stdlib.h>

 struct Predictor_entry {
    Addr mem_addr;
    Predictor_entry *next;
};

typedef struct {
    Predictor_entry *first; /* Predicted mem_addr */
    int nof_entries; /* Last time called */
    int last_evicted;
} Predictor_row;

#define PRED_TABLE_MAX_SIZE 100
#define PRED_TABLE_ROW_SIZE 5

static std::map<Addr, Predictor_row*> predictor_table;

void insert_predictor_table(Addr mem_addr, Addr prev_pc) {
    std::map<Addr, Predictor_row*>::iterator it;
    it = predictor_table.find(prev_pc);
    if (predictor_table.size() >= PRED_TABLE_MAX_SIZE)
        return;
    else if (it == predictor_table.end()) {
        Predictor_entry *entry = (Predictor_entry*)malloc(sizeof(Predictor_entry));
        entry->mem_addr = mem_addr;
        entry->next = NULL;

        Predictor_row *row = (Predictor_row*)malloc(sizeof(Predictor_entry));
        row->first = entry;
        row->nof_entries = 1;
        row->last_evicted = 0;

        predictor_table.insert( std::pair<Addr, Predictor_row*>(prev_pc, row) );
    }
    else {
        Predictor_row *row = it->second;
        if (row->nof_entries >= PRED_TABLE_MAX_SIZE) {
            Predictor_entry *replace_entry = row->first;
            for (int i = 0; i < row->last_evicted; i++) {
                replace_entry = replace_entry->next;
            }
            replace_entry->mem_addr = mem_addr;
            row->last_evicted++;
            if (row->last_evicted == PRED_TABLE_ROW_SIZE) row->last_evicted = 0;
        }
        else {
            Predictor_entry *last_entry = row->first;
            for (int i = 0; i < row->nof_entries-1; i++) {
                last_entry = last_entry->next;
            }

            Predictor_entry *new_entry = (Predictor_entry*)malloc(sizeof(Predictor_entry));
            new_entry->mem_addr = mem_addr;
            new_entry->next = NULL; 

            last_entry->next = new_entry;
        }
    }




}

void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */
    DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}

void prefetch_access(AccessStat stat)
{
    static Addr prev_pc;

    if (predictor_table.empty()) {
        prev_pc = stat.pc;
        insert_predictor_table(stat.mem_addr + BLOCK_SIZE, prev_pc);
    }
    else if (stat.miss) {
        insert_predictor_table(stat.mem_addr, prev_pc);
        prev_pc = stat.pc;
    }
    // std::map<Addr, Predictor_row*>::iterator it;
    // it = predictor_table.find(stat.pc);
    // if (it != predictor_table.end()) {
    //     Predictor_row *row = it->second;
    //     Predictor_entry *entry = row->first;
    //     for (int i = 0; i < row->nof_entries; i++) {
    //         if (!in_cache(entry->mem_addr)) 
    //             issue_prefetch(entry->mem_addr);
    //         entry = entry->next;
    //     }
    // }
    /* pf_addr is now an address within the _next_ cache block */
    Addr pf_addr = stat.mem_addr + BLOCK_SIZE;



    /*
     * Issue a prefetch request if a demand miss occured,
     * and the block is not already in cache.
     */
    if (stat.miss && !in_cache(pf_addr)) {
        issue_prefetch(pf_addr);
    }
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
