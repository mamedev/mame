// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
#pragma once

#ifndef __YMF262_H__
#define __YMF262_H__

/* select number of output bits: 8 or 16 */
#define OPL3_SAMPLE_BITS 16

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

typedef stream_sample_t OPL3SAMPLE;
/*
#if (OPL3_SAMPLE_BITS==16)
typedef INT16 OPL3SAMPLE;
#endif
#if (OPL3_SAMPLE_BITS==8)
typedef INT8 OPL3SAMPLE;
#endif
*/

typedef void (*OPL3_TIMERHANDLER)(void *param,int timer,const attotime &period);
typedef void (*OPL3_IRQHANDLER)(void *param,int irq);
typedef void (*OPL3_UPDATEHANDLER)(void *param,int min_interval_us);


void *ymf262_init(device_t *device, int clock, int rate);
void ymf262_shutdown(void *chip);
void ymf262_reset_chip(void *chip);
int  ymf262_write(void *chip, int a, int v);
unsigned char ymf262_read(void *chip, int a);
int  ymf262_timer_over(void *chip, int c);
void ymf262_update_one(void *chip, OPL3SAMPLE **buffers, int length);

void ymf262_set_timer_handler(void *chip, OPL3_TIMERHANDLER TimerHandler, void *param);
void ymf262_set_irq_handler(void *chip, OPL3_IRQHANDLER IRQHandler, void *param);
void ymf262_set_update_handler(void *chip, OPL3_UPDATEHANDLER UpdateHandler, void *param);


#endif /* __YMF262_H__ */
