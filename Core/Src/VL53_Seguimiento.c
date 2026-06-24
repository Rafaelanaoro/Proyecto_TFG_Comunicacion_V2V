/* ================= INCLUDES ================= */
#include "VL53_Seguimiento.h"
#include "joystick_motor.h"
#include "main.h"

#include "vl53l0x_api.h"
#include "vl53l0x_platform.h"
#include "Comunicacion_Wifi.h"
#include "Infrarojo_id.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* ================= VARIABLES ================= */
static int16_t error_anterior = 0;
static int32_t correccion = 0;

static uint8_t id_candidato = 0;
static uint8_t aciertos_consecutivos = 0;

#define PENDIENTE_M 1.5292f
#define TERMINO_INDEPENDIENTE_N 674.51f

static uint32_t tiempo_frenando = 0;
static uint8_t frenado_en_espera = 0;

extern I2C_HandleTypeDef hi2c1;

static VL53L0X_Dev_t vl53;
static VL53L0X_Dev_t *pDev = &vl53;

static int16_t pwm_actual = 0;

static uint8_t curva_pendiente = 0;
static uint32_t tiempo_inicio_espera = 0;
static uint32_t tiempo_espera_calculado = 0;
static uint32_t tiempo_inicio_giro = 0;
static uint32_t ticks_impresion = 0;
static uint8_t id_lider_objetivo = 0;

/* ================= DESATASCADOR I2C ================= */
static void Recover_I2C(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_SET);
	HAL_Delay(1);

	for (int i = 0; i < 9; i++) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
		HAL_Delay(1);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
		HAL_Delay(1);
	}

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
	HAL_Delay(1);

	HAL_I2C_DeInit(&hi2c1);
}

/* ================= FUNCION INIT ================= */
void Seguimiento_Init(void) {
	error_anterior = 0;
	correccion = 0;
	frenado_en_espera = 0;
	tiempo_frenando = 0;
	id_lider_objetivo = 0;
	pwm_actual = 0;

	HAL_GPIO_WritePin(VL53L0X_XSHUT_GPIO_Port, VL53L0X_XSHUT_Pin,
			GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(VL53L0X_XSHUT_GPIO_Port, VL53L0X_XSHUT_Pin, GPIO_PIN_SET);
	HAL_Delay(50);

	Recover_I2C();
	HAL_I2C_Init(&hi2c1);

	vl53.I2cHandle = &hi2c1;
	vl53.I2cDevAddr = 0x52;

	if (VL53L0X_DataInit(pDev) != VL53L0X_ERROR_NONE) {
		printf("Error DataInit VL53L0X\r\n");
		return;
	}

	if (VL53L0X_StaticInit(pDev) != VL53L0X_ERROR_NONE) {
		printf("Error StaticInit VL53L0X\r\n");
		return;
	}

	uint8_t VhvSettings = 0;
	uint8_t PhaseCal = 0;
	VL53L0X_PerformRefCalibration(pDev, &VhvSettings, &PhaseCal);

	uint32_t refSpadCount = 0;
	uint8_t isApertureSpads = 0;
	VL53L0X_PerformRefSpadManagement(pDev, &refSpadCount, &isApertureSpads);

	VL53L0X_SetDeviceMode(pDev, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING);
	VL53L0X_SetLimitCheckEnable(pDev,
			VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
	VL53L0X_SetLimitCheckValue(pDev,
			VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE,
			(FixPoint1616_t) (0.2 * 65536));
	VL53L0X_SetMeasurementTimingBudgetMicroSeconds(pDev, 50000);

	if (VL53L0X_StartMeasurement(pDev) != VL53L0X_ERROR_NONE) {
		printf("Error StartMeasurement\r\n");
		return;
	}

	VL53L0X_ClearInterruptMask(pDev,
			VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_NEW_SAMPLE_READY);
	printf("VL53L0X Listo para funcionar.\r\n");
	HAL_Delay(60);

}

/* ================= LECTURA CONTINUA DEL SENSOR ================= */
uint16_t VL53_ReadDistance(void) {
	VL53L0X_RangingMeasurementData_t medicion;
	uint8_t ready = 0;

	static uint16_t distancia_filtrada = 8190;
	static uint8_t errores_consecutivos = 0;
	static uint16_t sensor_dormido = 0;

	// Watchdog
	if (VL53L0X_GetMeasurementDataReady(pDev, &ready) != VL53L0X_ERROR_NONE
			|| !ready) {
		sensor_dormido++;
		if (sensor_dormido > 40) {
					Motor_Stop();
					uint8_t id_backup = id_lider_objetivo;
					Seguimiento_Init();
					id_lider_objetivo = id_backup;

					sensor_dormido = 0;
					distancia_filtrada = 8190;
					return 8190;
				}
		return distancia_filtrada;
	}

	sensor_dormido = 0;

	if (VL53L0X_GetRangingMeasurementData(pDev, &medicion) != VL53L0X_ERROR_NONE)
		return distancia_filtrada;

	VL53L0X_ClearInterruptMask(pDev,
			VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_NEW_SAMPLE_READY);

	// Filtro del limite
	if (medicion.RangeStatus != 0) {
		errores_consecutivos++;
		if (errores_consecutivos > 5) {
			distancia_filtrada = 8190;
			return 8190;
		}
		return distancia_filtrada;
	}

	errores_consecutivos = 0;
	uint16_t distancia_nueva = medicion.RangeMilliMeter;

	// Filtro de suavizado para evitar picos de ditancias
	if (distancia_filtrada != 0 && distancia_filtrada != 8190) {
		distancia_nueva = (uint16_t) ((distancia_filtrada * 0.7)
				+ (distancia_nueva * 0.3));
	}

	distancia_filtrada = distancia_nueva;
	return distancia_nueva;
}

/* ================= FUNCION UPDATE PRINCIPAL ================= */
SeguimientoState_t Seguimiento_Update(void) {
	uint32_t tiempo_actual = HAL_GetTick();

	uint16_t distancia_actual = VL53_ReadDistance(); //Lee la distancia

	// Emparejamiento por infrarrojos
	if (id_lider_objetivo == 0) {
		if (HAL_GPIO_ReadPin(IR_RX_PORT, IR_RX_PIN) == GPIO_PIN_RESET) {
			uint8_t id_detectado = 0;
			if (Infrarrojo_Recibir_ID(&id_detectado)) {
				if (id_detectado > 0 && id_detectado != 255) {
					if (id_detectado == id_candidato) {
						aciertos_consecutivos++;
						if (aciertos_consecutivos >= 3) {
							id_lider_objetivo = id_detectado;
							printf("\r\nCANDADO CERRADO: LIDER ID %d \r\n",
									id_lider_objetivo);
						}
					} else {
						id_candidato = id_detectado;
						aciertos_consecutivos = 1;
					}
				}
			}
		}
	}

	// Escucha las ordenes del WIFI
	Comunicacion_t paquete_wifi;
	if (Wifi_Recibir_Datos(&paquete_wifi)) {
		if (paquete_wifi.id_lider == id_lider_objetivo
				&& id_lider_objetivo != 0) {
			if (curva_pendiente == 0 && abs(paquete_wifi.angulo_giro) > 200) {
				float vel_estimada_mms = (PENDIENTE_M * (float) pwm_actual)
						- TERMINO_INDEPENDIENTE_N;

				if (vel_estimada_mms < 294.0f)
					vel_estimada_mms = 294.0f;

				uint16_t dist_segura = distancia_actual;
				if (dist_segura > 1500 || dist_segura < 50)
					dist_segura = 300;

				tiempo_espera_calculado = (uint32_t) (((float) dist_segura
						/ vel_estimada_mms) * 1000.0f);

				tiempo_inicio_espera = tiempo_actual;
				curva_pendiente = (paquete_wifi.angulo_giro > 0) ? 1 : 2;

				printf(
						"\r\nALERTA CURVA: Gira hacia: %s | Dist: %d mm | Espera: %lu ms\r\n",
						(curva_pendiente == 1) ? "DERECHA" : "IZQUIERDA",
						dist_segura, tiempo_espera_calculado);
			}
		}
	}

	// Sistema de prediccion de curva
	if (curva_pendiente > 0) {

		if ((tiempo_actual - tiempo_inicio_espera) < tiempo_espera_calculado) {
			// Avanza hasta el punto de giro
			int16_t pwm_avance =
					(pwm_actual < PWM_MIN_ARRANQUE) ?
							PWM_MIN_ARRANQUE : pwm_actual;
			Motor_Set(pwm_avance, -pwm_avance);
			tiempo_inicio_giro = tiempo_actual;
		} else {
			uint32_t tiempo_pulso = tiempo_actual % 100;

			//Rotaciona trompicones para permitir al receptor leer la id
			if (tiempo_pulso < 65) {
				if (curva_pendiente == 1)
					Motor_Set(-1000, -1000); // Rotación Derecha
				else
					Motor_Set(1000, 1000);   // Rotación Izquierda
			} else {
				Motor_Stop();
			}

			// REENGANCHE
			if (HAL_GPIO_ReadPin(IR_RX_PORT, IR_RX_PIN) == GPIO_PIN_RESET) {
				uint8_t id_detectado = 0;
				if (Infrarrojo_Recibir_ID(&id_detectado)
						&& id_detectado == id_lider_objetivo) {
					printf(
							"Líder reenganchado. Volviendo a recta.\r\n");
					curva_pendiente = 0;

					Motor_Freno_Seco();
					HAL_Delay(40);
					Motor_Stop();

					error_anterior = distancia_actual - Distancia_seguimeinto;
					correccion = 0;
				}
			}

			// TIMEOUT
			if ((tiempo_actual - tiempo_inicio_giro) > 20000) {
				printf(
						"Han pasado 20 segundos. Abortando barrido.\r\n");
				curva_pendiente = 0;
				Motor_Stop();
				pwm_actual = 0;
				error_anterior = 0;
				correccion = 0;
				return Seguimiento_BUSCANDO;
			}
		}
		return Seguimiento_BUSCANDO;
	}

	// Control PID
	if (distancia_actual > 0 && distancia_actual < 2000) {

		// Freno de Emergencia
		if (distancia_actual < Distancia_frenado_brusco) {
			if (!frenado_en_espera) {
				frenado_en_espera = 1;
				tiempo_frenando = tiempo_actual;
			} else if ((tiempo_actual - tiempo_frenando)
					> Filtro_temporal_frenado) {
				Motor_Freno_Seco();
				HAL_Delay(30);
				Motor_Stop();
				pwm_actual = 0;
				return Seguimiento_PARADA_EMERGENCIA;
			}
		} else {
			frenado_en_espera = 0;
		}

		int16_t error = distancia_actual - Distancia_seguimeinto;
		int16_t derivada_error = error - error_anterior;

		if (error <= 0)
			correccion = 0;
		else {
			correccion += error;
			if (correccion > Limite_correccion)
				correccion = Limite_correccion;
		}

		int16_t velocidad_objetivo = 0;
		if (error > 0) {
			float pid = KP * error + KI * correccion + KD * derivada_error;
			velocidad_objetivo = PWM_MIN_ARRANQUE + (int16_t) pid;
		}

		if (velocidad_objetivo > MAX_PWM)
			velocidad_objetivo = MAX_PWM;

		if (velocidad_objetivo > pwm_actual + PWM_RAMP)
			pwm_actual += PWM_RAMP;
		else if (velocidad_objetivo < pwm_actual - PWM_RAMP)
			pwm_actual -= PWM_RAMP;
		else
			pwm_actual = velocidad_objetivo;

		if (tiempo_actual - ticks_impresion >= 200) {
			ticks_impresion = tiempo_actual;
			//printf("PID -> Medicion: %d mm | Error: %d | PWM_Out: %d\r\n", distancia_actual, error, pwm_actual);
		}

		Motor_Set(pwm_actual, -pwm_actual);
		error_anterior = error;

		return Seguimiento_OK;

	} else {
		if (tiempo_actual - ticks_impresion >= 500) {
			ticks_impresion = tiempo_actual;
			printf(
					"Láser leyendo fuera del rango: %d mm.\r\n",
					distancia_actual);
		}
		Motor_Stop();
		pwm_actual = 0;
		frenado_en_espera = 0;
		return Seguimiento_BUSCANDO;
	}
}

void Seguimiento_Reset_Emparejamiento(void) {
	id_lider_objetivo = 0;
	id_candidato = 0;
	aciertos_consecutivos = 0;
	printf(
			"\r\nEmparejamiento borrado. Esperando nuevo lider...\r\n");
}
