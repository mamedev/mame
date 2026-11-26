// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

    IGS003 I/O and protection chip

    DIP40. Several revisions exist (IGS003, IGS003A, IGS003c, IGS003e).

    Everything below is derived from the game code, there is no
    documentation for this chip.

    The CPU writes an index, then reads or writes the data port:

    index  read                           write
    00-02  port A-C                       port A-C output latch
    03     protection result              port A direction (1 = input)
    04-05  -                              port B-C direction (1 = input)
    20-34  fixed table ("IGS" etc.)       -
    40     -                              protection word (shifted in)
    41-47  -                              games write the same as 40, use unknown
    48     -                              latch protection x from the word
    50     -                              latch protection mode from the word
    60-6b  fixed table                    60-62 written with 0 at boot, purpose unknown
    80-87  -                              protection step
    a0     -                              clear the protection value

    The protection keeps a 16-bit value. Each step rotates it left,
    xors it with 0x2bad, xors bit 0 with three bits of the old value
    (picked by the mode) and with bit (index & 7) of the written data,
    and xors four more bits with x. The result is a bitswap of the low
    byte. The taps and the positions of the x bits are fixed in each
    chip variant: different versions of the same game, and different
    regions of orlegend, need different ones.

    TODO:
    - What reading a port returns on bits set as outputs
    - The meaning of 60-62 writes
    - Identify differences between different revisions

***************************************************************************/

#include "emu.h"
#include "igs003.h"

#define LOG_UNKNOWN (1U << 1)
#define LOG_HOLD    (1U << 2)

// #define VERBOSE (LOG_GENERAL | LOG_UNKNOWN | LOG_HOLD)

#include "logmacro.h"

#define LOGUNKNOWN(...) LOGMASKED(LOG_UNKNOWN, __VA_ARGS__)
#define LOGHOLD(...)    LOGMASKED(LOG_HOLD,    __VA_ARGS__)


DEFINE_DEVICE_TYPE(IGS003, igs003_device, "igs003", "IGS003 I/O and protection")


namespace {

// fixed tables, the 0xff entries are placeholders for bytes that no known game reads
constexpr u8 TABLE_20[0x15] =
{
	0x49, 0x47, 0x53, 0xff, 0x41, 0x41, 0x7f, 0x41, 0x41, 0xff, 0x3e, 0x41, 0x49, 0xf9, 0x0a, 0xff,
	0x26, 0x49, 0x49, 0x49, 0x32
};

constexpr u8 TABLE_60[0x0c] =
{
	0x30, 0x31, 0x3e, 0xff, 0x3c, 0x31, 0x39, 0x33, 0x35, 0xff, 0x40, 0x43
};

// bits of the old value xored into bit 0, per mode
constexpr u8 HOLD_TAPS[2][4][3] =
{
	// A: iqblocka, iqblockf, lhb2, nkishusp, tygn, vbowl, orlegend (regions 0-2)
	{ { 10, 8, 5 }, { 12, 8, 3 }, { 11, 6, 2 }, {  3, 1, 0 } },
	// B: genius6, lhb3, lhb2cpgs, sdmg2641c, vbowlhk, orlegend (regions 3-4)
	{ {  7, 6, 5 }, {  9, 6, 1 }, { 12, 8, 4 }, {  6, 5, 3 } }
};

} // anonymous namespace


igs003_device::igs003_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, IGS003, tag, owner, clock),
	m_hold_taps(hold_taps::A),
	m_hold_xpos{ 4, 6, 10, 12 },
	m_index(0),
	m_latch{ 0, 0, 0 },
	m_dir{ 0xff, 0xff, 0xff },
	m_hold(0),
	m_word(0),
	m_x(0),
	m_mode(0),
	m_in_port_cb(*this, 0xff),
	m_out_port_cb(*this)
{
}

void igs003_device::device_start()
{
	save_item(NAME(m_index));
	save_item(NAME(m_latch));
	save_item(NAME(m_dir));
	save_item(NAME(m_hold));
	save_item(NAME(m_word));
	save_item(NAME(m_x));
	save_item(NAME(m_mode));
}

void igs003_device::device_reset()
{
	// all games set the directions at boot, all inputs and a cleared protection state are guesses
	for (int i = 0; i < 3; i++)
	{
		m_latch[i] = 0;

		dir_w(i, 0xff);
	}

	m_hold = 0;
	m_word = 0;
	m_x = 0;
	m_mode = 0;
}


u8 igs003_device::port_r(int port)
{
	// TODO: what bits set as outputs read back as is unknown, the input callback is read for all bits
	// (with the output latch, jinhulu2 sees its port C outputs as pressed inputs)
	return m_in_port_cb[port]();
}

void igs003_device::port_w(int port, u8 data)
{
	m_latch[port] = data;

	if (u8(~m_dir[port]))
		m_out_port_cb[port](0, data, u8(~m_dir[port]));
}

void igs003_device::dir_w(int port, u8 data)
{
	m_dir[port] = data;

	if (u8(~data))
		m_out_port_cb[port](0, m_latch[port], u8(~data));
}


void igs003_device::hold_step(int data_bit)
{
	const u16 old = m_hold;
	const u8 *taps = HOLD_TAPS[int(m_hold_taps)][m_mode];

	m_hold = ((old << 1) | (old >> 15)) ^ 0x2bad;

	m_hold ^= BIT(old, taps[0]) ^ BIT(old, taps[1]) ^ BIT(old, taps[2]) ^ data_bit;

	for (int i = 0; i < 4; i++)
		m_hold ^= BIT(m_x, i) << m_hold_xpos[i];

	LOGHOLD("%s: hold step mode %d x %x bit %d: %04x -> %04x\n", machine().describe_context(), m_mode, m_x, data_bit, old, m_hold);
}

u8 igs003_device::hold_r() const
{
	return bitswap<8>(m_hold, 5, 2, 9, 7, 10, 13, 12, 15);
}


void igs003_device::index_w(u8 data)
{
	m_index = data;
}

u8 igs003_device::data_r()
{
	if (m_index < 0x03)
		return port_r(m_index);

	if (m_index == 0x03)
		return hold_r();

	if ((m_index >= 0x20) && (m_index <= 0x34))
		return TABLE_20[m_index - 0x20];

	if ((m_index >= 0x60) && (m_index <= 0x6b))
		return TABLE_60[m_index - 0x60];

	if (!machine().side_effects_disabled())
		LOGUNKNOWN("%s: read with unknown index %02x\n", machine().describe_context(), m_index);

	// no known game reads these indices, the value is an arbitrary choice
	return 0xff;
}

void igs003_device::data_w(u8 data)
{
	switch (m_index)
	{
		case 0x00:
		case 0x01:
		case 0x02:
			port_w(m_index, data);
			break;

		case 0x03:
		case 0x04:
		case 0x05:
			dir_w(m_index - 0x03, data);
			break;

		case 0x40:
			m_word = (m_word << 8) | data;
			break;

		case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
			break;

		case 0x48:
			// the games only write values where a whole nibble matches or not
			m_x = 0;
			if ((m_word & 0x0f00) != 0x0a00) m_x |= 0x08;
			if ((m_word & 0xf000) != 0x9000) m_x |= 0x04;
			if ((m_word & 0x000f) != 0x0006) m_x |= 0x02;
			if ((m_word & 0x00f0) != 0x0090) m_x |= 0x01;
			LOGHOLD("%s: x = %x (word %04x)\n", machine().describe_context(), m_x, m_word);
			break;

		case 0x50:
			// the games write 0x53, 0x73, 0x51 or 0x77 for modes 0-3
			m_mode = 0;
			if ((m_word & 0x000f) != 0x0003) m_mode |= 0x02;
			if ((m_word & 0x00f0) != 0x0050) m_mode |= 0x01;
			LOGHOLD("%s: mode = %d (word %04x)\n", machine().describe_context(), m_mode, m_word);
			break;

		case 0x60:
		case 0x61:
		case 0x62:
			LOGUNKNOWN("%s: write %02x to index %02x\n", machine().describe_context(), data, m_index);
			break;

		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
			hold_step(BIT(data, m_index & 7));
			break;

		case 0xa0:
			m_hold = 0;
			break;

		default:
			LOGUNKNOWN("%s: write %02x to unknown index %02x\n", machine().describe_context(), data, m_index);
			break;
	}
}
