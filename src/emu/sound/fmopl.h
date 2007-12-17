#ifndef __FMOPL_H_
#define __FMOPL_H_

/* --- select emulation chips --- */
#define BUILD_YM3812 (HAS_YM3812)
#define BUILD_YM3526 (HAS_YM3526)
#define BUILD_Y8950  (HAS_Y8950)

/* select output bits size of output : 8 or 16 */
#define OPL_SAMPLE_BITS 16

/* compiler dependence */
#ifndef __OSDCOMM_H__
#define __OSDCOMM_H__
typedef unsigned char	UINT8;   /* unsigned  8bit */
typedef unsigned short	UINT16;  /* unsigned 16bit */
typedef unsigned int	UINT32;  /* unsigned 32bit */
typedef signed char		INT8;    /* signed  8bit   */
typedef signed short	INT16;   /* signed 16bit   */
typedef signed int		INT32;   /* signed 32bit   */
#endif

typedef stream_sample_t OPLSAMPLE;
/*
#if (OPL_SAMPLE_BITS==16)
typedef INT16 OPLSAMPLE;
#endif
#if (OPL_SAMPLE_BITS==8)
typedef INT8 OPLSAMPLE;
#endif
*/

typedef void (*OPL_TIMERHANDLER)(void *param,int timer,attotime period);
typedef void (*OPL_IRQHANDLER)(void *param,int irq);
typedef void (*OPL_UPDATEHANDLER)(void *param,int min_interval_us);
typedef void (*OPL_PORTHANDLER_W)(void *param,unsigned char data);
typedef unsigned char (*OPL_PORTHANDLER_R)(void *param);


#if BUILD_YM3812

void *YM3812Init(int sndindex, UINT32 clock, UINT32 rate);
void YM3812Shutdown(void *chip);
void YM3812ResetChip(void *chip);
int  YM3812Write(void *chip, int a, int v);
unsigned char YM3812Read(void *chip, int a);
int  YM3812TimerOver(void *chip, int c);
void YM3812UpdateOne(void *chip, OPLSAMPLE *buffer, int length);

void YM3812SetTimerHandler(void *chip, OPL_TIMERHANDLER TimerHandler, void *param);
void YM3812SetIRQHandler(void *chip, OPL_IRQHANDLER IRQHandler, void *param);
void YM3812SetUpdateHandler(void *chip, OPL_UPDATEHANDLER UpdateHandler, void *param);

#endif


#if BUILD_YM3526

/*
** Initialize YM3526 emulator(s).
**
** 'num' is the number of virtual YM3526's to allocate
** 'clock' is the chip clock in Hz
** 'rate' is sampling rate
*/
void *YM3526Init(int sndindex, UINT32 clock, UINT32 rate);
/* shutdown the YM3526 emulators*/
void YM3526Shutdown(void *chip);
void YM3526ResetChip(void *chip);
int  YM3526Write(void *chip, int a, int v);
unsigned char YM3526Read(void *chip, int a);
int  YM3526TimerOver(void *chip, int c);
/*
** Generate samples for one of the YM3526's
**
** 'which' is the virtual YM3526 number
** '*buffer' is the output buffer pointer
** 'length' is the number of samples that should be generated
*/
void YM3526UpdateOne(void *chip, OPLSAMPLE *buffer, int length);

void YM3526SetTimerHandler(void *chip, OPL_TIMERHANDLER TimerHandler, void *param);
void YM3526SetIRQHandler(void *chip, OPL_IRQHANDLER IRQHandler, void *param);
void YM3526SetUpdateHandler(void *chip, OPL_UPDATEHANDLER UpdateHandler, void *param);

#endif


#if BUILD_Y8950

/* Y8950 port handlers */
void Y8950SetPortHandler(void *chip, OPL_PORTHANDLER_W PortHandler_w, OPL_PORTHANDLER_R PortHandler_r, void *param);
void Y8950SetKeyboardHandler(void *chip, OPL_PORTHANDLER_W KeyboardHandler_w, OPL_PORTHANDLER_R KeyboardHandler_r, void *param);
void Y8950SetDeltaTMemory(void *chip, void * deltat_mem_ptr, int deltat_mem_size );

void * Y8950Init (int sndindex, UINT32 clock, UINT32 rate);
void Y8950Shutdown (void *chip);
void Y8950ResetChip (void *chip);
int  Y8950Write (void *chip, int a, int v);
unsigned char Y8950Read (void *chip, int a);
int  Y8950TimerOver (void *chip, int c);
void Y8950UpdateOne (void *chip, OPLSAMPLE *buffer, int length);

void Y8950SetTimerHandler (void *chip, OPL_TIMERHANDLER TimerHandler, void *param);
void Y8950SetIRQHandler (void *chip, OPL_IRQHANDLER IRQHandler, void *param);
void Y8950SetUpdateHandler (void *chip, OPL_UPDATEHANDLER UpdateHandler, void *param);

#endif


#endif /* __FMOPL_H_ */
