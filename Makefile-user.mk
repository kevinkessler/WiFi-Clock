## Local build configuration
## Parameters configured here will override default and ENV values.
## Uncomment and change examples:

#Add your source directories here separated by space
MODULES = app

ifeq ($(OS),Windows_NT)
	ESP_HOME=D:/ESP8266/esp-open-sdk
	SMING_HOME=D:/ESP8266/Sming/Sming
	COM_PORT = COM8
	ESPTOOL2=D:/ESP8266/Tools/esptool2.exe
else
	ESP_HOME = /esptools/esp-open-sdk
	SMING_HOME = /esptools/Sming/Sming
	COM_PORT = /dev/tty.SLAB_USBtoUART
	ESPTOOL2 = /esptools/esp8266/esptool2/esptool2
endif

#

# Com port speed
COM_SPEED	= 115200
#


SPIFF_FILES=web
SPI_SIZE=4M
RBOOT_ENABLED=1
RBOOT_TWO_ROMS  = 0
RBOOT_ROM_0     = rom0
## input linker file for first rom
RBOOT_LD_0      = rom0.ld
## these next options only needed when using two rom mode
# RBOOT_ROM_1     = rom1
# RBOOT_LD_1		= rom1.ld
TERMINAL = true
KILL_TERM= true

.PHONY: copy flashparam
copy:
	scp out/firmware/*.bin kevin@raspberrypi2:/var/www
flashparam:
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED_ESPTOOL) write_flash $(flashimageoptions) 0x3fc000 esp_init_data_adc_33.bin