#include "BackgroundLight.h"


// calculates intensity of the sun light for a given LED
float Background::intensity_factor(int i){
    // define time offset for smooth sunrise/-set
    int sun_offset = periodic_time - ( (sunset - sunrise) / 2 );

    // only map intensity when time is between sunrise and sunset +- offset 
    if ((current_time >= sunrise - sun_offset && current_time <= sunset + sun_offset) || fixed_minutes){
        int half_leds = led_count / 2;
        float peak = fmap(current_time, sunrise - sun_offset, sunset + sun_offset, (int)background_index - half_leds, (int)next_index - 1 + half_leds);
        return gauss(opposite ? led_count - 1 - i : i, peak); // use Gaussian distribution over the LED's indices
    }
    else {
        return 0;
    }
}


// calculates intensity of the sun light for a given LED for a given time
float Background::intensity_factor(int i, long time){
    // define time offset for smooth sunrise/-set
    int sun_offset = periodic_time - ( (sunset - sunrise) / 2 );

    // only map intensity when time is between sunrise and sunset +- offset
    if ((time >= sunrise - sun_offset && time <= sunset + sun_offset) || fixed_minutes){
        int half_leds = led_count / 2;
        float peak = fmap(time, sunrise - sun_offset, sunset + sun_offset, (int)background_index - half_leds, (int)next_index - 1 + half_leds);
        return gauss(opposite ? led_count - 1 - i : i, peak); // use Gaussian distribution over the LED's indices
    }
    else {
        return 0;
    }
}


// maps requested colour to sun's intensity between morning red and noon. Uses intensity_factor() funtion.
uint32_t Background::eclipse(int index, uint8_t red, uint8_t green, uint8_t blue){
    // init target colours to morning light
    uint8_t target_red = morning_red.red;
    uint8_t target_green = morning_red.green;
    uint8_t target_blue = morning_red.blue;

    // define current sun height for current time
    float sun_height = day_sine(current_time);

    if (sun_height >= 0){ // only map to noon's colour if the sun's heght is > 0
        target_red = fmap(sun_height, 0, max(), morning_red.red, noon.red);
        target_green = fmap(sun_height, 0, max(), morning_red.green, noon.green);
        target_blue = fmap(sun_height, 0, max(), morning_red.blue, noon.blue);
    }

    float intensity = intensity_factor(index);

    uint8_t new_red = fmap(intensity, 0, 1, red, target_red);
    uint8_t new_green = fmap(intensity, 0, 1, green, target_green);
    uint8_t new_blue = fmap(intensity, 0, 1, blue, target_blue);

    // return LED's colour with regard to the sun (can be the plain background)
    return strip->Color(new_red, new_green, new_blue);
}


// maps requested colour to sun's intensity between morning and noon for a chosen time.
uint8_t Background::eclipse(int index, uint8_t colour, uint8_t morning, uint8_t noon, long time){
    // init target colour to morning light
    uint8_t target_col = morning;
    float sun_height = day_sine(time);

    if (sun_height >= 0){ // only map to noon's colour if the sun's heght is > 0
        target_col = fmap(sun_height, 0, max(), morning, noon);
    }

    // map sky colour to sun's intensity for a pixel and time
    float intensity = intensity_factor(index, time);
    uint8_t new_colour = fmap(intensity, 0, 1, colour, target_col);

    return new_colour;
}


// adjusts the sunset and sunrise time as soon as current time exceeds mean between sunset and the next sunrise
void Background::time_management(){
    // general expression, works for a variable as well as constant periodic time
    if (current_time > sunset + ( ( (2 * periodic_time) - (sunset - sunrise) ) / 2))
    {
        sunrise += 2 * periodic_time;
        sunset += 2 * periodic_time;
    }
    // should be called for each adjustment of pixel colour, see e.g. background_sky()
}


// main background function
// calls functions for each LED's current colour state
// ensures smooth transition by also computing next colour and interpolate between succeeding LED colours
void Background::background_sky(){
    time(&current_time); // refresh current time

    float sun_height = day_sine(current_time);
    float sun_height_next = day_sine(current_time + 1); // next time's sun height (i.e. next second)

    t_ref = millis();

    for (int i=background_index; i < next_index; i++) // iterate through every LED
    {
        // init target colours for current time ...
        uint8_t target_red = fmap(sun_height, min(), max(), midnight_blue.red, sky_blue.red);
        uint8_t target_green = fmap(sun_height, min(), max(), midnight_blue.green, sky_blue.green);
        uint8_t target_blue = fmap(sun_height, min(), max(), midnight_blue.blue, sky_blue.blue);

        // ... and succeeding time
        uint8_t target_red_next = fmap(sun_height_next, min(), max(), midnight_blue.red, sky_blue.red);
        uint8_t target_green_next = fmap(sun_height_next, min(), max(), midnight_blue.green, sky_blue.green);
        uint8_t target_blue_next = fmap(sun_height_next, min(), max(), midnight_blue.blue, sky_blue.blue);

        // apply sky- and sunlight
        target_red = eclipse(i, target_red, morning_red.red, noon.red, current_time);
        target_green = eclipse(i, target_green, morning_red.green, noon.green, current_time);
        target_blue = eclipse(i, target_blue, morning_red.blue, noon.blue, current_time);

        target_red_next = eclipse(i, target_red_next, morning_red.red, noon.red, current_time + 1);
        target_green_next = eclipse(i, target_green_next, morning_red.green, noon.green, current_time + 1);
        target_blue_next = eclipse(i, target_blue_next, morning_red.blue, noon.blue, current_time + 1);

        // calculate colour differences for smooth transition
        int diff_red = target_red_next - target_red >=0 ? target_red_next - target_red : target_red - target_red_next;
        int diff_green = target_green_next - target_green >= 0 ? target_green_next - target_green : target_green - target_green_next;
        int diff_blue = target_blue_next - target_blue >= 0 ? target_blue_next - target_blue : target_blue - target_blue_next;

        // make sure the division in the next step is well defined
        diff_red = diff_red == 0 ? 1 : diff_red;
        diff_green = diff_green == 0 ? 1 : diff_green;
        diff_blue = diff_blue == 0 ? 1 : diff_blue;

        // handle smooth transitions between integer seconds for smooth colour transition
        if (current_time == ref_time){ // check if time is in between integer seconds
            // divide the second in equal time periods per colour difference
            if ((int)(t_ref - t_ref_red[i-background_index]) > (1000 / diff_red)){
                // adjust colour value in corresponding direction
                if((int)target_red_next - (int)red_state[i-background_index] < -1){
                    red_state[i-background_index] += -1;
                }
                if((int)target_red_next - (int)red_state[i-background_index] > 1){
                    red_state[i-background_index] += 1;
                }
                t_ref_red[i-background_index] += (1000 / diff_red);
            }

            // repeat for green ...
            if ((int)(t_ref - t_ref_green[i-background_index]) > (1000 / diff_green)){
                if((int)target_green_next - (int)green_state[i-background_index] < -1){
                    green_state[i-background_index] += -1;
                }
                if((int)target_green_next - (int)green_state[i-background_index] > 1){
                    green_state[i-background_index] += 1;
                }
                t_ref_green[i-background_index] += (1000 / diff_green);
            }

            // ... and blue
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
            // if current time is the next second, set the new colour values
            red_state[i-background_index] = target_red;
            green_state[i-background_index] = target_green;
            blue_state[i-background_index] = target_blue;

            t_ref_red[i-background_index] = t_ref;
            t_ref_green[i-background_index] = t_ref;
            t_ref_blue[i-background_index] = t_ref;

            // set reference time to the current time
            ref_time = current_time;
        }

        // set the LED's colour
        uint32_t to_display = strip->Color(red_state[i-background_index], green_state[i-background_index], blue_state[i-background_index]);
        strip->setPixelColor(i, strip->gamma32(to_display));
    }

    strip->show();
    time_management(); // let the programme check if the time parameters need to be adapted
}

// Constructors

//initialize Background object for user defined periodic time, daytime = nighttime; no Adafruit_NeoPixel object initialized before
Background::Background(uint16_t pin, bool opp_startpoint, uint16_t to, int minutes, uint16_t from, uint8_t brightness)
: background_index(from), next_index(to), new_used(true), opposite(opp_startpoint), fixed_minutes(true)
{
    time(&current_time);

    // set/calculate parameters
    sunrise = current_time;
    led_count = next_index - background_index;

    // set fixed day/nighttime
    sunset = sunrise + (minutes * 60);
    periodic_time = minutes * 60;

    periodic = M_PI / periodic_time;
    init_phase = sunrise;

    // init LED strip's object
    strip = new Adafruit_NeoPixel(to, pin, DATA_TRANSMISSION);
    strip->begin();
    strip->setBrightness(brightness);

    // choose width of the sun depending on the LED strip's length
    sigma = (next_index - background_index) / 8 < 1 ? 1 : (next_index - background_index) / 8;

    float sun_height = day_sine(current_time);
    uint8_t temp = 0; // temporary variable for storing the sky colour

    // initialize each pixel's colour and arrays to store them
    red_state = new uint8_t[led_count];
    temp = fmap(sun_height, min(), max(), midnight_blue.red, sky_blue.red);
    for (int i=0; i < (led_count); i++){
        red_state[i] = eclipse(i, temp, morning_red.red, noon.red, current_time);
    }

    green_state = new uint8_t[led_count];
    temp = fmap(sun_height, min(), max(), midnight_blue.green, sky_blue.green);
    for (int i=0; i < (led_count); i++){
        green_state[i] = eclipse(i, temp, morning_red.green, noon.green, current_time);
    }

    blue_state = new uint8_t[led_count];
    temp = fmap(sun_height, min(), max(), midnight_blue.blue, sky_blue.blue);
    for (int i=0; i < (led_count); i++){
        blue_state[i] = eclipse(i, temp, morning_red.blue, noon.blue, current_time);
    }

    // init arrays for the pixel colour's references
    t_ref_red = new unsigned long[led_count];
    t_ref_green = new unsigned long[led_count];
    t_ref_blue = new unsigned long[led_count];
}


// initialize Background object for given sunrise and sunset, variable day/nighttime; no Adafruit_NeoPixel object initialized before
Background::Background(uint16_t pin, bool opp_startpoint, long rise, long set, uint16_t to, uint16_t from, uint8_t brightness)
: background_index(from), next_index(to), new_used(true), opposite(opp_startpoint), fixed_minutes(false)
{
    time(&current_time);

    // set/calculate parameters
    sunrise = rise;
    sunset = set;
    led_count = next_index - background_index;

    // calculate initial day/nighttime
    periodic_time = 43200;
    periodic = M_PI / float(periodic_time);

    init_phase = (sunset + sunrise) / 2;
    day_offset = cos(periodic * (sunset - sunrise) / 2.);

    // init LED strip's object
    strip = new Adafruit_NeoPixel(to, pin, DATA_TRANSMISSION);
    strip->begin();
    strip->setBrightness(brightness);

    // choose width of the sun depending on the LED strip's length
    sigma = (next_index - background_index) / 8 < 1 ? 1 : (next_index - background_index) / 8;

    float sun_height = day_sine(current_time);
    uint8_t temp = 0; // temporary variable for storing the sky colour

    // initialize each pixel's colour and arrays to store them
    red_state = new uint8_t[next_index - background_index];
    temp = fmap(sun_height, min(), max(), midnight_blue.red, sky_blue.red);
    for (int i=0; i < (next_index - background_index); i++){
        red_state[i] = eclipse(i, temp, morning_red.red, noon.red, current_time);
    }

    green_state = new uint8_t[next_index - background_index];
    temp = fmap(sun_height, min(), max(), midnight_blue.green, sky_blue.green);
    for (int i=0; i < (next_index - background_index); i++){
        green_state[i] = eclipse(i, temp, morning_red.green, noon.green, current_time);
    }

    blue_state = new uint8_t[next_index - background_index];
    temp = fmap(sun_height, min(), max(), midnight_blue.blue, sky_blue.blue);
    for (int i=0; i < (next_index - background_index); i++){
        blue_state[i] = eclipse(i, temp, morning_red.blue, noon.blue, current_time);
    }

    // init arrays for the pixel colour's references
    t_ref_red = new unsigned long[next_index - background_index];
    t_ref_green = new unsigned long[next_index - background_index];
    t_ref_blue = new unsigned long[next_index - background_index];
}


//initialize Background object for user defined periodic time, daytime = nighttime; use already initialized Adafruit_NeoPixel object
Background::Background(Adafruit_NeoPixel &neo, bool opp_startpoint, int minutes, uint16_t from, uint16_t to)
: background_index(from), next_index(to <= 0 || to <= from ? neo.numPixels() : to), new_used(false), opposite(opp_startpoint), fixed_minutes(true)
{
    time(&current_time);

    // set/calculate parameters
    sunrise = current_time;
    led_count = next_index - background_index;

    // set fixed day/nighttime
    sunset = sunrise + (minutes * 60);
    periodic_time = minutes * 60;

    periodic = M_PI / periodic_time;
    init_phase = sunrise;

    // assign LED strip object to this class' internal LED strip object
    strip = &neo;

    // choose width of the sun depending on the LED strip's length
    sigma = (next_index - background_index) / 8 < 1 ? 1 : (next_index - background_index) / 8;

    float sun_height = day_sine(current_time);
    uint8_t temp = 0; // temporary variable for storing the sky colour

    // initialize each pixel's colour and arrays to store them
    red_state = new uint8_t[next_index - background_index];
    temp = fmap(sun_height, min(), max(), midnight_blue.red, sky_blue.red);
    for (int i=0; i < (next_index - background_index); i++){
        red_state[i] = eclipse(i, temp, morning_red.red, noon.red, current_time);
    }

    green_state = new uint8_t[next_index - background_index];
    temp = fmap(sun_height, min(), max(), midnight_blue.green, sky_blue.green);
    for (int i=0; i < (next_index - background_index); i++){
        green_state[i] = eclipse(i, temp, morning_red.green, noon.green, current_time);
    }

    blue_state = new uint8_t[next_index - background_index];
    temp = fmap(sun_height, min(), max(), midnight_blue.blue, sky_blue.blue);
    for (int i=0; i < (next_index - background_index); i++){
        blue_state[i] = eclipse(i, temp, morning_red.blue, noon.blue, current_time);
    }

    // init arrays for the pixel colour's references
    t_ref_red = new unsigned long[next_index - background_index];
    t_ref_green = new unsigned long[next_index - background_index];
    t_ref_blue = new unsigned long[next_index - background_index];
}


// initialize Background object for given sunrise and sunset, variable day/nighttime; use already initialized Adafruit_NeoPixel object
Background::Background(Adafruit_NeoPixel &neo, bool opp_startpoint, long rise, long set, uint16_t from, uint16_t to)
: background_index(from), next_index(to <= 0 || to <= from ? neo.numPixels() : to), new_used(false), opposite(opp_startpoint), fixed_minutes(false)
{
    time(&current_time);

    // set/calculate parameters
    sunrise = rise;
    sunset = set;
    led_count = next_index - background_index;

    // calculate initial day/nighttime
    periodic_time = 43200;
    periodic = M_PI / periodic_time;

    init_phase = (sunset + sunrise) / 2;
    day_offset = cos(periodic * float(sunset - sunrise) / 2);

    // assign LED strip object to this class' internal LED strip object
    strip = &neo;

    // choose width of the sun depending on the LED strip's length
    sigma = (next_index - background_index) / 8 < 1 ? 1 : (next_index - background_index) / 8;

    float sun_height = day_sine(current_time);
    uint8_t temp = 0;

    // initialize each pixel's colour and arrays to store them
    red_state = new uint8_t[next_index - background_index];
    temp = fmap(sun_height, min(), max(), midnight_blue.red, sky_blue.red);
    for (int i=0; i < (next_index - background_index); i++){
        red_state[i] = eclipse(i, temp, morning_red.red, noon.red, current_time);
    }

    green_state = new uint8_t[next_index - background_index];
    temp = fmap(sun_height, min(), max(), midnight_blue.green, sky_blue.green);
    for (int i=0; i < (next_index - background_index); i++){
        green_state[i] = eclipse(i, temp, morning_red.green, noon.green, current_time);
    }

    blue_state = new uint8_t[next_index - background_index];
    temp = fmap(sun_height, min(), max(), midnight_blue.blue, sky_blue.blue);
    for (int i=0; i < (next_index - background_index); i++){
        blue_state[i] = eclipse(i, temp, morning_red.blue, noon.blue, current_time);
    }

    // init arrays for the pixel colour's references
    t_ref_red = new unsigned long[next_index - background_index];
    t_ref_green = new unsigned long[next_index - background_index];
    t_ref_blue = new unsigned long[next_index - background_index];
}


// destructor
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
