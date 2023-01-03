/*
 * stm32f401xx_i2c_driver.c
 *
 *  Created on: Dec 25, 2022
 *      Author: rafaela
 */

#include "stm32f401xx_i2c_driver.h"

#define XTAL_VALUE 8000000

uint16_t AHB_PreScaler[8] = {2,4,8,16,64,128,256,512}; // aux RCCGetPCLK1Value function
uint8_t APB1_PreScaler[4] = {2,4,8,16}; // aux RCCGetPCLK1Value function

static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx);
static void I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr);
static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr);
static void I2C_ClearADDRFlag(I2C_Handle_t *pI2CHandle);
static void I2C_MasterHandleRXNEInterrupt(I2C_Handle_t *pI2CHandle);
static void I2C_MasterHandleTXEInterrupt(I2C_Handle_t *pI2CHandle);

/*
 *	@name 	I2C_PeriClkCtrl
 *	@brief	Clock Control for I2C peripheral
 *
 *	@param	*pI2Cx -> pointer to I2Cx address (structure I2C_RegDef_t type)
 *	@param	EnOrDi can be ENABLE or DISABLE
 *	@return void type
 */
void I2C_PeriClkCtrl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi){
	if (EnOrDi == ENABLE){
		if(pI2Cx == I2C1){
			I2C1_PCLK_EN();
		} else if (pI2Cx == I2C2){
			I2C2_PCLK_EN();
		} else if (pI2Cx == I2C3){
			I2C3_PCLK_EN();
		}
	}else{
		if(pI2Cx == I2C1){
			I2C1_PCLK_DI();
		} else if (pI2Cx == I2C2){
			I2C2_PCLK_DI();
		} else if (pI2Cx == I2C3){
			I2C3_PCLK_DI();
		}
	}
}

/*
 *	@name 	I2C_Init
 *	@brief
 *
 *	@param
 *	@return void type
 */
void I2C_Init(I2C_Handle_t *pI2CHandle){
	uint32_t tempreg = 0 ;

	//enable the clock for the i2cx peripheral
	I2C_PeriClkCtrl(pI2CHandle->pI2Cx,ENABLE);

	//configure the FREQ field of CR2
	tempreg = 0;
	tempreg |= RCC_GetPCLK1Value() /1000000U ;
	pI2CHandle->pI2Cx->CR2 |=  (tempreg & 0x3F);

	//program the device own address
	tempreg = 0;
	tempreg |= pI2CHandle->I2C_Config.I2C_DeviceAddress << 1;
	tempreg |= ( 1 << 14); // must be kept by sw as 1
	pI2CHandle->pI2Cx->OAR1 |= tempreg;

	//CCR calculations
	uint16_t ccr_value = 0;
	tempreg = 0;
	if(pI2CHandle->I2C_Config.I2C_SCLSpeed <= I2C_SCL_SPEED_SM)	{
		//mode is standard mode
		ccr_value = (RCC_GetPCLK1Value() / ( 2 * pI2CHandle->I2C_Config.I2C_SCLSpeed ) );
		tempreg |= (ccr_value & 0xFFF);
	}else{
		//mode is fast mode
		tempreg |= ( 1 << 15);
		tempreg |= (pI2CHandle->I2C_Config.I2C_FMDutyCycle << 14); // must be kept by sw as 1
		if(pI2CHandle->I2C_Config.I2C_FMDutyCycle == I2C_FM_DUTY_2)
		{
			ccr_value = (RCC_GetPCLK1Value() / ( 3 * pI2CHandle->I2C_Config.I2C_SCLSpeed ) );
		}else{
			ccr_value = (RCC_GetPCLK1Value() / ( 25 * pI2CHandle->I2C_Config.I2C_SCLSpeed ) );
		}
	tempreg |= (ccr_value & 0xFFF);
	}
	pI2CHandle->pI2Cx->CCR |= tempreg;

	//TRISE Configuration
	if(pI2CHandle->I2C_Config.I2C_SCLSpeed <= I2C_SCL_SPEED_SM){
		//mode is standard mode
		tempreg = (RCC_GetPCLK1Value() /1000000U) + 1 ;
	}else{
		//mode is fast mode
		tempreg = ( (RCC_GetPCLK1Value() * 300) / 1000000000U ) + 1;
	}
	pI2CHandle->pI2Cx->TRISE = 0; //Reset value of TRISE is not 0 (0x0002)
	pI2CHandle->pI2Cx->TRISE |= (tempreg & 0x3F);

	tempreg |= (1 << I2C_SR2_MSL);
	pI2CHandle->pI2Cx->SR2 = tempreg; // Trying to change MSL bit
}

/*
 *	@name 	I2C_DeInit
 *	@brief
 *
 *	@param
 *	@return void type
 */
void I2C_DeInit(I2C_RegDef_t *pI2Cx){
	// TODO -> Refer to RCC reset registers
}

/*
 *	@name 	I2C_MasterSendData
 *	@brief
 *
 *	@param	*pI2CHandle
 *	@param	*pTxbuffer
 *	@param	Len
 *	@param	SlaveAddr
 *	@param	Sr
 *	@return void type
 */
void I2C_MasterSendData(I2C_Handle_t *pI2CHandle,uint8_t *pTxbuffer, uint32_t Len, uint8_t SlaveAddr,uint8_t Sr){
	// 1. Generate the START condition
	I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

	// 2. Confirm that start generation is completed by checking the SB flag in the SR1
	//   Note: Until SB is cleared SCL will be stretched (pulled to LOW)
	while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx,I2C_FLAG_SB));

	// 3. Send the address of the slave with r/nw bit set to w(0) (total 8 bits )
	I2C_ExecuteAddressPhaseWrite(pI2CHandle->pI2Cx,SlaveAddr);

	// 4. Confirm that address phase is completed by checking the ADDR flag in the SR1
	while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx,I2C_FLAG_ADDR));

	// 5. Clear the ADDR flag according to its software sequence
	//   Note: Until ADDR is cleared SCL will be stretched (pulled to LOW)
	I2C_ClearADDRFlag(pI2CHandle);

	// 6. Send the data until len becomes 0
	while(Len > 0)
	{
		while(! I2C_GetFlagStatus(pI2CHandle->pI2Cx,I2C_FLAG_TXE)); //Wait till TXE is set
		pI2CHandle->pI2Cx->DR = *pTxbuffer;
		pTxbuffer++;
		Len--;
	}

	// 7. when Len becomes zero wait for TXE=1 and BTF=1 before generating the STOP condition
	//   Note: TXE=1 , BTF=1 , means that both SR and DR are empty and next transmission should begin
	//   when BTF=1 SCL will be stretched (pulled to LOW)
	while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx,I2C_FLAG_TXE));
	//while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx,I2C_FLAG_BTF)); // parei

	// 8. Generate STOP condition and master need not to wait for the completion of stop condition.
	//   Note: generating STOP, automatically clears the BTF
	if(Sr == I2C_DISABLE_SR)I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
}

void I2C_MasterReceiveData(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint8_t Len, uint8_t SlaveAddr){
	// 1. Generate the Start Condition
	I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

	// 2. Confirm that start generation is completed by checking the SB flag in the SR1 register
	// Note: Until SB is cleared SCL will be stretched (pulled to low)
	while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx,I2C_FLAG_SB));

	// 3. Send the address of the slave with r/nw bit set to R(1)
	I2C_ExecuteAddressPhaseRead(pI2CHandle->pI2Cx,SlaveAddr);

	// 4. Wait until address phase is completed by checking the ADDR flag in SR1
	while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx,I2C_FLAG_ADDR));

	// 5.a Read only 1 byte
	if(Len == 1){
		// Disable Acking
		I2C_AckControl(pI2CHandle->pI2Cx, DISABLE);

		// Clear the ADDR flag
		I2C_ClearADDRFlag(pI2CHandle);

		// Wait until RXNE becomes 1
		while(! I2C_GetFlagStatus(pI2CHandle->pI2Cx,I2C_FLAG_RXNE)); //Wait till RXNE is set

		// Generate STOP condition
		I2C_GenerateStopCondition(pI2CHandle->pI2Cx);

		// Read data in Rx buffer
		*pRxBuffer = pI2CHandle->pI2Cx->DR;

	}
	// 5.b Read more than 1 byte
	if(Len > 1){
		// Wait until RXNE becomes 1
		while(! I2C_GetFlagStatus(pI2CHandle->pI2Cx,I2C_FLAG_RXNE)); //Wait till RXNE is set

		// read data in Rx buffer until Len becomes 0
		for (uint32_t i = Len; i>0 ; i--){
			// Wait until RXNE becomes 1

			if (i ==2){
				// Disable Acking,
				I2C_AckControl(pI2CHandle->pI2Cx, DISABLE);

				// Generate STOP condition
				I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
			}
			// Read data in Rx buffer
			*pRxBuffer = pI2CHandle->pI2Cx->DR;

			// Increemt the buffer address
			pRxBuffer++;
		}
	}
	// Re-enable ack
	if(pI2CHandle->I2C_Config.I2C_ACKControl == I2C_ACK_ENABLE){
		I2C_AckControl(pI2CHandle->pI2Cx, I2C_ACK_ENABLE);
	}
	return;
}


uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx , uint32_t FlagName){
	if(pI2Cx->SR1 & FlagName)
	{
		return FLAG_SET;
	}
	return FLAG_RESET;
}

void I2C_GenerateStopCondition(I2C_RegDef_t *pI2Cx){
	pI2Cx->CR1 |= ( 1 << I2C_CR1_STOP);
}


void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi){
	if(EnOrDi == ENABLE){
		pI2Cx->CR1 |= (1 << I2C_CR1_PE);
		//I2C_AckControl(pI2Cx, ENABLE); //Enable ack, this can not be set before PE = 1
	}else{
		pI2Cx->CR1 &= ~(1 << 0);
	}
}

void I2C_AckControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi){
	if (EnOrDi == I2C_ACK_ENABLE) {
		pI2Cx->CR1 |= (1 << I2C_CR1_ACK);
	} else {
		pI2Cx->CR1 &= ~(1 << I2C_CR1_ACK);
	}
}

uint32_t RCC_GetPLLOutputClock (void){
	// To be implemented
	return 0;
}




uint32_t RCC_GetPCLK1Value(void){
	uint32_t pclk1, SystemClk;
	uint8_t clksrc,temp,ahbp,apb1;

	// SWS
	clksrc = ((RCC->CFGR >>2) & 0x3);  // Bring those 2 bits to LSB and mask
	if(clksrc == 0){
		SystemClk = 16000000; // HSI
	} else if(clksrc == 1){
		SystemClk = XTAL_VALUE; // replace with xtal value (HSE)
	} else if(clksrc == 2){
		SystemClk = RCC_GetPLLOutputClock (); // To be implemented
	}

	// AHB prescaler
	temp = (RCC->CFGR >> 4) & 0xF; // Bring those 4 bits to LSB and mask
	if(temp <8){
		ahbp = 1;
	} else{
		ahbp = AHB_PreScaler[temp-8];
	}

	//APB1 prescaler
	temp = (RCC->CFGR >> 10) & 0x7; // Bring those 3 bits to LSB and mask
		if(temp <4){
			apb1 = 1;
		} else{
			apb1 = APB1_PreScaler[temp-4];
		}

	pclk1 = (SystemClk / ahbp) / apb1;
	return pclk1;
}

/************************************************
 * 				IT
 ***********************************************/
/*
 *	@fn 	I2C_MasterSendDataIT
 *	@brief
 *
 *	@param  *pI2CHandle
 *	@param
 *	@param
 *	@param
 *
 *	@return RxTxState -> uint8_t type
 */
uint8_t I2C_MasterSendDataIT(I2C_Handle_t *pI2CHandle,uint8_t *pTxbuffer, uint32_t Len, uint8_t SlaveAddr,uint8_t Sr){
	uint8_t busystate = pI2CHandle->TxRxState;

	if( (busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))	{
		pI2CHandle->pTxBuffer = pTxbuffer;
		pI2CHandle->TxLen = Len;
		pI2CHandle->TxRxState = I2C_BUSY_IN_TX;
		pI2CHandle->DevAddr = SlaveAddr;
		pI2CHandle->Sr = Sr;

		//Implement code to Generate START Condition
		I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

		//Implement the code to enable ITBUFEN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITBUFEN);

		//Implement the code to enable ITEVTEN Control Bit
		pI2CHandle->pI2Cx->CR2 |= (1 << I2C_CR2_ITEVTEN);

		//Implement the code to enable ITERREN Control Bit
		pI2CHandle->pI2Cx->CR2 |= (1 << I2C_CR2_ITERREN);

	}
	return busystate;
}

uint8_t I2C_MasterReceiveDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint8_t Len, uint8_t SlaveAddr){
	uint8_t busystate = pI2CHandle->TxRxState;

	if( (busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))	{
		pI2CHandle->pRxBuffer = pRxBuffer;
		pI2CHandle->RxLen = Len;
		pI2CHandle->TxRxState = I2C_BUSY_IN_RX;
		pI2CHandle->RxSize = Len; //Rxsize is used in the ISR code to manage the data reception
		pI2CHandle->DevAddr = SlaveAddr;
		//pI2CHandle->Sr = Sr; USA ELA AQUI? ,uint8_t Sr

		//Implement code to Generate START Condition
		I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

		//Implement the code to enable ITBUFEN Control Bit
		pI2CHandle->pI2Cx->CR2 |= (1 << I2C_CR2_ITBUFEN);

		//Implement the code to enable ITEVTEN Control Bit
		pI2CHandle->pI2Cx->CR2 |= (1 << I2C_CR2_ITEVTEN);

		//Implement the code to enable ITERREN Control Bit
		pI2CHandle->pI2Cx->CR2 |= (1 << I2C_CR2_ITERREN);
	}
	return busystate;
}

void I2C_SlaveSendData(I2C_RegDef_t *pI2C, uint8_t data){
	pI2C-> DR = data;
}
uint8_t I2C_SlaveReceiveData(I2C_RegDef_t *pI2C){
	return (uint8_t) pI2C->DR;
}

void I2C_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi){
	if (EnOrDi == ENABLE){
			if(IRQNumber <= 32){
				//program ISER0 register
				*NVIC_ISER0 |= (1 << IRQNumber);
			}else if (IRQNumber > 31 &&  IRQNumber < 64){ //32 to 63
				//program ISER1 register
				*NVIC_ISER1 |= (1 << IRQNumber % 32);
			}else if (IRQNumber >= 64 &&  IRQNumber < 96){ // 64 to 95 (it ends in 81 for our MCU)
				//program ISER2 register
				*NVIC_ISER2 |= (1 << IRQNumber % 64);
			}
		}else{
			if(IRQNumber <= 32){
				//program ICER0 register
				*NVIC_ICER0 |= (1 << IRQNumber);
			}else if (IRQNumber > 31 &&  IRQNumber < 64){
				//program ICER1 register
				*NVIC_ICER1 |= (1 << IRQNumber % 32);
			}else if (IRQNumber >= 64 &&  IRQNumber < 96){
				//program ICER2 register
				*NVIC_ICER2 |= (1 << IRQNumber % 64);
			}
		}
}
void I2C_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority){
	//1. first lets find out the IPR register
		uint8_t iprx = IRQNumber /4;
		uint8_t iprx_section = IRQNumber %4;

		//*(NVIC_PR_BASE_ADDR + (iprx*4) |= (IRQPriority << (8 * iprx_section));
			//lower 4 bits are not implemented, so we need to shift 8 bits to the right

		uint8_t shift_amout = (8 * iprx_section) + (8 - NO_PR_BITS_IMPLEMENTED);
		*(NVIC_PR_BASE_ADDR + iprx) |= (IRQPriority << shift_amout);
}

void I2C_EV_IRQHandling(I2C_Handle_t *pI2CHandle){
	uint32_t temp1, temp2, temp3;
	temp1 = pI2CHandle->pI2Cx->CR2 & (1 << I2C_CR2_ITBUFEN);
	temp2 = pI2CHandle->pI2Cx->CR2 & (1 << I2C_CR2_ITEVTEN);

	// 1. Check and Handle for interrupt generated by SB event (only Master mode)
	temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_SB);
	if(temp1 && temp3){ //SB flag is set, what comes after start? -> ADDRESS!
		// SET 		- when a Start condition is generated
		// CLEARED 	- by reading the SR1 register and followed by writing the DR register OR
		// CLEARED 	- when PE = 0
		if (pI2CHandle->TxRxState == I2C_BUSY_IN_TX){
			I2C_ExecuteAddressPhaseWrite(pI2CHandle->pI2Cx, pI2CHandle->DevAddr);
		} else if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX){
			I2C_ExecuteAddressPhaseRead(pI2CHandle->pI2Cx, pI2CHandle->DevAddr);
		}

	}

	// 2. Check and Handle for interrupt generated by ADDR event
	temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_ADDR);
	if(temp1 && temp3){ //ADDR flag is set
		// SET (master) - address is sent
		// SET (slave)	- address matched with own address
		I2C_ClearADDRFlag(pI2CHandle);

	}

	// 3. Check and Handle for interrupt generated by BTF (Byte Transfer Finished) event
	temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_BTF);
	if(temp1 && temp3){ //BTF flag is set
		// SET (transmission)	- TXE=1, SR=empty, DR=empty
		// SET (reception) 		- RXNE=1, SR=full, DR=full
		if (pI2CHandle->TxRxState == I2C_BUSY_IN_TX){
			// make sure that TXE is also set
			if(pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_TXE)){
				// BFT, TXE=1, SR=empty, DR=empty
				if(pI2CHandle->TxLen == 0){
					// 1. Generate the STOP condition (to close transmission)
					if(pI2CHandle->Sr == I2C_DISABLE_SR) I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
					// 2. Reset all the member elements of the handle structure
					I2C_CloseSendData(pI2CHandle);
					// 3. Notify application about transmission completed
					I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_TX_CMPLT);
				}
			}
		} else if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX){
			// nothing to do
		}
	}

	// 4. Check and Handle for interrupt generated by STOPF event (only in Slave mode)
	temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_STOPF); // (A) Reading SR1 - ALREADY DONE
	if(temp1 && temp3){ //STOPF flag is set
		// SET		- Stop condition detected by slave after an acknowledge
		// CLEARED 	- (A) reading SR1 register followed by a (B) write in the CR1 register OR
		// CLEARED 	- when PE = 0
		// (B) NOW WE JUST NEED TO WRITE CR1
		pI2CHandle->pI2Cx->CR1 |= 0x0000;
		// Notify application
		I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_STOP);
	}

	// 5. Check and Handle for interrupt generated by TXE event
	temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_TXE);
	if(temp1 && temp2 && temp3){ //TXE flag is set
		// CHECK FOR DEVICE MODE (for MASTER)
		if(pI2CHandle->pI2Cx->SR2 & (1 << I2C_SR2_MSL)){
			// Do the data transmission (if is busy in TX)
			if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX){
				I2C_MasterHandleTXEInterrupt(pI2CHandle);
			}
		} else{ // CHECK FOR DEVICE MODE (for SLAVE)
			//SR2 -> TRA says if device is in transmitter or receiver mode
			if (pI2CHandle->pI2Cx->SR2 & (1 << I2C_SR2_TRA)){
				I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_REQ);
			}
		}
	}

	// 6. Check and Handle for interrupt generated by RXNE event
	temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_RXNE);
	if(temp1 && temp2 && temp3){ //RXNE flag is set
		// Check for device mode (master)
		if(pI2CHandle->pI2Cx->SR2 & (1 << I2C_SR2_MSL)){
			// The device is master
			if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX){
				I2C_MasterHandleRXNEInterrupt(pI2CHandle);
			}
		} else { // The device is SLAVE
			if (pI2CHandle->pI2Cx->SR2 & (1 << I2C_SR2_TRA)){
				I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_RCV);
			}
		}
	}
}



static void I2C_MasterHandleTXEInterrupt(I2C_Handle_t *pI2CHandle){
	if(pI2CHandle->TxLen > 0){
		// 1. Load the data to DR
		pI2CHandle->pI2Cx->DR = *(pI2CHandle->pTxBuffer);
		// 2. Decrement the TxLen
		pI2CHandle->TxLen--;
		// 3. Increment buffer address
		pI2CHandle->pTxBuffer++;
	}
}


static void I2C_MasterHandleRXNEInterrupt(I2C_Handle_t *pI2CHandle){
	// Do the data reception
	if(pI2CHandle->RxSize == 1){
		*pI2CHandle->pRxBuffer = pI2CHandle->pI2Cx->DR;
		pI2CHandle->RxLen--;
	}
	if(pI2CHandle->RxSize > 1){
		if(pI2CHandle->RxLen == 2){
			// Clear the ACK bit
			I2C_AckControl(pI2CHandle->pI2Cx, DISABLE);
			}
		// Read DR
		*pI2CHandle->pRxBuffer = pI2CHandle->pI2Cx->DR;
		pI2CHandle->pRxBuffer++;
		pI2CHandle->RxLen--;
	}
	if(pI2CHandle->RxLen == 0){
		// Generate stop condition
		if(pI2CHandle->Sr == I2C_DISABLE_SR) I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
		// Close the I2C data reception
		I2C_CloseReceiveData(pI2CHandle);
		// Notify application
		I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_RX_CMPLT);
	}
}

void I2C_CloseReceiveData(I2C_Handle_t *pI2CHandle){
	//Implement the code to disable ITBUFEN Control Bit
	pI2CHandle->pI2Cx->CR2 &= ~( 1 << I2C_CR2_ITBUFEN);

	//Implement the code to disable ITEVFEN Control Bit
	pI2CHandle->pI2Cx->CR2 &= ~( 1 << I2C_CR2_ITEVTEN);

	pI2CHandle->TxRxState = I2C_READY;
	pI2CHandle->pRxBuffer = NULL;
	pI2CHandle->RxLen = 0;
	pI2CHandle->RxSize = 0;

	if(pI2CHandle->I2C_Config.I2C_ACKControl == I2C_ACK_ENABLE)	{
		I2C_AckControl(pI2CHandle->pI2Cx,ENABLE);
	}

}

void I2C_CloseSendData(I2C_Handle_t *pI2CHandle){
	//Implement the code to disable ITBUFEN Control Bit
	pI2CHandle->pI2Cx->CR2 &= ~( 1 << I2C_CR2_ITBUFEN);

	//Implement the code to disable ITEVFEN Control Bit
	pI2CHandle->pI2Cx->CR2 &= ~( 1 << I2C_CR2_ITEVTEN);

	pI2CHandle->TxRxState = I2C_READY;
	pI2CHandle->pTxBuffer = NULL;
	pI2CHandle->TxLen = 0;
}


/*********************************************************************
 * @fn      		  - I2C_ER_IRQHandling
 *
 * @brief             - Copied from instructor
 * https://github.com/niekiran/MasteringMCU/blob/master/Resources/Source_code/Workspace/stm32f4xx_drivers/drivers/src/stm32f407xx_i2c_driver.c
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              - Complete the code also define these macros in the driver
						header file
						#define I2C_ERROR_BERR  3
						#define I2C_ERROR_ARLO  4
						#define I2C_ERROR_AF    5
						#define I2C_ERROR_OVR   6
						#define I2C_ERROR_TIMEOUT 7
 */

void I2C_ER_IRQHandling(I2C_Handle_t *pI2CHandle){
	uint32_t temp1,temp2;
    //Know the status of  ITERREN control bit in the CR2
	temp2 = (pI2CHandle->pI2Cx->CR2) & ( 1 << I2C_CR2_ITERREN);
/***********************Check for Bus error************************************/
	temp1 = (pI2CHandle->pI2Cx->SR1) & ( 1<< I2C_SR1_BERR);
	if(temp1  && temp2 ){
		//This is Bus error
		//Implement the code to clear the buss error flag
		pI2CHandle->pI2Cx->SR1 &= ~( 1 << I2C_SR1_BERR);
		//Implement the code to notify the application about the error
	   I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_BERR);
	}
/***********************Check for arbitration lost error************************************/
	temp1 = (pI2CHandle->pI2Cx->SR1) & ( 1 << I2C_SR1_ARLO );
	if(temp1  && temp2)	{
		//This is arbitration lost error
		//Implement the code to clear the arbitration lost error flag
		pI2CHandle->pI2Cx->SR1 &= ~( 1 << I2C_SR1_ARLO);
		//Implement the code to notify the application about the error
		I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_ARLO);
	}
/***********************Check for ACK failure  error************************************/
	temp1 = (pI2CHandle->pI2Cx->SR1) & ( 1 << I2C_SR1_AF);
	if(temp1  && temp2)	{
		//This is ACK failure error
	    //Implement the code to clear the ACK failure error flag
		pI2CHandle->pI2Cx->SR1 &= ~( 1 << I2C_SR1_AF);
		//Implement the code to notify the application about the error
		I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_AF);
	}
/***********************Check for Overrun/underrun error************************************/
	temp1 = (pI2CHandle->pI2Cx->SR1) & ( 1 << I2C_SR1_OVR);
	if(temp1  && temp2)	{
		//This is Overrun/underrun
	    //Implement the code to clear the Overrun/underrun error flag
		pI2CHandle->pI2Cx->SR1 &= ~( 1 << I2C_SR1_OVR);
		//Implement the code to notify the application about the error
		I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_OVR);
	}
/***********************Check for Time out error************************************/
	temp1 = (pI2CHandle->pI2Cx->SR1) & ( 1 << I2C_SR1_TIMEOUT);
	if(temp1  && temp2)	{
		//This is Time out error
	    //Implement the code to clear the Time out error flag
		pI2CHandle->pI2Cx->SR1 &= ~( 1 << I2C_SR1_TIMEOUT);
		//Implement the code to notify the application about the error
		I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_TIMEOUT);
	}
}

void I2C_SlaveEnableDisableCallbackEvents(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi){
	if(EnOrDi == ENABLE){
		pI2Cx->CR2 |= (1 << I2C_CR2_ITBUFEN);
		pI2Cx->CR2 |= (1 << I2C_CR2_ITERREN);
		pI2Cx->CR2 |= (1 << I2C_CR2_ITEVTEN);
	} else{
		pI2Cx->CR2 &= ~(1 << I2C_CR2_ITBUFEN);
		pI2Cx->CR2 &= ~(1 << I2C_CR2_ITERREN);
		pI2Cx->CR2 &= ~(1 << I2C_CR2_ITEVTEN);
	}
}


/////////      local

static void  I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx){
	pI2Cx->CR1 |= (1 << I2C_CR1_START); // 8
}

static void I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr){
	SlaveAddr = SlaveAddr << 1;
	SlaveAddr &= ~(1); //SlaveAddr is Slave address + r/nw bit=0
	pI2Cx->DR = SlaveAddr;
}

static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr){
	SlaveAddr = SlaveAddr << 1;
	SlaveAddr |= 1; //SlaveAddr is Slave address + r/nw bit=0
	pI2Cx->DR = SlaveAddr;

}

static void I2C_ClearADDRFlag(I2C_Handle_t *pI2CHandle)
{
	uint32_t dummy_read;
	//check for device mode
	if(pI2CHandle->pI2Cx->SR2 & ( 1 << I2C_SR2_MSL))
	{
		//device is in master mode
		if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
		{
			if(pI2CHandle->RxSize  == 1)
			{
				//first disable the ack
				I2C_AckControl(pI2CHandle->pI2Cx, DISABLE);
				//clear the ADDR flag ( read SR1 , read SR2)
				dummy_read = pI2CHandle->pI2Cx->SR1;
				dummy_read = pI2CHandle->pI2Cx->SR2;
				(void)dummy_read;
			}
		}
		else
		{
			//clear the ADDR flag ( read SR1 , read SR2)
			dummy_read = pI2CHandle->pI2Cx->SR1;
			dummy_read = pI2CHandle->pI2Cx->SR2;
			(void)dummy_read;
		}
	}
	else
	{
		//device is in slave mode
		//clear the ADDR flag ( read SR1 , read SR2)
		dummy_read = pI2CHandle->pI2Cx->SR1;
		dummy_read = pI2CHandle->pI2Cx->SR2;
		(void)dummy_read;
	}
}
