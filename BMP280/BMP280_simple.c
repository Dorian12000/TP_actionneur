/*
 * BMP280.c
 *
 *  Created on: Oct 3, 2020
 *      Author: cbares
 */

#include "stdio.h"
#include "stdlib.h"

#include "main.h"
#include "BMP280_simple.h"

extern I2C_HandleTypeDef hi2c1;

uint16_t dig_T1;
int16_t dig_T2;
int16_t dig_T3;
uint16_t dig_P1;
int16_t dig_P2;
int16_t dig_P3;
int16_t dig_P4;
int16_t dig_P5;
int16_t dig_P6;
int16_t dig_P7;
int16_t dig_P8;
int16_t dig_P9;

BMP280_S32_t t_fine;

int BMP280_check() {
	uint8_t buf[1];
	HAL_StatusTypeDef ret;
	buf[0] = BMP280_ID_REG;

	ret = HAL_I2C_Master_Transmit(&hi2c1, BMP280_ADDR, buf, 1, HAL_MAX_DELAY);
	if (ret != 0) {
		printf("Problem with check (I2C Transmit)\r\n");
	}

	ret = HAL_I2C_Master_Receive(&hi2c1, BMP280_ADDR, buf, BMP280_ID_LEN,
			HAL_MAX_DELAY);
	if (ret != 0) {
		printf("Problem with check (I2C Receive) \r\n");
	}

	printf("Id: 0x%x...", buf[0]);
	if (buf[0] == BMP280_ID_VAL) {
		printf("Ok\r\n");
		return 0;
	} else {
		printf("not Ok!\r\n");
		return 1;
	}
}

int BMP280_init() {
	HAL_StatusTypeDef ret;
	uint8_t ctrl = (0b010 << 5) | (0b101 << 2) | (0b11);
	/* 				osr_t x2       osr_p x16       normal mode   */

	printf("Configure...\r\n");
	ret = BMP280_Write_Reg(BMP280_CTRL_MEAS_REG, ctrl);
	if (ret == 0) {
		printf("Config Ok\r\n");
	} else {
		printf("Config not Ok!\r\n");
		return 1;
	}
	BMP280_get_trimming();
	return 0;
}

int BMP280_Write_Reg(uint8_t reg, uint8_t value) {
	uint8_t buf[3];
	HAL_StatusTypeDef ret;

	buf[0] = reg;
	buf[1] = value;
	ret = HAL_I2C_Master_Transmit(&hi2c1, BMP280_ADDR, buf, 2, HAL_MAX_DELAY);
	if (ret != 0) {
		printf("Problem with I2C Transmit\r\n");
	}

	ret = HAL_I2C_Master_Receive(&hi2c1, BMP280_ADDR, buf, 1, HAL_MAX_DELAY);
	if (ret != 0) {
		printf("Problem with I2C Receive\r\n");
	}

	if (buf[0] == value) {
		return 0;
	} else {
		return 1;
	}
}

uint8_t* BMP280_Read_Reg(uint8_t reg, uint8_t length) {
	uint8_t *buf;
	HAL_StatusTypeDef ret;

	ret = HAL_I2C_Master_Transmit(&hi2c1, BMP280_ADDR, &reg, 1, HAL_MAX_DELAY);
	if (ret != 0) {
		printf("Problem with I2C Transmit\r\n");
	}

	buf = (uint8_t*) malloc(length);
	ret = HAL_I2C_Master_Receive(&hi2c1, BMP280_ADDR, buf, length,
			HAL_MAX_DELAY);
	if (ret != 0) {
		printf("Problem with I2C Receive\r\n");
	}

	return buf;
}

BMP280_S32_t BMP280_get_temperature() {
	uint8_t *buf;
	BMP280_S32_t adc_T;

	buf = BMP280_Read_Reg(BMP280_TEMP_REG_MSB, BMP280_TEMP_LEN);

	adc_T = ((BMP280_S32_t) (buf[0]) << 12) | ((BMP280_S32_t) (buf[1]) << 4)
			| ((BMP280_S32_t) (buf[2]) >> 4);

	free(buf);

	printf("Temperature: ");
	printf("0X%05lX", adc_T);
	printf("\r\n");

	return adc_T;
}

int BMP280_get_pressure() {
	uint8_t *buf;
	BMP280_S32_t adc_P;

	buf = BMP280_Read_Reg(BMP280_PRES_REG_MSB, BMP280_PRES_LEN);

	adc_P = ((BMP280_S32_t) (buf[0]) << 12) | ((BMP280_S32_t) (buf[1]) << 4)
			| ((BMP280_S32_t) (buf[2]) >> 4);

	free(buf);

	printf("Pressure:    0x");
	printf("%05lX", adc_P);
	printf("\r\n");

	return 0;
}
