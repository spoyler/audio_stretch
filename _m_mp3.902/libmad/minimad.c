/*
 * libmad - MPEG audio decoder library
 * Copyright (C) 2000-2004 Underbit Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: minimad.c,v 1.4 2004/01/23 09:41:32 rob Exp $
 */

# include <stdio.h>
# include <efs.h>
# include <ls.h>
# include <string.h>

#include "includes.h"
#include "mad.h"
#include "debug.h"
#include "lpc_io_stereo.h"
#include "midmad.h"

#define SAMPLE_F  22050

unsigned char _header_ready=0x0;
unsigned long _bitrate=0;		/* stream bitrate (bps) */
unsigned int _samplerate=0;		/* sampling frequency (Hz) */


static unsigned char  mp3_stream_buf[512*3];
static EmbeddedFile  *mp3_file;


unsigned char  _tone_up(void); //из keyb.c
unsigned char  _tone_down(void); //из keyb.c
unsigned char  _play_stop(void); //из keyb.c
unsigned char  _forward_frag(void); //из keyb.c
unsigned char  _prev_frag(void); //из keyb.c
unsigned char  _fast_up(void); //из keyb.c
unsigned char  _fast_down(void); //из keyb.c
unsigned char  _mode(void); //из keyb.c
void set_dac_sample_rate(unsigned int sample_rate); //из lpc_io.c

unsigned char m_env1=0x0;
unsigned char m_env2=0x0;
unsigned char m_env3=0x01;
unsigned char m_env4=0x0;
unsigned char m_env6=0x0;
unsigned char m_env7=0x0;
unsigned char m_env8=0x08; //режимы работы
unsigned char m_env9=0x0; //задача на воспр. звука пик
unsigned char m_env10=0x0;
unsigned char m_env11=0x0; //задача на воспр. сохранение закладки + номер
unsigned char m_env12=0x0;
unsigned char m_env14=0x0;
unsigned char m_env15=0x0; //режимы:0x0->книга/0x01->всякая mp3 хрень
unsigned char m_env16=0x0;
unsigned char __k=0x0060;//15*16+1;
unsigned short _boost_inc=0x0000;
unsigned char m_env18=0x0;
unsigned char m_env19_=0x0;
unsigned char m_env20_=0x0;
unsigned char m_env21_=0x0;
unsigned char m_env22_=0x0;
unsigned char m_env23_=0x0;
unsigned char m_env24_=0x0;
unsigned char _b_vol_down=0x0;
unsigned char _b_vol_down1=0x0;
unsigned char m_env25_=0x0;
unsigned char m_env26_=0x0;
unsigned char m_env27_=0x0;
unsigned char m_env27___=0x01;
unsigned char m_env28_=0x0;
unsigned char m_env29_=0x0;
unsigned char _var55=0x0;
unsigned char m_env31_=0x0;
unsigned short m_env32_=0;
unsigned short m_env32__=0;
unsigned char m_env33=0;
unsigned char _var77=0x0;
unsigned char _var78=0x0;
unsigned char _var81=0x0;
unsigned char _var90=0x0;
unsigned char _b_s_up=0x0;
unsigned char _var92=0x0;
unsigned char _var95=0x0;
unsigned char _var96=0x0;
unsigned char m_env100=0x0; //изменения настроек темп корректора

unsigned long m_time_play=0x0;

/*			  
 * This is perhaps the simplest example use of the MAD high-level API.
 * Standard input is mapped into memory via mmap(), then the high-level API
 * is invoked with three callbacks: input, output, and error. The output
 * callback converts MAD's high-resolution PCM samples to 16 bits, then
 * writes them to standard output in little-endian, stereo-interleaved
 * format.
 */
static int decode(unsigned char const *, unsigned long);

void abort(void)
{
}

void mp3_play(EmbeddedFile *file)
{ 
    mp3_file = file;
    decode((void *)mp3_stream_buf, sizeof(mp3_stream_buf)); 
}

/*
 * This is a private message structure. A generic pointer to this structure
 * is passed to each of the callback functions. Put here any data you need
 * to access from within the callbacks.
 */

struct buffer {
  unsigned char const *start;
  unsigned long length;
};

/*
 * This is the input callback. The purpose of this callback is to (re)fill
 * the stream buffer which is to be decoded. In this example, an entire file
 * has been mapped into memory, so we just call mad_stream_buffer() with the
 * address and length of the mapping. When this callback is called a second
 * time, we are finished decoding.
 */

static
enum mad_flow input(void *data, struct mad_stream *stream)
{
  struct buffer *buffer = data;
  unsigned int lb , rb = 0;
  unsigned char _b_forward_book;
  unsigned char _b_prev_book;
  unsigned char _b_prev_frag;
  unsigned char _b_next_frag;
  unsigned char _mode1;
  
  
  _reset_WD(); //reset WD
  
  
#if 0 //DEBUG  
//я буду decode некое число frame для demo и останавливать play
static unsigned char _frame_counter=0xff;


  _frame_counter++; //будем считать frames с 0x0
  if (_frame_counter == 0x90)
  {
    _frame_counter=0xff;
    return MAD_FLOW_STOP; //завершим play
  }
#endif
    

  
if(!m_mylock) //проверим не стои ли lock
{
  
    if((m_env22_ & 0x02) == 0x02)
    {
      m_env22_ |= 0x01;
      m_env22_ &= ~0x02;
     m_place._fpos = mp3_file->FilePtr;
        
     m_env5=0;      
      return MAD_FLOW_STOP;
    }
    
#if 1
    
//посмотрим не надо ли менять уровень громкости
//DEBUG    
    //if(_volume_up()==0x01)
    if(((_b_vol_down1=_volume_up())==0x01) && (!m_env24_))
    {
      if(__k>0x4C)
      {
        __k-=4;
        _aic23_svolume(__k);
      }
      else
      {
        //m_env9 |= 0x01; //поставим задачу воспр. пик 
        _aic23_svolume(__k);
      }
    }
    
#if 1    
    //_b_vol_down=_volume_down();
    if(_b_vol_down1 && _b_s_down && !m_env24_)
    {
      m_env24_ |= 0x01;
     m_place._fpos = mp3_file->FilePtr;
        
     m_env5=0;      
      return MAD_FLOW_STOP;
    }

    if(_b_vol_down1 && _b_s_down && (m_env24_ & 0x01))
    {
      m_env24_ |= 0x02;
     m_place._fpos = mp3_file->FilePtr;
        
     m_env5=0;      
      return MAD_FLOW_STOP;
    }
#endif
    _b_vol_down=_volume_down();
    if((m_env24_ & 0x01) == 0x01)
    {
     m_place._fpos = mp3_file->FilePtr;
        
     m_env5=0;      
      
      return MAD_FLOW_STOP;
    }
    
    
    //if((_b_vol_down) && !m_env24_ && !((IO0PIN >> 16)&0x00000001))//FIX_DEBUG
    if((_b_vol_down) && !m_env24_ && !((IOCONF_GPIO_PIN >> 14)&0x00000001))
    {
      if((0x7f-4)>__k)
      {
        _aic23_svolume(__k);
        __k+=4;
        
      }
      else
      {
        __k = 0x7f;
        _aic23_svolume(__k);
        //m_env9 |= 0x01; //поставим задачу воспр. пик
      }
      
    }
    
#endif
  
  if((m_env9 & 0x01) == 0x01)
    return MAD_FLOW_STOP;
    
  if((m_env100 & 0x01) == 0x01)
    return MAD_FLOW_STOP;    
    
    
  if((m_env4 & 0x01) == 0x01)  
  {
//мы сказали фразу "закладка сохранена" и теперь возвращаемся на
//позицию в текущем фрагменте     
      m_env4 = m_env4 & ~0x01;
      mp3_file->FilePtr = _fpos;
    
  }
  
  if((m_env & 0x10) == 0x10)
   {
//в этот фрагмент мы попали через сдвиг назад из след. фрагмента
//идем в конец с отступом назад     
    m_env = m_env & ~0x10;
    mp3_file->FilePtr = mp3_file->FileSize - 50000;
   }
  
  
  if((m_env & 0x20) == 0x20)
   {
//мы сказали фразу "Переход по книгам" и теперь возвращаемся на
//позицию в текущем фрагменте     
      m_env = m_env & ~0x20;
      mp3_file->FilePtr = _fpos;
   }          


  if((m_env & 0x40) == 0x40)
   {
//мы сказали фразу "Вперед по фрагменту" и теперь возвращаемся на
//позицию в текущем фрагменте     
      m_env = m_env & ~0x40;
      mp3_file->FilePtr = _fpos;
   }          
  

  if((m_env & 0x80) == 0x80)
   {
//мы сказали фразу "Назад по фрагменту" и теперь возвращаемся на
//позицию в текущем фрагменте     
      m_env = m_env & ~0x80;
      mp3_file->FilePtr = _fpos;
   }          

  if((m_env6 & 0x01) == 0x01)
   {
//мы сказали фразу "Сон через ..." и теперь возвращаемся на
//позицию в текущем фрагменте     
      m_env6 = m_env6 & ~0x01;
      mp3_file->FilePtr = _fpos;
   }          
  

  _b_forward_book=_forward_book(); //считаем здесь, позже проанализируем
  _b_prev_book=_prev_book(); //считаем здесь, позже проанализируем
  _b_prev_frag=_prev_frag(); //считаем здесь, позже проанализируем
  _b_next_frag= _forward_frag(); //считаем здесь, позже проанализируем
  //_b_prev_book;

  //тест навигации по внутр фрагментам внутр памяти
if((m_env1 & 0x06) == 0x0)
{
//может нужен след. фрагмент на воспр.(короткое нажатие)        
  if(_b_forward_book==0x01)
   {
        //поставим задачу: 3 bit -> нужно след. фрагмент(set)        
        m_env = m_env | 0x08;
        m_env26_ = 0x01;
        //m_env28_=0x0;
        
        return MAD_FLOW_STOP;
   }

  
//может нужен пред. фрагмент на воспр.                
  if(_b_prev_book==0x01)  
   {
        //поставим задачу: 2 bit -> нужно пред. фрагмент(set)
        m_env = m_env | 0x04;
        m_env26_ = 0x01;
        
        return MAD_FLOW_STOP;
   }
}

  
if(!_int_rec)
{
//может нужна отмена перехода
  if((_b_prev_frag==0x02) && (_b_prev_book==0x02) && ((m_env19_ & 0x02) == 0))
  {
        //поставим задачу: 0 bit -> отмена перехода         
        m_env19_ = m_env19_ | 0x01;
        return MAD_FLOW_STOP;    
  }

  
if((m_mode & 0x01) == 0x0) //длинное нажатие при переходе только по фрагментам
{
  
//может нужен след. фрагмент на воспр.(длинное нажатие)        
  if(_b_forward_book==0x02)
   {
        //поставим задачу: 1 bit -> нужно след. фрагмент(set) с перечислением номеров фрагментов        
        m_env1 = m_env1 | 0x02;
        //m_env28_=0x0;
        return MAD_FLOW_STOP;
   }
  
//может нужен пред. фрагмент на воспр.(длинное нажатие)        
  if(_b_prev_book==0x02 && (_b_prev_frag==0x01))
   {
        //поставим задачу: 2 bit -> нужно пред. фрагмент(set) с перечислением номеров фрагментов        
        m_env1 = m_env1 | 0x04;
        return MAD_FLOW_STOP;
   }
}

if((m_env1 & 0x06) == 0x0)
{
//может нужен след. фрагмент на воспр.(короткое нажатие)        
  if(_b_forward_book==0x01)
   {
        //поставим задачу: 3 bit -> нужно след. фрагмент(set)        
        m_env = m_env | 0x08;
        m_env26_ = 0x01;
        //m_env28_=0x0;
        
        return MAD_FLOW_STOP;
   }

  
//может нужен пред. фрагмент на воспр.                
  if(_b_prev_book==0x01)  
   {
        //поставим задачу: 2 bit -> нужно пред. фрагмент(set)
        m_env = m_env | 0x04;
        m_env26_ = 0x01;
        
        return MAD_FLOW_STOP;
   }
}


//----------------
//во время воспр. выбираем номер закладки для сохранения
  if(((m_env10 & 0x02)!=0x02))
  {
  _mode1 = _mode();
//может переключим режим перехода: книги/фрагменты                
  if(_mode1==0x01)  
   {
        //сменим режим перехода: книги/фрагменты                
        m_mode = m_mode ^ 0x01;
        //поставим задачу: 5 bit -> нужно сказать фразу "Переход по книгам"(set)
        m_env = m_env | 0x20;
        return MAD_FLOW_STOP;
   }

//сохраним закладку  
  if(_mode1==0x02)  
   {
      m_env10 |= 0x02; //режим сохранения закладки on
      m_time_play = 17; //счетчик времени воспр. номеров закладок
      m_env11 = m_env11 | 0x08;
      return MAD_FLOW_STOP;
   }
    
  }
  
  if(((m_env10 & 0x02)==0x02))
  {
     if((m_env11 & 0x04) == 0x04)
     {
      m_env11 = m_env11 & ~0x04; //после 2-x сек фразы о сохранен. заклаки
      m_time_play++;
     }
     
    while((m_env10 & 0x02)==0x02)
    {
      if((m_time_play%16)==0)
      {
        m_env11 |= 0x04; //поставим задачу сказать фразу о сохр. закладки
        return MAD_FLOW_STOP;
      }
//завершим сохранение закладок
      if(_mode()==0x03)  
      {
        m_env10 = 0; //режим сохранения закладки завершен
//сохраним текущие позицию в закладке           
        _save_bookmark_par(m_place._bookmark_index);
        m_env5=0;
        return MAD_FLOW_STOP;        
      }
    }
   }


//----------------  
  

//во время воспр. играем вперед с убыстрением  
  if(_max_counter==0x02)  
   {
        _max_counter=0x03; //пусть быстрее смещается по фраг. вперед
//поставим задачу: 6 bit -> нужно сказать фразу "Вперед по фрагменту"(set)
        m_env = m_env | 0x40;
        return MAD_FLOW_STOP;

   }

  
  
unsigned char _var10; 
  if(((m_env7 & 0x02)!=0x02))
//во время воспр. скачем назад по удержанию
  if(((m_env7 & 0x08)!=0x08))
  {
   _var10= _b_prev_frag;//_prev_frag();
   if((_var10==0x02) && (_b_prev_book==0x03))  
   {
      m_env7 |= 0x08; //режим хитрой назад вперед начат
      m_time_play = 7; //счетчик времени воспр. кусочков при хитрой перемотке назад  
      //VICIntEnClear |= 0x0000001<<4; //FIX_DEBUG 
      wait_end_of_excerpt();
   }
   if(_var10==0x03)
   {
//скачем на 10сек назад     
     if(buffer->length*40>=mp3_file->FilePtr)
     {
          mp3_file->FilePtr=0;
          m_env = m_env | 0x04;
          m_env2 |= 0x01; //нужен конец фрагмента

     }
     else
     {
      mp3_file->FilePtr -= buffer->length*40;
      m_place._fpos = mp3_file->FilePtr;
      m_env5=0;
     }
     return MAD_FLOW_STOP;
     
   }   
  }
#if 0  
  if(((m_env7 & 0x02)!=0x02))
  if(((m_env7 & 0x08)==0x08))
  {
     if((_b_next_frag == 0x03) || (_b_next_frag == 0x02))
     {
        m_env23_ = 0x01; //пусть автоматом мотается назад
     }
    
     if((m_env7 & 0x04) == 0x04)
     {
       if(m_env18==10)
       {
         m_env7 = m_env7 & ~0x04; //после сек пика
         m_time_play++;
         m_env18=0x0;
         m_place._fpos = mp3_file->FilePtr;
         //VICIntEnClear |= 0x0000001<<4; //FIX_DEBUG 
         wait_end_of_excerpt();
       }
       else
         m_env18++;
     }
     
    if((m_env7 & 0x04) == 0x0) 
    while((m_env7 & 0x08)==0x08)
    {
      if((m_time_play%6)==0)
      {
        m_env7 |= 0x04; //поставим задачу пикнуть что сек прошла
        _clear_buff();
        
        m_env18=0x0;
        if(((unsigned long)((m_time_play-6)/6))<=1)
        {
//скачем на 15сек назад 
          if(5*buffer->length*12>=mp3_file->FilePtr)
          {
            mp3_file->FilePtr=0;
            m_env = m_env | 0x04;
            m_env2 |= 0x01; //нужен конец фрагмента
            
          }
          else
          {
            mp3_file->FilePtr -= 5*buffer->length*12;
            m_place._fpos = mp3_file->FilePtr;
            m_env5=0;
          }
        }
        else
        {
//скачем на 45сек назад          
          if(buffer->length*170>=mp3_file->FilePtr)
          {
            mp3_file->FilePtr=0;
            m_env = m_env | 0x04;
            m_env2 |= 0x01; //нужен конец фрагмента

          }
          else
          {
            mp3_file->FilePtr -= buffer->length*170;
            m_place._fpos = mp3_file->FilePtr;
            m_env5=0;
          }
        }
        
        //VICIntEnable |= 0x0000001<<4; //FIX_DEBUG         
        enable_audio_render();
        return MAD_FLOW_STOP;
      }
//завершим хитрую перемотку назад
      //if(_prev_frag()==0x01) 
      if((_b_prev_frag==0x01)  && (m_env23_ != 0x01))   
      {
        m_env7 = 0; //режим хитрой перемотки назад завершен
        /*if(((unsigned long)((m_time_play-6)/6))<=5)
        {
//первые 5 сек -> 1сек=3сек смещения          
          mp3_file->FilePtr -= ((unsigned long)((m_time_play-6)/6))*buffer->length*10;
        }
        else
        {
//с 6 сек -> 1сек=45сек смещения          
          mp3_file->FilePtr -= 5*buffer->length*10;
          mp3_file->FilePtr -= (((unsigned long)((m_time_play-6)/6))-5)*buffer->length*160;
        }
        if(((long)mp3_file->FilePtr) < 0)
          mp3_file->FilePtr=0;
        
        m_place._fpos = mp3_file->FilePtr;*/
        _b_prev_frag=0x0;
        _b_next_frag=0x0;

        
        m_env5=0;
        //VICIntEnable |= 0x0000001<<4; //FIX_DEBUG 
        enable_audio_render();
        return MAD_FLOW_STOP;
      }
    }
   }
#endif


  
//во время воспр. скачем вперед по удержанию
  if(((m_env7 & 0x08)!=0x08))
  if(((m_env7 & 0x02)!=0x02))
  {
   _var10= _b_next_frag;//_forward_frag();
   if(_var10==0x02)  
   {
      m_env7 |= 0x02; //режим хитрой перемотки вперед начат
      m_time_play = 7; //счетчик времени воспр. кусочков при хитрой перемотке назад  
      //VICIntEnClear |= 0x0000001<<4; //FIX_DEBUG
      wait_end_of_excerpt();
      //m_env28_=0x0;
   }
   if(_var10==0x03)
   {
//скачем на 10сек вперед
     //m_env28_=0x0;     
     if(buffer->length*30>=(mp3_file->FileSize-mp3_file->FilePtr))
     {
          mp3_file->FilePtr=mp3_file->FileSize-buffer->length*10;
          m_env = m_env | 0x08;
          //m_env28_=0x0;
     }
     else
     {
        mp3_file->FilePtr += buffer->length*30;
        m_place._fpos = mp3_file->FilePtr;
        m_env5=0;
     }
     return MAD_FLOW_STOP;
     
   }
  }
#if 0  
  if(((m_env7 & 0x08)!=0x08))
  if(((m_env7 & 0x02)==0x02))
  {
     if((_b_prev_frag == 0x03) || (_b_prev_frag == 0x02))
     {
        m_env23_ = 0x01; //пусть автоматом мотается назад
     }
     if((m_env7 & 0x04) == 0x04)
       if(m_env18==10)
       {
        m_env7 = m_env7 & ~0x04; //после сек пика
        m_time_play++;
        
        m_env18=0x0;
        m_place._fpos = mp3_file->FilePtr;
        //VICIntEnClear |= 0x0000001<<4; //FIX_DEBUG
        disable_audio_render();
       }
       else
         m_env18++;
     
    if((m_env7 & 0x04) == 0x0) 
    while((m_env7 & 0x02)==0x02)
    {
      if((m_time_play%6)==0)
      {
        m_env7 |= 0x04; //поставим задачу пикнуть что сек прошла
        _clear_buff();
        
        m_env18=0x0;
        if(((unsigned long)((m_time_play-6)/6))<=1)
        {
//скачем на 15сек вперед
          if(5*buffer->length*10>=(mp3_file->FileSize-mp3_file->FilePtr))
          {
            mp3_file->FilePtr=mp3_file->FileSize-buffer->length*10;
            m_env = m_env | 0x08;
            //m_env28_=0x0;
          }
          else
          {
            mp3_file->FilePtr += 5*buffer->length*10;
          
          
            m_place._fpos = mp3_file->FilePtr;
            m_env5=0;
          }
        }
        else
        {
//скачем на 45сек вперед          
          
          if(buffer->length*160>=(mp3_file->FileSize-mp3_file->FilePtr))
          {
            mp3_file->FilePtr=mp3_file->FileSize-buffer->length*10;
            m_env = m_env | 0x08;
            //m_env28_=0x0;
          }
          else
          {
            mp3_file->FilePtr += buffer->length*160;
          
            m_place._fpos = mp3_file->FilePtr;
            m_env5=0;
          }
        }
        
        //VICIntEnable |= 0x0000001<<4; //FIX_DEBUG
        enable_audio_render();
        return MAD_FLOW_STOP;
      }
//завершим хитрую перемотку вперед
      if((_b_next_frag==0x01) && (m_env23_ != 0x01))  
      {
        m_env7 = 0; //режим хитрой перемотки вперед завершен
        /*if(((unsigned long)((m_time_play-6)/6))<=5)
//первые 5 сек -> 1сек=3сек смещения          
          mp3_file->FilePtr += ((unsigned long)((m_time_play-6)/6))*buffer->length*10;
        else
        {
//с 6 сек -> 1сек=45сек смещения          
          mp3_file->FilePtr += 5*buffer->length*10;
          mp3_file->FilePtr += (((unsigned long)((m_time_play-6)/6))-5)*buffer->length*160;
        }
        m_place._fpos = mp3_file->FilePtr;
        */
        _b_prev_frag=0x0;
        _b_next_frag=0x0;
        
        m_env5=0;
        //VICIntEnable |= 0x0000001<<4; //FIX_DEBUG
        enable_audio_render();
        return MAD_FLOW_STOP;
      }
    }
   }
#endif
 



#if 1  
  if((_sleep()==0x01))
  {
    m_sleep._mode++;
    if(m_sleep._mode == 0x04)
      m_sleep._mode=0x0;
    
    m_sleep._max_time =0x0;
    switch(m_sleep._mode)
    {
    case 0x01:
         m_sleep._min = 13000;//19000; //15 мин 
        break;
    case 0x02:
         m_sleep._min = 26000;//38100; //30 мин 
        break;
    case 0x03:
         m_sleep._min = 38500;//57200; //45 мин 
        break;
        
    };
    
    m_env6 = m_env6 | 0x01;
    return MAD_FLOW_STOP;
      
  }
#endif
 

static unsigned char _var30=0x0; 
  _var55=_sleep();
  if(!_var55)
   _var30=0x0; 
  

//переход по закладкам  
  if((_var55==0x02))
  {
    
    m_env6 = m_env6 | 0x02;
    if(!_var30)
    {
      m_place_back._fpos = mp3_file->FilePtr;
#if !_FIX1      
      memcpy((void *)m_place_back._folder_0,(void *)m_place._folder_0,sizeof(m_place._folder_0));
#endif      
      if(m_env17_ == 0x03)
       _virt_sample_back(_nfrag,1);
      else
       _virt_sample_back(_nfrag,0);  
      _var30=0x01;
      m_env19_ &= ~0x02; //сохраним текущ. положение для отмены перехода      
    }
    return MAD_FLOW_STOP;
      
  }
#if 1  
  if(m_env12==0x01)
  if((_var55==0x04))
  {
    
    m_env12=0x0;
      
  }
#endif  
  
}
//переход по режимам работы   
  if((_options()==0x02))
  {
    
    m_env6 = m_env6 | 0x03;
    return MAD_FLOW_STOP;
      
  }  

//переход по режимам воспроизв  
  if(_int_rec)
  if((_mode()==0x04))
  {
    
    m_env100 = m_env100 | 0x01;
    return MAD_FLOW_STOP;
      
  }  

  //if(!_int_rec) 
  if(m_env20_==0x01)
  {
    if((m_env3 & 0x01) == 0x0)
    {
unsigned char _p1;
     _p1 = _play_stop();      
    if((_p1 == 0x01) || (_p1 == 0x02) || ((m_sleep._max_time >= m_sleep._min) && (m_sleep._mode != 0x0)))
    {
      if(m_env23_ == 0x01)
      {
        _b_prev_frag=0x0;
        _b_next_frag=0x0;
        
        m_env7 = 0; //режим хитрой перемотки
        m_env23_ = 0x0;
        m_env5=0xff; //сбросим
        //VICIntEnable |= 0x0000001<<4; //FIX_DEBUG
        enable_audio_render();
        return 0x0;
        
      }
      
      m_env3 = m_env3 | 0x01; //остановим воспроизв
      m_sleep._max_time = 0; //сбросим режим сон
    
      return MAD_FLOW_STOP;
      
    }
    }
  }
  else
    _play_stop();

}
  
  if (!buffer->length)
    return MAD_FLOW_STOP;

  if (stream->this_frame && stream->next_frame)
  {
    rb = (unsigned int)(buffer->length) - 
         (unsigned int)(stream->next_frame - stream->buffer);

    memmove((void *)stream->buffer, (void *)stream->next_frame, rb);
    lb = file_read(mp3_file, buffer->length - rb, (void *)(stream->buffer + rb));
    if(_int_rec && (lb < (buffer->length - rb)))
    {
      _iplay_id++;
      return MAD_FLOW_STOP;    
    }
  }
  else 
  {
    lb = file_read(mp3_file, buffer->length, (void *)buffer->start);
    if(_int_rec && (lb < (buffer->length)))
    {
      _iplay_id++;
      return MAD_FLOW_STOP;
    }    
  }

  
  if (lb == 0)
  {
    if(((IOCONF_UART_PIN >> 1) & 0x00000001) && !m_env21_)
    {
      _turnon(0x0);
      while(1);
    }
    
    wait_end_of_excerpt();
    buffer->length = 0;
    if(!m_mylock) //проверим не стои ли lock
    //поставим задачу: 3 bit -> нужно след. фрагмент(set)
    {
      m_env = m_env | 0x08;
      m_env27_ = 0x01;
      m_env27___=0x01;
    }
    
    return MAD_FLOW_STOP;
  }
  else 
    buffer->length = lb + rb;

  mad_stream_buffer(stream, buffer->start, buffer->length);
  //  buffer->length = 0;

  return MAD_FLOW_CONTINUE;
}

/*
 * The following utility routine performs simple rounding, clipping, and
 * scaling of MAD's high-resolution samples down to 16 bits. It does not
 * perform any dithering or noise shaping, which would be recommended to
 * obtain any exceptional audio quality. It is therefore not recommended to
 * use this routine if high-quality output is desired.
 */

static inline
signed int scale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

/*
 * This is the output callback function. It is called after each frame of
 * MPEG audio data has been completely decoded. The purpose of this callback
 * is to output (or play) the decoded PCM audio.
 */

static
enum mad_flow output(void *data,
		     struct mad_header const *header,
		     struct mad_pcm *pcm)
{
  unsigned int nchannels, nsamples;
  unsigned int samplerate;
  //  unsigned int i;

  /* pcm->samplerate contains the sampling frequency */

  nchannels = pcm->channels;
  nsamples  = pcm->length;
  samplerate = pcm->samplerate;

  

  return MAD_FLOW_CONTINUE;
}

/*
 * This is the error callback function. It is called whenever a decoding
 * error occurs. The error is indicated by stream->error; the list of
 * possible MAD_ERROR_* errors can be found in the mad.h (or stream.h)
 * header file.
 */

static
enum mad_flow error(void *data,
		    struct mad_stream *stream,
		    struct mad_frame *frame)
{
  // struct buffer *buffer = data;

  // printf("decoding error 0x%04x (%s) at byte offset %u\n",
  //	  stream->error, mad_stream_errorstr(stream),
  // 	  stream->this_frame - buffer->start);

  /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

  return MAD_FLOW_CONTINUE;
}

/*
 * This is the function called by main() above to perform all the decoding.
 * It instantiates a decoder object and configures it with the input,
 * output, and error callback functions above. A single call to
 * mad_decoder_run() continues until a callback function returns
 * MAD_FLOW_STOP (to stop decoding) or MAD_FLOW_BREAK (to stop decoding and
 * signal an error).
 */

static
int decode(unsigned char const *start, unsigned long length)
{
  struct buffer buffer;
  struct mad_decoder decoder;
  int result;

  /* initialize our private message structure */

  buffer.start  = start;
  buffer.length = length;

  /* configure input, output, and error functions */

  mad_decoder_init(&decoder, 
                  &buffer,
		   input, 
                   0 /* header */, 
                   0 /* filter */,
                   output,
		   error, 
                   0 /* message */);

  /* start decoding */

  result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

  /* release the decoder */

  mad_decoder_finish(&decoder);

  return result;
}
