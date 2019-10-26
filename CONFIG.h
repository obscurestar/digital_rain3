/*CONFIG.h   (an unsatisfying solution)
 * Let us consider this the first step in refactoring how we handle details 
 * that we might want to change for hardware reasons.
 * For now, we'll constrain them to a file and include them everywhere.  I 
 */
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
#include "color.h"  //Our color datatype.
namespace CONFIG 
{
  //Hardware constants
  const short PIN_LED = 6;  //Pin connecting the IN on the LED strip to the CPU board.
  const short NUM_LEDS = 16; //Num LEDS in our array.

  //Global Variables
  byte MAX_BRITE=255;  //The brightest we want our display to get. 

  /*pix is 4 bytes per LED. Remember our 328 CPU only has 2K of active memory.
   * if you have 512 pix, that's all of the memory including the memory for calling functions
   * making computations, talking to our libraries, etc. 
   * We will address this in a later version by using a 32K expansion chip on the IC2 bus.
   */
  COLOR pix[NUM_LEDS]; //Array of data for our pixels.  See above!

};

#else  //CONFIG_H 

extern const short NUM_LEDS; //Let's address this in V3!
extern COLOR pix[];  //Ditto!

#endif //CONFIG_H
