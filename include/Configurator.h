/*
 * Configurator.h
 *
 *  Created on: Mar 14, 2016
 *      Author: kevin
 */

#ifndef INCLUDE_CONFIGURATOR_H_
#define INCLUDE_CONFIGURATOR_H_

#define CONFIG_MAGIC 0xBEAD
#define CONFIG_ADDR 0x1fc000
#define DEFAULT_MQTT_SERVER "pi-hub.home"
#define DEFAULT_MQTT_PORT 1883
#define DEFAULT_FIRMWARE_SERVER "192.168.1.4"
#define DEFAULT_FIRMWARE_PORT 80

typedef struct {
	uint16_t magic;
	char mqtt_server[100];
	uint16_t mqtt_port;
	char firmware_server[100];
	uint16_t firmware_port;
} clock_config;

class Configurator
{
private:
	clock_config config;

public:
	Configurator(void);
	char *getMQTTServer(void);
	uint16_t getMQTTPort(void);
	void setMQTTServer(const char *);
	void setMQTTPort(uint16_t);
	char *getFirmwareServer(void);
	uint16_t getFirmwarePort(void);
	void setFirmwareServer(const char *);
	void setFirmwarePort(uint16_t);
	uint8_t saveConfiguration(void);
};


#endif /* INCLUDE_CONFIGURATOR_H_ */
