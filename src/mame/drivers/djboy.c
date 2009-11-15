/*
DJ Boy (c)1989 Kanako

Hardware has many similarities to Airbusters.

Self Test has two parts:
1) color test : press button#3 to advance past color pattern
2) i/o and sound test: use buttons 1,2,3 to select and play sound/music

- CPU0 manages sprites, which are also used to display text
        irq (0x10) - timing/watchdog
        irq (0x30) - processes sprites
        nmi: wakes up this cpu

- CPU1 manages the protection device, palette, and tilemap(s)
        nmi: resets this cpu
        irq: game update

- CPU2 manages sound chips
        irq: update music
        nmi: handle sound command

- The "BEAST" protection device has access to dipswitches and player inputs.

Genre: Scrolling Fighter
Orientation: Horizontal
Type: Raster: Standard Resolution
CRT: Color
Conversion Class: JAMMA
Number of Simultaneous Players: 2
Maximum number of Players: 2
Gameplay: Joint
Control Panel Layout: Multiple Player
Joystick: 8-way
Buttons: 3 - Punch, Kick, Jump
Sound: Amplified Mono (one channel) - Stereo sound is available
through a 4-pin header (voice of Wolfman Jack!!)

DJ Boy
1990, Kaneko / American Sammy Corp.

PCB Layout
----------

BS
|----------------------------------------------|
| 6264               BS15   6116               |
|                    BS-101 6116               |
| BS-005             780C-2                    |
| BS-004                           DSW1 DSW2   |
|                    6264              IO-JAMMA|
| 16MHz                     PAL1|----------|   |
| 12MHz                         |  BEAST   |  J|
|                               |----------|  A|
| BS-003              *                       M|
|            6116     BS-203  6295     YM2203 M|
| BS-000                           YM3014     A|
|          |-------|          6295  324  4558  |
| BS-001   |KANEKO |  780C-2        324        |
|          |PANDORA|                    VOL  JP|
| BS-002   |       |  BS-100  PAL2   324       |
|          |-------|           6264     VOL CN1|
| BS07     4464 4464  BS19     BS-200   LA4460 |
|          4464 4464  PAL3     D780C-2  LA4460 |
|----------------------------------------------|

Notes:
      D780C-2 - Z80 CPU. clock 6.000MHz [12/2] (for all 3 Z80 CPUs)
      BEAST   - DIP40 Microcontroller, 8xxx series (8041/8042/8751 etc).
                     Clock 6.000MHz on pins 18 & 19
                chip is stamped 'KANEKO Beast (C)Intel '80 (C)KANEKO 1988'
      YM2203  - Yamaha YM2203, clock 3.000MHz [12/4]
      6295    - OKI M6295, clock 1.500MHz [12/8]. Sample rate (Hz) = 12000000 / 8 / 165
      PANDORA - Custom Kaneko graphics generator chip stamped 'PX79C480FP-3 PANDORA-CHIP' (QFP160)
      4464    - 64k x4 DRAM (DIP18)
      6116    - 2k x8 SRAM (DIP24)
      6264    - 8k x8 SRAM (DIP28)
      VSync   - 57.5Hz
      HSync   - 15.68kHz
      JP      - 3 pin jumper to set mono/stereo sound output
      CN1     - 4 pin connector for speakers when jumper is set for stereo sound output
      PAL1    - PAL16L8 stamped 'BS-501'
      PAL2    - PAL16L8 stamped 'BS-502'
      PAL3    - PAL16L8 stamped 'BS-500'
      IO-JAMMA- Custom Kaneko ceramic I/O input resistor pack stamped 'I/O JAMMA MC-8282837'
      LA4460  - Sanyo 12W Power Amplifier (SIL10)
      *       - Unpopulated DIP32 position
      ROMs    -
                BS15.6Y    27C512 EPROM (DIP28)   \ There is an alt. set of labels used for these ROMs with an 'S'
                BS07.1B    27C512 EPROM (DIP28)   | added to the name (i.e. 'BS15S'), but the actual ROM contents is identical
                BS19.4B    27C1001 EPROM (DIP32)  / to the regular set (both sets dumped / verified)
                BS-000.1H  4M MASKROM (DIP32) {sprite}
                BS-001.1F  4M MASKROM (DIP32) {sprite}
                BS-002.1D  4M MASKROM (DIP32) {sprite}
                BS-003.1K  4M MASKROM (DIP32) {sprite}
                BS-004.1S  4M MASKROM (DIP32) {tile}
                BS-005.1U  4M MASKROM (DIP32) {tile}
                BS-100.4D  1M MASKROM (DIP28) {z80}
                BS-101.6W  1M MASKROM (DIP28) {z80 data}
                BS-200.8C  1M MASKROM (DIP28) {z80}
                BS-203.5J  2M MASKROM (DIP32) {oki-m6295 samples}

      DIPs    - SW1
                |--------------------------------------------|
                |              1   2   3   4   5   6   7   8 |
                |--------------------------------------------|
                |SCREEN NORMAL    OFF                        |
                |       FLIP      ON                         |
                |--------------------------------------------|
                |GAME   NORMAL        OFF                    |
                |MODE   TEST          ON                     |
                |--------------------------------------------|
                |COIN1  1C/1P                 OFF OFF        |
                |       1C/2P                 ON  OFF        |
                |       2C/1P                 OFF ON         |
                |       2C/3P                 ON  ON         |
                |                                            |
                |COIN2  1C/1P                         OFF OFF|
                |       1C/2P                         ON  OFF|
                |       2C/1P                         OFF ON |
                |       2C/3P                         ON  ON |
                |--------------------------------------------|
                |SW1 & SW4 NOT USED ALWAYS OFF               |
                |--------------------------------------------|

                SW2
                |--------------------------------------------|
                |              1   2   3   4   5   6   7   8 |
                |--------------------------------------------|
                |DIFFICULTY                                  |
                |NORMAL       OFF OFF                        |
                |EASY         ON  OFF                        |
                |HARD         OFF ON                         |
                |HARDEST      ON  ON                         |
                |--------------------------------------------|
                |BONUS                                       |
                |10,30,50,70,90       OFF OFF                |
                |10,20,30,40,50,                             |
                |60,70,80,90          ON  OFF                |
                |20,50                OFF ON                 |
                |NONE                 ON  ON                 |
                |--------------------------------------------|
                |LIVES    5                   OFF OFF        |
                |         3                   ON  OFF        |
                |         7                   OFF ON         |
                |         9                   ON  ON         |
                |--------------------------------------------|
                |DEMO SOUND  YES                      OFF    |
                |            NO                       ON     |
                |--------------------------------------------|
                |SPEAKER     STEREO                       OFF|
                |OUTPUT      MONO                          ON|
                |--------------------------------------------|
*/

#include "driver.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "sound/okim6295.h"
#include "video/kan_pand.h"

/* public functions from video/djboy.h */
extern void djboy_set_videoreg( UINT8 data );
extern WRITE8_HANDLER( djboy_scrollx_w );
extern WRITE8_HANDLER( djboy_scrolly_w );
extern WRITE8_HANDLER( djboy_videoram_w );
extern WRITE8_HANDLER( djboy_paletteram_w );
extern VIDEO_START( djboy );
extern VIDEO_UPDATE( djboy );
extern VIDEO_EOF( djboy );

/******************************************************************************/

/* KANEKO BEAST state */

#define PROT_OUTPUT_BUFFER_SIZE 8

static int prot_busy_count;
static UINT8 prot_output_buffer[PROT_OUTPUT_BUFFER_SIZE];
static int prot_available_data_count;
static int prot_offs; /* internal state */
static UINT8 prot_ram[0x80]; /* internal RAM */
static UINT8 prot_param[8];

static int coin;
static int complete;
static int lives[2];
static int addr;
static int bankxor;

static enum
{
	eDJBOY_ATTRACT_HIGHSCORE,
	eDJBOY_ATTRACT_TITLE,
	eDJBOY_ATTRACT_GAMEPLAY,
	eDJBOY_PRESS_P1_START,
	eDJBOY_PRESS_P1_OR_P2_START,
	eDJBOY_ACTIVE_GAMEPLAY
} mDjBoyState;

static enum
{
	ePROT_NORMAL,
	ePROT_WRITE_BYTES,
	ePROT_WRITE_BYTE,
	ePROT_READ_BYTES,
	ePROT_WAIT_DSW1_WRITEBACK,
	ePROT_WAIT_DSW2_WRITEBACK,
	ePROT_STORE_PARAM
} prot_mode;

static void
ProtectionOut( int i, UINT8 data )
{
	if( prot_available_data_count == i )
	{
		prot_output_buffer[prot_available_data_count++] = data;
	}
	else
	{
		logerror( "prot_output_buffer overflow!\n" );
		exit(1);
	}
} /* ProtectionOut */

static int
GetLives( running_machine *machine )
{
	int dsw = input_port_read(machine, "DSW2");
	switch( dsw&0x30 )
	{
	case 0x10: return 3;
	case 0x00: return 5;
	case 0x20: return 7;
	case 0x30: return 9;
	}
	return 0;
} /* GetLives */

static WRITE8_HANDLER( coinplus_w )
{
	int dsw = input_port_read(space->machine, "DSW1");
	coin_counter_w( 0, data&1 );
	coin_counter_w( 1, data&2 );
	if( data&1 )
	{ /* TODO: coinage adjustments */
		logerror( "COIN A+\n" );
		switch( (dsw&0x30)>>4 )
		{
		case 0: coin += 4; break; /* 1 coin, 1 credit */
		case 1: coin += 8; break; /* 1 coin, 2 credits */
		case 2: coin += 2; break; /* 2 coins, 1 credit */
		case 3: coin += 6; break; /* 2 coins, 3 credits */
		}
	}
	if( data&2 )
	{
		logerror( "COIN B+\n" );
		switch( (dsw&0xc0)>>6 )
		{
		case 0: coin += 4; break; /* 1 coin, 1 credit */
		case 1: coin += 8; break; /* 1 coin, 2 credits */
		case 2: coin += 2; break; /* 2 coins, 1 credit */
		case 3: coin += 6; break; /* 2 coins, 3 credits */
		}
	}
} /* coinplus_w */

static void
OutputProtectionState( running_machine *machine, int i, int type )
{
	int io = ~input_port_read(machine, "IN0");
	int dat = 0x00;

	switch( mDjBoyState )
	{
	case eDJBOY_ATTRACT_HIGHSCORE:
		if( coin>=4 )
		{
			dat = 0x01;
			mDjBoyState = eDJBOY_PRESS_P1_START;
			logerror( "COIN UP\n" );
		}
		else if( complete )
		{
			dat = 0x06;
			mDjBoyState = eDJBOY_ATTRACT_TITLE;
		}
		break;

	case eDJBOY_ATTRACT_TITLE:
		if( coin>=4 )
		{
			dat = 0x01;
			mDjBoyState = eDJBOY_PRESS_P1_START;
			logerror( "COIN UP\n" );
		}
		else if( complete )
		{
			dat = 0x15;
			mDjBoyState = eDJBOY_ATTRACT_GAMEPLAY;
		}
		break;

	case eDJBOY_ATTRACT_GAMEPLAY:
		if( coin>=4 )
		{
			dat = 0x01;
			mDjBoyState = eDJBOY_PRESS_P1_START;
			logerror( "COIN UP\n" );
		}
		else if( complete )
		{
			dat = 0x0b;
			mDjBoyState = eDJBOY_ATTRACT_HIGHSCORE;
		}
		break;

	case eDJBOY_PRESS_P1_START:
		if( io&1 )
		{ /* p1 start */
			dat = 0x16;
			mDjBoyState = eDJBOY_ACTIVE_GAMEPLAY;
			logerror( "P1 START\n" );
		}
		else if( coin>=8 )
		{
			dat = 0x05;
			mDjBoyState = eDJBOY_PRESS_P1_OR_P2_START;
			logerror( "COIN2 UP\n" );
		}
		break;

	case eDJBOY_PRESS_P1_OR_P2_START:
		if( io&1 )
		{ /* p1 start */
			dat = 0x16;
			mDjBoyState = eDJBOY_ACTIVE_GAMEPLAY;
			lives[0] = GetLives(machine);
			logerror( "P1 START!\n" );
			coin-=4;
		}
		else if( io&2 )
		{ /* p2 start */
			dat = 0x0a;
			mDjBoyState = eDJBOY_ACTIVE_GAMEPLAY;
			lives[0] = GetLives(machine);
			lives[1] = GetLives(machine);
			logerror( "P2 START!\n" );
			coin-=8;
		}
		break;

	case eDJBOY_ACTIVE_GAMEPLAY:
		if( lives[0]==0 && lives[1]==0 && complete )
		{ /* continue countdown complete */
			dat = 0x0f;
			logerror( "countdown complete!\n" );
			mDjBoyState = eDJBOY_ATTRACT_HIGHSCORE;
		}
		else if( coin>=4 )
		{
			if( (io&1) && lives[0]==0 )
			{
				dat = 0x12; /* continue (P1) */
				lives[0] = GetLives(machine);
				mDjBoyState = eDJBOY_ACTIVE_GAMEPLAY;
				coin-=4;
				logerror( "P1 CONTINUE!\n" );
			}
			else if( (io&2) && lives[1]==0 )
			{
				dat = 0x08; /* continue (P2) */
				lives[1] = GetLives(machine);
				mDjBoyState = eDJBOY_ACTIVE_GAMEPLAY;
				coin-=4;
				logerror( "P2 CONTINUE!\n" );
			}
		}
		break;
	}
	complete = 0;
	ProtectionOut( i, dat );
} /* OutputProtectionState */

static void
CommonProt( running_machine *machine, int i, int type )
{
	int displayedCredits = coin/4;
	if( displayedCredits>9 )
	{
		displayedCredits = 9;
	}
	ProtectionOut( i++, displayedCredits );
	ProtectionOut( i++, input_port_read(machine, "IN0") ); /* COIN/START */
	OutputProtectionState( machine, i, type );
} /* CommonProt */

static WRITE8_HANDLER( beast_data_w )
{
	prot_busy_count = 1;

	logerror( "0x%04x: prot_w(0x%02x)\n", cpu_get_pc(space->cpu), data );

	watchdog_reset_w(space,0,0);

	if( prot_mode == ePROT_WAIT_DSW1_WRITEBACK )
	{
		logerror( "[DSW1_WRITEBACK]\n" );
		ProtectionOut( 0, input_port_read(space->machine, "DSW2") ); /* DSW2 */
		prot_mode = ePROT_WAIT_DSW2_WRITEBACK;
	}
	else if( prot_mode == ePROT_WAIT_DSW2_WRITEBACK )
	{
		logerror( "[DSW2_WRITEBACK]\n" );
		prot_mode = ePROT_STORE_PARAM;
		prot_offs = 0;
	}
	else if( prot_mode == ePROT_STORE_PARAM )
	{
		logerror( "prot param[%d]: 0x%02x\n", prot_offs, data );
		if( prot_offs<8 )
		{
			prot_param[prot_offs++] = data;
		}
		if( prot_offs == 8 )
		{
			prot_mode = ePROT_NORMAL;
		}
	}
	else if( prot_mode == ePROT_WRITE_BYTE )
	{ /* pc == 0x79cd */
		prot_ram[(prot_offs++)&0x7f] = data;
		prot_mode = ePROT_WRITE_BYTES;
	}
	else
	{
		switch( data )
		{
		case 0x00:
			if( prot_mode == ePROT_WRITE_BYTES )
			{ /* next byte is data to write to internal prot RAM */
				prot_mode = ePROT_WRITE_BYTE;
			}
			else if( prot_mode == ePROT_READ_BYTES )
			{ /* request next byte of internal prot RAM */
				ProtectionOut( 0, prot_ram[(prot_offs++)&0x7f] );
			}
			else
			{
				logerror( "UNEXPECTED PREFIX!\n" );
			}
			break;

		case 0x01: // pc=7389
			OutputProtectionState( space->machine, 0, 0x01 );
			break;

		case 0x02:
			CommonProt( space->machine,0,0x02 );
			break;

		case 0x03: /* prepare for memory write to protection device ram (pc == 0x7987) */ // -> 0x02
			logerror( "[WRITE BYTES]\n" );
			prot_mode = ePROT_WRITE_BYTES;
			prot_offs = 0;
			break;

		case 0x04:
			ProtectionOut( 0,0 ); // ?
			ProtectionOut( 1,0 ); // ?
			ProtectionOut( 2,0 ); // ?
			ProtectionOut( 3,0 ); // ?
			CommonProt(    space->machine, 4,0x04 );
			break;

		case 0x05: /* 0x71f4 */
			ProtectionOut( 0,input_port_read(space->machine, "IN1") ); // to $42
			ProtectionOut( 1,0 ); // ?
			ProtectionOut( 2,input_port_read(space->machine, "IN2") ); // to $43
			ProtectionOut( 3,0 ); // ?
			ProtectionOut( 4,0 ); // ?
			CommonProt(    space->machine, 5,0x05 );
			break;

		case 0x07:
			CommonProt( space->machine, 0,0x07 );
			break;

		case 0x08: /* pc == 0x727a */
			ProtectionOut( 0,input_port_read(space->machine, "IN0") ); /* COIN/START */
			ProtectionOut( 1,input_port_read(space->machine, "IN1") ); /* JOY1 */
			ProtectionOut( 2,input_port_read(space->machine, "IN2") ); /* JOY2 */
			ProtectionOut( 3,input_port_read(space->machine, "DSW1") ); /* DSW1 */
			ProtectionOut( 4,input_port_read(space->machine, "DSW2") ); /* DSW2 */
			CommonProt(    space->machine, 5, 0x08 );
			break;

		case 0x09:
			ProtectionOut( 0,0 ); // ?
			ProtectionOut( 1,0 ); // ?
			ProtectionOut( 2,0 ); // ?
			CommonProt(    space->machine, 3, 0x09 );
			break;

		case 0x0a:
			CommonProt( space->machine,0,0x0a );
			break;

		case 0x0c:
			CommonProt( space->machine,1,0x0c );
			break;

		case 0x0d:
			CommonProt( space->machine,2,0x0d );
			break;

		case 0xfe: /* prepare for memory read from protection device ram (pc == 0x79ee, 0x7a3f) */
			if( prot_mode == ePROT_WRITE_BYTES )
			{
				prot_mode = ePROT_READ_BYTES;
				logerror( "[READ BYTES]\n" );
			}
			else
			{
				prot_mode = ePROT_WRITE_BYTES;
				logerror( "[WRITE BYTES*]\n" );
			}
			prot_offs = 0;
			break;

		case 0xff: /* read DSW (pc == 0x714d) */
			ProtectionOut( 0,input_port_read(space->machine, "DSW1") ); /* DSW1 */
			prot_mode = ePROT_WAIT_DSW1_WRITEBACK;
			break;

		case 0xa9: /* 1-player game: P1 dies
                         2-player game: P2 dies */
			if( lives[0]>0 && lives[1]>0 )
			{
				lives[1]--;
				logerror( "%02x P2 DIE(%d)\n", data, lives[1] );
			}
			else if( lives[0]>0 )
			{
				lives[0]--;
				logerror( "%02x P1 DIE(%d)\n", data, lives[0] );
			}
			else
			{
				logerror( "%02x COMPLETE.\n", data );
				complete = 0xa9;
			}
			break;

		case 0x92: /* p2 lost life; in 2-p game, P1 died */
			if( lives[0]>0 && lives[1]>0 )
			{
				lives[0]--;
				logerror( "%02x P1 DIE(%d)\n", data, lives[0] );
			}
			else if( lives[1]>0 )
			{
				lives[1]--;
				logerror( "%02x P2 DIE (%d)\n", data, lives[1] );
			}
			else
			{
				logerror( "%02x COMPLETE.\n", data );
				complete = 0x92;
			}
			break;

		case 0xa3: /* p2 bonus life */
			lives[1]++;
			logerror( "%02x P2 BONUS(%d)\n", data, lives[1] );
			break;

		case 0xa5: /* p1 bonus life */
			lives[0]++;
			logerror( "%02x P1 BONUS(%d)\n", data, lives[0] );
			break;

		case 0xad: /* 1p game start ack */
			logerror( "%02x 1P GAME START\n", data );
			break;

		case 0xb0: /* 1p+2p game start ack */
			logerror( "%02x 1P+2P GAME START\n", data );
			break;

		case 0xb3: /* 1p continue ack */
			logerror( "%02x 1P CONTINUE\n", data );
			break;

		case 0xb7: /* 2p continue ack */
			logerror( "%02x 2P CONTINUE\n", data );
			break;

		default:
		case 0x97:
		case 0x9a:
			logerror( "!!0x%04x: prot_w(0x%02x)\n", cpu_get_pc(space->cpu), data );
			break;
		}
	}
} /* beast_data_w */

static READ8_HANDLER( beast_data_r )
{ /* port#4 */
	UINT8 data = 0x00;
	if( prot_available_data_count )
	{
		int i;
		data = prot_output_buffer[0];
		prot_available_data_count--;
		for( i=0; i<prot_available_data_count; i++ )
		{
			prot_output_buffer[i] = prot_output_buffer[i+1];
		}
	}
	else
	{
		logerror( "prot_r: data expected!\n" );
	}
	logerror( "0x%04x: prot_r() == 0x%02x\n", cpu_get_pc(space->cpu), data );
	return data;
} /* beast_data_r */

static READ8_HANDLER( beast_status_r )
{ /* port 0xc */
	UINT8 result = 0;
	if( prot_busy_count )
	{
		prot_busy_count--;
		result |= 1<<3;
	}
	if( !prot_available_data_count )
	{
		result |= 1<<2;
	}
	return result;
} /* beast_status_r */

/******************************************************************************/
static DRIVER_INIT( djboy )
{
	coin = 0;
	complete = 0;
	memset(lives, 0, sizeof(lives));
	addr = 0xff;
	bankxor = 0x00;
}

static DRIVER_INIT( djboyj )
{
	DRIVER_INIT_CALL( djboy );
	bankxor = 0x1f;
}

static WRITE8_HANDLER( trigger_nmi_on_cpu0 )
{
	cputag_set_input_line(space->machine, "maincpu", INPUT_LINE_NMI, PULSE_LINE);
}

static WRITE8_HANDLER( cpu0_bankswitch_w )
{
	unsigned char *RAM = memory_region(space->machine, "maincpu");
	data ^= bankxor;
	memory_set_bankptr(space->machine, 4,&RAM[0x10000]); /* unsure if/how this area is banked */
	if( data < 4 )
	{
		RAM = &RAM[0x2000 * data];
	}
	else
	{
		RAM = &RAM[0x10000 + 0x2000 * (data-4)];
	}
	memory_set_bankptr(space->machine, 1,RAM);
}

/******************************************************************************/

/**
 * xx------ msb scrollx
 * --x----- msb scrolly
 * ---x---- screen flip
 * ----xxxx bank
 */
static WRITE8_HANDLER( cpu1_bankswitch_w )
{
	UINT8 *RAM = memory_region(space->machine, "cpu1");
	djboy_set_videoreg( data );
	switch( data&0xf )
	{
	/* bs65.5y */
	case 0x00: memory_set_bankptr(space->machine, 2,&RAM[0x00000]); break;
	case 0x01: memory_set_bankptr(space->machine, 2,&RAM[0x04000]); break;
	case 0x02: memory_set_bankptr(space->machine, 2,&RAM[0x10000]); break;
	case 0x03: memory_set_bankptr(space->machine, 2,&RAM[0x14000]); break;

	/* bs101.6w */
	case 0x08: memory_set_bankptr(space->machine, 2,&RAM[0x18000]); break;
	case 0x09: memory_set_bankptr(space->machine, 2,&RAM[0x1c000]); break;
	case 0x0a: memory_set_bankptr(space->machine, 2,&RAM[0x20000]); break;
	case 0x0b: memory_set_bankptr(space->machine, 2,&RAM[0x24000]); break;
	case 0x0c: memory_set_bankptr(space->machine, 2,&RAM[0x28000]); break;
	case 0x0d: memory_set_bankptr(space->machine, 2,&RAM[0x2c000]); break;
	case 0x0e: memory_set_bankptr(space->machine, 2,&RAM[0x30000]); break;
	case 0x0f: memory_set_bankptr(space->machine, 2,&RAM[0x34000]); break;

	default:
		break;
	}
}

/******************************************************************************/

static WRITE8_HANDLER( trigger_nmi_on_sound_cpu2 )
{
	soundlatch_w(space, 0, data);
	cputag_set_input_line(space->machine, "cpu2", INPUT_LINE_NMI, PULSE_LINE);
} /* trigger_nmi_on_sound_cpu2 */

static WRITE8_HANDLER( cpu2_bankswitch_w )
{
	UINT8 *RAM = memory_region(space->machine, "cpu2");

	if( data < 3 )
	{
		RAM = &RAM[0x04000 * data];
	}
	else
	{
		RAM = &RAM[0x10000 + 0x4000 * (data - 3)];
	}
	memory_set_bankptr(space->machine, 3, RAM);
}

/******************************************************************************/

static ADDRESS_MAP_START( cpu0_am, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xafff) AM_ROMBANK(4)
	AM_RANGE(0xb000, 0xbfff) AM_READWRITE( pandora_spriteram_r, pandora_spriteram_w )
	AM_RANGE(0xc000, 0xdfff) AM_ROMBANK(1)
	AM_RANGE(0xe000, 0xefff) AM_RAM AM_SHARE(1)
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu0_port_am, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(cpu0_bankswitch_w)
ADDRESS_MAP_END

/******************************************************************************/

static ADDRESS_MAP_START( cpu1_am, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK(2)
	AM_RANGE(0xc000, 0xcfff) AM_RAM_WRITE(djboy_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0xd000, 0xd3ff) AM_RAM_WRITE(djboy_paletteram_w) AM_BASE(&paletteram)
	AM_RANGE(0xd400, 0xd8ff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE(1)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu1_port_am, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(cpu1_bankswitch_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(trigger_nmi_on_sound_cpu2)
	AM_RANGE(0x04, 0x04) AM_READWRITE(beast_data_r,beast_data_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(djboy_scrolly_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(djboy_scrollx_w)
	AM_RANGE(0x0a, 0x0a) AM_WRITE(trigger_nmi_on_cpu0)
	AM_RANGE(0x0c, 0x0c) AM_READ(beast_status_r)
	AM_RANGE(0x0e, 0x0e) AM_WRITE(coinplus_w)
ADDRESS_MAP_END

/******************************************************************************/

static ADDRESS_MAP_START( cpu2_am, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK(3)
	AM_RANGE(0xc000, 0xdfff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu2_port_am, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(cpu2_bankswitch_w)
	AM_RANGE(0x02, 0x03) AM_DEVREADWRITE("ymsnd", ym2203_r, ym2203_w)
	AM_RANGE(0x04, 0x04) AM_READ(soundlatch_r)
	AM_RANGE(0x06, 0x06) AM_DEVREADWRITE("oki1", okim6295_r, okim6295_w)
	AM_RANGE(0x07, 0x07) AM_DEVREADWRITE("oki2", okim6295_r, okim6295_w)
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( djboy )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* labeled "TEST" in self test */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* punch */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) /* kick */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) /* jump */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x00, "Bonus" )
	PORT_DIPSETTING(    0x00, "10,30,50,70,90" )
	PORT_DIPSETTING(    0x04, "10,20,30,40,50,60,70,80,90" )
	PORT_DIPSETTING(    0x08, "20,50" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x20, "7" )
	PORT_DIPSETTING(    0x30, "9" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Stereo Sound" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tile_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{
		0*4,1*4,2*4,3*4,4*4,5*4,6*4,7*4,
		8*32+0*4,8*32+1*4,8*32+2*4,8*32+3*4,8*32+4*4,8*32+5*4,8*32+6*4,8*32+7*4
	},
	{
		0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,
		16*32+0*32,16*32+1*32,16*32+2*32,16*32+3*32,16*32+4*32,16*32+5*32,16*32+6*32,16*32+7*32
	},
	4*8*32
};

static GFXDECODE_START( djboy )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout, 0x100, 16 ) /* sprite bank */
	GFXDECODE_ENTRY( "gfx2", 0, tile_layout, 0x000, 16 ) /* background tiles */
GFXDECODE_END

/******************************************************************************/

static INTERRUPT_GEN( djboy_interrupt )
{ /* CPU1 uses interrupt mode 2. For now, just alternate the two interrupts. */
	addr ^= 0x02;
	cpu_set_input_line_and_vector(device, 0, HOLD_LINE, addr);
}

static MACHINE_DRIVER_START( djboy )
	MDRV_CPU_ADD("maincpu", Z80,6000000)
	MDRV_CPU_PROGRAM_MAP(cpu0_am)
	MDRV_CPU_IO_MAP(cpu0_port_am)
	MDRV_CPU_VBLANK_INT_HACK(djboy_interrupt,2)

	MDRV_CPU_ADD("cpu1", Z80,6000000)
	MDRV_CPU_PROGRAM_MAP(cpu1_am)
	MDRV_CPU_IO_MAP(cpu1_port_am)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("cpu2", Z80, 6000000)
	MDRV_CPU_PROGRAM_MAP(cpu2_am)
	MDRV_CPU_IO_MAP(cpu2_port_am)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_QUANTUM_TIME(HZ(6000))

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)

	MDRV_GFXDECODE(djboy)
	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(djboy)
	MDRV_VIDEO_UPDATE(djboy)
	MDRV_VIDEO_EOF(djboy)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2203, 3000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MDRV_SOUND_ADD("oki1", OKIM6295, 12000000 / 8)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7low)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("oki2", OKIM6295, 12000000 / 8)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7low)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


ROM_START( djboy )
	ROM_REGION( 0x48000, "maincpu", 0 )
	ROM_LOAD( "bs64.4b",  0x00000, 0x08000, CRC(b77aacc7) SHA1(78100d4695738a702f13807526eb1bcac759cce3) )
	ROM_CONTINUE( 0x10000, 0x18000 )
	ROM_LOAD( "bs100.4d", 0x28000, 0x20000, CRC(081e8af8) SHA1(3589dab1cf31b109a40370b4db1f31785023e2ed) )

	ROM_REGION( 0x38000, "cpu1", 0 )
	ROM_LOAD( "bs65.5y",  0x00000, 0x08000, CRC(0f1456eb) SHA1(62ed48c0d71c1fabbb3f6ada60381f57f692cef8) )
	ROM_CONTINUE( 0x10000, 0x08000 )
	ROM_LOAD( "bs101.6w", 0x18000, 0x20000, CRC(a7c85577) SHA1(8296b96d5f69f6c730b7ed77fa8c93496b33529c) )

	ROM_REGION( 0x24000, "cpu2", 0 ) /* sound */
	ROM_LOAD( "bs200.8c", 0x00000, 0x0c000, CRC(f6c19e51) SHA1(82193f71122df07cce0a7f057a87b89eb2d587a1) )
	ROM_CONTINUE( 0x10000, 0x14000 )

	ROM_REGION( 0x1000, "mcu", 0 ) /* i8751 microcontroller */
	ROM_LOAD( "i8751_beast", 0x00000, 0x1000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "bs000.1h", 0x000000, 0x80000, CRC(be4bf805) SHA1(a73c564575fe89d26225ca8ec2d98b6ac319ac18) )
	ROM_LOAD( "bs001.1f", 0x080000, 0x80000, CRC(fdf36e6b) SHA1(a8762458dfd5201304247c113ceb85e96e33d423) )
	ROM_LOAD( "bs002.1d", 0x100000, 0x80000, CRC(c52fee7f) SHA1(bd33117f7a57899fd4ec0a77413107edd9c44629) )
	ROM_LOAD( "bs003.1k", 0x180000, 0x80000, CRC(ed89acb4) SHA1(611af362606b73cd2cf501678b463db52dcf69c4) )
	ROM_LOAD( "bs07.1b",  0x1f0000, 0x10000, CRC(d9b7a220) SHA1(ba3b528d50650c209c986268bb29b42ff1276eb2) )  // replaces last 0x200 tiles

	ROM_REGION( 0x100000, "gfx2", 0 ) /* background */
	ROM_LOAD( "bs004.1s", 0x000000, 0x80000, CRC(2f1392c3) SHA1(1bc3030b3612766a02133eef0b4d20013c0495a4) )
	ROM_LOAD( "bs005.1u", 0x080000, 0x80000, CRC(46b400c4) SHA1(35f4823364bbff1fc935994498d462bbd3bc6044) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* OKI-M6295 samples */
	ROM_LOAD( "bs203.5j", 0x000000, 0x40000, CRC(805341fb) SHA1(fb94e400e2283aaa806814d5a39d6196457dc822) )

	ROM_REGION( 0x40000, "oki2", 0 ) /* OKI-M6295 samples */
	ROM_LOAD( "bs203.5j", 0x000000, 0x40000, CRC(805341fb) SHA1(fb94e400e2283aaa806814d5a39d6196457dc822) )
ROM_END

ROM_START( djboya )
	ROM_REGION( 0x48000, "maincpu", 0 )
	ROM_LOAD( "bs19s.rom",  0x00000, 0x08000, CRC(17ce9f6c) SHA1(a0c1832b05dc46991e8949067ca0278f5498835f) )
	ROM_CONTINUE( 0x10000, 0x18000 )
	ROM_LOAD( "bs100.4d", 0x28000, 0x20000, CRC(081e8af8) SHA1(3589dab1cf31b109a40370b4db1f31785023e2ed) )

	ROM_REGION( 0x38000, "cpu1", 0 )
	ROM_LOAD( "bs15s.rom",  0x00000, 0x08000, CRC(e6f966b2) SHA1(f9df16035a8b09d87eb70315b216892e25d99b03) )
	ROM_CONTINUE( 0x10000, 0x08000 )
	ROM_LOAD( "bs101.6w", 0x18000, 0x20000, CRC(a7c85577) SHA1(8296b96d5f69f6c730b7ed77fa8c93496b33529c) )

	ROM_REGION( 0x24000, "cpu2", 0 ) /* sound */
	ROM_LOAD( "bs200.8c", 0x00000, 0x0c000, CRC(f6c19e51) SHA1(82193f71122df07cce0a7f057a87b89eb2d587a1) )
	ROM_CONTINUE( 0x10000, 0x14000 )

	ROM_REGION( 0x1000, "mcu", 0 ) /* i8751 microcontroller */
	ROM_LOAD( "i8751_beast", 0x00000, 0x1000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "bs000.1h", 0x000000, 0x80000, CRC(be4bf805) SHA1(a73c564575fe89d26225ca8ec2d98b6ac319ac18) )
	ROM_LOAD( "bs001.1f", 0x080000, 0x80000, CRC(fdf36e6b) SHA1(a8762458dfd5201304247c113ceb85e96e33d423) )
	ROM_LOAD( "bs002.1d", 0x100000, 0x80000, CRC(c52fee7f) SHA1(bd33117f7a57899fd4ec0a77413107edd9c44629) )
	ROM_LOAD( "bs003.1k", 0x180000, 0x80000, CRC(ed89acb4) SHA1(611af362606b73cd2cf501678b463db52dcf69c4) )
	ROM_LOAD( "bs07.1b",  0x1f0000, 0x10000, CRC(d9b7a220) SHA1(ba3b528d50650c209c986268bb29b42ff1276eb2) )  // replaces last 0x200 tiles

	ROM_REGION( 0x100000, "gfx2", 0 ) /* background */
	ROM_LOAD( "bs004.1s", 0x000000, 0x80000, CRC(2f1392c3) SHA1(1bc3030b3612766a02133eef0b4d20013c0495a4) )
	ROM_LOAD( "bs005.1u", 0x080000, 0x80000, CRC(46b400c4) SHA1(35f4823364bbff1fc935994498d462bbd3bc6044) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* OKI-M6295 samples */
	ROM_LOAD( "bs203.5j", 0x000000, 0x40000, CRC(805341fb) SHA1(fb94e400e2283aaa806814d5a39d6196457dc822) )

	ROM_REGION( 0x40000, "oki2", 0 ) /* OKI-M6295 samples */
	ROM_LOAD( "bs203.5j", 0x000000, 0x40000, CRC(805341fb) SHA1(fb94e400e2283aaa806814d5a39d6196457dc822) )
ROM_END

ROM_START( djboyj )
	ROM_REGION( 0x48000, "maincpu", 0 )
	ROM_LOAD( "bs12.4b",  0x00000, 0x08000, CRC(0971523e) SHA1(f90cd02cedf8632f4b651de7ea75dc8c0e682f6e) )
	ROM_CONTINUE( 0x10000, 0x18000 )
	ROM_LOAD( "bs100.4d", 0x28000, 0x20000, CRC(081e8af8) SHA1(3589dab1cf31b109a40370b4db1f31785023e2ed) )

	ROM_REGION( 0x38000, "cpu1", 0 )
	ROM_LOAD( "bs13.5y",  0x00000, 0x08000, CRC(5c3f2f96) SHA1(bb7ee028a2d8d3c76a78a29fba60bcc36e9399f5) )
	ROM_CONTINUE( 0x10000, 0x08000 )
	ROM_LOAD( "bs101.6w", 0x18000, 0x20000, CRC(a7c85577) SHA1(8296b96d5f69f6c730b7ed77fa8c93496b33529c) )

	ROM_REGION( 0x24000, "cpu2", 0 ) /* sound */
	ROM_LOAD( "bs200.8c", 0x00000, 0x0c000, CRC(f6c19e51) SHA1(82193f71122df07cce0a7f057a87b89eb2d587a1) )
	ROM_CONTINUE( 0x10000, 0x14000 )

	ROM_REGION( 0x1000, "mcu", 0 ) /* i8751 microcontroller */
	ROM_LOAD( "i8751_beast", 0x00000, 0x1000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "bs000.1h", 0x000000, 0x80000, CRC(be4bf805) SHA1(a73c564575fe89d26225ca8ec2d98b6ac319ac18) )
	ROM_LOAD( "bs001.1f", 0x080000, 0x80000, CRC(fdf36e6b) SHA1(a8762458dfd5201304247c113ceb85e96e33d423) )
	ROM_LOAD( "bs002.1d", 0x100000, 0x80000, CRC(c52fee7f) SHA1(bd33117f7a57899fd4ec0a77413107edd9c44629) )
	ROM_LOAD( "bs003.1k", 0x180000, 0x80000, CRC(ed89acb4) SHA1(611af362606b73cd2cf501678b463db52dcf69c4) )
	ROM_LOAD( "bsxx.1b",  0x1f0000, 0x10000, CRC(22c8aa08) SHA1(5521c9d73b4ee82a2de1992d6edc7ef62788ad72) ) // replaces last 0x200 tiles

	ROM_REGION( 0x100000, "gfx2", 0 ) /* background */
	ROM_LOAD( "bs004.1s", 0x000000, 0x80000, CRC(2f1392c3) SHA1(1bc3030b3612766a02133eef0b4d20013c0495a4) )
	ROM_LOAD( "bs005.1u", 0x080000, 0x80000, CRC(46b400c4) SHA1(35f4823364bbff1fc935994498d462bbd3bc6044) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* OKI-M6295 samples */
	ROM_LOAD( "bs-204.5j", 0x000000, 0x40000, CRC(510244f0) SHA1(afb502d46d268ad9cd209ae1da72c50e4e785626) )

	ROM_REGION( 0x40000, "oki2", 0 ) /* OKI-M6295 samples */
	ROM_LOAD( "bs-204.5j", 0x000000, 0x40000, CRC(510244f0) SHA1(afb502d46d268ad9cd209ae1da72c50e4e785626) )
ROM_END



/*     YEAR, NAME,  PARENT, MACHINE, INPUT, INIT, MNTR,  COMPANY, FULLNAME, FLAGS */
GAME( 1989, djboy,  0,      djboy,   djboy, djboy,    ROT0, "Kaneko (American Sammy license)", "DJ Boy (set 1)", 0) // Sammy & Williams logos in FG ROM
GAME( 1989, djboya, djboy,  djboy,   djboy, djboy,    ROT0, "Kaneko (American Sammy license)", "DJ Boy (set 2)", 0) // Sammy & Williams logos in FG ROM
GAME( 1989, djboyj, djboy,  djboy,   djboy, djboyj,   ROT0, "Kaneko (Sega license)", "DJ Boy (Japan)", 0 ) // Sega logo in FG ROM
