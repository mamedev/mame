/* Sega Picture Magic (codename JANUS) */
// http://segaretro.org/Sega_Picture_Magic

// this uses a Sega 32X PCB (not in a 32X case) attached to a stripped down 68k based board rather than a full Genesis / Megadrive
// it is likely the internal SH2 bios roms differ


#include "emu.h"
#include "cpu/m68000/m68000.h"


class segapm_state : public driver_device
{
public:
	segapm_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	virtual void video_start() override;
	UINT32 screen_update_segapm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
};


void segapm_state::video_start()
{
}

UINT32 segapm_state::screen_update_segapm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}



static ADDRESS_MAP_START( segapm_map, AS_PROGRAM, 16, segapm_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM

	// A15100

	AM_RANGE(0xe00000, 0xe7ffff) AM_RAM

ADDRESS_MAP_END

static INPUT_PORTS_START( segapm )
INPUT_PORTS_END




static MACHINE_CONFIG_START( segapm, segapm_state )

	MCFG_CPU_ADD("maincpu", M68000, 8000000) // ??
	MCFG_CPU_PROGRAM_MAP(segapm_map)

	// + 2 sh2s on 32x board

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(segapm_state, screen_update_segapm)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x200)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)
MACHINE_CONFIG_END



ROM_START( segapm ) // was more than one cartridge available? if so softlist them?
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "picture magic boot cart (j) [!].bin", 0x00000, 0x80000, CRC(c9ab4e60) SHA1(9c4d4ab3e59c8acde86049a1ba3787aa03b549a3) ) // internal header is GOUSEI HENSYUU

	// todo, sh2 bios roms etc.
ROM_END

GAME( 1996, segapm,    0,        segapm,    segapm, driver_device,    0, ROT0,  "Sega", "Picture Magic", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
