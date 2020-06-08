/*
 * MSE_OS_Core.c
 *
 *  Created on: 17 mayo 2020
 *      Author: Alejandro Permingeat
 */

#include "MSE_OS_Core.h"
#include "board.h"


typedef enum
{
	os_state_running,
	os_state_ready,
	os_state_blocked,
	os_state_suspended

} os_TaskState_t;

typedef struct
{
	uint32_t stack[STACK_SIZE];
	uint32_t *stackPointer;
	void *entryPoint;
	os_TaskState_t state;
	uint8_t priority;
	uint8_t taskID;
	uint32_t blockedTicks;
} os_TaksHandler_t;

static uint8_t id_tarea_actual = 0;


/*************************************************************************************************
	 *  @brief Inicializa las tareas que correran en el OS.
     *
     *  @details
     *   Inicializa una tarea para que pueda correr en el OS implementado.
     *   Es necesario llamar a esta funcion para cada tarea antes que inicie
     *   el OS.
     *
	 *  @param *tarea			Puntero a la tarea que se desea inicializar.
	 *  @param *stack			Puntero al espacio reservado como stack para la tarea.
	 *  @param *stack_pointer   Puntero a la variable que almacena el stack pointer de la tarea.
	 *  @return     None.
***************************************************************************************************/
void os_InitTarea(void *tarea, uint32_t *stack_tarea, uint32_t *stack_pointer)  {

	stack_tarea[STACK_SIZE/4 - XPSR] = INIT_XPSR;								//necesari para bit thumb
	stack_tarea[STACK_SIZE/4 - PC_REG] = (uint32_t)tarea;		//direccion de la tarea (ENTRY_POINT)

	/**
	 * El valor previo de LR (que es EXEC_RETURN en este caso) es necesario dado que
	 * en esta implementacion, se llama a una funcion desde dentro del handler de PendSV
	 * con lo que el valor de LR se modifica por la direccion de retorno para cuando
	 * se termina de ejecutar getContextoSiguiente
	 */
	stack_tarea[STACK_SIZE/4 - LR_PREV] = EXEC_RETURN;

	*stack_pointer = (uint32_t) (stack_tarea + STACK_SIZE/4 - STACK_FRAME_ALL_RECORDS_SIZE);

}
/*************************************************************************************************
	 *  @brief Inicializa el OS.
     *
     *  @details
     *   Inicializa el OS seteando la prioridad de PendSV como la mas baja posible. Es necesario
     *   llamar esta funcion antes de que inicie el sistema. Es recomendable llamarla luego de
     *   inicializar las tareas
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
void os_Init(void)  {
	/*
	 * Todas las interrupciones tienen prioridad 0 (la maxima) al iniciar la ejecucion. Para que
	 * no se de la condicion de fault mencionada en la teoria, debemos bajar su prioridad en el
	 * NVIC. La cuenta matematica que se observa da la probabilidad mas baja posible.
	 */
	NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS)-1);
}


/*************************************************************************************************
	 *  @brief SysTick Handler.
     *
     *  @details
     *   El handler del Systick no debe estar a la vista del usuario. Dentro se setea como
     *   pendiente la excepcion PendSV.
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
void SysTick_Handler(void)  {

	/**
	 * Se setea el bit correspondiente a la excepcion PendSV
	 */
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;

	/**
	 * Instruction Synchronization Barrier; flushes the pipeline and ensures that
	 * all previous instructions are completed before executing new instructions
	 */
	__ISB();

	/**
	 * Data Synchronization Barrier; ensures that all memory accesses are
	 * completed before next instruction is executed
	 */
	__DSB();
}

/*************************************************************************************************
	 *  @brief Get contexto siguiente.
     *
     *  @details
     *   Esta rutina determinará cual tarea debe ejecutarse a continuación. Para ello devolverá
     *   un puntero al stack de la siguiente tarea a ejecutar
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
uint32_t getContextoSiguiente(uint32_t p_stack_actual)
{
	uint32_t p_stack_siguiente = p_stack_actual; /* por defecto continuo con la tarea actual*/

	switch(id_tarea_actual)
	{
		case 0 : /* es la inicialización, debo continuar con la tarea con ID 1 */
			/*aquí no resguardo el stack del main, lo descarto porque no se utilizará más*/
			id_tarea_actual = 1;
			p_stack_siguiente = sp_tarea1;
			break;
		case 1: /* tarea actual con ID 1, debo continuar con la tarea con ID 2 */
			sp_tarea1 = p_stack_actual;
			id_tarea_actual = 2;
			p_stack_siguiente = sp_tarea2;
			break;
		case 2: /* tarea actual con ID 2, debo continuar con la tarea con ID 3 */
			sp_tarea2 = p_stack_actual;
			id_tarea_actual = 3;
			p_stack_siguiente = sp_tarea3;
			break;
		case 3: /* tarea actual con ID 3, debo continuar con la tarea con ID 1 */
			sp_tarea3 = p_stack_actual;
			id_tarea_actual = 1;
			p_stack_siguiente = sp_tarea1;
			break;
	}
	return(p_stack_siguiente);
}

