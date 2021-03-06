/*
 * ota.cpp
 *
 *  Created on: Dec 10, 2015
 *      Author: Kevin
 */
#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <ota.h>
#include <application.h>
#include "display.h"
#include "Configurator.h"

bool otaInt = false;
bool otaPress = false;
uint8_t otaCount=0;
rBootHttpUpdate* otaUpdater = 0;

extern ClockDisplay led_display;
extern Configurator *config;

void IRAM_ATTR otaInterruptHandler()
{
	otaInt=true;
}


void checkOTA()
{
	if(otaInt==true)
	{
		debugf("Button Change");
		otaInt=false;
		otaPress=!digitalRead(OTA_BUTTON);
	}

	if((otaPress)&&(!digitalRead(OTA_BUTTON)))
	{

		otaCount++;
	}
	else
	{
		otaCount=0;
		otaPress=0;
	}

	if(otaCount>60)
	{
		startOtaUpdate();
		otaCount=0;
	}
}

void startOtaUpdate()
{
		blinkTimer.initializeMs(100, blink).start();
		led_display.showFlash();
		mqtt->disconnect();
		timeTimer.stop();
		pubTimer.stop();
		buttonTimer.stop();

		OtaUpdate();

}
void OtaUpdate_CallBack(bool result) {

	Serial.println("In callback...");
	if(result == true) {
		// success
		uint8_t slot;
		slot = rboot_get_current_rom();
		if (slot == 0) slot = 1; else slot = 0;
		// set to boot new rom and then reboot
		Serial.printf("Firmware updated, rebooting to rom %d...\r\n", slot);
		rboot_set_current_rom(slot);
		System.restart();
	} else {
		// fail
		Serial.println("Firmware update failed!");
	}
}

void OtaUpdate() {

	uint8_t slot;
	rboot_config bootconf;

	Serial.println("Updating...");

	// need a clean object, otherwise if run before and failed will not run again
	if (otaUpdater) delete otaUpdater;
	otaUpdater = new rBootHttpUpdate();

	// select rom slot to flash
	bootconf = rboot_get_config();
	slot = bootconf.current_rom;
	if (slot == 0) slot = 1; else slot = 0;

	char buffer[50];
	sprintf(buffer,"http://%s:%d/%s",config->getFirmwareServer(),config->getFirmwarePort(),ROM_0_URL);
	debugf("Rom %s",buffer);

	otaUpdater->addItem(bootconf.roms[slot], buffer);

	sprintf(buffer,"http://%s:%d/%s",config->getFirmwareServer(),config->getFirmwarePort(),SPIFFS_URL);
	debugf("Spiffs %s",buffer);

    if (slot == 0) {
        otaUpdater->addItem(RBOOT_SPIFFS_0, buffer);
    } else {
        otaUpdater->addItem(RBOOT_SPIFFS_1, buffer);
    }

	otaUpdater->setCallback(OtaUpdate_CallBack);

	// start update
	otaUpdater->start();

}

