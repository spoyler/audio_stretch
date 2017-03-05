#include "lpc32xx_gpio_driver.h"

#if 0
#define _BOOST_DOWN_SET  {IOCONF_EBI_I2STX_0_M1_SET = (1<<6);IOCONF_EBI_I2STX_0_M0_SET  = (1<<6);}
#define _BOOST_DOWN_CLR  {IOCONF_EBI_I2STX_0_M1_SET = (1<<6);IOCONF_EBI_I2STX_0_M0_CLR  = (1<<6);}

#define _BOOST_MUX_SET  {IOCONF_EBI_MCI_M1_SET = (1<<6);IOCONF_EBI_MCI_M0_SET  = (1<<6);}
#define _BOOST_MUX_CLR  {IOCONF_EBI_MCI_M1_SET = (1<<6);IOCONF_EBI_MCI_M0_CLR  = (1<<6);}


#define _BOOST_SILENCE_SET  {IOCONF_SPI_M1_SET = (1<<4);IOCONF_SPI_M0_SET  = (1<<4);}
#define _BOOST_SILENCE_CLR  {IOCONF_SPI_M1_SET = (1<<4);IOCONF_SPI_M0_CLR  = (1<<4);}


#define _LED_SET  {IOCONF_EBI_I2STX_0_M1_SET = (1<<5);IOCONF_EBI_I2STX_0_M0_SET  = (1<<5);}
#define _LED_CLR  {IOCONF_EBI_I2STX_0_M1_SET = (1<<5);IOCONF_EBI_I2STX_0_M0_CLR  = (1<<5);}


#define _TURNON_SET  {IOCONF_I2S0_RX_M1_SET = (1<<2);IOCONF_I2S0_RX_M0_SET  = (1<<2);}
#define _TURNON_CLR  {IOCONF_I2S0_RX_M1_SET = (1<<2);IOCONF_I2S0_RX_M0_CLR  = (1<<2);}
#endif

//PORT
void _booster(unsigned char _on)
{
  if(_on)
  {
    ;//_BOOST_DOWN_SET; 
  }
  else  
  {
    ;//_BOOST_DOWN_CLR; 
  }
}

void _booster_mux(unsigned char _on)
{
  if(_on)
  {
    ;//_BOOST_MUX_SET;
  }
  else  
  {
    ;//_BOOST_MUX_CLR;
  }
}

void _booster_silence(unsigned char _on)
{
  if(_on)
  {
    ;//_BOOST_SILENCE_SET;
  }
  else  
  {
    ;//_BOOST_SILENCE_CLR;
  }
}

extern void _led(unsigned char _on)
{
  if(_on)
  {
    ;//_LED_SET;
  }
  else  
  {
    ;//_LED_CLR;
  }
}

extern void _turnon(unsigned char _on)
{
  if(_on)
  {
    GPIO->p3_outp_set |= (0x1<<1); //signal _TURNON
  }
  else  
  {
    GPIO->p3_outp_clr |= (0x1<<1); //signal _TURNof
  }
}