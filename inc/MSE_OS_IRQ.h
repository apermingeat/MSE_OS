/*
 * MSE_OS_IRQ.h
 *
 *  Created on: 15 jun. 2020
 *      Author: Alejandro Permingeat
 */

#ifndef INC_MSE_OS_IRQ_H_
#define INC_MSE_OS_IRQ_H_

#include "MSE_OS_Core.h"
#include "MSE_OS_API.h"
#include "board.h"
#include "cmsis_43xx.h"

#define OS_NUMBER_OF_IRQ	53

bool os_insertIRQ(LPC43XX_IRQn_Type irq, void* isr_user_handler);

bool os_removeIRQ(LPC43XX_IRQn_Type irq);

#endif /* INC_MSE_OS_IRQ_H_ */
