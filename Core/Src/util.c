/*
 * util.c
 *
 *  Created on: Nov 6, 2022
 *      Author: worker
 */

#include <stdlib.h>
#include "util.h"
#include "main.h"
#include "stm32f4xx_hal.h"

static void Digit_Update(uint16_t data);

static const uint8_t digits[] = {
		0x7E, 0x50, 0x6D, 0x79, 0x53, 0x3B, 0x3F, 0x70, 0x7F, 0x7B
};

static uint16_t digit_state = 0x00;

uint32_t timer_key2;
uint32_t timer_key3;

unsigned int timer_enc = 0;

void Digit_Init(void) {
	Digit_Update(digit_state);
}

void Digit_Number(uint16_t number) {
	digit_state = ((uint16_t)digits[(number/10)%10]<<8) | digits[number % 10];
	Digit_Update(digit_state);
}

void Digit_NumberPos(uint8_t number, uint8_t pos) {
	switch(pos) {
		case 0:
			digit_state &= 0xFF00;
			digit_state |= digits[number%10];
			break;
		case 1:
			digit_state &= 0x00FF;
			digit_state |= (((uint16_t)digits[number%10])<<8);
			break;
	}

	Digit_Update(digit_state);
}

void Digit_SymolPos(uint8_t value, uint8_t pos) {
	switch(pos) {
		case 0:
			digit_state &= 0xFF00;
			digit_state |= value;
			break;
		case 1:
			digit_state &= 0x00FF;
			digit_state |= ((uint16_t)value)<<8;
			break;
	}
	Digit_Update(digit_state);
}

static void Digit_Update(uint16_t data) {
	uint32_t  j = 16;

	// negative reset pulse
	HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET);

	while (j--) { // while(data) -- BAD!!!!

		  // set data bit
		  if (data & 0x1) {
			  HAL_GPIO_WritePin(SDI_GPIO_Port, SDI_Pin, GPIO_PIN_SET);
		  } else {
			  HAL_GPIO_WritePin(SDI_GPIO_Port, SDI_Pin, GPIO_PIN_RESET);
		  }

		  // clock pulse
		  HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_SET);

		  HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_RESET);

		  data >>= 1;
	}

	// positive update (write) pulse
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
}

void Buzz_On(void) {
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
}

void Buzz_Off(void) {
	HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
}

void Buzz_SetFreq(uint32_t freq) {
	TIM4->ARR = freq;
	TIM4->CCR3 = freq;
}


