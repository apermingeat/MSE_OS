/*
 * MSE_OS_API.h
 *
 *  Created on: 13 junio 2020
 *      Author: Alejandro Permingeat
 *
 *  @brief Librería que contiene la interface pública
 *         del sistema operativo
 */

#ifndef INC_MSE_OS_API_H_
#define INC_MSE_OS_API_H_

/*==================[inclusions]=============================================*/
#include "MSE_OS_Core.h"

/*==================[macros and definitions]=================================*/
#define OS_QUEUE_HEAP_SIZE		256
#define OS_QUEUE_DEFAULT_VALUE 0xFF

typedef struct
{
	os_TaskHandler_t * takenByTask;
	bool	taken;
} os_Semaphore_t;

typedef struct
{
	uint16_t headID; /** queue header index */
	uint16_t tailID; /** queue tail index */
	uint16_t queueSize; /** queue actual number of inserted elements */
	uint16_t maxElements; /** queue maximum number of elements that can be inserted */
	uint16_t elementSize; /** element size in bytes */
	uint8_t data[OS_QUEUE_HEAP_SIZE]; /** queue internal heap*/
	os_TaskHandler_t * taskWaitingForIt; /** task waiting for element insertion in the queue */
} os_Queue_t;


/*==================[public functions]=======================================*/

/******************************************************************************
 *  @brief Espera no bloqueante.
 *
 *  @details
 *   Realiza una espera no bloqueante.
 *
 *  @param ticks				ticks del sistema operativo a esperar
 *  @return     none.
******************************************************************************/
void os_Delay(uint32_t ticks);

/******************************************************************************
 *  @brief Inicialización de un semáforo.
 *
 *  @details
 *   Los semáforos son binarios.
 *
 *  @param *sem				puntero a semáforo
 *  @return     none.
******************************************************************************/
void os_sem_init(os_Semaphore_t * sem);

/******************************************************************************
 *  @brief Tomar un semáforo.
 *
 *  @details
 *   Los semáforos son binarios y pueden ser tomados por solo una tarea a
 *   la vez. Solo una terea puede ser bloqueada a la espera del semáforo
 *
 *  @param *sem				puntero a semáforo
 *  @return     none.
******************************************************************************/
void os_sem_take(os_Semaphore_t * sem);


/******************************************************************************
 *  @brief Dar un semáforo.
 *
 *  @details
 *   Los semáforos son binarios y pueden ser tomados por solo una tarea a
 *   la vez. Una tarea al dar un semáforo no se bloquea si no hay ninguna
 *   otra tarea esperando por el semáforo.
 *
 *  @param *sem				puntero a semáforo
 *  @return     none.
******************************************************************************/
void os_sem_give(os_Semaphore_t * sem);


/******************************************************************************
 *  @brief Inicialización de una cola.
 *
 *  @details
 *   El tamáño de los elementos de la cola se selecciona al momento de
 *   inicializar la cola. Las colas tienen asignados un heap estático.
 *
 *  @param *queue				puntero a la cola
 *  @param dataSize				tamaño en bytes que tendrá cada elemento
 *  							de la cola
 *  @return     none.
******************************************************************************/
void os_queue_init(os_Queue_t * queue, uint16_t dataSize);

/******************************************************************************
 *  @brief Insersión de un elemento a la cola.
 *
 *  @details
 *   Si la cola no tiene espacio,la tarea que intenta insertar un elemento
 *   queda bloqueada hasta que otra tarea quite un elemento de la cola.
 *
 *  @param *queue				puntero a la cola
 *  @param *data				puntero al elemento que se insertará en la cola
 *  @return     none.
******************************************************************************/
void os_queue_insert(os_Queue_t * queue, void * data);

/******************************************************************************
 *  @brief Remoción de un elemento a la cola.
 *
 *  @details
 *   Si la cola está vacía,la tarea que intenta remover un elemento
 *   queda bloqueada hasta que otra tarea agregue un elemento de la cola.
 *
 *  @param *queue				puntero a la cola
 *  @param *data				puntero al elemento que se removió de la cola
 *  @return     none.
******************************************************************************/
void os_queue_remove(os_Queue_t * queue, void * data);


#endif /* INC_MSE_OS_API_H_ */
