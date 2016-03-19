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
						0b10111000, /* F */
						0b11001000, /* H */
						0b11110001, /* L */
						0b10011001, /* R */
						0b10100100, /* S */
						0b11111111  /* Blank */

};



uint16_t light_level[] = {
		0x300,
		0x200,
		0x100,
		0
};

uint16_t bright_level[] = {
		16666,
		5000,
		1000,
		300
};

uint8_t pins[1]={PWM_GPIO};

void ClockDisplay::setBrightness()
{
	uint32_t total=0;
	uint8_t n;
	uint16_t adc;
	for(n=0;n<16;n++)
	{
		adc=system_adc_read();
		total+=adc;
	}
	adc=total/16;
	totalADC+=adc;
	countADC++;

	if(setBright)
	{

		for(n=0;n<4;n++)
			if(adc>light_level[n])
				break;

		HWpwm->analogWrite(PWM_GPIO,bright_level[n]);
		if(flash)
		{
			if(blink)
				HWpwm->analogWrite(PWM_GPIO,0);

		}
		if(toggle)
		{
			if(blink)
				displayTime();
			else
				showError(errCode);
		}
	}

	blink=!blink;

	if(displayADC)
		displayNumber(adc);

}

ClockDisplay::ClockDisplay()
{

	pinMode(SCK_GPIO,OUTPUT);
	pinMode(RCK_GPIO,OUTPUT);
	pinMode(SI_GPIO,OUTPUT);
	pinMode(PWM_GPIO,OUTPUT);

	digitalWrite(SCK_GPIO,0);
	digitalWrite(RCK_GPIO,0);

	HWpwm=new HardwarePWM(pins,1);
	HWpwm->analogWrite(PWM_GPIO,11111);

	//dimTimer=new Timer();
	dimTimer.initializeMs(1000,TimerDelegate(&ClockDisplay::setBrightness,this)).start();
	//dimTimer->start();

}



void ClockDisplay::manualBright(int level)
{
	if(level<0)
	{
		setBright=true;
	}
	else
	{
		setBright=false;
		HWpwm->analogWrite(PWM_GPIO,level);
	}
}

void ClockDisplay::showADC(int dis)
{
	if(dis==0)
		displayADC=false;
	else
		displayADC=true;
}

void ClockDisplay::showFlash()
{
	flash=true;

	pushChar(H_CHAR,false);
	pushChar(S_CHAR,false);
	pushChar(L_CHAR,false);
	pushChar(F_CHAR,false);

	latchLED();
}
void ClockDisplay::showError(uint8 err)
{
	errCode=err;
	if(errCode==1)
		flash=true;
	else
		toggle=true;

	pushChar(errCode,false);
	pushChar(R_CHAR,false);
	pushChar(R_CHAR,false);
	pushChar(E_CHAR,false);

	latchLED();
}
void ClockDisplay::displayNumber(uint16_t num)
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
void ClockDisplay::displayTime()
{
	if(flash)
		return;

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

double ClockDisplay::adcAve()
{
	if(countADC!=0)
	{
		double retval=totalADC/countADC;
		totalADC=0;
		countADC=0;
		return retval;
	}
	else
		return 0.0;
}

void ClockDisplay::setDisplayLevel()
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

void ClockDisplay::pushChar(uint8_t symbol, bool dp)
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

void ClockDisplay::latchLED()
{
	digitalWrite(RCK_GPIO,1);
	digitalWrite(RCK_GPIO,0);
}


