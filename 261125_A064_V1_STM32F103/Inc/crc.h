/* CRC.h*/
/* Includes ------------------------------------------------------------------*/
#ifndef __CRC_H
#define __CRC_H
#include "stm32f1xx_hal.h"

uint16_t SLIP_CalculateCRC16 (); // розрахунок контрольної суми CRC16
void SLIP_AddCrcToFrame(); // додати контрольну суму до наявних даних

#endif
