#ifndef _OS_H_
#define _OS_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "board.h"

#define STACK_SIZE 512



/********************************************************************************
 * Definicion de los estados posibles para las tareas
 *******************************************************************************/

enum _estadoTarea{
	TASK_ERROR = 0,
	TASK_READY = 1,
	TASK_RUNNING = 2,
	TASK_BLOKED = 3,
	TASK_HALTED = 4
};
typedef enum _estadoTarea estadoTarea;

/********************************************************************************
 Definicion de la estructura para cada tarea
 *******************************************************************************/
struct _task  {
	uint32_t pila[STACK_SIZE/4];
	uint32_t sp;
	void *entry_point;
	uint8_t task_id;
	estadoTarea estado;
	uint8_t prioridad;
	uint32_t ticks_bloqueada;
	char nombre[10];
};

typedef struct _task tarea;

/********************************************************************************
 * Definicion de la estructura de control para el sistema operativo
 *******************************************************************************/

	tarea *ListaTareas[32];		//array de punteros a tareas


	uint8_t cantidad_Tareas;		//cantidad de tareas definidas por el usuario



	/********************************************************************************
	 * Definicion de los estados posibles de nuestro OS
	 *******************************************************************************/

	enum _estadoOS  {
		OS_IRQ,
		OS_NORMAL_RUN
	};

	typedef enum _estadoOS estadoOS;

	estadoOS estado_sistema;
	bool llamar_scheduler;
	uint32_t contador_critico;

void start_os();
void init_task(void *tareaEp, tarea *tarea_struct, uint8_t prioridad);
void Delay(uint32_t ticks);
void PendSV(void);
inline void os_enter_critical();
inline void os_exit_critical();

void TickHook(void);

/********************************************************************************
 Definicion de la estructura para los semaforos
 *******************************************************************************/
struct _semaforo  {
	tarea* tarea_asociada;
	bool tomado;
};

typedef struct _semaforo osSemaforo;




//prototipos de funciones
void os_Semaforo_init(osSemaforo* X);
void os_Semaforo_take(osSemaforo* X);
void os_Semaforo_give(osSemaforo* X);

bool os_install_IRQ(LPC43XX_IRQn_Type irq, void* usr_irq);
bool os_remove_IRQ(LPC43XX_IRQn_Type irq);

#endif /* #ifndef _OS_H_ */
