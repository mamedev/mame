// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

Rock'n Rage (GX620) (c) 1986 Konami

Driver by Manuel Abadia <emumanu+mame@gmail.com>

GX620 PWB302109A
|------------------------------------------------------|
| LA4445  VOL1 JP1  LM324    6809    6309              |
|         VOL2 JP2  YM2151                             |
|         YM3012 3.579545MHz                           |
|                                                      |
|       CN1                                            |
|         LM324                007420                  |
|                    6116 620G3.11C   6264  620N2.15C  |
|                                             620N1.16C|
|J                                                     |
|A                                                     |
|M                                   24MHz             |
|M        620D4.6E                                     |
|A                             |-------|               |
|            VLM5030           |007342 |               |
|  DSW3(4) DSW2(8)             |       |         6264  |
|          DSW1(8) 620D11B.7F  |       |620D6B.15F     |
|    |-----------|   620G10B.8F|-------|  620D5B.16F   |
|    |           |                                     |
|    |  007327   |            620D9.11G                |
|    |           | 620G11A.7G   620D8.12G  620D6A.15G  |
|    |-----------|    620D10A.8G  620D7.13G  620D5A.16G|
|------------------------------------------------------|

Notes:
      6809 clock 1.500MHz [24/16]
      6309 clock 3.000MHz [24/8]
      VLM5030 clock 3.579545MHz
      YM2151 clock 3.579545MHz
      VSync 60Hz
      HSync 15.16kHz
      JP1/JP2 - 4-pin jumper to set stereo/mono output
      CN1 - 4 pin right speaker sound output connector
      6116 - 2k x8 SRAM (DIP24)
      6264 - 8k x8 SRAM (DIP28)
      Konami custom ICs -
                          007342 3905 67 3147B (PGA181)
                          007420 3916 67 23 52 A (SDIP64)
                          007327 (custom ceramic wide DIP40)

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m6809/hd6309.h"
#include "sound/2151intf.h"
#include "includes/rockrage.h"
#include "includes/konamipt.h"


INTERRUPT_GEN_MEMBER(rockrage_state::rockrage_interrupt)
{
	if (m_k007342->is_int_enabled())
		device.execute().set_input_line(HD6309_IRQ_LINE, HOLD_LINE);
}

WRITE8_MEMBER(rockrage_state::rockrage_bankswitch_w)
{
	/* bits 4-6 = bank number */
	m_rombank->set_entry((data & 0x70) >> 4);

	/* bits 0 & 1 = coin counters */
	machine().bookkeeping().coin_counter_w(0,data & 0x01);
	machine().bookkeeping().coin_counter_w(1,data & 0x02);

	/* other bits unknown */
}

WRITE8_MEMBER(rockrage_state::rockrage_sh_irqtrigger_w)
{
	soundlatch_byte_w(space, offset, data);
	m_audiocpu->set_input_line(M6809_IRQ_LINE, HOLD_LINE);
}

READ8_MEMBER(rockrage_state::rockrage_VLM5030_busy_r)
{
	return (m_vlm->bsy() ? 1 : 0);
}

WRITE8_MEMBER(rockrage_state::rockrage_speech_w)
{
	/* bit2 = data bus enable */
	m_vlm->rst((data >> 1) & 0x01);
	m_vlm->st((data >> 0) & 0x01);
}

static ADDRESS_MAP_START( rockrage_map, AS_PROGRAM, 8, rockrage_state )
	AM_RANGE(0x0000, 0x1fff) AM_DEVREADWRITE("k007342", k007342_device, read, write)                    /* Color RAM + Video RAM */
	AM_RANGE(0x2000, 0x21ff) AM_DEVREADWRITE("k007420", k007420_device, read, write)                    /* Sprite RAM */
	AM_RANGE(0x2200, 0x23ff) AM_DEVREADWRITE("k007342", k007342_device, scroll_r, scroll_w)  /* Scroll RAM */
	AM_RANGE(0x2400, 0x247f) AM_RAM_DEVWRITE("palette", palette_device, write_indirect) AM_SHARE("palette")
	AM_RANGE(0x2600, 0x2607) AM_DEVWRITE("k007342", k007342_device, vreg_w)                          /* Video Registers */
	AM_RANGE(0x2e00, 0x2e00) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x2e01, 0x2e01) AM_READ_PORT("P1")
	AM_RANGE(0x2e02, 0x2e02) AM_READ_PORT("P2")
	AM_RANGE(0x2e03, 0x2e03) AM_READ_PORT("DSW2")
	AM_RANGE(0x2e40, 0x2e40) AM_READ_PORT("DSW1")
	AM_RANGE(0x2e80, 0x2e80) AM_WRITE(rockrage_sh_irqtrigger_w)                 /* cause interrupt on audio CPU */
	AM_RANGE(0x2ec0, 0x2ec0) AM_WRITE(watchdog_reset_w)                         /* watchdog reset */
	AM_RANGE(0x2f00, 0x2f00) AM_WRITE(rockrage_vreg_w)                          /* ??? */
	AM_RANGE(0x2f40, 0x2f40) AM_WRITE(rockrage_bankswitch_w)                    /* bankswitch control */
	AM_RANGE(0x4000, 0x5fff) AM_RAM                                             /* RAM */
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("rombank")                              /* banked ROM */
	AM_RANGE(0x8000, 0xffff) AM_ROM                                             /* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( rockrage_sound_map, AS_PROGRAM, 8, rockrage_state )
	AM_RANGE(0x2000, 0x2000) AM_DEVWRITE("vlm", vlm5030_device, data_w)              /* VLM5030 */
	AM_RANGE(0x3000, 0x3000) AM_READ(rockrage_VLM5030_busy_r)           /* VLM5030 */
	AM_RANGE(0x4000, 0x4000) AM_WRITE(rockrage_speech_w)                /* VLM5030 */
	AM_RANGE(0x5000, 0x5000) AM_READ(soundlatch_byte_r)                             /* soundlatch_byte_r */
	AM_RANGE(0x6000, 0x6001) AM_DEVREADWRITE("ymsnd", ym2151_device,read,write)         /* YM 2151 */
	AM_RANGE(0x7000, 0x77ff) AM_RAM                                             /* RAM */
	AM_RANGE(0x8000, 0xffff) AM_ROM                                             /* ROM */
ADDRESS_MAP_END

/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( rockrage )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* Invalid = both coin slots disabled */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "30k & Every 70k" )
	PORT_DIPSETTING(    0x00, "40k & Every 80k" )
	PORT_DIPNAME( 0x10, 0x10, "Freeze Screen" )         PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW3:2" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW3:3" )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,            /* 8 x 8 characters */
	0x40000/32,     /* 8192 characters */
	4,              /* 4bpp */
	{ 0, 1, 2, 3 }, /* the four bitplanes are packed in one nibble */
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8            /* every character takes 32 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	8,8,            /* 8*8 sprites */
	0x40000/32, /* 8192 sprites */
	4,              /* 4 bpp */
	{ 0, 1, 2, 3 }, /* the four bitplanes are packed in one nibble */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8            /* every sprite takes 32 consecutive bytes */
};

static GFXDECODE_START( rockrage )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 32 )  /* colors 00..31, using 2 lookup tables */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 512, 16 )  /* colors 32..47, using lookup table */
GFXDECODE_END

/***************************************************************************

    Machine Driver

***************************************************************************/

void rockrage_state::machine_start()
{
	UINT8 *ROM = memregion("maincpu")->base();

	m_rombank->configure_entries(0, 8, &ROM[0x10000], 0x2000);

	save_item(NAME(m_vreg));
}

void rockrage_state::machine_reset()
{
	m_vreg = 0;
}

static MACHINE_CONFIG_START( rockrage, rockrage_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD6309, 3000000*4)      /* 24MHz/8 */
	MCFG_CPU_PROGRAM_MAP(rockrage_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", rockrage_state,  rockrage_interrupt)

	MCFG_CPU_ADD("audiocpu", M6809, 1500000)        /* 24MHz/16 */
	MCFG_CPU_PROGRAM_MAP(rockrage_sound_map)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(rockrage_state, screen_update_rockrage)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_K007342_ADD("k007342")
	MCFG_K007342_GFXNUM(0)
	MCFG_K007342_CALLBACK_OWNER(rockrage_state, rockrage_tile_callback)
	MCFG_K007342_GFXDECODE("gfxdecode")

	MCFG_K007420_ADD("k007420")
	MCFG_K007420_BANK_LIMIT(0x3ff)
	MCFG_K007420_CALLBACK_OWNER(rockrage_state, rockrage_sprite_callback)
	MCFG_K007420_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", rockrage)
	MCFG_PALETTE_ADD("palette", 16*16*3)
	MCFG_PALETTE_INDIRECT_ENTRIES(64)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)
	MCFG_PALETTE_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_PALETTE_INIT_OWNER(rockrage_state, rockrage)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", 3579545)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.60)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.60)

	MCFG_SOUND_ADD("vlm", VLM5030, 3579545)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.60)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.60)
MACHINE_CONFIG_END


/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( rockrage )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "620q01.16c", 0x08000, 0x08000, CRC(0ddb5ef5) SHA1(71b38c9f957858371f0ac95720d3c6d07339e5c5) )    /* fixed ROM */
	ROM_LOAD( "620q02.15c", 0x10000, 0x10000, CRC(b4f6e346) SHA1(43fded4484836ff315dd6e40991f909dad73f1ed) )    /* banked ROM */

	ROM_REGION(  0x10000,  "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "620k03.11c", 0x08000, 0x08000, CRC(9fbefe82) SHA1(ab42b7e519a0dd08f2249dad0819edea0976f39a) )

	ROM_REGION( 0x040000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "620k05.rom", 0x00000, 0x20000, CRC(145d387c) SHA1(4fb0c54f9a218d512d8aec09ef995494a06912d6)  )  /* tiles */
	ROM_LOAD16_BYTE( "620k06.rom", 0x00001, 0x20000, CRC(7fa2c57c) SHA1(8c5d85c31dc26cb59a012ebb1ea195c3db80cda8)  ) /* Both World & Japan use the same "K" code for these??? */

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "620k11.rom", 0x000000, 0x20000, CRC(70449239) SHA1(07653ea3bfe0063c9d2b2102ac52a1b50fc2971e) )   /* sprites */
	ROM_LOAD( "620l10.8g",  0x020000, 0x20000, CRC(06d108e0) SHA1(cae8c5f2fc4e84bc7adbf27f71a18a74968c4296) ) /* One "K" & one "L" code version??? */

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "620k07.13g", 0x00000, 0x00100, CRC(b6135ee0) SHA1(248a978987cff86c2bbad10ef332f63a6abd5bee) )    /* layer 0 lookup table */
	ROM_LOAD( "620k08.12g", 0x00100, 0x00100, CRC(b499800c) SHA1(46fa4e071ebceed12027de109be1e16dde5e846e) )    /* layer 1 lookup table */
	ROM_LOAD( "620k09.11g", 0x00200, 0x00100, CRC(9f0e0608) SHA1(c95bdb370e4a91f27afbd5ff3b39b2e0ad87da73) )    /* sprite lookup table */

	ROM_REGION( 0x08000, "vlm", 0 ) /* VLM3050 data */
	ROM_LOAD( "620k04.6e", 0x00000, 0x08000, CRC(8be969f3) SHA1(9856b4c13fac77b645aed67a08cb4965b4966492) )
ROM_END

ROM_START( rockragea )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "620n01.16c", 0x08000, 0x10000, CRC(f89f56ea) SHA1(64ba2575e09af257b242d913eab69130f7341894) )    /* fixed ROM */
	ROM_LOAD( "620n02.15c", 0x10000, 0x10000, CRC(5bc1f1cf) SHA1(d5bb9971d778449e0c01495f9888c0da7ac617a7) )    /* banked ROM */

	ROM_REGION(  0x10000,  "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "620k03.11c", 0x08000, 0x08000, CRC(9fbefe82) SHA1(ab42b7e519a0dd08f2249dad0819edea0976f39a) ) /* Same rom but labeled as ver "G" */

	ROM_REGION( 0x040000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "620d05a.16g", 0x00000, 0x10000, CRC(4d53fde9) SHA1(941fb6c94922727516945330b4b738aa052f7734) )  /* tiles */
	ROM_LOAD16_BYTE( "620d06a.15g", 0x00001, 0x10000, CRC(8cc05d4b) SHA1(0d6fef98bdc4d299229de4e0044241aedee83b85) )
	ROM_LOAD16_BYTE( "620d05b.16f", 0x20000, 0x10000, CRC(69f4599f) SHA1(664581874d74ed7bf59bde6730799e15f4e0144d) )
	ROM_LOAD16_BYTE( "620d06b.15f", 0x20001, 0x10000, CRC(3892d41d) SHA1(c49f2e61f24a59be4e59e2f3c60e731b8a05ddd3) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "620g11a.7g", 0x000000, 0x10000, CRC(0ef40c2c) SHA1(2c0b7e611333a072ebcef60c1985211d5936bf66) )   /* sprites */
	ROM_LOAD( "620d11b.7f", 0x010000, 0x10000, CRC(8f116cbf) SHA1(0400609aadde39c6f02ab954c78bc67a1d23da1d) )
	ROM_LOAD( "620d10a.8g", 0x020000, 0x10000, CRC(4789ae7b) SHA1(8885ca20bf746fb3ed229486c0e3903ababfacc9) )
	ROM_LOAD( "620g10b.8f", 0x030000, 0x10000, CRC(1618854a) SHA1(0afb34a9ed97f13c1910acd7767cb8546ea7e6cd) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "620k07.13g", 0x00000, 0x00100, CRC(b6135ee0) SHA1(248a978987cff86c2bbad10ef332f63a6abd5bee) )    /* layer 0 lookup table */
	ROM_LOAD( "620k08.12g", 0x00100, 0x00100, CRC(b499800c) SHA1(46fa4e071ebceed12027de109be1e16dde5e846e) )    /* layer 1 lookup table */
	ROM_LOAD( "620k09.11g", 0x00200, 0x00100, CRC(9f0e0608) SHA1(c95bdb370e4a91f27afbd5ff3b39b2e0ad87da73) )    /* sprite lookup table */

	ROM_REGION( 0x08000, "vlm", 0 ) /* VLM3050 data */
	ROM_LOAD( "620k04.6e", 0x00000, 0x08000, CRC(8be969f3) SHA1(9856b4c13fac77b645aed67a08cb4965b4966492) ) /* Same rom but labeled as ver "G" */
ROM_END

ROM_START( rockragej )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "620k01.16c", 0x08000, 0x08000, CRC(4f5171f7) SHA1(5bce9e3f9d01c113c697853763cd891b91297eb2) )    /* fixed ROM */
	ROM_LOAD( "620k02.15c", 0x10000, 0x10000, CRC(04c4d8f7) SHA1(2a1a024fc38bb934c454092b0aed74d0f1d1c4af) )    /* banked ROM */

	ROM_REGION(  0x10000,  "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "620k03.11c", 0x08000, 0x08000, CRC(9fbefe82) SHA1(ab42b7e519a0dd08f2249dad0819edea0976f39a) )

	ROM_REGION( 0x040000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "620k05.16g", 0x00000, 0x20000, CRC(ca9d9346) SHA1(fee8d98def802f312c6cd0ec751c67aa18acfacd) )   /* tiles */
	ROM_LOAD16_BYTE( "620k06.15g", 0x00001, 0x20000, CRC(c0e2b35c) SHA1(fb37a151188f27f883fed5fdfb0094c3efa9470d) ) /* Both World & Japan use the same "K" code for these??? */

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "620k11.7g",  0x000000, 0x20000, CRC(7430f6e9) SHA1(5d488c7b7b0eb4e502b3e566ac102cd3267e8568) )   /* sprites */
	ROM_LOAD( "620k10.8g",  0x020000, 0x20000, CRC(0d1a95ab) SHA1(be565424f17af31dcd07004c6be03bbb00aef514) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "620k07.13g", 0x00000, 0x00100, CRC(b6135ee0) SHA1(248a978987cff86c2bbad10ef332f63a6abd5bee) )    /* layer 0 lookup table */
	ROM_LOAD( "620k08.12g", 0x00100, 0x00100, CRC(b499800c) SHA1(46fa4e071ebceed12027de109be1e16dde5e846e) )    /* layer 1 lookup table */
	ROM_LOAD( "620k09.11g", 0x00200, 0x00100, CRC(9f0e0608) SHA1(c95bdb370e4a91f27afbd5ff3b39b2e0ad87da73) )    /* sprite lookup table */

	ROM_REGION( 0x08000, "vlm", 0 ) /* VLM3050 data */
	ROM_LOAD( "620k04.6e", 0x00000, 0x08000, CRC(8be969f3) SHA1(9856b4c13fac77b645aed67a08cb4965b4966492) )
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR, NAME,      PARENT,   MACHINE,  INPUT,    INIT,MONITOR,COMPANY,FULLNAME,FLAGS
GAME( 1986, rockrage,  0,        rockrage, rockrage, driver_device, 0,   ROT0,   "Konami", "Rock'n Rage (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, rockragea, rockrage, rockrage, rockrage, driver_device, 0,   ROT0,   "Konami", "Rock'n Rage (prototype?)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, rockragej, rockrage, rockrage, rockrage, driver_device, 0,   ROT0,   "Konami", "Koi no Hotrock (Japan)", MACHINE_SUPPORTS_SAVE )
