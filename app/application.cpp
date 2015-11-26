#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <rboot/rboot.h>

#define LED_PIN 4
#define OTA_BUTTON 0

#define ERROR_NO_TZ 3

Timer procTimer;
Timer buttonTimer;
Timer timeTimer;
Timer delayTimer;

NtpClient *ntpC;
HttpClient webReq;
MqttClient *mqtt;

bool state = true;
bool otaInt = false;
bool otaPress = false;
int otaCount=0;
uint8_t errorCode;
double timeZ=0.0;

rBootHttpUpdate* otaUpdater = 0;

void blink(void);
void OtaUpdate(void);
void updateTime(void);
void geoRecv(HttpClient& , bool );
void geoSend(void);

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

	if (slot == 0) {
		otaUpdater->addItem(bootconf.roms[slot], ROM_0_URL);
	} else {
		otaUpdater->addItem(bootconf.roms[slot], ROM_1_URL);
	}

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

void geoRecv(HttpClient& client, bool successful)
{
	if(!successful)
	{
		debugf("Geo Recv Failed");
		delayTimer.initializeMs(10000,geoSend).start();

		return;
	}
	debugf("Geo Recv Success");

	String resp=client.getResponseString();
	StaticJsonBuffer<1000> jsonBuffer;
	char buffer[1000];
	resp.toCharArray(buffer,1000,0);
	JsonObject& root = jsonBuffer.parseObject(buffer);
	const char* tz=root["geobytestimezone"];
	Serial.println(tz);

	if(tz!=null)
	{
		int8_t sign = 1;
		if(tz[0]=='-')
			sign=-1;

		uint8_t hr=(tz[1]-'0')*10+tz[2]-'0';
		uint8_t mn=(tz[4]-'0')*10+tz[5]-'0';

		Serial.print("HR ");
		Serial.println(hr);
		Serial.print("MN ");
		Serial.println(mn);

		timeZ=sign*((double)hr+(double)mn/60);

	}
	else
		errorCode=ERROR_NO_TZ;

	Serial.println(timeZ);

	SystemClock.setTimeZone(timeZ);
	ntpC->requestTime();	// Time must get set to update TZ


}

void geoSend()
{
	delayTimer.stop();

	webReq.reset();
	webReq.downloadString("http://getcitydetails.geobytes.com/GetCityDetails",geoRecv);
}
void connectOK()
{
	debugf("Connect OK");
	ntpC=new NtpClient("pool.ntp.org",600);
	ntpC->requestTime();

	delayTimer.initializeMs(10000,geoSend).start();

}

void connectFail()
{
	Serial.println("Connect Fail");
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);
	debugf("WiFi Clock Beginning");

	pinMode(OTA_BUTTON,INPUT);
	attachInterrupt(otaPress,otaInterruptHandler,CHANGE);

	pinMode(LED_PIN, OUTPUT);
	procTimer.initializeMs(500, blink).start();
	buttonTimer.initializeMs(50,checkOTA).start();
	timeTimer.initializeMs(1000,updateTime).start();

	WifiStation.waitConnection(connectOK,60,connectFail);
}
