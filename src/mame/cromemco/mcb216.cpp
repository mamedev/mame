// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

2013-12-01 Driver for Cromemco MCB-216 SCC (Single Card Computer),
and also the earlier CB-308.


Memory allocation
- 0000 to 0FFF - standard roms
- 1000 to 1FFF - optional roms or ram (expect roms)
- 2000 to 23FF - standard ram
- 2400 to FFFF - optional whatever the user wants (expect ram)

All commands to be in uppercase.

MCB-216:
Press Enter twice. You will see the Basic OK prompt. To get into the
monitor, use the QUIT command, and to return use the B command.

The mcb216 can use an optional floppy-disk-drive unit. The only other
storage is paper-tape, which is expected to be attached to the terminal.

CB-308:
Press Enter twice. You will see the Monitor logo. To get into the BASIC,
enter GE400. To return to the monitor, use the QUIT command followed by
pressing Enter twice. All monitor commands must be in uppercase. The
only storage is paper-tape.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/tms5501.h"
#include "bus/rs232/rs232.h"


namespace {

class mcb216_state : public driver_device
{
public:
	mcb216_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_tms5501(*this, "tms5501")
	{ }

	void mcb216(machine_config &config);

protected:
	u8 tms5501_status_r();
	IRQ_CALLBACK_MEMBER(irq_callback);
	void mcb216_io(address_map &map) ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device<tms5501_device> m_tms5501;

private:
	void mcb216_mem(address_map &map) ATTR_COLD;
};

class cb308_state : public mcb216_state
{
public:
	using mcb216_state::mcb216_state;
	void cb308(machine_config &config);

private:
	void cb308_mem(address_map &map) ATTR_COLD;
	void machine_reset() override ATTR_COLD;
};

u8 mcb216_state::tms5501_status_r()
{
	// D7  D6  D5  D4  D3  D2  D1  D0
	// TBE RDA IPG TBE RDA SRV ORE FME
	return bitswap<8>(m_tms5501->sta_r(), 4, 3, 5, 4, 3, 2, 1, 0);
}

void mcb216_state::mcb216_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom().region("roms", 0);  // SCC board has space for 2k of roms
	map(0x2000, 0x23ff).ram();
	map(0x2400, 0xffff).ram();
}

void mcb216_state::mcb216_io(address_map &map)
{
	map.global_mask(0xff);
	// 74904 PROM provides custom remapping for TMS5501 registers
	map(0x00, 0x00).r(FUNC(mcb216_state::tms5501_status_r)).w(m_tms5501, FUNC(tms5501_device::rr_w));
	map(0x01, 0x01).rw(m_tms5501, FUNC(tms5501_device::rb_r), FUNC(tms5501_device::tb_w));
	map(0x02, 0x02).w(m_tms5501, FUNC(tms5501_device::cmd_w));
	map(0x03, 0x03).rw(m_tms5501, FUNC(tms5501_device::rst_r), FUNC(tms5501_device::mr_w));
	map(0x04, 0x04).rw(m_tms5501, FUNC(tms5501_device::xi_r), FUNC(tms5501_device::xo_w));
	map(0x05, 0x09).w(m_tms5501, FUNC(tms5501_device::tmr_w));
}

void cb308_state::cb308_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).ram();
	map(0xe000, 0xefff).rom().region("roms", 0);
}


/* Input ports */
static INPUT_PORTS_START( mcb216 )
INPUT_PORTS_END


IRQ_CALLBACK_MEMBER(mcb216_state::irq_callback)
{
	return m_tms5501->get_vector();
}

void cb308_state::machine_reset()
{
	m_maincpu->set_state_int(Z80_PC, 0xe000);
}

void mcb216_state::mcb216(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 8_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &mcb216_state::mcb216_mem);
	m_maincpu->set_addrmap(AS_IO, &mcb216_state::mcb216_io);
	m_maincpu->set_irq_acknowledge_callback(FUNC(mcb216_state::irq_callback));

	TMS5501(config, m_tms5501, 8_MHz_XTAL / 4);
	m_tms5501->xmt_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	m_tms5501->int_callback().set_inputline(m_maincpu, 0);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_tms5501, FUNC(tms5501_device::rcv_w));
}

void cb308_state::cb308(machine_config &config)
{
	mcb216(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &cb308_state::cb308_mem);
}

/* ROM definition */
ROM_START( mcb216 )
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD( "mcb216r0", 0x0000, 0x0800, CRC(86d20cea) SHA1(9fb8fdbcb8d31bd3304a0b3339c7f423188e9d37) )
	ROM_LOAD( "mcb216r1", 0x0800, 0x0800, CRC(68a25b2c) SHA1(3eadd4a5d65726f767742deb4b51a97df813f37d) )

	ROM_REGION(0x20, "prom", 0)
	ROM_LOAD( "74904.ic25", 0x00, 0x20, NO_DUMP ) // TBP18S030 or equivalent
ROM_END

ROM_START( cb308 )
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD( "cb308r0",  0x0000, 0x0400, CRC(62f50531) SHA1(3071e2ab7fc6b2ca889e4fb5cf7cc9ee8fbe53d3) )
	ROM_LOAD( "cb308r1",  0x0400, 0x0400, CRC(03191ac1) SHA1(84665dfc797c9f51bb659291b18399986ed846fb) )
	ROM_LOAD( "cb308r2",  0x0800, 0x0400, CRC(695ea521) SHA1(efe36a712e2a038ee804e556c5ebe05443cf798e) )
	ROM_LOAD( "cb308r3",  0x0c00, 0x0400, CRC(e3e4a778) SHA1(a7c14458f8636d860ae25b10387fa6f7f2ef6ef9) )

	ROM_REGION(0x20, "prom", 0)
	ROM_LOAD( "74904.ic25", 0x00, 0x20, NO_DUMP ) // TBP18S030 or equivalent
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY     FULLNAME   FLAGS */
COMP( 1979, mcb216, 0,      0,      mcb216,  mcb216, mcb216_state, empty_init, "Cromemco", "MCB-216", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1977, cb308,  mcb216, 0,      cb308,   mcb216, cb308_state,  empty_init, "Cromemco", "CB-308",  MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
