/*
 * configuration.cpp
 *
 *  Created on: Mar 14, 2016
 *      Author: kevin
 */

#include <SmingCore/SmingCore.h>
#include "application.h"
#include <string.h>
#include "Configurator.h"

Configurator::Configurator()
{
	uint8_t retval=flashmem_read((void *)&config,0x40200000+CONFIG_ADDR, sizeof(clock_config));
	if(config.magic!=CONFIG_MAGIC)
	{
		debugf("Invalid Magic Number %x, saving default configuration",config.magic);
		config.magic=CONFIG_MAGIC;
		strcpy(config.mqtt_server,DEFAULT_MQTT_SERVER);
		strcpy(config.firmware_server,DEFAULT_FIRMWARE_SERVER);
		config.mqtt_port=DEFAULT_MQTT_PORT;
		config.firmware_port=DEFAULT_FIRMWARE_PORT;
		uint8_t retval=saveConfiguration();
		debugf("Save returned %d",retval);
	}
}

char *Configurator::getMQTTServer()
{
	return config.mqtt_server;
}

uint16_t Configurator::getMQTTPort()
{
	return config.mqtt_port;
}

char *Configurator::getFirmwareServer()
{
	return config.firmware_server;
}

uint16_t Configurator::getFirmwarePort()
{
	return config.firmware_port;
}

void Configurator::setMQTTServer(const char *server)
{
	uint8_t len=strlen(server);
	if(len>100)
		len=100;

	strncpy(config.mqtt_server,server,len);
	config.mqtt_server[len]='\0';
}

void Configurator::setMQTTPort(uint16_t port)
{
	config.mqtt_port=port;
}

void Configurator::setFirmwareServer(const char *server)
{
	uint8_t len=strlen(server);
	if(len>100)
		len=100;

	strncpy(config.firmware_server,server,len);
	config.firmware_server[len]='\0';
}

void Configurator::setFirmwarePort(uint16_t port)
{
	config.firmware_port=port;
}

uint8_t Configurator::saveConfiguration()
{
	uint32_t sect=flashmem_get_sector_of_address(0x40200000+CONFIG_ADDR);
	debugf("Sect Num %lx",sect);
	uint8_t retval=flashmem_erase_sector(sect);
	if(retval==0)
	{
		debugf("Error erasing Flash");
		return retval;
	}

	retval=flashmem_write(&config,0x40200000+CONFIG_ADDR,sizeof(clock_config));
	return retval;
}
