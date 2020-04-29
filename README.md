# Library for daylight spectrum cycle background lighting w/ WS2812b LEDs

This C++ library uses the [Adafruit_NeoPixel library](https://github.com/adafruit/Adafruit_NeoPixel) to provide an easy-to-use method for background lighting installations of WS2812b-LEDs, controlled by a microcontroller like an ESP32, ESP8266 or Arduino. A use case might be an LED strip around a model-railway or something similar that needs a daylight spectrum cycle backround lighting.

It mimics the light of the sky from a bright blue around noon to dark blue around midnight during a course of the day. The time period can be chosen arbitrarily. Twilight in the morning and eveining are also built in.

## Prerequisites

This project was tested with an ESP8266 and the Arduino Uno board so far and should also run on an ESP32. **The Arduino Uno only has 2 kB in RAM. Each additional pixel on the strip takes up ~18 byte so be careful controlling large numbers of LEDs with the UNO.**

Using an ESP board is recommended.

* ESP32, ESP8266 or Arduino board
* The ArduinoIDE. Download and install [here](https://www.arduino.cc/en/Main/Software)
* If you don't use an Arduino, add your board to the ArduinoIDE
    * [ESP32](https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md)
    * [ESP8266](https://github.com/esp8266/Arduino#installing-with-boards-manager)
* Install [Adafruit_NeoPixel library](https://github.com/adafruit/Adafruit_NeoPixel) according to their installation instructions on their [GitHub repo](https://github.com/adafruit/Adafruit_NeoPixel). Additional information about this library can be found [here](https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-use).

## Installation

1. Download this Repo as a `.zip` file (use the 'Clone or download' button -> 'Download ZIP')
2. Open ArduinoIDE
3. Navigate to Sketch -> Include Library -> Add .ZIP Library...
4. Select the previously downloaded `.zip` file and hit 'OK'
5. Restart ArduinoIDE

## Usage Example

Open a new sketch in the ArduinoIDE and paste the following snippet. **Make sure to set the definitions according to your setup!**

```C++
#include <BackgroundLight.h>

#define LED_PIN 0 // sets the output pin for the data to the strip. See the pinout for your microcontroller
#define LED_COUNT 8 // sets the total number of pixels in your setup
#define MINUTES 5 // sets the real-time in minutes from sunrise to sunset

Background * background = nullptr;

void setup() {
    // initialize Background object
    background = new Background(LED_PIN, LED_COUNT, MINUTES); // Parameters are the quantities defined above
}

void loop() {
    background->background_sky();
}
```

This is the simplest usage example. The library allows for more options to work with the background function to meet additional demands for a project.

Further examples are work in progress.

## Licence

This work is published under MIT licence. See `LICENCE` for further details.
