//��������: ���������� ���������� ������� TPA6011 



#include "includes.h"



//��� ������
void _init_boost()
{
#if 0 //FIX_DEBUG
  
//��������� ��� _BOOST_DAC valume
    PINSEL1 = 0x000080000;      /* Connect DAC out to P0.25 */    
//dec2hex(int32((2.59V*1024)/3.3)) ��������� �������    
    DACR = (0x0 << 6) & 0xFFC0; //*V �� DAC

#if 1
//���������� mux ������ VS1001
          IO1DIR |= 0x00000001<<23; //as output P1.23 
          IO1SET |= 0x00000001<<23; //set P1.23
          
#endif
          
#if 1  
//P1.21(shutdown -> up)
    IO1DIR |= 0x00000001 << 21; //P1.21 �� output
    //IO1SET |= (0x00000001 << 21); //down ������  
    IO1CLR |= (0x00000001 << 21); //up ������  
#endif

#endif    
}

//��� ������
void _on_boost()
{
#if 0//FIX_DEBUG
  
#if 1  
//P1.21(shutdown -> up)
    IO1DIR |= 0x00000001 << 21; //P1.21 �� output
    //IO1SET |= (0x00000001 << 21); //down ������  
    IO1CLR |= (0x00000001 << 21); //up ������  
#endif   
    
#endif    
}

//��� ������ ��� �����
void _init_boost_book()
{
#if 0//FIX_DEBUG
  
//dec2hex(int32((2.59V*1024)/3.3)) ��������� �������

    DACR = (_boost_inc << 6) & 0xFFC0; //*V �� DAC
    //DACR = (0x03ff << 6) & 0xFFC0; //*V �� DAC

#endif    
}


//������ ����� ����.
unsigned char _boost_book_inc(unsigned short _vol)
{
#if 0//FIX_DEBUG
  
//dec2hex(int32((2.59V*1024)/3.3)) ��������� �������
    if(_vol > (0x0290+0x00a0))
      return 0x01; //������ �� �����
    //_vol += 0x000f;
    DACR = (_vol << 6) & 0xFFC0; //*V �� DAC
    //DACR = (0x03ff << 6) & 0xFFC0; //*V �� DAC
    return 0x0;
#else
    return 0x0;    
#endif
    
}


//������ ����� ����.
unsigned char _boost_book_dec(unsigned short _vol)
{
#if 0//FIX_DEBUG
    if(_vol < 0x00a0)
      return 0x01; //���� �� �����     
//dec2hex(int32((2.59V*1024)/3.3)) ��������� �������
    //_vol -= 0x000f;
    DACR = (_vol << 6) & 0xFFC0; //*V �� DAC
    //DACR = (0x03ff << 6) & 0xFFC0; //*V �� DAC
    return 0x0;
#else
    return 0x0;    
#endif    
}




//��� ������ ��� �����
void _init_boost_radio(unsigned short _vol)
{
#if 0//FIX_DEBUG
  
//dec2hex(int32((2.59V*1024)/3.3)) ��������� �������    
    DACR = (_vol << 6) & 0xFFC0; //*V �� DAC
    
#endif
}

//��� ������ ��� mp3
void _init_boost_mp3()
{
#if 0//FIX_DEBUG
  
//dec2hex(int32((2.59V*1024)/3.3)) ��������� �������    
    //DACR = (0x0300 << 6) & 0xFFC0; //*V �� DAC
  
    DACR = (0x03ff << 6) & 0xFFC0; //*V �� DAC
  
#endif
}


//���� ������
void _uninit_boost()
{
#if 0 //FIX_DEBUG 
   DACR = (0x0 << 6) & 0xFFC0; //*V �� DAC
   IO1DIR &= ~(0x00000001 << 21); //P1.21 �� input
#endif   
}

//����� ������
void _uninit_boost1()
{
#if 0  //FIX_DEBUG
   DACR = (0x0 << 6) & 0xFFC0; //*V �� DAC
   
#endif   
}

//shutdown ������
void _shutdown_boost1()
{
#if 0  //FIX_DEBUG
   IO1DIR &= ~(0x00000001 << 21); //P1.21 �� input
   
#endif   
}

//������ ������ �� ����������
void _init_boost1()
{
#if 0  //FIX_DEBUG
//��������� ��� _BOOST_DAC valume
    PINSEL1 = 0x000080000;      /* Connect DAC out to P0.25 */    
//dec2hex(int32((2.59V*1024)/3.3)) ��������� �������    
    DACR = (0x0136 << 6) & 0xFFC0; //*V �� DAC


#endif   
}

