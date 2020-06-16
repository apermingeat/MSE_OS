/*==================[inclusions]=============================================*/

#include "board.h"
#include "sapi.h"

#include "MSE_OS_Core.h"
#include "MSE_OS_API.h"
#include "MSE_OS_IRQ.h"

#include <string.h>

/*==================[macros and definitions]=================================*/

#define MILISEC		1000

#define TEC1_PORT_NUM   0
#define TEC1_BIT_VAL    4

#define TEC2_PORT_NUM   0
#define TEC2_BIT_VAL    8

#define MAX_STRING_MESSAGE	256
#define MAX_STRING_LENGTH_FOR_NUMBER_VALUE	20

/*==================[Global data declaration]==============================*/

os_TaskHandler_t task1, task2, task3, task4, task5, button_task;
os_Semaphore_t	semLed1, semLed2, semLed3;
os_Queue_t		queue1, queueLed, queueUart;

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/
typedef enum
{
	tecla1_down,
	tecla2_down,
	tecla1_up,
	tecla2_up
} eventos_t;

typedef enum
{
	wait_tecla1_down_o_tecla2_down,
	wait_tecla1_down,
	wait_tecla2_down,
	wait_tecla1_up_o_tecla2_up,
	wait_tecla1_up,
	wait_tecla2_up,
} estados_t;

typedef struct
{
	eventos_t evento;
} queueElement_t;


typedef enum
{
	led_verde,
	led_rojo,
	led_amarillo,
	led_azul
} led_id_t;

typedef struct
{
	led_id_t led;
	uint32_t time;
} ledQueueElement_t;

typedef struct
{
	led_id_t led;
	uint32_t t1;
	uint32_t t2;
} uartQueueElement_t;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/** @brief hardware initialization function
 *	@return none
 */
static void initHardware(void)  {
	Board_Init();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / MILISEC);		//systick 1ms


	/*
	 * Seteamos la interrupcion 0 para el flanco descendente en la tecla 1
	 */
	Chip_SCU_GPIOIntPinSel( 0, TEC1_PORT_NUM, TEC1_BIT_VAL );
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 0 ) ); // INT0 flanco descendente
	Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( 0 ) );
	Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH( 0 ) );

	/*
	 * Seteamos la interrupcion 1 para el flanco ascendente en la tecla 1
	 */
	Chip_SCU_GPIOIntPinSel( 1, TEC1_PORT_NUM, TEC1_BIT_VAL );
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 1 ) ); // INT1 flanco ascendente
	Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( 1 ) );
	Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH( 1 ) );


	/*
	 * Seteamos la interrupcion 2 para el flanco descendente en la tecla 2
	 */
	Chip_SCU_GPIOIntPinSel( 2, TEC2_PORT_NUM, TEC2_BIT_VAL );
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 2 ) ); // INT0 flanco descendente
	Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( 2 ) );
	Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH( 2 ) );

	/*
	 * Seteamos la interrupcion 3 para el flanco ascendente en la tecla 2
	 */
	Chip_SCU_GPIOIntPinSel( 3, TEC2_PORT_NUM, TEC2_BIT_VAL );
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 3 ) ); // INT1 flanco ascendente
	Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( 3 ) );
	Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH( 3 ) );

	/* Inicializar UART_USB a 115200 baudios */
	uartConfig( UART_USB, 115200 );

}


/*==================[Definicion de tareas para el OS]==========================*/

void tareaControl()
{
	estados_t estado;
	queueElement_t mesg;
	ledQueueElement_t ledMsg;
	uartQueueElement_t uartMsg;

	uint32_t t1_start_timestamp,t2_start_timestamp;
	uint32_t t1, t2;

	bool firstTec1Down, firstTec1Up, finDeSecuencia;

	finDeSecuencia = false;

	estado = wait_tecla1_down_o_tecla2_down;

	while(1)
	{

		os_queue_remove(&queue1, &mesg);

		switch (estado)
		{
			case wait_tecla1_down_o_tecla2_down:
				if (tecla1_down == mesg.evento)
				{
					firstTec1Down = true;
					estado = wait_tecla2_down;
					t1_start_timestamp = os_get_systemClockMs();
				}
				else if (tecla2_down == mesg.evento)
				{
					firstTec1Down = false;
					estado = wait_tecla1_down;
					t1_start_timestamp = os_get_systemClockMs();
				}
				break;
			case wait_tecla1_down:
				if (tecla1_down == mesg.evento)
				{
					estado = wait_tecla1_up_o_tecla2_up;
					t1 = os_get_systemClockMs() - t1_start_timestamp;
				}
				else if (tecla2_up == mesg.evento)
				{
					/* Se soltó la tecla 2 sin antes presionar la tecla 1*/
					estado = wait_tecla1_down_o_tecla2_down;
				}
				break;
			case wait_tecla2_down:
				if (tecla2_down == mesg.evento)
				{
					estado = wait_tecla1_up_o_tecla2_up;
					t1 = os_get_systemClockMs() - t1_start_timestamp;
				}
				else if (tecla1_up == mesg.evento)
				{
					/* Se soltó la tecla 1 sin antes presionar la tecla 2*/
					estado = wait_tecla1_down_o_tecla2_down;
				}
				break;
			case wait_tecla1_up_o_tecla2_up:
				if (tecla1_up == mesg.evento)
				{
					firstTec1Up = true;
					estado = wait_tecla2_up;
					t2_start_timestamp = os_get_systemClockMs();
				}
				else if (tecla2_up == mesg.evento)
				{
					firstTec1Up = false;
					estado = wait_tecla1_up;
					t2_start_timestamp = os_get_systemClockMs();
				}
				break;
			case wait_tecla1_up:
				if (tecla1_up == mesg.evento)
				{
					estado = wait_tecla1_down_o_tecla2_down;
					t2 = os_get_systemClockMs() - t2_start_timestamp;
					finDeSecuencia  = true;
					/* */
				}
				else if (tecla2_down == mesg.evento)
				{
					/* Se volvio a presionar la tecla 2 sin antes soltar la tecla 1*/
					/* TODO ver que hacer*/
				}
				break;
			case wait_tecla2_up:
				if (tecla2_up == mesg.evento)
				{
					estado = wait_tecla1_down_o_tecla2_down;
					t2 = os_get_systemClockMs() - t2_start_timestamp;
					finDeSecuencia  = true;
					/* */
				}
				else if (tecla1_down == mesg.evento)
				{
					/* Se volvio a presionar la tecla 2 sin antes soltar la tecla 1*/
					/* TODO ver que hacer*/
				}
				break;
			default:
				estado = wait_tecla1_down_o_tecla2_down;
				finDeSecuencia = false;
				break;

		}

		if (finDeSecuencia)
		{
			finDeSecuencia = false;
			if (firstTec1Down)
			{
				if (firstTec1Up)
				{
					ledMsg.led = led_verde;
				}
				else
				{
					ledMsg.led = led_rojo;
				}
			}
			else
			{
				if (firstTec1Up)
				{
					ledMsg.led = led_amarillo;
				}
				else
				{
					ledMsg.led = led_azul;
				}
			}
			ledMsg.time = t1 + t2;
			os_queue_insert(&queueLed, &ledMsg);
			uartMsg.led = ledMsg.led;
			uartMsg.t1 = t1;
			uartMsg.t2 = t2;
			os_queue_insert(&queueUart, &uartMsg);

		}
	}
}

void ledsControlTask()
{
	gpioMap_t led;
	ledQueueElement_t msg;
	while(1)
	{
		os_queue_remove(&queueLed, &msg);
		switch (msg.led)
		{
			case led_verde: led = LEDG; break;
			case led_rojo: led = LEDR; break;
			case led_amarillo: led = LED2; break;
			case led_azul: led = LEDB; break;
		}
		gpioWrite(led, true);
		os_Delay(msg.time);
		gpioWrite(led, false);
	}
}


void uartNotificationTask()
{
	static char message[MAX_STRING_MESSAGE];
	static char number[MAX_STRING_LENGTH_FOR_NUMBER_VALUE];
	uartQueueElement_t msg;
	message[0] = 0;

	while(1)
	{
		os_queue_remove(&queueUart, &msg);
		message[0] = 0;
		strcat(&message, "Led ");
		switch (msg.led)
		{
			case led_verde: strcat(&message, "Verde"); break;
			case led_rojo: strcat(&message, "Rojo"); break;
			case led_amarillo: strcat(&message, "Amarillo"); break;
			case led_azul: strcat(&message, "Azul"); break;
		}
		strcat(&message, " encendido:​ \n\r");

		strcat(&message, " \t ​ Tiempo encendido: ​");
		itoa(msg.t1+msg.t2,number,10);
		strcat(&message, number);
		strcat(&message, " ms​ \n\r");

		strcat(&message, " \t ​ Tiempo entre flancos descendentes: ​");
		itoa(msg.t1,number,10);
		strcat(&message, number);
		strcat(&message, " ms​ \n\r");

		strcat(&message, " \t ​ Tiempo entre flancos ascendentes:: ​");
		itoa(msg.t2,number,10);
		strcat(&message, number);
		strcat(&message, " ms​ \n\r");

		uartWriteString(UART_USB, message);
	}
}

void tecla1_down_ISR()
{
	queueElement_t mesg;
	mesg.evento = tecla1_down;
	os_queue_insert(&queue1, &mesg);
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 0 ) );
}

void tecla1_up_ISR()
{
	queueElement_t mesg;
	mesg.evento = tecla1_up;
	os_queue_insert(&queue1, &mesg);
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 1 ) );
}

void tecla2_down_ISR()
{
	queueElement_t mesg;
	mesg.evento = tecla2_down;
	os_queue_insert(&queue1, &mesg);
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 2 ) );
}

void tecla2_up_ISR()
{
	queueElement_t mesg;
	mesg.evento = tecla2_up;
	os_queue_insert(&queue1, &mesg);
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 3 ) );
}

/*============================================================================*/

int main(void)  {

	initHardware();

	os_queue_init(&queue1, sizeof(queueElement_t));
	os_queue_init(&queueLed, sizeof(ledQueueElement_t));
	os_queue_init(&queueUart, sizeof(uartQueueElement_t));

	os_InitTask(&task1, tareaControl, 1);
	os_InitTask(&task2, ledsControlTask, 0);
	os_InitTask(&task3, uartNotificationTask, 2);

	os_insertIRQ(PIN_INT0_IRQn, tecla1_down_ISR);
	os_insertIRQ(PIN_INT1_IRQn,tecla1_up_ISR);
	os_insertIRQ(PIN_INT2_IRQn, tecla2_down_ISR);
	os_insertIRQ(PIN_INT3_IRQn,tecla2_up_ISR);



	os_Init();

	while (1) {
		__WFI();
	}
}

static uint32_t global_tickCounter = 0;
void tickHook(void)
{
	global_tickCounter++;
}

static uint32_t globla_idleTaskCounter = 0;
void taskIdleHook()
{
	while(1)
	{
		globla_idleTaskCounter++;
	}
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
