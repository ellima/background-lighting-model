#include "BackgroundLight.h"


float Background::intensity_factor(int i){
    int sun_offset = (sunset - sunrise) / 2;

    if (current_time >= sunrise - sun_offset && current_time <= sunset + sun_offset){
        int half_leds = (next_index - background_index) / 2;
        float peak = fmap(current_time, sunrise - sun_offset, sunset + sun_offset, (int)background_index - half_leds, (int)next_index - 1 + half_leds);
        return gauss(i, peak);
    }
    else {
        return 0;
    }
}

float Background::intensity_factor(int i, long time){
    int sun_offset = (sunset - sunrise) / 2;

    if (time >= sunrise - sun_offset && time <= sunset + sun_offset){
        int half_leds = (next_index - background_index) / 2;
        float peak = fmap(time, sunrise - sun_offset, sunset + sun_offset, (int)background_index - half_leds, (int)next_index - 1 + half_leds);
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

uint8_t Background::eclipse(int index, uint8_t colour, uint8_t morning, uint8_t noon, long time){

    uint8_t target_col = morning;

    float sun_height = sin(periodic * (time - init_phase));

    if (sun_height >= 0){
        target_col = fmap(sun_height, 0, 1, morning, noon);
    }

    float intensity = intensity_factor(index, time);
    uint8_t new_colour = fmap(intensity, 0, 1, colour, target_col);

    return new_colour;
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

    t_ref = millis();

    for (int i=background_index; i < next_index; i++){
        uint8_t target_red = fmap(sun_height, -1, 1, midnight_blue.red, sky_blue.red);
        uint8_t target_green = fmap(sun_height, -1, 1, midnight_blue.green, sky_blue.green);
        uint8_t target_blue = fmap(sun_height, -1, 1, midnight_blue.blue, sky_blue.blue);

        uint8_t target_red_next = fmap(sun_height_next, -1, 1, midnight_blue.red, sky_blue.red);
        uint8_t target_green_next = fmap(sun_height_next, -1, 1, midnight_blue.green, sky_blue.green);
        uint8_t target_blue_next = fmap(sun_height_next, -1, 1, midnight_blue.blue, sky_blue.blue);

        target_red = eclipse(i, target_red, morning_red.red, noon.red, current_time);
        target_green = eclipse(i, target_green, morning_red.green, noon.green, current_time);
        target_blue = eclipse(i, target_blue, morning_red.blue, noon.blue, current_time);

        target_red_next = eclipse(i, target_red_next, morning_red.red, noon.red, current_time + 1);
        target_green_next = eclipse(i, target_green_next, morning_red.green, noon.green, current_time + 1);
        target_blue_next = eclipse(i, target_blue_next, morning_red.blue, noon.blue, current_time + 1);

        int diff_red = target_red_next - target_red >=0 ? target_red_next - target_red : target_red - target_red_next;
        int diff_green = target_green_next - target_green >= 0 ? target_green_next - target_green : target_green - target_green_next;
        int diff_blue = target_blue_next - target_blue >= 0 ? target_blue_next - target_blue : target_blue - target_blue_next;

        diff_red = diff_red == 0 ? 1001 : diff_red;
        diff_green = diff_green == 0 ? 1001 : diff_green;
        diff_blue = diff_blue == 0 ? 1001 : diff_blue;

        if (current_time == ref_time && true){
            if ((int)(t_ref - t_ref_red[i-background_index]) > (1000 / diff_red)){
                if((int)target_red_next - (int)red_state[i-background_index] < -1){
                    red_state[i-background_index] += -1;
                }
                if((int)target_red_next - (int)red_state[i-background_index] > 1){
                    red_state[i-background_index] += 1;
                }
                t_ref_red[i-background_index] += (1000 / diff_red);
            }

            if ((int)(t_ref - t_ref_green[i-background_index]) > (1000 / diff_green)){
                if((int)target_green_next - (int)green_state[i-background_index] < -1){
                    green_state[i-background_index] += -1;
                }
                if((int)target_green_next - (int)green_state[i-background_index] > 1){
                    green_state[i-background_index] += 1;
                }
                t_ref_green[i-background_index] += (1000 / diff_green);
            }

            if ((int)(t_ref - t_ref_blue[i-background_index]) > (1000 / diff_blue)){
                if((int)target_blue_next - (int)blue_state[i-background_index] < -1){
                    blue_state[i-background_index] += -1;
                }
                if((int)target_blue_next - (int)blue_state[i-background_index] > 1){
                    blue_state[i-background_index] += 1;
                }
                t_ref_blue[i-background_index] += (1000 / diff_blue);
            }
        }
        else {
            red_state[i-background_index] = target_red;
            green_state[i-background_index] = target_green;
            blue_state[i-background_index] = target_blue;

            t_ref_red[i-background_index] = t_ref;
            t_ref_green[i-background_index] = t_ref;
            t_ref_blue[i-background_index] = t_ref;

            ref_time = current_time;
        }

        uint32_t to_display = strip->Color(red_state[i-background_index], green_state[i-background_index], blue_state[i-background_index]);
        strip->setPixelColor(i, strip->gamma32(to_display));
    }

    strip->show();
    time_management();
}

Background::Background(uint16_t pin, uint16_t to, int minutes, uint16_t from, uint8_t brightness)
: background_index(from), next_index(to), new_used(true)
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

    sigma = (to - from) / 8 < 1 ? 1 : (to - from) / 8;

    float sun_height = sin(periodic * (current_time - init_phase));
    uint8_t temp = 0;

    red_state = new uint8_t[next_index - background_index];
    temp = fmap(sun_height, -1, 1, midnight_blue.red, sky_blue.red);
    for (int i=0; i < (next_index - background_index); i++){
        red_state[i] = eclipse(i, temp, morning_red.red, noon.red, current_time);
    }

    green_state = new uint8_t[next_index - background_index];
    temp = fmap(sun_height, -1, 1, midnight_blue.green, sky_blue.green);
    for (int i=0; i < (next_index - background_index); i++){
        green_state[i] = eclipse(i, temp, morning_red.green, noon.green, current_time);
    }

    blue_state = new uint8_t[next_index - background_index];
    temp = fmap(sun_height, -1, 1, midnight_blue.blue, sky_blue.blue);
    for (int i=0; i < (next_index - background_index); i++){
        blue_state[i] = eclipse(i, temp, morning_red.blue, noon.blue, current_time);
    }

    t_ref_red = new unsigned long[next_index - background_index];
    t_ref_green = new unsigned long[next_index - background_index];
    t_ref_blue = new unsigned long[next_index - background_index];
}

Background::Background(uint16_t pin, long rise, long set, uint16_t to, uint16_t from, uint8_t brightness)
: background_index(from), next_index(to), new_used(true)
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

    sigma = (to - from) / 8 < 1 ? 1 : (to - from) / 8;

    float sun_height = sin(periodic * (current_time - init_phase));
    uint8_t temp = 0;

    red_state = new uint8_t[next_index - background_index];
    temp = fmap(sun_height, -1, 1, midnight_blue.red, sky_blue.red);
    for (int i=0; i < (next_index - background_index); i++){
        red_state[i] = eclipse(i, temp, morning_red.red, noon.red, current_time);
    }

    green_state = new uint8_t[next_index - background_index];
    temp = fmap(sun_height, -1, 1, midnight_blue.green, sky_blue.green);
    for (int i=0; i < (next_index - background_index); i++){
        green_state[i] = eclipse(i, temp, morning_red.green, noon.green, current_time);
    }

    blue_state = new uint8_t[next_index - background_index];
    temp = fmap(sun_height, -1, 1, midnight_blue.blue, sky_blue.blue);
    for (int i=0; i < (next_index - background_index); i++){
        blue_state[i] = eclipse(i, temp, morning_red.blue, noon.blue, current_time);
    }

    t_ref_red = new unsigned long[next_index - background_index];
    t_ref_green = new unsigned long[next_index - background_index];
    t_ref_blue = new unsigned long[next_index - background_index];
}

Background::Background(Adafruit_NeoPixel &neo, int minutes, uint16_t from, uint16_t to)
: background_index(from), next_index(to <= 0 || to <= from ? neo.numPixels() : to), new_used(false)
{
    time(&current_time);

    sunrise = current_time;

    sunset = sunrise + (minutes * 60);
    periodic_time = minutes * 60;

    periodic = M_PI / periodic_time;
    init_phase = sunrise;

    strip = &neo;

    sigma = to <= 0 || to <= from ? neo.numPixels() : to;
    sigma = (sigma - from) / 8 < 1 ? 1 : (sigma - from) / 8;

    float sun_height = sin(periodic * (current_time - init_phase));
    uint8_t temp = 0;

    red_state = new uint8_t[next_index - background_index];
    temp = fmap(sun_height, -1, 1, midnight_blue.red, sky_blue.red);
    for (int i=0; i < (next_index - background_index); i++){
        red_state[i] = eclipse(i, temp, morning_red.red, noon.red, current_time);
    }

    green_state = new uint8_t[next_index - background_index];
    temp = fmap(sun_height, -1, 1, midnight_blue.green, sky_blue.green);
    for (int i=0; i < (next_index - background_index); i++){
        green_state[i] = eclipse(i, temp, morning_red.green, noon.green, current_time);
    }

    blue_state = new uint8_t[next_index - background_index];
    temp = fmap(sun_height, -1, 1, midnight_blue.blue, sky_blue.blue);
    for (int i=0; i < (next_index - background_index); i++){
        blue_state[i] = eclipse(i, temp, morning_red.blue, noon.blue, current_time);
    }

    t_ref_red = new unsigned long[next_index - background_index];
    t_ref_green = new unsigned long[next_index - background_index];
    t_ref_blue = new unsigned long[next_index - background_index];
}

Background::Background(Adafruit_NeoPixel &neo, long rise, long set, uint16_t from, uint16_t to)
: background_index(from), next_index(to == 0 ? neo.numPixels() : to), new_used(false)
{
    time(&current_time);
    sunrise = rise;
    sunset = set;
    periodic_time = 43200;

    periodic = M_PI / periodic_time;
    init_phase = sunrise;

    strip = &neo;

    sigma = to <= 0 || to <= from ? neo.numPixels() : to;
    sigma = (sigma - from) / 8 < 1 ? 1 : (sigma - from) / 8;

    float sun_height = sin(periodic * (current_time - init_phase));
    uint8_t temp = 0;

    red_state = new uint8_t[next_index - background_index];
    temp = fmap(sun_height, -1, 1, midnight_blue.red, sky_blue.red);
    for (int i=0; i < (next_index - background_index); i++){
        red_state[i] = eclipse(i, temp, morning_red.red, noon.red, current_time);
    }

    green_state = new uint8_t[next_index - background_index];
    temp = fmap(sun_height, -1, 1, midnight_blue.green, sky_blue.green);
    for (int i=0; i < (next_index - background_index); i++){
        green_state[i] = eclipse(i, temp, morning_red.green, noon.green, current_time);
    }

    blue_state = new uint8_t[next_index - background_index];
    temp = fmap(sun_height, -1, 1, midnight_blue.blue, sky_blue.blue);
    for (int i=0; i < (next_index - background_index); i++){
        blue_state[i] = eclipse(i, temp, morning_red.blue, noon.blue, current_time);
    }

    t_ref_red = new unsigned long[next_index - background_index];
    t_ref_green = new unsigned long[next_index - background_index];
    t_ref_blue = new unsigned long[next_index - background_index];
}

Background::~Background(){
    if (new_used){
        delete strip; strip = nullptr;
    }

    delete[] red_state; red_state = nullptr;
    delete[] green_state; green_state = nullptr;
    delete[] blue_state; blue_state = nullptr;

    delete[] t_ref_red; t_ref_red = nullptr;
    delete[] t_ref_green; t_ref_green = nullptr;
    delete[] t_ref_blue; t_ref_blue = nullptr;
}
