//Описание: контроль за зарядкой батарейки, 
//контроль за U_POWER1,U_POWER2 зарядки


#include "includes.h"




//инит портов для мониторинга подключенной сети для зарядки
void _init_bat_monitor()
{
#if 0 //FIX_DEBUG 
//установим pins 
//P0.23 как output
//P0.29 как output
IO0DIR &= ~(0x00000001 << 23);
IO0DIR &= ~(0x00000001 << 29);
#endif
}
//PORT
//смотреть что на U_POWER1
unsigned char _get_U_POWER1()
{
#if 0 //FIX_DEBUG 
    if((IOCONF_I2S0_RX_PIN >> 1) & 0x00000001)
      return 0x01; //активно
#endif    
      
    return 0x0; //не активно
}

//смотреть что на U_POWER2
unsigned char _get_U_POWER2()
{
#if 0//FIX_DEBUG  
    if((IOCONF_I2S0_RX_PIN >> 0) & 0x00000001)
      return 0x01; //активно
#endif
    
    return 0x0; //не активно
}


//тест на зарядку через ADC
unsigned char _check1()
{
#if 0 //PORT
  if(((double)(3.3*((double)ADC_Read()/(double)0x03ff))+0.3) > 2.1)
    return 0x01;
#endif  
  return 0x0;
}

#define INT_32  unsigned int
/* ADC device handle */
static INT_32 adcdev;
static unsigned short _ADC_res;

static void program_adc(INT_32 dev,
                 INT_32 conv_mode,
                 INT_32 channel_0, INT_32 resolution_0,
                 INT_32 channel_1, INT_32 resolution_1,
                 INT_32 channel_2, INT_32 resolution_2,
                 INT_32 channel_3, INT_32 resolution_3)
{
#if 0   //PORT
  /* select channel  and resolution */
  if (channel_0 == ADC_SELECT_CHANNEL_0) adc_ioctl(dev, ADC_SELECT_CHANNEL_0, resolution_0);
  if (channel_1 == ADC_SELECT_CHANNEL_1) adc_ioctl(dev, ADC_SELECT_CHANNEL_1, resolution_1);
  if (channel_2 == ADC_SELECT_CHANNEL_2) adc_ioctl(dev, ADC_SELECT_CHANNEL_2, resolution_2);
  if (channel_3 == ADC_SELECT_CHANNEL_3) adc_ioctl(dev, ADC_SELECT_CHANNEL_3, resolution_3);

  /* select scan mode */
  if (conv_mode == ADC_SELECT_SINGLE_CONV_MODE) adc_ioctl(dev, ADC_SELECT_SINGLE_CONV_MODE, 0);
  if (conv_mode == ADC_SELECT_CONTINUOUS_SCAN_MODE) adc_ioctl(dev, ADC_SELECT_CONTINUOUS_SCAN_MODE, 0);

  /* start A/D convert */
  adc_ioctl(dev, ADC_START_CONVERSION, 0);

  /* stop A/D convert */
  adc_ioctl(dev, ADC_STOP_CONVERSION, 0);
#endif  
}

static unsigned short _ADC_result(INT_32 devid)
{
#if 0 //PORT 
  ADC_REGS_T *adcregsptr;
  ADC_CFG_T *adccfgptr = (ADC_CFG_T *) devid;
  adcregsptr = adccfgptr->regptr;
  //while (convdone == 0);
  while(1)
    if (adcregsptr->adc_csel_reg & 0x000000F0)
    {
      _ADC_res =  adccfgptr->rx[1];
      break;
    }
#endif
}




//тест на разрядку через ADC
unsigned char _check2()
{
#if 0  //PORT
  adcdev = adc_open((void *) ADC , 0);
  program_adc(adcdev,
              ADC_SELECT_SINGLE_CONV_MODE,
              ADC_SELECT_CHANNEL_0, 10,
              ADC_SELECT_CHANNEL_1, 10,
              ADC_SELECT_CHANNEL_2, 10,
              ADC_SELECT_CHANNEL_3, 10);

  poll_adc(adcdev);
  _ADC_result(adcdev);
  adc_close(adcdev);
  
  if(((double)(3.3*((double)_ADC_res/(double)0x03ff))) < 1.7)
    return 0x01;
#endif  
  return 0x0;
}


//тест на зарядку через ADC
unsigned char _check3()
{
#if 0 //PORT  
  adcdev = adc_open((void *) ADC , 0);
  program_adc(adcdev,
              ADC_SELECT_SINGLE_CONV_MODE,
              ADC_SELECT_CHANNEL_0, 10,
              ADC_SELECT_CHANNEL_1, 10,
              ADC_SELECT_CHANNEL_2, 10,
              ADC_SELECT_CHANNEL_3, 10);

  poll_adc(adcdev);
  _ADC_result(adcdev);
  adc_close(adcdev);
  
  if(((double)(3.3*((double)_ADC_res/(double)0x03ff))) > 2.2)
    return 0x01;
  
#endif  
  return 0x0;
}


unsigned short ADC_Read()
{
unsigned int i;
#if 0//FIX_DEBUG
  PINSEL1 |= 0x00000001 << 12;
  AD1CR = 0x00200380; // Init ADC (Pclk = 12MHz) and select AD1 7 ch
  AD1CR |= 0x01000000; // Start A/D Conversion
  do
  {

    i = AD1DR7;

    
  } while ((i & 0x80000000) == 0); // Wait for end of A/D Conversion
  
  
  return (i >> 6) & 0x03FF; // bit 6:15 is 10 bit AD value
#else
  return 0x03ff;  
#endif  
}