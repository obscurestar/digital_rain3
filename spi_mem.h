#include <SPI.h>
#include <SpiRAM.h>

#ifndef SPI_MEM_H  
#define SPI_MEM_H

#include "CONFIG.h"

/*This module uses the SPI bus to attach a 23K256 (32K) RAM module where we can 
 * store larger arrays than the arduino's 2K memory allows.
 */

 /*HARDWARE CONNECTIONS
  * The following pairs are the pins which are connected between the arduino and 
  * 23K256 chip  { [Arduino, 23K256], [10,1], [11,5], [12,2], [13,6] }
  * The 23K256 pins 7 and 8 connect to +5V, pin 4 to Ground and pin 3 is unused.
  */

#define MEMSIZE 32767
/*spiMem should be a singleton
 * spiMem is going to be our super-crude memory manager.  We'll refine it in future
 * iterations but for now let's just make it as simple as possible. 
 * Let's say byte 0 on our chip is reserved (largely because we'd like to model 
 * normal c++ memory usage as much as possible so we'll overload the meaning of
 * addr on our chip with the concept of NULL pointer. 
 */

byte clock = 0;
//SpiRAM SpiRam(0, CONFIG::PIN_SPIRAM);
SpiRAM SpiRam(0, 10);
namespace spiMem
{
  int next_addr=1;  /*For now pretend we never release memory*/
  
  int* spi_alloc(int num_bytes)
  {
    int* ret=NULL;
    int new_addr = next_addr + num_bytes;
    if ( (new_addr) < MEMSIZE) 
    {
      ret = malloc(sizeof(int));
      if (ret != NULL )
      {
        *ret = next_addr;
        next_addr = new_addr;
      }
    }
    return ret;
  }
};
#endif SPI_MEM_H
