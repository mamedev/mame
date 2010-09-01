/***************************************************************************

    Double Dribble (GX690) (c) Konami 1986

    Driver by Manuel Abadia <manu@teleline.es>

    2008-08
    Dip locations and suggested settings verified with US manual.

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "sound/2203intf.h"
#include "sound/vlm5030.h"
#include "sound/flt_rc.h"
#include "includes/konamipt.h"
#include "includes/ddribble.h"


static INTERRUPT_GEN( ddribble_interrupt_0 )
{
	ddribble_state *state = device->machine->driver_data<ddribble_state>();
	if (state->int_enable_0)
		cpu_set_input_line(device, M6809_FIRQ_LINE, HOLD_LINE);
}

static INTERRUPT_GEN( ddribble_interrupt_1 )
{
	ddribble_state *state = device->machine->driver_data<ddribble_state>();
	if (state->int_enable_1)
		cpu_set_input_line(device, M6809_FIRQ_LINE, HOLD_LINE);
}


static WRITE8_HANDLER( ddribble_bankswitch_w )
{
	memory_set_bank(space->machine, "bank1", data & 0x0f);
}


static READ8_HANDLER( ddribble_sharedram_r )
{
	ddribble_state *state = space->machine->driver_data<ddribble_state>();
	return state->sharedram[offset];
}

static WRITE8_HANDLER( ddribble_sharedram_w )
{
	ddribble_state *state = space->machine->driver_data<ddribble_state>();
	state->sharedram[offset] = data;
}

static READ8_HANDLER( ddribble_snd_sharedram_r )
{
	ddribble_state *state = space->machine->driver_data<ddribble_state>();
	return state->snd_sharedram[offset];
}

static WRITE8_HANDLER( ddribble_snd_sharedram_w )
{
	ddribble_state *state = space->machine->driver_data<ddribble_state>();
	state->snd_sharedram[offset] = data;
}

static WRITE8_HANDLER( ddribble_coin_counter_w )
{
	/* b4-b7: unused */
	/* b2-b3: unknown */
	/* b1: coin counter 2 */
	/* b0: coin counter 1 */
	coin_counter_w(space->machine, 0,(data) & 0x01);
	coin_counter_w(space->machine, 1,(data >> 1) & 0x01);
}

static READ8_DEVICE_HANDLER( ddribble_vlm5030_busy_r )
{
	return mame_rand(device->machine); /* patch */
	/* FIXME: remove ? */
#if 0
	if (vlm5030_bsy(device)) return 1;
	else return 0;
#endif
}

static WRITE8_DEVICE_HANDLER( ddribble_vlm5030_ctrl_w )
{
	ddribble_state *state = device->machine->driver_data<ddribble_state>();
	UINT8 *SPEECH_ROM = memory_region(device->machine, "vlm");

	/* b7 : vlm data bus OE   */

	/* b6 : VLM5030-RST       */
	vlm5030_rst(device, data & 0x40 ? 1 : 0);

	/* b5 : VLM5030-ST        */
	vlm5030_st(device, data & 0x20 ? 1 : 0);

	/* b4 : VLM5300-VCU       */
	vlm5030_vcu(device, data & 0x10 ? 1 : 0);

	/* b3 : ROM bank select   */
	vlm5030_set_rom(device, &SPEECH_ROM[data & 0x08 ? 0x10000 : 0]);

	/* b2 : SSG-C rc filter enable */
	filter_rc_set_RC(state->filter3, FLT_RC_LOWPASS, 1000, 2200, 1000, data & 0x04 ? CAP_N(150) : 0); /* YM2203-SSG-C */

	/* b1 : SSG-B rc filter enable */
	filter_rc_set_RC(state->filter2, FLT_RC_LOWPASS, 1000, 2200, 1000, data & 0x02 ? CAP_N(150) : 0); /* YM2203-SSG-B */

	/* b0 : SSG-A rc filter enable */
	filter_rc_set_RC(state->filter1, FLT_RC_LOWPASS, 1000, 2200, 1000, data & 0x01 ? CAP_N(150) : 0); /* YM2203-SSG-A */
}


static ADDRESS_MAP_START( cpu0_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0004) AM_WRITE(K005885_0_w)												/* video registers (005885 #1) */
	AM_RANGE(0x0800, 0x0804) AM_WRITE(K005885_1_w)												/* video registers (005885 #2) */
	AM_RANGE(0x1800, 0x187f) AM_RAM AM_BASE_MEMBER(ddribble_state, paletteram)										/* palette */
	AM_RANGE(0x2000, 0x2fff) AM_RAM_WRITE(ddribble_fg_videoram_w) AM_BASE_MEMBER(ddribble_state, fg_videoram)	/* Video RAM 1 */
	AM_RANGE(0x3000, 0x3fff) AM_RAM AM_BASE_MEMBER(ddribble_state, spriteram_1)								/* Object RAM 1 */
	AM_RANGE(0x4000, 0x5fff) AM_RAM AM_BASE_MEMBER(ddribble_state, sharedram)									/* shared RAM with CPU #1 */
	AM_RANGE(0x6000, 0x6fff) AM_RAM_WRITE(ddribble_bg_videoram_w) AM_BASE_MEMBER(ddribble_state, bg_videoram)	/* Video RAM 2 */
	AM_RANGE(0x7000, 0x7fff) AM_RAM AM_BASE_MEMBER(ddribble_state, spriteram_2)								/* Object RAM 2 */
	AM_RANGE(0x8000, 0x8000) AM_WRITE(ddribble_bankswitch_w)										/* bankswitch control */
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank1")														/* banked ROM */
	AM_RANGE(0xa000, 0xffff) AM_ROM																/* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu1_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_READWRITE(ddribble_sharedram_r, ddribble_sharedram_w)			/* shared RAM with CPU #0 */
	AM_RANGE(0x2000, 0x27ff) AM_READWRITE(ddribble_snd_sharedram_r, ddribble_snd_sharedram_w)	/* shared RAM with CPU #2 */
	AM_RANGE(0x2800, 0x2800) AM_READ_PORT("DSW1")
	AM_RANGE(0x2801, 0x2801) AM_READ_PORT("P1")
	AM_RANGE(0x2802, 0x2802) AM_READ_PORT("P2")
	AM_RANGE(0x2803, 0x2803) AM_READ_PORT("SYSTEM")											/* coinsw & start */
	AM_RANGE(0x2c00, 0x2c00) AM_READ_PORT("DSW2")
	AM_RANGE(0x3000, 0x3000) AM_READ_PORT("DSW3")
	AM_RANGE(0x3400, 0x3400) AM_WRITE(ddribble_coin_counter_w)								/* coin counters */
	AM_RANGE(0x3c00, 0x3c00) AM_WRITE(watchdog_reset_w)										/* watchdog reset */
	AM_RANGE(0x8000, 0xffff) AM_ROM															/* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu2_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_BASE_MEMBER(ddribble_state, snd_sharedram)		/* shared RAM with CPU #1 */
	AM_RANGE(0x1000, 0x1001) AM_DEVREADWRITE("ymsnd", ym2203_r, ym2203_w)	/* YM2203 */
	AM_RANGE(0x3000, 0x3000) AM_DEVWRITE("vlm", vlm5030_data_w)			/* Speech data */
	AM_RANGE(0x8000, 0xffff) AM_ROM										/* ROM */
ADDRESS_MAP_END

static INPUT_PORTS_START( ddribble )
	PORT_START("P1")
	KONAMI8_B132(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	KONAMI8_B132(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("DSW1")
	KONAMI_COINAGE_ALT_LOC(SW1)

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "SW2:1" )	/* Manual says it's Unused */
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "SW2:2" )	/* Manual says it's Unused */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x04, DEF_STR( Cocktail ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW2:4" )	/* Manual says it's Unused */
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW2:5" )	/* Manual says it's Unused */
	PORT_DIPNAME( 0x60, 0x20, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(	0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(	0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "SW3:2" )	/* Manual says it's Unused */
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPNAME( 0x08, 0x08, "Allow vs match with 1 Credit" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
			32*8+0*4, 32*8+1*4, 32*8+2*4, 32*8+3*4, 32*8+4*4, 32*8+5*4, 32*8+6*4, 32*8+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	32*32
};

static GFXDECODE_START( ddribble )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout,    48,  1 )	/* colors 48-63 */
	GFXDECODE_ENTRY( "gfx2", 0x00000, charlayout,    16,  1 )	/* colors 16-31 */
	GFXDECODE_ENTRY( "gfx1", 0x20000, spritelayout,  32,  1 )	/* colors 32-47 */
	GFXDECODE_ENTRY( "gfx2", 0x40000, spritelayout,  64, 16 )	/* colors  0-15 but using lookup table */
GFXDECODE_END

static const ym2203_interface ym2203_config =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_NULL,
		DEVCB_DEVICE_HANDLER("vlm", ddribble_vlm5030_busy_r),
		DEVCB_DEVICE_HANDLER("vlm", ddribble_vlm5030_ctrl_w),
		DEVCB_NULL
	},
	NULL
};

static const vlm5030_interface vlm5030_config =
{
	0x10000     /* memory size 64Kbyte * 2 bank */
};



static MACHINE_START( ddribble )
{
	ddribble_state *state = machine->driver_data<ddribble_state>();
	UINT8 *ROM = memory_region(machine, "maincpu");
	memory_configure_bank(machine, "bank1", 0, 5, &ROM[0x10000], 0x2000);

	state->filter1 = machine->device("filter1");
	state->filter2 = machine->device("filter2");
	state->filter3 = machine->device("filter3");

	state_save_register_global(machine, state->int_enable_0);
	state_save_register_global(machine, state->int_enable_1);
	state_save_register_global_array(machine, state->vregs[0]);
	state_save_register_global_array(machine, state->vregs[1]);
	state_save_register_global_array(machine, state->charbank);
}

static MACHINE_RESET( ddribble )
{
	ddribble_state *state = machine->driver_data<ddribble_state>();
	int i;

	for (i = 0; i < 5; i++)
	{
		state->vregs[0][i] = 0;
		state->vregs[1][i] = 0;
	}

	state->int_enable_0 = 0;
	state->int_enable_1 = 0;
	state->charbank[0] = 0;
	state->charbank[1] = 0;
}

static MACHINE_CONFIG_START( ddribble, ddribble_state )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6809,	XTAL_18_432MHz/12)	/* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(cpu0_map)
	MDRV_CPU_VBLANK_INT("screen", ddribble_interrupt_0)

	MDRV_CPU_ADD("cpu1", M6809,	XTAL_18_432MHz/12)	/* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(cpu1_map)
	MDRV_CPU_VBLANK_INT("screen", ddribble_interrupt_1)

	MDRV_CPU_ADD("cpu2", M6809,	XTAL_18_432MHz/12)	/* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(cpu2_map)

	MDRV_QUANTUM_TIME(HZ(6000))	/* we need heavy synch */

	MDRV_MACHINE_START(ddribble)
	MDRV_MACHINE_RESET(ddribble)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
/*  MDRV_SCREEN_SIZE(64*8, 32*8)
    MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1) */

	MDRV_GFXDECODE(ddribble)
	MDRV_PALETTE_LENGTH(64 + 256)

	MDRV_PALETTE_INIT(ddribble)
	MDRV_VIDEO_START(ddribble)
	MDRV_VIDEO_UPDATE(ddribble)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2203, XTAL_3_579545MHz) /* verified on pcb */
	MDRV_SOUND_CONFIG(ym2203_config)
	MDRV_SOUND_ROUTE(0, "filter1", 0.25)
	MDRV_SOUND_ROUTE(1, "filter2", 0.25)
	MDRV_SOUND_ROUTE(2, "filter3", 0.25)
	MDRV_SOUND_ROUTE(3, "mono", 0.25)

	MDRV_SOUND_ADD("vlm", VLM5030, XTAL_3_579545MHz) /* verified on pcb */
	MDRV_SOUND_CONFIG(vlm5030_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("filter1", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD("filter2", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD("filter3", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


ROM_START( ddribble )
	ROM_REGION( 0x1a000, "maincpu", 0 ) /* 64K CPU #0 + 40K for Banked ROMS */
	ROM_LOAD( "690c03.bin",	0x10000, 0x0a000, CRC(07975a58) SHA1(96fd1b2348bbdf560067d8ee3cd4c0514e263d7a) )
	ROM_CONTINUE(			0x0a000, 0x06000 )

	ROM_REGION( 0x10000, "cpu1", 0 ) /* 64 for the CPU #1 */
	ROM_LOAD( "690c02.bin", 0x08000, 0x08000, CRC(f07c030a) SHA1(db96a10f8bb657bf285266db9e775fa6af82f38c) )

	ROM_REGION( 0x10000, "cpu2", 0 )	/* 64k for the SOUND CPU */
	ROM_LOAD( "690b01.bin", 0x08000, 0x08000, CRC(806b8453) SHA1(3184772c5e5181438a17ac72129070bf164b2965) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "690a05.bin",	0x00000, 0x20000, CRC(6a816d0d) SHA1(73f2527d5f2b9d51b784be36e07e0d0c566a28d9) )	/* characters & objects */
	ROM_LOAD16_BYTE( "690a06.bin",	0x00001, 0x20000, CRC(46300cd0) SHA1(07197a546fff452a41575fcd481da64ac6bf601e) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "690a10.bin", 0x00000, 0x20000, CRC(61efa222) SHA1(bd7b993ad1c06d8f6ac29fbc07c4a987abe1ab42) )	/* characters */
	ROM_LOAD16_BYTE( "690a09.bin", 0x00001, 0x20000, CRC(ab682186) SHA1(a28982835042a07354557e1539b097cdf93fc466) )
	ROM_LOAD16_BYTE( "690a08.bin", 0x40000, 0x20000, CRC(9a889944) SHA1(ca96815aefb1e336bd2288841b00a5c21cacf90f) )	/* objects */
	ROM_LOAD16_BYTE( "690a07.bin", 0x40001, 0x20000, CRC(faf81b3f) SHA1(0bd647b4cdd3f2209472e303fd22eedd5533d1b1) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "690a11.i15", 0x0000, 0x0100, CRC(f34617ad) SHA1(79ceba6fe204472a5a659641ac4f14bb1f0ee3f6) )	/* sprite lookup table */

	ROM_REGION( 0x20000, "vlm", 0 )	/* 128k for the VLM5030 data */
	ROM_LOAD( "690a04.bin", 0x00000, 0x20000, CRC(1bfeb763) SHA1(f3e9acb2a7a9b4c8dee6838c1344a7a65c27ff77) )

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pal10l8-007553.bin", 0x0000, 0x002c, CRC(0ae5a161) SHA1(87571addf434b332019ea0e22372eb24b4fd0197) )
ROM_END


GAME( 1986, ddribble, 0, ddribble, ddribble, 0, ROT0, "Konami", "Double Dribble", GAME_SUPPORTS_SAVE )
