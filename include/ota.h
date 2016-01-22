/*
 * ota.h
 *
 *  Created on: Dec 10, 2015
 *      Author: Kevin
 */

#ifndef INCLUDE_OTA_H_
#define INCLUDE_OTA_H_

#include "KMKMqttClient.h"

extern Timer blinkTimer;
extern Timer buttonTimer;
extern Timer timeTimer;
extern Timer pubTimer;

extern KMKMqttClient mqtt;

#endif /* INCLUDE_OTA_H_ */
