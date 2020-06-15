/*
 * MSE_OS_Core.h
 *
 *  Created on: 17 mayo 2020
 *      Author: Alejandro Permingeat
 */

#ifndef ISO_I_2020_MSE_OS_INC_MSE_OS_CORE_H_
#define ISO_I_2020_MSE_OS_INC_MSE_OS_CORE_H_

#include <stdint.h>
#include <stdbool.h>
#include "board.h"

/************************************************************************************
 * 			Tamaño del stack predefinido para cada tarea expresado en bytes
 ***********************************************************************************/

#define STACK_SIZE 256
#define OS_IDLE_TASK_ID	0xFF

#define OS_CONTROL_MAX_PRIORITY	4

typedef enum
{
	os_control_error_none,
	os_control_error_max_task_exceeded,
	os_control_error_no_task_added,
	os_control_error_task_with_invalid_state,
	os_control_error_task_max_priority_exceeded,
	os_control_error_daly_from_IRQ
} os_control_error_t;

typedef enum
{
	os_control_state__os_from_reset,
	os_control_state__os_running,
	os_control_state__os_scheduling,
	os_control_state__os_error,
	os_control_state__running_from_IRQ
} os_control_state_t;

typedef enum
{
	os_task_state__running,
	os_task_state__ready,
	os_task_state__blocked,
	os_task_state__suspended

} os_TaskState_t;

typedef struct
{
	uint32_t stack[STACK_SIZE/4];
	uint32_t stackPointer;
	void *entryPoint;
	os_TaskState_t state;
	uint8_t priority;
	uint8_t taskID;
	uint32_t blockedTicks;
} os_TaskHandler_t;



//----------------------------------------------------------------------------------

extern uint32_t sp_tarea1;					//Stack Pointer para la tarea 1
extern uint32_t sp_tarea2;					//Stack Pointer para la tarea 2
extern uint32_t sp_tarea3;					//Stack Pointer para la tarea 3

/************************************************************************************
 * 	Posiciones dentro del stack frame de los registros que conforman el stack frame
 ***********************************************************************************/

#define XPSR		1
#define PC_REG		2
#define LR			3
#define R12			4
#define R3			5
#define R2			6
#define R1			7
#define R0			8
#define LR_PREV		9
#define R4		   10
#define R5		   11
#define R6		   12
#define R7		   13
#define R8		   14
#define R9		   15
#define R10		   16
#define R11		   17

//----------------------------------------------------------------------------------


/************************************************************************************
 * 			Valores necesarios para registros del stack frame inicial
 ***********************************************************************************/

#define INIT_XPSR 	1 << 24				//xPSR.T = 1
#define EXEC_RETURN	0xFFFFFFF9			//retornar a modo thread con MSP, FPU no utilizada

//----------------------------------------------------------------------------------


/************************************************************************************
 * 						Definiciones varias
 ***********************************************************************************/
#define STACK_FRAME_SIZE				8
#define STACK_FRAME_ALL_RECORDS_SIZE	17 /*esto incluye a LR*/


/*==================[definicion de prototipos]=================================*/

void os_InitTask(os_TaskHandler_t *taskHandler, void* entryPoint, uint8_t priority);

void os_Init(void);

void os_CpuYield(void);

os_TaskHandler_t* os_getActualtask();

void os_enter_critical_zone();
void os_exit_critical_zone();

os_control_state_t os_get_controlState();

void os_set_controlState(os_control_state_t newState);

void os_setSchedulingFromIRQ();

void os_clearSchedulingFromIRQ();

bool os_isSchedulingFromIRQ();

void os_setError(os_control_error_t err, void* caller);


#endif /* ISO_I_2020_MSE_OS_INC_MSE_OS_CORE_H_ */
