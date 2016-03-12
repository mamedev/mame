// license:BSD-3-Clause
// copyright-holders:David Haywood

// Note, another VERY different version of this also exists, see https://www.youtube.com/watch?v=5rtqZqMBACI (uploaded by Chris Hardy)
// it is unclear if that version is on the same hardware as this

#include "emu.h"
#include "cpu/z80/z80.h"



class androidp_state : public driver_device
{
public:
	androidp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_bgram(*this, "bgram")
	
		{ }

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_androidp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<UINT8> m_bgram;
	tilemap_t *m_bg_tilemap;
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	DECLARE_WRITE8_MEMBER(androidp_bgram_w);
	DECLARE_WRITE8_MEMBER(bg_scrollx_w);

	DECLARE_WRITE8_MEMBER(port_3_w);

	DECLARE_WRITE8_MEMBER(port_6_w);
	DECLARE_WRITE8_MEMBER(port_7_w);
	DECLARE_WRITE8_MEMBER(port_8_w);
	DECLARE_WRITE8_MEMBER(port_9_w);

	DECLARE_WRITE8_MEMBER(port_b_w);


};

TILE_GET_INFO_MEMBER(androidp_state::get_bg_tile_info)
{
	int code = (m_bgram[tile_index*2]);
	int attr = (m_bgram[(tile_index*2)+1]);

	code |= (attr & 0x7) << 8;

	SET_TILE_INFO_MEMBER(1,
			code,
			0,
			0);
}

WRITE8_MEMBER(androidp_state::androidp_bgram_w)
{
	m_bgram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset/2);
}



void androidp_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(androidp_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}

UINT32 androidp_state::screen_update_androidp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

WRITE8_MEMBER(androidp_state::bg_scrollx_w)
{
	m_bg_tilemap->set_scrollx(0, data);
}


WRITE8_MEMBER(androidp_state::port_3_w)
{
	// 9b on startup
//	printf("port3_w %02x\n", data);
}

WRITE8_MEMBER(androidp_state::port_6_w)
{
	// seems most likely candidate for ROM BANK

	// 04 during title, 08 during high score
	//printf("port6_w %02x\n", data);
}

WRITE8_MEMBER(androidp_state::port_7_w)
{
	// 92 on startup
	//printf("port7_w %02x\n", data);
}

WRITE8_MEMBER(androidp_state::port_8_w)
{
	// 00 between screens
	//printf("port8_w %02x\n", data);
}

WRITE8_MEMBER(androidp_state::port_9_w)
{
	if (data!=0x00)
		printf("port9_w %02x\n", data);
}

WRITE8_MEMBER(androidp_state::port_b_w)
{
	// 00 on startup
	// 23 ff between scenes
//	printf("portb_w %02x\n", data);
}

static ADDRESS_MAP_START( androidp_map, AS_PROGRAM, 8, androidp_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_RAM
	AM_RANGE(0xa000, 0xa0ff) AM_RAM  // sprites?
	AM_RANGE(0xa800, 0xa8ff) AM_RAM  // palette?
	AM_RANGE(0xac00, 0xacff) AM_RAM  // palette?
	AM_RANGE(0xb000, 0xbfff) AM_RAM_WRITE(androidp_bgram_w) AM_SHARE("bgram")
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( androidp_portmap, AS_IO, 8, androidp_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("02") // probably from sub-cpu (inputs read by that?)

	AM_RANGE(0x03, 0x03) AM_WRITE( port_3_w )

	AM_RANGE(0x06, 0x06) AM_WRITE( port_6_w )
	AM_RANGE(0x07, 0x07) AM_WRITE( port_7_w )
	AM_RANGE(0x08, 0x08) AM_WRITE( port_8_w )
	AM_RANGE(0x09, 0x09) AM_WRITE( port_9_w )

	AM_RANGE(0x0a, 0x0a) AM_WRITE( bg_scrollx_w )
	AM_RANGE(0x0b, 0x0b) AM_WRITE( port_b_w )

ADDRESS_MAP_END



static INPUT_PORTS_START( androidp )

	PORT_START("02")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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



static const gfx_layout sprite16x16_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4,0  ,12,8,  20,16,  28,24,  36,32,  44,40,  52, 48,  60, 56},
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64 },
	64*16
};

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( androidp )
	GFXDECODE_ENTRY( "sprites", 0, sprite16x16_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END



void androidp_state::machine_start()
{
	membank("bank1")->configure_entries(0, 4, memregion("maincpu")->base() + 0x8000, 0x4000);
}

void androidp_state::machine_reset()
{
	membank("bank1")->set_entry(0);
}

static MACHINE_CONFIG_START( androidp, androidp_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,4000000)         /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(androidp_map)
	MCFG_CPU_IO_MAP(androidp_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", androidp_state,  irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, (30*8)-1, 0, (24*8)-1)
	MCFG_SCREEN_UPDATE_DRIVER(androidp_state, screen_update_androidp)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", androidp)
	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)
	MCFG_PALETTE_ENDIANNESS(ENDIANNESS_BIG)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END



ROM_START( androidp )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "MITSUBISHI_A01.toppcb.m5l27256k.k1.BIN", 0x00000, 0x08000, CRC(25ab85eb) SHA1(e1fab149c83ff880b119258206d5818f3db641c5) )
	ROM_LOAD( "MITSUBISHI_A02.toppcb.m5l27256k.J1.BIN", 0x08000, 0x08000, CRC(e41426be) SHA1(e7e06ef3ff5160bb7d870e148ba2799da52cf24c) )
	ROM_LOAD( "MITSUBISHI_A03.toppcb.m5l27256k.G1.BIN", 0x10000, 0x08000, CRC(6cf5f48a) SHA1(b9b4e5e7bace0e8d98fbc9f4ad91bc56ef42099e) )

	ROM_REGION( 0x18000, "soundcpu", 0 )
	ROM_LOAD( "MITSUBISHI_A04.toppcb.m5l27256k.N6.BIN", 0x00000, 0x08000, CRC(13c38fe4) SHA1(34a35fa057159a5c83892a88b8c908faa39d5cb3) )	

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD16_BYTE( "MITSUBISHI_A06.botpcb.m5l27512k.9E.BIN", 0x00000, 0x10000, CRC(5e42984e) SHA1(2a928960c740dfb94589e011cce093bed2fd7685) )
	ROM_LOAD16_BYTE( "MITSUBISHI_A07.botpcb.m5l27512k.9B.BIN", 0x00001, 0x10000, CRC(611ff400) SHA1(1a9aed33d0e3f063811f92b9fee3ecbff0e965bf) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "MITSUBISHI_A05.toppcb.m5l27512k.F5.BIN", 0x00000, 0x10000, CRC(4c72a930) SHA1(f1542844391b55fe43293eef7ce48c09b7aca75a) )

	// + 2 undumped PLDs
ROM_END


GAME( 198?, androidp, 0, androidp, androidp, driver_device, 0, ROT90, "Nasco",         "Android (early build?)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
