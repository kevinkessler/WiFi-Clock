#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <rboot/rboot.h>
#include <application.h>
#include "display.h"

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
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);
	debugf("WiFi Clock Beginning Version 3.9");

	pinMode(OTA_BUTTON,INPUT);
	attachInterrupt(OTA_BUTTON,otaInterruptHandler,CHANGE);

	pinMode(LED_PIN, OUTPUT);
	blinkTimer.initializeMs(500, blink).start();
	buttonTimer.initializeMs(50,checkOTA).start();
	timeTimer.initializeMs(1000,updateTime).start();

	sprintf(mqttClientID,"clock-%lx",system_get_chip_id());
	WifiAccessPoint.enable(false);
	WifiStation.enable(true);
	//WifiStation.config("xxxxxxxx","XXXXXXXXXX");
	WifiStation.waitConnection(connectOK,60,connectFail);

	Serial.println("Initialization completed.");


}
