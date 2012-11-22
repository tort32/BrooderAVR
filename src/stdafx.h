#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

#define setbits(port,mask)	(port)|=(mask)
#define clrbits(port,mask)	(port)&=~(mask)
#define tglbits(port,mask)	(port)^=(mask)
#define wrtbits(port,bits,mask) (port)=((port)&(~(mask)))|((bits)&(mask))

#define getbits(port,mask) ((port)&(mask))
#define is_bits(port,mask) (((port)&(mask))!=0)

#define HI(wd) ((BYTE)((0xff00&wd)>>8))
#define LO(wd) ((BYTE)(0x00ff&wd))

#define CHK_BUT(pin,mask) ((~(pin))&(mask))
