// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

Bomb Jack

driver by Mirko Buffoni

bombjack2 has YOU ARE LUCY instead of LUCKY, so it's probably an older version


MAIN BOARD:

0000-1fff ROM 0
2000-3fff ROM 1
4000-5fff ROM 2
6000-7fff ROM 3
8000-83ff RAM 0
8400-87ff RAM 1
8800-8bff RAM 2
8c00-8fff RAM 3
9000-93ff Video RAM (RAM 4)
9400-97ff Color RAM (RAM 4)
9c00-9cff Palette RAM
c000-dfff ROM 4

memory mapped ports:
read:
b000      IN0
b001      IN1
b002      IN2
b003      watchdog reset?
b004      DSW1
b005      DSW2

write:
9820-987f sprites
9a00      ? number of small sprites for video controller
9e00      background image selector
b000      interrupt enable
b004      flip screen
b800      command to soundboard & trigger NMI on sound board



SOUND BOARD:
0x0000 0x1fff ROM
0x2000 0x43ff RAM

memory mapped ports:
read:
0x6000 command from soundboard
write :
none

IO ports:
write:
0x00 AY#1 control
0x01 AY#1 write
0x10 AY#2 control
0x11 AY#2 write
0x80 AY#3 control
0x81 AY#3 write

interrupts:
NMI triggered by the commands sent by MAIN BOARD (?)
NMI interrupts for music timing


Stephh's notes (based on the game Z80 code and some tests) :

  - "Bonus Life" Dip Switches NEVER give extra lives because of patched code at 0x5a07 :
    "push af" instruction (0xe5) has been replaced by "ret" instruction (0xc9).
    However, they still affect hi-scores table due to code at 0x0945 :

    value     bonus life       hi-scores

     0x00   none                 10000
     0x01   every 100k          100000
     0x02   every 30k            30000
     0x03   50k only             50000
     0x04   100k only           100000
     0x05   50k and 100k         50000
     0x06   100k and 300k       100000
     0x07   50k 100k and 300k    50000

  - Ingame bug : if game is reset when screen is flipped, the screen remains
    flipped for the start-up tests and we'll be OK when scores are displayed.

  - The only difference between 'bombjack' and 'bombjack2' is that 'bombjack'
    fixes the message when you get a 'S' for extra credit (text at 0xd24a) :
      * 'bombjack' : "YOU ARE LUCKY"
      * 'bombjack2' : "YOU ARE LUCY"

2008-07
Dip Locations and factory settings verified with manual

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/bombjack.h"


TIMER_CALLBACK_MEMBER(bombjack_state::soundlatch_callback)
{
	m_latch = param;
}

WRITE8_MEMBER(bombjack_state::bombjack_soundlatch_w)
{
	/* make all the CPUs synchronize, and only AFTER that write the new command to the latch */
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(bombjack_state::soundlatch_callback),this), data);
}

READ8_MEMBER(bombjack_state::bombjack_soundlatch_r)
{
	int res;

	res = m_latch;
	m_latch = 0;
	return res;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

WRITE8_MEMBER(bombjack_state::irq_mask_w)
{
	m_nmi_mask = data & 1;
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, bombjack_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(bombjack_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9400, 0x97ff) AM_RAM_WRITE(bombjack_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x9820, 0x987f) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0x9a00, 0x9a00) AM_WRITENOP
	AM_RANGE(0x9c00, 0x9cff) AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x9e00, 0x9e00) AM_WRITE(bombjack_background_w)
	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("P1")
	AM_RANGE(0xb000, 0xb000) AM_WRITE(irq_mask_w)
	AM_RANGE(0xb001, 0xb001) AM_READ_PORT("P2")
	AM_RANGE(0xb002, 0xb002) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xb003, 0xb003) AM_READNOP /* watchdog reset? */
	AM_RANGE(0xb004, 0xb004) AM_READ_PORT("DSW1")
	AM_RANGE(0xb004, 0xb004) AM_WRITE(bombjack_flipscreen_w)
	AM_RANGE(0xb005, 0xb005) AM_READ_PORT("DSW2")
	AM_RANGE(0xb800, 0xb800) AM_WRITE(bombjack_soundlatch_w)
	AM_RANGE(0xc000, 0xdfff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( audio_map, AS_PROGRAM, 8, bombjack_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_READ(bombjack_soundlatch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( audio_io_map, AS_IO, 8, bombjack_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x10, 0x11) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0x80, 0x81) AM_DEVWRITE("ay3", ay8910_device, address_data_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( bombjack )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* probably unused */

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* probably unused */

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	/* Manual states DSW2 bits 0-2 are unused and have to be left on OFF (0x00) */
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!1,!2,!3")  /* see notes */
	PORT_DIPSETTING(    0x02, "Every 30k" )
	PORT_DIPSETTING(    0x01, "Every 100k" )
	PORT_DIPSETTING(    0x07, "50k, 100k and 300k" )
	PORT_DIPSETTING(    0x05, "50k and 100k" )
	PORT_DIPSETTING(    0x03, "50k only" )
	PORT_DIPSETTING(    0x06, "100k and 300k" )
	PORT_DIPSETTING(    0x04, "100k only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x18, 0x10, "Bird Speed" ) PORT_DIPLOCATION("SW2:!4,!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x60, 0x40, "Enemies Number & Speed" ) PORT_DIPLOCATION("SW2:!6,!7")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, "Special Coin" ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout1 =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,3),    /* 512 characters */
	3,  /* 3 bits per pixel */
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },  /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, /* pretty straightforward layout */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout charlayout2 =
{
	16,16,  /* 16*16 characters */
	RGN_FRAC(1,3),    /* 256 characters */
	3,  /* 3 bits per pixel */
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },    /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,   /* pretty straightforward layout */
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8    /* every character takes 32 consecutive bytes */
};

static const gfx_layout spritelayout1 =
{
	16,16,  /* 16*16 sprites */
	128,    /* 128 sprites */
	3,  /* 3 bits per pixel */
	{ 0, 1024*8*8, 2*1024*8*8 },    /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static const gfx_layout spritelayout2 =
{
	32,32,  /* 32*32 sprites */
	32, /* 32 sprites */
	3,  /* 3 bits per pixel */
	{ 0, 1024*8*8, 2*1024*8*8 },    /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
			40*8+0, 40*8+1, 40*8+2, 40*8+3, 40*8+4, 40*8+5, 40*8+6, 40*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
			64*8, 65*8, 66*8, 67*8, 68*8, 69*8, 70*8, 71*8,
			80*8, 81*8, 82*8, 83*8, 84*8, 85*8, 86*8, 87*8 },
	128*8   /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( bombjack )
	GFXDECODE_ENTRY( "chars",   0x0000, charlayout1,      0, 16 )   /* characters */
	GFXDECODE_ENTRY( "tiles",   0x0000, charlayout2,      0, 16 )   /* background tiles */
	GFXDECODE_ENTRY( "sprites", 0x0000, spritelayout1,    0, 16 )   /* normal sprites */
	GFXDECODE_ENTRY( "sprites", 0x1000, spritelayout2,    0, 16 )   /* large sprites */
GFXDECODE_END




/*************************************
 *
 *  Machine driver
 *
 *************************************/

void bombjack_state::machine_start()
{
	save_item(NAME(m_latch));
	save_item(NAME(m_background_image));
}


void bombjack_state::machine_reset()
{
	m_latch = 0;
	m_background_image = 0;
}


INTERRUPT_GEN_MEMBER(bombjack_state::vblank_irq)
{
	if(m_nmi_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( bombjack, bombjack_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_4MHz)     /* Confirmed from PCB */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", bombjack_state,  vblank_irq)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_12MHz/4) /* Confirmed from PCB */
	MCFG_CPU_PROGRAM_MAP(audio_map)
	MCFG_CPU_IO_MAP(audio_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", bombjack_state,  nmi_line_pulse)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(bombjack_state, screen_update_bombjack)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT90)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", bombjack)
	MCFG_PALETTE_ADD("palette", 128)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_12MHz/8) /* Confirmed from PCB */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.13)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_12MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.13)

	MCFG_SOUND_ADD("ay3", AY8910, XTAL_12MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.13)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( bombjack )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "09_j01b.bin",  0x0000, 0x2000, CRC(c668dc30) SHA1(51dd6a2688b42e9f28f0882bd76f75be7ec3222a) )
	ROM_LOAD( "10_l01b.bin",  0x2000, 0x2000, CRC(52a1e5fb) SHA1(e1cdc4b4efbc6c7a1e4fa65019486617f2acba1b) )
	ROM_LOAD( "11_m01b.bin",  0x4000, 0x2000, CRC(b68a062a) SHA1(43bae56494ac0202aaa8f1ed5c1ed1bff775b2b8) )
	ROM_LOAD( "12_n01b.bin",  0x6000, 0x2000, CRC(1d3ecee5) SHA1(8b3c49e21ea4952cae7042890d1be2115f7d6fda) )
	ROM_LOAD( "13.1r",        0xc000, 0x2000, CRC(70e0244d) SHA1(67654155e42821ea78a655f869fb81c8d6387f63) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound board */
	ROM_LOAD( "01_h03t.bin",  0x0000, 0x2000, CRC(8407917d) SHA1(318face9f7a7ab6c7eeac773995040425e780aaf) )

	ROM_REGION( 0x3000, "chars", 0 )
	ROM_LOAD( "03_e08t.bin",  0x0000, 0x1000, CRC(9f0470d5) SHA1(94ef52ef47b4399a03528fe3efeac9c1d6983446) )    /* chars */
	ROM_LOAD( "04_h08t.bin",  0x1000, 0x1000, CRC(81ec12e6) SHA1(e29ba193f21aa898499187603b25d2e226a07c7b) )
	ROM_LOAD( "05_k08t.bin",  0x2000, 0x1000, CRC(e87ec8b1) SHA1(a66808ef2d62fca2854396898b86bac9be5f17a3) )

	ROM_REGION( 0x6000, "tiles", 0 )
	ROM_LOAD( "06_l08t.bin",  0x0000, 0x2000, CRC(51eebd89) SHA1(515128a3971fcb97b60c5b6bdd2b03026aec1921) )    /* background tiles */
	ROM_LOAD( "07_n08t.bin",  0x2000, 0x2000, CRC(9dd98e9d) SHA1(6db6006a6e20ff7c243d88293ca53681c4703ea5) )
	ROM_LOAD( "08_r08t.bin",  0x4000, 0x2000, CRC(3155ee7d) SHA1(e7897dca4c145f10b7d975b8ef0e4d8aa9354c25) )

	ROM_REGION( 0x6000, "sprites", 0 )
	ROM_LOAD( "16_m07b.bin",  0x0000, 0x2000, CRC(94694097) SHA1(de71bcd67f97d05527f2504fc8430be333fb9ec2) )    /* sprites */
	ROM_LOAD( "15_l07b.bin",  0x2000, 0x2000, CRC(013f58f2) SHA1(20c64593ab9fcb04cefbce0cd5d17ce3ff26441b) )
	ROM_LOAD( "14_j07b.bin",  0x4000, 0x2000, CRC(101c858d) SHA1(ed1746c15cdb04fae888601d940183d5c7702282) )

	ROM_REGION( 0x1000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "02_p04t.bin",  0x0000, 0x1000, CRC(398d4a02) SHA1(ac18a8219f99ba9178b96c9564de3978e39c59fd) )
ROM_END

ROM_START( bombjack2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "09_j01b.bin",  0x0000, 0x2000, CRC(c668dc30) SHA1(51dd6a2688b42e9f28f0882bd76f75be7ec3222a) )
	ROM_LOAD( "10_l01b.bin",  0x2000, 0x2000, CRC(52a1e5fb) SHA1(e1cdc4b4efbc6c7a1e4fa65019486617f2acba1b) )
	ROM_LOAD( "11_m01b.bin",  0x4000, 0x2000, CRC(b68a062a) SHA1(43bae56494ac0202aaa8f1ed5c1ed1bff775b2b8) )
	ROM_LOAD( "12_n01b.bin",  0x6000, 0x2000, CRC(1d3ecee5) SHA1(8b3c49e21ea4952cae7042890d1be2115f7d6fda) )
	ROM_LOAD( "13_r01b.bin",  0xc000, 0x2000, CRC(bcafdd29) SHA1(d243eb1249e885aa75fc910fce6e7744770d6e82) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound board */
	ROM_LOAD( "01_h03t.bin",  0x0000, 0x2000, CRC(8407917d) SHA1(318face9f7a7ab6c7eeac773995040425e780aaf) )

	ROM_REGION( 0x3000, "chars", 0 )
	ROM_LOAD( "03_e08t.bin",  0x0000, 0x1000, CRC(9f0470d5) SHA1(94ef52ef47b4399a03528fe3efeac9c1d6983446) )    /* chars */
	ROM_LOAD( "04_h08t.bin",  0x1000, 0x1000, CRC(81ec12e6) SHA1(e29ba193f21aa898499187603b25d2e226a07c7b) )
	ROM_LOAD( "05_k08t.bin",  0x2000, 0x1000, CRC(e87ec8b1) SHA1(a66808ef2d62fca2854396898b86bac9be5f17a3) )

	ROM_REGION( 0x6000, "tiles", 0 )
	ROM_LOAD( "06_l08t.bin",  0x0000, 0x2000, CRC(51eebd89) SHA1(515128a3971fcb97b60c5b6bdd2b03026aec1921) )    /* background tiles */
	ROM_LOAD( "07_n08t.bin",  0x2000, 0x2000, CRC(9dd98e9d) SHA1(6db6006a6e20ff7c243d88293ca53681c4703ea5) )
	ROM_LOAD( "08_r08t.bin",  0x4000, 0x2000, CRC(3155ee7d) SHA1(e7897dca4c145f10b7d975b8ef0e4d8aa9354c25) )

	ROM_REGION( 0x6000, "sprites", 0 )
	ROM_LOAD( "16_m07b.bin",  0x0000, 0x2000, CRC(94694097) SHA1(de71bcd67f97d05527f2504fc8430be333fb9ec2) )    /* sprites */
	ROM_LOAD( "15_l07b.bin",  0x2000, 0x2000, CRC(013f58f2) SHA1(20c64593ab9fcb04cefbce0cd5d17ce3ff26441b) )
	ROM_LOAD( "14_j07b.bin",  0x4000, 0x2000, CRC(101c858d) SHA1(ed1746c15cdb04fae888601d940183d5c7702282) )

	ROM_REGION( 0x1000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "02_p04t.bin",  0x0000, 0x1000, CRC(398d4a02) SHA1(ac18a8219f99ba9178b96c9564de3978e39c59fd) )
ROM_END



ROM_START( bombjackt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9.1j",    0x0000, 0x4000, CRC(4b59a3bb) SHA1(dae45985d2575821c86757ead14b8313e922570d) ) // == 09_j01b.bin + 10_l01b.bin
	ROM_LOAD( "12.1n",   0x4000, 0x4000, CRC(0a32506a) SHA1(2fb3ce695caebbae3ca7dd9f3d34ac5b734d77ed) ) // == 11_m01b.bin + (97.229004%) 12_n01b.bin
	ROM_LOAD( "13.1r",   0xc000, 0x2000, CRC(964ac5c5) SHA1(8d235ae91aea1ae86411671c5aa050c146a52026) ) // sldh - (99.877930%) 13_r01b.bin

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound board */
	ROM_LOAD( "1.6h",  0x0000, 0x2000, CRC(8407917d) SHA1(318face9f7a7ab6c7eeac773995040425e780aaf) )

	/*
	ROM_REGION( 0x3000, "chars", 0 )
	ROM_LOAD( "03_e08t.bin",  0x0000, 0x1000, CRC(9f0470d5) SHA1(94ef52ef47b4399a03528fe3efeac9c1d6983446) )
	ROM_LOAD( "04_h08t.bin",  0x1000, 0x1000, CRC(81ec12e6) SHA1(e29ba193f21aa898499187603b25d2e226a07c7b) )
	ROM_LOAD( "05_k08t.bin",  0x2000, 0x1000, CRC(e87ec8b1) SHA1(a66808ef2d62fca2854396898b86bac9be5f17a3) )
	*/

	ROM_REGION( 0x6000, "chars", 0 ) // the Tecfri produced boards apparently use double size roms here (content duplicated in each half)
	ROM_LOAD( "3.1e",  0x0000, 0x2000, CRC(54e1dac1) SHA1(3c5d8b932b2a87acf42e0b4632195776689c1154) )
	ROM_LOAD( "4.1h",  0x2000, 0x2000, CRC(05e428ab) SHA1(0b2cae76aba8372482a4e315a9f49fd15cb94625) )
	ROM_LOAD( "5.1k",  0x4000, 0x2000, CRC(f282f29a) SHA1(521a110213d6ecdf54be0f50f41c3c266d65d84c) )



	ROM_REGION( 0x6000, "tiles", 0 ) // ok
	ROM_LOAD( "6.1l",  0x0000, 0x2000, CRC(51eebd89) SHA1(515128a3971fcb97b60c5b6bdd2b03026aec1921) )    /* background tiles */
	ROM_LOAD( "7.1n",  0x2000, 0x2000, CRC(9dd98e9d) SHA1(6db6006a6e20ff7c243d88293ca53681c4703ea5) )
	ROM_LOAD( "8.1r",  0x4000, 0x2000, CRC(3155ee7d) SHA1(e7897dca4c145f10b7d975b8ef0e4d8aa9354c25) )

	ROM_REGION( 0x6000, "sprites", 0 ) // ok
	ROM_LOAD( "16.7m",  0x0000, 0x2000, CRC(94694097) SHA1(de71bcd67f97d05527f2504fc8430be333fb9ec2) )    /* sprites */
	ROM_LOAD( "15.7k",  0x2000, 0x2000, CRC(013f58f2) SHA1(20c64593ab9fcb04cefbce0cd5d17ce3ff26441b) )
	ROM_LOAD( "14.7j",  0x4000, 0x2000, CRC(101c858d) SHA1(ed1746c15cdb04fae888601d940183d5c7702282) )

	ROM_REGION( 0x2000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "2.5n",  0x0000, 0x2000, CRC(de796158) SHA1(e004f10ada5c282f3b4208031e274190a54bf94f) ) // 1xxxxxxxxxxxx = 0xFF (double size, second half empty, otherwise the same)
ROM_END

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1984, bombjack,  0,        bombjack, bombjack, driver_device, 0, ROT90, "Tehkan", "Bomb Jack (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, bombjack2, bombjack, bombjack, bombjack, driver_device, 0, ROT90, "Tehkan", "Bomb Jack (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, bombjackt, bombjack, bombjack, bombjack, driver_device, 0, ROT90, "Tehkan (Tecfri licence)", "Bomb Jack (Tecfri, Spain)", MACHINE_SUPPORTS_SAVE ) // official licence
