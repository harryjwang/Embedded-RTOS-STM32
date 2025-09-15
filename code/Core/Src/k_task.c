/*
 * k_task.c
 *
 *  Created on: May 28, 2025
 *      Author: tange
 */


#include <stdio.h>
#include "k_task.h"
#include "main.h"

TCB TCB_array[MAX_TASKS];
int kernel_running = 0;
int current_task = 0;
task_t tids_sorted[MAX_TASKS];
int num_tasks = 0;
U32* PSP_next = NULL;
int first_tick = 0;
int sys_tick_check = 0;

int osCreateDeadlineTask(int deadline, TCB* task) {

	if (deadline <= 0) {
		return RTX_ERR;
	}

	int result;
    __asm volatile(
			"MOV r1, %[task]\n"
			"MOV r2, %[deadline]\n"
			"SVC #1\n"
			"MOV %[result], r0\n"
			: [result] "=r" (result)
			: [task] "r" (task), 
			  [deadline] "r" (deadline)
			: "r1", "r2", "r0"
		);
    	// Should return value placed in R0 (svc_args[0]) in SVC_Handler_Main

	return result;
}

void scheduler() {
	//Find next task to run
	int earliest_deadline = -1;
	int j = 0; //switch to null task if no other task is available
	for (int i = 1 ; i < MAX_TASKS; i++) {
		TCB task = TCB_array[i];

		if (task.state == DORMANT || task.state == SLEEPING) {
			continue;
		}

		if (earliest_deadline == - 1 || task.time_remaining < earliest_deadline) {
			earliest_deadline = task.time_remaining;
			j = i;
		}
	}

	TCB_array[j].state = RUNNING;

	if (j == current_task) {
		//no context switch needed
		return;
	}

	//store psp
	U32* current_psp;
	__asm volatile ("MRS %0, PSP" : "=r" (current_psp));

	current_psp -= 8; //will store 8 more registers
	TCB_array[current_task].stackptr = current_psp;

	//set up next task for context switch
	current_task = j;
	PSP_next = TCB_array[current_task].stackptr;

	//schedule pendsv interrupt
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void osPeriodYield() {
	//calculate sleep time
	__asm volatile(
		"SVC #12"
	);
}

void osSleep(int timeInMs) {
	__asm volatile(
		"MOV r1, %[timeInMs]\n"
		"SVC #11\n"
		: 
		: [timeInMs] "r" (timeInMs)
		: "r1"
	);
}

int osSetDeadline(int deadline, task_t TID) {
	if (deadline < 1 || TID >= MAX_TASKS) {
		return RTX_ERR;
	} else if (TCB_array[TID].state != READY) {
		return RTX_ERR;
	}

	int result;

	__asm volatile(
		"MOV r0, %[deadline]\n"
		"MOV r1, %[TID]\n"
		"SVC #13\n"
		"MOV %[result], r0\n"
		: [result] "=r" (result)
		: [deadline] "r" (deadline),
		[TID] "r" (TID)
		: "r1", "r0"
	);

	return result;
}

int osSetDeadlineHandler(int deadline, task_t TID) {
	if (TID == current_task) {
		return RTX_OK;
	}

	TCB_array[TID].deadline = deadline;
	TCB_array[TID].time_remaining = deadline;

	if (deadline < TCB_array[current_task].deadline) {
		TCB_array[current_task].state = READY;
		TCB_array[TID].state = RUNNING;

		//store psp
		U32* current_psp;
		__asm volatile ("MRS %0, PSP" : "=r" (current_psp));

		current_psp -= 8; //will store 8 more registers
		TCB_array[current_task].stackptr = current_psp;

		//set up next task for context switch
		current_task = TID;
		PSP_next = TCB_array[current_task].stackptr;

		//schedule pendsv interrupt
		SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
	}

	return RTX_OK;
}

void osSleepHandler(int timeInMs) {
	TCB_array[current_task].time_remaining = TCB_array[current_task].deadline;
	TCB_array[current_task].sleep_time = timeInMs;
	TCB_array[current_task].state = SLEEPING;
	scheduler();
}

void osKernelInit(void){
	__asm volatile(
		"SVC #0\n"
	);
}

int osCreateTask(TCB* task) {
	osCreateDeadlineTask(5, task);
}

int osCreateTaskHandler(int deadline, TCB * task) {
	task_t free;

	if (task == NULL) {
		return RTX_ERR;
	}

	for (task_t i = 0; i < MAX_TASKS; i++) {
		if (TCB_array[i].state == DORMANT) {
			free = i;
			break;
		}

		// TCB_array is full
		if (i == MAX_TASKS - 1) {
			return RTX_ERR;
		}
	}

	task->tid = free;
	task->state = READY;

	//stack size must be minimum
	if (task->stack_size < STACK_SIZE) {
		return 0;
	}

	if (free != 0) {
		metadata* new_meta = k_mem_alloc_handler(task->stack_size);
		if (new_meta == NULL) {
			return RTX_ERR;
		}
		new_meta--;
		//set owner of stack to created task
		new_meta->owner = free;
		new_meta ++;
		//stack high must be top of allocated region
		task->stack_high = (U32) new_meta +  task->stack_size;
	} else {
		//null task no dynamic allocation
		task->stack_high = (U32) (*(U32**)0x0) - MAIN_STACK_SIZE
	}

	//default deadline
	task->deadline = deadline;
	task->time_remaining = deadline;
	task->sleep_time = 0;

	TCB_array[free] = * task;
	TCB_array[free].stackptr = TCB_array[free].stack_high;

	*(--TCB_array[free].stackptr) = (1U << 24);
	*(--TCB_array[free].stackptr) = (U32) task->ptask;

	for (int i = 0; i < 14; i++) {
		*(--TCB_array[free].stackptr) = 0xA;
	}

	task->stackptr = TCB_array[free].stackptr;

	//scheduler only runs if kernel started
	if (kernel_running) {
		scheduler();
	} else if (current_task == 0 || deadline < TCB_array[current_task].time_remaining){
		//the task with the lowest deadline should be the first to start.
		current_task = free;
		PSP_next = TCB_array[free].stackptr;
	}

	return RTX_OK;
}

int osKernelStart(void) {
	if (kernel_running) {
		return RTX_ERR;
	}
	sys_tick_check = 1;
	
	//wait for a systick to reset the timer
	while (!first_tick) {
	}

	sys_tick_check = 0;
	kernel_running = 1;

	if (PSP_next == NULL) {
		return RTX_ERR;
	}

	__asm volatile("SVC #2");
	return RTX_ERR; // Should never reach here
}

void osYield(void) {
	//assume it is called when inside a running task (after start)
	__asm volatile("SVC #3");
}

int osTaskInfo(task_t TID, TCB* task_copy) {
	// Need to create a TCB object before osTaskInfo is called
	if (TCB_array[TID].state == DORMANT || task_copy == NULL || TID >= MAX_TASKS) {
		// Task does not exist
		return RTX_ERR;
	} else {
		__asm volatile (
		        "MOV r0, %[TID]\n"
		        "MOV r1, %[task]\n"
		        "SVC #4\n"
		        :
		        : [TID] "r" (TID), [task] "r" (task_copy)
		        : "r0", "r1"
		    );
	}

	return RTX_OK;
}

task_t osGetTID(void) {
	task_t result;

	if (!kernel_running) {
		return 0;
	}
	__asm volatile(
			"SVC #5\n"
			"MOV %[result], r0\n"
			: [result] "=r" (result)
			: 
			: "r0"
			);
	// Should return value placed in R0 (svc_args[0]) in SVC_Handler_Main
	return result;
}

int osTaskExit(void) {
	__asm volatile("SVC #6");
	//osYield(); // Move to next available task
}


void null_task(void *args) {
	while (1) {
		osYield();
	}
}

