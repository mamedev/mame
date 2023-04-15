// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************

    gew7.cpp

    Yamaha GEW2 (65c02-based)

***************************************************************************/

#include "emu.h"
#include "gew12.h"

DEFINE_DEVICE_TYPE(GEW12, gew12_device, "gew12", "Yamaha YMW728-F (GEW12)")

gew12_device::gew12_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m65c02_device(mconfig, GEW12, tag, owner, clock)
	, device_mixer_interface(mconfig, *this, 2)
	, m_in_cb(*this), m_out_cb(*this)
	, m_rom(*this, DEVICE_SELF)
	, m_bank(*this, "bank%u", 0U)
	, m_uart(*this, "uart")
{
	program_config.m_internal_map = address_map_constructor(FUNC(gew12_device::internal_map), this);
}


void gew12_device::device_add_mconfig(machine_config &config)
{
	// TODO: PCM

	GEW12_UART(config, m_uart, DERIVED_CLOCK(1, 1));
	m_uart->tx_irq_handler().set(FUNC(gew12_device::internal_irq<INTERNAL_IRQ_MIDI_TX>));
	m_uart->rx_irq_handler().set(FUNC(gew12_device::internal_irq<INTERNAL_IRQ_MIDI_RX>));
}

void gew12_device::device_start()
{
	m65c02_device::device_start();

	m_timer[0] = timer_alloc(FUNC(gew12_device::timer_tick), this);
	m_timer[1] = timer_alloc(FUNC(gew12_device::timer_tick), this);

	m_in_cb.resolve_all_safe(0);
	m_out_cb.resolve_all_safe();

	m_bank[0]->configure_entries(0, m_rom->bytes() >> 14, m_rom->base(), 1 << 14);
	m_bank[1]->configure_entries(0, m_rom->bytes() >> 14, m_rom->base(), 1 << 14);

	memset(m_port_data, 0, sizeof m_port_data);
	memset(m_port_ddr, 0, sizeof m_port_ddr);

	save_item(NAME(m_irq_pending));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_timer_count));
	save_item(NAME(m_port_data));
	save_item(NAME(m_port_ddr));
}

void gew12_device::device_reset()
{
	m65c02_device::device_reset();

	m_irq_pending = 0;
	m_irq_enable = 0;

	m_timer_count[0] = m_timer_count[1] = 0;

	// TODO: how are the timers enabled/disabled?
	const attotime period = clocks_to_attotime(48);
	m_timer[0]->adjust(period, 0, period);
	m_timer[1]->adjust(period, 1, period);
}


void gew12_device::internal_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();

	// TODO: sound registers
	map(0x1000, 0x103f).ram();

	map(0x1f00, 0x1f00).rw(FUNC(gew12_device::irq_stat_r), FUNC(gew12_device::irq_en_w));
	map(0x1f01, 0x1f01).w(FUNC(gew12_device::timer_stat_w));

	map(0x1f11, 0x1f11).r(m_uart, FUNC(gew12_uart_device::data_r));
	map(0x1f12, 0x1f12).r(m_uart, FUNC(gew12_uart_device::status_r));
	map(0x1f13, 0x1f13).w(m_uart, FUNC(gew12_uart_device::data_w));

	map(0x1f20, 0x1f23).w(FUNC(gew12_device::timer_count_w));
	map(0x1f24, 0x1f25).r(FUNC(gew12_device::timer_count_r));

	// TODO - only the last port (PL) is known for sure
	// on psr160/260, the rest seem to all be handled by built-in key/button scanning
	map(0x1f2a, 0x1f2f).rw(FUNC(gew12_device::port_r), FUNC(gew12_device::port_w));
	map(0x1f3a, 0x1f3f).rw(FUNC(gew12_device::port_ddr_r), FUNC(gew12_device::port_ddr_w));

	map(0x1f52, 0x1f52).lw8(NAME([this](uint8_t data) { m_bank[0]->set_entry(data); }));
	map(0x1f53, 0x1f53).nopw(); // bank 0 MSB? (always zero)
	map(0x1f54, 0x1f54).lw8(NAME([this](uint8_t data) { m_bank[1]->set_entry(data); }));
	map(0x1f55, 0x1f55).nopw(); // bank 1 MSB? (always zero)

	map(0x4000, 0x7fff).bankr(m_bank[0]);
	map(0x8000, 0xbfff).bankr(m_bank[1]);
	map(0xc000, 0xffff).rom().region(DEVICE_SELF, 0);
}


u8 gew12_device::irq_stat_r()
{
	return m_irq_pending & m_irq_enable;
}

void gew12_device::irq_en_w(u8 data)
{
	m_irq_enable = data;
	irq_update();
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
		set_input_line(M65C02_IRQ_LINE, ASSERT_LINE);
	else
		set_input_line(M65C02_IRQ_LINE, CLEAR_LINE);
}


void gew12_device::timer_stat_w(u8 data)
{
	if (BIT(data, 0))
		internal_irq(INTERNAL_IRQ_TIMER0, CLEAR_LINE);
	if (BIT(data, 1))
		internal_irq(INTERNAL_IRQ_TIMER1, CLEAR_LINE);
}

u8 gew12_device::timer_count_r(offs_t offset)
{
	// both timer IRQ handlers read the current count from the same registers
	// apparently reading timer 1 count if its IRQ is pending, else timer 0 count
	const unsigned timer = BIT(m_irq_pending, INTERNAL_IRQ_TIMER1);

	if (!BIT(offset, 0))
		return m_timer_count[timer];
	else
		return m_timer_count[timer] >> 8;
}

void gew12_device::timer_count_w(offs_t offset, u8 data)
{
	const unsigned timer = offset >> 1;

	if (!BIT(offset, 0))
		m_timer_count[timer] = (m_timer_count[timer] & 0xff00) | data;
	else
		m_timer_count[timer] = (m_timer_count[timer] & 0x00ff) | (data << 8);
}

TIMER_CALLBACK_MEMBER(gew12_device::timer_tick)
{
	if (!m_timer_count[param])
		internal_irq(INTERNAL_IRQ_TIMER0 + param, ASSERT_LINE);

	m_timer_count[param]--;
}


u8 gew12_device::port_r(offs_t offset)
{
	const u8 out_data = m_port_data[offset] & ~m_port_ddr[offset];
	return out_data | (m_in_cb[offset]() & m_port_ddr[offset]);
}

void gew12_device::port_w(offs_t offset, u8 data)
{
	m_port_data[offset] = data;

	const u8 out_data = m_port_data[offset] & ~m_port_ddr[offset];
	m_out_cb[offset](out_data | (m_in_cb[offset]() & m_port_ddr[offset]));
}

u8 gew12_device::port_ddr_r(offs_t offset)
{
	return m_port_ddr[offset];
}

void gew12_device::port_ddr_w(offs_t offset, u8 data)
{
	m_port_ddr[offset] = data;

	const u8 out_data = m_port_data[offset] & ~m_port_ddr[offset];
	m_out_cb[offset](out_data | (m_in_cb[offset]() & m_port_ddr[offset]));
}
