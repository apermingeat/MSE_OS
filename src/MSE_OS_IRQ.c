/*
 * MSE_OS_IRQ.c
 *
 *  Created on: 15 jun. 2020
 *      Author: Alejandro Permingeat
 */

#include "MSE_OS_IRQ.h"

static void* isr_user_handler_vector[OS_NUMBER_OF_IRQ];				//vector de punteros a funciones para nuestras interrupciones

bool os_insertIRQ(LPC43XX_IRQn_Type irq, void* isr_user_handler)
{
	bool result = false;

	if (isr_user_handler_vector[irq] == NULL)
	{
		isr_user_handler_vector[irq] = isr_user_handler;
		NVIC_ClearPendingIRQ(irq);
		NVIC_EnableIRQ(irq);
		result = true;
	}

	return result;
}

bool os_removeIRQ(LPC43XX_IRQn_Type irq)
{
	bool result = false;

	if (isr_user_handler_vector[irq] != NULL)
	{
		isr_user_handler_vector[irq] = NULL;
		NVIC_ClearPendingIRQ(irq);
		NVIC_DisableIRQ(irq);
		result = true;
	}

	return result;
}

void os_IRQHandler(LPC43XX_IRQn_Type IRQn)
{
	void (*user_IRQ_handler)(void);

	os_control_state_t previus_os_control_state;

	/*Guardar estado anterior del sistema operativo*/
	previus_os_control_state = os_get_controlState();

	/*Establecer el estado del sistema operativo para indicar que está atendiendo una interrupción */
	os_set_controlState(os_control_state__running_from_IRQ);

	/*Obtener el handler de interrupción asociado a la interrupción actual */
	user_IRQ_handler = isr_user_handler_vector[IRQn];

	/*Ejecutar el handler de interrupción del usuario*/
	if (NULL != user_IRQ_handler)
	{
		user_IRQ_handler();
	}

	/*Restablecer el estado anterior del sistema operativo*/
	os_set_controlState(previus_os_control_state);

	/*Limpiar el flag de la interrupción, sino volvería a generarse la misma interrupción*/
	NVIC_ClearPendingIRQ(IRQn);

	 /* Si hubo alguna llamada desde una interrupcion a una api liberando un evento, entonces
	 * llamamos al scheduler
	 */
	if (os_isSchedulingFromIRQ())  {
		os_clearSchedulingFromIRQ();
		os_CpuYield();
	}
}

/*==================[interrupt service routines]=============================*/

void DAC_IRQHandler(void){os_IRQHandler(         DAC_IRQn         );}
void M0APP_IRQHandler(void){os_IRQHandler(       M0APP_IRQn       );}
void DMA_IRQHandler(void){os_IRQHandler(         DMA_IRQn         );}
void FLASH_EEPROM_IRQHandler(void){os_IRQHandler(RESERVED1_IRQn   );}
void ETH_IRQHandler(void){os_IRQHandler(         ETHERNET_IRQn    );}
void SDIO_IRQHandler(void){os_IRQHandler(        SDIO_IRQn        );}
void LCD_IRQHandler(void){os_IRQHandler(         LCD_IRQn         );}
void USB0_IRQHandler(void){os_IRQHandler(        USB0_IRQn        );}
void USB1_IRQHandler(void){os_IRQHandler(        USB1_IRQn        );}
void SCT_IRQHandler(void){os_IRQHandler(         SCT_IRQn         );}
void RIT_IRQHandler(void){os_IRQHandler(         RITIMER_IRQn     );}
void TIMER0_IRQHandler(void){os_IRQHandler(      TIMER0_IRQn      );}
void TIMER1_IRQHandler(void){os_IRQHandler(      TIMER1_IRQn      );}
void TIMER2_IRQHandler(void){os_IRQHandler(      TIMER2_IRQn      );}
void TIMER3_IRQHandler(void){os_IRQHandler(      TIMER3_IRQn      );}
void MCPWM_IRQHandler(void){os_IRQHandler(       MCPWM_IRQn       );}
void ADC0_IRQHandler(void){os_IRQHandler(        ADC0_IRQn        );}
void I2C0_IRQHandler(void){os_IRQHandler(        I2C0_IRQn        );}
void SPI_IRQHandler(void){os_IRQHandler(         I2C1_IRQn        );}
void I2C1_IRQHandler(void){os_IRQHandler(        SPI_INT_IRQn     );}
void ADC1_IRQHandler(void){os_IRQHandler(        ADC1_IRQn        );}
void SSP0_IRQHandler(void){os_IRQHandler(        SSP0_IRQn        );}
void SSP1_IRQHandler(void){os_IRQHandler(        SSP1_IRQn        );}
void UART0_IRQHandler(void){os_IRQHandler(       USART0_IRQn      );}
void UART1_IRQHandler(void){os_IRQHandler(       UART1_IRQn       );}
void UART2_IRQHandler(void){os_IRQHandler(       USART2_IRQn      );}
void UART3_IRQHandler(void){os_IRQHandler(       USART3_IRQn      );}
void I2S0_IRQHandler(void){os_IRQHandler(        I2S0_IRQn        );}
void I2S1_IRQHandler(void){os_IRQHandler(        I2S1_IRQn        );}
void SPIFI_IRQHandler(void){os_IRQHandler(       RESERVED4_IRQn   );}
void SGPIO_IRQHandler(void){os_IRQHandler(       SGPIO_INT_IRQn   );}
void GPIO0_IRQHandler(void){os_IRQHandler(       PIN_INT0_IRQn    );}
void GPIO1_IRQHandler(void){os_IRQHandler(       PIN_INT1_IRQn    );}
void GPIO2_IRQHandler(void){os_IRQHandler(       PIN_INT2_IRQn    );}
void GPIO3_IRQHandler(void){os_IRQHandler(       PIN_INT3_IRQn    );}
void GPIO4_IRQHandler(void){os_IRQHandler(       PIN_INT4_IRQn    );}
void GPIO5_IRQHandler(void){os_IRQHandler(       PIN_INT5_IRQn    );}
void GPIO6_IRQHandler(void){os_IRQHandler(       PIN_INT6_IRQn    );}
void GPIO7_IRQHandler(void){os_IRQHandler(       PIN_INT7_IRQn    );}
void GINT0_IRQHandler(void){os_IRQHandler(       GINT0_IRQn       );}
void GINT1_IRQHandler(void){os_IRQHandler(       GINT1_IRQn       );}
void EVRT_IRQHandler(void){os_IRQHandler(        EVENTROUTER_IRQn );}
void CAN1_IRQHandler(void){os_IRQHandler(        C_CAN1_IRQn      );}
void ADCHS_IRQHandler(void){os_IRQHandler(       ADCHS_IRQn       );}
void ATIMER_IRQHandler(void){os_IRQHandler(      ATIMER_IRQn      );}
void RTC_IRQHandler(void){os_IRQHandler(         RTC_IRQn         );}
void WDT_IRQHandler(void){os_IRQHandler(         WWDT_IRQn        );}
void M0SUB_IRQHandler(void){os_IRQHandler(       M0SUB_IRQn       );}
void CAN0_IRQHandler(void){os_IRQHandler(        C_CAN0_IRQn      );}
void QEI_IRQHandler(void){os_IRQHandler(         QEI_IRQn         );}
