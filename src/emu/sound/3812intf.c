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
#include "3812intf.h"
#include "fm.h"
#include "sound/fmopl.h"


struct ym3812_state
{
	sound_stream *  stream;
	emu_timer *     timer[2];
	void *          chip;
	const ym3812_interface *intf;
	device_t *device;
};


INLINE ym3812_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == YM3812);
	return (ym3812_state *)downcast<ym3812_device *>(device)->token();
}



static void IRQHandler(void *param,int irq)
{
	ym3812_state *info = (ym3812_state *)param;
	if (info->intf->handler) (info->intf->handler)(info->device, irq ? ASSERT_LINE : CLEAR_LINE);
}
static TIMER_CALLBACK( timer_callback_0 )
{
	ym3812_state *info = (ym3812_state *)ptr;
	ym3812_timer_over(info->chip,0);
}

static TIMER_CALLBACK( timer_callback_1 )
{
	ym3812_state *info = (ym3812_state *)ptr;
	ym3812_timer_over(info->chip,1);
}

static void TimerHandler(void *param,int c,attotime period)
{
	ym3812_state *info = (ym3812_state *)param;
	if( period == attotime::zero )
	{   /* Reset FM Timer */
		info->timer[c]->enable(false);
	}
	else
	{   /* Start FM Timer */
		info->timer[c]->adjust(period);
	}
}


static STREAM_UPDATE( ym3812_stream_update )
{
	ym3812_state *info = (ym3812_state *)param;
	ym3812_update_one(info->chip, outputs[0], samples);
}

static void _stream_update(void * param, int interval)
{
	ym3812_state *info = (ym3812_state *)param;
	info->stream->update();
}


static DEVICE_START( ym3812 )
{
	static const ym3812_interface dummy = { 0 };
	ym3812_state *info = get_safe_token(device);
	int rate = device->clock()/72;

	info->intf = device->static_config() ? (const ym3812_interface *)device->static_config() : &dummy;
	info->device = device;

	/* stream system initialize */
	info->chip = ym3812_init(device,device->clock(),rate);
	assert_always(info->chip != NULL, "Error creating YM3812 chip");

	info->stream = device->machine().sound().stream_alloc(*device,0,1,rate,info,ym3812_stream_update);

	/* YM3812 setup */
	ym3812_set_timer_handler (info->chip, TimerHandler, info);
	ym3812_set_irq_handler   (info->chip, IRQHandler, info);
	ym3812_set_update_handler(info->chip, _stream_update, info);

	info->timer[0] = device->machine().scheduler().timer_alloc(FUNC(timer_callback_0), info);
	info->timer[1] = device->machine().scheduler().timer_alloc(FUNC(timer_callback_1), info);
}

static DEVICE_STOP( ym3812 )
{
	ym3812_state *info = get_safe_token(device);
	ym3812_shutdown(info->chip);
}

static DEVICE_RESET( ym3812 )
{
	ym3812_state *info = get_safe_token(device);
	ym3812_reset_chip(info->chip);
}


READ8_DEVICE_HANDLER( ym3812_r )
{
	ym3812_state *info = get_safe_token(device);
	return ym3812_read(info->chip, offset & 1);
}

WRITE8_DEVICE_HANDLER( ym3812_w )
{
	ym3812_state *info = get_safe_token(device);
	ym3812_write(info->chip, offset & 1, data);
}

READ8_DEVICE_HANDLER( ym3812_status_port_r ) { return ym3812_r(device, space, 0); }
READ8_DEVICE_HANDLER( ym3812_read_port_r ) { return ym3812_r(device, space, 1); }
WRITE8_DEVICE_HANDLER( ym3812_control_port_w ) { ym3812_w(device, space, 0, data); }
WRITE8_DEVICE_HANDLER( ym3812_write_port_w ) { ym3812_w(device, space, 1, data); }


const device_type YM3812 = &device_creator<ym3812_device>;

ym3812_device::ym3812_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, YM3812, "YM3812", tag, owner, clock),
		device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(ym3812_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ym3812_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ym3812_device::device_start()
{
	DEVICE_START_NAME( ym3812 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ym3812_device::device_reset()
{
	DEVICE_RESET_NAME( ym3812 )(this);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void ym3812_device::device_stop()
{
	DEVICE_STOP_NAME( ym3812 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void ym3812_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}
