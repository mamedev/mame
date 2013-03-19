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


// device type definition
const device_type ZSG2 = &device_creator<zsg2_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  zsg2_device - constructor
//-------------------------------------------------

zsg2_device::zsg2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ZSG2, "ZSG-2", tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_alow(0),
		m_ahigh(0),
		m_bank_samples(NULL),
		m_sample_rate(0),
		m_stream(NULL)
{
	memset(m_act, 0, sizeof(UINT16)*3);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void zsg2_device::device_start()
{
	const zsg2_interface *intf = (const zsg2_interface *)static_config();

	m_sample_rate = clock();

	memset(&m_zc, 0, sizeof(m_zc));
	memset(&m_act, 0, sizeof(m_act));

	m_stream = stream_alloc(0, 2, m_sample_rate);

	m_bank_samples = memregion(intf->samplergn)->base();
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void zsg2_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *dest1 = outputs[0];
	stream_sample_t *dest2 = outputs[1];

	memset(dest1, 0, sizeof(stream_sample_t) * samples);
	memset(dest2, 0, sizeof(stream_sample_t) * samples);
}



void zsg2_device::chan_w(int chan, int reg, UINT16 data)
{
	m_zc[chan].v[reg] = data;
	//  log_event("ZOOMCHAN", "chan %02x reg %x = %04x", chan, reg, data);
}

UINT16 zsg2_device::chan_r(int chan, int reg)
{
	//  log_event("ZOOMCHAN", "chan %02x read reg %x: %04x", chan, reg, zc[chan].v[reg]);
	return m_zc[chan].v[reg];
}

void zsg2_device::check_channel(int chan)
{
	//  log_event("ZOOM", "chan %02x e=%04x f=%04x", chan, zc[chan].v[14], zc[chan].v[15]);
}

void zsg2_device::keyon(int chan)
{
#if 0
	log_event("ZOOM", "keyon %02x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x",
		chan,
		m_zc[chan].v[0x0], m_zc[chan].v[0x1], m_zc[chan].v[0x2], m_zc[chan].v[0x3],
		m_zc[chan].v[0x4], m_zc[chan].v[0x5], m_zc[chan].v[0x6], m_zc[chan].v[0x7],
		m_zc[chan].v[0x8], m_zc[chan].v[0x9], m_zc[chan].v[0xa], m_zc[chan].v[0xb],
		m_zc[chan].v[0xc], m_zc[chan].v[0xd], m_zc[chan].v[0xe], m_zc[chan].v[0xf]);
#endif
}

void zsg2_device::control_w(int reg, UINT16 data)
{
	switch(reg)
	{
		case 0x00: case 0x02: case 0x04:
		{
			int base = (reg & 6) << 3;
			int i;
			for(i=0; i<16; i++)
				if(data & (1<<i))
					keyon(base+i);
			break;
		}

		case 0x08: case 0x0a: case 0x0c:
		{
			int base = (reg & 6) << 3;
			int i;
			for(i=0; i<16; i++)
				if(data & (1<<i))
					check_channel(base+i);
			break;
		}

		case 0x30:
			break;

		case 0x38:
			m_alow = data;
			break;

		case 0x3a:
			m_ahigh = data;
			break;

		default:
		//      log_event("ZOOMCTRL", "%02x = %04x", reg, data);
			break;
	}
}

UINT16 zsg2_device::control_r(int reg)
{
	switch(reg)
	{
		case 0x28:
			return 0xff00;

		case 0x3c: case 0x3e:
		{
			UINT32 adr = (m_ahigh << 16) | m_alow;
			UINT32 val = *(unsigned int *)(m_bank_samples+adr);
//          log_event("ZOOMCTRL", "rom read.%c %06x = %08x", reg == 0x3e ? 'h' : 'l', adr, val);
			return (reg == 0x3e) ? (val >> 16) : val;
		}
	}

//  log_event("ZOOMCTRL", "read %02x", reg);

	return 0xffff;
}


WRITE16_MEMBER( zsg2_device::zsg2_w )
{
	int adr = offset * 2;

	assert(mem_mask == 0xffff); // we only support full 16-bit accesses

	m_stream->update();

	if (adr < 0x600)
	{
		int chan = adr >> 5;
		int reg = (adr >> 1) & 15;

		chan_w(chan, reg, data);
	}
	else
	{
		control_w(adr - 0x600, data);
	}
}


READ16_MEMBER( zsg2_device::zsg2_r )
{
	int adr = offset * 2;

	assert(mem_mask == 0xffff); // we only support full 16-bit accesses

	if (adr < 0x600)
	{
		int chan = adr >> 5;
		int reg = (adr >> 1) & 15;
		return chan_r(chan, reg);
	}
	else
	{
		return control_r(adr - 0x600);
	}

	return 0;
}
