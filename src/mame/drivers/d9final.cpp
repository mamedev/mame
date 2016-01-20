// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood
/*******************************************************************************************

    Dream 9 Final (c) 1992 Excellent Systems

    driver by Angelo Salese & David Haywood

    TODO:
    - Don't know where the ES8712 & RTC62421b chips route;
    - A bunch of missing port outputs;
    - screen disable? Start-up fading looks horrible;
    - Game looks IGS-esque, is there any correlation?

============================================================================================

    PCB: ES-9112

    Main Chips: Z80, ES8712, 24Mhz OSC, RTC62421B 9262, YM2413, 4x8DSW

*******************************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2413intf.h"


class d9final_state : public driver_device
{
public:
	d9final_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_lo_vram(*this, "lo_vram"),
		m_hi_vram(*this, "hi_vram"),
		m_cram(*this, "cram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<UINT8> m_lo_vram;
	required_shared_ptr<UINT8> m_hi_vram;
	required_shared_ptr<UINT8> m_cram;

	tilemap_t *m_sc0_tilemap;

	DECLARE_WRITE8_MEMBER(sc0_lovram);
	DECLARE_WRITE8_MEMBER(sc0_hivram);
	DECLARE_WRITE8_MEMBER(sc0_cram);
	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_READ8_MEMBER(prot_latch_r);

	TILE_GET_INFO_MEMBER(get_sc0_tile_info);

	virtual void machine_start() override;
	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};



TILE_GET_INFO_MEMBER(d9final_state::get_sc0_tile_info)
{
	int tile = ((m_hi_vram[tile_index] & 0x3f)<<8) | m_lo_vram[tile_index];
	int color = m_cram[tile_index] & 0x3f;

	SET_TILE_INFO_MEMBER(0,
			tile,
			color,
			0);
}

void d9final_state::video_start()
{
	m_sc0_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(d9final_state::get_sc0_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,32);
}

UINT32 d9final_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_sc0_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

WRITE8_MEMBER(d9final_state::sc0_lovram)
{
	m_lo_vram[offset] = data;
	m_sc0_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(d9final_state::sc0_hivram)
{
	m_hi_vram[offset] = data;
	m_sc0_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(d9final_state::sc0_cram)
{
	m_cram[offset] = data;
	m_sc0_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(d9final_state::bank_w)
{
	membank("bank1")->set_entry(data & 0x7);
}

/* game checks this after three attract cycles, otherwise coin inputs stop to work. */
READ8_MEMBER(d9final_state::prot_latch_r)
{
//  printf("PC=%06x\n",space.device().safe_pc());

	return 0x04;
}


static ADDRESS_MAP_START( d9final_map, AS_PROGRAM, 8, d9final_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xcbff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xcc00, 0xcfff) AM_RAM_DEVWRITE("palette", palette_device, write_ext) AM_SHARE("palette_ext")
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(sc0_lovram) AM_SHARE("lo_vram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM_WRITE(sc0_hivram) AM_SHARE("hi_vram")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(sc0_cram) AM_SHARE("cram")
	AM_RANGE(0xf000, 0xf000) AM_READ(prot_latch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( d9final_io, AS_IO, 8, d9final_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//  AM_RANGE(0x00, 0x00) AM_WRITENOP //bit 0: irq enable? screen enable?
	AM_RANGE(0x00, 0x00) AM_READ_PORT("DSWA")
	AM_RANGE(0x20, 0x20) AM_READ_PORT("DSWB")
	AM_RANGE(0x40, 0x40) AM_READ_PORT("DSWC")
	AM_RANGE(0x40, 0x41) AM_DEVWRITE("ymsnd", ym2413_device, write)
	AM_RANGE(0x60, 0x60) AM_READ_PORT("DSWD")
	AM_RANGE(0x80, 0x80) AM_READ_PORT("IN0")
	AM_RANGE(0xa0, 0xa0) AM_READ_PORT("IN1") AM_WRITE(bank_w)
	AM_RANGE(0xe0, 0xe0) AM_READ_PORT("IN2")
ADDRESS_MAP_END

static INPUT_PORTS_START( d9final )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Reset")
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Analyzer")

	PORT_START("IN1")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) //another reset button
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x32, 0x32, "Credit Limit" ) PORT_DIPLOCATION("SW1:2,5,6")
	PORT_DIPSETTING(    0x32, "1000" )
	PORT_DIPSETTING(    0x12, "5000" )
	PORT_DIPSETTING(    0x22, "10000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x02, "50000" )
// 0x10 20000
// 0x30 20000
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPNAME( 0x08, 0x08, "Auto Start" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Hopper Switch" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( High ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPNAME( 0x80, 0x80, "Payout" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB") //odd rates / difficulty stuff
	PORT_DIPNAME( 0x07, 0x07, "Win Percentage" ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x00, "60%" )
	PORT_DIPSETTING(    0x04, "65%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x06, "75%" )
	PORT_DIPSETTING(    0x01, "80%" )
	PORT_DIPSETTING(    0x05, "85%" )
	PORT_DIPSETTING(    0x03, "90%" )
	PORT_DIPSETTING(    0x07, "95%" )
	PORT_DIPNAME( 0x18, 0x18, "Bet Max" ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "8" )
	PORT_DIPSETTING(    0x08, "16" )
	PORT_DIPSETTING(    0x10, "32" )
	PORT_DIPSETTING(    0x00, "64" )
	PORT_DIPNAME( 0x60, 0x60, "Double-Up Difficulty" ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("DSWD") //coinage C & D
	PORT_DIPNAME( 0x0f, 0x0e, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW3:1,2,3,4")
	PORT_DIPSETTING(    0x00, "10 Coins / 1 Credit" )
	PORT_DIPSETTING(    0x08, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, "5 Coins / 2 Credits" )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0d, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x0b, "1 Coin / 25 Credits" )
	PORT_DIPSETTING(    0x07, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x0f, "1 Coin / 100 Credits" )
	PORT_DIPNAME( 0x70, 0x30, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW3:5,6,7")
	PORT_DIPSETTING(    0x00, "10 Coins / 1 Credit" )
	PORT_DIPSETTING(    0x40, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x70, "1 Coin / 50 Credits" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW3:8" )

	PORT_START("DSWC") //coinage C & Key In Coinage
	PORT_DIPNAME( 0x07, 0x00, "Coin C" ) PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x01, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin / 25 Credits" )
	PORT_DIPSETTING(    0x07, "1 Coin / 50 Credits" )
	PORT_DIPNAME( 0x38, 0x00, "Key In Credit" ) PORT_DIPLOCATION("SW4:4,5,6")
	PORT_DIPSETTING(    0x00, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x10, "1 Coin / 40 Credits" )
	PORT_DIPSETTING(    0x30, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin / 100 Credits" )
	PORT_DIPSETTING(    0x28, "1 Coin / 200 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin / 250 Credits" )
	PORT_DIPSETTING(    0x38, "1 Coin / 500 Credits" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW4:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW4:8" )
INPUT_PORTS_END

static const gfx_layout tiles16x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 8, 0, 12, 4, 24, 16, 28, 20 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( d9final )
	GFXDECODE_ENTRY( "gfx1", 0, tiles16x8_layout, 0, 16*4 )
GFXDECODE_END

void d9final_state::machine_start()
{
	membank("bank1")->configure_entries(0, 8, memregion("maincpu")->base() + 0x10000, 0x4000);
}

static MACHINE_CONFIG_START( d9final, d9final_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 24000000/4)/* ? MHz */
	MCFG_CPU_PROGRAM_MAP(d9final_map)
	MCFG_CPU_IO_MAP(d9final_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", d9final_state,  irq0_line_hold)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(d9final_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", d9final)
	MCFG_PALETTE_ADD_INIT_BLACK("palette", 0x400)
	MCFG_PALETTE_FORMAT(xxxxBBBBRRRRGGGG)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


ROM_START( d9final )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "2.4h", 0x00000, 0x8000, CRC(a8d838c8) SHA1(85b2cd1b73569e0e4fc13bfff537cfc2b4d569a1)  )
	ROM_CONTINUE(        0x10000, 0x08000 )
	ROM_COPY( "maincpu", 0x10000, 0x18000, 0x08000 ) //or just 0xff
	ROM_LOAD( "1.2h", 0x20000, 0x10000, CRC(901281ec) SHA1(7b4cae343f1b025d988a507141c0fa8229a0fea1)  )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "3.13h", 0x00001, 0x40000, CRC(a2de0cce) SHA1(d510671b75417c10ce479663f6f21367121384b4) )
	ROM_LOAD16_BYTE( "4.15h", 0x00000, 0x40000, CRC(859b7105) SHA1(1b36f84706473afaa50b6546d7373a2ee6602b9a) )
ROM_END



GAME( 1992, d9final, 0, d9final, d9final, driver_device, 0, ROT0, "Excellent System", "Dream 9 Final (v2.24)", MACHINE_SUPPORTS_SAVE )
