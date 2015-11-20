#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <rboot/rboot.h>

#define LED_PIN 4
#define OTA_BUTTON 0

Timer procTimer;
Timer buttonTimer;

bool state = true;
bool otaInt = false;
bool otaPress = false;
int otaCount=0;

rBootHttpUpdate* otaUpdater = 0;

void blink(void);
void OtaUpdate(void);

void IRAM_ATTR otaInterruptHandler()
{
	otaInt=true;
}

void checkOTA()
{
	if(otaInt==true)
	{
		Serial.print(digitalRead(OTA_BUTTON));
		Serial.println(" Change");
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
		procTimer.initializeMs(100, blink).start();
		Serial.println("OTA Here");
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

	if (slot == 0) {
		otaUpdater->addItem(bootconf.roms[slot], ROM_0_URL);
	} else {
		otaUpdater->addItem(bootconf.roms[slot], ROM_1_URL);
	}

	otaUpdater->setCallback(OtaUpdate_CallBack);

	// start update
	otaUpdater->start();

}

void blink()
{
	digitalWrite(LED_PIN, state);
	state = !state;
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.println("Clock Beginning V3");

	pinMode(OTA_BUTTON,INPUT);
	attachInterrupt(otaPress,otaInterruptHandler,CHANGE);

	pinMode(LED_PIN, OUTPUT);
	procTimer.initializeMs(500, blink).start();
	buttonTimer.initializeMs(50,checkOTA).start();
}
