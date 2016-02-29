/*
 * mqtt.cpp
 *
 *  Created on: Feb 25, 2016
 *      Author: Kevin
 */
#include <SmingCore/SmingCore.h>
#include "KMKMqttClient.h"
#include "mqtt.h"

double timeZ=100.0;
char mqttClientID[15];

KMKMqttClient mqtt("pi-hub.home",1883,mqttMessageRecv);

bool prefix(const char *pre, const char *str)
{
	return strncmp(pre,str,strlen(pre)) == 0;
}
void mqttMessageRecv(String topic, String message)
{
	char buffer[20];

	topic.toCharArray(buffer,20,0);
	debugf("Topic: %s",buffer);

	message.toCharArray(buffer,20,0);
	debugf("Message: %s",buffer);

	if(prefix("TZ:",buffer))
	{
		double mTZ=atof(buffer+3);
		debugf("TZ: %f\n",mTZ);

		if(mTZ!=timeZ)
		{
			timeZ=mTZ;
			SystemClock.setTimeZone(timeZ);
			ntpC->requestTime();
		}
	}
	else if (prefix("DIM:",buffer))
	{
		int level=atoi(buffer+4);
		debugf("DIM: %d",level);
		manualBright(level);
	}
	else if(prefix("ADC:",buffer))
	{
		int d=atoi(buffer+4);
		debugf("ADC: %d",d);
		showADC(d);
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


