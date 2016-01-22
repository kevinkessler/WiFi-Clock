#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <rboot/rboot.h>
#include <application.h>
#include "KMKMqttClient.h"

Timer blinkTimer;
Timer buttonTimer;
Timer timeTimer;
Timer pubTimer;

NtpClient *ntpC;


bool state = true;
uint8_t errorCode;
double timeZ=100.0;
char mqttClientID[15];

KMKMqttClient mqtt("pi-hub.home",1883,mqttMessageRecv);
extern void pushChar(uint8_t, bool );
extern void latchLED(void);

void updateTime()
{

	Serial.println(SystemClock.getSystemTimeString(eTZ_Local));
	DateTime dt=SystemClock.now();


	uint32_t remaining=(60-dt.Second)*1000-dt.Milliseconds;
	timeTimer.setIntervalMs(remaining);

	displayTime();

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
	displayTime();

}

void mqttMessageRecv(String topic, String message)
{
	char buffer[20];

	topic.toCharArray(buffer,20,0);
	debugf("Topic: %s",buffer);

	message.toCharArray(buffer,20,0);
	debugf("Message: %s",buffer);
	double mTZ=atof(buffer);
	debugf("TZ: %f\n",mTZ);

	if(mTZ!=timeZ)
	{
		timeZ=mTZ;
		SystemClock.setTimeZone(timeZ);
		ntpC->requestTime();
	}
}

void startMqtt()
{

	debugf("Client ID: %s",mqttClientID);
	mqtt.connect(mqttClientID);
	mqtt.subscribe(TZ_TOPIC);

}
void kaPub()
{
	if (mqtt.getConnectionState() != eTCS_Connected)
		startMqtt(); // Auto reconnect

	mqtt.publish("pi-hub/clients",mqttClientID);
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
	debugf("WiFi Clock Beginning Version 3.8");

	displayInit();
	pinMode(OTA_BUTTON,INPUT);
	attachInterrupt(OTA_BUTTON,otaInterruptHandler,CHANGE);

	pinMode(LED_PIN, OUTPUT);
	blinkTimer.initializeMs(500, blink).start();
	buttonTimer.initializeMs(50,checkOTA).start();
	timeTimer.initializeMs(1000,updateTime).start();

	sprintf(mqttClientID,"clock-%lx",system_get_chip_id());
	WifiAccessPoint.enable(false);
	WifiStation.enable(true);
	WifiStation.config("AstroNet","IHateComputers");
	WifiStation.waitConnection(connectOK,60,connectFail);

	Serial.println("Initialization completed.");


}
