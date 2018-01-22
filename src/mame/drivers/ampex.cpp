// license:BSD-3-Clause
// copyright-holders:
/*******************************************************************************

2017-11-05 Skeleton

Ampex Dialogue 80 terminal

Chips: CRT-5037, COM8017, SMC (COM)5016-5, MK3880N (Z80), SN74LS424N (TIM8224)
Crystals: 4.9152, 23.814
Other: Beeper, 5x 10sw-dips.

The program code seems to have been designed with a 8080 CPU in mind, using no
Z80-specific opcodes. This impression is reinforced by the IC types present on
the PCB, which go so far as to include the standard 8224 clock generator.

*******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
//#include "machine/ay31015.h"
//#include "machine/com8116.h"
#include "video/tms9927.h"
#include "screen.h"

#define CHAR_WIDTH 7

class ampex_state : public driver_device
{
public:
	ampex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
	{ }

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void ampex(machine_config &config);
private:
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
};

u32 ampex_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static ADDRESS_MAP_START( mem_map, AS_PROGRAM, 8, ampex_state )
	AM_RANGE(0x0000, 0x2fff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0x4000, 0x43ff) AM_RAM // main RAM
	AM_RANGE(0x4400, 0x57ff) AM_RAM // expansion RAM
	AM_RANGE(0x5841, 0x5841) AM_WRITENOP // ???
	AM_RANGE(0x5842, 0x5842) AM_READNOP // ???
	AM_RANGE(0x5c00, 0x5c0f) AM_DEVREADWRITE("vtac", crt5037_device, read, write)
	AM_RANGE(0x8000, 0x9fff) AM_RAM
	AM_RANGE(0xc000, 0xcfff) AM_RAM // video RAM
ADDRESS_MAP_END

static INPUT_PORTS_START( ampex )
INPUT_PORTS_END

MACHINE_CONFIG_START(ampex_state::ampex)
	MCFG_CPU_ADD("maincpu", Z80, XTAL_23_814MHz / 9) // clocked by 8224?
	MCFG_CPU_PROGRAM_MAP(mem_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_23_814MHz / 2, 105 * CHAR_WIDTH, 0, 80 * CHAR_WIDTH, 270, 0, 250)
	MCFG_SCREEN_UPDATE_DRIVER(ampex_state, screen_update)

	// FIXME: dot clock should be divided by char width
	MCFG_DEVICE_ADD("vtac", CRT5037, XTAL_23_814MHz / 2)
	MCFG_TMS9927_CHAR_WIDTH(CHAR_WIDTH)
	MCFG_VIDEO_SET_SCREEN("screen")
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

	ROM_REGION( 0x0200, "proms", 0 ) // unknown TI 16-pin DIPs
	ROM_LOAD( "417129-010.u16",  0x0000, 0x0100, NO_DUMP )
	ROM_LOAD( "417129-010.u87",  0x0100, 0x0100, NO_DUMP )
ROM_END

COMP( 1980, dialog80, 0, 0, ampex, ampex, ampex_state, 0, "Ampex", "Dialogue 80", MACHINE_IS_SKELETON )
