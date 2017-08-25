/*
 * mqtt.cpp
 *
 *  Created on: Feb 25, 2016
 *      Author: Kevin
 */
#include <SmingCore/SmingCore.h>
#include "KMKMqttClient.h"
#include "mqtt.h"
#include "display.h"
#include "Configurator.h"
#include <Libraries/OneWire/OneWire.h>
#include <Libraries/DS18S20/ds18s20.h>
#include "application.h"

double timeZ=100.0;
char mqttClientID[15];

KMKMqttClient *mqtt=null;
extern ClockDisplay led_display;
extern void enable_web_server(uint8);
extern void disable_web_server();
extern void startOtaUpdate(void);
extern Configurator *config;
DS18S20 ReadTemp;

uint8_t mqtt_error_count=0;
String cmdTopic;

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
		led_display.manualBright(level);
	}
	else if(prefix("ADC:",buffer))
	{
		int d=atoi(buffer+4);
		debugf("ADC: %d",d);
		led_display.showADC(d);
	}
	else if(prefix("CMD:",buffer))
	{
		if(strcmp(buffer,"CMD:CONFIG")==0)
		{
			enable_web_server(3);
		}
		else if(strcmp(buffer,"CMD:RESTART")==0)
		{
			System.restart();
		}
		else if(strcmp(buffer,"CMD:FLASH")==0)
		{
			startOtaUpdate();
		}
	}
}

void startMqtt()
{
	sprintf(mqttClientID,"clock-%lx",system_get_chip_id());

	ReadTemp.Init(TEMP_PIN);
	ReadTemp.StartMeasure();

	debugf("Client ID: %s",mqttClientID);
	if(mqtt==null)
		mqtt = new KMKMqttClient(config->getMQTTServer(),config->getMQTTPort(),mqttMessageRecv);
	bool mqtt_status=mqtt->connect(mqttClientID);
	if(!mqtt_status)
	{
		debugf("MQTT Connect fail");
		enable_web_server(2);
	}
	String tzTopic=BASE_TOPIC;
	tzTopic+=TZ_TOPIC;
	mqtt->subscribe(tzTopic);
	cmdTopic=BASE_TOPIC;
	cmdTopic+=mqttClientID;
	mqtt->subscribe(cmdTopic);

}
void kaPub()
{
	if (mqtt->getConnectionState() != eTCS_Connected)
	{
		if(++mqtt_error_count>3)
		{
			debugf("Connection Failure to MQTT Server %s:%d",config->getMQTTServer(),config->getMQTTPort());
			enable_web_server(2);
			return;
		}

		startMqtt(); // Auto reconnect
		debugf("MQTT Connect Error %d",mqtt_error_count);
		return;

	}

	mqtt_error_count=0;
	//disable_web_server();
	mqtt->publish(CLIENTS_TOPIC,mqttClientID);


	char buffer[50];
	String *tempC=new String(readTempData(),3);
	sprintf(buffer,"TEMP:%s",tempC->c_str());
	mqtt->publish(cmdTopic,buffer);

	delete tempC;

	String *adcA=new String(led_display.adcAve(),3);
	sprintf(buffer,"LIGHT:%s",adcA->c_str());
	mqtt->publish(cmdTopic,buffer);

	delete adcA;
}

double readTempData()
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
			ReadTemp.StartMeasure();
			debugf("Temp %f C",tempC);
			return tempC;
		}
		else
			debugf("No Valid Data");


	}
	else
		debugf("Invalid Status");

	ReadTemp.StartMeasure();
	return 0.0;
}
