// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

    HAL PCG-8100

    Used on a bunch of PC-8001 games, adds a Programmable Character Generator overlay,
    a write-only 8253 with channels tied to three DAC1BIT

**************************************************************************************************/

#include "emu.h"
#include "pcg8100.h"


DEFINE_DEVICE_TYPE(PCG8100, pcg8100_device, "pcg8100", "HAL Laboratory PCG-8100")

pcg8100_device::pcg8100_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc8801_exp_device(mconfig, PCG8100, tag, owner, clock)
	, m_pit(*this, "pit")
	, m_dac1bit(*this, "dac1bit%u", 0L)
	, m_cg_rom(*this, "^^cgrom")
{
}

void pcg8100_device::io_map(address_map &map)
{
	map(0x00, 0x00).lw8(NAME([this] (u8 data) { m_pcg_data = data; }));
	map(0x01, 0x01).lw8(NAME([this] (u8 data) { m_pcg_address = (m_pcg_address & 0x300) | data; }));
	map(0x02, 0x02).w(FUNC(pcg8100_device::pcg_latch_w));
	map(0x0c, 0x0f).w(m_pit, FUNC(pit8253_device::write));
}

void pcg8100_device::device_add_mconfig(machine_config &config)
{
	constexpr XTAL pcg_clock = XTAL(31'948'800) / 8;

	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(pcg_clock);
	m_pit->out_handler<0>().set(m_dac1bit[0], FUNC(speaker_sound_device::level_w));
	m_pit->set_clk<1>(pcg_clock);
	m_pit->out_handler<1>().set(m_dac1bit[1], FUNC(speaker_sound_device::level_w));
	m_pit->set_clk<2>(pcg_clock);
	m_pit->out_handler<2>().set(m_dac1bit[2], FUNC(speaker_sound_device::level_w));

	for (auto &dac1bit : m_dac1bit)
	{
		SPEAKER_SOUND(config, dac1bit);
		dac1bit->add_route(ALL_OUTPUTS, "^^speaker", 0.25, 0);
		dac1bit->add_route(ALL_OUTPUTS, "^^speaker", 0.25, 1);
	}
}

void pcg8100_device::device_start()
{
	save_item(NAME(m_pcg_data));
	save_item(NAME(m_pcg_address));

	// PCG overlays directly to the internal PC-8001 3301 CG ROM base.
	// Save a local copy of the original data here, card can eventually restore data at later time.
	for (int i = 0x000; i < 0x400; i++)
		m_original_rom[i] = m_cg_rom[i | 0x400];

	save_item(NAME(m_original_rom));
}

void pcg8100_device::device_reset()
{
	int i;
	m_pcg_data = 0;
	m_pcg_address = 0;
	m_pcg_latch = 0;

	for (i = 0x000; i < 0x400; i++)
		m_cg_rom[i | 0x400] = m_original_rom[i];

	for (i = 0; i < 3; i++)
		audio_channel(i, false);
}

void pcg8100_device::audio_channel(u8 ch, bool keyon)
{
	m_dac1bit[ch]->set_output_gain(0, keyon ? 1.0 : 0.0);
}

/*
 * x--- ---- DAC1BIT #2
 * -x-- ---- DAC1BIT #1
 * --x- ---- (0) select overlay on latch (1) select PC-8001/PC-8801 original CG ROM
 * ---x ---- latch data into current PCG address RAM
 * ---- x--- DAC1BIT #0
 * ---- --xx upper PCG address
 */
void pcg8100_device::pcg_latch_w(u8 data)
{
	if (BIT(m_pcg_latch, 4) && !(BIT(data, 4)))
	{
		if (BIT(m_pcg_latch, 5))
			m_cg_rom[0x400 | m_pcg_address] = m_original_rom[m_pcg_address];
		else
			m_cg_rom[0x400 | m_pcg_address] = m_pcg_data;
	}
	m_pcg_address = (m_pcg_address & 0xff) | ((data & 0x3) << 8);
	m_pcg_latch = data;

	audio_channel(0, bool(BIT(data, 3)));
	audio_channel(1, bool(BIT(data, 6)));
	audio_channel(2, bool(BIT(data, 7)));
}
