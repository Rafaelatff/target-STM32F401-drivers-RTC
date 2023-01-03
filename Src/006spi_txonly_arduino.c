/*
 *
 *  Created on: 22 de dez de 2022
 *      Author: r.freitas
 *
 *      For this exercise we are going to:
 *      B1 says when to send PSI data
 *
 *      SPI Full-duplex mode
 *		(ST - master) === SPI ===  (arduino - slave)
 *			DFF = 0
 *			SSM = 0 (NSS controlled by HW)
 * 			SCLK = 2 Mhz
 *
 *	DOWNLOAD ARDUINO CODE: https://github.com/niekiran/MasteringMCU
 *
 *      PB14 --> MISO
 *      PB15 --> MOSI
 *      PB13 --> SCLK
 *      PB12 --> NSS
 *
 */
#include <string.h> // to use strlen() function
#include "stm32f401xx.h"

void delay(void){
	for (uint32_t i=0; i<500000; i++);
}

void SPI_GPIOInits(void);
void SPI2_Inits(void);
void GPIO_ButtonInit(void);


int main (void){

	char user_data[] = "Aello World"; // cannot pass 256 bytes (uint8_t dataLen)

	SPI_GPIOInits(); //Initialize GPIO pins to behave as SPI2 pins

	GPIO_ButtonInit(); ////Initialize GPIO pin for B1

	SPI2_Inits(); //Initialize the SPI2 peripheral parameters

	// Turn on SSOE to control NSS pin according to SPEnable.
	SPI_SSOEConfig(SPI2,ENABLE);

	while(1){
		while(GPIO_Read(GPIOC, GPIO_PIN_NO_13));
		delay();

		// Enable the SPI2 peripheral
		SPI_PeripheralControl(SPI2,ENABLE);

		// Arduino code '001SPISlaveRxString.ino' expects 1 byte with
		// Size frame information (8 bits or 16 bits), then data
		uint8_t dataLen = strlen(user_data);
		SPI_SendData(SPI2,&dataLen,1);

		SPI_SendData(SPI2,(uint8_t*)user_data,strlen(user_data));

		// Need to wait all bits are sent (SPI Comm not busy) bf disable SPI
		// We use budy flag for that (BSY)
		 while(SPI_GetFlagStatus(SPI2,SPI_BUSY_FLAG)); //pooling, not ideal technique

		// Disable the SPI2 peripheral
		SPI_PeripheralControl(SPI2,DISABLE);
	}
}

void SPI_GPIOInits(void){
	GPIO_Handle_t SPIPins;

	SPIPins.pGPIOx = GPIOB;
	SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode = 5;
	SPIPins.GPIO_PinConfig.GPIO_PinOpType = GPIO_OP_TYPE_PP;
	//SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
	SPIPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

	//PB13 --> SCLK
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
	GPIO_Init(&SPIPins);

	//PB15 --> MOSI
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_15;
	GPIO_Init(&SPIPins);

	//PB14 --> MISO --> not used, can be commented
	//SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14;
	//GPIO_Init(&SPIPins);

	//PB12 --> NSS
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
	GPIO_Init(&SPIPins);

}

void SPI2_Inits(void){
	SPI_Handle_t SPI2Handle;

	SPI2Handle.pSPIx = SPI2;
	SPI2Handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;
	SPI2Handle.SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;
	SPI2Handle.SPIConfig.SPI_SclkSpeed = SPI_SCLK_SPEED_DIV8; // Generates SCLK 2MHz
	SPI2Handle.SPIConfig.SPI_DFF = SPI_DFF_8BITS;
	SPI2Handle.SPIConfig.SPI_CPOL = SPI_CPOL_LOW;
	SPI2Handle.SPIConfig.SPI_CPHA = SPI_CPHA_LOW;
	SPI2Handle.SPIConfig.SPI_SSM = SPI_SSM_DI; // HW slave management en for NSS pin

	// With HW NS (SSM =0), when SPE is enable, NSS pin is automatically pulled to low.
	// PB12 --> NSS
	// But for that, the NSS O/P -: Slave Select Output Enable must be set!

	SPI_Init(&SPI2Handle);
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
