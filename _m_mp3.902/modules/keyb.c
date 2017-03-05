//��������: ������ � ����������� 

//                  0 bit->|K12 GPIO17 GPIO 11 _B8  P0.12->button 4(����������� �����)
//                  1 bit->|H11 GPIO19 GPIO 13 RESET1 P0.15->button 1(�������� �����)
//                  2 bit->|K13 GPIO20 GPIO 14 _B11 P0.16->button 2(�������� ����) 
//                  3 bit->|J12 GPIO13 GPIO 7 _B4 P0.8->button 7(������� ��������/����� ������)
//                  4 bit->|J13 GPIO15 GPIO 9 _B61  P0.10->button 6(����/�����.)
//                  5 bit->|J14 GPIO14 GPIO 8 _B5 P0.9->button 9(����� ������)
//                  6 bit->|J11 GPIO16 GPIO 10 _B7  P0.11->button 5(������� ��������/����� �����)
//                  7 bit->|K14 GPIO18 GPIO 12 _B9  P0.13->button 3(��� ����. ����� N ���)
//                  8 bit->|H10 GPIO12 GPIO 6 _B3 P0.5->button 8(����������� ������)
//                  9 bit->|B11 GPIO3 GPIO 3 _B0  P0.1->button 12(��������� ����) 
//                  10 bit->|C11 GPIO4 GPIO 4 RESET2  P0.2->button 11(��������� �����)-> P0.30
//                  11 bit->|H13 GPIO11 GPIO 5 _B2  P0.3->button 10(�����) -> P0.0


#define _BUTTON_SIZE  12

#include "lpc313x_timer_driver.h"
#include "includes.h"

#define _LONG_PRESS 10//25 //_LONG_PRESS*50ms ������� ������ ������� ������

#define _IO1_16_MASK   0x00000001 << 16 

//array ���������� ��� ������ ������(����� ��� ������� ����� �������)
//0-10 bits -> ������ ������ ?ms �����
//������ ������ �� ������ ������ ���� bit set
volatile static unsigned short _key_press=0x0000;
//0-10 bits -> ������ ���� ������ ����� ���� bit set
volatile static unsigned short _key_pressed=0x0000;
//0-10 bits -> ������ ���� ����� ������ ����� ���� bit set
volatile static unsigned short _key_long_pressed=0x0000;
//��� 0-10 bits �� _key_pressed ������� 50ms ticks ����� bit ��� ������ set
volatile static unsigned char  _key_pressed_counter[_BUTTON_SIZE];

//��� �������
volatile static unsigned short _key_pressed_seen=0x0000;
/* Timer device handles */
static INT_32 timer0dev;


void _set_curr_keys(void);
extern unsigned char  _volume_up(void);
extern unsigned char  _volume_down(void);



//every ~70ms ����� �������� �� ������
static void timer0_user_interrupt(void)
{
  static int state = 0;
  /* Clear latched timer interrupt */
  timer_ioctl(timer0dev, TMR_CLEAR_INTS, 1);

  _set_curr_keys();
#if 0  
  /* Toggle mode0 LED */
  if (state)
    _led(0x01);
  else
    _led(0x0);

  state ^= 1;
#endif
}


/* Setup the Timer Counter 1 Interrupt */
static void init_timer1()
{
/* Install timer interrupts handlers as a IRQ interrupts */
  int_install_irq_handler(IRQ_TIMER0, (PFV) timer0_user_interrupt);

/* Open timers - this will enable the clocks for the timers */
  timer0dev = timer_open((void *)TIMER_CNTR0, 0);


//every ~70ms ����� �������� �� ������
  timer_ioctl(timer0dev, TMR_SET_MSECS, 70);
  timer_ioctl(timer0dev, TMR_SET_PERIODIC_MODE, 0);
/* Clear any latched timer 0 interrupts and enable match
   interrupt */
  timer_ioctl(timer0dev, TMR_CLEAR_INTS, 1);
  /******************************************************************/

  timer_ioctl(timer0dev, TMR_ENABLE, 1);
  int_enable(IRQ_TIMER0);

}


//����. ������ ��� �����
//�������� ����� �� ���� array's ��� ������� ������� ������
unsigned char _var1=0x00,_var2=0x00;  
extern void keyb_Init(void)
{

//��������� pins as input   
IOCONF_GPIO_M0_CLR = (0x0fff<<3);
IOCONF_GPIO_M1_CLR = (0x0fff<<3);

IOCONF_I2S0_RX_M0_CLR = (0x0001<<1); //M10(U_POWER1)
IOCONF_I2S0_RX_M1_CLR = (0x0001<<1); //M10(U_POWER1)
IOCONF_I2S0_RX_M0_CLR = (0x0001<<0); //N10(U_POWER2)
IOCONF_I2S0_RX_M1_CLR = (0x0001<<0); //N10(U_POWER2)


//����. timer �� 100ms
init_timer1();

//������� �������� 70ms ticks
memset((void *)_key_pressed_counter,0x0,_BUTTON_SIZE);
_key_press=0x0000;
_key_pressed=0x0000;
_key_long_pressed=0x0000;

//��� �������
volatile static unsigned short _key_pressed_seen=0x0000;
#if 0
//DEBUG
while(1)
{

  _set_curr_keys();
  /*_var1 = _volume_up();
  if(_var1)
    _var1 = 0x0;
  _var2 = _volume_down();
  if(_var2)
    _var2 = 0x0;*/
  
}

#endif
}


//��������� ����� ������� ������ � �������� ������ ����� ������� ������
static void _set_curr_keys(void)
{
unsigned  char _var;
unsigned short _format_keys=0x0000; // bits ������ � ������� 11-0
//����� ��������� 5��� �� ������ �������� �� ����
static unsigned short _start_stop=0;

//                  0 bit->|K12 GPIO17 GPIO 11 _B8  P0.12->button 4(����������� �����)
//                  1 bit->|H11 GPIO19 GPIO 13 RESET1 P0.15->button 1(�������� �����)
//                  2 bit->|K13 GPIO20 GPIO 14 _B11 P0.16->button 2(�������� ����) 
//                  3 bit->|J12 GPIO13 GPIO 7 _B4 P0.8->button 7(������� ��������/����� ������)
//                  4 bit->|J13 GPIO15 GPIO 9 _B61  P0.10->button 6(����/�����.)
//                  5 bit->|J14 GPIO14 GPIO 8 _B5 P0.9->button 9(����� ������)
//                  6 bit->|J11 GPIO16 GPIO 10 _B7  P0.11->button 5(������� ��������/����� �����)
//                  7 bit->|K14 GPIO18 GPIO 12 _B9  P0.13->button 3(��� ����. ����� N ���)
//                  8 bit->|H10 GPIO12 GPIO 6 _B3 P0.5->button 8(����������� ������)
//                  9 bit->|B11 GPIO3 GPIO 3 _B0  P0.1->button 12(��������� ����) 
//                  10 bit->|C11 GPIO4 GPIO 4 RESET2  P0.2->button 11(��������� �����)-> P0.30
//                  11 bit->|H13 GPIO11 GPIO 5 _B2  P0.3->button 10(�����) -> P0.0

//IOCONF_GPIO_PIN  mask 0x0fff<<3
   _format_keys |= ((IOCONF_GPIO_PIN >> 11) << 0) & 0x0001; //P0.12 �� 0 bit
   _format_keys |= ((IOCONF_GPIO_PIN >> 13) << 1) & 0x0002; //P0.15 �� 1 bit
   _format_keys |= ((IOCONF_GPIO_PIN >> 14) << 2) & 0x0004; //P0.16 �� 2 bit
   _format_keys |= ((IOCONF_GPIO_PIN >> 7)  << 3) & 0x0008; //P0.8 �� 3 bit
   _format_keys |= ((IOCONF_GPIO_PIN >> 9)  << 4) & 0x0010; //P0.10 �� 4 bit
   _format_keys |= ((IOCONF_GPIO_PIN >> 8)  << 5) & 0x0020; //P0.9 �� 5 bit
   _format_keys |= ((IOCONF_GPIO_PIN >> 10) << 6)  & 0x0040; //P0.11 �� 6 bit
   _format_keys |= ((IOCONF_GPIO_PIN >> 12) << 7)  & 0x0080 ; //P0.13 �� 7 bit
   _format_keys |= ((IOCONF_GPIO_PIN >> 6)  << 8)  & 0x0100; //P0.5 �� 8 bit
   _format_keys |= ((IOCONF_GPIO_PIN >> 3)  << 9)  & 0x0200; //P0.1 �� 10 bit
   _format_keys |= ((IOCONF_GPIO_PIN >> 4)  << 10)  & 0x0400; //P0.2 �� 9 bit -> P0.30
   _format_keys |= ((IOCONF_GPIO_PIN >> 5)  << 11)  & 0x0800; //P0.3 �� 11 bit -> P0.0

  _key_pressed |= _key_press & _format_keys; //���� ����� 50ms bit ������ �������� '1' �� ������� ������ �������
  _key_press = _format_keys; //������� �����. ��������� bits ������ 
  
//����� inc ������� ��� ������ bit 0-11
//���� ������ � ������ �� ������ ������ � pressed_counter < 20

  for(_var=0x0; _var < _BUTTON_SIZE; _var++)
  {
    if(_key_pressed_counter[_var] < (unsigned char)_LONG_PRESS)
    {
      _key_pressed_counter[_var] += (((_key_pressed & _key_press) >> _var) & 0x0001);
    }
  }
  

//���� ����� ���� ��������� � ������ ��� � � ����� �� ����� ���������, ������ ����� ���� Timer  
  if(m_sleep._mode != 0x0)
    m_sleep._max_time++;

//� ���� ����� ���� ��������� � ������ ����� ������ stop � �� 10��� ���� ������ ��� ����-��
  if(m_wait_stop != 0x0)
   if(m_ws_counter < (0x1a00>>2)) //1.7min
    m_ws_counter++;

#if 1  
//���� ����� ���� ���������� � ������ ����� ���� ��������� � ��� ����� ��������� 1���
  _start_stop++;
  if(_start_stop == 100)
    m_env20_=0x01; //����� ������������ stop
  
  
  if((m_env7 & 0x02)==0x02)
  {

      if((m_time_play%6)!=0)
        m_time_play++;
  }
  
  if((m_env7 & 0x08)==0x08)
  {
      if((m_time_play%6)!=0)
        m_time_play++;
  }

  if((m_env10 & 0x02)==0x02)
  {
      if((m_time_play%16)!=0)
        m_time_play++;
  }
#endif  
//--------------------
  
}

unsigned char _var1;
//�� bit ������ ����: 0 bit -> set -> ���� ������
//                    1 bit -> set -> �� ������ ������ ������
//                    2 bit -> set -> ���������� ������� 
//                    3 bit -> set -> ���� ���������� �������       
extern unsigned char _get_status_bit(unsigned char _bit)
{

    if((_bit != 0x09) && (_bit != 0x0a) && (_bit != 0x01) && (_bit != 0x02))
    {
      //T1MCR &= 0xfffe; //�������� interrupt �� Timer1
      if((_key_pressed_counter[_bit] == _LONG_PRESS) && !((_key_long_pressed >> _bit)& 0x0001))
      {
        _var1 = (((_key_press >> _bit) & 0x0001) << 1) | ((_key_pressed >> _bit) & 0x0001) | 0x004;
        _key_pressed_counter[_bit] = 0x0;
        _key_long_pressed |= (0x0001 << _bit);
      }
      else
        _var1 = (((_key_long_pressed >> _bit) & 0x0001) << 3) | (((_key_press >> _bit) & 0x0001) << 1) | ((_key_pressed >> _bit) & 0x0001);
    }
    else
    {
      //T1MCR &= 0xfffe; //�������� interrupt �� Timer1
      if(_key_pressed_counter[_bit] == _LONG_PRESS)
      {
        _var1 = (((_key_press >> _bit) & 0x0001) << 1) | ((_key_pressed >> _bit) & 0x0001) | 0x004;
        _key_pressed_counter[_bit] = 0x0;
        _key_long_pressed |= (0x0001 << _bit);
      }
      else
        _var1 = (((_key_long_pressed >> _bit) & 0x0001) << 3) | (((_key_press >> _bit) & 0x0001) << 1) | ((_key_pressed >> _bit) & 0x0001);
    }
      
    
    if(((((_key_press >> _bit) & 0x0001) << 1) | ((_key_pressed >> _bit) & 0x0001)) == 0x01)
    {
      _key_pressed &= ~(0x0001 << _bit); //�������, ��� ������ ���� ������
      _key_pressed_counter[_bit] = 0x0;
      _key_long_pressed &= ~(0x0001 << _bit); //�������, ��� ������ ���� ������ ����� �� ���� ������
    }
    //T1MCR |= 0x0001; //���. interrupt �� Timer1
    
    return _var1;    
}



//��������� ��������� P0.4->button 6(����/�����.)
extern unsigned char  _play_stop(void)
{
unsigned char _var;

    if(m_env24_)
      return 0x0; //�� ������

   _var = _get_status_bit(0x004);
    if(((_var & 0x0f) == 0x07))
      return 0x02; //���� ������� �������
   
    if(((_var & 0x0f) == 0x01))
      return 0x01; //���� �������� �������

    
    if(((_var & 0x02) == 0x00))
      return 0x03; //������ ������ �� ������
    
  
  
  return 0x0; //�� ������
}


//��������� ��������� P0.5->button 9(����� ������)
extern unsigned char  _mode(void)
{
unsigned char _var;

    if(m_env24_)
      return 0x0; //�� ������

   _var = _get_status_bit(0x005);
    if(((_var & 0x0f) == 0x07))
      return 0x02; //���� ������� �������
    
    if(((_var & 0x0f) == 0x0B))
      return 0x04; //���� ������� �������    
   
    if(((_var & 0x0f) == 0x01))
      return 0x01; //���� �������, ������ ������ �� ������

    if(!_int_play())
    if(((_var & 0x02) == 0x00))
      return 0x03; //���� �������, ������ ������ �� ������
    


    
    /*if(((_var & 0x02) == 0x00))
      return 0x03; //������ ������ �� ������*/
  
  
  return 0x0; //�� ������
}


//��������� ��������� P0.8->button 8(����������� ������ � ���������)
extern unsigned char  _forward_frag(void)
{
unsigned char _var;

    if(m_env24_)
      return 0x0; //�� ������


   _var = _get_status_bit(0x008);
    if(((_var & 0x0f) == 0x01))
      return 0x03; //���� �������, ������ ������ �� ������
   
    if(((_var & 0x02) == 0x00))
      return 0x01; //���� �������, ������ ������ �� ������

    if(((_var & 0x0f) == 0x07))
      return 0x02; //���� ������� �������

    
    
  return 0x0; //�� ������
}


//��������� ��������� P0.0->button 4(����������� ����� � ���������)
extern unsigned char  _prev_frag(void)
{
unsigned char _var;

    if(m_env24_)
      return 0x0; //�� ������

   _var = _get_status_bit(0x000);
    if(((_var & 0x0f) == 0x01))
      return 0x03; //���� �������, ������ ������ �� ������
   
    if(((_var & 0x02) == 0x00))
      return 0x01; //���� �������, ������ ������ �� ������

    if(((_var & 0x0f) == 0x07))
      return 0x02; //���� ������� �������
    
  return 0x0; //�� ������
}


//��������� ��������� P0.3->button 7(������� ��������/����� ������)
extern unsigned char  _forward_book(void)
{
unsigned char _var;

    if(m_env24_)
      return 0x0; //�� ������

   _var = _get_status_bit(0x003);
    if(((_var & 0x0f) == 0x01))
      return 0x01; //���� �������, ������ ������ �� ������

    if(((_var & 0x0f) == 0x07))
      return 0x02; //���� ������� �������

    
    if(((_var & 0x02) == 0x00))
      return 0x03; //������ ������ �� ������
    
    
  return 0x0; //�� ������
}


//��������� ��������� P0.6->button 5(������� ��������/����� �����)
extern unsigned char  _prev_book(void)
{
unsigned char _var;

    if(m_env24_)
      return 0x0; //�� ������

   _var = _get_status_bit(0x006);
    if(((_var & 0x0f) == 0x01))
      return 0x01; //���� �������, ������ ������ �� ������

    if(((_var & 0x0f) == 0x07))
      return 0x02; //���� ������� �������

    
    if(((_var & 0x02) == 0x00))
      return 0x03; //������ ������ �� ������
  
  return 0x0; //�� ������
}


//��������� ��������� P0.7->button 3(��� ����. ����� N ���)
extern unsigned char  _sleep(void)
{
unsigned char _var;

    if(m_env24_)
      return 0x0; //�� ������


   _var = _get_status_bit(0x007);
    if(((_var & 0x0f) == 0x01))
      return 0x01; //���� �������� �������
    
    if(((_var & 0x0f) == 0x0B))
      return 0x02; //���� ������� ������� 
    
    if(((_var & 0x0f) == 0x07))
      return 0x03; //���� ������� �������    
  
    if(((_var & 0x02) == 0x00))
      return 0x04; //���� �������, ������ ������ �� ������    
    
  return 0x0; //�� ������
}


//��������� ��������� P0.10->button 11(��������� ����)
extern unsigned char  _volume_up(void)
{
unsigned char _var;



   _var = _get_status_bit(0x009);
    if(((_var & 0x0f) == 0x07))
      return 0x01; //���� ������� �������
    
    if(((_var & 0x0f) == 0x01))
      return 0x01; //���� �������� �������
#if 0
    if(((_var & 0x08) == 0x08))
      return 0x01; //������� ������� ������������
#endif    
    
  return 0x0; //�� ������
}


//��������� ��������� P0.9->button 12(��������� �����) 
extern unsigned char  _volume_down(void)
{
unsigned char _var;

    if(m_env24_)
      return 0x0; //�� ������
    
   _var = _get_status_bit(0x00a);
    if(((_var & 0x0f) == 0x07))
      return 0x02; //���� �������, ������ ������ �� ������

    if(((_var & 0x0f) == 0x01))
      return 0x01; //���� �������� �������
#if 0
    if(((_var & 0x08) == 0x08))
      return 0x01; //������� ������� ������������    
#endif
    
  return 0x0; //�� ������
}



//��������� ��������� P0.1->button 1(�������� �����)
extern unsigned char  _fast_up(void)
{
unsigned char _var;

    if(m_env24_)
      return 0x0; //�� ������

   _var = _get_status_bit(0x001);
    if(((_var & 0x0f) == 0x07))      
#if 0      
      return 0x01; //���� �������, ������ ������ �� ������
#else    
      return 0x02; //������� �������
#endif    

    if(((_var & 0x0f) == 0x01))
      return 0x01; //���� �������� �������

#if 0 
    if(((_var & 0x08) == 0x08))
      return 0x02; //������� ������� ������������
#endif    
    
  return 0x0; //�� ������
}


//��������� ��������� P0.2->button 2(�������� ����) 
extern unsigned char  _fast_down(void)
{
unsigned char _var;

    
   _var = _get_status_bit(0x002);
    if(((_var & 0x0f) == 0x07))
      return 0x02; //���� �������, ������ ������ �� ������
#if 1
    if(((_var & 0x08) == 0x08))
      return 0x02; //������� ������� ������������
#endif 
    
    if(((_var & 0x0f) == 0x01))
      return 0x01; //���� �������� �������

#if 0    
    if(((_var & 0x02) == 0x00))
      return 0x03; //������ ������ �� ������
#endif    
  
  return 0x0; //�� ������
}


//��������� ��������� P0.3->button 10(�����) 
extern unsigned char  _options(void)
{
unsigned char _var;

    if(m_env24_)
      return 0x0; //�� ������

   _var = _get_status_bit(0x00B);
   
    if(((_var & 0x0f) == 0x0B))
      return 0x02; //���� ������� �������  
    
    if(((_var & 0x0f) == 0x07))
      return 0x01; //���� �������, ������ ������ �� ������

    if(((_var & 0x0f) == 0x01))
      return 0x01; //���� �������� �������

  
  return 0x0; //�� ������
}


//�������� ������
extern void _bcheck(void)
{
unsigned char _res;  

while(1)
{
//�������� �������� ����:
// 0 bit->|P0.12->button 4(����������� �����) ->������� 1 ���� ���� �������� �������
  _res = _prev_frag();
  if(_res==0x03)
  {
    for(unsigned char _i=0;_i<0x01; _i++)
    {
      _led(0x0);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
      _led(0x01);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
    }
  }


// 1 bit->|P0.15->button 1(�������� �����) ->������� 2 ����
  _res = _fast_up();
  if(_res==0x01)
  {
    for(unsigned char _i=0;_i<0x02; _i++)
    {
      _led(0x0);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
      _led(0x01);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
    }
  }
  
// 2 bit->|P0.16->button 2(�������� ����) ->������� 3 ����
  _res = _fast_down();
  if(_res==0x01)
  {
    for(unsigned char _i=0;_i<0x03; _i++)
    {
      _led(0x0);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
      _led(0x01);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
    }
  }
  
// 3 bit->|P0.8->button 7(������� ��������/����� ������) ->������� 4 ����
  _res = _forward_book();
  if(_res==0x01)
  {
    for(unsigned char _i=0;_i<0x04; _i++)
    {
      _led(0x0);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
      _led(0x01);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
    }
  }
  
// 4 bit->|P0.10->button 6(����/�����.) ->������� 5 ����
  _res = _play_stop(); //�� ��������
  if(_res==0x01)
  {
    for(unsigned char _i=0;_i<0x05; _i++)
    {
      _led(0x0);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
      _led(0x01);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
    }
#if 0   
    //DEBUG
    IO1DIR &= ~_IO1_16_MASK; //P1.16 �� input, �������� ����
#endif    
    
  }
  
// 5 bit->|P0.9->button 9(����� ������) ->������� 6 ����
  _res = _mode();
  if(_res==0x01)
  {
    for(unsigned char _i=0;_i<0x06; _i++)
    {
      _led(0x0);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
      _led(0x01);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
    }
  }
  
// 6 bit->|P0.11->button 5(������� ��������/����� �����) ->������� 7 ����
  _res = _prev_book();
  if(_res==0x01)
  {
    for(unsigned char _i=0;_i<0x07; _i++)
    {
      _led(0x0);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
      _led(0x01);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
    }
  }
  
// 7 bit->|P0.13->button 3(��� ����. ����� N ���) ->������� 8 ����
  _res = _sleep();
  if(_res==0x01)
  {
    for(unsigned char _i=0;_i<0x08; _i++)
    {
      _led(0x0);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
      _led(0x01);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
    }
  }  
// 8 bit->|P0.5->button 8(����������� ������) ->������� 9 ����
 _res = _forward_frag();
  if(_res==0x3)
  {
    for(unsigned char _i=0;_i<0x09; _i++)
    {
      _led(0x0);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
      _led(0x01);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
    }
  }    
// 9 bit->|P0.1->button 12(��������� ����) ->������� 10 ���� 
 _res = _volume_up(); //�� ��������
  if(_res==0x1)
  {
    for(unsigned char _i=0;_i<0x0a; _i++)
    {
      _led(0x0);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
      _led(0x01);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
    }
  }      
// 10 bit->|P0.2->button 11(��������� �����)-> P0.30 ->������� 11 ����
 _res = _volume_down();
  if(_res==0x1)
  {
    for(unsigned char _i=0;_i<0x0b; _i++)
    {
      _led(0x0);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
      _led(0x01);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
    }
  }        
// 11 bit->|P0.3->button 10(�����) -> P0.0 ->������� 12 ����
 _res = _options();
  if(_res==0x1)
  {
    for(unsigned char _i=0;_i<0x0c; _i++)
    {
      _led(0x0);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
      _led(0x01);
      for(unsigned long _p=0;_p<0x005fffff;_p++)
        asm("NOP");
    }
  }        
}  
}

void _keyb_reset(void)
{
  memset((void *)_key_pressed_counter,0x0,_BUTTON_SIZE);
  _key_press=0x0000;
  _key_pressed=0x0000;
  _key_long_pressed=0x0000;
}










