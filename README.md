# Day- and Nightlight Background Library for WS2812b LED Strips

This C++ library uses the [Adafruit_NeoPixel library](https://github.com/adafruit/Adafruit_NeoPixel) to provide an easy-to-use method for background lighting installations of WS2812b-LEDs, controlled by a microcontroller like an ESP32, ESP8266 or Arduino. A use case might be an LED strip around a model-railway or something similar that needs a day-like backround lighting.

It mimics the light of the sky from a bright blue around noon to dark blue around midnight during a day cycle. The time period can be set arbitrarily. Twilight in the morning and eveining are also built in.

## Prerequisites

This project was tested with an ESP8266 so far, but should run on the most common devices which can be programmed using the [ArduinoIDE](https://www.arduino.cc/en/Main/Software).

* The ArduinoIDE. Download and install [here](https://www.arduino.cc/en/Main/Software)
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

This is the simplest usage example. The library allows more options to work with the background function to meet additional demands for a project.

Further examples are work in progress.

## Licence

This work is published under MIT licence. See `LICENCE` for further details.
