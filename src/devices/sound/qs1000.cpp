// license:BSD-3-Clause
// copyright-holders:R. Belmont, Philip Bennett
/***************************************************************************

    qs1000.cpp

    QS1000 device emulator.
    Emulation by R. Belmont and Philip Bennett

****************************************************************************

    The QS1000 is a 32-voice wavetable synthesizer, believed to be based on
    the OPTi 82C941. It contains an 8051 core, 256b of RAM and an (undumped)
    internal program ROM. The internal ROM can be bypassed in favour of an
    external ROM. Commands are issued to the chip via the 8051 serial port.

    The QS1000 can access 24Mb of sample ROM. To reduce demand on the CPU,
    instrument parameters such as playback rate, envelope and filter values
    are encoded in ROM and directly accessed by the wavetable engine.
    There are table entries for every note of every instrument.

    Registers
    =========

    [200] = Command. Page 0: 1 = begin a parameter group.
            Page 3: 0 = stop the selected voice, 2 = key off (release)
    [201] = Address byte 0 (MSB)
    [202] = Address byte 1
    [203] = Address byte 2 (LSB)
    [204] = Pitch multiplier offset (bit 7: up, bits 0-6: magnitude)
    [205] = Unknown; the firmware always writes 0
    [206] = Right volume
    [207] = Left volume
    [208] = Volume (channel volume x expression through a lookup table)
    [209] = Unknown; melodic key-on writes 40, the modulation handler
            and sound effects write 00
    [20a] = Unknown; always ff
    [20b] = Unknown; always 00
    [20c] = Unknown; always 00
    [20d] = Velocity (firmware translates MIDI velocity through a lookup table)
    [20e] = Voice select. In page 0 this also loads the staged parameters
            (201-210) into the voice and starts it.
    [20f] = LFO depth: 00 off, 01 for CC1 1-29, 18 for CC1 30-127,
            1a for programs with automatic vibrato (not implemented)
    [210] = LFO rate? The firmware always writes 15 (not implemented)
    [211] = 0 - Page 0: writes stage parameters for the next voice load
            3 - Page 3: writes go directly to the selected voice

    Velocity register values for MIDI range 0-127:

    01 01 01 01 01 01 01 02 02 03 03 04 04 05 05 06
    06 07 07 08 08 09 09 0A 0A 0B 0B 0C 0C 0D 0D 0E
    0E 0F 10 11 11 12 13 14 14 15 16 17 17 18 19 1A
    1A 1B 1C 1D 1D 1E 1F 20 20 21 22 23 23 24 25 26
    26 27 28 29 29 2A 2B 2C 2C 2D 2E 2F 2F 30 31 32
    35 38 3B 3E 41 44 47 4A 4D 50 4F 51 52 53 54 56
    57 58 59 5B 5C 5D 5E 60 61 62 63 65 66 67 6A 6B
    6C 6E 6F 70 71 73 74 75 76 78 79 7A 7B 7D 7E 7F

    The hc_u107 firmware's pitch bend tables imply the following multipliers
    for register 204, where m is bits 0-6:
        bit 7 clear: (256 - m) / 256
        bit 7 set:   (128 + m) / 128
    Both 00 and 80 give the unmodified rate.

    This is the sequence of register writes the firmware uses to play a note
    (Iron Fortress credit sound):

    [211] 0     Page 0
    [200] 1     Begin parameter group
    [203] d6    Address byte 2
    [202] a9    Address byte 1
    [201] 1     Address byte 0
    [204] 80    Pitch
    [205] 0     Unknown
    [206] 80    Right volume
    [207] 80    Left volume
    [208] b3    Volume
    [209] 0     Unknown (sound effects write 0 here)
    [20a] ff    Unknown
    [20b] 0     Unknown
    [20c] 0     Unknown
    [20d] 78    Velocity
    [211] 3     Page 3
    [20e] 0     Select voice 0
    [200] 0     Stop voice 0 (it may be a stolen voice in release)
    [211] 0     Page 0
    [200] 1     Begin parameter group
    [20e] 0     Load the staged parameters into voice 0 and start it
    [211] 3     Page 3
    [20f] 0     LFO depth (off)
    [210] 15    LFO rate

    Two-layer programs stop two voices, load the first as above, then stage
    the second layer's note table address and load the second voice. The
    second layer shares pan, volume and velocity with the first.


    Sound Headers
    =============

    The address registers point to a 6 byte entry in the sound ROM:

    [019be0]
    097b 397f 1510
    ^    ^    ^
    |    |    |
    |    |    +----- Sound descriptor pointer
    |    +---------- Filter? (upper byte 8n+1; not implemented)
    +--------------- Playback frequency (fixed point value representing 24MHz clock periods)

    This in turn points to a 16 byte descriptor:

    [1510]:
    0 4502D 4508E 45F91 D0 7F 0F 2A 1F 90 00 FF
    ^ ^     ^     ^     ^  ^  ^  ^  ^  ^  ^  ^
    | |     |     |     |  |  |  |  |  |  |  |
    | |     |     |     |  |  |  |  |  |  |  +-- Unknown (usually ff)
    | |     |     |     |  |  |  |  |  |  +----- Unknown (usually 00)
    | |     |     |     |  |  |  |  +--+-------- Envelope curve pointer (big-endian)
    | |     |     |     |  |  |  +-------------- Initial envelope value
    | |     |     |     |  |  +----------------- Release rate (wall clock, see below)
    | |     |     |     |  +-------------------- Envelope curve clock (source samples)
    | |     |     |     +----------------------- Flags (bit 3: ADPCM)
    | |     |     +----------------------------- Inclusive loop end address
    | |     +----------------------------------- Loop start address
    | +----------------------------------------- Start address
    +------------------------------------------- Address most-significant nibble (shared with loop addresses)

    Addresses are in bytes. For ADPCM, flag bits 0 and 1 select the low
    nibble of the loop start and end (unverified). ADPCM descriptors with
    equal loop start and end play once. Compressed samples are
    high-nibble-first IMA ADPCM with a wrapping 16-bit predictor.

    The envelope pointer selects a sampled curve in ROM, stepped every
    (byte 9 + 1) * 128 source samples with linear interpolation; a zero
    value terminates. Key-off ramps to zero at a fixed slope: full scale
    takes (byte 10 + 1) * 768 output frames (about 16 ms per unit).


    TODO:
    * Verify envelope interpolation, termination and the release constant
    * Verify ADPCM nibble selectors and identify loop/non-loop/short-loop modes
    * Figure out unknown sound header parameters
    * Figure out and implement filters
    * Determine the role of pitch register 205
    * Determine the velocity response (currently approximated as linear gain)
    * Implement modulation
    * Dump the internal ROM

***************************************************************************/
#include "emu.h"
#include "qs1000.h"

#include <algorithm>
#include <iterator>


//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


// device type definition
DEFINE_DEVICE_TYPE(QS1000, qs1000_device, "qs1000", "QS1000")

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

void qs1000_device::qs1000_prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}


void qs1000_device::qs1000_data_map(address_map &map)
{
	map(0x0000, 0x00ff).ram();
	map(0x0200, 0x0211).w(FUNC(qs1000_device::wave_w));
}


// ROM definition for the QS1000 internal program ROM
ROM_START( qs1000 )
	ROM_REGION( 0x10000, "cpu", 0 )
	ROM_LOAD( "qs1000.bin", 0x0000, 0x10000, NO_DUMP )
ROM_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  qs1000_device - constructor
//-------------------------------------------------
qs1000_device::qs1000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, QS1000, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	device_rom_interface(mconfig, *this),
	m_external_rom(false),
	m_in_p1_cb(*this, 0),
	m_in_p2_cb(*this, 0),
	m_in_p3_cb(*this, 0),
	m_out_p1_cb(*this),
	m_out_p2_cb(*this),
	m_out_p3_cb(*this),
	m_stream(nullptr),
	m_cpu(*this, "cpu"),
	m_wave_regs{},
	m_channels{}
{
}


//-------------------------------------------------
//  rom_region - return a pointer to the device's
//  internal ROM region
//-------------------------------------------------
const tiny_rom_entry *qs1000_device::device_rom_region() const
{
	return m_external_rom ? nullptr : ROM_NAME( qs1000 );
}


//-------------------------------------------------
//  device_add_mconfig - add machine configuration
//-------------------------------------------------

void qs1000_device::device_add_mconfig(machine_config &config)
{
	I8052(config, m_cpu, DERIVED_CLOCK(1, 1));
	m_cpu->set_addrmap(AS_PROGRAM, &qs1000_device::qs1000_prg_map);
	m_cpu->set_addrmap(AS_DATA, &qs1000_device::qs1000_data_map);
	m_cpu->port_in_cb<1>().set(FUNC(qs1000_device::p1_r));
	m_cpu->port_out_cb<1>().set(FUNC(qs1000_device::p1_w));
	m_cpu->port_in_cb<2>().set(FUNC(qs1000_device::p2_r));
	m_cpu->port_out_cb<2>().set(FUNC(qs1000_device::p2_w));
	m_cpu->port_in_cb<3>().set(FUNC(qs1000_device::p3_r));
	m_cpu->port_out_cb<3>().set(FUNC(qs1000_device::p3_w));
}


//-------------------------------------------------
//  rom_bank_pre_change - refresh the stream if the
//  ROM banking changes
//-------------------------------------------------

void qs1000_device::rom_bank_pre_change()
{
	m_stream->update();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void qs1000_device::device_start()
{
	// The QS1000 operates at 24MHz. Creating a stream at that rate
	// would be overkill so we opt for a fraction of that rate which
	// gives reasonable results
	m_stream = stream_alloc(0, 2, clock() / 32);

	save_item(NAME(m_wave_regs));

	for (int i = 0; i < QS1000_CHANNELS; i++)
	{
		save_item(NAME(m_channels[i].m_acc), i);
		save_item(NAME(m_channels[i].m_previous_sample), i);
		save_item(NAME(m_channels[i].m_adpcm_signal), i);
		save_item(NAME(m_channels[i].m_addr), i);
		save_item(NAME(m_channels[i].m_adpcm_addr), i);
		save_item(NAME(m_channels[i].m_loop_start), i);
		save_item(NAME(m_channels[i].m_loop_end), i);
		save_item(NAME(m_channels[i].m_freq), i);
		save_item(NAME(m_channels[i].m_flags), i);
		save_item(NAME(m_channels[i].m_env_addr), i);
		save_item(NAME(m_channels[i].m_env_period), i);
		save_item(NAME(m_channels[i].m_release_period), i);
		save_item(NAME(m_channels[i].m_env_remaining), i);
		save_item(NAME(m_channels[i].m_env_level), i);
		save_item(NAME(m_channels[i].m_env_step), i);
		save_item(NAME(m_channels[i].m_env_target), i);
		save_item(NAME(m_channels[i].m_regs), i);
		save_item(NAME(m_channels[i].m_adpcm.m_signal), i);
		save_item(NAME(m_channels[i].m_adpcm.m_step), i);
		save_item(NAME(m_channels[i].m_adpcm.m_loop_signal), i);
		save_item(NAME(m_channels[i].m_adpcm.m_loop_step), i);
		save_item(NAME(m_channels[i].m_adpcm.m_saved), i);
	}
}


//-------------------------------------------------
//  set_irq - interrupt the internal CPU
//-------------------------------------------------
void qs1000_device::set_irq(int state)
{
	// Signal to the CPU that data is available
	m_cpu->set_input_line(MCS51_INT1_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void qs1000_device::device_reset()
{
	for (auto &elem : m_channels)
	{
		elem.m_flags = 0;
	}
}


//-------------------------------------------------
//  p0_r
//-------------------------------------------------
uint8_t  qs1000_device::p0_r()
{
	return 0xff;
}


//-------------------------------------------------
//  p1_r
//-------------------------------------------------
uint8_t  qs1000_device::p1_r()
{
	return m_in_p1_cb(0);
}


//-------------------------------------------------
//  p2_r
//-------------------------------------------------
uint8_t  qs1000_device::p2_r()
{
	return m_in_p2_cb(0);
}


//-------------------------------------------------
//  p3_r
//-------------------------------------------------
uint8_t qs1000_device::p3_r()
{
	return m_in_p3_cb(0);
}


//-------------------------------------------------
//  p0_w
//-------------------------------------------------
void qs1000_device::p0_w(uint8_t data)
{
}


//-------------------------------------------------
//  p1_w
//-------------------------------------------------

void qs1000_device::p1_w(uint8_t data)
{
	m_out_p1_cb(data);
}


//-------------------------------------------------
//  p2_w
//-------------------------------------------------

void qs1000_device::p2_w(uint8_t data)
{
	m_out_p2_cb(data);
}


//-------------------------------------------------
//  p3_w
//-------------------------------------------------

void qs1000_device::p3_w(uint8_t data)
{
	m_out_p3_cb(data);
}


//-------------------------------------------------
//  wave_w - process writes to wavetable engine
//-------------------------------------------------

void qs1000_device::wave_w(offs_t offset, uint8_t data)
{
	m_stream->update();

	LOG("QS1000 W[%x] %x\n", 0x200 + offset, data);

	const bool voice_page = m_wave_regs[0x11] == 3;

	switch (offset)
	{
		case 0x00:
		{
			// Page 0: 1 begins a parameter group
			if (!voice_page)
			{
				break;
			}

			auto &chan = m_channels[m_wave_regs[0xe] & (QS1000_CHANNELS - 1)];
			if (data == 0)
			{
				chan.m_flags = 0;
			}
			else if (data == 2)
			{
				// Repeated key-offs do not restart the release ramp
				if (chan.m_flags & QS1000_KEYON)
				{
					chan.m_flags &= ~QS1000_KEYON;
					// Fixed slope, so a lower level ends sooner
					const uint32_t period = (uint64_t(chan.m_env_level) * chan.m_release_period) / (255 << 16);
					set_envelope_target(chan, 0, std::max<uint32_t>(period, 1));
				}
			}
			break;
		}

		case 0x0e:
		{
			m_wave_regs[offset] = data;
			if (!voice_page)
			{
				// Page 0: selecting a voice loads the staged parameters and starts it
				const int ch = data & (QS1000_CHANNELS - 1);
				std::copy_n(m_wave_regs, std::size(m_channels[ch].m_regs), m_channels[ch].m_regs);
				start_voice(ch);
			}
			break;
		}

		case 0x11:
			m_wave_regs[offset] = data;
			break;

		default:
		{
			if (voice_page)
			{
				m_channels[m_wave_regs[0xe] & (QS1000_CHANNELS - 1)].m_regs[offset] = data;
			}
			else
			{
				m_wave_regs[offset] = data;
			}
			break;
		}
	}
}


//-------------------------------------------------
//  ADPCM decoder
//-------------------------------------------------

void qs1000_device::qs1000_adpcm::reset()
{
	m_signal = m_loop_signal = 0;
	m_step = m_loop_step = 0;
	m_saved = false;
}

int16_t qs1000_device::qs1000_adpcm::clock(uint8_t nibble)
{
	static constexpr int step_table[89] =
	{
		7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31,
		34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143,
		157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544,
		598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878,
		2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358, 5894,
		6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899, 15289, 16818,
		18500, 20350, 22385, 24623, 27086, 29794, 32767
	};
	static constexpr int index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

	const int step = step_table[m_step];
	const int delta = (step >> 3) + (BIT(nibble, 0) ? (step >> 2) : 0)
			+ (BIT(nibble, 1) ? (step >> 1) : 0) + (BIT(nibble, 2) ? step : 0);

	// Wraps rather than saturating like ima_adpcm_state; QS1001A crash and ride attacks need it
	m_signal = util::sext(m_signal + (BIT(nibble, 3) ? -delta : delta), 16);
	m_step = std::clamp(m_step + index_shift[nibble & 7], 0, 88);
	return m_signal;
}

void qs1000_device::qs1000_adpcm::save()
{
	if (!m_saved)
	{
		m_loop_signal = m_signal;
		m_loop_step = m_step;
		m_saved = true;
	}
}

void qs1000_device::qs1000_adpcm::restore()
{
	m_signal = m_loop_signal;
	m_step = m_loop_step;
}


//-------------------------------------------------
//  sound_stream_update -
//-------------------------------------------------
void qs1000_device::sound_stream_update(sound_stream &stream)
{
	// Iterate over voices and accumulate sample data
	for (auto &chan : m_channels)
	{
		if (chan.m_flags & QS1000_PLAYING)
		{
			// Velocity is approximated as linear gain; the firmware writes 206 for right and 207 for left
			const float vol = float(chan.m_regs[8]) * chan.m_regs[0x0d] / (127.0f * (32768 << 12));
			const float lvol = chan.m_regs[7] * vol;
			const float rvol = chan.m_regs[6] * vol;

			// Pitch multiplier with eight extra fractional bits, applied to the descriptor rate
			const uint8_t pitch = chan.m_regs[4];
			const uint32_t freq = chan.m_freq * (BIT(pitch, 7) ? (256 + 2 * (pitch & 0x7f)) : (256 - pitch));

			for (int samp = 0; samp < stream.samples(); samp++)
			{
				if (!(chan.m_flags & QS1000_PLAYING))
				{
					break;
				}

				if (chan.m_addr > chan.m_loop_end)
				{
					if (chan.m_loop_start > chan.m_loop_end)
					{
						chan.m_flags &= ~QS1000_PLAYING;
						break;
					}

					// Inclusive loop end; the envelope decides when a releasing voice stops
					chan.m_addr = chan.m_loop_start + (chan.m_addr - chan.m_loop_end - 1) % (chan.m_loop_end - chan.m_loop_start + 1);
					if (chan.m_flags & QS1000_ADPCM)
					{
						chan.m_adpcm.restore();
						chan.m_adpcm_addr = chan.m_loop_start - 1;
					}
				}

				float result;
				if (chan.m_flags & QS1000_ADPCM)
				{
					while (chan.m_adpcm_addr != chan.m_addr)
					{
						++chan.m_adpcm_addr;
						// Save the predictor before decoding the first loop nibble
						if (chan.m_adpcm_addr == chan.m_loop_start)
						{
							chan.m_adpcm.save();
						}

						const uint8_t data = read_byte(chan.m_adpcm_addr >> 1);
						const uint8_t nibble = BIT(chan.m_adpcm_addr, 0) ? (data & 0x0f) : (data >> 4);
						chan.m_adpcm_signal = chan.m_adpcm.clock(nibble);
					}
					result = float(chan.m_adpcm_signal) * (1.0f / 64.0f);
				}
				else
				{
					result = read_byte(chan.m_addr) - 128;
				}

				// Linear interpolation; the real reconstruction filter is unknown
				const float fraction = float(chan.m_acc) / (1U << QS1000_ACC_FRAC_BITS);
				const float interpolated = chan.m_previous_sample + (result - chan.m_previous_sample) * fraction;
				const float envelope = float(chan.m_env_level) / (255 << 16);
				stream.add(0, samp, interpolated * lvol * envelope);
				stream.add(1, samp, interpolated * rvol * envelope);

				chan.m_acc += freq;
				const uint32_t advance = chan.m_acc >> QS1000_ACC_FRAC_BITS;
				if (advance)
				{
					chan.m_previous_sample = result;
				}
				chan.m_addr += advance;
				chan.m_acc &= util::make_bitmask<uint32_t>(QS1000_ACC_FRAC_BITS);

				// The held curve steps in source samples, the release ramp in stream samples
				advance_envelope(chan, (chan.m_flags & QS1000_KEYON) ? advance : 1);
			}
		}
	}
}


//-------------------------------------------------
//  set_envelope_target - begin an amplitude segment
//-------------------------------------------------

void qs1000_device::set_envelope_target(qs1000_channel &chan, uint8_t target, uint32_t period)
{
	chan.m_env_target = target;
	chan.m_env_remaining = period;
	chan.m_env_step = ((int32_t(target) << 16) - int32_t(chan.m_env_level)) / int32_t(period);
}


//-------------------------------------------------
//  advance_envelope - step the amplitude envelope
//-------------------------------------------------

void qs1000_device::advance_envelope(qs1000_channel &chan, uint32_t samples)
{
	while (samples && (chan.m_flags & QS1000_PLAYING))
	{
		const uint32_t advance = std::min<uint32_t>(samples, chan.m_env_remaining);
		chan.m_env_level = int32_t(chan.m_env_level) + chan.m_env_step * int32_t(advance);
		chan.m_env_remaining -= advance;
		samples -= advance;

		if (!chan.m_env_remaining)
		{
			chan.m_env_level = uint32_t(chan.m_env_target) << 16;
			if (!chan.m_env_target || !(chan.m_flags & QS1000_KEYON))
			{
				chan.m_flags &= ~QS1000_PLAYING;
			}
			else
			{
				++chan.m_env_addr;
				set_envelope_target(chan, read_byte(chan.m_env_addr), chan.m_env_period);
			}
		}
	}
}


void qs1000_device::start_voice(int ch)
{
	m_channels[ch].m_flags = 0;
	const uint32_t table_addr = (m_channels[ch].m_regs[0x01] << 16) | (m_channels[ch].m_regs[0x02] << 8) | m_channels[ch].m_regs[0x03];

	// Fetch the sound information
	const uint16_t freq = (read_byte(table_addr + 0) << 8) | read_byte(table_addr + 1);
	const uint16_t base = (read_byte(table_addr + 4) << 8) | read_byte(table_addr + 5);

	LOG("[%.6x] Freq:%.4x  ????:%.4x  Addr:%.4x\n", table_addr, freq, (read_byte(table_addr + 2) << 8) | read_byte(table_addr + 3), base);

	// See Raccoon World and Wyvern Wings nullptr sound
	if (freq == 0)
	{
		return;
	}

	// Fetch the sample pointers and flags
	const uint8_t byte0 = read_byte(base);

	uint32_t start_addr;

	start_addr  = byte0 << 16;
	start_addr |= read_byte(base + 1) << 8;
	start_addr |= read_byte(base + 2) << 0;
	start_addr &= QS1000_ADDRESS_MASK;

	uint32_t loop_start;

	loop_start = (byte0 & 0xf0) << 16;
	loop_start |= read_byte(base + 3) << 12;
	loop_start |= read_byte(base + 4) << 4;
	loop_start |= read_byte(base + 5) >> 4;
	loop_start &= QS1000_ADDRESS_MASK;

	uint32_t loop_end;

	loop_end = (byte0 & 0xf0) << 16;
	loop_end |= (read_byte(base + 5) & 0xf) << 16;
	loop_end |= read_byte(base + 6) << 8;
	loop_end |= read_byte(base + 7);
	loop_end &= QS1000_ADDRESS_MASK;

	const uint8_t byte8 = read_byte(base + 8);

	LOG("[%.6x] Sample Start:%.6x  Loop Start:%.6x  Loop End:%.6x  Params: %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x\n", base, start_addr, loop_start, loop_end, byte8,
			read_byte(base + 9), read_byte(base + 10), read_byte(base + 11), read_byte(base + 12), read_byte(base + 13), read_byte(base + 14), read_byte(base + 15));

	// ADPCM addresses are nibbles; equal loop byte pointers or an invalid loop play once
	const bool adpcm = BIT(byte8, 3);
	const bool no_loop = adpcm && (loop_start == loop_end);
	const uint32_t start = adpcm ? (start_addr << 1) : start_addr;
	const uint32_t loop = adpcm ? ((loop_start << 1) | BIT(byte8, 0)) : loop_start;
	const uint32_t end = adpcm ? ((loop_end << 1) | BIT(byte8, 1)) : loop_end;
	if (end < start)
	{
		return;
	}

	auto &chan = m_channels[ch];
	chan.m_acc = 0;
	chan.m_previous_sample = 0;
	chan.m_addr = start;
	chan.m_loop_start = (loop >= start && !no_loop) ? loop : (end + 1);
	chan.m_loop_end = end;
	chan.m_freq = freq;
	chan.m_flags = QS1000_PLAYING | QS1000_KEYON;

	// Bytes 12-13 point to the ROM envelope curve, byte 11 is the initial level
	chan.m_env_addr = (read_byte(base + 12) << 8) | read_byte(base + 13);
	chan.m_env_period = (uint16_t(read_byte(base + 9)) + 1) * 128;
	chan.m_release_period = (uint32_t(read_byte(base + 10)) + 1) * QS1000_RELEASE_UNIT;
	chan.m_env_level = uint32_t(read_byte(base + 11)) << 16;
	uint8_t target = read_byte(chan.m_env_addr);
	if (target == read_byte(base + 11))
	{
		target = read_byte(++chan.m_env_addr);
	}
	set_envelope_target(chan, target, chan.m_env_period);

	if (adpcm)
	{
		chan.m_adpcm.reset();
		chan.m_adpcm_addr = start - 1;
		chan.m_adpcm_signal = 0;
		chan.m_flags |= QS1000_ADPCM;
	}
}
