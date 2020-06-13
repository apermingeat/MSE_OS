/*
 * MSE_OS_API.c
 *
 *  Created on: 13 jun. 2020
 *      Author: A. Permingeat
 */

#include "MSE_OS_API.h"
#include "MSE_OS_Core.h"

void os_Delay(uint32_t ticks)
{
	os_TaskHandler_t* actualTask;

	actualTask = os_getActualtask();

	if ((os_task_state__running == actualTask->state) &&
		(0 < ticks))
	{
		actualTask->blockedTicks = ticks;

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

void os_sem_init(os_Semaphore_t * sem)
{
	sem->taken = false;
	sem->takenByTask = NULL;
}

void os_sem_take(os_Semaphore_t * sem)
{
	bool taken = false;

	os_TaskHandler_t* actualTask;

	actualTask = os_getActualtask();

	if (os_task_state__running == actualTask->state)
	{
		while (!taken)
		{
			if (sem->taken)
			{
				/* esperar hasta que esté libre el semaforo*/
				actualTask->state = os_task_state__blocked;
				sem->takenByTask = actualTask;
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
}

void os_sem_give(os_Semaphore_t * sem)
{
	os_TaskHandler_t* actualTask;

	actualTask = os_getActualtask();

	if ((os_task_state__running == actualTask->state) &&
		(sem->taken) &&
		(NULL != sem->takenByTask))
	{
		sem->taken = false;
		sem->takenByTask->state = os_task_state__ready;
	}
}

