// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************

    gew7.cpp

    Yamaha GEW2 (65c02-based)

***************************************************************************/

#include "emu.h"
#include "gew12.h"

#include "m6502mcu.ipp"

#include "bus/generic/slot.h"


DEFINE_DEVICE_TYPE(GEW12, gew12_device, "gew12", "Yamaha YMW728-F (GEW12)")

gew12_device::gew12_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m6502_mcu_device_base<w65c02_device>(mconfig, GEW12, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_in_cb(*this, 0xff), m_out_cb(*this)
	, m_rom(*this, DEVICE_SELF)
	, m_bank(*this, "bank%u", 0U)
{
	program_config.m_internal_map = address_map_constructor(FUNC(gew12_device::internal_map), this);
}


void gew12_device::device_add_mconfig(machine_config &config)
{
	// TODO: PCM, UART
}

void gew12_device::device_start()
{
	w65c02_device::device_start();

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

	save_item(NAME(m_irq_pending));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_timer_count));
	save_item(NAME(m_timer_base));
	save_item(NAME(m_port_data));
	save_item(NAME(m_port_ddr));
}

void gew12_device::device_reset()
{
	m6502_mcu_device_base<w65c02_device>::device_reset();

	internal_update();
	m_irq_pending = 0;
	m_irq_enable = 0;

	m_timer_count[0] = m_timer_count[1] = 0;
	internal_update();
}


void gew12_device::internal_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();

	// TODO: sound registers
	map(0x1000, 0x103f).ram();

	map(0x1f00, 0x1f00).rw(FUNC(gew12_device::irq_stat_r), FUNC(gew12_device::irq_en_w));
	map(0x1f01, 0x1f01).w(FUNC(gew12_device::timer_stat_w));

	// TODO: UART
	// map(0x1f11, 0x1f11) Rx data
	// map(0x1f12, 0x1f12) status (bits 0-1: error, bit 2: Rx ready, bit 3: Tx ready)
	// map(0x1f13, 0x1f13) Tx data

	map(0x1f20, 0x1f23).w(FUNC(gew12_device::timer_count_w));
	map(0x1f24, 0x1f25).r(FUNC(gew12_device::timer_count_r));

	// TODO - only the last port (PL) is known for sure
	// on psr160/260, the rest seem to all be handled by built-in key/button scanning
	map(0x1f2a, 0x1f2f).rw(FUNC(gew12_device::port_r), FUNC(gew12_device::port_w));
	map(0x1f3a, 0x1f3f).rw(FUNC(gew12_device::port_ddr_r), FUNC(gew12_device::port_ddr_w));

	map(0x1f52, 0x1f52).lw8(NAME([this](uint8_t data) { m_bank[0]->set_entry(data & m_bank_mask); }));
	map(0x1f53, 0x1f53).nopw(); // bank 0 MSB? (always zero)
	map(0x1f54, 0x1f54).lw8(NAME([this](uint8_t data) { m_bank[1]->set_entry(data & m_bank_mask); }));
	map(0x1f55, 0x1f55).nopw(); // bank 1 MSB? (always zero)

	map(0x4000, 0x7fff).bankr(m_bank[0]);
	map(0x8000, 0xbfff).bankr(m_bank[1]);
	map(0xc000, 0xffff).rom().region(DEVICE_SELF, 0);
}


void gew12_device::internal_update(u64 current_time)
{
	u64 event_time(0U);
	add_event(event_time, timer_update(0, current_time));
	add_event(event_time, timer_update(1, current_time));
	recompute_bcount(event_time);
}


u8 gew12_device::irq_stat_r()
{
	internal_update();
	return m_irq_pending & m_irq_enable;
}

void gew12_device::irq_en_w(u8 data)
{
	internal_update();
	m_irq_enable = data;
	irq_update();
	internal_update();
}

void gew12_device::internal_irq(int num, int state)
{
	if (state == ASSERT_LINE)
		m_irq_pending |= (1 << num);
	else
		m_irq_pending &= ~(1 << num);

	irq_update();
}

void gew12_device::irq_update()
{
	if (m_irq_pending & m_irq_enable)
		set_input_line(W65C02_IRQ_LINE, ASSERT_LINE);
	else
		set_input_line(W65C02_IRQ_LINE, CLEAR_LINE);
}


void gew12_device::timer_stat_w(u8 data)
{
	internal_update();
	if (BIT(data, 0))
		internal_irq(INTERNAL_IRQ_TIMER0, CLEAR_LINE);
	if (BIT(data, 1))
		internal_irq(INTERNAL_IRQ_TIMER1, CLEAR_LINE);
	internal_update();
}

u8 gew12_device::timer_count_r(offs_t offset)
{
	// both timer IRQ handlers read the current count from the same registers
	// apparently reading timer 1 count if its IRQ is pending, else timer 0 count
	internal_update();
	const unsigned timer = BIT(m_irq_pending, INTERNAL_IRQ_TIMER1);

	if (!BIT(offset, 0))
		return m_timer_count[timer];
	else
		return m_timer_count[timer] >> 8;
}

void gew12_device::timer_count_w(offs_t offset, u8 data)
{
	const unsigned timer = offset >> 1;

	internal_update();
	if (!BIT(offset, 0))
		m_timer_count[timer] = (m_timer_count[timer] & 0xff00) | data;
	else
		m_timer_count[timer] = (m_timer_count[timer] & 0x00ff) | (data << 8);
	internal_update();
}

u64 gew12_device::timer_update(int num, u64 current_time)
{
	static constexpr unsigned tick_rate = 48;

	const u64 ticks = current_time / tick_rate;
	const u64 elapsed = ticks - m_timer_base[num];
	m_timer_base[num] = ticks;

	// TODO: how are the timers enabled/disabled?
	// for now, just have 'timer IRQ enable' double as 'timer enable'
	if (!BIT(m_irq_enable, INTERNAL_IRQ_TIMER0 + num))
		return 0;

	if (elapsed > m_timer_count[num])
		internal_irq(INTERNAL_IRQ_TIMER0 + num, ASSERT_LINE);

	m_timer_count[num] -= elapsed;

	return (m_timer_base[num] + m_timer_count[num] + 1) * tick_rate;
}


u8 gew12_device::port_r(offs_t offset)
{
	const u8 out_data = m_port_data[offset] & ~m_port_ddr[offset];
	return out_data | (m_in_cb[offset]() & m_port_ddr[offset]);
}

void gew12_device::port_w(offs_t offset, u8 data)
{
	m_port_data[offset] = data;
	m_out_cb[offset](0, m_port_data[offset], ~m_port_ddr[offset]);
}

u8 gew12_device::port_ddr_r(offs_t offset)
{
	return m_port_ddr[offset];
}

void gew12_device::port_ddr_w(offs_t offset, u8 data)
{
	m_port_ddr[offset] = data;
	m_out_cb[offset](0, m_port_data[offset], ~m_port_ddr[offset]);
}
