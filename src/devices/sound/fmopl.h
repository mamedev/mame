// license:GPL-2.0+
// copyright-holders:Jarek Burczynski,Tatsuyuki Satoh
#ifndef MAME_SOUND_FMOPL_H
#define MAME_SOUND_FMOPL_H

#pragma once

#include <stdint.h>


/* --- select emulation chips --- */
#define BUILD_YM3812 (1)
#define BUILD_YM3526 (1)
#define BUILD_Y8950  (1)

/* select output bits size of output : 8 or 16 */
#define OPL_SAMPLE_BITS 16

typedef stream_sample_t OPLSAMPLE;
/*
#if (OPL_SAMPLE_BITS==16)
typedef int16_t OPLSAMPLE;
#endif
#if (OPL_SAMPLE_BITS==8)
typedef int8_t OPLSAMPLE;
#endif
*/

typedef void (*OPL_TIMERHANDLER)(device_t *device,int timer,const attotime &period);
typedef void (*OPL_IRQHANDLER)(device_t *device,int irq);
typedef void (*OPL_UPDATEHANDLER)(device_t *device,int min_interval_us);
typedef void (*OPL_PORTHANDLER_W)(device_t *device,unsigned char data);
typedef unsigned char (*OPL_PORTHANDLER_R)(device_t *device);


#if BUILD_YM3812

void *ym3812_init(device_t *device, uint32_t clock, uint32_t rate);
void ym3812_clock_changed(void *chip, uint32_t clock, uint32_t rate);
void ym3812_shutdown(void *chip);
void ym3812_reset_chip(void *chip);
int  ym3812_write(void *chip, int a, int v);
unsigned char ym3812_read(void *chip, int a);
int  ym3812_timer_over(void *chip, int c);
void ym3812_update_one(void *chip, OPLSAMPLE *buffer, int length);

void ym3812_set_timer_handler(void *chip, OPL_TIMERHANDLER TimerHandler, device_t *device);
void ym3812_set_irq_handler(void *chip, OPL_IRQHANDLER IRQHandler, device_t *device);
void ym3812_set_update_handler(void *chip, OPL_UPDATEHANDLER UpdateHandler, device_t *device);

#endif /* BUILD_YM3812 */


#if BUILD_YM3526

/*
** Initialize YM3526 emulator(s).
**
** 'num' is the number of virtual YM3526's to allocate
** 'clock' is the chip clock in Hz
** 'rate' is sampling rate
*/
void *ym3526_init(device_t *device, uint32_t clock, uint32_t rate);
void ym3526_clock_changed(void *chip, uint32_t clock, uint32_t rate);
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

void ym3526_set_timer_handler(void *chip, OPL_TIMERHANDLER TimerHandler, device_t *device);
void ym3526_set_irq_handler(void *chip, OPL_IRQHANDLER IRQHandler, device_t *device);
void ym3526_set_update_handler(void *chip, OPL_UPDATEHANDLER UpdateHandler, device_t *device);

#endif /* BUILD_YM3526 */


#if BUILD_Y8950

/* Y8950 port handlers */
void y8950_set_port_handler(void *chip, OPL_PORTHANDLER_W PortHandler_w, OPL_PORTHANDLER_R PortHandler_r, device_t *device);
void y8950_set_keyboard_handler(void *chip, OPL_PORTHANDLER_W KeyboardHandler_w, OPL_PORTHANDLER_R KeyboardHandler_r, device_t *device);
void y8950_set_delta_t_memory(void *chip, void * deltat_mem_ptr, int deltat_mem_size );

void * y8950_init(device_t *device, uint32_t clock, uint32_t rate);
void y8950_shutdown(void *chip);
void y8950_reset_chip(void *chip);
int  y8950_write(void *chip, int a, int v);
unsigned char y8950_read (void *chip, int a);
int  y8950_timer_over(void *chip, int c);
void y8950_update_one(void *chip, OPLSAMPLE *buffer, int length);

void y8950_set_timer_handler(void *chip, OPL_TIMERHANDLER TimerHandler, device_t *device);
void y8950_set_irq_handler(void *chip, OPL_IRQHANDLER IRQHandler, device_t *device);
void y8950_set_update_handler(void *chip, OPL_UPDATEHANDLER UpdateHandler, device_t *device);

#endif /* BUILD_Y8950 */


#endif // MAME_SOUND_FMOPL_H
