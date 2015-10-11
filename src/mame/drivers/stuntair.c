// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

 Stunt Air by Nuova Videotron 1983

  driver todo: (SOME OF THIS WILL NEED PCB REFERENCES / MEASUREMENTS)
  - correct colour PROM decoding (resistor values?)
  - correct FG colour handling (currently use a hardcoded white)
  - correct sound (discrete stuff is for filtering? or drums?)
  - correct remaining GFX / sprite issues (flicker sometimes, might need better vblank timing?)


Hardware info (complete):
Main cpu Z80A
Sound cpu Z80A
Sound ic AY-3-8910 x2 @ 1.536MHz
Note: stereo sound output.Op amps LM3900 x3, audio amps TDA2002 x2, many discrete components

Osc: 18.432 Mhz

Ram:
Work 2kb (6116)
Sound 1kb (2114 x2)
Chars 1kb (2114 x2)
Bg 1,5kb (2114 x3)
Sprites 1kb (2148 x2)
color 320byte (27ls00 x10)

Rom definiton:
-top pcb-
stuntair.a0,a1,a3,a4,a6 main program
stuntair.e14 sound program
stuntair.a9 character gfx
stuntair.a11,a12 background gfx
stuntair.a13,a15 obj/sprites gfx
82s123.a7 (removing it results in garbage boot screen with high score table music)

-bottom pcb-
82s129.l11 green,blue colors
82s129.m11 red color

Eproms are 2764
Bproms are 82s123,82s129

Dip switches (by direct test,no manuals present):
-DIP A-
SW 1   2   3   5   6   7   8
   unknown
SW 4
   OFF infinite lives (test)
   ON  normal game
-DIP B-
SW 1   2  (coin 1)
   OFF OFF 1 coin 1 play,1 coin 2 play (alternate)
   ON  OFF 2 coin 1 play
   OFF ON  1 coin 2 play
   ON  ON  1 coin 1 play
SW 3   4  (coin 2)
   OFF OFF 1 coin 1 play,1 coin 2 play (alternate)
   ON  OFF 2 coin 1 play
   OFF ON  1 coin 2 play
   ON  ON  1 coin 1 play
SW 5   6
   OFF OFF bonus 50000 pts,100000 every
   ON  OFF bonus 30000 pts,50000 every
   OFF ON  bonus 20000 pts,30000 every
   ON  ON  bonus 10000 pts,20000 every
SW 7   8
   OFF OFF 5 lives
   ON  OFF 4 lives
   OFF ON  3 lives
   ON  ON  2 lives
Note: no table-upright mode sw,upright fixed.No picture flip sw too


Eprom dump,hw and dip info by tirino73
Bprom dump by f205v

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

class stuntair_state : public driver_device
{
public:
	stuntair_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_fgram(*this, "fgram"),
		m_bgram(*this, "bgram"),
		m_bgattrram(*this, "bgattrram"),
		m_sprram(*this, "sprram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_shared_ptr<UINT8> m_fgram;
	required_shared_ptr<UINT8> m_bgram;
	required_shared_ptr<UINT8> m_bgattrram;
	required_shared_ptr<UINT8> m_sprram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;

	UINT8 m_bg_xscroll;
	UINT8 m_nmi_enable;
	UINT8 m_spritebank0;
	UINT8 m_spritebank1;

	TILE_GET_INFO_MEMBER(get_stuntair_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_stuntair_bg_tile_info);
	DECLARE_WRITE8_MEMBER(stuntair_fgram_w);
	DECLARE_WRITE8_MEMBER(stuntair_bgram_w);
	DECLARE_WRITE8_MEMBER(stuntair_bgattrram_w);
	DECLARE_WRITE8_MEMBER(stuntair_bgxscroll_w);
	DECLARE_WRITE8_MEMBER(stuntair_nmienable_w);
	DECLARE_WRITE8_MEMBER(stuntair_spritebank0_w);
	DECLARE_WRITE8_MEMBER(stuntair_spritebank1_w);
	DECLARE_WRITE8_MEMBER(stuntair_coin_w);
	DECLARE_WRITE8_MEMBER(stuntair_sound_w);
	DECLARE_WRITE8_MEMBER(ay8910_portb_w);
	INTERRUPT_GEN_MEMBER(stuntair_irq);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_stuntair(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_PALETTE_INIT(stuntair);
};



/***************************************************************************

  Video

***************************************************************************/

PALETTE_INIT_MEMBER(stuntair_state, stuntair)
{
	/* need resistor weights etc. */
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();

	for (int i = 0; i < 0x100; i++)
	{
		UINT8 data = color_prom[i];

		int b = (data&0xc0)>>6;
		int g = (data&0x38)>>3;
		int r = (data&0x07)>>0;

		palette.set_pen_color(i,rgb_t(r<<5,g<<5,b<<6));
	}

	// just set the FG layer to black and white
	palette.set_pen_color(0x100,rgb_t(0x00,0x00,0x00));
	palette.set_pen_color(0x101,rgb_t(0xff,0xff,0xff));
}


TILE_GET_INFO_MEMBER(stuntair_state::get_stuntair_fg_tile_info)
{
	int tileno = m_fgram[tile_index];
	int opaque = tileno & 0x80;

	// where does the FG palette come from? it's a 1bpp layer..

	SET_TILE_INFO_MEMBER(0, tileno&0x7f, 0, opaque?TILE_FORCE_LAYER0 : TILE_FORCE_LAYER1);
}

TILE_GET_INFO_MEMBER(stuntair_state::get_stuntair_bg_tile_info)
{
	int tileno = m_bgram[tile_index];
	tileno |= (m_bgattrram[tile_index] & 0x08)<<5;
	int colour = (m_bgattrram[tile_index] & 0x07);

	SET_TILE_INFO_MEMBER(1, tileno, colour, 0);
}


void stuntair_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(stuntair_state::get_stuntair_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap->set_transparent_pen(0);

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(stuntair_state::get_stuntair_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


void stuntair_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(2);

	/* there seem to be 2 spritelists with something else (fixed values) between them.. is that significant? */
	for (int i=0;i<0x400;i+=16)
	{
		// +2, +3, +4(high bits): always 00
		// +6 to +15: unused
		int x      = m_sprram[i+5];
		int y      = m_sprram[i+0];
		int colour = m_sprram[i+4] & 0x7;
		int tile   = m_sprram[i+1] & 0x3f;
		int flipy = (m_sprram[i+1] & 0x80)>>7; // used
//      int flipx = (m_sprram[i+1] & 0x40)>>6; // guessed , wrong
		int flipx = 0;

		if (m_spritebank1) tile |= 0x40;
		if (m_spritebank0) tile |= 0x80;

		y = 240 - y;

		gfx->transpen(bitmap,cliprect,tile,colour,flipx,flipy,x,y,0);
	}
}

UINT32 stuntair_state::screen_update_stuntair(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_bg_xscroll);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, TILEMAP_PIXEL_LAYER0);

	draw_sprites(bitmap, cliprect);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, TILEMAP_PIXEL_LAYER1|TILEMAP_DRAW_OPAQUE);

	return 0;
}



/***************************************************************************

  Memory Maps, I/O

***************************************************************************/

WRITE8_MEMBER(stuntair_state::stuntair_bgattrram_w)
{
	m_bgattrram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(stuntair_state::stuntair_bgram_w)
{
	m_bgram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(stuntair_state::stuntair_fgram_w)
{
	m_fgram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER( stuntair_state::stuntair_bgxscroll_w )
{
	m_bg_xscroll = data;
}


WRITE8_MEMBER(stuntair_state::stuntair_spritebank0_w)
{
	m_spritebank0 = data&0x01;
	// other bits are unused
}

WRITE8_MEMBER(stuntair_state::stuntair_spritebank1_w)
{
	m_spritebank1 = data&0x01;
	// other bits are unused
}


WRITE8_MEMBER(stuntair_state::stuntair_nmienable_w)
{
	m_nmi_enable = data&0x01;
	if (!m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	// other bits are unused
}

WRITE8_MEMBER(stuntair_state::stuntair_coin_w)
{
	// lower 2 bits are coin counters, excluding 1st coin(?)
	coin_counter_w(machine(), 0, data >> 0 & 1);
	coin_counter_w(machine(), 1, data >> 1 & 1);

	// other bits: unknown
	if (data & 0xfc)
		logerror("stuntair_coin_w %02x\n", data);
}


WRITE8_MEMBER(stuntair_state::stuntair_sound_w)
{
	soundlatch_byte_w(space, 0, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

// main Z80
static ADDRESS_MAP_START( stuntair_map, AS_PROGRAM, 8, stuntair_state )
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xcbff) AM_RAM_WRITE(stuntair_bgattrram_w) AM_SHARE("bgattrram")
	AM_RANGE(0xd000, 0xd3ff) AM_RAM_WRITE(stuntair_bgram_w) AM_SHARE("bgram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_SHARE("sprram")
	AM_RANGE(0xe000, 0xe000) AM_READ_PORT("DSWB") AM_WRITE(stuntair_coin_w)
	AM_RANGE(0xe800, 0xe800) AM_READ_PORT("DSWA") AM_WRITE(stuntair_bgxscroll_w)
	AM_RANGE(0xf000, 0xf000) AM_READ_PORT("IN2")
	AM_RANGE(0xf001, 0xf001) AM_WRITE(stuntair_nmienable_w)
	AM_RANGE(0xf002, 0xf002) AM_READ_PORT("IN3")
	AM_RANGE(0xf003, 0xf003) AM_READNOP AM_WRITE(stuntair_spritebank1_w)
//  AM_RANGE(0xf004, 0xf004) AM_WRITENOP
	AM_RANGE(0xf005, 0xf005) AM_WRITE(stuntair_spritebank0_w)
//  AM_RANGE(0xf006, 0xf006) AM_WRITENOP
//  AM_RANGE(0xf007, 0xf007) AM_WRITENOP
	AM_RANGE(0xf800, 0xfbff) AM_RAM_WRITE(stuntair_fgram_w) AM_SHARE("fgram")
	AM_RANGE(0xfc03, 0xfc03) AM_WRITE(stuntair_sound_w)
ADDRESS_MAP_END

// sound Z80
static ADDRESS_MAP_START( stuntair_sound_map, AS_PROGRAM, 8, stuntair_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( stuntair_sound_portmap, AS_IO, 8, stuntair_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x03, 0x03) AM_DEVWRITE("ay2", ay8910_device, address_w)
	AM_RANGE(0x07, 0x07) AM_DEVWRITE("ay2", ay8910_device, data_w)
	AM_RANGE(0x0c, 0x0d) AM_DEVREADWRITE("ay1", ay8910_device, data_r, address_data_w)
ADDRESS_MAP_END



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( stuntair )
	PORT_START("DSWB") // the bit order is scrambled, but this matches the text above
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coin_A ) )        PORT_DIPLOCATION("SWB:2,1")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, "1 Coin/1 Credit - 2 Coins/3 Credits" )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x24, 0x00, DEF_STR( Coin_B ) )        PORT_DIPLOCATION("SWB:4,3")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x24, "1 Coin/1 Credit - 2 Coins/3 Credits" )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x42, 0x40, DEF_STR( Bonus_Life ) )    PORT_DIPLOCATION("SWB:6,5")
	PORT_DIPSETTING(    0x00, "10000 20000" )
	PORT_DIPSETTING(    0x40, "20000 30000" )
	PORT_DIPSETTING(    0x02, "30000 50000" )
	PORT_DIPSETTING(    0x42, "50000 100000" )
	PORT_DIPNAME( 0x81, 0x80, "Lives" )                  PORT_DIPLOCATION("SWB:8,7")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x81, "5" )

	PORT_START("DSWA") // the bit order is scrambled, not sure if the dip locations are correct
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SWA:1" ) // test related? $05c7
	PORT_DIPNAME( 0x28, 0x08, DEF_STR( Difficulty ) )    PORT_DIPLOCATION("SWA:2,3") // $298f
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x28, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x00, "Infinite Lives (Cheat)" ) PORT_DIPLOCATION("SWA:4") // $3f49
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SWA:5" ) // not accessed in game code
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SWA:6" ) // "
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWA:7" ) // "
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SWA:8" ) // "

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x04, 0x04, "Clear Credits on Reset" ) // I doubt this is a real switch
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/***************************************************************************

  GFX Layouts

***************************************************************************/

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static const gfx_layout tiles8x8x2_layout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static const gfx_layout tiles16x8x2_layout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,64,65,66,67,68,69,70,71 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8, 128+0*8, 128+1*8, 128+2*8, 128+3*8, 128+4*8, 128+5*8, 128+6*8, 128+7*8 },
	16*16
};


static GFXDECODE_START( stuntair )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0x100, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8x2_layout, 0xe0, 8 )
	GFXDECODE_ENTRY( "gfx3", 0, tiles16x8x2_layout, 0xe0, 8 )
GFXDECODE_END



/***************************************************************************

  AY8910 Config

***************************************************************************/

WRITE8_MEMBER(stuntair_state::ay8910_portb_w)
{
	// it writes $e8 and $f0 for music drums?
	// possibly to discrete sound circuitry?
	logerror("ay8910_portb_w: %02x\n", data);
}

/***************************************************************************

  Machine Config

***************************************************************************/

INTERRUPT_GEN_MEMBER(stuntair_state::stuntair_irq)
{
	if(m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void stuntair_state::machine_start()
{
	m_bg_xscroll = 0;
	m_nmi_enable = 0;
	m_spritebank0 = 0;
	m_spritebank1 = 0;

	save_item(NAME(m_bg_xscroll));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_spritebank0));
	save_item(NAME(m_spritebank1));
}

void stuntair_state::machine_reset()
{
}

static MACHINE_CONFIG_START( stuntair, stuntair_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,  XTAL_18_432MHz/6)         /* 3 MHz? */
	MCFG_CPU_PROGRAM_MAP(stuntair_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", stuntair_state, stuntair_irq)

	MCFG_CPU_ADD("audiocpu", Z80,  XTAL_18_432MHz/6)         /* 3 MHz? */
	MCFG_CPU_PROGRAM_MAP(stuntair_sound_map)
	MCFG_CPU_IO_MAP(stuntair_sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(stuntair_state, irq0_line_hold, 420) // drives music tempo, timing is approximate based on PCB audio recording.. and where is irq ack?

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60) // ?
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(stuntair_state, screen_update_stuntair)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT90)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", stuntair)
	MCFG_PALETTE_ADD("palette", 0x100+2)

	MCFG_PALETTE_INIT_OWNER(stuntair_state, stuntair)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono") // stereo?

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_18_432MHz/12)
	MCFG_AY8910_PORT_A_READ_CB(READ8(driver_device, soundlatch_byte_r))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(stuntair_state, ay8910_portb_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_18_432MHz/12)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( stuntair )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "stuntair.a0", 0x0000, 0x2000, CRC(f61c4a1d) SHA1(29a227b447866e27e7619a0a676c9ad66364c323) )
	ROM_LOAD( "stuntair.a1", 0x2000, 0x2000, CRC(1546f041) SHA1(6a2346edf39700f7b1e609f19e0f2e46b3a78a2a) )
	ROM_LOAD( "stuntair.a3", 0x4000, 0x2000, CRC(63d00b97) SHA1(63efa151147a3c0ac33e226d38aecfd06b36ad38) )
	ROM_LOAD( "stuntair.a4", 0x6000, 0x2000, CRC(01fe2697) SHA1(f7efc6af8047245ad92dff0e62f61abc71a2e9d1) )
	ROM_LOAD( "stuntair.a6", 0x8000, 0x2000, CRC(6704d05c) SHA1(5b1af8be86ffc44ae0207397b33769556ab456df) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "stuntair.e14", 0x0000, 0x2000, CRC(641fc9db) SHA1(959a4b6617f840f52d1856e12a9fad8e12293387) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "stuntair.a9", 0x0000, 0x2000, CRC(bfd861f5) SHA1(ff089ec2e98b21202aeefc31158961e3b1d1ccca) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "stuntair.a11", 0x0000, 0x2000, CRC(421fef4c) SHA1(e46abead8cd44253cf6da74326f6f3bdd3aa26e5) )
	ROM_LOAD( "stuntair.a12", 0x2000, 0x2000, CRC(e6ee7489) SHA1(6eeea137fc8968e84d2aadbbac982fd9cd161b16) )

	ROM_REGION( 0x4000, "gfx3", 0 )
	ROM_LOAD( "stuntair.a13", 0x0000, 0x2000, CRC(bfdc0d38) SHA1(ea0a22971e9cf1b1682c35facc9c4e30607faed7) )
	ROM_LOAD( "stuntair.a15", 0x2000, 0x2000, CRC(4531cab5) SHA1(35271555377ec3454a5d74bf8c21d7e8acc05782) )

	ROM_REGION( 0x120, "proms", 0 )
	ROM_LOAD_NIB_LOW ( "dm74s287n.11m", 0x000, 0x100, CRC(d330ff90) SHA1(e223935464109a3c4c7b29641b3736484c22c47a) ) // only the last few entries are used?
	ROM_LOAD_NIB_HIGH( "dm74s287n.11l", 0x000, 0x100, CRC(6c98f964) SHA1(abf7bdeccd33e62fa106d2056d1949cf278483a7) ) // "
	ROM_LOAD_NIB_LOW ( "dm74s288n.7a",  0x100, 0x020, CRC(5779e751) SHA1(89c955ef8635ad3e9d699f33ec0e4d6c9205d01c) ) // ?
ROM_END


GAME( 1983, stuntair,  0,    stuntair, stuntair, driver_device,  0, ROT90, "Nuova Videotron", "Stunt Air",  MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
