# IR-Intervalometer
DIY Nikon intervalometer based on NODEMCU and a cheap IR blaster. Tested on D3400. Should work on all Nikon cameras (untested), although the mount for the LED then will be different and might have to be improvised.

This project was born out of frustration - Nikon's D3400 does not have a port for an intervalometer, and is therefore incompatible with most solutions available on the market. This intervalometer allows for shooting multiple images to create a timelapse and is thought primarily as a tool for amateur astrophotographers.
Note that the intervalometer currently only supports time intervals of a second or more in between shots!

![IMG_5361](https://user-images.githubusercontent.com/21038842/143140376-850f8ab7-52cc-430c-b58e-d0ecd77182b7.jpg)
![IMG_5362](https://user-images.githubusercontent.com/21038842/143140336-2f3ae510-6921-4e4f-9d9b-f6b1476ed0e1.jpg)

# Needed materials
Here is a list of materials you'll need:
- 1 NODEMCU board
- 1 IR LED (I obtained mine from an old laser printer - they are used as optical switches)
- 1 suitable resistor
- Some wires
- Access to a 3D printer

# How to build?
The build is fairly straightforward. Start by 3D printing the .stl files found here:

https://www.thingiverse.com/thing:5145118

Solder two wires to the IR LED and add a resistor. Solder the two wires to PIN 13 and GND on the NODEMCU (pay attention to polarity), use a connector if you want to remove the wire when storing away. Make sure that the wires are long enough that the LED can be placed in front of the IR receiver on your camera.

Place the board in the case and secure closed with four screws. 

# Flash the board
You will need the .ino and .h files from this repository. Make sure to have the following libraries:
- ESP8266 WiFi: https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi
- ESPAsyncTCP: https://github.com/me-no-dev/ESPAsyncTCP
- ESPAsyncWebServer: https://github.com/me-no-dev/ESPAsyncWebServer
- Nikon remote: https://github.com/outofjungle/NikonRemote

Choose the WiFi network SSID and password by changing the appropriate variables in "AP_Wifi.ino".

Complile and load "AP_Wifi.ino" to your NODEMCU. Make sure to note down the IP address that is sent to the serial port when the board is powered up - this will be needed for connecting to your intervalometer!

# How to use it

This should be pretty straightforward. Mount the intervalometer to the flash mount of your camera (or otherwise secure it to the camera) and place the IR LED in its mount in front of the sensor. If you are using this intervalometer for a camera other than a D3400, you might have to find an alternative way to place the IR LED in front of the receiver. Power up the intervalometer through the MicroUSB port of the NODEMCU.

On your camera, choose an appropriate shooting mode (that supports shooting with a remote). If you are going to shoot using the intervalometer to time the exposure select "Bulb" as exposure speed on your camera (you'll need to be in M/A/S mode on a Nikon). If you DON'T want to use the intervalometer to select the shot speed (for example for a timelapse), select an appropriate shot speed (or don't and let the camera decide from shot to shot). 

Connect to your NODEMCU WiFi network and open your browser. Navigate to the IP address you previously wrote down.

You can change three parameters:
- exposure length,
- delay between shots,
- number of shots.

The first two parameters can be incremented in 0.5s steps for a minimum of 1s. This was a design choice, since this intervalometer will be primarly used in astrophotography. If you want to shoot timelapses notice that the intervalometer sends two signals - one to initiate a shot and one to end it. Therefore, if you want to shoot in intervals of 30s you'll need to select 30s for both aperture and delay settings. Notice also that in this case you effectively only need to select half the number of shots you need.

Once you are done selecting, click "shoot".

There are also options to shoot a single picture (using your phone as a remote), which can be useful for taking group pictures and such.

It might happen that your phone loses connection to the WiFi network. Since the time shown on the screen is only syncronized with the intervalometer once per shot, if the phone/computer loses connection it might lose track of the actual shot the camera is taking, and the information displayed might be off. To fix this, reconnect to the WiFi network and press "Sync". This asks the intervalometer to send information about the current shot number and exposure and delay information. NOTE: this requires some processing time from the intervalometer. It is untested whether this might create problems with the timing of the shots. To minimize the problem, try to syncronize right after you hear the camera click. 

# TO-DO
- Syncronize clock with your computer/phone - to make sure that the internal clock of the NODEMCU is reliable
- New "Timelapse" mode, where only the delay is selected and the exposure is left for the camera to regulate
- Better code in general

# About the web interface
The web interface is an HTML page built on a combination of Bootstrap (https://getbootstrap.com/), JQuery (https://jquery.com/) and BootstrapInputSpinner (https://github.com/shaack/bootstrap-input-spinner). The page, together with cached minified versions of the resources mentioned, are stored in the ".h" file.
