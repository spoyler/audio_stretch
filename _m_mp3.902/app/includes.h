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

//����� ��������� ��������
#define _STATUS_REG       0x111575C6
#define _LOAD_BLOCK_NUM   0x111575CA
#define _FIRST_FILE_SIZE  0x111575CE
#define _KEYBS_LOCK       0x111575D2
#define _VER              0x111575D3
#define _M_PLACE          0x111575D8



//0x81 -> ���������� ���������
//0x80 -> ����� ������ ��������
extern unsigned char _header_ready;
extern unsigned long _bitrate;		/* stream bitrate (bps) */
extern unsigned int _samplerate;		/* sampling frequency (Hz) */

//��� IAP
extern unsigned long _iap_write_data(unsigned cclk,unsigned flash_address,unsigned char* flash_data_buf, unsigned count);
extern unsigned long _iap_erase_sector(unsigned start_sector,unsigned end_sector,unsigned cclk);
extern unsigned long _iap_prepare_sector(unsigned start_sector,unsigned end_sector,unsigned cclk);
extern unsigned char _iap_find_erase_prepare_sector(unsigned cclk, unsigned flash_address);
extern void _iap_entry(unsigned param_tab[],unsigned result_tab[]);
#define _IAP_BUFF_SIZE  256

extern unsigned char _src_buff[_IAP_BUFF_SIZE];

struct _level_tab
{
unsigned short _folder_num; //����� ������� �������� ����� �� ������
unsigned short _file_num; //����� ��������� �������� ����� �� ������,
                          //���� � ��� ��� ����� �� ��������� ����� 
                          //�� ������ ��� ���������� ���� ����� ���� ��
                          //����� ����� ����� ��������� ���� ��� �����
};

#if 1
struct _place
{
//�� ������ ��������� � ��������  
  struct _level_tab _folder_0[_LEVEL_MAX]; //������� ������� ������� ��������� 
  struct _level_tab _folder_01[_LEVEL_MAX]; //����������� ������� ������� �������� �������� 
  unsigned char _sample[15]; //�����. ��������  
  euint32 _fpos;  //������� � ������� ��������� 
  struct _level_tab _folder_1[_LEVEL_MAX]; //������� �������� 1
  unsigned char _sample_1[15]; //�������� �������� 1  
  euint32 _fpos_1;  //������� � ��������� �������� 1
  struct _level_tab _folder_2[_LEVEL_MAX]; //������� �������� 2
  unsigned char _sample_2[15]; //�������� �������� 2  
  euint32 _fpos_2;  //������� � ��������� �������� 2
  struct _level_tab _folder_3[_LEVEL_MAX]; //������� �������� 3
  unsigned char _sample_3[15]; //�������� �������� 3  
  euint32 _fpos_3;  //������� � ��������� �������� 3
  struct _level_tab _folder_4[_LEVEL_MAX]; //������� �������� 4
  unsigned char _sample_4[15]; //�������� �������� 4  
  euint32 _fpos_4;  //������� � ��������� �������� 4
  struct _level_tab _folder_5[_LEVEL_MAX]; //������� �������� 5
  unsigned char _sample_5[15]; //�������� �������� 5  
  euint32 _fpos_5;  //������� � ��������� �������� 5
  unsigned char _bookmark_index; //������ ������� �������� �� ������
  unsigned char _bookmark_index_p; //������ ������� �������� �� ���������.
//���� �����. �� ��������� �� ���� -> "\0"  
  unsigned char _k; //������� ��������� dac
  unsigned short _boost_inc; //������� ��������� dac
  unsigned char _d; //��������� dsp
};

#pragma location = _M_PLACE
extern __no_init struct _place m_place;
#endif


#if 1
struct _place_back
{
//�� ������ ��������� � ��������  
  struct _level_tab _folder_0[_LEVEL_MAX]; //�����. ������� ���� 01,02... 
  struct _level_tab _folder_01[_LEVEL_MAX]; //����������� �����. ������� ���� 01,02... 
  unsigned char _sample[15]; //�����. ��������  
  euint32 _fpos;  //������� � ������� ��������� 
};


extern struct _place_back m_place_back;
#endif


struct _sleep
{
  unsigned long _max_time; //������� ��� �� T1MR0(50ms) �� ��� �������, 0x0 ��� ����
  unsigned long _min; // ������� ��� �� T1MR0(50ms) �� ���
//  15 min = 18000;//((15*60)/50e-3)  
//  30 min = 36000;//((30*60)/50e-3)    
//  45 min = 54000;//((45*60)/50e-3)    
  
  unsigned char _mode; //0->����., 0x1->15min, 0x2->30min, 0x3->45min
};

extern struct _sleep m_sleep;

//������
//0 bit -> ����� �����.(set)
//1 bit -> ����� ����(set)
//2 bit -> ����� ����. ��������(set)
//3 bit -> ����� ����. ��������(set)
//4 bit -> ����� � ����� ��������� � � ��������� �������� �����(set)
//5 bit -> ����� ������� ����� "������� �� ������"(set)
//6 bit -> ����� ������� ����� "������ �� ���������"(set)
//7 bit -> ����� ������� ����� "����� �� ���������"(set)
extern unsigned char m_env;
//������
//1 bit -> ����� ����������� "1,2,3...." ��� ��������� �� ����������(set) ������
//2 bit -> ����� ����������� "1,2,3...." ��� ��������� �� ����������(set) �����
extern unsigned char m_env1;
//������
//0 bit -> ������� �� ������(set),������� �� ����������(unset)
extern unsigned char m_mode;

//������
//0 bit -> ����. �������� � �������� � �����(set)
extern unsigned char m_env2;

//������
//0 bit -> stop(set)
//1 bit -> ��������. � �����/��������(set)
extern unsigned char m_env3;


//������
//0 bit -> ���������� ������� ��������(set)
extern unsigned char m_env4;

//������
//0xff -> ������ ���
//0 -> ����������������� �� ���. ���������
//1 -> ����������������� �� �������� 1
//2 -> ����������������� �� �������� 2
//3 -> ����������������� �� �������� 3
//4 -> ����������������� �� �������� 4
//5 -> ����������������� �� �������� 5
extern unsigned char m_env5;

//������
//0 bit -> ������� ����� "��� ����� ..."(set)
extern unsigned char m_env6;

//������
//0 bit -> 0 bit set ����� ������ ��������� �����
extern unsigned char m_env7;

//������ ������: ��������� �����, mp3 � ��
extern unsigned char m_env8;
//����� ������������� ����� ���� ���
extern unsigned char m_env9;
extern unsigned char m_env10;
extern unsigned char m_env11;
extern unsigned char m_env12;
extern unsigned char m_env14;
extern unsigned char m_env15;
extern unsigned char m_env16;
extern unsigned char m_env17_;
//������
//0 bit -> 1 ������ �� ������ ��������
//1 bit -> 1 ������ �������� �������
//2 bit -> 1 �������� ���������
extern unsigned char m_env19_;
//������ ���������� ��������� player
extern unsigned char m_env20_;
//������ ��� �����
extern unsigned char m_env21_;

//������� ������� ����� � ��������
extern unsigned char m_env22_;

//������� ���������
extern unsigned char m_env23_;


//����������
//0x01 -> �������������
//0x02 -> ��������������
extern unsigned char m_env24_;
extern unsigned char _b_s_down;
extern unsigned char _b_s_up;
extern unsigned char _b_vol_down;

//������ ����� ����������� ��������� �����
extern unsigned char m_env25_;

//������� ������� �� ���� ����� ���������� ����� �� ������ 
extern unsigned char m_env26_;

//������ ����� ���������
extern unsigned char m_env27_;

//������ ����� ��������� ��� ���������� SD �����
extern unsigned char m_env27___;

//������ ���������� ���������
extern unsigned char m_env28_;


//������ �������� � ����. �����
extern unsigned char m_env29_;


//������ ������ ������ ����� � ������
extern unsigned char m_env30_;


//������ ���������� � ����
//0 bit -> ���� �����������
//1 bit -> �������������
//4 bit -> �� ���� ��������� ����������
extern unsigned char m_env31_;

//����� ���������� � ������� �����
extern unsigned short m_env32_;

//����� ���������� � ������� �����
extern unsigned short m_env32__;

//������ ����� �����
extern unsigned char m_env33;


//������ ������� ������ sleep
extern unsigned char _var55;


//������ �������� �� ����� �����
extern unsigned char _var77;


//������ ������ � dac ������
extern unsigned char _var78;

//����� ������ � ���: 0x01->si4704, 0x02-> ������� B801-B
extern unsigned char _var79;

//�� ���������� ������ � mp3 ������ �����
extern unsigned char _var81; 

//������ ���� ��� � ������ mp3 ���� �� ��������� ������ �� ���� ����� �� ������� ������ 
extern unsigned char _var90;

//������� ��� �������� ����� ������� ����� ����� � ����� ����� �� �����
extern unsigned char _var92;

//������� �� ��������� ������ ����� �� 0 ������
extern unsigned char _var95;

//������� �� �� ��� � ����� ���� ����� � �� ������ �����
extern unsigned char _var96;

//��������� �������� ���� ����������
extern unsigned char m_env100;

extern unsigned short _nfrag;
extern void _virt_sample_back(unsigned short _num,unsigned char _ext);

//������� ��� ������ ���������
extern unsigned long m_time_play;


//�������� ���������������(timer0 tick)
extern float _k1;
//������� ���������
//extern unsigned short _k;
extern unsigned char __k;
extern unsigned short _boost_inc;
//������� ���������� ����������� �� ��������� ������
extern unsigned char _max_counter;

extern unsigned char _file__[10];
extern unsigned char _ext__[10];

extern unsigned char _int_rec; //0 bit->1 ����� �� ���� ������


//��������������� ������� �������
extern void _play_rem(char *_p, char _par);
//��������������� ������� ������� ��� radio
extern void _play_rem_r(char *_p, char _par, short _rvol);


extern euint32 _fpos;

//����� ������ rem �� �������� � player 
//lock ����� � �����
extern unsigned char m_mylock;


extern unsigned char m_wait_stop; //���� 0x01->������ 2��� �������� ����
extern unsigned long m_ws_counter; //������� ��������� �������� ����



//�������� m_place.* �� flash
extern void _param_put(unsigned char _p1);
//������� m_place.* �� flash
extern unsigned char _param_get();
extern unsigned char _param_init();
extern void _put_ver(unsigned char *_p);
extern unsigned char _get_ver(unsigned char *_p);
extern void _my_main();


//handle FS �� flash
extern EmbeddedFileSystem  efs_flash;
//handle FS �� SD
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
