/*
 * k_task.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#ifndef INC_K_TASK_H_
#define INC_K_TASK_H_

#include "common.h"

extern int kernel_running;
extern TCB TCB_array[MAX_TASKS];
extern U32* stackptr;
extern int num_tasks;
extern task_t tids_sorted[MAX_TASKS];
extern int first_tick;
extern int sys_tick_check;

void startThread(void);
void osKernelInit(void);
int osCreateTask(TCB* task);
int osKernelStart(void);
void osYield(void);
int osTaskInfo(task_t TID, TCB* task_copy);
task_t osGetTID(void);
int osTaskExit(void);
void null_task(void *args);
void osSleep(int timeInMs);
void osPeriodYield();
void scheduler();
int osCreateTaskHandler(int deadline, TCB * task);
void osSleepHandler(int timeInMs);
int osCreateDeadlineTask(int deadline, TCB* task);
int osSetDeadline(int deadline, task_t TID);
int osSetDeadlineHandler(int deadline, task_t TID);
#endif /* INC_K_TASK_H_ */
