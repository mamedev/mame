// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Atomwide Serial Expansion Card

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/Atomwide_SerialExp.html

**********************************************************************/

#include "emu.h"
#include "serial.h"
#include "machine/ins8250.h"
#include "bus/rs232/rs232.h"


namespace {

// ======================> arc_serial_device

class arc_serial_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;

private:
	required_memory_region m_podule_rom;

	u8 m_rom_page;
};


void arc_serial_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0xf800)]; })).umask32(0x000000ff);
	map(0x2000, 0x201f).rw("uart2", FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w)).umask32(0x000000ff);
	map(0x2400, 0x241f).rw("uart1", FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w)).umask32(0x000000ff);
	map(0x2800, 0x281f).rw("uart0", FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w)).umask32(0x000000ff);
	map(0x3c00, 0x3c00).lw8(NAME([this](u8 data) { m_rom_page = data; }));
}


//-------------------------------------------------
//  ROM( serial )
//-------------------------------------------------

ROM_START( serial )
	ROM_REGION(0x10000, "podule_rom", 0)
	ROM_LOAD("atomwide_serial.rom", 0x0000, 0x10000, CRC(2dcf3b28) SHA1(c100cc4f5e3d87233e6db6a1da04ac7c19ac2357))
ROM_END

const tiny_rom_entry *arc_serial_device::device_rom_region() const
{
	return ROM_NAME( serial );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_serial_device::device_add_mconfig(machine_config &config)
{
	ns16550_device &uart0(NS16550(config, "uart0", 7.3728_MHz_XTAL));
	uart0.out_tx_callback().set("serial0", FUNC(rs232_port_device::write_txd));
	uart0.out_dtr_callback().set("serial0", FUNC(rs232_port_device::write_dtr));
	uart0.out_rts_callback().set("serial0", FUNC(rs232_port_device::write_rts));

	rs232_port_device &serial0(RS232_PORT(config, "serial0", default_rs232_devices, nullptr));
	serial0.rxd_handler().set("uart0", FUNC(ns16550_device::rx_w));
	serial0.dcd_handler().set("uart0", FUNC(ns16550_device::dcd_w));
	serial0.dsr_handler().set("uart0", FUNC(ns16550_device::dsr_w));
	serial0.cts_handler().set("uart0", FUNC(ns16550_device::cts_w));

	ns16550_device &uart1(NS16550(config, "uart1", 7.3728_MHz_XTAL));
	uart1.out_tx_callback().set("serial1", FUNC(rs232_port_device::write_txd));
	uart1.out_dtr_callback().set("serial1", FUNC(rs232_port_device::write_dtr));
	uart1.out_rts_callback().set("serial1", FUNC(rs232_port_device::write_rts));

	rs232_port_device &serial1(RS232_PORT(config, "serial1", default_rs232_devices, nullptr));
	serial1.rxd_handler().set("uart1", FUNC(ns16550_device::rx_w));
	serial1.dcd_handler().set("uart1", FUNC(ns16550_device::dcd_w));
	serial1.dsr_handler().set("uart1", FUNC(ns16550_device::dsr_w));
	serial1.cts_handler().set("uart1", FUNC(ns16550_device::cts_w));

	ns16550_device &uart2(NS16550(config, "uart2", 7.3728_MHz_XTAL));
	uart2.out_tx_callback().set("serial2", FUNC(rs232_port_device::write_txd));
	uart2.out_dtr_callback().set("serial2", FUNC(rs232_port_device::write_dtr));
	uart2.out_rts_callback().set("serial2", FUNC(rs232_port_device::write_rts));

	rs232_port_device &serial2(RS232_PORT(config, "serial2", default_rs232_devices, nullptr));
	serial2.rxd_handler().set("uart2", FUNC(ns16550_device::rx_w));
	serial2.dcd_handler().set("uart2", FUNC(ns16550_device::dcd_w));
	serial2.dsr_handler().set("uart2", FUNC(ns16550_device::dsr_w));
	serial2.cts_handler().set("uart2", FUNC(ns16550_device::cts_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_serial_device - constructor
//-------------------------------------------------

arc_serial_device::arc_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_SERIAL, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_rom_page(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_serial_device::device_start()
{
	save_item(NAME(m_rom_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_serial_device::device_reset()
{
	m_rom_page = 0x00;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_SERIAL, device_archimedes_podule_interface, arc_serial_device, "arc_serial", "Atomwide Serial Expansion Card")
