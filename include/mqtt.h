/*
 * mqtt.h
 *
 *  Created on: Feb 25, 2016
 *      Author: Kevin
 */

#ifndef INCLUDE_MQTT_H_
#define INCLUDE_MQTT_H_

void mqttMessageRecv(String,String);

#define BASE_TOPIC "pi-red/clock/"
#define TZ_TOPIC "tz"
#define CLIENTS_TOPIC "pi-red/clients"
extern NtpClient *ntpC;

void showADC(int);
void manualBright(int);
double readTempData(void);

#endif /* INCLUDE_MQTT_H_ */
