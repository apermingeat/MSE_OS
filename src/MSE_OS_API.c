/*
 * MSE_OS_API.c
 *
 *  Created on: 13 jun. 2020
 *      Author: alejandro
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
