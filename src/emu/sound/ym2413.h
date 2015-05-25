// license:???
// copyright-holders:Jarek Burczynski
#pragma once

#ifndef __YM2413_H__
#define __YM2413_H__

/* select output bits size of output : 8 or 16 */
#define SAMPLE_BITS 16

/* compiler dependence */
#ifndef __OSDCOMM_H__
#define __OSDCOMM_H__
typedef unsigned char   UINT8;   /* unsigned  8bit */
typedef unsigned short  UINT16;  /* unsigned 16bit */
typedef unsigned int    UINT32;  /* unsigned 32bit */
typedef signed char     INT8;    /* signed  8bit   */
typedef signed short    INT16;   /* signed 16bit   */
typedef signed int      INT32;   /* signed 32bit   */
#endif

typedef stream_sample_t SAMP;
/*
#if (SAMPLE_BITS==16)
typedef INT16 SAMP;
#endif
#if (SAMPLE_BITS==8)
typedef INT8 SAMP;
#endif
*/



void *ym2413_init(device_t *device, int clock, int rate);
void ym2413_shutdown(void *chip);
void ym2413_reset_chip(void *chip);
void ym2413_write(void *chip, int a, int v);
unsigned char ym2413_read(void *chip, int a);
void ym2413_update_one(void *chip, SAMP **buffers, int length);

typedef void (*OPLL_UPDATEHANDLER)(void *param,int min_interval_us);

void ym2413_set_update_handler(void *chip, OPLL_UPDATEHANDLER UpdateHandler, void *param);

#endif /*__YM2413_H__*/
