If you have trouble with basic ESP8266 module with 512K Flash you can flash this firmware

Taken from here: http://bbs.espressif.com/viewtopic.php?t=400&start=10


Is somewhat stable and fast, more than my ESP8266-01 default one (that I broke trying to upgrade).


To flash it you must connect in flash mode and use:

https://github.com/themadinventor/esptool -- Python script, Linux compatible:
	esptool.py  -p /dev/ttyACM0 -b 19200 write_flash 0x00000 firmware1.0.1.bin

http://www.electrodragon.com/w/ESP8266_firmware_flasher
	Use flasher tool and flash bin file to 0x00000 address 
