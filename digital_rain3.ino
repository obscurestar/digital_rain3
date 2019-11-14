/*Digital Rain
 * An ongoing exercise in refactoring, generalizing, and optimizing code and making 
 * some sensual textures for an enviroment sculpted in light.

  Overview:  Using coin tosses and a few simple rules, we shall cycle a limited palette in a
  way that shimmers.  See rain.h tab at top of window for more details.
  
  V3 Goals:
  Improve scoping of global variables.
  Add support for SPI memory use of large arrays.
  
  */

#include "CONFIG.h"
#include "rain.h"  //The digital rain algorithm

//TODO protect globals in V3.
//Global variables
Rain rain;           //Container class for rain algorithm.  See rain.h for details

void setup() {       //Builtin function run once at start of app.
  Serial.begin(9600); //Open serial(Com speed) Useful for debug but watch string memory use.

  //strip.setBrightness(BRIGHTNESS);  //TODO experiment with this feature.
  H_LEDS.begin(); //Initialize communication with WS281* chain.
  H_LEDS.show(); //No values are set so this should be black.
}

void loop() { //Builtin function.
  rain.loopStep();
  CONFIG::pix.display();
  //TODO:
  /*
   * This delay is hard-coded and we have to recompile and re-upload any time
   * we want to change it.  A variable control would be cool.  In V3 we'll 
   * think of that control as a discrete concept and discuss implementing it
   * over serial, potentiometer, or digital signal and some example uses of 
   * each.
   */
  //delay(50);
}

/*display_pix transcribes the contents of the pix array to the LED driver hardware.
 * In some future version we may want different display routines for stateless 
 * patterns or scaling, mirroring, compressing and otherwise distorting part of a 
 * pattern and projecting it across the array.
 */
 
void display_pix()
{
  for (int i=0;i<CONFIG::NUM_LEDS;++i)
  {
    COLOR c;
    c = CONFIG::pix.get(i);
    CONFIG::H_LEDS.setPixelColor( ( (CONFIG::NUM_LEDS - i) - 1), CONFIG::H_LEDS.Color(c.c[0], c.c[1], c.c[2]));
  }
  CONFIG::H_LEDS.show();
}
