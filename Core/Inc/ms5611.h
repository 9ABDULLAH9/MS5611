#ifndef MS5611_H
#define MS5611_H

#include "stm32f4xx_hal.h"
#include <math.h>
#include <stdint.h>


#define MS5611_RESET 0x1E
#define MS5611_PROM_READ 0xA2
#define MS5611_CONVERT_D1 0x48
#define MS5611_CONVERT_D2 0x58
#define MS5611_ADC_READ 0x00

#define P0 1013.25 // Deniz seviyesindeki referans basınç (milibar)
#define T0 288.15 // Deniz seviyesindeki referans sıcaklık (Kelvin)
#define L 0.0065 // Troposferdeki sıcaklık gradyanı (K/m)
#define R 8.3144598 // Gaz sabiti (J/(mol·K))
#define g 9.80665 // Yerçekimi ivmesi (m/s²)
#define M 0.0289644 // Havanın molar kütlesi (kg/mol)


#define convert8bitto16bit(x, y)  (((x) << 8) | (y))
#define convert16bittoLSB(x) ((x) & 255)
#define convert16bittoMSB(x) ((x) >> 8)


#define MS5611_ADDR 0x77 << 1  //0xEE


typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint16_t C[6];  // Calibration coefficients
} MS5611_HandleTypeDef;

HAL_StatusTypeDef MS5611_Init(MS5611_HandleTypeDef *ms5611);
HAL_StatusTypeDef MS5611_ReadTemperature(MS5611_HandleTypeDef *ms5611, int32_t *temperature);
HAL_StatusTypeDef MS5611_ReadPressure(MS5611_HandleTypeDef *ms5611, int32_t *pressure);
HAL_StatusTypeDef calculateAltitude(double *altitude, int32_t pressure);

#endif
