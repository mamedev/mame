// license:BSD-3-Clause
// copyright-holders:cam900
/***************************************************************************

    Namco 163 internal sound emulation by cam900
    4 bit wavetable (variable length), 1 ~ 8 channel
    Reference : https://wiki.nesdev.com/w/index.php/Namco_163_audio

***************************************************************************/

#include "emu.h"
#include "namco_163.h"

#include <algorithm>


DEFINE_DEVICE_TYPE(NAMCO_163, namco_163_sound_device, "namco_163_sound", "Namco 163 (Sound)")

namco_163_sound_device::namco_163_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NAMCO_163, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_reg_addr(0x78)
	, m_addr(0)
	, m_inc(false)
	, m_disable(false)
	, m_stream(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_163_sound_device::device_start()
{
	std::fill(std::begin(m_ram), std::end(m_ram), 0);

	m_stream = stream_alloc(0, 1, clock() / 15);

	save_item(NAME(m_ram));
	save_item(NAME(m_reg_addr));
	save_item(NAME(m_addr));
	save_item(NAME(m_inc));
	save_item(NAME(m_disable));
}


//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

void namco_163_sound_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / 15);
}


inline s8 namco_163_sound_device::get_sample(u16 addr)
{
	return ((m_ram[(addr >> 1) & 0x7f] >> ((addr & 1) << 2)) & 0xf) - 8;
}


void namco_163_sound_device::disable_w(int state)
{
	m_disable = state;
}


/********************************************************************************/

/*
    Register Map (in RAM)

    40  ffff ffff   Channel 0 Frequency bits 0 - 7
    41  pppp pppp   Channel 0 Phase bits 0 - 7
    42  ffff ffff   Channel 0 Frequency bits 8 - 15
    43  pppp pppp   Channel 0 Phase bits 8 - 15
    44  ---- --ff   Channel 0 Frequency bits 16 - 17
        llll ll--   Channel 0 Waveform Length (256 - (l * 4)) 4 bit samples
    45  pppp pppp   Channel 0 Phase bits 16 - 23
    46  oooo oooo   Channel 0 Waveform Offset at 4 bit samples
    47  ---- cccc   Channel 0 Volume

    48  ffff ffff   Channel 1 Frequency bits 0 - 7
    49  pppp pppp   Channel 1 Phase bits 0 - 7
    4a  ffff ffff   Channel 1 Frequency bits 8 - 15
    4b  pppp pppp   Channel 1 Phase bits 8 - 15
    4c  ---- --ff   Channel 1 Frequency bits 16 - 17
        llll ll--   Channel 1 Waveform Length (256 - (l * 4)) 4 bit samples
    4d  pppp pppp   Channel 1 Phase bits 16 - 23
    4e  oooo oooo   Channel 1 Waveform Offset at 4 bit samples
    4f  ---- cccc   Channel 1 Volume

    .
    .
    .

    78  ffff ffff   Channel 7 Frequency bits 0 - 7
    79  pppp pppp   Channel 7 Phase bits 0 - 7
    7a  ffff ffff   Channel 7 Frequency bits 8 - 15
    7b  pppp pppp   Channel 7 Phase bits 8 - 15
    7c  ---- --ff   Channel 7 Frequency bits 16 - 17
        llll ll--   Channel 7 Waveform Length (256 - (l * 4)) 4 bit samples
    7d  pppp pppp   Channel 7 Phase bits 16 - 23
    7e  oooo oooo   Channel 7 Waveform Offset at 4 bit samples
    7f  ---- cccc   Channel 7 Volume
        -ccc ----   Enable channels
        -000 ----   Enable channel 7 only
        -001 ----   Enable channel 7, 6
        -010 ----   Enable channel 7, 6, 5

        .
        .
        .

        -111 ----   Enable all channels
*/

void namco_163_sound_device::addr_w(u8 data)
{
	m_inc = data & 0x80;
	m_addr = data & 0x7f;
}


void namco_163_sound_device::data_w(u8 data)
{
	m_stream->update();
	m_ram[m_addr] = data;
	if (m_inc)
		m_addr = (m_addr + 1) & 0x7f;
}


u8 namco_163_sound_device::data_r()
{
	const u8 val = m_ram[m_addr];
	if (!machine().side_effects_disabled() && m_inc)
		m_addr = (m_addr + 1) & 0x7f;

	return val;
}


void namco_163_sound_device::sound_stream_update(sound_stream &stream)
{
	if (m_disable)
		return;

	// Slightly noisy but closer to real hardware behavior
	for (int s = 0; s < stream.samples(); s++)
	{
		u32 phase = (m_ram[m_reg_addr + 5] << 16) | (m_ram[m_reg_addr + 3] << 8) | m_ram[m_reg_addr + 1];
		const u32 freq = ((m_ram[m_reg_addr + 4] & 0x3) << 16) | (m_ram[m_reg_addr + 2] << 8) | m_ram[m_reg_addr + 0];
		const u16 length = 256 - (m_ram[m_reg_addr + 4] & 0xfc);
		const u16 offset = m_ram[m_reg_addr + 6];
		const u8 vol = m_ram[m_reg_addr + 7] & 0xf;

		phase = (phase + freq) % (length << 16);
		s32 output = get_sample((phase >> 16) + offset) * vol;

		m_ram[m_reg_addr + 1] = phase & 0xff;
		m_ram[m_reg_addr + 3] = phase >> 8;
		m_ram[m_reg_addr + 5] = phase >> 16;

		m_reg_addr += 8;
		if (m_reg_addr >= 0x80)
		{
			m_reg_addr = 0x78 - ((m_ram[0x7f] & 0x70) >> 1);
		}
		stream.put_int(0, s, output, 128);
	}
}
