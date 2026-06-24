/*
 * Status_led.c
 *
 *  Created on: Jan 23, 2026
 *      Author: raana
 */

/* ================= INCLUDES ================= */

#include "Estado_LED.h"

/* ================= VARIABLES ================= */

volatile MODO_COCHE_t cocheModo = CocheModo_MANUAL;   	// Modo del coche
static uint32_t ultimoCambio = 0;
static GPIO_PinState estadoLED = GPIO_PIN_RESET;		// Modo del LED

/* ================= FUNCIONES LED ================= */

void LED_Init(void) {
	HAL_GPIO_WritePin(LED_MODO_GPIO_Port, LED_MODO_Pin, GPIO_PIN_RESET);
}

void LED_Update(COCHE_ESTADO_t estado) {
	uint32_t temp_now = HAL_GetTick();

	switch (estado) {
	case MODO_MANUAL:
		HAL_GPIO_WritePin(LED_MODO_GPIO_Port, LED_MODO_Pin, GPIO_PIN_RESET);
		break;

	case MODO_AUTO_SEGUIMIENTO:
		if (temp_now - ultimoCambio >= 800) {
			ultimoCambio = temp_now;
			if (estadoLED == GPIO_PIN_SET) {
				estadoLED = GPIO_PIN_RESET;
			} else {
				estadoLED = GPIO_PIN_SET;
			}

			HAL_GPIO_WritePin(LED_MODO_GPIO_Port, LED_MODO_Pin, estadoLED);
		}
		break;

	case MODO_AUTO_FRENANDO:
		if (temp_now - ultimoCambio >= 50) {
			ultimoCambio = temp_now;
			if (estadoLED == GPIO_PIN_SET) {
				estadoLED = GPIO_PIN_RESET;
			} else {
				estadoLED = GPIO_PIN_SET;
			}
			HAL_GPIO_WritePin(LED_MODO_GPIO_Port, LED_MODO_Pin, estadoLED);
		}
		break;

	case MODO_AUTO_STOP:
		HAL_GPIO_WritePin(LED_MODO_GPIO_Port, LED_MODO_Pin, GPIO_PIN_SET);
		break;
	}
}
