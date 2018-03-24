/*==================[inclusions]=============================================*/

#include "main.h"
#include "board.h"
#include <string.h>

/*==================[macros and definitions]=================================*/

#define STACK_SIZE 512

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/** @brief hardware initialization function
 *	@return none
 */
static void initHardware(void);

/*==================[internal data definition]===============================*/

static uint32_t pila1[STACK_SIZE/4];
static uint32_t pila2[STACK_SIZE/4];
uint32_t sp1;
uint32_t sp2;

uint32_t Delay_LED = 500;


/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void initHardware(void)
{
	Board_Init();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);
	//100000b - 1b = 01111b => prioridad mas baja del Pendsv(siempre pendsv debe tener la minima)
	NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS)-1);
}

void init_task(void (*entry_point) (void), uint32_t *stack, uint32_t * sp)
{
	memset(stack, 0, STACK_SIZE);
	*sp = (uint32_t)(stack+STACK_SIZE/4-17);//direccion de memo que tiene que quedar el stack pointer
	stack [STACK_SIZE/4-1] = 1<<24;//psr.t = 1 INITIAL_xPSR
	stack [STACK_SIZE/4-2] = (uint32_t)entry_point;//pc
	stack [STACK_SIZE/4-3] = 0; //lr return hook
	stack [STACK_SIZE/4-8] = 0; //r0 lst parametro
	stack [STACK_SIZE/4-9] = 0xFFFFFFF9; //lr para el stack EXC_RETURN
}

static void start_os(void)
{

}

/*==================[external functions definition]==========================*/
//lo sacamos xq lo hacemos en assembler
/*void SysTick_Handler(void)
{
}*/

void task1(void)
{
	int i;
	while (1) {
		i++;
		if(i >= 1000)
		{
			Board_LED_Toggle(3);
			i=0;
		}
	}
}

void task2(void)
{
	int j;
	while (1) {
		j++;
		if(j >= 1000)
		{
			Board_LED_Toggle(4);
			j=0;
		}
	}
}

void SysTick_Handler(void)
{
	SCB -> ICSR = SCB_ICSR_PENDSVSET_Msk;
	__ISB();
	__DSB();
}

//uint32_t get_next_context(uint32_t sp_actual)
uint32_t get_next_context(uint32_t sp_actual)
{
	//hacer esta funcion para n tares
	//crear_tarea(numero de tarea, prioridad)


	static int actual_task = -1;
	uint32_t sp_next;

	switch(actual_task)
	{
	case 1:
		sp1 = sp_actual;
		sp_next = sp2;
		actual_task = 2;
		break;
	case 2:
		sp2 = sp_actual;
		sp_next = sp1;
		actual_task = 1;
		break;
	default:
		sp_next = sp1;
		actual_task = 1;
		break;
	}
	return sp_next;

}

int main(void)
{
	initHardware();

	init_task(task1, pila1, &sp1);
	init_task(task2, pila2, &sp2);

	start_os();

	while (1) {
	}
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
