// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

2013-12-01 Driver for Cromemco MCB-216 SCC (Single Card Computer),
and also the earlier CB-308.

TODO:
- Confirm cpu clock speed

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


class mcb216_state : public driver_device
{
public:
	mcb216_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_tms5501(*this, "tms5501")
	{ }

	DECLARE_READ8_MEMBER(tms5501_status_r);

	DECLARE_MACHINE_RESET(mcb216);
	DECLARE_MACHINE_RESET(cb308);

	IRQ_CALLBACK_MEMBER(irq_callback);

	void mcb216(machine_config &config);
	void cb308(machine_config &config);
	void cb308_mem(address_map &map);
	void mcb216_io(address_map &map);
	void mcb216_mem(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
	required_device<tms5501_device> m_tms5501;
};

READ8_MEMBER(mcb216_state::tms5501_status_r)
{
	// D7  D6  D5  D4  D3  D2  D1  D0
	// TBE RDA IPG TBE RDA SRV ORE FME
	return bitswap<8>(m_tms5501->sta_r(space, 0), 4, 3, 5, 4, 3, 2, 1, 0);
}

ADDRESS_MAP_START(mcb216_state::mcb216_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x2400, 0xffff) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(mcb216_state::mcb216_io)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	// 74904 PROM provides custom remapping for TMS5501 registers
	AM_RANGE(0x00, 0x00) AM_READ(tms5501_status_r) AM_DEVWRITE("tms5501", tms5501_device, rr_w)
	AM_RANGE(0x01, 0x01) AM_DEVREADWRITE("tms5501", tms5501_device, rb_r, tb_w)
	AM_RANGE(0x02, 0x02) AM_DEVWRITE("tms5501", tms5501_device, cmd_w)
	AM_RANGE(0x03, 0x03) AM_DEVREADWRITE("tms5501", tms5501_device, rst_r, mr_w)
	AM_RANGE(0x04, 0x04) AM_DEVREADWRITE("tms5501", tms5501_device, xi_r, xo_w)
	AM_RANGE(0x05, 0x09) AM_DEVWRITE("tms5501", tms5501_device, tmr_w)
ADDRESS_MAP_END

ADDRESS_MAP_START(mcb216_state::cb308_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0xe000, 0xefff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( mcb216 )
INPUT_PORTS_END


IRQ_CALLBACK_MEMBER(mcb216_state::irq_callback)
{
	return m_tms5501->get_vector();
}

MACHINE_RESET_MEMBER( mcb216_state, mcb216 )
{
}

MACHINE_RESET_MEMBER( mcb216_state, cb308 )
{
	m_maincpu->set_state_int(Z80_PC, 0xe000);
}

MACHINE_CONFIG_START(mcb216_state::mcb216)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 8_MHz_XTAL / 2)
	MCFG_CPU_PROGRAM_MAP(mcb216_mem)
	MCFG_CPU_IO_MAP(mcb216_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(mcb216_state, irq_callback)

	MCFG_MACHINE_RESET_OVERRIDE(mcb216_state, mcb216)

	MCFG_DEVICE_ADD("tms5501", TMS5501, 8_MHz_XTAL / 4)
	MCFG_TMS5501_XMT_CALLBACK(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_TMS5501_IRQ_CALLBACK(INPUTLINE("maincpu", 0))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("tms5501", tms5501_device, rcv_w))
MACHINE_CONFIG_END

MACHINE_CONFIG_START(mcb216_state::cb308)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 8_MHz_XTAL / 2)
	MCFG_CPU_PROGRAM_MAP(cb308_mem)
	MCFG_CPU_IO_MAP(mcb216_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(mcb216_state, irq_callback)

	MCFG_MACHINE_RESET_OVERRIDE(mcb216_state, cb308)

	MCFG_DEVICE_ADD("tms5501", TMS5501, 8_MHz_XTAL / 4)
	MCFG_TMS5501_XMT_CALLBACK(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_TMS5501_IRQ_CALLBACK(INPUTLINE("maincpu", 0))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("tms5501", tms5501_device, rcv_w))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( mcb216 )
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "mcb216r0", 0x0000, 0x0800, CRC(86d20cea) SHA1(9fb8fdbcb8d31bd3304a0b3339c7f423188e9d37) )
	ROM_LOAD( "mcb216r1", 0x0800, 0x0800, CRC(68a25b2c) SHA1(3eadd4a5d65726f767742deb4b51a97df813f37d) )

	ROM_REGION(0x20, "prom", 0)
	ROM_LOAD( "74904.ic25", 0x00, 0x20, NO_DUMP ) // TBP18S030 or equivalent
ROM_END

ROM_START( cb308 )
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "cb308r0",  0x0000, 0x0400, CRC(62f50531) SHA1(3071e2ab7fc6b2ca889e4fb5cf7cc9ee8fbe53d3) )
	ROM_LOAD( "cb308r1",  0x0400, 0x0400, CRC(03191ac1) SHA1(84665dfc797c9f51bb659291b18399986ed846fb) )
	ROM_LOAD( "cb308r2",  0x0800, 0x0400, CRC(695ea521) SHA1(efe36a712e2a038ee804e556c5ebe05443cf798e) )
	ROM_LOAD( "cb308r3",  0x0c00, 0x0400, CRC(e3e4a778) SHA1(a7c14458f8636d860ae25b10387fa6f7f2ef6ef9) )

	ROM_REGION(0x20, "prom", 0)
	ROM_LOAD( "74904.ic25", 0x00, 0x20, NO_DUMP ) // TBP18S030 or equivalent
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT   CLASS          INIT  COMPANY      FULLNAME  FLAGS */
COMP( 1979, mcb216, 0,      0,       mcb216,    mcb216, mcb216_state,  0,    "Cromemco", "MCB-216", MACHINE_NO_SOUND_HW )
COMP( 1977, cb308,  mcb216, 0,       cb308,     mcb216, mcb216_state,  0,    "Cromemco", "CB-308",  MACHINE_NO_SOUND_HW )
