# target-STM32F401-drivers-RTC
This project uses STM32CubeIDE and it's a program created to practice my C habilities during the course 'Mastering Microcontroller and Embedded Driver Development' from FastBit Embedded Brain Academy. I am using a NUCLEO-F401RE board.

## challenge

Create a BSP (board support package) to communicate with the RTC board (model Tiny RTC I2C modules) that uses the IC DS1307. 

![RTC](https://user-images.githubusercontent.com/58916022/210247290-eae4c12d-ea3f-4e45-b999-0500a2d25f5a.jpeg)

*About the RTC* (some info were collected from datasheet)

![image](https://user-images.githubusercontent.com/58916022/210247891-a66752e5-ea74-4f41-b138-5c4e226b9804.png)

- The DS1307 Serial Real-Time Clock is a low-power, full binary-coded decimal (BCD) clock/calendar plus 56 bytes of NV SRAM.

- The DS1307 operates as a slave device on the serial bus. DS1307 address is 1101000 (0x68).

- The RTC registers are located in address locations 00h to 07h.

- The RAM registers are located in address locations 08h to 3Fh.

Since the RTC uses BCD code, we need two functions. One to convert the binary number (integer) to its value in BCD format, so we can configure the initial time for RTC. The second code to convert the read BCD value to its decimal.

The fist code is showed next (binary to BCD):

```
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
```
The code above, receives a **value** with uint8_t type that represents a binary integer value (0 to 255). Then, if the value is less than 10, it just return the same value (in this case, binary value and bcd are the same). For values bigger than 10 we need to separe the digits and then convert the digits to hexadecimal. Eg.: 15 is equal to 0x15.

If the value is bigger (or equal to 10), **m** receives the decimal place (second order digit) of **value** by having **value** divided by 10. 

Then **n** receives the remainder of the integer divission of **value** by 10. This is done by using the modulo operator (%). In **n** we have the first order digit separeted.

Ten, **bcd**, that is a 8 bits value, receives in its most significant nibble the **m** value (m<<4) and **n** in its less significant nibble (|n). 

Now the code for BCD to decimal:

```
static uint8_t bcd_to_binary(uint8_t value){
	uint8_t m,n, binary;
	m = (uint8_t)((value>>4) * 10);
	n = value & (uint8_t)0x0F;
	binary = m + n;
	return binary;
}
```
To convert the bcd code to binary, we need to separed the most significant nibble (that represents the second order digit or decimal place) from the less signficant nibble (that represents the first order digit).

The **m** receveis the **value** after shiffted 4 positions, so the second order digit becomes a first order digit and then we multiply the result by 10. We have a real value that represents the second order digit digit.

The **n** value does a mask of **value** with the less significant nibble, keeping the value on those bits and make as 0's the most significant nibble. This way, **n** keeps only the first order digit.

To find the integer value (binary), it is just needed to sum **m** with **n**.

Note tha both code are helper (local) functions of the bsp code for the ds1307 RTC. Both are declared at the top of the 'ds1307.c' file.

## bsp code, header file

The source and header files of the bsp of the RTC (DS1307) are kept inside the created 'bsp' folder inside the project three, as showed next:

![image](https://user-images.githubusercontent.com/58916022/210258759-42d6f60e-2ba6-48c5-ba74-1f4a3b6da7b4.png)

In the header file (ds1307.h), we create the structs that will hold informations such as time and date. 

![image](https://user-images.githubusercontent.com/58916022/210258900-81ac9269-4fce-40de-bb20-975f40b8d25f.png)

We create some macros that would keep the register addresses, slave address and others.

![image](https://user-images.githubusercontent.com/58916022/210259009-c5e8a9ee-ead4-4fd3-af22-db422689d29c.png)

We create the function prototypes.

![image](https://user-images.githubusercontent.com/58916022/210259066-9b1916d9-485a-4414-a200-99a27ab6ad86.png)

And last, at the top of the document, we create the application configurable items:

![image](https://user-images.githubusercontent.com/58916022/210259187-1245d748-7a0e-4876-9ea5-704229812bad.png)

Those items, are the importantant #include's and #define's that will connect to others peripherals (and also how they are going to connect). We add the #include for the target microcontrolador and the #define's for the I2Cx channel, Port x, SDA and SCL pins, serial clock speed and also use of internal pull-up resistor.

## bsp code, source file

The first function to be called by the user code is the initialization function. This function must configure the I2C pins to communicate with the RTC and also need to clear the CH (Clock Halt) bit in 0x00 register address.

![image](https://user-images.githubusercontent.com/58916022/210262052-66fd6182-b2f1-4f23-afd3-fb6e1e51e127.png)

After clearing the CH bit, we also read back its value and return it as a result of this function. This way, user code can stop and be held by the following code that init and check (easy to check if there is an error while debbuging):

![image](https://user-images.githubusercontent.com/58916022/210262475-fd0bffcc-d916-40c2-a3ae-dc5931cabb11.png)

The whole code for initialization is presented:

![image](https://user-images.githubusercontent.com/58916022/210261815-e8825388-de4d-43c6-b8e8-cd85419b9fad.png)

We use the *memset* function to clear all the bits of the GPIO_handle variables that we create to configure the GPIO pins. 

- *memset(&<variable name>, <value to be set>, <size of the variable>);*

The first function called inside the *ds1307_init* is the *ds1307_i2c_pin_config*. This function is responsable to configure the GPIO pins of the I2C as Alternate Function and later as I2C. **Note that we used macros inside this function, so if the user wants to change the I2C channel or pins, the user must go to the ds1307.h file and change the macros there!**.

Just two small remembers: 

```
	i2c_sxx.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	// This line configure the GPIO as Alternate Function (GPIOx_MODER = 0x2 or 0b10)
```
To know which alternate function must be used, the datasheet must be checked. In our case, to use pins PB8 and PB9 (I2C1), the AF04 must be set.

![image](https://user-images.githubusercontent.com/58916022/210263472-6fd12b63-ebca-48b5-9ea3-a59f8b778b83.png)

```
	i2c_sxx.GPIO_PinConfig.GPIO_PinAltFunMode = 4;
	// This line chooses the AFx to be used (GPIOx_AFLx = 0x4 or 0b0100)
```
Then, the second function called by *ds1307_init* is the *ds1307_i2c_config*. Once the I2C GPIOs are configured, the I2C channel must be configured. That is also a local/helper function. A global variable with type **I2C_Handle_t** with name **g_ds1307I2cHandle** was declared. The *g_* denotes global.
	
![image](https://user-images.githubusercontent.com/58916022/210264074-09c62777-07af-481d-805a-289990c3ef1e.png)

We configure the pI2Cx to be the one declared as macro inside the header file (ds1307.h). We enable the acknolodge bit (I guess this line can be deleted, once this bit can be set only after the peripheral is enabled). We also configure the I2C clock speed (using the macro in header file) and call the *I2C_Init* function with the values inside the **g_ds1307I2cHandle**.
	
To learn more about the I2C configuration just [click here](https://github.com/Rafaelatff/target-STM32F401-drivers-I2C). The main focus here is the RTC.
	
We enable the clock for the I2C peripheral and then clear the bit CH of DS1307. The function that writes commands on the RTC register using the I2C is the *ds1307_write*.

![image](https://user-images.githubusercontent.com/58916022/210264805-10a61a1f-2a88-4943-ad31-6dadc434ae48.png)

This function receives the register address (uint8_t reg_addr) and the value that must be write (uint8_t value). The function that sends it by the I2C is the *I2C_MasterSendData(&<I2C_Handle_t variable name>, <register address and data>, <number of bytes to be written>, <slave address>, <repeated start>)*.

We have then, the functions to set date and time and the functions to get current date and time. Lets start by the *ds1307_set_current_date*.

The *ds1307_set_current_date* function simply uses the ds1307 function to write inside the register addresses of the DS1307. Note that we have the register address represented as macros already and the value is the returned result of the functions *binary_to_bcd*.
	
![image](https://user-images.githubusercontent.com/58916022/210265384-56887792-783e-4fe6-8784-38847a755684.png)

The function *ds1307_set_current_time* is a little bit more complicated, once it needs to:

	1. Write the seconds but garantee that the CH bit won't be alterated;
	2. Write the minutes;
	3. White the hours, but need to check the time format (12H or 24H) and also need to check if is AM or PM.

![image](https://user-images.githubusercontent.com/58916022/210265546-d098793c-65b1-4dfd-b293-044a20fe55cc.png)

Now the get functions, starting by the *ds1307_get_current_date*.
	
![image](https://user-images.githubusercontent.com/58916022/210266079-e8afbaba-b7ae-4301-bbd3-218f92ec5aa1.png)

This functions receives the value of ds1307_read and converts to its integer value (the received value is in BCD format).

The *ds1307_read* function, is a local/helper function. It writes the register address that will be read by using the *I2C_MasterSendData* function and then read the received value from the slave by the *I2C_MasterReceiveData*. Note that this function will read only 1 byte.
	
![image](https://user-images.githubusercontent.com/58916022/210266250-21feb0a2-8712-4f54-a76f-59664a8303b2.png)

The function *ds1307_get_current_time* works similarly, but also with a few complexity due to the time format (12h or 24h, AM or PM).

![image](https://user-images.githubusercontent.com/58916022/210266556-602724a2-c19f-43e3-bd1a-668ba8feabc7.png)
	
## user code 010rtc-lcd.c
	
The initial code is really simple. We create two variables to hold current time and current date information. Then We use the ITM monitor to help during debug of code and we initialize the ds1307. We also initialize the systick timer and configure it to generate 1 interrupt per second.
	
![image](https://user-images.githubusercontent.com/58916022/210270157-27a3c48d-aaf8-42f1-9f82-0854ccbd272f.png)

We atribute some values to the variables that holds current date and current time and write those values inside DS1307.
	
![image](https://user-images.githubusercontent.com/58916022/210270223-2fc9d2be-7036-487f-baad-6faef36d6555.png)

Then we get those values from the DS1307. To print those values in ITM terminal, we use the *printf* function with some helper/local functions.
	
![image](https://user-images.githubusercontent.com/58916022/210270339-200cbc01-cc2a-4ae9-911d-ded31416db6a.png)

The *time_to_string* function fill an array of 9 characters (hh:mm:ss) that will receive the values of type *RTC_time_t*. Position 2 and 5 of the array holds the fix character of ':'.

![image](https://user-images.githubusercontent.com/58916022/210272836-993d6304-ee40-4ed1-955a-317874900e92.png)
	
The *time_to_string* function call another helper/local function called *number_to_string*. This function converts the integer number into its value according to the ASCII table. It is possible to see (next image from the [internet](https://www.johndcook.com/blog/2022/05/28/how-to-memorize-the-ascii-table/)) that the ASCII number  is the number plus 48

![image](https://user-images.githubusercontent.com/58916022/210273575-f41264a0-e523-4d9e-812a-041b0e81733e.png)

The code *number_to_string* separates the first digit to the second digit and them convert them to the its ASCII value.
	
![image](https://user-images.githubusercontent.com/58916022/210273146-81430779-206b-4db6-8856-7ff6c902fdb6.png)
	
The *date_to_string* function is similar to the *time_to_string*.
	
![image](https://user-images.githubusercontent.com/58916022/210273822-1ed80807-b48c-4c1f-b75b-902c075a6c1e.png)
	
The *get_day_of_week* function is responsable to convert the number in *current_date.day* variable into a string.

![image](https://user-images.githubusercontent.com/58916022/210273896-2ee01182-60a1-4f8a-8da2-0828e2a69b58.png)

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

And also the code after debugging both lines:

```
	ds1307_get_current_date(&current_date);
	ds1307_get_current_time(&current_time);	
```
And the result:

![image](https://user-images.githubusercontent.com/58916022/210245688-8ec5bdfc-9c4a-45ff-9cbe-aee54c23c92b.png)

*Note: The difference between the programmed time and the read time is because I leave the computer to do a different task, and later (29 minutes later) I returned to my desk and continued with the debbuging*

## code in loop


## Other errors during compilation/debugging

1: ![image](https://user-images.githubusercontent.com/58916022/210183921-83712f20-6c47-4459-8f87-f8a49519ab7c.png)

It alto wans't programming the MCU. I disconnected all the cables to RTC and then it worked.

