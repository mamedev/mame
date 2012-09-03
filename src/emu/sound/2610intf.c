/***************************************************************************

  2610intf.c

  The YM2610 emulator supports up to 2 chips.
  Each chip has the following connections:
  - Status Read / Control Write A
  - Port Read / Data Write A
  - Control Write B
  - Data Write B

***************************************************************************/

#include "emu.h"
#include "ay8910.h"
#include "2610intf.h"
#include "fm.h"

typedef struct _ym2610_state ym2610_state;
struct _ym2610_state
{
	sound_stream *	stream;
	emu_timer *		timer[2];
	void *			chip;
	void *			psg;
	const ym2610_interface *intf;
	device_t *device;
};


INLINE ym2610_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == YM2610 || device->type() == YM2610B);
	return (ym2610_state *)downcast<ym2610_device *>(device)->token();
}


static void psg_set_clock(void *param, int clock)
{
	ym2610_state *info = (ym2610_state *)param;
	ay8910_set_clock_ym(info->psg, clock);
}

static void psg_write(void *param, int address, int data)
{
	ym2610_state *info = (ym2610_state *)param;
	ay8910_write_ym(info->psg, address, data);
}

static int psg_read(void *param)
{
	ym2610_state *info = (ym2610_state *)param;
	return ay8910_read_ym(info->psg);
}

static void psg_reset(void *param)
{
	ym2610_state *info = (ym2610_state *)param;
	ay8910_reset_ym(info->psg);
}

static const ssg_callbacks psgintf =
{
	psg_set_clock,
	psg_write,
	psg_read,
	psg_reset
};

/*------------------------- TM2610 -------------------------------*/
/* IRQ Handler */
static void IRQHandler(void *param,int irq)
{
	ym2610_state *info = (ym2610_state *)param;
	if(info->intf->handler) info->intf->handler(info->device, irq);
}

/* Timer overflow callback from timer.c */
static TIMER_CALLBACK( timer_callback_0 )
{
	ym2610_state *info = (ym2610_state *)ptr;
	ym2610_timer_over(info->chip,0);
}

static TIMER_CALLBACK( timer_callback_1 )
{
	ym2610_state *info = (ym2610_state *)ptr;
	ym2610_timer_over(info->chip,1);
}

static void timer_handler(void *param,int c,int count,int clock)
{
	ym2610_state *info = (ym2610_state *)param;
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
void ym2610_update_request(void *param)
{
	ym2610_state *info = (ym2610_state *)param;
	info->stream->update();
}


static STREAM_UPDATE( ym2610_stream_update )
{
	ym2610_state *info = (ym2610_state *)param;
	ym2610_update_one(info->chip, outputs, samples);
}

static STREAM_UPDATE( ym2610b_stream_update )
{
	ym2610_state *info = (ym2610_state *)param;
	ym2610b_update_one(info->chip, outputs, samples);
}


static void ym2610_intf_postload(ym2610_state *info)
{
	ym2610_postload(info->chip);
}


static DEVICE_START( ym2610 )
{
	static const ym2610_interface generic_2610 = { 0 };
	static const ay8910_interface generic_ay8910 =
	{
		AY8910_LEGACY_OUTPUT | AY8910_SINGLE_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL
	};
	const ym2610_interface *intf = device->static_config() ? (const ym2610_interface *)device->static_config() : &generic_2610;
	int rate = device->clock()/72;
	void *pcmbufa,*pcmbufb;
	int  pcmsizea,pcmsizeb;
	ym2610_state *info = get_safe_token(device);
	astring name;
	device_type type = device->type();

	info->intf = intf;
	info->device = device;
	info->psg = ay8910_start_ym(NULL, device->type(), device, device->clock(), &generic_ay8910);
	assert_always(info->psg != NULL, "Error creating YM2610/AY8910 chip");

	/* Timer Handler set */
	info->timer[0] = device->machine().scheduler().timer_alloc(FUNC(timer_callback_0), info);
	info->timer[1] = device->machine().scheduler().timer_alloc(FUNC(timer_callback_1), info);

	/* stream system initialize */
	info->stream = device->machine().sound().stream_alloc(*device,0,2,rate,info,(type == YM2610) ? ym2610_stream_update : ym2610b_stream_update);
	/* setup adpcm buffers */
	pcmbufa  = *device->region();
	pcmsizea = device->region()->bytes();
	name.printf("%s.deltat", device->tag());
	pcmbufb  = (void *)(device->machine().root_device().memregion(name)->base());
	pcmsizeb = device->machine().root_device().memregion(name)->bytes();
	if (pcmbufb == NULL || pcmsizeb == 0)
	{
		pcmbufb = pcmbufa;
		pcmsizeb = pcmsizea;
	}

	/**** initialize YM2610 ****/
	info->chip = ym2610_init(info,device,device->clock(),rate,
		           pcmbufa,pcmsizea,pcmbufb,pcmsizeb,
		           timer_handler,IRQHandler,&psgintf);
	assert_always(info->chip != NULL, "Error creating YM2610 chip");

	device->machine().save().register_postload(save_prepost_delegate(FUNC(ym2610_intf_postload), info));
}

static DEVICE_STOP( ym2610 )
{
	ym2610_state *info = get_safe_token(device);
	ym2610_shutdown(info->chip);
	ay8910_stop_ym(info->psg);
}

static DEVICE_RESET( ym2610 )
{
	ym2610_state *info = get_safe_token(device);
	ym2610_reset_chip(info->chip);
}


READ8_DEVICE_HANDLER( ym2610_r )
{
	ym2610_state *info = get_safe_token(device);
	return ym2610_read(info->chip, offset & 3);
}

WRITE8_DEVICE_HANDLER( ym2610_w )
{
	ym2610_state *info = get_safe_token(device);
	ym2610_write(info->chip, offset & 3, data);
}


READ8_DEVICE_HANDLER( ym2610_status_port_a_r ) { return ym2610_r(device, 0); }
READ8_DEVICE_HANDLER( ym2610_status_port_b_r ) { return ym2610_r(device, 2); }
READ8_DEVICE_HANDLER( ym2610_read_port_r ) { return ym2610_r(device, 1); }

WRITE8_DEVICE_HANDLER( ym2610_control_port_a_w ) { ym2610_w(device, 0, data); }
WRITE8_DEVICE_HANDLER( ym2610_control_port_b_w ) { ym2610_w(device, 2, data); }
WRITE8_DEVICE_HANDLER( ym2610_data_port_a_w ) { ym2610_w(device, 1, data); }
WRITE8_DEVICE_HANDLER( ym2610_data_port_b_w ) { ym2610_w(device, 3, data); }

const device_type YM2610 = &device_creator<ym2610_device>;

ym2610_device::ym2610_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, YM2610, "YM2610", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(ym2610_state));
}
ym2610_device::ym2610_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(ym2610_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ym2610_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ym2610_device::device_start()
{
	DEVICE_START_NAME( ym2610 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ym2610_device::device_reset()
{
	DEVICE_RESET_NAME( ym2610 )(this);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void ym2610_device::device_stop()
{
	DEVICE_STOP_NAME( ym2610 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void ym2610_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


const device_type YM2610B = &device_creator<ym2610b_device>;

ym2610b_device::ym2610b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ym2610_device(mconfig, YM2610B, "YM2610B", tag, owner, clock)
{
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void ym2610b_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


