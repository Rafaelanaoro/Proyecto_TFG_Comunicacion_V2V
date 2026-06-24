/*
 * joystick_motor.C
 *
 *  Created on: Jan 21, 2026
 *      Author: raana
 */

/* ================= INCLUDES ================= */

#include "joystick_motor.h"
#include "main.h"
#include <stdlib.h>
#include <stdio.h>
#include "stm32l4xx_hal.h"

/* ================= Variables ================= */
extern TIM_HandleTypeDef htim3;
extern ADC_HandleTypeDef hadc1;

static uint16_t centro_x = 0;
static uint16_t centro_y = 0;

int16_t g_velocidad = 0;
int16_t g_angulo = 0;
uint8_t g_freno = 0;
/* ================= LLamadas ================= */

static int16_t mapJoystick(uint32_t val, uint16_t center);

/* ================= FUNCION INIT ================= */

void JoystickMotor_Init(void) {
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

    uint32_t sum_x = 0, sum_y = 0;

    // Calibración manual
    for (int i = 0; i < 100; i++) {
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 10);
        sum_x += HAL_ADC_GetValue(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 10);
        sum_y += HAL_ADC_GetValue(&hadc1);
        HAL_ADC_Stop(&hadc1);
    }

    centro_x = sum_x / 100;
    centro_y = sum_y / 100;
}

/* ================= FUNCIONES DE MOVIMIENTO DE LOS MOTORES ================= */

void Motor_Stop(void) {
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);

	HAL_GPIO_WritePin(MOTOR_A_IN1_GPIO_Port, MOTOR_A_IN1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(MOTOR_A_IN2_GPIO_Port, MOTOR_A_IN2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(MOTOR_B_IN3_GPIO_Port, MOTOR_B_IN3_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(MOTOR_B_IN4_GPIO_Port, MOTOR_B_IN4_Pin, GPIO_PIN_RESET);
}

void Motor_Freno_Seco(void) {
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, PWM_MAX);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, PWM_MAX);

	HAL_GPIO_WritePin(MOTOR_A_IN1_GPIO_Port, MOTOR_A_IN1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR_A_IN2_GPIO_Port, MOTOR_A_IN2_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(MOTOR_B_IN3_GPIO_Port, MOTOR_B_IN3_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR_B_IN4_GPIO_Port, MOTOR_B_IN4_Pin, GPIO_PIN_SET);
}

void Motor_Set(int16_t izq, int16_t der) {
    // Motor izquierdo
    if (izq > 0) {
        HAL_GPIO_WritePin(MOTOR_A_IN1_GPIO_Port, MOTOR_A_IN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(MOTOR_A_IN2_GPIO_Port, MOTOR_A_IN2_Pin, GPIO_PIN_RESET);
    } else if (izq < 0) {
        HAL_GPIO_WritePin(MOTOR_A_IN1_GPIO_Port, MOTOR_A_IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(MOTOR_A_IN2_GPIO_Port, MOTOR_A_IN2_Pin, GPIO_PIN_SET);
        izq = -izq;
    } else {
        // Freno si es exactamente 0, es decir si el joystick no se toca
        HAL_GPIO_WritePin(MOTOR_A_IN1_GPIO_Port, MOTOR_A_IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(MOTOR_A_IN2_GPIO_Port, MOTOR_A_IN2_Pin, GPIO_PIN_RESET);
    }

    // Motor derecho
    if (der > 0) {
        HAL_GPIO_WritePin(MOTOR_B_IN3_GPIO_Port, MOTOR_B_IN3_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(MOTOR_B_IN4_GPIO_Port, MOTOR_B_IN4_Pin, GPIO_PIN_RESET);
    } else if (der < 0) {
        HAL_GPIO_WritePin(MOTOR_B_IN3_GPIO_Port, MOTOR_B_IN3_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(MOTOR_B_IN4_GPIO_Port, MOTOR_B_IN4_Pin, GPIO_PIN_SET);
        der = -der;
    } else {
        HAL_GPIO_WritePin(MOTOR_B_IN3_GPIO_Port, MOTOR_B_IN3_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(MOTOR_B_IN4_GPIO_Port, MOTOR_B_IN4_Pin, GPIO_PIN_RESET);
    }

    // Saturación final
    if (izq > PWM_MAX) izq = PWM_MAX;
    if (der > PWM_MAX) der = PWM_MAX;

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, izq);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, der);
}

/* ================= FUNCION JOYSTICK ================= */

void Joystick_Update(void) {
	uint32_t adc_x, adc_y;

	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	adc_x = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	adc_y = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);

	int16_t x = -mapJoystick(adc_x, centro_x);
	int16_t y = -mapJoystick(adc_y, centro_y);

	if (x == 0 && y == 0) {
		g_freno = 1;  // Parado
	} else {
		g_freno = 0;  // Andando
	}

	int16_t abs_x = abs(x);

	//Limitador de giro
	if (abs_x > 300) {
		y = (y * 300) / abs_x;
	}

	// Asignación de ejes
	g_velocidad = x;
	g_angulo = y;

	int16_t izq = g_velocidad + g_angulo;
	int16_t der = g_velocidad - g_angulo;

	if (g_velocidad > 0) {
		if (izq < 0) izq = 0;
		if (der < 0) der = 0;
	}
	else if (g_velocidad < 0) {
		if (izq > 0) izq = 0;
		if (der > 0) der = 0;
	}

	// Limitador lógico final (Saturación de seguridad)
	if (izq > PWM_MAX) izq = PWM_MAX;
	if (izq < -PWM_MAX) izq = -PWM_MAX;
	if (der > PWM_MAX) der = PWM_MAX;
	if (der < -PWM_MAX) der = -PWM_MAX;

	Motor_Set(izq, der);
}
/* ================= FUNCIONES AUXILIARES ================= */

static int16_t mapJoystick(uint32_t val, uint16_t centro) {
	int32_t dif = (int32_t) val - (int32_t) centro;

	if (abs(dif) < ZONA_MUERTA)
		return 0;

	dif = (dif * PWM_MAX) / 1000;  // Escalado

	if (dif > 0 && dif < PWM_MIN)
		dif = PWM_MIN;
	if (dif < 0 && dif > -PWM_MIN)
		dif = -PWM_MIN;

	if (dif > PWM_MAX)
		dif = PWM_MAX;
	if (dif < -PWM_MAX)
		dif = -PWM_MAX;

	return (int16_t) dif;
}

