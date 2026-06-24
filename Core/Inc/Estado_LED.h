/*
 * Status_led.h
 *
 *  Created on: Jan 23, 2026
 *      Author: raana
 */

#ifndef STATUS_LED_H
#define STATUS_LED_H

/* ================= INCLUDES ================= */

#include "main.h"
#include <stdint.h>
#include "stm32l4xx_hal.h"
/* ================= VARIABLES ================= */

// Enum para el modo del coche
typedef enum {
    CocheModo_MANUAL = 0,    // Se maneja con el Joystick
    CocheModo_AUTOMATIC       // Seguimiento automatico
} MODO_COCHE_t;

// Enum para el estado del LED
typedef enum {
    MODO_MANUAL = 0,
    MODO_AUTO_SEGUIMIENTO,
    MODO_AUTO_FRENANDO,
    MODO_AUTO_STOP
} COCHE_ESTADO_t;


extern volatile MODO_COCHE_t cocheModo;

/* ================= FUNCIONES LED ================= */

void LED_Init(void);
void LED_Update(COCHE_ESTADO_t estado);

#endif /* STATUS_LED_H */
