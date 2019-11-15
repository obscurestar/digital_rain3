/* pixelarray.h
 *  Manages an array of pixel values for our RGB array. 
 *  If #define USE_SPI_PIX is set, will store the array in
 *  SPI RAM.  Otherwise it will use arduino memory.  
 *  See CONFIG.h
 */
#ifndef PIXELARRAY_H  
#define PIXELARRAY_H

#include "CONFIG.h"
#include "spi_mem.h"

 /*pix is 4 bytes per LED. Remember our 328 CPU only has 2K of active memory.
   * if you have 512 pix, that's all of the memory including the memory for calling functions
   * making computations, talking to our libraries, etc. 
   * We will address this in a later version by using a 32K expansion chip on the IC2 bus.
   */
class PixelArray
{
private:
  int mNumPix;  //Number of pixels in array
  bool mUseSPI; //Set true if using SPI memory to hold our array.
  bool mUseUpdateFlags; //If true, store changes to limit writing to pix array.
  bool mUseSPIFlags; //True to use SPI mem for flags.
  COLOR* mMemAddr; //Base address in memory where our array is held.
  int*  mUFAddr;  //Base address for update flags array.
public:
  PixelArray(int num_pix, bool use_spi_mem, bool use_update_flags, bool use_spi_flags );
  void set(int id, COLOR color);
  void set(int id, int col, byte val);
  void set(int id, byte r, byte g, byte b);
  COLOR get(int id);
  void flushFlags();
  void flag(int id);
  byte getFlags(int id);
private:
  COLOR* memAlloc( int num_pix );
  COLOR* spiAlloc( int num_pix );
};


COLOR* PixelArray::memAlloc( int num_pix )
{
  COLOR* ret = NULL;
  /*TODO 256 is terrible constant and this is a bad place for it.
   * None the less, I want to do a bare first pass and not overflowing
   * memory here.  I know COLOR is4 bytes and 256x4=1024 or half the 
   * available memory so let's limit to that for now.  We can tune this
   * later if it ever comes up.
   */
  if (num_pix > 256)  
  {
    return ret;  //Too big, return NULL ptr;
  }
  return (COLOR*)malloc(sizeof(COLOR) * num_pix);
}

COLOR* PixelArray::spiAlloc( int num_pix )
{
  COLOR* ret = NULL;
  return (COLOR*)spiMem::spi_alloc( sizeof(COLOR) * num_pix );
}

PixelArray::PixelArray(int num_pix, bool use_spi_mem, bool use_update_flags, bool use_spi_flags)
{
  mNumPix = num_pix;
  mUseUpdateFlags = use_update_flags;
  mUseSPIFlags = use_spi_flags;
  mUseSPI = use_spi_mem;
  /*TODO:  Right now we have nothing else in memory on our SPI RAM chip so 
   * we're just going to set our base address to 0.   This means we're doing 
   * some extra computation on derferencing pixels, but in the future we may 
   * want to put more things in our auxiallary memory, but let's put off writing
   * our own memory manager for a later version.  Right now we want light!
   */
   if (mUseSPI)
   {
      mMemAddr = spiAlloc( sizeof(COLOR) * mNumPix );
   }
   else
   {
      mMemAddr = memAlloc (sizeof(COLOR) * mNumPix);
   }
   if (!mMemAddr) //Allocation failed
   {
      //??? what are we going to do for error handling?
      //TODO decide allocate fail behavior and fail behavior in general. 
   }

  if (mUseUpdateFlags)
  {
    if (mUseSPIFlags)
    {
     /*The bool type 
      * is actually stored as an int internally.  2 bytes for every bit.
      * What we'll do instead is create a bit-to-bit mapping using divide
      * and modulo to address our mask flags.
      * We'll start that process here by allocating the memory.  Ceil is 
      * divide and round up.  So 7/8=1, 8/8=1, 9/8=2 etc 
      */
      mUFAddr = spiMem::spi_alloc( ceil(mNumPix/8) );
    }
    else
    {
      mUFAddr = (int *)malloc( ceil(mNumPix/8) ); //See above comment.
    }
    if (!mUFAddr) //Allocation failed
    {
       //This isnt' fatal, it will just make our updates slow.
       //TODO warn the developer the allocation failed.
       mUseUpdateFlags = false;  //If no memory addr, don't use.
    }
  }
}

void PixelArray::flag(int id)
{
  static int oidx=0xFFFF;
  static byte flags;
  if (mUseUpdateFlags)
  {
    int idx1, idx2;
    idx1 = id/8;
    idx2 = id%8;

    if (idx1 != oidx)
    {
      if (mUseSPIFlags)
      {
        if (oidx!=0xFFFF)
        {
          SpiRam.write_stream(*mUFAddr + oidx, &flags, 1 );
        }
        //Read bitmask for 8 sequential pixels
        SpiRam.read_stream(*mUFAddr + idx1, flags, 1);
      }
      else
      {
        if (oidx!=0xFFFF)
        {
           mUFAddr[oidx] = flags;
        }
        flags=mUFAddr[idx1];
      }

      oidx = idx1;

      /*Take a 1, shift it over the modulo bits, logically OR it with 
       * the bitmask, and write it back out.  
       * Say we are changing pixel 3.  Because idx1 is an int, 
       * 3/8=0 so we'll be getting the 0th block, pixels 0-7
       * We'll then take 1 and shift it 3 places to binary 1000
       * Logical OR (|) basically gives you all the 1s from 2 byte arrays.
       * IE 10101010 | 01010101 = 11111111; 11101111 | 10101010 = 11101111
       */
      flags = flags | (1<<idx2);
    }
  }
}

void PixelArray::set(int id, COLOR col)
{
  int csize = sizeof(COLOR);

  if (mUseSPI)
  {
/*In C++ you can do ANYTHING you want.  With this great power should come
* great responsibility, but sometimes you are going to see code that looks 
* like this.  Power blatantly abused for reasons of dubious value.  
*
*Data is data.  We're casting our *COLOR memory address to an integer. 
*NOTE: using int rather than short is least-fragile here.  If we went to a 
*system where int is defined as long, this awful conversion still works. 
*So, we're getting the number we stashed there as an index into spi mem
*and adding to it the byte offset of the pixel ID times the sizeof our 
*color data.
*Next, we are doing some ugly type conversion, casting the address of 
*our local variable to a byte array pointer. 
*/
      SpiRam.write_stream( (int*)mMemAddr + ( id * csize ),
                          (byte*)(&col), csize);
  }
  else
  {
      *(mMemAddr + id) = col;
  }
  flag(id);
}

void PixelArray::set(int id, int col, byte val)
{
  COLOR foo = get(id);
  foo.c[col] = val;
  set(id, foo);
}

void PixelArray::set(int id, byte r, byte g, byte b)
{
  COLOR foo;
  foo.c[0] = r;
  foo.c[1] = g;
  foo.c[2] = b;
  set(id, foo);
}

COLOR PixelArray::get(int id)
{
  COLOR col;
  int csize = sizeof(long);

  if (mMemAddr)
  {
    if (mUseSPI)
    {
      SpiRam.read_stream((int *)mMemAddr + (id *csize), (byte*)(&col), csize);
    }
    else
    {
      col.l = mMemAddr[id].l;
    }
  }
  return col;
}

void PixelArray::flushFlags()
{
  if (mUseUpdateFlags)
  {
    int sz = ceil(mNumPix/8);
    if (mUseSPI)
    {
      for (int i=0;i<sz;++i)
      {
        SpiRam.write_stream(*mUFAddr + i, 0, 1);
      }
    }
    else
    {
      memset(mUFAddr, 0, sz);
    }
  }
}

byte PixelArray::getFlags(int id)
{
  byte flags = 0xFF; //Return true if not using updateflags.

  if (mUseUpdateFlags)
  {
    int idx1, idx2;
    idx1 = id/8;
    idx2 = id%8;
    //Read bitmask for 8 sequential pixels
    SpiRam.read_stream(*mUFAddr + idx1, flags, 1);
  }
  return flags;
}

#endif //PIXELARRAY_H
