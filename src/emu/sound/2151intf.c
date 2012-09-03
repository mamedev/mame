/***************************************************************************

  2151intf.c

  Support interface YM2151(OPM)

***************************************************************************/

#include "emu.h"
#include "fm.h"
#include "2151intf.h"
#include "ym2151.h"


typedef struct _ym2151_state ym2151_state;
struct _ym2151_state
{
	sound_stream *			stream;
	emu_timer *				timer[2];
	void *					chip;
	UINT8					lastreg;
	devcb_resolved_write_line irqhandler;
	devcb_resolved_write8 portwritehandler;
};


INLINE ym2151_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == YM2151);
	return (ym2151_state *)downcast<ym2151_device *>(device)->token();
}


static STREAM_UPDATE( ym2151_update )
{
	ym2151_state *info = (ym2151_state *)param;
	ym2151_update_one(info->chip, outputs, samples);
}

static void ym2151_irq_frontend(device_t *device, int irq)
{
	ym2151_state *info = get_safe_token(device);
	info->irqhandler(irq);
}

static void ym2151_port_write_frontend(device_t *device, offs_t offset, UINT8 data)
{
	ym2151_state *info = get_safe_token(device);
	info->portwritehandler(offset, data);
}

static DEVICE_START( ym2151 )
{
	static const ym2151_interface dummy = { DEVCB_NULL };
	ym2151_state *info = get_safe_token(device);
	int rate;

	const ym2151_interface *intf = device->static_config() ? (const ym2151_interface *)device->static_config() : &dummy;
	info->irqhandler.resolve(intf->irqhandler, *device);
	info->portwritehandler.resolve(intf->portwritehandler, *device);

	rate = device->clock()/64;

	/* stream setup */
	info->stream = device->machine().sound().stream_alloc(*device,0,2,rate,info,ym2151_update);

	info->chip = ym2151_init(device,device->clock(),rate);
	assert_always(info->chip != NULL, "Error creating YM2151 chip");

	ym2151_set_irq_handler(info->chip,ym2151_irq_frontend);
	ym2151_set_port_write_handler(info->chip,ym2151_port_write_frontend);
}


static DEVICE_STOP( ym2151 )
{
	ym2151_state *info = get_safe_token(device);
	ym2151_shutdown(info->chip);
}

static DEVICE_RESET( ym2151 )
{
	ym2151_state *info = get_safe_token(device);
	ym2151_reset_chip(info->chip);
}


READ8_DEVICE_HANDLER( ym2151_r )
{
	ym2151_state *token = get_safe_token(device);

	if (offset & 1)
	{
		token->stream->update();
		return ym2151_read_status(token->chip);
	}
	else
		return 0xff;	/* confirmed on a real YM2151 */
}

WRITE8_DEVICE_HANDLER( ym2151_w )
{
	ym2151_state *token = get_safe_token(device);

	if (offset & 1)
	{
		token->stream->update();
		ym2151_write_reg(token->chip, token->lastreg, data);
	}
	else
		token->lastreg = data;
}


READ8_DEVICE_HANDLER( ym2151_status_port_r ) { return ym2151_r(device, 1); }

WRITE8_DEVICE_HANDLER( ym2151_register_port_w ) { ym2151_w(device, 0, data); }
WRITE8_DEVICE_HANDLER( ym2151_data_port_w ) { ym2151_w(device, 1, data); }



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( ym2151 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(ym2151_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( ym2151 );		break;
		case DEVINFO_FCT_STOP:							info->stop = DEVICE_STOP_NAME( ym2151 );		break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME( ym2151 );		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "YM2151");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Yamaha FM");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


const device_type YM2151 = &device_creator<ym2151_device>;

ym2151_device::ym2151_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, YM2151, "YM2151", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(ym2151_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ym2151_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ym2151_device::device_start()
{
	DEVICE_START_NAME( ym2151 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ym2151_device::device_reset()
{
	DEVICE_RESET_NAME( ym2151 )(this);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void ym2151_device::device_stop()
{
	DEVICE_STOP_NAME( ym2151 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void ym2151_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


