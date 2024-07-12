/**
 * @file
 * @author Abdullah Mert Genç
 * */

#include "ms5611.h"

static HAL_StatusTypeDef MS5611_Reset(MS5611_HandleTypeDef *ms5611) {
    uint8_t cmd = MS5611_RESET;
    return HAL_I2C_Master_Transmit(ms5611->hi2c, MS5611_ADDR, &cmd, 1, HAL_MAX_DELAY);
}

static HAL_StatusTypeDef MS5611_ReadPROM(MS5611_HandleTypeDef *ms5611) {
    for (uint8_t i = 0; i < 6; i++) {
        uint8_t addr = MS5611_PROM_READ + (i * 2);
        uint8_t data[2];
        HAL_StatusTypeDef res = HAL_I2C_Mem_Read(ms5611->hi2c, MS5611_ADDR, addr, 1, data, 2, HAL_MAX_DELAY);

        if (res != HAL_OK) 
			return res;

        ms5611->C[i] = convert8bitto16bit(data[0], data[1]);
    }
    return HAL_OK;
}

static HAL_StatusTypeDef MS5611_ReadADC(MS5611_HandleTypeDef *ms5611, uint32_t *adc_value) {
    uint8_t data[3];

    HAL_StatusTypeDef res = HAL_I2C_Mem_Read(ms5611->hi2c, MS5611_ADDR, MS5611_ADC_READ, 1, data, 3, HAL_MAX_DELAY);
    if (res != HAL_OK) 
		return res;

    *adc_value = (data[0] << 16) | (data[1] << 8) | data[2];
    return HAL_OK;
}

HAL_StatusTypeDef MS5611_Init(MS5611_HandleTypeDef *ms5611) {
    if (MS5611_Reset(ms5611) != HAL_OK) 
		return HAL_ERROR;
    HAL_Delay(10);  // Wait for reset to complete
    return MS5611_ReadPROM(ms5611);
}

HAL_StatusTypeDef MS5611_ReadTemperature(MS5611_HandleTypeDef *ms5611, int32_t *temperature) {
    uint8_t cmd = MS5611_CONVERT_D2;
    if (HAL_I2C_Master_Transmit(ms5611->hi2c, MS5611_ADDR, &cmd, 1, HAL_MAX_DELAY) != HAL_OK) 
		return HAL_ERROR;
    HAL_Delay(10);  // Wait for conversion to complete

    uint32_t D2;
    if (MS5611_ReadADC(ms5611, &D2) != HAL_OK) 
		return HAL_ERROR;

    int32_t dT = D2 - ((int32_t)ms5611->C[4] << 8);
    *temperature = 2000 + ((dT * (int64_t)ms5611->C[5]) >> 23);
    return HAL_OK;
}

HAL_StatusTypeDef MS5611_ReadPressure(MS5611_HandleTypeDef *ms5611, int32_t *pressure) {
    uint8_t cmd = MS5611_CONVERT_D2;
    if (HAL_I2C_Master_Transmit(ms5611->hi2c, MS5611_ADDR, &cmd, 1, HAL_MAX_DELAY) != HAL_OK) 
		return HAL_ERROR;
    HAL_Delay(10);  // Wait for conversion to complete

    uint32_t D2;
    if (MS5611_ReadADC(ms5611, &D2) != HAL_OK) 
		return HAL_ERROR;

    int32_t dT = D2 - ((int32_t)ms5611->C[4] << 8);
	
	cmd = MS5611_CONVERT_D1;

    if (HAL_I2C_Master_Transmit(ms5611->hi2c, MS5611_ADDR, &cmd, 1, HAL_MAX_DELAY) != HAL_OK)
		return HAL_ERROR;
    HAL_Delay(10);  // Wait for conversion to complete

    uint32_t D1;
    if (MS5611_ReadADC(ms5611, &D1) != HAL_OK)
		return HAL_ERROR;

    int64_t OFF = ((int64_t)ms5611->C[1] << 16) + (((int64_t)ms5611->C[3] * dT) >> 7);
    int64_t SENS = ((int64_t)ms5611->C[0] << 15) + (((int64_t)ms5611->C[2] * dT) >> 8);

    *pressure = (((D1 * SENS) >> 21) - OFF) >> 15;
    return HAL_OK;
}

HAL_StatusTypeDef calculateAltitude(double *altitude, int32_t pressure) {
	if(pressure <= 0)
		return HAL_ERROR;

    *altitude = (T0 / L) * (1 - pow(((double)pressure / 100.0) / P0, (L * R) / (g * M)));
    return HAL_OK;
}
