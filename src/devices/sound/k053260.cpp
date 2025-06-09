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

#include <algorithm>

#define LOG 0

static constexpr int CLOCKS_PER_SAMPLE = 64;


// device type definition
DEFINE_DEVICE_TYPE(K053260, k053260_device, "k053260", "K053260 KDSC")


// Pan multipliers.  Set according to integer angles in degrees, amusingly.
// Exact precision hard to know, the floating point-ish output format makes
// comparisons iffy.  So we used a 1.16 format.
const int k053260_device::pan_mul[8][2] =
{
	{     0,     0 }, // No sound for pan 0
	{ 65536,     0 }, //  0 degrees
	{ 59870, 26656 }, // 24 degrees
	{ 53684, 37950 }, // 35 degrees
	{ 46341, 46341 }, // 45 degrees
	{ 37950, 53684 }, // 55 degrees
	{ 26656, 59870 }, // 66 degrees
	{     0, 65536 }  // 90 degrees
};


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  k053260_device - constructor
//-------------------------------------------------

k053260_device::k053260_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, K053260, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_sh1_cb(*this)
	, m_sh2_cb(*this)
	, m_stream(nullptr)
	, m_timer(nullptr)
	, m_keyon(0)
	, m_mode(0)
	, m_timer_state(0)
	, m_voice{ { *this }, { *this }, { *this }, { *this } }
{
	std::fill(std::begin(m_portdata), std::end(m_portdata), 0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k053260_device::device_start()
{
	m_stream = stream_alloc(0, 2, clock() / CLOCKS_PER_SAMPLE);

	/* register with the save state system */
	save_item(NAME(m_portdata));
	save_item(NAME(m_keyon));
	save_item(NAME(m_mode));
	save_item(NAME(m_timer_state));

	for (int i = 0; i < 4; i++)
		m_voice[i].voice_start(i);

	m_timer = timer_alloc(FUNC(k053260_device::update_state_outputs), this);
}


//-------------------------------------------------
//  device_clock_changed
//-------------------------------------------------

void k053260_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / CLOCKS_PER_SAMPLE);
	attotime period = attotime::from_ticks(16, clock());
	m_timer->adjust(period, 0, period);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k053260_device::device_reset()
{
	attotime period = attotime::from_ticks(16, clock());
	m_timer->adjust(period, 0, period);

	for (auto & voice : m_voice)
		voice.voice_reset();
}


//-------------------------------------------------
//  rom_bank_pre_change - refresh the stream if the
//  ROM banking changes
//-------------------------------------------------

void k053260_device::rom_bank_pre_change()
{
	m_stream->update();
}


TIMER_CALLBACK_MEMBER(k053260_device::update_state_outputs)
{
	switch (m_timer_state)
	{
		case 0: m_sh1_cb(ASSERT_LINE); break;
		case 1: m_sh1_cb(CLEAR_LINE); break;
		case 2: m_sh2_cb(ASSERT_LINE); break;
		case 3: m_sh2_cb(CLEAR_LINE); break;
	}
	m_timer_state = (m_timer_state+1) & 3;
}

u8 k053260_device::main_read(offs_t offset)
{
	// sub-to-main ports
	return m_portdata[2 + (offset & 1)];
}


void k053260_device::main_write(offs_t offset, u8 data)
{
	// main-to-sub ports
	m_portdata[offset & 1] = data;
}


u8 k053260_device::read(offs_t offset)
{
	offset &= 0x3f;
	u8 ret = 0;

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
				ret = m_voice[0].read_rom(!(machine().side_effects_disabled()));
			else
				logerror("%s: Attempting to read K053260 ROM without mode bit set\n", machine().describe_context());
			break;

		default:
			logerror("%s: Read from unknown K053260 register %02x\n", machine().describe_context(), offset);
	}
	return ret;
}


void k053260_device::write(offs_t offset, u8 data)
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

		case 0x28: // key on/off and reverse
		{
			u8 rising_edge = data & ~m_keyon;

			for (int i = 0; i < 4; i++)
			{
				m_voice[i].set_reverse(BIT(data, i | 4));

				if (BIT(rising_edge, i))
					m_voice[i].key_on();
				else if (!BIT(data, i))
					m_voice[i].key_off();
			}
			m_keyon = data;
			break;
		}

		// 0x29 is a read register

		case 0x2a: // loop and pcm/adpcm select
			for (int i = 0; i < 4; i++)
			{
				m_voice[i].set_loop(BIT(data, i));
				m_voice[i].set_kadpcm(BIT(data, i | 4));
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

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void k053260_device::sound_stream_update(sound_stream &stream)
{
	if (m_mode & 2)
	{
		for (int j = 0; j < stream.samples(); j++)
		{
			s32 buffer[2] = {0, 0};

			for (auto & voice : m_voice)
			{
				if (voice.playing())
					voice.play(buffer);
			}

			stream.put_int_clamp(0, j, buffer[0], 32768);
			stream.put_int_clamp(1, j, buffer[1], 32768);
		}
	}
}


//**************************************************************************
//  KDSC_Voice - one of the four voices
//**************************************************************************

void k053260_device::KDSC_Voice::voice_start(int index)
{
	voice_reset();

	m_device.save_item(NAME(m_position), index);
	m_device.save_item(NAME(m_pan_volume), index);
	m_device.save_item(NAME(m_counter), index);
	m_device.save_item(NAME(m_output), index);
	m_device.save_item(NAME(m_playing), index);
	m_device.save_item(NAME(m_start), index);
	m_device.save_item(NAME(m_length), index);
	m_device.save_item(NAME(m_pitch), index);
	m_device.save_item(NAME(m_volume), index);
	m_device.save_item(NAME(m_pan), index);
	m_device.save_item(NAME(m_loop), index);
	m_device.save_item(NAME(m_kadpcm), index);
	m_device.save_item(NAME(m_reverse), index);
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
	m_reverse = false;
	update_pan_volume();
}

void k053260_device::KDSC_Voice::set_register(offs_t offset, u8 data)
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

void k053260_device::KDSC_Voice::set_loop(int state)
{
	m_loop = bool(state);
}

void k053260_device::KDSC_Voice::set_kadpcm(int state)
{
	m_kadpcm = bool(state);
}

void k053260_device::KDSC_Voice::set_reverse(int state)
{
	m_reverse = bool(state);
}

void k053260_device::KDSC_Voice::set_pan(u8 data)
{
	m_pan = data & 0x7;
	update_pan_volume();
}

void k053260_device::KDSC_Voice::update_pan_volume()
{
	m_pan_volume[0] = m_volume * pan_mul[m_pan][0];
	m_pan_volume[1] = m_volume * pan_mul[m_pan][1];
}

void k053260_device::KDSC_Voice::key_on()
{
	m_position = m_kadpcm ? 1 : 0; // for kadpcm low bit is nybble offset, so must start at 1 due to preincrement
	m_counter = 0x1000 - CLOCKS_PER_SAMPLE; // force update on next sound_stream_update
	m_output = 0;
	m_playing = true;

	if (LOG)
	{
		m_device.logerror("K053260: start = %06x, length = %06x, pitch = %04x, vol = %02x:%x, loop = %s, reverse = %s, %s\n",
				m_start, m_length, m_pitch, m_volume, m_pan,
				m_loop ? "yes" : "no",
				m_reverse ? "yes" : "no",
				m_kadpcm ? "KADPCM" : "PCM");
	}
}

void k053260_device::KDSC_Voice::key_off()
{
	m_position = 0;
	m_output = 0;
	m_playing = false;
}

void k053260_device::KDSC_Voice::play(s32 *outputs)
{
	m_counter += CLOCKS_PER_SAMPLE;

	while (m_counter >= 0x1000)
	{
		m_counter = m_counter - 0x1000 + m_pitch;

		u32 bytepos = ++m_position >> (m_kadpcm ? 1 : 0);
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

		u8 romdata = m_device.read_byte(m_start + (m_reverse ? -bytepos : +bytepos));

		if (m_kadpcm)
		{
			if (m_position & 1) romdata >>= 4; // decode low nybble, then high nybble
			static const s8 kadpcm_table[] = {0,1,2,4,8,16,32,64,-128,-64,-32,-16,-8,-4,-2,-1};
			m_output += kadpcm_table[romdata & 0xf];
		}
		else
		{
			m_output = romdata;
		}
	}

	outputs[0] += (m_output * m_pan_volume[0]) >> 15;
	outputs[1] += (m_output * m_pan_volume[1]) >> 15;
}

u8 k053260_device::KDSC_Voice::read_rom(bool side_effects)
{
	u32 offs = m_start + m_position;

	if (side_effects)
		m_position = (m_position + 1) & 0xffff;

	return m_device.read_byte(offs);
}
