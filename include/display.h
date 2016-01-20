/*
 * display.h
 *
 *  Created on: Dec 10, 2015
 *      Author: Kevin
 */

#ifndef INCLUDE_DISPLAY_H_
#define INCLUDE_DISPLAY_H_

#define SCK_GPIO 14  // SHCP
#define SCK_GPIO_MUX PERIPHS_IO_MUX_MTMS_U
#define SCK_GPIO_FUNC FUNC_GPIO14

#define SI_GPIO 13  // DS
#define SI_GPIO_MUX PERIPHS_IO_MUX_MTCK_U
#define SI_GPIO_FUNC FUNC_GPIO13

#define RCK_GPIO 12 //STCP
#define RCK_GPIO_MUX PERIPHS_IO_MUX_MTDI_U
#define RCK_GPIO_FUNC FUNC_GPIO12

#define PWM_GPIO 15  //OE
#define PWM_0_OUT_IO_MUX PERIPHS_IO_MUX_MTDO_U
#define PWM_0_OUT_IO_NUM 15
#define PWM_0_OUT_IO_FUNC  FUNC_GPIO15
#define PWM_PERIOD 1000

uint8_t character[] = {	0b10000001, /* 0 */
						0b11001111, /* 1 */
						0b10010010, /* 2 */
						0b10000110, /* 3 */
						0b11001100, /* 4 */
						0b10100100, /* 5 */
						0b10100000, /* 6 */
						0b10001111, /* 7 */
						0b10000000, /* 8 */
						0b10001100, /* 9 */
						0b10110000, /* E */
						0b10011001, /* R */
						0b11111111  /* Blank */

};

#define BLANK 12

uint16_t light_level[] = {
		0x300,
		0x200,
		0x100,
		0
};

uint16_t bright_level[] = {
		0,
		5555,
		11111,
		16666
};

void pushChar(uint8_t, bool );
void latchLED(void);

#endif /* INCLUDE_DISPLAY_H_ */
