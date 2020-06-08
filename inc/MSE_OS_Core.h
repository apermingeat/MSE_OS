/*
 * MSE_OS_Core.h
 *
 *  Created on: 17 mayo 2020
 *      Author: Alejandro Permingeat
 */

#ifndef ISO_I_2020_MSE_OS_INC_MSE_OS_CORE_H_
#define ISO_I_2020_MSE_OS_INC_MSE_OS_CORE_H_

#include <stdint.h>


/************************************************************************************
 * 			Tamaño del stack predefinido para cada tarea expresado en bytes
 ***********************************************************************************/

#define STACK_SIZE 256

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

void os_InitTarea(void *tarea, uint32_t *stack, uint32_t *stack_pointer);

void os_Init(void);



#endif /* ISO_I_2020_MSE_OS_INC_MSE_OS_CORE_H_ */
