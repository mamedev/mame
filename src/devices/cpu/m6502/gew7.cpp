// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************

    gew7.cpp

    Yamaha GEW7, GEW7I, GEW7S (65c02-based)

***************************************************************************/

#include "emu.h"
#include "gew7.h"

#include "m6502mcu.ipp"

#include "bus/generic/slot.h"


DEFINE_DEVICE_TYPE(GEW7, gew7_device, "gew7", "Yamaha YMW270-F (GEW7)")

gew7_device::gew7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m6502_mcu_device_base<w65c02_device>(mconfig, GEW7, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_in_cb(*this, 0xff), m_out_cb(*this)
	, m_rom(*this, DEVICE_SELF)
	, m_bank(*this, "bank%u", 0U)
	, m_pcm(*this, "pcm")
{
	program_config.m_internal_map = address_map_constructor(FUNC(gew7_device::internal_map), this);

	std::fill(std::begin(m_port_force_bits), std::end(m_port_force_bits), 0);
	std::fill(std::begin(m_port_force_mask), std::end(m_port_force_mask), 0);
}


void gew7_device::device_add_mconfig(machine_config &config)
{
	GEW7_PCM(config, m_pcm, DERIVED_CLOCK(1, 1));
	m_pcm->set_device_rom_tag(m_rom);
	m_pcm->add_route(0, *this, 1.0, 0);
	m_pcm->add_route(1, *this, 1.0, 1);
}

void gew7_device::device_start()
{
	m6502_mcu_device_base<w65c02_device>::device_start();

	m_bank_mask = device_generic_cart_interface::map_non_power_of_two(
			unsigned(m_rom->bytes() >> 14),
			[this, base = &m_rom->as_u8()] (unsigned entry, unsigned page)
			{
				m_bank[0]->configure_entry(entry, &base[page << 14]);
				m_bank[1]->configure_entry(entry, &base[page << 14]);
			});

	m_timer_base[0] = m_timer_base[1] = 0;

	std::fill(std::begin(m_port_data), std::end(m_port_data), 0);
	std::fill(std::begin(m_port_ddr), std::end(m_port_ddr), 0);

	save_item(NAME(m_timer_stat));
	save_item(NAME(m_timer_en));
	save_item(NAME(m_timer_count));
	save_item(NAME(m_timer_base));
	save_item(NAME(m_port_data));
	save_item(NAME(m_port_ddr));
}

void gew7_device::device_reset()
{
	m6502_mcu_device_base<w65c02_device>::device_reset();

	internal_update();
	m_timer_stat = m_timer_en = 0;
	m_timer_count[0] = m_timer_count[1] = 0;
	internal_update();
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


void gew7_device::internal_update(u64 current_time)
{
	u64 event_time(0U);
	add_event(event_time, timer_update(0, current_time));
	add_event(event_time, timer_update(1, current_time));
	recompute_bcount(event_time);
}

u8 gew7_device::timer_stat_r()
{
	internal_update();
	return m_timer_stat;
}

void gew7_device::timer_stat_w(u8 data)
{
	internal_update();
	m_timer_stat &= ~data;

	if (!(m_timer_stat & 3))
		set_input_line(W65C02_IRQ_LINE, CLEAR_LINE);
	internal_update();
}

void gew7_device::timer_en_w(u8 data)
{
	internal_update();
	m_timer_en = data;
	internal_update();
}

u8 gew7_device::timer_count_r(offs_t offset)
{
	const unsigned timer = offset >> 1;

	internal_update();
	if (!BIT(offset, 0))
		return m_timer_count[timer];
	else
		return m_timer_count[timer] >> 8;
}

void gew7_device::timer_count_w(offs_t offset, u8 data)
{
	const unsigned timer = offset >> 1;

	internal_update();
	if (!BIT(offset, 0))
		m_timer_count[timer] = (m_timer_count[timer] & 0xff00) | data;
	else
		m_timer_count[timer] = (m_timer_count[timer] & 0x00ff) | (data << 8);
	internal_update();
}

u64 gew7_device::timer_update(int num, u64 current_time)
{
	static constexpr unsigned tick_rate = 48;

	const u64 ticks = current_time / tick_rate;
	const u64 elapsed = ticks - m_timer_base[num];
	m_timer_base[num] = ticks;

	if (!BIT(m_timer_en, num))
		return 0; // timer disabled

	if (elapsed > m_timer_count[num])
	{
		m_timer_stat |= (1 << num);
		set_input_line(W65C02_IRQ_LINE, ASSERT_LINE);
	}

	m_timer_count[num] -= elapsed;

	return (m_timer_base[num] + m_timer_count[num] + 1) * tick_rate;
}


void gew7_device::bank_w(offs_t offset, u8 data)
{
	m_bank[offset]->set_entry(data & m_bank_mask);
}


void gew7_device::port_force_bits(offs_t num, u8 bits, u8 mask)
{
	// Evidently (at least on the original YMW270F), tying a port pin to +5V or ground is expected
	// to affect the value read from the port even when the pin is configured as output.
	// PSS-11, PSS-21, PSS-31, and PSR-75 (which all use the same ROM) all rely on this in order to
	// correctly detect what model they are.
	m_port_force_bits[num] = bits;
	m_port_force_mask[num] = mask;
}

u8 gew7_device::port_r(offs_t offset)
{
	const u8 out_data = m_port_data[offset] & m_port_ddr[offset];
	const u8 in_data = m_in_cb[offset]() & ~m_port_ddr[offset];

	return ((out_data | in_data) & ~m_port_force_mask[offset]) | m_port_force_bits[offset];
}

void gew7_device::port_w(offs_t offset, u8 data)
{
	m_port_data[offset] = data;
	m_out_cb[offset](0, m_port_data[offset], m_port_ddr[offset]);
}

u8 gew7_device::port_ddr_r(offs_t offset)
{
	return m_port_ddr[offset];
}

void gew7_device::port_ddr_w(offs_t offset, u8 data)
{
	m_port_ddr[offset] = data;
	m_out_cb[offset](0, m_port_data[offset], m_port_ddr[offset]);
}
