/*
 * MSE_OS_API.h
 *
 *  Created on: 13 jun. 2020
 *      Author: A. Permingeat
 */

#ifndef INC_MSE_OS_API_H_
#define INC_MSE_OS_API_H_

#include "MSE_OS_Core.h"

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

void os_Delay(uint32_t ticks);

void os_sem_init(os_Semaphore_t * sem);
void os_sem_take(os_Semaphore_t * sem);
void os_sem_give(os_Semaphore_t * sem);

void os_queue_init(os_Queue_t * queue, uint16_t dataSize);
void os_queue_insert(os_Queue_t * queue, void * data);
void os_queue_remove(os_Queue_t * queue, void * data);


#endif /* INC_MSE_OS_API_H_ */
