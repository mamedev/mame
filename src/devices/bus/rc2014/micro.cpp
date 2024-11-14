// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 Micro Module

****************************************************************************/

#include "emu.h"
#include "micro.h"

#include "modules.h"

#include "bus/ata/ataintf.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/6850acia.h"
#include "machine/clock.h"


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
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void card_int_w(int state) override { m_maincpu->set_input_line(INPUT_LINE_IRQ0, state); }
	virtual void card_rx_w(int state) override  { m_acia->write_rxd(state); }

	void clk_w(int state) { m_bus->clk_w(state); }
	void tx_w(int state) { m_bus->tx_w(state); }

	void addrmap_mem(address_map &map) { map.unmap_value_high(); }
	void addrmap_io(address_map &map) { map.unmap_value_high(); }

	required_device<z80_device> m_maincpu;
	required_device<acia6850_device> m_acia;
	required_memory_region m_rom;
	required_ioport m_rom_selector;
	required_ioport m_rom_present;

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
	, m_rom_present(*this, "ROM")
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
}

void rc2014_micro::device_reset()
{
	// Setup ACIA
	// A15-A8 and A5-A1 not connected
	m_bus->installer(AS_IO)->install_readwrite_handler(0x80, 0x81, 0, 0xff3e, 0, read8sm_delegate(*m_acia, FUNC(acia6850_device::read)), write8sm_delegate(*m_acia, FUNC(acia6850_device::write)));

	// Setup ROM
	if (m_rom_present->read())
		m_bus->installer(AS_PROGRAM)->install_rom(0x0000, 0x1fff, 0x0000, m_rom->base() + (m_rom_selector->read() & 7) * 0x2000);
}

void rc2014_micro::device_resolve_objects()
{
	// Setup CPU
	m_bus->assign_installer(AS_PROGRAM, &m_maincpu->space(AS_PROGRAM));
	m_bus->assign_installer(AS_IO, &m_maincpu->space(AS_IO));
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
	rs232.set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(terminal));
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
	PORT_START("ROM")
	PORT_CONFNAME( 0x1, 0x1, "ROM Socket" )
	PORT_CONFSETTING( 0x0, "Empty" )
	PORT_CONFSETTING( 0x1, "Populated" )
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

//**************************************************************************
//  RC2014 Mini CP/M Upgrade module
//  Module author: Spencer Owen
//**************************************************************************

class rc2014_mini_cpm : public device_t, public device_rc2014_card_interface
{
public:
	// construction/destruction
	rc2014_mini_cpm(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override { update_banks(); }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void tx_w(int state) { m_bus->tx_w(state); }

	uint8_t ide_cs0_r(offs_t offset) { return m_ata->cs0_r(offset); }
	void ide_cs0_w(offs_t offset, uint8_t data) { m_ata->cs0_w(offset, data); }
	void reset_bank_w(offs_t, uint8_t) { m_view_num = 0; update_banks(); }
	void toggle_bank_w(offs_t, uint8_t) { m_view_num = m_view_num ? 0 : 1; update_banks(); }

private:
	void update_banks() { m_view.select(m_view_num); }

	// base-class members
	uint8_t m_view_num;
	std::unique_ptr<u8[]> m_ram;

	required_device<ata_interface_device> m_ata;
	required_device<rc2014_bus_device> m_rc2014_bus;
	required_memory_region m_rom;
	required_ioport m_jp1;
	memory_view m_view;
	memory_bank_creator m_rombank;
	memory_bank_creator m_rambank;
};

rc2014_mini_cpm::rc2014_mini_cpm(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_MINI_CPM, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
	, m_ata(*this, "ata")
	, m_rc2014_bus(*this, ":bus")
	, m_rom(*this, "rom")
	, m_jp1(*this, "JP1-JP2")
	, m_view(*this, "view")
	, m_rombank(*this, "bank")
	, m_rambank(*this, "ram")
{
}

void rc2014_mini_cpm::device_start()
{
	// Additional 32K RAM
	m_ram = std::make_unique<u8[]>(0x8000);
	std::fill_n(m_ram.get(), 0x8000, 0xff);
	save_pointer(NAME(m_ram), 0x8000);
	save_item(NAME(m_view_num));

	// Install RAM/ROM
	m_bus->installer(AS_PROGRAM)->install_view(0x0000, 0x7fff, m_view);
	m_view[0].install_read_bank(0x0000, 0x3fff, m_rombank);
	m_view[0].install_write_bank(0x0000, 0x3fff, m_rambank);
	m_view[1].install_readwrite_bank(0x0000, 0x7fff, m_rambank);

	// Setup banks
	m_rombank->configure_entry(0, m_rom->base() + 0x0000);
	m_rombank->configure_entry(1, m_rom->base() + 0x4000);
	m_rombank->configure_entry(2, m_rom->base() + 0x8000);
	m_rombank->configure_entry(3, m_rom->base() + 0xc000);

	m_rambank->configure_entry(0, m_ram.get());
}

void rc2014_mini_cpm::device_reset()
{
	// A15-A8, A7 and A2-A0 not connected, A6 must be 0
	m_bus->installer(AS_IO)->install_write_handler(0x30, 0x30, 0, 0xff87, 0, write8sm_delegate(*this, FUNC(rc2014_mini_cpm::reset_bank_w)));
	m_bus->installer(AS_IO)->install_write_handler(0x38, 0x38, 0, 0xff87, 0, write8sm_delegate(*this, FUNC(rc2014_mini_cpm::toggle_bank_w)));
	// A15-A8 and A7 not connected
	m_bus->installer(AS_IO)->install_readwrite_handler(0x10, 0x17, 0, 0xff80, 0, read8sm_delegate(*this, FUNC(rc2014_mini_cpm::ide_cs0_r)), write8sm_delegate(*this, FUNC(rc2014_mini_cpm::ide_cs0_w)));

	// Set default banks
	m_view_num = 0;
	m_rombank->set_entry(m_jp1->read());
	update_banks();
}

void rc2014_mini_cpm::device_add_mconfig(machine_config &config)
{
	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, true);

	RC2014_SLOT(config, "1", m_rc2014_bus, rc2014_bus_modules, nullptr);
	RC2014_SLOT(config, "2", m_rc2014_bus, rc2014_bus_modules, nullptr);
}

static INPUT_PORTS_START( rc2014_mini_cpm_jumpers )
	PORT_START("JP1-JP2")
	PORT_CONFNAME( 0x3, 0x1, "Bank" )
	PORT_CONFSETTING( 0x0, "0" )
	PORT_CONFSETTING( 0x1, "1" )
	PORT_CONFSETTING( 0x2, "2" )
	PORT_CONFSETTING( 0x3, "3" )
INPUT_PORTS_END

ioport_constructor rc2014_mini_cpm::device_input_ports() const
{
	return INPUT_PORTS_NAME( rc2014_mini_cpm_jumpers );
}

ROM_START(rc2014_mini_cpm)
	ROM_REGION( 0x10000, "rom",0 )
	ROM_LOAD( "r0881099.bin", 0x00000, 0x10000, CRC(5619f399) SHA1(c96e9ebd6ce019c264aaea156532fcf807d4c74c) )
ROM_END

const tiny_rom_entry *rc2014_mini_cpm::device_rom_region() const
{
	return ROM_NAME( rc2014_mini_cpm );
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_MICRO, device_rc2014_card_interface, rc2014_micro, "rc2014_micro", "RC2014 Micro module")
DEFINE_DEVICE_TYPE_PRIVATE(RC2014_MINI_CPM, device_rc2014_card_interface, rc2014_mini_cpm, "rc2014_mini_cpm", "Mini CP/M Upgrade module")
