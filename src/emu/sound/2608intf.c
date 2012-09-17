/***************************************************************************

  2608intf.c

  The YM2608 emulator supports up to 2 chips.
  Each chip has the following connections:
  - Status Read / Control Write A
  - Port Read / Data Write A
  - Control Write B
  - Data Write B

***************************************************************************/

#include "emu.h"
#include "ay8910.h"
#include "2608intf.h"
#include "fm.h"

struct ym2608_state
{
	sound_stream *	stream;
	emu_timer *	timer[2];
	void *			chip;
	void *			psg;
	const ym2608_interface *intf;
	device_t *device;
};


INLINE ym2608_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == YM2608);
	return (ym2608_state *)downcast<ym2608_device *>(device)->token();
}



static void psg_set_clock(void *param, int clock)
{
	ym2608_state *info = (ym2608_state *)param;
	ay8910_set_clock_ym(info->psg, clock);
}

static void psg_write(void *param, int address, int data)
{
	ym2608_state *info = (ym2608_state *)param;
	ay8910_write_ym(info->psg, address, data);
}

static int psg_read(void *param)
{
	ym2608_state *info = (ym2608_state *)param;
	return ay8910_read_ym(info->psg);
}

static void psg_reset(void *param)
{
	ym2608_state *info = (ym2608_state *)param;
	ay8910_reset_ym(info->psg);
}

static const ssg_callbacks psgintf =
{
	psg_set_clock,
	psg_write,
	psg_read,
	psg_reset
};


/* IRQ Handler */
static void IRQHandler(void *param,int irq)
{
	ym2608_state *info = (ym2608_state *)param;
	if(info->intf->handler) info->intf->handler(info->device, irq);
}

/* Timer overflow callback from timer.c */
static TIMER_CALLBACK( timer_callback_2608_0 )
{
	ym2608_state *info = (ym2608_state *)ptr;
	ym2608_timer_over(info->chip,0);
}

static TIMER_CALLBACK( timer_callback_2608_1 )
{
	ym2608_state *info = (ym2608_state *)ptr;
	ym2608_timer_over(info->chip,1);
}

static void timer_handler(void *param,int c,int count,int clock)
{
	ym2608_state *info = (ym2608_state *)param;
	if( count == 0 )
	{	/* Reset FM Timer */
		info->timer[c]->enable(false);
	}
	else
	{	/* Start FM Timer */
		attotime period = attotime::from_hz(clock) * count;
		if (!info->timer[c]->enable(true))
			info->timer[c]->adjust(period);
	}
}

/* update request from fm.c */
void ym2608_update_request(void *param)
{
	ym2608_state *info = (ym2608_state *)param;
	info->stream->update();
}

static STREAM_UPDATE( ym2608_stream_update )
{
	ym2608_state *info = (ym2608_state *)param;
	ym2608_update_one(info->chip, outputs, samples);
}


static void ym2608_intf_postload(ym2608_state *info)
{
	ym2608_postload(info->chip);
}


static DEVICE_START( ym2608 )
{
	static const ym2608_interface generic_2608 =
	{
		{
			AY8910_LEGACY_OUTPUT | AY8910_SINGLE_OUTPUT,
			AY8910_DEFAULT_LOADS,
			DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL
		},
		NULL
	};
	const ym2608_interface *intf = device->static_config() ? (const ym2608_interface *)device->static_config() : &generic_2608;
	int rate = device->clock()/72;
	void *pcmbufa;
	int  pcmsizea;

	ym2608_state *info = get_safe_token(device);

	info->intf = intf;
	info->device = device;

	/* FIXME: Force to use simgle output */
	info->psg = ay8910_start_ym(NULL, YM2608, device, device->clock(), &intf->ay8910_intf);
	assert_always(info->psg != NULL, "Error creating YM2608/AY8910 chip");

	/* Timer Handler set */
	info->timer[0] = device->machine().scheduler().timer_alloc(FUNC(timer_callback_2608_0), info);
	info->timer[1] = device->machine().scheduler().timer_alloc(FUNC(timer_callback_2608_1), info);

	/* stream system initialize */
	info->stream = device->machine().sound().stream_alloc(*device,0,2,rate,info,ym2608_stream_update);
	/* setup adpcm buffers */
	pcmbufa  = *device->region();
	pcmsizea = device->region()->bytes();

	/* initialize YM2608 */
	info->chip = ym2608_init(info,device,device->clock(),rate,
		           pcmbufa,pcmsizea,
		           timer_handler,IRQHandler,&psgintf);
	assert_always(info->chip != NULL, "Error creating YM2608 chip");

	device->machine().save().register_postload(save_prepost_delegate(FUNC(ym2608_intf_postload), info));
}

static DEVICE_STOP( ym2608 )
{
	ym2608_state *info = get_safe_token(device);
	ym2608_shutdown(info->chip);
	ay8910_stop_ym(info->psg);
}

static DEVICE_RESET( ym2608 )
{
	ym2608_state *info = get_safe_token(device);
	ym2608_reset_chip(info->chip);
}


READ8_DEVICE_HANDLER( ym2608_r )
{
	ym2608_state *info = get_safe_token(device);
	return ym2608_read(info->chip, offset & 3);
}

WRITE8_DEVICE_HANDLER( ym2608_w )
{
	ym2608_state *info = get_safe_token(device);
	ym2608_write(info->chip, offset & 3, data);
}

READ8_DEVICE_HANDLER( ym2608_read_port_r ) { return ym2608_r(device, space, 1); }
READ8_DEVICE_HANDLER( ym2608_status_port_a_r ) { return ym2608_r(device, space, 0); }
READ8_DEVICE_HANDLER( ym2608_status_port_b_r ) { return ym2608_r(device, space, 2); }

WRITE8_DEVICE_HANDLER( ym2608_control_port_a_w ) { ym2608_w(device, space, 0, data); }
WRITE8_DEVICE_HANDLER( ym2608_control_port_b_w ) { ym2608_w(device, space, 2, data); }
WRITE8_DEVICE_HANDLER( ym2608_data_port_a_w ) { ym2608_w(device, space, 1, data); }
WRITE8_DEVICE_HANDLER( ym2608_data_port_b_w ) { ym2608_w(device, space, 3, data); }

const device_type YM2608 = &device_creator<ym2608_device>;

ym2608_device::ym2608_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, YM2608, "YM2608", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(ym2608_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ym2608_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ym2608_device::device_start()
{
	DEVICE_START_NAME( ym2608 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ym2608_device::device_reset()
{
	DEVICE_RESET_NAME( ym2608 )(this);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void ym2608_device::device_stop()
{
	DEVICE_STOP_NAME( ym2608 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void ym2608_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


