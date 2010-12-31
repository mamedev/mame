/***************************************************************************

  262intf.c

  MAME interface for YMF262 (OPL3) emulator

***************************************************************************/
#include "emu.h"
#include "streams.h"
#include "262intf.h"
#include "ymf262.h"


typedef struct _ymf262_state ymf262_state;
struct _ymf262_state
{
	sound_stream *	stream;
	emu_timer *		timer[2];
	void *			chip;
	const ymf262_interface *intf;
	device_t *device;
};


INLINE ymf262_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == YMF262);
	return (ymf262_state *)downcast<legacy_device_base *>(device)->token();
}




static void IRQHandler_262(void *param,int irq)
{
	ymf262_state *info = (ymf262_state *)param;
	if (info->intf->handler) (info->intf->handler)(info->device, irq);
}

static TIMER_CALLBACK( timer_callback_262_0 )
{
	ymf262_state *info = (ymf262_state *)ptr;
	ymf262_timer_over(info->chip, 0);
}

static TIMER_CALLBACK( timer_callback_262_1 )
{
	ymf262_state *info = (ymf262_state *)ptr;
	ymf262_timer_over(info->chip, 1);
}

static void timer_handler_262(void *param,int timer, attotime period)
{
	ymf262_state *info = (ymf262_state *)param;
	if( attotime_compare(period, attotime_zero) == 0 )
	{	/* Reset FM Timer */
		timer_enable(info->timer[timer], 0);
	}
	else
	{	/* Start FM Timer */
		timer_adjust_oneshot(info->timer[timer], period, 0);
	}
}

static STREAM_UPDATE( ymf262_stream_update )
{
	ymf262_state *info = (ymf262_state *)param;
	ymf262_update_one(info->chip, outputs, samples);
}

static void _stream_update(void *param, int interval)
{
	ymf262_state *info = (ymf262_state *)param;
	stream_update(info->stream);
}


static DEVICE_START( ymf262 )
{
	static const ymf262_interface dummy = { 0 };
	ymf262_state *info = get_safe_token(device);
	int rate = device->clock()/288;

	info->intf = device->baseconfig().static_config() ? (const ymf262_interface *)device->baseconfig().static_config() : &dummy;
	info->device = device;

	/* stream system initialize */
	info->chip = ymf262_init(device,device->clock(),rate);
	assert_always(info->chip != NULL, "Error creating YMF262 chip");

	info->stream = stream_create(device,0,4,rate,info,ymf262_stream_update);

	/* YMF262 setup */
	ymf262_set_timer_handler (info->chip, timer_handler_262, info);
	ymf262_set_irq_handler   (info->chip, IRQHandler_262, info);
	ymf262_set_update_handler(info->chip, _stream_update, info);

	info->timer[0] = timer_alloc(device->machine, timer_callback_262_0, info);
	info->timer[1] = timer_alloc(device->machine, timer_callback_262_1, info);
}

static DEVICE_STOP( ymf262 )
{
	ymf262_state *info = get_safe_token(device);
	ymf262_shutdown(info->chip);
}

/* reset */
static DEVICE_RESET( ymf262 )
{
	ymf262_state *info = get_safe_token(device);
	ymf262_reset_chip(info->chip);
}


READ8_DEVICE_HANDLER( ymf262_r )
{
	ymf262_state *info = get_safe_token(device);
	return ymf262_read(info->chip, offset & 3);
}

WRITE8_DEVICE_HANDLER( ymf262_w )
{
	ymf262_state *info = get_safe_token(device);
	ymf262_write(info->chip, offset & 3, data);
}

READ8_DEVICE_HANDLER ( ymf262_status_r ) { return ymf262_r(device, 0); }
WRITE8_DEVICE_HANDLER( ymf262_register_a_w ) { ymf262_w(device, 0, data); }
WRITE8_DEVICE_HANDLER( ymf262_register_b_w ) { ymf262_w(device, 2, data); }
WRITE8_DEVICE_HANDLER( ymf262_data_a_w ) { ymf262_w(device, 1, data); }
WRITE8_DEVICE_HANDLER( ymf262_data_b_w ) { ymf262_w(device, 3, data); }


/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( ymf262 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(ymf262_state);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( ymf262 );				break;
		case DEVINFO_FCT_STOP:							info->stop = DEVICE_STOP_NAME( ymf262 );				break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME( ymf262 );				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "YMF262");							break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Yamaha FM");						break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");								break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);							break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(YMF262, ymf262);
