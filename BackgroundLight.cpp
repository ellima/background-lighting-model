#include "BackgroundLight.h"


float Background::intensity_factor(int i){
    float sun_offset = ( (float)(sunset - sunrise) / (float)(next_index - background_index) ) * (SPREADWIDTH + 2);
    if (current_time >= sunrise - sun_offset && current_time <= sunset + sun_offset){
        float peak = fmap(current_time, sunrise - sun_offset, sunset + sun_offset, background_index - (SPREADWIDTH + 2), next_index + (SPREADWIDTH + 1));
        return gauss(i, peak);
    }
    else {
        return 0;
    }
}

uint32_t Background::eclipse(int index, uint8_t red, uint8_t green, uint8_t blue){

    uint8_t target_red = morning_red.red;
    uint8_t target_green = morning_red.green;
    uint8_t target_blue = morning_red.blue;

    float sun_height = sin(periodic * (current_time - init_phase));

    if (sun_height >= 0){
        target_red = fmap(sun_height, 0, 1, morning_red.red, noon.red);
        target_green = fmap(sun_height, 0, 1, morning_red.green, noon.green);
        target_blue = fmap(sun_height, 0, 1, morning_red.blue, noon.blue);
    }

    float intensity = intensity_factor(index);

    uint8_t new_red = fmap(intensity, 0, 1, red, target_red);
    uint8_t new_green = fmap(intensity, 0, 1, green, target_green);
    uint8_t new_blue = fmap(intensity, 0, 1, blue, target_blue);

    return strip->Color(new_red, new_green, new_blue);
}

void Background::time_management(){
    if (current_time > sunset + ( ( (2 * periodic_time) - (sunset - sunrise) ) / 2)){
        sunrise = sunset + periodic_time;
        sunset = sunrise + periodic_time;
    }
}

void Background::background_sky(){
    time(&current_time);

    float sun_height = sin(periodic * (current_time - init_phase));
    float sun_height_next = sin(periodic * (current_time + 1 - init_phase));

    uint8_t target_red = fmap(sun_height, -1, 1, midnight_blue.red, sky_blue.red);;
    uint8_t target_green = fmap(sun_height, -1, 1, midnight_blue.green, sky_blue.green);
    uint8_t target_blue = fmap(sun_height, -1, 1, midnight_blue.blue, sky_blue.blue);

    uint8_t target_red_next = fmap(sun_height_next, -1, 1, midnight_blue.red, sky_blue.red);
    uint8_t target_green_next = fmap(sun_height_next, -1, 1, midnight_blue.green, sky_blue.green);
    uint8_t target_blue_next = fmap(sun_height_next, -1, 1, midnight_blue.blue, sky_blue.blue);

    int diff_red = target_red_next - target_red >=0 ? target_red_next - target_red : target_red - target_red_next;
    int diff_green = target_green_next - target_green >= 0 ? target_green_next - target_green : target_green - target_green_next;
    int diff_blue = target_blue_next - target_blue >= 0 ? target_blue_next - target_blue : target_blue - target_blue_next;

    diff_red = diff_red == 0 ? 1000 : diff_red;
    diff_green = diff_green == 0 ? 1000 : diff_green;
    diff_blue = diff_blue == 0 ? 1000 : diff_blue;

    t_ref = millis();

    if (current_time == ref_time && true){
        temp_red = red_state;
        temp_green = green_state;
        temp_blue = blue_state;

        if ((int)(t_ref - t_ref_red) > (1000 / diff_red)){
            if((int)target_red_next - (int)temp_red < -1){
                temp_red += -1;
            }
            if((int)target_red_next - (int)temp_red > 1){
                temp_red += 1;
            }
            t_ref_red += (1000 / diff_red);
        }

        if ((int)(t_ref - t_ref_green) > (1000 / diff_green)){
            if((int)target_green_next - (int)temp_green < -1){
                temp_green += -1;
            }
            if((int)target_green_next - (int)temp_green > 1){
                temp_green += 1;
            }
            t_ref_green += (1000 / diff_green);
        }

        if ((int)(t_ref - t_ref_blue) > (1000 / diff_blue)){
            if((int)target_blue_next - (int)temp_blue < -1){
                temp_blue += -1;
            }
            if((int)target_blue_next - (int)temp_blue > 1){
                temp_blue += 1;
            }
            t_ref_blue += (1000 / diff_blue);
        }
    }
    else {
        temp_red = target_red;
        temp_green = target_green;
        temp_blue = target_blue;

        t_ref_red = t_ref;
        t_ref_green = t_ref;
        t_ref_blue = t_ref;

        ref_time = current_time;
    }

    red_state = temp_red;
    green_state = temp_green;
    blue_state = temp_blue;

    for (int i=background_index; i < next_index; i++){
        uint32_t to_display = eclipse(i, red_state, green_state, blue_state);
        strip->setPixelColor(i, strip->gamma32(to_display));
    }

    strip->show();
    time_management();
}

Background::Background(uint16_t pin, uint16_t to, int minutes, uint16_t from, uint8_t brightness)
: background_index(from), next_index(to)
{
    time(&current_time);

    sunrise = current_time;

    sunset = sunrise + (minutes * 60);
    periodic_time = minutes * 60;

    periodic = M_PI / periodic_time;
    init_phase = sunrise;

    strip = new Adafruit_NeoPixel(to, pin, DATA_TRANSMISSION);
    strip->begin();
    strip->setBrightness(brightness);
    strip->show();

    new_used = true;
}


Background::Background(uint16_t pin, time_t rise, time_t set, uint16_t to, uint16_t from, uint8_t brightness)
: background_index(from), next_index(to)
{
    time(&current_time);
    sunrise = rise;
    sunset = set;
    periodic_time = 43200;

    periodic = M_PI / periodic_time;
    init_phase = sunrise;

    strip = new Adafruit_NeoPixel(to, pin, DATA_TRANSMISSION);
    strip->begin();
    strip->setBrightness(brightness);
    strip->show();

    new_used = true;
}

Background::Background(Adafruit_NeoPixel &neo, int minutes, uint16_t start_index, uint16_t to)
: background_index(start_index), next_index(to == 0 ? neo.numPixels() : to)
{
    time(&current_time);

    sunrise = current_time;

    sunset = sunrise + (minutes * 60);
    periodic_time = minutes * 60;

    periodic = M_PI / periodic_time;
    init_phase = sunrise;

    strip = &neo;

    new_used = false;
}

Background::Background(Adafruit_NeoPixel &neo, time_t rise, time_t set, uint16_t start_index, uint16_t to)
: background_index(start_index), next_index(to == 0 ? neo.numPixels() : to)
{
    time(&current_time);
    sunrise = rise;
    sunset = set;
    periodic_time = 43200;

    periodic = M_PI / periodic_time;
    init_phase = sunrise;

    strip = &neo;

    new_used = false;
}

Background::~Background(){
    if (new_used){
        delete strip;
    }
}
