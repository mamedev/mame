// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * IBM 5080 Peripheral Adapter.
 *
 * Sources:
 *  - IBM RT PC Hardware Technical Reference, Volume III (84X0873), Second Edition (September 1986), International Business Machines Corporation 1986.
 *
 * TODO:
 *  - fix failing diagnostic
 *  - shared interrupts
 */

#include "emu.h"
#include "5080pa.h"

#include "bus/rs232/rs232.h"
#include "machine/ins8250.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

namespace {

class isa16_5080pa_device
	: public device_t
	, public device_isa16_card_interface
{
public:
	isa16_5080pa_device(machine_config const &mconfig, char const *const tag, device_t *owner, u32 clock)
		: device_t(mconfig, ISA16_5080PA, tag, owner, clock)
		, device_isa16_card_interface(mconfig, *this)
		, m_uart(*this, "uart%u", 0U)
		, m_port(*this, "port%c", 'a')
		, m_sw1(*this, "SW1")
		, m_sw2(*this, "SW2")
	{
	}

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void pio_map(address_map &map);

	u8 int_r();

	template <unsigned N> void irq_w(int state);
	void interrupt();

private:
	required_device_array<ns16450_device, 4> m_uart;
	required_device_array<rs232_port_device, 3> m_port;

	required_ioport m_sw1;
	required_ioport m_sw2;

	// interrupt state
	u8 m_int;
	bool m_int_state;

	// internal state
	u8 m_irq;
	bool m_installed;
};

void isa16_5080pa_device::device_add_mconfig(machine_config &config)
{
	NS16450(config, m_uart[0], 1.8432_MHz_XTAL).out_tx_callback().set(m_port[0], FUNC(rs232_port_device::write_txd));
	NS16450(config, m_uart[1], 1.8432_MHz_XTAL).out_tx_callback().set(m_port[1], FUNC(rs232_port_device::write_txd));
	NS16450(config, m_uart[2], 1.8432_MHz_XTAL).out_tx_callback().set(m_port[2], FUNC(rs232_port_device::write_txd));
	NS16450(config, m_uart[3], 1.8432_MHz_XTAL); // TODO: board photo shows 4th uart

	RS232_PORT(config, m_port[0]).rxd_handler().set(m_uart[0], FUNC(ns16450_device::rx_w));
	RS232_PORT(config, m_port[1]).rxd_handler().set(m_uart[1], FUNC(ns16450_device::rx_w));
	RS232_PORT(config, m_port[2]).rxd_handler().set(m_uart[2], FUNC(ns16450_device::rx_w));

	m_uart[0]->out_int_callback().set(FUNC(isa16_5080pa_device::irq_w<0>));
	m_uart[1]->out_int_callback().set(FUNC(isa16_5080pa_device::irq_w<1>));
	m_uart[2]->out_int_callback().set(FUNC(isa16_5080pa_device::irq_w<2>));
	m_uart[3]->out_int_callback().set(FUNC(isa16_5080pa_device::irq_w<3>)); // TODO: assumed
}

void isa16_5080pa_device::device_start()
{
	save_item(NAME(m_int));
	save_item(NAME(m_int_state));

	set_isa_device();

	m_int = 0xd0;
	m_int_state = false;

	m_irq = 0;
	m_installed = false;
}

void isa16_5080pa_device::device_reset()
{
	if (!m_installed)
	{
		switch (m_sw1->read())
		{
		case 0x01: m_irq = 9; break;  // TODO: shared interrupt enable 0x2f2
		case 0x02: m_irq = 10; break; // TODO: shared interrupt enable 0x6f2
		case 0x04: m_irq = 11; break; // TODO: shared interrupt enable 0x6f3
		}

		offs_t base = 0;
		switch (m_sw2->read())
		{
		case 0x01: base = 0x1230; break;
		case 0x02: base = 0x2230; break;
		case 0x04: base = 0x3230; break;
		case 0x08: base = 0x4230; break;
		}

		if (base)
			m_isa->install_device(base, base + 0x17, *this, &isa16_5080pa_device::pio_map);

		m_installed = true;
	}

	interrupt();
}

void isa16_5080pa_device::pio_map(address_map &map)
{
	map(0x00, 0x07).rw(m_uart[0], FUNC(ns16450_device::ins8250_r), FUNC(ns16450_device::ins8250_w));
	map(0x08, 0x0f).rw(m_uart[1], FUNC(ns16450_device::ins8250_r), FUNC(ns16450_device::ins8250_w));
	map(0x10, 0x17).rw(m_uart[2], FUNC(ns16450_device::ins8250_r), FUNC(ns16450_device::ins8250_w));
	map(0x18, 0x1f).rw(m_uart[3], FUNC(ns16450_device::ins8250_r), FUNC(ns16450_device::ins8250_w)); // TODO: assumed

	map(0x07, 0x07).r(FUNC(isa16_5080pa_device::int_r));
}

u8 isa16_5080pa_device::int_r()
{
	return m_int;
}

template <unsigned N> void isa16_5080pa_device::irq_w(int state)
{
	if (state)
		m_int |= 1U << N;
	else
		m_int &= ~(1U << N);

	interrupt();
}

void isa16_5080pa_device::interrupt()
{
	bool const int_state = bool(m_int & 0x0f);

	if (int_state != m_int_state)
	{
		LOG("interrupt %u\n", int_state);

		m_int_state = int_state;
		switch (m_irq)
		{
		case 9: m_isa->irq2_w(int_state ? ASSERT_LINE : CLEAR_LINE); break;
		case 10: m_isa->irq10_w(int_state ? ASSERT_LINE : CLEAR_LINE); break;
		case 11: m_isa->irq11_w(int_state ? ASSERT_LINE : CLEAR_LINE); break;
		}
	}
}

INPUT_PORTS_START(5080pa)
	PORT_START("SW1")
	PORT_DIPNAME(0x07, 0x04, "Interrupt Level Select") PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(0x01, "Level 9")
	PORT_DIPSETTING(0x02, "Level 10")
	PORT_DIPSETTING(0x04, "Level 11")

	PORT_START("SW2")
	PORT_DIPNAME(0x0f, 0x01, "Address Range") PORT_DIPLOCATION("SW2:!1,!2,!3,!4")
	PORT_DIPSETTING(0x01, "0x1230")
	PORT_DIPSETTING(0x02, "0x2230")
	PORT_DIPSETTING(0x04, "0x3230")
	PORT_DIPSETTING(0x08, "0x4230")
	PORT_DIPUNUSED(0xf0, 0x00) PORT_DIPLOCATION("SW2:!5,!6,!7,!8")
INPUT_PORTS_END

ioport_constructor isa16_5080pa_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(5080pa);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(ISA16_5080PA, device_isa16_card_interface, isa16_5080pa_device, "5080pa", "IBM 5080 Peripheral Adapter")
