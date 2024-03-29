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
#include "litepixel.h"

//TODO protect globals in V3.
//Global variables
Rain rain;           //Container class for rain algorithm.  See rain.h for details
LitePixel lite;

void setup() {       //Builtin function run once at start of app.
  Serial.begin(9600); //Open serial(Com speed) Useful for debug but watch string memory use.

  lite.setup(); //Initialize communication with WS281* chain.
}

void loop() { //Builtin function.
  rain.loopStep();
  display_pix();
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
  cli();
  for (int i=0;i<CONFIG::NUM_LEDS;++i)
  {
    COLOR c;
    c = CONFIG::pix.get(i);
    lite.sendPixel( c.c[0], c.c[1], c.c[2]);
  }
  sei();
  lite.show();
}
