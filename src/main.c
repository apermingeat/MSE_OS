/*==================[inclusions]=============================================*/

#include "main.h"

#include "board.h"
#include "sapi.h"

#include "MSE_OS_Core.h"
#include "MSE_OS_API.h"

/*==================[macros and definitions]=================================*/

#define MILISEC		1000

/*==================[Global data declaration]==============================*/

os_TaskHandler_t task1, task2, task3, button_task;
os_Semaphore_t	semLed1, semLed2, semLed3;


/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

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
	gpioWrite(LED3,false);
	while (1) {
		if (i%10 == 0)
		{
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

void button_task_handler(void)
{
	while(1)  {
		if(!gpioRead( TEC1 ))
			os_sem_give(&semLed1);

		if(!gpioRead( TEC2 ))
			os_sem_give(&semLed2);

		if(!gpioRead( TEC3 ))
			os_sem_give(&semLed3);

		os_Delay(100);
	}
}

/*============================================================================*/

int main(void)  {

	initHardware();

	os_sem_init(&semLed1);
	os_sem_init(&semLed2);
	os_sem_init(&semLed3);

	os_InitTask(&task1, tarea1, 2);
	os_InitTask(&task2, tarea2, 2);
	os_InitTask(&task3, tarea3, 1);
	os_InitTask(&task3, tarea3, 1);
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
