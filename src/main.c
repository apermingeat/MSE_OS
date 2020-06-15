/*==================[inclusions]=============================================*/

#include "main.h"

#include "board.h"
#include "sapi.h"

#include "MSE_OS_Core.h"
#include "MSE_OS_API.h"

#include <string.h>

/*==================[macros and definitions]=================================*/

#define MILISEC		1000

/*==================[Global data declaration]==============================*/

os_TaskHandler_t task1, task2, task3, task4, task5, button_task;
os_Semaphore_t	semLed1, semLed2, semLed3;
os_Queue_t		queue1, queue2;

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/
typedef struct
{
	float	floatData;
	int16_t intData;
	char message[8];
} queueElement_t;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/** @brief hardware initialization function
 *	@return none
 */
static void initHardware(void)  {
	Board_Init();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / MILISEC);		//systick 1ms
}


/*==================[Definicion de tareas para el OS]==========================*/
void tarea1(void)  {
	int i = 0;
	queueElement_t queueData;
	queueData.floatData = 3.1421;
	queueData.intData = -90;
	strcpy(queueData.message, "test");
	gpioWrite(LED3,false);
	while (1) {
		if (i%10 == 0)
		{
			os_queue_insert(&queue1, &queueData);
			os_sem_take(&semLed3);
		}
		gpioToggle(LED3);
		os_Delay(200);
		i++;
	}
}

void tarea2(void)  {
	int j = 0;
	gpioWrite(LED2,false);
	while (1) {
		if (j%10 == 0)
		{
			os_sem_take(&semLed2);
		}
		gpioToggle(LED2);
		os_Delay(300);
		j++;
	}
}

void tarea3(void)  {
	int k = 0;
	gpioWrite(LED1,false);
	while (1) {
		if (k%10 == 0)
		{
			os_sem_take(&semLed1);
		}
		gpioToggle(LED1);
		os_Delay(150);
		k++;
	}
}

void tarea4(void)  {
	int k = 0;
	queueElement_t queueData;
	while (1) {
		if (k%4 == 0)
		{
			os_queue_remove(&queue1, &queueData);
		}
		gpioWrite(LEDB,true);
		os_Delay(500);
		gpioWrite(LEDB,false);
		os_Delay(500);
		k++;
	}
}

void tarea5(void)  {
	int k = 0;
	queueElement_t queueData;
	while (1) {
		os_queue_remove(&queue2, &queueData);
		gpioWrite(LEDG,true);
		os_Delay(1000);
		gpioWrite(LEDG,false);
		os_Delay(1000);
		k++;
	}
}

void button_task_handler(void)
{
	queueElement_t queueData;
	queueData.floatData = 189.1421;
	queueData.intData = -87;
	strcpy(queueData.message, "test2");
	while(1)  {
		if(!gpioRead( TEC1 ))
			os_sem_give(&semLed1);

		if(!gpioRead( TEC2 ))
			os_sem_give(&semLed2);

		if(!gpioRead( TEC3 ))
			os_sem_give(&semLed3);

		if(!gpioRead( TEC4 ))
			os_queue_insert(&queue2, &queueData);;

		os_Delay(100);
	}
}

/*============================================================================*/

int main(void)  {

	initHardware();

	os_sem_init(&semLed1);
	os_sem_init(&semLed2);
	os_sem_init(&semLed3);

	os_queue_init(&queue1, sizeof(queueElement_t));
	os_queue_init(&queue2, sizeof(queueElement_t));

	os_InitTask(&task1, tarea1, 2);
	os_InitTask(&task2, tarea2, 2);
	os_InitTask(&task3, tarea3, 1);
	os_InitTask(&task4, tarea4, 0);
	os_InitTask(&task5, tarea5, 2);
	os_InitTask(&button_task, button_task_handler, 3);


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
