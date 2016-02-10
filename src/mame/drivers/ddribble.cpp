// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

    Double Dribble (GX690) (c) Konami 1986

    Driver by Manuel Abadia <emumanu+mame@gmail.com>

    2008-08
    Dip locations and suggested settings verified with US manual.

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "sound/2203intf.h"
#include "includes/konamipt.h"
#include "includes/ddribble.h"


INTERRUPT_GEN_MEMBER(ddribble_state::ddribble_interrupt_0)
{
	if (m_int_enable_0)
		device.execute().set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(ddribble_state::ddribble_interrupt_1)
{
	if (m_int_enable_1)
		device.execute().set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
}


WRITE8_MEMBER(ddribble_state::ddribble_bankswitch_w)
{
	membank("bank1")->set_entry(data & 0x07);
}


READ8_MEMBER(ddribble_state::ddribble_sharedram_r)
{
	return m_sharedram[offset];
}

WRITE8_MEMBER(ddribble_state::ddribble_sharedram_w)
{
	m_sharedram[offset] = data;
}

READ8_MEMBER(ddribble_state::ddribble_snd_sharedram_r)
{
	return m_snd_sharedram[offset];
}

WRITE8_MEMBER(ddribble_state::ddribble_snd_sharedram_w)
{
	m_snd_sharedram[offset] = data;
}

WRITE8_MEMBER(ddribble_state::ddribble_coin_counter_w)
{
	/* b4-b7: unused */
	/* b2-b3: unknown */
	/* b1: coin counter 2 */
	/* b0: coin counter 1 */
	machine().bookkeeping().coin_counter_w(0,(data) & 0x01);
	machine().bookkeeping().coin_counter_w(1,(data >> 1) & 0x01);
}

READ8_MEMBER(ddribble_state::ddribble_vlm5030_busy_r)
{
	return machine().rand(); /* patch */
	/* FIXME: remove ? */
#if 0
	if (m_vlm->bsy()) return 1;
	else return 0;
#endif
}

WRITE8_MEMBER(ddribble_state::ddribble_vlm5030_ctrl_w)
{
	UINT8 *SPEECH_ROM = memregion("vlm")->base();

	/* b7 : vlm data bus OE   */

	/* b6 : VLM5030-RST       */
	m_vlm->rst(data & 0x40 ? 1 : 0);

	/* b5 : VLM5030-ST        */
	m_vlm->st(data & 0x20 ? 1 : 0);

	/* b4 : VLM5300-VCU       */
	m_vlm->vcu(data & 0x10 ? 1 : 0);

	/* b3 : ROM bank select   */
	m_vlm->set_rom(&SPEECH_ROM[data & 0x08 ? 0x10000 : 0]);

	/* b2 : SSG-C rc filter enable */
	m_filter3->filter_rc_set_RC(FLT_RC_LOWPASS, 1000, 2200, 1000, data & 0x04 ? CAP_N(150) : 0); /* YM2203-SSG-C */

	/* b1 : SSG-B rc filter enable */
	m_filter2->filter_rc_set_RC(FLT_RC_LOWPASS, 1000, 2200, 1000, data & 0x02 ? CAP_N(150) : 0); /* YM2203-SSG-B */

	/* b0 : SSG-A rc filter enable */
	m_filter1->filter_rc_set_RC(FLT_RC_LOWPASS, 1000, 2200, 1000, data & 0x01 ? CAP_N(150) : 0); /* YM2203-SSG-A */
}


static ADDRESS_MAP_START( cpu0_map, AS_PROGRAM, 8, ddribble_state )
	AM_RANGE(0x0000, 0x0004) AM_WRITE(K005885_0_w)                                              /* video registers (005885 #1) */
	AM_RANGE(0x0800, 0x0804) AM_WRITE(K005885_1_w)                                              /* video registers (005885 #2) */
	AM_RANGE(0x1800, 0x187f) AM_RAM_DEVWRITE("palette", palette_device, write_indirect) AM_SHARE("palette")  /* palette */
	AM_RANGE(0x2000, 0x2fff) AM_RAM_WRITE(ddribble_fg_videoram_w) AM_SHARE("fg_videoram")   /* Video RAM 1 */
	AM_RANGE(0x3000, 0x3fff) AM_RAM AM_SHARE("spriteram_1")                             /* Object RAM 1 */
	AM_RANGE(0x4000, 0x5fff) AM_RAM AM_SHARE("sharedram")                                   /* shared RAM with CPU #1 */
	AM_RANGE(0x6000, 0x6fff) AM_RAM_WRITE(ddribble_bg_videoram_w) AM_SHARE("bg_videoram")   /* Video RAM 2 */
	AM_RANGE(0x7000, 0x7fff) AM_RAM AM_SHARE("spriteram_2")                             /* Object RAM 2 */
	AM_RANGE(0x8000, 0x8000) AM_WRITE(ddribble_bankswitch_w)                                        /* bankswitch control */
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank1")                                                        /* banked ROM */
	AM_RANGE(0xa000, 0xffff) AM_ROM                                                             /* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu1_map, AS_PROGRAM, 8, ddribble_state )
	AM_RANGE(0x0000, 0x1fff) AM_READWRITE(ddribble_sharedram_r, ddribble_sharedram_w)           /* shared RAM with CPU #0 */
	AM_RANGE(0x2000, 0x27ff) AM_READWRITE(ddribble_snd_sharedram_r, ddribble_snd_sharedram_w)   /* shared RAM with CPU #2 */
	AM_RANGE(0x2800, 0x2800) AM_READ_PORT("DSW1")
	AM_RANGE(0x2801, 0x2801) AM_READ_PORT("P1")
	AM_RANGE(0x2802, 0x2802) AM_READ_PORT("P2")
	AM_RANGE(0x2803, 0x2803) AM_READ_PORT("SYSTEM")                                         /* coinsw & start */
	AM_RANGE(0x2c00, 0x2c00) AM_READ_PORT("DSW2")
	AM_RANGE(0x3000, 0x3000) AM_READ_PORT("DSW3")
	AM_RANGE(0x3400, 0x3400) AM_WRITE(ddribble_coin_counter_w)                              /* coin counters */
	AM_RANGE(0x3c00, 0x3c00) AM_WRITE(watchdog_reset_w)                                     /* watchdog reset */
	AM_RANGE(0x8000, 0xffff) AM_ROM                                                         /* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu2_map, AS_PROGRAM, 8, ddribble_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("snd_sharedram")       /* shared RAM with CPU #1 */
	AM_RANGE(0x1000, 0x1001) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)    /* YM2203 */
	AM_RANGE(0x3000, 0x3000) AM_DEVWRITE("vlm", vlm5030_device, data_w)          /* Speech data */
	AM_RANGE(0x8000, 0xffff) AM_ROM                                     /* ROM */
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
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "SW2:1" )   /* Manual says it's Unused */
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "SW2:2" )   /* Manual says it's Unused */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW2:4" )   /* Manual says it's Unused */
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW2:5" )   /* Manual says it's Unused */
	PORT_DIPNAME( 0x60, 0x20, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "SW3:2" )   /* Manual says it's Unused */
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPNAME( 0x08, 0x08, "Allow vs match with 1 Credit" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout,    48,  1 )   /* colors 48-63 */
	GFXDECODE_ENTRY( "gfx2", 0x00000, charlayout,    16,  1 )   /* colors 16-31 */
	GFXDECODE_ENTRY( "gfx1", 0x20000, spritelayout,  32,  1 )   /* colors 32-47 */
	GFXDECODE_ENTRY( "gfx2", 0x40000, spritelayout,  64, 16 )   /* colors  0-15 but using lookup table */
GFXDECODE_END


void ddribble_state::machine_start()
{
	membank("bank1")->configure_entries(0, 8, memregion("maincpu")->base(), 0x2000);

	save_item(NAME(m_int_enable_0));
	save_item(NAME(m_int_enable_1));
	save_item(NAME(m_vregs[0]));
	save_item(NAME(m_vregs[1]));
	save_item(NAME(m_charbank));
}

void ddribble_state::machine_reset()
{
	int i;

	for (i = 0; i < 5; i++)
	{
		m_vregs[0][i] = 0;
		m_vregs[1][i] = 0;
	}

	m_int_enable_0 = 0;
	m_int_enable_1 = 0;
	m_charbank[0] = 0;
	m_charbank[1] = 0;
}

static MACHINE_CONFIG_START( ddribble, ddribble_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809,  XTAL_18_432MHz/12)  /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(cpu0_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", ddribble_state,  ddribble_interrupt_0)

	MCFG_CPU_ADD("cpu1", M6809, XTAL_18_432MHz/12)  /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(cpu1_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", ddribble_state,  ddribble_interrupt_1)

	MCFG_CPU_ADD("cpu2", M6809, XTAL_18_432MHz/12)  /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(cpu2_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))  /* we need heavy synch */


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
/*  MCFG_SCREEN_SIZE(64*8, 32*8)
    MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1) */
	MCFG_SCREEN_UPDATE_DRIVER(ddribble_state, screen_update_ddribble)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ddribble)
	MCFG_PALETTE_ADD("palette", 64 + 256)
	MCFG_PALETTE_INDIRECT_ENTRIES(64)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)
	MCFG_PALETTE_INIT_OWNER(ddribble_state, ddribble)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, XTAL_3_579545MHz) /* verified on pcb */
	MCFG_AY8910_PORT_B_READ_CB(READ8(ddribble_state, ddribble_vlm5030_busy_r))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(ddribble_state, ddribble_vlm5030_ctrl_w))
	MCFG_SOUND_ROUTE(0, "filter1", 0.25)
	MCFG_SOUND_ROUTE(1, "filter2", 0.25)
	MCFG_SOUND_ROUTE(2, "filter3", 0.25)
	MCFG_SOUND_ROUTE(3, "mono", 0.25)

	MCFG_SOUND_ADD("vlm", VLM5030, XTAL_3_579545MHz) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_FILTER_RC_ADD("filter1", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MCFG_FILTER_RC_ADD("filter2", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MCFG_FILTER_RC_ADD("filter3", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


ROM_START( ddribble )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64K for the CPU #0 */
	ROM_LOAD( "690c03.bin", 0x00000, 0x10000, CRC(07975a58) SHA1(96fd1b2348bbdf560067d8ee3cd4c0514e263d7a) )

	ROM_REGION( 0x10000, "cpu1", 0 ) /* 64 for the CPU #1 */
	ROM_LOAD( "690c02.bin", 0x08000, 0x08000, CRC(f07c030a) SHA1(db96a10f8bb657bf285266db9e775fa6af82f38c) )

	ROM_REGION( 0x10000, "cpu2", 0 )    /* 64k for the SOUND CPU */
	ROM_LOAD( "690b01.bin", 0x08000, 0x08000, CRC(806b8453) SHA1(3184772c5e5181438a17ac72129070bf164b2965) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "690a05.bin",  0x00000, 0x20000, CRC(6a816d0d) SHA1(73f2527d5f2b9d51b784be36e07e0d0c566a28d9) )    /* characters & objects */
	ROM_LOAD16_BYTE( "690a06.bin",  0x00001, 0x20000, CRC(46300cd0) SHA1(07197a546fff452a41575fcd481da64ac6bf601e) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "690a10.bin", 0x00000, 0x20000, CRC(61efa222) SHA1(bd7b993ad1c06d8f6ac29fbc07c4a987abe1ab42) ) /* characters */
	ROM_LOAD16_BYTE( "690a09.bin", 0x00001, 0x20000, CRC(ab682186) SHA1(a28982835042a07354557e1539b097cdf93fc466) )
	ROM_LOAD16_BYTE( "690a08.bin", 0x40000, 0x20000, CRC(9a889944) SHA1(ca96815aefb1e336bd2288841b00a5c21cacf90f) ) /* objects */
	ROM_LOAD16_BYTE( "690a07.bin", 0x40001, 0x20000, CRC(faf81b3f) SHA1(0bd647b4cdd3f2209472e303fd22eedd5533d1b1) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "690a11.i15", 0x0000, 0x0100, CRC(f34617ad) SHA1(79ceba6fe204472a5a659641ac4f14bb1f0ee3f6) )  /* sprite lookup table */

	ROM_REGION( 0x20000, "vlm", 0 ) /* 128k for the VLM5030 data */
	ROM_LOAD( "690a04.bin", 0x00000, 0x20000, CRC(1bfeb763) SHA1(f3e9acb2a7a9b4c8dee6838c1344a7a65c27ff77) )

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pal10l8-007553.bin", 0x0000, 0x002c, CRC(0ae5a161) SHA1(87571addf434b332019ea0e22372eb24b4fd0197) )
ROM_END

ROM_START( ddribblep )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64K for the CPU #0 */
	ROM_LOAD( "ebs_11-19.c19",  0x00000, 0x10000, CRC(0a81c926) SHA1(1ecd30f0d352cf6c96d246bb443b5a6738624b9b) )

	ROM_REGION( 0x10000, "cpu1", 0 ) /* 64 for the CPU #1 */
	ROM_LOAD( "eb_11-19.c12", 0x08000, 0x08000, CRC(22130292) SHA1(a5f9bf3f63ff85d171f096867433513419458b0e) )

	ROM_REGION( 0x10000, "cpu2", 0 )    /* 64k for the SOUND CPU */
	ROM_LOAD( "master_sound.a6", 0x08000, 0x08000, CRC(090e3a31) SHA1(4c645b55d52abb859354ea2ea401e4ab99f5d493) )

	ROM_REGION( 0x40000, "gfx1", 0 ) /* same content as parent */
	ROM_LOAD16_BYTE( "v1a.e12", 0x00000, 0x10000, CRC(53724765) SHA1(55a45ab71f7bf55ed805d4dc2345cadc4171f323) )    /* characters & objects */
	ROM_LOAD16_BYTE( "01a.e11", 0x20000, 0x10000, CRC(1ae5d725) SHA1(d8dd41cc1872c6d218cc425d1cd03f8d8eefe3e3) )    /* characters & objects */
	ROM_LOAD16_BYTE( "v1b.e13", 0x00001, 0x10000, CRC(d9dc6f1a) SHA1(f50169525c5109ba65acdccbb01dddb92926462a) )
	ROM_LOAD16_BYTE( "01b.d14", 0x20001, 0x10000, CRC(054c5242) SHA1(411389e36d33fd27e13ffc6a7d4b295a42f08869) )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* same content as parent */
	ROM_LOAD16_BYTE( "v2a00.i13",         0x00000, 0x10000, CRC(a33f7d6d) SHA1(c2b9a9a66e4712785250cad69a5e43338af60a82) )  /* characters */
	ROM_LOAD16_BYTE( "v2a10.h13",         0x20000, 0x10000, CRC(8fbc7454) SHA1(93782d148afe64b14fa46deb4d227ef167030c94) )  /* characters */
	ROM_LOAD16_BYTE( "v2b00.i12",         0x00001, 0x10000, CRC(e63759bb) SHA1(df7e94f40266aa8995509346cdfdce08a885de16) )
	ROM_LOAD16_BYTE( "v2b10.h12",         0x20001, 0x10000, CRC(8a7d4062) SHA1(5b5eb4edc765f0e13e22f9de62ddae7380ba3790) )
	ROM_LOAD16_BYTE( "02a00.i11",         0x40000, 0x10000, CRC(6751a942) SHA1(a71c9cbbf1fba92664144d571d49cf2c15f45408) )  /* objects */
	ROM_LOAD16_BYTE( "02a10.h11",         0x60000, 0x10000, CRC(bc5ff11c) SHA1(b02296982298e1a659ce05606b291eda9a605cc8) )  /* objects */
	ROM_LOAD16_BYTE( "02b00_11-4.i8.bin", 0x40001, 0x10000, CRC(460aa7b4) SHA1(9e928d6150e7a91d411c0510198e80d523a88272) )
	ROM_LOAD16_BYTE( "02b10.h8",          0x60001, 0x10000, CRC(2cc7ee28) SHA1(c96890383dbef755953f851a43449cf563e2e1a5) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "6301-1.i15", 0x0000, 0x0100, CRC(f34617ad) SHA1(79ceba6fe204472a5a659641ac4f14bb1f0ee3f6) )  /* sprite lookup table */

	ROM_REGION( 0x20000, "vlm", 0 )  /* same content as parent */ /* 128k for the VLM5030 data */
	ROM_LOAD( "voice_00.e7", 0x00000, 0x10000, CRC(8bd0fcf7) SHA1(d55644f8b33eff6f960725f00ba842e0253e3b36) )
	ROM_LOAD( "voice_10.d7", 0x10000, 0x10000, CRC(b4c97494) SHA1(93f7c3c93f6f790c3f480e183da0105b5ac3593b) )
ROM_END

GAME( 1986, ddribble,  0,        ddribble, ddribble, driver_device, 0, ROT0, "Konami", "Double Dribble", MACHINE_SUPPORTS_SAVE )
GAME( 1986, ddribblep, ddribble, ddribble, ddribble, driver_device, 0, ROT0, "Konami", "Double Dribble (prototype?)", MACHINE_SUPPORTS_SAVE )
