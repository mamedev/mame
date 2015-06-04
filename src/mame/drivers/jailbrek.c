// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

Jailbreak - (c) 1986 Konami

Ernesto Corvi
ernesto@imagina.com


Konami designated Jail Break with the label of GX507.  (There is also
the label PWB 300394A silk screened onto the board.)


Board Parts:

    Konami A082 @ 11a (Encrypted 6809 CPU)
    VLM5030 @ 6a
    Konami 005849 @ 8e
    18.432000MHz @ OSC
    3.579545MHz @ XTAL
    SN76489AN @ 6d
    6301 @ 6f and 7f (PROM's)
    6331 @ 1f and 2f (PROM's)

Jail Break
Konami 1986

PCB Layout
----------

GX507
PWB 300394A
|----------------------------------------|
| KONAMI-1   507P03.11D    6264     4416 |
|            507P02.9D    |-------| 4416 |
|                         |KONAMI | 4416 |
|       507L01.8C         |005849 | 4416 |
|                         |-------|      |
| VLM5030                       507J13.7F|
|                    SN76489    507J12.6F|
| 3.579545MHz        18.432MHz           |
| DSW3                          507J09.5F|
|                               507L08.4F|
| DSW2                          507J07.3F|
|           UPC324                       |
| DSW1                          507J11.2F|
|      LA4460                   507J10.1F|
|             005273 005273              |
|             005273 005273              |
|     VOL           18-WAY        CN1    |
|----------------------------------------|
Notes:
      KONAMI1   - Custom encrypted 6809 CPU, clock 1.536MHz [18.432/12] (DIP42)
      VLM5030   - Sanyo VLM5030 speech chip, clock 3.579545MHz (DIP40)
      SN76489   - Texas Instruments SN76489 noise generator IC, clock 1.536MHz [18.432/12] (DIP16)
      UPC324    - NEC UPC324 op amp. Also compatible with LM324 and Mitsubishi M5224P (DIP14)
      LA4460    - Power amp IC
      DSW1/2    - 8-position dip switches
      DSW3      - 4-position dip switch
      CN1       - 4-pin connector for monitors that require separate syncs. Pin 1 is negative vertical sync output.
                  Horizontal sync is taken from the regular composite sync on the PCB on pin B14.
      18-WAY    - 18-way edge connector. Pinout matches standard Konami 18-way pinout (Scramble/Frogger/Yie-Ar Kung Fu etc)
      6264      - 8kx8 SRAM (DIP28)
      4416      - 16kx4 DRAM (DIP18)
      005849    - Konami custom graphics generator (PGA179)
      005273    - Konami custom resistor array (SIL10)
      507J12/13 - MMI 63S141 256x4 Bipolar PROM (= 82S129 & 6301 etc)
      507J10/11 - MMI 63S081 32x8 Bipolar PROM (= 82S123 & 6331 etc)
      All ROMs type 27C128. Pin 26 of 8C is tied high, the lower half is empty.

      Measurements
      ------------
      Xtal  - 3.57867MHz
      OSC   - 18.43199MHz
      HSync - 15.5185kHz
      VSync - 60.6059Hz


****************************************************************************

    TODO:

    - coin counters

***************************************************************************/

#include "emu.h"
#include "machine/konami1.h"
#include "cpu/m6809/m6809.h"
#include "sound/sn76496.h"
#include "includes/konamipt.h"
#include "includes/jailbrek.h"


WRITE8_MEMBER(jailbrek_state::ctrl_w)
{
	m_nmi_enable = data & 0x01;
	m_irq_enable = data & 0x02;
	flip_screen_set(data & 0x08);
}

INTERRUPT_GEN_MEMBER(jailbrek_state::jb_interrupt)
{
	if (m_irq_enable)
		device.execute().set_input_line(0, HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(jailbrek_state::jb_interrupt_nmi)
{
	if (m_nmi_enable)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


READ8_MEMBER(jailbrek_state::jailbrek_speech_r)
{
	return (m_vlm->bsy() ? 1 : 0);
}

WRITE8_MEMBER(jailbrek_state::jailbrek_speech_w)
{
	/* bit 0 could be latch direction like in yiear */
	m_vlm->st((data >> 1) & 1);
	m_vlm->rst((data >> 2) & 1);
}

static ADDRESS_MAP_START( jailbrek_map, AS_PROGRAM, 8, jailbrek_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM_WRITE(jailbrek_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x0800, 0x0fff) AM_RAM_WRITE(jailbrek_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x1000, 0x10bf) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x10c0, 0x14ff) AM_RAM /* ??? */
	AM_RANGE(0x1500, 0x1fff) AM_RAM /* work ram */
	AM_RANGE(0x2000, 0x203f) AM_RAM AM_SHARE("scroll_x")
	AM_RANGE(0x2040, 0x2040) AM_WRITENOP /* ??? */
	AM_RANGE(0x2041, 0x2041) AM_WRITENOP /* ??? */
	AM_RANGE(0x2042, 0x2042) AM_RAM AM_SHARE("scroll_dir") /* bit 2 = scroll direction */
	AM_RANGE(0x2043, 0x2043) AM_WRITENOP /* ??? */
	AM_RANGE(0x2044, 0x2044) AM_WRITE(ctrl_w) /* irq, nmi enable, screen flip */
	AM_RANGE(0x3000, 0x307f) AM_RAM /* related to sprites? */
	AM_RANGE(0x3100, 0x3100) AM_READ_PORT("DSW2") AM_DEVWRITE("snsnd", sn76489a_device, write)
	AM_RANGE(0x3200, 0x3200) AM_READ_PORT("DSW3") AM_WRITENOP /* mirror of the previous? */
	AM_RANGE(0x3300, 0x3300) AM_READ_PORT("SYSTEM") AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x3301, 0x3301) AM_READ_PORT("P1")
	AM_RANGE(0x3302, 0x3302) AM_READ_PORT("P2")
	AM_RANGE(0x3303, 0x3303) AM_READ_PORT("DSW1")
	AM_RANGE(0x4000, 0x4000) AM_WRITE(jailbrek_speech_w) /* speech pins */
	AM_RANGE(0x5000, 0x5000) AM_DEVWRITE("vlm", vlm5030_device, data_w) /* speech data */
	AM_RANGE(0x6000, 0x6000) AM_READ(jailbrek_speech_r)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END



static INPUT_PORTS_START( jailbrek )
	PORT_START("SYSTEM")    /* $3300 */
	KONAMI8_SYSTEM_10
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")        /* $3301 */
	KONAMI8_B12_UNK(1)  // button1 = shoot, button2 = select

	PORT_START("P2")        /* $3302 */
	KONAMI8_B12_UNK(2)

	PORT_START("DSW1")      /* $3303 */
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* "Invalid" = both coin slots disabled */

	PORT_START("DSW2")      /* $3100 */
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )       PORT_DIPLOCATION( "SW2:1,2" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )     PORT_DIPLOCATION( "SW2:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )  PORT_DIPLOCATION( "SW2:4" )
	PORT_DIPSETTING(    0x08, "30K 70K+" )
	PORT_DIPSETTING(    0x00, "40K 80K+" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Difficulty ) )  PORT_DIPLOCATION( "SW2:5,6" )
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )        PORT_DIPLOCATION( "SW2:7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION( "SW2:8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")      /* $3200 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION( "SW3:1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )     PORT_DIPLOCATION( "SW3:2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )        PORT_DIPLOCATION( "SW3:3" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )        PORT_DIPLOCATION( "SW3:4" )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	1024,   /* 1024 characters */
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the four bitplanes are packed in one nibble */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    /* every char takes 32 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	512,    /* 512 sprites */
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the bitplanes are packed in one nibble */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
			32*8+0*4, 32*8+1*4, 32*8+2*4, 32*8+3*4, 32*8+4*4, 32*8+5*4, 32*8+6*4, 32*8+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	128*8   /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( jailbrek )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 ) /* characters */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 16*16, 16 ) /* sprites */
GFXDECODE_END


void jailbrek_state::machine_start()
{
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_nmi_enable));
}

void jailbrek_state::machine_reset()
{
	m_irq_enable = 0;
	m_nmi_enable = 0;
}

static MACHINE_CONFIG_START( jailbrek, jailbrek_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", KONAMI1, MASTER_CLOCK/12)
	MCFG_CPU_PROGRAM_MAP(jailbrek_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", jailbrek_state,  jb_interrupt)
	MCFG_CPU_PERIODIC_INT_DRIVER(jailbrek_state, jb_interrupt_nmi,  500) /* ? */


	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", jailbrek)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(jailbrek_state, jailbrek)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK/3, 396, 8, 248, 256, 16, 240)
	MCFG_SCREEN_UPDATE_DRIVER(jailbrek_state, screen_update_jailbrek)
	MCFG_SCREEN_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("snsnd", SN76489A, MASTER_CLOCK/12)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("vlm", VLM5030, VOICE_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( jailbrek )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "507p03.11d", 0x8000, 0x4000, CRC(a0b88dfd) SHA1(f999e382b9d3b812fca41f4d0da3ea692fef6b19) )
	ROM_LOAD( "507p02.9d",  0xc000, 0x4000, CRC(444b7d8e) SHA1(c708b67c2d249448dae9a3d10c24d13ba6849597) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "507l08.4f",  0x0000, 0x4000, CRC(e3b7a226) SHA1(c19a02a2def65648bf198fccec98ebbd2fc7c0fb) )  /* characters */
	ROM_LOAD( "507j09.5f",  0x4000, 0x4000, CRC(504f0912) SHA1(b51a45dd5506bccdf0061dd6edd7f49ac86ed0f8) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "507j04.3e",  0x0000, 0x4000, CRC(0d269524) SHA1(a10ddb405e884bfec521a3c7a29d22f63e535b59) )  /* sprites */
	ROM_LOAD( "507j05.4e",  0x4000, 0x4000, CRC(27d4f6f4) SHA1(c42c064dbd7c5cf0b1d99651367e0bee1728a5b0) )
	ROM_LOAD( "507j06.5e",  0x8000, 0x4000, CRC(717485cb) SHA1(22609489186dcb3d7cd49b7ddfdc6f04d0739354) )
	ROM_LOAD( "507j07.3f",  0xc000, 0x4000, CRC(e933086f) SHA1(c0fd1e8d23c0f7e14c0b75f629448034420cf8ef) )

	ROM_REGION( 0x0240, "proms", 0 )
	ROM_LOAD( "507j10.1f",  0x0000, 0x0020, CRC(f1909605) SHA1(91eaa865375b3bc052897732b64b1ff7df3f78f6) ) /* red & green */
	ROM_LOAD( "507j11.2f",  0x0020, 0x0020, CRC(f70bb122) SHA1(bf77990260e8346faa3d3481718cbe46a4a27150) ) /* blue */
	ROM_LOAD( "507j13.7f",  0x0040, 0x0100, CRC(d4fe5c97) SHA1(972e9dab6c53722545dd3a43e3ada7921e88708b) ) /* char lookup */
	ROM_LOAD( "507j12.6f",  0x0140, 0x0100, CRC(0266c7db) SHA1(a8f21e86e6d974c9bfd92a147689d0e7316d66e2) ) /* sprites lookup */

	ROM_REGION( 0x4000, "vlm", 0 ) /* speech rom */
	ROM_LOAD( "507l01.8c",  0x0000, 0x4000, CRC(0c8a3605) SHA1(d886b66d3861c3a90a1825ccf5bf0011831ca366) )
ROM_END

ROM_START( manhatan )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "507n03.11d", 0x8000, 0x4000, CRC(e5039f7e) SHA1(0f12484ed40444d978e0405c27bdd027ae2e2a0b) )
	ROM_LOAD( "507n02.9d",  0xc000, 0x4000, CRC(143cc62c) SHA1(9520dbb1b6f1fa439e03d4caa9bed96ef8f805f2) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "507j08.4f",  0x0000, 0x4000, CRC(175e1b49) SHA1(4cfe982cdf7729bd05c6da803480571876320bf6) )  /* characters */
	ROM_LOAD( "507j09.5f",  0x4000, 0x4000, CRC(504f0912) SHA1(b51a45dd5506bccdf0061dd6edd7f49ac86ed0f8) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "507j04.3e",  0x0000, 0x4000, CRC(0d269524) SHA1(a10ddb405e884bfec521a3c7a29d22f63e535b59) )  /* sprites */
	ROM_LOAD( "507j05.4e",  0x4000, 0x4000, CRC(27d4f6f4) SHA1(c42c064dbd7c5cf0b1d99651367e0bee1728a5b0) )
	ROM_LOAD( "507j06.5e",  0x8000, 0x4000, CRC(717485cb) SHA1(22609489186dcb3d7cd49b7ddfdc6f04d0739354) )
	ROM_LOAD( "507j07.3f",  0xc000, 0x4000, CRC(e933086f) SHA1(c0fd1e8d23c0f7e14c0b75f629448034420cf8ef) )

	ROM_REGION( 0x0240, "proms", 0 )
	ROM_LOAD( "507j10.1f",  0x0000, 0x0020, CRC(f1909605) SHA1(91eaa865375b3bc052897732b64b1ff7df3f78f6) ) /* red & green */
	ROM_LOAD( "507j11.2f",  0x0020, 0x0020, CRC(f70bb122) SHA1(bf77990260e8346faa3d3481718cbe46a4a27150) ) /* blue */
	ROM_LOAD( "507j13.7f",  0x0040, 0x0100, CRC(d4fe5c97) SHA1(972e9dab6c53722545dd3a43e3ada7921e88708b) ) /* char lookup */
	ROM_LOAD( "507j12.6f",  0x0140, 0x0100, CRC(0266c7db) SHA1(a8f21e86e6d974c9bfd92a147689d0e7316d66e2) ) /* sprites lookup */

	ROM_REGION( 0x4000, "vlm", 0 ) /* speech rom */
	ROM_LOAD( "507p01.8c",  0x0000, 0x4000, CRC(973fa351) SHA1(ac360d05ed4d03334e00c80e70d5ae939d93af5f) )
ROM_END

/*
    Jail Break Bootleg Chip Locations Map (Not to scale)

      |--------------------------------------------|
      |                                            |
    A |                                            |
      |                                            |
    B |                                            |
      |                                            |
    C |                                            |
      |                                            |
    D |                                            |
      |                                            |
    E |-|                                          |
        |                                          |
      |-|                                          |
    F |                                            |
      |     <=- Edge Connector                     |
    G |                                            |
      |-|                                          |
        |                                          |
    H |-|                                          |
      |                                            |
    I |                                            |
      |                                            |
    J |                                            |
      |                                            |
    K |                                            |
      |                                            |
    L |                                            |
      |                                            |
      |--------------------------------------------|
         1  2  3  4  5  6  7  8  9  1  1  1  1  1
                                    0  1  2  3  4
*/

ROM_START( jailbrekb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.k6",    0x8000, 0x8000, CRC(df0e8fc7) SHA1(62e59dbb3941ed8af365e96906315318d9aee060) )

	ROM_REGION( 0x08000, "gfx1", 0 ) /* characters */
	ROM_LOAD( "3.h6",    0x0000, 0x8000, CRC(bf67a8ff) SHA1(9aca8de7e2c2cc0ff9fe3f316a9300574df4ff06) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* sprites */
	ROM_LOAD( "5.f6",    0x0000, 0x8000, CRC(081d2eea) SHA1(dae66b2607d1a56e72e9cb456bdb3c0c21337d6c) )
	ROM_LOAD( "4.g6",    0x8000, 0x8000, CRC(e34b93b8) SHA1(fb6ed12ab017ac1e5006165f435cf0ed95a49c17) )

	ROM_REGION( 0x0240, "proms", 0 )
	ROM_LOAD( "prom.j2", 0x0000, 0x0020, CRC(f1909605) SHA1(91eaa865375b3bc052897732b64b1ff7df3f78f6) ) /* red & green */
	ROM_LOAD( "prom.i2", 0x0020, 0x0020, CRC(f70bb122) SHA1(bf77990260e8346faa3d3481718cbe46a4a27150) ) /* blue */
	ROM_LOAD( "prom.d6", 0x0040, 0x0100, CRC(d4fe5c97) SHA1(972e9dab6c53722545dd3a43e3ada7921e88708b) ) /* char lookup */
	ROM_LOAD( "prom.e6", 0x0140, 0x0100, CRC(0266c7db) SHA1(a8f21e86e6d974c9bfd92a147689d0e7316d66e2) ) /* sprites lookup */

	ROM_REGION( 0x2000, "vlm", 0 ) /* speech rom */
	ROM_LOAD( "2.i6",    0x0000, 0x2000, CRC(d91d15e3) SHA1(475fe50aafbf8f2fb79880ef0e2c25158eda5270) )

	ROM_REGION( 0x0004, "plds", 0 )
	ROM_LOAD( "k4.bin",  0x0000, 0x0001, NO_DUMP ) /* PAL16L8 */
	ROM_LOAD( "a7.bin",  0x0000, 0x0001, NO_DUMP ) /* PAL16R4 */
	ROM_LOAD( "g9.bin",  0x0000, 0x0001, NO_DUMP ) /* PAL16R6 */
	ROM_LOAD( "k8.bin",  0x0000, 0x0001, NO_DUMP ) /* PAL16L8 */
ROM_END

DRIVER_INIT_MEMBER(jailbrek_state,jailbrek)
{
	UINT8 *SPEECH_ROM = memregion("vlm")->base();
	int ind;

	/*
	   Check if the rom used for the speech is not a 2764, but a 27128.  If a
	   27128 is used then the data is stored in the upper half of the eprom.
	   (The schematics and board refer to a 2764, but all the boards I have seen
	   use a 27128.  According to the schematics pin 26 is tied high so if a 2764
	   is used then the pin is ignored, but if a 27128 is used then pin 26
	   represents address line A13.)
	*/

	if (memregion("vlm")->bytes() == 0x4000)
	{
		for (ind = 0; ind < 0x2000; ++ind)
		{
			SPEECH_ROM[ind] = SPEECH_ROM[ind + 0x2000];
		}
	}
}

GAME( 1986, jailbrek, 0,        jailbrek, jailbrek, jailbrek_state, jailbrek, ROT0, "Konami", "Jail Break", GAME_SUPPORTS_SAVE )
GAME( 1986, jailbrekb,jailbrek, jailbrek, jailbrek, jailbrek_state, jailbrek, ROT0, "bootleg","Jail Break (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1986, manhatan, jailbrek, jailbrek, jailbrek, jailbrek_state, jailbrek, ROT0, "Konami", "Manhattan 24 Bunsyo (Japan)", GAME_SUPPORTS_SAVE )
