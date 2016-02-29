/*
 * mqtt.h
 *
 *  Created on: Feb 25, 2016
 *      Author: Kevin
 */

#ifndef INCLUDE_MQTT_H_
#define INCLUDE_MQTT_H_

void mqttMessageRecv(String,String);

#define TZ_TOPIC "pi-red/clock/tz"

extern NtpClient *ntpC;

void showADC(int);
void manualBright(int);

#endif /* INCLUDE_MQTT_H_ */
