/***************************************************************************

Atari Boxer (prototype) driver

  AKA Boxing, both game titles appear in the schematics

  This game had some weird controls that don't work well in MAME.

***************************************************************************/

#include "driver.h"
#include "cpu/m6502/m6502.h"

#define MASTER_CLOCK XTAL_12_096MHz

extern UINT8* boxer_tile_ram;
extern UINT8* boxer_sprite_ram;

extern VIDEO_UPDATE( boxer );

static UINT8 pot_state;
static UINT8 pot_latch;


static TIMER_CALLBACK( pot_interrupt )
{
	int mask = param;

	if (pot_latch & mask)
		cputag_set_input_line(machine, "maincpu", INPUT_LINE_NMI, ASSERT_LINE);

	pot_state |= mask;
}


static TIMER_CALLBACK( periodic_callback )
{
	int scanline = param;

	cputag_set_input_line(machine, "maincpu", 0, ASSERT_LINE);

	if (scanline == 0)
	{
		UINT8 mask[256];

		int i;

		memset(mask, 0, sizeof mask);

		mask[input_port_read(machine, "STICK0_X")] |= 0x01;
		mask[input_port_read(machine, "STICK0_Y")] |= 0x02;
		mask[input_port_read(machine, "PADDLE0")]  |= 0x04;
		mask[input_port_read(machine, "STICK1_X")] |= 0x08;
		mask[input_port_read(machine, "STICK1_Y")] |= 0x10;
		mask[input_port_read(machine, "PADDLE1")]  |= 0x20;

		for (i = 1; i < 256; i++)
			if (mask[i] != 0)
				timer_set(machine, video_screen_get_time_until_pos(machine->primary_screen, i, 0), NULL, mask[i], pot_interrupt);

		pot_state = 0;
	}

	scanline += 64;

	if (scanline >= 262)
		scanline = 0;

	timer_set(machine, video_screen_get_time_until_pos(machine->primary_screen, scanline, 0), NULL, scanline, periodic_callback);
}


static PALETTE_INIT( boxer )
{
	palette_set_color(machine,0, MAKE_RGB(0x00,0x00,0x00));
	palette_set_color(machine,1, MAKE_RGB(0xff,0xff,0xff));

	palette_set_color(machine,2, MAKE_RGB(0xff,0xff,0xff));
	palette_set_color(machine,3, MAKE_RGB(0x00,0x00,0x00));
}


static MACHINE_RESET( boxer )
{
	timer_set(machine, video_screen_get_time_until_pos(machine->primary_screen, 0, 0), NULL, 0, periodic_callback);

	pot_latch = 0;
}


static READ8_HANDLER( boxer_input_r )
{
	UINT8 val = input_port_read(space->machine, "IN0");

	if (input_port_read(space->machine, "IN3") < video_screen_get_vpos(space->machine->primary_screen))
		val |= 0x02;

	return (val << ((offset & 7) ^ 7)) & 0x80;
}


static READ8_HANDLER( boxer_misc_r )
{
	UINT8 val = 0;

	switch (offset & 3)
	{
	case 0:
		val = pot_state & pot_latch;
		break;

	case 1:
		val = video_screen_get_vpos(space->machine->primary_screen);
		break;

	case 2:
		val = input_port_read(space->machine, "IN1");
		break;

	case 3:
		val = input_port_read(space->machine, "IN2");
		break;
	}

	return val ^ 0x3f;
}




static WRITE8_HANDLER( boxer_bell_w )
{
}


static WRITE8_HANDLER( boxer_sound_w )
{
}


static WRITE8_HANDLER( boxer_pot_w )
{
	/* BIT0 => HPOT1 */
	/* BIT1 => VPOT1 */
	/* BIT2 => RPOT1 */
	/* BIT3 => HPOT2 */
	/* BIT4 => VPOT2 */
	/* BIT5 => RPOT2 */

	pot_latch = data & 0x3f;

	cputag_set_input_line(space->machine, "maincpu", INPUT_LINE_NMI, CLEAR_LINE);
}


static WRITE8_HANDLER( boxer_irq_reset_w )
{
	cputag_set_input_line(space->machine, "maincpu", 0, CLEAR_LINE);
}


static WRITE8_HANDLER( boxer_crowd_w )
{
	/* BIT0 => ATTRACT */
	/* BIT1 => CROWD-1 */
	/* BIT2 => CROWD-2 */
	/* BIT3 => CROWD-3 */

	coin_lockout_global_w(data & 1);
}


static WRITE8_HANDLER( boxer_led_w )
{
	set_led_status(1, !(data & 1));
	set_led_status(0, !(data & 2));
}




static ADDRESS_MAP_START( boxer_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x0200, 0x03ff) AM_RAM AM_BASE(&boxer_tile_ram)
	AM_RANGE(0x0800, 0x08ff) AM_READ(boxer_input_r)
	AM_RANGE(0x1000, 0x17ff) AM_READ(boxer_misc_r)
	AM_RANGE(0x1800, 0x1800) AM_WRITE(boxer_pot_w)
	AM_RANGE(0x1900, 0x19ff) AM_WRITE(boxer_led_w)
	AM_RANGE(0x1a00, 0x1aff) AM_WRITE(boxer_sound_w)
	AM_RANGE(0x1b00, 0x1bff) AM_WRITE(boxer_crowd_w)
	AM_RANGE(0x1c00, 0x1cff) AM_WRITE(boxer_irq_reset_w)
	AM_RANGE(0x1d00, 0x1dff) AM_WRITE(boxer_bell_w)
	AM_RANGE(0x1e00, 0x1eff) AM_WRITEONLY AM_BASE(&boxer_sprite_ram)
	AM_RANGE(0x1f00, 0x1fff) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x3000, 0x3fff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( boxer )

	PORT_START("IN0")
	PORT_BIT ( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT ( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) /* TIMER */
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT ( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT ( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x01, "Number of Rounds" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )

	PORT_START("STICK0_X")
	PORT_BIT ( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(30) PORT_KEYDELTA(16) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("STICK0_Y")
	PORT_BIT ( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(30) PORT_KEYDELTA(16) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("PADDLE0")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(30) PORT_KEYDELTA(16) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_CENTERDELTA(0) PORT_PLAYER(1)

	PORT_START("STICK1_X")
	PORT_BIT ( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(30) PORT_KEYDELTA(16) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("STICK1_Y")
	PORT_BIT ( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(30) PORT_KEYDELTA(16) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("PADDLE1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(30) PORT_KEYDELTA(16) PORT_CODE_DEC(KEYCODE_Q) PORT_CODE_INC(KEYCODE_W) PORT_CENTERDELTA(0) PORT_PLAYER(2)

	PORT_START("IN3")
	PORT_DIPNAME( 0xff, 0x5C, "Round Time" ) /* actually a potentiometer */
	PORT_DIPSETTING(    0x3C, "15 seconds" )
	PORT_DIPSETTING(    0x5C, "30 seconds" )
	PORT_DIPSETTING(    0x7C, "45 seconds" )

INPUT_PORTS_END


static const gfx_layout tile_layout =
{
	8, 8,
	64,
	1,
	{ 0 },
	{
		0x7, 0x6, 0x5, 0x4, 0xf, 0xe, 0xd, 0xc
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
	},
	0x80
};


static const gfx_layout sprite_layout =
{
	8, 8,
	64,
	1,
	{ 0 },
	{
		0x4, 0x5, 0x6, 0x7, 0xc, 0xd, 0xe, 0xf
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
	},
	0x80
};


static GFXDECODE_START( boxer )
	GFXDECODE_ENTRY( "gfx1", 0, sprite_layout, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, sprite_layout, 0, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, tile_layout, 2, 1 )
GFXDECODE_END


static MACHINE_DRIVER_START(boxer)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6502, MASTER_CLOCK / 16)
	MDRV_CPU_PROGRAM_MAP(boxer_map)

	/* video hardware */
	MDRV_MACHINE_RESET(boxer)


	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 262)
	MDRV_SCREEN_VISIBLE_AREA(8, 247, 0, 239)

	MDRV_GFXDECODE(boxer)
	MDRV_PALETTE_LENGTH(4)
	MDRV_PALETTE_INIT(boxer)
	MDRV_VIDEO_UPDATE(boxer)

	/* sound hardware */
MACHINE_DRIVER_END


ROM_START( boxer )

	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD_NIB_LOW ( "3400l.e1", 0x3400, 0x0400, CRC(df85afa4) SHA1(5a74a08f1e0b0bbec02999d5e46513d8afd333ac) )
	ROM_LOAD_NIB_HIGH( "3400m.a1", 0x3400, 0x0400, CRC(23fe06aa) SHA1(03a4eedbf60f07d1dd8d7af576828df5f032146e) )
	ROM_LOAD_NIB_LOW ( "3800l.j1", 0x3800, 0x0400, CRC(087263fb) SHA1(cc3715a68bd05f23b4abf9f18ca14a8fe55163f7) )
	ROM_LOAD_NIB_HIGH( "3800m.d1", 0x3800, 0x0400, CRC(3bbf605e) SHA1(be4ff1702eb837710421a7dafcdc60fe2d3259e8) )
	ROM_LOAD_NIB_LOW ( "3c00l.h1", 0x3C00, 0x0400, CRC(09e204f2) SHA1(565d4c8865da7d96a45e909973d570101de61f63) )
	ROM_LOAD_NIB_HIGH( "3c00m.c1", 0x3C00, 0x0400, CRC(2f8ebc85) SHA1(05a4e29ec7e49173200d5fe5344274fd6afd16d7) )

	ROM_REGION( 0x0400, "gfx1", ROMREGION_DISPOSE ) /* lower boxer */
	ROM_LOAD( "bx137l.c8", 0x0000, 0x0400, CRC(e91f2048) SHA1(64039d07557e210aa4f6663cd7e72814cb881310) )

	ROM_REGION( 0x0400, "gfx2", ROMREGION_DISPOSE ) /* upper boxer */
	ROM_LOAD( "bx137u.m8", 0x0000, 0x0400, CRC(e4fee386) SHA1(79b70aca4a92c56363689a363b643d46294d3e88) )

	ROM_REGION( 0x0400, "gfx3", ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "9417.k2", 0x0000, 0x0400, CRC(7e3d22cf) SHA1(92e6bbe049dc8fcd674f2ff96cde3786f714508d) )

	ROM_REGION( 0x0200, "user1", 0 ) /* lower boxer map */
	ROM_LOAD( "bx115l.b7", 0x0000, 0x0200, CRC(31f2234f) SHA1(d53f3a1d0db3cf3024de61ef64f76c6dfdf6861c) )

	ROM_REGION( 0x0200, "user2", 0 ) /* upper boxer map */
	ROM_LOAD( "bx115u.l7", 0x0000, 0x0200, CRC(124d3f24) SHA1(09fab2ae218b8584c0e3c8e02f5680ce083a33d6) )

	ROM_REGION( 0x0100, "proms", 0 ) /* sync prom */
	ROM_LOAD( "9402.m3", 0x0000, 0x0100, CRC(00e224a0) SHA1(1a384ef488791c62566c91b18d6a1fb4a5def2ba) )
ROM_END


GAME( 1978, boxer, 0, boxer, boxer, 0, 0, "Atari", "Boxer (prototype)", GAME_NO_SOUND )
