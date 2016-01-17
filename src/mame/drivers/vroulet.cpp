// license:BSD-3-Clause
// copyright-holders:Curt Coder,Tomasz Slanina
/*

    Vegas Roulette
    World Games 1989

    LE1000

           6116  PGM  Z80    8255

                             8255  SW1

    GFX  6116                YM2149   SW3
                                      SW2
                        2148
                        2148
                        2148
                N4
    24MHz

    ---

    Driver by Curt Coder

TODO:

Find 'payout on' command to add simulator

Tomasz Slanina 20050225
 - colors (4bpp tiles and 3bpp palette ? something is wrong then ....)
 - 8255x2
 - ball sprite (maybe it's something else in real machine , not sprite)
   (hardcoded tile number and palette for now .. maybe x/y must be swapped)
   are writes to 8000/c000 related to sprite tile/pal ?

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"


class vroulet_state : public driver_device
{
public:
	vroulet_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_generic_paletteram_8(*this, "paletteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_ball(*this, "ball") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT8> m_generic_paletteram_8;

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_ball;

	tilemap_t *m_bg_tilemap;

	DECLARE_WRITE8_MEMBER(paletteram_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(colorram_w);
	DECLARE_WRITE8_MEMBER(ppi8255_a_w);
	DECLARE_WRITE8_MEMBER(ppi8255_b_w);
	DECLARE_WRITE8_MEMBER(ppi8255_c_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/* video */


WRITE8_MEMBER(vroulet_state::paletteram_w)
{
	/*
	 paletteram_xxxxBBBBGGGGRRRR_byte_be_w
	 but... each palette has 8 colors only, not 16 as expected...
	*/

	int i,j,a,b;
	m_generic_paletteram_8[offset]=data;
	for(i=0;i<32;i++)
	{
		for(j=0;j<16;j++)
		{
			a=m_generic_paletteram_8[((i*8+j)*2)&0xff ];
			b=m_generic_paletteram_8[((i*8+j)*2+1)&0xff ];
			m_palette->set_pen_color(i*16+j,pal4bit(b),pal4bit(b>>4),pal4bit(a));
		}
	}
}

WRITE8_MEMBER(vroulet_state::videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(vroulet_state::colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(vroulet_state::get_bg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + ((attr & 0xc0) << 2);
	int color = attr & 0x1f;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void vroulet_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(vroulet_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS,
		8, 8, 32, 32);
}

UINT32 vroulet_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, 0x320, 1, 0, 0,
		m_ball[1], m_ball[0] - 12, 0);
	return 0;
}

/* Memory Maps */

static ADDRESS_MAP_START( vroulet_map, AS_PROGRAM, 8, vroulet_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x67ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x8000, 0x8000) AM_NOP
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9400, 0x97ff) AM_RAM_WRITE(colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xa000, 0xa001) AM_RAM AM_SHARE("ball")
	AM_RANGE(0xb000, 0xb0ff) AM_WRITE(paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0xc000, 0xc000) AM_NOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( vroulet_io_map, AS_IO, 8, vroulet_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( vroulet )
	PORT_START("IN0")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F1) PORT_NAME("Memory Reset")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Reset Machine")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("Payout")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Red")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Blue")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("6")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("10")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("20")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x02, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x38, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0xc0, 0xc0, "Revolutions" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Max Payout Adjust" )
	PORT_DIPSETTING(    0x02, "48" )
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPNAME( 0x04, 0x04, "Extra Payout Control" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Red & Blue Select" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Winning %" )
	PORT_DIPSETTING(    0xc0, "50%" )
	PORT_DIPSETTING(    0xa0, "60%" )
	PORT_DIPSETTING(    0x80, "65%" )
	PORT_DIPSETTING(    0x60, "70%" )
	PORT_DIPSETTING(    0xe0, "75%" )
	PORT_DIPSETTING(    0x40, "80%" )
	PORT_DIPSETTING(    0x20, "90%" )
	PORT_DIPSETTING(    0x00, "100%" )
INPUT_PORTS_END

/* Graphics Layout */

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

/* Graphics Decode Information */

static GFXDECODE_START( vroulet )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout,    0, 32 )
GFXDECODE_END

/* PPI8255 Interface */

WRITE8_MEMBER(vroulet_state::ppi8255_a_w){}// watchdog ?
WRITE8_MEMBER(vroulet_state::ppi8255_b_w){}// lamps ?
WRITE8_MEMBER(vroulet_state::ppi8255_c_w){}

/* Machine Driver */

static MACHINE_CONFIG_START( vroulet, vroulet_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, 4000000)   //???
	MCFG_CPU_PROGRAM_MAP(vroulet_map)
	MCFG_CPU_IO_MAP(vroulet_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", vroulet_state,  irq0_line_hold)

	MCFG_NVRAM_ADD_1FILL("nvram")

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN2"))

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(vroulet_state, ppi8255_a_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(vroulet_state, ppi8255_b_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(vroulet_state, ppi8255_c_w))

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(vroulet_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", vroulet)
	MCFG_PALETTE_ADD("palette", 128*4)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 2000000)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSWA"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSWB"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

MACHINE_CONFIG_END

/* ROMs */

ROM_START( vroulet )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "roul1.bin", 0x0000, 0x2000, CRC(0cff99e5) SHA1(0aa6680c4b8d780d71b3e6c6fe511f86f40abc4c) )
	ROM_LOAD( "roul2.bin", 0x2000, 0x2000, CRC(61924d9f) SHA1(8334d6825ed40e8347909817b8b73be97d23faf8) )
	ROM_LOAD( "roul3.bin", 0x4000, 0x2000, CRC(73dedff6) SHA1(d01c4fc99ac8dc03bd6e0cf779c221d403b2b648) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "roul.gfx", 0x0000, 0x8000, CRC(4e4f46d2) SHA1(efd00e2b564ff4a9013c67ffaaf91124089b310b) )
ROM_END

/* Game Driver */

GAME( 1989, vroulet, 0, vroulet, vroulet, driver_device, 0, ROT90, "World Game", "Vegas Roulette", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
