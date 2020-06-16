/*
 * MSE_OS_IRQ.h
 *
 *  Created on: 15 junio 2020
 *      Author: Alejandro Permingeat
 *
 *  @brief Librería que contiene el manejo de interrupciones
 *         del sistema operativo
 */

#ifndef INC_MSE_OS_IRQ_H_
#define INC_MSE_OS_IRQ_H_

/*==================[inclusions]=============================================*/
#include "MSE_OS_Core.h"
#include "MSE_OS_API.h"
#include "board.h"
#include "cmsis_43xx.h"

/*==================[macros and definitions]=================================*/
#define OS_NUMBER_OF_IRQ	53


/*==================[public functions]=======================================*/

/******************************************************************************
 *  @brief Inserta un handler de una interrupción.
 *
 *  @details
 *   Adicionalmente habilita dicha interrupcion.
 *
 *  @param irq					ID de interrupción.
 *  @param *isr_user_handler	Puntero al handler de interrupcion
 *  @return     True si tuvo éxito.
 *****************************************************************************/
bool os_insertIRQ(LPC43XX_IRQn_Type irq, void* isr_user_handler);

/******************************************************************************
 *  @brief Remueve un handler de una interrupción.
 *
 *  @details
 *   Adicionalmente deshabilita dicha interrupcion.
 *
 *  @param irq					ID de interrupción.
 *  @return     True si tuvo éxito.
 *****************************************************************************/
bool os_removeIRQ(LPC43XX_IRQn_Type irq);

#endif /* INC_MSE_OS_IRQ_H_ */
