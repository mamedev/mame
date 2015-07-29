// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Korg M1 (c) 1988

    skeleton driver

    Note: driver isn't yet hooked up to mess.lst / mess.mak, until a ROM
    dump is found.

***************************************************************************/


#include "emu.h"
#include "cpu/nec/nec.h"
//#include "sound/ay8910.h"

#define MAIN_CLOCK XTAL_16MHz // Unknown clock

class korgm1_state : public driver_device
{
public:
	korgm1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
	virtual void palette_init();
};

void korgm1_state::video_start()
{
}

UINT32 korgm1_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return 0;
}

static ADDRESS_MAP_START( korgm1_map, AS_PROGRAM, 16, korgm1_state )
	AM_RANGE(0x00000, 0x0ffff) AM_RAM // 64 KB
//  AM_RANGE(0x50000, 0x57fff) AM_RAM // memory card 32 KB
	AM_RANGE(0xe0000, 0xfffff) AM_ROM AM_REGION("ipl", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( korgm1_io, AS_IO, 16, korgm1_state )
//  AM_RANGE(0x0000, 0x00ff) internal peripheral (?)
//  AM_RANGE(0x0100, 0x01ff) VDF 1 (MB87404)
//  AM_RANGE(0x0200, 0x02ff) VDF 2 (MB87404)
//  AM_RANGE(0x0500, 0x0503) MDE (MB87405)
//  AM_RANGE(0x0600, 0x0601) OPZ 1 (8-bit port)
//  AM_RANGE(0x0700, 0x0701) OPZ 2 (8-bit port)
//  AM_RANGE(0x0800, 0x0801) SCAN (8-bit port) (keyboard)
//  AM_RANGE(0x0900, 0x09??) A/D Converter (M58990P, Joystick, "value" and After Touch routes here) **
//  AM_RANGE(0x0a00, 0x0a03) PPI (CXD1095, presumably i8255 compatible, LCD, LED and SW routes here) *
//  AM_RANGE(0x0b00, 0x0b01) LCDC (8-bit port)
//  AM_RANGE(0x1000, 0x11ff) TG (MB87402)
//  AM_RANGE(0x2000, 0x23ff) SCSI
//  AM_RANGE(0x3000, 0x33ff) FDC
//  TG 2?
ADDRESS_MAP_END

// * Rail Chase 2 shares this (iocpu)
// ** Lock-On shares this

static INPUT_PORTS_START( korgm1 )
	/* dummy active high structure */
	PORT_START("SYSA")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	/* dummy active low structure */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

#if 0
static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ RGN_FRAC(0,1) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};
#endif

static GFXDECODE_START( korgm1 )
//  GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 )
GFXDECODE_END


void korgm1_state::machine_start()
{
}

void korgm1_state::machine_reset()
{
}


PALETTE_INIT_MEMBER(korgm1_state, korgm1)
{
}

static MACHINE_CONFIG_START( korgm1, korgm1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",V30,MAIN_CLOCK) // V50 actually
	MCFG_CPU_PROGRAM_MAP(korgm1_map)
	MCFG_CPU_IO_MAP(korgm1_io)

	/* video hardware */
	/* TODO: LCD actually */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(korgm1_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", korgm1)

	MCFG_PALETTE_ADD("palette", 8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( korgm1 )
	ROM_REGION( 0x20000, "ipl", ROMREGION_ERASE00 )
	ROM_LOAD( "bios.rom", 0x00000, 0x20000, NO_DUMP )

	ROM_REGION( 0x200000, "pcm", ROMREGION_ERASE00 )
	ROM_LOAD( "pcm.rom", 0x00000, 0x200000, NO_DUMP )

//  ROM_REGION( 0x10000, "gfx1", ROMREGION_ERASE00 )
ROM_END

GAME( 1988, korgm1,  0,   korgm1,  korgm1,  0,       ROT0, "Korg",      "M1", MACHINE_IS_SKELETON )
