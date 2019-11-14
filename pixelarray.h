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
  COLOR* mMemAddr; //Base address in SPI memory where our array is held.
public:
  PixelArray(int num_pix, bool use_spi_mem );
  void set(int id, COLOR color);
  void set(int id, int col, byte val);
  void set(int id, byte r, byte g, byte b);
  COLOR get(int id);
private:
  COLOR* memAlloc( int num_pix );
  COLOR* spiAlloc( int num_pix );
};

COLOR* PixelArray::memAlloc( int num_pix )
{
  COLOR* ret = NULL;
  /*TODO 26 is terrible constant and this is a bad place for it.
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

PixelArray::PixelArray(int num_pix, bool use_spi_mem)
{
  mNumPix = num_pix;
  mUseSPI = use_spi_mem;
  /*TODO:  Right now we have nothing else in memory on our SPI RAM chip so 
   * we're just going to set our base address to 0.   This means we're doing 
   * some extra computation on derferencing pixels, but in the future we may 
   * want to put more things in our auxiallary memory, but let's put off writing
   * our own memory manager for a later version.  Right now we want light!
   */
   if (mUseSPI)
   {
      mMemAddr = spiAlloc( mNumPix );
   }
   else
   {
      mMemAddr = memAlloc (mNumPix);
   }
   if (!mMemAddr) //Allocation failed
   {
      Serial.println("!malloc");
      //??? what are we going to do for error handling?
      //TODO decide allocate fail behavior and fail behavior in general. 
   }
}

void PixelArray::set(int id, COLOR col)
{
  int csize = sizeof(COLOR);
  if (mMemAddr)
  {
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
  }
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
#endif //PIXELARRAY_H
