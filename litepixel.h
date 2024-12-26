/*Based on code by Josh at 
 * More info at http://wp.josh.com/2014/05/11/ws2812-neopixels-made-easy/
 */

#ifndef LITE_PIXEL_H  
#define LITE_PIXEL_H

 // These values depend on which pin your string is connected to and what board you are using 
// More info on how to find these at http://www.arduino.cc/en/Reference/PortManipulation
#define PIXEL_PORT  PORTD  // Port of the pin the pixels are connected to
#define PIXEL_DDR   DDRD   // Port of the pin the pixels are connected to

// These are the timing constraints taken mostly from the WS2812 datasheets 
// These are chosen to be conservative and avoid problems rather than for maximum throughput 

#define T1H  900    // Width of a 1 bit in ns
#define T1L  600    // Width of a 1 bit in ns

#define T0H  400    // Width of a 0 bit in ns
#define T0L  900    // Width of a 0 bit in ns

// The reset gap can be 6000 ns, but depending on the LED strip it may have to be increased
// to values like 600000 ns. If it is too small, the pixels will show nothing most of the time.
#define RES 6000    // Width of the low gap between bits to cause a frame to latch

// Here are some convience defines for using nanoseconds specs to generate actual CPU delays

#define NS_PER_SEC (1000000000L)          // Note that this has to be SIGNED since we want to be able to check for negative values of derivatives

#define CYCLES_PER_SEC (F_CPU)

#define NS_PER_CYCLE ( NS_PER_SEC / CYCLES_PER_SEC )

#define NS_TO_CYCLES(n) ( (n) / NS_PER_CYCLE )

class LitePixel
{
  public:
    inline void sendBit( bool vitVal );
    inline void sendByte( unsigned char byte );
    inline void sendPixel( unsigned char r, unsigned char g , unsigned char b );
    void showColor( unsigned char r , unsigned char g , unsigned char b );
    void setup();
    void show();
};
// Actually send a bit to the string. We must to drop to asm to enusre that the complier does
// not reorder things and make it so the delay happens in the wrong place.

inline void LitePixel::sendBit( bool bitVal ) 
{  
  if (  bitVal )
  {        // 0 bit
    asm volatile (
      "sbi %[port], %[bit] \n\t"        // Set the output bit
      ".rept %[onCycles] \n\t"                                // Execute NOPs to delay exactly the specified number of cycles
      "nop \n\t"
      ".endr \n\t"
      "cbi %[port], %[bit] \n\t"                              // Clear the output bit
      ".rept %[offCycles] \n\t"                               // Execute NOPs to delay exactly the specified number of cycles
      "nop \n\t"
      ".endr \n\t"
      ::
      [port]    "I" (_SFR_IO_ADDR(PIXEL_PORT)),
      [bit]   "I" (CONFIG::PIN_LED),
      [onCycles]  "I" (NS_TO_CYCLES(T1H) - 2),    // 1-bit width less overhead  for the actual bit setting, note that this delay could be longer and everything would still work
      [offCycles]   "I" (NS_TO_CYCLES(T1L) - 2)     // Minimum interbit delay. Note that we probably don't need this at all since the loop overhead will be enough, but here for correctness

    );
                                  
  } 
  else
  {          // 1 bit
    // **************************************************************************
    // This line is really the only tight goldilocks timing in the whole program!
    // **************************************************************************
    
      asm volatile (
        "sbi %[port], %[bit] \n\t"        // Set the output bit
        ".rept %[onCycles] \n\t"        // Now timing actually matters. The 0-bit must be long enough to be detected but not too long or it will be a 1-bit
        "nop \n\t"                                              // Execute NOPs to delay exactly the specified number of cycles
        ".endr \n\t"
        "cbi %[port], %[bit] \n\t"                              // Clear the output bit
        ".rept %[offCycles] \n\t"                               // Execute NOPs to delay exactly the specified number of cycles
        "nop \n\t"
        ".endr \n\t"
        ::
        [port]    "I" (_SFR_IO_ADDR(PIXEL_PORT)),
        [bit]   "I" (CONFIG::PIN_LED),
        [onCycles]  "I" (NS_TO_CYCLES(T0H) - 2),
        [offCycles] "I" (NS_TO_CYCLES(T0L) - 2)
        
      );    
  }
    
  // Note that the inter-bit gap can be as long as you want as long as it doesn't exceed the 5us reset timeout (which is A long time) 
  // Here I have been generous and not tried to squeeze the gap tight but instead erred on the side of lots of extra time.
  // This has thenice side effect of avoid glitches on very long strings becuase   
}  

  
inline void LitePixel::sendByte( unsigned char byte )
{    
  for( unsigned char bit = 0 ; bit < 8 ; bit++ ) {
    
    sendBit( bitRead( byte , 7 ) );                // Neopixel wants bit in highest-to-lowest order
                                                   // so send highest bit (bit #7 in an 8-bit byte since they start at 0)
    byte <<= 1;                                    // and then shift left so bit 6 moves into 7, 5 moves into 6, etc
    
  }           
} 

/*

  The following three functions are the public API:
  
  ledSetup() - set up the pin that is connected to the string. Call once at the begining of the program.  
  sendPixel( r g , b ) - send a single pixel to the string. Call this once for each pixel in a frame.
  show() - show the recently sent pixel on the LEDs . Call once per frame. 
  
*/


// Set the specified pin up as digital out

void LitePixel::setup()
{
  bitSet( PIXEL_DDR , CONFIG::PIN_LED ); 
  showColor(0,0,0); //Black the output.
}

inline void LitePixel::sendPixel( unsigned char r, unsigned char g , unsigned char b )
{  
  sendByte(g);          // Neopixel wants colors in green then red then blue order
  sendByte(r);
  sendByte(b);
}


// Just wait long enough without sending any bits to cause the pixels to latch and display the last sent frame

void LitePixel::show()
{
  delayMicroseconds(120);
}

// Display a single color on the whole string

void LitePixel::showColor( unsigned char r , unsigned char g , unsigned char b )
{
  cli();  
  for( int p=0; p<CONFIG::NUM_LEDS; p++ ) {
    sendPixel( r , g , b );
  }
  sei();
  show();
}
  
#endif
