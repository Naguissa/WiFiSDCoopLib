/**
 * Library to use ESP8266 WiFi with SD card reader using collaborative multitasking.
 *
 * Attached routes ar functions in form:
 *     void function HANDLER(const string ROUTE, const int IPD);
 *
 * Used defines, used to configure library:
 *   WiFiSDCoopLib_DEV Serial device to use. Default: Serial2 on STM32, Serial on others
 *   WiFiSDCoopLib_BAUDS Bauds of serial device. Default: 115200
 *   WiFiSDCoopLib_COOP_SD_CHUNK When SD cooperative multitasking is enabled, data chunk size in unsigned charS. Default: 64.
 * 
 * It's not formely correct that a library depends on the program, but as this is a resource-limited environment (microcontroller) I prefer to do this
 * instead including all code (lot of program space and even RAM) or creating a bunch of libraries, one for each configuration.
 * This method should never be used on a current computer program.
 * 
 * @copyright Naguissa
 * @author Naguissa
 * @email naguissa.com@gmail.com
 * @version 1.0.0
 * @created 2015-06-13
 */
#ifndef __WiFiSDCoopLib__
	#define __WiFiSDCoopLib__
	#include "Arduino.h"

	#ifndef __SD_H__
		#include "SD.h"
	#endif
	#ifndef WiFiSDCoopLib_COOP_SD_CHUNK
		#define WiFiSDCoopLib_COOP_SD_CHUNK 64
	#endif

	#ifdef _VARIANT_ARDUINO_STM32_
		#define WiFiSDCoopLib_COOP_SD_MAX_IPDS 8
	#else
		#define WiFiSDCoopLib_COOP_SD_MAX_IPDS 5
	#endif



	///// YOU NEED TO ADJUST THIS TO YOUR MODULE; try-and-error, each of my modules came at different speed
	//#define WiFiSDCoopLib_BAUDS 19200
	//#define WiFiSDCoopLib_BAUDS 9600
	#ifndef WiFiSDCoopLib_BAUDS
		#define WiFiSDCoopLib_BAUDS 115200
	#endif


	#ifndef WiFiSDCoopLib_DEV
		#ifdef _VARIANT_ARDUINO_STM32_
			#define WiFiSDCoopLib_DEV Serial2
		#else
			#define WiFiSDCoopLib_DEV Serial
		#endif
	#endif


	#define WiFiSDCoopLib_TYPE_DATA 0
	#define WiFiSDCoopLib_TYPE_FILE 1
	#define WiFiSDCoopLib_TYPE_COMMAND 2
	#define WiFiSDCoopLib_TYPE_CLOSEIPD 3

	#define WiFiSDCoopLib_RESPONSE_NO 1
	#define WiFiSDCoopLib_RESPONSE_GENERIC 2
	#define WiFiSDCoopLib_RESPONSE_CIPSEND 3
	#define WiFiSDCoopLib_RESPONSE_DATA 4
	#define WiFiSDCoopLib_RESPONSE_RESET 5

	// delay after a CIPCLOSE until new transmission, in ms;. Neded to avoid "Busy" problems 
	#define WiFiSDCoopLib_TYPE_CLOSEIPD_DELAY 500


	class WiFiSDCoopLib {
		public:
			WiFiSDCoopLib();

			void reinit();
			void setMode(const char);
			void setSSID(const String);
			void setSSID(const char []);
			void setPass(const String);
			void setPass(const char []);
			// Used for setting-up Wifi Module to desired speed.
			// Remember to change Arduino sketch speed when changing to adapt to new one.
			void setBaudRate(const String);

			String getIPInfo(); // Dangerous, don't use on cooperative mode, only on reinit or setup().

			void attachRoute(const String, void (*)(const String, const unsigned char), const char = 0);
			void attachRoute(const char[], void (*)(const String, const unsigned char), const char = 0);
			void clearRoutes();

			void sendDataByIPD(const unsigned char, const String, const int = 2000);
			void sendDataByIPD(const unsigned char, const char *, const int = 2000);
			void sendDataByIPD(const unsigned char, const char, const int = 500);
			void sendDataByIPD(const unsigned char, const int, const int = 500);

			void sendFileByIPD(const unsigned char, const String, const int = 2000);
			void sendFileByIPD(const unsigned char, const char *, const int = 2000);

			// Internal use, but public because may be useful externally
			void itocp(char *, int);

			void wifiLoop();

		private: 
			String buffer;
			char *ssid = NULL;
			char *pass = NULL;
			char mode = '2'; //1= Sta, 2= AP, 3=both. Sta is a device, AP is a router
			typedef struct {
				char * route = NULL;
				void (* fp)(const String, const unsigned char);
				char mode; // 0 same string, 1 starts with, 2 ends with, 3 found in any position
				void * next = NULL;
			} IPDStruct;
			IPDStruct * IPDs = NULL;
			void _clearRoutes(IPDStruct *);
			void * _attachRoute_common();
			typedef struct {
				char * str = NULL;
				char mode; // 0 string, 1 file, 2 command
				unsigned char ipd;
				int timeout;
				void * next = NULL;
			} WorkItemStruct;
			WorkItemStruct * WorkQueue = NULL;
			WorkItemStruct * _actualFileSendRegiter = NULL;
			File _actualFile;
			char *_fileBuffer;
			unsigned char _fileBufferPos = 0;
			byte _chunkSize = 64;

			void _init();

			char _dev_read();
			bool _dev_available();

			void _cleanWorkQueue();
			void _cleanWorkQueueSub(WorkItemStruct * item);
			void * _getNewWorkQueueItem(const unsigned char, char, const int);
			void _removeWorkQueueItem(WorkItemStruct *);

			void _startFileTransaction(WorkItemStruct *);
			void _fileLoop();

			String _send(const String, const int, const bool = false, byte = WiFiSDCoopLib_RESPONSE_GENERIC);
			String _send(const char*, const int, const bool = false, byte = WiFiSDCoopLib_RESPONSE_GENERIC);
			String _send(const int, const int, const bool = false, byte = WiFiSDCoopLib_RESPONSE_GENERIC);
			String _send(const char, const int, const bool = false, byte = WiFiSDCoopLib_RESPONSE_GENERIC);
			String _send(const unsigned char, const int, const bool = false, byte = WiFiSDCoopLib_RESPONSE_GENERIC);
			String _send_common(const int, const bool, byte = WiFiSDCoopLib_RESPONSE_GENERIC);

			#define _sendPart(s) _send(s, 0, true, WiFiSDCoopLib_RESPONSE_NO)
			#define _getResponse(timeout, type) _send_common(timeout, true, type);

			void _sendDataByIPD(const unsigned char, const char*, const int = 2000);
			void _sendDataByIPD_common(const unsigned char);

			void _sendCommandByIPD(const unsigned char, const char*, const int = 500);
			void _sendCommandByIPD(const unsigned char, const String, const int = 500);
			void _sendCloseIPD(const unsigned char);

			unsigned long int _waitAfterIPDTimer = 0;


			void _checkESPAvailableData(const int, String * = NULL, const byte response = WiFiSDCoopLib_RESPONSE_NO);
	};

	// THESE FUNCTIONS ARE DEFINED HERE TO BE ABLE TO USE DEFINITIONS ON MAIN PROGRAM
	#ifndef __WiFiSDCoopLib_C__

		void WiFiSDCoopLib::_init() {
			_chunkSize = WiFiSDCoopLib_COOP_SD_CHUNK;
			_fileBuffer = (char *) malloc(sizeof(char) * (WiFiSDCoopLib_COOP_SD_CHUNK + 1));
		}

		char WiFiSDCoopLib::_dev_read() {
			return WiFiSDCoopLib_DEV.read();
		}

		bool WiFiSDCoopLib::_dev_available() {
			return WiFiSDCoopLib_DEV.available();
		}


		void WiFiSDCoopLib::reinit() {
			_cleanWorkQueue();
		  	WiFiSDCoopLib_DEV.begin(WiFiSDCoopLib_BAUDS);
			_send(F("AT+RST"), 1500, false, WiFiSDCoopLib_RESPONSE_RESET); // RST produces an "OK" that returns from command _send but still has to reset.
			_sendPart(F("AT+CWMODE="));
			_send(String(mode), 300);
			if (mode != '1') { // Configure AP
				_sendPart(F("AT+CWSAP=\""));
				_sendPart(ssid);
				_sendPart(F("\",\""));
				_sendPart(pass);
				_send(F("\",1,0"), 1500); // Last param, ench: 0 OPEN; 2 WPA_PSK; 3 WPA2_PSK; 4 WPA_WPA2_PSK 
			}
			if (mode != '2') { // Configure STA
				for(char i = 0; i < 5; i++) {
					_sendPart(F("AT+CWJAP=\""));
					_sendPart(ssid);
					_sendPart(F("\",\""));
					_sendPart(pass);
					String res = _send(F("\""), 10000);
					if(res.indexOf("OK") >= 0) {
						break;
					}
				}
			}
			_send(F("AT+CIPMUX=1"), 400); // configure for multiple connections
			_send(F("AT+CIPSERVER=1,80"), 500); // turn on server on port 80
		}

		String WiFiSDCoopLib::_send(const String command, const int timeout, const bool removeNL, const byte type) {
			while (_dev_available()) _checkESPAvailableData(50);
			WiFiSDCoopLib_DEV.print(command);
			return _send_common(timeout, removeNL, type);
		}

		String WiFiSDCoopLib::_send(const char * command, const int timeout, const bool removeNL, const byte type) {
			while (_dev_available()) _checkESPAvailableData(50);
			WiFiSDCoopLib_DEV.print(command);
			return _send_common(timeout, removeNL, type);
		}

		String WiFiSDCoopLib::_send(const char command, const int timeout, const bool removeNL, const byte type) {
			while (_dev_available()) _checkESPAvailableData(50);
			WiFiSDCoopLib_DEV.print(command);
			return _send_common(timeout, removeNL, type);
		}

		String WiFiSDCoopLib::_send(const int command, const int timeout, const bool removeNL, const byte type) {
			while (_dev_available()) _checkESPAvailableData(50);
			WiFiSDCoopLib_DEV.print(command);
			return _send_common(timeout, removeNL, type);
		}

		String WiFiSDCoopLib::_send(const unsigned char command, const int timeout, const bool removeNL, const byte type) {
			char str[4];
			itocp(str, command);
			while (_dev_available()) _checkESPAvailableData(50);
			WiFiSDCoopLib_DEV.print(str);
			return _send_common(timeout, removeNL, type);
		}

		String WiFiSDCoopLib::_send_common(const int timeout, const bool removeNL, const byte type) {
			String response = "";
			if (!removeNL) {
				WiFiSDCoopLib_DEV.print(F("\r\n"));
			}
			if (type != WiFiSDCoopLib_RESPONSE_NO && timeout > 0) {
				_checkESPAvailableData(timeout, &response, type); 
			}
			return response;
		}

	#endif
#endif
