// license:BSD-3-Clause
// copyright-holders: TODO
/***********************************************************************************************************************************

2017-11-20 Skeleton

Milwaukee Computer MC-1000 series (MC-1000/1100/1200/1300/1400) all the same except for disk options.

Chips: SY6502, 2x 6821, 2x MC6850P, 6852, INS8253
Other: 2x 7-position rotary "dips" to select baud rates on each 6850 (19.2K, 9600, 4800, 2400, 1200, 600, 300).


Status:
- When booted it asks for a test to perform. Valid answers are A,B,D,E,H,I,M,O,P,S,W. To exit a test, press E.
         M displays * then loops. E just asks the Test question again. P gets caught in a loop (fdc test).
         When W pressed, N is displayed.


************************************************************************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/pit8253.h"
#include "machine/mc6852.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"


namespace {

class milwaukee_state : public driver_device
{
public:
	milwaukee_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void milwaukee(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;
	required_device<cpu_device> m_maincpu;
};
void milwaukee_state::mem_map(address_map &map)
{
	map(0x0000, 0xf7ff).ram();
	//map(0xf800, 0xf87f) expansion i/o
	map(0xf880, 0xf881).rw("acia1", FUNC(acia6850_device::read), FUNC(acia6850_device::write)); // terminal
	map(0xf882, 0xf883).rw("acia2", FUNC(acia6850_device::read), FUNC(acia6850_device::write)); // remote
	map(0xf884, 0xf887).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // centronics
	map(0xf888, 0xf88b).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xf88c, 0xf88f).rw("pia2", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // disk controller
	map(0xf890, 0xf891).rw("ssda", FUNC(mc6852_device::read), FUNC(mc6852_device::write));
	map(0xf898, 0xffff).rom().region("roms", 0x0098);
}

static INPUT_PORTS_START( milwaukee )
INPUT_PORTS_END

void milwaukee_state::milwaukee(machine_config &config)
{
	M6502(config, m_maincpu, 16_MHz_XTAL / 16);
	m_maincpu->set_addrmap(AS_PROGRAM, &milwaukee_state::mem_map);

	pit8253_device &pit(PIT8253(config, "pit", 0));
	pit.set_clk<0>(16_MHz_XTAL / 16 / 4); // 250 kHz
	pit.out_handler<0>().set("pit", FUNC(pit8253_device::write_gate0)).invert();
	pit.set_clk<1>(16_MHz_XTAL / 2 / 13 / 2048 / 5); // 60.09 Hz?
	pit.out_handler<1>().set("pit", FUNC(pit8253_device::write_clk2)).invert();

	PIA6821(config, "pia1");
	PIA6821(config, "pia2");
	ACIA6850(config, "acia2", 0);
	MC6852(config, "ssda", 0);

	clock_device &acia_clock(CLOCK(config, "acia_clock", 16_MHz_XTAL / 2 / 13 / 4));
	acia_clock.signal_handler().set("acia1", FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append("acia1", FUNC(acia6850_device::write_rxc));

	acia6850_device &acia1(ACIA6850(config, "acia1", 0));
	acia1.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	acia1.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("acia1", FUNC(acia6850_device::write_rxd));
	rs232.cts_handler().set("acia1", FUNC(acia6850_device::write_cts));
}

ROM_START( mc1200 )
	ROM_REGION( 0x0c00, "roms", 0 )
	ROM_LOAD( "mfm_6-29-82_10_sector.u15", 0x0000, 0x0800, CRC(40b0af66) SHA1(c988e1f90c9abb93171c4e40a6585ce9cc3fd495) )
	ROM_LOAD( "2758.u14", 0x0800, 0x0400, CRC(b20e2345) SHA1(da498cc0c746897a85d6f2d1a5bd70a726c1e4ef) ) // big white sticker, but nothing on it
ROM_END

} // anonymous namespace


COMP( 1980, mc1200, 0, 0, milwaukee, milwaukee, milwaukee_state, empty_init, "Milwaukee Computers", "MC-1200", MACHINE_IS_SKELETON )
