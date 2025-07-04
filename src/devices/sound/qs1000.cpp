// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    qs1000.cpp

    QS1000 device emulator.

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

    [200] = Key on/off
            0 = Key on
            1 = ?
            2 = key off
    [201] = Address byte 0 (LSB)
    [202] = Address byte 1
    [203] = Address byte 2
    [204] = Pitch
    [205] = Pitch high byte? (Usually 0)
    [206] = Left volume
    [207] = Right volume
    [208] = Volume
    [209] = ?
    [20a] = ?
    [20b] = ?
    [20c] = ?
    [20d] = Velocity
    [20e] = Channel select
    [20f] = Modulation
    [210] = Modulation
    [211] = 0 - Select global registers?
            3 - Select channel registers?

    Velocity register values for MIDI range 0-127:

    01 01 01 01 01 01 01 02 02 03 03 04 04 05 05 06
    06 07 07 08 08 09 09 0A 0A 0B 0B 0C 0C 0D 0D 0E
    0E 0F 10 11 11 12 13 14 14 15 16 17 17 18 19 1A
    1A 1B 1C 1D 1D 1E 1F 20 20 21 22 23 23 24 25 26
    26 27 28 29 29 2A 2B 2C 2C 2D 2E 2F 2F 30 31 32
    35 38 3B 3E 41 44 47 4A 4D 50 4F 51 52 53 54 56
    57 58 59 5B 5C 5D 5E 60 61 62 63 65 66 67 6A 6B
    6C 6E 6F 70 71 73 74 75 76 78 79 7A 7B 7D 7E 7F

    (TODO: Other register values)

    This is the sequence of register writes used to play the Iron Fortress credit sound:

    [211] 0     Select global registers?
    [200] 1     ?
    [203] d6    Address byte 2
    [202] a9    Address byte 1
    [201] 1     Address byte 0
    [204] 80    Pitch
    [205] 0     ?
    [206] 80    Left volume
    [207] 80    Right volume
    [208] b3    Volume
    [209] 0     ?
    [20a] ff    ?
    [20b] 0     ?
    [20c] 0     ?
    [20d] 78    Velocity
    [211] 3     Select channel registers
    [20e] 0     Select channel
    [200] 0     Key on


    Sound Headers
    =============

    The address registers point to a 6 byte entry in the sound ROM:

    [019be0]
    097b 397f 1510
    ^    ^    ^
    |    |    |
    |    |    +----- Sound descriptor pointer
    |    +---------- ?
    +--------------- Playback frequency (fixed point value representing 24MHz clock periods)

    This in turn points to a 24 byte descriptor:

    [1510]:
    0 4502D 4508E 45F91 D0 7F 0F 2A 1F 90 00 FF
    ^ ^     ^     ^     ^  ^  ^  ^  ^  ^  ^  ^
    | |     |     |     |  |  |  |  |  |  |  |
    | |     |     |     |  |  |  |  |  |  |  +-- ?
    | |     |     |     |  |  |  |  |  |  +----- ?
    | |     |     |     |  |  |  |  |  +-------- ?
    | |     |     |     |  |  |  |  +----------- ?
    | |     |     |     |  |  |  +-------------- ?
    | |     |     |     |  |  +----------------- Bit 7: Format (0:PCM 1:ADPCM)
    | |     |     |     |  +-------------------- ?
    | |     |     |     +----------------------- ?
    | |     |     +----------------------------- Loop end address
    | |     +----------------------------------- Loop start address
    | +----------------------------------------- Start address
    +------------------------------------------- Address most-significant nibble (shared with loop addresses)

    * The unknown parameters are most likely envelope and filter parameters.
    * Is there a loop flag or do sounds loop indefinitely until stopped?


    TODO:
    * Looping is currently disabled
    * Figure out unknown sound header parameters
    * Figure out and implement envelopes and filters
    * Pitch bending
    * Dump the internal ROM

***************************************************************************/
#include "emu.h"
#include "qs1000.h"


#define LOGGING_ENABLED     0


// device type definition
DEFINE_DEVICE_TYPE(QS1000, qs1000_device, "qs1000", "QS1000")

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

void qs1000_device::qs1000_prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}


void qs1000_device::qs1000_io_map(address_map &map)
{
	map(0x0000, 0x00ff).ram();
	map(0x0200, 0x0211).w(FUNC(qs1000_device::wave_w));
}


// ROM definition for the QS1000 internal program ROM
ROM_START( qs1000 )
	ROM_REGION( 0x10000, "cpu", 0 )
	ROM_LOAD_OPTIONAL( "qs1000.bin", 0x0000, 0x10000, NO_DUMP )
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
	m_cpu(*this, "cpu")
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
	m_cpu->set_addrmap(AS_IO, &qs1000_device::qs1000_io_map);
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
		save_item(NAME(m_channels[i].m_adpcm_signal), i);
		save_item(NAME(m_channels[i].m_start), i);
		save_item(NAME(m_channels[i].m_addr), i);
		save_item(NAME(m_channels[i].m_adpcm_addr), i);
		save_item(NAME(m_channels[i].m_loop_start), i);
		save_item(NAME(m_channels[i].m_loop_end), i);
		save_item(NAME(m_channels[i].m_freq), i);
		save_item(NAME(m_channels[i].m_flags), i);
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
	for (auto & elem : m_channels)
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
	m_out_p1_cb((offs_t)0, data);
}


//-------------------------------------------------
//  p2_w
//-------------------------------------------------

void qs1000_device::p2_w(uint8_t data)
{
	m_out_p2_cb((offs_t)0, data);
}


//-------------------------------------------------
//  p3_w
//-------------------------------------------------

void qs1000_device::p3_w(uint8_t data)
{
	m_out_p3_cb((offs_t)0, data);
}


//-------------------------------------------------
//  wave_w - process writes to wavetable engine
//-------------------------------------------------

void qs1000_device::wave_w(offs_t offset, uint8_t data)
{
	m_stream->update();

	if (LOGGING_ENABLED)
		printf("QS1000 W[%x] %x\n", 0x200 + offset, data);

	switch (offset)
	{
		case 0x00:
		{
			int ch = m_wave_regs[0xe];

			if (data == 0)
			{
				// TODO
				for (int i = 0; i < 16; ++i)
					m_channels[ch].m_regs[i] = m_wave_regs[i];

				// Key on
				start_voice(ch);
			}
			if (data == 1)
			{
				// ?
			}
			else if (data == 2)
			{
				// Key off
				m_channels[ch].m_flags &= ~QS1000_KEYON;
			}
			break;
		}

		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		{
			if (m_wave_regs[0x11] == 3)
			{
				// Channel-specific write?
				m_channels[m_wave_regs[0xe]].m_regs[offset] = data;
			}
			else
			{
				// Global write?
				m_wave_regs[offset] = data;
			}
			break;
		}

		default:
			m_wave_regs[offset] = data;
	}
}


//-------------------------------------------------
//  sound_stream_update -
//-------------------------------------------------
void qs1000_device::sound_stream_update(sound_stream &stream)
{
	// Iterate over voices and accumulate sample data
	for (auto & chan : m_channels)
	{
		uint8_t lvol = chan.m_regs[6];
		uint8_t rvol = chan.m_regs[7];
		uint8_t vol  = chan.m_regs[8];

		if (chan.m_flags & QS1000_PLAYING)
		{
			if (chan.m_flags & QS1000_ADPCM)
			{
				for (int samp = 0; samp < stream.samples(); samp++)
				{
					if (chan.m_addr >= chan.m_loop_end)
					{
#if 0 // Looping disabled until envelopes work
						if (chan.m_flags & QS1000_KEYON)
						{
							chan.m_addr = chan.m_loop_start;
						}
						else
#endif
						{
							chan.m_flags &= ~QS1000_PLAYING;
							break;
						}
					}

					// Not too keen on this but it'll do for now
					while (chan.m_start + chan.m_adpcm_addr != chan.m_addr)
					{
						chan.m_adpcm_addr++;

						if (chan.m_start + chan.m_adpcm_addr >= chan.m_loop_end)
						{
							chan.m_adpcm_addr = chan.m_loop_start - chan.m_start;
#if 0 // Looping disabled until envelopes work
							chan.m_adpcm.restore();
						}
						if (chan.m_start + chan.m_adpcm_addr == chan.m_loop_start)
						{
							chan.m_adpcm.save();
#endif
						}

						uint8_t data = read_byte(chan.m_start + (chan.m_adpcm_addr >> 1));
						uint8_t nibble = (chan.m_adpcm_addr & 1 ? data : data >> 4) & 0xf;
						chan.m_adpcm_signal = chan.m_adpcm.clock(nibble);
					}

					int8_t result = (chan.m_adpcm_signal >> 4);
					chan.m_acc += chan.m_freq;
					chan.m_addr = (chan.m_addr + (chan.m_acc >> 18)) & QS1000_ADDRESS_MASK;
					chan.m_acc &= ((1 << 18) - 1);

					stream.add_int(0, samp, result * 4 * lvol * vol, 32768 << 12);
					stream.add_int(1, samp, result * 4 * rvol * vol, 32768 << 12);
				}
			}
			else
			{
				for (int samp = 0; samp < stream.samples(); samp++)
				{
					if (chan.m_addr >= chan.m_loop_end)
					{
#if 0 // Looping disabled until envelopes work
						if (chan.m_flags & QS1000_KEYON)
						{
							chan.m_addr = chan.m_loop_start;
						}
						else
#endif
						{
							chan.m_flags &= ~QS1000_PLAYING;
							break;
						}
					}

					int8_t result = read_byte(chan.m_addr) - 128;

					chan.m_acc += chan.m_freq;
					chan.m_addr = (chan.m_addr + (chan.m_acc >> 18)) & QS1000_ADDRESS_MASK;
					chan.m_acc &= ((1 << 18) - 1);

					stream.add_int(0, samp, result * lvol * vol, 32768 << 12);
					stream.add_int(1, samp, result * rvol * vol, 32768 << 12);
				}
			}
		}
	}
}


void qs1000_device::start_voice(int ch)
{
	uint32_t table_addr = (m_channels[ch].m_regs[0x01] << 16) | (m_channels[ch].m_regs[0x02] << 8) | m_channels[ch].m_regs[0x03];

	// Fetch the sound information
	uint16_t freq = (read_byte(table_addr + 0) << 8) | read_byte(table_addr + 1);
	uint16_t word1 = (read_byte(table_addr + 2) << 8) | read_byte(table_addr + 3);
	uint16_t base = (read_byte(table_addr + 4) << 8) | read_byte(table_addr + 5);

	if (LOGGING_ENABLED)
		printf("[%.6x] Freq:%.4x  ????:%.4x  Addr:%.4x\n", table_addr, freq, word1, base);

	// See Raccoon World and Wyvern Wings nullptr sound
	if (freq == 0)
		return;

	// Fetch the sample pointers and flags
	uint8_t byte0 = read_byte(base);

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

	uint8_t byte8 = read_byte(base + 8);

	if (LOGGING_ENABLED)
	{
		uint8_t byte9 = read_byte(base + 9);
		uint8_t byte10 = read_byte(base + 10);
		uint8_t byte11 = read_byte(base + 11);
		uint8_t byte12 = read_byte(base + 12);
		uint8_t byte13 = read_byte(base + 13);
		uint8_t byte14 = read_byte(base + 14);
		uint8_t byte15 = read_byte(base + 15);

		printf("[%.6x] Sample Start:%.6x  Loop Start:%.6x  Loop End:%.6x  Params: %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x\n", base, start_addr, loop_start, loop_end, byte8, byte9, byte10, byte11, byte12, byte13, byte14, byte15);
	}

	m_channels[ch].m_acc = 0;
	m_channels[ch].m_start = start_addr;
	m_channels[ch].m_addr = start_addr;
	m_channels[ch].m_loop_start = loop_start;
	m_channels[ch].m_loop_end = loop_end;
	m_channels[ch].m_freq = freq;
	m_channels[ch].m_flags = QS1000_PLAYING | QS1000_KEYON;

	if (byte8 & 0x08)
	{
		m_channels[ch].m_adpcm.reset();
		m_channels[ch].m_adpcm_addr = -1;
//      m_channels[ch].m_adpcm_signal = -2;
		m_channels[ch].m_flags |= QS1000_ADPCM;
	}
}
