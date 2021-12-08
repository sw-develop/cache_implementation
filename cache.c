/*
 * cache.c
 *
 * 20493-01 Computer Architecture
 * Term Project on Implentation of Cache Mechanism
 *
 * Skeleton Code Prepared by Prof. HyungJune Lee
 * Nov 15, 2021
 *
 */


#include <stdio.h>
#include <string.h>
#include "cache_impl.h"

/* hit ratio = (num_cache_hits / (num_cache_hits + num_cache_misses))*/
extern int num_cache_hits;  // # of hits
extern int num_cache_misses;    // # of missses

/* bandwidth = (num_bytes / num_acess_cycles) */
extern int num_bytes;   // # of accessed bytes
extern int num_access_cycles;   // # of clock cycles 

extern int global_timestamp;    // # of data access trials 

cache_entry_t cache_array[CACHE_SET_SIZE][DEFAULT_CACHE_ASSOC]; // Cache 
int memory_array[DEFAULT_MEMORY_SIZE_WORD]; // Memory 

extern unsigned long int block_addr;   // block address in cache (Byte Address / Cache Block Size Byte)
extern unsigned long int byte_offset;   // byte offset in cache 

int binary[100]; // Store the value obtained by converting the decimal to binary 
int size_of_binary; // actual number of meaningful value 

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
void init_memory_content() {
    unsigned char sample_upward[16] = {0x001, 0x012, 0x023, 0x034, 0x045, 0x056, 0x067, 0x078, 0x089, 0x09a, 0x0ab, 0x0bc, 0x0cd, 0x0de, 0x0ef};
    unsigned char sample_downward[16] = {0x0fe, 0x0ed, 0x0dc, 0x0cb, 0x0ba, 0x0a9, 0x098, 0x087, 0x076, 0x065, 0x054, 0x043, 0x032, 0x021, 0x010};
    int index, i=0, j=1, gap = 1;
    
    for (index=0; index < DEFAULT_MEMORY_SIZE_WORD; index++) {
        memory_array[index] = (sample_upward[i] << 24) | (sample_upward[j] << 16) | (sample_downward[i] << 8) | (sample_downward[j]);
        if (++i >= 16)
            i = 0;
        if (++j >= 16)
            j = 0;
        
        if (i == 0 && j == i+gap)
            j = i + (++gap);
            
        printf("mem[%d] = %#x\n", index, memory_array[index]);
    }
}   

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
void init_cache_content() {
    int i, j;
    
    for (i=0; i<CACHE_SET_SIZE; i++) {
        for (j=0; j < DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t *pEntry = &cache_array[i][j];
            pEntry->valid = 0;
            pEntry->tag = -1;
            pEntry->timestamp = 0;
        }
    }
}

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
/* This function is a utility function to print all the cache entries. It will be useful for your debugging */
void print_cache_entries() {
    int i, j, k;
    
    for (i=0; i<CACHE_SET_SIZE; i++) {
        printf("[Set %d] ", i);
        for (j=0; j <DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t *pEntry = &cache_array[i][j];
            printf("V: %d Tag: %#x Time: %d Data: ", pEntry->valid, pEntry->tag, pEntry->timestamp);
            for (k=0; k<DEFAULT_CACHE_BLOCK_SIZE_BYTE; k++) {
                printf("%#x(%d) ", pEntry->data[k], k);
            }
            printf("\t");
        }
        printf("\n");
    }
}


/* Return the requested value from the cache */
int get_data_by_byte_addr_and_data_type_in_cache(char memory_data[], char type){  
    // b(1 byte), h(2 byte), w(4 byte)
    if(type == 'b'){       // if requested type is byte
        return memory_data[byte_offset];    // return byte 
    }
    else if(type == 'h'){   // if requested type is half word 
        return ((memory_data[byte_offset + 1] & 0x000000FF) << 8) | 
        (memory_data[byte_offset] & 0x000000FF);    // return half word
    }
    else{   // type == 'w', if requested type is word 
        return (
            (memory_data[byte_offset + 3] & 0x000000FF) << 24) | 
            ((memory_data[byte_offset+2] & 0x000000FF) << 16) | 
            ((memory_data[byte_offset+1] & 0x000000FF) << 8) | 
            (memory_data[byte_offset] & 0x000000FF);    // return word
    }

}

/* function to find the tag value of the cache block */
int caculate_tag(int num_of_indexs_determine_cache_set_index, int size_of_binary){
    int current_tag = 0;    // initiate the value 
    int multiplication_of_2 = 1;    // value to be multiplied
    for(int i = num_of_indexs_determine_cache_set_index; i <= size_of_binary; i++){ // Repeated for a valid value 
        current_tag += binary[i] * multiplication_of_2; // caculate tag
        multiplication_of_2 *= 2;   // multiplying by 2
    }

    return current_tag; // return tag value 
}

/* If a value already exists in the corresponding cache block, check whether the tag is the same */
int check_cache_block_if_data_is_exist(cache_entry_t cache_block, int num_of_indexs_determine_cache_set_index){
    /* Tag value should be the same */
    int current_tag = caculate_tag(num_of_indexs_determine_cache_set_index, size_of_binary);    // calculate tag value

    /* Cache miss */
    if(current_tag != cache_block.tag){     // if the tag value is different with existing one
        return 0;   // this is cosidered 'false'
    }
    /* Cache hit */
    return 1;   // this is considered 'true'
}

/* A function that converts decimal into binary - order of the binary value is stored in reverse */
int decimal_to_binary(unsigned long int block_addr){
    int i;
    for(i = 0; block_addr > 0; i++){
        binary[i] = block_addr % 2; // calculte binary value 
        block_addr /= 2;    // divide by 2 
    }
    return i-1; // return the last valid index indicating the corresponding value 
}

int check_cache_data_hit_direct_mapped(void *addr, char type){  // function of check cache data hit if cache is direct mapped
    int cache_set_index = binary[0]*1 + binary[1]*2;    // Set of Cache
    cache_entry_t cache_block = cache_array[cache_set_index][0];    // save cache block at that location

    if(cache_block.valid == 1){ /* Check cache if data is exist */
        if(check_cache_block_if_data_is_exist(cache_block, 2)){ /* if Cache hit */
            num_access_cycles += 1; // cache access time = 1 cycle 
            cache_block.timestamp = ++global_timestamp; // increase the timestamp 
            num_cache_hits++;   // increase the number of cache hit
            return get_data_by_byte_addr_and_data_type_in_cache(cache_block.data, type); // return accessed data
        } 
        else{
            /* Cache miss - because tag is different */
            num_cache_misses++; // increase the number of cache miss 
            return 0;        
        }
    }

    /* Cache miss - becasue valid is 0 (No data in cache set) */
    num_cache_misses++; // increase the number of cache miss 
    return 0;
}

int check_cache_data_hit_2way_set_associative(void *addr, char type){   // function of check cache data hit if cache is 2-way associative 
    int cache_set_index = binary[0]*1;  // Set of Cache 

    for(int i = 0; i < 2; i++){ // repeat twice because there are 2 blocks in one set 
        cache_entry_t cache_block = cache_array[cache_set_index][i];    // get the corresponding cache block 

        if(cache_block.valid == 1){ /* Check cache if data is exist */
            if(check_cache_block_if_data_is_exist(cache_block, 1)){ /* if Cache hit */
                num_access_cycles += 1; // cache access time = 1 cycle 
                cache_block.timestamp = ++global_timestamp; // increase the timestamp 
                num_cache_hits++;   // increase the number of cache hit
                return get_data_by_byte_addr_and_data_type_in_cache(cache_block.data, type); // return accessed data
            }
        }
    }

    /* Cache miss - becasue valid is 0 (No data in cache set) */
    num_cache_misses++; // increase the number of cache miss 
    return 0;
}

int check_cache_data_hit_fully_associative(void *addr, char type){  // function of check cache data hit if cache is fully associative 
    // No Set in Cache 

    /* Check Cache if data is exist */
    for(int i = 0; i < 4; i++){ // repeat fourth because there are 4 blocks in one set 
        cache_entry_t cache_block = cache_array[0][i];  // get the corresponding cache block 

        if(cache_block.valid == 1){ /* Check cache if data is exist */
            if(check_cache_block_if_data_is_exist(cache_block, 0)){ /* Cache hit */
                num_access_cycles += 1; // cache access time = 1 cycle 
                cache_block.timestamp = ++global_timestamp; // increase the timestamp 
                num_cache_hits++;   // increase the number of cache hit
                return get_data_by_byte_addr_and_data_type_in_cache(cache_block.data, type);    // return accessed data
            }
        }
    }     

    /* Cache Miss - becasue valid is 0 (No data in Cache set) */
    num_cache_misses++; // increase the number of cache miss 
    return 0;
}


int check_cache_data_hit(void* addr, char type) { // Functions are divided according to the type of cache 
    size_of_binary = decimal_to_binary(block_addr); // Change decimal to binary
    
    /* Fill out here */
    if(DEFAULT_CACHE_ASSOC == 1){   // if cache is direct mapped
        /* Return the data */
        return check_cache_data_hit_direct_mapped(addr, type);  
    }
    else if(DEFAULT_CACHE_ASSOC == 2){  // if cache is 2-way associative
        /* Return the data */
        return check_cache_data_hit_2way_set_associative(addr, type);
    }
    else if(DEFAULT_CACHE_ASSOC == 4){  // if cache is fully associative 
        /* Return the data */
        return check_cache_data_hit_fully_associative(addr, type);
    }
 
    /* Return the data */
    return 0;    
}

/* Find the location of the cache to copy the memory value */
int find_entry_index_in_set(int cache_index) {  
    int entry_index;    // position of the block within one set 

    /* Check if there exists any empty cache space by checking 'valid' */
    /* Otherwise, search over all entries to find the least recently used entry by checking 'timestamp' */
    int least_recently_used = 100;
    for(int i = 0; i < DEFAULT_CACHE_ASSOC; i++){   // repeat according to the type of cache 
        cache_entry_t cache_block = cache_array[cache_index][i];    // get the cache block 
        if(cache_block.valid == 0){ // if there is no data
            return i;   // return position 
        }
        else{   // if there is data 
            if(least_recently_used > cache_block.timestamp){    // if that data's timestamp is smaller than least recently user value
                least_recently_used = cache_block.timestamp;    // update the value 
                entry_index = i;    // update the entry index value 
            }
        }
    }

    return entry_index; // return index 
}

/* Get data from memory and save to cache */
void fetch_data_from_main_memory_to_cache(int memory_index, int cache_set_index ,int entry_index_of_set, cache_entry_t cache_block){
    int memory_data_a = memory_array[memory_index]; // get data from memory
    int memory_data_b = memory_array[memory_index + 1]; // get data from memory

    char data[DEFAULT_CACHE_BLOCK_SIZE_BYTE] = {    // create an array
        (memory_data_a << 24) >> 24, 
        (memory_data_a << 16) >> 24,
        (memory_data_a << 8) >> 24,
        memory_data_a >> 24,
        (memory_data_b << 24) >> 24, 
        (memory_data_b << 16) >> 24,
        (memory_data_b << 8) >> 24,
        memory_data_b >> 24
    };
    memcpy(cache_block.data, data, sizeof(data));   // copy array to cache_block data array

    // entry_index_of_set : location of the block in the set 
    // store cache block into cache array
    cache_array[cache_set_index][entry_index_of_set] = cache_block;
}

int access_memory(void *addr, char type) {  
    /* Fetch the data from the main memory and copy them to the cache */
    /* void *addr: addr is byte address, whereas your main memory address is word address due to 'int memory_array[]' */

    size_of_binary = decimal_to_binary(block_addr); // Change decimal to binary

    /* 1) create the cache_block to store */
    cache_entry_t cache_block;
    cache_block.valid = 1;  // set valid 1
    cache_block.timestamp = ++global_timestamp; // increment timestamp and set 
    num_access_cycles += 101; // cache access time + main memory access time 
    
    int cache_set_index = 0; // initiate value 
    if(DEFAULT_CACHE_ASSOC == 1){   // direct mapped cache
        cache_block.tag = caculate_tag(2, size_of_binary);  // set tag value 
        cache_set_index = binary[0]*1 + binary[1]*2;    // Set Index of Cache
    }
    else if(DEFAULT_CACHE_ASSOC == 2){  // 2-way assoc cache
        cache_block.tag = caculate_tag(1, size_of_binary);  // set tag value 
        cache_set_index = binary[0]*1;  // Set Index of Cache
    }
    else{   // DEFAULT_CACHE_ASSOC == 4, fully assoc cache 
        cache_block.tag = caculate_tag(0, size_of_binary);  // set tag value 
        cache_set_index = 0;    // Only 1 Set Index
    }

    /* 2) You need to invoke find_entry_index_in_set() for copying to the cache */
    int entry_index_of_set = find_entry_index_in_set(cache_set_index);

    /* 3) Fetch the data from the main memory and copy them to the cache */
    int memory_index = block_addr * DEFAULT_CACHE_BLOCK_SIZE_BYTE / WORD_SIZE_BYTE;

    int memory_word_addr = *(unsigned long int*)addr / 4;   // caculate memory word address
    int memory_word_offset = *(unsigned long int*)addr % 4; // caculate memory word offset 
    
    if(memory_word_addr % 2 != 0) byte_offset = memory_word_offset + 4; // If the memory approach index is even, then offset is equal, and if it is odd, offset + 4
    fetch_data_from_main_memory_to_cache(memory_index, cache_set_index, entry_index_of_set, cache_block);   /* Get data from memory and save to cache */

    /* 4) Return the accessed data with a suitable type */  
    cache_block = cache_array[cache_set_index][entry_index_of_set]; // get cache block from cache 
    return get_data_by_byte_addr_and_data_type_in_cache(cache_block.data, type);    // return accessed data
}
