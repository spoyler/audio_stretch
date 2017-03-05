//Описание: реализация decrypt фрагментов



#include "includes.h"
#include "lpc_irq_fiq.h"

void _en_irq(void)
{
  enable_irq();
}

void _dis_irq(void)
{
  disable_irq_fiq(); //DEBUG
}

unsigned char K[]={0x27,0x4c,0xc1,0x8a,0xc1,0x5a,0x84,0x42,0xbb,0x06,0x65,0x13,0x66,0x7c,0xd4,0x05};


/* #define MX (z>>5^y<<2) + (y>>3^z<<4)^(sum^y) + (key[p&3^e]^z) */
#define MX  ( (((z>>5)^(y<<2))+((y>>3)^(z<<4)))^((sum^y)+(key1[(p&3)^e]^z)) )

//decrypt 512 bytes
long btea(long* v, long length)
{
    unsigned long z /* = v[length-1] */, y=v[0], sum=0, e, DELTA=0x9e3779b9;
    long p, q ;
    long* key1 = (long* )K;
    if (length > 1) {          /* Coding Part */
      z=v[length-1];           /* Moved z=v[length-1] to here, else segmentation fault in decode when length < 0 */
      q = 6 + 52/length;
      q=3;
      while (q-- > 0) {
        sum += DELTA;
        e = (sum >> 2) & 3;
        for (p=0; p<length-1; p++) 
        {
            y = v[p+1];
            z = v[p] += MX;
        }
        y = v[0];
        z = v[length-1] += MX;
      }
      return 0 ; 
    } else if (length < -1) {  /* Decoding Part */
      length = -length;
      q = 6 + 52/length;
      q=3;
      sum = q*DELTA ;
      while (sum != 0) {
        e = (sum >> 2) & 3;
        for (p=length-1; p>0; p--) z = v[p-1], y = v[p] -= MX;
        z = v[length-1];
        y = v[0] -= MX;
        sum -= DELTA;
      }
      return 0;
    }
    return 1;
  }



