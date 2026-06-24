/*
 * VL53_Seguimiento.h
 *
 *  Created on: Jan 21, 2026
 *      Author: raana
 */

#ifndef INC_VL53_SEGUIMIENTO_H_
#define INC_VL53_SEGUIMIENTO_H_

#include "stm32l4xx_hal.h"
#include <stdint.h>

/* ================= CONFIGURACIÓN ================= */
#define Distancia_seguimeinto 250
#define Distancia_frenado_brusco 150
#define MAX_PWM 1000

// PID
#define KP 2.2f 					//Error Presente(el coche acelera mas o menos agresivo)
#define KI 0.10f 					//Error Pasado(ayuda al coche a moverse milimetricamente)
#define KD 1.0f 					//Error futuro (Ayuda a predecir y evitar tirones)

#define Limite_correccion 900 		//Evita que el error aumente demasiado
#define Filtro_temporal_frenado  80 //Evita "fantasmas"
#define PWM_RAMP 200					//Evita que pase de 0 a 1000 y produzca problemas(Se reinice el microcontrolador)
#define PWM_MIN_ARRANQUE 600		//Fuerza que rompre la friccion de los motores

/* ================= ESTADOS ================= */
typedef enum {
	Seguimiento_OK = 0, Seguimiento_FRENADO, Seguimiento_PARADA_EMERGENCIA, Seguimiento_BUSCANDO
} SeguimientoState_t;

/* ================= API ================= */
void Seguimiento_Init(void);
SeguimientoState_t Seguimiento_Update(void);
uint16_t VL53_ReadDistance(void);
static void Recover_I2C(void);

#endif
