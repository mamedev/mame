/******************************************************************************
* FILE
*   Yamaha 3812 emulator interface - MAME VERSION
*
* CREATED BY
*   Ernesto Corvi
*
* UPDATE LOG
*   JB  28-04-2002  Fixed simultaneous usage of all three different chip types.
*                       Used real sample rate when resample filter is active.
*       AAT 12-28-2001  Protected Y8950 from accessing unmapped port and keyboard handlers.
*   CHS 1999-01-09  Fixes new ym3812 emulation interface.
*   CHS 1998-10-23  Mame streaming sound chip update
*   EC  1998        Created Interface
*
* NOTES
*
******************************************************************************/
#include "emu.h"
#include "streams.h"
#include "8950intf.h"
#include "fm.h"
#include "sound/fmopl.h"


typedef struct _y8950_state y8950_state;
struct _y8950_state
{
	sound_stream *	stream;
	emu_timer *		timer[2];
	void *			chip;
	const y8950_interface *intf;
	running_device *device;
};


INLINE y8950_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == SOUND_Y8950);
	return (y8950_state *)downcast<legacy_device_base *>(device)->token();
}


static void IRQHandler(void *param,int irq)
{
	y8950_state *info = (y8950_state *)param;
	if (info->intf->handler) (info->intf->handler)(info->device, irq ? ASSERT_LINE : CLEAR_LINE);
}
static TIMER_CALLBACK( timer_callback_0 )
{
	y8950_state *info = (y8950_state *)ptr;
	y8950_timer_over(info->chip,0);
}
static TIMER_CALLBACK( timer_callback_1 )
{
	y8950_state *info = (y8950_state *)ptr;
	y8950_timer_over(info->chip,1);
}
static void TimerHandler(void *param,int c,attotime period)
{
	y8950_state *info = (y8950_state *)param;
	if( attotime_compare(period, attotime_zero) == 0 )
	{	/* Reset FM Timer */
		timer_enable(info->timer[c], 0);
	}
	else
	{	/* Start FM Timer */
		timer_adjust_oneshot(info->timer[c], period, 0);
	}
}


static unsigned char Y8950PortHandler_r(void *param)
{
	y8950_state *info = (y8950_state *)param;
	if (info->intf->portread)
		return info->intf->portread(info->device,0);
	return 0;
}

static void Y8950PortHandler_w(void *param,unsigned char data)
{
	y8950_state *info = (y8950_state *)param;
	if (info->intf->portwrite)
		info->intf->portwrite(info->device,0,data);
}

static unsigned char Y8950KeyboardHandler_r(void *param)
{
	y8950_state *info = (y8950_state *)param;
	if (info->intf->keyboardread)
		return info->intf->keyboardread(info->device,0);
	return 0;
}

static void Y8950KeyboardHandler_w(void *param,unsigned char data)
{
	y8950_state *info = (y8950_state *)param;
	if (info->intf->keyboardwrite)
		info->intf->keyboardwrite(info->device,0,data);
}

static STREAM_UPDATE( y8950_stream_update )
{
	y8950_state *info = (y8950_state *)param;
	y8950_update_one(info->chip, outputs[0], samples);
}

static void _stream_update(void *param, int interval)
{
	y8950_state *info = (y8950_state *)param;
	stream_update(info->stream);
}


static DEVICE_START( y8950 )
{
	static const y8950_interface dummy = { 0 };
	y8950_state *info = get_safe_token(device);
	int rate = device->clock()/72;

	info->intf = device->baseconfig().static_config() ? (const y8950_interface *)device->baseconfig().static_config() : &dummy;
	info->device = device;

	/* stream system initialize */
	info->chip = y8950_init(device,device->clock(),rate);
	assert_always(info->chip != NULL, "Error creating Y8950 chip");

	/* ADPCM ROM data */
	y8950_set_delta_t_memory(info->chip, *device->region(), device->region()->bytes());

	info->stream = stream_create(device,0,1,rate,info,y8950_stream_update);

	/* port and keyboard handler */
	y8950_set_port_handler(info->chip, Y8950PortHandler_w, Y8950PortHandler_r, info);
	y8950_set_keyboard_handler(info->chip, Y8950KeyboardHandler_w, Y8950KeyboardHandler_r, info);

	/* Y8950 setup */
	y8950_set_timer_handler (info->chip, TimerHandler, info);
	y8950_set_irq_handler   (info->chip, IRQHandler, info);
	y8950_set_update_handler(info->chip, _stream_update, info);

	info->timer[0] = timer_alloc(device->machine, timer_callback_0, info);
	info->timer[1] = timer_alloc(device->machine, timer_callback_1, info);
}

static DEVICE_STOP( y8950 )
{
	y8950_state *info = get_safe_token(device);
	y8950_shutdown(info->chip);
}

static DEVICE_RESET( y8950 )
{
	y8950_state *info = get_safe_token(device);
	y8950_reset_chip(info->chip);
}


READ8_DEVICE_HANDLER( y8950_r )
{
	y8950_state *info = get_safe_token(device);
	return y8950_read(info->chip, offset & 1);
}

WRITE8_DEVICE_HANDLER( y8950_w )
{
	y8950_state *info = get_safe_token(device);
	y8950_write(info->chip, offset & 1, data);
}

READ8_DEVICE_HANDLER( y8950_status_port_r ) { return y8950_r(device, 0); }
READ8_DEVICE_HANDLER( y8950_read_port_r ) { return y8950_r(device, 1); }
WRITE8_DEVICE_HANDLER( y8950_control_port_w ) { y8950_w(device, 0, data); }
WRITE8_DEVICE_HANDLER( y8950_write_port_w ) { y8950_w(device, 1, data); }


/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( y8950 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(y8950_state);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( y8950 );				break;
		case DEVINFO_FCT_STOP:							info->stop = DEVICE_STOP_NAME( y8950 );				break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME( y8950 );				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Y8950");							break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Yamaha FM");						break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");								break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);							break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(Y8950, y8950);
