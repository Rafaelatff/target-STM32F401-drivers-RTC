/*
 * 003button_toogle.c
 *
 *  Created on: Dec 20, 2022
 *      Author: r.freitas
 */
#include <string.h> //To use memset
#include "stm32f401xx.h"

#define LOW 0
#define BTN_PRESSED LOW

void delay(void){
	for (uint32_t i=0; i<400000; i++);
}

int main (void){
	GPIO_Handle_t LD2 , B1;
	memset(&LD2,0,sizeof(LD2)); //Clean random values from LD2 memory space
	memset(&B1,0,sizeof(B1)); //Clean random values from B1 memory space
	//memset(&address, value, size);

	//LD2 configuration  GPIO A (port A) and pin 5
	LD2.pGPIOx = GPIOA;
	LD2.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_5;
	LD2.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	LD2.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	LD2.GPIO_PinConfig.GPIO_PinOpType = GPIO_OP_TYPE_PP;
	LD2.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	//No need of config. PUPD since it's already PP (push pull)

	//B1 configuration GPIO C (port C) and pin 13
	B1.pGPIOx = GPIOC;
	B1.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
	B1.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IT_FT; // changed to interrupt
	B1.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	B1.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD; //It has an external PU
	// Up to here, just PinMode changes

	GPIO_PeriClkCtrl(GPIOA, ENABLE);
	GPIO_PeriClkCtrl(GPIOC, ENABLE);
	GPIO_Init(&LD2);
	GPIO_Init(&B1);

	// IRQ Configurations
	GPIO_IRQPriorityConfig(IRQ_NO_EXTI15_10, NVIC_IRQ_PRI15); // Any number from 0 to 15
	GPIO_IRQInterruptConfig(IRQ_NO_EXTI15_10, ENABLE);

	while(1);
}

void EXTI15_10_IRQHandler(void){
	delay();
	GPIO_IRQHandling(GPIO_PIN_NO_13); //Clear pending event from EXTI line
	GPIO_Toggle(GPIOA, GPIO_PIN_NO_5);
}
