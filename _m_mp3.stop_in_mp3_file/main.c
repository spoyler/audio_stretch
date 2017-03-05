/*************************************************************************
 *
 *   Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : main.c
 *    Description :
 *
 *   This example project shows how to use the IAR Embedded Workbench for ARM
 *  to develop code for a Embedded Artsists LPC313x evaluation board. It shows
 *  basic use of I/O, Timers, ADC, Interrup controller, SPI and SDRAM. The projec
 *  also shows how the system can boot from the SPI NOR Flash.
 *  It starts by blinking CPIO2_BOOT2 LED. The blinking period can be changed
 *  by the trimer
 *
 *COMPATIBILITY
 *=============
 *
 *   The Getting Started example project is compatible with Embedded Artsists
 *  LPC313x evaluation board. By default the project is configured to use the
 *  J-Link JTAG interface.
 *
 *CONFIGURATION
 *=============
 *
 *  The Project contains the following configurations:
 *
 *  Debug_iRAM: This configuration is intended for debugging in the
 *              microcontroller's internal RAM.
 *
 *  Debud_SDRAM: This configuration is intended for debugging in the external
 *               SDRAM.
 *
 *  Debug_SPINOR_Boot_iRAM: This confuguration will download the example in the
 *                          SPI NOR flash. The application is linked to run in
 *                          the internal RAM and is loaded by LPC IROM
 *                          bootloader. The configuration uses
 *                          FlashEmbArtLPC313x_boot.xml flash loader.
 *                          Note: This configuration uses a Hardware reset with
 *                          delay to start IROM bootloader and allow it to copy
 *                          the application. The boot jumpers should be
 *                          configured for SPI NOR flash boot.
 *
 * Debug_SPINOR_Boot_SDRAM: This confuguration will download the example in the
 *                          SPI NOR flash. The application is linked to run in
 *                          the external SDRAM and a second level bootloader is
 *                          linked to run in the internal RAM. The IROM
 *                          bootloader copies the secon level bootloader from
 *                          the SPI NOR to the interan RAM and starts it.
 *                          The second level bootloader copies the rest of the
 *                          application to SDRAM. The configuration uses
 *                          FlashEmbArtLPC313x_boot.xml flash loader to program
 *                          the secon level bootloader and
 *                          FlashEmbArtLPC313x_img.xml flash loader to program
 *                          the application.
 *
 *                          Note: This configuration uses a Hardware reset with
 *                          delay to start IROM bootloader and allow it to copy
 *                          the second level bootloader. The boot jumpers should
 *                          be configured for SPI NOR flash boot.
 *    History :
 *    1. Date        : 18.3.2009
 *       Author      : Stoyan Choynev
 *       Description : initial revision.
 *
 *    $Revision: 30870 $
 **************************************************************************/

#include "includes.h"
//#include "lpc_io_stereo.h"
//#include "lpc_irq_fiq.h"  //DEBUG
#include "midmad.h"



#include "nand.h"


#include <intrinsics.h>
#include <nxp/iolpc3250.h>
#include <stdio.h>
#include "arm926ej_cp15_drv.h"
#include "ttbl.h"
#include "lpc_types.h"
#include "lpc_irq_fiq.h"
#include "lpc32xx_intc_driver.h"
#include "_pll.h"
#include "lpc32xx_slcnand_driver.h"
#include "lpc32xx_gpio_driver.h"

#include "efs.h"
#include "ls.h"
#include "mkfs.h"
#include "debug.h"
//#include "lpc_config.h"
#include "inttypes.h"

#include "string.h"


/** local definitions **/
#define ISROM_MMU_TTBL  0x1201C000
#define SDRAM_BASE_ADDR 0x30000000
#define IRAM_BASE_ADDR  0x11028000

#define LED1_SET  {IOCONF_GPIO_M1_SET = (1<<11);IOCONF_GPIO_M0_SET  = (1<<11);}
#define LED1_CLR  {IOCONF_GPIO_M1_SET = (1<<11);IOCONF_GPIO_M0_CLR  = (1<<11);}
#define LED2_SET  {IOCONF_GPIO_M1_SET = (1<<12);IOCONF_GPIO_M0_SET  = (1<<12);}
#define LED2_CLR  {IOCONF_GPIO_M1_SET = (1<<12);IOCONF_GPIO_M0_CLR  = (1<<12);}


#define   TIMER1_IN_FREQ       6MHZ
#define   TIMER1_TICK_MAX      1000   //in miliseconds
#define   TIMER1_TICK_MIN      20     //in miliseconds

#define   ADC_MAX              1024

#define   EXP_COEF             0.0038 //it is ln(TIMER1_TICK_MAX/TIMER1_TICK_MIN)/ADC_MAX

#define   TMR1TICK(tick)        ((TIMER1_IN_FREQ/1000)*tick)/256 //Convert miliseconds in Tmr ticks

/** external functions **/
extern void InitSDRAM(void);
extern void Dly_us(Int32U Dly);

volatile Int32U Ticks;

void Timer1_wait_ticks(Int32U ticks)
{
  Ticks = 0;
  
  do
  {
  for (int i=0;  i<327; i++);
  Ticks++;
  }
  while(Ticks < ticks);
}

#define _SP  0x01
#define _HP  (0x01<<1)


volatile unsigned char _modes;


/** internal functions **/
/*************************************************************************
 * Function Name: Timer1IntrHandler
 * Parameters: None
 *
 * Return: None
 *
 * Description: Timer 1 Interrupt service routine. Reload Timer
 *              with Tmr1Tick and toggle the GPIO2_BOOT LED
 *
 *************************************************************************/


/** public data **/

/** private data **/

/** public functions **/

/** private functions **/
//#include "Sin_Table.h"
/*************************************************************************
 * Function Name: main
 * Parameters: None
 *
 * Return: None
 *
 * Description: Getting Started main loop
 *
 *************************************************************************/



EmbeddedFileSystem  efs;
EmbeddedFileSystem  efs_flash;
EmbeddedFile        file;
EmbeddedFile        file1;
DirList             list;
unsigned char       file_name[100];
unsigned int        size;
char device;
//статус карты с книгой:
//0x01-> книга lkf
//0x02-> книга mp3
//0x03-> mp3 музыка
unsigned char m_env17_=0;

void SPI0_Init(void); //из spi0.c
void keyb_Init(void); //из keyb.c
unsigned char  _tone_up(void); //из keyb.c
unsigned char  _tone_down(void); //из keyb.c
unsigned char  _play_stop(void); //из keyb.c
unsigned char  _forward(void); //из keyb.c
unsigned char  _prev(void); //из keyb.c
unsigned char _get_status_bit(unsigned char _bit); //из keyb.c
void _init_BOD(); //из bod.c
void _int_off(); //из bod.c
void _init_bat_monitor(); //из charger.c
unsigned char _get_U_POWER1(); //из charger.c
unsigned char _get_U_POWER2(); //из charger.c
unsigned long _folder_n(void);

struct _place m_place;
unsigned char m_env=0;
unsigned char m_mode=0;
unsigned char m_env5=0xff;
struct _sleep m_sleep;
unsigned char m_wait_stop=0x0; //статус 2сек ожидания выкл
unsigned long m_ws_counter=0x0; //счетчик интервала ожидания выкл
//unsigned char m_turn_on; //статус того, что не надо себя включать после RESET
struct _place_back m_place_back;

static unsigned char _mode_list=0; //0->переход в mp3/lkf
                                 //1->переход в воспр. записанного
                                 //2->переход в lin rec
                                 //3->переход в mic rec

unsigned char _rec_list=2; //0->переход в воспр с диктофона
                                 //1->переход в воспр с лин входа
                                 //2->переход в воспр с fm radio
                                 

unsigned char _int_rec; //0 bit->1 воспр из внут памяти

unsigned long _iplay_id; //номер id для воспр из внутр памяти

//проверим наличие карты посредством открытия файла
unsigned char _check_card()
{
    if(file_fopen(&file,&efs.myFs,(unsigned char *)"/BOOK_001/0001.lkf",'r')!=0)
        return 0x01; //может карту вынули
    

            file_fclose(&file);
            _int_rec=0x0;
 
   return 0x0; //карта на месте      
}



//инит таблицы уровней каталогов
static void _init_level_tab(unsigned char _fol)
{
  memset(m_place._folder_0,0x0,sizeof(m_place._folder_0));
  memset(m_place._folder_01,0x0,sizeof(m_place._folder_01));
  m_place._folder_0[0]._folder_num = _fol; //текущая открытая папка _fol на 0 уровне 
  m_place._folder_01[0]._folder_num = _fol; //текущая открытая папка _fol на 0 уровне 
}  

//определим есть ли backup папка и вернем ее номер
static unsigned short _backup_number()
{
unsigned char _i=0x0;
  
    while(!m_place._folder_0[_i]._file_num && _i<_LEVEL_MAX)
      _i++;
    if(_i!= _LEVEL_MAX)
      return m_place._folder_0[_i]._file_num;

    return 0;
}

//пустая таблица уровней
static unsigned char _tab_empty()
{
  if(!m_place._folder_0[0]._folder_num && !m_place._folder_0[0]._file_num)
    return 0x0;
  
  return 0x01;
}


//определим текущий уровень открытой папки
static unsigned char _curr_level()
{
unsigned char _i=0x0;
  while(m_place._folder_0[_i]._folder_num && _i<_LEVEL_MAX)
    _i++;
  
  return _i;
}

//надо закрыть все папки до текущей на 0 уровне
static unsigned char _close_folder(void)
{
unsigned char _i=0x01;
    while(_i<_LEVEL_MAX)
    {
      m_place._folder_0[_i]._file_num =0;
      m_place._folder_0[_i]._folder_num = 0;
      _i++;
    }
  
}

//при next/prev папок на уровне надо закрывать папку, затем открывать новую
//input: _in=0x01->сделать backup папки и закрыть ее
//_in=0x02-сделать текущую открытую папку
//_in=0x03-открыть папку в текущей папке и сделать ее текущей
//_op=0x01->next папка на уровне
//_op=0x02->prev папка на уровне
static unsigned char _level_update(unsigned char _op,unsigned char _in)
{
unsigned char _i=0x0;
unsigned long _folders;


  if(_in == 0x01) //закроем текущую папку
  {
    while(m_place._folder_0[_i]._folder_num && _i<_LEVEL_MAX)
      _i++;
      m_place._folder_0[_i-1]._file_num = m_place._folder_0[_i-1]._folder_num;
      m_place._folder_0[_i-1]._folder_num = 0; //закрытие текущей папки
      if(_i<_LEVEL_MAX)
        m_place._folder_0[_i]._file_num=0; //в папке могли быть забэкапированы другие папки когда то
  }
//------------------------

  if(_in == 0x02)
  {
    _i=_LEVEL_MAX-1;
    while(!m_place._folder_0[_i]._file_num && _i<_LEVEL_MAX)
      _i--;
      //новая текущая открытая папка будет backup +/- 1
    if(_op==0x01) //откроем след папку
      m_place._folder_0[_i]._folder_num = m_place._folder_0[_i]._file_num+1;
      
    if(_op==0x02)
    {
        if(m_place._folder_0[_i]._file_num > 1)
          //не первая папка на уровне была текущая открытая
          m_place._folder_0[_i]._folder_num = m_place._folder_0[_i]._file_num-1;
        else
        {
          //была первая папка текущая на уровне, ставим статус что папок более нет
          m_place._folder_0[_i]._folder_num = 0xffff;
        }
    }
    m_place._folder_0[_i]._file_num=0; //сбросим значение backup
  }
  
  if(_in == 0x03)
  {
    while(m_place._folder_0[_i]._folder_num && _i<_LEVEL_MAX)
      _i++;
    if(_i<_LEVEL_MAX)
    {
      m_place._folder_0[_i]._folder_num = _op;
      m_place._folder_0[_i]._file_num=0; //сбросим значение backup
    }
    
  }
  
  return _i;
}

//завершающие пробелы убирает
void _trim(unsigned char *_str)
{
unsigned char _i=1;

  while(_str[strlen(_str)-_i] != ' ')
  {
    if((strlen(_str)-_i) == 0x0)
      return;
    _i++;
  }
  
  while(_str[strlen(_str)-_i] == ' ')
  {
    if((strlen(_str)-_i) == 0x0)
      return;
    _i++;
  }
  _str[strlen(_str)-_i+1] = '\0';
}

//DEBUG  
unsigned char _var;

void format_file_name(unsigned char *dest, unsigned char *src)
{
  unsigned char i,k;

    for (i=7; i>0 && (src[i] == 0x20); i--);

    strncpy(dest, src, i+1);
    dest[i+1] = '.';

    for (k=2; k>0 && (src[8+k] == 0x20); k--);

    strncpy(&dest[i+2], &src[8], k+1);
    dest[i+5] = '\0';
}


//каталоги книг идут в виде ##(BOOK_001/BOOK_001/...,BOOK_002/BOOK_001/..,...) и храняться в m_place._folder_0
//как n0,n1,n2,n3,n4 где n* - номер текущей папки на уровне вложенности(n*=0 если то на данном уровне нет текущей папки)
//с глубиной вложенности папок 5
//input: 0x01->был последний фрагмент в текущей самой низшей по вложенности папке, 
//       0x02->нужно сделать следующую текущей папку на самом низшем уровне, закрыв текущую
//       0x03->нужно сделать следующую текущей папку на самом низшем уровне 
unsigned char _next_book(unsigned char _op,unsigned char _par)
{
unsigned char _res;

  if(_par==0x01)
    _res=_level_update(0x0,0x01); //закроем текущую папку
  if(_par==0x02)
  { //для перехода на след. папку по кнопке
    _res=_level_update(0x0,0x01); //закроем текущую папку
    _res=_level_update(0x01,0x02); //откроем след папку
  }
  if(_par==0x03)
  {
    _res=_level_update(_op,0x03); //откроем папку в текущей
  }
  
  
  memcpy(m_place._folder_01,m_place._folder_0,sizeof(m_place._folder_0));
  return _res;
}

//input:  0x02->нужно сделать предыдущую текущей папку на самом низшем уровне, закрыв текущую         
//        0x03->нужно сделать следующую текущей папку на самом низшем уровне 
void __prev_book(unsigned char _par)
{
  if(_par==0x02)
  {
    _level_update(0x0,0x01); //закроем текущую папку
    _level_update(0x02,0x02);
  }
  if(_par==0x03)
  {
    _level_update(0x02,0x02);
  }
  
  memcpy(m_place._folder_01,m_place._folder_0,sizeof(m_place._folder_0));
  
}

void _play_rem_r(char *_p, char _par, short _rvol)
{
double _prev_k1 = _k1;
double __prev_k=__k;
unsigned char _prev_max_counter=_max_counter;

    //if(m_env17_ == 0x03)
      //return;
    //if(file_fopen(&file,&efs_flash.myFs,_p,'r')==0)
    if(file_fopen(&file,&efs_flash.myFs,_p,'r')==0)
    {
          _k1 = 1;
          __k=0x10;
          m_mylock = 0x01;
          _max_counter=1;
          _header_ready = 0; //заголовок не нужен, играем
          //_stretch_en(0);
            mp3_play(&file); 
          //_stretch_en(1);  

            file_fclose(&file);
            _int_rec=0x0;
          _k1 = _prev_k1;
          __k = __prev_k;
          _max_counter = _prev_max_counter;
          m_mylock = 0x0;
          
          if(_par)
            m_env = 0;
    }
  
}



//при _par >= 1 очищаем все поставленные задачи 
void _play_rem(char *_p, char _par)
{
double _prev_k1 = _k1;
double __prev_k=__k;
unsigned char _prev_max_counter=_max_counter;


        
    //if(m_env17_ == 0x03)
      //return;
    //if(file_fopen(&file,&efs_flash.myFs,_p,'r')==0)
    if(file_fopen(&file,&efs_flash.myFs,_p,'r')==0)
    {
          _k1 = 1;
          //__k=__k*2;
          m_mylock = 0x01;
          _max_counter=1;
          _header_ready = 0; //заголовок не нужен, играем
          //_stretch_en(0);
            mp3_play(&file);  
          //_stretch_en(1);  

            file_fclose(&file);
            _int_rec=0x0;
          _k1 = _prev_k1;
          __k = __prev_k;
          _max_counter = _prev_max_counter;
          m_mylock = 0x0;
          
          if(_par)
            m_env = 0;
    }
    
  
}


void _play_rem_p1(char *_p, char _par)
{
double _prev_k1 = _k1;
double __prev_k=__k;
unsigned char _prev_max_counter=_max_counter;
    
    //if(m_env17_ == 0x03)
      //return;
    
    //if(file_fopen(&file,&efs_flash.myFs,_p,'r')==0)
    if(file_fopen(&file,&efs_flash.myFs,_p,'r')==0)
    {
          _k1 = 3;
          __k=0x0060;//5*16+1;
          m_mylock = 0x01;
          _max_counter=1;
          _header_ready = 0; //заголовок не нужен, играем
          //_stretch_en(0);
          mp3_play(&file);
          //_stretch_en(1);

            file_fclose(&file);
            _int_rec=0x0;
          _k1 = _prev_k1;
          __k = __prev_k;
          _max_counter = _prev_max_counter;
          m_mylock = 0x0;
          
          if(_par)
            m_env = 0;
    }
    
  
}


void _play_rem_p(char *_p, char _par)
{
double _prev_k1 = _k1;
double __prev_k=__k;
unsigned char _prev_max_counter=_max_counter;

    //if(m_env17_ == 0x03)
      //return;
    
    //if(file_fopen(&file,&efs_flash.myFs,_p,'r')==0)
    if(file_fopen(&file,&efs_flash.myFs,_p,'r')==0)
    {
          _k1 = 3;
          __k=__k*3;
          m_mylock = 0x01;
          _max_counter=1;
          _header_ready = 0; //заголовок не нужен, играем
          //_stretch_en(0);
            mp3_play(&file);
          //_stretch_en(1);  

            file_fclose(&file);
            _int_rec=0x0;
          _k1 = _prev_k1;
          __k = __prev_k;
          _max_counter = _prev_max_counter;
          m_mylock = 0x0;
          
          if(_par)
            m_env = 0;
    }
    
  
}


static void _n_play(EmbeddedFile *file, unsigned char *_file_str)
{
double _prev_k1 = _k1;
double _prev_k=__k;
unsigned char _prev_max_counter=_max_counter;

    
    if(file_fopen(file,&efs_flash.myFs,_file_str,'r')==0)
    {
          _k1 = 1;
          //__k=__k*2;
          m_mylock = 0x01;
          _max_counter=1;
          _header_ready = 0; //заголовок не нужен, играем
          //_stretch_en(0);
            mp3_play(file);
          //_stretch_en(1);

            file_fclose(file);
            _int_rec=0x0;
          _k1 = _prev_k1;
          __k = _prev_k;
          _max_counter = _prev_max_counter;
          m_mylock = 0x0;
          

    }
  
}

//при _par >= 1 очищаем все поставленные задачи 
//_frag_name_file имеет формат ****.D, где **** - номер фрагмента
//в папке /SYS/****.MP3 озвучка номера
//надо соответствующую озвучку воспроизвести
unsigned char _buff1[20];
void _play_frag_number(char *_frag_name_file, char _par)
{
double _prev_k1 = _k1;
double _prev_k=__k;
unsigned char _prev_max_counter=_max_counter;
unsigned char *_point1;
unsigned short _frag_n;
unsigned short l;  
unsigned short _base;
    //if(m_env17_ == 0x03)
      //return;

    
    
    _point1 = strchr(_frag_name_file,'.'); //точка наверняка найдется
    *_point1 = '\0';
    if(_par == 2)
      _frag_n = atoi(_frag_name_file);
    else
      _frag_n = list.mCount;
    
    if(_par==0x01)
       m_env = 0;

//произнесем 1000 если надо
    if(_frag_n >= 1000)
    {
      sprintf(_buff1,"/SYS/1000.MP3\0");
      _n_play(&file, _buff1);
      _frag_n -= 1000;
    }
//номера кратные 10(10,20, 70,...,110,330,...)    
    if((_frag_n >= 10) && ((_frag_n%10)==0))
    {
      sprintf(_buff1,"/SYS/%d.MP3\0",_frag_n);
      _n_play(&file, _buff1);
      return;
    }
    
    
for(_base = 0; _base <= 900; _base += 100)
{
//произнесем номера _base+09..._base+19    
    if(_frag_n > _base && _frag_n < _base+20)
    {
      sprintf(_buff1,"/SYS/%d.MP3\0",_frag_n);
      _n_play(&file, _buff1);
      return;
    }
//произнесем номера _base+21..._base+99        
    for(l=(_base+20);l<=(_base+90);l+=10)
    {
      if(_frag_n > l && _frag_n < (l+10))
      {
        sprintf(_buff1,"/SYS/%d.MP3\0",l);
        _n_play(&file, _buff1);
        sprintf(_buff1,"/SYS/%d.MP3\0",(_frag_n-l));
        _n_play(&file, _buff1);
        return;
      }
    }

}    


}



static void _n_play_r(EmbeddedFile *file, unsigned char *_file_str)
{
double _prev_k1 = _k1;
double _prev_k=__k;
unsigned char _prev_max_counter=_max_counter;


    if(file_fopen(file,&efs_flash.myFs,_file_str,'r')==0)
    {
          _k1 = 1;
          __k=0x10;
          m_mylock = 0x01;
          _max_counter=1;
          _header_ready = 0; //заголовок не нужен, играем
          //_stretch_en(0);
          mp3_play(file);
          //_stretch_en(1);

            file_fclose(file);
            _int_rec=0x0;

          _k1 = _prev_k1;
          __k = _prev_k;
          _max_counter = _prev_max_counter;
          m_mylock = 0x0;
          

    }
  
}

//при _par >= 1 очищаем все поставленные задачи 
//в папке /SYS/****.MP3 озвучка номера
//надо соответствующую озвучку воспроизвести
void _play_number_r(unsigned long _frag_n)
{
double _prev_k1 = _k1;
double _prev_k=__k;
unsigned char _prev_max_counter=_max_counter;
unsigned char *_point1;
unsigned short l;  
unsigned short _base;
    //if(m_env17_ == 0x03)
      //return;

    

//произнесем 1000 если надо
    if(_frag_n >= 1000)
    {
      sprintf(_buff1,"/SYS/1000.MP3\0");
      _n_play_r(&file, _buff1);
      _frag_n -= 1000;
    }
//номера кратные 10(10,20, 70,...,110,330,...)    
    if((_frag_n >= 10) && ((_frag_n%10)==0))
    {
      sprintf(_buff1,"/SYS/%d.MP3\0",_frag_n);
      _n_play_r(&file, _buff1);
      return;
    }
    
    
for(_base = 0; _base <= 900; _base += 100)
{
//произнесем номера _base+09..._base+19    
    if(_frag_n > _base && _frag_n < _base+20)
    {
      sprintf(_buff1,"/SYS/%d.MP3\0",_frag_n);
      _n_play_r(&file, _buff1);
      return;
    }
//произнесем номера _base+21..._base+99        
    for(l=(_base+20);l<=(_base+90);l+=10)
    {
      if(_frag_n > l && _frag_n < (l+10))
      {
        sprintf(_buff1,"/SYS/%d.MP3\0",l);
        _n_play_r(&file, _buff1);
        sprintf(_buff1,"/SYS/%d.MP3\0",(_frag_n-l));
        _n_play_r(&file, _buff1);
        return;
      }
    }

}    


}


//при _par >= 1 очищаем все поставленные задачи 
//_bookmark_n содержит **** - номер закладки
//в папке /SYS/****.MP3 озвучка номера
//надо соответствующую озвучку воспроизвести
void _play_bookmark_number(unsigned char _bookmark_n, char _par)
{
double _prev_k1 = _k1;
double _prev_k=__k;
unsigned char _prev_max_counter=_max_counter;
unsigned char *_point1; 
    
    //if(m_env17_ == 0x03)
      //return;
    
    sprintf(_buff1,"/SYS/%d.MP3\0",_bookmark_n);
    if(file_fopen(&file,&efs_flash.myFs,_buff1,'r')==0)
    {
          _k1 = 1;
          //__k=__k*2;
          m_mylock = 0x01;
          _max_counter=1;
          _header_ready = 0; //заголовок не нужен, играем
          //_stretch_en(0);
          mp3_play(&file);
          //_stretch_en(1);
 
            file_fclose(&file);
            _int_rec=0x0;

          _k1 = _prev_k1;
          __k = _prev_k;
          _max_counter = _prev_max_counter;
          m_mylock = 0x0;
          
          if(_par)
            m_env = 0;
    }
    
  
}

//получим номер фрагмента
unsigned short _num_frag(unsigned char *_p)
{
unsigned char *_p1;
unsigned short _num;

    _p1 = strchr(_p,'.');
    if(_p1 != NULL)
    {
       *_p1 = '\0';
       _num = atoi(_p);
       *_p1 = '.';
    }
    else
    {
      _p1 = strchr(_p,' ');
      if(_p1 != NULL)
        *_p1 = '\0';
      _num = atoi(_p);
      *_p1 = ' ';
      
    }
    
    
    
    return _num;
}

//проверка на существ. папки
//returned 0x01 exist
//returned 0x03 last & exist
static unsigned char _check_folder(void)
{
unsigned short _num;
DirList        _list1;
unsigned char _lout[50];
unsigned char *_pout=_lout;

    sprintf(_pout,"/\0");
    _num = m_place._folder_0[0]._folder_num;  
    ls_openDir(&_list1, &(efs.myFs), _lout);
    while(_num != 0)
    {
    _reset_WD(); //reset WD
     if(ls_getNext(&_list1) != 0)
     {
       _lout[0]='\0';
       return 0x0; //нет такой папки
     }
    
     if((_list1.currentEntry.Attribute & 0x10) == 0x10) //папка 
      _num--;
     
    } 
    while(!ls_getNext(&_list1))
      if((_list1.currentEntry.Attribute & 0x10) == 0x10) //папка 
        return 0x01;
  
  return 0x03;
}


#if 0
//по таблице уровней делаем реальный путь
//return 0x01 если папка последняя
unsigned char _real_folder(struct _level_tab *_in, unsigned char *_out)
{
unsigned char *_p1;
unsigned short _num;
DirList _list1;
unsigned char _i;
unsigned char *_pout=_out;
unsigned short _end_dir=0x0;

  

   
  _i=0x0;
  *_pout='\0';
  if(!m_place._folder_0[0]._folder_num)
  {
    sprintf(_out,"/\0");
    return 0x0;
  }  
  if(m_place._folder_0[0]._folder_num==0xffff)
  { //надо на последнюю книгу бежать
    sprintf(_pout,"/\0");
    _num=0x0;
    ls_openDir(&_list1, &(efs.myFs), _out);
    while(!ls_getNext(&_list1))
      if((_list1.currentEntry.Attribute & 0x10) == 0x10)
      {
        _num++;
      }
    ls_getPrev(&_list1);
    m_place._folder_0[0]._folder_num=_num;
    _pout += strlen(_pout);
    
    _patch_dot__(_list1.currentEntry.FileName);
    if(_list1.currentEntry.FileName[8]!=0x20)
    {
      sprintf(_pout,"%s.%s\0",_file__,_ext__);
    }
    else  
      sprintf(_pout,"%s\0",_list1.currentEntry.FileName);

//---------    
    _trim(_out); //убрать заверш пробелы
    _pout += strlen(_pout);
    //скажем фразу "Переход на книгу" 
     sprintf(file_name,"/SYS/_33.mp3\0");
    _play_rem(file_name,0);
#if !_FIX1    
    sprintf(file_name,"%d.",m_place._folder_0[0]._folder_num);
//воспроизв номер книги    
    _play_frag_number(file_name,2);
#endif
    
    return 0x0;    
  }
  
  while(m_place._folder_0[_i]._folder_num && _i<_LEVEL_MAX)
  {
    sprintf(_pout,"/\0");
    _num = m_place._folder_0[_i++]._folder_num;  
    ls_openDir(&_list1, &(efs.myFs), _out);
    while(_num != 0)
    {
    _reset_WD(); //reset WD
     if(ls_getNext(&_list1) != 0)
     {
       _out[0]='\0';
       return 0x0; //нет такой папки
     }
    
     if((_list1.currentEntry.Attribute & 0x10) == 0x10) //папка 
      _num--;
     
    }       
    _pout += strlen(_pout);
    _patch_dot__(_list1.currentEntry.FileName);
    if(_list1.currentEntry.FileName[8]!=0x20)
    {
      sprintf(_pout,"%s.%s\0",_file__,_ext__); //для папок с .
    }
    else  
      sprintf(_pout,"%s\0",_list1.currentEntry.FileName);
    _trim(_out); //убрать заверш пробелы
    _pout += strlen(_pout);
    if(_i==1)
    {
//может последняя книга  
      while(!ls_getNext(&_list1))
      if((_list1.currentEntry.Attribute & 0x10) == 0x10)
      {
        _end_dir=0x01; //не последняя папка
        break;
        
      }
      
    }
  }
  if(strlen(_out)==0)
  {
    sprintf(_out,"/\0");
    return 0x0;
  }
  
  if(_end_dir)
    return 0x0;
  
  return 0x01;
}
#endif

//определим сколько папок
unsigned long _folder_n(void)
{
unsigned char *_p1;
unsigned long _num=0;
DirList        _list1;
unsigned char _rfolder[100];

  _real_folder(m_place._folder_0, _rfolder);
  ls_openDir(&_list1, &(efs.myFs), _rfolder);
  while(1)
  {
    _reset_WD(); //reset WD
     if(ls_getNext(&_list1) != 0)
     {
       
       return _num; 
     }
    
     if((_list1.currentEntry.Attribute & 0x10) == 0x10) //папка 
      _num++;
     
  }        
  
  return 0x01;
}

//определим сколько файлов в первой папке корня 
static unsigned long _folder_file(unsigned char *_in)
{
unsigned char *_p1;
unsigned long _num=0;
DirList        _list1;
unsigned char _fol[100];
unsigned char *_p=_fol;
unsigned char *_p2;

  sprintf(_fol,"%s\0",_in);
  _p++;
  if((_p2=strchr(_p,'/'))!= NULL)
    *_p2='\0';

  ls_openDir(&_list1, &(efs.myFs), _fol);
  while(ls_getNext(&_list1) == 0)
  {
    _reset_WD(); //reset WD
    _num++;
  }        
  
  return _num;
}

//определим сколько файлов в папке 
static unsigned long _folder_file1(unsigned char *_in)
{
unsigned char *_p1;
unsigned long _num=0;
DirList        _list1;
unsigned char _folder[100];
unsigned char *_p=_folder;
unsigned char *_p2;

  sprintf(_folder,"%s\0",_in);
#if 0  
  _p++;
  
  if((_p2=strchr(_p,'/'))!= NULL)
    *_p2='\0';
#endif  

  ls_openDir(&_list1, &(efs.myFs), _folder);
  while(ls_getNext(&_list1) == 0)
  {
    _reset_WD(); //reset WD
    _num++;
  }        
  
  return _num;
}


//делаем virt m_place._sample из реального номера фрагмента 
//sprintf(m_place._sample,"%s\0",list.currentEntry.FileName);
static void _virt_sample(unsigned short _num,unsigned char _ext)
{
unsigned char _buf[5];
    sprintf(_buf,"%d\0",_num);
    switch(strlen(_buf))
    {

      case 1:
        sprintf(m_place._sample,"000%d       \0",_num);
      break;
      case 2:
        sprintf(m_place._sample,"00%d       \0",_num);
      break;
      case 3:
        sprintf(m_place._sample,"0%d       \0",_num);
      break;
      case 4:
        sprintf(m_place._sample,"%d       \0",_num);
      break;
      
    };
    if(_ext == 0)
    {
      m_place._sample[8]='L';
      m_place._sample[9]='K';
      m_place._sample[10]='F';
    }
    if(_ext == 1)
    {
      m_place._sample[8]='M';
      m_place._sample[9]='P';
      m_place._sample[10]='3';
    }
    
}


//делаем virt m_place_back._sample из реального номера фрагмента 
//sprintf(m_place_back._sample,"%s\0",list.currentEntry.FileName);
void _virt_sample_back(unsigned short _num,unsigned char _ext)
{
unsigned char _buf[5];
    sprintf(_buf,"%d\0",_num);
    switch(strlen(_buf))
    {

      case 1:
        sprintf(m_place_back._sample,"000%d       \0",_num);
      break;
      case 2:
        sprintf(m_place_back._sample,"00%d       \0",_num);
      break;
      case 3:
        sprintf(m_place_back._sample,"0%d       \0",_num);
      break;
      case 4:
        sprintf(m_place_back._sample,"%d       \0",_num);
      break;
      
    };
    if(_ext == 0)
    {
      m_place_back._sample[8]='L';
      m_place_back._sample[9]='K';
      m_place_back._sample[10]='F';
    }
    if(_ext == 1)
    {
      m_place_back._sample[8]='M';
      m_place_back._sample[9]='P';
      m_place_back._sample[10]='3';
    }
    
}

//делаем virt m_place._sample из реального номера фрагмента 
//sprintf(m_place._sample_*,"%s\0",list.currentEntry.FileName);
static void _virt_sample_b(unsigned char *_p,unsigned short _num,unsigned char _ext)
{
unsigned char _buf[5];
    sprintf(_buf,"%d\0",_num);
    switch(strlen(_buf))
    {

      case 1:
        sprintf(_p,"000%d       \0",_num);
      break;
      case 2:
        sprintf(_p,"00%d       \0",_num);
      break;
      case 3:
        sprintf(_p,"0%d       \0",_num);
      break;
      case 4:
        sprintf(_p,"%d       \0",_num);
      break;
      
    };
    if(_ext == 0)
    {
      _p[8]='L';
      _p[9]='K';
      _p[10]='F';
    }
    if(_ext == 1)
    {
      _p[8]='M';
      _p[9]='P';
      _p[10]='3';
    }
    
}


//найдем в каталоге фрагмент _frag_name и сделаем его текущим
//_par->0x0 фрагмент текущий 
//_par->0x01 фрагмент закладки 1 
//_par->0x02 фрагмент закладки 2 
//_par->0x03 фрагмент закладки 3 
//_par->0x04 фрагмент закладки 4 
//_par->0x05 фрагмент закладки 5 
//_par->0x06 в конец книги
void _find_frag(unsigned char _par)
{
unsigned char _folder[100];

  switch(_par)
  {
    case 0x0:
#if !_FIX1      
      if(!_var55)
        memcpy(m_place._folder_0,m_place._folder_01,sizeof(m_place._folder_0));
      _real_folder(m_place._folder_0, _folder);
      if (ls_openDir(&list, &(efs.myFs), _folder) != 0)
      {
        _init_level_tab(1);
        _real_folder(m_place._folder_0, _folder);
        if (ls_openDir(&list, &(efs.myFs), _folder) != 0)
        {
          _init_level_tab(0);
          _real_folder(m_place._folder_0, _folder);
          ls_openDir(&list, &(efs.myFs), _folder);          
        }
        m_place._fpos = 0xffffffff;
      }
#endif      
      if(ls_getNext(&list))
      {
        m_env = m_env | 0x08; //ищим след фрагмент
        break;
      }
      if(((unsigned long)m_place._fpos) != 0xffffffff)
      {
        while(_num_frag(m_place._sample) != list.mCount)
        {
          _reset_WD(); //reset WD
          if(ls_getNext(&list) != 0)
          {
            ls_openDir(&list, &(efs.myFs), _folder);
            ls_getNext(&list);
            break;
          }
        }
#if 0        
        while(strncmp(m_place._sample,list.currentEntry.FileName,8) != 0)
          ls_getNext(&list);
#endif                
      }
      break;
    case 0x01:
      //if(!_var55)
#if !_FIX1 
        memcpy(m_place._folder_0,m_place._folder_1,sizeof(m_place._folder_0));
        memcpy(m_place._folder_01,m_place._folder_0,sizeof(m_place._folder_0));
      _real_folder(m_place._folder_0, _folder);
      if (ls_openDir(&list, &(efs.myFs), _folder) != 0)
      {
        _init_level_tab(1);
        _real_folder(m_place._folder_0, _folder);
        ls_openDir(&list, &(efs.myFs), _folder);
        m_place._fpos_1 = 0xffffffff;
      }
#endif      
      if(ls_getNext(&list))
      {
        m_env = m_env | 0x08; //ищим след фрагмент
        break;
      }
      if(((unsigned long)m_place._fpos_1) != 0xffffffff)
      {
        while(_num_frag(m_place._sample_1) != list.mCount)
        {
          _reset_WD(); //reset WD
          if(ls_getNext(&list) != 0)
          {
            ls_openDir(&list, &(efs.myFs), _folder);
            ls_getNext(&list);
            break;
          }
        }  

#if 0        
        while(strncmp(m_place._sample_1,list.currentEntry.FileName,8) != 0)
          ls_getNext(&list);
#endif        
      }
      
      break;
#if 1      
    case 0x02:
      //if(!_var55)
#if !_FIX1      
       memcpy(m_place._folder_0,m_place._folder_2,sizeof(m_place._folder_0));
       memcpy(m_place._folder_01,m_place._folder_0,sizeof(m_place._folder_0));
      _real_folder(m_place._folder_0, _folder);
      if (ls_openDir(&list, &(efs.myFs), _folder) != 0)
      {
        _init_level_tab(1);
        _real_folder(m_place._folder_0, _folder);
        ls_openDir(&list, &(efs.myFs), _folder);
        m_place._fpos_2 = 0xffffffff; 
      }
#endif      
      if(ls_getNext(&list))
      {
        m_env = m_env | 0x08; //ищим след фрагмент
        break;
      }
      if(((unsigned long)m_place._fpos_2) != 0xffffffff)
      {
        while(_num_frag(m_place._sample_2) != list.mCount)
        {
          _reset_WD(); //reset WD
          if(ls_getNext(&list) != 0)
          {
            ls_openDir(&list, &(efs.myFs), _folder);
            ls_getNext(&list);
            break;
          }
        }
#if 0        
        while(strncmp(m_place._sample_2,list.currentEntry.FileName,8) != 0)
          ls_getNext(&list);
#endif        
      }
      
      break;
    case 0x03:
      //if(!_var55)
#if !_FIX1
       memcpy(m_place._folder_0,m_place._folder_3,sizeof(m_place._folder_0));
       memcpy(m_place._folder_01,m_place._folder_0,sizeof(m_place._folder_0));
      _real_folder(m_place._folder_0, _folder);
      if (ls_openDir(&list, &(efs.myFs), _folder) != 0)
      {
        _init_level_tab(1);
        _real_folder(m_place._folder_0, _folder);
        ls_openDir(&list, &(efs.myFs), _folder);
        m_place._fpos_3 = 0xffffffff;
      }
#endif
      if(ls_getNext(&list))
      {
        m_env = m_env | 0x08; //ищим след фрагмент
        break;
      }
      if(((unsigned long)m_place._fpos_3) != 0xffffffff)
      {
        while(_num_frag(m_place._sample_3) != list.mCount)
        {
          _reset_WD(); //reset WD
          if(ls_getNext(&list) != 0)
          {
            ls_openDir(&list, &(efs.myFs), _folder);
            ls_getNext(&list);
            break;
          }
        }
#if 0        
        while(strncmp(m_place._sample_3,list.currentEntry.FileName,8) != 0)
          ls_getNext(&list);
#endif        
      }
      
      break;
    case 0x04:
      //if(!_var55)
#if !_FIX1
       memcpy(m_place._folder_0,m_place._folder_4,sizeof(m_place._folder_0));
       memcpy(m_place._folder_01,m_place._folder_0,sizeof(m_place._folder_0));
      _real_folder(m_place._folder_0, _folder);
      if (ls_openDir(&list, &(efs.myFs), _folder) != 0)
      {
        _init_level_tab(1);
        _real_folder(m_place._folder_0, _folder);
        ls_openDir(&list, &(efs.myFs), _folder);
        m_place._fpos_4= 0xffffffff;
      }
#endif
      if(ls_getNext(&list))
      {
        m_env = m_env | 0x08; //ищим след фрагмент
        break;
      }
      if(((unsigned long)m_place._fpos_4) != 0xffffffff)
      {
        while(_num_frag(m_place._sample_4) != list.mCount)
        {
          _reset_WD(); //reset WD
          if(ls_getNext(&list) != 0)
          {
            ls_openDir(&list, &(efs.myFs), _folder);
            ls_getNext(&list);
            break;
          }
        }
#if 0        
        while(strncmp(m_place._sample_4,list.currentEntry.FileName,8) != 0)
          ls_getNext(&list);
#endif        
      }
      
      break;
    case 0x05:
      //if(!_var55)
#if !_FIX1 
       memcpy(m_place._folder_0,m_place._folder_5,sizeof(m_place._folder_0));
       memcpy(m_place._folder_01,m_place._folder_0,sizeof(m_place._folder_0));
      _real_folder(m_place._folder_0, _folder);
      if (ls_openDir(&list, &(efs.myFs), _folder) != 0)
      {
        _init_level_tab(1);
        _real_folder(m_place._folder_0, _folder);
        ls_openDir(&list, &(efs.myFs), _folder);
        m_place._fpos_5= 0xffffffff;
      }
#endif
      if(ls_getNext(&list))
      {
        m_env = m_env | 0x08; //ищим след фрагмент
        break;
      }
      if(((unsigned long)m_place._fpos_5) != 0xffffffff)
      {
        while(_num_frag(m_place._sample_5) != list.mCount)
        {
          _reset_WD(); //reset WD
          if(ls_getNext(&list) != 0)
          {
            ls_openDir(&list, &(efs.myFs), _folder);
            ls_getNext(&list);
            break;
          }
        }
#if 0        
        while(strncmp(m_place._sample_5,list.currentEntry.FileName,8) != 0)
          ls_getNext(&list);
#endif        
      }
      
      break;
    case 0x06:
#if !_FIX1      
      memcpy(m_place._folder_0,m_place._folder_01,sizeof(m_place._folder_0));
      _real_folder(m_place._folder_0, _folder);
      if (ls_openDir(&list, &(efs.myFs), _folder) != 0)
      {
        _init_level_tab(1);
        _real_folder(m_place._folder_0, _folder);
        ls_openDir(&list, &(efs.myFs), _folder);
      }
#endif
      while(ls_getNext(&list) != -1)
      {
        _reset_WD(); //reset WD
          ls_getNext(&list);
      }
      ls_getPrev(&list); //по идеи мы в последнем фрагменте
      if(list.currentEntry.FileName[8] == 'M' && list.currentEntry.FileName[9] == 'P' && list.currentEntry.FileName[10] == '3')
        list.mCount++;        
      m_env25_ |= 0x01; //правильно отступим от конца фрагмента для воспр.
      //m_env28_ = 0x01; //последний фрагм книги
      break;
#endif
    
  };
  
}

//установим позицию в фрагментe _frag_name
//_par->0x0 фрагмент текущий 
//_par->0x01 фрагмент закладки 1 
//_par->0x02 фрагмент закладки 2 
//_par->0x03 фрагмент закладки 3 
//_par->0x04 фрагмент закладки 4 
//_par->0x05 фрагмент закладки 5 
void _pos_frag()
{

  switch(m_env5)
  {
    case 0x0:
      if(((unsigned long)m_place._fpos) != 0xffffffff)
      {
        file.FilePtr = m_place._fpos;
      }
      break;
    case 0x01:
      if(((unsigned long)m_place._fpos_1) != 0xffffffff)
      {
        file.FilePtr = m_place._fpos_1;        
      }
      break;
    case 0x02:
      if(((unsigned long)m_place._fpos_2) != 0xffffffff)
      {
        file.FilePtr = m_place._fpos_2;                
      }
      break;
    case 0x03:
      if(((unsigned long)m_place._fpos_3) != 0xffffffff)
      {
        file.FilePtr = m_place._fpos_3;                
      }
      break;
    case 0x04:
      if(((unsigned long)m_place._fpos_4) != 0xffffffff)
      {
        file.FilePtr = m_place._fpos_4;                
      }
      break;
    case 0x05:
      if(((unsigned long)m_place._fpos_5) != 0xffffffff)
      {
        file.FilePtr = m_place._fpos_5;                
      }
      break;
    case 0x06:
      file.FilePtr = file.FileSize-(file.FileSize/3); //где то в конце фрагмента                 
      
      break;
    
  };
  
  m_env5=0xff; //сбросим
  
}

//сохранение параметров закладки
void _save_bookmark_par(unsigned char _par)
{
unsigned short _nfrag;

  _nfrag = list.mCount;
    
  switch(_par)
  {
    case 0x01:
            m_place._fpos_1 = file.FilePtr;
            //sprintf(m_place._sample_1,"%s\0",list.currentEntry.FileName);
            if(m_env17_ == 0x03)
             _virt_sample_b(m_place._sample_1,_nfrag,1);
            else
             _virt_sample_b(m_place._sample_1,_nfrag,0); //делаем virt m_place._sample из реального номера фрагмента 
                                         // 0-> фрагмент LKF,1-> фрагмент MP3 
            memcpy(m_place._folder_1,m_place._folder_0,sizeof(m_place._folder_0));
      break;
    case 0x02:
            m_place._fpos_2 = file.FilePtr;
            //sprintf(m_place._sample_2,"%s\0",list.currentEntry.FileName);
            if(m_env17_ == 0x03)
             _virt_sample_b(m_place._sample_2,_nfrag,1);
            else
             _virt_sample_b(m_place._sample_2,_nfrag,0); //делаем virt m_place._sample из реального номера фрагмента 
                                         // 0-> фрагмент LKF,1-> фрагмент MP3 
            memcpy(m_place._folder_2,m_place._folder_0,sizeof(m_place._folder_0));
            
      break;
    case 0x03:
            m_place._fpos_3 = file.FilePtr;
            //sprintf(m_place._sample_3,"%s\0",list.currentEntry.FileName);
            if(m_env17_ == 0x03)
             _virt_sample_b(m_place._sample_3,_nfrag,1);
            else
             _virt_sample_b(m_place._sample_3,_nfrag,0); //делаем virt m_place._sample из реального номера фрагмента 
                                         // 0-> фрагмент LKF,1-> фрагмент MP3 
            
            
            memcpy(m_place._folder_3,m_place._folder_0,sizeof(m_place._folder_0));
      
      break;
    case 0x04:
            m_place._fpos_4 = file.FilePtr;
            //sprintf(m_place._sample_4,"%s\0",list.currentEntry.FileName);
            if(m_env17_ == 0x03)
             _virt_sample_b(m_place._sample_4,_nfrag,1);
            else
             _virt_sample_b(m_place._sample_4,_nfrag,0); //делаем virt m_place._sample из реального номера фрагмента 
                                         // 0-> фрагмент LKF,1-> фрагмент MP3 
            
            
            memcpy(m_place._folder_4,m_place._folder_0,sizeof(m_place._folder_0));
      
      break;
    case 0x05:
            m_place._fpos_5 = file.FilePtr;
            //sprintf(m_place._sample_5,"%s\0",list.currentEntry.FileName);
            if(m_env17_ == 0x03)
             _virt_sample_b(m_place._sample_5,_nfrag,1);
            else
             _virt_sample_b(m_place._sample_5,_nfrag,0); //делаем virt m_place._sample из реального номера фрагмента 
                                         // 0-> фрагмент LKF,1-> фрагмент MP3 
            
            
            memcpy(m_place._folder_5,m_place._folder_0,sizeof(m_place._folder_0));
      
      break;
   
    default:  
  }
  
  m_env19_ |= 0x04; //чтобы не сохранять переход для отмены
}

//копирование на flash файлов озвучки
void _copy_rems()
{
unsigned char _buff_t[512];
unsigned long res;
unsigned long res1;
long i;
unsigned char _fname[15];
unsigned char _var=0;

  for(i=1;i<=31;i++)
  {
    memset(_fname,'\0',15);
    sprintf(_fname,"/sys/_%d.d",i);
    if(file_fopen(&file,&(efs.myFs),_fname,'r')==0)
    {
      if(file_fopen(&file1,&(efs_flash.myFs),_fname,'w')==0)
      {
        while(1)
        {
          if(res=file_read(&file,512,_buff_t))
          {
            res1=file_write(&file1,res,_buff_t);
            if(res1 != res)
               break;
          }
          else
            break;
        }
        file_fclose(&file);
        file_fclose(&file1);
//для отладки        
#if 0        
        _var++;
        if((_var & 0x01) == 0x0)
          IO0SET = 0x00200000;
        else
          IO0CLR = 0x00200000;
#endif        
//------------------        
      }
    }
  }
  
#if 1  
  for(i=1;i<=150;i++)
  {
    memset(_fname,'\0',15);
    sprintf(_fname,"/sys/__%d.d",i);
    if(file_fopen(&file,&(efs.myFs),_fname,'r')==0)
    {
      if(file_fopen(&file1,&(efs_flash.myFs),_fname,'w')==0)
      {
        while(1)
        {
          if(res=file_read(&file,512,_buff_t))
          {
            res1=file_write(&file1,res,_buff_t);
            if(res1 != res)
               break;
          }
          else
            break;
        }
        file_fclose(&file);
        file_fclose(&file1);
//для отладки  
#if 0        
        _var++;
        if((_var & 0x01) == 0x0)
          IO0SET = 0x00200000;
        else
          IO0CLR = 0x00200000;
#endif        
//------------------        
        
      }
    }
  }
#endif  
  
  
  
//----------------- для отладки
#if 0
unsigned char _buff_t[512];
  memset(_buff_t,0x80,512);
  if(file_fopen(&file,&(efs_flash.myFs),"/sys/test.dat",'w')==0)
  {
    for(long i=0; i< 1900; i++)
      file_write(&file,512,_buff_t);  
    file_fclose(&file);    
  //ls_openDir(&list, &(efs.myFs), "/dir1\0");
  }
  if(file_fopen(&file,&(efs_flash.myFs),"/sys/test1.dat",'w')==0)
  {
    for(long i=0; i< 1900; i++)
      file_write(&file,512,_buff_t);  
    file_fclose(&file);    
  //ls_openDir(&list, &(efs.myFs), "/dir1\0");
  }

  
  if(file_fopen(&file,&(efs_flash.myFs),"/sys/test.dat",'r')==0)
  {
    for(long i=0; i< 1900; i++)
    {
      memset(_buff_t,0x0,512);
      file_read(&file,512,_buff_t);  
    }
    file_fclose(&file);    
  //ls_openDir(&list, &(efs.myFs), "/dir1\0");
  }
  if(file_fopen(&file,&(efs_flash.myFs),"/sys/test1.dat",'r')==0)
  {
    for(long i=0; i< 1900; i++)
    {
      memset(_buff_t,0x0,512);
      file_read(&file,512,_buff_t);  
    }
    file_fclose(&file);    
  //ls_openDir(&list, &(efs.myFs), "/dir1\0");
  }
#endif  
//----------------  
  
}

void _reset_curr_pos(void)
{
#if !_FIX1
      _init_level_tab(1);  
      memcpy(m_place._folder_01,m_place._folder_0,sizeof(m_place._folder_0));
#endif      
#if 0      
      sprintf(m_place._folder_1,"/BOOK_001\0");    
      sprintf(m_place._folder_2,"/BOOK_001\0");    
      sprintf(m_place._folder_3,"/BOOK_001\0");    
      sprintf(m_place._folder_4,"/BOOK_001\0");    
      sprintf(m_place._folder_5,"/BOOK_001\0");    
#endif      
      sprintf(m_place._sample,"0001.lkf");  
  
 
      m_place._bookmark_index = 0x01;
      m_place._bookmark_index_p = 0x01;
      m_place._fpos = 0xffffffff; //нет текущей позиции текущ. фрагмента
      m_place._fpos_1 = 0xffffffff; //нет текущей позиции фрагмента закладки 1
      m_place._fpos_2 = 0xffffffff; //нет текущей позиции фрагмента закладки 2
      m_place._fpos_3 = 0xffffffff; //нет текущей позиции фрагмента закладки 3
      m_place._fpos_4 = 0xffffffff; //нет текущей позиции фрагмента закладки 4  
      m_place._fpos_5 = 0xffffffff; //нет текущей позиции фрагмента закладки 5
  
}

//инит текущих папок книг, фрагментов и режимов
void _init_curr_pos(void)
{
  
  if(m_env33)
  {
      m_place._bookmark_index = 0x05;
      m_place._bookmark_index_p = 0x01;
      m_place._fpos = 0xffffffff; //нет текущей позиции текущ. фрагмента
      m_place._fpos_1 = 0xffffffff; //нет текущей позиции фрагмента закладки 1
      m_place._fpos_2 = 0xffffffff; //нет текущей позиции фрагмента закладки 2
      m_place._fpos_3 = 0xffffffff; //нет текущей позиции фрагмента закладки 3
      m_place._fpos_4 = 0xffffffff; //нет текущей позиции фрагмента закладки 4  
      m_place._fpos_5 = 0xffffffff; //нет текущей позиции фрагмента закладки 5
      __k=0x10;
      _boost_inc=0;
      m_place._d=0x0e;    
  }
  else
  {
//проверим а считанные параметры текущ. позиции можно применить к текущей карте    
  if(_param_get() || _valid_get())
  {
#if 0    
      sprintf(m_place._folder_0,"/BOOK_001\0");  
      sprintf(m_place._folder_01,"/BOOK_001\0");    
#endif   
#if 0      
      sprintf(m_place._folder_1,"/BOOK_001\0");    
      sprintf(m_place._folder_2,"/BOOK_001\0");    
      sprintf(m_place._folder_3,"/BOOK_001\0");    
      sprintf(m_place._folder_4,"/BOOK_001\0");    
      sprintf(m_place._folder_5,"/BOOK_001\0");    
#endif  
      _init_level_tab(1);
      sprintf(m_place._sample,"0001.lkf");  
  
 
      m_place._bookmark_index = 0x05;
      m_place._bookmark_index_p = 0x01;
      m_place._fpos = 0xffffffff; //нет текущей позиции текущ. фрагмента
      m_place._fpos_1 = 0xffffffff; //нет текущей позиции фрагмента закладки 1
      m_place._fpos_2 = 0xffffffff; //нет текущей позиции фрагмента закладки 2
      m_place._fpos_3 = 0xffffffff; //нет текущей позиции фрагмента закладки 3
      m_place._fpos_4 = 0xffffffff; //нет текущей позиции фрагмента закладки 4  
      m_place._fpos_5 = 0xffffffff; //нет текущей позиции фрагмента закладки 5
      __k=0x10;
      _boost_inc=0;
      m_place._d=0x0e;
  }
  else
  {
#if 0
      __k=0x0060;
      _boost_inc=0;    
#else    
    if(m_place._k<=0x1B)
    {
      __k=m_place._k;
      _boost_inc = m_place._boost_inc;
    }
    else
    {
      __k=0x10;
      _boost_inc=0;
    }
    _aic23_svolume(__k);
#endif    
  }
  }
  
  
  _find_frag(0x0); //установим текущую позицию(книгу, фрагмент)
  
//инит для режима сна  
  m_sleep._max_time = 0; //счетчик сна сброшен
  m_sleep._mode = 0x0; //сон выкл
  
//инит куда идти при отмене перехода
#if !_FIX1
  memcpy(m_place_back._folder_0,m_place._folder_0,sizeof(m_place_back._folder_0));
  m_place_back._folder_0[0]._folder_num = m_place._folder_0[0]._folder_num; //текущая открытая папка 1 на 0 уровне 
#endif  
  *m_place_back._sample='\0';
  m_place_back._fpos=0xffffffff;
}

//может на карте mp3 хрень
unsigned char _get_card()
{
    device = _SDCARD; //SD card(здесь SSP инит!!!!)
    if(efs_init(&efs,&device)==0)
    {
      if(file_fopen(&file,&(efs.myFs),"/BOOK_001/0001.lkf",'r')==0)
      {
        file_fclose(&file);
        m_env15=0x0;
        fs_umount(&(efs.myFs));
//переключим mux на radio
#if 0        
        IO1DIR |= 0x00000001<<22; //as output P1.22 
        IO1CLR |= 0x00000001<<22; //unset P1.22
#endif        
        //I2C_Init();
        //_turn_on_radio();
        
        return 0x0; //книга
      }
      fs_umount(&(efs.myFs));
    }
    
    m_env15=0x01;
    return 0x0; //mp3 хрень
}

static void _keep_crc(unsigned char *_p,unsigned char _par)
{
unsigned long _crc;  
      _crc=0;
      for(unsigned char _l=0;_l<strlen(_p);_l++)
        _crc += _p[_l];
      _crc %= 256; //crc папки
      if(!_get_file_size(_crc))
      {
           _reset_curr_pos();
           m_env33=0x01; //новая карта
           _init_level_tab(_par);
             
      }
     
      _keep_file_size(_crc);   
}



//проверим sample rate файла в папке
static unsigned char _check_mp3_file(unsigned char *_folder1)
{
DirList             list1;
unsigned char _folder[15];
unsigned char _file_name[15];
unsigned char _folder_file_name[50];

      
    
      if(strstr(_folder1,"MP3") != NULL)
      {
       if(file_fopen(&file,&(efs.myFs),_folder1,'r')==0)
       {
         _header_ready |= 0x80; //нужен только заголовок
         _bitrate = 0;
         m_mylock = 0x01;
         //_stretch_en(0);
         mp3_play(&file);
         //_stretch_en(1);
         m_mylock = 0x0;
#if 0         
         if((_bitrate > 10000) && (_bitrate < 50000))
         {
          file_fclose(&file);
          return 0x01; //книга(48000bits/s)
         }         
         if((_bitrate > 60000) && (_bitrate < 400000))
         {
          file_fclose(&file);
          return 0x02;
         }
#else
         if(_samplerate == 22050)
         {
          file_fclose(&file);
          return 0x01; //книга(48000bits/s)
         }         
         if(_samplerate == 44100)
         {
          file_fclose(&file);
          return 0x02; //музыка
         }         
#endif
         
       }
      }

  return 0;
}


void _dac_mux()
{
#if 0
          //_uninit_boost1();
  //переключим mux усилка VS1001
          IO1DIR |= 0x00000001<<23; //as output P1.23 
          IO1SET |= 0x00000001<<23; //set P1.23
#endif          
          //_on_boost(); //вкл усилок
}

//детектор конца карты
unsigned char _detect_end()
{
unsigned char _res2_;
unsigned char _folder[100];

#if !_FIX1
    _res2_ = _real_folder(m_place._folder_0, _folder);   
#endif
   
    if((_folder_file(_folder) == list.mCount) && (_res2_ == 0x01))
      return 0x01;
    
  return 0x0;  
}

void _root_file(void)
{
DirList        _list1;

  ls_openDir(&_list1, &(efs.myFs), "/\0");
  while(1)
  {
    
     if(ls_getNext(&_list1) != 0)
      return; 
    
    
     if((_list1.currentEntry.Attribute & 0x10) == 0x0) //файл
     {
       if(_list1.currentEntry.FileName[8] == 'M' && _list1.currentEntry.FileName[9] == 'P' && _list1.currentEntry.FileName[10] == '3')
       {
         if (ls_openDir(&_list1, &(efs.myFs), "/BOOK_001\0") != 0)
         {
            _var96=0x01;
            return;
         }
       }
     }
     
  }        
  
  return;
}


//от куда воспроизводим
unsigned char _int_play(void)
{

  
  return 0x0;
}

euint32 _fpos;

unsigned char m_mylock=0x0;
unsigned char _var;
unsigned short _bat_value;
unsigned short _nfrag;

unsigned char _p2;
void main(void)
{
//unsigned char _fFolder[100];   
unsigned char _res2_;
unsigned char _folder[100];
char device;
char device1;


 

  
  _int_rec = 0x00;
  
// Init MMU
  CP15_Mmu(FALSE);            // Dis able MMU
  CP15_Cache(FALSE);          //cach dis
  // Privileged permissions  User permissions AP
  // Read-only               Read-only        0 
  CP15_SysProt(FALSE);
  CP15_RomProt(TRUE);
  CP15_InitMmuTtb(TtSB,TtTB); // Build L1 and L2 Translation  tables
  CP15_SetTtb(L1Table);       // Set base address of the L1 Translation table
  CP15_SetDomain( (DomainManager << 2*1) | (DomainClient << 0)); // Set domains
  CP15_Mmu(TRUE);             // Enable MMU
  CP15_Cache(TRUE);           // Enable ICache,DCache

  /* Disable interrupts in ARM core */
  disable_irq();
  
    /* Initialize interrupt system */
  int_initialize(0xFFFFFFFF);


  
  _init_pll();
  
 
  sdr_sdram_setup(AHB_CLK);
  
  _modes |= _SP;
  //_modes |= _HP;
  

  _init_boost();
  disable_irq();
  init_IO(); //init_timer() ?????????? ?????
  enable_irq();
  keyb_Init();//???? ???????

//определим как то текущую позицию в каталоге книг или mp3
#if 0  

#if !_FIX1
  _init_level_tab(1);
  memcpy(m_place._folder_01,m_place._folder_0,sizeof(m_place._folder_0));
#endif  
#if 0  
  sprintf(m_place._folder_1,"/BOOK_001\0");    
  sprintf(m_place._folder_2,"/BOOK_001\0");    
  sprintf(m_place._folder_3,"/BOOK_001\0");    
  sprintf(m_place._folder_4,"/BOOK_001\0");    
  sprintf(m_place._folder_5,"/BOOK_001\0");
#endif  
  sprintf(m_place._sample,"0001.lkf");  
  
  
  m_place._bookmark_index = 0x01;
  m_place._bookmark_index_p = 0x01;
  m_place._fpos = 0xffffffff; //нет текущей позиции текущ. фрагмента
  m_place._fpos_1 = 0xffffffff; //нет текущей позиции фрагмента закладки 1
  m_place._fpos_2 = 0xffffffff; //нет текущей позиции фрагмента закладки 2
  m_place._fpos_3 = 0xffffffff; //нет текущей позиции фрагмента закладки 3
  m_place._fpos_4 = 0xffffffff; //нет текущей позиции фрагмента закладки 4  
  m_place._fpos_5 = 0xffffffff; //нет текущей позиции фрагмента закладки 5
//инит для режима сна  
  m_sleep._max_time = 0; //счетчик сна сброшен
  m_sleep._mode = 0x0; //сон выкл
#endif  
//--------------

  
  _init_bat_monitor(); //инит монитора сетевой зарядки   
  m_env21_ = 0x0; //статус наличия карты 
unsigned long res;
//установим pins as input _SD_EXIST  
/* //PORT
IOCONF_UART_M0_CLR = (0x0001<<1);
IOCONF_UART_M1_CLR = (0x0001<<1);
*/



  _sdcard();

//if(!((IO0PIN >> 21) & 0x00000001)) //FIX_DEBUG
if(1/*!((IOCONF_UART_PIN >> 1) & 0x00000001)*/) //_SD_EXIST
{
  device = _SDCARD; //SD card(????? SSP ????!!!!)
  if(efs_init(&efs,&device)!=0)
  {
    while(1)
      //IO1DIR &= ~_IO1_16_MASK; //P1.16 ?? input, ???????? ???? //FIX_DEBUG
      _turnon(0x0);
    return;
  }  
}
else
{
  m_env21_ = 0x01; //?????? ??? ?????
}  
  
  enable_irq();
  _nand_setup(); 

  device = NAND; //??????? FS ?? flash 
  if(efs_init(&efs_flash,&device)!=0)
  {
    while(1)
      //IO1DIR &= ~_IO1_16_MASK; //P1.16 ?? input, ???????? ???? //FIX_DEBUG
      _turnon(0);
    return;
  }
 
//смотрим наличие mp3 файлов в корне  
  _root_file();  
  
  m_env17_ = 0x02; //книга c ext mp3   
  
  

unsigned char _prev_k1;  
  _init_curr_pos(); //инит текущих книги, фрагмента, и тд 
  if(m_env33)
  {
    m_env33=0x0;
    //_reset_curr_pos();
  }
  _init_boost_book(); //уровень усилка для книги 
  _prev_k1 = __k;
  __k=0x10;
  _en_irq();
  _play_rem("/SYS/_4.mp3\0",0x0);
  __k = _prev_k1;

  
  for(unsigned long _l=0;_l<0x0001ffff;_l++)
  asm("NOP");

  //keyb_Init(); //из keybs
  if(_get_key_block())
  {
    //клава блокирована
                  //if(((IO0PIN >> 16-2) & 0x0004) != 0x04) //FIX_DEBUG
                  if(0/*(((IOCONF_GPIO_PIN >> 14) << 2) & 0x0004) != 0x04*/)
                  {
                    sprintf(file_name,"/SYS/_25.mp3\0");
                    _play_rem(file_name,0x0);
                    
                    _uninit_boost(); //выкл усилок
                    for(unsigned long _l=0;_l<0x0cfffff;_l++)
                      asm("NOP");       
            
                    //IO1DIR &= ~_IO1_16_MASK; //P1.16 на input //FIX_DEBUG          
                    _turnon(0);
                    while(1)
                      asm("  NOP  ");                    
                  }
                  else
                  {
                    m_env31_=0x0;
                    sprintf(file_name,"/SYS/_26.mp3\0");
                    _play_rem(file_name,0x0);
                    m_env9 &= ~0x01; //сброим пик
                  }
    
  }
  
  
  _en_irq();
  
  
  
m_env5 = 0; //будем позиц. по текущ. фрагменту 
m_env3 = m_env3 & ~0x01;
m_env22_=0x02; //скажем текущую книгу и фрагмент
//ждем пока отпустят кнопку стоп      
//while(_play_stop()==0x02);
unsigned short _backup_n;
_play_stop();
goto _1234__0987__; //при первом старте начальный код след. цикла не нужен
while(1)
{
  _reset_WD(); //reset WD
  
  if(m_env21_)
  {
    m_env22_=0x0;
    goto __123_987___;
  }
  
  _res2_ = _real_folder(m_place._folder_0, _folder);
  if (ls_openDir(&list, &(efs.myFs), _folder) != 0)
  {
    _init_level_tab(1); //инит табл уровней как то
    //скажем фразу "Переход на книгу" 
     sprintf(file_name,"/SYS/_8.mp3\0");
    _play_rem(file_name,0);
#if !_FIX1    
    sprintf(file_name,"%d.",m_place._folder_0[0]._folder_num);
//воспроизв номер книги    
    _play_frag_number(file_name,2);
#endif
    
    continue;
  }
  _backup_n = _backup_number(); //номер последней зарытой папки
  while(_backup_n != 0)
  {
    _reset_WD(); //reset WD
     if(ls_getNext(&list) != 0)
     {
       _init_level_tab(1); //инит табл уровней как то
       break;
     }
    
     if((list.currentEntry.Attribute & 0x10) == 0x10) //папка 
      _backup_n--;
     
  }       
  
 
  m_env32_ = _folder_file(_folder); //число фрагментов в книге в папке 1 уровня
  if(_curr_level()==1)
    m_env32__=m_env32_;
  else
    m_env32__ = _folder_file1(_folder); //число фрагментов в книге на уровне

//--- 

  if(_res2_ == 0x01)
    //фраза "последняя книга"
    sprintf(file_name,"/SYS/_33.mp3\0");
  else
  //скажем фразу "Переход на книгу" 
    sprintf(file_name,"/SYS/_8.mp3\0");
  if(!_curr_level() && !_tab_empty())
    _play_rem(file_name,0);
  
#if _FIX1    
    sprintf(file_name,"%d.",m_place._folder_0[0]._folder_num);
//воспроизв номер книги    
    _play_frag_number(file_name,2);
#endif
    
unsigned char _res5__;  
if(1/*ls_getNext(&list) == 0*/) 
{
  if(!m_env29_)
  if((m_env1 & 0x02) != 0x02)
  {
    if(ls_getNext(&list)!=0x0)
    {
      //_next_book(0x0,0x01); //был последний фрагмент, закроем папку
      
//на 0 уровне видимо SD карта завершена
      _res5__=_curr_level(); //уровень открытой папки, если 0 то мы в корне SD
      if(!_res5__)
      {
          if(m_env27___)
          {
            //конец карты, выкл
            sprintf(file_name,"/SYS/_35.mp3\0");
            _play_rem(file_name,0x0);            
            sprintf(file_name,"/SYS/_3.mp3\0");
            _play_rem(file_name,0x0);
#if !_FIX1
            _init_level_tab(1);            
            memcpy(m_place._folder_01,m_place._folder_0,sizeof(m_place._folder_0));
#endif            
            sprintf(m_place._sample,"0001.lkf");  
            m_place._fpos = 0xffffffff; //нет текущей позиции текущ. фрагмента
            
            goto __4320987__;
          }        
      }
      _next_book(0x0,0x01); //был последний фрагмент, закроем папку
      
      continue;
    }
    
    if((m_env & 0x04) != 0x04)
    {
      if(!m_env29_)
      {
//VER4        
        if(!_var77 || m_env28_)
        {
          sprintf(file_name,"/SYS/_2.mp3\0");
          if(_curr_level()==1)
            _play_rem(file_name,0);
  
          if((_curr_level()<1) && _tab_empty())
          {
            sprintf(file_name,"/SYS/_8.mp3\0");
            _play_rem(file_name,0);
            _var95=0x01;
          }          
          format_file_name(file_name, list.currentEntry.FileName);
//VER4      
          if(!_var95 || ((list.currentEntry.Attribute & 0x10) == 0x10))
          {
            if(_curr_level()<=1 && (!_var96 || ((list.currentEntry.Attribute & 0x10) == 0x0)&&(_curr_level()>0)))
              _play_frag_number(file_name,0); 
          }
          _var95=0x0;
            
        }
//VER4        
        if(_var77)
          _var77=0x0;
        if(m_env28_)
          m_env28_=0x0;        
      }
    }
  }
#if 0 
      
      
//скажем фразу "Воспроиз. с текущей позиции" 
      sprintf(file_name,"/SYS/_4.mp3\0");
      _play_rem(file_name,0x01);
#endif
unsigned char _prev_k;

_1234__0987__:
for(unsigned long _var44=0;_var44<0x000000ff;_var44++)
  asm("NOP");

      
#if 0 
            if(_get_U_POWER2()==0x01)
            {
//зарядка идет
//скажем пик            
              sprintf(file_name,"/SYS/_36.mp3\0");
              _play_rem_p(file_name,0x01);              
            }
#endif
while(1)  
{
  

  
#if 0
  if(_get_U_POWER1() || _get_U_POWER2())
  {
//цикл ожидания начала воспроизв
  while((m_env3 & 0x01) == 0x01)  
  {
    if(_play_stop()==0x02)
    {
      m_env3 = m_env3 & ~0x01;
#if 0      
//скажем фразу "Воспроиз. с текущей позиции" 
      sprintf(file_name,"/SYS/_4.mp3\0");
      _play_rem(file_name,0x01);
#endif      
      _find_frag(0x0);
      m_env5 = 0; //будем позиц. по текущ. фрагменту
      break;      
    }
#if 0    
    if(_mode()==0x01)
    {
      if(m_place._bookmark_index_p==0x06)
        m_place._bookmark_index_p=0x01;

//скажем фразу "Воспроиз. c закладки №1,2,3,4,5"       
         sprintf(file_name,"/SYS/_15.mp3\0");
        _play_rem(file_name,0x01);
        _play_bookmark_number(m_place._bookmark_index_p,1);
        _find_frag(m_place._bookmark_index_p);
        m_env5 = m_place._bookmark_index_p; //будем позиц. по текущ. фрагменту
        m_place._bookmark_index_p++;
      

      m_env3 = m_env3 & ~0x01;
      break;
    }
#endif    

  }
  }
#endif

//если поставлена задача: 0 bit -> нужно отмена перехода        
 if((m_env19_ & 0x01) == 0x01)
 {
    m_env19_ &= ~0x01;
    m_env19_ |= 0x02; //отмена сделана
//скажем фразу "Отмена перехода" 
    sprintf(file_name,"/SYS/_30.mp3\0");
    _play_rem(file_name,1);   

    m_place._fpos = m_place_back._fpos;
#if !_FIX1    
    memcpy(m_place._folder_0,m_place_back._folder_0,sizeof(m_place._folder_0));
    memcpy(m_place._folder_01,m_place_back._folder_0,sizeof(m_place._folder_01));
#endif    
    sprintf(m_place._sample,"%s\0",m_place_back._sample);
    
      
    _find_frag(0);
    m_env5 = 0; //будем позиц. по текущ. фрагменту
              
    
 }
 
//если стоит задача заблокировать клаву
 if((m_env24_ & 0x81) == 0x01) 
 {
    sprintf(file_name,"/SYS/_25.mp3\0");
    _play_rem(file_name,1);
    m_env24_  = 0x80; //клава блокирована
 }
//если стоит задача разблокировать клаву
 if((m_env24_ & 0x81) == 0x81) 
 {
    sprintf(file_name,"/SYS/_26.mp3\0");
    _play_rem(file_name,1);  
    m_env24_ = 0x0; //клава разблокирована
    _keyb_reset();
 }
 
 
//если поставлена задача воспр текущей папки и фрагмента
 if((m_env22_ & 0x01) == 0x01)
 {
      
//проверим а не от сети работаем
  if((_get_U_POWER1() || _get_U_POWER2()) && !(_get_U_POWER1() && _get_U_POWER2()))
  {
    sprintf(file_name,"/SYS/_1.mp3\0"); //питание от сети
    _play_rem(file_name,0x01); 
    if(_get_U_POWER2() &&  _check3())
    {
      sprintf(file_name,"/SYS/_22.mp3\0"); //батарея заряжена
      _play_rem(file_name,0x01); 
      
    }
  }
  if(_check2())
  {
    sprintf(file_name,"/SYS/_21.mp3\0"); //батарея разряжена
    _play_rem(file_name,0x01);  
  } 
  
#if !_FIX1   
    _res2_ = _real_folder(m_place._folder_0, _folder);  
#endif

//DEBUG
    m_env32_ = _folder_file(_folder); //число фрагментов в книге на первом уровне
    
    if(_res2_ == 0x01)
    //фраза "последняя книга"
      sprintf(file_name,"/SYS/_33.mp3\0");
    else
  //скажем фразу "Переход на книгу" 
      sprintf(file_name,"/SYS/_8.mp3\0");
    _play_rem(file_name,1);
#if !_FIX1    
    sprintf(file_name,"%d.",m_place._folder_0[0]._folder_num);
//воспроизв номер книги    
    _play_frag_number(file_name,2);
#endif
 
     
    if(_curr_level()==1)
      m_env32__= m_env32_;
    else
      m_env32__ = _folder_file1(_folder); 
    
    if(m_env32__ == list.mCount)
         m_env28_ = 0x01; //последний фрагм книги
    else
         m_env28_ = 0x0;

     
    if(_curr_level()<=1)
    {
      if((m_env32_ == list.mCount) && (_res2_ == 0x01))  
//скажем фразу "последний фрагмент посл книги"      
        sprintf(file_name,"/SYS/_34.mp3\0");
      else
//скажем фразу "Фрагмент" 
        sprintf(file_name,"/SYS/_2.mp3\0");
      _play_rem(file_name,1); 
      format_file_name(file_name, list.currentEntry.FileName);
      _play_frag_number(file_name,1);
    }
    
    if(strncmp(_folder,"/LIN",strlen(_folder))==0)
        _play_rem("/SYS/_44.mp3\0",0x01);       
    if(strncmp(_folder,"/MIC",strlen(_folder))==0)
        _play_rem("/SYS/_43.mp3\0",0x01);       
    if(strncmp(_folder,"/RADIO",strlen(_folder))==0)
        _play_rem("/SYS/_45.mp3\0",0x01);       

      
    
    m_env22_ = m_env22_ & ~0x01;
 }

  
//если поставлена задача: 3 bit -> нужно след. фрагмент(set)        
 if((m_env & 0x08) == 0x08)
 {
   

      
#if 0   
if(m_env17_ != 0x03)
{
//patch от бульков
unsigned char __k_b;

      
      __k_b = __k;
      __k=0x0060;
      //DACR = (0 << 6) & 0xFFC0; //*V на DAC //FIX_DEBUG
      _var78=0x01;
      //sprintf(file_name,"/SYS/_2.mp3\0");
      sprintf(file_name,"/SYS/_36.mp3\0");
      _play_rem(file_name,1);
      _var78=0x0;
      __k=__k_b;
      //DACR = (_boost_inc << 6) & 0xFFC0; //*V на DAC //FIX_DEBUG
      
//--------------------   
}      
#endif
      
    if((m_env32__ == list.mCount))
         m_env28_ = 0x01; //последний фрагм книги
//озвучка   
    if(m_env28_)
    {
         while(!ls_getNext(&list)); //бежим на последний фрагмент
         list.currentEntry.Attribute=0x0;
         ls_getPrev(&list);       
    }
unsigned char _res7__;     
    m_env = m_env & ~0x08;
    if( (((m_mode & 0x01) == 0x01)  && m_env26_) || m_env28_) 
    {
//похоже задача идти на след. книгу
      if(m_env28_==0x01)
        _next_book(0x0,0x01); //был последний фрагмент, закроем папку
      else
      {
        _close_folder(); //закроем все папки на уровнях ниже 0
        _next_book(0x0,0x02); //перейдем на след. папку 0 уровня
         _res7__=_check_folder(); //проверка на сущест папки
    //скажем фразу "Переход на книгу"
        if(_res7__ ==0x01) 
          sprintf(file_name,"/SYS/_8.mp3\0");
        if(_res7__ ==0x03) 
          sprintf(file_name,"/SYS/_33.mp3\0");        
        if(_res7__)
          _play_rem(file_name,0);
#if !_FIX1    
        sprintf(file_name,"%d.",m_place._folder_0[0]._folder_num);
//воспроизв номер книги
        if(_res7__)
          _play_frag_number(file_name,2);
#endif
                          
      }
      if(((m_mode & 0x01) == 0x01) && !_var90)
        _var77=0x01; //по кнопке идем на новую книгу
      _var90=0x0;
#if 0     
//скажем фразу "Переход на книгу" 
    sprintf(file_name,"/SYS/_8.mp3\0");
    _play_rem(file_name,1);
#endif 
      m_env26_ = 0x0;
//VER4      
      if(!_var77)
       m_env28_ = 0x0; //сброим статус посл фраг книги
      m_env27_ = 0x0;
_54321__:      
      break;
    }

unsigned char _res4,_res5__; 

    do {
_7654__:      
    if(ls_getNext(&list) != 0)
    {
      _res5__=_curr_level(); //уровень открытой папки, если 0 то мы в корне SD
      if(!_res5__)
      {
          if(m_env27___)
          {
            //конец карты, выкл
            sprintf(file_name,"/SYS/_35.mp3\0");
            _play_rem(file_name,0x0);            
            sprintf(file_name,"/SYS/_3.mp3\0");
            _play_rem(file_name,0x0);
#if !_FIX1
            _init_level_tab(1);            
            memcpy(m_place._folder_01,m_place._folder_0,sizeof(m_place._folder_0));
#endif            
            sprintf(m_place._sample,"0001.lkf");  
            m_place._fpos = 0xffffffff; //нет текущей позиции текущ. фрагмента
            
            goto __4320987__;
          }        
      }
      
      _next_book(0x0,0x01); //был последний фрагмент, закроем папку
      goto _54321__;
    }
    else
    {
      _res4=_curr_level(); //уровень открытой папки, если 0 то мы в корне SD
      if(((list.currentEntry.Attribute & 0x10) == 0x10) && (_res4==_LEVEL_MAX)) //папка
       goto _7654__; 
      memset(file_name,'\0',sizeof(file_name));
      format_file_name(file_name, list.currentEntry.FileName);
      
    }
    }
    while(!strncmp(m_place._sample,file_name,strlen(file_name)));
    if(_var96 && !_res4)
      {
        sprintf(file_name,"/SYS/_8.mp3\0");
        _play_rem(file_name,0);
      }
    
#if !_FIX1
    _res2_ = _real_folder(m_place._folder_0, _folder);
#endif    
    if((_folder_file1(_folder) == list.mCount)  && m_env27_)
         m_env28_ = 0x01; //последний фрагм книги
    else
      m_env28_ = 0x0;
    
    if(!m_env27_)
    {
       
      if((_folder_file(_folder) == list.mCount) && (_res2_ == 0x01))
//скажем фразу "последний фрагмент посл книги"      
        sprintf(file_name,"/SYS/_34.mp3\0");
      else
//скажем фразу "Фрагмент" 
        sprintf(file_name,"/SYS/_2.mp3\0");
      if(_curr_level()==1)
        _play_rem(file_name,1);
    
//скажем номер фрагмента  
      if(_curr_level()==1)
        _play_frag_number(file_name,1);
      
    }
    else
      m_env27_ = 0x0;
 }
 
static unsigned char _cy1; 
//если поставлена задача: 1 bit -> нужно листать след. фрагмент(set) и называть номер        
 if(m_env17_ == 0x01) //только для LKF
 {
 if((m_env1 & 0x02) == 0x02)
 {
#if 0   
if(m_env17_ != 0x03)
{
//patch от бульков
unsigned char __k_b;

      
      __k_b = __k;
      __k=0x0060;
      //DACR = (0 << 6) & 0xFFC0; //*V на DAC //FIX_DEBUG
      _var78=0x01;
      //sprintf(file_name,"/SYS/_2.mp3\0");
      sprintf(file_name,"/SYS/_36.mp3\0");
      _play_rem(file_name,1);
      _var78=0x0;
      __k=__k_b;
      // DACR = (_boost_inc << 6) & 0xFFC0; //*V на DAC //FIX_DEBUG
      
//--------------------
}
#endif
    

    _cy1=0x01;
    do {
      if((_folder_file1(_folder) == list.mCount))
      {
#if 0        
         m_env28_ = 0x01; //последний фрагм книги
         m_env = m_env | 0x08;
#else
         //на новую книгу не переходим пока
               //сбросим задачу: 1 bit -> нужно след. фрагмент(set) с перечислением номеров фрагментов        
        m_env1 = m_env1 & ~0x02; 
#endif        
        
         break;
      }
        
    if(ls_getNext(&list) != 0)
    {
      _cy1=0x0;
//идем на первый фрагмент след книги 
      _next_book(0x0,0x01); //был последний фрагмент, закроем текущую папку
#if !_FIX1      
      _real_folder(m_place._folder_0, _folder);
      if (ls_openDir(&list, &(efs.myFs), _folder) != 0)
      {
          _init_level_tab(1);
      }
      _backup_n = _backup_number(); //номер последней зарытой папки
      while(_backup_n != 0)
      {
        _reset_WD(); //reset WD
        if(ls_getNext(&list) != 0)
        {
        _init_level_tab(1); //инит табл уровней как то
        break;
        }
    
        if((list.currentEntry.Attribute & 0x10) == 0x10) //папка 
          _backup_n--;
      }       
      
#endif      
//скажем фразу "Переход на книгу" 
      sprintf(file_name,"/SYS/_8.mp3\0");
      if(!_curr_level())
        _play_rem(file_name,0);
#if _FIX1      
unsigned char *p_point;    
      p_point=strchr(m_place._folder_0,'_');
      p_point++;
      sprintf(file_name,"%s.",p_point);
//воспроизв номер книги    
      _play_frag_number(file_name,2);
#endif      
//скажем фразу "Переход на фрагмент" 
      sprintf(file_name,"/SYS/_2.mp3\0");
      if(_curr_level()==1)
        _play_rem(file_name,1);
#if 0      
      _real_folder(m_place._folder_0, _folder);
      
      if (ls_openDir(&list, &(efs.myFs), _folder) != 0)
      {
          return(-2);
      }
#endif      
      ls_getNext(&list);
      memset(file_name,'\0',sizeof(file_name));
      format_file_name(file_name, list.currentEntry.FileName);
#if _FIX1       
//скажем номер фрагмента      
      _play_frag_number(file_name,1);
#endif  
//листать фрагменты прекращаем(длинное нажатие отпущено)        
      if(_forward_book()==0x03)
      {
        //сбросим задачу: 1 bit -> нужно след. фрагмент(set) с перечислением номеров фрагментов        
        m_env1 = m_env1 & ~0x02;
        m_env = m_env & ~0x08;
        break;
      }
      
      
    }
    else
    {
      if(_cy1)
      {
//скажем фразу "Переход на фрагмент" 
        sprintf(file_name,"/SYS/_2.mp3\0");
        if(_curr_level()==1)
          _play_rem(file_name,1);
        _cy1=0x0;
      }
      
      memset(file_name,'\0',sizeof(file_name));
      format_file_name(file_name, list.currentEntry.FileName);
//скажем номер фрагмента   
      if(_curr_level()<=1)
        _play_frag_number(file_name,1);
      if((_folder_file(_folder) == list.mCount))
        for(unsigned long _i1=0;_i1<0x001f0000;_i1++)
          asm("NOP");
  
//листать фрагменты прекращаем(длинное нажатие отпущено)        
      if(_forward_book()==0x03)
      {
        //сбросим задачу: 1 bit -> нужно след. фрагмент(set) с перечислением номеров фрагментов        
        m_env1 = m_env1 & ~0x02;
        break;
      }
#if !_FIX1      
      _res2_ = _real_folder(m_place._folder_0, _folder);
#endif    
      if((_folder_file1(_folder) == list.mCount))
      {
#if 0        
         m_env28_ = 0x01; //последний фрагм книги
         m_env = m_env | 0x08;
#else
         //на новую книгу не переходим пока
               //сбросим задачу: 1 bit -> нужно след. фрагмент(set) с перечислением номеров фрагментов        
        m_env1 = m_env1 & ~0x02; 
#endif        
        
         break;
      }
      else
        m_env28_ = 0x0;
      
    }
    
    
    }
    while(1);
    _cy1=0x0;

}
 }
 else
 {
           //сбросим задачу: 1 bit -> нужно след. фрагмент(set) с перечислением номеров фрагментов        
        m_env1 = m_env1 & ~0x02;
 }

 //если поставлена задача: 2 bit -> нужно пред. фрагмент(set)
 if((m_env & 0x04) == 0x04)
 {
   

#if 0   
if(m_env17_ != 0x03)
{
//patch от бульков
unsigned char __k_b;

      
      __k_b = __k;
      __k=0x0060;
      //DACR = (0 << 6) & 0xFFC0; //*V на DAC //FIX_DEBUG
      _var78=0x01;
      //sprintf(file_name,"/SYS/_2.mp3\0");
      sprintf(file_name,"/SYS/_36.mp3\0");
      _play_rem(file_name,1);
      _var78=0x0;
      __k=__k_b;
      // DACR = (_boost_inc << 6) & 0xFFC0; //*V на DAC //FIX_DEBUG
      
//--------------------   
}
#endif
   
    m_env = m_env & ~0x04;
    m_env27_ = 0x0;
    if(!_int_play())
    if(((m_mode & 0x01) == 0x01) && m_env26_) 
    {
      if(_var81)
      {
        m_env29_=0x01;
        m_env |= 0x04;
        _var81=0x0;
        _next_book(0x0,0x01); //был первый фрагмент, закроем папку
        goto _3456099;        
      }
      
      _close_folder(); //закроем все папки на уровнях ниже 0
//похоже задача идти на пред. книгу
_3456098:      
      __prev_book(0x02); 
      if((m_mode & 0x01) == 0x01)
        _var77=0x01; //по кнопке идем на новую книгу
//скажем фразу "Переход на книгу" 
     sprintf(file_name,"/SYS/_8.mp3\0");
     if(m_place._folder_0[0]._folder_num!=0xffff)
      _play_rem(file_name,0);
#if !_FIX1    
    sprintf(file_name,"%d.",m_place._folder_0[0]._folder_num);
//воспроизв номер книги    
    if(m_place._folder_0[0]._folder_num!=0xffff)
      _play_frag_number(file_name,2);
#endif
      
      
#if 0
//скажем фразу "Переход на книгу" 
    sprintf(file_name,"/SYS/_8.mp3\0");
    _play_rem(file_name,1);
#endif 
_3456099:    
      m_env26_ = 0x0;
      m_env28_ = 0x0; //сброим статутс посл фраг книги
      break;
    }


unsigned char _res1,_res3__,_res4,_res4__; 
    if(!_int_play())
    do {
_7654__0:      
    if((_res3__=ls_getPrev(&list)) != 0 || m_env29_==0x01)
    {
      if(!m_env29_)
      {
        m_env29_=0x01;
        m_env |= 0x04;
        _next_book(0x0,0x01); //был первый фрагмент, закроем папку
        goto _3456099;
      }
      
      
      
      _res1=0x0;
      _res4=_curr_level(); //уровень открытой папки, если 0 то мы в корне SD
      if((list.mCount==0x01) && !_res4)
      {
         ls_getNext(&list); 
         while(!ls_getNext(&list)); //бежим на последний фрагмент
         list.currentEntry.Attribute=0x0;
         ls_getPrev(&list); 
         
      }
      else
      {
        if(m_env29_)
        {
          _res3__=ls_getPrev(&list);
          //list.fCount++; //возможно для LKF это не надо
        }
      }
      if(_res3__!=0)
      {
        m_env29_=0x01;
        m_env |= 0x04;
//VER4
        if(!_res4)  
        {
          ls_openDir(&list, &(efs.myFs), _folder);
          while(!ls_getNext(&list)); //бежим на последний фрагмент
          list.currentEntry.Attribute=0x0;
          ls_getPrev(&list);  
          if((list.currentEntry.Attribute & 0x10) == 0x10) //папка
            _var92=0x01;
        }
        else
        {
          _next_book(0x0,0x01); //был первый фрагмент, закроем папку
          goto _3456099;
        }        
      }
      m_env29_=0x0;  
      
      if((list.currentEntry.Attribute & 0x10) == 0x10) //папка
      {
        
        while(_res1<_LEVEL_MAX)
        {
_9876000:          
          _next_book(list.fCount,0x03); //перейдем на след. папку уровня
#if !_FIX1  
          sprintf(file_name,"/SYS/_8.mp3\0");
          if(_curr_level()==1)
          {
            if(!_var92)
              _play_rem(file_name,0);
            else
              _var92=0x0;
          }
          sprintf(file_name,"%d.\0",m_place._folder_0[0]._folder_num);
//воспроизв номер книги    
          if(_curr_level()<=1)
            _play_frag_number(file_name,2);
          
          if(_curr_level()==2)
          {
//скажем фразу "Фрагмент" 
            sprintf(file_name,"/SYS/_2.mp3\0");
            _play_rem(file_name,1);            
            format_file_name(file_name, list.currentEntry.FileName);
            _play_frag_number(file_name,1);
          }
#endif
          
//идем на последний фрагмент
      //ls_getNext(&list);
          _real_folder(m_place._folder_0, _folder);
          m_env32_ = _folder_file(_folder); //число фрагментов в книге
          ls_openDir(&list, &(efs.myFs), _folder);
        
          while(!ls_getNext(&list)); //бежим на последний фрагмент
          if(ls_getPrev(&list)!=0)
            break;
          if(list.currentEntry.FileName[8] == 'M' && list.currentEntry.FileName[9] == 'P' && list.currentEntry.FileName[10] == '3')
            list.mCount++; //для LKF не надо
          if((list.currentEntry.Attribute & 0x10) == 0x10) //папка
            list.mCount++; //для LKF не надо
          
          _res4__=_curr_level(); //уровень открытой папки, если 0 то мы в корне SD
          if(((list.currentEntry.Attribute & 0x10) == 0x10) && (_res4__==_LEVEL_MAX)) //папка
            goto _7654__0; 
          if((list.currentEntry.Attribute & 0x10) == 0x0) //файл
            break;
          
        }
      }
      /*else
        list.fCount--;*/
      memset(file_name,'\0',sizeof(file_name));
      format_file_name(file_name, list.currentEntry.FileName);
      m_env = m_env & ~0x10;
      break;
    }
    else
    {
      _res4__=_curr_level(); //уровень открытой папки, если 0 то мы в корне SD
      if(((list.currentEntry.Attribute & 0x10) == 0x10) && (_res4__==_LEVEL_MAX)) //папка
       goto _7654__0; 
      
      if((list.currentEntry.Attribute & 0x10) == 0x10) //папка
        goto _9876000;
      memset(file_name,'\0',sizeof(file_name));
      format_file_name(file_name, list.currentEntry.FileName);
      
    }
    }
    while(!strncmp(m_place._sample,file_name,strlen(file_name)));
#if !_FIX1    
    _res2_ = _real_folder(m_place._folder_0, _folder);
#endif    
    if((_folder_file1(_folder) == list.mCount))
         m_env28_ = 0x01; //последний фрагм книги
    else
      m_env28_ = 0x0;
   
    if((_folder_file(_folder) == list.mCount) && (_res2_ == 0x01))
//скажем фразу "последний фрагмент посл книги"      
      sprintf(file_name,"/SYS/_34.mp3\0");
    else
//скажем фразу "Фрагмент" 
      sprintf(file_name,"/SYS/_2.mp3\0");
    if(_curr_level()==1)
      _play_rem(file_name,1);
    
//скажем номер фрагмента
    if(_curr_level()==1)
      _play_frag_number(file_name,1);

 }
 
//если поставлена задача: 2 bit -> нужно пред. фрагмент(set) с перечисл. фрагментов
 if(m_env17_ == 0x01) //только для LKF
 {
 if((m_env1 & 0x04) == 0x04)
 {
#if 0   
if(m_env17_ != 0x03)
{
//patch от бульков
unsigned char __k_b;

      
      __k_b = __k;
      __k=0x0060;
      //DACR = (0 << 6) & 0xFFC0; //*V на DAC //FIX_DEBUG
      _var78=0x01;
      //sprintf(file_name,"/SYS/_2.mp3\0");
      sprintf(file_name,"/SYS/_36.mp3\0");
      _play_rem(file_name,1);
      _var78=0x0;
      __k=__k_b;
      // DACR = (_boost_inc << 6) & 0xFFC0; //*V на DAC //FIX_DEBUG
      
//--------------------   
}
#endif

    _cy1=0x01;
    do {
    if(list.mCount==0x01)
    {
//пока мы просто остановимся на первом фрагменте      
        //сбросим задачу: 2 bit -> нужно след. фрагмент(set) с перечислением номеров фрагментов        
        m_env1 = m_env1 & ~0x04;
        break;      
    }
    if(ls_getPrev(&list) != 0  || m_env29_==0x01)
    {

      
      _cy1=0x0;
      if(!m_env29_)
      {
        m_env29_=0x01;
        m_env1 |= 0x04;
        goto _3456098; 
      }
      else
        m_env29_=0x0;      
//идем на последний фрагмент
#if !_FIX1      
      _res2_=_real_folder(m_place._folder_0, _folder);
#endif      
      if (ls_openDir(&list, &(efs.myFs), _folder) != 0)
      {
          return;
      }
      //ls_getNext(&list);
      while(!ls_getNext(&list)); //бежим на последний фрагмент
      ls_getPrev(&list);
      if(list.currentEntry.FileName[8] == 'M' && list.currentEntry.FileName[9] == 'P' && list.currentEntry.FileName[10] == '3')
        list.mCount++;      
//скажем фразу "Переход на фрагмент" 
    if((_folder_file(_folder) == list.mCount) && (_res2_ == 0x01))
//скажем фразу "последний фрагмент посл книги"      
      sprintf(file_name,"/SYS/_34.mp3\0");
    else
//скажем фразу "Фрагмент" 
      sprintf(file_name,"/SYS/_2.mp3\0");
      if(_curr_level()==1)
        _play_rem(file_name,0);
      memset(file_name,'\0',sizeof(file_name));
      format_file_name(file_name, list.currentEntry.FileName);
      m_env = m_env & ~0x10;
//скажем номер фрагмента      
      _play_frag_number(file_name,1);
  
//листать фрагменты прекращаем(длинное нажатие отпущено)        
      if(_prev_book()==0x03)
      {
        //сбросим задачу: 2 bit -> нужно след. фрагмент(set) с перечислением номеров фрагментов        
        m_env1 = m_env1 & ~0x04;
        break;
      }
      
      
    }
    else
    {
      if(_cy1)
      {
//скажем фразу "Переход на фрагмент" 
        sprintf(file_name,"/SYS/_2.mp3\0");
        if(_curr_level()==1)
          _play_rem(file_name,0);
        _cy1=0x0;
      }
      
      memset(file_name,'\0',sizeof(file_name));
      format_file_name(file_name, list.currentEntry.FileName);
//скажем номер фрагмента      
      _play_frag_number(file_name,1);
#if !_FIX1  
      _res2_ = _real_folder(m_place._folder_0, _folder);
#endif    
      if((_folder_file1(_folder) == list.mCount))
         m_env28_ = 0x01; //последний фрагм книги
      else
         m_env28_ = 0x0;
      
//листать фрагменты прекращаем(длинное нажатие отпущено)        
      if(_prev_book()==0x03)
      {
        //сбросим задачу: 2 bit -> нужно след. фрагмент(set) с перечислением номеров фрагментов        
        m_env1 = m_env1 & ~0x04;
        break;
      }
      
    }
    
    }
    while(1);
    _cy1=0x0;
 }
 }
 else
 {
           //сбросим задачу: 2 bit -> нужно след. фрагмент(set) с перечислением номеров фрагментов        
        m_env1 = m_env1 & ~0x04;
 }

unsigned char _prev_k;
    _nfrag = list.mCount;
    
//если в одной папке папке MP3, а в другой LKF -> до свидания
      if((m_env17_ == 0x03) || (m_env17_ == 0x02))
      {
          if (list.currentEntry.FileName[8] == 'L' && list.currentEntry.FileName[9] == 'K' && list.currentEntry.FileName[10] == 'F') 
          {
            _play_rem("/SYS/_17.mp3\0",0x01);
            _uninit_boost(); //выкл усилок
            for(unsigned long _l=0;_l<0x0cfffff;_l++)
              asm("NOP");       
        
            while(1)
              //IO1DIR &= ~_IO1_16_MASK; //P1.16 на input, выключим себя //FIX_DEBUG
              _turnon(0);
            
          }
      }

      
//если в одной папке папке LKF, а в другой MP3 -> до свидания
      if(m_env17_ == 0x01)
      {
          if (list.currentEntry.FileName[8] == 'M' && list.currentEntry.FileName[9] == 'P' && list.currentEntry.FileName[10] == '3') 
          {
            _play_rem("/SYS/_17.mp3\0",0x01);
            _uninit_boost(); //выкл усилок
            for(unsigned long _l=0;_l<0x0cfffff;_l++)
              asm("NOP");       
        
            while(1)
              //IO1DIR &= ~_IO1_16_MASK; //P1.16 на input, выключим себя //FIX_DEBUG
              _turnon(0);
            
          }
      }
      
      
    
      if (!((list.currentEntry.FileName[8] == 'L' && list.currentEntry.FileName[9] == 'K' && list.currentEntry.FileName[10] == 'F') //
        || (list.currentEntry.FileName[8] == 'M' && list.currentEntry.FileName[9] == 'P' && list.currentEntry.FileName[10] == '3'))
        && ((list.currentEntry.Attribute & 0x10) == 0x0) && (!_int_play()))
      {
          _play_rem("/SYS/_17.mp3\0",0x01);
#if 1          
          _uninit_boost(); //выкл усилок
          for(unsigned long _l=0;_l<0x0cfffff;_l++)
              asm("NOP");     
#else
                _dis_irq();
                m_env31_ |= 0x10;
                _booster_silence(0x01); 
                _booster(0x0);
                fs_umount(&(efs_flash.myFs));
                if(!m_env21_)
                  fs_umount(&(efs.myFs)); 
                __dma_free_channel();
unsigned char *_p;       
 	        _p = (unsigned char *)_LOAD_BLOCK_NUM; 
                memset(_p,0,4);
                *_p = GetValidBlockNumber(5); //5 модуль это radio
                _run(); //переход в _boot 2 //DEBUG   
                while(1)
                  asm("  NOP  ");
#endif          
        
          while(1)
            //IO1DIR &= ~_IO1_16_MASK; //P1.16 на input, выключим себя //FIX_DEBUG          
            _turnon(0);
      }
      else  
      {
unsigned char _res8_;        
        _res8_=_curr_level(); //уровень открытой папки, если 0 то мы в корне SD
        if(((list.currentEntry.Attribute & 0x10) == 0x10) && (_res8_==_LEVEL_MAX)) //папка
        {
          _next_book(0x0,0x01); //была папка, закроем папку в которой она
          break;
        }
        if((list.currentEntry.Attribute & 0x10) == 0x10) //папка
        {
          _next_book(list.fCount,0x03); //перейдем на след. папку уровня
          break; //уйдем открывать папку
        }
        memset(file_name,'\0',sizeof(file_name));
        format_file_name(file_name, list.currentEntry.FileName);
        sprintf(m_place._sample,"%s\0",file_name);
        _var = strlen(file_name);
#if !_FIX1        
        _real_folder(m_place._folder_0, _folder);
#endif        
#if 0        
        memmove(file_name+strlen(m_place._folder_0)+2,file_name,strlen(file_name));
        sprintf(file_name,"%s/",m_place._folder_0);
        memmove(file_name+strlen(m_place._folder_0)+1,file_name+strlen(m_place._folder_0)+2,_var);    
        file_name[strlen(m_place._folder_0)+1+_var] = '\0';
#endif
        if(strlen(_folder)>1)
        {
          memmove(file_name+strlen(_folder)+2,file_name,strlen(file_name));
          sprintf(file_name,"%s/",_folder);
          memmove(file_name+strlen(_folder)+1,file_name+strlen(_folder)+2,_var);    
          file_name[strlen(_folder)+1+_var] = '\0';

        }
        else
        {
          memmove(file_name+strlen(_folder)+1,file_name,strlen(file_name));
          sprintf(file_name,"%s",_folder);
          memmove(file_name+strlen(_folder),file_name+strlen(_folder)+1,_var);    
          file_name[strlen(_folder)+_var] = '\0';
          
        }
        if(_folder_file1(_folder)/*m_env32__*/ != list.mCount)    
          m_env28_ = 0x0;
__123_987___:
        if(_int_play())
          _int_rec = 0x01;
        if(_int_rec)
        {
              if((_rec_list == 0)) //0->переход в radio
              {
                _int_rec = 0x00;
                
                
                if(_int_play())
                  _int_rec = 0x01;
                file_name[0]=0;
              }
              if((_rec_list == 1)) //2->переход в lin rec
              {
                _int_rec = 0x00;
                
                
                if(_int_play())
                  _int_rec = 0x01;
                file_name[0]=1;
              }
              if((_rec_list == 2)) //3->переход в mic rec
              {
                _int_rec = 0x00;
                
                
                if(_int_play())
                  _int_rec = 0x01;
                file_name[0]=2;
              }          
          
        }
  
        while(1)
	if(file_fopen(&file,&efs.myFs,file_name,'r')==0)
	{
          _pos_frag(); //позиционируемся в фрагменте
          if((m_env2 & 0x01) == 0x01)
          {
//задача на воспроиз. в конце фрагмента
            if(m_env25_)
            {
              file.FilePtr = file.FileSize-(1500*80);            
              m_env25_ = 0x0;
            }
            else
              file.FilePtr = file.FileSize-(1500*20);            
            m_env2 = m_env2 & ~0x01;
            
          }          
          if(m_env17_ == 0x02)
          {
            m_env27___=0x0;
            _header_ready = 0; //заголовок не нужен, играем
            if(_int_play())
              _int_rec=0x1;
            mp3_play(&file); //играем mp3 хрень
            _int_rec=0x0;
            wait_end_of_excerpt();
          }
#if 0          
          if(m_env27_ && _detect_end())
          {
            //конец карты, выкл
            sprintf(file_name,"/SYS/_35.mp3\0");
            _play_rem(file_name,0x0);            
            sprintf(file_name,"/SYS/_3.mp3\0");
            _play_rem(file_name,0x0);
#if !_FIX1
            _init_level_tab(1);            
            memcpy(m_place._folder_01,m_place._folder_0,sizeof(m_place._folder_0));
#endif            
            sprintf(m_place._sample,"0001.lkf");  
            m_place._fpos = 0xffffffff; //нет текущей позиции текущ. фрагмента
            
            goto __4320987__;
          }
#endif          
//не надо сохранять если выход из воспроизв сделан по:
//- стоп
//- отмена перехода
//- смена режима книга/фрагмент
//- сохранение закладки          
          if(((m_env3 & 0x01) != 0x01)  
          && ((m_env19_ & 0x01) != 0x01) 
          && ((m_env & 0x20) != 0x20) 
          && ((m_env10 & 0x02) != 0x02)
          && ((m_env19_ & 0x04) != 0x04)
          && ((m_env6 & 0x02) != 0x02))  
          {
            m_env19_ &= ~0x02; //видимо идем на переход
                               //сохраним текущ. положение для отмены перехода
            m_place_back._fpos = file.FilePtr;
#if !_FIX1            
            memcpy(m_place_back._folder_0,m_place._folder_0,sizeof(m_place._folder_0));
#endif            
            if(m_env17_ == 0x03)
             _virt_sample_back(_nfrag,1);
            else
             _virt_sample_back(_nfrag,0); //делаем virt m_place._sample из реального номера фрагмента 
                                         // 0-> фрагмент LKF,1-> фрагмент MP3 
            
          }
          if((m_env19_ & 0x04) == 0x04)
            m_env19_ &=  ~0x04;
          
//если поставлена задача: 0 bit -> воспроизвести rem "стоп воспроизвед."        
          if(((m_env3 & 0x01) == 0x01))
          {
            
//сохраним текущие pos в фрагменте и сам фрагмент и книгу           
            m_place._fpos = file.FilePtr;
            //sprintf(m_place._sample,"%s\0",list.currentEntry.FileName);
            if(m_env17_ == 0x03)
             _virt_sample(_nfrag,1);
            else
             _virt_sample(_nfrag,0); //делаем virt m_place._sample из реального номера фрагмента 
                                         // 0-> фрагмент LKF,1-> фрагмент MP3 
#if !_FIX1            
            memcpy(m_place._folder_01,m_place._folder_0,sizeof(m_place._folder_0));
#endif    
            if(_int_play())
              _int_rec=0x1;
            file_fclose(&file);
            _int_rec=0x0;
#if 0   
if(m_env17_ != 0x03)
{
//patch от бульков
unsigned char __k_b;

      
      __k_b = __k;
      __k=0x0060;
      //DACR = (0 << 6) & 0xFFC0; //*V на DAC //FIX_DEBUG
      _var78=0x01;
      sprintf(file_name,"/SYS/_36.mp3\0");
      _play_rem(file_name,1);
      _var78=0x0;
      __k=__k_b;
      //DACR = (_boost_inc << 6) & 0xFFC0; //*V на DAC //FIX_DEBUG
      
//--------------------   
}
#endif
            
//скажем фразу "Остановка на текущ. позиции" 
            m_env3 = m_env3 & ~0x01;
            sprintf(file_name,"/SYS/_3.mp3\0");
            _play_rem(file_name,0x0);
            if(_int_play())
            { //выкл себя
                _booster_silence(0x01); 
                _booster(0x0);
                _turnon(0);
                while(1) asm("  NOP  ");
            }
            m_env3 = m_env3 | 0x01;
            m_wait_stop=0x01; //ждем или нажатия stop или 10сек
            //_uninit_boost1(); //выкл усилок
            m_env22_=0x02; //скажем текущую книгу и фрагмент
            if((m_env31_ & 0x01) == 0x01)
            {
              m_env31_|=0x02;
              sprintf(file_name,"/SYS/_25.mp3\0");
              _play_rem(file_name,0x0);
              m_env9 &= ~0x01; //сброим пик
            }
            // DACR = (0 << 6) & 0xFFC0; //сбросим громкость на усилке //FIX_DEBUG
            while(1)
            {
              _reset_WD();
              if(0/*((IOCONF_UART_PIN >> 1) & 0x00000001)*/) //PORT
                goto __4320987__; //идем выключаться, карту вынули

unsigned char _p1;

              _p1=_play_stop();
              if((_p1 == 0x01) || (_p1 == 0x02))
              {
                if((m_env31_ & 0x02) == 0x02)
                {
                  
                  if(_fast_down() == 0x03)
                  {
                    //DACR = (_boost_inc << 6) & 0xFFC0; //установим громкость //FIX_DEBUG
                    sprintf(file_name,"/SYS/_25.mp3\0");
                    _play_rem(file_name,0x0);
                    //DACR = (0 << 6) & 0xFFC0; //сбросим громкость на усилке //FIX_DEBUG
                    continue;
                  }
                  else
                  {
                    m_env31_=0x0;
                    //DACR = (_boost_inc << 6) & 0xFFC0; //установим громкость //FIX_DEBUG
                    sprintf(file_name,"/SYS/_26.mp3\0");
                    _play_rem(file_name,0x0);
                    //DACR = (0 << 6) & 0xFFC0; //сбросим громкость на усилке //FIX_DEBUG
                    m_env9 &= ~0x01; //сброим пик
                  }
                }
                
                m_wait_stop=0x0;
                m_env3 = m_env3 & ~0x01;
                m_ws_counter=0x0;
                _find_frag(0x0);
                m_env5 = 0; //будем позиц. по текущ. фрагменту
                //DACR = (_boost_inc << 6) & 0xFFC0; //установим громкость //FIX_DEBUG
                //_init_boost_book(); //вкл усилок
#if 1 
unsigned char _prev_k;                
//скажем фразу "Воспроиз. с текущей позиции" 
                /*sprintf(file_name,"/SYS/_11.D\0"); 
                _prev_k = __k;
                __k=0x0060;
                _play_rem(file_name,0x01);
                __k = _prev_k;*/
/*
                if(_get_U_POWER1() || _get_U_POWER2())
                {
                  sprintf(file_name,"/SYS/_1.mp3\0"); //питание от сети
                  _play_rem(file_name,0x01);  
                }
                if(_check2())
                {
                  sprintf(file_name,"/SYS/_21.mp3\0"); //батарея разряжена
                  _play_rem(file_name,0x01);  
                }
*/                
                sprintf(file_name,"/SYS/_4.mp3\0");
                _play_rem(file_name,0x01);
#endif                
                break; //играем дальше
              }
              if(m_ws_counter == (0x1a00>>2))
                goto __4320987__; //идем выключаться
            }
            break;
__4320987__:
  
//сохраняем парам и выкл. себя
            _reset_WD();
#if 0            
//скажем пик            
            sprintf(file_name,"/SYS/_36.mp3\0");
            _play_rem_p(file_name,0x01);
#endif            
            _reset_WD();
            if(m_env17_ != 0x03)
              m_place._k=__k; //сохраним текущую громкость
              m_place._boost_inc = _boost_inc;
            _param_put(0);
            _keep_key_block(m_env31_);
            //_int_off(); //не надо себя включать на всяк случай
            _reset_WD();
#if 0            
//скажем пик            
            sprintf(file_name,"/SYS/_36.mp3\0");
            _play_rem_p(file_name,0x01); 
            if(_get_U_POWER1()==0x01)
            {
//зарядка идет
//скажем пик            
              sprintf(file_name,"/SYS/_36.mp3\0");
              _play_rem_p(file_name,0x01);              
            }
#endif            
            _uninit_boost(); //выкл усилок
            fs_umount(&(efs_flash.myFs));
            fs_umount(&(efs.myFs));
            _dis_irq();
            //CP15_DCache(FALSE); 
            //CP15_ICache(FALSE);
            //IO1DIR &= ~_IO1_16_MASK; //P1.16 на input  //FIX_DEBUG
            //_soft_intr();
            _run(); //переход в _boot 2
            //_turnon(0);
            break;            
          }
          
//если поставлена задача: 0 bit -> воспроизвести rem "номер закладки"        
          if((m_env11 & 0x08) == 0x08)
          {
            m_place._fpos = file.FilePtr;
            if(_int_play())
              _int_rec=0x1;
            file_fclose(&file);
            _int_rec=0x0;
            sprintf(file_name,"/SYS/_7.mp3\0");
            _play_rem(file_name,0x0);
            m_env11 = m_env11 & ~0x08;
            m_env5=0;
             
          }
          
//если поставлена задача: 0 bit -> воспроизвести rem "номер закладки"        
          if((m_env11 & 0x04) == 0x04)
          {
            m_place._fpos = file.FilePtr;
            if(_int_play())
              _int_rec=0x1;
            file_fclose(&file);
            _int_rec=0x0;
            if(!((m_place._bookmark_index>=1)&&(m_place._bookmark_index<=5)))
              m_place._bookmark_index=0x0;
            if(m_place._bookmark_index==0x05)
              m_place._bookmark_index=0x01;
            else
              m_place._bookmark_index++;
            
            _play_bookmark_number(m_place._bookmark_index,1);
            m_env5=0;
            
            
             
          }
//поставлена задача перечислить режимы работы
//unsigned char _p2;
unsigned long _l;          
          if((m_env6 & 0x03) == 0x03)
          {
            m_env6 = m_env6 & ~0x03;
            while(1)
            {
              _p2=_options();
              if((_p2 == 0x0) && (_mode_list == 0)) //0->переход в radio
              {
                
                m_place._fpos = file.FilePtr;
                m_env5 = 0x0;
            if(_int_play())
              _int_rec=0x1;
            file_fclose(&file);
            _int_rec=0x0;
            
                if(m_sleep._mode != 0x0)
                {
                  m_sleep._mode=0x0; //сбросим sleep режим
//скажем фразу "режим сон выключен"                  
                  sprintf(file_name,"/SYS/_14.mp3\0");
                  _play_rem(file_name,0x01);
                }
           
//скажем фразу "Говорящая книга" или другие режимы       
              //sprintf(file_name,"/SYS/_%d.mp3\0",m_env8+1);
              //_play_rem(file_name,0x01);
              if(m_env8 == 8)
              {
//сохраним текущие pos в фрагменте и сам фрагмент и книгу
                if(m_env17_ == 0x03)
                  _virt_sample(list.mCount,1);
                else
                  _virt_sample(list.mCount,0); //делаем virt m_place._sample из реального номера фрагмента 
                                         // 0-> фрагмент LKF,1-> фрагмент MP3 
#if !_FIX1                
                memcpy(m_place._folder_01,m_place._folder_0,sizeof(m_place._folder_0));
#endif                
                
                _reset_WD();
                if(m_env17_ != 0x03)
                {
                  m_place._k=__k; //сохраним текущую громкость
                  m_place._boost_inc = _boost_inc;
                }
                
                _dis_irq();
                m_env31_ |= 0x10;
                _booster_silence(0x01); 
                _booster(0x0);
                fs_umount(&(efs_flash.myFs));
                if(!m_env21_)
                  fs_umount(&(efs.myFs)); 
                __dma_free_channel();
                if(!_int_play())
                  _param_put(1); //сохраним парметры 
                else   
                  _param_put(4);
                //_tci_loop();
                //CP15_InvalAllCache();
                //CP15_DCache(FALSE); 
                //CP15_ICache(FALSE);  
                _run(); //переход в _boot 2 //DEBUG  
                
               if(m_env17_ != 0x03)
                  _init_boost_book();
                else
                {
                   _init_boost_mp3(); //уровень усилка для mp3
                   //T0MCR = 0; //остановим TimerA //FIX_DEBUG
                   //VS1001_Init(); //будем играть mp3 хрень на SD  //FIX_DEBUG

                  
                }
//скажем фразу "Говорящая книга" или другие режимы      
                //m_env8=0x07;
                //sprintf(file_name,"/SYS/_%d.mp3\0",m_env8+3);
                //_play_rem(file_name,0x01);
                m_env8=0x08;
              }
              m_env8++;
              if(m_env8==0x09)
                  m_env8=0x08;
              
            }

        if((_p2 == 0x0) && (_mode_list == 1)) //2->переход в lin rec
        {
                m_place._fpos = file.FilePtr;
                m_env5 = 0x0;
            if(_int_play())
              _int_rec=0x1;
            file_fclose(&file);
            _int_rec=0x0;
            
                if(m_sleep._mode != 0x0)
                {
                  m_sleep._mode=0x0; //сбросим sleep режим
//скажем фразу "режим сон выключен"                  
                  sprintf(file_name,"/SYS/_14.mp3\0");
                  _play_rem(file_name,0x01);
                }
           
//скажем фразу "Говорящая книга" или другие режимы       
              //sprintf(file_name,"/SYS/_%d.mp3\0",m_env8+1);
              //_play_rem(file_name,0x01);
                
                _dis_irq();
                m_env31_ |= 0x10;
                _booster_silence(0x01); 
                _booster(0x0);
                fs_umount(&(efs_flash.myFs));
                if(!m_env21_)
                  fs_umount(&(efs.myFs)); 
                __dma_free_channel();
                _param_put(2); //сохраним парметры                 
                //_tci_loop();
                //CP15_InvalAllCache();
                //CP15_DCache(FALSE); 
                //CP15_ICache(FALSE);  
                _run(); //переход в _boot 2 //DEBUG  

        }
        if((_p2 == 0x0) && (_mode_list == 2)) //3->переход в mic rec
        {
                m_place._fpos = file.FilePtr;
                m_env5 = 0x0;
            if(_int_play())
              _int_rec=0x1;
            file_fclose(&file);
            _int_rec=0x0;
            
                if(m_sleep._mode != 0x0)
                {
                  m_sleep._mode=0x0; //сбросим sleep режим
//скажем фразу "режим сон выключен"                  
                  sprintf(file_name,"/SYS/_14.mp3\0");
                  _play_rem(file_name,0x01);
                }
           
//скажем фразу "Говорящая книга" или другие режимы       
              //sprintf(file_name,"/SYS/_%d.mp3\0",m_env8+1);
              //_play_rem(file_name,0x01);
                
                _dis_irq();
                m_env31_ |= 0x10;
                _booster_silence(0x01); 
                _booster(0x0);
                fs_umount(&(efs_flash.myFs));
                if(!m_env21_)
                  fs_umount(&(efs.myFs)); 
                __dma_free_channel();
                _param_put(3); //сохраним парметры                 
                //_tci_loop();
                //CP15_InvalAllCache();
                //CP15_DCache(FALSE); 
                //CP15_ICache(FALSE);  
                _run(); //переход в _boot 2 //DEBUG  

        }
        if((_p2 == 0x0) && (_mode_list == 3)) //3->переход в tts
        {
                m_place._fpos = file.FilePtr;
                m_env5 = 0x0;
            if(_int_play())
              _int_rec=0x1;
            file_fclose(&file);
            _int_rec=0x0;
            
                if(m_sleep._mode != 0x0)
                {
                  m_sleep._mode=0x0; //сбросим sleep режим
//скажем фразу "режим сон выключен"                  
                  sprintf(file_name,"/SYS/_14.mp3\0");
                  _play_rem(file_name,0x01);
                }
           
//скажем фразу "Говорящая книга" или другие режимы       
              //sprintf(file_name,"/SYS/_%d.mp3\0",m_env8+1);
              //_play_rem(file_name,0x01);
                
                _dis_irq();
                m_env31_ |= 0x10;
                _booster_silence(0x01); 
                _booster(0x0);
                fs_umount(&(efs_flash.myFs));
                if(!m_env21_)
                  fs_umount(&(efs.myFs)); 
                __dma_free_channel();
                _param_put(5); //сохраним парметры                 
                //_tci_loop();
                //CP15_InvalAllCache();
                //CP15_DCache(FALSE); 
                //CP15_ICache(FALSE);  
                _run(); //переход в _boot 2 //DEBUG  

        }
        
        _mode_list++;
        if(_mode_list>2)
          _mode_list=0;         
        switch(_mode_list)
        {
          case 0:
            sprintf(file_name,"/SYS/_%d.mp3\0",m_env8+1);
            _play_rem(file_name,1);
            for(_l=0;_l<0x00ffffff;_l++)
              asm(" NOP ");
          break; 
          case 1:
            _play_rem((unsigned char *)"/sys/_41.mp3",1);
            for(_l=0;_l<0x00ffffff;_l++)
              asm(" NOP ");            
          break; 
          case 2:
            _play_rem((unsigned char *)"/sys/_40.mp3",1);
            for(_l=0;_l<0x00ffffff;_l++)
              asm(" NOP ");            
          break;   
          case 3:
            _play_rem((unsigned char *)"/sys/_46.mp3",1);
            for(_l=0;_l<0x00ffffff;_l++)
              asm(" NOP ");            
          break;           
        };          
            
            }
          }
          
//поставлена задача перечислить режимы воспр
          if((m_env100 & 0x01) == 0x01)
          {
            m_env100 = m_env100 & ~0x01;

              
          }          

#if 0
//поставлена задача пикнуть сек
          if(((m_env & 0x04) != 0x04) && ((m_env & 0x08) != 0x08))
          if(((m_env7 & 0x04) == 0x04) && (m_env17_ != 0x03))
          {
            
            m_place._fpos = file.FilePtr;
            m_env5 = 0x0;

            file_fclose(&file);
            _int_rec=0x0;

//скажем фразу "пик" быстро      
              sprintf(file_name,"/SYS/_36.mp3\0");
              _play_rem_p1(file_name,0x01);
              _clear_buff();
          }
#endif          
//поставлена задача воспр. пик
          if((m_env9 & 0x01) == 0x01)
          {
            m_env9 = m_env9 & ~0x01;
            m_place._fpos = file.FilePtr;
            m_env5 = 0x0;

            file_fclose(&file);
            _int_rec=0x0;

//скажем фразу "пик"       
              sprintf(file_name,"/SYS/_36.mp3\0");
              _play_rem(file_name,0x01); 
          }
          
//поставлена задача измен темп корректора
          if(!_int_rec)
          if((m_env100 & 0x01) == 0x01)
          {
            m_env100 = m_env100 & ~0x01;
            m_place._fpos = file.FilePtr;
            m_env5 = 0x0;
            if(_int_play())
              _int_rec=0x1;
            file_fclose(&file);
            _int_rec=0x0;

            //_stretch_init(_k1);
          }          
          
//поставлена задача идти на закладку
          if((m_env6 & 0x02) == 0x02)
          {
            
            if(_int_play())
              _int_rec=0x1;
            file_fclose(&file);
            _int_rec=0x0;

_765433200:            
            do{
            if((m_env17_ != 0x01) && (m_place._bookmark_index_p==0x06)) //только для LKF
            {
              m_place._bookmark_index_p=0x01;
              break;
            }
              
            if(m_place._bookmark_index_p==0x06)
            {
              
//скажем фразу "В конец книги"
               sprintf(file_name,"/SYS/_16.mp3\0");
                _play_rem(file_name,0x01);
              //_find_frag(m_place._bookmark_index_p);
                m_env5=0x06;
              m_env2 |= 0x01;
              m_place._bookmark_index_p=0x01;
              m_env12=0x0;
            }
            else
            {

  
//скажем фразу "Воспроиз. c закладки №1,2,3,4,5"
//найдем не пустую закладку
              while(1)
              {
                if(m_place._bookmark_index_p == 0x01)
                {
                  if(m_place._fpos_1 != 0xffffffff)
                    break;
                }
                if(m_place._bookmark_index_p == 0x02)
                {
                  if(m_place._fpos_2 != 0xffffffff)
                    break;
                }
                if(m_place._bookmark_index_p == 0x03)
                {
                  if(m_place._fpos_3 != 0xffffffff)
                    break;
                }
                if(m_place._bookmark_index_p == 0x04)
                {
                  if(m_place._fpos_4 != 0xffffffff)
                    break;                  
                }
                if(m_place._bookmark_index_p == 0x05)
                {
                  if(m_place._fpos_5 != 0xffffffff)
                    break;                  
                }
                if(m_place._bookmark_index_p == 0x06)
                {
                     goto _765433200;                
                            
                }                
                m_place._bookmark_index_p++;
              }
              if(m_env12==0x0)
              {
                sprintf(file_name,"/SYS/_15.mp3\0");
                _play_rem(file_name,0x01);
                m_env12=0x01;
              }  
              _play_bookmark_number(m_place._bookmark_index_p,1);
              //_find_frag(m_place._bookmark_index_p);
              m_env5 = m_place._bookmark_index_p; //будем позиц. по текущ. фрагменту
              m_place._bookmark_index_p++;
              m_env2 &= ~0x01;
            }  
            for(unsigned long _var78=0;_var78<=0x00cfffff;_var78++)
              asm("NOP");
            
              if((_var55=_sleep()) == 0x04)
              {
                break;
              }
              
            }
            while(1);
            _find_frag(m_env5);
            m_env6 = m_env6 & ~0x02;
          }          
          
          if((m_env6 & 0x01) == 0x01)
          {
//поставлена задача: 0 bit -> нужно сказать фразу "Сон через ..."(set)            
            _fpos = file.FilePtr;
            if(_int_play())
              _int_rec=0x1;
            file_fclose(&file);
            _int_rec=0x0;


            m_env6 = m_env6 & ~0x01;
            if(m_sleep._mode == 0x01)
            {
//скажем фразу "Сон через 15min"             
              _play_rem((char *)"/SYS/_11.mp3\0",0x0);
            }
            if(m_sleep._mode == 0x02)
            {
//скажем фразу "Сон через 30min"             
              _play_rem((char *)"/SYS/_12.mp3\0",0x0);
            }
            if(m_sleep._mode == 0x03)
            {
//скажем фразу "Сон через 45min"             
              _play_rem((char *)"/SYS/_13.mp3\0",0x0);
            }
            if(m_sleep._mode == 0x0)
            {
//скажем фразу "Сон выкл."             
              _play_rem((char *)"/SYS/_14.mp3\0",0x0);
            }
                
            m_env6 = m_env6 | 0x01;
            
          }
          
          
          if((m_env & 0x20) == 0x20)
          {
//поставлена задача: 5 bit -> нужно сказать фразу "Переход по книгам"(set)            
            _fpos = file.FilePtr;
            if(_int_play())
              _int_rec=0x1;
            file_fclose(&file);
            _int_rec=0x0;


            m_env = m_env & ~0x20;
            if((m_mode &0x01) == 0x01)
            {
//скажем фразу "Режим по книгам"             
              _play_rem((char *)"/SYS/_5.mp3\0",0x0);
            }
            else
            {
//скажем фразу "Режим по фрагментам"             
              _play_rem((char *)"/SYS/_6.mp3\0",0x0);            
            }
            m_env = m_env | 0x20;
            
          }
          else
          if((m_env & 0x40) == 0x40)
          {
//поставлена задача: 6 bit -> нужно сказать фразу "Вперед по фрагменту"(set)
            if(file.FilePtr >= file.FileSize-(file.FileSize/10))            
            {
//если мы уже в клонце фрагмента чего "вперед по фрагменту ходить"
//идем на след. фрагмент              
             if(_int_play())
              _int_rec=0x1;
            file_fclose(&file);
            _int_rec=0x0;

              m_env = m_env & ~0x40;
              m_env = m_env | 0x08;
              break;
               
            }
            _fpos = file.FilePtr;
            //file_fclose(&file);
//скажем фразу "Вперед по фрагменту" 
            //m_env = m_env & ~0x40;
            //_play_rem((char *)"/SYS/_2.D\0",0x01);
            //m_env = m_env | 0x40;
            
          }
          else
          if((m_env & 0x80) == 0x80)
          {
//поставлена задача: 7 bit -> нужно сказать фразу "Назад по фрагменту"(set)
            _fpos = file.FilePtr;
            //file_fclose(&file);
//скажем фразу "Назад по фрагменту" 
            //m_env = m_env & ~0x80;
            //_play_rem((char *)"/SYS/_3.D\0",0x01);
            //m_env = m_env | 0x80;
            
          }
          else          
          {
            if(_int_play())
              _int_rec=0x1;
            file_fclose(&file);
            _int_rec=0x0;

            break;
          }
        }
        /*else
        {
          
          while(1)
            IO1DIR &= ~_IO1_16_MASK; //P1.16 на input, выключим себя           
        }*/

      } 
  
}
}
}
  fs_umount(&(efs.myFs));
 
  return;

  
}


