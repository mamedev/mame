/***************************************************************************

============================================================================

SEGA GOLDEN POKER SERIES "JOKER'S WILD" (REV.B)
(c) SEGA

MAIN CPU  : 68000 Z-80
CRTC      : HITACHI HD63484 (24KHz OUTPUT)
SOUND     : YM3438

14584B.EPR  ; MAIN BOARD  IC20 EPR-14584B (27C1000 MAIN-ODD)
14585B.EPR  ; MAIN BOARD  IC22 EPR-14585B (27C1000 MAIN-EVEN)
14586.EPR   ; MAIN BOARD  IC26 EPR-14586  (27C4096 BG)
14587A.EPR  ; SOUND BOARD IC51 EPR-14587A (27C1000 SOUND)

------------------------------------------------------------------

***************************************************************************/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "video/hd63484.h"

class segajw_state : public driver_device
{
public:
	segajw_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual bool screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect);
};


void segajw_state::video_start()
{

}

bool segajw_state::screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect)
{
	return 0;
}


static ADDRESS_MAP_START( segajw_map, AS_PROGRAM, 16, segajw_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM

//  AM_RANGE(0x080000, 0x080001) AM_DEVREADWRITE_LEGACY("hd63484", hd63484_status_r, hd63484_address_w)
//  AM_RANGE(0x080002, 0x080003) AM_DEVREADWRITE_LEGACY("hd63484", hd63484_data_r, hd63484_data_w)

	AM_RANGE(0x1a000e, 0x1a000f) AM_NOP
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( segajw )
INPUT_PORTS_END


void segajw_state::machine_start()
{
}


void segajw_state::machine_reset()
{
}

static PALETTE_INIT( segajw )
{

}

static MACHINE_CONFIG_START( segajw, segajw_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000,8000000) // unknown clock
	MCFG_CPU_PROGRAM_MAP(segajw_map)
	MCFG_CPU_VBLANK_INT("screen",irq4_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MCFG_PALETTE_INIT(segajw)
	MCFG_PALETTE_LENGTH(16)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4) /* guess */
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( segajw )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "14584b.epr",   0x00001, 0x20000, CRC(d3a6d63d) SHA1(ce9d4769b7514294a91af1dfd7cd10ee40b3572c) )
	ROM_LOAD16_BYTE( "14585b.epr",   0x00000, 0x20000, CRC(556d0a62) SHA1(d2def433a511cbdebbe2cd0c8e51fc8c4ff1ed7b) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "14587a.epr",   0x00000, 0x20000, CRC(66163b6c) SHA1(88e994bcad86c58dc730a93b48226e9296df7667) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "14586.epr",   0x00000, 0x80000, CRC(daeb0616) SHA1(17a8bb7137ad46a7c3ac07d22cbc4430e76e2f71) )
ROM_END


GAME( 198?, segajw,  0,   segajw,  segajw,  0, ROT0, "Sega", "Golden Poker Series \"Joker's Wild\" (Rev. B)", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS ) // TODO: correct title
