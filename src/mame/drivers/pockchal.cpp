// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

base unit contains

1x Toshiba TMP90C845AF

1x SANYO LC21003 BLA5

3x SEC C941A KS0108B

1x Toshiba T9842B

(system has no bios ROM)


*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/tlcs90/tlcs90.h"
#include "softlist.h"

class pockchalv1_state : public driver_device
{
public:
	pockchalv1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	 { }

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_pockchalv1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
};



void pockchalv1_state::video_start()
{
}

UINT32 pockchalv1_state::screen_update_pockchalv1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


static ADDRESS_MAP_START( pockchalv1_map, AS_PROGRAM, 8, pockchalv1_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( pockchalv1 )
INPUT_PORTS_END



void pockchalv1_state::machine_start()
{
}

void pockchalv1_state::machine_reset()
{
}

static MACHINE_CONFIG_START( pockchalv1, pockchalv1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMP90845,8000000)         /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(pockchalv1_map)
//	MCFG_CPU_VBLANK_INT_DRIVER("screen", pockchalv1_state,  irq0_line_hold)

	// wrong, it's a b&w / greyscale thing
	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)
	MCFG_PALETTE_ENDIANNESS(ENDIANNESS_BIG)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(pockchalv1_state, screen_update_pockchalv1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("pc1_list","pockchalv1")

MACHINE_CONFIG_END



ROM_START( pockchal )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
ROM_END

GAME( 199?, pockchal,  0,    pockchalv1, pockchalv1, driver_device,  0, ROT0, "Benesse Corporation", "Pocket Challenge (Japan)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
