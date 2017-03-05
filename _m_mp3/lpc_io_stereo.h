/*--------------------------------------------------------------------------*/
/*
 * MP3 player DEMO - MPEG queue management functions interface file
 * Copyright (C) 2006 MXP semicontuctor
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
 * $Id: lpc_io.c 1.0 2006/10/16 09:41:32 rob Exp $
 */
/*--------------------------------------------------------------------------*/
#ifndef  __IO_H
#define  __IO_H


#define  bool unsigned char

#define  STREAM_DECODED_SIZE   10000
#define  ENABLED                  1
#define  DISABLED                 0

typedef struct {
  unsigned long raw[STREAM_DECODED_SIZE];	/* 16 bit PCM output samples [ch][sample] */
  volatile  unsigned short wr_idx;
  volatile  unsigned short rd_idx;
}decoded_stream_t;



extern void init_IO(void);
extern void set_dac_sample_rate(unsigned int sample_rate);
extern void wait_end_of_excerpt(void);
extern void render_sample_block(unsigned short *blk_ch1, unsigned short *blk_ch2, unsigned short int len, unsigned char nch);
extern void disable_audio_render(void);
extern void enable_audio_render(void);
extern void init_timer(void);



#endif
