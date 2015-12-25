// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Tomasz Slanina, Olivier Galibert
/*
****************************************************
Mirax (C)1985 Current Technologies

driver by
Tomasz Slanina analog[AT]op[DOT]pl
Angelo Salese
Olivier Galibert

TODO:
- sound ports are a mystery (PC=0x02e0)
- video offsets?
- score / credits display should stay above the sprites?

====================================================

CPU: Ceramic potted module, Z80C
Sound: AY-3-8912 (x2)
RAM: 74S201, 74S201 (x6), 2148 (x6), 2114 (x2), 58725 (x2), 6116
PROMS: 82S123 (x2)
XTAL: 12 MHz

Here comes a high energy space game of its own kind.

Mirax(TM)
* First person perspective
* Fully integrated game play.
* Continually changing 3-D graphics plus powerful sound effects.

HOW TO PLAY
1. Your goal is to terminate Mirax City - a giant enemy central station shown at stage 10.
2. Destroy all enemy objects (air or ground) as many as you can. Avoid indestructible building blocks.
3. Shooting a group of ground targets causes satellites to rise.
4. Enemy flagship appears after a wave of far ground vessels completely destroyed.
5. Hitting flagship puts you into power shooting mode - no fire button required.
6. Bonus flag is given for each flagship hit. 5 or 8 flags award you an extra fighter.
7. Use joystick to take sight before firing.


Pinouts

Parts          Solder
1 Gnd          Gnd
2 Gnd          Gnd
3 Gnd          Video gnd
4 Gnd          Gnd
5
6
7
8
9 Coin 1       Video sync
10 Up
11 Fire
12 Down
13 start1      start2
14 left
15 right
16 Video green
17 Video blue
18 Video red
19 Speaker-    Speaker+
20 +5          +5
21 +5          +5
22 +12         +12


Stephh's notes (based on the games Z80 code and some tests) :

1) 'mirax'

  - Screen flipping settings (DSW1 bit 3) are only tested AFTER the ROM/RAM checks;
    so the first texts may be upside down related to screen orientation (ingame bug).
  - Coin A and Coin B use the same coinage based on DSW1 bits 0 and 1.
    Furthermore, they share the same coin counter at 0xf500.
  - When reaching 10th "boss" (level 100), the game will consider that you're on level 1 :
    it will display "STAGE 1 - MIRAX CITY: 90000 MILES" on "presentation" screen and
    "STAGE 1" at the bottom right when fighting the "boss".
    Once you have defeated it, you'll go back to normal stage 2 : it will display
    "STAGE 2 - MIRAX CITY: 80000 MILES" on "presentation" screen and "STAGE 2"
    at the bottom right (ingame bug).

2) 'miraxa'

  - Screen flipping settings (DSW1 bit 3) are only BEFORE the ROM/RAM checks
    due to additional code at 0x0200, so the first texts always fit screen orientation.
  - Coin A and Coin B use different coinages even if both are based on DSW1 bits 0 and 1.
    There are 2 coin counters : Coin A at 0xf500 and Coin B at 0xf502.
  - Other noticeable differences with 'mirax' :
      * different lives settings (DSW1 bits 4 and 5)
      * different bonus lives settings (DSW2 bits 0 and 1)
      * different stages names :
          . stages  1 to 10 : "LUXORI" instead of "MIRAX"
          . stages 71 to 80 : "DESCOM" instead of "DESBOM"
        furthermore, for all stages, it's written "UNIT" instead of "CITY"
  - Same ingame bug as in 'mirax' when you reach level 100 (of course, it will display
    "LUXORI UNIT" instead of "MIRAX CITY" on "presentation" screen).

************************************************
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"


class mirax_state : public driver_device
{
public:
	mirax_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram")  { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_colorram;

	UINT8 m_nAyCtrl;
	UINT8 m_nmi_mask;
	UINT8 m_flipscreen_x;
	UINT8 m_flipscreen_y;

	DECLARE_WRITE8_MEMBER(audio_w);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(sound_cmd_w);
	DECLARE_WRITE8_MEMBER(coin_counter0_w);
	DECLARE_WRITE8_MEMBER(coin_counter1_w);
	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_WRITE8_MEMBER(ay1_sel);
	DECLARE_WRITE8_MEMBER(ay2_sel);

	DECLARE_DRIVER_INIT(mirax);
	DECLARE_PALETTE_INIT(mirax);
	virtual void machine_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_tilemap(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 draw_flag);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(vblank_irq);
};


PALETTE_INIT_MEMBER(mirax_state, mirax)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0;i < palette.entries();i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = 0x4f * bit0 + 0xa8 * bit1;

		palette.set_pen_color(i,rgb_t(r,g,b));
	}
}


void mirax_state::draw_tilemap(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 draw_flag)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int y,x;
	int res_x,res_y,wrapy;

	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			int tile = m_videoram[32*y+x];
			int color = (m_colorram[x*2]<<8) | (m_colorram[(x*2)+1]);
			int x_scroll = (color & 0xff00)>>8;
			tile |= ((color & 0xe0)<<3);

			res_x = (m_flipscreen_x) ? 248-x*8 : x*8;
			res_y = (m_flipscreen_y) ? 248-y*8+x_scroll : y*8-x_scroll;
			wrapy = (m_flipscreen_y) ? -256 : 256;

			if((x <= 1 || x >= 30) ^ draw_flag)
			{
				gfx->opaque(bitmap,cliprect,tile,color & 7,(m_flipscreen_x),(m_flipscreen_y),res_x,res_y);
				/* wrap-around */
				gfx->opaque(bitmap,cliprect,tile,color & 7,(m_flipscreen_x),(m_flipscreen_y),res_x,res_y+wrapy);
			}
		}
	}
}

void mirax_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for(int count=0;count<0x200;count+=4)
	{
		int spr_offs,x,y,color,fx,fy;

		if(m_spriteram[count] == 0x00 || m_spriteram[count+3] == 0x00)
			continue;

		spr_offs = (m_spriteram[count+1] & 0x3f);
		color = m_spriteram[count+2] & 0x7;
		fx = (m_flipscreen_x) ^ ((m_spriteram[count+1] & 0x40) >> 6); //<- guess
		fy = (m_flipscreen_y) ^ ((m_spriteram[count+1] & 0x80) >> 7);

		spr_offs += (m_spriteram[count+2] & 0xe0)<<1;
		spr_offs += (m_spriteram[count+2] & 0x10)<<5;

		y = (m_flipscreen_y) ? m_spriteram[count] : 0x100 - m_spriteram[count] - 16;
		x = (m_flipscreen_x) ? 240 - m_spriteram[count+3] : m_spriteram[count+3];

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,spr_offs,color,fx,fy,x,y,0);
	}
}

UINT32 mirax_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_tilemap(bitmap,cliprect,1);
	draw_sprites(bitmap,cliprect);
	draw_tilemap(bitmap,cliprect,0);
	return 0;
}


void mirax_state::machine_start()
{
	m_nAyCtrl = 0x00;

	save_item(NAME(m_nAyCtrl));
	save_item(NAME(m_nmi_mask));
	save_item(NAME(m_flipscreen_x));
	save_item(NAME(m_flipscreen_y));
}

WRITE8_MEMBER(mirax_state::audio_w)
{
	m_nAyCtrl=offset;
}

WRITE8_MEMBER(mirax_state::ay1_sel)
{
	ay8910_device *ay8910 = machine().device<ay8910_device>("ay1");
	ay8910->address_w(space,0,m_nAyCtrl);
	ay8910->data_w(space,0,data);
}

WRITE8_MEMBER(mirax_state::ay2_sel)
{
	ay8910_device *ay8910 = machine().device<ay8910_device>("ay2");
	ay8910->address_w(space,0,m_nAyCtrl);
	ay8910->data_w(space,0,data);
}

WRITE8_MEMBER(mirax_state::nmi_mask_w)
{
	m_nmi_mask = data & 1;
	if(data & 0xfe)
		printf("Warning: %02x written at $f501\n",data);
}

WRITE8_MEMBER(mirax_state::sound_cmd_w)
{
	soundlatch_byte_w(space, 0, data & 0xff);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


WRITE8_MEMBER(mirax_state::coin_counter0_w)
{
	coin_counter_w(machine(), 0, data & 1);
}

WRITE8_MEMBER(mirax_state::coin_counter1_w)
{
	coin_counter_w(machine(), 1, data & 1);
}

/* One address flips X, the other flips Y, but I can't tell which is which - Since the value is the same for the 2 addresses, it doesn't really matter */
WRITE8_MEMBER(mirax_state::flip_screen_w)
{
	if (offset == 0)
		m_flipscreen_x = data & 0x01;

	if (offset == 1)
		m_flipscreen_y = data & 0x01;
}

static ADDRESS_MAP_START( mirax_main_map, AS_PROGRAM, 8, mirax_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc800, 0xd7ff) AM_RAM
	AM_RANGE(0xe000, 0xe3ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xe800, 0xe9ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xea00, 0xea3f) AM_RAM AM_SHARE("colorram") //per-column color + bank bits for the videoram
	AM_RANGE(0xf000, 0xf000) AM_READ_PORT("P1")
	AM_RANGE(0xf100, 0xf100) AM_READ_PORT("P2")
	AM_RANGE(0xf200, 0xf200) AM_READ_PORT("DSW1")
	AM_RANGE(0xf300, 0xf300) AM_READNOP //watchdog? value is always read then discarded
	AM_RANGE(0xf400, 0xf400) AM_READ_PORT("DSW2")
	AM_RANGE(0xf500, 0xf500) AM_WRITE(coin_counter0_w)
	AM_RANGE(0xf501, 0xf501) AM_WRITE(nmi_mask_w)
	AM_RANGE(0xf502, 0xf502) AM_WRITE(coin_counter1_w) // only used in 'miraxa' - see notes
	AM_RANGE(0xf506, 0xf507) AM_WRITE(flip_screen_w)
	AM_RANGE(0xf800, 0xf800) AM_WRITE(sound_cmd_w)
//  AM_RANGE(0xf900, 0xf900) //sound cmd mirror? ack?
ADDRESS_MAP_END

static ADDRESS_MAP_START( mirax_sound_map, AS_PROGRAM, 8, mirax_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_byte_r)

	AM_RANGE(0xe000, 0xe000) AM_WRITENOP
	AM_RANGE(0xe001, 0xe001) AM_WRITENOP
	AM_RANGE(0xe003, 0xe003) AM_WRITE(ay1_sel) //1st ay ?

	AM_RANGE(0xe400, 0xe400) AM_WRITENOP
	AM_RANGE(0xe401, 0xe401) AM_WRITENOP
	AM_RANGE(0xe403, 0xe403) AM_WRITE(ay2_sel) //2nd ay ?

	AM_RANGE(0xf900, 0xf9ff) AM_WRITE(audio_w)
ADDRESS_MAP_END


/* verified from Z80 code */
static INPUT_PORTS_START( mirax )
	/* up/down directions are trusted according of the continue screen */
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_HIGH )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_HIGH )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )       /* table at 0x11b5 (2 * 3 * 2 bytes) */
	PORT_DIPSETTING(    0x00, "30k 80k 150k" )
	PORT_DIPSETTING(    0x01, "900k 950k 990k" )
	PORT_DIPNAME( 0x02, 0x00, "Flags for Extra Life" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x02, "8" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	/* this dip makes the game to behave like attract mode, even if you insert a coin */
	PORT_DIPNAME( 0x10, 0x00, "Auto-Play Mode (Debug)" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_HIGH )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_HIGH )
INPUT_PORTS_END

/* verified from Z80 code */
static INPUT_PORTS_START( miraxa )
	PORT_INCLUDE( mirax )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, "A: 2C/1C - B: 12C/1C" )
	PORT_DIPSETTING(    0x00, "A: 1C/1C - B: 6C/1C" )
	PORT_DIPSETTING(    0x01, "A: 1C/2C - B: 3C/1C" )
	PORT_DIPSETTING(    0x02, "A: 1C/3C - B: 2C/1C" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "6" )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )       /* table at 0x1276 (2 * 3 * 2 bytes) */
	PORT_DIPSETTING(    0x00, "30k 80k 150k" )
	PORT_DIPSETTING(    0x01, "50k 100k 900k" )
INPUT_PORTS_END


static const gfx_layout layout16 =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3),RGN_FRAC(1,3),RGN_FRAC(0,3)},
	{ 0, 1, 2, 3, 4, 5, 6, 7 ,
		0+8*8,1+8*8,2+8*8,3+8*8,4+8*8,5+8*8,6+8*8,7+8*8},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	0*8+8*8*2, 1*8+8*8*2, 2*8+8*8*2, 3*8+8*8*2, 4*8+8*8*2, 5*8+8*8*2, 6*8+8*8*2, 7*8+8*8*2},
	16*16
};

static const gfx_layout layout8 =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3),RGN_FRAC(1,3),RGN_FRAC(0,3)},
	{ 0, 1, 2, 3, 4, 5, 6, 7},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8
};

static GFXDECODE_START( mirax )
	GFXDECODE_ENTRY( "gfx1", 0, layout8,     0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0, layout16,    0, 8 )
GFXDECODE_END


INTERRUPT_GEN_MEMBER(mirax_state::vblank_irq)
{
	if(m_nmi_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( mirax, mirax_state )
	MCFG_CPU_ADD("maincpu", Z80, 12000000/4) // ceramic potted module, encrypted z80
	MCFG_CPU_PROGRAM_MAP(mirax_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", mirax_state, vblank_irq)

	MCFG_CPU_ADD("audiocpu", Z80, 12000000/4)
	MCFG_CPU_PROGRAM_MAP(mirax_sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(mirax_state, irq0_line_hold,  4*60)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(mirax_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x40)
	MCFG_PALETTE_INIT_OWNER(mirax_state, mirax)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mirax)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay1", AY8910, 12000000/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_SOUND_ADD("ay2", AY8910, 12000000/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END


ROM_START( mirax )
	ROM_REGION( 0xc000, "maincpu", ROMREGION_ERASE00 ) // put decrypted code there

	ROM_REGION( 0xc000, "data_code", 0 ) // encrypted code for the main cpu
	ROM_LOAD( "mxp5-42.rom",   0x0000, 0x4000, CRC(716410a0) SHA1(55171376e1e164b1d5e728789da6e04a3a33c172) )
	ROM_LOAD( "mxr5-4v.rom",   0x4000, 0x4000, CRC(c9484fc3) SHA1(101c5e4b9d49d2424ad80970eb3bdb87949a9966) )
	ROM_LOAD( "mxs5-4v.rom",   0x8000, 0x4000, CRC(e0085f91) SHA1(cf143b94048e1ebb5c899b94b500e193dfd42e18) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mxr2-4v.rom",   0x0000, 0x2000, CRC(cd2d52dc) SHA1(0d4181dc68beac338f47a2065c7b755008877896) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "mxe3-4v.rom",   0x0000, 0x4000, CRC(0cede01f) SHA1(c723dd8ee9dc06c94a7fe5d5b5bccc42e2181af1) )
	ROM_LOAD( "mxh3-4v.rom",   0x4000, 0x4000, CRC(58221502) SHA1(daf5c508939b44616ca76308fc33f94d364ed587) )
	ROM_LOAD( "mxk3-4v.rom",   0x8000, 0x4000, CRC(6dbc2961) SHA1(5880c28f1ef704fee2d625a42682c7d65613acc8) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "mxe2-4v.rom",   0x04000, 0x4000, CRC(2cf5d8b7) SHA1(f66bce4d413a48f6ae07974870dc0f31eefa68e9) )
	ROM_LOAD( "mxf2-4v.rom",   0x0c000, 0x4000, CRC(1f42c7fa) SHA1(33e56c6ddf7676a12f57de87ec740c6b6eb1cc8c) )
	ROM_LOAD( "mxh2-4v.rom",   0x14000, 0x4000, CRC(cbaff4c6) SHA1(2dc4a1f51b28e98be0cfb5ab7576047c748b6728) )
	ROM_LOAD( "mxf3-4v.rom",   0x00000, 0x4000, CRC(14b1ca85) SHA1(775a4c81a81b78490d45095af31e24c16886f0a2) )
	ROM_LOAD( "mxi3-4v.rom",   0x08000, 0x4000, CRC(20fb2099) SHA1(da6bbd5d2218ba49b8ef98e7affdcab912f84ade) )
	ROM_LOAD( "mxl3-4v.rom",   0x10000, 0x4000, CRC(918487aa) SHA1(47ba6914722a253f65c733b5edff4d15e73ea6c2) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "mra3.prm",   0x0000, 0x0020, CRC(ae7e1a63) SHA1(f5596db77c1e352ef7845465db3e54e19cd5df9e) )
	ROM_LOAD( "mrb3.prm",   0x0020, 0x0020, CRC(e3f3d0f5) SHA1(182b06c9db5bec1e3030f705247763bd2380ba83) )
	ROM_LOAD( "mirax.prm",  0x0040, 0x0020, NO_DUMP ) // data ? encrypted roms for cpu1 ?
ROM_END

ROM_START( miraxa )
	ROM_REGION( 0xc000, "maincpu", ROMREGION_ERASE00 ) // put decrypted code there

	ROM_REGION( 0xc000, "data_code", 0 ) // encrypted code for the main cpu
	ROM_LOAD( "mx_p5_43v.p5",   0x0000, 0x4000, CRC(87664903) SHA1(ccc11ecf0658e7af0db3229f60a16b1a44bd12bc) )
	ROM_LOAD( "mx_r5_43v.r5",   0x4000, 0x4000, CRC(1ba4cd8e) SHA1(8fd22d3a4bca7c989382aaf7b08ac381a3566493) )
	ROM_LOAD( "mx_s5_43v.s5",   0x8000, 0x4000, CRC(c58cc151) SHA1(0846e22f4da6d85c6dc29ff1472bc84b419b2289) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mxr2-4v.rom",   0x0000, 0x2000, CRC(cd2d52dc) SHA1(0d4181dc68beac338f47a2065c7b755008877896) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "mxe3-4v.rom",   0x0000, 0x4000, CRC(0cede01f) SHA1(c723dd8ee9dc06c94a7fe5d5b5bccc42e2181af1) )
	ROM_LOAD( "mxh3-4v.rom",   0x4000, 0x4000, CRC(58221502) SHA1(daf5c508939b44616ca76308fc33f94d364ed587) )
	ROM_LOAD( "mxk3-4v.rom",   0x8000, 0x4000, CRC(6dbc2961) SHA1(5880c28f1ef704fee2d625a42682c7d65613acc8) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "mxe2-4v.rom",   0x04000, 0x4000, CRC(2cf5d8b7) SHA1(f66bce4d413a48f6ae07974870dc0f31eefa68e9) )
	ROM_LOAD( "mxf2-4v.rom",   0x0c000, 0x4000, CRC(1f42c7fa) SHA1(33e56c6ddf7676a12f57de87ec740c6b6eb1cc8c) )
	ROM_LOAD( "mxh2-4v.rom",   0x14000, 0x4000, CRC(cbaff4c6) SHA1(2dc4a1f51b28e98be0cfb5ab7576047c748b6728) )
	ROM_LOAD( "mxf3-4v.rom",   0x00000, 0x4000, CRC(14b1ca85) SHA1(775a4c81a81b78490d45095af31e24c16886f0a2) )
	ROM_LOAD( "mxi3-4v.rom",   0x08000, 0x4000, CRC(20fb2099) SHA1(da6bbd5d2218ba49b8ef98e7affdcab912f84ade) )
	ROM_LOAD( "mxl3-4v.rom",   0x10000, 0x4000, CRC(918487aa) SHA1(47ba6914722a253f65c733b5edff4d15e73ea6c2) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "mra3.prm",   0x0000, 0x0020, CRC(ae7e1a63) SHA1(f5596db77c1e352ef7845465db3e54e19cd5df9e) )
	ROM_LOAD( "mrb3.prm",   0x0020, 0x0020, CRC(e3f3d0f5) SHA1(182b06c9db5bec1e3030f705247763bd2380ba83) )
ROM_END


DRIVER_INIT_MEMBER(mirax_state,mirax)
{
	UINT8 *DATA = memregion("data_code")->base();
	UINT8 *ROM = memregion("maincpu")->base();
	int i;

	for(i=0x0000;i<0x4000;i++)
		ROM[BITSWAP16(i, 15,14,13,12,11,10,9, 5,7,6,8, 4,3,2,1,0)] = (BITSWAP8(DATA[i], 1, 3, 7, 0, 5, 6, 4, 2) ^ 0xff);

	for(i=0x4000;i<0x8000;i++)
		ROM[BITSWAP16(i, 15,14,13,12,11,10,9, 5,7,6,8, 4,3,2,1,0)] = (BITSWAP8(DATA[i], 2, 1, 0, 6, 7, 5, 3, 4) ^ 0xff);

	for(i=0x8000;i<0xc000;i++)
		ROM[BITSWAP16(i, 15,14,13,12,11,10,9, 5,7,6,8, 4,3,2,1,0)] = (BITSWAP8(DATA[i], 1, 3, 7, 0, 5, 6, 4, 2) ^ 0xff);

	/* These values need to be initialised only once, not on every soft reset */
	m_flipscreen_x = 0;
	m_flipscreen_y = 0;
}

GAME( 1985, mirax,    0,        mirax,    mirax, mirax_state,    mirax,    ROT90, "Current Technologies", "Mirax (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, miraxa,   mirax,    mirax,    miraxa, mirax_state,   mirax,    ROT90, "Current Technologies", "Mirax (set 2)", MACHINE_SUPPORTS_SAVE )
