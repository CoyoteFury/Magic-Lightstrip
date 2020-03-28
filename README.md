# Magic Lightstrip

<h2>DESCRIPTION</h2>
Magic Lightsript is an easy and friendly User Interface to drive your ledstrip from ESP32.
It is based on FastLED and WifiManager library. The source file is for ESP32 and APA102 led strip.
<br /><br />
First time, the ESP32 is in AccessPoint mode, then after you enter your Wifi credentials it will autoconnect to your network.

<h2>PRE-REQUISITE</h2>
You must install the SPIFFS module to have hability to upload files on your ESP32 (data folder)
See 'ESP32 Sketch Data Upload', an exemple here : https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/

<h2>WIRING</h2>
ESP32        | APA102 | POWER SUPPLY (DC 5V - 1A)<br />
VIN (5V) --> VIN ---> +<br />
GND -------> GND ---> -<br />
GPIO22 ----> CLOCK<br />
GPIO23 ----> DATA<br />
<br />
<b>Note:</b> Power Supply correspond to the number of your leds, be carreful!!<br />
People add components (Resistor and NPN transistor) to protect ESP32. I don't but it's a better wiring.

<h2>INSTALLATION</h2>
- Just copy files in your Documents/Arduino projets folder<br />
- Download iro.min.js put it in your data folder then upload it to your ESP32<br />
- Download any icon called "Magic-icon.png"<br />
- Modify source to the number of your led, Data and Clock Gpio and change your wifi password<br />
<br />
<br />
<b>Note:</b> if you don't want to upload files and you ESP32 has internet you can replace these lines
'<script src="iro.min.js">' by '<script src="https://cdn.jsdelivr.net/npm/@jaames/iro@5">' and '<link rel="icon" type="image/png" href="/Magic-icon.png">' by any picture of your choice.

<h2>THANKS</h2>
-Arduino | https://www.arduino.cc<br />
-Daniel Garcia and Mark Kriegsman and FastLED Community | https://fastled.io/<br />
-Tsapu for WifiManager | https://github.com/tzapu/WiFiManager<br />
-James Daniel for his Color wheel widget for JavaScript | https://iro.js.org/<br />
-Philips for Hue inspiration | https://www.meethue.com<br />
