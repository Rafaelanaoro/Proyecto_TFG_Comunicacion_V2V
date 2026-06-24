/*
 * Comunicacion_Wifi.c
 *
 *  Created on: Feb 5, 2026
 *      Author: raana
 */

#include "Comunicacion_Wifi.h"
#include "wifi.h"
#include <string.h>
#include <stdio.h>

static int32_t socket_id = -1;
static uint32_t packet_counter = 0;

/* ================= FUNCION INIT (MODO MULTICAST) ================= */
int8_t Wifi_Init_System(void) {
    printf("\r\n--- Iniciando WIFI con Multicast ---\r\n");

    if (WIFI_Init() != WIFI_STATUS_OK) {
        printf(" ERROR: El chip no responde.\r\n");
        return -1;
    }

    if (WIFI_Connect(WIFI_SSID, WIFI_PASSWORD, WIFI_ECN_WPA2_PSK) != WIFI_STATUS_OK) {
        printf(" ERROR: El router rechazo la conexion.\r\n");
        return -1;
    }

    uint8_t mi_ip[4];
    HAL_Delay(1000);
    if (WIFI_GetIP_Address(mi_ip, 4) != WIFI_STATUS_OK) {
        printf(" ERROR al pedir IP.\r\n");
        return -1;
    }
    printf("IP del coche: %d.%d.%d.%d\r\n", mi_ip[0], mi_ip[1], mi_ip[2], mi_ip[3]);

    WIFI_CloseClientConnection(0);
    HAL_Delay(500);

    //IP Multicast
    ///uint8_t ip_multicast[4] = {10, 56, 251, 198}; Prueba ordenador
    uint8_t ip_multicast[4] = {224, 0, 0, 1};

    printf("Abriendo canal Multicast 224.0.0.1 en Puerto %d... ", UDP_PORT_DESTINO);

    // Abrimos el socket
    if (WIFI_OpenClientConnection(0, WIFI_UDP_PROTOCOL, "UDP_Coche", ip_multicast,
                                  UDP_PORT_DESTINO, UDP_PORT_DESTINO) == WIFI_STATUS_OK) {
        socket_id = 0;
        printf("Listos para enviar/recibir.\r\n\r\n");
        return 0;
    } else {
        printf("Error abriendo socket.\r\n");
        return -1;
    }
}

/* ================= FUNCION ENVIAR DATOS ================= */
void Wifi_Enviar_Datos(uint8_t mi_id, int16_t velocidad, int16_t angulo_giro, uint8_t freno) {
    if (socket_id < 0) {
        //printf("TX Omitido: Socket no abierto.\r\n");
        return;
    }

    Comunicacion_t paquete;
    paquete.cabecera = 'C';
    paquete.id_lider = mi_id;
    paquete.velocidad = velocidad;
    paquete.angulo_giro = angulo_giro;
    paquete.freno = freno;
    paquete.contador = packet_counter++;

    uint16_t bytes_enviados = 0;
    if (WIFI_SendData(socket_id, (uint8_t*)&paquete, sizeof(Comunicacion_t), &bytes_enviados, WIFI_WRITE_TIMEOUT) == WIFI_STATUS_OK) {
        //printf("-> TX UDP OK: %d bytes enviados al aire.\r\n", bytes_enviados);
    } else {
        printf(" ERROR: El chip Wi-Fi rechazo el envio.\r\n");
    }
}
/* ================= FUNCION RECIBIR DATOS ================= */
uint8_t Wifi_Recibir_Datos(Comunicacion_t *paquete_recibido) {
    if (socket_id < 0) return 0;

    uint8_t buffer[sizeof(Comunicacion_t)];
    uint16_t bytes_recibidos = 0;

    // Escucha lo que llegue al puerto 8888
    if (WIFI_ReceiveData(socket_id, buffer, sizeof(buffer), &bytes_recibidos, 10) == WIFI_STATUS_OK) {
        if (bytes_recibidos == sizeof(Comunicacion_t) && buffer[0] == 'C') {
            memcpy(paquete_recibido, buffer, sizeof(Comunicacion_t));
            return 1; // Paquete recibido
        }
    }
    return 0; // Nada recibido
}
