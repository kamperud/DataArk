// Stride prefetcher implemented with an array
// (it might need to be upgraded to a hash table to work properly)
#include "interface.hh"
#include <stdlib.h>

#define SIZE 50    //size of initial array and added extension of that array

typedef struct array_entry{
    int PC;
    int prev_address;
    int stride;
    bool status;
}array_entry;



const int prefetches_per_miss = 6;
int array_size = SIZE;
int array_index = 0;
array_entry *array;

void array_addEntry( array_entry entry){
    array_index++;
    if (array_size==array_index){
        array_size += SIZE;
        array = (array_entry*)realloc(array, sizeof(array_entry)*array_size);
    }
    array[array_index] = entry;
}

//searches for a PC value, returns index if found, returns -1 if PC value isn't in the array
int array_search (int newPC){
    for(int i=0; i<=array_index; i++){
        if(array[i].PC == newPC)    return i;
    }
    return -1;
}

void prefetch_init(void){
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */
    array = (array_entry*)malloc(sizeof(array_entry)*array_size);


    
    DPRINTF(HWPrefetch, "Initialized stride prefetcher\n");
}

void prefetch_access(AccessStat stat){
    /* pf_addr is now an address within the _next_ cache block */
    //Addr pf_addr = stat.mem_addr + BLOCK_SIZE;
    
   
    int index = array_search(stat.pc);
    //new entry in the stride table. Doesn't
    if (index == -1){
        array_entry entry;
        entry.PC = stat.pc;
        entry.prev_address = stat.mem_addr;
        entry.stride = 0;
        entry.status = false;
        array_addEntry(entry);
    }
    else {
        int new_stride = stat.mem_addr - array[index].prev_address;
        //If the stride is zero we don't need to do anything (I think...)
        if (new_stride!=0){
            //If this is the first time we see a stride
            if (new_stride!=array[index].stride){
                array[index].stride = new_stride;
                array[index].prev_address = stat.mem_addr;
                array[index].status = false;
            }
            else {
                //We see a stride for the second time
                array[index].status = true;
                array[index].prev_address = stat.mem_addr;
                //if there wasn't a miss on this then the next is probably also in memory so we don't prefetch
                if(stat.miss) {
                    Addr pf_addr;
                    for(int i=0; i<prefetches_per_miss; i++){
                        pf_addr = stat.mem_addr + i*array[index].stride;
                        if (!in_cache(pf_addr)&&!in_mshr_queue(pf_addr)) {
                            issue_prefetch(pf_addr);
                        }
                    }
                }
            }
        }
    }

    /*
     * Issue a prefetch request if a demand miss occured,
     * and the block is not already in cache.
//     */
//
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}

















