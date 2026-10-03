// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// thanks-to:Guru
/***************************************************************************

    Subsino SS9804 / SS9904 4-channel sample player (QFP100)

    Found on the H8/3044 based Subsino boards (Bishou Jan, Xiao Ao Jiang Hu,
    Last Fighting, New 2001, Queen Bee, X-Reel, Humlan's Lyckohjul, ...).
    The chip reads its own sample ROM (4 or 8 Mbit) and is driven through a
    single byte-wide port:

      4E a0 a1 a2 a3   init / configuration.  Every game sends 4E 00 10 52 xx
                       at boot; bit 4 of xx clear (0xE5) selects the address
                       scrambled ROM layout (1999-2000 games), set (0xF5) the
                       plain layout (2000+ games).  The other bits are unknown.
      0x xxxx          key off: stop the channels whose bit is set (0F = all)
      88+ch pp nn      key on: play sample nn of table page pp on channel ch
                       (pp is always 04 -> table at byte address 0x2000)
      read             bits 0-3: channel 0-3 busy

    Writes of 10-13 to the same port drive the Dallas 1-wire EEPROM line on
    these boards and are ignored here.

    Sample ROM: table entry n at (pp << 11) + 8*n, two little-endian u32
    (start, length) in nibbles; data are 4-bit codes, low nibble first.
    In the scrambled layout each 256-byte page is permuted (see rom_byte).

    Code format (c = nibble, m = c & 7, sign = c & 8), fully verified against
    real-hardware recordings of Last Fighting and Xiao Ao Jiang Hu:

      step k starts at 1 for every sample
      delta = m * k + (k >> 1)       (code 0 moves the output by +k/2)
      acc  += sign ? -delta : delta
      k     = max(1, (k * {12,13,16,16,16,16,24,32}[m]) >> 4)

    i.e. an adaptive delta modulator whose 8-bit step is scaled by 3/4, 13/16,
    1, 1, 1, 1, 3/2 and 2 with truncation.  The encoder keeps the output within
    about +-600 and returns to zero at the end of each sample.

    Sample rate: 32 MHz / 4608 = 6944 Hz measured on Last Fighting (SS9804),
    44.1 MHz / 6144 = 7178 Hz measured on Xiao Ao Jiang Hu (SS9904).  Whether
    the divider depends on the chip type or on a strap is not known.

    The real output ramps about half way to the new value during the preceding
    sample period before stepping to it (a gentle low-pass); not emulated.

***************************************************************************/

#include "emu.h"
#include "ss9904.h"

#include <algorithm>

#define VERBOSE 0
#include "logmacro.h"


DEFINE_DEVICE_TYPE(SS9904, ss9904_device, "ss9904", "Subsino SS9904 Sound")
DEFINE_DEVICE_TYPE(SS9804, ss9804_device, "ss9804", "Subsino SS9804 Sound")


// scrambled layout: physical in-page offset, indexed by the transformed logical offset (see rom_byte)
constexpr u8 ss9904_device::s_scramble[256] =
{
	0x98, 0x18, 0xd8, 0x58, 0x9f, 0x1f, 0xdf, 0x5f, 0x93, 0x13, 0xd3, 0x53, 0x91, 0x11, 0xd1, 0x51,
	0x88, 0x08, 0xc8, 0x48, 0x8f, 0x0f, 0xcf, 0x4f, 0x83, 0x03, 0xc3, 0x43, 0x81, 0x01, 0xc1, 0x41,
	0xb8, 0x38, 0xf8, 0x78, 0xbf, 0x3f, 0xff, 0x7f, 0xb3, 0x33, 0xf3, 0x73, 0xb1, 0x31, 0xf1, 0x71,
	0xa8, 0x28, 0xe8, 0x68, 0xaf, 0x2f, 0xef, 0x6f, 0xa3, 0x23, 0xe3, 0x63, 0xa1, 0x21, 0xe1, 0x61,
	0x99, 0x19, 0xd9, 0x59, 0x9e, 0x1e, 0xde, 0x5e, 0x94, 0x14, 0xd4, 0x54, 0x96, 0x16, 0xd6, 0x56,
	0x89, 0x09, 0xc9, 0x49, 0x8e, 0x0e, 0xce, 0x4e, 0x84, 0x04, 0xc4, 0x44, 0x86, 0x06, 0xc6, 0x46,
	0xb9, 0x39, 0xf9, 0x79, 0xbe, 0x3e, 0xfe, 0x7e, 0xb4, 0x34, 0xf4, 0x74, 0xb6, 0x36, 0xf6, 0x76,
	0xa9, 0x29, 0xe9, 0x69, 0xae, 0x2e, 0xee, 0x6e, 0xa4, 0x24, 0xe4, 0x64, 0xa6, 0x26, 0xe6, 0x66,
	0x9a, 0x1a, 0xda, 0x5a, 0x9d, 0x1d, 0xdd, 0x5d, 0x92, 0x12, 0xd2, 0x52, 0x90, 0x10, 0xd0, 0x50,
	0x8a, 0x0a, 0xca, 0x4a, 0x8d, 0x0d, 0xcd, 0x4d, 0x82, 0x02, 0xc2, 0x42, 0x80, 0x00, 0xc0, 0x40,
	0xba, 0x3a, 0xfa, 0x7a, 0xbd, 0x3d, 0xfd, 0x7d, 0xb2, 0x32, 0xf2, 0x72, 0xb0, 0x30, 0xf0, 0x70,
	0xaa, 0x2a, 0xea, 0x6a, 0xad, 0x2d, 0xed, 0x6d, 0xa2, 0x22, 0xe2, 0x62, 0xa0, 0x20, 0xe0, 0x60,
	0x9b, 0x1b, 0xdb, 0x5b, 0x9c, 0x1c, 0xdc, 0x5c, 0x95, 0x15, 0xd5, 0x55, 0x97, 0x17, 0xd7, 0x57,
	0x8b, 0x0b, 0xcb, 0x4b, 0x8c, 0x0c, 0xcc, 0x4c, 0x85, 0x05, 0xc5, 0x45, 0x87, 0x07, 0xc7, 0x47,
	0xbb, 0x3b, 0xfb, 0x7b, 0xbc, 0x3c, 0xfc, 0x7c, 0xb5, 0x35, 0xf5, 0x75, 0xb7, 0x37, 0xf7, 0x77,
	0xab, 0x2b, 0xeb, 0x6b, 0xac, 0x2c, 0xec, 0x6c, 0xa5, 0x25, 0xe5, 0x65, 0xa7, 0x27, 0xe7, 0x67
};

// step multipliers in 1/16 units, indexed by code magnitude
constexpr u8 ss9904_device::s_step_mul[8] = { 12, 13, 16, 16, 16, 16, 24, 32 };


ss9904_device::ss9904_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 divider)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_busy_timer(nullptr)
	, m_divider(divider)
	, m_scramble_override(-1)
	, m_scrambled(false)
	, m_busy(false)
	, m_cmd(0)
	, m_argcnt(0)
	, m_argneed(0)
{
	std::fill(std::begin(m_args), std::end(m_args), 0);
}

ss9904_device::ss9904_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ss9904_device(mconfig, SS9904, tag, owner, clock, 6144)
{
}

ss9804_device::ss9804_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ss9904_device(mconfig, SS9804, tag, owner, clock, 4608)
{
}


void ss9904_device::device_start()
{
	m_stream = stream_alloc(0, 1, clock() / m_divider);
	m_busy_timer = timer_alloc(FUNC(ss9904_device::busy_done), this);

	save_item(NAME(m_scrambled));
	save_item(NAME(m_busy));
	save_item(NAME(m_cmd));
	save_item(NAME(m_args));
	save_item(NAME(m_argcnt));
	save_item(NAME(m_argneed));
	save_item(STRUCT_MEMBER(m_voice, playing));
	save_item(STRUCT_MEMBER(m_voice, pos));
	save_item(STRUCT_MEMBER(m_voice, remain));
	save_item(STRUCT_MEMBER(m_voice, step));
	save_item(STRUCT_MEMBER(m_voice, acc));
}

void ss9904_device::device_reset()
{
	m_scrambled = (m_scramble_override == 1);
	m_busy = false;
	m_busy_timer->adjust(attotime::never);
	m_cmd = 0;
	m_argcnt = m_argneed = 0;
	for (voice &v : m_voice)
	{
		v.playing = false;
		v.pos = v.remain = 0;
		v.step = 1;
		v.acc = 0;
	}
}

void ss9904_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / m_divider);
}

void ss9904_device::rom_bank_pre_change()
{
	m_stream->update();
}


//-------------------------------------------------
//  rom_byte - read the sample ROM, undoing the
//  per-page address scrambling when enabled
//-------------------------------------------------

u8 ss9904_device::rom_byte(u32 addr)
{
	if (!m_scrambled)
		return read_byte(addr);

	// the permutation depends on address bits 8-11 of the page
	const u32 page = addr >> 8;
	u8 x = addr & 0xff;
	if (BIT(page, 1))
		x = (x << 4) | (x >> 4);
	if (BIT(page, 2))
		x = bitswap<8>(x, 0, 1, 2, 3, 4, 5, 6, 7);
	if (BIT(page, 0) ^ BIT(page, 2) ^ BIT(page, 3))
		x ^= 0xff;
	return read_byte((page << 8) | s_scramble[x]);
}


//-------------------------------------------------
//  host interface
//-------------------------------------------------

u8 ss9904_device::read()
{
	m_stream->update();
	u8 status = 0;
	for (int ch = 0; ch < 4; ch++)
		if (m_voice[ch].playing)
			status |= 1 << ch;
	return status;
}

void ss9904_device::write(u8 data)
{
	// the host polls a handshake line and waits for it to drop after each byte
	m_busy = true;
	m_busy_timer->adjust(attotime::from_usec(20));

	if (m_argneed != 0)
	{
		m_args[m_argcnt++] = data;
		if (m_argcnt < m_argneed)
			return;
		m_argneed = 0;

		switch (m_cmd & 0xf0)
		{
		case 0x40:
			// 4E 00 10 52 E5/F5: configuration
			if (m_scramble_override < 0)
			{
				m_scrambled = !BIT(m_args[3], 4);
				logerror("init %02X %02X %02X %02X %02X -> %s sample ROM layout\n", m_cmd, m_args[0], m_args[1], m_args[2], m_args[3], m_scrambled ? "scrambled" : "plain");
			}
			break;

		case 0x80:
			key_on(m_cmd & 3, m_args[0], m_args[1]);
			break;
		}
		return;
	}

	switch (data & 0xf0)
	{
	case 0x00:
		key_off(data & 0x0f);
		break;

	case 0x10:
		// 1-wire EEPROM bit-banging shares the port
		break;

	case 0x40:
		m_cmd = data;
		m_argcnt = 0;
		m_argneed = 4;
		break;

	case 0x80:
		if ((data & 0x0c) != 0x08)
			logerror("unknown key on command %02X\n", data);
		m_cmd = data;
		m_argcnt = 0;
		m_argneed = 2;
		break;

	default:
		logerror("unknown command %02X\n", data);
		break;
	}
}

TIMER_CALLBACK_MEMBER(ss9904_device::busy_done)
{
	m_busy = false;
}

void ss9904_device::key_off(u8 mask)
{
	m_stream->update();
	for (int ch = 0; ch < 4; ch++)
		if (BIT(mask, ch))
			m_voice[ch].playing = false;
}

void ss9904_device::key_on(int ch, u8 page, u8 number)
{
	m_stream->update();

	const u32 entry = (u32(page) << 11) + (u32(number) << 3);
	u32 start = 0, length = 0;
	for (int i = 3; i >= 0; i--)
	{
		start = (start << 8) | rom_byte(entry + i);
		length = (length << 8) | rom_byte(entry + 4 + i);
	}

	voice &v = m_voice[ch];
	v.playing = (length != 0);
	v.pos = start;
	v.remain = length;
	v.step = 1;
	v.acc = 0;

	LOG("key on ch%d: page %02X sample %02X -> nibble %06X length %u\n", ch, page, number, start, length);
}


//-------------------------------------------------
//  sound_stream_update
//-------------------------------------------------

void ss9904_device::sound_stream_update(sound_stream &stream)
{
	for (voice &v : m_voice)
	{
		if (!v.playing)
			continue;

		for (int i = 0; i < stream.samples(); i++)
		{
			const u8 byte = rom_byte(v.pos >> 1);
			const u8 code = BIT(v.pos, 0) ? (byte >> 4) : (byte & 0x0f);
			const u32 m = code & 7;

			const s32 delta = m * v.step + (v.step >> 1);
			v.acc += BIT(code, 3) ? -delta : delta;

			// the encoder never leaves the 10/11-bit range, this is only a safety net
			v.acc = std::clamp(v.acc, -2048, 2047);

			v.step = std::max<u32>(1, std::min<u32>(255, (v.step * s_step_mul[m]) >> 4));

			stream.add_int(0, i, v.acc, 1024);

			v.pos++;
			if (--v.remain == 0)
			{
				v.playing = false;
				break;
			}
		}
	}
}
