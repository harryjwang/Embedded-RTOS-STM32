/*
 * k_mem.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#ifndef INC_K_MEM_H_
#define INC_K_MEM_H_

#include "common.h"

extern metadata* heap_head;
extern U32 *heap_start;
extern U32 *heap_end;

int k_mem_init();
void *k_mem_alloc(size_t size);
int k_mem_dealloc(void* ptr);
int k_mem_count_extfrag(size_t size);

int k_mem_init_handler();
void *k_mem_alloc_handler(size_t size);
int k_mem_dealloc_handler(void * ptr);
int k_mem_count_extfrag_handler(size_t size);

#endif /* INC_K_MEM_H_ */
