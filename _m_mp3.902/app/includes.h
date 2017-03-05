/***************************************************************************
 **
 **
 **    Master inlude file
 **
 **    Used with ARM IAR C/C++ Compiler
 **
 **    (c) Copyright IAR Systems 2007
 **
 **    $Revision: 31493 $
 **
 ***************************************************************************/

#ifndef __INCLUDES_H
#define __INCLUDES_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <intrinsics.h>
#include <math.h>
#include <assert.h>

#include "arm_comm.h"

#include "arm926ej_cp15_drv.h"
#include "arm_comm.h"
#include "drv_cgu.h"
#include "drv_spi.h"
#include "drv_spinor.h"
#include "drv_intc.h"

#include "usb_cnfg.h"
#include "usb_desc.h"
#include "usb_hw.h"
#include "usb_buffer.h"
#include "usb_t9.h"
#include "usb_hooks.h"
#include "usb_dev_desc.h"

#include "audio_desc.h"
#include "audio_class.h"

#include "drv_i2c.h"
#include "UDA1380.h"

#include "efs.h"
#include "ls.h"
#include "mkfs.h"
#include "debug.h"
#include "inttypes.h"

#include "lpc313x_dma_driver.h"

#include "lpc313x_intc_driver.h"
#include "lpc313x_adc10b_driver.h"

#include "nand_file.h"

#define _IO1_16_MASK   0x00000001 << 16

#define _LEVEL_MAX    6
#define _FIX1 0

//карта системных областей
#define _STATUS_REG       0x111575C6
#define _LOAD_BLOCK_NUM   0x111575CA
#define _FIRST_FILE_SIZE  0x111575CE
#define _KEYBS_LOCK       0x111575D2
#define _VER              0x111575D3
#define _M_PLACE          0x111575D8



//0x81 -> готовность заголовка
//0x80 -> нужен только заголовк
extern unsigned char _header_ready;
extern unsigned long _bitrate;		/* stream bitrate (bps) */
extern unsigned int _samplerate;		/* sampling frequency (Hz) */

//для IAP
extern unsigned long _iap_write_data(unsigned cclk,unsigned flash_address,unsigned char* flash_data_buf, unsigned count);
extern unsigned long _iap_erase_sector(unsigned start_sector,unsigned end_sector,unsigned cclk);
extern unsigned long _iap_prepare_sector(unsigned start_sector,unsigned end_sector,unsigned cclk);
extern unsigned char _iap_find_erase_prepare_sector(unsigned cclk, unsigned flash_address);
extern void _iap_entry(unsigned param_tab[],unsigned result_tab[]);
#define _IAP_BUFF_SIZE  256

extern unsigned char _src_buff[_IAP_BUFF_SIZE];

struct _level_tab
{
unsigned short _folder_num; //номер текущей открытой папки на уровне
unsigned short _file_num; //номер последней закрытой папки на уровне,
                          //дело в том что когда мы закрываем папку 
                          //на уровне нам необходимо чуть позже знат ее
                          //номер чтобы найти следующий файл или папку
};

#if 1
struct _place
{
//мы всегда находимся в каталоге  
  struct _level_tab _folder_0[_LEVEL_MAX]; //текущая таблица уровней каталогов 
  struct _level_tab _folder_01[_LEVEL_MAX]; //сохраненный таблица уровней текущего каталога 
  unsigned char _sample[15]; //текущ. фрагмент  
  euint32 _fpos;  //позиция в текущем фрагменте 
  struct _level_tab _folder_1[_LEVEL_MAX]; //каталог закладки 1
  unsigned char _sample_1[15]; //фрагмент закладки 1  
  euint32 _fpos_1;  //позиция в фрагменте закладки 1
  struct _level_tab _folder_2[_LEVEL_MAX]; //каталог закладки 2
  unsigned char _sample_2[15]; //фрагмент закладки 2  
  euint32 _fpos_2;  //позиция в фрагменте закладки 2
  struct _level_tab _folder_3[_LEVEL_MAX]; //каталог закладки 3
  unsigned char _sample_3[15]; //фрагмент закладки 3  
  euint32 _fpos_3;  //позиция в фрагменте закладки 3
  struct _level_tab _folder_4[_LEVEL_MAX]; //каталог закладки 4
  unsigned char _sample_4[15]; //фрагмент закладки 4  
  euint32 _fpos_4;  //позиция в фрагменте закладки 4
  struct _level_tab _folder_5[_LEVEL_MAX]; //каталог закладки 5
  unsigned char _sample_5[15]; //фрагмент закладки 5  
  euint32 _fpos_5;  //позиция в фрагменте закладки 5
  unsigned char _bookmark_index; //индекс текущей закладки на запись
  unsigned char _bookmark_index_p; //индекс текущей закладки на воспроизв.
//если текущ. не определен то поле -> "\0"  
  unsigned char _k; //текущая громкость dac
  unsigned short _boost_inc; //текущая громкость dac
  unsigned char _d; //громкость dsp
};

#pragma location = _M_PLACE
extern __no_init struct _place m_place;
#endif


#if 1
struct _place_back
{
//мы всегда находимся в каталоге  
  struct _level_tab _folder_0[_LEVEL_MAX]; //текущ. каталог типа 01,02... 
  struct _level_tab _folder_01[_LEVEL_MAX]; //сохраненный текущ. каталог типа 01,02... 
  unsigned char _sample[15]; //текущ. фрагмент  
  euint32 _fpos;  //позиция в текущем фрагменте 
};


extern struct _place_back m_place_back;
#endif


struct _sleep
{
  unsigned long _max_time; //сколько раз по T1MR0(50ms) до сна текущее, 0x0 сон выкл
  unsigned long _min; // сколько раз по T1MR0(50ms) до сна
//  15 min = 18000;//((15*60)/50e-3)  
//  30 min = 36000;//((30*60)/50e-3)    
//  45 min = 54000;//((45*60)/50e-3)    
  
  unsigned char _mode; //0->выкл., 0x1->15min, 0x2->30min, 0x3->45min
};

extern struct _sleep m_sleep;

//задачи
//0 bit -> нужно воспр.(set)
//1 bit -> нужно стоп(set)
//2 bit -> нужно пред. фрагмент(set)
//3 bit -> нужно след. фрагмент(set)
//4 bit -> нужно в конец фрагмента и с небольшим отступом назад(set)
//5 bit -> нужно сказать фразу "Переход по книгам"(set)
//6 bit -> нужно сказать фразу "Вперед по фрагменту"(set)
//7 bit -> нужно сказать фразу "Назад по фрагменту"(set)
extern unsigned char m_env;
//задачи
//1 bit -> нужно перечислять "1,2,3...." для переходов по фрагментам(set) вперед
//2 bit -> нужно перечислять "1,2,3...." для переходов по фрагментам(set) назад
extern unsigned char m_env1;
//режимы
//0 bit -> переход по книгам(set),переход по фрагментам(unset)
extern unsigned char m_mode;

//задачи
//0 bit -> пред. фрагмент с отступом в конце(set)
extern unsigned char m_env2;

//задачи
//0 bit -> stop(set)
//1 bit -> воспроиз. с текущ/закладки(set)
extern unsigned char m_env3;


//задачи
//0 bit -> сохранение текущей закладки(set)
extern unsigned char m_env4;

//задачи
//0xff -> задачи нет
//0 -> позиционироваться по тек. фрагменту
//1 -> позиционироваться по закладке 1
//2 -> позиционироваться по закладке 2
//3 -> позиционироваться по закладке 3
//4 -> позиционироваться по закладке 4
//5 -> позиционироваться по закладке 5
extern unsigned char m_env5;

//задачи
//0 bit -> сказать фразу "Сон через ..."(set)
extern unsigned char m_env6;

//задачи
//0 bit -> 0 bit set режим хитрой перемотки назад
extern unsigned char m_env7;

//режимы работы: говорящая книга, mp3 и тд
extern unsigned char m_env8;
//режим необходимости воспр звук ПИК
extern unsigned char m_env9;
extern unsigned char m_env10;
extern unsigned char m_env11;
extern unsigned char m_env12;
extern unsigned char m_env14;
extern unsigned char m_env15;
extern unsigned char m_env16;
extern unsigned char m_env17_;
//задачи
//0 bit -> 1 задача на отмену перехода
//1 bit -> 1 отмена перехода сделана
//2 bit -> 1 закладка сохранена
extern unsigned char m_env19_;
//статус возможнсти выключить player
extern unsigned char m_env20_;
//статус нет карты
extern unsigned char m_env21_;

//сказать текущую книгу и фрагмент
extern unsigned char m_env22_;

//автомат перемотка
extern unsigned char m_env23_;


//блокировка
//0x01 -> заблокировать
//0x02 -> разблокировать
extern unsigned char m_env24_;
extern unsigned char _b_s_down;
extern unsigned char _b_s_up;
extern unsigned char _b_vol_down;

//статус конца последненго фрагмента книги
extern unsigned char m_env25_;

//автомат переход по фраг когда установлен режим по книгам 
extern unsigned char m_env26_;

//статус конца фрагмента
extern unsigned char m_env27_;

//статус конца фрагмента для завершения SD карты
extern unsigned char m_env27___;

//статус последнего фрагмента
extern unsigned char m_env28_;


//статус перехода в пред. книгу
extern unsigned char m_env29_;


//статус сброса поиска радио в начало
extern unsigned char m_env30_;


//статус блокировки в выкл
//0 bit -> надо блокировать
//1 bit -> заблокировано
//4 bit -> не надо проверять блокировку
extern unsigned char m_env31_;

//число фрагментов в текущей книге
extern unsigned short m_env32_;

//число фрагментов в текущей папке
extern unsigned short m_env32__;

//статус новой карты
extern unsigned char m_env33;


//статус нажатой кнопки sleep
extern unsigned char _var55;


//статус перехода на новую книгу
extern unsigned char _var77;


//статус записи в dac тишины
extern unsigned char _var78;

//какой модуль у нас: 0x01->si4704, 0x02-> наверно B801-B
extern unsigned char _var79;

//по внутренним папкам в mp3 ходить будем
extern unsigned char _var81; 

//статус того что в режиме mp3 клик по перемотке вперед на след папку на внутрен уровне 
extern unsigned char _var90;

//затычка для ситуации когда сначала файлы пишут а потом папки на карту
extern unsigned char _var92;

//затычка на воспроизв номера книги на 0 уровне
extern unsigned char _var95;

//затычка на то что в корне есть файлы а не только папки
extern unsigned char _var96;

//изменения настроек темп корректора
extern unsigned char m_env100;

extern unsigned short _nfrag;
extern void _virt_sample_back(unsigned short _num,unsigned char _ext);

//счетчик для хитрой перемотки
extern unsigned long m_time_play;


//скорость воспроизведения(timer0 tick)
extern float _k1;
//уровень громкости
//extern unsigned short _k;
extern unsigned char __k;
extern unsigned short _boost_inc;
//счетчик убыстрения перемещения по фрагменту вперед
extern unsigned char _max_counter;

extern unsigned char _file__[10];
extern unsigned char _ext__[10];

extern unsigned char _int_rec; //0 bit->1 воспр из внут памяти


//воспроизведение озвучки событий
extern void _play_rem(char *_p, char _par);
//воспроизведение озвучки событий для radio
extern void _play_rem_r(char *_p, char _par, short _rvol);


extern euint32 _fpos;

//когда звучит rem на действие в player 
//lock клавы и задач
extern unsigned char m_mylock;


extern unsigned char m_wait_stop; //если 0x01->статус 2сек ожидания выкл
extern unsigned long m_ws_counter; //счетчик интервала ожидания выкл



//сохраним m_place.* на flash
extern void _param_put(unsigned char _p1);
//возьмем m_place.* на flash
extern unsigned char _param_get();
extern unsigned char _param_init();
extern void _put_ver(unsigned char *_p);
extern unsigned char _get_ver(unsigned char *_p);
extern void _my_main();


//handle FS на flash
extern EmbeddedFileSystem  efs_flash;
//handle FS на SD
extern EmbeddedFileSystem  efs;

extern void _reset_WD();
extern void _init_WD();
extern void _fast_init_WD();
extern void _clear_buff();

extern void _en_irq(void);
extern void _dis_irq(void);

extern long btea(long* v, long length);
extern void _init_boost();
extern void _uninit_boost();
extern void _init_boost1();
extern void _uninit_boost1();
extern void _init_boost_book();
extern unsigned char  _boost_book_inc(unsigned short _vol);
extern unsigned char  _boost_book_dec(unsigned short _vol);
extern void _init_boost_mp3();
void _init_boost_radio(unsigned short _vol);
extern void _shutdown_boost1();
extern void _on_boost();
extern unsigned char _check1();
extern unsigned char _check2();

extern void I2C_Init(void);
extern void _radio_keys();
extern void _turn_on_radio();
extern void _turn_off_radio();
extern void _set_ch(unsigned long _ch);
extern unsigned long _get_curr_ch();
extern unsigned long autosearchup(unsigned long cha);
extern unsigned long _scan_up(unsigned long cha);
extern unsigned short _num_frag(unsigned char *_p);
extern unsigned char _real_folder(struct _level_tab *_in, unsigned char *_out);
extern void _mp3_play(EmbeddedFile *file);
extern void _mp3_voices(EmbeddedFile *file);
extern void _play_number_r(unsigned long _n);

extern unsigned short ADC_Read();

extern unsigned char _get_key_block();
extern unsigned char _keep_key_block(unsigned char _code);
unsigned short _num_frag(unsigned char *_p);
extern void _init_render();
extern unsigned  char _get_file_size(unsigned long _curr);
extern unsigned  char _keep_file_size(unsigned long _code);
extern unsigned char _check3();
extern void _patch_dot__(unsigned char *_src);
extern void _trim(unsigned char *_str);
extern void _put_empty_ver();
extern void _param_to_tmp(void);
extern void _myi2s_clk(void);
void _set_srate(unsigned short sample_rate);
extern void _set_master(void);

extern DMAC_REGS_T *dmaregs;

extern unsigned char _int_play(void);

extern unsigned long _iplay_id;


#endif  // __INCLUDES_H
