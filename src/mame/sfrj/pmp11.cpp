// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/t11/t11.h"
#include "imagedev/floppy.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/wd_fdc.h"

namespace {

class pmp11_state : public driver_device
{
public:
	pmp11_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_uart1(*this, "uart1")
		, m_uart2(*this, "uart2")
		, m_fdc(*this, "fdc")
	{ }

	void pmp11(machine_config &config) ATTR_COLD;

private:
	void pdp11_mem(address_map &map) ATTR_COLD;

	required_device<t11_device> m_maincpu;
	required_device<i8251_device> m_uart1;
	required_device<i8251_device> m_uart2;
	required_device<wd2797_device> m_fdc;	
};

void pmp11_state::pdp11_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xefff).ram();
	map(0xf000, 0xf7ff).rom();
	map(0xf800, 0xfbff).ram();
	// TODO: Not sure if these are mirrored, but no control_w on alternate address
	map(0xff70, 0xff71).mirror(0x4).umask16(0x00ff).rw(m_uart1, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0xff72, 0xff73).mirror(0x4).umask16(0x00ff).rw(m_uart1, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
}

/* Input ports */
static INPUT_PORTS_START( pmp11 )
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void pmp11_state::pmp11(machine_config &config)
{
	T11(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_initial_mode(7 << 13);
	m_maincpu->set_addrmap(AS_PROGRAM, &pmp11_state::pdp11_mem);

	clock_device &uart_clock(CLOCK(config, "uart_clock", 8_MHz_XTAL / 4 / 13));
	uart_clock.signal_handler().set(m_uart1, FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append(m_uart1, FUNC(i8251_device::write_rxc));
	uart_clock.signal_handler().append(m_uart2, FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append(m_uart2, FUNC(i8251_device::write_rxc));

	I8251(config, m_uart1, 0);
	m_uart1->txd_handler().set("rs232a", FUNC(rs232_port_device::write_txd));
	m_uart1->dtr_handler().set("rs232a", FUNC(rs232_port_device::write_dtr));
	m_uart1->rts_handler().set("rs232a", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set("uart1", FUNC(i8251_device::write_rxd));
	rs232a.dsr_handler().set("uart1", FUNC(i8251_device::write_dsr));
	rs232a.cts_handler().set("uart1", FUNC(i8251_device::write_cts));
	rs232a.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	I8251(config, m_uart2, 0);
	m_uart2->txd_handler().set("rs232b", FUNC(rs232_port_device::write_txd));
	m_uart2->dtr_handler().set("rs232b", FUNC(rs232_port_device::write_dtr));
	m_uart2->rts_handler().set("rs232b", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set("uart2", FUNC(i8251_device::write_rxd));
	rs232b.dsr_handler().set("uart2", FUNC(i8251_device::write_dsr));
	rs232b.cts_handler().set("uart2", FUNC(i8251_device::write_cts));
	
	WD2797(config, m_fdc, 8_MHz_XTAL / 4); 
}

ROM_START( pmp11 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "rom.bin", 0xf000, 0x0800, CRC(2cfdc3a3) SHA1(50ffa2a3bd0b75c1ecb4ab6c691796ab0d85dd4e))
ROM_END

} // Anonymous namespace


/* Driver */

/*    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT  CLASS        INIT        COMPANY                  FULLNAME     FLAGS */
COMP( 1985, pmp11,    0,       0,      pmp11,    pmp11, pmp11_state, empty_init, "Institut Jožef Stefan", "PMP-11",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
