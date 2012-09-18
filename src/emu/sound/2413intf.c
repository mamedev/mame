/****************************************************************

    MAME / MESS functions

****************************************************************/

#include "emu.h"
#include "ym2413.h"
#include "2413intf.h"


/* for stream system */
struct ym2413_state
{
	sound_stream *	stream;
	void *			chip;
};


INLINE ym2413_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == YM2413);
	return (ym2413_state *)downcast<ym2413_device *>(device)->token();
}


#ifdef UNUSED_FUNCTION
void YM2413DAC_update(int chip,stream_sample_t **inputs, stream_sample_t **_buffer,int length)
{
    INT16 *buffer = _buffer[0];
    static int out = 0;

    if ( ym2413[chip].reg[0x0F] & 0x01 )
    {
        out = ((ym2413[chip].reg[0x10] & 0xF0) << 7);
    }
    while (length--) *(buffer++) = out;
}
#endif

static STREAM_UPDATE( ym2413_stream_update )
{
	ym2413_state *info = (ym2413_state *)param;
	ym2413_update_one(info->chip, outputs, samples);
}

static void _stream_update(void *param, int interval)
{
	ym2413_state *info = (ym2413_state *)param;
	info->stream->update();
}

static DEVICE_START( ym2413 )
{
	ym2413_state *info = get_safe_token(device);
	int rate = device->clock()/72;

	/* emulator create */
	info->chip = ym2413_init(device, device->clock(), rate);
	assert_always(info->chip != NULL, "Error creating YM2413 chip");

	/* stream system initialize */
	info->stream = device->machine().sound().stream_alloc(*device,0,2,rate,info,ym2413_stream_update);

	ym2413_set_update_handler(info->chip, _stream_update, info);




#if 0
	int i, tst;
	char name[40];

	num = intf->num;

	tst = YM3812_sh_start (msound);
	if (tst)
		return 1;

	for (i=0;i<num;i++)
	{
		ym2413_reset (i);

		ym2413[i].DAC_stream = device->machine().sound().stream_alloc(*device, 0, 1, device->clock()/72, i, YM2413DAC_update);

		if (ym2413[i].DAC_stream == -1)
			return 1;
	}
	return 0;
#endif

}

static DEVICE_STOP( ym2413 )
{
	ym2413_state *info = get_safe_token(device);
	ym2413_shutdown(info->chip);
}

static DEVICE_RESET( ym2413 )
{
	ym2413_state *info = get_safe_token(device);
	ym2413_reset_chip(info->chip);
}


WRITE8_DEVICE_HANDLER( ym2413_w )
{
	ym2413_state *info = get_safe_token(device);
	ym2413_write(info->chip, offset & 1, data);
}

WRITE8_DEVICE_HANDLER( ym2413_register_port_w ) { ym2413_w(device, space, 0, data); }
WRITE8_DEVICE_HANDLER( ym2413_data_port_w ) { ym2413_w(device, space, 1, data); }

const device_type YM2413 = &device_creator<ym2413_device>;

ym2413_device::ym2413_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, YM2413, "YM2413", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(ym2413_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ym2413_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ym2413_device::device_start()
{
	DEVICE_START_NAME( ym2413 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ym2413_device::device_reset()
{
	DEVICE_RESET_NAME( ym2413 )(this);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void ym2413_device::device_stop()
{
	DEVICE_STOP_NAME( ym2413 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void ym2413_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


