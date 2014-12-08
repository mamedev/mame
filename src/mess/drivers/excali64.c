// license:MAME
// copyright-holders:Robbbert
/***************************************************************************

Excalibur 64 kit computer, designed and sold in Australia by BGR Computers.

Skeleton driver created on 2014-12-09.

Chips: Z80A, 8251, 8253, 8255, 6845

****************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
//#include "machine/6850acia.h"
//#include "machine/clock.h"
//#include "bus/centronics/ctronics.h"


class excali64_state : public driver_device
{
public:
	excali64_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

private:
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START(excali64_mem, AS_PROGRAM, 8, excali64_state)
	AM_RANGE(0x0000, 0x3FFF) AM_ROM
	AM_RANGE(0x4000, 0xFFFF) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(excali64_io, AS_IO, 8, excali64_state)
	ADDRESS_MAP_GLOBAL_MASK(0x7f)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( excali64 )
INPUT_PORTS_END


static MACHINE_CONFIG_START( excali64, excali64_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(excali64_mem)
	MCFG_CPU_IO_MAP(excali64_io)

	//MCFG_DEVICE_ADD("acia", ACIA6850, 0)
	//MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	//MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	//MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	//MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia", acia6850_device, write_rxd))
	//MCFG_RS232_CTS_HANDLER(DEVWRITELINE("acia", acia6850_device, write_cts))

	//MCFG_DEVICE_ADD("acia_clock", CLOCK, 153600)
	//MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(excali64_state, write_acia_clock))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( excali64 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "rom_1.bin", 0x0000, 0x4000, CRC(e129a305) SHA1(e43ec7d040c2b2e548d22fd6bbc7df8b45a26e5a) )
	ROM_LOAD( "rom_2.bin", 0x2000, 0x2000, CRC(916d9f5a) SHA1(91c527cce963481b7bebf077e955ca89578bb553) )

	ROM_REGION(0x1000, "videoram", ROMREGION_ERASE00)

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "genex_3.bin", 0x0000, 0x1000, CRC(b91619a9) SHA1(2ced636cb7b94ba9d329868d7ecf79963cefe9d9) )
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT     CLASS          INIT    COMPANY         FULLNAME        FLAGS */
COMP( 1984, excali64, 0,      0,       excali64,  excali64, driver_device,  0,  "BGR Computers", "Excalibur 64", GAME_IS_SKELETON )
