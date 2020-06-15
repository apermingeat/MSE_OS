/*
 * MSE_OS_API.c
 *
 *  Created on: 13 jun. 2020
 *      Author: A. Permingeat
 */

#include "MSE_OS_API.h"
#include "MSE_OS_Core.h"
#include <string.h>

void os_Delay(uint32_t ticks)
{
	os_TaskHandler_t* actualTask;

	if (0 < ticks)
	{
		os_enter_critical_zone();
		actualTask = os_getActualtask();
		actualTask->blockedTicks = ticks;
		os_exit_critical_zone();

		/*
		 * El proximo bloque while tiene la finalidad de asegurarse que la tarea solo se desbloquee
		 * en el momento que termine la cuenta de ticks. Si por alguna razon la tarea se vuelve a
		 * ejecutar antes que termine el periodo de bloqueado, queda atrapada.
		 *
		 */

		while (0 < actualTask->blockedTicks)
		{
			actualTask->state = os_task_state__blocked;
			os_CpuYield();
		}
	}
}


/******************************************************************************
 *	Semaforos
 ******************************************************************************/

void os_sem_init(os_Semaphore_t * sem)
{
	sem->taken = false;
	sem->takenByTask = NULL;
}

void os_sem_take(os_Semaphore_t * sem)
{
	bool taken = false;

	os_TaskHandler_t* actualTask;

	while (!taken)
	{
		if (sem->taken)
		{
			/* esperar hasta que esté libre el semaforo*/

			os_enter_critical_zone();
			actualTask = os_getActualtask();
			actualTask->state = os_task_state__blocked;
			sem->takenByTask = actualTask;
			os_exit_critical_zone();

			os_CpuYield();
		}
		else
		{
			sem->taken = true;
			sem->takenByTask = actualTask;
			taken = true;
		}
	}
}

void os_sem_give(os_Semaphore_t * sem)
{
	if ((sem->taken) &&
		(NULL != sem->takenByTask))
	{
		sem->taken = false;
		sem->takenByTask->state = os_task_state__ready;
	}
}

/******************************************************************************
 *	Colas
 ******************************************************************************/
void os_queue_init(os_Queue_t * queue, uint16_t dataSize)
{
	queue->elementSize = dataSize;
	queue->queueSize = 0;
	queue->maxElements = OS_QUEUE_HEAP_SIZE / dataSize;
	queue->taskWaitingForIt = NULL;
	queue->headID = 0;
	queue->tailID = 0;
	memset(queue->data, OS_QUEUE_DEFAULT_VALUE,OS_QUEUE_HEAP_SIZE);
}

void os_queue_insert(os_Queue_t * queue, void * data)
{
	os_TaskHandler_t* actualTask;

	/** Se debe realizar los siguiente:
	 *  1 - Verificar si la cola estaba vacia.
	 *  2 - En caso de que estaba vacia verificar si había una tarea esperando por un elemento
	 *  3 - Si había una tarea esperando por un elemento, verificar si su estado era blocked y
	 *      pasarla a ready */

	if (0 == queue->queueSize)
	{
		if ((NULL != queue->taskWaitingForIt) &&
				(os_task_state__blocked == queue->taskWaitingForIt->state))
		{
			queue->taskWaitingForIt->state = os_task_state__ready;
		}
	}

	/* Mientras la cola esté llena, poner la tarea actual en estado bloqueado y
	 * ceder el CPU */
	while (queue->queueSize >= queue->maxElements)
	{
		os_enter_critical_zone();
		actualTask = os_getActualtask();
		actualTask->state = os_task_state__blocked;
		queue->taskWaitingForIt = actualTask;
		os_exit_critical_zone();

		os_CpuYield();
	}

	/* Realizar la inserción del elemento en la cola */
	memcpy(queue->data + (queue->headID * queue->elementSize), data, queue->elementSize);
	queue->headID++;
	if (queue->headID >= queue->maxElements)
	{
		queue->headID = 0;
	}
	queue->queueSize++;
	queue->taskWaitingForIt = NULL;
}

void os_queue_remove(os_Queue_t * queue, void * data)
{
	os_TaskHandler_t* actualTask;

	/** Se debe realizar los siguiente:
	 *  1 - Verificar si la cola estaba llena.
	 *  2 - En caso de que estaba llena verificar si había una tarea esperando por insertar un elemento
	 *  3 - Si había una tarea esperando por insertar un elemento, verificar si su estado era blocked y
	 *      pasarla a ready */

	if (queue->queueSize >= queue->maxElements)
	{
		if ((NULL != queue->taskWaitingForIt) &&
				(os_task_state__blocked == queue->taskWaitingForIt->state))
		{
			queue->taskWaitingForIt->state = os_task_state__ready;
		}
	}

	/* Verifico que la tarea actual esté corriendo (y que no haya sido desalojada por el scheduler durante
	 * la ejecución de la primera parte de esta rutina */


	/* Mientras la cola esté vacia, poner la tarea actual en estado bloqueado y
	 * ceder el CPU */
	while (0 == queue->queueSize)
	{
		os_enter_critical_zone();
		actualTask = os_getActualtask();
		actualTask->state = os_task_state__blocked;
		queue->taskWaitingForIt = actualTask;
		os_exit_critical_zone();

		os_CpuYield();
	}

	/* Realizar la remoción del elemento en la cola */
	memcpy(data, queue->data + (queue->tailID * queue->elementSize), queue->elementSize);
	queue->tailID++;
	if (queue->tailID >= queue->maxElements)
	{
		queue->tailID = 0;
	}
	queue->queueSize--;
	queue->taskWaitingForIt = NULL;
}

