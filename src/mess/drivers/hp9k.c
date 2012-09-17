/***************************************************************************

    Skeleton driver for HP9816 (HP Series 200 series, Technical Desktops)

    see for docs: http://www.hpmuseum.net/collection_document.php

    TODO: everything!

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
//#include "video/mc6845.h"
//#include "machine/ins8250.h"


class hp9k_state : public driver_device
{
public:
	hp9k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	,
		m_p_videoram(*this, "p_videoram"){ }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT16> m_p_videoram;
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_hp9k(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


static ADDRESS_MAP_START(hp9k_mem, AS_PROGRAM, 16, hp9k_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x010000, 0x3fffff) AM_RAM // guessing
	AM_RANGE(0x512000, 0x512fff) AM_RAM AM_SHARE("p_videoram") // videoram
	AM_RANGE(0xffc000, 0xffffff) AM_RAM // system ram
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( hp9k )
INPUT_PORTS_END


void hp9k_state::machine_reset()
{
}

/* F4 Character Displayer */
static const gfx_layout hp9k_charlayout =
{
	8, 16,					/* 8 x 16 characters */
	256,					/* 256 characters */
	1,					/* 1 bits per pixel */
	{ 0 },					/* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 1*8, 0*8, 3*8, 2*8, 5*8, 4*8, 7*8, 6*8, 9*8, 8*8, 11*8, 10*8, 13*8, 12*8, 15*8, 14*8 },
	8*16					/* every char takes 16 bytes */
};

static GFXDECODE_START( hp9k )
	GFXDECODE_ENTRY( "maincpu", 0x2000, hp9k_charlayout, 0, 1 )
GFXDECODE_END


void hp9k_state::video_start()
{
}

UINT32 hp9k_state::screen_update_hp9k(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static MACHINE_CONFIG_START( hp9k, hp9k_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(hp9k_mem)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(hp9k_state, screen_update_hp9k)
	MCFG_GFXDECODE(hp9k)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( hp9816 )
	ROM_REGION16_BE(0x1000000, "maincpu", 0)
	ROM_DEFAULT_BIOS("bios40")
	ROM_SYSTEM_BIOS(0, "bios40",  "Bios v4.0")
	ROMX_LOAD( "rom40.bin", 0x0000, 0x10000, CRC(36005480) SHA1(645a077ffd95e4c31f05cd8bbd6e4554b12813f1), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "bios30",  "Bios v3.0")
	ROMX_LOAD( "rom30.bin", 0x0000, 0x10000, CRC(05c07e75) SHA1(3066a65e6137482041f9a77d09ee2289fe0974aa), ROM_BIOS(2) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1982, hp9816,	0,       0, 		hp9k,	hp9k, driver_device,	 0, 	  "Hewlett Packard",   "HP 9816", GAME_NOT_WORKING | GAME_NO_SOUND)
