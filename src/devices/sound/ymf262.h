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
typedef unsigned char   uint8_t;   /* unsigned  8bit */
typedef unsigned short  uint16_t;  /* unsigned 16bit */
typedef unsigned int    uint32_t;  /* unsigned 32bit */
typedef signed char     int8_t;    /* signed  8bit   */
typedef signed short    int16_t;   /* signed 16bit   */
typedef signed int      int32_t;   /* signed 32bit   */
#endif

typedef stream_sample_t OPL3SAMPLE;
/*
#if (OPL3_SAMPLE_BITS==16)
typedef int16_t OPL3SAMPLE;
#endif
#if (OPL3_SAMPLE_BITS==8)
typedef int8_t OPL3SAMPLE;
#endif
*/

typedef void (*OPL3_TIMERHANDLER)(void *param,int timer,const attotime &period);
typedef void (*OPL3_IRQHANDLER)(void *param,int irq);
typedef void (*OPL3_UPDATEHANDLER)(void *param,int min_interval_us);


void *ymf262_init(device_t *device, int clock, int rate);
void ymf262_post_load(void *chip);
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
