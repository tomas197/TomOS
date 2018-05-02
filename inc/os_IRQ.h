#ifndef _OS_IRQ_H_
#define _OS_IRQ_H_


#include "os.h"
#include "board.h"
#include "cmsis_43xx.h"

#define CANT_IRQ	53

bool install_IRQ(LPC43XX_IRQn_Type irq);
bool remove_IRQ(LPC43XX_IRQn_Type irq);


#endif /* G_OS_INC_G_OS_IRQ_H_ */
