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
#include "8950intf.h"
#include "fm.h"
#include "sound/fmopl.h"


struct y8950_state
{
	sound_stream *	stream;
	emu_timer *		timer[2];
	void *			chip;
	const y8950_interface *intf;
	device_t *device;
};


INLINE y8950_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == Y8950);
	return (y8950_state *)downcast<y8950_device *>(device)->token();
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
	if( period == attotime::zero )
	{	/* Reset FM Timer */
		info->timer[c]->enable(false);
	}
	else
	{	/* Start FM Timer */
		info->timer[c]->adjust(period);
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
	info->stream->update();
}


static DEVICE_START( y8950 )
{
	static const y8950_interface dummy = { 0 };
	y8950_state *info = get_safe_token(device);
	int rate = device->clock()/72;

	info->intf = device->static_config() ? (const y8950_interface *)device->static_config() : &dummy;
	info->device = device;

	/* stream system initialize */
	info->chip = y8950_init(device,device->clock(),rate);
	assert_always(info->chip != NULL, "Error creating Y8950 chip");

	/* ADPCM ROM data */
	y8950_set_delta_t_memory(info->chip, *device->region(), device->region()->bytes());

	info->stream = device->machine().sound().stream_alloc(*device,0,1,rate,info,y8950_stream_update);
	/* port and keyboard handler */
	y8950_set_port_handler(info->chip, Y8950PortHandler_w, Y8950PortHandler_r, info);
	y8950_set_keyboard_handler(info->chip, Y8950KeyboardHandler_w, Y8950KeyboardHandler_r, info);

	/* Y8950 setup */
	y8950_set_timer_handler (info->chip, TimerHandler, info);
	y8950_set_irq_handler   (info->chip, IRQHandler, info);
	y8950_set_update_handler(info->chip, _stream_update, info);

	info->timer[0] = device->machine().scheduler().timer_alloc(FUNC(timer_callback_0), info);
	info->timer[1] = device->machine().scheduler().timer_alloc(FUNC(timer_callback_1), info);
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


const device_type Y8950 = &device_creator<y8950_device>;

y8950_device::y8950_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, Y8950, "Y8950", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(y8950_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void y8950_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void y8950_device::device_start()
{
	DEVICE_START_NAME( y8950 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void y8950_device::device_reset()
{
	DEVICE_RESET_NAME( y8950 )(this);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void y8950_device::device_stop()
{
	DEVICE_STOP_NAME( y8950 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void y8950_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


