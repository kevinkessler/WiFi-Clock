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

#define BLANK 16
#define E_CHAR 10
#define F_CHAR 11
#define H_CHAR 12
#define L_CHAR 13
#define R_CHAR 14
#define S_CHAR 15

class ClockDisplay {
private:
	HardwarePWM *HWpwm;
	Timer dimTimer;
	bool setBright=true;
	bool displayADC=false;
	bool flash=false;
	bool blink=false;
	bool toggle=false;
	uint8_t errCode=0;
	uint32_t totalADC;
	uint16_t countADC;

	void pushChar(uint8_t, bool );
	void latchLED(void);
	void displayNumber(uint16_t);
	void setBrightness(void);
	void setDisplayLevel(void);

public:
	ClockDisplay(void);
	void manualBright(int);
	void showADC(int);
	void displayTime(void);
	void showFlash(void);
	void showError(uint8);
	double adcAve(void);

};
#endif /* INCLUDE_DISPLAY_H_ */
