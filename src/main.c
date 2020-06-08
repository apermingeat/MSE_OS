/*==================[inclusions]=============================================*/

#include "main.h"

#include "board.h"

#include "MSE_OS_Core.h"

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
	}
}

void tarea2(void)  {
	int j;
	while (1) {
		j++;
	}
}

void tarea3(void)  {
	int k;
	while (1) {
		k++;
	}
}

/*============================================================================*/

int main(void)  {

	initHardware();

	os_Init();

	os_InitTask(&task1, tarea1);
	os_InitTask(&task2, tarea2);
	os_InitTask(&task3, tarea3);

	while (1) {
		__WFI();
	}
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
