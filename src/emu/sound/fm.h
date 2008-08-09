/*
  File: fm.h -- header file for software emulation for FM sound generator

*/
#ifndef _H_FM_FM_
#define _H_FM_FM_

/* --- select emulation chips --- */
#define BUILD_YM2203  (HAS_YM2203)		/* build YM2203(OPN)   emulator */
#define BUILD_YM2608  (HAS_YM2608)		/* build YM2608(OPNA)  emulator */
#define BUILD_YM2610  (HAS_YM2610)		/* build YM2610(OPNB)  emulator */
#define BUILD_YM2610B (HAS_YM2610B)		/* build YM2610B(OPNB?)emulator */
#define BUILD_YM2612  (HAS_YM2612)		/* build YM2612(OPN2)  emulator */
#define BUILD_YM3438  (HAS_YM3438)		/* build YM3438(OPN) emulator */

/* select bit size of output : 8 or 16 */
#define FM_SAMPLE_BITS 16

/* select timer system internal or external */
#define FM_INTERNAL_TIMER 0

/* --- speedup optimize --- */
/* busy flag enulation , The definition of FM_GET_TIME_NOW() is necessary. */
#define FM_BUSY_FLAG_SUPPORT 1

/* --- external SSG(YM2149/AY-3-8910)emulator interface port */
/* used by YM2203,YM2608,and YM2610 */
typedef struct _ssg_callbacks ssg_callbacks;
struct _ssg_callbacks
{
	void (*set_clock)(void *param, int clock);
	void (*write)(void *param, int address, int data);
	int (*read)(void *param);
	void (*reset)(void *param);
};

/* --- external callback funstions for realtime update --- */

#if FM_BUSY_FLAG_SUPPORT
#define TIME_TYPE 					attotime
#define UNDEFINED_TIME				attotime_zero
#define FM_GET_TIME_NOW() 			timer_get_time()
#define ADD_TIMES(t1, t2)    		attotime_add((t1), (t2))
#define COMPARE_TIMES(t1, t2)		attotime_compare((t1), (t2))
#define MULTIPLY_TIME_BY_INT(t,i)	attotime_mul(t, i)
#endif

#if BUILD_YM2203
  /* in 2203intf.c */
  void YM2203UpdateRequest(void *param);
  #define YM2203UpdateReq(chip) YM2203UpdateRequest(chip)
#endif
#if BUILD_YM2608
  /* in 2608intf.c */
  void YM2608UpdateRequest(void *param);
  #define YM2608UpdateReq(chip) YM2608UpdateRequest(chip);
#endif
#if (BUILD_YM2610||BUILD_YM2610B)
  /* in 2610intf.c */
  void YM2610UpdateRequest(void *param);
  #define YM2610UpdateReq(chip) YM2610UpdateRequest(chip);
#endif
#if (BUILD_YM2612||BUILD_YM3438)
  /* in 2612intf.c */
  void YM2612UpdateRequest(void *param);
  #define YM2612UpdateReq(chip) YM2612UpdateRequest(chip);
#endif

/* compiler dependence */
#if 0
#ifndef OSD_CPU_H
#define OSD_CPU_H
typedef unsigned char	UINT8;   /* unsigned  8bit */
typedef unsigned short	UINT16;  /* unsigned 16bit */
typedef unsigned int	UINT32;  /* unsigned 32bit */
typedef signed char		INT8;    /* signed  8bit   */
typedef signed short	INT16;   /* signed 16bit   */
typedef signed int		INT32;   /* signed 32bit   */
#endif
#endif

#ifndef INLINE
#define INLINE static __inline__
#endif




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
void * YM2203Init(void *param, int index, int baseclock, int rate,
               FM_TIMERHANDLER TimerHandler,FM_IRQHANDLER IRQHandler, const ssg_callbacks *ssg);

/*
** shutdown the YM2203 emulators
*/
void YM2203Shutdown(void *chip);

/*
** reset all chip registers for YM2203 number 'num'
*/
void YM2203ResetChip(void *chip);

/*
** update one of chip
*/
void YM2203UpdateOne(void *chip, FMSAMPLE *buffer, int length);

/*
** Write
** return : InterruptLevel
*/
int YM2203Write(void *chip,int a,unsigned char v);

/*
** Read
** return : InterruptLevel
*/
unsigned char YM2203Read(void *chip,int a);

/*
**  Timer OverFlow
*/
int YM2203TimerOver(void *chip, int c);

/*
**  State Save
*/
void YM2203Postload(void *chip);
#endif /* BUILD_YM2203 */

#if BUILD_YM2608
/* -------------------- YM2608(OPNA) Interface -------------------- */
void * YM2608Init(void *param, int index, int baseclock, int rate,
               void *pcmroma,int pcmsizea,
               FM_TIMERHANDLER TimerHandler,FM_IRQHANDLER IRQHandler, const ssg_callbacks *ssg);
void YM2608Shutdown(void *chip);
void YM2608ResetChip(void *chip);
void YM2608UpdateOne(void *chip, FMSAMPLE **buffer, int length);

int YM2608Write(void *chip, int a,unsigned char v);
unsigned char YM2608Read(void *chip,int a);
int YM2608TimerOver(void *chip, int c );
void YM2608Postload(void *chip);
#endif /* BUILD_YM2608 */

#if (BUILD_YM2610||BUILD_YM2610B)
/* -------------------- YM2610(OPNB) Interface -------------------- */
void * YM2610Init(void *param, int index, int baseclock, int rate,
               void *pcmroma,int pcmasize,void *pcmromb,int pcmbsize,
               FM_TIMERHANDLER TimerHandler,FM_IRQHANDLER IRQHandler, const ssg_callbacks *ssg);
void YM2610Shutdown(void *chip);
void YM2610ResetChip(void *chip);
void YM2610UpdateOne(void *chip, FMSAMPLE **buffer, int length);
#if BUILD_YM2610B
void YM2610BUpdateOne(void *chip, FMSAMPLE **buffer, int length);
#endif

int YM2610Write(void *chip, int a,unsigned char v);
unsigned char YM2610Read(void *chip,int a);
int YM2610TimerOver(void *chip, int c );
void YM2610Postload(void *chip);
#endif /* (BUILD_YM2610||BUILD_YM2610B) */

#if (BUILD_YM2612||BUILD_YM3438)
void * YM2612Init(void *param, int index, int baseclock, int rate,
               FM_TIMERHANDLER TimerHandler,FM_IRQHANDLER IRQHandler);
void YM2612Shutdown(void *chip);
void YM2612ResetChip(void *chip);
void YM2612UpdateOne(void *chip, FMSAMPLE **buffer, int length);

int YM2612Write(void *chip, int a,unsigned char v);
unsigned char YM2612Read(void *chip,int a);
int YM2612TimerOver(void *chip, int c );
void YM2612Postload(void *chip);
#endif /* (BUILD_YM2612||BUILD_YM3438) */


#endif /* _H_FM_FM_ */
