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

bool otaInt = false;
bool otaPress = false;
uint8_t otaCount=0;
rBootHttpUpdate* otaUpdater = 0;


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

	if(otaPress)
	{
		if(!digitalRead(OTA_BUTTON))
		{
			otaCount++;
		}
		else
		{
			otaCount=0;
			otaPress=0;
		}
	}
	if(otaCount>100)
	{
		blinkTimer.initializeMs(100, blink).start();
		mqtt.disconnect();
		timeTimer.stop();
		pubTimer.stop();
		buttonTimer.stop();

		OtaUpdate();
		otaCount=0;
	}
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

	otaUpdater->addItem(bootconf.roms[slot], ROM_0_URL);

	otaUpdater->setCallback(OtaUpdate_CallBack);

	// start update
	otaUpdater->start();

}

