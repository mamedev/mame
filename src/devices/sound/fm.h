// license:GPL-2.0+
// copyright-holders:Jarek Burczynski,Tatsuyuki Satoh
/*
  File: fm.h -- header file for software emulation for FM sound generator

*/

#pragma once

#ifndef __FM_H__
#define __FM_H__

/* --- select emulation chips --- */
#define BUILD_YM2203  (1)       /* build YM2203(OPN)   emulator */
#define BUILD_YM2608  (1)       /* build YM2608(OPNA)  emulator */
#define BUILD_YM2610  (1)       /* build YM2610(OPNB)  emulator */
#define BUILD_YM2610B (1)       /* build YM2610B(OPNB?)emulator */
#define BUILD_YM2612  (1)       /* build YM2612(OPN2)  emulator */
#define BUILD_YM3438  (1)       /* build YM3438(OPN) emulator */

/* select bit size of output : 8 or 16 */
#define FM_SAMPLE_BITS 16

/* select timer system internal or external */
#define FM_INTERNAL_TIMER 0

/* --- speedup optimize --- */
/* busy flag enulation , The definition of FM_GET_TIME_NOW() is necessary. */
#define FM_BUSY_FLAG_SUPPORT 1

/* --- external SSG(YM2149/AY-3-8910)emulator interface port */
/* used by YM2203,YM2608,and YM2610 */
struct ssg_callbacks
{
	void (*set_clock)(void *param, int clock);
	void (*write)(void *param, int address, int data);
	int (*read)(void *param);
	void (*reset)(void *param);
};

/* --- external callback funstions for realtime update --- */

#if FM_BUSY_FLAG_SUPPORT
#define TIME_TYPE                   attotime
#define UNDEFINED_TIME              attotime::zero
#define FM_GET_TIME_NOW(machine)    (machine)->time()
#define ADD_TIMES(t1, t2)           ((t1) + (t2))
#define COMPARE_TIMES(t1, t2)       (((t1) == (t2)) ? 0 : ((t1) < (t2)) ? -1 : 1)
#define MULTIPLY_TIME_BY_INT(t,i)   ((t) * (i))
#endif

#if BUILD_YM2203
	/* in 2203intf.c */
	void ym2203_update_request(void *param);
	#define ym2203_update_req(chip) ym2203_update_request(chip)
#endif /* BUILD_YM2203 */

#if BUILD_YM2608
	/* in 2608intf.c */
	void ym2608_update_request(void *param);
	#define ym2608_update_req(chip) ym2608_update_request(chip);
#endif /* BUILD_YM2608 */

#if (BUILD_YM2610||BUILD_YM2610B)
	/* in 2610intf.c */
	void ym2610_update_request(void *param);
	#define ym2610_update_req(chip) ym2610_update_request(chip);
#endif /* (BUILD_YM2610||BUILD_YM2610B) */

#if (BUILD_YM2612||BUILD_YM3438)
	/* in 2612intf.c */
	void ym2612_update_request(void *param);
	#define ym2612_update_req(chip) ym2612_update_request(chip);
#endif /* (BUILD_YM2612||BUILD_YM3438) */


typedef stream_sample_t FMSAMPLE;
/*
#if (FM_SAMPLE_BITS==16)
typedef INT16 FMSAMPLE;
#endif
#if (FM_SAMPLE_BITS==8)
typedef unsigned char  FMSAMPLE;
#endif
*/

typedef void (*FM_TIMERHANDLER)(void *param,int c,int cnt,int clock);
typedef void (*FM_IRQHANDLER)(void *param,int irq);
/* FM_TIMERHANDLER : Stop or Start timer         */
/* int n          = chip number                  */
/* int c          = Channel 0=TimerA,1=TimerB    */
/* int count      = timer count (0=stop)         */
/* doube stepTime = step time of one count (sec.)*/

/* FM_IRQHHANDLER : IRQ level changing sense     */
/* int n       = chip number                     */
/* int irq     = IRQ level 0=OFF,1=ON            */

#if BUILD_YM2203
/* -------------------- YM2203(OPN) Interface -------------------- */

/*
** Initialize YM2203 emulator(s).
**
** 'num'           is the number of virtual YM2203's to allocate
** 'baseclock'
** 'rate'          is sampling rate
** 'TimerHandler'  timer callback handler when timer start and clear
** 'IRQHandler'    IRQ callback handler when changed IRQ level
** return      0 = success
*/
void * ym2203_init(void *param, device_t *device, int baseclock, int rate,
				FM_TIMERHANDLER TimerHandler,FM_IRQHANDLER IRQHandler, const ssg_callbacks *ssg);

/*
** shutdown the YM2203 emulators
*/
void ym2203_shutdown(void *chip);

/*
** reset all chip registers for YM2203 number 'num'
*/
void ym2203_reset_chip(void *chip);

/*
** update one of chip
*/
void ym2203_update_one(void *chip, FMSAMPLE *buffer, int length);

/*
** Write
** return : InterruptLevel
*/
int ym2203_write(void *chip,int a,unsigned char v);

/*
** Read
** return : InterruptLevel
*/
unsigned char ym2203_read(void *chip,int a);

/*
**  Timer OverFlow
*/
int ym2203_timer_over(void *chip, int c);

/*
**  State Save
*/
void ym2203_postload(void *chip);
#endif /* BUILD_YM2203 */

#if BUILD_YM2608
/* -------------------- YM2608(OPNA) Interface -------------------- */
void * ym2608_init(void *param, device_t *device, int baseclock, int rate,
				void *pcmroma,int pcmsizea,
				FM_TIMERHANDLER TimerHandler,FM_IRQHANDLER IRQHandler, const ssg_callbacks *ssg);
void ym2608_shutdown(void *chip);
void ym2608_reset_chip(void *chip);
void ym2608_update_one(void *chip, FMSAMPLE **buffer, int length);

int ym2608_write(void *chip, int a,unsigned char v);
unsigned char ym2608_read(void *chip,int a);
int ym2608_timer_over(void *chip, int c );
void ym2608_postload(void *chip);
#endif /* BUILD_YM2608 */

#if (BUILD_YM2610||BUILD_YM2610B)
/* -------------------- YM2610(OPNB) Interface -------------------- */
void * ym2610_init(void *param, device_t *device, int baseclock, int rate,
				void *pcmroma,int pcmasize,void *pcmromb,int pcmbsize,
				FM_TIMERHANDLER TimerHandler,FM_IRQHANDLER IRQHandler, const ssg_callbacks *ssg);
void ym2610_shutdown(void *chip);
void ym2610_reset_chip(void *chip);
void ym2610_update_one(void *chip, FMSAMPLE **buffer, int length);

#if BUILD_YM2610B
void ym2610b_update_one(void *chip, FMSAMPLE **buffer, int length);
#endif /* BUILD_YM2610B */

int ym2610_write(void *chip, int a,unsigned char v);
unsigned char ym2610_read(void *chip,int a);
int ym2610_timer_over(void *chip, int c );
void ym2610_postload(void *chip);
#endif /* (BUILD_YM2610||BUILD_YM2610B) */

#if (BUILD_YM2612||BUILD_YM3438)
void * ym2612_init(void *param, device_t *device, int baseclock, int rate,
				FM_TIMERHANDLER TimerHandler,FM_IRQHANDLER IRQHandler);
void ym2612_shutdown(void *chip);
void ym2612_reset_chip(void *chip);
void ym2612_update_one(void *chip, FMSAMPLE **buffer, int length);

int ym2612_write(void *chip, int a,unsigned char v);
unsigned char ym2612_read(void *chip,int a);
int ym2612_timer_over(void *chip, int c );
void ym2612_postload(void *chip);
#endif /* (BUILD_YM2612||BUILD_YM3438) */


#endif /* __FM_H__ */
