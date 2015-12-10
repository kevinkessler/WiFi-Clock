#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <rboot/rboot.h>

#include <Libraries/TFT_ILI9163C/TFT_ILI9163C.h>
#include <Libraries/RF24/nRF24L01.h>
#include <Libraries/RF24/RF24.h>

#define LED_PIN 4
#define OTA_BUTTON 0

#define ERROR_NO_TZ 3

Timer procTimer;
Timer buttonTimer;
Timer timeTimer;
Timer pubTimer;

NtpClient *ntpC;


bool state = true;
bool otaInt = false;
bool otaPress = false;
int otaCount=0;
uint8_t errorCode;
double timeZ=100.0;
char mqttClientID[15];

rBootHttpUpdate* otaUpdater = 0;

void blink(void);
void OtaUpdate(void);
void updateTime(void);
void mqttMessageRecv(String,String);

MqttClient mqtt("pi-hub",1883,mqttMessageRecv);
RF24 radio(4, 5);
Timer rfTimer;
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
//const uint64_t pipes[2] = { 0xF0F0F0F0D2LL, 0xF0F0F0F0E1LL};

void loopListen()
{
    // if there is data ready
//    if (radio.available())
//    {
      // Dump the payloads until we've gotten everything
//      unsigned long got_time;
//      bool done = false;
//      while (!done)
//      {
        // Fetch the payload, and see if this was the last one.
//        done = radio.read( &got_time, sizeof(unsigned long) );

        // Spew it
//        Serial.printf("Got payload %lu...\n",got_time);

        // Forward this data packet to UDP/HTTP/MQTT here.

//        delay(5);
//      }

      // First, stop listening so we can talk
//      radio.stopListening();

      // Send the final one back.
	  uint16_t got_time=0x0f0f;
      radio.write( &got_time, sizeof(unsigned long) );
      Serial.println("Sent response.");

      // Now, resume listening so we catch the next packets.
//      radio.startListening();


//    }
}

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
		procTimer.initializeMs(100, blink).start();
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

//	if (slot == 0) {
//		otaUpdater->addItem(bootconf.roms[slot], ROM_0_URL);
//	} else {
//		otaUpdater->addItem(bootconf.roms[slot], ROM_1_URL);
//	}

	otaUpdater->setCallback(OtaUpdate_CallBack);

	// start update
	otaUpdater->start();

}

void updateTime()
{

	Serial.println(SystemClock.getSystemTimeString(eTZ_Local));
	DateTime dt=SystemClock.now();


	uint32_t remaining=(60-dt.Second)*1000-dt.Milliseconds;
	timeTimer.setIntervalMs(remaining);
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
	mqtt.subscribe("pi-red/clock/tz");

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
	debugf("WiFi Clock Beginning Version 3.2");

	pinMode(OTA_BUTTON,INPUT);
	attachInterrupt(otaPress,otaInterruptHandler,CHANGE);

//	pinMode(LED_PIN, OUTPUT);
	//procTimer.initializeMs(500, blink).start();
	buttonTimer.initializeMs(50,checkOTA).start();
	timeTimer.initializeMs(1000,updateTime).start();

	sprintf(mqttClientID,"clock-%lx",system_get_chip_id());
	WifiStation.waitConnection(connectOK,60,connectFail);

	radio.begin();
	radio.setChannel(0x40);
	// optionally, increase the delay between retries & # of retries
	radio.setRetries(15,15);

	// optionally, reduce the payload size.  seems to
	// improve reliability
	radio.setPayloadSize(8);

	//
	// Open pipes to other nodes for communication
	//

	// This simple sketch opens two pipes for these two nodes to communicate
	// back and forth.
	// Open 'our' pipe for writing
	// Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)
	// In this case we will pong back (because we are the server)
	radio.openWritingPipe(pipes[1]);
	//radio.openReadingPipe(1,pipes[0]);

	//
	// Start listening
	//
	//radio.startListening();

	//
	// Dump the configuration of the rf unit for debugging
	//
	radio.printDetails();
	Serial.println("Initialization completed.");

	//rfTimer.initializeMs(1000, loopListen).start();

}
