#include "emu.h"
#include "2203intf.h"
#include "fm.h"


struct ym2203_state
{
	sound_stream *  stream;
	emu_timer *     timer[2];
	void *          chip;
	void *          psg;
	const ym2203_interface *intf;
	devcb_resolved_write_line irqhandler;
	device_t *device;
};


INLINE ym2203_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == YM2203);
	return (ym2203_state *)downcast<ym2203_device *>(device)->token();
}


static void psg_set_clock(void *param, int clock)
{
	ym2203_state *info = (ym2203_state *)param;
	ay8910_set_clock_ym(info->psg, clock);
}

static void psg_write(void *param, int address, int data)
{
	ym2203_state *info = (ym2203_state *)param;
	ay8910_write_ym(info->psg, address, data);
}

static int psg_read(void *param)
{
	ym2203_state *info = (ym2203_state *)param;
	return ay8910_read_ym(info->psg);
}

static void psg_reset(void *param)
{
	ym2203_state *info = (ym2203_state *)param;
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
	ym2203_state *info = (ym2203_state *)param;
	if (!info->irqhandler.isnull())
		info->irqhandler(irq);
}

/* Timer overflow callback from timer.c */
static TIMER_CALLBACK( timer_callback_2203_0 )
{
	ym2203_state *info = (ym2203_state *)ptr;
	ym2203_timer_over(info->chip,0);
}

static TIMER_CALLBACK( timer_callback_2203_1 )
{
	ym2203_state *info = (ym2203_state *)ptr;
	ym2203_timer_over(info->chip,1);
}

/* update request from fm.c */
void ym2203_update_request(void *param)
{
	ym2203_state *info = (ym2203_state *)param;
	info->stream->update();
}


static void timer_handler(void *param,int c,int count,int clock)
{
	ym2203_state *info = (ym2203_state *)param;
	if( count == 0 )
	{   /* Reset FM Timer */
		info->timer[c]->enable(false);
	}
	else
	{   /* Start FM Timer */
		attotime period = attotime::from_hz(clock) * count;
		if (!info->timer[c]->enable(true))
			info->timer[c]->adjust(period);
	}
}

static STREAM_UPDATE( ym2203_stream_update )
{
	ym2203_state *info = (ym2203_state *)param;
	ym2203_update_one(info->chip, outputs[0], samples);
}


static void ym2203_intf_postload (ym2203_state *info)
{
	ym2203_postload(info->chip);
}


static DEVICE_START( ym2203 )
{
	static const ym2203_interface generic_2203 =
	{
		{
			AY8910_LEGACY_OUTPUT,
			AY8910_DEFAULT_LOADS,
			DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL
		},
		DEVCB_NULL
	};
	const ym2203_interface *intf = device->static_config() ? (const ym2203_interface *)device->static_config() : &generic_2203;
	ym2203_state *info = get_safe_token(device);
	int rate = device->clock()/72; /* ??? */

	info->irqhandler.resolve(intf->irqhandler, *device);
	info->intf = intf;
	info->device = device;
	info->psg = ay8910_start_ym(NULL, YM2203, device, device->clock(), &intf->ay8910_intf);
	assert_always(info->psg != NULL, "Error creating YM2203/AY8910 chip");

	/* Timer Handler set */
	info->timer[0] = device->machine().scheduler().timer_alloc(FUNC(timer_callback_2203_0), info);
	info->timer[1] = device->machine().scheduler().timer_alloc(FUNC(timer_callback_2203_1), info);

	/* stream system initialize */
	info->stream = device->machine().sound().stream_alloc(*device,0,1,rate,info,ym2203_stream_update);

	/* Initialize FM emurator */
	info->chip = ym2203_init(info,device,device->clock(),rate,timer_handler,IRQHandler,&psgintf);
	assert_always(info->chip != NULL, "Error creating YM2203 chip");

	device->machine().save().register_postload(save_prepost_delegate(FUNC(ym2203_intf_postload), info));
}

static DEVICE_STOP( ym2203 )
{
	ym2203_state *info = get_safe_token(device);
	ym2203_shutdown(info->chip);
	ay8910_stop_ym(info->psg);
}

static DEVICE_RESET( ym2203 )
{
	ym2203_state *info = get_safe_token(device);
	ym2203_reset_chip(info->chip);
}



READ8_DEVICE_HANDLER( ym2203_r )
{
	ym2203_state *info = get_safe_token(device);
	return ym2203_read(info->chip, offset & 1);
}

WRITE8_DEVICE_HANDLER( ym2203_w )
{
	ym2203_state *info = get_safe_token(device);
	ym2203_write(info->chip, offset & 1, data);
}


READ8_DEVICE_HANDLER( ym2203_status_port_r ) { return ym2203_r(device, space, 0); }
READ8_DEVICE_HANDLER( ym2203_read_port_r ) { return ym2203_r(device, space, 1); }
WRITE8_DEVICE_HANDLER( ym2203_control_port_w ) { ym2203_w(device, space, 0, data); }
WRITE8_DEVICE_HANDLER( ym2203_write_port_w ) { ym2203_w(device, space, 1, data); }

const device_type YM2203 = &device_creator<ym2203_device>;

ym2203_device::ym2203_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, YM2203, "YM2203", tag, owner, clock),
		device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(ym2203_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ym2203_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ym2203_device::device_start()
{
	DEVICE_START_NAME( ym2203 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ym2203_device::device_reset()
{
	DEVICE_RESET_NAME( ym2203 )(this);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void ym2203_device::device_stop()
{
	DEVICE_STOP_NAME( ym2203 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void ym2203_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}
