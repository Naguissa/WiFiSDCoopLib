# Arduino and STM32 WiFiSDCoopLib


## What is this repository for? ##

Library to use ESP8266 WiFi with SD card reader using collaborative multitasking.



## How do I get set up? ##

 * Get the ZIP.
 * Install library on Arduino

 * Optionally you can edit WiFiLib.h to change default values for essid, password or mode (not needed, you can change them using setters).


You can change device Serial and Baud rate using before #include:
 * WiFiSDCoopLib_DEV Serial device to use. Default: Serial2 on STM32, Serial on others
 * WiFiSDCoopLib_BAUDS Bauds of serial device. Default: 115200
 * WiFiSDCoopLib_COOP_SD_CHUNK When SD cooperative multitasking is enabled, data chunk size in unsigned charS. Default: 64.


## Important ##

Remember you cannot transfer binary files from Arduino to ESP, so you can't transfer images.

HINT: You can use SVG, even with embedded images; at least in Inkscape they are embedded as base64 encoded images.
HINT: You can use data embedding: data:[<mediatype>][;base64],<data> -- as:

<img src="data:image/gif;base64,R0lGODlhEAAOALMAAOazToeHh0tLS/7LZv/0jvb29t/f3//Ub//ge8WSLf/rhf/3kdbW1mxsbP//mf///yH5BAAAAAAALAAAAAAQAA4AAARe8L1Ekyky67QZ1hLnjM5UUde0ECwLJoExKcppV0aCcGCmTIHEIUEqjgaORCMxIC6e0CcguWw6aFjsVMkkIr7g77ZKPJjPZqIyd7sJAgVGoEGv2xsBxqNgYPj/gAwXEQA7" width="16" height="14" alt="embedded folder icon">


## Example ##

Included on example folder, available on Arduino IDE.

Extra data files to copy on SD for the exmple available on etras/ folder; not needed (library can operate without SD).

If unmodified, example creates an open WiFi network with essid "default"



## Who do I talk to? ##

 * [Naguissa](https://github.com/Naguissa)
 * http://www.naguissa.com
