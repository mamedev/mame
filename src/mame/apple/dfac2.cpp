// license:BSD-3-Clause
// copyright-holders:R. Belmont, Tim Douglas
/***************************************************************************

    dfac2.cpp - Apple Digitally Filtered Audio Chip, which controls input and
              output gain and filtering.

    DFAC2 is a newer version of the DFAC chip which uses a true I2C interface.

    Inputs are:
    - a digital stereo signal from the sound chip with bit and sample clocks
    - a mono microphone input
    - a mono modem input
    - a general purpose stereo input
    - analog audio from the CD-ROM drive

    Outputs are:
    - an analog monoural internal speaker
    - a stereo line level output
    - a mono recording signal to the sound chip

***************************************************************************/

#include "emu.h"
#include "dfac2.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <string>

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(APPLE_DFAC2, dfac2_device, "apldfac2", "Apple Digitally Filtered Audio Chip II")

static constexpr double atten_table[8] =
{
	0.0,                    // -infinite
	0.125892541179417,      // -18dB
	0.177827941003892,      // -15dB
	0.251188643150958,      // -12dB
	0.354813389233575,      // -9dB
	0.501187233627272,      // -6dB
	0.707945784384138,      // -3dB
	1.0                     // No attenuation
};

// Which bits of each register are writable on a Performa 6200CD
static constexpr u8 WRITE_MASK[0x11] =
{
	0x3f, 0x00, 0x9f, 0x00, 0xff, 0x00, 0xff, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0xf7, 0xff, 0xc7, 0xf3,
	0x00
};

dfac2_device::dfac2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, APPLE_DFAC2, tag, owner, clock),
	device_mixer_interface(mconfig, *this),
	i2c_hle_interface(mconfig, *this, 0x6f),
	m_dac(*this, "dac_%u", 0),
	m_filter(*this, "filter3_%u", 0),
	m_rec(*this, "rec_%u", 0)
{
}

void dfac2_device::device_add_mconfig(machine_config &config)
{
	for (u32 ch = 0; ch < 2; ch++)
	{
		const std::string filter1 = util::string_format("filter1_%u", ch);
		const std::string filter2 = util::string_format("filter2_%u", ch);
		FILTER_VOLUME(config, m_dac[ch]).add_route(0, *this, 1.0, ch);
		m_dac[ch]->add_route(0, filter1.c_str(), 1.0);
		// The measured 7kHz filter, approximated by a fifth-order Chebyshev low-pass with 0.5dB ripple and a 6750Hz passband edge
		FILTER_BIQUAD(config, filter1.c_str()).setup(filter_biquad_device::biquad_type::LOWPASS1P1Z, 2445.7, 0.5, 1.0)
			.add_route(0, filter2.c_str(), 1.0);
		FILTER_BIQUAD(config, filter2.c_str()).setup(filter_biquad_device::biquad_type::LOWPASS, 4660.8, 1.1778, 1.0)
			.add_route(0, m_filter[ch], 1.0);
		FILTER_BIQUAD(config, m_filter[ch]).setup(filter_biquad_device::biquad_type::LOWPASS, 6869.7, 4.5450, 1.0)
			.add_route(0, *this, 1.0, ch);
	}

	for (u32 i = 0; i < 4; i++)
		FILTER_VOLUME(config, m_rec[i]);
}

void dfac2_device::device_start()
{
	save_item(NAME(m_regs));
}

void dfac2_device::device_reset()
{
	// Values measured on a Performa 6200CD after a warm restart, minus the AGC bit the ROM sets in $02.
	// $10 is 0x10 when nothing is plugged into either jack.
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	m_regs[0x00] = 0x20;
	m_regs[0x02] = 0x16;
	m_regs[0x04] = 0x1a;
	m_regs[0x06] = 0x16;
	m_regs[0x08] = 0x00;
	m_regs[0x0a] = 0x10;
	m_regs[0x10] = 0x10;

	update_controls();
}

void dfac2_device::update_controls()
{
	const bool mute_all = BIT(m_regs[0x0e], 7);
	const bool mute_l = mute_all || BIT(m_regs[0x0d], 6);
	const bool mute_r = mute_all || BIT(m_regs[0x0d], 7);

	double gain = ((m_regs[0x0d] & 3) == 2) ? atten_table[m_regs[0x0c] & 7] : 0.0;
	if (BIT(m_regs[0x0d], 2))
		gain *= 0.5;
	if (BIT(m_regs[0x0d], 3))
		gain *= 0.5;

	const bool filter = BIT(m_regs[0x0f], 7);
	for (u32 ch = 0; ch < 2; ch++)
	{
		m_dac[ch]->set_gain(gain * ((ch && BIT(m_regs[0x0c], 7)) ? atten_table[4] : 1.0));
		m_dac[ch]->set_route_gain(0, this, ch, filter ? 0.0 : 1.0);
		m_filter[ch]->set_output_gain(0, filter ? 1.0 : 0.0);
		set_output_gain(ch, (ch ? mute_r : mute_l) ? 0.0 : 1.0);
	}

	const u8 source = m_regs[0x0e] & 7;
	const double rec_gain = 0.5 * std::pow(10.0, (1.5 * (m_regs[0x08] & 0x1f) - 8.0) / 20.0);
	for (u32 i = 0; i < 4; i++)
		m_rec[i]->set_gain((source == ((i < 2) ? 4 : 3)) ? rec_gain : 0.0);
}

/*
    Reverse-engineering DFAC2 is a work in progress.

    Known registers. The odd ones below 0x0c and any above 0x10 aren't present. The input
    side is programmed per input source by the Mac ROM's 68k sound layer, and only the
    source select and 0x08 are emulated.
    0x00    - 6 writable bits, never written by the ROM or Mac OS
    0x02    - bit 7 AGC, set by the ROM when the ASC's $806 is writable; bits 4-0 a
              per-source setting, 0x16
    0x04    - per-source setting, 0x1a
    0x06    - per-source setting, 0x16; the ROM writes 0x02 on this chip family
    0x08    - second input gain stage: the ROM writes the 0x0a step plus a per-source offset
              here, and the same value to ASC $802. 1.5dB per step, -8dB at 0, mixed to mono,
              measured through the output loopback on a Performa 6200CD
    0x0a    - input gain, 0-31, for Sound Input Manager siInputGain 0.5-1.5; 0x10 for the
              internal mic and 0x17 for an external one on the Color Classic
    0x0c    - bits 2-0 output attenuator, 3dB per step, 0 = mute, 7 = full
              bit 7 attenuates the right side by a further 9dB
    0x0d    - bits 1-0 must be 10 for output; bit 2 playthrough, which attenuates the output
              by 6dB and mixes in the input, though the input mix isn't emulated; bit 3
              attenuates both sides by 6dB; bits 6/7 mute left/right
    0x0e    - bits 2-0 input source: 7 = none, the idle default; 3 = the sound input jack,
              with mic or line gain; 4 = the CD-ROM drive; 2 = the video input module; 5/6 =
              the left/right output, not emulated. Bit 7 mutes both sides
    0x0f    - bit 7 switches in the 7kHz output filter; the other bits are input settings
    0x10    - read only: bit 5 = something plugged into sound out, bit 7 = sound in
*/

bool dfac2_device::one_byte_transfers() const
{
	return true;
}

bool dfac2_device::subaddress_valid(u8 subaddress) const
{
	return (subaddress < 0x0c) ? !(subaddress & 1) : (subaddress < std::size(m_regs));
}

u8 dfac2_device::read_data(u16 offset)
{
	return m_regs[offset];
}

void dfac2_device::write_data(u16 offset, u8 data)
{
	LOG("DFAC2: write %02x to %04x (%s)\n", data, offset, machine().describe_context());

	m_regs[offset] = (m_regs[offset] & ~WRITE_MASK[offset]) | (data & WRITE_MASK[offset]);
	if ((offset == 0x08) || ((offset >= 0x0c) && (offset <= 0x0f)))
		update_controls();
}
