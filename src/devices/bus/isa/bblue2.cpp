// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * Microlog Baby Blue II CPU Plus
 *
 * ISA card used to emulate CP/M-80 on a PC or XT
 * Includes a Z80B CPU, 2 DB25 RS-232C serial ports (8250), a Centronics parallel port, and an RTC
 *
 * Issues:
 *   RTC type is unknown
 *   Z80 location test and Interrupt test sometimes fail
 *   Parallel port test fails if set to LPT2
 */

#include "emu.h"

#include "bblue2.h"
#include "bus/rs232/terminal.h"
#include "bus/rs232/null_modem.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_BABYBLUE2, isa8_babyblue2_device, "isa_bblue2", "Microlog Baby Blue II CPU Plus")

static void isa_com(device_slot_interface &device)
{
//  device.option_add("microsoft_mouse", MSFT_HLE_SERIAL_MOUSE);
//  device.option_add("logitech_mouse", LOGITECH_HLE_SERIAL_MOUSE);
//  device.option_add("wheel_mouse", WHEEL_HLE_SERIAL_MOUSE);
//  device.option_add("msystems_mouse", MSYSTEMS_HLE_SERIAL_MOUSE);
//  device.option_add("rotatable_mouse", ROTATABLE_HLE_SERIAL_MOUSE);
	device.option_add("terminal", SERIAL_TERMINAL);
	device.option_add("null_modem", NULL_MODEM);
//  device.option_add("sun_kbd", SUN_KBD_ADAPTOR);
}

void isa8_babyblue2_device::z80_program_map(address_map &map)
{
	map(0x0000, 0xffff).ram().rw(FUNC(isa8_babyblue2_device::z80_ram_r),FUNC(isa8_babyblue2_device::z80_ram_w));
}

void isa8_babyblue2_device::z80_io_map(address_map &map)
{
	map(0x0030, 0x0037).rw(m_serial2, FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	// 0x40-0x47 - RTC
	map(0x0070, 0x0077).rw(m_serial1, FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	// LPT mapped at reset
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_babyblue2_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_z80, 6_MHz_XTAL);
	m_z80->set_addrmap(AS_PROGRAM, &isa8_babyblue2_device::z80_program_map);
	m_z80->set_addrmap(AS_IO, &isa8_babyblue2_device::z80_io_map);

	RAM(config, m_ram);
	m_ram->set_default_size("64k");

	INS8250(config, m_serial1, XTAL(1'843'200));
	m_serial1->out_tx_callback().set("rs232_1", FUNC(rs232_port_device::write_txd));
	m_serial1->out_dtr_callback().set("rs232_1", FUNC(rs232_port_device::write_dtr));
	m_serial1->out_rts_callback().set("rs232_1", FUNC(rs232_port_device::write_rts));
	m_serial1->out_int_callback().set(FUNC(isa8_babyblue2_device::port1_irq));
	INS8250(config, m_serial2, XTAL(1'843'200));
	m_serial2->out_tx_callback().set("rs232_2", FUNC(rs232_port_device::write_txd));
	m_serial2->out_dtr_callback().set("rs232_2", FUNC(rs232_port_device::write_dtr));
	m_serial2->out_rts_callback().set("rs232_2", FUNC(rs232_port_device::write_rts));
	m_serial2->out_int_callback().set(FUNC(isa8_babyblue2_device::port2_irq));

	RS232_PORT(config, m_rs232_1, isa_com, nullptr);
	m_rs232_1->rxd_handler().set(m_serial1, FUNC(ins8250_uart_device::rx_w));
	m_rs232_1->dcd_handler().set(m_serial1, FUNC(ins8250_uart_device::dcd_w));
	m_rs232_1->dsr_handler().set(m_serial1, FUNC(ins8250_uart_device::dsr_w));
	m_rs232_1->ri_handler().set(m_serial1, FUNC(ins8250_uart_device::ri_w));
	m_rs232_1->cts_handler().set(m_serial1, FUNC(ins8250_uart_device::cts_w));
	RS232_PORT(config, m_rs232_2, isa_com, nullptr);
	m_rs232_2->rxd_handler().set(m_serial2, FUNC(ins8250_uart_device::rx_w));
	m_rs232_2->dcd_handler().set(m_serial2, FUNC(ins8250_uart_device::dcd_w));
	m_rs232_2->dsr_handler().set(m_serial2, FUNC(ins8250_uart_device::dsr_w));
	m_rs232_2->ri_handler().set(m_serial2, FUNC(ins8250_uart_device::ri_w));
	m_rs232_2->cts_handler().set(m_serial2, FUNC(ins8250_uart_device::cts_w));

	PC_LPT(config, m_parallel);
}


static INPUT_PORTS_START( babyblue2 )
	PORT_START("SW1")
	PORT_DIPNAME( 0x0f, 0x0e, "Z80 Memory and Port location" )  PORT_DIPLOCATION( "SW1:7,6,5,4" )
	PORT_DIPSETTING( 0x00, "Invalid?" )
	PORT_DIPSETTING( 0x01, "RAM: 10000  I/O: 302" )
	PORT_DIPSETTING( 0x02, "RAM: 20000  I/O: 304" )
	PORT_DIPSETTING( 0x03, "RAM: 30000  I/O: 306" )
	PORT_DIPSETTING( 0x04, "RAM: 40000  I/O: 308" )
	PORT_DIPSETTING( 0x05, "RAM: 50000  I/O: 30a" )
	PORT_DIPSETTING( 0x06, "RAM: 60000  I/O: 30c" )
	PORT_DIPSETTING( 0x07, "RAM: 70000  I/O: 30e" )
	PORT_DIPSETTING( 0x08, "RAM: 80000  I/O: 310" )
	PORT_DIPSETTING( 0x09, "RAM: 90000  I/O: 312" )
	PORT_DIPSETTING( 0x0a, "RAM: A0000  I/O: 314" )
	PORT_DIPSETTING( 0x0b, "RAM: B0000  I/O: 316" )
	PORT_DIPSETTING( 0x0c, "RAM: C0000  I/O: 318" )
	PORT_DIPSETTING( 0x0d, "RAM: D0000  I/O: 31a" )
	PORT_DIPSETTING( 0x0e, "RAM: E0000  I/O: 31c" )
	PORT_DIPSETTING( 0x0f, "RAM: F0000  I/O: 31f" )
	PORT_DIPUNKNOWN( 0x70, 0x00 )  PORT_DIPLOCATION( "SW1:3,2,1" )

	// exact setting unknown at this stage
	PORT_START("SW2")
	PORT_DIPNAME( 0x07, 0x07, "Device Module Ports" )  PORT_DIPLOCATION( "SW2:3,2,1" )
	PORT_DIPSETTING( 0x00, "0x00" )
	PORT_DIPSETTING( 0x01, "0x01" )
	PORT_DIPSETTING( 0x02, "0x02" )
	PORT_DIPSETTING( 0x03, "0x03" )
	PORT_DIPSETTING( 0x04, "0x04" )
	PORT_DIPSETTING( 0x05, "0x05" )
	PORT_DIPSETTING( 0x06, "0x06" )
	PORT_DIPSETTING( 0x07, "0x07" )

	// exact setting unknown at this stage
	PORT_START("SW3")
	PORT_DIPNAME( 0x0f, 0x0e, "RAM Module Page" )  PORT_DIPLOCATION( "SW3:7,6,5,4" )
	PORT_DIPSETTING( 0x00, "0x00" )
	PORT_DIPSETTING( 0x01, "0x01" )
	PORT_DIPSETTING( 0x02, "0x02" )
	PORT_DIPSETTING( 0x03, "0x03" )
	PORT_DIPSETTING( 0x04, "0x04" )
	PORT_DIPSETTING( 0x05, "0x05" )
	PORT_DIPSETTING( 0x06, "0x06" )
	PORT_DIPSETTING( 0x07, "0x07" )
	PORT_DIPSETTING( 0x08, "0x08" )
	PORT_DIPSETTING( 0x09, "0x09" )
	PORT_DIPSETTING( 0x0a, "0x0a" )
	PORT_DIPSETTING( 0x0b, "0x0b" )
	PORT_DIPSETTING( 0x0c, "0x0c" )
	PORT_DIPSETTING( 0x0d, "0x0d" )
	PORT_DIPSETTING( 0x0e, "0x0e" )
	PORT_DIPSETTING( 0x0f, "0x0f" )

	PORT_DIPNAME(0x40, 0x40, "RAM Bank I" )  PORT_DIPLOCATION( "SW3:1" )
	PORT_DIPSETTING( 0x00, "Enabled" )
	PORT_DIPSETTING( 0x40, "Disabled" )
	PORT_DIPNAME(0x20, 0x20, "RAM Bank II" )  PORT_DIPLOCATION( "SW3:2" )
	PORT_DIPSETTING( 0x00, "Enabled" )
	PORT_DIPSETTING( 0x20, "Disabled" )
	PORT_DIPNAME(0x10, 0x10, "RAM Bank III" )  PORT_DIPLOCATION( "SW3:3" )
	PORT_DIPSETTING( 0x00, "Enabled" )
	PORT_DIPSETTING( 0x10, "Disabled" )

	PORT_START("H2")
	PORT_CONFNAME( 0x01, 0x01, "LPT port config" )
	PORT_CONFSETTING( 0x00, "LPT1" )
	PORT_CONFSETTING( 0x01, "LPT2" )
INPUT_PORTS_END

//-------------------------------------------------
//  isa8_adlib_device - constructor
//-------------------------------------------------

isa8_babyblue2_device::isa8_babyblue2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA8_BABYBLUE2, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_z80(*this, "z80cpu")
	, m_serial1(*this, "serial1")
	, m_serial2(*this, "serial2")
	, m_rs232_1(*this, "rs232_1")
	, m_rs232_2(*this, "rs232_2")
	, m_parallel(*this, "parallel")
	, m_dsw1(*this, "SW1")
	, m_dsw2(*this, "SW2")
	, m_dsw3(*this, "SW3")
	, m_h2(*this, "H2")
	, m_ram(*this, "z80ram")
	, m_devices_installed(false)
{
}

ioport_constructor isa8_babyblue2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( babyblue2 );
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_babyblue2_device::device_start()
{
	set_isa_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_babyblue2_device::device_reset()
{
	uint32_t ramloc;
	uint16_t ioloc, lptloc, z80lptloc;

	ramloc = (m_dsw1->read() & 0x0f) << 16;
	ioloc =  ((m_dsw1->read() & 0x0f) << 1) | 0x300;
	lptloc = (m_h2->read() & 0x01) ? 0x278 : 0x378;
	z80lptloc = (m_h2->read() & 0x01) ? 0x10 : 0x50;

	// halt Z80
	m_z80->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	if(!m_devices_installed)  // will need a hard reset to put DIP switch and jumper changes into effect
	{
		// map Z80 LPT port based on jumper setting
		m_z80->space(AS_IO).install_readwrite_handler(z80lptloc, z80lptloc+7, read8sm_delegate(m_parallel, FUNC(pc_lpt_device::read)), write8sm_delegate(m_parallel, FUNC(pc_lpt_device::write)));

		m_isa->install_device(ioloc, ioloc+1, read8sm_delegate(*this, FUNC(isa8_babyblue2_device::z80_control_r)), write8sm_delegate(*this, FUNC(isa8_babyblue2_device::z80_control_w)));
		m_isa->install_device(lptloc, lptloc+7, read8sm_delegate(m_parallel, FUNC(pc_lpt_device::read)), write8sm_delegate(m_parallel, FUNC(pc_lpt_device::write)));
		m_isa->install_device(0x3f8, 0x03ff, read8sm_delegate(m_serial1, FUNC(ins8250_device::ins8250_r)), write8sm_delegate(m_serial1, FUNC(ins8250_device::ins8250_w)));
		m_isa->install_device(0x2f8, 0x02ff, read8sm_delegate(m_serial2, FUNC(ins8250_device::ins8250_r)), write8sm_delegate(m_serial2, FUNC(ins8250_device::ins8250_w)));
		// TODO: RTC
		m_isa->install_memory(ramloc, ramloc+0xffff, read8sm_delegate(*this, FUNC(isa8_babyblue2_device::z80_ram_r)),write8sm_delegate(*this, FUNC(isa8_babyblue2_device::z80_ram_w)));
		m_devices_installed = true;
	}
}

uint8_t isa8_babyblue2_device::z80_control_r(offs_t offset)
{
	logerror("Z80 control line read\b");
	return 0xff;
}

void isa8_babyblue2_device::z80_control_w(offs_t offset, uint8_t data)
{
	if(offset == 0)
	{
		m_z80->set_input_line(INPUT_LINE_HALT, (data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
		m_z80->set_input_line(INPUT_LINE_IRQ0, (data & 0x04) ? ASSERT_LINE : CLEAR_LINE);
		m_z80->set_input_line(INPUT_LINE_NMI, (data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
		logerror("Z80 control write: %02x\n",data);
	}
}

void isa8_babyblue2_device::lpt_irq(int state)
{
	if(m_h2->read() & 0x01)
		m_isa->irq5_w(state);
	else
		m_isa->irq7_w(state);
}
