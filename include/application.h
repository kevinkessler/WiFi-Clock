/*
 * application.h
 *
 *  Created on: Dec 10, 2015
 *      Author: Kevin
 */

#ifndef INCLUDE_APPLICATION_H_
#define INCLUDE_APPLICATION_H_


#define LED_PIN 4
#define OTA_BUTTON 0

#define ERROR_NO_TZ 3

#define COMMON_CATHODE 1

void blink(void);
void OtaUpdate(void);
void updateTime(void);

void checkOTA(void);
void displayTime(void);
void displayInit(void);

void IRAM_ATTR otaInterruptHandler();

void startMqtt(void);
void kaPub(void);

#endif /* INCLUDE_APPLICATION_H_ */
