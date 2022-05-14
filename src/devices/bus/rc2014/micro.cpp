// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 Micro Module

****************************************************************************/

#include "emu.h"
#include "micro.h"

#include "cpu/z80/z80.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"

namespace {

//**************************************************************************
//  RC2014 Micro module
//  Module author: Spencer Owen
//**************************************************************************

class rc2014_micro : public device_t, public device_rc2014_card_interface
{
public:
	// construction/destruction
	rc2014_micro(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

private:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_resolve_objects() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	DECLARE_WRITE_LINE_MEMBER( clk_w ) { m_bus->clk_w(state); }
	DECLARE_WRITE_LINE_MEMBER( tx_w ) { m_bus->tx_w(state); }

	void addrmap_mem(address_map &map) { map.unmap_value_high(); }
	void addrmap_io(address_map &map) { map.unmap_value_high(); }

	required_device<z80_device> m_maincpu;
	required_device<acia6850_device> m_acia;
	required_memory_region m_rom;
	required_ioport m_rom_selector;

	std::unique_ptr<u8[]> m_ram;

	static constexpr XTAL MAIN_CLOCK = XTAL(7'372'800);
};

rc2014_micro::rc2014_micro(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_MICRO, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_acia(*this, "acia")
	, m_rom(*this, "rom")
	, m_rom_selector(*this, "JP2-JP4")
	, m_ram(nullptr)
{
}

void rc2014_micro::device_start()
{
	// Setup CPU
	m_bus->set_bus_clock(MAIN_CLOCK);
	m_maincpu->set_daisy_config(m_bus->get_daisy_chain());

	// Setup RAM
	m_ram = std::make_unique<u8[]>(0x8000);
	std::fill_n(m_ram.get(), 0x8000, 0xff);
	save_pointer(NAME(m_ram), 0x8000);
	m_bus->installer(AS_PROGRAM)->install_ram(0x8000, 0xffff, m_ram.get());

	// Setup ACIA
	// A15-A8 and A5-A1 not connected
	m_bus->installer(AS_IO)->install_readwrite_handler(0x80, 0x81, 0, 0xff3e, 0, read8sm_delegate(*m_acia, FUNC(acia6850_device::read)), write8sm_delegate(*m_acia, FUNC(acia6850_device::write)));
}

void rc2014_micro::device_reset()
{
	// Setup ROM
	m_bus->installer(AS_PROGRAM)->install_rom(0x0000, 0x1fff, 0x0000, m_rom->base() + (m_rom_selector->read() & 7) * 0x2000);
}

void rc2014_micro::device_resolve_objects()
{
	// Setup CPU
	m_bus->assign_installer(AS_PROGRAM, &m_maincpu->space(AS_PROGRAM));
	m_bus->assign_installer(AS_IO, &m_maincpu->space(AS_IO));
	m_bus->int_callback().append_inputline(m_maincpu, INPUT_LINE_IRQ0);

	// Setup ACIA
	m_bus->rx_callback().append(m_acia, FUNC(acia6850_device::write_rxd));
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void rc2014_micro::device_add_mconfig(machine_config &config)
{
	Z80(config, m_maincpu, MAIN_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &rc2014_micro::addrmap_mem);
	m_maincpu->set_addrmap(AS_IO, &rc2014_micro::addrmap_io);

	clock_device &clock(CLOCK(config, "clock", MAIN_CLOCK));
	clock.signal_handler().append(FUNC(rc2014_micro::clk_w));
	clock.signal_handler().append(m_acia, FUNC(acia6850_device::write_txc));
	clock.signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));

	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->txd_handler().append(FUNC(rc2014_micro::tx_w));
	m_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_acia->irq_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
}

static INPUT_PORTS_START( rc2014_micro_jumpers )
	// JP1 is used to enable power from USB-to-Serial cable
	PORT_START("JP2-JP4")
	PORT_CONFNAME( 0x7, 0x0, "ROM Bank (A13-A15)" )
	PORT_CONFSETTING( 0x0, "BASIC" )
	PORT_CONFSETTING( 0x1, "EMPTY1" )
	PORT_CONFSETTING( 0x2, "EMPTY2" )
	PORT_CONFSETTING( 0x3, "EMPTY3" )
	PORT_CONFSETTING( 0x4, "EMPTY4" )
	PORT_CONFSETTING( 0x5, "EMPTY5" )
	PORT_CONFSETTING( 0x6, "EMPTY6" )
	PORT_CONFSETTING( 0x7, "SCM" )
INPUT_PORTS_END

ioport_constructor rc2014_micro::device_input_ports() const
{
	return INPUT_PORTS_NAME( rc2014_micro_jumpers );
}

ROM_START(rc2014_rom)
	ROM_REGION( 0x10000, "rom",0 )
	ROM_LOAD( "r0000009.bin",    0x0000, 0x10000, CRC(3fb1ced7) SHA1(40a030b931ebe6cca654ce056c228297f245b057) )
ROM_END

const tiny_rom_entry *rc2014_micro::device_rom_region() const
{
	return ROM_NAME( rc2014_rom );
}

}
//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_MICRO, device_rc2014_card_interface, rc2014_micro, "rc2014_micro", "RC2014 Micro module")
