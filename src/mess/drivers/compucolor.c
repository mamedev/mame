// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    Compucolor II

    http://www.compucolor.org/index.html

*/

#include "emu.h"
#include "cpu/i8085/i8085.h"

class compucolor2_state : public driver_device
{
public:
	compucolor2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }
};

static ADDRESS_MAP_START( compucolor2_mem, AS_PROGRAM, 8, compucolor2_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( compucolor2_io, AS_IO, 8, compucolor2_state )
ADDRESS_MAP_END

static INPUT_PORTS_START( compucolor2 )
INPUT_PORTS_END

static MACHINE_CONFIG_START( compucolor2, compucolor2_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", I8080, 1996800)
	MCFG_CPU_PROGRAM_MAP(compucolor2_mem)
	MCFG_CPU_IO_MAP(compucolor2_io)
MACHINE_CONFIG_END

ROM_START( compclr2 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "678", "v6.78" )
	ROMX_LOAD( "v678.rom", 0x0000, 0x4000, BAD_DUMP CRC(5e559469) SHA1(fe308774aae1294c852fe24017e58d892d880cd3), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "879", "v8.79" )
	ROMX_LOAD( "v879.rom", 0x0000, 0x4000, BAD_DUMP CRC(4de8e652) SHA1(e5c55da3ac893b8a5a99c8795af3ca72b1645f3f), ROM_BIOS(2) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "chargen.uf6", 0x000, 0x400, BAD_DUMP CRC(7eef135a) SHA1(be488ef32f54c6e5f551fb84ab12b881aef72dd9) )
	ROM_LOAD( "chargen.uf7", 0x400, 0x400, BAD_DUMP CRC(2bee7cf6) SHA1(808e0fc6f2332b4297de190eafcf84668703e2f4) )

	ROM_REGION( 0x20, "crt", 0 )
	ROM_LOAD( "timing.rom", 0x00, 0x20, BAD_DUMP CRC(27ae54bc) SHA1(ccb056fbc1ec2132f2602217af64d77237494afb) )
ROM_END

COMP( 1977, compclr2,    0,      0,      compucolor2,        compucolor2, driver_device, 0,      "Intelligent System Corporation",  "Compucolor II",  GAME_IS_SKELETON | GAME_NOT_WORKING | GAME_NO_SOUND )
