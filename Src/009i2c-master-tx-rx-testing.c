/*
 * 009i2c-master-tx-rx-testing.c
 *
 *  Created on: Dec 27, 2022
 *      Author: rafaela
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h> // to use strlen() function
#include "stm32f401xx.h"

extern void initialise_monitor_handles(); // to work semihosting
// also in debug config. -> 'Startup' add in 'Run Commands': monitor arm semihosting enable

#define MY_ADDR_I2C1 		0x61
#define SLAVE_ADDR_I2C2		0x78
#define MY_ADDR_I2C2 		SLAVE_ADDR_I2C2


// I2C1_SCL - PB8 (AF04)
// I2C1_SDA - PB9 (AF04)

// I2C2_SCL - PB10 (AF04)
// I2C2_SDA - PB3 (AF04)


void I2C1_GPIOInits(void);
void GPIO_ButtonInit(void);
void I2C1_Inits(void);

void delay(void){
	for (uint32_t i=0; i<500000; i++);
}

I2C_Handle_t I2C1Handle, I2C2Handle;
uint8_t some_data[] = "We are testing I2C master Tx\n"; //must be less than 32 bytes
uint8_t Tx_buf[32] = "Slave mode testing";
uint8_t commandCode = 0;

int main (void){
	//uint8_t commandcode;
	//uint8_t len;

	initialise_monitor_handles(); // semihosting
	printf("Application is running\n");

	GPIO_ButtonInit(); ////Initialize GPIO pin for B1

	I2C_GPIOInits(); //Initialize GPIO pins to behave as I2C1 and I2C2 pins

	I2C1_Inits(); //Initialize the I2C1 peripheral parameters
	I2C2_Inits();

	//I2C IRQ configurations
	I2C_IRQInterruptConfig(IRQ_NO_I2C1_EV, ENABLE);
	I2C_IRQInterruptConfig(IRQ_NO_I2C1_ER, ENABLE);

	I2C_IRQInterruptConfig(IRQ_NO_I2C2_EV, ENABLE);
	I2C_IRQInterruptConfig(IRQ_NO_I2C2_ER, ENABLE);

	I2C_SlaveEnableDisableCallbackEvents(I2C2, ENABLE);

	I2C_PeripheralControl(I2C1, ENABLE);
	I2C_AckControl(I2C1, I2C_ACK_ENABLE); // moved here since there is a function for that

	I2C_PeripheralControl(I2C2, ENABLE);
	I2C_AckControl(I2C2, I2C_ACK_ENABLE); // moved here since there is a function for that

	while(1){
		while(GPIO_Read(GPIOC, GPIO_PIN_NO_13));
		delay();
		while(I2C_MasterSendDataIT(&I2C1Handle, some_data, strlen((char*)some_data), SLAVE_ADDR_I2C2, 0) != I2C_READY);
		delay();

		//rcv_buf[len+1] = '\0'; //  add null character
		//printf("Data: %s", rcv_buf);
	}

}
/*
 * 	IRQ Handlers
 */
void I2C1_EV_IRQHandler(void){
	I2C_EV_IRQHandling(&I2C1Handle);
}
void I2C1_ER_IRQHandler (void){
	I2C_ER_IRQHandling(&I2C1Handle);
}
void I2C2_EV_IRQHandler(void){
	I2C_EV_IRQHandling(&I2C2Handle);
}
void I2C2_ER_IRQHandler (void){
	I2C_ER_IRQHandling(&I2C2Handle);
}
/*void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle, uint8_t AppEvent){
	if(AppEvent == I2C_EV_TX_CMPLT){
		printf("Tx is completed\n");
	} else if (AppEvent == I2C_EV_RX_CMPLT){
		printf("Rx is completed\n");
	} else if (AppEvent == I2C_ERROR_AF){
		printf("Error: Ack failure\n");
		I2C_CloseSendData(pI2CHandle);
		// and generate the stop condition to release the bus
		I2C_GenerateStopCondition(I2C1);
		// Take actions [code dependent]
	}
}*/

void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle, uint8_t AppEvent){
	static uint8_t commandCode = 0;
	static  uint8_t Cnt = 0;

	if(AppEvent == I2C_EV_DATA_REQ){
		//Master wants some data. slave has to send it
		if(commandCode == 0x51){
			//send the length information to the master
			I2C_SlaveSendData(pI2CHandle->pI2Cx,strlen((char*)Tx_buf));
		}else if (commandCode == 0x52){
			//Send the contents of Tx_buf
			I2C_SlaveSendData(pI2CHandle->pI2Cx,Tx_buf[Cnt++]);

		}
	}else if (AppEvent == I2C_EV_DATA_RCV){
		//Data is waiting for the slave to read . slave has to read it
		commandCode = I2C_SlaveReceiveData(pI2CHandle->pI2Cx);

	}else if (AppEvent == I2C_ERROR_AF){
		//This happens only during slave txing .
		//Master has sent the NACK. so slave should understand that master doesnt need
		//more data.
		commandCode = 0xff;
		Cnt = 0;
	}
	else if (AppEvent == I2C_EV_STOP){
		//This happens only during slave reception .
		//Master has ended the I2C communication with the slave.
	}
}

void I2C_GPIOInits(void){
	GPIO_Handle_t I2C1Pins, I2C2Pins;

		//I2C1 config GPIO pins
	I2C1Pins.pGPIOx = GPIOB;
	I2C1Pins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	I2C1Pins.GPIO_PinConfig.GPIO_PinOpType = GPIO_OP_TYPE_OD;
	I2C1Pins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
	I2C1Pins.GPIO_PinConfig.GPIO_PinAltFunMode = 4;
	I2C1Pins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	//SCL
	I2C1Pins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_8;
	GPIO_Init(&I2C1Pins);
	//SDA
	I2C1Pins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_9;
	GPIO_Init(&I2C1Pins);

		//I2C2 config GPIO pins
	I2C2Pins.pGPIOx = GPIOB;
	I2C2Pins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	I2C2Pins.GPIO_PinConfig.GPIO_PinOpType = GPIO_OP_TYPE_OD;
	I2C2Pins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	I2C2Pins.GPIO_PinConfig.GPIO_PinAltFunMode = 4;
	I2C2Pins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	//SCL
	I2C2Pins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_10;
	GPIO_Init(&I2C2Pins);
	//SDA
	I2C2Pins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_3;
	GPIO_Init(&I2C2Pins);
}

void I2C1_Inits(void){
	I2C1Handle.pI2Cx = I2C1;
	//I2C1Handle.I2C_Config.I2C_ACKControl = I2C_ACK_ENABLE;
	I2C1Handle.I2C_Config.I2C_ACKControl = ENABLE;
	I2C1Handle.I2C_Config.I2C_DeviceAddress = MY_ADDR_I2C1; //Master address (not slave)
	I2C1Handle.I2C_Config.I2C_FMDutyCycle = I2C_FM_DUTY_2; // Does't matter since its SM
	I2C1Handle.I2C_Config.I2C_SCLSpeed = I2C_SCL_SPEED_SM;

	I2C_Init(&I2C1Handle);
}

void I2C2_Inits(void){
	I2C2Handle.pI2Cx = I2C2;
	//I2C1Handle.I2C_Config.I2C_ACKControl = I2C_ACK_ENABLE;
	I2C2Handle.I2C_Config.I2C_ACKControl = ENABLE;
	I2C2Handle.I2C_Config.I2C_DeviceAddress = MY_ADDR_I2C2; //Master address (not slave)
	I2C2Handle.I2C_Config.I2C_FMDutyCycle = I2C_FM_DUTY_2; // Does't matter since its SM
	I2C2Handle.I2C_Config.I2C_SCLSpeed = I2C_SCL_SPEED_SM;

	I2C_Init(&I2C2Handle);
}

void GPIO_ButtonInit(void){
	GPIO_Handle_t B1;

	//B1 configuration
	B1.pGPIOx = GPIOC;
	B1.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
	B1.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	B1.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	B1.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD; //It has an external PU

	GPIO_Init(&B1);
}
