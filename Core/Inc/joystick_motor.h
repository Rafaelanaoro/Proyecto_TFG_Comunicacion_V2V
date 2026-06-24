/*
 * joystick_motor.h
 *
 *  Created on: Jan 21, 2026
 *      Author: raana
 */

#ifndef INC_JOYSTICK_MOTOR_H_
#define INC_JOYSTICK_MOTOR_H_

#include "stm32l4xx_hal.h"

/* ================= Variables ================= */

extern int16_t g_velocidad;
extern int16_t g_angulo;
extern uint8_t g_freno;

/* ========= CONFIGURACIÓN ========= */

#define PWM_MAX         1000   // maxima potencia de los motores
#define PWM_MIN  		180     // mínimo para que el motor gire
#define ZONA_MUERTA     80

/* ========= INICIALIZACIÓN ========= */

void JoystickMotor_Init(void);

/* ========= CONTROL DE MOTORES ========= */

void Motor_Stop(void);
void Motor_Set(int16_t left, int16_t right);
void Motor_Freno_Seco(void);

/* ========= JOYSTICK ========= */

void Joystick_Update(void);

#endif /* __JOYSTICK_MOTOR_H */
