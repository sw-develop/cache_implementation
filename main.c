/*
 * main.c
 *
 * 20493-01 Computer Architecture
 * Term Project on Implentation of Cache Mechanism
 *
 * Skeleton Code Prepared by Prof. HyungJune Lee
 * Nov 15, 2021
 *
 */

#include <stdio.h>
#include "cache_impl.h"

#include <string.h>

int num_cache_hits = 0; // number of cache hits 
int num_cache_misses = 0;   // number of cache misses

int num_bytes = 0;  
int num_access_cycles = 0;

int global_timestamp = 0;

unsigned long int block_addr = 0;   // Block Address in cache
unsigned long int byte_offset = 0;  // Byte Offset in cache

int retrieve_data(void *addr, char data_type) {
    int value_returned = -1; /* accessed data */

    // Count # of accessed bytes
    if(data_type == 'b'){  // if data type is byte
        num_bytes += 1;
    }
    else if(data_type == 'h'){  // if data type is half word(2 bytes)
        num_bytes += 2;
    }
    else{   // data_type == 'w' (4 bytes)
        num_bytes += 4;
    }

    /* Invoke check_cache_data_hit() */
    value_returned = check_cache_data_hit(addr, data_type);
    if(value_returned == 0){
        /* In case of the cache miss event, access the main memory by invoking access_memory() */
        value_returned = access_memory(addr, data_type);
    }

    return value_returned;   // return accessed data  
}

int main(void) {
    FILE *ifp = NULL, *ofp = NULL;
    unsigned long int access_addr; /* byte address (located at 1st column) in "access_input.txt" */
    char access_type; /* 'b'(byte), 'h'(halfword), or 'w'(word) (located at 2nd column) in "access_input.txt" */
    int accessed_data; /* This is the data that you want to retrieve first from cache, and then from memory */ 
    
    init_memory_content(); // Initialize contents of a main memory
    init_cache_content();  // Initialize contents of a cache memory
    
    ifp = fopen("./test_input_output_files/access_input3.txt", "r");    // open test input file
    if (ifp == NULL) {  // if there is no file
        printf("Can't open input file\n");  // print error message
        return -1;  
    }
    ofp = fopen("./test_input_output_files/access_output3.txt", "w"); // open output file to write results of program
    if (ofp == NULL) {  // if there is no file
        printf("Can't open output file\n"); // print error message
        fclose(ifp);    // close input file
        return -1; 
    }

    fprintf(ofp, "[Accessed Data]\n");  // write it to file 
    
    /* Fill out here by invoking retrieve_data() */
    while(fscanf(ifp, "%lu %c", &access_addr, &access_type) != EOF){
        fprintf(ofp, "%lu %c \t", access_addr, access_type);    // write it to file 

        block_addr = access_addr / 8;   // caculate block address in cache
        byte_offset = access_addr % 8;  // caculate byte offset in cache 

        accessed_data = retrieve_data(&access_addr, access_type);   // store returned data in accessed_data 
        fprintf(ofp, "%#x\n", accessed_data);   // write it to file
    }

    /* Write a line in output file */
    if(DEFAULT_CACHE_ASSOC == 1){   // if cache setting is Direct Mapped Cache
        fprintf(ofp, "-----------------------------------------\n[Direct mapped cache performance]\n");     // write it to file 
    }
    else if(DEFAULT_CACHE_ASSOC == 2){  // if cache setting is 2-Way Associative Cache
        fprintf(ofp, "-----------------------------------------\n[2-way set associative cache performance]\n"); // write it to file 
    }
    else{   // if cache setting is Fully Associative Cache 
        fprintf(ofp, "-----------------------------------------\n[fully associative cache performance]\n"); // write  it to file 
    }

    
    int num_of_access_cache = num_cache_hits + num_cache_misses;    // calculate total number of accessing cache(hit + miss)
    float hit_ratio = (float)num_cache_hits / num_of_access_cache;  // caculate hit ratio 
    fprintf(ofp, "Hit ratio = %.2f (%d/%d)\n", hit_ratio, num_cache_hits, num_of_access_cache); // write it to file 


    float bandwidth = (float)num_bytes / num_access_cycles; // calculate the bandwidth 
    fprintf(ofp, "Bandwidth = %.2f (%d/%d)", bandwidth, num_bytes, num_access_cycles);  // write it to file 

    fclose(ifp);    // close file
    fclose(ofp);    // close file 
    
    print_cache_entries();  // print cache 
    return 0;
}
