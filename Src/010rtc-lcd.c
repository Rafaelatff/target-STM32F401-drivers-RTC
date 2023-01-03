/*
 * 010etc-lcd.c
 *
 *  Created on: 28 de dez de 2022
 *      Author: r.freitas
 */

/*
 * 			  I2C
 * 		SDA_PIN		B9
 * 		SCL_PIN 	B8
 * 		// MCU1 - 270
 * 		// Absolute Beginners - 54 (ITM code) and
 * 		// 55 Testing printf over ARM Cortex M4 ITM+SWO line
 */

#include <stdio.h>
#include "ds1307.h"

void delay(void){
	for (uint32_t i=0; i<5000000; i++);
}

//extern void initialise_monitor_handles(void); //// For SEMI HOSTING only

char* get_day_of_week(uint8_t t);
char* time_to_string(RTC_time_t *rtc_time);
char* date_to_string(RTC_date_t *rtc_date);

// CORTEX SYSTEM TIMER [clock three of uc]
// Since we don't config any clock, we are using BY DEFAULT the HSI
// HSI is an RC oscillator that runs at 16 MHz
#define SYSTICK_TIM_CLK	16000000UL
void init_systick_timer(uint32_t tick_hz) // COPIED FROM AUTHOR
{
	uint32_t *pSRVR = (uint32_t*)0xE000E014;
	uint32_t *pSCSR = (uint32_t*)0xE000E010;

    /* calculation of reload value */
    uint32_t count_value = (SYSTICK_TIM_CLK/tick_hz)-1;

    //Clear the value of SVR
    *pSRVR &= ~(0x00FFFFFFFF);

    //load the value in to SVR
    *pSRVR |= count_value;

    //do some settings
    *pSCSR |= ( 1 << 1); //Enables SysTick exception request:
    *pSCSR |= ( 1 << 2);  //Indicates the clock source, processor clock source

    //enable the systick
    *pSCSR |= ( 1 << 0); //enables the counter
}

int main (){
	RTC_time_t current_time;
	RTC_date_t current_date;

	// initialise_monitor_handles(); // For SEMI HOSTING only

	printf("RTC test\n");

	if(ds1307_init()){
		printf("RTC init has failed");
		while(1);
	}

	//init_systick_timer(1); // 1 interrupt per sec

	printf("Systick init pass\n");

	current_date.day = TUESDAY;
	current_date.date = 03;
	current_date.month = 1;
	current_date.year = 23;

	current_time.hours = 9;
	current_time.minutes = 56;
	current_time.seconds = 0;
	current_time.time_format = TIME_FORMAT_24HR;

	ds1307_set_current_date(&current_date);

	printf("RTC with initial value for date\n");

	ds1307_set_current_time(&current_time);

	printf("RTC with initial value for time\n");

	ds1307_get_current_date(&current_date);
	ds1307_get_current_time(&current_time);

	char *am_pm;
	if(current_time.time_format != TIME_FORMAT_24HR){
		am_pm = (current_time.time_format) ? "PM" : "AM";
		printf ("Current time = %s %s\n", time_to_string(&current_time), am_pm); // 04:25:00 PM
	} else{
		printf ("Current time = %s\n", time_to_string(&current_time)); // 04:25:00
	}
	printf("Current date = %s <%s>\n", date_to_string(&current_date), get_day_of_week(current_date.day));

	printf("Main will enter while (1) loop\n");

	while(1);
}
/// HANDLER
void SysTick_Handler(void){ // Tick UPCASE
	RTC_time_t current_time;
	RTC_date_t current_date;

	ds1307_get_current_date(&current_date);
	ds1307_get_current_time(&current_time);

	char *am_pm;
	if(current_time.time_format != TIME_FORMAT_24HR){
		am_pm = (current_time.time_format) ? "PM" : "AM";
		printf ("Current time = %s %s\n", time_to_string(&current_time), am_pm); // 04:25:00 PM
	} else{
		printf ("Current time = %s\n", time_to_string(&current_time)); // 04:25:00
	}
	printf("Current date = %s <%s>\n", date_to_string(&current_date), get_day_of_week(current_date.day));
}

char* get_day_of_week(uint8_t i){
	char* days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
	return days[i-1];
}

void number_to_string(uint8_t num, char* buf){
	if(num<10){
		buf[0] = '0';
		buf[1] = num+48;
	} else if (num >= 10 && num < 99){
		buf[0] = (num/10) +48;
		buf[1] = (num%10) +48;
	}
}

char* time_to_string(RTC_time_t *rtc_time){ //hh:mm:ss -> 9 char
	static char buf[9];
	buf[2]=':';
	buf[5]=':';
	number_to_string (rtc_time->hours,buf);
	number_to_string (rtc_time->minutes,&buf[3]);
	number_to_string (rtc_time->hours,&buf[6]);
	buf[8] = '\0';
	return buf;
}

char* date_to_string(RTC_date_t *rtc_date){ //dd/mm/yy  -> 9 char
	static char buf[9];
	buf[2]='/';
	buf[5]='/';
	number_to_string (rtc_date->date,buf);
	number_to_string (rtc_date->month,&buf[3]);
	number_to_string (rtc_date->year,&buf[6]);
	buf[8] = '\0';
	return buf;
}
