#ifndef _H_YM2413_
#define _H_YM2413_

/* select output bits size of output : 8 or 16 */
#define SAMPLE_BITS 16

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

typedef stream_sample_t SAMP;
/*
#if (SAMPLE_BITS==16)
typedef INT16 SAMP;
#endif
#if (SAMPLE_BITS==8)
typedef INT8 SAMP;
#endif
*/



void *YM2413Init(int clock, int rate);
void YM2413Shutdown(void *chip);
void YM2413ResetChip(void *chip);
void YM2413Write(void *chip, int a, int v);
unsigned char YM2413Read(void *chip, int a);
void YM2413UpdateOne(void *chip, SAMP **buffers, int length);

typedef void (*OPLL_UPDATEHANDLER)(void *param,int min_interval_us);

void YM2413SetUpdateHandler(void *chip, OPLL_UPDATEHANDLER UpdateHandler, void *param);


#endif /*_H_YM2413_*/
