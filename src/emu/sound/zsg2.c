/*
    ZOOM ZSG-2 custom wavetable synthesizer

    Written by Olivier Galibert
    MAME conversion by R. Belmont

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

    ---------------------------------------------------------

    Additional notes on the sample format, reverse-engineered
    by Olivier Galibert and David Haywood:

    The zoom sample rom is decomposed in 0x40000 bytes pages.  Each page
    starts by a header and is followed by compressed samples.

    The header is a vector of 16 bytes structures composed of 4 32bits
    little-endian values representing:
    - sample start position in bytes, always a multiple of 4
    - sample end position in bytes, minus 4, always...
    - loop position in bytes, always....
    - flags, probably

    It is interesting to note that this header is *not* parsed by the
    ZSG.  The main program reads the rom through appropriate ZSG
    commands, and use the results in subsequent register setups.  It's
    not even obvious that the ZSG cares about the pages, it may just
    see the address space as linear.  In the same line, the
    interpretation of the flags is obviously dependant on the main
    program, not the ZSG, but some of the bits are directly copied to
    some of the registers.

    The samples are compressed with a 2:1 ratio.  Each bloc of 4-bytes
    becomes 4 16-bits samples.  Reading the 4 bytes as a *little-endian*
    32bits values, the structure is:

    1444 4444 1333 3333 1222 2222 ssss1111

    's' is a 4-bit scale value.  '1', '2', '3', '4' are signed 7-bits
    values corresponding to the 4 samples.  To compute the final 16bits
    value just shift left by (9-s).  Yes, that simple.

*/

#include "emu.h"
#include "zsg2.h"

// 16 registers per channel, 48 channels
typedef struct _zchan zchan;
struct _zchan
{
	UINT16 v[16];
};

typedef struct _zsg2_state zsg2_state;
struct _zsg2_state
{
	zchan zc[48];
	UINT16 act[3];
	UINT16 alow, ahigh;
	UINT8 *bank_samples;

	int sample_rate;
	sound_stream *stream;
};

INLINE zsg2_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == ZSG2);
	return (zsg2_state *)downcast<zsg2_device *>(device)->token();
}

static STREAM_UPDATE( update_stereo )
{
//  zsg2_state *info = (zsg2_state *)param;
	stream_sample_t *dest1 = outputs[0];
	stream_sample_t *dest2 = outputs[1];

	memset(dest1, 0, sizeof(stream_sample_t) * samples);
	memset(dest2, 0, sizeof(stream_sample_t) * samples);
}

static void chan_w(zsg2_state *info, int chan, int reg, UINT16 data)
{
  info->zc[chan].v[reg] = data;
  //  log_event("ZOOMCHAN", "chan %02x reg %x = %04x", chan, reg, data);
}

static UINT16 chan_r(zsg2_state *info, int chan, int reg)
{
  //  log_event("ZOOMCHAN", "chan %02x read reg %x: %04x", chan, reg, zc[chan].v[reg]);
  return info->zc[chan].v[reg];
}

static void check_channel(zsg2_state *info, int chan)
{
  //  log_event("ZOOM", "chan %02x e=%04x f=%04x", chan, zc[chan].v[14], zc[chan].v[15]);
}

static void keyon(zsg2_state *info, int chan)
{
#if 0
  log_event("ZOOM", "keyon %02x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x",
	    chan,
	    info->zc[chan].v[0x0], info->zc[chan].v[0x1], info->zc[chan].v[0x2], info->zc[chan].v[0x3],
	    info->zc[chan].v[0x4], info->zc[chan].v[0x5], info->zc[chan].v[0x6], info->zc[chan].v[0x7],
	    info->zc[chan].v[0x8], info->zc[chan].v[0x9], info->zc[chan].v[0xa], info->zc[chan].v[0xb],
	    info->zc[chan].v[0xc], info->zc[chan].v[0xd], info->zc[chan].v[0xe], info->zc[chan].v[0xf]);
#endif
}

static void control_w(zsg2_state *info, int reg, UINT16 data)
{
	switch(reg)
	{
		case 0x00: case 0x02: case 0x04:
		{
			int base = (reg & 6) << 3;
			int i;
			for(i=0; i<16; i++)
				if(data & (1<<i))
					keyon(info, base+i);
			break;
		}

		case 0x08: case 0x0a: case 0x0c:
		{
			int base = (reg & 6) << 3;
			int i;
			for(i=0; i<16; i++)
				if(data & (1<<i))
					check_channel(info, base+i);
			break;
		}

		case 0x30:
			break;

		case 0x38:
			info->alow = data;
			break;

		case 0x3a:
			info->ahigh = data;
			break;

		default:
		//      log_event("ZOOMCTRL", "%02x = %04x", reg, data);
			break;
	}
}

static UINT16 control_r(zsg2_state *info, int reg)
{
	switch(reg)
	{
		case 0x28:
			return 0xff00;

		case 0x3c: case 0x3e:
		{
			UINT32 adr = (info->ahigh << 16) | info->alow;
			UINT32 val = *(unsigned int *)(info->bank_samples+adr);
//          log_event("ZOOMCTRL", "rom read.%c %06x = %08x", reg == 0x3e ? 'h' : 'l', adr, val);
			return (reg == 0x3e) ? (val >> 16) : val;
		}
	}

//  log_event("ZOOMCTRL", "read %02x", reg);

	return 0xffff;
}

WRITE16_DEVICE_HANDLER( zsg2_w )
{
	zsg2_state *info = get_safe_token(device);
	int adr = offset * 2;

	assert(mem_mask == 0xffff);	// we only support full 16-bit accesses

	info->stream->update();

	if (adr < 0x600)
	{
		int chan = adr >> 5;
		int reg = (adr >> 1) & 15;

		chan_w(info, chan, reg, data);
	}
	else
	{
		control_w(info, adr - 0x600, data);
	}
}

READ16_DEVICE_HANDLER( zsg2_r )
{
	zsg2_state *info = get_safe_token(device);
	int adr = offset * 2;

	assert(mem_mask == 0xffff);	// we only support full 16-bit accesses

	if (adr < 0x600)
	{
		int chan = adr >> 5;
		int reg = (adr >> 1) & 15;
		return chan_r(info, chan, reg);
	}
	else
	{
		return control_r(info, adr - 0x600);
	}

	return 0;
}

static DEVICE_START( zsg2 )
{
	const zsg2_interface *intf = (const zsg2_interface *)device->static_config();
	zsg2_state *info = get_safe_token(device);

	info->sample_rate = device->clock();

	memset(&info->zc, 0, sizeof(info->zc));
	memset(&info->act, 0, sizeof(info->act));

	info->stream = device->machine().sound().stream_alloc(*device, 0, 2, info->sample_rate, info, update_stereo);

	info->bank_samples = device->machine().root_device().memregion(intf->samplergn)->base();
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( zsg2 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(zsg2_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( zsg2 );		break;
		case DEVINFO_FCT_STOP:							/* nothing */								break;
		case DEVINFO_FCT_RESET:							/* nothing */								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:						strcpy(info->s, "ZSG-2");					break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Zoom custom");				break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


const device_type ZSG2 = &device_creator<zsg2_device>;

zsg2_device::zsg2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ZSG2, "ZSG-2", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(zsg2_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void zsg2_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void zsg2_device::device_start()
{
	DEVICE_START_NAME( zsg2 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void zsg2_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


