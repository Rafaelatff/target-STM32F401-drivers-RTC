# target-STM32F401-drivers-RTC
This project uses STM32CubeIDE and it's a program created to practice my C habilities during the course 'Mastering Microcontroller and Embedded Driver Development' from FastBit Embedded Brain Academy. I am using a NUCLEO-F401RE board.

## challenge


## code

## debugging with ITM

1. Is necessary to configurate the debugging option by:

![image](https://user-images.githubusercontent.com/58916022/210183330-324c8dc0-5b79-4211-a9c2-acfccf8e56f7.png)


2. Add the ITM code (syscalls.c)

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
3. Add the SWV ITM Data Console

Switch to the debbuging view, and go to 'Window'-> 'Show View' -> 'SWV' and selecte the option 'SWV ITM Data Console'.

![image](https://user-images.githubusercontent.com/58916022/210184296-7699f2d9-f10b-41b1-8af4-a03667e0b147.png)

Click in the 'Configure trace' button.

![image](https://user-images.githubusercontent.com/58916022/210184332-15a6e00f-d8d3-4015-93a3-12562ccffa84.png)

Select the 'ITM Stimulus Port' number 0 and click 'OK'.

![image](https://user-images.githubusercontent.com/58916022/210184356-51f9900f-0826-455f-ae0e-56e789e3b216.png)

Click in the 'Start trace' button and if needed 'Terminate and Relaunch'.

![image](https://user-images.githubusercontent.com/58916022/210184378-c45514fd-6701-4380-bec0-08c2da2bfca8.png)

Now, the *printf* messages will appear at the console:

![image](https://user-images.githubusercontent.com/58916022/210184405-eddd21ed-6ce9-4ed9-bead-5176f25cbb01.png)

## Other errors during compilation/debugging

1: ![image](https://user-images.githubusercontent.com/58916022/210183921-83712f20-6c47-4459-8f87-f8a49519ab7c.png)

It alto wans't programming the MCU. I disconnected all the cables to RTC and then it worked.

