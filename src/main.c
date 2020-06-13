/*==================[inclusions]=============================================*/

#include "main.h"

#include "board.h"
#include "sapi.h"

#include "MSE_OS_Core.h"
#include "MSE_OS_API.h"

/*==================[macros and definitions]=================================*/

#define MILISEC		1000

/*==================[Global data declaration]==============================*/

os_TaskHandler_t task1, task2, task3;


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
	int i;
	while (1) {
		i++;
		/*gpioToggle(LED1);
		os_Delay(200);*/
	}
}

void tarea2(void)  {
	int j;
	while (1) {
		j++;
		gpioToggle(LED2);
		os_Delay(300);
	}
}

void tarea3(void)  {
	int k;
	while (1) {
		k++;
		gpioToggle(LED3);
		os_Delay(50);
	}
}

/*============================================================================*/

int main(void)  {

	initHardware();

	os_InitTask(&task1, tarea1, 2);
	os_InitTask(&task2, tarea2, 2);
	os_InitTask(&task3, tarea3, 1);

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
