
/*

fruit fresh by chain leisure electronic co., ltd.

cpu 68000 xtal 24Mhz

4* 8 dipswitchs

2 jumpers HOP or SSR positions

SW1 for reset?

2x Altera epm7064lc84

rom 5 and 6 are prg roms

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"





class fresh_state : public driver_device
{
public:
	fresh_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bg_videoram(*this, "bg_videoram"),
		m_bg_videoram_2(*this, "bg_videoram_2"),
		m_paletteram_1(*this, "paletteram_1"),
		m_paletteram_2(*this, "paletteram_2")
	
	{ }

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	required_shared_ptr<UINT16> m_bg_videoram;
	required_shared_ptr<UINT16> m_bg_videoram_2;

	required_shared_ptr<UINT16> m_paletteram_1;
	required_shared_ptr<UINT16> m_paletteram_2;

	DECLARE_WRITE16_MEMBER(fresh_bg_videoram_w);
	TILE_GET_INFO_MEMBER(get_fresh_bg_tile_info);

	DECLARE_READ16_MEMBER( unk_r )
	{
		return machine().rand();
	}
	DECLARE_READ16_MEMBER( unk2_r )
	{
		return 0x10;
	}
	

	virtual void video_start();
	UINT32 screen_update_fresh(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


TILE_GET_INFO_MEMBER(fresh_state::get_fresh_bg_tile_info)
{
	int tileno;
	tileno = m_bg_videoram[tile_index];
	SET_TILE_INFO_MEMBER(2, tileno, 0, 0);
}


WRITE16_MEMBER(fresh_state::fresh_bg_videoram_w)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

void fresh_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fresh_state::get_fresh_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8,  64, 512);
//	m_fg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fresh_state::get_fresh_fg_tile_info),this), TILEMAP_SCAN_ROWS, 4, 4, 128, 64);

//	m_fg_tilemap->set_transparent_pen(255);
}

UINT32 fresh_state::screen_update_fresh(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
//	m_fg_tilemap->draw(bitmap, cliprect, 0, 0);

	return 0;
}


static ADDRESS_MAP_START( fresh_map, AS_PROGRAM, 16, fresh_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM

	AM_RANGE(0xC00000, 0xC0ffff) AM_RAM
	AM_RANGE(0xC10000, 0xC1ffff) AM_RAM
	AM_RANGE(0xC20000, 0xC2ffff) AM_RAM_WRITE( fresh_bg_videoram_w ) AM_SHARE( "bg_videoram" )
	AM_RANGE(0xC30000, 0xC3ffff) AM_RAM AM_SHARE("bg_videoram_2")


	// written together
	AM_RANGE(0xC40000, 0xC417ff) AM_RAM AM_SHARE( "paletteram_1" ) // 16-bit
	AM_RANGE(0xC50000, 0xC517ff) AM_RAM AM_SHARE( "paletteram_2" ) // 8-bit

	AM_RANGE(0xD40000, 0xD40001) AM_READ(unk_r)
	AM_RANGE(0xD70000, 0xD70001) AM_READ(unk2_r)
	AM_RANGE(0xF00000, 0xF0FFFF) AM_RAM

	 
//	AM_RANGE(0x200000, 0x200fff) AM_RAM_WRITE(fresh_bg_videoram_w) AM_SHARE("bg_videoram") // Background
//	AM_RANGE(0x210000, 0x213fff) AM_RAM_WRITE(fresh_fg_videoram_w) AM_SHARE("fg_videoram") // Foreground
//	AM_RANGE(0x220000, 0x2203ff) AM_RAM_WRITE(paletteram_xRRRRRGGGGGBBBBB_word_w) AM_SHARE("paletteram")
//	AM_RANGE(0x230000, 0x230001) AM_WRITE(soundlatch_word_w)
//	AM_RANGE(0x230100, 0x230101) AM_READ_PORT("DSW")
//	AM_RANGE(0x230200, 0x230201) AM_READ_PORT("INPUTS")
ADDRESS_MAP_END

static INPUT_PORTS_START( fresh )
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1, 2,3, 4,5,6,7 },
	{ 0, 8, 16, 24, 32, 40, 48, 56 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};


static GFXDECODE_START( fresh )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0, 2 )
	GFXDECODE_ENTRY( "gfx3", 0, tiles8x8_layout, 0, 2 )
	GFXDECODE_ENTRY( "gfx4", 0, tiles8x8_layout, 0, 2 )
GFXDECODE_END

static MACHINE_CONFIG_START( fresh, fresh_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 24000000/2 )
	MCFG_CPU_PROGRAM_MAP(fresh_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", fresh_state,  irq6_line_hold) // 4,5,6 valid

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(fresh_state, screen_update_fresh)

	MCFG_PALETTE_LENGTH(0x200)
	MCFG_GFXDECODE(fresh)

	/* sound hw? */
MACHINE_CONFIG_END


ROM_START( fresh )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "fruit-fresh5.u44", 0x00001, 0x20000, CRC(cb37d3c5) SHA1(3b7797d475769d37ed1e9774df8d4b5899fb92a3) )
	ROM_LOAD16_BYTE( "fruit-fresh6.u59", 0x00000, 0x20000, CRC(fc0290be) SHA1(02e3b3563b15ae585684a8f510f48a8c90b248fa) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "fruit-fresh1.u18", 0x00000, 0x80000, CRC(ee77cdcd) SHA1(8e162640d23bd1b5a2ed9305cc4b9df1cb0f3e80) )
	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "fruit-fresh3.u19", 0x00000, 0x80000, CRC(80cc71b3) SHA1(89a2272266ccdbd01abbc85c1f8200fa9d8aa441) )
	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "fruit-fresh2.u45", 0x00000, 0x80000, CRC(8a06a1ab) SHA1(4bc020e4a031df995e6ebaf49d62989004092b60) )
	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "fruit-fresh4.u46", 0x00000, 0x80000, CRC(9b6c7571) SHA1(649cf3c50e2cd8c02f0f730e5ded59cf0ea37c37) )

ROM_END




GAME( 199?, fresh, 0, fresh, fresh, driver_device, 0, ROT0, "Chain Leisure", "Fruit Fresh", GAME_NOT_WORKING|GAME_NO_SOUND )
