/*****************************************************************************\
*              efs - General purpose Embedded Filesystem library              *
*          --------------------- -----------------------------------          *
*                                                                             *
* Filename : file.h                                                           *
* Description : Headerfile for file.c                                         *
*                                                                             *
* This program is free software; you can redistribute it and/or               *
* modify it under the terms of the GNU General Public License                 *
* as published by the Free Software Foundation; version 2                     *
* of the License.                                                             *
                                                                              *
* This program is distributed in the hope that it will be useful,             *
* but WITHOUT ANY WARRANTY; without even the implied warranty of              *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               *
* GNU General Public License for more details.                                *
*                                                                             *
* As a special exception, if other files instantiate templates or             *
* use macros or inline functions from this file, or you compile this          *
* file and link it with other works to produce a work based on this file,     *
* this file does not by itself cause the resulting work to be covered         *
* by the GNU General Public License. However the source code for this         *
* file must still be made available in accordance with section (3) of         *
* the GNU General Public License.                                             *
*                                                                             *
* This exception does not invalidate any other reasons why a work based       *
* on this file might be covered by the GNU General Public License.            *
*                                                                             *
*                                                    (c)2006 Lennart Yseboodt *
*                                                    (c)2006 Michael De Nil   *
\*****************************************************************************/

#ifndef __NAND_FILE_H_
#define __NAND_FILE_H_

/*****************************************************************************/
#include "types.h"
/*****************************************************************************/
#include "nand.h"

#define MODE_READ 0x72
#define MODE_WRITE 0x77
#define MODE_APPEND 0x61

#define FILE_STATUS_OPEN 0
#define FILE_STATUS_WRITE 1

#define IMG_BLOCK 12 //начальный блок с озвучкой 


//#define BEGIN_BLOCK   30
extern unsigned long BEGIN_BLOCK;
//#define END_BLOCK     130
extern unsigned long END_BLOCK;

#define CLASTER_SIZE  50 //число блоков в кластере
                         //последний блок кластера TMP блок !!!!!
#define PARAM_PAGE    63 //номер страницы с параметрами в 
                                        //блоке

//#define SYSTEM_BLOCK  END_BLOCK
extern unsigned long SYSTEM_BLOCK;
#define SYSTEM_PAGE   0
//#define TMP_BLOCK     END_BLOCK+1
extern unsigned long TMP_BLOCK;

#define N_CLASTER     16 //(END_BLOCK-BEGIN_BLOCK)/CLASTER_SIZE //число кластеров


#define MAX_ID      0xfffffffe
#define EMPTY_ID    0xffffffff

#define TYPE_RADIO  0
#define TYPE_MIC    1
#define TYPE_LIN    2

#define MAX_IN_BLOCK  50 //max число фрагментов в блоке может лежать  
                         //(PAGE_SIZE*PAGES_IN_BLOCK)/5e+3 (5k min размер фраг)



typedef struct {
  unsigned long _id; //id фрагмента(натур число, уникально для типа, получается путем inc)
  unsigned char _type; //тип фрагмента
  unsigned long _begin_page; //начальная страницах от 0
  unsigned long _begin_offset; //смещение внутри начальной страницы 
  unsigned long _end_page; //последняя страницах от 0
  unsigned long _end_fill; //сколько занято внутри последней страницы 
} _param_page;


typedef struct {
  unsigned long _id_begin;
  unsigned long _id_end;
  unsigned long _block_begin;
  unsigned long _block_end;  
} _id_span;


typedef struct {
  unsigned long _id;
  _id_span  _radio_span[2];
  _id_span  _mic_span[2];
  _id_span  _lin_span[2];
} _system_page;

typedef struct {
  unsigned long _id_radio;
  unsigned long _id_mic;
  unsigned long _id_lin;
  unsigned long _block;
  unsigned long _page;
  unsigned long _offset;
  unsigned char _ctrl; //0 bit -> список текущего блока считан из flash
                       //1 bit -> список текущего блока commited
                       //2 bit -> список кластеров commited 
                       //4 bit -> список текущего блока изменен 
} _end_param;


typedef struct {
  unsigned long _id; //id фрагмента(натур число, уникально для типа, получается путем inc)
  unsigned char _type; //тип фрагмента
  unsigned long _begin_block; //начальный блок фрагмента для чтения
  unsigned long _begin_page; //начальная страницах
  unsigned long _begin_offset; //смещение внутри начальной страницы 
  unsigned long _end_page; //последняя страницах
  unsigned long _end_fill; //сколько занято внутри последней страницы 
} _file_param;


typedef struct {
  unsigned long _begin_block; //начальный блок фрагмента для чтения
  unsigned long _begin_page; //начальная страницах
  unsigned long _begin_offset; //смещение внутри начальной страницы 
  unsigned long _end_page; //последняя страницах
  unsigned long _end_fill; //сколько занято внутри последней страницы   
} _b_frag;


extern _param_page _frag_list[MAX_IN_BLOCK];
extern _system_page _claster_list[N_CLASTER];
extern _end_param  _curr_param;
extern unsigned char page[PAGE_SIZE];
extern _file_param _frag_param;
extern unsigned char _swap_id;

extern unsigned char _init_nand_w(void);
extern void _sys_page_modify(unsigned long _block,unsigned long _id,unsigned long _type);
extern void _sys_page_commit(unsigned char _p);
extern unsigned char _init_frag_list(unsigned _block);
extern void _frag_list_modify(unsigned long _id, unsigned char _type,
                       unsigned long _begin_page,unsigned long _begin_offset,
                       unsigned long _end_page,unsigned long _end_fill
                      );
extern void _frag_list_commit(unsigned _block);
extern _b_frag _begin_frag(unsigned long _id,unsigned long _type);
extern void _restore_page(unsigned long _block,unsigned long _page,unsigned long _offset);
extern void _del_frag_list(unsigned long _page,unsigned long _offset);

extern unsigned long efile_fopen(unsigned long _id, unsigned char _type, eint8 mode);
extern esint8 efile_fclose(void);
extern euint32 efile_read(euint32 size,euint8 *buf);
extern euint32 efile_write(euint32 size,euint8* buf); 

#endif
