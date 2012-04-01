/***************************************************************************

    Rollergames (GX999) (c) 1991 Konami

    driver by Nicola Salmoria


    2009-03:
    Added dsw locations and verified factory settings based on Guru's notes

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/konicdev.h"
#include "machine/k053252.h"
#include "cpu/konami/konami.h" /* for the callback and the firq irq definition */
#include "sound/3812intf.h"
#include "sound/k053260.h"
#include "includes/rollerg.h"

/* prototypes */
static KONAMI_SETLINES_CALLBACK( rollerg_banking );

static WRITE8_HANDLER( rollerg_0010_w )
{
	rollerg_state *state = space->machine().driver_data<rollerg_state>();
	logerror("%04x: write %02x to 0010\n",cpu_get_pc(&space->device()), data);

	/* bits 0/1 are coin counters */
	coin_counter_w(space->machine(), 0, data & 0x01);
	coin_counter_w(space->machine(), 1, data & 0x02);

	/* bit 2 enables 051316 ROM reading */
	state->m_readzoomroms = data & 0x04;

	/* bit 5 enables 051316 wraparound */
	k051316_wraparound_enable(state->m_k051316, data & 0x20);

	/* other bits unknown */
}

static READ8_HANDLER( rollerg_k051316_r )
{
	rollerg_state *state = space->machine().driver_data<rollerg_state>();

	if (state->m_readzoomroms)
		return k051316_rom_r(state->m_k051316, offset);
	else
		return k051316_r(state->m_k051316, offset);
}

static READ8_DEVICE_HANDLER( rollerg_sound_r )
{
	/* If the sound CPU is running, read the status, otherwise
       just make it pass the test */
	return k053260_r(device, 2 + offset);
}

static WRITE8_HANDLER( soundirq_w )
{
	rollerg_state *state = space->machine().driver_data<rollerg_state>();
	device_set_input_line_and_vector(state->m_audiocpu, 0, HOLD_LINE, 0xff);
}

static TIMER_CALLBACK( nmi_callback )
{
	rollerg_state *state = machine.driver_data<rollerg_state>();
	device_set_input_line(state->m_audiocpu, INPUT_LINE_NMI, ASSERT_LINE);
}

static WRITE8_HANDLER( sound_arm_nmi_w )
{
	rollerg_state *state = space->machine().driver_data<rollerg_state>();
	device_set_input_line(state->m_audiocpu, INPUT_LINE_NMI, CLEAR_LINE);
	space->machine().scheduler().timer_set(attotime::from_usec(50), FUNC(nmi_callback));	/* kludge until the K053260 is emulated correctly */
}

static READ8_HANDLER( pip_r )
{
	return 0x7f;
}

static ADDRESS_MAP_START( rollerg_map, AS_PROGRAM, 8, rollerg_state )
	AM_RANGE(0x0010, 0x0010) AM_WRITE_LEGACY(rollerg_0010_w)
	AM_RANGE(0x0020, 0x0020) AM_READWRITE_LEGACY(watchdog_reset_r,watchdog_reset_w)
	AM_RANGE(0x0030, 0x0031) AM_DEVREADWRITE_LEGACY("k053260", rollerg_sound_r, k053260_w)	/* K053260 */
	AM_RANGE(0x0040, 0x0040) AM_WRITE_LEGACY(soundirq_w)
	AM_RANGE(0x0050, 0x0050) AM_READ_PORT("P1")
	AM_RANGE(0x0051, 0x0051) AM_READ_PORT("P2")
	AM_RANGE(0x0052, 0x0052) AM_READ_PORT("DSW3")
	AM_RANGE(0x0053, 0x0053) AM_READ_PORT("DSW1")
	AM_RANGE(0x0060, 0x0060) AM_READ_PORT("DSW2")
	AM_RANGE(0x0061, 0x0061) AM_READ_LEGACY(pip_r)				/* ????? */
	AM_RANGE(0x0100, 0x010f) AM_DEVREADWRITE_LEGACY("k053252",k053252_r,k053252_w)		/* 053252? */
	AM_RANGE(0x0200, 0x020f) AM_DEVWRITE_LEGACY("k051316", k051316_ctrl_w)
	AM_RANGE(0x0300, 0x030f) AM_DEVREADWRITE_LEGACY("k053244", k053244_r, k053244_w)
	AM_RANGE(0x0800, 0x0fff) AM_READ_LEGACY(rollerg_k051316_r) AM_DEVWRITE_LEGACY("k051316", k051316_w)
	AM_RANGE(0x1000, 0x17ff) AM_DEVREADWRITE_LEGACY("k053244", k053245_r, k053245_w)
	AM_RANGE(0x1800, 0x1fff) AM_RAM_WRITE_LEGACY(paletteram_xBBBBBGGGGGRRRRR_be_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x2000, 0x3aff) AM_RAM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( rollerg_sound_map, AS_PROGRAM, 8, rollerg_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xa000, 0xa02f) AM_DEVREADWRITE_LEGACY("k053260", k053260_r,k053260_w)
	AM_RANGE(0xc000, 0xc001) AM_DEVREADWRITE_LEGACY("ymsnd", ym3812_r,ym3812_w)
	AM_RANGE(0xfc00, 0xfc00) AM_WRITE_LEGACY(sound_arm_nmi_w)
ADDRESS_MAP_END


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( rollerg )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )		PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "No Credits" )
	/* No Credits = both coin slots open, but no effect on coin counters */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:3" )			/* Manual says it's unused */
	PORT_DIPNAME( 0x18, 0x10, "Bonus Energy" )			PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x00, "1/2 for Stage Winner" )
	PORT_DIPSETTING(    0x08, "1/4 for Stage Winner" )
	PORT_DIPSETTING(    0x10, "1/4 for Cycle Winner" )
	PORT_DIPSETTING(    0x18, DEF_STR( None ) )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:2" )			/* Manual says it's unused */
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:4" )			/* Manual says it's unused */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )
INPUT_PORTS_END



/***************************************************************************

    Machine Driver

***************************************************************************/

static const k05324x_interface rollerg_k05324x_intf =
{
	"gfx1", 0,
	NORMAL_PLANE_ORDER,
	-3, -1,
	KONAMI_ROM_DEINTERLEAVE_2,
	rollerg_sprite_callback
};

static const k051316_interface rollerg_k051316_intf =
{
	"gfx2", 1,
	4, FALSE, 0,
	0, 22, 1,
	rollerg_zoom_callback
};

static WRITE_LINE_DEVICE_HANDLER( rollerg_irq_ack_w )
{
	cputag_set_input_line(device->machine(), "maincpu", 0, CLEAR_LINE);
}

static const k053252_interface rollerg_k053252_intf =
{
	"screen",
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_LINE(rollerg_irq_ack_w),
	DEVCB_NULL,
	14*8, 2*8
};

static MACHINE_START( rollerg )
{
	rollerg_state *state = machine.driver_data<rollerg_state>();
	UINT8 *ROM = machine.region("maincpu")->base();

	memory_configure_bank(machine, "bank1", 0, 6, &ROM[0x10000], 0x4000);
	memory_configure_bank(machine, "bank1", 6, 2, &ROM[0x10000], 0x4000);
	memory_set_bank(machine, "bank1", 0);

	state->m_maincpu = machine.device("maincpu");
	state->m_audiocpu = machine.device("audiocpu");
	state->m_k053244 = machine.device("k053244");
	state->m_k051316 = machine.device("k051316");
	state->m_k053260 = machine.device("k053260");

	state->save_item(NAME(state->m_readzoomroms));
}

static MACHINE_RESET( rollerg )
{
	rollerg_state *state = machine.driver_data<rollerg_state>();

	konami_configure_set_lines(machine.device("maincpu"), rollerg_banking);

	state->m_readzoomroms = 0;
}

static MACHINE_CONFIG_START( rollerg, rollerg_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", KONAMI, 3000000)		/* ? */
	MCFG_CPU_PROGRAM_MAP(rollerg_map)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_assert)

	MCFG_CPU_ADD("audiocpu", Z80, 3579545)
	MCFG_CPU_PROGRAM_MAP(rollerg_sound_map)
								/* NMIs are generated by the 053260 */
	MCFG_MACHINE_START(rollerg)
	MCFG_MACHINE_RESET(rollerg)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_UPDATE_STATIC(rollerg)

	MCFG_PALETTE_LENGTH(1024)

	MCFG_VIDEO_START(rollerg)

	MCFG_K053244_ADD("k053244", rollerg_k05324x_intf)
	MCFG_K051316_ADD("k051316", rollerg_k051316_intf)
	MCFG_K053252_ADD("k053252", 3000000*2, rollerg_k053252_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, 3579545)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("k053260", K053260, 3579545)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)
MACHINE_CONFIG_END



/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( rollerg )
	ROM_REGION( 0x28000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "999m02.g7",  0x10000, 0x18000, CRC(3df8db93) SHA1(10c46d53d11b12b8f7cc6417601baef4638c1efe) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "999m01.e11", 0x0000, 0x8000, CRC(1fcfb22f) SHA1(ef058a7de6ba7cf310b91975345113acc6078f8a) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "999h06.k2",  0x000000, 0x100000, CRC(eda05130) SHA1(b52073a4a4651035d5f1e112601ceb2d004b2143) ) /* sprites */
	ROM_LOAD( "999h05.k8",  0x100000, 0x100000, CRC(5f321c7d) SHA1(d60a3480891b83ac109f2fecfe2b958bac310c15) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "999h03.d23", 0x000000, 0x040000, CRC(ea1edbd2) SHA1(a17d19f873384287e1e47222d46274e7408b40d4) ) /* zoom */
	ROM_LOAD( "999h04.f23", 0x040000, 0x040000, CRC(c1a35355) SHA1(615606d30500a8f2be19171893e985b085fff2fc) )

	ROM_REGION( 0x80000, "k053260", 0 )	/* samples for 053260 */
	ROM_LOAD( "999h09.c5",  0x000000, 0x080000, CRC(c5188783) SHA1(d9ab69e4197ba2b42e3b0bb713236c8037fc2ab3) )
ROM_END

ROM_START( rollergj )
	ROM_REGION( 0x28000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "999v02.bin", 0x10000, 0x18000, CRC(0dd8c3ac) SHA1(4c3d5514dec317c6640ceaaa06411766632f4412) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "999m01.e11", 0x0000, 0x8000, CRC(1fcfb22f) SHA1(ef058a7de6ba7cf310b91975345113acc6078f8a) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "999h06.k2",  0x000000, 0x100000, CRC(eda05130) SHA1(b52073a4a4651035d5f1e112601ceb2d004b2143) ) /* sprites */
	ROM_LOAD( "999h05.k8",  0x100000, 0x100000, CRC(5f321c7d) SHA1(d60a3480891b83ac109f2fecfe2b958bac310c15) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "999h03.d23", 0x000000, 0x040000, CRC(ea1edbd2) SHA1(a17d19f873384287e1e47222d46274e7408b40d4) ) /* zoom */
	ROM_LOAD( "999h04.f23", 0x040000, 0x040000, CRC(c1a35355) SHA1(615606d30500a8f2be19171893e985b085fff2fc) )

	ROM_REGION( 0x80000, "k053260", 0 )	/* samples for 053260 */
	ROM_LOAD( "999h09.c5",  0x000000, 0x080000, CRC(c5188783) SHA1(d9ab69e4197ba2b42e3b0bb713236c8037fc2ab3) )
ROM_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

static KONAMI_SETLINES_CALLBACK( rollerg_banking )
{
	memory_set_bank(device->machine(), "bank1", lines & 0x07);
}


GAME( 1991, rollerg,  0,       rollerg, rollerg, 0, ROT0, "Konami", "Rollergames (US)", GAME_SUPPORTS_SAVE )
GAME( 1991, rollergj, rollerg, rollerg, rollerg, 0, ROT0, "Konami", "Rollergames (Japan)", GAME_SUPPORTS_SAVE )
