/*
 * MSE_OS_API.c
 *
 *  Created on: 17 mayo 2020
 *      Author: Alejandro Permingeat
 *
 *  @brief Librería que contiene el núcleo
 *         del sistema operativo
 */

/*==================[inclusions]=============================================*/
#include "MSE_OS_Core.h"
#include "board.h"

/*==================[macros and definitions]=================================*/
#define OS_MAX_ALLOWED_TASKS	8

/*==================[internal data definition]===============================*/
typedef struct
{
	os_TaskHandler_t *tasks[OS_MAX_ALLOWED_TASKS];
	uint8_t actualTaskId;
	uint8_t numberOfTasks;

} os_schedule_elem_t;

typedef struct
{
	os_schedule_elem_t tasksGroupedByPriority[OS_CONTROL_MAX_PRIORITY+1];

} os_schedule_control_t;


typedef struct
{
	os_schedule_control_t schedule;
	uint8_t actualTaskIndex;
	uint8_t tasksAdded;
	os_TaskHandler_t * actualTask;
	os_TaskHandler_t * nextTask;
	os_control_error_t error;
	os_control_state_t state;
	bool contextChangeNeeded;
	int16_t tasksInCriticalZone;
	bool schedulingFromIRQ;
	uint32_t systemClockTicks;
} os_control_t;

/*==================[Private data declaration]==============================*/

static os_control_t os_control;
static os_TaskHandler_t os_idleTask;

/*==================[Weak functions definition]=============================*/

void __attribute__((weak)) returnHook(void)  {
	while(1);
}

void __attribute__((weak)) tickHook(void)  {
	__asm volatile( "nop" );
}

/*El siguinte hook no está implementado aun*/
void __attribute__((weak)) taskIdleHook(void)  {
	__asm volatile( "nop" );
}

void __attribute__((weak)) errorHook(void *caller)  {
	/* el error del sistema operativo se encuentra en os_control.error
	 * TODO: agregar un método que devuelva el error*/
	while(1);
}

/*==================[Static headers]=========================================*/

static void setPendSV();
static void os_schedule();
static void initIdleTask();


/******************************************************************************
 * Funciones públicas (descripción de las mimas en MSE_OS_API.h)
 *****************************************************************************/

void os_InitTask(os_TaskHandler_t *taskHandler,
		void* entryPoint,
		uint8_t priority)
{

	if (os_control.tasksAdded >= OS_MAX_ALLOWED_TASKS)
	{
		os_control.error = os_control_error_max_task_exceeded;
		errorHook(os_InitTask);
	}
	else if (priority > OS_CONTROL_MAX_PRIORITY)
	{
		os_control.error = os_control_error_task_max_priority_exceeded;
		errorHook(os_InitTask);
	}
	else
	{
		taskHandler->stack[STACK_SIZE/4 - XPSR] = INIT_XPSR;					//necesario para bit thumb
		taskHandler->stack[STACK_SIZE/4 - PC_REG] = (uint32_t)entryPoint;		//direccion de la tarea (ENTRY_POINT)
		taskHandler->stack[STACK_SIZE/4 - LR] = (uint32_t)returnHook;			//Retorno en la rutina de la tarea. Esto no está permitido
		/**
		 * El valor previo de LR (que es EXEC_RETURN en este caso) es necesario dado que
		 * en esta implementacion, se llama a una funcion desde dentro del handler de PendSV
		 * con lo que el valor de LR se modifica por la direccion de retorno para cuando
		 * se termina de ejecutar getContextoSiguiente
		 */
		taskHandler->stack[STACK_SIZE/4 - LR_PREV] = EXEC_RETURN;

		taskHandler->entryPoint = entryPoint;

		taskHandler->stackPointer = (uint32_t)
				(taskHandler->stack + STACK_SIZE/4 - STACK_FRAME_ALL_RECORDS_SIZE);

		taskHandler->state = os_task_state__ready;

		taskHandler->taskID = os_control.tasksAdded;

		taskHandler->priority = priority;

		os_control.schedule.tasksGroupedByPriority[priority].tasks[
		      os_control.schedule.tasksGroupedByPriority[priority].numberOfTasks] = taskHandler;

		os_control.schedule.tasksGroupedByPriority[priority].numberOfTasks++;

		os_control.tasksAdded++;
	}
}

void os_Init(void)
{

	NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS)-1);

	initIdleTask();

	os_control.actualTask = NULL;
	os_control.nextTask = NULL;

	os_control.error = os_control_error_none;
	os_control.state = os_control_state__os_from_reset;

	os_control.tasksInCriticalZone = 0;

	os_setSchedulingFromIRQ(false);

	os_control.systemClockTicks = 0;
}

uint32_t getContextoSiguiente(uint32_t p_stack_actual)
{
	/* por defecto continuo con la tarea actual*/
	uint32_t p_stack_siguiente = p_stack_actual;

	if (os_control_state__os_from_reset == os_control.state)
	{
		p_stack_siguiente = os_control.actualTask->stackPointer;
		os_control.actualTask->state = os_task_state__running;
		os_control.state = os_control_state__os_running;
	}
	else
	{

		os_control.actualTask->stackPointer = p_stack_actual;

		if (os_task_state__running == os_control.actualTask->state)
		{
			os_control.actualTask->state = os_task_state__ready;
		}

		p_stack_siguiente = os_control.nextTask->stackPointer;

		os_control.actualTask = os_control.nextTask;
		os_control.actualTask->state = os_task_state__running;

	}

	return(p_stack_siguiente);
}

void os_CpuYield(void)
{
	os_schedule();
}


os_TaskHandler_t* os_getActualtask()
{
	return (os_control.actualTask);
}

os_control_state_t os_get_controlState()
{
	return(os_control.state);
}

void os_set_controlState(os_control_state_t newState)
{
	os_control.state = newState;
}

void os_enter_critical_zone()
{
	__disable_irq();
	os_control.tasksInCriticalZone++;
}

void os_exit_critical_zone()
{
	os_control.tasksInCriticalZone--;
	if (0 >= os_control.tasksInCriticalZone)
	{
		os_control.tasksInCriticalZone = 0;
		__enable_irq();
	}
}

void os_setSchedulingFromIRQ()
{
	os_control.schedulingFromIRQ = true;
}

void os_clearSchedulingFromIRQ()
{
	os_control.schedulingFromIRQ = false;
}

bool os_isSchedulingFromIRQ()
{
	return(os_control.schedulingFromIRQ);
}

void os_setError(os_control_error_t err, void* caller)
{
	os_control.error = err;
	errorHook(caller);
}

uint32_t os_get_systemClockMs()
{
	return(os_control.systemClockTicks);
}

/******************************************************************************
 * Funciones privadas
 *****************************************************************************/
/******************************************************************************
 *  @brief Inicialización de la tarea Idle
 *
 *  @details
 *   La tarea Idle es una tarea que se ejecuta cuando ninguna otra tarea
 *   esté en condiciones de ser ejecutada
 *
 *  @return     none.
 *****************************************************************************/
static void initIdleTask()
{
	os_idleTask.stack[STACK_SIZE/4 - XPSR] = INIT_XPSR;					//necesario para bit thumb
	os_idleTask.stack[STACK_SIZE/4 - PC_REG] = (uint32_t)taskIdleHook;		//direccion de la tarea (ENTRY_POINT)
	os_idleTask.stack[STACK_SIZE/4 - LR] = (uint32_t)returnHook;			//Retorno en la rutina de la tarea. Esto no está permitido
			/**
			 * El valor previo de LR (que es EXEC_RETURN en este caso) es necesario dado que
			 * en esta implementacion, se llama a una funcion desde dentro del handler de PendSV
			 * con lo que el valor de LR se modifica por la direccion de retorno para cuando
			 * se termina de ejecutar getContextoSiguiente
			 */
	os_idleTask.stack[STACK_SIZE/4 - LR_PREV] = EXEC_RETURN;

	os_idleTask.entryPoint = taskIdleHook;

	os_idleTask.stackPointer = (uint32_t) (os_idleTask.stack + STACK_SIZE/4 - STACK_FRAME_ALL_RECORDS_SIZE);

	os_idleTask.state = os_task_state__ready;

	os_idleTask.taskID = OS_IDLE_TASK_ID;
}

/******************************************************************************
 *  @brief Rutina de scheduling para tareas de la misma prioridad
 *
 *  @details
 *   Esta rutina es una rutina auxiliar utilizada por el scheduler.
 *   Particularmente se ocupa de seleccionar una tarea dentro de
 *   un conjunto de tareas de la misma prioridad
 *
 *  @param priority					prioridad del grupo de tareas a analizar
 *  @return     none.
 *****************************************************************************/
static os_TaskHandler_t * os_select_next_task_by_pririty(uint8_t priority)
{
	uint8_t id;
	bool seekForTask, allBlocked;
	uint8_t blockedTasksCounter = 0;
	os_TaskHandler_t * taskSelected = NULL;

	id = os_control.schedule.tasksGroupedByPriority[priority].actualTaskId;
	/* Solo buscar una posible tarea a ejecutar, si al menos hay una tarea
	 * agregada en esta prioridad */
	if (0 < os_control.schedule.tasksGroupedByPriority[priority].numberOfTasks)
	{
		seekForTask = true;
		allBlocked = false;
		while (seekForTask)
		{
			id++;
			if (id >= os_control.schedule.tasksGroupedByPriority[priority].numberOfTasks)
			{
				id = 0;
			}
			switch (os_control.schedule.tasksGroupedByPriority[priority].tasks[id]->state)
			{
				case 	os_task_state__ready:
					seekForTask = false;
					break;
				case	os_task_state__blocked:
					blockedTasksCounter++;
					if (blockedTasksCounter >=
							os_control.schedule.tasksGroupedByPriority[priority].numberOfTasks)
					{
						/*all tasks blocked*/
						seekForTask = false;
						allBlocked = true;
					}
					break;
				case os_task_state__running:
					/*all tasks are blocked except the one that was running*/
					seekForTask = false;
					break;
				case os_task_state__suspended:
					/*Nothing to do now*/
					break;
				default:
					os_control.state = os_control_state__os_error;
					os_control.error = os_control_error_task_with_invalid_state;
					seekForTask = false;
					errorHook(os_select_next_task_by_pririty);
			}
		}

		if (allBlocked)
		{
			/*No one task ready for actual priority */
			taskSelected = NULL;
		}
		else if (id != os_control.schedule.tasksGroupedByPriority[priority].actualTaskId)
		{
			os_control.schedule.tasksGroupedByPriority[priority].actualTaskId = id;
			taskSelected = os_control.schedule.tasksGroupedByPriority[priority].tasks[id];
		}
		else
		{
			/*only actual task is ready to continue*/
			taskSelected = os_control.schedule.tasksGroupedByPriority[priority].tasks[id];
		}
	}

	return (taskSelected);
}

/******************************************************************************
 *  @brief Implementa la política de scheduling.
 *
 *  @details
 *   Implementa una política de scheduling con prioridades. Para aquellas
 *   tareas que tengan la misma prioridad, ejecuta una política de
 *   scheduling
 *
 *  @return     none.
 *****************************************************************************/
static void os_schedule()
{
	uint8_t priority = 0;
	os_TaskHandler_t * taskSelected = NULL;

	os_control.contextChangeNeeded = false;

	if (os_control_state__os_from_reset == os_control.state)
	{
		if (0 == os_control.tasksAdded)
		{
			os_control.state = os_control_state__os_error;
			os_control.error = os_control_error_no_task_added;
			errorHook(os_schedule);

		}
		else
		{
			/*seleccionar la primer tarea para que sea ejecutada*/
			/*se selecciona como primer tarea a ejecutar la tarea idle*/
			os_control.actualTask = &os_idleTask;
			os_control.contextChangeNeeded = true;
		}
	}
	else
	{
		/* Checkear que el SO no esté en medio de un scheduling en otro hilo */
		if (os_control_state__os_running == os_control.state)
		{
			os_control.state = os_control_state__os_scheduling;
			while ((OS_CONTROL_MAX_PRIORITY >= priority) && (NULL == taskSelected))
			{
				taskSelected = os_select_next_task_by_pririty(priority);
				priority++;
			}
			if (NULL == taskSelected)
			{
				taskSelected = &os_idleTask;
			}
			os_control.contextChangeNeeded = (os_control.nextTask != taskSelected);

			os_control.nextTask = taskSelected;
			os_control.state = os_control_state__os_running;
		}
	}

	if (os_control.contextChangeNeeded)
	{
		setPendSV();
	}
}

/******************************************************************************
 *  @brief Actualiza los ticks restantes en las tareas bloqueadas
 *
 *  @details
 *   Para aquellas tareas que están bloqueadas y tienen que esperar una
 *   cierta cantidad de ticks, actualiza los ticks restantes.
 *   Esta rutina debe llamarse cada vez que se produce un tick del sistema.
 *
 *  @return     none.
 *****************************************************************************/
static void os_updateTicksInAllTaskBlocked()
{
	uint8_t priorityID;
	uint8_t i;
	for (priorityID = 0; priorityID <= OS_CONTROL_MAX_PRIORITY; priorityID++)
	{
		for (i = 0; i< os_control.schedule.tasksGroupedByPriority[priorityID].numberOfTasks; i++)
		{
			if ((os_task_state__blocked == os_control.schedule.tasksGroupedByPriority[priorityID].tasks[i]->state) &&
				(0 < os_control.schedule.tasksGroupedByPriority[priorityID].tasks[i]->blockedTicks))
			{
				os_control.schedule.tasksGroupedByPriority[priorityID].tasks[i]->blockedTicks--;
				if (0 == os_control.schedule.tasksGroupedByPriority[priorityID].tasks[i]->blockedTicks)
				{
					os_control.schedule.tasksGroupedByPriority[priorityID].tasks[i]->state = os_task_state__ready;
				}
			}
		}
	}
}

/******************************************************************************
 *  @brief Activa el flag de PendSV.
 *
 *  @details
 *   Habilita el bit correspondiente a la excepción de PendSV.
 *
 *  @return     none.
 *****************************************************************************/
static void setPendSV()
{
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



/******************************************************************************
 * Handler de interrupciones
 *****************************************************************************/

/******************************************************************************
 *  @brief Handler de la interrupción de SysTick.
 *
 *  @details
 *   Ejecuta el scheduling para determinar cual será la próxima tarea a
 *   ejecutarse y ejecuta el tickHook.
 *
 *  @return     none.
 *****************************************************************************/
void SysTick_Handler(void)
{
	os_control.systemClockTicks++;

	os_updateTicksInAllTaskBlocked();

	os_schedule();

	/*Ejecutar el hook asociado al tick*/
	tickHook();
}



