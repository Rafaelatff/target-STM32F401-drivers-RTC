/*
 * 001led_toogle.c
 *
 *  Created on: Dec 16, 2022
 *      Author: r.freitas
 */
#include "stm32f401xx.h"

void delay(void){
	for (uint32_t i=0; i<500000; i++);
}

int main (void){
	GPIO_Handle_t LD2; // LD2 connected to GPIO A (port A) and pin 5

	LD2.pGPIOx = GPIOA;
	LD2.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_5;
	LD2.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	LD2.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	LD2.GPIO_PinConfig.GPIO_PinOpType = GPIO_OP_TYPE_PP;
	LD2.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	//No need of config. PUPD since it's already PP (push pull)

	GPIO_PeriClkCtrl(GPIOA, ENABLE);
	GPIO_Init(&LD2);

	while (1){
		GPIO_Toggle(GPIOA,GPIO_PIN_NO_5);
		delay();
	}
}

