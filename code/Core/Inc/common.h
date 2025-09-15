/*
 * common.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: If you feel that there are common
 *      C functions corresponding to this
 *      header, then any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#include <stdio.h>

#define TID_NULL 0 //predefined Task ID for the NULL task
#define MAX_TASKS 16 //maximum number of tasks in the system
#define MAIN_STACK_SIZE  0X400;
//#define MAX_STACK_SIZE 0x4000;
#define STACK_SIZE 0x200 //min. size of each task’s stack
#define DORMANT 0 //state of terminated task
#define READY 1 //state of task that can be scheduled but is not running
#define RUNNING 2 //state of running task
#define SLEEPING 3
#define RTX_OK 0
#define RTX_ERR 1

typedef unsigned int U32;
typedef unsigned short U16;
typedef char U8;
typedef unsigned int task_t;

typedef struct task_control_block{
	void (*ptask)(void* args); //entry address
	U32 stack_high; //starting address of stack (high address)
	task_t tid; //task ID
	U8 state; //task's state
	U16 stack_size; //stack size. Must be a multiple of 8
	//your own fields at the end
	U32 *stackptr;
	int deadline;
	int time_remaining;
	int sleep_time;
}TCB;

// Values given prevent incorrectly reading state
#define FREE 0xDEAD
#define ALLOCATED 0xBEEF

// Metadata struct always 4-byte aligned
typedef struct __attribute__((aligned(4))) metadata {
    size_t size; // Does not include metadata struct
    U32 state;						// state of the block of memory (Allocated/Free)
    struct metadata* next;			// points to the next block of memory
    struct metadata* prev;			// points to previous block of memory

    struct metadata* next_free;
    struct metadata* prev_free;

	task_t owner;					// specifies ownership of the metadata
} metadata;

extern int current_task;
extern U32* PSP_next;

#endif /* INC_COMMON_H_ */
