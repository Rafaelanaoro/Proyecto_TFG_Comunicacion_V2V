/*
 * Infrarojo_id.c
 *
 *  Created on: May 29, 2026
 *      Author: raana
 */

#include "Infrarojo_id.h"
#include <stdio.h>
#include "main.h"

extern TIM_HandleTypeDef htim15;
extern uint32_t SystemCoreClock;

static void delay_us_dwt(uint32_t us) {
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000U);
    while ((DWT->CYCCNT - start) < ticks);
}

static void burst_38khz(uint32_t duracion_us) {
    HAL_TIM_PWM_Start(&htim15, TIM_CHANNEL_1);
    delay_us_dwt(duracion_us);
    HAL_TIM_PWM_Stop(&htim15, TIM_CHANNEL_1);
}

void Infrarrojo_Init(void) {
    __HAL_TIM_SET_COMPARE(&htim15, TIM_CHANNEL_1, 1052);

    // Activamos el reloj DWT
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void Infrarrojo_Enviar_ID(uint8_t id) {
    burst_38khz(4000);
    delay_us_dwt(2000);
    for (int i = 0; i < 8; i++) {
        burst_38khz(600);
        if ((id >> i) & 0x01) delay_us_dwt(1600);
        else delay_us_dwt(600);
    }
    burst_38khz(600);
}

uint8_t Infrarrojo_Recibir_ID(uint8_t *id_recibido) {
    uint32_t start;
    uint32_t duracion;

    // Medir el pulso inicial LOW
    start = DWT->CYCCNT;
    while (HAL_GPIO_ReadPin(IR_RX_PORT, IR_RX_PIN) == GPIO_PIN_RESET) {
        duracion = (DWT->CYCCNT - start) / (SystemCoreClock / 1000000U);
        if (duracion > 6000) return 0; // Aborta si es ruido o se queda bloqueado
    }
    if (duracion < 2000) return 0; // Si es muy corto se descarta

    // Medir el silencio inicial HIGH
    start = DWT->CYCCNT;
    while (HAL_GPIO_ReadPin(IR_RX_PORT, IR_RX_PIN) == GPIO_PIN_SET) {
        duracion = (DWT->CYCCNT - start) / (SystemCoreClock / 1000000U);
        if (duracion > 3000) return 0;
    }
    if (duracion < 1000) return 0;

    uint8_t temp_id = 0;

    // Decodificar los 8 bits exactos
    for (int i = 0; i < 8; i++) {

        // Pulso de 600
        start = DWT->CYCCNT;
        while (HAL_GPIO_ReadPin(IR_RX_PORT, IR_RX_PIN) == GPIO_PIN_RESET) {
            duracion = (DWT->CYCCNT - start) / (SystemCoreClock / 1000000U);
            if (duracion > 1200) return 0;
        }

        // Silencios
        start = DWT->CYCCNT;
        while (HAL_GPIO_ReadPin(IR_RX_PORT, IR_RX_PIN) == GPIO_PIN_SET) {
            duracion = (DWT->CYCCNT - start) / (SystemCoreClock / 1000000U);
            if (duracion > 2500) return 0;
        }

        // Si el silencio fue más largo de 1000us es un 1
        if (duracion > 1000) {
            temp_id |= (1 << i);
        }
    }

    *id_recibido = temp_id;
    printf("\r\n ID RECIBIDA: %d \r\n", temp_id);
    return 1;
}
