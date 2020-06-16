/*
 * MSE_OS_API.c
 *
 *  Created on: 17 mayo 2020
 *      Author: Alejandro Permingeat
 *
 *  @brief Librería que contiene el núcleo
 *         del sistema operativo
 */

#ifndef ISO_I_2020_MSE_OS_INC_MSE_OS_CORE_H_
#define ISO_I_2020_MSE_OS_INC_MSE_OS_CORE_H_

/*==================[inclusions]=============================================*/

#include <stdint.h>
#include <stdbool.h>
#include "board.h"

/*==================[macros and definitions]=================================*/

#define STACK_SIZE 256 /** Tamaño del stack predefinido para cada tarea expresado en bytes */

#define OS_IDLE_TASK_ID	0xFF

#define OS_CONTROL_MAX_PRIORITY	3

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
/******************************************************************************
 *  @brief Inicialización de tarea
 *
 *  @details
 *   Esta rutina inicializa la estructura de datos de la tarea y asociará
 *   un handler que se ejecutará cuando la tarea le toque ejecutarse
 *
 *  @param *taskHandler		puntero a la estructura de datos de la tarea
 *  @param *entryPoint		puntero a la rutina que se ejecutará cuando
 *  						deba ejecutarse esta tarea
 *  @param priority			prioridad de la tarea (valor entre 0 y 3, donde 0
 *  						0 es la mayor prioridad)
 *  @return     none.
 *****************************************************************************/
void os_InitTask(os_TaskHandler_t *taskHandler,
		void* entryPoint,
		uint8_t priority);

/******************************************************************************
 *  @brief Inicialización del sistema operativo
 *
 *  @details
 *   Inicializa y pone a correr el sistema operativo. Las tareas, semáforos
 *   y colas deben ser inicializadas antes de ejecutar esta rutina
 *
 *  @return     none.
 *****************************************************************************/
void os_Init(void);

/******************************************************************************
 *  @brief Obtiene el contexto siguiente
 *
 *  @details
 *   Esta obtiene el contexto de la próxima tarea a ser ejecutada
 *
 *  @param *p_stack_actual		puntero al stack de la tarea actual
 *  @return     none.
 *****************************************************************************/
uint32_t getContextoSiguiente(uint32_t p_stack_actual);

/******************************************************************************
 *  @brief Fuerza la ejecución del scheduler
 *
 *  @details
 *   Esta rutina es muy útil para no desperdiciar tiempo cuando una tarea
 *   se bloquea antes del próximo tick
 *
 *  @param 		none
 *  @return     none.
 *****************************************************************************/
void os_CpuYield(void);

/******************************************************************************
 *  @brief Obtiene la tarea actual
 *
 *  @details
 *   Obtiene la estructura de datos de control de la tarea actualmente en
 *   ejecución
 *
 *  @param 		none
 *  @return     puntero a la tarea actual
 *****************************************************************************/
os_TaskHandler_t* os_getActualtask();

/******************************************************************************
 *  @brief Obtiene el estado del sistema operativo
 *
 *  @details
 *   Obtiene el estado actual del sistema operativo
 *
 *  @param 		none
 *  @return     Estados del sistema operativo
 *****************************************************************************/
os_control_state_t os_get_controlState();

/******************************************************************************
 *  @brief Establece el estado del sistema operativo
 *
 *  @details
 *   Establece el estado actual del sistema operativo
 *
 *  @param 		newState	estado al que entrará el sistema operativo
 *  @return     none
 *****************************************************************************/
void os_set_controlState(os_control_state_t newState);

/******************************************************************************
 *  @brief Establece el inicio de una sección de código critica
 *
 *  @details
 *   Es muy importante que la sección crítica sea lo más corta posible
 *   ya que en dicha sección las interrupciones están deshabilitadas
 *
 *  @param 		none
 *  @return     none
 *****************************************************************************/
void os_enter_critical_zone();

/******************************************************************************
 *  @brief Establece el fin de una sección de código critica
 *
 *  @details
 *   Es muy importante que la sección crítica sea lo más corta posible
 *   ya que en dicha sección las interrupciones están deshabilitadas
 *
 *  @param 		none
 *  @return     none
 *****************************************************************************/
void os_exit_critical_zone();

/******************************************************************************
 *  @brief Deja una marca para indicar que se corre desde interrupción
 *
 *  @details
 *   Establece bandera para indicar al scheduler que se está ejecutando
 *   desde una interrupción
 *
 *  @param 		none
 *  @return     none
 *****************************************************************************/
void os_setSchedulingFromIRQ();

/******************************************************************************
 *  @brief Limpia la marca que indica que se corre desde interrupción
 *
 *  @details
 *   Limpia la bandera y de esta manera indica al scheduler que ya no se está
 *   ejecutando desde una interrupción
 *
 *  @param 		none
 *  @return     none
 *****************************************************************************/
void os_clearSchedulingFromIRQ();

/******************************************************************************
 *  @brief Informa si se está ejecutando desde una interrupción
 *
 *  @details
 *   Devuelve el estado de la bandera que indica que se está ejecutando
 *   desde una interrupción
 *
 *  @param 		none
 *  @return     true	Bandera indica ejecución desde una interrupción
 *****************************************************************************/
bool os_isSchedulingFromIRQ();

/******************************************************************************
 *  @brief Establece una situación de error
 *
 *  @details
 *   Adicionalmente llama al errorHook();
 *
 *  @param 	err			ID del error
 *  @param *caller		puntero a la rutina desde la cual se generó el error
 *  @return     true	Bandera indica ejecución desde una interrupción
 *****************************************************************************/
void os_setError(os_control_error_t err, void* caller);

/******************************************************************************
 *  @brief Obtiene el tiempo actual del sistema operativo
 *
 *  @details
 *   La granularidad es en ticks del sistema
 *
 *  @param 	none
 *  @return   ticks desde que comenzó a ejecutarse el sistema operativo
 *****************************************************************************/
uint32_t os_get_systemClockMs();

#endif /* ISO_I_2020_MSE_OS_INC_MSE_OS_CORE_H_ */
