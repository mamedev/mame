// license:GPL-2.0+
// copyright-holders:Jarek Burczynski,Tatsuyuki Satoh
#pragma once

#ifndef __FMOPL_H__
#define __FMOPL_H__

/* --- select emulation chips --- */
#define BUILD_YM3812 (1)
#define BUILD_YM3526 (1)
#define BUILD_Y8950  (1)

/* select output bits size of output : 8 or 16 */
#define OPL_SAMPLE_BITS 16

/* compiler dependence */
#ifndef __OSDCOMM_H__
#define __OSDCOMM_H__
typedef unsigned char   UINT8;   /* unsigned  8bit */
typedef unsigned short  UINT16;  /* unsigned 16bit */
typedef unsigned int    UINT32;  /* unsigned 32bit */
typedef signed char     INT8;    /* signed  8bit   */
typedef signed short    INT16;   /* signed 16bit   */
typedef signed int      INT32;   /* signed 32bit   */
#endif /* __OSDCOMM_H__ */

typedef stream_sample_t OPLSAMPLE;
/*
#if (OPL_SAMPLE_BITS==16)
typedef INT16 OPLSAMPLE;
#endif
#if (OPL_SAMPLE_BITS==8)
typedef INT8 OPLSAMPLE;
#endif
*/

typedef void (*OPL_TIMERHANDLER)(void *param,int timer,const attotime &period);
typedef void (*OPL_IRQHANDLER)(void *param,int irq);
typedef void (*OPL_UPDATEHANDLER)(void *param,int min_interval_us);
typedef void (*OPL_PORTHANDLER_W)(void *param,unsigned char data);
typedef unsigned char (*OPL_PORTHANDLER_R)(void *param);


#if BUILD_YM3812

void *ym3812_init(device_t *device, UINT32 clock, UINT32 rate);
void ym3812_shutdown(void *chip);
void ym3812_reset_chip(void *chip);
int  ym3812_write(void *chip, int a, int v);
unsigned char ym3812_read(void *chip, int a);
int  ym3812_timer_over(void *chip, int c);
void ym3812_update_one(void *chip, OPLSAMPLE *buffer, int length);

void ym3812_set_timer_handler(void *chip, OPL_TIMERHANDLER TimerHandler, void *param);
void ym3812_set_irq_handler(void *chip, OPL_IRQHANDLER IRQHandler, void *param);
void ym3812_set_update_handler(void *chip, OPL_UPDATEHANDLER UpdateHandler, void *param);

#endif /* BUILD_YM3812 */


#if BUILD_YM3526

/*
** Initialize YM3526 emulator(s).
**
** 'num' is the number of virtual YM3526's to allocate
** 'clock' is the chip clock in Hz
** 'rate' is sampling rate
*/
void *ym3526_init(device_t *device, UINT32 clock, UINT32 rate);
/* shutdown the YM3526 emulators*/
void ym3526_shutdown(void *chip);
void ym3526_reset_chip(void *chip);
int  ym3526_write(void *chip, int a, int v);
unsigned char ym3526_read(void *chip, int a);
int  ym3526_timer_over(void *chip, int c);
/*
** Generate samples for one of the YM3526's
**
** 'which' is the virtual YM3526 number
** '*buffer' is the output buffer pointer
** 'length' is the number of samples that should be generated
*/
void ym3526_update_one(void *chip, OPLSAMPLE *buffer, int length);

void ym3526_set_timer_handler(void *chip, OPL_TIMERHANDLER TimerHandler, void *param);
void ym3526_set_irq_handler(void *chip, OPL_IRQHANDLER IRQHandler, void *param);
void ym3526_set_update_handler(void *chip, OPL_UPDATEHANDLER UpdateHandler, void *param);

#endif /* BUILD_YM3526 */


#if BUILD_Y8950

/* Y8950 port handlers */
void y8950_set_port_handler(void *chip, OPL_PORTHANDLER_W PortHandler_w, OPL_PORTHANDLER_R PortHandler_r, void *param);
void y8950_set_keyboard_handler(void *chip, OPL_PORTHANDLER_W KeyboardHandler_w, OPL_PORTHANDLER_R KeyboardHandler_r, void *param);
void y8950_set_delta_t_memory(void *chip, void * deltat_mem_ptr, int deltat_mem_size );

void * y8950_init(device_t *device, UINT32 clock, UINT32 rate);
void y8950_shutdown(void *chip);
void y8950_reset_chip(void *chip);
int  y8950_write(void *chip, int a, int v);
unsigned char y8950_read (void *chip, int a);
int  y8950_timer_over(void *chip, int c);
void y8950_update_one(void *chip, OPLSAMPLE *buffer, int length);

void y8950_set_timer_handler(void *chip, OPL_TIMERHANDLER TimerHandler, void *param);
void y8950_set_irq_handler(void *chip, OPL_IRQHANDLER IRQHandler, void *param);
void y8950_set_update_handler(void *chip, OPL_UPDATEHANDLER UpdateHandler, void *param);

#endif /* BUILD_Y8950 */


#endif /* __FMOPL_H__ */
