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
	ymf262_timer_over(info->chip, 0);
}

static TIMER_CALLBACK( timer_callback_262_1 )
{
	struct ymf262_info *info = ptr;
	ymf262_timer_over(info->chip, 1);
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
	ymf262_update_one(info->chip, buffers, length);
}

static void _stream_update(void *param, int interval)
{
	struct ymf262_info *info = param;
	stream_update(info->stream);
}


static SND_START( ymf262 )
{
	static const ymf262_interface dummy = { 0 };
	struct ymf262_info *info;
	int rate = clock/288;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	info->intf = config ? config : &dummy;

	/* stream system initialize */
	info->chip = ymf262_init(clock,rate);
	if (info->chip == NULL)
		return NULL;

	info->stream = stream_create(0,4,rate,info,ymf262_stream_update);

	/* YMF262 setup */
	ymf262_set_timer_handler (info->chip, timer_handler_262, info);
	ymf262_set_irq_handler   (info->chip, IRQHandler_262, info);
	ymf262_set_update_handler(info->chip, _stream_update, info);

	info->timer[0] = timer_alloc(device->machine, timer_callback_262_0, info);
	info->timer[1] = timer_alloc(device->machine, timer_callback_262_1, info);

	return info;
}

static SND_STOP( ymf262 )
{
	struct ymf262_info *info = device->token;
	ymf262_shutdown(info->chip);
}

/* reset */
static SND_RESET( ymf262 )
{
	struct ymf262_info *info = device->token;
	ymf262_reset_chip(info->chip);
}

/* chip #0 */
READ8_HANDLER( ymf262_status_0_r ) {
	struct ymf262_info *info = sndti_token(SOUND_YMF262, 0);
	return ymf262_read(info->chip, 0);
}
WRITE8_HANDLER( ymf262_register_a_0_w ) {
	struct ymf262_info *info = sndti_token(SOUND_YMF262, 0);
	ymf262_write(info->chip, 0, data);
}
WRITE8_HANDLER( ymf262_data_a_0_w ) {
	struct ymf262_info *info = sndti_token(SOUND_YMF262, 0);
	ymf262_write(info->chip, 1, data);
}
WRITE8_HANDLER( ymf262_register_b_0_w ) {
	struct ymf262_info *info = sndti_token(SOUND_YMF262, 0);
	ymf262_write(info->chip, 2, data);
}
WRITE8_HANDLER( ymf262_data_b_0_w ) {
	struct ymf262_info *info = sndti_token(SOUND_YMF262, 0);
	ymf262_write(info->chip, 3, data);
}

/* chip #1 */
READ8_HANDLER( ymf262_status_1_r ) {
	struct ymf262_info *info = sndti_token(SOUND_YMF262, 1);
	return ymf262_read(info->chip, 0);
}
WRITE8_HANDLER( ymf262_register_a_1_w ) {
	struct ymf262_info *info = sndti_token(SOUND_YMF262, 1);
	ymf262_write(info->chip, 0, data);
}
WRITE8_HANDLER( ymf262_data_a_1_w ) {
	struct ymf262_info *info = sndti_token(SOUND_YMF262, 1);
	ymf262_write(info->chip, 1, data);
}
WRITE8_HANDLER( ymf262_register_b_1_w ) {
	struct ymf262_info *info = sndti_token(SOUND_YMF262, 1);
	ymf262_write(info->chip, 2, data);
}
WRITE8_HANDLER( ymf262_data_b_1_w ) {
	struct ymf262_info *info = sndti_token(SOUND_YMF262, 1);
	ymf262_write(info->chip, 3, data);
}


/**************************************************************************
 * Generic get_info
 **************************************************************************/

static SND_SET_INFO( ymf262 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( ymf262 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( ymf262 );		break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( ymf262 );				break;
		case SNDINFO_PTR_STOP:							info->stop = SND_STOP_NAME( ymf262 );				break;
		case SNDINFO_PTR_RESET:							info->reset = SND_RESET_NAME( ymf262 );				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "YMF262";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Yamaha FM";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}

