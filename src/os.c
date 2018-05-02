/*==================[inclusions]=============================================*/

#include "main.h"
#include "os.h"
#include "board.h"

/*==================[internal data definition]===============================*/

//osControl g_sControl_OS;
uint32_t IndTarea = 0;
uint32_t IndTareaSig = 0;
uint32_t CantidadTareas;

/********************************************************************************
Inicializacion de las tareas
 *******************************************************************************/

void init_task(void *tareaEp, tarea *tarea_struct, uint8_t prioridad)
{
	memset(tarea_struct->pila, 0, STACK_SIZE);
	tarea_struct->sp = (uint32_t)(tarea_struct->pila + STACK_SIZE/4 - 17);//direccion de memo que tiene que quedar el stack pointer
	tarea_struct->pila[STACK_SIZE/4-1] = 1<<24;//psr.t = 1 INITIAL_xPSR
	tarea_struct->pila[STACK_SIZE/4-2] = (uint32_t)tareaEp;//pc
	tarea_struct->pila[STACK_SIZE/4-3] = 0; //lr return hook
	//tarea_struct->pila[STACK_SIZE/4-8] = 0; //r0 lst parametro
	tarea_struct->pila[STACK_SIZE/4-9] = 0xFFFFFFF9; //lr para el stack EXC_RETURN

	tarea_struct->entry_point = tareaEp;

	tarea_struct->task_id = IndTarea;
	tarea_struct->prioridad = prioridad;
	tarea_struct->estado = TASK_READY;
	tarea_struct->ticks_bloqueada = 0;

	ListaTareas[IndTarea] = tarea_struct;
	IndTarea++;
	cantidad_Tareas = IndTarea;

}

/********************************************************************************
 Start_os
 *******************************************************************************/

void start_os(void)
{
	NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS)-1);
	//estado_sistema = OS_NORMAL_RUN;	//comenzamos nuestro sistema en modo normal (tarea)
	int i = 0;
	int a = 0;
	int b = 0;
	int c = 0;
	int d = 0;
	tarea *ListaPrio1[32];
	tarea *ListaPrio2[32];
	tarea *ListaPrio3[32];
	int P1 = 0;
	int P2 = 0;
	int P3 = 0;
	/*ORDENAR POR PRIORIDAD LA LISTA DE TEREAS:
	 * PRIORIDAD = 1; ALTA
	 * PRIORIDAD = 2; MEDIA
	 * PRIORIDAD = 3; BAJA
	 */
	for(i=0;i<=cantidad_Tareas - 1;i++)
	{
		switch(ListaTareas[i]->prioridad)
		{
		case 1:
			ListaPrio1[P1] = ListaTareas[i];
			P1++;
			break;
		case 2:
			ListaPrio2[P2] = ListaTareas[i];
			P2++;
			break;
		case 3:
			ListaPrio3[P3] = ListaTareas[i];
			P3++;
			break;
		default:
			break;
		}
	}
	for(a=0;a<P1;a++)
	{
		ListaTareas[a] = ListaPrio1[b];
		b++;
	}
	if(P1<cantidad_Tareas)
	{
		for(a=P1;a<P2+P1;a++)
		{
			ListaTareas[a] = ListaPrio2[c];
			c++;
		}
	}
	if(P1+P2<cantidad_Tareas)
	{
		for(a=P2+P1;a<P1+P2+P3;a++)
		{
			ListaTareas[a] = ListaPrio3[d];
			d++;
		}
	}
}

/********************************************************************************
 SysTick_Handler
 *******************************************************************************/

void SysTick_Handler(void)
{
	uint8_t i;
	for(i=0;i<cantidad_Tareas;i++)
	{
		if(ListaTareas[i]->estado == TASK_BLOKED && ListaTareas[i]->ticks_bloqueada > 0)
		{
			ListaTareas[i]->ticks_bloqueada--;
			if(ListaTareas[i]->ticks_bloqueada <= 0)
			{
				ListaTareas[i]->estado = TASK_READY;
			}
		}
	}
	TickHook();
	PendSV();

}

/********************************************************************************
 get_next_context
 *******************************************************************************/

uint32_t get_next_context(uint32_t sp_actual)
{
	static int actual_task = -1;
	uint32_t sp_next;
	uint32_t salir;

	switch(actual_task)
	{
	case 1:
		ListaTareas[IndTareaSig]->sp = sp_actual;
		//ListaTareas[IndTareaSig]->estado = TASK_READY;

		switch(ListaTareas[IndTareaSig]->estado)
					{
					case TASK_ERROR:

						break;
					case TASK_READY:

						break;
					case TASK_RUNNING:
						ListaTareas[IndTareaSig]->estado = TASK_READY;
						break;
					case TASK_BLOKED:

						break;
					case TASK_HALTED:

						break;
					default:

						break;
					}


		salir = 1;
		while(salir)
		{
			IndTareaSig++;
			if(IndTareaSig > cantidad_Tareas - 1)
			{
				IndTareaSig = 0;
			}
			switch(ListaTareas[IndTareaSig]->estado)
			{
			case TASK_ERROR:
				salir = 1;
				break;
			case TASK_READY:
				sp_next = ListaTareas[IndTareaSig]->sp;
				ListaTareas[IndTareaSig]->estado = TASK_RUNNING;
				salir = 0;
				break;
			case TASK_RUNNING:
				salir = 1;
				break;
			case TASK_BLOKED:
				salir = 1;
				break;
			case TASK_HALTED:
				salir = 1;
				break;
			default:
				salir = 1;
				break;
			}
		}
		/*IndTareaSig++;
		if(IndTareaSig > cantidad_Tareas - 1)
		{
			IndTareaSig = 0;
		}
		if(ListaTareas[IndTareaSig]->estado == TASK_READY)
		{
			sp_next = ListaTareas[IndTareaSig]->sp;
			ListaTareas[IndTareaSig]->estado = TASK_RUNNING;
		}*/
		actual_task = 2;
		break;
	case 2:
		ListaTareas[IndTareaSig]->sp = sp_actual;
		//ListaTareas[IndTareaSig]->estado = TASK_READY;

		switch(ListaTareas[IndTareaSig]->estado)
					{
					case TASK_ERROR:

						break;
					case TASK_READY:

						break;
					case TASK_RUNNING:
						ListaTareas[IndTareaSig]->estado = TASK_READY;
						break;
					case TASK_BLOKED:

						break;
					case TASK_HALTED:

						break;
					default:

						break;
					}


		salir = 1;
		while(salir)
		{
			IndTareaSig++;
			if(IndTareaSig > cantidad_Tareas - 1)
			{
				IndTareaSig = 0;
			}
			switch(ListaTareas[IndTareaSig]->estado)
			{
			case TASK_ERROR:
				salir = 1;
				break;
			case TASK_READY:
				sp_next = ListaTareas[IndTareaSig]->sp;
				ListaTareas[IndTareaSig]->estado = TASK_RUNNING;
				salir = 0;
				break;
			case TASK_RUNNING:
				salir = 1;
				break;
			case TASK_BLOKED:
				salir = 1;
				break;
			case TASK_HALTED:
				salir = 1;
				break;
			default:
				salir = 1;
				break;
			}
		}
		/*IndTareaSig++;
		if(IndTareaSig > cantidad_Tareas - 1)
		{
			IndTareaSig = 0;
		}
		if(ListaTareas[IndTareaSig]->estado == TASK_READY)
		{
			sp_next = ListaTareas[IndTareaSig]->sp;
			ListaTareas[IndTareaSig]->estado = TASK_RUNNING;
		}*/
		actual_task = 1;
		break;
	default:
		sp_next = ListaTareas[IndTareaSig]->sp;
		ListaTareas[IndTareaSig]->estado = TASK_RUNNING;
		actual_task = 1;
		break;
	}
	return sp_next;
}

void Delay(uint32_t ticks)
{
	os_enter_critical();
	ListaTareas[IndTareaSig]->estado = TASK_BLOKED;
	ListaTareas[IndTareaSig]->ticks_bloqueada = ticks;
	os_exit_critical();
	PendSV();
}

void PendSV(void)
{
	SCB -> ICSR = SCB_ICSR_PENDSVSET_Msk;
	__ISB();
	__DSB();
}

/********************************************************************************
 seccion critica
 *******************************************************************************/
inline void os_enter_critical()  {
	__disable_irq();
	contador_critico++;
}


/********************************************************************************
 seccion critica
 *******************************************************************************/
inline void os_exit_critical()  {
	if (--contador_critico <= 0)  {
		contador_critico = 0;
		__enable_irq();
	}
}

/********************************************************************************
 Funcion de inicializacion para los semaforos.
 *******************************************************************************/
void os_Semaforo_init(osSemaforo* X)  {
	X->tomado = TRUE;
	X->tarea_asociada = NULL;
}


//-------------------------------------------------------------------------------

/********************************************************************************
 * Funcion take semaforos
 *******************************************************************************/
void os_Semaforo_take(osSemaforo* X)  {
	bool Salir = FALSE;

	if (ListaTareas[IndTareaSig]->estado == TASK_RUNNING)  {
		/*
		 * Esto lo puse dentro de un while porque la tarea puede desbloquearse por otro
		 * evento, pero debe volver a bloquearse si no fue el semaforo quien la desbloqueo
		 */
		while (!Salir)  {

			if(X->tomado)  {
				os_enter_critical();

				//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				ListaTareas[IndTareaSig]->estado = TASK_BLOKED;		//estado de la tarea actual
				X->tarea_asociada = ListaTareas[IndTareaSig];			//guardamos la tarea que bloqueamos
				//----------------------------------------------------------------------------------------------------

				os_exit_critical();
				PendSV();			//devolvemos el CPU
			}
			else  {
				X->tomado = TRUE;
				Salir = TRUE;
			}
		}
	}
}

/********************************************************************************
 * Funcion give semaforos
 *******************************************************************************/
void os_Semaforo_give(osSemaforo* X)  {

	if (ListaTareas[IndTareaSig]->estado == TASK_RUNNING &&
		X->tomado == TRUE && X->tarea_asociada != NULL)  {

		X->tomado = FALSE;
		X->tarea_asociada->estado = TASK_READY;		//liberamos la tarea que habiamos bloqueado

		/*
		 * Si semaforo give se llama desde una interrupcion, debemos entonces
		 * avisar que es necesario llamar al scheduler
		 */
		if (estado_sistema == OS_IRQ)
			llamar_scheduler = TRUE;
	}
}
