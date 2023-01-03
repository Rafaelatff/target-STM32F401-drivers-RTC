/*
 * ds1307.h
 *
 *  Created on: 28 de dez de 2022
 *      Author: r.freitas
 */

#ifndef DS1307_H_
#define DS1307_H_

/*
 * 	Application configurable items
 */
#include "stm32f401xx.h"
#define 	DS1307_I2C				I2C1
#define 	DS1307_I2C_GPIO_PORT	GPIOB
#define 	DS1307_I2C_SDA_PIN		GPIO_PIN_NO_9
#define 	DS1307_I2C_SCL_PIN		GPIO_PIN_NO_8
#define 	DS1307_I2C_SPEED		I2C_SCL_SPEED_SM
#define 	DS1307_I2C_PUPD			GPIO_NO_PUPD

/*----------------------------------Timekeeper Registers---------------------------------
 *|  Address | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 | Function |
 *|    0x00  |  CH   |      10 Seconds       |             Seconds           | Seconds  |
 *|    0x01  |   0   |      10 Minutes       |             Minutes           | Minutes  |
 *|    0x02  |   0   | 12/24 | H/AmPm|  10H  |              Hours            |  Hours   |
 *|    0x03  |   0   |   0   |   0   |   0   |   0   |          Day          |   Day    |
 *|    0x04  |   0   |   0   |     10 Date   |               Date            |  Date    |
 *|    0x05  |   0   |   0   |   0   |  10M  |              Month            |  Month   |
 *|    0x06  |            10 Year            |               Year            |  Year    |
 *|    0x07  |  OUT  |   0   |   0   | SQWE  |   0   |   0   |  RS1  |  RS0  |  Control |
 *|  08h-3Fh |                                                               | RAM 56x8 |
 *--------------------------------------------------------------------------------------*/

/*
 * Register address macros
 */
#define DS1307_ADDR_SEC		0x00
#define DS1307_ADDR_MIN		0x01
#define DS1307_ADDR_HRS		0x02
#define DS1307_ADDR_DAY		0x03
#define DS1307_ADDR_DATE	0x04
#define DS1307_ADDR_MONTH	0x05
#define DS1307_ADDR_YEAR	0x06


#define TIME_FORMAT_12HR_AM		0
#define TIME_FORMAT_12HR_PM		1
#define TIME_FORMAT_24HR		2

#define DS1307_I2C_ADDR		0x68

#define SUNDAY		1
#define MONDAY		2
#define TUESDAY		3
#define WEDNESDAY	4
#define THURSDAY	5
#define FRIDAY		6
#define SATURDAY	7

typedef struct{
	uint8_t date;
	uint8_t month;
	uint8_t year;
	uint8_t day;
}RTC_date_t;

typedef struct{
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t time_format;
}RTC_time_t;

/*
 * 	Function prototypes
 */

uint8_t ds1307_init(void);
void ds1307_set_current_time(RTC_time_t *rtc_time);
void ds1307_get_current_time(RTC_time_t *rtc_time);

void ds1307_set_current_date(RTC_date_t *rtc_date);
void ds1307_get_current_date(RTC_date_t *rtc_date);

#endif /* DS1307_H_ */
