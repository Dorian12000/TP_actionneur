/*
 * shell.c
 *
 *  Created on: Oct 1, 2023
 *      Author: nicolas
 */
#include "usart.h"
#include "mylibs/shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tim.h"
#include "adc.h"
#include "../current.h"

#define MAX_DUTY_CYCLE 4249
#define MAX_SPEED 100
#define MIN_SPEED 0

uint8_t prompt[]="user@Nucleo-STM32G474RET6>>";
uint8_t started[]=
		"\r\n*-----------------------------*"
		"\r\n| Welcome on Nucleo-STM32G474 |"
		"\r\n*-----------------------------*"
		"\r\n";
uint8_t newline[]="\r\n";
uint8_t backspace[]="\b \b";
uint8_t cmdNotFound[]="Command not found\r\n";
uint8_t brian[]="Brian is in the kitchen\r\n";
uint8_t errorMsg[] = "Error";
uint8_t uartRxReceived;
uint8_t uartRxBuffer[UART_RX_BUFFER_SIZE];
uint8_t uartTxBuffer[UART_TX_BUFFER_SIZE];
uint16_t currentSpeed = 50;

char	 	cmdBuffer[CMD_BUFFER_SIZE];
int 		idx_cmd;
char* 		argv[MAX_ARGS];
int		 	argc = 0;
char*		token;
int 		newCmdReady = 0;

void Shell_Init(void){
	memset(argv, NULL, MAX_ARGS*sizeof(char*));
	memset(cmdBuffer, NULL, CMD_BUFFER_SIZE*sizeof(char));
	memset(uartRxBuffer, NULL, UART_RX_BUFFER_SIZE*sizeof(char));
	memset(uartTxBuffer, NULL, UART_TX_BUFFER_SIZE*sizeof(char));

	HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
	HAL_UART_Transmit(&huart2, started, strlen((char *)started), HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart2, prompt, strlen((char *)prompt), HAL_MAX_DELAY);
}

void Shell_Loop(void){
	if(uartRxReceived){
		switch(uartRxBuffer[0]){
		case ASCII_CR: // Nouvelle ligne, instruction à traiter
			HAL_UART_Transmit(&huart2, newline, sizeof(newline), HAL_MAX_DELAY);
			cmdBuffer[idx_cmd] = '\0';
			argc = 0;
			token = strtok(cmdBuffer, " ");
			while(token!=NULL){
				argv[argc++] = token;
				token = strtok(NULL, " ");
			}
			idx_cmd = 0;
			newCmdReady = 1;
			break;
		case ASCII_BACK: // Suppression du dernier caractère
			cmdBuffer[idx_cmd--] = '\0';
			HAL_UART_Transmit(&huart2, backspace, sizeof(backspace), HAL_MAX_DELAY);
			break;

		default: // Nouveau caractère
			cmdBuffer[idx_cmd++] = uartRxBuffer[0];
			HAL_UART_Transmit(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE, HAL_MAX_DELAY);
		}
		uartRxReceived = 0;
	}

	if(newCmdReady){
		if(strcmp(argv[0],"WhereisBrian?")==0){
			HAL_UART_Transmit(&huart2, brian, sizeof(brian), HAL_MAX_DELAY);
		}
		else if(strcmp(argv[0],"help")==0){
			int uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, "Print all available functions here\r\n");
			HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
		}
		else if(strcmp(argv[0], "speed") == 0)
		{
			uint16_t speedValue = atoi(argv[1]);

			if(speedValue > MAX_SPEED)
			{
				speedValue = MAX_SPEED;
			}
			else if(speedValue < MIN_SPEED)
			{
				speedValue = MIN_SPEED;
			}

			int uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, "Wait\r\n");
			HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);

			while(currentSpeed != speedValue)
			{
				if(currentSpeed > speedValue)
				{
					currentSpeed--;
				}
				else if(currentSpeed < speedValue)
				{
					currentSpeed++;
				}

				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (MAX_DUTY_CYCLE * currentSpeed)/100);
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, MAX_DUTY_CYCLE - (MAX_DUTY_CYCLE * currentSpeed)/100);

				HAL_Delay(100);
			}
		}
		else if(strcmp(argv[0], "start") == 0)
		{
			if(HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) != HAL_OK)
			{
				HAL_UART_Transmit(&huart2, errorMsg, sizeof(errorMsg), HAL_MAX_DELAY);
			}
			if(HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1) != HAL_OK)
			{
				HAL_UART_Transmit(&huart2, errorMsg, sizeof(errorMsg), HAL_MAX_DELAY);
			}
			if(HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2) != HAL_OK)
			{
				HAL_UART_Transmit(&huart2, errorMsg, sizeof(errorMsg), HAL_MAX_DELAY);
			}
			if(HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2) != HAL_OK)
			{
				HAL_UART_Transmit(&huart2, errorMsg, sizeof(errorMsg), HAL_MAX_DELAY);
			}
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, MAX_DUTY_CYCLE / 2);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, MAX_DUTY_CYCLE / 2);
		}
		else if(strcmp(argv[0], "stop") == 0)
		{
			HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
			HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
			HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
			HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
		}
		else if(strcmp(argv[0], "readCurrent") == 0)
		{
			int uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, "%f\r\n", adcValue[0]);
			HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
		}
		else if(strcmp(argv[0], "readSpeed") == 0)
		{
			int uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, "%f, %f\r\n", adcEncoder[0], adcEncoder[1]);
			HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
		}
		else{
			HAL_UART_Transmit(&huart2, cmdNotFound, sizeof(cmdNotFound), HAL_MAX_DELAY);
		}
		HAL_UART_Transmit(&huart2, prompt, sizeof(prompt), HAL_MAX_DELAY);
		newCmdReady = 0;
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart){
	uartRxReceived = 1;
	HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
}
