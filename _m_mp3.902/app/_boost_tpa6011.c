//Описание: реализация управления усилком TPA6011 



#include "includes.h"



//вкл усилка
void _init_boost()
{
#if 0 //FIX_DEBUG
  
//настройки для _BOOST_DAC valume
    PINSEL1 = 0x000080000;      /* Connect DAC out to P0.25 */    
//dec2hex(int32((2.59V*1024)/3.3)) расчетная формула    
    DACR = (0x0 << 6) & 0xFFC0; //*V на DAC

#if 1
//переключим mux усилка VS1001
          IO1DIR |= 0x00000001<<23; //as output P1.23 
          IO1SET |= 0x00000001<<23; //set P1.23
          
#endif
          
#if 1  
//P1.21(shutdown -> up)
    IO1DIR |= 0x00000001 << 21; //P1.21 на output
    //IO1SET |= (0x00000001 << 21); //down усилок  
    IO1CLR |= (0x00000001 << 21); //up усилок  
#endif

#endif    
}

//вкл усилок
void _on_boost()
{
#if 0//FIX_DEBUG
  
#if 1  
//P1.21(shutdown -> up)
    IO1DIR |= 0x00000001 << 21; //P1.21 на output
    //IO1SET |= (0x00000001 << 21); //down усилок  
    IO1CLR |= (0x00000001 << 21); //up усилок  
#endif   
    
#endif    
}

//вкл усилка для книги
void _init_boost_book()
{
#if 0//FIX_DEBUG
  
//dec2hex(int32((2.59V*1024)/3.3)) расчетная формула

    DACR = (_boost_inc << 6) & 0xFFC0; //*V на DAC
    //DACR = (0x03ff << 6) & 0xFFC0; //*V на DAC

#endif    
}


//усилок книги увел.
unsigned char _boost_book_inc(unsigned short _vol)
{
#if 0//FIX_DEBUG
  
//dec2hex(int32((2.59V*1024)/3.3)) расчетная формула
    if(_vol > (0x0290+0x00a0))
      return 0x01; //громче не можем
    //_vol += 0x000f;
    DACR = (_vol << 6) & 0xFFC0; //*V на DAC
    //DACR = (0x03ff << 6) & 0xFFC0; //*V на DAC
    return 0x0;
#else
    return 0x0;    
#endif
    
}


//усилок книги умен.
unsigned char _boost_book_dec(unsigned short _vol)
{
#if 0//FIX_DEBUG
    if(_vol < 0x00a0)
      return 0x01; //тише не можем     
//dec2hex(int32((2.59V*1024)/3.3)) расчетная формула
    //_vol -= 0x000f;
    DACR = (_vol << 6) & 0xFFC0; //*V на DAC
    //DACR = (0x03ff << 6) & 0xFFC0; //*V на DAC
    return 0x0;
#else
    return 0x0;    
#endif    
}




//вкл усилка для радио
void _init_boost_radio(unsigned short _vol)
{
#if 0//FIX_DEBUG
  
//dec2hex(int32((2.59V*1024)/3.3)) расчетная формула    
    DACR = (_vol << 6) & 0xFFC0; //*V на DAC
    
#endif
}

//вкл усилка для mp3
void _init_boost_mp3()
{
#if 0//FIX_DEBUG
  
//dec2hex(int32((2.59V*1024)/3.3)) расчетная формула    
    //DACR = (0x0300 << 6) & 0xFFC0; //*V на DAC
  
    DACR = (0x03ff << 6) & 0xFFC0; //*V на DAC
  
#endif
}


//выкл усилка
void _uninit_boost()
{
#if 0 //FIX_DEBUG 
   DACR = (0x0 << 6) & 0xFFC0; //*V на DAC
   IO1DIR &= ~(0x00000001 << 21); //P1.21 на input
#endif   
}

//тихий усилок
void _uninit_boost1()
{
#if 0  //FIX_DEBUG
   DACR = (0x0 << 6) & 0xFFC0; //*V на DAC
   
#endif   
}

//shutdown усилок
void _shutdown_boost1()
{
#if 0  //FIX_DEBUG
   IO1DIR &= ~(0x00000001 << 21); //P1.21 на input
   
#endif   
}

//усилок скакой то громкостью
void _init_boost1()
{
#if 0  //FIX_DEBUG
//настройки для _BOOST_DAC valume
    PINSEL1 = 0x000080000;      /* Connect DAC out to P0.25 */    
//dec2hex(int32((2.59V*1024)/3.3)) расчетная формула    
    DACR = (0x0136 << 6) & 0xFFC0; //*V на DAC


#endif   
}

