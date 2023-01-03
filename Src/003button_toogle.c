/*
 * 003button_toogle.c
 *
 *  Created on: Dec 16, 2022
 *      Author: r.freitas
 */

//LD2 = PA5
//B1 = PC13 with pull-up resistor
#include "stm32f401xx.h"

#define LOW 0
#define BTN_PRESSED LOW

void delay(void){
	for (uint32_t i=0; i<400000; i++);
}

int main (void){
	GPIO_Handle_t LD2, B1; // LD2 connected to GPIO A (port A) and pin 5

	//LD2 configuration
	LD2.pGPIOx = GPIOA;
	LD2.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_5;
	LD2.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	LD2.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	LD2.GPIO_PinConfig.GPIO_PinOpType = GPIO_OP_TYPE_PP;
	LD2.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	//No need of config. PUPD since it's already PP (push pull)

	//B1 configuration
	B1.pGPIOx = GPIOC;
	B1.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
	B1.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	B1.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	B1.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD; //It has an external PU


	GPIO_PeriClkCtrl(GPIOA, ENABLE);
	GPIO_PeriClkCtrl(GPIOC, ENABLE);
	GPIO_Init(&LD2);
	GPIO_Init(&B1);

	while (1){
		if(GPIO_Read(GPIOC, GPIO_PIN_NO_13)==BTN_PRESSED){
			delay(); //to minimize deboung effect of button
			GPIO_Toggle(GPIOA,GPIO_PIN_NO_5);

		}
	}
}
