/*
 * MSE_OS_API.h
 *
 *  Created on: 13 jun. 2020
 *      Author: A. Permingeat
 */

#ifndef INC_MSE_OS_API_H_
#define INC_MSE_OS_API_H_

#include "MSE_OS_Core.h"

typedef struct
{
	os_TaskHandler_t * takenByTask;
	bool	taken;
} os_Semaphore_t;

void os_Delay(uint32_t ticks);

void os_sem_init(os_Semaphore_t * sem);
void os_sem_take(os_Semaphore_t * sem);
void os_sem_give(os_Semaphore_t * sem);

#endif /* INC_MSE_OS_API_H_ */
