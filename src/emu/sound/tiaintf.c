#include "emu.h"
#include "tiaintf.h"
#include "tiasound.h"

struct tia_state
{
	sound_stream * channel;
	void *chip;
};

INLINE tia_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == TIA);
	return (tia_state *)downcast<tia_device *>(device)->token();
}


static STREAM_UPDATE( tia_update )
{
	tia_state *info = (tia_state *)param;
	tia_process(info->chip, outputs[0], samples);
}


static DEVICE_START( tia )
{
	tia_state *info = get_safe_token(device);

	info->channel = device->machine().sound().stream_alloc(*device, 0, 1, device->clock(), info, tia_update);

	info->chip = tia_sound_init(device->clock(), device->clock(), 16);
	assert_always(info->chip != NULL, "Error creating TIA chip");
}

static DEVICE_STOP( tia )
{
	tia_state *info = get_safe_token(device);
	tia_sound_free(info->chip);
}

WRITE8_DEVICE_HANDLER( tia_sound_w )
{
	tia_state *info = get_safe_token(device);
	info->channel->update();
	tia_write(info->chip, offset, data);
}

const device_type TIA = &device_creator<tia_device>;

tia_device::tia_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TIA, "TIA", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(tia_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void tia_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tia_device::device_start()
{
	DEVICE_START_NAME( tia )(this);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void tia_device::device_stop()
{
	DEVICE_STOP_NAME( tia )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void tia_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


