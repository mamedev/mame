/* Puckman Pockimon
  -- original driver by Luca Elia

Seems to be based around genesis hardware, despite containing no original Sega chips

Supported:

Puckman Pockimon - (c)2000 Genie? (there should be a way to show Sun Mixing copyright, roms are the same
                                   on a version with the SM (c)

*/

#include "driver.h"
#include "genesis.h"

#define MASTER_CLOCK		53693100

static UINT16* main_ram;

static INPUT_PORTS_START( puckpkmn ) /* Puckman Pockimon Input Ports */
	PORT_START_TAG("P2")	/* Player 2 Controls ($700011.b) */
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(  0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START_TAG("P1")	/* Player 1 Controls ($700013.b) */
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(10)
	PORT_BIT(  0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START_TAG("IN0")	/* ? ($700015.b) */

	PORT_START_TAG("DSW1")	/* DSW 1 ($700017.b) */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x28, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x38, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x28, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy )    )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal )  )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard )    )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START_TAG("DSW2")	/* DSW 1 ($700019.b) */
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static READ16_HANDLER( puckpkmn_YM3438_r )
{
	return	YM3438_status_port_0_A_r(machine, 0) << 8;
}

static WRITE16_HANDLER( puckpkmn_YM3438_w )
{
	switch (offset)
	{
		case 0:
			if (ACCESSING_BITS_8_15)	YM3438_control_port_0_A_w	(machine, 0,	(data >> 8) & 0xff);
			else 				YM3438_data_port_0_A_w		(machine, 0,	(data >> 0) & 0xff);
			break;
		case 1:
			if (ACCESSING_BITS_8_15)	YM3438_control_port_0_B_w	(machine, 0,	(data >> 8) & 0xff);
			else 				YM3438_data_port_0_B_w		(machine, 0,	(data >> 0) & 0xff);
			break;
	}
}


static ADDRESS_MAP_START( puckpkmn_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_READ(SMH_ROM)					/* Main 68k Program Roms */
	AM_RANGE(0x700010, 0x700011) AM_READ(input_port_0_word_r)		/* Input (P2) */
	AM_RANGE(0x700012, 0x700013) AM_READ(input_port_1_word_r)		/* Input (P1) */
	AM_RANGE(0x700014, 0x700015) AM_READ(input_port_2_word_r)		/* Input (?) */
	AM_RANGE(0x700016, 0x700017) AM_READ(input_port_3_word_r)		/* Input (DSW1) */
	AM_RANGE(0x700018, 0x700019) AM_READ(input_port_4_word_r)		/* Input (DSW2) */
	AM_RANGE(0x700022, 0x700023) AM_READ(OKIM6295_status_0_lsb_r)	/* M6295 Sound Chip Status Register */
	AM_RANGE(0xa04000, 0xa04001) AM_READ(puckpkmn_YM3438_r)			/* Ym3438 Sound Chip Status Register */
	AM_RANGE(0xc00000, 0xc0001f) AM_READ(genesis_vdp_r)				/* VDP Access */
	AM_RANGE(0xe00000, 0xe1ffff) AM_READ(SMH_BANK1)				/* VDP sees the roms here */
	AM_RANGE(0xfe0000, 0xfeffff) AM_READ(SMH_BANK2)				/* VDP sees the ram here */
	AM_RANGE(0xff0000, 0xffffff) AM_READ(SMH_RAM	)					/* Main Ram */

	/* Unknown reads: */
//  AM_RANGE(0xa10000, 0xa10001) AM_READ(SMH_NOP)                 /* ? once */
	AM_RANGE(0xa10002, 0xa10005) AM_READ(SMH_NOP)					/* ? alternative way of reading inputs ? */
	AM_RANGE(0xa11100, 0xa11101) AM_READ(SMH_NOP)					/* ? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( puckpkmn_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_WRITE(SMH_ROM)					/* Main 68k Program Roms */
	AM_RANGE(0x700022, 0x700023) AM_WRITE(OKIM6295_data_0_lsb_w)		/* M6295 Sound Chip Writes */
	AM_RANGE(0xa04000, 0xa04003) AM_WRITE(puckpkmn_YM3438_w)			/* Ym3438 Sound Chip Writes */
	AM_RANGE(0xc00000, 0xc0001f) AM_WRITE(genesis_vdp_w)				/* VDP Access */
	AM_RANGE(0xff0000, 0xffffff) AM_WRITE(SMH_RAM) AM_BASE(&main_ram)		/* Main Ram */

	/* Unknown writes: */
	AM_RANGE(0xa00000, 0xa00551) AM_WRITE(SMH_RAM)					/* ? */
	AM_RANGE(0xa10002, 0xa10005) AM_WRITE(SMH_NOP)					/* ? alternative way of reading inputs ? */
//  AM_RANGE(0xa10008, 0xa1000d) AM_WRITE(SMH_NOP)                    /* ? once */
//  AM_RANGE(0xa14000, 0xa14003) AM_WRITE(SMH_NOP)                    /* ? once */
	AM_RANGE(0xa11100, 0xa11101) AM_WRITE(SMH_NOP)					/* ? */
	AM_RANGE(0xa11200, 0xa11201) AM_WRITE(SMH_NOP)					/* ? */
ADDRESS_MAP_END


static const struct YM3438interface ym3438_intf =
{
	genesis_irq2_interrupt		/* IRQ handler */
};

static MACHINE_DRIVER_START( puckpkmn )

	/* basic machine hardware */
	MDRV_CPU_ADD("main",M68000, MASTER_CLOCK/7) 		/*???*/
	MDRV_CPU_PROGRAM_MAP(puckpkmn_readmem,puckpkmn_writemem)
	MDRV_CPU_VBLANK_INT("main", genesis_vblank_interrupt)

	MDRV_MACHINE_START(genesis)
	MDRV_MACHINE_RESET(genesis)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS | VIDEO_HAS_HIGHLIGHTS)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(342,262)
	MDRV_SCREEN_VISIBLE_AREA(0, 319, 0, 223)
	MDRV_PALETTE_LENGTH(64)

	MDRV_VIDEO_START(genesis)
	MDRV_VIDEO_UPDATE(genesis)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym", YM3438, MASTER_CLOCK/7)
	MDRV_SOUND_CONFIG(ym3438_intf)
	MDRV_SOUND_ROUTE(0, "mono", 0.50)
	MDRV_SOUND_ROUTE(1, "mono", 0.50)

	MDRV_SOUND_ADD("sn", SN76496, MASTER_CLOCK/15)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* sound hardware */
	MDRV_SOUND_ADD("oki", OKIM6295, 1056000)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

/* Genie's Hardware (contains no real sega parts) */

/***************************************************************************
Puckman Pokemon Genie 2000
(c) 2000?  Manufacturer ?

Hardware looks bootleg-ish, but is newly manufactured.

CPU: ? (one of the SMD chips)
SND: OKI6295, U6612 (probably YM3812), U6614B (Probably YM3014B)
XTAL: 3.579545MHz, 4.0000MHz
OSC: 53.693175MHz
Other Chips: Possible CPU: TA-06SD 9933 B816453 128 pin square SMD
             GFX support chips: Y-BOX TA891945 100 pin SMD
                                TV16B 0010 ME251271 160 pin SMD

There are 13 PAL's on the PCB !

RAM: 62256 x 3, D41264 x 2 (ZIP Ram)
DIPS: 2 x 8 position
SW1:
                        1   2   3   4   5   6   7   8
coins   1coin 1 Cred.   off off off
        2c 1c           on  off off
        3c 1c           off on  off
        4c 1c           on  on  off
        5c 1c           off off on
        1c 2c           on  off on
        1c 3c           off on  on
        1c 4c           on  on  on

players 1                           off off off
        2                           on  off off
        3                           off on  off
        4                           on  on  off
        5                           off off on
        6                           on  off on
        7                           off on  on
        8                           on  on  on

diffic-
ulty    v.easy                                  off off
        normal                                  on  off
        diffic.                                 off on
        v. diffic.                              on  on


SW2

note position 3-8 not used

                    1   2   3   4   5   6   7   8
test mode   no      off
            yes     on

demo sound  yes         off
            no          on


ROMS:
PUCKPOKE.U3 M5M27C201   Sound
PUCKPOKE.U4 27C040--\
PUCKPOKE.U5 27C040---\
PUCKPOKE.U7 27C040----- Main program & GFX
PUCKPOKE.U8 27C4001---/

ROM sockets U63 & U64 empty

Hi-res PCB scan available on request.
Screenshots available on my site at http://unemulated.emuunlim.com (under PCB Shop Raid #2)


****************************************************************************/

static DRIVER_INIT( puckpkmn )
{
	UINT8 *rom	=	memory_region(machine, REGION_CPU1);
	size_t len		=	memory_region_length(machine, REGION_CPU1);
	int i;

	for (i = 0; i < len; i++)
		rom[i] = BITSWAP8(rom[i],1,4,2,0,7,5,3,6);

	memory_set_bankptr(1, memory_region(machine, REGION_CPU1) );	// VDP reads the roms from here
	memory_set_bankptr(2, main_ram );						// VDP reads the ram from here
}

ROM_START( puckpkmn ) /* Puckman Pockimon  (c)2000 Genie */
	ROM_REGION( 0x200000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "puckpoke.u5", 0x000000, 0x080000, CRC(fd334b91) SHA1(cf8bf6645a4082ea4392937e169b1686c9c7e246) )
	ROM_LOAD16_BYTE( "puckpoke.u4", 0x000001, 0x080000, CRC(839cc76b) SHA1(e15662a7175db7a8e222dda176a8ed92e0d56e9d) )
	ROM_LOAD16_BYTE( "puckpoke.u8", 0x100000, 0x080000, CRC(7936bec8) SHA1(4b350105abe514fbfeabae1c6f3aeee695c3d07a) )
	ROM_LOAD16_BYTE( "puckpoke.u7", 0x100001, 0x080000, CRC(96b66bdf) SHA1(3cc2861ad9bc232cbe683e01b58090f832d03db5) )

	ROM_REGION( 0x40000, REGION_SOUND1, 0 )
	ROM_LOAD( "puckpoke.u3", 0x00000, 0x40000, CRC(7b066bac) SHA1(429616e21c672b07e0705bc63234249cac3af56f) )
ROM_END

/* Genie Hardware (uses Genesis VDP) also has 'Sun Mixing Co' put into tile ram */
GAME( 2000, puckpkmn, 0,        puckpkmn, puckpkmn, puckpkmn, ROT0, "Genie",                  "Puckman Pockimon", 0 )
