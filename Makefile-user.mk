## Local build configuration
## Parameters configured here will override default and ENV values.
## Uncomment and change examples:

#Add your source directories here separated by space
MODULES = app

## ESP_HOME sets the path where ESP tools and SDK are located.
## Windows:
# ESP_HOME = c:/Espressif

## MacOS / Linux:
ESP_HOME = /esptools/esp-open-sdk-1.3

## SMING_HOME sets the path where Sming framework is located.
## Windows:
# SMING_HOME = c:/tools/sming/Sming 

# MacOS / Linux
SMING_HOME = /esptools/Sming/Sming

## COM port parameter is reqruied to flash firmware correctly.
## Windows: 
# COM_PORT = COM3

# MacOS / Linux:
COM_PORT = /dev/tty.SLAB_USBtoUART

# Com port speed
COM_SPEED	= 115200
ESPTOOL2 = /esptools/esp8266/esptool2/esptool2

DISABLE_SPIFFS=1
SPI_SIZE=1M
RBOOT_ENABLED=1
RBOOT_TWO_ROMS  = 1
RBOOT_ROM_0     = rom0
## input linker file for first rom
RBOOT_LD_0      = rom0.ld
## these next options only needed when using two rom mode
RBOOT_ROM_1     = rom1
TERMINAL = true
KILL_TERM= true

.PHONY: copy
copy:
	scp out/firmware/rom*.bin kevin@raspberrypi2:/var/www