// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-11-05 Skeleton

Ampex Dialogue 80 terminal

Chips: CRT-5037, COM8017, SMC5016-5, MK3880N (Z80)
Crystals: 4.9152, 23.814
Other: Beeper, 5x 10sw-dips.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"

class ampex_state : public driver_device
{
public:
	ampex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
	{ }

private:
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
};

static ADDRESS_MAP_START( mem_map, AS_PROGRAM, 8, ampex_state )
	AM_RANGE(0x0000, 0x2fff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0x4000, 0x43ff) AM_RAM // main RAM
	AM_RANGE(0x4400, 0x57ff) AM_RAM // expansion RAM
	AM_RANGE(0x5841, 0x5841) AM_WRITENOP // ???
	AM_RANGE(0x5842, 0x5842) AM_READNOP // ???
	AM_RANGE(0x8000, 0x9fff) AM_RAM
	AM_RANGE(0xc000, 0xcfff) AM_RAM // video RAM
ADDRESS_MAP_END

static INPUT_PORTS_START( ampex )
INPUT_PORTS_END

static MACHINE_CONFIG_START( ampex )
	MCFG_CPU_ADD("maincpu", Z80, 2'000'000) // no idea of clock.
	MCFG_CPU_PROGRAM_MAP(mem_map)
MACHINE_CONFIG_END

ROM_START( dialog80 )
	ROM_REGION( 0x3000, "roms", 0 )
	ROM_LOAD( "3505240-01.u102", 0x0000, 0x0800, CRC(c5315780) SHA1(f2a8924f277d04bf4407f9b71b8d2788df0b1dc2) )
	ROM_LOAD( "3505240-02.u104", 0x0800, 0x0800, CRC(3fefa114) SHA1(d83c00605ae6c02d3aac7b572eb2bf615f0d4f3a) )
	ROM_LOAD( "3505240-03.u103", 0x1000, 0x0800, CRC(03abbcb2) SHA1(e5d382eefc3baff8f3e4d6b13219cb5eb1ca32f2) )
	ROM_LOAD( "3505240-04.u105", 0x1800, 0x0800, CRC(c051e15f) SHA1(16a066c39743ddf9a7da54bb8c03e2090d461862) )
	ROM_LOAD( "3505240-05.u100", 0x2000, 0x0800, CRC(6db6365b) SHA1(a68c83e554c2493645287e369749a07474723452) )
	ROM_LOAD( "3505240-06.u101", 0x2800, 0x0800, CRC(8f9a4969) SHA1(f9cd434f8d287c584cda429b45ca2537fdfb436b) )

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "3505240-07.u69",  0x0000, 0x0800, CRC(838a16cb) SHA1(4301324b9fe9453c2d277972f9464c4214c6793d) )
ROM_END

COMP( 1980, dialog80, 0, 0, ampex, ampex, ampex_state, 0, "Ampex", "Dialogue 80", MACHINE_IS_SKELETON )
