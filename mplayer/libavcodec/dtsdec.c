/* 
 * dtsdec.c : free DTS Coherent Acoustics stream decoder.
 * Copyright (C) 2004 Benjamin Zores <ben@geexbox.org>
 *
 * This file is part of libavcodec.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *  
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef HAVE_AV_CONFIG_H
#undef HAVE_AV_CONFIG_H
#endif

#include "avcodec.h"
#include <dts.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#define INBUF_SIZE 4096
#define BUFFER_SIZE 24576
#define HEADER_SIZE 14

#ifdef LIBDTS_FIXED
#define CONVERT_LEVEL (1 << 26)
#define CONVERT_BIAS 0
#else
#define CONVERT_LEVEL 1
#define CONVERT_BIAS 384
#endif

static inline
int16_t convert (int32_t i)
{
#ifdef LIBDTS_FIXED
    i >>= 15;
#else
    i -= 0x43c00000;
#endif
    return (i > 32767) ? 32767 : ((i < -32768) ? -32768 : i);
}

void
convert2s16_2 (sample_t * _f, int16_t * s16)
{
  int i;
  int32_t * f = (int32_t *) _f;

  for (i = 0; i < 256; i++)
    {
      s16[2*i] = convert (f[i]);
      s16[2*i+1] = convert (f[i+256]);
    }  
}

void
convert2s16_4 (sample_t * _f, int16_t * s16)
{
  int i;
  int32_t * f = (int32_t *) _f;

  for (i = 0; i < 256; i++)
    {
      s16[4*i] = convert (f[i]);
      s16[4*i+1] = convert (f[i+256]);
      s16[4*i+2] = convert (f[i+512]);
      s16[4*i+3] = convert (f[i+768]);
    }
}

void
convert2s16_5 (sample_t * _f, int16_t * s16)
{
  int i;
  int32_t * f = (int32_t *) _f;

  for (i = 0; i < 256; i++)		
    {
      s16[5*i] = convert (f[i+256]);
      s16[5*i+1] = convert (f[i+512]);
      s16[5*i+2] = convert (f[i+768]);
      s16[5*i+3] = convert (f[i+1024]);
      s16[5*i+4] = convert (f[i]);
    }
}

static void
convert2s16_multi (sample_t * _f, int16_t * s16, int flags)
{
  int i;
  int32_t * f = (int32_t *) _f;

  switch (flags)
    {
    case DTS_MONO:			//MON 2
      for (i = 0; i < 256; i++)
        {
          s16[2*i] = s16[2*i+1] = convert (f[i]);
        }
      break;
    case DTS_CHANNEL:			//L R 2
    case DTS_STEREO:
    case DTS_STEREO_TOTAL:
      convert2s16_2 (_f, s16);
      break;
    case DTS_STEREO_SUMDIFF:		//L+R L-R 2
      for (i = 0; i < 256; i++)
        {
          s16[2*i] = convert (f[i])/2 + convert (f[i+256])/2;
          s16[2*i+1] = convert (f[i])/2 - convert (f[i+256])/2;
        }        
      break;      
    case DTS_3F:			//C L R 5
      for (i = 0; i < 256; i++)
        {
          s16[5*i] = convert (f[i+256]);
          s16[5*i+1] = convert (f[i+512]);
          s16[5*i+2] = s16[5*i+3] = 0;
          s16[5*i+4] = convert (f[i]);
        }
      break;
    case DTS_2F1R:			//L R S 4
      for (i = 0; i < 256; i++)
        {
          s16[4*i] = convert (f[i]);
          s16[4*i+1] = convert (f[i+256]);
          s16[4*i+2] = s16[4*i+3] = convert (f[i+512]);
        }
      break;
    case DTS_3F1R:			//C L R S 5
      for (i = 0; i < 256; i++)
        {
          s16[5*i] = convert (f[i]+256);
          s16[5*i+1] = convert (f[i+512]);
          s16[5*i+2] = s16[5*i+3] = convert (f[i+768]);
          s16[5*i+4] = convert (f[i]);
        }
      break;                 
    case DTS_2F2R:			//L R SL SR 4
      convert2s16_4 (_f, s16);
      break;
    case DTS_3F2R:			//C L R SL SR 5
      convert2s16_5 (_f, s16);
      break;
    case DTS_MONO | DTS_LFE:		//MON LFE 6
      for (i = 0; i < 256; i++)
        {
          s16[6*i] = s16[6*i+1] = s16[6*i+2] = s16[6*i+3] = 0;
          s16[6*i+4] = convert (f[i]);
          s16[6*i+5] = convert (f[i+256]);
        }
      break;
    case DTS_CHANNEL | DTS_LFE:
    case DTS_STEREO | DTS_LFE:
    case DTS_STEREO_TOTAL | DTS_LFE:	//L R LFE 6
      for (i = 0; i < 256; i++)
        {
          s16[6*i] = convert (f[i]);
          s16[6*i+1] = convert (f[i+256]);
          s16[6*i+2] = s16[6*i+3] = s16[6*i+4] = 0;
          s16[6*i+5] = convert (f[i+512]);
        }
      break;
    case DTS_STEREO_SUMDIFF | DTS_LFE:	//L+R L-R LFE 6
      for (i = 0; i < 256; i++)
        {
          s16[6*i] = convert (f[i])/2 + convert (f[i+256])/2;
          s16[6*i+1] = convert (f[i])/2 - convert (f[i+256])/2;
          s16[6*i+2] = s16[6*i+3] = s16[6*i+4] = 0;
          s16[6*i+5] = convert (f[i+512]);
        }
      break;          
    case DTS_3F | DTS_LFE:	//C L R LFE 6
      for (i = 0; i < 256; i++)
        {
          s16[6*i] = convert (f[i+256]);
          s16[6*i+1] = convert (f[i+512]);
          s16[6*i+2] = s16[6*i+3] = 0;
          s16[6*i+4] = convert (f[i]);
          s16[6*i+5] = convert (f[i+768]);
        }
      break;
    case DTS_2F1R | DTS_LFE:	//L R S LFE 6
      for (i = 0; i < 256; i++)
        {
          s16[6*i] = convert (f[i]);
          s16[6*i+1] = convert (f[i+256]);
          s16[6*i+2] = s16[6*i+3] = convert (f[i+512]);
          s16[6*i+4] = 0;
          s16[6*i+5] = convert (f[i+768]);
        }
      break;            
    case DTS_3F1R | DTS_LFE:	//C L R S LFE 6
      for (i = 0; i < 256; i++)
        {
          s16[6*i] = convert (f[i+256]);
          s16[6*i+1] = convert (f[i+512]);
          s16[6*i+2] = s16[6*i+3] = convert (f[i+768]);
          s16[6*i+4] = convert (f[i]);
          s16[6*i+5] = convert (f[i+1024]);
        }
      break;      
    case DTS_2F2R | DTS_LFE:	//L R SL SR LFE 6
      for (i = 0; i < 256; i++)
        {
          s16[6*i] = convert (f[i]);
          s16[6*i+1] = convert (f[i+256]);
          s16[6*i+2] = convert (f[i+512]);
          s16[6*i+3] = convert (f[i+768]);
          s16[6*i+4] = 0;
          s16[6*i+5] = convert (f[i+1024]);
        }
      break;
    case DTS_3F2R | DTS_LFE:	//C L R SL SR LFE 6
      for (i = 0; i < 256; i++)
        {
          s16[6*i] = convert (f[i+256]);
          s16[6*i+1] = convert (f[i+512]);
          s16[6*i+2] = convert (f[i+768]);
          s16[6*i+3] = convert (f[i+1024]);
          s16[6*i+4] = convert (f[i]);
          s16[6*i+5] = convert (f[i+1280]); 
          
        }
//   	printf("samples %8x %8x %8x %8x %8x %8x", f[0], f[256], f[512], f[768], f[1024], f[1280] );
      break;
    }
}

static int
channels_multi (int flags)
{
  static int dts_channels[11] = {2, 2, 2, 2, 2, 5, 4, 5, 4, 5, 6}; 
  
  if (flags & DTS_LFE)
    return 6;
  else 
    return dts_channels[flags & DTS_CHANNEL_MASK];
}

static int
dts_decode_frame (AVCodecContext *avctx, void *data, int *data_size,
                  uint8_t *buff, int buff_size)
{
  uint8_t * start = buff;
  uint8_t * end = buff + buff_size;
  *data_size = 0;

  static uint8_t buf[BUFFER_SIZE];
  static uint8_t * bufptr = buf;
  static uint8_t * bufpos = buf + HEADER_SIZE;

  static int sample_rate;
  static int frame_length;
  static int flags;
  int bit_rate;
  int len;
  dts_state_t *state = avctx->priv_data;
  int finished;
  finished = 0;

  while (1)
    {
      len = end - start;
      if (!len)
        break;
      if (len > bufpos - bufptr)
        len = bufpos - bufptr;
      memcpy (bufptr, start, len);
      bufptr += len;
      start += len;
      if (bufptr == bufpos)
        {
          if (bufpos == buf + HEADER_SIZE)
            {
              int length;

              length = dts_syncinfo (state, buf, &flags, &sample_rate,
                                     &bit_rate, &frame_length);
              if (!length)
                {
                  av_log (NULL, AV_LOG_INFO, "skip\n");
                  for (bufptr = buf; bufptr < buf + HEADER_SIZE-1; bufptr++)
                    bufptr[0] = bufptr[1];
                  continue;
                }
              bufpos = buf + length;
              finished += length;
            }
          else
            {
              level_t level;
              sample_t bias;
              int i;
              
//              flags = 2; /* ????????????  */ force 2 CH dts code removed, libdts now has correct bias in new libdts.a 
              level = CONVERT_LEVEL;
              bias = CONVERT_BIAS;

              flags |= DTS_ADJUST_LEVEL;
              if (dts_frame (state, buf, &flags, &level, bias))
                goto error;
              avctx->sample_rate = sample_rate;
              avctx->channels = channels_multi (flags);
              avctx->bit_rate = bit_rate;
              for (i = 0; i < dts_blocks_num (state); i++)
                {
                  if (dts_block (state))
                    goto error;
                  {
                    int chans;
                    chans = channels_multi (flags);
                    convert2s16_multi (dts_samples (state), data,
                                       flags & (DTS_CHANNEL_MASK | DTS_LFE));

                    data += 256 * sizeof (int16_t) * chans;
                    *data_size += 256 * sizeof (int16_t) * chans;
                  }
                }
              bufptr = buf;
              bufpos = buf + HEADER_SIZE;
              if (finished < 8192 )
              	continue;
              else
                return finished;
            error:
              av_log (NULL, AV_LOG_ERROR, "error\n");
              bufptr = buf;
              bufpos = buf + HEADER_SIZE;
            }
        }
    }

  return buff_size;
}

static int
dts_decode_init (AVCodecContext *avctx)
{
  avctx->priv_data = dts_init (0);
  if (avctx->priv_data == NULL)
    return 1;

  return 0;
}

static int
dts_decode_end (AVCodecContext *s)
{
  //hack because libdts does not clean itself
  if(s->priv_data) {
  	if(dts_samples(s->priv_data)) free(dts_samples(s->priv_data));
  	free(s->priv_data);
  	s->priv_data = NULL;
	}
  
  return 0;
}

AVCodec dts_decoder = {
  "dts", 
  CODEC_TYPE_AUDIO,
  CODEC_ID_DTS,
  0,//sizeof (dts_state_t *), //libdts alloc its own priv data
  dts_decode_init,
  NULL,
  dts_decode_end,
  dts_decode_frame,
};
