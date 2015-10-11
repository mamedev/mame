// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood
/*************************************************************************************

Submarine (c) 1985 Sigma

driver by David Haywood & Angelo Salese

TODO:
- finish dip-switches;
- a bunch of unemulated writes at 0xe*** (I believe that there are individual
  flip screen x & y)
- flip screen support;

======================================================================================

 2 PCBs

 PCB1 - Bottom? (video?) board

|------------------------------------------------------------------- --|
|           B                                                          |
|     A           C     D     E     F     -     H     -    J           |
|                                                                      |
|                                                                      |
|1                                             (rom)                   |
|                                               OBJ1             T     |
|2                           Sony                                e    --
|                            CX23001           (rom)             x    --
|3                                              OBJ2             t    --
|                                                                     --C
|4                           (prom)            (rom)                  --N
|                              E4               OBJ3                  --3
|5                           (prom)                                   --
|                              E5                                     --
|6                           Sony                                     --
|                            CX23001                                  --
|7              (prom)                                                 |
|                 C7                                                   |
|8              (prom)                                                 |
|                 C8                                                   |
|9  (prom)                                                            --
|    A9    R                                                          --
|10 (prom) e                                                          --
|    A10   s                         HM6116P-3                        --C
|11 (prom)                                                            --N
|    A11                             (rom)                            --4
|12                                  VRAM1                            --
|                                                                     --
|13                                  (rom)                            --
|                                    VRAM2                            --
|14                                                                    |
|                                    (rom)                             |
|15                                  VRAM3                             |
|                                                                      |
|                                                                      |
|----------------------------------------------------------------------|

 Text = sigma enterprises, inc.
                 JAPAN F-021BCRT
         (rotated 90 degress left)

  Res = a bunch of resistors (colour weighting?)


PCB2  (Top board, CPU board)

|----------------------------------------------------------------------|
|                                                                      |
|         A       B       C       D      E       F       G      H      |
--|                                                        16000  18.432
  |                                                        OSC1    OSC2|
  |1           - - - - - - - - - - -                                   |
--|           |                                                    T   |
--|2                 NECD780        |                              e  --
--|           |      (z80 CPU)                                     x  --
--|  3                              |                              t  --
--|            - - - - - - - - - - -                                  --C
--|  4                                                                --N
--|                                                                   --3
--|C 5          T         T        T                                  --
  |N            E(rom)    E(rom)   E(rom)                             --
  |2 6          M         M        M                                  --
--|             P         P        P                                  --
|    7          1         2        3                                   |
--|C                                                                   |
  |N 8                                                                 |
  |1                                                                   |
--|  9                                                                --
--|                                                                   --
--| 10                                                                --
--|                                                                   --C
--| 11                                                                --N
--|                                                                   --4
--| 12                             DSW1 DSW2                          --
--|                                                                   --
--| 13                                                                --
  |                                                                   --
  | 14                                                         L       |
--|                                AY8910     R(rom)           H(z80A) |
|          15                                 O                0       |
|                                  AY8910     M                0       |
|          16                                 M                8       |
|                                                              0A      |
|----------------------------------------------------------------------|

 Text = F-020   CPU1 JAPAN   sigma enterprises, inc.

*************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

#define MASTER_CLOCK            XTAL_18_432MHz

class sub_state : public driver_device
{
public:
	sub_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_attr(*this, "attr"),
		m_vid(*this, "vid"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_scrolly(*this, "scrolly") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_attr;
	required_shared_ptr<UINT8> m_vid;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_spriteram2;
	required_shared_ptr<UINT8> m_scrolly;

	UINT8 m_nmi_en;

	DECLARE_WRITE8_MEMBER(to_sound_w);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);

	virtual void machine_start();
	DECLARE_PALETTE_INIT(sub);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(sound_irq);
};

void sub_state::machine_start()
{
	save_item(NAME(m_nmi_en));
}

UINT32 sub_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	gfx_element *gfx_1 = m_gfxdecode->gfx(1);
	int y,x;
	int count = 0;

	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			UINT16 tile = m_vid[count];
			UINT8 col;
			UINT8 y_offs = m_scrolly[x];

			tile += (m_attr[count]&0xe0)<<3;
			col = (m_attr[count]&0x1f);

			gfx->opaque(bitmap,cliprect,tile,col+0x40,0,0,x*8,(y*8)-y_offs);
			gfx->opaque(bitmap,cliprect,tile,col+0x40,0,0,x*8,(y*8)-y_offs+256);

			count++;
		}
	}


	/*
	sprite bank 1
	0 xxxx xxxx X offset
	1 tttt tttt tile offset
	sprite bank 2
	0 yyyy yyyy Y offset
	1 f--- ---- flips the X offset
	1 -f-- ---- flip y, inverted
	1 --cc cccc color
	*/
	{
		UINT8 *spriteram = m_spriteram;
		UINT8 *spriteram_2 = m_spriteram2;
		UINT8 x,y,spr_offs,i,col,fx,fy;

		for(i=0;i<0x40;i+=2)
		{
			spr_offs = spriteram[i+1];
			x = spriteram[i+0];
			y = 0xe0 - spriteram_2[i+1];
			col = (spriteram_2[i+0])&0x3f;
			fx = (spriteram_2[i+0] & 0x80) ? 0 : 1;
			if(fx) { x = 0xe0 - x; }
			fy = (spriteram_2[i+0] & 0x40) ? 0 : 1;

			gfx_1->transpen(bitmap,cliprect,spr_offs,col,0,fy,x,y,0);
		}
	}

	count = 0;

	/* re-draw score display above the sprites (window effect) */
	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			UINT16 tile = m_vid[count];
			UINT8 col;
			UINT8 y_offs = m_scrolly[x];

			tile += (m_attr[count]&0xe0)<<3;
			col = (m_attr[count]&0x1f);

			if(x >= 28)
			{
				gfx->opaque(bitmap,cliprect,tile,col+0x40,0,0,x*8,(y*8)-y_offs);
				gfx->opaque(bitmap,cliprect,tile,col+0x40,0,0,x*8,(y*8)-y_offs+256);
			}

			count++;
		}
	}

	return 0;
}

static ADDRESS_MAP_START( subm_map, AS_PROGRAM, 8, sub_state )
	AM_RANGE(0x0000, 0xafff) AM_ROM
	AM_RANGE(0xb000, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xc3ff) AM_RAM AM_SHARE("attr")
	AM_RANGE(0xc400, 0xc7ff) AM_RAM AM_SHARE("vid")
	AM_RANGE(0xd000, 0xd03f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xd800, 0xd83f) AM_RAM AM_SHARE("spriteram2")
	AM_RANGE(0xd840, 0xd85f) AM_RAM AM_SHARE("scrolly")

	AM_RANGE(0xe000, 0xe000) AM_NOP
	AM_RANGE(0xe800, 0xe800) AM_NOP
	AM_RANGE(0xe801, 0xe801) AM_NOP
	AM_RANGE(0xe802, 0xe802) AM_NOP
	AM_RANGE(0xe803, 0xe803) AM_NOP
	AM_RANGE(0xe805, 0xe805) AM_NOP

	AM_RANGE(0xf000, 0xf000) AM_READ_PORT("DSW0") // DSW0?
	AM_RANGE(0xf020, 0xf020) AM_READ_PORT("DSW1") // DSW1?
	AM_RANGE(0xf040, 0xf040) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xf060, 0xf060) AM_READ_PORT("IN0")
ADDRESS_MAP_END

WRITE8_MEMBER(sub_state::to_sound_w)
{
	soundlatch_byte_w(space, 0, data & 0xff);
	m_soundcpu->set_input_line(0, HOLD_LINE);
}

WRITE8_MEMBER(sub_state::nmi_mask_w)
{
	m_nmi_en = data & 1;
}

static ADDRESS_MAP_START( subm_io, AS_IO, 8, sub_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch2_byte_r) AM_WRITE(to_sound_w) // to/from sound CPU
ADDRESS_MAP_END

static ADDRESS_MAP_START( subm_sound_map, AS_PROGRAM, 8, sub_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_WRITE(nmi_mask_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( subm_sound_io, AS_IO, 8, sub_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(soundlatch_byte_r, soundlatch2_byte_w) // to/from main CPU
	AM_RANGE(0x40, 0x41) AM_DEVREADWRITE("ay1", ay8910_device, data_r, address_data_w)
	AM_RANGE(0x80, 0x81) AM_DEVREADWRITE("ay2", ay8910_device, data_r, address_data_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( sub )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "DSWC" )
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
	PORT_DIPNAME( 0xf0, 0x10, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
// Duplicates
//  PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
//  PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
//  PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )
//  PORT_DIPSETTING(    0xd0, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )  /* separate controls for each player */
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) ) /* Controls via player 1 for both, but need to get x/y screen flip working to fully test */
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8
};

static const gfx_layout tiles16x32_layout = {
	16,32,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 64+0, 64+1, 64+2, 64+3, 64+4, 64+5, 64+6, 64+7, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 55*8, 54*8, 53*8, 52*8, 51*8, 50*8, 49*8, 48*8,
		39*8, 38*8, 37*8, 36*8, 35*8, 34*8, 33*8, 32*8,
		23*8, 22*8, 21*8, 20*8, 19*8, 18*8, 17*8, 16*8,
		7*8,  6*8,  5*8,  4*8,  3*8,  2*8,  1*8,  0*8
	},
	64*8
};

static GFXDECODE_START( sub )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 0x80 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles16x32_layout, 0, 0x80 )
GFXDECODE_END

PALETTE_INIT_MEMBER(sub_state, sub)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;
	UINT8* lookup = memregion("proms2")->base();

	for (i = 0;i < 0x100;i++)
	{
		int r,g,b;
		r = (color_prom[0x000] >> 0);
		g = (color_prom[0x100] >> 0);
		b = (color_prom[0x200] >> 0);

		//palette.set_indirect_color(i, rgb_t(r, g, b));
		palette.set_indirect_color(i, rgb_t(pal4bit(r), pal4bit(g), pal4bit(b)));

		color_prom++;
	}


	for (i = 0;i < 0x400;i++)
	{
		UINT8 ctabentry = lookup[i+0x400] | (lookup[i+0x000] << 4);
		palette.set_pen_indirect(i, ctabentry);
	}

}


INTERRUPT_GEN_MEMBER(sub_state::sound_irq)
{
	if(m_nmi_en)
		m_soundcpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( sub, sub_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,MASTER_CLOCK/6)      /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(subm_map)
	MCFG_CPU_IO_MAP(subm_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sub_state,  irq0_line_hold)

	MCFG_CPU_ADD("soundcpu", Z80,MASTER_CLOCK/6)         /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(subm_sound_map)
	MCFG_CPU_IO_MAP(subm_sound_io)
	MCFG_CPU_PERIODIC_INT_DRIVER(sub_state, sound_irq,  120) //???


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(sub_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT270)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sub)
	MCFG_PALETTE_ADD("palette", 0x400)
	MCFG_PALETTE_INDIRECT_ENTRIES(0x100)
	MCFG_PALETTE_INIT_OWNER(sub_state, sub)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, MASTER_CLOCK/6/2) /* ? Mhz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.23)

	MCFG_SOUND_ADD("ay2", AY8910, MASTER_CLOCK/6/2) /* ? Mhz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.23)
MACHINE_CONFIG_END


ROM_START( sub )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "temp 1 pos b6 27128.bin",      0x0000, 0x4000, CRC(6875b31d) SHA1(e7607e53687f1331cc97de939de144a7954ca3c3) )
	ROM_LOAD( "temp 2 pos c6 27128.bin",      0x4000, 0x4000, CRC(bc7f8f43) SHA1(088156a66acb2214c638d9d1ad18e9836b27eff0) )
	ROM_LOAD( "temp 3 pos d6 2764.bin",   0x8000, 0x2000, CRC(3546c226) SHA1(35e53c0db75c89e8e222d2139b841e77f5cc282c) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "m sound pos f14 2764.bin",     0x0000, 0x2000, CRC(61536a97) SHA1(84effc2251bf7c91e0bb670a651117503de8940d) )
	ROM_RELOAD( 0x2000, 0x2000 )

	ROM_REGION( 0xc000, "gfx1", 0)
	ROM_LOAD( "vram 1 pos f12 27128  version3.bin",   0x0000, 0x4000, CRC(8d176ba0) SHA1(b0bf4af97e991545d6b38e8159eb909376e6df35) )
	ROM_LOAD( "vram 2 pos f14 27128  version3.bin",   0x4000, 0x4000, CRC(0677cf3a) SHA1(072e9391f6a230b78124e820da0f0d27ffa45dc3) )
	ROM_LOAD( "vram 3 pos f15 27128  version3.bin",   0x8000, 0x4000, CRC(9a4cd1a0) SHA1(a321b88386424d73d7d73a7f321317b0f21d2eb6) )

	ROM_REGION( 0xc000, "gfx2", 0 )
	ROM_LOAD( "obj 1 pos h1 27128  version3.bin",     0x0000, 0x4000, CRC(63173e65) SHA1(2be3776c0e08d2c876cfce842e02345389e1fba0) )
	ROM_LOAD( "obj 2 pos h3 27128  version3.bin",     0x4000, 0x4000, CRC(3898d1a8) SHA1(acd3d7695a0fe9faa5e4315032c65e131d24a3ce) )
	ROM_LOAD( "obj 3 pos h4 27128  version3.bin",     0x8000, 0x4000, CRC(304e2145) SHA1(d4eb49b5502872718d64e53f02acd2150f6bf713) )

	ROM_REGION( 0x300, "proms", 0 ) // color proms
	ROM_LOAD( "prom pos a9 n82s129",      0x0200, 0x100, CRC(8df9cefe) SHA1(86320eb8135932d79c4478929b9fd90ffba55712) )
	ROM_LOAD( "prom pos a10 n82s129",     0x0100, 0x100, CRC(3c834094) SHA1(4d681431376a8ed071566d74d4accc737bf965dd) )
	ROM_LOAD( "prom pos a11 n82s129",     0x0000, 0x100, CRC(339afa95) SHA1(ff4ff712960f41c26419a681e8dcceaeef75d2e3) )

	ROM_REGION( 0x800, "proms2", 0 ) // look-up tables
	ROM_LOAD( "prom pos e5 n82s131",      0x0000, 0x200, CRC(0024b5dd) SHA1(7d623f8e8964336d643820850cef0fb641e52e22) )
	ROM_LOAD( "prom pos c7 n82s129",      0x0200, 0x100, CRC(9072d259) SHA1(9679fa01372d14a866836c9193204ff6e33cf67c) )
	ROM_LOAD( "prom pos e4 n82s131",      0x0400, 0x200, CRC(307aa2cf) SHA1(839eccf1d34adaf9a5006bfb30e3524bc19a9b41) )
	ROM_LOAD( "prom pos c8 n82s129",      0x0600, 0x100, CRC(351e1ef8) SHA1(530c9012ff5abda1c4ba9787ca999ca1ae1a893d) )
ROM_END

GAME( 1985, sub,  0,    sub, sub, driver_device,  0, ROT270, "Sigma Enterprises Inc.", "Submarine (Sigma)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
