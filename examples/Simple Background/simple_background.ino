#include <BackgroundLight.h>

#define LED_PIN 0 // sets the output pin for the data to the strip. See the pinout for your microcontroller
#define LED_COUNT 8 // sets the total number of pixels in your setup
#define MINUTES 5 // sets the real-time in minutes from sunrise to sunset
#define OPPOSITE_START false // switch to 'true' on order to reverse the "sun's" direction

Background * background = nullptr;

void setup() {
    // initialize Background object
    background = new Background(LED_PIN, OPPOSITE_START, LED_COUNT, MINUTES); // the parameters are the quantities defined above
}

void loop() {
    background->background_sky();
}
