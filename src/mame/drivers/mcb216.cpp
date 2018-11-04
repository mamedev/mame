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
#include "machine/ay31015.h"
#include "bus/rs232/rs232.h"


class mcb216_state : public driver_device
{
public:
	mcb216_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_uart(*this, "uart")
	{ }

	DECLARE_READ8_MEMBER(port00_r);
	DECLARE_READ8_MEMBER(port01_r);
	DECLARE_WRITE8_MEMBER(port01_w);
	DECLARE_MACHINE_RESET(mcb216);
	DECLARE_MACHINE_RESET(cb308);

	void mcb216(machine_config &config);
	void cb308(machine_config &config);
	void cb308_mem(address_map &map);
	void mcb216_io(address_map &map);
	void mcb216_mem(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
	required_device<ay31015_device> m_uart;
};

ADDRESS_MAP_START(mcb216_state::mcb216_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x2400, 0xffff) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(mcb216_state::mcb216_io)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(port00_r)
	AM_RANGE(0x01, 0x01) AM_READWRITE(port01_r,port01_w)
ADDRESS_MAP_END

ADDRESS_MAP_START(mcb216_state::cb308_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0xe000, 0xefff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( mcb216 )
INPUT_PORTS_END


READ8_MEMBER( mcb216_state::port01_r )
{
	m_uart->set_input_pin(AY31015_RDAV, 0);
	u8 result = m_uart->get_received_data();
	m_uart->set_input_pin(AY31015_RDAV, 1);
	return result;
}

// 0x40 - a keystroke is available
// 0x80 - ok to send to terminal
READ8_MEMBER( mcb216_state::port00_r )
{
	return (m_uart->get_output_pin(AY31015_DAV) << 6) | (m_uart->get_output_pin(AY31015_TBMT) << 7);
}

WRITE8_MEMBER( mcb216_state::port01_w )
{
	m_uart->set_transmit_data(data);
}

MACHINE_RESET_MEMBER( mcb216_state, mcb216 )
{
	m_uart->set_input_pin(AY31015_XR, 0);
	m_uart->set_input_pin(AY31015_XR, 1);
	m_uart->set_input_pin(AY31015_SWE, 0);
	m_uart->set_input_pin(AY31015_NP, 1);
	m_uart->set_input_pin(AY31015_TSB, 0);
	m_uart->set_input_pin(AY31015_NB1, 1);
	m_uart->set_input_pin(AY31015_NB2, 1);
	m_uart->set_input_pin(AY31015_EPS, 1);
	m_uart->set_input_pin(AY31015_CS, 1);
	m_uart->set_input_pin(AY31015_CS, 0);
}

MACHINE_RESET_MEMBER( mcb216_state, cb308 )
{
	m_uart->set_input_pin(AY31015_XR, 0);
	m_uart->set_input_pin(AY31015_XR, 1);
	m_uart->set_input_pin(AY31015_SWE, 0);
	m_uart->set_input_pin(AY31015_NP, 1);
	m_uart->set_input_pin(AY31015_TSB, 0);
	m_uart->set_input_pin(AY31015_NB1, 1);
	m_uart->set_input_pin(AY31015_NB2, 1);
	m_uart->set_input_pin(AY31015_EPS, 1);
	m_uart->set_input_pin(AY31015_CS, 1);
	m_uart->set_input_pin(AY31015_CS, 0);
	m_maincpu->set_state_int(Z80_PC, 0xe000);
}

MACHINE_CONFIG_START(mcb216_state::mcb216)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(mcb216_mem)
	MCFG_CPU_IO_MAP(mcb216_io)
	MCFG_MACHINE_RESET_OVERRIDE(mcb216_state, mcb216)

	/* video hardware */
	MCFG_DEVICE_ADD("uart", AY51013, 0) // exact uart type is unknown
	MCFG_AY51013_TX_CLOCK(153600)
	MCFG_AY51013_RX_CLOCK(153600)
	MCFG_AY51013_READ_SI_CB(DEVREADLINE("rs232", rs232_port_device, rxd_r))
	MCFG_AY51013_WRITE_SO_CB(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
MACHINE_CONFIG_END

MACHINE_CONFIG_START(mcb216_state::cb308)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(cb308_mem)
	MCFG_CPU_IO_MAP(mcb216_io)
	MCFG_MACHINE_RESET_OVERRIDE(mcb216_state, cb308)

	/* video hardware */
	MCFG_DEVICE_ADD("uart", AY51013, 0) // exact uart type is unknown
	MCFG_AY51013_TX_CLOCK(153600)
	MCFG_AY51013_RX_CLOCK(153600)
	MCFG_AY51013_READ_SI_CB(DEVREADLINE("rs232", rs232_port_device, rxd_r))
	MCFG_AY51013_WRITE_SO_CB(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( mcb216 )
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "mcb216r0", 0x0000, 0x0800, CRC(86d20cea) SHA1(9fb8fdbcb8d31bd3304a0b3339c7f423188e9d37) )
	ROM_LOAD( "mcb216r1", 0x0800, 0x0800, CRC(68a25b2c) SHA1(3eadd4a5d65726f767742deb4b51a97df813f37d) )
ROM_END

ROM_START( cb308 )
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "cb308r0",  0x0000, 0x0400, CRC(62f50531) SHA1(3071e2ab7fc6b2ca889e4fb5cf7cc9ee8fbe53d3) )
	ROM_LOAD( "cb308r1",  0x0400, 0x0400, CRC(03191ac1) SHA1(84665dfc797c9f51bb659291b18399986ed846fb) )
	ROM_LOAD( "cb308r2",  0x0800, 0x0400, CRC(695ea521) SHA1(efe36a712e2a038ee804e556c5ebe05443cf798e) )
	ROM_LOAD( "cb308r3",  0x0c00, 0x0400, CRC(e3e4a778) SHA1(a7c14458f8636d860ae25b10387fa6f7f2ef6ef9) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT   CLASS          INIT  COMPANY      FULLNAME  FLAGS */
COMP( 1979, mcb216, 0,      0,       mcb216,    mcb216, mcb216_state,  0,    "Cromemco", "MCB-216", MACHINE_NO_SOUND_HW )
COMP( 1977, cb308,  mcb216, 0,       cb308,     mcb216, mcb216_state,  0,    "Cromemco", "CB-308",  MACHINE_NO_SOUND_HW )
