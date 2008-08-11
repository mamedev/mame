/***************************************************************************

  2612intf.c

  The YM2612 emulator supports up to 2 chips.
  Each chip has the following connections:
  - Status Read / Control Write A
  - Port Read / Data Write A
  - Control Write B
  - Data Write B

***************************************************************************/

#include "sndintrf.h"
#include "deprecat.h"
#include "streams.h"
#include "sound/fm.h"
#include "sound/2612intf.h"


struct ym2612_info
{
	sound_stream *	stream;
	emu_timer *	timer[2];
	void *			chip;
	const ym2612_interface *intf;
};

/*------------------------- TM2612 -------------------------------*/
/* IRQ Handler */
static void IRQHandler(void *param,int irq)
{
	struct ym2612_info *info = param;
	if(info->intf->handler) info->intf->handler(Machine, irq);
}

/* Timer overflow callback from timer.c */
static TIMER_CALLBACK( timer_callback_2612_0 )
{
	struct ym2612_info *info = ptr;
	ym2612_timer_over(info->chip,0);
}

static TIMER_CALLBACK( timer_callback_2612_1 )
{
	struct ym2612_info *info = ptr;
	ym2612_timer_over(info->chip,1);
}

static void timer_handler(void *param,int c,int count,int clock)
{
	struct ym2612_info *info = param;
	if( count == 0 )
	{	/* Reset FM Timer */
		timer_enable(info->timer[c], 0);
	}
	else
	{	/* Start FM Timer */
		attotime period = attotime_mul(ATTOTIME_IN_HZ(clock), count);
		if (!timer_enable(info->timer[c], 1))
			timer_adjust_oneshot(info->timer[c], period, 0);
	}
}

/* update request from fm.c */
void ym2612_update_request(void *param)
{
	struct ym2612_info *info = param;
	stream_update(info->stream);
}

/***********************************************************/
/*    YM2612                                               */
/***********************************************************/

static void ym2612_stream_update(void *param, stream_sample_t **inputs, stream_sample_t **buffers, int length)
{
	struct ym2612_info *info = param;
	ym2612_update_one(info->chip, buffers, length);
}


static STATE_POSTLOAD( ym2612_intf_postload )
{
	struct ym2612_info *info = param;
	ym2612_postload(info->chip);
}


static void *ym2612_start(const char *tag, int sndindex, int clock, const void *config)
{
	static const ym2612_interface dummy = { 0 };
	struct ym2612_info *info;
	int rate = clock/72;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	info->intf = config ? config : &dummy;

	/* FM init */
	/* Timer Handler set */
	info->timer[0] = timer_alloc(timer_callback_2612_0, info);
	info->timer[1] = timer_alloc(timer_callback_2612_1, info);

	/* stream system initialize */
	info->stream = stream_create(0,2,rate,info,ym2612_stream_update);

	/**** initialize YM2612 ****/
	info->chip = ym2612_init(info,sndindex,clock,rate,timer_handler,IRQHandler);

	state_save_register_postload(Machine, ym2612_intf_postload, info);

	if (info->chip)
		return info;
	/* error */
	return NULL;
}


static void ym2612_stop(void *token)
{
	struct ym2612_info *info = token;
	ym2612_shutdown(info->chip);
}

static void ym2612_reset(void *token)
{
	struct ym2612_info *info = token;
	ym2612_reset_chip(info->chip);
}


/************************************************/
/* Status Read for YM2612 - Chip 0              */
/************************************************/
READ8_HANDLER( ym2612_status_port_0_a_r )
{
  struct ym2612_info *info = sndti_token(SOUND_YM2612,0);
  return ym2612_read(info->chip,0);
}

READ8_HANDLER( ym2612_status_port_0_b_r )
{
  struct ym2612_info *info = sndti_token(SOUND_YM2612,0);
  return ym2612_read(info->chip,2);
}

/************************************************/
/* Status Read for YM2612 - Chip 1              */
/************************************************/
READ8_HANDLER( ym2612_status_port_1_a_r ) {
  struct ym2612_info *info = sndti_token(SOUND_YM2612,1);
  return ym2612_read(info->chip,0);
}

READ8_HANDLER( ym2612_status_port_1_b_r ) {
  struct ym2612_info *info = sndti_token(SOUND_YM2612,1);
  return ym2612_read(info->chip,2);
}

/************************************************/
/* Port Read for YM2612 - Chip 0                */
/************************************************/
READ8_HANDLER( ym2612_read_port_0_r ){
  struct ym2612_info *info = sndti_token(SOUND_YM2612,0);
  return ym2612_read(info->chip,1);
}

/************************************************/
/* Port Read for YM2612 - Chip 1                */
/************************************************/
READ8_HANDLER( ym2612_read_port_1_r ){
  struct ym2612_info *info = sndti_token(SOUND_YM2612,1);
  return ym2612_read(info->chip,1);
}

/************************************************/
/* Control Write for YM2612 - Chip 0            */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( ym2612_control_port_0_a_w )
{
  struct ym2612_info *info = sndti_token(SOUND_YM2612,0);
  ym2612_write(info->chip,0,data);
}

WRITE8_HANDLER( ym2612_control_port_0_b_w )
{
  struct ym2612_info *info = sndti_token(SOUND_YM2612,0);
  ym2612_write(info->chip,2,data);
}

/************************************************/
/* Control Write for YM2612 - Chip 1            */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( ym2612_control_port_1_a_w ){
  struct ym2612_info *info = sndti_token(SOUND_YM2612,1);
  ym2612_write(info->chip,0,data);
}

WRITE8_HANDLER( ym2612_control_port_1_b_w ){
  struct ym2612_info *info = sndti_token(SOUND_YM2612,1);
  ym2612_write(info->chip,2,data);
}

/************************************************/
/* Data Write for YM2612 - Chip 0               */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( ym2612_data_port_0_a_w )
{
  struct ym2612_info *info = sndti_token(SOUND_YM2612,0);
  ym2612_write(info->chip,1,data);
}

WRITE8_HANDLER( ym2612_data_port_0_b_w )
{
  struct ym2612_info *info = sndti_token(SOUND_YM2612,0);
  ym2612_write(info->chip,3,data);
}

/************************************************/
/* Data Write for YM2612 - Chip 1               */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( ym2612_data_port_1_a_w ){
  struct ym2612_info *info = sndti_token(SOUND_YM2612,1);
  ym2612_write(info->chip,1,data);
}
WRITE8_HANDLER( ym2612_data_port_1_b_w ){
  struct ym2612_info *info = sndti_token(SOUND_YM2612,1);
  ym2612_write(info->chip,3,data);
}

#if BUILD_YM3438

/************************************************/
/* Status Read for YM3438 - Chip 0              */
/************************************************/
READ8_HANDLER( ym3438_status_port_0_a_r )
{
  struct ym2612_info *info = sndti_token(SOUND_YM3438,0);
  return ym2612_read(info->chip,0);
}

READ8_HANDLER( ym3438_status_port_0_b_r )
{
  struct ym2612_info *info = sndti_token(SOUND_YM3438,0);
  return ym2612_read(info->chip,2);
}

/************************************************/
/* Status Read for YM3438 - Chip 1              */
/************************************************/
READ8_HANDLER( ym3438_status_port_1_a_r ) {
  struct ym2612_info *info = sndti_token(SOUND_YM3438,1);
  return ym2612_read(info->chip,0);
}

READ8_HANDLER( ym3438_status_port_1_b_r ) {
  struct ym2612_info *info = sndti_token(SOUND_YM3438,1);
  return ym2612_read(info->chip,2);
}

/************************************************/
/* Port Read for YM3438 - Chip 0                */
/************************************************/
READ8_HANDLER( ym3438_read_port_0_r ){
  struct ym2612_info *info = sndti_token(SOUND_YM3438,0);
  return ym2612_read(info->chip,1);
}

/************************************************/
/* Port Read for YM3438 - Chip 1                */
/************************************************/
READ8_HANDLER( ym3438_read_port_1_r ){
  struct ym2612_info *info = sndti_token(SOUND_YM3438,1);
  return ym2612_read(info->chip,1);
}

/************************************************/
/* Control Write for YM3438 - Chip 0            */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( ym3438_control_port_0_a_w )
{
  struct ym2612_info *info = sndti_token(SOUND_YM3438,0);
  ym2612_write(info->chip,0,data);
}

WRITE8_HANDLER( ym3438_control_port_0_b_w )
{
  struct ym2612_info *info = sndti_token(SOUND_YM3438,0);
  ym2612_write(info->chip,2,data);
}

/************************************************/
/* Control Write for YM3438 - Chip 1            */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( ym3438_control_port_1_a_w ){
  struct ym2612_info *info = sndti_token(SOUND_YM3438,1);
  ym2612_write(info->chip,0,data);
}

WRITE8_HANDLER( ym3438_control_port_1_b_w ){
  struct ym2612_info *info = sndti_token(SOUND_YM3438,1);
  ym2612_write(info->chip,2,data);
}

/************************************************/
/* Data Write for YM3438 - Chip 0               */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( ym3438_data_port_0_a_w )
{
  struct ym2612_info *info = sndti_token(SOUND_YM3438,0);
  ym2612_write(info->chip,1,data);
}

WRITE8_HANDLER( ym3438_data_port_0_b_w )
{
  struct ym2612_info *info = sndti_token(SOUND_YM3438,0);
  ym2612_write(info->chip,3,data);
}

/************************************************/
/* Data Write for YM3438 - Chip 1               */
/* Consists of 2 addresses                      */
/************************************************/
WRITE8_HANDLER( ym3438_data_port_1_a_w ){
  struct ym2612_info *info = sndti_token(SOUND_YM3438,1);
  ym2612_write(info->chip,1,data);
}
WRITE8_HANDLER( ym3438_data_port_1_b_w ){
  struct ym2612_info *info = sndti_token(SOUND_YM3438,1);
  ym2612_write(info->chip,3,data);
}

#endif

/**************** end of file ****************/

/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void ym2612_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void ym2612_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = ym2612_set_info;		break;
		case SNDINFO_PTR_START:							info->start = ym2612_start;				break;
		case SNDINFO_PTR_STOP:							info->stop = ym2612_stop;				break;
		case SNDINFO_PTR_RESET:							info->reset = ym2612_reset;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "YM2612";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Yamaha FM";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

#if BUILD_YM3438

static void ym3438_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void ym3438_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = ym3438_set_info;		break;
		case SNDINFO_PTR_START:							info->start = ym2612_start;				break;
		case SNDINFO_PTR_STOP:							info->stop = ym2612_stop;				break;
		case SNDINFO_PTR_RESET:							info->reset = ym2612_reset;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "YM3438";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Yamaha FM";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}

#endif
