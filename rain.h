//rain.h
//Use random walks to do a palette shift.  

#ifndef RAIN_H  //These preprocessor directives let you include without worrying about dupes.
#define RAIN_H  //Once defined it doesn't need to be re-included.

#include "color.h"
#include "CONFIG.h"

using namespace CONFIG;

//Class
class Rain
{
public:   //public variables.
  int mShiftOdds; //The 1 in n odds of picking a new hue mask.
public:   //public functions.
  Rain(); //Default constructor
  void loopStep();  //Per frame call from loop.
private:
  byte pickHueMask();   //Get a new hue mask
  byte walkPixels();    //The math of the shimmer.
private:  //class private variables
  byte mHueMask;  //Which bits are active on this iteration.
  bool mDirty;    //Set to true when any RGB not in current set is set for any pixel in chain.
};

Rain::Rain() : mDirty(false),
               mShiftOdds(1000)  //TODO: Setting arbitrary inline const is ugly
{
  mHueMask = pickHueMask();
}

void Rain::loopStep()
{
  if (!walkPixels())
  {
    if (!random(mShiftOdds))
    {
      mHueMask = pickHueMask();
    }
  }
}

byte Rain::pickHueMask()
{
  /*Generate a color.  I want to use mostly calming colors with bursts of pure 
   * red being somewhat uncommon and never picking black or white. 
   * Our LEDs output RGB red, green, blue.  We'll use a 3-bit mask to represent 
   * whether or not each LED is on with 1 as on and 0 as off. 
   * Additionally, or mask should never be 000 or 111 (black and white) Since we
   * want to do a special behavior for red, let's omit 001 as well.
   * We could randomly create each bit R=random(2); G=random(2); B=random(2)
   * and then use conditional logic to verify that we have met the conditions
   * we set but we're free to think of this another way too.  I we think of the bitmask
   * as a binary representation of a number (see table below) we see that the conditions
   * for our logic are a set of numbers from 2 to 6.  Therefor we can generate all of 
   * the random bits and entirely skip the valid range and special handling for red
   * by simply generating a random number that is either 2,3,4,5, or 6.
   * 0 000 black
   * 1 001 red       primary
   * 2 010 green     primary
   * 3 011 yellow    red+green
   * 4 100 blue      primary
   * 5 101 magenta   red+blue
   * 6 110 cyan      green+blue
   * 7 111 white     red+green+blue
   */
  
  byte cbits=random(5)+2;  //generate value from 2-6

   /*Now let's create the rare opportunity for red.  We'll do this by inverting the bitmask
    * when our random value is 0.  if the color happens to be cyan, it will be flipped to 
    * red.  The only time this will actually happen is when the randomizer rolls 0 AND the  
    * color generated above is 110 (cyan)
    */

  if (!random(10))  //If 0, invert the bitmask.
  {
    cbits = ~cbits;  // ~ is the complimentary operator 010 becomes 101 etc
  }
  return cbits;
}

//Iterate through pixels and stagger around in the relative color space. 
byte Rain::walkPixels()
{
  mDirty=false;
  for (int p=0;p<CONFIG::NUM_LEDS; ++p)  //Loop through pixels.
  {
    for (int c=0;c<3;++c)     //Loop through RBG sub-pixels of each pixel.
    {
      /*in pickHueMask we set mHueMask to the subset of LEDs in a pixel we want to touch
       * (mHueMask >> c) means 'bitshift the value of mHueMask right by c bits. 
       * For example, recall that bits are powers of 2.  IE 2^0=1, 2^1=2, 2^2=4, 2^3=8
       * >> simply moves all the bits in a word one to the right. 8 becomes 4, 4 2, etc. 
       * whatever was in the 1's column is thrown away. If you have a number like 
       * 23 (binary 10111) >> 2 the result is: 101 (5 decimal) 
       * We then & with 1 to see if this Hue is to be lit.  
       * if ( mHueMask & (1<<c) ) is functionally identical but shifts the comparator val 
       * instead.
       */
      if ( (mHueMask >> c) & 1 )
      {
          /*Now we need to decide if this individual LED is going to step forward, backwards
           * or stay where it is.  I've done this by using random(3)-1 which produces 
           * either -1, 0, or 1 as its output. and simply added it to the value of the 
           * hue.  Note that my value is a signed int (2 bytes) whereas hues are 
           * (unsigned) byte.  Our result can be outside the range of the byte (256 or -1) 
           * so we use the extra space of the signed integer to absorb this, then range check
           * and only adopt valid range values. 
           */
          signed int val = CONFIG::pix.get(p).c[c] + (random(3) - 1); // rand result set [0,1,2] - 1 = [-1, 0, 1]
          if (val > 0 && val < 255)
          {
            CONFIG::pix.set(p,c,val);  //Stagger around in the relative color space.
          }
      }
      else
      {                       //Stagger towards 0, let iterator know this one doesn't count. 
        if (CONFIG::pix.get(p).c[c] > 0) //This RGB should not be set in this hue. Still draining previous color
        {
          mDirty=true;  //Indicates some pixels of the skipped hue are still set. 
          /*Initially I had simply pix.get(p).c[c] -= random(2); (50/50 chance of darkening) 
           * but decided this moved to 0 too quickly and played around to find a chance 
           * to descend that was more visually appealing to me. 
           */
          if (!random(6))
          {
            /*TODO: This is a bit uglier than our previous interface of 
             * simply -- Let's leave this for the moment.  If we find later
             * that we're doing a lot of math directly on elements, we might
             * consider writing some operator overloads to handle this.
             */
            COLOR col = CONFIG::pix.get(p); //Get value
            col.c[c] --;                    //Subtract
            CONFIG::pix.set(p, col);        //Write
          }
        }
      }
    }
  }
  return mDirty;
}


#endif //RAIN_H
