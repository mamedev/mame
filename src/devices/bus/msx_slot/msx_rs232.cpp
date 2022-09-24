// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "msx_rs232.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_RS232, msx_slot_rs232_device, "msx_slot_msx_rs232", "MSX Internal RS-232C")


msx_slot_rs232_device::msx_slot_rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_rom_device(mconfig, MSX_SLOT_RS232, tag, owner, clock)
	, m_i8251(*this, "i8251")
	, m_i8253(*this, "i8253")
	, m_switch_port(*this, "SWITCH")
	, m_rs232(*this, "rs232")
	, m_irq_handler(*this)
	, m_irq_mask(0)
	, m_out2(false)
	, m_cts(false)
	, m_dcd(false)
	, m_ri(false)
{
}


static INPUT_PORTS_START(msx_rs232)
	PORT_START("SWITCH")
	PORT_CONFNAME(0x01, 0x01, "RS-232C is")
	PORT_CONFSETTING(0x00, "disabled")
	PORT_CONFSETTING(0x01, "enabled")
INPUT_PORTS_END


ioport_constructor msx_slot_rs232_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(msx_rs232);
}


void msx_slot_rs232_device::device_add_mconfig(machine_config &config)
{
	// Config from svi738

	I8251(config, m_i8251, 1.8432_MHz_XTAL);
	m_i8251->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_i8251->dtr_handler().set(m_rs232, FUNC(rs232_port_device::write_dtr));
	m_i8251->rts_handler().set(m_rs232, FUNC(rs232_port_device::write_rts));
	m_i8251->rxrdy_handler().set(*this, FUNC(msx_slot_rs232_device::rxrdy_w));

	PIT8253(config, m_i8253);
	m_i8253->set_clk<0>(1.8432_MHz_XTAL);
	m_i8253->set_clk<1>(1.8432_MHz_XTAL);
	m_i8253->set_clk<2>(1.8432_MHz_XTAL);
	m_i8253->out_handler<0>().set(m_i8251, FUNC(i8251_device::write_rxc));
	m_i8253->out_handler<1>().set(m_i8251, FUNC(i8251_device::write_txc));
	m_i8253->out_handler<2>().set(*this, FUNC(msx_slot_rs232_device::out2_w));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_i8251, FUNC(i8251_device::write_rxd));
	m_rs232->dcd_handler().set(*this, FUNC(msx_slot_rs232_device::dcd_w));
	// RI not connected on svi738!
	m_rs232->ri_handler().set(*this, FUNC(msx_slot_rs232_device::ri_w));
	m_rs232->cts_handler().set(*this, FUNC(msx_slot_rs232_device::cts_w));
	m_rs232->dsr_handler().set(m_i8251, FUNC(i8251_device::write_dsr));
}


void msx_slot_rs232_device::device_start()
{
	msx_slot_rom_device::device_start();

	m_irq_handler.resolve_safe();

	// Install IO read/write handlers
	io_space().install_readwrite_handler(0x80, 0x81, read8sm_delegate(*m_i8251, FUNC(i8251_device::read)), write8sm_delegate(*m_i8251, FUNC(i8251_device::write)));
	io_space().install_readwrite_handler(0x82, 0x82, read8sm_delegate(*this, FUNC(msx_slot_rs232_device::status_r)), write8sm_delegate(*this, FUNC(msx_slot_rs232_device::irq_mask_w)));
	io_space().install_readwrite_handler(0x84, 0x87, read8sm_delegate(*m_i8253, FUNC(pit8253_device::read)), write8sm_delegate(*m_i8253, FUNC(pit8253_device::write)));

	save_item(NAME(m_irq_mask));
	save_item(NAME(m_out2));
	save_item(NAME(m_cts));
	save_item(NAME(m_dcd));
	save_item(NAME(m_ri));
}


void msx_slot_rs232_device::irq_mask_w(offs_t offset, uint8_t data)
{
	// bit3 - timer interrupt from i8253 channel 2
	// bit2 - sync character detect / break detect
	// bit1 - transmit data ready
	// bit0 - receive data ready
	m_irq_mask = data;
}


uint8_t msx_slot_rs232_device::status_r(offs_t offset)
{
	// bit7 - CTS
	// bit6 - 8253 channel 2 output
	// bit3 - Switch detect (on HX22 and other Toshiba models(?))
	// bit1 - Ring indicator (NC on SVI738)
	// bit0 - Carrier detect

	uint8_t result = 0x00;


	if (BIT(m_irq_mask, 0))
	{
		// This should also check that RxReady is high
		if (m_switch_port->read())
		{
			result |= 0x08;
		}
	}
	if (m_cts)
		result |= 0x80;

	if (m_out2)
		result |= 0x40;

	if (m_ri)
		result |= 0x02;

	if (m_dcd)
		result |= 0x01;

	return result;
}

WRITE_LINE_MEMBER(msx_slot_rs232_device::out2_w)
{
	m_out2 = state;
}

WRITE_LINE_MEMBER(msx_slot_rs232_device::cts_w)
{
	m_cts = state;
	m_i8251->write_cts(state);
}

WRITE_LINE_MEMBER(msx_slot_rs232_device::dcd_w)
{
	m_dcd = state;
}

WRITE_LINE_MEMBER(msx_slot_rs232_device::ri_w)
{
	m_ri = state;
}

WRITE_LINE_MEMBER(msx_slot_rs232_device::rxrdy_w)
{
	m_irq_handler(state);
}
