// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************

    gew7.cpp

    Yamaha GEW7, GEW7I, GEW7S (65c02-based)

***************************************************************************/

#include "emu.h"
#include "gew7.h"

DEFINE_DEVICE_TYPE(YMW270F, ymw270f_device, "ymw270f", "Yamaha YMW270-F (GEW7)")
DEFINE_DEVICE_TYPE(YMW276F, ymw276f_device, "ymw276f", "Yamaha YMW276-F (GEW7I)")
DEFINE_DEVICE_TYPE(YMW282F, ymw282f_device, "ymw282f", "Yamaha YMW282-F (GEW7S)")

gew7_device::gew7_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: m65c02_device(mconfig, type, tag, owner, clock)
	, device_mixer_interface(mconfig, *this, 2)
	, m_in_cb(*this), m_out_cb(*this)
	, m_rom(*this, DEVICE_SELF)
	, m_bank(*this, "bank%u", 0U)
	, m_pcm(*this, "pcm")
{
	program_config.m_internal_map = address_map_constructor(FUNC(gew7_device::internal_map), this);

	// GEW7 and GEW7S seem to have different behavior when reading input bits
	// (see gew7_device::port_r)
	m_ignore_in_ddr = true;
}

ymw270f_device::ymw270f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gew7_device(mconfig, YMW270F, tag, owner, clock)
{
}

ymw276f_device::ymw276f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gew7_device(mconfig, YMW276F, tag, owner, clock)
{
}

ymw282f_device::ymw282f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gew7_device(mconfig, YMW282F, tag, owner, clock)
{
	m_ignore_in_ddr = false;
}


void gew7_device::device_add_mconfig(machine_config &config)
{
	GEW7_PCM(config, m_pcm, DERIVED_CLOCK(1, 1));
	m_pcm->set_device_rom_tag(m_rom);
	m_pcm->add_route(0, *this, 1.0, AUTO_ALLOC_INPUT, 0);
	m_pcm->add_route(1, *this, 1.0, AUTO_ALLOC_INPUT, 1);
}

void gew7_device::device_start()
{
	m65c02_device::device_start();

	m_timer[0] = timer_alloc(FUNC(gew7_device::timer_tick), this);
	m_timer[1] = timer_alloc(FUNC(gew7_device::timer_tick), this);

	m_in_cb.resolve_all_safe(0);
	m_out_cb.resolve_all_safe();

	m_bank[0]->configure_entries(0, m_rom->bytes() >> 14, m_rom->base(), 1 << 14);
	m_bank[1]->configure_entries(0, m_rom->bytes() >> 14, m_rom->base(), 1 << 14);

	memset(m_port_data, 0, sizeof m_port_data);
	memset(m_port_ddr, 0, sizeof m_port_ddr);

	save_item(NAME(m_timer_stat));
	save_item(NAME(m_timer_count));
	save_item(NAME(m_port_data));
	save_item(NAME(m_port_ddr));
}

void gew7_device::device_reset()
{
	m65c02_device::device_reset();

	m_timer_stat = 0;
	m_timer_count[0] = m_timer_count[1] = 0;

	m_timer[0]->adjust(attotime::never);
	m_timer[1]->adjust(attotime::never);
}


void gew7_device::internal_map(address_map &map)
{
	map(0x0000, 0x005f).rw(m_pcm, FUNC(gew7_pcm_device::read), FUNC(gew7_pcm_device::write));
	map(0x0060, 0x00ff).mirror(0x0100).ram();
	map(0x0200, 0x022f).rw(m_pcm, FUNC(gew7_pcm_device::read_hi), FUNC(gew7_pcm_device::write_hi));
	map(0x0230, 0x023f).ram();

	map(0x0400, 0x0400).rw(FUNC(gew7_device::timer_stat_r), FUNC(gew7_device::timer_stat_w));
	map(0x0401, 0x0401).w(FUNC(gew7_device::timer_en_w));
	map(0x0402, 0x0405).rw(FUNC(gew7_device::timer_count_r), FUNC(gew7_device::timer_count_w));
	map(0x0408, 0x0409).w(FUNC(gew7_device::bank_w));
	map(0x040a, 0x040f).rw(FUNC(gew7_device::port_r), FUNC(gew7_device::port_w));
	map(0x0410, 0x0415).rw(FUNC(gew7_device::port_ddr_r), FUNC(gew7_device::port_ddr_w));

	map(0x4000, 0x7fff).bankr(m_bank[0]);
	map(0x8000, 0xbfff).bankr(m_bank[1]);
	map(0xc000, 0xffff).rom().region(DEVICE_SELF, 0);
}


u8 gew7_device::timer_stat_r()
{
	return m_timer_stat;
}

void gew7_device::timer_stat_w(u8 data)
{
	m_timer_stat &= ~data;

	if (!(m_timer_stat & 3))
		set_input_line(M65C02_IRQ_LINE, CLEAR_LINE);
}

void gew7_device::timer_en_w(u8 data)
{
	const attotime period = clocks_to_attotime(48);

	for (int i = 0; i < 2; i++)
	{
		if (BIT(data, i))
			m_timer[i]->adjust(period, i, period);
		else
			m_timer[i]->adjust(attotime::never);
	}
}

u8 gew7_device::timer_count_r(offs_t offset)
{
	const unsigned timer = offset >> 1;

	if (!BIT(offset, 0))
		return m_timer_count[timer];
	else
		return m_timer_count[timer] >> 8;
}

void gew7_device::timer_count_w(offs_t offset, u8 data)
{
	const unsigned timer = offset >> 1;

	if (!BIT(offset, 0))
		m_timer_count[timer] = (m_timer_count[timer] & 0xff00) | data;
	else
		m_timer_count[timer] = (m_timer_count[timer] & 0x00ff) | (data << 8);
}

TIMER_CALLBACK_MEMBER(gew7_device::timer_tick)
{
	if (!m_timer_count[param])
	{
		m_timer_stat |= (1 << param);
		set_input_line(M65C02_IRQ_LINE, ASSERT_LINE);
	}

	m_timer_count[param]--;
}


void gew7_device::bank_w(offs_t offset, u8 data)
{
	m_bank[offset]->set_entry(data & 0x1f);
}


u8 gew7_device::port_r(offs_t offset)
{
	const u8 out_data = m_port_data[offset] & m_port_ddr[offset];

	if (m_ignore_in_ddr) // GEW7 (and GEW7I?)
	{
		// ignore DDR for input, otherwise key scanning on PSS-21/31 and PSR-75 doesn't work
		// (sets bits 6 and 7 of port F to outputs then tries to read them as inputs)
		return out_data | m_in_cb[offset]();
	}
	else // GEW7S
	{
		// don't ignore DDR for input, otherwise key/button scanning on PSR-180 doesn't work
		return out_data | (m_in_cb[offset]() & ~m_port_ddr[offset]);
	}
}

void gew7_device::port_w(offs_t offset, u8 data)
{
	m_port_data[offset] = data;

	const u8 out_data = m_port_data[offset] & m_port_ddr[offset];
	m_out_cb[offset](out_data | (m_in_cb[offset]() & ~m_port_ddr[offset]));
}

u8 gew7_device::port_ddr_r(offs_t offset)
{
	return m_port_ddr[offset];
}

void gew7_device::port_ddr_w(offs_t offset, u8 data)
{
	m_port_ddr[offset] = data;

	const u8 out_data = m_port_data[offset] & m_port_ddr[offset];
	m_out_cb[offset](out_data | (m_in_cb[offset]() & ~m_port_ddr[offset]));
}
