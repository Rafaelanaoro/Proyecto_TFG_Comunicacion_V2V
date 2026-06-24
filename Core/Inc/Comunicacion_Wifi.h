/*
 * Comunicacion_Wifi.h
 *
 *  Created on: Feb 5, 2026
 *      Author: raana
 */

#ifndef INC_COMUNICACION_WIFI_H_
#define INC_COMUNICACION_WIFI_H_

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* ================= DATOS DEL WIFI ================= */
#define WIFI_SSID       "rafa"
#define WIFI_PASSWORD   "rafael23"

/* =================  LA IP DEL ORDENADOR ================= */
#define PC_IP_ADDR_0    255
#define PC_IP_ADDR_1    255
#define PC_IP_ADDR_2    255
#define PC_IP_ADDR_3    255

#define UDP_PORT_DESTINO 8888
#define WIFI_WRITE_TIMEOUT 500

/* ================= VARIABLES ================= */

// Estructura de los datos
typedef struct __attribute__((packed)) {
	uint8_t cabecera;
	uint8_t id_lider;
	int16_t velocidad;
	int16_t angulo_giro;
	uint8_t freno;
	uint32_t contador;
} Comunicacion_t;

/* ================= FUNCIONES LED ================= */

int8_t Wifi_Init_System(void);
void Wifi_Enviar_Datos(uint8_t mi_id, int16_t velocidad, int16_t angulo_giro, uint8_t freno);
uint8_t Wifi_Recibir_Datos(Comunicacion_t *paquete_recibido);

#endif /* INC_COMUNICACION_WIFI_H_ */
