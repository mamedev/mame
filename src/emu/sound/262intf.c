/***************************************************************************

  262intf.c

  MAME interface for YMF262 (OPL3) emulator

***************************************************************************/
#include "sndintrf.h"
#include "deprecat.h"
#include "streams.h"
#include "262intf.h"
#include "ymf262.h"


struct ymf262_info
{
	sound_stream *	stream;
	emu_timer *	timer[2];
	void *			chip;
	const ymf262_interface *intf;
};




static void IRQHandler_262(void *param,int irq)
{
	struct ymf262_info *info = param;
	if (info->intf->handler) (info->intf->handler)(Machine, irq);
}

static TIMER_CALLBACK( timer_callback_262_0 )
{
	struct ymf262_info *info = ptr;
	YMF262TimerOver(info->chip, 0);
}

static TIMER_CALLBACK( timer_callback_262_1 )
{
	struct ymf262_info *info = ptr;
	YMF262TimerOver(info->chip, 1);
}

static void timer_handler_262(void *param,int timer, attotime period)
{
	struct ymf262_info *info = param;
	if( attotime_compare(period, attotime_zero) == 0 )
	{	/* Reset FM Timer */
		timer_enable(info->timer[timer], 0);
	}
	else
	{	/* Start FM Timer */
		timer_adjust_oneshot(info->timer[timer], period, 0);
	}
}

static void ymf262_stream_update(void *param, stream_sample_t **inputs, stream_sample_t **buffers, int length)
{
	struct ymf262_info *info = param;
	YMF262UpdateOne(info->chip, buffers, length);
}

static void _stream_update(void *param, int interval)
{
	struct ymf262_info *info = param;
	stream_update(info->stream);
}


static void *ymf262_start(const char *tag, int sndindex, int clock, const void *config)
{
	static const ymf262_interface dummy = { 0 };
	struct ymf262_info *info;
	int rate = clock/288;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	info->intf = config ? config : &dummy;

	/* stream system initialize */
	info->chip = YMF262Init(clock,rate);
	if (info->chip == NULL)
		return NULL;

	info->stream = stream_create(0,4,rate,info,ymf262_stream_update);

	/* YMF262 setup */
	YMF262SetTimerHandler (info->chip, timer_handler_262, info);
	YMF262SetIRQHandler   (info->chip, IRQHandler_262, info);
	YMF262SetUpdateHandler(info->chip, _stream_update, info);

	info->timer[0] = timer_alloc(timer_callback_262_0, info);
	info->timer[1] = timer_alloc(timer_callback_262_1, info);

	return info;
}

static void ymf262_stop(void *token)
{
	struct ymf262_info *info = token;
	YMF262Shutdown(info->chip);
}

/* reset */
static void ymf262_reset(void *token)
{
	struct ymf262_info *info = token;
	YMF262ResetChip(info->chip);
}

/* chip #0 */
READ8_HANDLER( YMF262_status_0_r ) {
	struct ymf262_info *info = sndti_token(SOUND_YMF262, 0);
	return YMF262Read(info->chip, 0);
}
WRITE8_HANDLER( YMF262_register_A_0_w ) {
	struct ymf262_info *info = sndti_token(SOUND_YMF262, 0);
	YMF262Write(info->chip, 0, data);
}
WRITE8_HANDLER( YMF262_data_A_0_w ) {
	struct ymf262_info *info = sndti_token(SOUND_YMF262, 0);
	YMF262Write(info->chip, 1, data);
}
WRITE8_HANDLER( YMF262_register_B_0_w ) {
	struct ymf262_info *info = sndti_token(SOUND_YMF262, 0);
	YMF262Write(info->chip, 2, data);
}
WRITE8_HANDLER( YMF262_data_B_0_w ) {
	struct ymf262_info *info = sndti_token(SOUND_YMF262, 0);
	YMF262Write(info->chip, 3, data);
}

/* chip #1 */
READ8_HANDLER( YMF262_status_1_r ) {
	struct ymf262_info *info = sndti_token(SOUND_YMF262, 1);
	return YMF262Read(info->chip, 0);
}
WRITE8_HANDLER( YMF262_register_A_1_w ) {
	struct ymf262_info *info = sndti_token(SOUND_YMF262, 1);
	YMF262Write(info->chip, 0, data);
}
WRITE8_HANDLER( YMF262_data_A_1_w ) {
	struct ymf262_info *info = sndti_token(SOUND_YMF262, 1);
	YMF262Write(info->chip, 1, data);
}
WRITE8_HANDLER( YMF262_register_B_1_w ) {
	struct ymf262_info *info = sndti_token(SOUND_YMF262, 1);
	YMF262Write(info->chip, 2, data);
}
WRITE8_HANDLER( YMF262_data_B_1_w ) {
	struct ymf262_info *info = sndti_token(SOUND_YMF262, 1);
	YMF262Write(info->chip, 3, data);
}


/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void ymf262_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void ymf262_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = ymf262_set_info;		break;
		case SNDINFO_PTR_START:							info->start = ymf262_start;				break;
		case SNDINFO_PTR_STOP:							info->stop = ymf262_stop;				break;
		case SNDINFO_PTR_RESET:							info->reset = ymf262_reset;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "YMF262";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Yamaha FM";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}

