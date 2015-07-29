// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

Side Pocket - (c) 1986 Data East

The original board has an 8751 protection mcu

Ernesto Corvi
ernesto@imagina.com

Thanks must go to Mirko Buffoni for testing the music.

i8751 protection simluation and other fixes by Bryan McPhail, 15/10/00.


ToDo:
 support screen flipping for sprites


Stephh's notes (based on the games M6809 code and some tests) :

1) 'sidepckt'

  - World version.
  - Credits are BCD coded on 1 byte (range 0x00-0x99) at location 0x0007.
  - Bonus lives settings don't match the Dip Switches page : even if the table at 0x40af (4 * 2 words) is good,
    there's a ingame bug in code at 0x4062 :

      4062: CE 40 AF         LDU   #$40AF         U = 40AF
      4065: F6 30 03         LDB   $3003
      4068: 53               COMB
      4069: 54               LSRB
      406A: 54               LSRB
      406B: C4 0C            ANDB  #$0C
      406D: EC C5            LDD   B,U            U still = 40AF
      406F: 33 42            LEAU  $2,U           U ALWAYS = 40B1

    So 2nd and next extra lives are ALWAYS set to 50k+ regardless of the Dip Switches settings !
  - Player 2 controls are never used ingame for player 2 due to extra code at 0x5a35 :

      5A2E: 96 1A            LDA   $1A
      5A30: 84 01            ANDA  #$01           A = 00 for player 1 and 01 for player 2
      5A32: 8E 30 00         LDX   #$3000
      5A35: D6 CA            LDB   $CA            B ALWAYS = 01 due to initialisation of $CA at 0x43f4
      5A37: C5 01            BITB  #$01
      5A39: 26 02            BNE   $5A3D          always jumps to 0x53ad
      5A3B: 30 86            LEAX  A,X            this instruction is NEVER executed
      5A3D: A6 84            LDA   ,X

  - Player 2 controls are also never used for player 2 when entering initials due to extra code at 0x8baf :

      8BAF: 96 1A            LDA   $1A
      8BB1: 84 01            ANDA  #$01           A = 00 for player 1 and 01 for player 2
      8BB3: D6 CA            LDB   $CA            B ALWAYS = 01 due to initialisation of $CA at 0x43f4
      8BB5: 27 02            BEQ   $8BB9          never jumps to 0x8bb9
      8BB7: 86 00            LDA   #$00
      8BB9: 8E 30 00         LDX   #$3000
      8BBC: A6 86            LDA   A,X

  - Screen never flips ingame or in "continue" screen for player 2 due to code at 0x662f :

      662F: 0F 0A            CLR   $0A            $CA = 0
      6631: D6 1A            LDB   $1A
      6633: 27 00            BEQ   $6635          continue regardless of player
      6635: CC 00 00         LDD   #$0000

      75DF: D6 0A            LDB   $0A
      75E1: F7 30 0C         STB   $300C

    Surprinsingly, the screen might flip for player 2 after GAME OVER due to original code at 0x4de8 :

      4DE8: D6 1A            LDB   $1A            A = 00 for player 1 and 01 for player 2
      4DEA: 27 03            BEQ   $4DEF
      4DEC: F7 30 0C         STB   $300C
      4DEF: C6 20            LDB   #$20


2) 'sidepcktj'

  - Japan version.
  - Credits are coded on 1 byte (range 0x00-0xff) at location 0x0007, but their display is limited to 9.
  - Same bonus lives ingame bug as in 'sidepckt'.
  - Player 2 controls are always used ingame for player 2 due to code at 0x58ab :

      58AB: 96 1A            LDA   $1A
      58AD: 84 01            ANDA  #$01           A = 00 for player 1 and 01 for player 2
      58AF: 8E 30 00         LDX   #$3000
      58B2: A6 86            LDA   A,X

  - Player 2 controls are also always used for player 2 when entering initials due to extra code at 0x8b9f :

      8B9F: 96 1A            LDA   $1A
      8BA1: 84 01            ANDA  #$01           A = 00 for player 1 and 01 for player 2
      8BA3: 8E 30 00         LDX   #$3000
      8BA6: A6 86            LDA   A,X

  - Screen always flips ingame or in "continue" screen for player 2 due to code at 0x662f :

      6473: 0F 0A            CLR   $0A            $CA = 0
      6475: D6 1A            LDB   $1A            A = 00 for player 1 and 01 for player 2
      6477: 27 04            BEQ   $647D          jumps if player 1
      6479: 86 20            LDA   #$20
      647B: 97 0A            STA   $0A            $CA = 0x20
      647D: CC 00 00         LDD   #$0000

      75A0: D6 0A            LDB   $0A
      75A2: F7 30 0C         STB   $300C

    After GAME OVER, code at 0x4d16 is slightly different than in 'sidepckt' :

      4D16: D6 1A            LDB   $1A            A = 00 for player 1 and 01 for player 2
      4D18: 27 07            BEQ   $4D21          jumps if player 1
      4D1A: C6 20            LDB   #$20
      4D1C: D7 0A            STB   $0A            $CA = 0x20
      4D1E: F7 30 0C         STB   $300C          flip screen
      4D21: C6 20            LDB   #$20

3) 'sidepcktb'

  - Bootleg heavily based on the World version, so ingame bugs about bonus lives, player 2 inputs and screen flipping are still there.
  - 2 little differences :
      * Lives settings (table at 0x4696) : 06 03 02 instead of 06 03 09
      * Timer settings (table at 0x9d99) : 30 20 18 instead of 40 30 20, so the timer is faster

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m6502/m6502.h"
#include "sound/2203intf.h"
#include "sound/3526intf.h"
#include "includes/sidepckt.h"

// protection tables
static const UINT8 sidepckt_prot_table_1[0x10]={0x05,0x03,0x02,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
static const UINT8 sidepckt_prot_table_2[0x10]={0x8e,0x42,0xad,0x58,0xec,0x85,0xdd,0x4c,0xad,0x9f,0x00,0x4c,0x7e,0x42,0xa2,0xff};
static const UINT8 sidepckt_prot_table_3[0x10]={0xbd,0x73,0x80,0xbd,0x73,0xa7,0xbd,0x73,0xe0,0x7e,0x72,0x56,0xff,0xff,0xff,0xff};

static const UINT8 sidepcktj_prot_table_1[0x10]={0x05,0x03,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
static const UINT8 sidepcktj_prot_table_2[0x10]={0x8e,0x42,0xb2,0x58,0xec,0x85,0xdd,0x4c,0xad,0x9f,0x00,0x4c,0x7e,0x42,0xa7,0xff};
static const UINT8 sidepcktj_prot_table_3[0x10]={0xbd,0x71,0xc8,0xbd,0x71,0xef,0xbd,0x72,0x28,0x7e,0x70,0x9e,0xff,0xff,0xff,0xff};


WRITE8_MEMBER(sidepckt_state::sound_cpu_command_w)
{
	soundlatch_byte_w(space, offset, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

READ8_MEMBER(sidepckt_state::i8751_r)
{
	return m_i8751_return;
}

WRITE8_MEMBER(sidepckt_state::i8751_w)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE); /* i8751 triggers FIRQ on main cpu */

	/* This function takes multiple parameters */
	if (m_in_math == 1)
	{
		m_in_math = 2;
		m_math_param = data;
		m_i8751_return = m_math_param;
	}
	else if (m_in_math == 2)
	{
		m_in_math = 0;
		m_i8751_return = (data) ? (m_math_param / data) : 0;
	}
	else switch (data)
	{
		case 1: /* ID Check */
		case 2: /* Protection data (executable code) */
		case 3: /* Protection data (executable code) */
			m_current_table = data - 1;
			m_current_ptr = 0;
		case 6: /* Read table data */
			m_i8751_return = m_prot_table[m_current_table][m_current_ptr];
			m_current_ptr = (m_current_ptr + 1) & 0x0f;
			break;

		case 4: /* Divide function - multiple parameters */
			m_in_math = 1;
			m_i8751_return = 4;
			break;

		default:
			break;
	}
}

/******************************************************************************/

static ADDRESS_MAP_START( sidepckt_map, AS_PROGRAM, 8, sidepckt_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x13ff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x1400, 0x17ff) AM_RAM // ???
	AM_RANGE(0x1800, 0x1bff) AM_RAM_WRITE(colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x1c00, 0x1fff) AM_RAM // ???
	AM_RANGE(0x2000, 0x20ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x2100, 0x24ff) AM_RAM // ???
	AM_RANGE(0x3000, 0x3000) AM_READ_PORT("P1")
	AM_RANGE(0x3001, 0x3001) AM_READ_PORT("P2")
	AM_RANGE(0x3002, 0x3002) AM_READ_PORT("DSW1")
	AM_RANGE(0x3003, 0x3003) AM_READ_PORT("DSW2")
	AM_RANGE(0x3004, 0x3004) AM_WRITE(sound_cpu_command_w)
	AM_RANGE(0x300c, 0x300c) AM_READNOP AM_WRITE(flipscreen_w)
	AM_RANGE(0x3014, 0x3014) AM_READ(i8751_r)
	AM_RANGE(0x3018, 0x3018) AM_WRITE(i8751_w)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sidepcktb_map, AS_PROGRAM, 8, sidepckt_state )
	AM_RANGE(0x3014, 0x3014) AM_READNOP
	AM_RANGE(0x3018, 0x3018) AM_WRITENOP
	AM_IMPORT_FROM( sidepckt_map )
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, sidepckt_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x1001) AM_DEVWRITE("ym1", ym2203_device, write)
	AM_RANGE(0x2000, 0x2001) AM_DEVWRITE("ym2", ym3526_device, write)
	AM_RANGE(0x3000, 0x3000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


/******************************************************************************/

/* verified from M6809 code */
static INPUT_PORTS_START( sidepckt )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")     /* see notes */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Timer Speed" )               /* table at 0x9d99 */
	PORT_DIPSETTING(    0x00, "Stopped (Cheat)")
	PORT_DIPSETTING(    0x03, "Slow" )                      /* 0x40 - "Normal" in the Dip Switches page */
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )           /* 0x30 - "Bit fast" in the Dip Switches page */
	PORT_DIPSETTING(    0x01, "Fast" )                      /* 0x20 - "Fast" in the Dip Switches page */
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPSETTING(    0x04, "9" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")           /* always gives 6 balls */
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )       /* table at 0x40af (4 * 2 words) - see notes */
	PORT_DIPSETTING(    0x30, "10k 60k 50k+" )              /* "10000, after each 50000" in the Dip Switches page */
	PORT_DIPSETTING(    0x20, "20k 70k 50k+" )              /* "20000, after each 70000" in the Dip Switches page */
	PORT_DIPSETTING(    0x10, "30k 80k 50k+" )              /* "30000, after each 100000" in the Dip Switches page */
//  PORT_DIPSETTING(    0x00, "20k 70k 50k+" )              /* "20000" in the Dip Switches page */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END

/* verified from M6809 code */
static INPUT_PORTS_START( sidepcktj )
	PORT_INCLUDE(sidepckt)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
INPUT_PORTS_END

/* verified from M6809 code */
static INPUT_PORTS_START( sidepcktb )
	PORT_INCLUDE(sidepckt)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Timer Speed" )
	PORT_DIPSETTING(    0x00, "Stopped (Cheat)")
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )           /* 0x30 */
	PORT_DIPSETTING(    0x02, "Fast" )                      /* 0x20 */
	PORT_DIPSETTING(    0x01, "Fastest" )                   /* 0x18 */
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")           /* always gives 6 balls */
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	2048,   /* 2048 characters */
	3,      /* 3 bits per pixel */
	{ 0, 0x8000*8, 0x10000*8 },     /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	1024,   /* 1024 sprites */
	3,      /* 3 bits per pixel */
	{ 0, 0x8000*8, 0x10000*8 },     /* the bitplanes are separated */
	{ 128+0, 128+1, 128+2, 128+3, 128+4, 128+5, 128+6, 128+7, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( sidepckt )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   128,  4 ) /* colors 128-159 */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,   0, 16 ) /* colors   0-127 */
GFXDECODE_END


void sidepckt_state::machine_reset()
{
	m_i8751_return = 0;
	m_current_ptr = 0;
	m_current_table = 0;
	m_in_math = 0;
	m_math_param = 0;
}

static MACHINE_CONFIG_START( sidepckt, sidepckt_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, 2000000) /* 2 MHz */
	MCFG_CPU_PROGRAM_MAP(sidepckt_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sidepckt_state, nmi_line_pulse)

	MCFG_CPU_ADD("audiocpu", M6502, 1500000) /* 1.5 MHz */
	MCFG_CPU_PROGRAM_MAP(sound_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58) /* VERIFY: May be 55 or 56 */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */ )
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(sidepckt_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sidepckt)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(sidepckt_state, sidepckt)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ym2", YM3526, 3000000)
	MCFG_YM3526_IRQ_HANDLER(DEVWRITELINE("audiocpu", m6502_device, irq_line))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sidepcktb, sidepckt )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sidepcktb_map)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( sidepckt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dh00",         0x00000, 0x10000, CRC(251b316e) SHA1(c777d87621b8fefe0e33156be03da8aed733db9a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dh04.bin",     0x08000, 0x8000, CRC(d076e62e) SHA1(720ff1a6a58697b4a9c7c4f31c24a2cf8a04900a) )

	ROM_REGION( 0x10000, "mcu", 0 ) //i8751 MCU
	ROM_LOAD( "i8751.mcu",     0x00000, 0x8000, NO_DUMP )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "sp_07.bin",    0x00000, 0x8000, CRC(9d6f7969) SHA1(583852be0861a89c63ce09eb39146ec379b9e12d) ) /* characters */
	ROM_LOAD( "sp_06.bin",    0x08000, 0x8000, CRC(580e4e43) SHA1(de152a5d4fbc52d80e3eb9af17835ecb6258d45e) )
	ROM_LOAD( "sp_05.bin",    0x10000, 0x8000, CRC(05ab71d2) SHA1(6f06d1d1440a5fb05c01f712457d0bb167e93099) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "dh01.bin",     0x00000, 0x8000, CRC(a2cdfbea) SHA1(0721e538e3306d616f11008f784cf21e679f330d) ) /* sprites */
	ROM_LOAD( "dh02.bin",     0x08000, 0x8000, CRC(eeb5c3e7) SHA1(57eda1cc29124e04fe5025a904634d8ca52c0f12) )
	ROM_LOAD( "dh03.bin",     0x10000, 0x8000, CRC(8e18d21d) SHA1(74f0ddf1fcbed386332eba882b4136295b4f096d) )

	ROM_REGION( 0x0200, "proms", 0 )    /* color PROMs */
	ROM_LOAD( "dh-09.bpr",    0x0000, 0x0100, CRC(ce049b4f) SHA1(e4918cef7b319dd40cf1722eb8bf5e79be04fd6c) )
	ROM_LOAD( "dh-08.bpr",    0x0100, 0x0100, CRC(cdf2180f) SHA1(123215d096f88b66396d40d7a579380d0b5b2b89) )
ROM_END

ROM_START( sidepcktj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dh00.bin",     0x00000, 0x10000, CRC(a66bc28d) SHA1(cd62ce1dce6fe42d9745eec50d11e86b076d28e1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dh04.bin",     0x08000, 0x8000, CRC(d076e62e) SHA1(720ff1a6a58697b4a9c7c4f31c24a2cf8a04900a) )

	ROM_REGION( 0x10000, "mcu", 0 ) //i8751 MCU
	ROM_LOAD( "i8751.mcu",     0x00000, 0x8000, NO_DUMP )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "dh07.bin",     0x00000, 0x8000, CRC(7d0ce858) SHA1(3a158f218a762e6841d2611f41ace67a1afefb35) ) /* characters */
	ROM_LOAD( "dh06.bin",     0x08000, 0x8000, CRC(b86ddf72) SHA1(7596dd1b646971d8df1bc4fd157ccf161a712d59) )
	ROM_LOAD( "dh05.bin",     0x10000, 0x8000, CRC(df6f94f2) SHA1(605796191f37cb76d496aa459243655070bb90c0) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "dh01.bin",     0x00000, 0x8000, CRC(a2cdfbea) SHA1(0721e538e3306d616f11008f784cf21e679f330d) ) /* sprites */
	ROM_LOAD( "dh02.bin",     0x08000, 0x8000, CRC(eeb5c3e7) SHA1(57eda1cc29124e04fe5025a904634d8ca52c0f12) )
	ROM_LOAD( "dh03.bin",     0x10000, 0x8000, CRC(8e18d21d) SHA1(74f0ddf1fcbed386332eba882b4136295b4f096d) )

	ROM_REGION( 0x0200, "proms", 0 )    /* color PROMs */
	ROM_LOAD( "dh-09.bpr",    0x0000, 0x0100, CRC(ce049b4f) SHA1(e4918cef7b319dd40cf1722eb8bf5e79be04fd6c) )
	ROM_LOAD( "dh-08.bpr",    0x0100, 0x0100, CRC(cdf2180f) SHA1(123215d096f88b66396d40d7a579380d0b5b2b89) )
ROM_END

ROM_START( sidepcktb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sp_09.bin",    0x04000, 0x4000, CRC(3c6fe54b) SHA1(4025ac48d75f171f4c979d3fcd6a2f8da18cef4f) )
	ROM_LOAD( "sp_08.bin",    0x08000, 0x8000, CRC(347f81cd) SHA1(5ab06130f35788e51a881cc0f387649532145bd6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dh04.bin",     0x08000, 0x8000, CRC(d076e62e) SHA1(720ff1a6a58697b4a9c7c4f31c24a2cf8a04900a) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "sp_07.bin",    0x00000, 0x8000, CRC(9d6f7969) SHA1(583852be0861a89c63ce09eb39146ec379b9e12d) ) /* characters */
	ROM_LOAD( "sp_06.bin",    0x08000, 0x8000, CRC(580e4e43) SHA1(de152a5d4fbc52d80e3eb9af17835ecb6258d45e) )
	ROM_LOAD( "sp_05.bin",    0x10000, 0x8000, CRC(05ab71d2) SHA1(6f06d1d1440a5fb05c01f712457d0bb167e93099) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "dh01.bin",     0x00000, 0x8000, CRC(a2cdfbea) SHA1(0721e538e3306d616f11008f784cf21e679f330d) ) /* sprites */
	ROM_LOAD( "dh02.bin",     0x08000, 0x8000, CRC(eeb5c3e7) SHA1(57eda1cc29124e04fe5025a904634d8ca52c0f12) )
	ROM_LOAD( "dh03.bin",     0x10000, 0x8000, CRC(8e18d21d) SHA1(74f0ddf1fcbed386332eba882b4136295b4f096d) )

	ROM_REGION( 0x0200, "proms", 0 )    /* color PROMs */
	ROM_LOAD( "dh-09.bpr",    0x0000, 0x0100, CRC(ce049b4f) SHA1(e4918cef7b319dd40cf1722eb8bf5e79be04fd6c) )
	ROM_LOAD( "dh-08.bpr",    0x0100, 0x0100, CRC(cdf2180f) SHA1(123215d096f88b66396d40d7a579380d0b5b2b89) )
ROM_END


DRIVER_INIT_MEMBER(sidepckt_state,sidepckt)
{
	m_prot_table[0] = sidepckt_prot_table_1;
	m_prot_table[1] = sidepckt_prot_table_2;
	m_prot_table[2] = sidepckt_prot_table_3;

	save_item(NAME(m_i8751_return));
	save_item(NAME(m_current_ptr));
	save_item(NAME(m_current_table));
	save_item(NAME(m_in_math));
	save_item(NAME(m_math_param));
}

DRIVER_INIT_MEMBER(sidepckt_state,sidepcktj)
{
	m_prot_table[0] = sidepcktj_prot_table_1;
	m_prot_table[1] = sidepcktj_prot_table_2;
	m_prot_table[2] = sidepcktj_prot_table_3;

	save_item(NAME(m_i8751_return));
	save_item(NAME(m_current_ptr));
	save_item(NAME(m_current_table));
	save_item(NAME(m_in_math));
	save_item(NAME(m_math_param));
}


GAME( 1986, sidepckt,  0,        sidepckt,  sidepckt,  sidepckt_state, sidepckt,  ROT0, "Data East Corporation", "Side Pocket (World)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1986, sidepcktj, sidepckt, sidepckt,  sidepcktj, sidepckt_state, sidepcktj, ROT0, "Data East Corporation", "Side Pocket (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1986, sidepcktb, sidepckt, sidepcktb, sidepcktb, driver_device,  0,         ROT0, "bootleg", "Side Pocket (bootleg)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
