// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Skeleton driver for 68k Single Board Computer

    29/03/2011

    http://www.kmitl.ac.th/~kswichit/68k/68k.html

    TODO:
    - Add RTC (type DS12887)

    All of the address and i/o decoding is done by a pair of XC9536
    mask-programmed custom devices.

    There are some chips used for unclear purposes (GPI, GPO, LCD).

    This computer has no sound, and no facility for saving or loading programs.

    When started, press ? key for a list of commands.

    Has 1MB of flash (which is actually just 12k of program),
    and 1MB of RAM. Memory map should probably be corrected.


****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/6850acia.h"
#include "machine/clock.h"


namespace {

class c68ksbc_state : public driver_device
{
public:
	c68ksbc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void c68ksbc(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void c68ksbc_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x002fff).rom();
	map(0x003000, 0x5fffff).ram();
	map(0x600000, 0x600003).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);
}


/* Input ports */
static INPUT_PORTS_START( c68ksbc )
INPUT_PORTS_END


void c68ksbc_state::c68ksbc(machine_config &config)
{
	M68000(config, m_maincpu, 8000000); // text says 8MHz, schematic says 10MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &c68ksbc_state::mem_map);

	acia6850_device &acia(ACIA6850(config, "acia", 0));
	acia.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	acia.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("acia", FUNC(acia6850_device::write_rxd));
	rs232.cts_handler().set("acia", FUNC(acia6850_device::write_cts));

	clock_device &acia_clock(CLOCK(config, "acia_clock", 153600));
	acia_clock.signal_handler().set("acia", FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append("acia", FUNC(acia6850_device::write_rxc));
}

/* ROM definition */
ROM_START( 68ksbc )
	ROM_REGION(0x3000, "maincpu", 0)
	ROM_LOAD( "t68k.bin", 0x0000, 0x2f78, CRC(20a8d0d0) SHA1(544fd8bd8ed017115388c8b0f7a7a59a32253e43) )
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY             FULLNAME                     FLAGS */
COMP( 2002, 68ksbc, 0,      0,      c68ksbc, c68ksbc, c68ksbc_state, empty_init, "Wichit Sirichote", "68k Single Board Computer", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
