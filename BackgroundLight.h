#ifndef BACKGROUND_H
#define BACKGROUND_H

#ifndef Arduino_h
#include <Arduino.h>
#endif

#ifndef ADAFRUIT_NEOPIXEL_H
#include <Adafruit_NeoPixel.h>
#endif

#ifndef _TIME_H
#include <time.h>
#endif

#ifndef _MATH_H
#include <math.h>
#endif

#define SPREADWIDTH 1
#define DATA_TRANSMISSION NEO_GRB + NEO_KHZ800

#define DEFAULT_MINUTES 5
#define DEFAULT_LED_COUNT 8

#endif


class Background{

  private:
    const uint16_t background_index = 0;
    const uint16_t next_index = 0;

    bool new_used = false;

    struct Fix_Color{
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };

    const Fix_Color sky_blue = {0x87, 0xCE, 0xEB};
    const Fix_Color midnight_blue = {0x19 - (0x19 / 0x1a), 0x19 - (0x19 / 0x1a), 0x70 - (0x70 / 0x71)};
    const Fix_Color morning_red = {0xFF, 0x58, 0x0};
    const Fix_Color noon = {0xFF, 0xFF, 0xFF};

    Adafruit_NeoPixel * strip = nullptr;

    float periodic = 0;
    int periodic_time = 0;

    int init_phase = 0;

    uint8_t red_state = 0;
    uint8_t green_state = 0;
    uint8_t blue_state = 0;

    uint8_t temp_red = 0;
    uint8_t temp_green = 0;
    uint8_t temp_blue = 0;

    unsigned long t_ref = 0;
    unsigned long t_ref_red = 0;
    unsigned long t_ref_green = 0;
    unsigned long t_ref_blue = 0;

    time_t ref_time = 0;

    time_t current_time = 0;
    time_t sunrise = 0;
    time_t sunset = 0;

    inline const float gauss(int i, float m)
    {
        return exp(-0.5 * pow( ( ((float)i - m) / SPREADWIDTH ), 2));
    }

    inline const float fmap(float val, float in_min, float in_max, float out_min, float out_max)
    {
        return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    float intensity_factor(int i);

    uint32_t eclipse(int index, uint8_t red, uint8_t green, uint8_t blue);

    void time_management();

  public:
    time_t const getCurrentTime(){ return current_time; };
    time_t const getSunrise(){ return sunrise; };
    time_t const getSunset(){ return sunset; };

    void background_sky();

    Background(uint16_t pin, uint16_t to = DEFAULT_LED_COUNT, int minutes = DEFAULT_MINUTES, uint16_t from = 0, uint8_t brightness = 255);
    Background(uint16_t pin, time_t rise, time_t set, uint16_t to = DEFAULT_LED_COUNT, uint16_t from = 0, uint8_t brightness = 255);
    Background(Adafruit_NeoPixel &neo, int minutes = DEFAULT_MINUTES, uint16_t start_index = 0, uint16_t to = 0);
    Background(Adafruit_NeoPixel &neo, time_t rise, time_t set, uint16_t start_index = 0, uint16_t to = 0);
    ~Background();
};
