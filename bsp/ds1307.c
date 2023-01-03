/*
 * ds1307.c
 *
 *  Created on: 28 de dez de 2022
 *      Author: r.freitas
 */

#include <stdint.h>
#include <string.h>
#include "ds1307.h"

// I2C addresses
// Register addresses
// Data structure to handle the information <3
// Function prototypes which are exposed to applications
// Application configurable items

/*
 * 	Local function prototypes
 */
static void ds1307_i2c_pin_config(void);
static void ds1307_i2c_config(void);
static void ds1307_write(uint8_t reg_addr, uint8_t value);
static uint8_t ds1307_read(uint8_t reg_addr);
static uint8_t binary_to_bcd(uint8_t value);
static uint8_t bcd_to_binary(uint8_t value);

/*
 * 	Variables
 */
I2C_Handle_t g_ds1307I2cHandle; // g_ for global

uint8_t ds1307_init(void){

	// 1. Initialize the I2C pins
	ds1307_i2c_pin_config();

	// 2. Initialize the I2C peripheral
	ds1307_i2c_config();

	// 3. Enable the I2C peripheral
	I2C_PeripheralControl(DS1307_I2C, ENABLE);

	// 4. Make clock halt (CH) equal to 0
	ds1307_write (DS1307_ADDR_SEC, 0x00);

	// 5. Read back CH bit
	uint8_t clock_state = ds1307_read(DS1307_ADDR_SEC);

	return (clock_state >> 7) & 0x1; // CH = bit masked with 0x1;
	// if this function return the value 1, CH = 1 - FAIL
	// if this function return the value 0, CH = 0 - SUCCESS
}

void ds1307_set_current_time(RTC_time_t *rtc_time){
	uint8_t seconds, hrs;

	seconds = binary_to_bcd(rtc_time->seconds);
	seconds &= ~(1<<7); // To guarantee that CH won't be 1 and will halt the clock
	ds1307_write(DS1307_ADDR_SEC, seconds);

	ds1307_write(DS1307_ADDR_SEC, binary_to_bcd(rtc_time->minutes));

	hrs = binary_to_bcd(rtc_time->hours);
	if(rtc_time->time_format==TIME_FORMAT_24HR){
		hrs &= ~(1 << 6);	//bit 6 = 0 = 24h format
	} else{
		hrs |= (1 << 6);	//bit 6 = 1 = 12h format
		// bit 5 = 1 -> PM   and bit 5 = 0 -> AM
		hrs = (rtc_time->time_format == TIME_FORMAT_12HR_PM) ? hrs | (1<<5) : hrs & ~(1<<5);
	}
	ds1307_write(DS1307_ADDR_HRS, hrs);
}

void ds1307_get_current_time(RTC_time_t *rtc_time){
	uint8_t seconds, hrs;

	seconds = ds1307_read(DS1307_ADDR_SEC);
	seconds &= ~(1<<7);
	rtc_time->seconds = bcd_to_binary(seconds);
	rtc_time->minutes = bcd_to_binary(ds1307_read(DS1307_ADDR_MIN));

	hrs = ds1307_read(DS1307_ADDR_HRS);
	if(hrs & (1<<6)){
		//12H format
		rtc_time->time_format = !((hrs & (1<<5)) == 0);
	} else{
		//24H format
		rtc_time->time_format = TIME_FORMAT_24HR;
	}
		rtc_time->hours = bcd_to_binary(hrs);
}

void ds1307_set_current_date(RTC_date_t *rtc_date){
	ds1307_write(DS1307_ADDR_DATE, binary_to_bcd(rtc_date->date));
	ds1307_write(DS1307_ADDR_MONTH, binary_to_bcd(rtc_date->month));
	ds1307_write(DS1307_ADDR_YEAR, binary_to_bcd(rtc_date->year));
	ds1307_write(DS1307_ADDR_DAY, binary_to_bcd(rtc_date->day));
}
void ds1307_get_current_date(RTC_date_t *rtc_date){
	rtc_date->day =  bcd_to_binary(ds1307_read(DS1307_ADDR_DAY));
	rtc_date->date = bcd_to_binary(ds1307_read(DS1307_ADDR_DATE));
	rtc_date->month = bcd_to_binary(ds1307_read(DS1307_ADDR_MONTH));
	rtc_date->year = bcd_to_binary(ds1307_read(DS1307_ADDR_YEAR));
}

/*
 * 	Local (helper) functions
 */

static void ds1307_i2c_pin_config(void){
	GPIO_Handle_t i2c_sda, i2c_scl;

	memset(&i2c_sda, 0, sizeof(i2c_sda));
	memset(&i2c_scl, 0, sizeof(i2c_scl));

	// I2C1_SDA - PB9 (AF04)
	i2c_sda.pGPIOx = DS1307_I2C_GPIO_PORT;
	i2c_sda.GPIO_PinConfig.GPIO_PinAltFunMode = 4;
	i2c_sda.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	i2c_sda.GPIO_PinConfig.GPIO_PinNumber = DS1307_I2C_SDA_PIN;
	i2c_sda.GPIO_PinConfig.GPIO_PinOpType = GPIO_OP_TYPE_OD;
	i2c_sda.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	i2c_sda.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GPIO_Init(&i2c_sda);

	// I2C1_SCL - PB8 (AF04)
	i2c_scl.pGPIOx = DS1307_I2C_GPIO_PORT;
	i2c_scl.GPIO_PinConfig.GPIO_PinAltFunMode = 4;
	i2c_scl.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	i2c_scl.GPIO_PinConfig.GPIO_PinNumber = DS1307_I2C_SCL_PIN;
	i2c_scl.GPIO_PinConfig.GPIO_PinOpType = GPIO_OP_TYPE_OD;
	i2c_scl.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	i2c_scl.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GPIO_Init(&i2c_scl);
}

static void ds1307_i2c_config(void){
	g_ds1307I2cHandle.pI2Cx = DS1307_I2C;
	g_ds1307I2cHandle.I2C_Config.I2C_ACKControl = I2C_ACK_ENABLE; // Don't know why (doesn't work bf set PE)
	g_ds1307I2cHandle.I2C_Config.I2C_SCLSpeed = DS1307_I2C_SPEED;
	I2C_Init(&g_ds1307I2cHandle);
}

static void ds1307_write(uint8_t reg_addr, uint8_t value){
	uint8_t tx[2];
	tx[0] = reg_addr;
	tx[1] = value;
	I2C_MasterSendData(&g_ds1307I2cHandle, tx, 2, DS1307_I2C_ADDR, 0);
}

static uint8_t ds1307_read(uint8_t reg_addr){
	uint8_t data;
	I2C_MasterSendData(&g_ds1307I2cHandle, &reg_addr, 1, DS1307_I2C_ADDR, 0);
	I2C_MasterReceiveData(&g_ds1307I2cHandle, &data, 1, DS1307_I2C_ADDR);
	return data;
}

static uint8_t binary_to_bcd(uint8_t value){
	uint8_t m,n, bcd;
	bcd = value;
	if(value >= 10){
		m = value/10;
		n = value %10;
		bcd = (uint8_t)((m<<4) |n);
	}
	return bcd;
}

static uint8_t bcd_to_binary(uint8_t value){
	uint8_t m,n, binary;
	m = (uint8_t)((value>>4) * 10);
	n = value & (uint8_t)0x0F;
	binary = m + n;
	return binary;
}
