# target-STM32F401-drivers-RTC
This project uses STM32CubeIDE and it's a program created to practice my C habilities during the course 'Mastering Microcontroller and Embedded Driver Development' from FastBit Embedded Brain Academy. I am using a NUCLEO-F401RE board.

## challenge


## code

## Debugging

![image](https://user-images.githubusercontent.com/58916022/210183330-324c8dc0-5b79-4211-a9c2-acfccf8e56f7.png)


## ITM code (syscalls.c)

Go to syscalls.c file and copy the following code from [niekiran](https://github.com/niekiran/Embedded-C/blob/master/All_source_codes/target/itm_send_data.c) just bellow the #include's.  

![image](https://user-images.githubusercontent.com/58916022/210183710-3e4ee4db-0a96-49b0-9991-e8424471d510.png)

Then, in the _write function, change from:

![image](https://user-images.githubusercontent.com/58916022/210183764-febb2ed0-4053-4213-a9d2-6c5d49190c7e.png)

To the bellow:

![image](https://user-images.githubusercontent.com/58916022/210183789-bcc7148f-67f7-479d-b87a-9743deeed6f3.png)

Make sure the configuration made for [semihosting](https://github.com/Rafaelatff/target-semihosting) aren't applied, else a lot of errors will appear during compilation (all pointed to syscall.c file).

```
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//					Implementation of printf like feature using ARM Cortex M3/M4/ ITM functionality
//					This function will not work for ARM Cortex M0/M0+
//					If you are using Cortex M0, then you can use semihosting feature of openOCD
/////////////////////////////////////////////////////////////////////////////////////////////////////////


//Debug Exception and Monitor Control Register base address
#define DEMCR        			*((volatile uint32_t*) 0xE000EDFCU )

/* ITM register addresses */
#define ITM_STIMULUS_PORT0   	*((volatile uint32_t*) 0xE0000000 )
#define ITM_TRACE_EN          	*((volatile uint32_t*) 0xE0000E00 )

void ITM_SendChar(uint8_t ch)
{

	//Enable TRCENA
	DEMCR |= ( 1 << 24);

	//enable stimulus port 0
	ITM_TRACE_EN |= ( 1 << 0);

	// read FIFO status in bit [0]:
	while(!(ITM_STIMULUS_PORT0 & 1));

	//Write to ITM stimulus port0
	ITM_STIMULUS_PORT0 = ch;
}
```

## Other errors during compilation

1: ![image](https://user-images.githubusercontent.com/58916022/210183921-83712f20-6c47-4459-8f87-f8a49519ab7c.png)

