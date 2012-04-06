/*
 Moero Pro Yakyuu Homerun - (c) 1988 Jaleco
 Dynamic Shooting - (c) 1988 Jaleco
 Driver by Tomasz Slanina

 *weird* hardware - based on NES version
 (gfx bank changed in the middle of screen,
  sprites in NES format etc)

Todo :
 - voice ( unemulated D7756C )
 - controls/dips
 - better emulation of gfx bank switching
 - is there 2 player mode ?

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/8255ppi.h"
#include "sound/2203intf.h"
#include "includes/homerun.h"


static WRITE8_DEVICE_HANDLER(pa_w)
{
	homerun_state *state = device->machine().driver_data<homerun_state>();
	state->m_xpa = data;
}

static WRITE8_DEVICE_HANDLER(pb_w)
{
	homerun_state *state = device->machine().driver_data<homerun_state>();
	state->m_xpb = data;
}

static WRITE8_DEVICE_HANDLER(pc_w)
{
	homerun_state *state = device->machine().driver_data<homerun_state>();
	state->m_xpc = data;
}



static const ppi8255_interface ppi8255_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(pa_w),
	DEVCB_HANDLER(pb_w),
	DEVCB_HANDLER(pc_w)
};


static ADDRESS_MAP_START( homerun_memmap, AS_PROGRAM, 8, homerun_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x9fff) AM_RAM_WRITE(homerun_videoram_w) AM_BASE(m_videoram)
	AM_RANGE(0xa000, 0xa0ff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size)
	AM_RANGE(0xb000, 0xb0ff) AM_WRITE(homerun_color_w)
	AM_RANGE(0xc000, 0xdfff) AM_RAM
ADDRESS_MAP_END

static CUSTOM_INPUT( homerun_40_r )
{
	UINT8 ret = (field.machine().primary_screen->vpos() > 116) ? 1 : 0;

	return ret;
}

static ADDRESS_MAP_START( homerun_iomap, AS_IO, 8, homerun_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_WRITENOP /* ?? */
	AM_RANGE(0x20, 0x20) AM_WRITENOP /* ?? */
	AM_RANGE(0x30, 0x33) AM_DEVREADWRITE_LEGACY("ppi8255", ppi8255_r, ppi8255_w)
	AM_RANGE(0x40, 0x40) AM_READ_PORT("IN0")
	AM_RANGE(0x50, 0x50) AM_READ_PORT("IN2")
	AM_RANGE(0x60, 0x60) AM_READ_PORT("IN1")
	AM_RANGE(0x70, 0x71) AM_DEVREADWRITE_LEGACY("ymsnd", ym2203_r, ym2203_w)
ADDRESS_MAP_END

static const ym2203_interface ym2203_config =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_INPUT_PORT("DSW"),
		DEVCB_NULL,
		DEVCB_NULL,
		DEVCB_HANDLER(homerun_banking_w)
	},
	NULL
};


static INPUT_PORTS_START( homerun )
	PORT_START("IN0")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(homerun_40_r, NULL)
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("IN2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(	0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(	0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x03, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(	0x80, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x00, DEF_STR( 1C_2C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dynashot )
	PORT_START("IN0")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(homerun_40_r, NULL)
	PORT_BIT( 0xb7, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("IN2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xdf, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(	0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(	0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x03, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(	0x80, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x00, DEF_STR( 1C_2C ) )

	PORT_DIPNAME( 0x7c, 0x7c, "Collisions ?" ) //not all bits
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x7c, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout gfxlayout =
{
   8,8,
   RGN_FRAC(1,1),
   2,
   { 8*8,0},
   { 0, 1, 2, 3, 4, 5, 6, 7},
   { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
   8*8*2
};



static const gfx_layout spritelayout =
{
   16,16,
   RGN_FRAC(1,1),
   2,
   { 8*8,0},
   { 0, 1, 2, 3, 4, 5, 6, 7,0+8*8*2,1+8*8*2,2+8*8*2,3+8*8*2,4+8*8*2,5+8*8*2,6+8*8*2,7+8*8*2},
   { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 0*8+2*8*8*2,1*8+2*8*8*2,2*8+2*8*8*2,3*8+2*8*8*2,4*8+2*8*8*2,5*8+2*8*8*2,6*8+2*8*8*2,7*8+2*8*8*2},
   8*8*2*4
};

static GFXDECODE_START( homerun )
	GFXDECODE_ENTRY( "gfx1", 0, gfxlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,   0, 16 )
GFXDECODE_END


static MACHINE_START( homerun )
{
	homerun_state *state = machine.driver_data<homerun_state>();
	UINT8 *ROM = machine.region("maincpu")->base();

	memory_configure_bank(machine, "bank1", 0, 1, &ROM[0x00000], 0x4000);
	memory_configure_bank(machine, "bank1", 1, 7, &ROM[0x10000], 0x4000);

	state->save_item(NAME(state->m_gfx_ctrl));
	state->save_item(NAME(state->m_gc_up));
	state->save_item(NAME(state->m_gc_down));
	state->save_item(NAME(state->m_xpa));
	state->save_item(NAME(state->m_xpb));
	state->save_item(NAME(state->m_xpc));
}

static MACHINE_RESET( homerun )
{
	homerun_state *state = machine.driver_data<homerun_state>();

	state->m_gfx_ctrl = 0;
	state->m_gc_up = 0;
	state->m_gc_down = 0;
	state->m_xpa = 0;
	state->m_xpb = 0;
	state->m_xpc = 0;
}

static MACHINE_CONFIG_START( homerun, homerun_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 5000000)
	MCFG_CPU_PROGRAM_MAP(homerun_memmap)
	MCFG_CPU_IO_MAP(homerun_iomap)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_MACHINE_START(homerun)
	MCFG_MACHINE_RESET(homerun)

	MCFG_PPI8255_ADD( "ppi8255", ppi8255_intf )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-25)
	MCFG_SCREEN_UPDATE_STATIC(homerun)

	MCFG_GFXDECODE(homerun)
	MCFG_PALETTE_LENGTH(16*4)

	MCFG_VIDEO_START(homerun)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, 6000000/2)
	MCFG_SOUND_CONFIG(ym2203_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

MACHINE_CONFIG_END


/*
Moero Pro Yakyuu Homerun Kyousou
Jaleco, 1988

PCB Layout
----------

HR-8847
-----------------------------------
| YM2203    Z80B         6264     |
|YM3014 DSW(8)     HOMERUN.43     |
|    D7756C   6264                |
|                                 |
|J  640KhZ   HOMERUN.60           |
|A 2018                           |
|M      2018    2018          8255|
|M          2018                  |
|A                                |
|                                 |
|                                 |
| HOMERUN.120                20MHz|
-----------------------------------

Notes:
      Z80 clock: 5.000MHz
          VSync: 60Hz
          HSync: 15.21kHz
*/

ROM_START( homerun )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "homerun.43",        0x0000, 0x4000, CRC(e759e476) SHA1(ad4f356ff26209033320a3e6353e4d4d9beb59c1) )
	ROM_CONTINUE(        0x10000,0x1c000)

	ROM_REGION( 0x010000, "gfx1", 0 )
	ROM_LOAD( "homerun.60",  0x00000, 0x10000, CRC(69a720d1) SHA1(0f0a4877578f358e9e829ece8c31e23f01adcf83) )

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "homerun.120",  0x00000, 0x20000, CRC(52f0709b) SHA1(19e675bcccadb774f60ec5929fc1fb5cf0d3f617) )

	ROM_REGION( 0x01000, "d7756c", 0 )
	ROM_LOAD( "homerun.snd",  0x00000, 0x1000, NO_DUMP ) /* D7756C internal rom */

ROM_END

ROM_START( dynashot )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "1.ic43",        0x0000, 0x4000, CRC(bf3c9586) SHA1(439effbda305f5fa265e5897c81dc1447e5d867d) )
	ROM_CONTINUE(        0x10000,0x1c000)

	ROM_REGION( 0x010000, "gfx1", 0 )
	ROM_LOAD( "3.ic60",  0x00000, 0x10000, CRC(77d6a608) SHA1(a31ff343a5d4d6f20301c030ecc2e252149bcf9d) )

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "2.ic120",  0x00000, 0x20000, CRC(bedf7b98) SHA1(cb6c5fcaf8df5f5c7636c3c8f79b9dda78e30c2e) )

	ROM_REGION( 0x01000, "d7756c", 0 )
	ROM_LOAD( "dynashot.snd",  0x00000, 0x1000, NO_DUMP ) /* D7756C internal rom */

ROM_END


GAME( 1988, homerun,  0, homerun, homerun,  0, ROT0, "Jaleco", "Moero Pro Yakyuu Homerun", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1988, dynashot, 0, homerun, dynashot, 0, ROT0, "Jaleco", "Dynamic Shooting",         GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
