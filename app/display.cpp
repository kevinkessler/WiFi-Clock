/*
 * display.cpp
 *
 *  Created on: Dec 10, 2015
 *      Author: Kevin
 */

#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <application.h>
#include <display.h>

uint8_t pins[1]={PWM_GPIO};
HardwarePWM HWpwm(pins,1);


Timer dimTimer;

uint16_t dim=0;


uint16_t b=0;

void IRAM_ATTR setBrightness()
{
	b+=5000;
	if (b>21000)
		b=0;
	HWpwm.analogWrite(PWM_GPIO,b);

	uint32_t total=0;
	uint8_t n;
	uint16_t adc;
	for(n=0;n<16;n++)
	{
		adc=system_adc_read();
		total+=adc;
	}
	adc=total/16;


	debugf("ADC %d",adc);
	displayNumber(adc);

}

void displayInit()
{
	pinMode(SCK_GPIO,OUTPUT);
	pinMode(RCK_GPIO,OUTPUT);
	pinMode(SI_GPIO,OUTPUT);
	pinMode(PWM_GPIO,OUTPUT);

	digitalWrite(SCK_GPIO,0);
	digitalWrite(RCK_GPIO,0);


	HWpwm.analogWrite(PWM_GPIO,11111);
	dimTimer.initializeMs(1000,setBrightness).start();

}

void displayNumber(uint16_t num)
{
	uint16_t val=num;
	uint8_t n;

	for (n=0;n<4;n++)
	{
		uint16_t tmp=val;
		val/=10;
		uint8_t dig=tmp-(val*10);
		pushChar(dig,false);
	}
	latchLED();
}
void displayTime()
{
	DateTime dt=SystemClock.now();


	bool am=true;
	bool pm=false;
	uint8_t hour12=dt.Hour;
	if(dt.Hour>12)
	{
		hour12=dt.Hour-12;
		am=false;
		pm=true;
	}

	if(dt.Hour==0)
	{
		hour12=12;
		am=true;
		pm=false;
	}

	uint8_t min_tens=dt.Minute/10;
	uint8_t min_ones=dt.Minute-min_tens*10;
	uint8_t hour_tens=hour12/10;
	uint8_t hour_ones=hour12-hour_tens*10;

	if(hour_tens==0)
		hour_tens=BLANK;

	pushChar(min_ones,pm);
	pushChar(min_tens,false);
	pushChar(hour_ones,true);
	pushChar(hour_tens,false);

	latchLED();
	debugf("Display %d:%d %d %d %d %d\n",dt.Hour,dt.Minute,hour_tens,hour_ones,min_tens,min_ones);

}

void setDisplayLevel()
{


	uint32_t total=0;
	uint8_t n;
	uint16_t adc;
	for(n=0;n<16;n++)
	{
//		adc=system_adc_read();
		total+=adc;
	}
	adc=total/16;

	for(n=0;n<4;n++)
	{
		if(adc>=light_level[n])
			break;
	}

//	pwm_set_duty(bright_level[n],0);
//	pwm_start();

}

void pushChar(uint8_t symbol, bool dp)
{
	uint8_t n;


	for (n=0;n<7;n++)
	{
		if(character[symbol] & (1 << n))
#ifdef COMMON_CATHODE
			digitalWrite(SI_GPIO,0);
#else
			digitalWrite(SI_GPIO,1);
#endif
		else
#ifdef COMMON_CATHODE
			digitalWrite(SI_GPIO,1);
#else
			digitalWrite(SI_GPIO,0);
#endif
			digitalWrite(SCK_GPIO,1);
			digitalWrite(SCK_GPIO,0);
	}

	if(dp==false)
#ifdef COMMON_CATHODE
			digitalWrite(SI_GPIO,0);
#else
			digitalWrite(SI_GPIO,1);
#endif
	else
#ifdef COMMON_CATHODE
		digitalWrite(SI_GPIO,1);
#else
		digitalWrite(SI_GPIO,0);
#endif
		digitalWrite(SCK_GPIO,1);
		digitalWrite(SCK_GPIO,0);

}

void latchLED()
{
	digitalWrite(RCK_GPIO,1);
	digitalWrite(RCK_GPIO,0);
}


