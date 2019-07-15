// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Brigent BootBug card

  Debugger support card for creators of NuBus slot firmware and other
  early-boot/low-level software.

  Basically a serial card with an overachieving declaration ROM.

***************************************************************************/

#include "emu.h"
#include "bootbug.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/terminal.h"
#include "bus/rs232/null_modem.h"
#include "screen.h"

static void isa_com(device_slot_interface &device)
{
	device.option_add("terminal", SERIAL_TERMINAL);
	device.option_add("null_modem", NULL_MODEM);
}

#define BOOTBUG_ROM_REGION  "btbug_rom"

ROM_START( bootbug )
	ROM_REGION(0x10000, BOOTBUG_ROM_REGION, 0)
	ROM_DEFAULT_BIOS("bb15")
	ROM_SYSTEM_BIOS(0, "bb15", "BootBug v1.5")
	ROMX_LOAD( "bootbug1.5.bin", 0x000000, 0x010000, CRC(432badf0) SHA1(914ad4bb28946cac732cf8b178508b69e4c1aae2), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "bb13", "BootBug v1.3")
	ROMX_LOAD( "bootbug1.3.bin", 0x000000, 0x010000, CRC(2902a234) SHA1(c783d19a5e4c536e58e1e7e201ec47e8fb78d435), ROM_BIOS(1))
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(NUBUS_BOOTBUG, nubus_bootbug_device, "nb_btbug", "Brigent BootBug debugger card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nubus_bootbug_device::device_add_mconfig(machine_config &config)
{
	NS16450(config, m_uart, XTAL(1'843'200));
	m_uart->out_tx_callback().set("serport0", FUNC(rs232_port_device::write_txd));
	m_uart->out_dtr_callback().set("serport0", FUNC(rs232_port_device::write_dtr));
	m_uart->out_rts_callback().set("serport0", FUNC(rs232_port_device::write_rts));

	rs232_port_device &serport0(RS232_PORT(config, "serport0", isa_com, "terminal"));
	serport0.rxd_handler().set(m_uart, FUNC(ins8250_uart_device::rx_w));
	serport0.dcd_handler().set(m_uart, FUNC(ins8250_uart_device::dcd_w));
	serport0.dsr_handler().set(m_uart, FUNC(ins8250_uart_device::dsr_w));
	serport0.ri_handler().set(m_uart, FUNC(ins8250_uart_device::ri_w));
	serport0.cts_handler().set(m_uart, FUNC(ins8250_uart_device::cts_w));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nubus_bootbug_device::device_rom_region() const
{
	return ROM_NAME( bootbug );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_bootbug_device - constructor
//-------------------------------------------------

nubus_bootbug_device::nubus_bootbug_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nubus_bootbug_device(mconfig, NUBUS_BOOTBUG, tag, owner, clock)
{
}

nubus_bootbug_device::nubus_bootbug_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	m_uart(*this, "uart_0")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_bootbug_device::device_start()
{
	uint32_t slotspace;

	install_declaration_rom(this, BOOTBUG_ROM_REGION);

	slotspace = get_slotspace();

	nubus().install_device(slotspace, slotspace+0xff, read32_delegate(FUNC(nubus_bootbug_device::dev_r), this), write32_delegate(FUNC(nubus_bootbug_device::dev_w), this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_bootbug_device::device_reset()
{
}

WRITE32_MEMBER( nubus_bootbug_device::dev_w )
{
	m_uart->ins8250_w(offset, data & 0xff);
}

READ32_MEMBER( nubus_bootbug_device::dev_r )
{
	return m_uart->ins8250_r(offset);
}
