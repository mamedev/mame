/*********************************************************************************

   Super Duck

   hardware appears to be roughly based off Bionic Commando, close to the
   Tiger Road / F1-Dream based Pushman / Bouncing Balls.


*********************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"


class supduck_state : public driver_device
{
public:
	supduck_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu"),
			m_tx_videoram(*this, "txvideoram"),
			m_gfxdecode(*this, "gfxdecode")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<z80_device> m_audiocpu;

	// shared pointers
	required_shared_ptr<UINT16> m_tx_videoram;

	required_device<gfxdecode_device> m_gfxdecode;

	tilemap_t     *m_tx_tilemap;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE16_MEMBER(tx_videoram_w);

protected:

	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();

	TILE_GET_INFO_MEMBER(get_tx_tile_info);
};

void supduck_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(supduck_state::get_tx_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

UINT32 supduck_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


WRITE16_MEMBER(supduck_state::tx_videoram_w)
{
	COMBINE_DATA(&m_tx_videoram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(supduck_state::get_tx_tile_info)
{
	int data = m_tx_videoram[tile_index];
	
	int tileno = data & 0xff;
//	tileno |= (data & 0x7000) >> 4;

	SET_TILE_INFO_MEMBER(0, tileno, 0, 0);
}




static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, supduck_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0xfe0000, 0xfe1fff) AM_RAM
//	AM_RANGE(0xfe0000, 0xfe07ff) AM_RAM /* RAM? */
//	AM_RANGE(0xfe0800, 0xfe0cff) AM_RAM AM_SHARE("spriteram")
//	AM_RANGE(0xfe0d00, 0xfe3fff) AM_RAM              /* RAM? */
//	AM_RANGE(0xfe4000, 0xfe4001) AM_WRITE(bionicc_gfxctrl_w)    /* + coin counters */
//	AM_RANGE(0xfe4000, 0xfe4001) AM_READ_PORT("SYSTEM")
//	AM_RANGE(0xfe4002, 0xfe4003) AM_READ(supduck_random_r)
	AM_RANGE(0xfe4004, 0xfe4005) AM_READ_PORT("DSW")
//	AM_RANGE(0xfe8010, 0xfe8017) AM_WRITE(bionicc_scroll_w)
//	AM_RANGE(0xfe801a, 0xfe801b) AM_WRITE(bionicc_mpu_trigger_w)    /* ??? not sure, but looks like it */
	AM_RANGE(0xfec000, 0xfecfff) AM_RAM_WRITE(tx_videoram_w) AM_SHARE("txvideoram")
	AM_RANGE(0xff0000, 0xff3fff) AM_RAM // AM_RAM_WRITE(bionicc_fgvideoram_w) AM_SHARE("fgvideoram")
	AM_RANGE(0xff4000, 0xff7fff) AM_RAM // AM_RAM_WRITE(bionicc_bgvideoram_w) AM_SHARE("bgvideoram")
	AM_RANGE(0xff8000, 0xff87ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xffc000, 0xffffff) AM_RAM /* working RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, supduck_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( supduck )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END



static const gfx_layout spritelayout_bionicc=
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,4),   /* 2048 sprites */
	4,      /* 4 bits per pixel */
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{
		0,1,2,3,4,5,6,7,
		(16*8)+0,(16*8)+1,(16*8)+2,(16*8)+3,
		(16*8)+4,(16*8)+5,(16*8)+6,(16*8)+7
	},
	{
		0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8,
	},
	256   /* every sprite takes 256 consecutive bytes */
};

static const gfx_layout vramlayout_bionicc=
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),   /* 1024 character */
	2,      /* 2 bitplanes */
	{ 4,0 },
	{ 0,1,2,3,8,9,10,11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	128   /* every character takes 128 consecutive bytes */
};

static const gfx_layout scroll2layout_bionicc=
{
	8,8,    /* 8*8 tiles */
	RGN_FRAC(1,2),   /* 2048 tiles */
	4,      /* 4 bits per pixel */
	{ RGN_FRAC(1,2)+4,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+4,RGN_FRAC(0,2)+0 },
	{ 0,1,2,3, 8,9,10,11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	128   /* every tile takes 128 consecutive bytes */
};

static const gfx_layout scroll1layout_bionicc=
{
	16,16,  /* 16*16 tiles */
	RGN_FRAC(1,2),   /* 2048 tiles */
	4,      /* 4 bits per pixel */
	{ RGN_FRAC(1,2)+4,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+4,RGN_FRAC(0,2)+0 },
	{
		0,1,2,3, 8,9,10,11,
		(8*4*8)+0,(8*4*8)+1,(8*4*8)+2,(8*4*8)+3,
		(8*4*8)+8,(8*4*8)+9,(8*4*8)+10,(8*4*8)+11
	},
	{
		0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16
	},
	512   /* each tile takes 512 consecutive bytes */
};

static GFXDECODE_START( supduck )
	GFXDECODE_ENTRY( "gfx1", 0, vramlayout_bionicc,    768, 64 )    /* colors 768-1023 */
	GFXDECODE_ENTRY( "gfx2", 0, scroll2layout_bionicc,   0,  4 )    /* colors   0-  63 */
	GFXDECODE_ENTRY( "gfx3", 0, scroll1layout_bionicc, 256,  4 )    /* colors 256- 319 */
	GFXDECODE_ENTRY( "gfx4", 0, spritelayout_bionicc,  512, 16 )    /* colors 512- 767 */
GFXDECODE_END



void supduck_state::machine_start()
{
}

void supduck_state::machine_reset()
{
}

/*

Crystal 1: 8mhz
Crystal 2: 24mhz

Following all come from crystal 1
68k - 8mhz
Z80 - 2mhz
OKI M6295 - 1mhz

*/

static MACHINE_CONFIG_START( supduck, supduck_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 8000000)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", supduck_state,  irq2_line_hold) // 2 & 4?

	MCFG_CPU_ADD("audiocpu", Z80, 8000000/4)
	MCFG_CPU_PROGRAM_MAP(sound_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_UPDATE_DRIVER(supduck_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", supduck)

	MCFG_PALETTE_ADD("palette", 0x800/2)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 8000000/8, OKIM6295_PIN7_HIGH) // pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( supduck )
	ROM_REGION( 0x40000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "5.u16n", 0x00000, 0x20000, CRC(837a559a) SHA1(ed5ad744a4145dfbef56ad2e6eec3ff14c20de1c) )
	ROM_LOAD16_BYTE( "6.u16l", 0x00001, 0x20000, CRC(508e9905) SHA1(2da3f12caa29066b4d54b22573cfdfcea8916f99) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "4.su6",  0x00000, 0x8000, CRC(d75863ea) SHA1(497d11b86f4f69134943fc3448d195c6e7acbe8f) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "3.cu15",   0x00000, 0x8000, CRC(b1cacca4) SHA1(b4a486618197cf2b85a121b5640cd773b2d453fc) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "7.uu29",   0x00000, 0x20000, CRC(f3251b20) SHA1(8ebb9b98324de14356c9a57ae8a77dc4118fb5c2) )
	ROM_LOAD( "8.uu30",   0x20000, 0x20000, CRC(03c60cbd) SHA1(bf3be7161f69187350eb9d9d4209b93f8b67d0f1) )
	ROM_LOAD( "9.uu31",   0x40000, 0x20000, CRC(9b6d3430) SHA1(ade2decc5bcf817498b1198a2244d1c65bc20bea) )
	ROM_LOAD( "10.uu32",  0x60000, 0x20000, CRC(beed2616) SHA1(c077a3de4a6d451a568694ab70e85830d585a41d) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "11.ul29",   0x00000, 0x20000, CRC(1b6958a4) SHA1(ca93f898702e14ece24d5cfced38d622d3596d0f) )
	ROM_LOAD( "12.ul30",   0x20000, 0x20000, CRC(3e6bd24b) SHA1(f93b5c78d815bd30ecb9cfe2cd257548e467e852) )
	ROM_LOAD( "13.ul31",   0x40000, 0x20000, CRC(bff7b7cd) SHA1(2f65cadcfcc02fe31ba721eea9f45d4a729e4374) )
	ROM_LOAD( "14.ul32",   0x60000, 0x20000, CRC(97a7310b) SHA1(76b82bfea64b59890c0ba2e1688b7321507a4da7) )

	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "15.u1d",   0x00000, 0x20000, CRC(81bf1f27) SHA1(7a66630a2da85387904917d3c136880dffcb9649) )
	ROM_LOAD( "16.u2d",   0x20000, 0x20000, CRC(9573d6ec) SHA1(9923be782bae47c49913d01554bcf3e5efb5395b) )
	ROM_LOAD( "17.u1c",   0x40000, 0x20000, CRC(21ef14d4) SHA1(66e389aaa1186921a07da9a9a9eda88a1083ad42) )
	ROM_LOAD( "18.u2c",   0x60000, 0x20000, CRC(33dd0674) SHA1(b95dfcc16d939bac77f338b8a8cada19328a1993) ) 

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "2.su12",   0x00000, 0x20000, CRC(745d42fb) SHA1(f9aee3ddbad3cc2f3a7002ee0d762eb041967e1e) ) // static sample data
	
	ROM_REGION( 0x80000, "okibank", 0 )
	ROM_LOAD( "1.su13",   0x00000, 0x80000, CRC(7fb1ed42) SHA1(77ec86a6454398e329066aa060e9b6a39085ce71) ) // banked sample data
ROM_END

GAME( 1993, supduck, 0, supduck, supduck, driver_device, 0, ROT0, "Comad", "Super Duck", GAME_NOT_WORKING )
