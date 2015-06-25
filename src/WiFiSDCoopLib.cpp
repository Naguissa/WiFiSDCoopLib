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
// To prevent double declaration on DEFINE-depending functions in .h
#ifndef __WiFiSDCoopLib_C__
	#define __WiFiSDCoopLib_C__
#endif

#include <Arduino.h>
#include "WiFiSDCoopLib.h"



WiFiSDCoopLib::WiFiSDCoopLib() {
	_init();
	setSSID("default");
	setPass("");
	_fileBuffer[0] = '\0';
	_fileBufferPos = 0;
}


String WiFiSDCoopLib::getIPInfo() {
	return _send(F("AT+CIFSR"), 150); // get ip address
}

void WiFiSDCoopLib::setMode(const char m) {
	mode = m;
}

void WiFiSDCoopLib::setSSID(const String s) {
	if(ssid != NULL) {
		free(ssid);
	}
	ssid = (char *) malloc((s.length() + 1) * sizeof(char));
	s.toCharArray(ssid, s.length());
	ssid[s.length()] = '\0';
}

void WiFiSDCoopLib::setSSID(const char s[]) {
	if(ssid != NULL) {
		free(ssid);
	}
	ssid = (char *) malloc((strlen(s) + 1) * sizeof(char));
	strcpy(ssid, s);
	ssid[strlen(s)] = '\0';
}

void WiFiSDCoopLib::setPass(const String s) {
	if(pass != NULL) {
		free(pass);
	}
	pass = (char *) malloc((s.length() + 1) * sizeof(char));
	s.toCharArray(pass, s.length());
	pass[s.length()] = '\0';
}

void WiFiSDCoopLib::setPass(const char s[]) {
	if(pass != NULL) {
		free(pass);
	}
	pass = (char *) malloc((strlen(s) + 1) * sizeof(char));
	strcpy(pass, s);
	pass[strlen(s)] = '\0';
}

// Used for setting-up Wifi Module to desired speed.
// Remember to change Arduino sketch speed when changing to adapt to new one.
void WiFiSDCoopLib::setBaudRate(const String br) {
	_sendPart(F("AT+CIOBAUD="));
	_send(br, 200);
}



void WiFiSDCoopLib::_checkESPAvailableData(const int timeout, String * response, const byte responseType) {
	char IPDSteps = 0, c;
	unsigned char ipd;
	long int time;
	char  endResponse[10];
	byte endFlag = 0;
	byte endLength;
	byte endPos = 0;
	String route = "";
	switch (responseType) {
		case WiFiSDCoopLib_RESPONSE_GENERIC:
			strcpy(endResponse, "OK\r\n");
			endLength = 4;
			break;

		case WiFiSDCoopLib_RESPONSE_DATA:
			strcpy(endResponse, "SEND OK\r\n");
			endLength = 9;
			break;

		case WiFiSDCoopLib_RESPONSE_CIPSEND:
			strcpy(endResponse, "> ");
			endLength = 2;
			break;

		case WiFiSDCoopLib_RESPONSE_RESET:
			strcpy(endResponse, "ready\r\n");
			endLength = 7;
			break;
		
		case WiFiSDCoopLib_RESPONSE_NO:
		default:
			endLength = 0;
			break;
	}

	time = millis() + timeout;
	while (time > millis()) {
		while (_dev_available()) {

			// READING - IPD checks
			c = _dev_read(); // read the next character.
			if (IPDSteps == 0 && c != '+' && c != '\n' && c != '\r') {
				IPDSteps = 10;
			} 
			if (IPDSteps == 10 && (c == '\n' || c == '\r')) {
				IPDSteps = 0;
			}
			if (response != NULL) {
				response->concat(c);
			}

			// Check if new IPD
			switch (IPDSteps) {
				case 0:
					if (c == '+') {
						IPDSteps++;
					}
					break;

				case 1:
					if (c == 'I') {
						IPDSteps++;
					} else {
						IPDSteps = 0;
					}
					break;

				case 2:
					if (c == 'P') {
						IPDSteps++;
					} else {
						IPDSteps = 0;
					}
					break;

				case 3:
					if (c == 'D') {
						IPDSteps++;
					} else {
						IPDSteps = 0;
					}
					break;

				case 4:
					if (c == ',') {
						IPDSteps++;
						ipd = 0;
					} else {
						IPDSteps = 0;
					}
					break;

				case 5: // Reading IPD channel
					if (c == ',') {
						IPDSteps++;
					} else {
						ipd = ipd * 10 + c - 48;
					}
					break;

				case 6: // Length, ignored
					if (c == ':') {
						IPDSteps++;
					}
					break;

				case 7: // GET, post, etc
					if (c == ' ') {
						IPDSteps++;
					}
					break;

				case 8: // Route
					if (c == ' ') {
						IPDSteps++;
					} else {
						route += c;
					}
					break;
		
				case 9: // Ignore, request completed
				case 10: // Ignore, request completed
				default:
					break;
			}

			// Logic: If end string is reached endFlag is set. Then, decreases to wait some cycles, that should be without data. If data comes means that end is not valid (probably a string identical to end, but no the end itself.
			// Once arrives 1, and endPos remains 0 (no more data has come) it's a valid end.
			if(endLength > 0) {
				if (endPos == endLength - 1 && c == endResponse[endPos]) {
					endPos = 0;
					endFlag = 10;
				} else if (endPos < endLength - 1) {
					if (c == endResponse[endPos]) {
						endPos++;
					} else {
						endPos = 0;
						endFlag = 0;
					}
				} else {
					endPos = 0;
					endFlag = 0;
				}
				// if endPos becomes 0, check again to don't miss a end string start.
				if (endPos == 0 && c == endResponse[endPos]) {
					endPos++;
				}
			} // End request end checks


		}
		if (endFlag > 1) {
			endFlag--;
		}
		if (endLength > 0 && endFlag == 1 && endPos == 0) { // valid END
			return;
		}

		if (IPDSteps == 9) { // Request fond, check routes
			IPDStruct * last = IPDs;
			bool found = false;
			while (last != NULL) {
				if (last->mode == 4) {// 4 Default route
					found = true;
				} else if (strlen(last->route) <= route.length()) {
					switch (last->mode) {
						case 3:// 3 found in any position
							found = route.indexOf(last->route) >= 0;
							break;

						case 2:// 2 ends with
							found = route.endsWith(last->route);
							break;

						case 1:// 1 starts with
							found = route.startsWith(last->route);
							break;

						case 0:// 0 same string
						default:
							found = route.equals(last->route);
							break;
					}
				}
				if (found) {
					last->fp(route, ipd);
					_sendCloseIPD(ipd);
					return;
				}
				last = (IPDStruct *) last->next;
			}
			sendDataByIPD(ipd, F("404 - Not found"));
			_sendCloseIPD(ipd);
		} // IPD found, routes checks end	

	} // End timeout while
}


void WiFiSDCoopLib::wifiLoop() {
    if(_dev_available()) {
		_checkESPAvailableData(500); // Request fond, check routes
	}

	if (_waitAfterIPDTimer < millis()) {
		// Work queue processing
		// Check if any send command is in list:
		bool freeIPDs[WiFiSDCoopLib_COOP_SD_MAX_IPDS];
		for (unsigned char tmp = 0; tmp < WiFiSDCoopLib_COOP_SD_MAX_IPDS; tmp++) {
			freeIPDs[tmp] = true;
		}
		WorkItemStruct * queueItem = WorkQueue;
		while (queueItem != NULL) {
			if (freeIPDs[queueItem->ipd]) {
				switch (queueItem->mode) {
					case 3: // close IPD
						_sendPart("AT+CIPCLOSE=");
						char cc[3];
						itocp(cc, (int) queueItem->ipd);
						_send(cc, queueItem->timeout);
						_removeWorkQueueItem(queueItem);
						_waitAfterIPDTimer = millis() + WiFiSDCoopLib_TYPE_CLOSEIPD_DELAY;
						// Prevent any sending on this timeout
						for (unsigned char tmp = 0; tmp < WiFiSDCoopLib_COOP_SD_MAX_IPDS; tmp++) {
							freeIPDs[tmp] = false;
						}
						break;

					case 2: // command
						_send(queueItem->str, queueItem->timeout);
						_removeWorkQueueItem(queueItem);
						break;

					case 1: // File
						if (_actualFileSendRegiter == NULL) { // No active file transaction now
							_startFileTransaction(queueItem);
						}
						break;

					case 0 : // String
					default:
						_sendDataByIPD(queueItem->ipd, queueItem->str, queueItem->timeout);
						_removeWorkQueueItem(queueItem);
						break;
				}
				freeIPDs[queueItem->ipd] = false;
			}
			queueItem = (WorkItemStruct *) queueItem->next;
		}
	}
	// File sending processing:
	if (_actualFileSendRegiter != NULL) {
		_fileLoop();
	}
}


void WiFiSDCoopLib::itocp(char *str, int n) {
	unsigned int i = 10;
	byte pos = 1;
	char ctmp;
	if (n >= 0) {
		str[0] = (n % 10)  + '0';
		str[1] = '\0';
		while (i < n) {
			str[pos] =(((int) (n / i)) % 10)  + '0';
			i *= 10;
			pos++;
		}
	} else {
		str[0] = ((-n) % 10)  + '0';
		str[1] = '\0';
		while (i < -n) {
			str[pos] =(((int) ((-n) * 10 / i)) % 10)  + '0';
			pos++;
			i *= 10;
		}
		str[pos] = '-';
		pos++;
	}
	str[pos] = '\0';
	// Do floor to position/2 - 1
	// and reverse the string
	i = (pos - (pos % 2)) / 2;
	while (i > 0) {
		i--;
		ctmp = str[i];
		str[i] = str[pos - 1 - i ];
		str[pos - 1 - i] = ctmp;
	}
}

void WiFiSDCoopLib::_clearRoutes(IPDStruct * act) {
	if (act->next != NULL) {
		_clearRoutes((IPDStruct *) act->next);
	}
	if (act->route != NULL) {
		free(act->route);
	}
	free(act);
}

void WiFiSDCoopLib::clearRoutes() {
	if (IPDs != NULL) {
		_clearRoutes((IPDStruct *) IPDs->next);
		IPDs = NULL;
	}
}


void WiFiSDCoopLib::attachRoute(const String route, void (*fp)(const String, const unsigned char), const char mode) {
	IPDStruct * last = (IPDStruct *) _attachRoute_common();
	last->route = (char *) malloc(sizeof(char) * (route.length() + 1));
	route.toCharArray(last->route, route.length());
	last->route[route.length()] = '\0';
	last->fp = fp;
	last->mode = mode;
}

void WiFiSDCoopLib::attachRoute(const char route[], void (*fp)(const String, const unsigned char), const char mode) {
	IPDStruct * last = (IPDStruct *) _attachRoute_common();
	last->route = (char *) malloc(sizeof(char) * (strlen(route) + 1));
	strcpy(last->route, route);
	last->route[strlen(route)] = '\0';
	last->fp = fp;
	last->mode = mode;
}

void * WiFiSDCoopLib::_attachRoute_common() {
	IPDStruct * last;
	if (IPDs != NULL) {
		last = IPDs;
		while (last->next != NULL) {
			last = (IPDStruct *) last->next;
		}
		last->next = (IPDStruct *) malloc(sizeof(IPDStruct));
		last = (IPDStruct *) last->next;
	} else {
		IPDs = (IPDStruct *) malloc(sizeof(IPDStruct));
		last = IPDs;
	}
	last->next = NULL;
	return last;
}



void WiFiSDCoopLib::_startFileTransaction(WorkItemStruct * item) {  
	_fileBufferPos = 0;
	_actualFile = SD.open(item->str);
	if (_actualFile) {
		_actualFileSendRegiter = item;
	} else {
		_removeWorkQueueItem(_actualFileSendRegiter);
		_actualFileSendRegiter = NULL;
		_sendDataByIPD(item->ipd, "ERROR - File not found: ");
		_sendDataByIPD(item->ipd, item->str);
		// Send 404?
	}
}

void WiFiSDCoopLib::_fileLoop() {  
	if (_actualFile) { // Active file
		bool sendBuffer = false;
		bool EoF = false;
		if (_fileBufferPos < _chunkSize) {
			if (_actualFile.available()) {
				char ch = _actualFile.read();
				_fileBuffer[_fileBufferPos] = ch;
				_fileBufferPos++;
			} else {
				sendBuffer = true;
				EoF = true;
			}
		} else {
			sendBuffer = true;
		}
		if (sendBuffer && _fileBufferPos > 0 && _waitAfterIPDTimer < millis()) {
			_fileBuffer[_fileBufferPos] = '\0';
			_sendDataByIPD(_actualFileSendRegiter->ipd, _fileBuffer, 150);
			_fileBufferPos = 0;
		}
		if (EoF && (_waitAfterIPDTimer < millis() || _fileBufferPos == 0)) { // close the file and clean register
			_actualFile.close();
			_removeWorkQueueItem(_actualFileSendRegiter);
			_actualFileSendRegiter = NULL;
		}
	}
}


void WiFiSDCoopLib::_removeWorkQueueItem(WorkItemStruct * item) {
	if (WorkQueue == item) { // Actual register is the 1st register of the queue
		if (WorkQueue->str != NULL) {
			free(WorkQueue->str);
		}
		free(WorkQueue);
		WorkQueue = (WorkItemStruct *) item->next;
		return;
	} else {
		WorkItemStruct * queueItem = WorkQueue;
		WorkItemStruct * lastQueueItem = NULL;
		while (queueItem != NULL) {
			if (queueItem == item) {
				lastQueueItem->next = queueItem->next;
				if (queueItem->str != NULL) {
					free(queueItem->str);
				}
				free(queueItem);
				return;
			}
			lastQueueItem = queueItem;
			queueItem = (WorkItemStruct *) queueItem->next;
		}
	}
}

void WiFiSDCoopLib::_cleanWorkQueueSub(WorkItemStruct * item) {
	if (item->next != NULL) {
		_cleanWorkQueueSub((WorkItemStruct *) item->next);
	}
	if (item->str != NULL) {
		free(item->str);
	}
	free(item);
}

void WiFiSDCoopLib::_cleanWorkQueue() {
	if (WorkQueue != NULL) {
		_cleanWorkQueueSub(WorkQueue);
	}
}



void * WiFiSDCoopLib::_getNewWorkQueueItem(const unsigned char ipd, char mode, const int timeout) {
		WorkItemStruct * queueItem;
	if (WorkQueue == NULL) { // Empty queue
		WorkQueue = (WorkItemStruct *) malloc(sizeof(WorkItemStruct));
		queueItem = WorkQueue;
	} else {
		queueItem = WorkQueue;
		while (queueItem->next != NULL) {
			queueItem = (WorkItemStruct *) queueItem->next;
		}
		queueItem->next = (WorkItemStruct *) malloc(sizeof(WorkItemStruct));
		queueItem = (WorkItemStruct *) queueItem->next;
	}
	queueItem->mode = mode;
	queueItem->ipd = ipd;
	queueItem->timeout = timeout;
	queueItem->next = NULL;
	queueItem->str = NULL;
	return (void *) queueItem;
}



// Data sending functions, here works as "attach work unit to queue".
void WiFiSDCoopLib::sendDataByIPD(const unsigned char ipd, const String data, const int timeout) {
	WorkItemStruct * item = (WorkItemStruct *) _getNewWorkQueueItem(ipd, WiFiSDCoopLib_TYPE_DATA, timeout);
	item->str = (char *) malloc(sizeof(char) * (data.length() + 1));
	data.toCharArray(item->str, data.length() + 1);
}

void WiFiSDCoopLib::sendDataByIPD(const unsigned char ipd, const char * data, const int timeout) {
	WorkItemStruct * item = (WorkItemStruct *) _getNewWorkQueueItem(ipd, WiFiSDCoopLib_TYPE_DATA, timeout);
	item->str = (char *) malloc(sizeof(char) * (strlen(data) + 1));
	strcpy(item->str, data);
}

void WiFiSDCoopLib::sendDataByIPD(const unsigned char ipd, const char data, const int timeout) {
	WorkItemStruct * item = (WorkItemStruct *) _getNewWorkQueueItem(ipd, WiFiSDCoopLib_TYPE_DATA, timeout);
	item->str = (char *) malloc(sizeof(char) * 2);
	item->str[0] = data;
	item->str[1] = '\0';
}

void WiFiSDCoopLib::sendDataByIPD(const unsigned char ipd, const int data, const int timeout) {
	char str[6];
	itocp(str, data);
	sendDataByIPD(ipd, str, timeout);
}

void WiFiSDCoopLib::_sendCloseIPD(const unsigned char ipd) {
	WorkItemStruct * item = (WorkItemStruct *) _getNewWorkQueueItem(ipd, WiFiSDCoopLib_TYPE_CLOSEIPD, 100);
}

void WiFiSDCoopLib::_sendCommandByIPD(const unsigned char ipd, const char * data, const int timeout) {
	WorkItemStruct * item = (WorkItemStruct *) _getNewWorkQueueItem(ipd, WiFiSDCoopLib_TYPE_COMMAND, timeout);
	item->str = (char *) malloc(sizeof(char) * (strlen(data) + 1));
	strcpy(item->str, data);
}

void WiFiSDCoopLib::_sendCommandByIPD(const unsigned char ipd, const String data, const int timeout) {
	WorkItemStruct * item = (WorkItemStruct *) _getNewWorkQueueItem(ipd, WiFiSDCoopLib_TYPE_COMMAND, timeout);
	item->str = (char *) malloc(sizeof(char) * (data.length() + 1));
	data.toCharArray(item->str, data.length());
}



// File sending functions, here works as "attach work unit to queue".
void WiFiSDCoopLib::sendFileByIPD(unsigned char ipd, const String data, const int timeout) {
	WorkItemStruct * item = (WorkItemStruct *) _getNewWorkQueueItem(ipd, WiFiSDCoopLib_TYPE_FILE, timeout);
	item->str = (char *) malloc(sizeof(char) * (data.length() + 1));
	data.toCharArray(item->str, data.length());
}

void WiFiSDCoopLib::sendFileByIPD(unsigned char ipd, const char * data, const int timeout) {
	WorkItemStruct * item =(WorkItemStruct *) _getNewWorkQueueItem(ipd, WiFiSDCoopLib_TYPE_FILE, timeout);
	item->str = (char *) malloc(sizeof(char) * (strlen(data) + 1));
	strcpy(item->str, data);
}



	// Real data sending to ESP
void WiFiSDCoopLib::_sendDataByIPD(const unsigned char ipd, const char * data, const int timeout) {
	char ipdStr[3];
	itocp(ipdStr, ipd);
	_sendPart(F("AT+CIPSEND="));
	_sendPart(ipdStr);
	_sendPart(F(","));
	_send((int) strlen(data), 30, false, WiFiSDCoopLib_RESPONSE_CIPSEND);
	_send(data, timeout, true, WiFiSDCoopLib_RESPONSE_DATA);
}

