#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <rboot/rboot.h>
#include <application.h>
#include "display.h"
#include <Libraries/OneWire/OneWire.h>
#include <Libraries/DS18S20/ds18s20.h>

DS18S20 ReadTemp;
ClockDisplay led_display;

Timer blinkTimer;
Timer buttonTimer;
Timer timeTimer;
Timer pubTimer;

NtpClient *ntpC;

bool state = true;
uint8_t errorCode;

extern char mqttClientID[15];

void updateTime()
{

	Serial.println(SystemClock.getSystemTimeString(eTZ_Local));
	DateTime dt=SystemClock.now();


	uint32_t remaining=(60-dt.Second)*1000-dt.Milliseconds;
	timeTimer.setIntervalMs(remaining);

	led_display.displayTime();

}

void blink()
{
	digitalWrite(LED_PIN, state);
	state = !state;

	readTempData();
}

void readTempData()
{
	if(!ReadTemp.MeasureStatus())
	{
		uint8_t tCount;
		tCount=ReadTemp.GetSensorsCount();
		debugf("Sensor Count %d",tCount);

		uint64_t id=ReadTemp.GetSensorID(0);
		debugf("Sensor ID %lx,id");

		if(ReadTemp.IsValidTemperature(0))
		{
			double tempC=ReadTemp.GetCelsius(0);
			debugf("Temp %f C",tempC);
		}
		else
			debugf("No Valid Data");

		ReadTemp.StartMeasure();
	}
	else
		debugf("Invalid Status");
}
void ntpUpdate(NtpClient& client, time_t timestamp)
{
	debugf("Setting Time");
	SystemClock.setTime(timestamp,eTZ_UTC);
	led_display.displayTime();

}

void connectOK()
{
	errorCode=ERROR_NO_TZ;
	debugf("Connect OK");

	startMqtt();
	pubTimer.initializeMs(30000,kaPub).start();

	ntpC=new NtpClient("pool.ntp.org",600);
	ntpC->requestTime();

}

void connectFail()
{
	Serial.println("Connect Fail");
	WifiStation.enable(false);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);
	debugf("WiFi Clock Beginning Version 3.9");

	pinMode(OTA_BUTTON,INPUT);
	attachInterrupt(OTA_BUTTON,otaInterruptHandler,CHANGE);

	pinMode(LED_PIN, OUTPUT);
//	blinkTimer.initializeMs(500, blink).start();
	blinkTimer.initializeMs(2000, blink).start();
	buttonTimer.initializeMs(50,checkOTA).start();
	timeTimer.initializeMs(1000,updateTime).start();

	sprintf(mqttClientID,"clock-%lx",system_get_chip_id());
	WifiAccessPoint.enable(false);
	WifiStation.enable(true);
	//WifiStation.config("xxxxxxxx","XXXXXXXXXX");
	WifiStation.waitConnection(connectOK,60,connectFail);
	ReadTemp.Init(TEMP_PIN);
	ReadTemp.StartMeasure();

	Serial.println("Initialization completed.");


}
