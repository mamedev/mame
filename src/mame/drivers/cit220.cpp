// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for VT220-compatible terminals by C. Itoh/CIE Terminals.

The CIT-220+ Video Terminal was introduced as a direct competitor to DEC's VT220. It copied the design of the VT220 closely
enough to provoke a lawsuit, which led to its eventual withdrawal in favor of its successor, the CIT224.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"


class cit220_state : public driver_device
{
public:
	cit220_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		//, m_p_chargen(*this, "chargen")
	{ }

	DECLARE_WRITE_LINE_MEMBER(sod_w);

private:
	required_device<cpu_device> m_maincpu;
	//required_region_ptr<u8> m_p_chargen;
};


WRITE_LINE_MEMBER(cit220_state::sod_w)
{
	logerror("SOD set %s\n", state ? "high" : "low");
}

static ADDRESS_MAP_START( mem_map, AS_PROGRAM, 8, cit220_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xa000, 0xa1ff) AM_ROM AM_REGION("eeprom", 0x800)
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, cit220_state )
	AM_RANGE(0x21, 0x21) AM_READNOP
ADDRESS_MAP_END


static INPUT_PORTS_START( cit220p )
INPUT_PORTS_END


static MACHINE_CONFIG_START( cit220p )
	MCFG_CPU_ADD("maincpu", I8085A, 6'000'000)
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_I8085A_SOD(WRITELINE(cit220_state, sod_w))
MACHINE_CONFIG_END


ROM_START( cit220p )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "v17_001a.ic23", 0x0000, 0x4000, CRC(2cc43026) SHA1(366f57292c6e44571368c29e3258203779847356) )
	ROM_LOAD( "v17_001b.ic24", 0x4000, 0x4000, CRC(a56b16f1) SHA1(c68f26b35453153f7defcf1cf2b7ad7fe36cc9e7) )

	ROM_REGION(0x1000, "eeprom", 0)
	ROM_LOAD( "eeprom.bin",    0x0000, 0x1000, CRC(7b24878a) SHA1(20086fb792a24339b65abe627aefbcf48e2abcf4) ) // don't know where this fits in

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "v20_cg.ic17",   0x0000, 0x1000, CRC(76ef7ca9) SHA1(6e7799ca0a41350fbc369bbbd4ab581150f37b10) )

	ROM_REGION(0x10000, "keyboard", 0)
	ROM_LOAD( "v00_kbd.bin",   0x0000, 0x1000, CRC(f9d24190) SHA1(c4e9ef8188afb18de373f2a537ca9b7a315bfb76) )
ROM_END

COMP( 1983, cit220p, 0, 0, cit220p, cit220p, cit220_state, 0, "C. Itoh", "CIT-220+ Video Terminal", MACHINE_IS_SKELETON )
