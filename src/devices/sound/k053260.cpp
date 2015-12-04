// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Alex W. Jackson
/*********************************************************

    Konami 053260 KDSC

    The 053260 is a four voice PCM/ADPCM sound chip that
    also incorporates four 8-bit ports for communication
    between a main CPU and audio CPU. The chip's output
    is compatible with a YM3012 DAC, and it has a digital
    auxiliary input compatible with the output of a YM2151.
    Some games (e.g. Simpsons) only connect one channel of
    the YM2151, but others (e.g. Thunder Cross II) connect
    both channels for stereo mixing.

    The 053260 has a 21-bit address bus and 8-bit data bus
    to ROM, allowing it to access up to 2 megabytes of
    sample data. Sample data can be either signed 8-bit
    PCM or a custom 4-bit ADPCM format. It is possible for
    two 053260 chips to share access to the same ROMs
    (used by Over Drive)

    The 053260 has separate address and data buses to the
    audio CPU controlling it and to the main CPU. Both data
    buses are 8 bit. The audio CPU address bus has 6 lines
    (64 addressable registers, but fewer than 48 are
    actually used) while the main CPU "bus" has only 1 line
    (2 addressable registers). All registers on the audio
    CPU side seem to be either read-only or write-only,
    although some games write 0 to all the registers in a
    loop at startup (including otherwise read-only or
    entirely unused registers).
    On the main CPU side, reads and writes to the same
    address access different communication ports.

    The sound data ROMs of Simpsons and Vendetta have
    "headers" listing all the samples in the ROM, their
    formats ("PCM" or "KADPCM"), start and end addresses.
    The header data doesn't seem to be used by the hardware
    (none of the other games have headers) but provides
    useful information about the chip.

    2004-02-28 (Oliver Achten)
    Fixed ADPCM decoding. Games sound much better now.

    2014-10-06 (Alex W. Jackson)
    Rewrote from scratch in C++; implemented communication
    ports properly; used the actual up counters instead of
    converting to fractional sample position; fixed ADPCM
    decoding bugs; added documentation.


*********************************************************/

#include "emu.h"
#include "k053260.h"

#define LOG 0

#define CLOCKS_PER_SAMPLE 32



// device type definition
const device_type K053260 = &device_creator<k053260_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  k053260_device - constructor
//-------------------------------------------------

k053260_device::k053260_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K053260, "K053260 KDSC", tag, owner, clock, "k053260", __FILE__),
		device_sound_interface(mconfig, *this),
		m_rgnoverride(nullptr),
		m_stream(nullptr),
		m_rom(nullptr),
		m_rom_size(0),
		m_keyon(0),
		m_mode(0)
{
	memset(m_portdata, 0, sizeof(m_portdata));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k053260_device::device_start()
{
	memory_region *ROM = (m_rgnoverride) ? owner()->memregion(m_rgnoverride) : region();
	m_rom = ROM->base();
	m_rom_size = ROM->bytes();

	m_stream = stream_alloc( 0, 2, clock() / CLOCKS_PER_SAMPLE );

	/* register with the save state system */
	save_item(NAME(m_portdata));
	save_item(NAME(m_keyon));
	save_item(NAME(m_mode));

	for (int i = 0; i < 4; i++)
		m_voice[i].voice_start(*this, i);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k053260_device::device_reset()
{
	for (auto & elem : m_voice)
		elem.voice_reset();
}


READ8_MEMBER( k053260_device::main_read )
{
	// sub-to-main ports
	return m_portdata[2 + (offset & 1)];
}


WRITE8_MEMBER( k053260_device::main_write )
{
	// main-to-sub ports
	m_portdata[offset & 1] = data;
}


READ8_MEMBER( k053260_device::read )
{
	offset &= 0x3f;
	UINT8 ret = 0;

	switch (offset)
	{
		case 0x00: // main-to-sub ports
		case 0x01:
			ret = m_portdata[offset];
			break;

		case 0x29: // voice status
			m_stream->update();
			for (int i = 0; i < 4; i++)
				ret |= m_voice[i].playing() << i;
			break;

		case 0x2e: // read ROM
			if (m_mode & 1)
				ret = m_voice[0].read_rom();
			else
				logerror("%s: Attempting to read K053260 ROM without mode bit set\n", machine().describe_context());
			break;

		default:
			logerror("%s: Read from unknown K053260 register %02x\n", machine().describe_context(), offset);
	}
	return ret;
}


WRITE8_MEMBER( k053260_device::write )
{
	offset &= 0x3f;

	m_stream->update();

	// per voice registers
	if ((offset >= 0x08) && (offset <= 0x27))
	{
		m_voice[(offset - 8) / 8].set_register(offset, data);
		return;
	}

	switch (offset)
	{
		// 0x00 and 0x01 are read registers

		case 0x02: // sub-to-main ports
		case 0x03:
			m_portdata[offset] = data;
			break;

		// 0x04 through 0x07 seem to be unused

		case 0x28: // key on/off
		{
			UINT8 rising_edge = data & ~m_keyon;

			for (int i = 0; i < 4; i++)
			{
				if (rising_edge & (1 << i))
					m_voice[i].key_on();
				else if (!(data & (1 << i)))
					m_voice[i].key_off();
			}
			m_keyon = data;
			break;
		}

		// 0x29 is a read register

		case 0x2a: // loop and pcm/adpcm select
			for (auto & elem : m_voice)
			{
				elem.set_loop_kadpcm(data);
				data >>= 1;
			}
			break;

		// 0x2b seems to be unused

		case 0x2c: // pan, voices 0 and 1
			m_voice[0].set_pan(data);
			m_voice[1].set_pan(data >> 3);
			break;

		case 0x2d: // pan, voices 2 and 3
			m_voice[2].set_pan(data);
			m_voice[3].set_pan(data >> 3);
			break;

		// 0x2e is a read register

		case 0x2f: // control
			m_mode = data;
			// bit 0 = enable ROM read from register 0x2e
			// bit 1 = enable sound output
			// bit 2 = enable aux input?
			//   (set by all games except Golfing Greats and Rollergames, both of which
			//    don't have a YM2151. Over Drive only sets it on one of the two chips)
			// bit 3 = aux input or ROM sharing related?
			//   (only set by Over Drive, and only on the same chip that bit 2 is set on)
			break;

		default:
			logerror("%s: Write to unknown K053260 register %02x (data = %02x)\n",
					machine().describe_context(), offset, data);
	}
}


INLINE int limit( int val, int max, int min )
{
	if ( val > max )
		val = max;
	else if ( val < min )
		val = min;

	return val;
}

#define MAXOUT 0x7fff
#define MINOUT -0x8000

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void k053260_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	if (m_mode & 2)
	{
		for ( int j = 0; j < samples; j++ )
		{
			stream_sample_t buffer[2] = {0, 0};

			for (auto & voice : m_voice)
			{
				
				if (voice.playing())
					voice.play(buffer);
			}

			outputs[0][j] = limit( buffer[0] >> 1, MAXOUT, MINOUT );
			outputs[1][j] = limit( buffer[1] >> 1, MAXOUT, MINOUT );
		}
	}
	else
	{
		memset( outputs[0], 0, samples * sizeof(*outputs[0]));
		memset( outputs[1], 0, samples * sizeof(*outputs[1]));
	}
}


//**************************************************************************
//  KDSC_Voice - one of the four voices
//**************************************************************************

void k053260_device::KDSC_Voice::voice_start(k053260_device &device, int index)
{
	m_device = &device;

	voice_reset();

	device.save_item(NAME(m_position), index);
	device.save_item(NAME(m_pan_volume), index);
	device.save_item(NAME(m_counter), index);
	device.save_item(NAME(m_output), index);
	device.save_item(NAME(m_playing), index);
	device.save_item(NAME(m_start), index);
	device.save_item(NAME(m_length), index);
	device.save_item(NAME(m_pitch), index);
	device.save_item(NAME(m_volume), index);
	device.save_item(NAME(m_pan), index);
	device.save_item(NAME(m_loop), index);
	device.save_item(NAME(m_kadpcm), index);
}

void k053260_device::KDSC_Voice::voice_reset()
{
	m_position = 0;
	m_counter = 0;
	m_output = 0;
	m_playing = false;
	m_start = 0;
	m_length = 0;
	m_pitch = 0;
	m_volume = 0;
	m_pan = 0;
	m_loop = false;
	m_kadpcm = false;
	update_pan_volume();
}

void k053260_device::KDSC_Voice::set_register(offs_t offset, UINT8 data)
{
	switch (offset & 0x7)
	{
		case 0: // pitch, lower 8 bits
			m_pitch = (m_pitch & 0x0f00) | data;
			break;
		case 1: // pitch, upper 4 bits
			m_pitch = (m_pitch & 0x00ff) | ((data << 8) & 0x0f00);
			break;
		case 2: // length, lower 8 bits
			m_length = (m_length & 0xff00) | data;
			break;
		case 3: // length, upper 8 bits
			m_length = (m_length & 0x00ff) | (data << 8);
			break;
		case 4: // start, lower 8 bits
			m_start = (m_start & 0x1fff00) | data;
			break;
		case 5: // start, middle 8 bits
			m_start = (m_start & 0x1f00ff) | (data << 8);
			break;
		case 6: // start, upper 5 bits
			m_start = (m_start & 0x00ffff) | ((data << 16) & 0x1f0000);
			break;
		case 7: // volume, 7 bits
			m_volume = data & 0x7f;
			update_pan_volume();
	}
}

void k053260_device::KDSC_Voice::set_loop_kadpcm(UINT8 data)
{
	m_loop = BIT(data, 0);
	m_kadpcm = BIT(data, 4);
}

void k053260_device::KDSC_Voice::set_pan(UINT8 data)
{
	m_pan = data & 0x7;
	update_pan_volume();
}

void k053260_device::KDSC_Voice::update_pan_volume()
{
	m_pan_volume[0] = m_volume * (8 - m_pan);
	m_pan_volume[1] = m_volume * m_pan;
}

void k053260_device::KDSC_Voice::key_on()
{
	if (m_start >= m_device->m_rom_size)
		m_device->logerror("K053260: Attempting to start playing past the end of the ROM ( start = %06x, length = %06x )\n", m_start, m_length);

	else if (m_start + m_length >= m_device->m_rom_size)
		m_device->logerror("K053260: Attempting to play past the end of the ROM ( start = %06x, length = %06x )\n",
					m_start, m_length);

	else
	{
		m_position = m_kadpcm ? 1 : 0; // for kadpcm low bit is nybble offset, so must start at 1 due to preincrement
		m_counter = 0x1000 - CLOCKS_PER_SAMPLE; // force update on next sound_stream_update
		m_output = 0;
		m_playing = true;
		if (LOG) m_device->logerror("K053260: start = %06x, length = %06x, pitch = %04x, vol = %02x, loop = %s, %s\n",
						m_start, m_length, m_pitch, m_volume, m_loop ? "yes" : "no", m_kadpcm ? "KADPCM" : "PCM" );
	}
}

void k053260_device::KDSC_Voice::key_off()
{
	m_position = 0;
	m_output = 0;
	m_playing = false;
}

void k053260_device::KDSC_Voice::play(stream_sample_t *outputs)
{
	m_counter += CLOCKS_PER_SAMPLE;

	while (m_counter >= 0x1000)
	{
		m_counter = m_counter - 0x1000 + m_pitch;

		UINT32 bytepos = ++m_position >> ( m_kadpcm ? 1 : 0 );
		/*
		Yes, _pre_increment. Playback must start 1 byte position after the
		start address written to the register, or else ADPCM sounds will
		have DC offsets (e.g. TMNT2 theme song) or will overflow and be
		distorted (e.g. various Vendetta sound effects)
		The "headers" in the Simpsons and Vendetta sound ROMs provide
		further evidence of this quirk (the start addresses listed in the
		ROM header are all 1 greater than the addresses the CPU writes
		into the register)
		*/
		if (bytepos > m_length)
		{
			if (m_loop)
			{
				m_position = m_output = bytepos = 0;
			}
			else
			{
				m_playing = false;
				return;
			}
		}

		UINT8 romdata = m_device->m_rom[m_start + bytepos];

		if (m_kadpcm)
		{
			if (m_position & 1) romdata >>= 4; // decode low nybble, then high nybble
			static const INT8 kadpcm_table[] = {0,1,2,4,8,16,32,64,-128,-64,-32,-16,-8,-4,-2,-1};
			m_output += kadpcm_table[romdata & 0xf];
		}
		else
		{
			m_output = romdata;
		}
	}

	outputs[0] += m_output * m_pan_volume[0];
	outputs[1] += m_output * m_pan_volume[1];
}

UINT8 k053260_device::KDSC_Voice::read_rom()
{
	UINT32 offs = m_start + m_position;

	m_position = (m_position + 1) & 0xffff;

	if (offs >= m_device->m_rom_size)
	{
		m_device->logerror("%s: K053260: Attempting to read past the end of the ROM (offs = %06x, size = %06x)\n",
					m_device->machine().describe_context(), offs, m_device->m_rom_size);
		return 0;
	}

	return m_device->m_rom[offs];
}
