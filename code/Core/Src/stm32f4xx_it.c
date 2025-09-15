/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
__attribute__((naked)) void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */
  __asm volatile(
	".global SVC_Handler_Main\n"
	"TST lr, #4\n"
	"ITE EQ\n"
	"MRSEQ r0, MSP\n"
	"MRSNE r0, PSP\n"
	"B SVC_Handler_Main\n"
  ) ;
  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
__attribute__((naked)) void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */
	__asm volatile(
		"MRS r0, PSP\n"
		"STMDB r0!, {r4-r11}\n"
		"LDR r1, =PSP_next\n"
		"LDR r1, [r1]\n"
		"LDMIA r1!, {r4-r11}\n"
		"MSR PSP, r1\n"
		"MOVS r0, #0xFFFFFFFD\n"
		"BX r0\n"
	);
  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

	if(sys_tick_check) {
		first_tick = 1;
	}

	if (!kernel_running) {
		return;
	}

	int run_scheduler = 0;

	for (int i=0; i < MAX_TASKS; ++i) {
		if (TCB_array[i].state != DORMANT && TCB_array[i].state != SLEEPING) {
			TCB_array[i].time_remaining = TCB_array[i].time_remaining - 1;
			if (TCB_array[i].time_remaining == 0 ) {
				run_scheduler = 1;
				TCB_array[i].time_remaining = TCB_array[i].deadline;
				TCB_array[i].state = READY;
			}
		} else if (TCB_array[i].state == SLEEPING) {
			TCB_array[i].sleep_time -= 1;
			if (TCB_array[i].sleep_time == 0) {
				TCB_array[i].state = READY;
				run_scheduler = 1;
			}
		}
	}

	if (run_scheduler) {
		scheduler();
	}

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/* USER CODE BEGIN 1 */
void SVC_Handler_Main(unsigned int* svc_args) {
	unsigned int svc_number;

	  /*
	  * Stack contains:
	  * r0, r1, r2, r3, r12, r14, the return address and xPSR
	  * First argument (r0) is svc_args[0]
	  */
	  svc_number = ( ( char * )svc_args[ 6 ] )[ -2 ] ;
	  switch( svc_number ) {
		 case 0: // osKernelInit
				U32* MSP_INIT_VAL = *(U32**)0x0;
				U32* temp = (char*)MSP_INIT_VAL - MAIN_STACK_SIZE;
				PSP_next = temp - 16; //next stack is the null task

				for (int i = 0; i<MAX_TASKS; ++i) {
					TCB_array[i].state = DORMANT;
				}

				TCB null_task_tcb;
				null_task_tcb.ptask = null_task;
				null_task_tcb.stack_size = STACK_SIZE;

				osCreateTaskHandler(5, &null_task_tcb);
			  break;
		 case 1: // osCreateDeadlineTask
			  	TCB * task = (TCB *) svc_args[1];
				int deadline = svc_args[2];
		  		svc_args[0] = osCreateTaskHandler(deadline, task);
				break;
		 case 2: // osKernelStart
				TCB_array[current_task].state = RUNNING;
		 		//mode switch for the first time
				__asm volatile (
					"LDR r1, =PSP_next\n"
					"LDR r1, [r1]\n"
					"LDMIA r1!, {r4-r11}\n"
					"MSR PSP, r1\n"
					"MOVS r0, #0xFFFFFFFD\n"
					"BX r0\n"
				);
			  break;
		 case 3: // osYield
			//must run scheduler
			TCB_array[current_task].time_remaining = TCB_array[current_task].deadline;
			TCB_array[current_task].state = READY;
			scheduler();
			break;
		 case 4: // osTaskInfo
			  task_t TID = (task_t) svc_args[0];
			  TCB* task_copy = (TCB *) svc_args[1];
			  task_copy->ptask = TCB_array[TID].ptask;
			  task_copy->stack_high = TCB_array[TID].stack_high;
			  task_copy->tid = TCB_array[TID].tid;
			  task_copy->state = TCB_array[TID].state;
			  task_copy->stack_size = TCB_array[TID].stack_size;
			  task_copy->stackptr = TCB_array[TID].stackptr;
			  task_copy->deadline = TCB_array[TID].deadline;
			  task_copy->time_remaining = TCB_array[TID].time_remaining;
			  task_copy->sleep_time = TCB_array[TID].sleep_time;
			  break;
		 case 5: // osGetTID
			  svc_args[0] = current_task;
			  break;
		 case 6: // osTaskExit
			  TCB *curr_tcb = &TCB_array[current_task];

			  // Set values of TCB entry to invalid values
			  k_mem_dealloc_handler(curr_tcb->stack_high - curr_tcb->stack_size);
			  curr_tcb->ptask = NULL;
			  curr_tcb->stack_high = NULL;
			  curr_tcb->tid = 0;
			  curr_tcb->state = DORMANT;
			  curr_tcb->stack_size = 0;
			  curr_tcb->stackptr = NULL;
			  curr_tcb->deadline = 0;
			  curr_tcb->time_remaining = 0;
			  curr_tcb->sleep_time = 0;
			  num_tasks --;
			  scheduler();
			  break;
		case 7: //k_mem_init
			svc_args[0] = k_mem_init_handler();
			break;
		case 8: {// k_mem_alloc
			size_t size = svc_args[1];
			svc_args[0] = k_mem_alloc_handler(size);
			break;
		}
		case 9: //k_mem_dealloc
			void * ptr = (void *) svc_args[1];
			svc_args[0] = k_mem_dealloc_handler(ptr);
			break;
		case 10: {
			size_t size = svc_args[1];
			svc_args[0] = k_mem_count_extfrag_handler(size);
			break;
		}
		case 11: { //osSleep
			osSleepHandler(svc_args[1]);
			break;
		}
		case 12: { //osPeriodYield
			osSleepHandler(TCB_array[current_task].time_remaining);
			break;
		}
		case 13: { // osSetDeadline
			int deadline = svc_args[0];
			task_t TID = svc_args[1];
			svc_args[0] = osSetDeadlineHandler(deadline, TID);
			break;
		}
		default:
			break;
	  }
}
/* USER CODE END 1 */
