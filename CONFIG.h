/*CONFIG.h   (an unsatisfying solution)
 * Let us consider this the first step in refactoring how we handle details 
 * that we might want to change for hardware reasons.
 * For now, we'll constrain them to a file and include them everywhere.  I 
 */

 
#include <Adafruit_NeoPixel.h>  //Someone else did the heavy lifting.  Say thanks!
#ifdef __AVR__
  #include <avr/power.h>
#endif

#ifndef CONFIG_H  
#define CONFIG_H  

/*after this definition, all of our formerly global variaable will be 
 * contained in a namespace.  This prevents someone in another module 
 * from accidentally overwriting one of our variables. 'pix' for example
 * could later be a notion of photos in another module.
 * namespace also forces us to somewhat contain these items by loose association.
 * It reminds us they are misfits and that it would be nice if there 
 * were a better place for them to belong.
 * Little steps in refactoring each time you review code makes a more robust
 * codebase in the long run.
 * 
 * You can access namespace variables in two ways.  Either ns::member or 
 * file-scope trust that you have no variable name collisions with
 * using namespace ns;
 * I prefer the :: approach.  It's an easy change use the compiler to
 * find the required syntax chanbes.  Now all of my ugly global use is exposed
 * and one step closer to safe refactoring should we ever decide it
 * is not as universal a concept as I had previously believed.
 */

 /*At last, a good use for a #define!  Let's define USE_SPI_MEM if we want to 
  * store requests in arduino's 2K our SPI's 32K.
  * I do not know if arduino cross compile is capable of weeding out
  * uncalled functions. I might investigate this later but right now I
  * want to use one set of calls to provide the same interfaces to the 
  * rest of my code.  #defines are pre-processor directives and code
  * I put in those blocks is certain to not take up precious resources.
  * The define is here in config to make sure it populates everywhere.
  * Note you'll more often see #define foo <expr>  than just a bare #define
  * like this.  
  */

//Uncomment the following line to use SPI memory for pix array.
#define USE_SPI_MEM //Comment out this line to use Arduino memory for pix array.

#include "color.h"  //Our color datatype.
#include "pixelarray.h" 
namespace CONFIG 
{
  //Hardware constants
  /*SPI RAM Also uses the pins 11, 12, and 13 on the arduino*/
  const short PIN_SPIRAM = 10;  //The data pin for talking to extended memory module.
  const short PIN_LED = 6;  //Pin connecting the IN on the LED strip to the CPU board.
  const short NUM_LEDS = 256; //Num LEDS in our array.
  
  /*Handle to the LED array. We will use this to apply the values to the pixels in the array.*/
  Adafruit_NeoPixel H_LEDS = Adafruit_NeoPixel(CONFIG::NUM_LEDS, CONFIG::PIN_LED, NEO_GRB + NEO_KHZ800);

  //Global Variables
  byte MAX_BRITE=255;  //The brightest we want our display to get. 

 
  PixelArray pix(NUM_LEDS, 1); //Either pointer to buffer  or offset for spiram.
};

#else  //CONFIG_H 

namespace CONFIG
{
  extern const short NUM_LEDS; 
  extern Adafruit_NeoPixel H_LEDS;

//  extern PixelArray pix;  //Ditto!
};

#endif //CONFIG_H
