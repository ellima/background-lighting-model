#ifndef BACKGROUNDLIGHT_H
#define BACKGROUNDLIGHT_H

#ifndef Arduino_h
#include <Arduino.h>
#endif

#ifndef ADAFRUIT_NEOPIXEL_H
#include <Adafruit_NeoPixel.h>
#endif

#define DATA_TRANSMISSION NEO_GRB + NEO_KHZ800

#define DEFAULT_MINUTES 12
#define DEFAULT_LED_COUNT 8


class Background{

  private:
    const uint16_t background_index = 0;
    const uint16_t next_index = 0;

    const bool new_used = false;

    struct Fix_Color{
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };

    const Fix_Color sky_blue = {0x87, 0xCE, 0xEB};
    const Fix_Color midnight_blue = {0x19, 0x19, 0x70};
    const Fix_Color morning_red = {0xFF, 0x58, 0x0};
    const Fix_Color noon = {0xFF, 0xFF, 0xFF};

    Adafruit_NeoPixel * strip = nullptr;

    float periodic = 0;
    int periodic_time = 0;

    int init_phase = 0;

    uint8_t sigma = 1;

    uint8_t * red_state = nullptr;
    uint8_t * green_state = nullptr;
    uint8_t * blue_state = nullptr;

    unsigned long t_ref = 0;
    unsigned long * t_ref_red = nullptr;
    unsigned long * t_ref_green = nullptr;
    unsigned long * t_ref_blue = nullptr;

    long ref_time = 0;

    long current_time = 0;
    long sunrise = 0;
    long sunset = 0;

    #ifndef ESP_H
    long time(long * t){
        *t = (millis()) / 1000; return *t;
    }
    #endif

    inline const float gauss(int i, float m)
    {
        return exp(-0.5 * pow( ( ((float)i - m) / sigma ), 2.));
    }

    inline const float fmap(float val, float in_min, float in_max, float out_min, float out_max)
    {
        return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    float intensity_factor(int i);
    float intensity_factor(int i, long time);

    uint32_t eclipse(int index, uint8_t red, uint8_t green, uint8_t blue);
    uint8_t eclipse(int index, uint8_t colour, uint8_t morning, uint8_t noon, long time);

    void time_management();

  public:
    long const getCurrentTime(){ return current_time; };
    long const getSunrise(){ return sunrise; };
    long const getSunset(){ return sunset; };
    int const getPeriodicTime() { return periodic_time; };

    void background_sky();

    Background(uint16_t pin, uint16_t to = DEFAULT_LED_COUNT, int minutes = DEFAULT_MINUTES, uint16_t from = 0, uint8_t brightness = 255);
    Background(uint16_t pin, long rise, long set, uint16_t to = DEFAULT_LED_COUNT, uint16_t from = 0, uint8_t brightness = 255);
    Background(Adafruit_NeoPixel &neo, int minutes = DEFAULT_MINUTES, uint16_t from = 0, uint16_t to = 0);
    Background(Adafruit_NeoPixel &neo, long rise, long set, uint16_t from = 0, uint16_t to = 0);
    ~Background();
};

#endif // BACKGROUNDLIGHT_H
