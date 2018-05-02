/*==================[inclusions]=============================================*/

#include "main.h"
#include "board.h"
#include "os.h"
#include "sapi_uart.h"
#include <stdlib.h>
#include <string.h>

#include "os_IRQ.h"

/*==================[macros and definitions]=================================*/

#define TEC1_PORT_NUM   0
#define TEC1_BIT_VAL    4

#define TEC2_PORT_NUM   0
#define TEC2_BIT_VAL    8


#define LED1_PORT_NUM   0
#define LED1_BIT_VAL    14

#define LED2_PORT_NUM   1
#define LED2_BIT_VAL    11


#define TEC1_P    1
#define TEC1_P_   0
#define TEC1_GPIO 0
#define TEC1_PIN  4

#define TEC2_P    1
#define TEC2_P_   1
#define TEC2_GPIO 0
#define TEC2_PIN  8

#define TEC3_P    1
#define TEC3_P_   2
#define TEC3_GPIO 0
#define TEC3_PIN  9

#define TEC4_P    1
#define TEC4_P_   6
#define TEC4_GPIO 1
#define TEC4_PIN  9

/*==================[internal data declaration]==============================*/

enum _Teclas {Orden_00, Orden_01,Orden_10,Orden_11, No_solapan};

typedef enum _Teclas Orden;

/*==================[internal functions declaration]=========================*/

/** @brief hardware initialization function
 *	@return none
 */
static void initHardware(void);

/*==================[internal data definition]===============================*/

tarea strucTarea1, strucTarea2, strucTarea3, strucTarea4, strucTarea5, strucTarea6, strucTarea7;

uint32_t Contador_global = 0;
uint32_t Descendente1, Descendente2 = 0;		//contador de ticks global definido por el usuario
uint32_t Ascendente1, Ascendente2 = 0;		//contador de ticks global definido por el usuario
osSemaforo sem_Tecla1_Down, sem_Tecla1_Up, sem_Tecla2_Down, sem_Tecla2_Up, sem_cont, sem_envio;

uint32_t flag = 0;
uint32_t flag1 = 0;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void initHardware(void)
{
	Board_Init();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);
	//100000b - 1b = 01111b => prioridad mas baja del Pendsv(siempre pendsv debe tener la minima)


	/* Config EDU-CIAA-NXP Button Pins as GPIOs */
	Chip_SCU_PinMux(TEC1_P, TEC1_P_, MD_PUP|MD_EZI|MD_ZI, FUNC0); /* P1_0,  GPIO0[4], TEC1 */
	Chip_SCU_PinMux(TEC2_P, TEC2_P_, MD_PUP|MD_EZI|MD_ZI, FUNC0); /* P1_1,  GPIO0[8], TEC2 */
	Chip_SCU_PinMux(TEC3_P, TEC3_P_, MD_PUP|MD_EZI|MD_ZI, FUNC0); /* P1_2,  GPIO0[9], TEC3 */
	Chip_SCU_PinMux(TEC4_P, TEC4_P_, MD_PUP|MD_EZI|MD_ZI, FUNC0); /* P1_6,  GPIO1[9], TEC4 */

	/* Config EDU-CIAA-NXP Button Pins as Inputs */
	Chip_GPIO_SetDir(LPC_GPIO_PORT, TEC1_GPIO, (1<<TEC1_PIN), 0);
	Chip_GPIO_SetDir(LPC_GPIO_PORT, TEC3_GPIO, (1<<TEC3_PIN), 0);
	Chip_GPIO_SetDir(LPC_GPIO_PORT, TEC2_GPIO, (1<<TEC2_PIN), 0);
	Chip_GPIO_SetDir(LPC_GPIO_PORT, TEC4_GPIO, (1<<TEC4_PIN), 0);


	/*
		 * Seteamos la interrupcion 0 para el flanco descendente en la tecla 1
		 */
		Chip_SCU_GPIOIntPinSel( 0, TEC1_PORT_NUM, TEC1_BIT_VAL );
		Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 0 ) ); // INT0
		Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( 0 ) );
		Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH( 0 ) );

		/*
		 * Seteamos la interrupcion 1 para el flanco ascendente en la tecla 1
		 */
		Chip_SCU_GPIOIntPinSel( 1, TEC1_PORT_NUM, TEC1_BIT_VAL );
		Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 1 ) ); // INT1
		Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( 1 ) );
		Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH( 1 ) );

		/*
		 * Seteamos la interrupcion 2 para el flanco descendente en la tecla 2
		 */
		Chip_SCU_GPIOIntPinSel( 2, TEC2_PORT_NUM, TEC2_BIT_VAL );
		Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 2 ) ); // INT2
		Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( 2 ) );
		Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH( 2 ) );

		/*
		 * Seteamos la interrupcion 3 para el flanco ascendente en la tecla 2
		 */
		Chip_SCU_GPIOIntPinSel( 3, TEC2_PORT_NUM, TEC2_BIT_VAL );
		Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 3 ) ); // INT3
		Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( 3 ) );
		Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH( 3 ) );

		/* Inicializar UART_USB a 115200 baudios */
		uartConfig( UART_USB, 115200 );


}

/*==================[INTERRUPCIONES DE LAS TECLAS]===========================*/
void GPIO0_IRQHandler(void)  {


     if ( Chip_PININT_GetFallStates( LPC_GPIO_PIN_INT ) & PININTCH0 )  {
        Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 0 ) );
        os_Semaforo_give(&sem_Tecla1_Down);

    }
     Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 0 ) );
}

void GPIO1_IRQHandler(void)  {


     if ( Chip_PININT_GetFallStates( LPC_GPIO_PIN_INT ) & PININTCH1 )  {
        Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 1 ) );
        os_Semaforo_give(&sem_Tecla1_Up);

    }
     Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 1 ) );
}

void GPIO2_IRQHandler(void)  {


     if ( Chip_PININT_GetFallStates( LPC_GPIO_PIN_INT ) & PININTCH2 )  {
        Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 2 ) );
        os_Semaforo_give(&sem_Tecla2_Down);
    }
     Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 2 ) );
}

void GPIO3_IRQHandler(void)  {


     if ( Chip_PININT_GetFallStates( LPC_GPIO_PIN_INT ) & PININTCH3 )  {
        Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 3 ) );
        os_Semaforo_give(&sem_Tecla2_Up);
    }
     Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 3 ) );
}

/*==================[external functions definition]==========================*/
void task1(void)
{
	while (1) {
		Delay(20);
		os_Semaforo_take(&sem_Tecla1_Down);
		Descendente1 = Contador_global;

	}

}

void task2(void)
{
	while (1) {
		Delay(20);
		os_Semaforo_take(&sem_Tecla1_Up);
		Ascendente1 = Contador_global;
		os_Semaforo_give(&sem_cont);
		flag =1;
	}
}

void task3(void)
{
	while (1) {
		Delay(20);
		os_Semaforo_take(&sem_Tecla2_Down);
		Descendente2 = Contador_global;

	}
}

void task4(void)
{
	while (1) {
		Delay(20);
		os_Semaforo_take(&sem_Tecla2_Up);
		Ascendente2 = Contador_global;
		os_Semaforo_give(&sem_cont);
		flag1 = 1;
	}
}

void task5(void)
{
	while (1) {
		os_Semaforo_take(&sem_cont);
		if(flag ==1 && flag1 == 1){
			os_Semaforo_give(&sem_envio);
			flag = 0;
			flag1 = 0;
		}


	}
}

void task6(void)
{

	uint32_t Orden_presion;
	uint32_t Tiempo1 = 0;
	uint32_t Tiempo2 = 0;
	char Buffer1[17];
	char Buffer2[17];
	char ms1[11];
	char ms2[11];
	while (1){

		os_Semaforo_take(&sem_envio);

		if(Descendente1 <= Descendente2 && Ascendente1 >= Ascendente2)
		{
			Orden_presion = Orden_00;
		}
		else if(Descendente1 >= Descendente2 && Ascendente1 <= Ascendente2)
		{
			Orden_presion = Orden_01;
		}
		else if(Descendente1 <= Descendente2 && Ascendente1 <= Ascendente2)
		{
			Orden_presion = Orden_10;
		}
		else if(Descendente1 >= Descendente2 && Ascendente1 >= Ascendente2)
		{
			Orden_presion = Orden_11;
		}
		else if(Ascendente1 < Descendente2)
		{
			Orden_presion = No_solapan;
		}
		else if(Descendente1 > Ascendente2)
		{
			Orden_presion = No_solapan;
		}

		Buffer1[0] = 0;		//Borramos la cadena al inicio de cada transmision
		Buffer2[0] = 0;
		strcat(Buffer1,"{");	//llave de apertura
		strcat(Buffer2,"{");	//llave de apertura


		switch (Orden_presion)
		{
		case Orden_00:
			Tiempo1 = abs(Descendente2 - Descendente1);
			Tiempo2 = abs(Ascendente1 - Ascendente2);
			strcat(Buffer1,"0:1:");
			strcat(Buffer2,"1:0:");
			break;
		case Orden_01:
			Tiempo1 = abs(Descendente2 - Descendente1);
			Tiempo2 = abs(Ascendente1 - Ascendente2);
			strcat(Buffer1,"1:0:");
			strcat(Buffer2,"0:1:");
			break;
		case Orden_10:
			Tiempo1 = abs(Descendente1 - Descendente2);
			Tiempo2 = abs(Ascendente1 - Ascendente2);
			strcat(Buffer1,"0:1:");
			strcat(Buffer2,"0:1:");
			break;
		case Orden_11:
			Tiempo1 = abs(Descendente2 - Descendente1);
			Tiempo2 = abs(Ascendente1 - Ascendente2);
			strcat(Buffer1,"1:0:");
			strcat(Buffer2,"1:0:");
			break;
		case No_solapan:
			strcat(Buffer1,"N:N:");
			strcat(Buffer2,"N:N:");
			break;
		default:
			break;
		}

		itoa(Tiempo1,ms1,10);
		itoa(Tiempo2,ms2,10);
		strcat(Buffer1,ms1);
		strcat(Buffer2,ms2);
		strcat(Buffer1,"}\n\r");
		strcat(Buffer2,"}\n\r");

		uartWriteString(UART_USB, Buffer1);
		uartWriteString(UART_USB, Buffer2);
		Tiempo1 = 0;
		Tiempo2 = 0;
		Ascendente1 = 0;
		Ascendente2 = 0;
	}
}

void task7(void)
{
	while (1){
	Board_LED_Toggle(5);
	//Delay(300);
	}
}

void TickHook(void)  {
	Contador_global++;
}

int main(void)
{
	initHardware();

	os_Semaforo_init(&sem_Tecla1_Down);
	os_Semaforo_init(&sem_Tecla1_Up);
	os_Semaforo_init(&sem_Tecla2_Down);
	os_Semaforo_init(&sem_Tecla2_Up);
	os_Semaforo_init(&sem_envio);
	os_Semaforo_init(&sem_cont);

	install_IRQ(PIN_INT0_IRQn);
	install_IRQ(PIN_INT1_IRQn);
	install_IRQ(PIN_INT2_IRQn);
	install_IRQ(PIN_INT3_IRQn);

	init_task(task1, &strucTarea1, 1);
	init_task(task2, &strucTarea2, 1);
	init_task(task3, &strucTarea3, 1);
	init_task(task4, &strucTarea4, 1);
	init_task(task5, &strucTarea5, 2);
	init_task(task6, &strucTarea6, 2);
	init_task(task7, &strucTarea7, 3);

	start_os();

	while (1) {
	}
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
