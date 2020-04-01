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
