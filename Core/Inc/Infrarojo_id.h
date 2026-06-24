/*
 * Infrarojo_id.h
 *
 *  Created on: May 29, 2026
 *      Author: raana
 */

#ifndef INC_INFRAROJO_ID_H_
#define INC_INFRAROJO_ID_H_

#include "stm32l4xx_hal.h"
#include <stdint.h>

#define IR_TX_PORT GPIOA
#define IR_TX_PIN  GPIO_PIN_2

#define IR_RX_PORT GPIOB
#define IR_RX_PIN  GPIO_PIN_1

/* ================= API ================= */
void Infrarrojo_Init(void);
void Infrarrojo_Enviar_ID(uint8_t id);
uint8_t Infrarrojo_Recibir_ID(uint8_t *id_recibido);


#endif /* INC_INFRAROJO_ID_H_ */
