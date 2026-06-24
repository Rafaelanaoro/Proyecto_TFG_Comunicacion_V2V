# Diseño de protocolo de comunicación para coches autónomos

Este repositorio contiene el código fuente y la documentación técnica desarrollada para el Trabajo de Fin de Grado (TFG) titulado **"Diseño de protocolo de comunicación para coches autónomos"** en la Universidad de Sevilla.

El proyecto consiste en el diseño, desarrollo e implementación de una arquitectura de hardware y firmware *bare-metal* para controlar un enjambre de vehículos a escala. Los vehículos son capaces de organizarse en modo líder-seguidor y mantener la formación de manera autónoma utilizando percepción espacial y comunicación Vehicle-to-Vehicle (V2V).

## Características Principales

* **Control de Distancia Autónomo:** Implementación de un controlador PID discreto con sistema *anti-windup* y filtro de media móvil exponencial (EMA) para suavizar las lecturas.
* **Percepción Espacial:** Uso de sensores de Tiempo de Vuelo (ToF) VL53L0X mediante bus I2C para medir la distancia relativa entre vehículos con precisión milimétrica.
* **Arquitectura de Doble Banda (V2V):**
  * **Wi-Fi (UDP Multicast):** Red local asíncrona de baja latencia para la transmisión de telemetría y comandos de giro.
  * **Infrarrojo a 38 kHz:** Canal óptico direccional programado mediante técnica *bit-banging* para la identificación física, validación visual y emparejamiento seguro del convoy.
* **Ejecución Determinista:** Firmware desarrollado en C sobre arquitectura *bare-metal*, garantizando que los cálculos del PID y las rutinas de interrupción se ejecuten sin retardos ni latencias impredecibles.

## Requisitos de Hardware

El código está diseñado para ser ejecutado en el siguiente ecosistema de hardware:

* **Microcontrolador:** STM32 B-L475E-IOT01A.
* **Driver de Potencia:** Módulo L298N.
* **Sensores de Distancia:** Módulo láser ToF VL53L0X.
* **Módulos Ópticos:** Diodos emisores y receptores infrarrojos.
* **Entrada de usuario:** Joystick analógico para el control manual del vehículo líder.

## Entorno de Desarrollo y Compilación

El proyecto ha sido desarrollado utilizando las herramientas oficiales de STMicroelectronics:

* **STM32CubeIDE (v1.13.0 o superior):** IDE principal basado en Eclipse.
* **STM32CubeMX:** Para la generación de la capa de abstracción de hardware (HAL) y la configuración del árbol de relojes a 80 MHz.

### Instalación rápida
1. Clona este repositorio en tu equipo local:
```bash
   git clone https://github.com/Rafaelanaoro/Proyecto_TFG_Comunicacion_V2V.git
