// license:BSD-3-Clause
// copyright-holders:Tatsuyuki Satoh
/***************************************************************************

    Appoooh memory map (preliminary)
    Similar to Bank Panic

    driver by Tatsuyuki Satoh


    0000-9fff ROM
    a000-dfff BANKED ROM
    e000-e7ff RAM
    e800-efff RAM??

    write:
    f000-f01f Sprite RAM #1
    f020-f3ff Video  RAM #1
    f420-f7ff Color  RAM #1
    f800-f81f Sprite RAM #2
    f820-fbff Video  RAM #2
    fc20-ffff Color  RAM #2

    I/O

    read:
    00  IN0
    01  IN1
    03  DSW
    04  IN2

    write:
    00  SN76496 #1
    01  SN76496 #2
    02  SN76496 #3
    03  MSM5205 address write
    04  bit 0   = NMI enable
        bit 1   = flipscreen
        bit 2-3 = ?
        bit 4-5 = priority
        bit 6   = bank rom select
        bit 7   = ?
    05  horizontal scroll ??

    Credits:
    - Tatsuyuki Satoh: MAME driver

    16.08.2004 - TS - Added 'Robo Wres 2001' (see details below)

****************************************************************************

    Robo Wres 2001
    Sega, (198x, possibly 1986?)

    Top Board
    =========
    PCB No: 834-5990 SEGA 1986
    CPU   : NEC D315-5179 (Z80?)
    SOUND : OKI MSM5205 + Resonator 384kHz, SN76489 (x3)
    RAM   : MB8128 (x1)
    OTHER : Volume Pot (x2, labelled VOICE and SOUND)
    PALs  : (x1, near EPR-7542.15D, labelled 315-5056)
    PROMs : (x1, near EPR-7543.12B, labelled PR7571)
    DIPSW : 8 position (x1)
    DIPSW Info:

                    1   2   3   4   5   6   7   8
-----------------------------------------------------------------------------------
Coin1
    1Coin 1Credit   OFF OFF OFF
    2Coin 1Credit   ON  OFF OFF
    3Coin 1Credit   OFF ON  OFF
    4Coin 1Credit   ON  ON  OFF
    1Coin 2Credit   OFF OFF ON
    1Coin 3Credit   ON  OFF ON
    1Coin 4Credit   OFF ON  ON
    2Coin 3Credit   ON  ON  ON
-----------------------------------------------------------------------------------
Coin2
    1Coin 1Credit               OFF OFF
    1Coin 2Credit               ON  OFF
    2Coin 1Credit               OFF ON
    3Coin 1Credit               ON  ON
-----------------------------------------------------------------------------------
Demo Sound
    Off                                 OFF
    On                                  ON
-----------------------------------------------------------------------------------
Not Used                                    OFF
-----------------------------------------------------------------------------------
Language
    Japanese                                    OFF
    English                                     ON
-----------------------------------------------------------------------------------


    PCB Edge Connector Pinout
    -------------------------

    Parts               Solder
    Side                Side
    --------------------------------
    GND         A  1    GND
    GND         B  2    GND
    COIN 2      C  3    -
    1P HOLD     D  4    2P HOLD
    1P PUNCH    E  5    2P PUNCH
    -           F  6    -
    1P START    H  7    2P START
    1P KICK     J  8    2P KICK
    1P UP       K  9    2P UP
    1P DOWN     L  10   2P DOWN
    1P LEFT     M  11   2P LEFT
    1P RIGHT    N  12   2P RIGHT
    COUNTER 1   P  13   COIN 1
    COUNTER 2   R  14   SERVICE
    RED         S  15   GREEN
    BLUE        T  16   SYNC
    -           U  17   -
    +5          V  18   +5
    +5          W  19   +5
    +12         X  20   +12
    SPEAKER 1   Y  21   SPEAKER 2
    SPEAKER GND Z  22   SPEAKER GND


    Controls via 8-way Joystick and 3 buttons (Punch, Hold, Kick)



                    Byte
    ROMs  :         C'sum (for Mike ;-)
    ----------------------------------------
    EPR-7540.13D    27C256  6A13h
    EPR-7541.14D      " 2723h
    EPR-7542.15D      " 01E7h
    EPR-7543.12B      " 6558h

    PR7571.10A      82s123  0A9Fh


    Lower Board
    ===========

    PCB No: 837-5992
    XTAL  : 18.432MHz
    RAM   : MB8128 (x2), SONY CXK5813D-55 (x2)
    PALs  : (x3, labelled 315-5054, 315-5053, 315-5203)
    PROMs : (x2, one near EPR-7547.7D labelled PR7572, one near EPR-7544.7H labelled PR7573)

                        Byte
    ROMs  :             C'Sum
    -------------------------------------
    EPR-7544.7H    27C256  \    ED1Ah
    EPR-7545.6H      "      |   E6CAh
    EPR-7546.5H      "      |   55EAh
    EPR-7547.7D      "      | Gfx   EA3Fh
    EPR-7548.6D      "      |   5D1Fh
    EPR-7549.5D      "     /    16D1h

    PR7572.7F      82s129       06F1h  \ Both have
    PR7573.7G      82s129       06F1h  / identical contents

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/appoooh.h"
#include "machine/segacrp2.h"
#include "sound/sn76496.h"


WRITE_LINE_MEMBER(appoooh_state::adpcm_int)
{
	if (m_adpcm_address != 0xffffffff)
	{
		if (m_adpcm_data == 0xffffffff)
		{
			UINT8 *RAM = memregion("adpcm")->base();

			m_adpcm_data = RAM[m_adpcm_address++];
			m_msm->data_w(m_adpcm_data >> 4);

			if (m_adpcm_data == 0x70)
			{
				m_adpcm_address = 0xffffffff;
				m_msm->reset_w(1);
			}
		}
		else
		{
			m_msm->data_w(m_adpcm_data & 0x0f);
			m_adpcm_data = -1;
		}
	}
}

/* adpcm address write */
WRITE8_MEMBER(appoooh_state::adpcm_w)
{
	m_adpcm_address = data << 8;
	m_msm->reset_w(0);
	m_adpcm_data = 0xffffffff;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, appoooh_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xdfff) AM_ROMBANK("bank1")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM
	AM_RANGE(0xe800, 0xefff) AM_RAM /* RAM ? */

	AM_RANGE(0xf000, 0xf01f) AM_SHARE("spriteram")
	AM_RANGE(0xf020, 0xf3ff) AM_WRITE(fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0xf420, 0xf7ff) AM_WRITE(fg_colorram_w) AM_SHARE("fg_colorram")
	AM_RANGE(0xf800, 0xf81f) AM_SHARE("spriteram_2")
	AM_RANGE(0xf820, 0xfbff) AM_WRITE(bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xfc20, 0xffff) AM_WRITE(bg_colorram_w) AM_SHARE("bg_colorram")
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, appoooh_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_SHARE("decrypted_opcodes")
	AM_RANGE(0x8000, 0x9fff) AM_ROM AM_REGION("maincpu", 0x8000)
	AM_RANGE(0xa000, 0xdfff) AM_ROMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_portmap, AS_IO, 8, appoooh_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1") AM_DEVWRITE("sn1", sn76489_device, write)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("P2") AM_DEVWRITE("sn2", sn76489_device, write)
	AM_RANGE(0x02, 0x02) AM_DEVWRITE("sn3", sn76489_device, write)
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW1") AM_WRITE(adpcm_w)
	AM_RANGE(0x04, 0x04) AM_READ_PORT("BUTTON3") AM_WRITE(out_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(scroll_w) /* unknown */
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( appoooh )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_START("BUTTON3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* probably unused */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( robowres )
	PORT_INCLUDE( appoooh )

	PORT_MODIFY("DSW1")
	PORT_DIPUNUSED_DIPLOC(0x40,0x40, "SW1:7" )          /* Listed as "Unused" */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Language ) )     PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x80, DEF_STR( English ) )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	2048,   /* 2048 characters */
	3,  /* 3 bits per pixel */
	{ 2*2048*8*8, 1*2048*8*8, 0*2048*8*8 }, /* the bitplanes are separated */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 8*8 characters */
	512,    /* 512 characters */
	3,  /* 3 bits per pixel */
	{ 2*2048*8*8, 1*2048*8*8, 0*2048*8*8 }, /* the bitplanes are separated */
	{ 7, 6, 5, 4, 3, 2, 1, 0 ,
		8*8+7,8*8+6,8*8+5,8*8+4,8*8+3,8*8+2,8*8+1,8*8+0},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8    /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( appoooh )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,        0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,     32*8, 32 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,      0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,   32*8, 32 )
GFXDECODE_END


static const gfx_layout robowres_charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 7,6,5,4, 3,2,1,0 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static const gfx_layout robowres_spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3),RGN_FRAC(1,3),RGN_FRAC(0,3) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 ,
		8*8+7,8*8+6,8*8+5,8*8+4,8*8+3,8*8+2,8*8+1,8*8+0},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8    /* every char takes 8 consecutive bytes */
};


static GFXDECODE_START( robowres )
	GFXDECODE_ENTRY( "gfx1", 0, robowres_charlayout,        0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, robowres_charlayout,         0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0, robowres_spritelayout,      0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, robowres_spritelayout,       0, 32 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void appoooh_state::machine_start()
{
	membank("bank1")->configure_entries(0, 2, memregion("maincpu")->base() + 0xa000, 0x6000);
	
	save_item(NAME(m_adpcm_data));
	save_item(NAME(m_adpcm_address));
}


void appoooh_state::machine_reset()
{
	m_adpcm_address = 0xffffffff;
	m_adpcm_data = 0;
	m_scroll_x = 0;
	m_priority = 0;
}

INTERRUPT_GEN_MEMBER(appoooh_state::vblank_irq)
{
	if(m_nmi_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( appoooh_common, appoooh_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,18432000/6) /* ??? the main xtal is 18.432 MHz */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", appoooh_state,  vblank_irq)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn1", SN76489, 18432000/6)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("sn2", SN76489, 18432000/6)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("sn3", SN76489, 18432000/6)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("msm", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(appoooh_state, adpcm_int)) /* interrupt function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S64_4B)  /* 6KHz               */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( appoooh, appoooh_common )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(appoooh_state, screen_update_appoooh)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", appoooh)
	MCFG_PALETTE_ADD("palette", 32*8+32*8)

	MCFG_PALETTE_INIT_OWNER(appoooh_state,appoooh)
	MCFG_VIDEO_START_OVERRIDE(appoooh_state,appoooh)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( robowres, appoooh_common )

	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(appoooh_state, screen_update_robowres)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", robowres)
	MCFG_PALETTE_ADD("palette", 32*8+32*8)

	MCFG_PALETTE_INIT_OWNER(appoooh_state,robowres)
	MCFG_VIDEO_START_OVERRIDE(appoooh_state,appoooh)
MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( appoooh )
	ROM_REGION( 0x14000, "maincpu", 0 ) /* 64k for code + 16k bank */
	ROM_LOAD( "epr-5906.bin", 0x00000, 0x2000, CRC(fffae7fe) SHA1(b4bb60eb6331e503759bd963eafefa69331d6b86) )
	ROM_LOAD( "epr-5907.bin", 0x02000, 0x2000, CRC(57696cd6) SHA1(74a005d18d55fed9ece9b579d2e7e6619a47538b) )
	ROM_LOAD( "epr-5908.bin", 0x04000, 0x2000, CRC(4537cddc) SHA1(ecb71cab7b9269d713399987cbc45ff54735019f) )
	ROM_LOAD( "epr-5909.bin", 0x06000, 0x2000, CRC(cf82718d) SHA1(4408c468a422735ae8f69c03003157782f1a0210) )
	ROM_LOAD( "epr-5910.bin", 0x08000, 0x2000, CRC(312636da) SHA1(18817df6f2e480810726f7b11f289c59e712ee45) )
	ROM_LOAD( "epr-5911.bin", 0x0a000, 0x2000, CRC(0bc2acaa) SHA1(1ae904658ce9e44cdb79f0a13202aaff5c9f9480) ) /* bank0      */
	ROM_LOAD( "epr-5913.bin", 0x0c000, 0x2000, CRC(f5a0e6a7) SHA1(7fad534d1fba52078c4ea580ca7601fdd23cbfa6) ) /* a000-dfff  */
	ROM_LOAD( "epr-5912.bin", 0x10000, 0x2000, CRC(3c3915ab) SHA1(28b501bda992ac06b10dbb5f1f7d6009f2f5f48c) ) /* bank1     */
	ROM_LOAD( "epr-5914.bin", 0x12000, 0x2000, CRC(58792d4a) SHA1(8acdb0ebee5faadadd64bd64db1fdf881ee70333) ) /* a000-dfff */

	ROM_REGION( 0x0c000, "gfx1", 0 )
	ROM_LOAD( "epr-5895.bin", 0x00000, 0x4000, CRC(4b0d4294) SHA1(f9f4d928c76b32cbcbaf7bfd0ebec2d4dfc37566) )   /* playfield #1 chars */
	ROM_LOAD( "epr-5896.bin", 0x04000, 0x4000, CRC(7bc84d75) SHA1(36e98eaac1ba23ab842080205bdb5b76b888ddc2) )
	ROM_LOAD( "epr-5897.bin", 0x08000, 0x4000, CRC(745f3ffa) SHA1(03f5d1d567e786e7835defc6995d1b39aee2c28d) )

	ROM_REGION( 0x0c000, "gfx2", 0 )
	ROM_LOAD( "epr-5898.bin", 0x00000, 0x4000, CRC(cf01644d) SHA1(0cc1b7f7a3b33b0edf4e277e320467b19dfc5bc8) )   /* playfield #2 chars */
	ROM_LOAD( "epr-5899.bin", 0x04000, 0x4000, CRC(885ad636) SHA1(d040948f7cf030e4ab0f0509df23cb855e9c920c) )
	ROM_LOAD( "epr-5900.bin", 0x08000, 0x4000, CRC(a8ed13f3) SHA1(31c4a52fea8f26b4a79564c7e8443a88d43aee12) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "pr5921.prm",   0x0000, 0x020, CRC(f2437229) SHA1(8fb4240142f4c77f820d7c153c22ff82f66aa7b1) )     /* palette */
	ROM_LOAD( "pr5922.prm",   0x0020, 0x100, CRC(85c542bf) SHA1(371d92fca2ae609a47d3a2ea349f14f30b846da8) )     /* charset #1 lookup table */
	ROM_LOAD( "pr5923.prm",   0x0120, 0x100, CRC(16acbd53) SHA1(e5791646730c6232efa2c0327b484472c47baf21) )     /* charset #2 lookup table */

	ROM_REGION( 0xa000, "adpcm", 0 )    /* adpcm voice data */
	ROM_LOAD( "epr-5901.bin", 0x0000, 0x2000, CRC(170a10a4) SHA1(7b0c8427c69525cbcbe9f88b22b12aafb6949bfd) )
	ROM_LOAD( "epr-5902.bin", 0x2000, 0x2000, CRC(f6981640) SHA1(1a93913ecb64d1c459e5bbcc28c4ca3ea90f21e1) )
	ROM_LOAD( "epr-5903.bin", 0x4000, 0x2000, CRC(0439df50) SHA1(1f981c1867366fa57de25ff8f421c121d82d7321) )
	ROM_LOAD( "epr-5904.bin", 0x6000, 0x2000, CRC(9988f2ae) SHA1(f70786a46515feb92fe168fc6c4334ab105c05b2) )
	ROM_LOAD( "epr-5905.bin", 0x8000, 0x2000, CRC(fb5cd70e) SHA1(c2b069ca29b78b845d0c35c7f7452b70c93cb867) )
ROM_END

ROM_START( robowres )
	ROM_REGION( 0x1c000, "maincpu", 0 ) /* 64k for code + 16k bank */
	ROM_LOAD( "epr-7540.13d", 0x00000, 0x8000, CRC(a2a54237) SHA1(06c80fe6725582d19aa957728977e871e79e79e1) )
	ROM_LOAD( "epr-7541.14d", 0x08000, 0x6000, CRC(cbf7d1a8) SHA1(5eb6d2130d4e5401a332df6db5cad07f3131e8e4) )
	ROM_CONTINUE(             0x10000, 0x2000 )
	ROM_LOAD( "epr-7542.15d", 0x14000, 0x8000, CRC(3475fbd4) SHA1(96b28d6492d2e6e8ca9c57abdc5ad4df3777894b) )
	ROM_COPY( "maincpu", 0x16000, 0x10000, 0x4000 )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "epr-7544.7h", 0x000000, 0x8000, CRC(07b846ce) SHA1(6d214fbb43003d2ab35340d5b9fece5f637cadc6) )
	ROM_LOAD( "epr-7545.6h", 0x008000, 0x8000, CRC(e99897be) SHA1(663f32b5290db7ab273e32510583f1aa8d8d4f46) )
	ROM_LOAD( "epr-7546.5h", 0x010000, 0x8000, CRC(1559235a) SHA1(eb0384248f900dcde2d2ca29c58344781dd20500) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "epr-7547.7d", 0x00000, 0x8000, CRC(b87ad4a4) SHA1(6e886a4939de19e0dc3ce702cc70701efd28ddf2) )
	ROM_LOAD( "epr-7548.6d", 0x08000, 0x8000, CRC(8b9c75b3) SHA1(ebc026374aac83b24bf5f6a241d8f15c1e682513) )
	ROM_LOAD( "epr-7549.5d", 0x10000, 0x8000, CRC(f640afbb) SHA1(3aa563866f7160038ce6b1aa3204bd9d286e0a46) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "pr7571.10a",   0x00000, 0x0020, CRC(e82c6d5c) SHA1(de3090bf922171abd1c30f20ca163f387adc60e1) )
	ROM_LOAD( "pr7572.7f",   0x00020, 0x0100, CRC(2b083d0c) SHA1(5b39bd4297bec788caac9e9de5128d43932a24e2) )
	ROM_LOAD( "pr7573.7g",   0x00120, 0x0100, CRC(2b083d0c) SHA1(5b39bd4297bec788caac9e9de5128d43932a24e2) )

	ROM_REGION( 0x8000, "adpcm", 0 )    /* adpcm voice data */
	ROM_LOAD( "epr-7543.12b", 0x00000, 0x8000, CRC(4d108c49) SHA1(a7c3c5a5ad36917ea7f6d917377c2392fa9beea3) )
ROM_END

ROM_START( robowresb )
	ROM_REGION( 0x1c000+0x8000, "maincpu", 0 )  /* 64k for code + 16k bank */
	ROM_LOAD( "dg4.e13",      0x00000, 0x8000, CRC(f7585d4f) SHA1(718879f8262681b6b66968eb49a0fb04fda5160b) )
	ROM_LOAD( "epr-7541.14d", 0x08000, 0x6000, CRC(cbf7d1a8) SHA1(5eb6d2130d4e5401a332df6db5cad07f3131e8e4) )
	ROM_CONTINUE(             0x10000, 0x2000 )
	ROM_LOAD( "epr-7542.15d", 0x14000, 0x8000, CRC(3475fbd4) SHA1(96b28d6492d2e6e8ca9c57abdc5ad4df3777894b) )
	ROM_COPY( "maincpu", 0x16000, 0x10000, 0x4000 )
	ROM_LOAD( "dg1.f13",      0x1c000, 0x8000, CRC(b724968d) SHA1(36618fb81da919d578c2aa1c62d964871903c49f) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "epr-7544.7h", 0x000000, 0x8000, CRC(07b846ce) SHA1(6d214fbb43003d2ab35340d5b9fece5f637cadc6) )
	ROM_LOAD( "epr-7545.6h", 0x008000, 0x8000, CRC(e99897be) SHA1(663f32b5290db7ab273e32510583f1aa8d8d4f46) )
	ROM_LOAD( "epr-7546.5h", 0x010000, 0x8000, CRC(1559235a) SHA1(eb0384248f900dcde2d2ca29c58344781dd20500) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "epr-7547.7d", 0x00000, 0x8000, CRC(b87ad4a4) SHA1(6e886a4939de19e0dc3ce702cc70701efd28ddf2) )
	ROM_LOAD( "epr-7548.6d", 0x08000, 0x8000, CRC(8b9c75b3) SHA1(ebc026374aac83b24bf5f6a241d8f15c1e682513) )
	ROM_LOAD( "epr-7549.5d", 0x10000, 0x8000, CRC(f640afbb) SHA1(3aa563866f7160038ce6b1aa3204bd9d286e0a46) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "pr7571.10a",   0x00000, 0x0020, CRC(e82c6d5c) SHA1(de3090bf922171abd1c30f20ca163f387adc60e1) )
	ROM_LOAD( "pr7572.7f",   0x00020, 0x0100, CRC(2b083d0c) SHA1(5b39bd4297bec788caac9e9de5128d43932a24e2) )
	ROM_LOAD( "pr7573.7g",   0x00120, 0x0100, CRC(2b083d0c) SHA1(5b39bd4297bec788caac9e9de5128d43932a24e2) )

	ROM_REGION( 0x8000, "adpcm", 0 )    /* adpcm voice data */
	ROM_LOAD( "epr-7543.12b", 0x00000, 0x8000, CRC(4d108c49) SHA1(a7c3c5a5ad36917ea7f6d917377c2392fa9beea3) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(appoooh_state,robowres)
{
	static const UINT8 xor_table[128] =
	{
		0x00,0x45,0x41,0x14,0x10,0x55,0x51,0x01,0x04,0x40,0x45,0x11,0x14,0x50,
		0x00,0x05,0x41,0x44,0x10,0x15,0x51,0x54,0x04,
		0x00,0x45,0x41,0x14,0x10,0x55,0x05,0x01,0x44,0x40,0x15,0x11,0x54,0x50,
		0x00,0x05,0x41,0x44,0x10,0x15,0x51,0x01,0x04,
		0x40,0x45,0x11,0x14,0x50,0x55,0x05,0x01,0x44,0x40,0x15,0x11,0x54,0x04,
		0x00,0x45,0x41,0x14,0x50,
		0x00,0x05,0x41,0x44,0x10,0x15,0x51,0x54,0x04,
		0x00,0x45,0x41,0x14,0x50,0x55,0x05,0x01,0x44,0x40,0x15,0x11,0x54,0x50,
		0x00,0x05,0x41,0x44,0x10,0x55,0x51,0x01,0x04,
		0x40,0x45,0x11,0x14,0x50,0x55,0x05,0x01,0x44,0x40,0x15,0x51,0x54,0x04,
		0x00,0x45,0x41,0x14,0x10,0x55,0x51,0x01,0x04,
		0x40,0x45,0x11,0x54,0x50,0x00,0x05,0x41,
	};

	static const int swap_table[128] =
	{
			8, 9,11,13,15, 0, 2, 4, 6,
			8, 9,11,13,15, 1, 2, 4, 6,
			8, 9,11,13,15, 1, 2, 4, 6,
			8, 9,11,13,15, 1, 2, 4, 6,
			8,10,11,13,15, 1, 2, 4, 6,
			8,10,11,13,15, 1, 2, 4, 6,
			8,10,11,13,15, 1, 3, 4, 6,
			8,
			7, 1, 2, 4, 6, 0, 1, 3, 5,
			7, 1, 2, 4, 6, 0, 1, 3, 5,
			7, 1, 2, 4, 6, 0, 2, 3, 5,
			7, 1, 2, 4, 6, 0, 2, 3, 5,
			7, 1, 2, 4, 6, 0, 2, 3, 5,
			7, 1, 3, 4, 6, 0, 2, 3, 5,
			7, 1, 3, 4, 6, 0, 2, 4, 5,
			7,
	};

	sega_decode_2(memregion("maincpu")->base(), m_decrypted_opcodes, xor_table, swap_table);
}

DRIVER_INIT_MEMBER(appoooh_state,robowresb)
{
	memcpy(m_decrypted_opcodes, memregion("maincpu")->base() + 0x1c000, 0x8000);
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1984, appoooh,   0,        appoooh,  appoooh, driver_device,  0,        ROT0, "Sanritsu / Sega", "Appoooh", GAME_SUPPORTS_SAVE )
GAME( 1986, robowres,  0,        robowres, robowres, appoooh_state, robowres, ROT0, "Sanritsu / Sega", "Robo Wres 2001", GAME_SUPPORTS_SAVE )
GAME( 1986, robowresb, robowres, robowres, robowres, appoooh_state, robowresb,ROT0, "bootleg",         "Robo Wres 2001 (bootleg)", GAME_SUPPORTS_SAVE )
