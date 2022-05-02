// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        SC111

        23/04/2022 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"
#include "bus/rs232/rs232.h"

class sc111_state : public driver_device
{
public:
	sc111_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void sc111(machine_config &config);
private:
	required_device<z180_device> m_maincpu;
	void sc111_io(address_map &map);
	void sc111_mem(address_map &map);
};


void sc111_state::sc111_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x7ffff).rom();
	map(0x80000, 0xfffff).ram();
}

void sc111_state::sc111_io(address_map &map)
{
	map.global_mask(0x00ff);
	map.unmap_value_high();
	map(0x0000, 0x007f).ram(); /* Z180 internal registers */
}

/* Input ports */
static INPUT_PORTS_START( sc111 )
INPUT_PORTS_END

// This is here only to configure our terminal for interactive use
static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void sc111_state::sc111(machine_config &config)
{
	Z8S180(config, m_maincpu, XTAL(18'432'000)); // location U17 HD64180
	m_maincpu->set_addrmap(AS_PROGRAM, &sc111_state::sc111_mem);
	m_maincpu->set_addrmap(AS_IO, &sc111_state::sc111_io);
	m_maincpu->subdevice<z180asci_channel>("asci_0")->tx_handler().set("rs232", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal)); // must be below the DEVICE_INPUT_DEFAULTS_START block
	rs232.rxd_handler().set("maincpu:asci_0", FUNC(z180asci_channel::write_rx));
}

/* ROM definition */
ROM_START( sc111 )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASEFF )
	// SCM v1.3
	ROM_LOAD( "sc111.bin", 0x0000, 0x8000, CRC(e676922c) SHA1(d6451bb6196d630023f666ba44aa43461be795f6))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                    FULLNAME   FLAGS */
COMP( 2020, sc111, 0,      0,      sc111,  sc111, sc111_state, empty_init, "Stephen C Cousins", "SC111", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
