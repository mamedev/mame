/***************************************************************************

  2612intf.c

  The YM2612 emulator supports up to 2 chips.
  Each chip has the following connections:
  - Status Read / Control Write A
  - Port Read / Data Write A
  - Control Write B
  - Data Write B

***************************************************************************/

#include "emu.h"
#include "sound/fm.h"
#include "sound/2612intf.h"


typedef struct _ym2612_state ym2612_state;
struct _ym2612_state
{
	sound_stream *	stream;
	emu_timer *		timer[2];
	void *			chip;
	const ym2612_interface *intf;
	device_t *device;
};


INLINE ym2612_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == YM2612 || device->type() == YM3438);
	return (ym2612_state *)downcast<ym2612_device *>(device)->token();
}



/*------------------------- TM2612 -------------------------------*/
/* IRQ Handler */
static void IRQHandler(void *param,int irq)
{
	ym2612_state *info = (ym2612_state *)param;
	if(info->intf->handler) info->intf->handler(info->device, irq);
}

/* Timer overflow callback from timer.c */
static TIMER_CALLBACK( timer_callback_2612_0 )
{
	ym2612_state *info = (ym2612_state *)ptr;
	ym2612_timer_over(info->chip,0);
}

static TIMER_CALLBACK( timer_callback_2612_1 )
{
	ym2612_state *info = (ym2612_state *)ptr;
	ym2612_timer_over(info->chip,1);
}

static void timer_handler(void *param,int c,int count,int clock)
{
	ym2612_state *info = (ym2612_state *)param;
	if( count == 0 )
	{	/* Reset FM Timer */
		info->timer[c]->enable(false);
	}
	else
	{	/* Start FM Timer */
		attotime period = attotime::from_hz(clock) * count;
		if (!info->timer[c]->enable(1))
			info->timer[c]->adjust(period);
	}
}

/* update request from fm.c */
void ym2612_update_request(void *param)
{
	ym2612_state *info = (ym2612_state *)param;
	info->stream->update();
}

/***********************************************************/
/*    YM2612                                               */
/***********************************************************/

static STREAM_UPDATE( ym2612_stream_update )
{
	ym2612_state *info = (ym2612_state *)param;
	ym2612_update_one(info->chip, outputs, samples);
}


static void ym2612_intf_postload(ym2612_state *info)
{
	ym2612_postload(info->chip);
}


static DEVICE_START( ym2612 )
{
	static const ym2612_interface dummy = { 0 };
	ym2612_state *info = get_safe_token(device);
	int rate = device->clock()/72;

	info->intf = device->static_config() ? (const ym2612_interface *)device->static_config() : &dummy;
	info->device = device;

	/* FM init */
	/* Timer Handler set */
	info->timer[0] = device->machine().scheduler().timer_alloc(FUNC(timer_callback_2612_0), info);
	info->timer[1] = device->machine().scheduler().timer_alloc(FUNC(timer_callback_2612_1), info);

	/* stream system initialize */
	info->stream = device->machine().sound().stream_alloc(*device,0,2,rate,info,ym2612_stream_update);

	/**** initialize YM2612 ****/
	info->chip = ym2612_init(info,device,device->clock(),rate,timer_handler,IRQHandler);
	assert_always(info->chip != NULL, "Error creating YM2612 chip");

	device->machine().save().register_postload(save_prepost_delegate(FUNC(ym2612_intf_postload), info));
}


static DEVICE_STOP( ym2612 )
{
	ym2612_state *info = get_safe_token(device);
	ym2612_shutdown(info->chip);
}

static DEVICE_RESET( ym2612 )
{
	ym2612_state *info = get_safe_token(device);
	ym2612_reset_chip(info->chip);
}


READ8_DEVICE_HANDLER( ym2612_r )
{
	ym2612_state *info = get_safe_token(device);
	return ym2612_read(info->chip, offset & 3);
}

WRITE8_DEVICE_HANDLER( ym2612_w )
{
	ym2612_state *info = get_safe_token(device);
	ym2612_write(info->chip, offset & 3, data);
}


READ8_DEVICE_HANDLER( ym2612_status_port_a_r ) { return ym2612_r(device, 0); }
READ8_DEVICE_HANDLER( ym2612_status_port_b_r ) { return ym2612_r(device, 2); }
READ8_DEVICE_HANDLER( ym2612_data_port_a_r ) { return ym2612_r(device, 1); }
READ8_DEVICE_HANDLER( ym2612_data_port_b_r ) { return ym2612_r(device, 3); }

WRITE8_DEVICE_HANDLER( ym2612_control_port_a_w ) { ym2612_w(device, 0, data); }
WRITE8_DEVICE_HANDLER( ym2612_control_port_b_w ) { ym2612_w(device, 2, data); }
WRITE8_DEVICE_HANDLER( ym2612_data_port_a_w ) { ym2612_w(device, 1, data); }
WRITE8_DEVICE_HANDLER( ym2612_data_port_b_w ) { ym2612_w(device, 3, data); }

const device_type YM2612 = &device_creator<ym2612_device>;

ym2612_device::ym2612_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, YM2612, "YM2612", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(ym2612_state));
}
ym2612_device::ym2612_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(ym2612_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ym2612_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ym2612_device::device_start()
{
	DEVICE_START_NAME( ym2612 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ym2612_device::device_reset()
{
	DEVICE_RESET_NAME( ym2612 )(this);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void ym2612_device::device_stop()
{
	DEVICE_STOP_NAME( ym2612 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void ym2612_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


const device_type YM3438 = &device_creator<ym3438_device>;

ym3438_device::ym3438_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ym2612_device(mconfig, YM3438, "YM3438", tag, owner, clock)
{
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void ym3438_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


