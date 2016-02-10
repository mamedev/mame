// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/***************************************************************************

Ghosts'n Goblins
Diamond Run


Notes:
- Diamond Run doesn't use all the ROMs. d5 is not used at all, and the second
  half of d3o is not used either. There are 0x38 levels in total, the data for
  levels 0x00-0x0f is taken from ROM space 0x8000-0xbfff, the data for levels
  0x10-0x37 is taken from the banked space 0x4000-0x5fff (5 banks) (see the
  table at 0xc66f). Actually, looking at the code it seems to roll overs at
  level 0x2f, and indeed the data for levels 0x30-0x37 isn't valid (player
  starts into a wall, or there are invisible walls, etc.)
  The 0x6000-0x7fff ROM space doesn't seem to be used: instead the game writes
  to 6048 and reads from 6000. Silly copy protection?

- Increased "gfx3" to address 0x400 sprites, to avoid Ghosts'n Goblins
  from drawing a bad sprite. (18/08/2005 Pierpaolo Prazzoli)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6809/m6809.h"
#include "sound/2203intf.h"
#include "includes/gng.h"


WRITE8_MEMBER(gng_state::gng_bankswitch_w)
{
	if (data == 4)
		membank("bank1")->set_entry(4);
	else
		membank("bank1")->set_entry((data & 0x03));
}

WRITE8_MEMBER(gng_state::gng_coin_counter_w)
{
	machine().bookkeeping().coin_counter_w(offset, data);
}

static ADDRESS_MAP_START( gng_map, AS_PROGRAM, 8, gng_state )
	AM_RANGE(0x0000, 0x1dff) AM_RAM
	AM_RANGE(0x1e00, 0x1fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x2000, 0x27ff) AM_RAM_WRITE(gng_fgvideoram_w) AM_SHARE("fgvideoram")
	AM_RANGE(0x2800, 0x2fff) AM_RAM_WRITE(gng_bgvideoram_w) AM_SHARE("bgvideoram")
	AM_RANGE(0x3000, 0x3000) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x3001, 0x3001) AM_READ_PORT("P1")
	AM_RANGE(0x3002, 0x3002) AM_READ_PORT("P2")
	AM_RANGE(0x3003, 0x3003) AM_READ_PORT("DSW1")
	AM_RANGE(0x3004, 0x3004) AM_READ_PORT("DSW2")
	AM_RANGE(0x3800, 0x38ff) AM_DEVWRITE("palette", palette_device, write_ext) AM_SHARE("palette_ext")
	AM_RANGE(0x3900, 0x39ff) AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x3a00, 0x3a00) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0x3b08, 0x3b09) AM_WRITE(gng_bgscrollx_w)
	AM_RANGE(0x3b0a, 0x3b0b) AM_WRITE(gng_bgscrolly_w)
	AM_RANGE(0x3c00, 0x3c00) AM_NOP /* watchdog? */
	AM_RANGE(0x3d00, 0x3d00) AM_WRITE(gng_flipscreen_w)
//  { 0x3d01, 0x3d01, reset sound cpu?
	AM_RANGE(0x3d02, 0x3d03) AM_WRITE(gng_coin_counter_w)
	AM_RANGE(0x3e00, 0x3e00) AM_WRITE(gng_bankswitch_w)
	AM_RANGE(0x4000, 0x5fff) AM_ROMBANK("bank1")
	AM_RANGE(0x6000, 0xffff) AM_ROM
ADDRESS_MAP_END



static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, gng_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xc800) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xe000, 0xe001) AM_DEVWRITE("ym1", ym2203_device, write)
	AM_RANGE(0xe002, 0xe003) AM_DEVWRITE("ym2", ym2203_device, write)
ADDRESS_MAP_END


static INPUT_PORTS_START( gng )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:8,7,6,5")
	PORT_DIPSETTING( 0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING( 0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING( 0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING( 0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING( 0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING( 0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING( 0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING( 0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING( 0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING( 0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING( 0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING( 0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING( 0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING( 0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING( 0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x10, "Coinage affects" )       PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING( 0x10, DEF_STR( Coin_A ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Coin_B ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x40, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:8,7")
	PORT_DIPSETTING( 0x03, "3" )
	PORT_DIPSETTING( 0x02, "4" )
	PORT_DIPSETTING( 0x01, "5" )
	PORT_DIPSETTING( 0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING( 0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING( 0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:5,4")
	PORT_DIPSETTING( 0x18, "20K 70K Every 70K" )
	PORT_DIPSETTING( 0x10, "30K 80K Every 80K" )
	PORT_DIPSETTING( 0x08, "20K and 80K Only" )
	PORT_DIPSETTING( 0x00, "30K and 80K Only" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:3,2")
	PORT_DIPSETTING( 0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING( 0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING( 0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:1" )        /* Listed as "Unused" */
INPUT_PORTS_END

/* identical to gng, but the "unknown" dip switch is Invulnerability */
static INPUT_PORTS_START( makaimur )
	PORT_INCLUDE( gng )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x80, 0x80, "Invulnerability (Cheat)")    PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( diamond )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x04, "*2" )
	PORT_DIPSETTING(    0x08, "*3" )
	PORT_DIPSETTING(    0x0c, "*4" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x40, 0x00, "Unknown DSW1 7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x07, "Energy Loss" )
	PORT_DIPSETTING(    0x00, "Slowest" )
	PORT_DIPSETTING(    0x01, "-6 Slower" )
	PORT_DIPSETTING(    0x02, "-5 Slower" )
	PORT_DIPSETTING(    0x03, "-4 Slower" )
	PORT_DIPSETTING(    0x04, "-3 Slower" )
	PORT_DIPSETTING(    0x05, "-2 Slower" )
	PORT_DIPSETTING(    0x06, "-1 Slower" )
	PORT_DIPSETTING(    0x07, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, "+1 Faster" )
	PORT_DIPSETTING(    0x09, "+2 Faster" )
	PORT_DIPSETTING(    0x0a, "+3 Faster" )
	PORT_DIPSETTING(    0x0b, "+4 Faster" )
	PORT_DIPSETTING(    0x0c, "+5 Faster" )
	PORT_DIPSETTING(    0x0d, "+6 Faster" )
	PORT_DIPSETTING(    0x0e, "+7 Faster" )
	PORT_DIPSETTING(    0x0f, "Fastest" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x10, "*2" )
	PORT_DIPSETTING(    0x20, "*3" )
	PORT_DIPSETTING(    0x30, "*4" )
	PORT_DIPNAME( 0x40, 0x00, "Unknown DSW2 7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Unknown DSW2 8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};
static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};
static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 33*8+0, 33*8+1, 33*8+2, 33*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};



static GFXDECODE_START( gng )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,  0x80, 16 ) /* colors 0x80-0xbf */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,  0x00,  8 ) /* colors 0x00-0x3f */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 0x40, 4 ) /* colors 0x40-0x7f */
GFXDECODE_END




void gng_state::machine_start()
{
	UINT8 *rombase = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 4, &rombase[0x10000], 0x2000);
	membank("bank1")->configure_entry(4, &rombase[0x4000]);

	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
}

void gng_state::machine_reset()
{
	m_scrollx[0] = 0;
	m_scrollx[1] = 0;
	m_scrolly[0] = 0;
	m_scrolly[1] = 0;

	{
		int i;

		/* TODO: PCB reference clearly shows that the POST has random/filled data on the paletteram.
		         For now let's fill everything with white colors until we have better info about it */
		for(i=0;i<0x100;i+=4)
		{
			m_palette->basemem().write8(i, 0x00); m_palette->extmem().write8(i, 0x00);
			m_palette->basemem().write8(i+1, 0x55); m_palette->extmem().write8(i+1, 0x55);
			m_palette->basemem().write8(i+2, 0xaa); m_palette->extmem().write8(i+2, 0xaa);
			m_palette->basemem().write8(i+3, 0xff); m_palette->extmem().write8(i+3, 0xff);
			m_palette->set_pen_color(i+0,0x00,0x00,0x00);
			m_palette->set_pen_color(i+1,0x55,0x55,0x55);
			m_palette->set_pen_color(i+2,0xaa,0xaa,0xaa);
			m_palette->set_pen_color(i+3,0xff,0xff,0xff);
		}
	}
}

static MACHINE_CONFIG_START( gng, gng_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, XTAL_12MHz/8)        /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(gng_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gng_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_12MHz/4)     /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(gng_state, irq0_line_hold, 4*60)


	/* video hardware */
	MCFG_BUFFERED_SPRITERAM8_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.59)    /* verified on pcb */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(gng_state, screen_update_gng)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram8_device, vblank_copy_rising)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", gng)

	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBxxxx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, XTAL_12MHz/8)     /* verified on pcb */
	MCFG_SOUND_ROUTE(0, "mono", 0.40)
	MCFG_SOUND_ROUTE(1, "mono", 0.40)
	MCFG_SOUND_ROUTE(2, "mono", 0.40)
	MCFG_SOUND_ROUTE(3, "mono", 0.20)

	MCFG_SOUND_ADD("ym2", YM2203, XTAL_12MHz/8)     /* verified on pcb */
	MCFG_SOUND_ROUTE(0, "mono", 0.40)
	MCFG_SOUND_ROUTE(1, "mono", 0.40)
	MCFG_SOUND_ROUTE(2, "mono", 0.40)
	MCFG_SOUND_ROUTE(3, "mono", 0.20)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( gng )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "gg4.bin",      0x04000, 0x4000, CRC(66606beb) SHA1(4c640f49be93c7d2b12d4d4c56c56e74099b6c2f) ) /* 4000-5fff is page 4 */
	ROM_LOAD( "gg3.bin",      0x08000, 0x8000, CRC(9e01c65e) SHA1(a87880d87c64a6d61313c3bc69c8d49511e0f9c3) )
	ROM_LOAD( "gg5.bin",      0x10000, 0x8000, CRC(d6397b2b) SHA1(39aa3cb8c229e60ac0ac410ff61e0c09dba78501) ) /* page 0, 1, 2, 3 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gg2.bin",      0x0000, 0x8000, CRC(615f5b6f) SHA1(7ef9ec5c2072e21c787a6bbf700033f50c759c1d) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "gg1.bin",      0x00000, 0x4000, CRC(ecfccf07) SHA1(0a1518e19a2e0a4cc3dde4b9568202ea911b5ece) ) /* characters */

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "gg11.bin",     0x00000, 0x4000, CRC(ddd56fa9) SHA1(f9d77eee5e2738b7e83ba02fcc55dd480391479f) ) /* tiles 0-1 Plane 1*/
	ROM_LOAD( "gg10.bin",     0x04000, 0x4000, CRC(7302529d) SHA1(8434c994cc55d2586641f3b90b6b15fd65dfb67c) ) /* tiles 2-3 Plane 1*/
	ROM_LOAD( "gg9.bin",      0x08000, 0x4000, CRC(20035bda) SHA1(bbb1fba0eb19471f66d29526fa8423ccb047bd63) ) /* tiles 0-1 Plane 2*/
	ROM_LOAD( "gg8.bin",      0x0c000, 0x4000, CRC(f12ba271) SHA1(1c42fa02cb27b35d10c3f7f036005e747f9f6b79) ) /* tiles 2-3 Plane 2*/
	ROM_LOAD( "gg7.bin",      0x10000, 0x4000, CRC(e525207d) SHA1(1947f159189b3a53f1251d8653b6e7c65c91fc3c) ) /* tiles 0-1 Plane 3*/
	ROM_LOAD( "gg6.bin",      0x14000, 0x4000, CRC(2d77e9b2) SHA1(944da1ce29a18bf0fc8deff78bceacba0bf23a07) ) /* tiles 2-3 Plane 3*/

	ROM_REGION( 0x20000, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "gg17.bin",     0x00000, 0x4000, CRC(93e50a8f) SHA1(42d367f57bb2fdf60a0445ac1533da99cfeaa617) ) /* sprites 0 Plane 1-2 */
	ROM_LOAD( "gg16.bin",     0x04000, 0x4000, CRC(06d7e5ca) SHA1(9e06012bcd82f98fad43de666ef9a75979d940ab) ) /* sprites 1 Plane 1-2 */
	ROM_LOAD( "gg15.bin",     0x08000, 0x4000, CRC(bc1fe02d) SHA1(e3a1421d465b87148ffa94f5673b2307f0246afe) ) /* sprites 2 Plane 1-2 */
	ROM_LOAD( "gg14.bin",     0x10000, 0x4000, CRC(6aaf12f9) SHA1(207a7407288182a4f3eddaea634c6a6452131182) ) /* sprites 0 Plane 3-4 */
	ROM_LOAD( "gg13.bin",     0x14000, 0x4000, CRC(e80c3fca) SHA1(cb641c25bb04b970b2cbeca41adb792bbe142fb5) ) /* sprites 1 Plane 3-4 */
	ROM_LOAD( "gg12.bin",     0x18000, 0x4000, CRC(7780a925) SHA1(3f129ca6d695548b659955fe538584bd9ac2ff17) ) /* sprites 2 Plane 3-4 */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbp24s10.14k", 0x0000, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )  /* video timing (not used) */
	ROM_LOAD( "63s141.2e",    0x0100, 0x0100, CRC(4a1285a4) SHA1(5018c3950b675af58db499e2883ecbc55419b491) )  /* priority (not used) */

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "gg-pal10l8.bin",  0x0000, 0x002c, CRC(87f1b7e0) SHA1(b719c3be7bd4a02660bb0887f752e9769cbd37d2) )
ROM_END

ROM_START( gnga )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "gng.n10",      0x04000, 0x4000, CRC(60343188) SHA1(dfc95d3f23a3a4b05b559f1dc76488b2659fbf66) )
	ROM_LOAD( "gng.n9",       0x08000, 0x4000, CRC(b6b91cfb) SHA1(019a38b1c4e987715be1575948a3dc06ee59123d) )
	ROM_LOAD( "gng.n8",       0x0c000, 0x4000, CRC(a5cfa928) SHA1(29dada8c4dbe04969d0d68faac559d2b4a3db711) )
	ROM_LOAD( "gng.n13",      0x10000, 0x4000, CRC(fd9a8dda) SHA1(222c3c759c6b60f82351b9e6bf748fb4872e82b4) )
	ROM_LOAD( "gng.n12",      0x14000, 0x4000, CRC(13cf6238) SHA1(0305908e922891a6a6b6c29e6a099867215d084e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gg2.bin",      0x0000, 0x8000, CRC(615f5b6f) SHA1(7ef9ec5c2072e21c787a6bbf700033f50c759c1d) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "gg1.bin",      0x00000, 0x4000, CRC(ecfccf07) SHA1(0a1518e19a2e0a4cc3dde4b9568202ea911b5ece) ) /* characters */

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "gg11.bin",     0x00000, 0x4000, CRC(ddd56fa9) SHA1(f9d77eee5e2738b7e83ba02fcc55dd480391479f) ) /* tiles 0-1 Plane 1*/
	ROM_LOAD( "gg10.bin",     0x04000, 0x4000, CRC(7302529d) SHA1(8434c994cc55d2586641f3b90b6b15fd65dfb67c) ) /* tiles 2-3 Plane 1*/
	ROM_LOAD( "gg9.bin",      0x08000, 0x4000, CRC(20035bda) SHA1(bbb1fba0eb19471f66d29526fa8423ccb047bd63) ) /* tiles 0-1 Plane 2*/
	ROM_LOAD( "gg8.bin",      0x0c000, 0x4000, CRC(f12ba271) SHA1(1c42fa02cb27b35d10c3f7f036005e747f9f6b79) ) /* tiles 2-3 Plane 2*/
	ROM_LOAD( "gg7.bin",      0x10000, 0x4000, CRC(e525207d) SHA1(1947f159189b3a53f1251d8653b6e7c65c91fc3c) ) /* tiles 0-1 Plane 3*/
	ROM_LOAD( "gg6.bin",      0x14000, 0x4000, CRC(2d77e9b2) SHA1(944da1ce29a18bf0fc8deff78bceacba0bf23a07) ) /* tiles 2-3 Plane 3*/

	ROM_REGION( 0x20000, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "gg17.bin",     0x00000, 0x4000, CRC(93e50a8f) SHA1(42d367f57bb2fdf60a0445ac1533da99cfeaa617) ) /* sprites 0 Plane 1-2 */
	ROM_LOAD( "gg16.bin",     0x04000, 0x4000, CRC(06d7e5ca) SHA1(9e06012bcd82f98fad43de666ef9a75979d940ab) ) /* sprites 1 Plane 1-2 */
	ROM_LOAD( "gg15.bin",     0x08000, 0x4000, CRC(bc1fe02d) SHA1(e3a1421d465b87148ffa94f5673b2307f0246afe) ) /* sprites 2 Plane 1-2 */
	ROM_LOAD( "gg14.bin",     0x10000, 0x4000, CRC(6aaf12f9) SHA1(207a7407288182a4f3eddaea634c6a6452131182) ) /* sprites 0 Plane 3-4 */
	ROM_LOAD( "gg13.bin",     0x14000, 0x4000, CRC(e80c3fca) SHA1(cb641c25bb04b970b2cbeca41adb792bbe142fb5) ) /* sprites 1 Plane 3-4 */
	ROM_LOAD( "gg12.bin",     0x18000, 0x4000, CRC(7780a925) SHA1(3f129ca6d695548b659955fe538584bd9ac2ff17) ) /* sprites 2 Plane 3-4 */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbp24s10.14k", 0x0000, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )  /* video timing (not used) */
	ROM_LOAD( "63s141.2e",    0x0100, 0x0100, CRC(4a1285a4) SHA1(5018c3950b675af58db499e2883ecbc55419b491) )  /* priority (not used) */
ROM_END

ROM_START( gngbl )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "5.84490.10n",      0x04000, 0x4000, CRC(66606beb) SHA1(4c640f49be93c7d2b12d4d4c56c56e74099b6c2f) )
	ROM_LOAD( "4.84490.9n",       0x08000, 0x4000, CRC(527f5c39) SHA1(fcef0159fcb2d8492537408758ac9629781a8c62) )
	ROM_LOAD( "3.84490.8n",       0x0c000, 0x4000, CRC(1c5175d5) SHA1(0e21756517ac39d03af212dd5a04a3bbf633e067) )
	ROM_LOAD( "7.84490.13n",      0x10000, 0x4000, CRC(fd9a8dda) SHA1(222c3c759c6b60f82351b9e6bf748fb4872e82b4) )
	ROM_LOAD( "6.84490.12n",      0x14000, 0x4000, CRC(c83dbd10) SHA1(0243fcb3d228c4c8177ebb6686b84ff512467533) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "2.8529.13h",      0x0000, 0x8000, CRC(55cfb196) SHA1(df9cdbb24c26bca226d7274225725d62ea854c7a) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "1.84490.11e",      0x00000, 0x4000, CRC(ecfccf07) SHA1(0a1518e19a2e0a4cc3dde4b9568202ea911b5ece) ) /* characters */

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "13.84490.3e",     0x00000, 0x4000, CRC(ddd56fa9) SHA1(f9d77eee5e2738b7e83ba02fcc55dd480391479f) ) /* tiles 0-1 Plane 1*/
	ROM_LOAD( "12.84490.1e",     0x04000, 0x4000, CRC(7302529d) SHA1(8434c994cc55d2586641f3b90b6b15fd65dfb67c) ) /* tiles 2-3 Plane 1*/
	ROM_LOAD( "11.84490.3c",      0x08000, 0x4000, CRC(20035bda) SHA1(bbb1fba0eb19471f66d29526fa8423ccb047bd63) ) /* tiles 0-1 Plane 2*/
	ROM_LOAD( "10.84490.1c",      0x0c000, 0x4000, CRC(f12ba271) SHA1(1c42fa02cb27b35d10c3f7f036005e747f9f6b79) ) /* tiles 2-3 Plane 2*/
	ROM_LOAD( "9.84490.3b",      0x10000, 0x4000, CRC(e525207d) SHA1(1947f159189b3a53f1251d8653b6e7c65c91fc3c) ) /* tiles 0-1 Plane 3*/
	ROM_LOAD( "8.84490.1b",      0x14000, 0x4000, CRC(2d77e9b2) SHA1(944da1ce29a18bf0fc8deff78bceacba0bf23a07) ) /* tiles 2-3 Plane 3*/

	ROM_REGION( 0x20000, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "19.84472.4n",     0x00000, 0x4000, CRC(4613afdc) SHA1(13e5a38a134bd7cfa16c63a18fa332c6d66b9345) ) /* sprites 0 Plane 1-2 */
	ROM_LOAD( "18.84472.3n",     0x04000, 0x4000, CRC(06d7e5ca) SHA1(9e06012bcd82f98fad43de666ef9a75979d940ab) ) /* sprites 1 Plane 1-2 */
	ROM_LOAD( "17.84472.1n",     0x08000, 0x4000, CRC(bc1fe02d) SHA1(e3a1421d465b87148ffa94f5673b2307f0246afe) ) /* sprites 2 Plane 1-2 */
	ROM_LOAD( "16.84472.4l",     0x10000, 0x4000, CRC(608d68d5) SHA1(af207f9ee2f93a0cf9cf25cfe72b0fdfe55481b8) ) /* sprites 0 Plane 3-4 */
	ROM_LOAD( "15.84490.3l",     0x14000, 0x4000, CRC(e80c3fca) SHA1(cb641c25bb04b970b2cbeca41adb792bbe142fb5) ) /* sprites 1 Plane 3-4 */
	ROM_LOAD( "14.84490.1l",     0x18000, 0x4000, CRC(7780a925) SHA1(3f129ca6d695548b659955fe538584bd9ac2ff17) ) /* sprites 2 Plane 3-4 */
ROM_END

ROM_START( gngprot )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "gg10n.bin",      0x04000, 0x4000, CRC(5d2a2c90) SHA1(39db20ebf95deb61d887bd88e3cb66c7bbd11f15) )
	ROM_LOAD( "gg9n.bin",       0x08000, 0x4000, CRC(30eb183d) SHA1(8df3d73c9d190edfa0b435cdf994b9f30def2ce0) )
	ROM_LOAD( "gg8n.bin",       0x0c000, 0x4000, CRC(4b5e2145) SHA1(99f269d52ab817fee456b157ce7931859711102a) )
	ROM_LOAD( "gg13n.bin",      0x10000, 0x4000, CRC(2664aae6) SHA1(d2dd3951d115da8a28096d4bad709b1d6f80fc50) )
	ROM_LOAD( "gg12n.bin",      0x14000, 0x4000, CRC(c7ef4ae8) SHA1(ffa34bad487b3a5b249bc8d1bbe614e34d1dc2c6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gg14h.bin",      0x0000, 0x8000, CRC(55cfb196) SHA1(df9cdbb24c26bca226d7274225725d62ea854c7a) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "1.84490.11e",      0x00000, 0x4000, BAD_DUMP CRC(ecfccf07) SHA1(0a1518e19a2e0a4cc3dde4b9568202ea911b5ece) ) /* characters */ // MISSING FROM THIS SET! (was on PCB, why wasn't it dumped?)

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "gg3e.bin",      0x00000, 0x4000, CRC(68db22c8) SHA1(ada859bfa60d9563a8a86b1b6526f626b932981c) ) /* tiles 0-1 Plane 1*/
	ROM_LOAD( "gg1e.bin",      0x04000, 0x4000, CRC(dad8dd2f) SHA1(30a6dd2f6b26acaab0a003f5099a2fcb46644d45) ) /* tiles 2-3 Plane 1*/
	ROM_LOAD( "gg3c.bin",      0x08000, 0x4000, CRC(7a158323) SHA1(183f33f214b4c04e9130cbe2c24e08b5303bb2de) ) /* tiles 0-1 Plane 2*/
	ROM_LOAD( "gg1c.bin",      0x0c000, 0x4000, CRC(7314d095) SHA1(1288eaf0d82ac65a1bb94e68114b4b2f84910901) ) /* tiles 2-3 Plane 2*/
	ROM_LOAD( "gg3b.bin",      0x10000, 0x4000, CRC(03a96d9b) SHA1(5d74e156b0cd1b54d7bd61e1664834f768d5b9f8) ) /* tiles 0-1 Plane 3*/
	ROM_LOAD( "gg1b.bin",      0x14000, 0x4000, CRC(7b9899bc) SHA1(be9b7b18542f38c8fb8b075760995acecace79ad) ) /* tiles 2-3 Plane 3*/

	ROM_REGION( 0x20000, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "gg4l.bin",     0x00000, 0x4000, CRC(49cf81b4) SHA1(b87aba71446884f9926ced28716876ad701b183f) ) /* sprites 0 Plane 1-2 */
	ROM_LOAD( "gg3l.bin",     0x04000, 0x4000, CRC(e61437b1) SHA1(7043ac80ee40057839bf7ee7af62961d9ff3d50b) ) /* sprites 1 Plane 1-2 */
	ROM_LOAD( "gg1l.bin",     0x08000, 0x4000, CRC(bc1fe02d) SHA1(e3a1421d465b87148ffa94f5673b2307f0246afe) ) /* sprites 2 Plane 1-2 */
	ROM_LOAD( "gg4n.bin",     0x10000, 0x4000, CRC(d5aff5a7) SHA1(b75b271c7d38ed9689bff7a3bc9a67a0aae9ed8b) ) /* sprites 0 Plane 3-4 */
	ROM_LOAD( "gg3n.bin",     0x14000, 0x4000, CRC(d589caeb) SHA1(f787557dc083f765aec3d64896ef6cdf5e8d54cc) ) /* sprites 1 Plane 3-4 */
	ROM_LOAD( "gg1n.bin",     0x18000, 0x4000, CRC(7780a925) SHA1(3f129ca6d695548b659955fe538584bd9ac2ff17) ) /* sprites 2 Plane 3-4 */
ROM_END


ROM_START( gngblita )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "3",      0x04000, 0x4000, CRC(4859d068) SHA1(8b22772ea383ecaee01da696c0c7b568ab1e4615) )
	ROM_LOAD( "4-5",    0x08000, 0x8000, CRC(233a4589) SHA1(c95c4dcbf53fac5a0c11466bc27369bf8328bd01) )
	ROM_LOAD( "1-2",    0x10000, 0x8000, CRC(ed28e86e) SHA1(064871918547a56be330c6994d4db2c9932e14db) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gg2.bin",      0x0000, 0x8000, CRC(615f5b6f) SHA1(7ef9ec5c2072e21c787a6bbf700033f50c759c1d) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "gg1.bin",      0x00000, 0x4000, CRC(ecfccf07) SHA1(0a1518e19a2e0a4cc3dde4b9568202ea911b5ece) ) /* characters */

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "gg11.bin",     0x00000, 0x4000, CRC(ddd56fa9) SHA1(f9d77eee5e2738b7e83ba02fcc55dd480391479f) ) /* tiles 0-1 Plane 1*/
	ROM_LOAD( "gg10.bin",     0x04000, 0x4000, CRC(7302529d) SHA1(8434c994cc55d2586641f3b90b6b15fd65dfb67c) ) /* tiles 2-3 Plane 1*/
	ROM_LOAD( "gg9.bin",      0x08000, 0x4000, CRC(20035bda) SHA1(bbb1fba0eb19471f66d29526fa8423ccb047bd63) ) /* tiles 0-1 Plane 2*/
	ROM_LOAD( "gg8.bin",      0x0c000, 0x4000, CRC(f12ba271) SHA1(1c42fa02cb27b35d10c3f7f036005e747f9f6b79) ) /* tiles 2-3 Plane 2*/
	ROM_LOAD( "gg7.bin",      0x10000, 0x4000, CRC(e525207d) SHA1(1947f159189b3a53f1251d8653b6e7c65c91fc3c) ) /* tiles 0-1 Plane 3*/
	ROM_LOAD( "gg6.bin",      0x14000, 0x4000, CRC(2d77e9b2) SHA1(944da1ce29a18bf0fc8deff78bceacba0bf23a07) ) /* tiles 2-3 Plane 3*/

	ROM_REGION( 0x20000, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "gg17.bin",     0x00000, 0x4000, CRC(93e50a8f) SHA1(42d367f57bb2fdf60a0445ac1533da99cfeaa617) ) /* sprites 0 Plane 1-2 */
	ROM_LOAD( "gg16.bin",     0x04000, 0x4000, CRC(06d7e5ca) SHA1(9e06012bcd82f98fad43de666ef9a75979d940ab) ) /* sprites 1 Plane 1-2 */
	ROM_LOAD( "gg15.bin",     0x08000, 0x4000, CRC(bc1fe02d) SHA1(e3a1421d465b87148ffa94f5673b2307f0246afe) ) /* sprites 2 Plane 1-2 */
	ROM_LOAD( "gg14.bin",     0x10000, 0x4000, CRC(6aaf12f9) SHA1(207a7407288182a4f3eddaea634c6a6452131182) ) /* sprites 0 Plane 3-4 */
	ROM_LOAD( "gg13.bin",     0x14000, 0x4000, CRC(e80c3fca) SHA1(cb641c25bb04b970b2cbeca41adb792bbe142fb5) ) /* sprites 1 Plane 3-4 */
	ROM_LOAD( "gg12.bin",     0x18000, 0x4000, CRC(7780a925) SHA1(3f129ca6d695548b659955fe538584bd9ac2ff17) ) /* sprites 2 Plane 3-4 */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbp24s10.14k", 0x0000, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )  /* video timing (not used) */
	ROM_LOAD( "63s141.2e",    0x0100, 0x0100, CRC(4a1285a4) SHA1(5018c3950b675af58db499e2883ecbc55419b491) )  /* priority (not used) */

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "gg-pal10l8.bin",  0x0000, 0x002c, CRC(87f1b7e0) SHA1(b719c3be7bd4a02660bb0887f752e9769cbd37d2) )
ROM_END

ROM_START( gngc )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "mm_c_04",    0x04000, 0x4000, CRC(4f94130f) SHA1(6863fee3c97c76ba314ccbada7efacb6783e7d32) )
	ROM_LOAD( "mm_c_03",    0x08000, 0x8000, CRC(1def138a) SHA1(d29e12082c4d5c06a9910b3500ef66242cdec905) )
	ROM_LOAD( "mm_c_05",    0x10000, 0x8000, CRC(ed28e86e) SHA1(064871918547a56be330c6994d4db2c9932e14db) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gg2.bin",      0x0000, 0x8000, CRC(615f5b6f) SHA1(7ef9ec5c2072e21c787a6bbf700033f50c759c1d) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "gg1.bin",      0x00000, 0x4000, CRC(ecfccf07) SHA1(0a1518e19a2e0a4cc3dde4b9568202ea911b5ece) ) /* characters */

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "gg11.bin",     0x00000, 0x4000, CRC(ddd56fa9) SHA1(f9d77eee5e2738b7e83ba02fcc55dd480391479f) ) /* tiles 0-1 Plane 1*/
	ROM_LOAD( "gg10.bin",     0x04000, 0x4000, CRC(7302529d) SHA1(8434c994cc55d2586641f3b90b6b15fd65dfb67c) ) /* tiles 2-3 Plane 1*/
	ROM_LOAD( "gg9.bin",      0x08000, 0x4000, CRC(20035bda) SHA1(bbb1fba0eb19471f66d29526fa8423ccb047bd63) ) /* tiles 0-1 Plane 2*/
	ROM_LOAD( "gg8.bin",      0x0c000, 0x4000, CRC(f12ba271) SHA1(1c42fa02cb27b35d10c3f7f036005e747f9f6b79) ) /* tiles 2-3 Plane 2*/
	ROM_LOAD( "gg7.bin",      0x10000, 0x4000, CRC(e525207d) SHA1(1947f159189b3a53f1251d8653b6e7c65c91fc3c) ) /* tiles 0-1 Plane 3*/
	ROM_LOAD( "gg6.bin",      0x14000, 0x4000, CRC(2d77e9b2) SHA1(944da1ce29a18bf0fc8deff78bceacba0bf23a07) ) /* tiles 2-3 Plane 3*/

	ROM_REGION( 0x20000, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "gg17.bin",     0x00000, 0x4000, CRC(93e50a8f) SHA1(42d367f57bb2fdf60a0445ac1533da99cfeaa617) ) /* sprites 0 Plane 1-2 */
	ROM_LOAD( "gg16.bin",     0x04000, 0x4000, CRC(06d7e5ca) SHA1(9e06012bcd82f98fad43de666ef9a75979d940ab) ) /* sprites 1 Plane 1-2 */
	ROM_LOAD( "gg15.bin",     0x08000, 0x4000, CRC(bc1fe02d) SHA1(e3a1421d465b87148ffa94f5673b2307f0246afe) ) /* sprites 2 Plane 1-2 */
	ROM_LOAD( "gg14.bin",     0x10000, 0x4000, CRC(6aaf12f9) SHA1(207a7407288182a4f3eddaea634c6a6452131182) ) /* sprites 0 Plane 3-4 */
	ROM_LOAD( "gg13.bin",     0x14000, 0x4000, CRC(e80c3fca) SHA1(cb641c25bb04b970b2cbeca41adb792bbe142fb5) ) /* sprites 1 Plane 3-4 */
	ROM_LOAD( "gg12.bin",     0x18000, 0x4000, CRC(7780a925) SHA1(3f129ca6d695548b659955fe538584bd9ac2ff17) ) /* sprites 2 Plane 3-4 */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbp24s10.14k", 0x0000, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )  /* video timing (not used) */
	ROM_LOAD( "63s141.2e",    0x0100, 0x0100, CRC(4a1285a4) SHA1(5018c3950b675af58db499e2883ecbc55419b491) )  /* priority (not used) */
ROM_END


/*
    Ghosts'n Goblins (US)

    CPU/Sound Board: 85606-A-4
    Video Board:     85606-B-3
*/

ROM_START( gngt )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "mmt04d.10n", 0x04000, 0x4000, CRC(652406f6) SHA1(3b2bafd31f670ea26c568c48f3bd00597e5a2ed6) ) /* 4000-5fff is page 4 */
	ROM_LOAD( "mmt03d.8n",  0x08000, 0x8000, CRC(fb040b42) SHA1(c1c58943bd20c6a2520b39fae90067769ec97ed6) )
	ROM_LOAD( "mmt05d.13n", 0x10000, 0x8000, CRC(8f7cff61) SHA1(1875f254a7737e1fbf6770ee4a322d675d043a44) ) /* page 0, 1, 2, 3 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mm02.14h",   0x0000, 0x8000, CRC(615f5b6f) SHA1(7ef9ec5c2072e21c787a6bbf700033f50c759c1d) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "mm01.11e",   0x00000, 0x4000, CRC(ecfccf07) SHA1(0a1518e19a2e0a4cc3dde4b9568202ea911b5ece) ) /* characters */

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "mm11.3e",    0x00000, 0x4000, CRC(ddd56fa9) SHA1(f9d77eee5e2738b7e83ba02fcc55dd480391479f) ) /* tiles 0-1 Plane 1*/
	ROM_LOAD( "mm10.1e",    0x04000, 0x4000, CRC(7302529d) SHA1(8434c994cc55d2586641f3b90b6b15fd65dfb67c) ) /* tiles 2-3 Plane 1*/
	ROM_LOAD( "mm09.3c",    0x08000, 0x4000, CRC(20035bda) SHA1(bbb1fba0eb19471f66d29526fa8423ccb047bd63) ) /* tiles 0-1 Plane 2*/
	ROM_LOAD( "mm08.1c",    0x0c000, 0x4000, CRC(f12ba271) SHA1(1c42fa02cb27b35d10c3f7f036005e747f9f6b79) ) /* tiles 2-3 Plane 2*/
	ROM_LOAD( "mm07.3b",    0x10000, 0x4000, CRC(e525207d) SHA1(1947f159189b3a53f1251d8653b6e7c65c91fc3c) ) /* tiles 0-1 Plane 3*/
	ROM_LOAD( "mm06.1b",    0x14000, 0x4000, CRC(2d77e9b2) SHA1(944da1ce29a18bf0fc8deff78bceacba0bf23a07) ) /* tiles 2-3 Plane 3*/

	ROM_REGION( 0x20000, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "mm17.4n",    0x00000, 0x4000, CRC(93e50a8f) SHA1(42d367f57bb2fdf60a0445ac1533da99cfeaa617) ) /* sprites 0 Plane 1-2 */
	ROM_LOAD( "mm16.3n",    0x04000, 0x4000, CRC(06d7e5ca) SHA1(9e06012bcd82f98fad43de666ef9a75979d940ab) ) /* sprites 1 Plane 1-2 */
	ROM_LOAD( "mm15.1n",    0x08000, 0x4000, CRC(bc1fe02d) SHA1(e3a1421d465b87148ffa94f5673b2307f0246afe) ) /* sprites 2 Plane 1-2 */
	ROM_LOAD( "mm14.4l",    0x10000, 0x4000, CRC(6aaf12f9) SHA1(207a7407288182a4f3eddaea634c6a6452131182) ) /* sprites 0 Plane 3-4 */
	ROM_LOAD( "mm13.3l",    0x14000, 0x4000, CRC(e80c3fca) SHA1(cb641c25bb04b970b2cbeca41adb792bbe142fb5) ) /* sprites 1 Plane 3-4 */
	ROM_LOAD( "mm12.1l",    0x18000, 0x4000, CRC(7780a925) SHA1(3f129ca6d695548b659955fe538584bd9ac2ff17) ) /* sprites 2 Plane 3-4 */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "m-02.14k",   0x0000, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )  /* 63s141, video timing (not used) */
	ROM_LOAD( "m-01.2e",    0x0100, 0x0100, CRC(4a1285a4) SHA1(5018c3950b675af58db499e2883ecbc55419b491) )  /* 63s141, priority (not used) */
ROM_END

ROM_START( makaimur )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "10n.rom",      0x04000, 0x4000, CRC(81e567e0) SHA1(2a917e562686e782ea7d034a54e9728695a19258) ) /* 4000-5fff is page 4 */
	ROM_LOAD( "8n.rom",       0x08000, 0x8000, CRC(9612d66c) SHA1(64c458d6d87b9c339488c9f0c89da2c796fcb759) )
	ROM_LOAD( "12n.rom",      0x10000, 0x8000, CRC(65a6a97b) SHA1(79de931ae183d8044cb6d9024b475196c679c86c) ) /* page 0, 1, 2, 3 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gg2.bin",      0x0000, 0x8000, CRC(615f5b6f) SHA1(7ef9ec5c2072e21c787a6bbf700033f50c759c1d) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "gg1.bin",      0x00000, 0x4000, CRC(ecfccf07) SHA1(0a1518e19a2e0a4cc3dde4b9568202ea911b5ece) ) /* characters */

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "gg11.bin",     0x00000, 0x4000, CRC(ddd56fa9) SHA1(f9d77eee5e2738b7e83ba02fcc55dd480391479f) ) /* tiles 0-1 Plane 1*/
	ROM_LOAD( "gg10.bin",     0x04000, 0x4000, CRC(7302529d) SHA1(8434c994cc55d2586641f3b90b6b15fd65dfb67c) ) /* tiles 2-3 Plane 1*/
	ROM_LOAD( "gg9.bin",      0x08000, 0x4000, CRC(20035bda) SHA1(bbb1fba0eb19471f66d29526fa8423ccb047bd63) ) /* tiles 0-1 Plane 2*/
	ROM_LOAD( "gg8.bin",      0x0c000, 0x4000, CRC(f12ba271) SHA1(1c42fa02cb27b35d10c3f7f036005e747f9f6b79) ) /* tiles 2-3 Plane 2*/
	ROM_LOAD( "gg7.bin",      0x10000, 0x4000, CRC(e525207d) SHA1(1947f159189b3a53f1251d8653b6e7c65c91fc3c) ) /* tiles 0-1 Plane 3*/
	ROM_LOAD( "gg6.bin",      0x14000, 0x4000, CRC(2d77e9b2) SHA1(944da1ce29a18bf0fc8deff78bceacba0bf23a07) ) /* tiles 2-3 Plane 3*/

	ROM_REGION( 0x20000, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "gng13.n4",     0x00000, 0x4000, CRC(4613afdc) SHA1(13e5a38a134bd7cfa16c63a18fa332c6d66b9345) ) /* sprites 0 Plane 1-2 */
	ROM_LOAD( "gg16.bin",     0x04000, 0x4000, CRC(06d7e5ca) SHA1(9e06012bcd82f98fad43de666ef9a75979d940ab) ) /* sprites 1 Plane 1-2 */
	ROM_LOAD( "gg15.bin",     0x08000, 0x4000, CRC(bc1fe02d) SHA1(e3a1421d465b87148ffa94f5673b2307f0246afe) ) /* sprites 2 Plane 1-2 */
	ROM_LOAD( "gng16.l4",     0x10000, 0x4000, CRC(608d68d5) SHA1(af207f9ee2f93a0cf9cf25cfe72b0fdfe55481b8) ) /* sprites 0 Plane 3-4 */
	ROM_LOAD( "gg13.bin",     0x14000, 0x4000, CRC(e80c3fca) SHA1(cb641c25bb04b970b2cbeca41adb792bbe142fb5) ) /* sprites 1 Plane 3-4 */
	ROM_LOAD( "gg12.bin",     0x18000, 0x4000, CRC(7780a925) SHA1(3f129ca6d695548b659955fe538584bd9ac2ff17) ) /* sprites 2 Plane 3-4 */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbp24s10.14k", 0x0000, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )  /* video timing (not used) */
	ROM_LOAD( "63s141.2e",    0x0100, 0x0100, CRC(4a1285a4) SHA1(5018c3950b675af58db499e2883ecbc55419b491) )  /* priority (not used) */
ROM_END

ROM_START( makaimurc )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "mj04c.bin",      0x04000, 0x4000, CRC(1294edb1) SHA1(35d3b3ce4ee25d3cfa27097de0c9a2ab5e4892aa) )   /* 4000-5fff is page 4 */
	ROM_LOAD( "mj03c.bin",      0x08000, 0x8000, CRC(d343332d) SHA1(3edf47ff2bd49b4451b737b6d3eb54256b489c81) )
	ROM_LOAD( "mj05c.bin",      0x10000, 0x8000, CRC(535342c2) SHA1(5f05a4965476f2db6fddec557d93d12ec1f9750c) )   /* page 0, 1, 2, 3 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gg2.bin",      0x0000, 0x8000, CRC(615f5b6f) SHA1(7ef9ec5c2072e21c787a6bbf700033f50c759c1d) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "gg1.bin",      0x00000, 0x4000, CRC(ecfccf07) SHA1(0a1518e19a2e0a4cc3dde4b9568202ea911b5ece) ) /* characters */

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "gg11.bin",     0x00000, 0x4000, CRC(ddd56fa9) SHA1(f9d77eee5e2738b7e83ba02fcc55dd480391479f) ) /* tiles 0-1 Plane 1*/
	ROM_LOAD( "gg10.bin",     0x04000, 0x4000, CRC(7302529d) SHA1(8434c994cc55d2586641f3b90b6b15fd65dfb67c) ) /* tiles 2-3 Plane 1*/
	ROM_LOAD( "gg9.bin",      0x08000, 0x4000, CRC(20035bda) SHA1(bbb1fba0eb19471f66d29526fa8423ccb047bd63) ) /* tiles 0-1 Plane 2*/
	ROM_LOAD( "gg8.bin",      0x0c000, 0x4000, CRC(f12ba271) SHA1(1c42fa02cb27b35d10c3f7f036005e747f9f6b79) ) /* tiles 2-3 Plane 2*/
	ROM_LOAD( "gg7.bin",      0x10000, 0x4000, CRC(e525207d) SHA1(1947f159189b3a53f1251d8653b6e7c65c91fc3c) ) /* tiles 0-1 Plane 3*/
	ROM_LOAD( "gg6.bin",      0x14000, 0x4000, CRC(2d77e9b2) SHA1(944da1ce29a18bf0fc8deff78bceacba0bf23a07) ) /* tiles 2-3 Plane 3*/

	ROM_REGION( 0x20000, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "gng13.n4",     0x00000, 0x4000, CRC(4613afdc) SHA1(13e5a38a134bd7cfa16c63a18fa332c6d66b9345) ) /* sprites 0 Plane 1-2 */
	ROM_LOAD( "gg16.bin",     0x04000, 0x4000, CRC(06d7e5ca) SHA1(9e06012bcd82f98fad43de666ef9a75979d940ab) ) /* sprites 1 Plane 1-2 */
	ROM_LOAD( "gg15.bin",     0x08000, 0x4000, CRC(bc1fe02d) SHA1(e3a1421d465b87148ffa94f5673b2307f0246afe) ) /* sprites 2 Plane 1-2 */
	ROM_LOAD( "gng16.l4",     0x10000, 0x4000, CRC(608d68d5) SHA1(af207f9ee2f93a0cf9cf25cfe72b0fdfe55481b8) ) /* sprites 0 Plane 3-4 */
	ROM_LOAD( "gg13.bin",     0x14000, 0x4000, CRC(e80c3fca) SHA1(cb641c25bb04b970b2cbeca41adb792bbe142fb5) ) /* sprites 1 Plane 3-4 */
	ROM_LOAD( "gg12.bin",     0x18000, 0x4000, CRC(7780a925) SHA1(3f129ca6d695548b659955fe538584bd9ac2ff17) ) /* sprites 2 Plane 3-4 */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbp24s10.14k", 0x0000, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )  /* video timing (not used) */
	ROM_LOAD( "63s141.2e",    0x0100, 0x0100, CRC(4a1285a4) SHA1(5018c3950b675af58db499e2883ecbc55419b491) )  /* priority (not used) */
ROM_END

ROM_START( makaimurg )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "mj04g.bin",      0x04000, 0x4000, CRC(757c94d3) SHA1(07f7cf788810a1425016e016ce3579adb3253ac7) )   /* 4000-5fff is page 4 */
	ROM_LOAD( "mj03g.bin",      0x08000, 0x8000, CRC(61b043bb) SHA1(23a0a17d0abc4b084ffeba90266ef455361771cc) )
	ROM_LOAD( "mj05g.bin",      0x10000, 0x8000, CRC(f2fdccf5) SHA1(7694a981a6196d77fd2279fc34042b4cfb40c054) )   /* page 0, 1, 2, 3 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gg2.bin",      0x0000, 0x8000, CRC(615f5b6f) SHA1(7ef9ec5c2072e21c787a6bbf700033f50c759c1d) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "gg1.bin",      0x00000, 0x4000, CRC(ecfccf07) SHA1(0a1518e19a2e0a4cc3dde4b9568202ea911b5ece) ) /* characters */

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "gg11.bin",     0x00000, 0x4000, CRC(ddd56fa9) SHA1(f9d77eee5e2738b7e83ba02fcc55dd480391479f) ) /* tiles 0-1 Plane 1*/
	ROM_LOAD( "gg10.bin",     0x04000, 0x4000, CRC(7302529d) SHA1(8434c994cc55d2586641f3b90b6b15fd65dfb67c) ) /* tiles 2-3 Plane 1*/
	ROM_LOAD( "gg9.bin",      0x08000, 0x4000, CRC(20035bda) SHA1(bbb1fba0eb19471f66d29526fa8423ccb047bd63) ) /* tiles 0-1 Plane 2*/
	ROM_LOAD( "gg8.bin",      0x0c000, 0x4000, CRC(f12ba271) SHA1(1c42fa02cb27b35d10c3f7f036005e747f9f6b79) ) /* tiles 2-3 Plane 2*/
	ROM_LOAD( "gg7.bin",      0x10000, 0x4000, CRC(e525207d) SHA1(1947f159189b3a53f1251d8653b6e7c65c91fc3c) ) /* tiles 0-1 Plane 3*/
	ROM_LOAD( "gg6.bin",      0x14000, 0x4000, CRC(2d77e9b2) SHA1(944da1ce29a18bf0fc8deff78bceacba0bf23a07) ) /* tiles 2-3 Plane 3*/

	ROM_REGION( 0x20000, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "gng13.n4",     0x00000, 0x4000, CRC(4613afdc) SHA1(13e5a38a134bd7cfa16c63a18fa332c6d66b9345) ) /* sprites 0 Plane 1-2 */
	ROM_LOAD( "gg16.bin",     0x04000, 0x4000, CRC(06d7e5ca) SHA1(9e06012bcd82f98fad43de666ef9a75979d940ab) ) /* sprites 1 Plane 1-2 */
	ROM_LOAD( "gg15.bin",     0x08000, 0x4000, CRC(bc1fe02d) SHA1(e3a1421d465b87148ffa94f5673b2307f0246afe) ) /* sprites 2 Plane 1-2 */
	ROM_LOAD( "gng16.l4",     0x10000, 0x4000, CRC(608d68d5) SHA1(af207f9ee2f93a0cf9cf25cfe72b0fdfe55481b8) ) /* sprites 0 Plane 3-4 */
	ROM_LOAD( "gg13.bin",     0x14000, 0x4000, CRC(e80c3fca) SHA1(cb641c25bb04b970b2cbeca41adb792bbe142fb5) ) /* sprites 1 Plane 3-4 */
	ROM_LOAD( "gg12.bin",     0x18000, 0x4000, CRC(7780a925) SHA1(3f129ca6d695548b659955fe538584bd9ac2ff17) ) /* sprites 2 Plane 3-4 */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbp24s10.14k", 0x0000, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )  /* video timing (not used) */
	ROM_LOAD( "63s141.2e",    0x0100, 0x0100, CRC(4a1285a4) SHA1(5018c3950b675af58db499e2883ecbc55419b491) )  /* priority (not used) */
ROM_END

ROM_START( diamond )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "d3o",          0x04000, 0x4000, CRC(ba4bf9f1) SHA1(460e01f5ba9cd0c76d1a2ea1e66e9ad49ef1e13b) ) /* 4000-5fff is page 4 */
	ROM_LOAD( "d3",           0x08000, 0x8000, CRC(f436d6fa) SHA1(18287ac51e717ea2ba9b307a738f76735120f21b) )
	ROM_LOAD( "d5o",          0x10000, 0x8000, CRC(ae58bd3a) SHA1(674611c7107cae53150fc249ffc616df891698fe) ) /* page 0, 1, 2, 3 */
	ROM_LOAD( "d5",           0x18000, 0x8000, CRC(453f3f9e) SHA1(b4dcf2eb0e6d4eca8ccde6e1a60f5e002e49a57b) ) /* is this supposed to be used? */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "d2",           0x0000, 0x8000, CRC(615f5b6f) SHA1(7ef9ec5c2072e21c787a6bbf700033f50c759c1d) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "d1",           0x00000, 0x4000, CRC(3a24e504) SHA1(56bc38413b8a0dc2829e9c8f7bcfabafe26fd257) ) /* characters */

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "d11",          0x00000, 0x4000, CRC(754357d7) SHA1(eb6e07a5f2d02687306711845001205bf0efa61b) ) /* tiles 0-1 Plane 1*/
	ROM_LOAD( "d10",          0x04000, 0x4000, CRC(7531edcd) SHA1(dc3eabf7e7503f0588f65620d26c1bc5eebde211) ) /* tiles 2-3 Plane 1*/
	ROM_LOAD( "d9",           0x08000, 0x4000, CRC(22eeca08) SHA1(6454b6c0a7a0991744386b79d4988a2517ad0636) ) /* tiles 0-1 Plane 2*/
	ROM_LOAD( "d8",           0x0c000, 0x4000, CRC(6b61be60) SHA1(a92ff6e922da523caec1919f53bea48dab4ca564) ) /* tiles 2-3 Plane 2*/
	ROM_LOAD( "d7",           0x10000, 0x4000, CRC(fd595274) SHA1(8d22f89a7251ecc8b56ee3f8cfaab2fd5a716b3f) ) /* tiles 0-1 Plane 3*/
	ROM_LOAD( "d6",           0x14000, 0x4000, CRC(7f51dcd2) SHA1(ff4a68a7a6a5caa558898b03ba4a4dc3ab43ce30) ) /* tiles 2-3 Plane 3*/

	ROM_REGION( 0x20000, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "d17",          0x00000, 0x4000, CRC(8164b005) SHA1(d03bf62734b03c90a8393a23f8ce0a3769c43bf7) ) /* sprites 0 Plane 1-2 */
	ROM_LOAD( "d14",          0x10000, 0x4000, CRC(6f132163) SHA1(cd1ebf9671bcce58896dadbf20f036eaadbe8bd5) ) /* sprites 0 Plane 3-4 */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "prom1",        0x0000, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )  /* video timing (not used) */
	ROM_LOAD( "prom2",        0x0100, 0x0100, CRC(4a1285a4) SHA1(5018c3950b675af58db499e2883ecbc55419b491) )  /* priority (not used) */
ROM_END



READ8_MEMBER(gng_state::diamond_hack_r)
{
	return 0;
}

DRIVER_INIT_MEMBER(gng_state,diamond)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x6000, 0x6000, read8_delegate(FUNC(gng_state::diamond_hack_r),this));
}



GAME( 1985, gng,       0,   gng, gng, driver_device,      0,       ROT0, "Capcom", "Ghosts'n Goblins (World? set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, gnga,      gng, gng, gng, driver_device,      0,       ROT0, "Capcom", "Ghosts'n Goblins (World? set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, gngbl,     gng, gng, gng, driver_device,      0,       ROT0, "bootleg", "Ghosts'n Goblins (bootleg with Cross)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, gngprot,   gng, gng, gng, driver_device,      0,       ROT0, "Capcom", "Ghosts'n Goblins (prototype)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, gngblita,  gng, gng, gng, driver_device,      0,       ROT0, "bootleg", "Ghosts'n Goblins (Italian bootleg, harder)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, gngc,      gng, gng, gng, driver_device,      0,       ROT0, "Capcom", "Ghosts'n Goblins (World? set 3)", MACHINE_SUPPORTS_SAVE ) // rev c?
GAME( 1985, gngt,      gng, gng, gng, driver_device,      0,       ROT0, "Capcom (Taito America license)", "Ghosts'n Goblins (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, makaimur,  gng, gng, makaimur, driver_device, 0,       ROT0, "Capcom", "Makai-Mura (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, makaimurc, gng, gng, makaimur, driver_device, 0,       ROT0, "Capcom", "Makai-Mura (Japan Revision C)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, makaimurg, gng, gng, makaimur, driver_device, 0,       ROT0, "Capcom", "Makai-Mura (Japan Revision G)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, diamond,   0,   gng, diamond, gng_state,  diamond, ROT0, "KH Video", "Diamond Run", MACHINE_SUPPORTS_SAVE )
