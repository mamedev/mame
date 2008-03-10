/***************************************************************************
Namco NA-1 / NA-2 System

NA-1 Games:
-   Bakuretsu Quiz Ma-Q Dai Bouken
-   F/A / Fighter & Attacker
-   Super World Court (C354, C357)
-   Exvania (C350, C354)
-   Cosmo Gang the Puzzle (C356)
-   Tinkle Pit (C354, C367)
-   Emeraldia (C354, C358)

NA-2 Games:
-   Knuckle Heads
-   Numan Athletics
-   Emeraldia (C354, C358)
-   Nettou! Gekitou! Quiztou!! (C354, C365 - both are 32pin)
-   X-Day 2

To Do:

- Pressing "F3" (reset) crashes MAME

- Emeralda:
    After selecting the game type, tilemap scrolling is briefly incorrect
    Shadow sprites, if enabled, make the score display invisible
    The demo sprites are incorrect, you should see a dolphin speaking
    the instructions and the playfield is wrong.
    Byte 0x25 of the NVRAM controls the FBI logo. 0x00 is on, 0x01 is off

- Hook up ROZ registers
    Example:  In Emeralda the game logo rotates and zooms in against a black
    background with bubbles during the intro.  Then at the second green screen
    when flashing Please Insert Coin, the word NAMCO rotates past the viewer.

- Is view area controlled by registers?

- Xday 2:
    has some graphics glitches (wrong sprite tiles); probably blitter-related

    Rom board  M112
    Rom board custom Key chip i.d. C394
    Game uses a small cash-register type printer (connects to rom board)
    Game also has a large L.E.D. type score board with several
    displays for various scores. (connects to rom board)
    Game uses coin-type battery on rom board. (not suicide)
    Game won't startup unless printer is connected and with paper.

The board has a 28c16 EEPROM

No other CPUs on the board, but there are many custom chips.
Mother Board:
210 (28pin SOP)
C69 or C70 (80pin PQFP)
215 (176pin PQFP)
C218 (208pin PQFP)
219 (160pin PQFP)
Plus 1 or 2 custom chips on ROM board.

Notes:

-   NA-2 is backwards compatible with NA-1.

-   Some NA-1 PCBs are known to have the NA-2 (C70) MCU BIOS, so that cannot
    be used to tell them apart.

-   Test mode for NA2 games includes an additional item: UART Test.
    No games are known to actually link up and use the UART feature.
    It's been confirmed that a Numan Athletics fails the UART test,
    behaving as it does in MAME.

-   Quiz games use 1p button 1 to pick test, 2p button 1 to begin test,
    and 2p button 2 to exit. Because quiz games doesn't have joystick.

-   Almost all quiz games using JAMMA edge connector assign
    button1 to up, button 2 to down, button 3 to left, button 4 to right.
    But Taito F2 quiz games assign button 3 to right and button 4 to left.


Namco NA2 hardware
------------------

PCB Layout
----------

NA-2 GAME PCB
8627961102
(8627963102)
|---------------------------------------------|
|  LA4705           T062               |----| |
|         JP3   51568  LC7881          | 6  | |
|J        210              LH52250     | 8  | |
|A    VOL1      VOL2       LH52250     | 0  | |
|M                         |--------|  | 0  | |
|M            |------|     |  C218  |  | 0  | |
|A  TC514256  | 215  |     |        |  |----| |
|   TC514256  |      |     |        |         |
|             |------|     |--------|50.113MHz|
|            |---------ROM-BOARD--------------+
|     TC51832|  LH5168       TC514256         |
|   DSW(2)   |               TC514256         |
|            |               TC514256         |
|            |               TC514256         |
|            | |----|       |--------|    J103|
|4   IR2C24  | |C70 |       |  219   |        |
|8           | |----|       |        |        |
|W   PC9D10  |              |        |        |
|A     IR2C24|              |--------|        |
|Y           |       3773           TC51832   |
|------------+--------------------------------|
Notes:
       68000 clock - 12.5282MHz [50.113/4]
       TC51832     - 32k x8 SRAM (SOP28)
       TC514256    - 256k x4 DRAM (ZIP19)
       LH5168      - 8k x8 SRAM (SOP28)
       LC7881      - Sanyo 16bit DAC (SOIC20)
       LH52250     - 32k x8 SRAM (SOP28)
       51568       - ??, possibly audio related functions, manufactured by Mitsubishi (SOIC24)
       LA4705      - 15W 2-Channel Audio Power Amplifier (SIL18)
       T062        - Op AMP? (SOIC8)
       3773        - System reset and watchdog IC (SOIC8)
       J103        - Multi pin connector for ROM Board
       JP3         - Stereo/Mono jumper. For stereo, use pin 1 and 2 of the 48-WAY Namco connector for the 2nd speaker
       VSync       - 60Hz
       HSync       - 15.73kHz
       Namco Custom-
                     C218- (QFP208)
                     C70 - rebadged M37702 (QFP80)
                     219 - (QFP160)
                     215 - (QFP176)
                     210 - tied to some audio parts & C215 (SOP28)

ROM Board
---------

NA E4M8 ROM PCB
8626961200
(8626963200)
|--------------------------------+
| PAL1    28C16        KEYCUS    |
| PAL2                           |
|                                |
| ROM.7F   ROM.5F  ROM.3F        |
|   ROM.6F  ROM.4F  ROM.2F   J201|
|                                |
|                                |
|                                |
| ROM.7C   ROM.5C  ROM.3C        |
|   ROM.6C  ROM.4C  ROM.2C       |
+--------------------------------|
Notes:
      J201   - Multi pin connector for main board
      KEYCUS - C365 for 'Nettou! Gekitou! Quiztou!!' (DIP32)
      PAL1   - PAL16L8 labelled 'NR1R2' (DIP20)
      PAL2   - PAL16L8 labelled 'NR1R10' (DIP20)
      ROMs   -
               7F/6F/7C/6C - 27C040 EPROM (DIP32)
               All other ROMs are MB838000 8MBit MaskROM (DIP32)
               ROM Names/locations for 'Nettou! Gekitou! Quiztou!!'....
               QT1EP1L.7C
               QT1EP1U.7F
               QT1EP0L.6C
               QT1EP0U.6F
               QT1MA3L.5C
               QT1MA3U.5F
               QT1MA2L.4C
               QT1MA2U.4F
               QT1MA1L.3C
               QT1MA1U.3F
               QT1MA0L.2C
               QT1MA0U.2F

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "namcona1.h"
#include "sound/c140.h"
#include "sound/namcona.h"
#include "cpu/m37710/m37710.h"

static UINT16 *mpBank0, *mpBank1;
static UINT8 mCoinCount[4];
static UINT8 mCoinState;
static UINT16 *mcu_ram;
static int mEnableInterrupts;
int namcona1_gametype;
static int is_na2;

/*************************************************************************/

static const UINT8 ExvaniaDefaultNvMem[] =
{
/* This data oughtn't be necessary; when Exbania's EPROM area is uninitialized,
 * the game software automatically writes these values there, but then jumps
 * to an unmapped (bogus) address, causing MAME to crash.
 */
 	0x30,0x32,0x4f,0x63,0x74,0x39,0x32,0x52,0x45,0x56,0x49,0x53,0x49,0x4f,0x4e,0x35,
	0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x00,0x00,0x01,
	0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x02
}; /* ExvaniaDefaultNvMem */

static const UINT8 QuiztouDefaultNvMem[] =
{
/* This data oughtn't be necessary; when QuiztouDefaultNvMem's EPROM area is uninitialized,
 * the game software automatically writes these values there, but then jumps
 * to an unmapped (bogus) address, causing MAME to crash.
 */
	0x30,0x32,0x4F,0x63,0x74,0x39,0x32,0x52,0x45,0x56,0x49,0x53,0x49,0x4F,0x4E,0x35,
	0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x00,0x00,0x01,
	0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02
}; /* QuiztouDefaultNvMem */

static const UINT8 CgangpzlDefaultNvMem[] =
{
/* This data oughtn't be necessary; when Cgangpzl's EPROM area is uninitialized,
 * the game software automatically writes these values there, but then hangs.
 */
	0x30,0x32,0x4F,0x63,0x74,0x39,0x32,0x52,0x45,0x56,0x49,0x53,0x49,0x4F,0x4E,0x35,
	0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x00,0x00,0x01,
	0x00,0x02,0x00,0x00,0x00,0x01
}; /* CgangpzlDefaultNvMem */

static UINT8 namcona1_nvmem[NA1_NVRAM_SIZE];

static NVRAM_HANDLER( namcosna1 )
{
	if( read_or_write )
	{
		mame_fwrite( file, namcona1_nvmem, NA1_NVRAM_SIZE );
	}
	else
	{
		if (file)
		{
			mame_fread( file, namcona1_nvmem, NA1_NVRAM_SIZE );
		}
		else
		{
			memset( namcona1_nvmem, 0x00, NA1_NVRAM_SIZE );

			switch( namcona1_gametype )
			{
			case NAMCO_EXBANIA:
				memcpy( namcona1_nvmem, ExvaniaDefaultNvMem, sizeof(ExvaniaDefaultNvMem) );
				break;

			case NAMCO_QUIZTOU:
				memcpy( namcona1_nvmem, QuiztouDefaultNvMem, sizeof(QuiztouDefaultNvMem) );
				break;

			case NAMCO_CGANGPZL:
				memcpy( namcona1_nvmem, CgangpzlDefaultNvMem, sizeof(CgangpzlDefaultNvMem) );
				break;
			}
		}
	}
} /* namcosna1_nvram_handler */

static READ16_HANDLER( namcona1_nvram_r )
{
	return namcona1_nvmem[offset];
} /* namcona1_nvram_r */

static WRITE16_HANDLER( namcona1_nvram_w )
{
	if( ACCESSING_LSB )
	{
		namcona1_nvmem[offset] = data&0xff;
	}
} /* namcona1_nvram_w */

/***************************************************************************/

static INPUT_PORTS_START( namcona1_joy )
	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START3 )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START
	PORT_DIPNAME( 0x01, 0x01, "DIP2 (Freeze)" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP1 (Test)" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )
INPUT_PORTS_END

static INPUT_PORTS_START( namcona1_quiz )
	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START3 )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START
	PORT_DIPNAME( 0x01, 0x01, "DIP2 (Freeze)" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP1 (Test)" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )
INPUT_PORTS_END

static INPUT_PORTS_START( namcona2_joy )
	PORT_START
	PORT_DIPNAME( 0x01, 0x00, "DIP2 (Freeze)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIP1 (Test)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Test ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "SERVICE" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START3 )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START4 )

	PORT_START
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN4 )
INPUT_PORTS_END

static INPUT_PORTS_START( namcona2_quiz )
	PORT_START
	PORT_DIPNAME( 0x01, 0x00, "DIP2 (Freeze)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIP1 (Test)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Test ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "SERVICE" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START3 )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START4 )

	PORT_START
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN4 )
INPUT_PORTS_END

/***************************************************************************/

static void
simulate_mcu( void )
{
	int i;
	UINT16 data;
	UINT8 poll_coins;

	namcona1_workram[0xf60/2] = 0x0000; /* mcu ready */

	if (is_na2)
	{
		mcu_ram[0xfc0/2] = readinputport(0x0); /* dipswitch */

		for( i=1; i<=4; i++ )
		{
			data = readinputport(i)<<8;
			switch( namcona1_gametype )
			{
			case NAMCO_KNCKHEAD:
			case NAMCO_BKRTMAQ:
			case NAMCO_QUIZTOU:
			case NAMCO_EXBANIA:
				data |= data>>8;
				break;

			case NAMCO_XDAY2:
				data |= data>>8; /* wrong! */
				break;

			case NAMCO_TINKLPIT:
				if( data&0x2000 ) data |= 0x20; /* throw */
				if( data&0x4000 ) data |= 0x10; /* jump */
				if( i==1 )
				{
					if( readinputport(1)&0x80 ) data |= 0x80; /* P1 start */
				}
				if( i==2 )
				{
					if( readinputport(2)&0x80 ) data |= 0x80; /* P2 start */
				}
				break;

			default:
				break;
			}
			mcu_ram[0xfc0/2+i] = data;
		}

		/* "analog" and "encoder" ports are polled during test mode,
         * but I haven't found any games that make use of them.
         */
		mcu_ram[0xfc0/2+0x05] = 0xffff; /* analog0,1 */
		mcu_ram[0xfc0/2+0x06] = 0xffff; /* analog2,3 */
		mcu_ram[0xfc0/2+0x07] = 0xffff; /* analog4,5 */
		mcu_ram[0xfc0/2+0x08] = 0xffff; /* analog6,7 */
		mcu_ram[0xfc0/2+0x09] = 0xffff; /* encoder0,1 */

		poll_coins = readinputport(5); /* coin input */
		if( (poll_coins&0x8)&~(mCoinState&0x8) ) mCoinCount[0]++;
		if( (poll_coins&0x4)&~(mCoinState&0x4) ) mCoinCount[1]++;
		if( (poll_coins&0x2)&~(mCoinState&0x2) ) mCoinCount[2]++;
		if( (poll_coins&0x1)&~(mCoinState&0x1) ) mCoinCount[3]++;
		mCoinState = poll_coins;

		mcu_ram[0xfc0/2+0xa] = (mCoinCount[0]<<8)|mCoinCount[1];
		mcu_ram[0xfc0/2+0xb] = (mCoinCount[2]<<8)|mCoinCount[3];

		/* special handling for F/A */
		data = ~((readinputport(1)<<8)|readinputport(2));
		mcu_ram[0xffc/2] = data;
		mcu_ram[0xffe/2] = data;

		if( namcona1_gametype == NAMCO_XDAY2 )
		{
			int p1 = readinputport(1);
			int p2 = readinputport(2);
			UINT32 code = 0;
			if( p2&0x40 ) code |= 0x2000; // enter (top-level of self-test)
			if( p2&0x20 ) code |= 0x1000; // exit  (top-level of self-test)
			if( p1&0x40 ) code |= 0x0020; // next  (top-level of self-test)
			if( p1&0x20 ) code |= 0x0010; // prev  (top-level of self-test)
			code = ~code;
			mcu_ram[0xffc/2] = code>>16;
			mcu_ram[0xffe/2] = code&0xffff;
		}
	}
} /* simulate_mcu */

#ifdef UNUSED_FUNCTION
static READ16_HANDLER( namcona1_mcu_r )
{
	return mcu_ram[offset];
}
#endif

static WRITE16_HANDLER( namcona1_mcu_w )
{
	COMBINE_DATA( &mcu_ram[offset] );
	if( offset>=0x400/2 && (offset<0x820/2 || (offset>=0xf30/2 && offset<0xf72/2)) )
	{
		logerror( "0x%03x: 0x%04x\n", offset*2, mcu_ram[offset] );
	}
	/*
        400..53d  code for MCU?

        820:                    song select
        822:                    song control (volume? tempo?)
        824,826,828,....89e:    sample select
            0x40 is written to odd addresses to signal the MCU that a sound command has been issued

        8f0: 0x07 unknown
        8f2: 0x01 unknown
        8f4: 0xa4 unknown

        f30..f71    data for MCU
        f72: MCU command:
            0x07 = identify version
            0x03 = process data
            0x87 = ?

        fc0..fc9: used by knuckleheads (NA2-specific?)

        fd8: ?

        fbf: watchdog
    */
}

/* NA2 hardware sends a special command to the MCU, then tests to
 * see if the proper BIOS version string appears in shared memory.
 */
static void write_version_info( void )
{
	static const UINT16 source[0x8] =
	{ /* "NSA-BIOS ver"... */
		0x534e,0x2d41,0x4942,0x534f,0x7620,0x7265,0x2e31,0x3133
	};
	int i;
	for( i=0; i<8; i++ )
	{
		namcona1_workram[0x1000/2+i] = source[i];
	}
} /* write_version_info */

static WRITE16_HANDLER( mcu_command_w )
{
	UINT16 cmd = mcu_ram[0xf72/2]>>8;

	switch( cmd ){
	case 0x03:
		/* Process data at 0xf30..0xf71
         *
         * f30: 0101 0020 0400 013e 8a00 0000 0000 011e
         * f40: 0301 0000 0000 0000 8a16 0000 0000 012c
         * f50: 0301 0000 0000 0000 8a61 0000 0000 019e
         * f60: 0301 0000 0000 0000 8a61 0000 0000 01ae
         * f70: 0000
         *
         * f30: 0301 0000 0000 0000 8a88 0000 0000 0120
         * f40: 0301 0000 0000 0000 8ad1 0000 0000 015a
         * f50: 0301 0000 0000 0000 8af6 0000 0000 011c
         * f60: 0301 0000 0000 0000 8b08 0000 0000 0114
         * f70: 0000
         *
         * f30: 0300 0000 0000 0000 8b1f 2004 0000 0000
         * f40: 0301 0000 0000 0000 8b33 0000 0000 8902
         * f50: 0000
         */
		break;

	case 0x07:
		/* This command is used to detect Namco NA-2 hardware; without it,
         * NA-2 games (Knuckleheads, Numan Athletics) refuse to run.
         */
		write_version_info();
		break;
	}
} /* mcu_command_w */

/***************************************************************************/
/* sound
 *
 * 8 bit signed PCM data
 * copied to workram
 *
 *  0x01fffc: pointer
 *  0x020000: samples
 *  0x040000: samples
 *  0x060000: samples
 *
 *  0x070000: metadata; 10 byte frames
 */

/***************************************************************************/

/**
 * "Custom Key" Emulation
 *
 * The primary purpose of the custom key chip is protection.  It prevents
 * ROM swaps from working with games that otherwise run on the same hardware.
 *
 * The secondary purpose of the custom key chip is to act as a random number
 * generator in some games.
 */
static READ16_HANDLER( custom_key_r )
{
	static UINT8 keyseq;
	static UINT16 count;
	int old_count;

	old_count = count;
	do
	{
		count = mame_rand(Machine);
	} while( old_count == count );

	switch( namcona1_gametype )
	{
	case NAMCO_BKRTMAQ:
		if( offset==2 ) return 0x015c;
		break;

	case NAMCO_FA:
		if( offset==2 ) return 0x015d;
		if( offset==4 ) return count;
		break;

	case NAMCO_EXBANIA:
		if( offset==2 ) return 0x015e;
		break;

	case NAMCO_CGANGPZL:
		if( offset==1 ) return 0x0164;
		if( offset==2 ) return count;
		break;

	case NAMCO_SWCOURT:
		if( offset==1 ) return 0x0165;
		if( offset==2 ) return count;
		break;

	case NAMCO_EMERALDA:
		if( offset==1 ) return 0x0166;
		if( offset==2 ) return count;
		break;

	case NAMCO_NUMANATH:
		if( offset==1 ) return 0x0167;
		if( offset==2 ) return count;
		break;

	case NAMCO_KNCKHEAD:
		if( offset==1 ) return 0x0168;
		if( offset==2 ) return count;
		break;

	case NAMCO_QUIZTOU:
		if( offset==2 ) return 0x016d;
		break;

	case NAMCO_TINKLPIT:
		if( offset==7 ) return 0x016f;
		if( offset==4 ) keyseq = 0;
		if( offset==3 )
		{
			static const UINT16 data[] =
			{
				0x0000,0x2000,0x2100,0x2104,0x0106,0x0007,0x4003,0x6021,
				0x61a0,0x31a4,0x9186,0x9047,0xc443,0x6471,0x6db0,0x39bc,
				0x9b8e,0x924f,0xc643,0x6471,0x6db0,0x19bc,0xba8e,0xb34b,
				0xe745,0x4576,0x0cb7,0x789b,0xdb29,0xc2ec,0x16e2,0xb491
			};
			return data[(keyseq++)&0x1f];
		}
		break;

	case NAMCO_XDAY2:
		if( offset==2 ) return 0x018a;
		if( offset==3 ) return count;
		break;

	default:
		return 0;
	}
	return mame_rand(Machine)&0xffff;
} /* custom_key_r */

static WRITE16_HANDLER( custom_key_w )
{
} /* custom_key_w */

/***************************************************************/

static READ16_HANDLER( namcona1_vreg_r )
{
	return namcona1_vreg[offset];
} /* namcona1_vreg_r */

static int
transfer_dword( running_machine *machine, UINT32 dest, UINT32 source )
{
	UINT16 data;

	if( source>=0x400000 && source<0xc00000 )
	{
		data = mpBank1[(source-0x400000)/2];
	}
	else if( source>=0xc00000 && source<0xe00000 )
	{
		data = mpBank0[(source-0xc00000)/2];
	}
	else if( source<0x80000 && source>=0x1000 )
	{
		data = namcona1_workram[source/2];
	}
	else
	{
		logerror( "bad blt src %08x\n", source );
		return -1;
	}
	if( dest>=0xf00000 && dest<=0xf02000 )
	{
		namcona1_paletteram_w( machine, (dest-0xf00000)/2, data, 0x0000 );
	}
	else if( dest>=0xf40000 && dest<=0xf80000 )
	{
		namcona1_gfxram_w( machine, (dest-0xf40000)/2, data, 0x0000 );
	}
	else if( dest>=0xff0000 && dest<0xff8000 )
	{
		namcona1_videoram_w( machine, (dest-0xff0000)/2, data, 0x0000 );
	}
	else if( dest>=0xff8000 && dest<=0xffdfff )
	{
		namcona1_sparevram[(dest-0xff8000)/2] = data;
	}
	else if( dest>=0xfff000 && dest<=0xffffff )
	{
		spriteram16[(dest-0xfff000)/2] = data;
	}
	else
	{
		logerror( "bad blt dst %08x\n", dest );
		return -1;
	}
	return 0;
} /* transfer_dword */

static void
blit_setup( int format, int *bytes_per_row, int *pitch, int mode )
{
	if( mode == 3 )
	{
		switch( format )
		{
		case 0x0001:
			*bytes_per_row = 0x1000;
			*pitch = 0x1000;
			break;

		case 0x0081:
			*bytes_per_row = 4*8;
			*pitch = 36*8;
			break;

		default:
//      case 0x00f1:
//      case 0x00f9:
//      case 0x00fd:
			*bytes_per_row = (64 - (format>>2))*0x08;
			*pitch = 0x200;
			break;
		}
	}
	else
	{
		switch( format )
		{
		case 0x00bd: /* Numan Athletics */
			*bytes_per_row = 4;
			*pitch = 0x120;
			break;
		case 0x008d: /* Numan Athletics */
			*bytes_per_row = 8;
			*pitch = 0x120;
			break;

		case 0x0000: /* Numan (used to clear spriteram) */
//      0000 0000 0000 : src0
//      0000 0001 0000 : dst0
//      003d 75a0      : src (7AEB40)
//      ---- ----      : spriteram
//      0800           : numbytes
//      0000           : blit
			*bytes_per_row = 0x10;
			*pitch = 0;
			break;

		case 0x0001:
			*bytes_per_row = 0x1000;
			*pitch = 0x1000;
			break;

		case 0x0401: /* F/A */
			*bytes_per_row = 4*0x40;
			*pitch = 36*0x40;
			break;

		default:
//      case 0x00f1:
//      case 0x0781:
//      case 0x07c1:
//      case 0x07e1:
			*bytes_per_row = (64 - (format>>5))*0x40;
			*pitch = 0x1000;
			break;
		}
	}
} /* blit_setup */

/*
$efff20: sprite control: 0x3a,0x3e,0x3f
            bit 0x01 selects spriteram bank

               0    2    4    6    8    a    c    e
$efff00:    src0 src1 src2 dst0 dst1 dst2 BANK [src
$efff10:    src] [dst dst] #BYT BLIT eINT 001f 0001
$efff20:    003f 003f IACK ---- ---- ---- ---- ----
$efff30:    ---- ---- ---- ---- ---- ---- ---- ----
$efff40:    ---- ---- ---- ---- ---- ---- ---- ----
$efff50:    ---- ---- ---- ---- ---- ---- ---- ----
$efff60:    ---- ---- ---- ---- ---- ---- ---- ----
$efff70:    ---- ---- ---- ---- ---- ---- ---- ----
$efff80:    0050 0170 0020 0100 0000 0000 0000 GFXE
$efff90:    0000 0001 0002 0003 FLIP ---- ---- ----
$efffa0:    PRI  PRI  PRI  PRI  ---- ---- 00c0 ----     priority (0..7)
$efffb0:    COLR COLR COLR COLR 0001 0004 0000 ----     color (0..f)
$efffc0:    ???? ???? ???? ???? ???? ???? ???? ----     ROZ

Emeralda:
$efff80:    0048 0177 0020 0100 0000 00fd 0000 GFXE
*/

static void namcona1_blit( running_machine *machine )
{
	int src0 = namcona1_vreg[0x0];
	int src1 = namcona1_vreg[0x1];
	int src2 = namcona1_vreg[0x2];

	int dst0 = namcona1_vreg[0x3];
	int dst1 = namcona1_vreg[0x4];
	int dst2 = namcona1_vreg[0x5];

	int gfxbank = namcona1_vreg[0x6];

	/* dest and source are provided as dword offsets */
	UINT32 src_baseaddr	= 2*((namcona1_vreg[0x7]<<16)|namcona1_vreg[0x8]);
	UINT32 dst_baseaddr	= 2*((namcona1_vreg[0x9]<<16)|namcona1_vreg[0xa]);

	int num_bytes = namcona1_vreg[0xb];

	int dest_offset, source_offset;
	int dest_bytes_per_row, dst_pitch;
	int source_bytes_per_row, src_pitch;

	(void)dst2;
	(void)dst0;
	(void)src2;
	(void)src0;

/*  logerror( "0x%08x: blt(%08x,%08x,%08x);%04x %04x %04x; %04x %04x %04x; gfx=%04x\n",
        activecpu_get_pc(),
        dst_baseaddr,src_baseaddr,num_bytes,
        src0,src1,src2,
        dst0,dst1,dst2,
        gfxbank );*/

	blit_setup( dst1, &dest_bytes_per_row, &dst_pitch, gfxbank);
	blit_setup( src1, &source_bytes_per_row, &src_pitch, gfxbank );

	if( num_bytes&1 )
	{
		num_bytes++;
	}

	if( dst_baseaddr < 0xf00000 )
	{
		dst_baseaddr += 0xf40000;
	}

	dest_offset		= 0;
	source_offset	= 0;

	while( num_bytes>0 )
	{
		if( transfer_dword(machine,
			dst_baseaddr + dest_offset,
			src_baseaddr + source_offset ) )
		{
			return;
		}

		num_bytes -= 2;

		dest_offset+=2;
		if( dest_offset >= dest_bytes_per_row )
		{
			dst_baseaddr += dst_pitch;
			dest_offset = 0;
		}

		source_offset+=2;
		if( source_offset >= source_bytes_per_row )
		{
			src_baseaddr += src_pitch;
			source_offset = 0;
		}
	}
} /* namcona1_blit */

static WRITE16_HANDLER( namcona1_vreg_w )
{
	COMBINE_DATA( &namcona1_vreg[offset] );

	switch( offset )
	{
	case 0x18/2:
		namcona1_blit(machine);
		/* see also 0x1e */
		break;

	case 0x1a/2:
		mEnableInterrupts = 1;
		/* interrupt enable mask; 0 enables INT level */
		break;
	}
} /* namcona1_vreg_w */

/***************************************************************/

static WRITE16_HANDLER( bogus_w )
{
//  DEBUGGER_BREAK;
}
static READ16_HANDLER( bogus_r )
{
//  DEBUGGER_BREAK;
	return 0;
}

// MCU "mailslot" handler - has 8 16-bit slots mirrored
static UINT16 mcu_mailbox[8];

static READ16_HANDLER( mcu_mailbox_r )
{
	return mcu_mailbox[offset%8];
}

static WRITE16_HANDLER( mcu_mailbox_w_68k )
{
//  logerror("mailbox_w_68k: %x @ %x\n", data, offset);

	if (offset == 4) cpunum_set_input_line(Machine, 1, M37710_LINE_IRQ0, HOLD_LINE);

	COMBINE_DATA(&mcu_mailbox[offset%8]);
}

static WRITE16_HANDLER( mcu_mailbox_w_mcu )
{
	COMBINE_DATA(&mcu_mailbox[offset%8]);
}

static ADDRESS_MAP_START( namcona1_mcu_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_RAM AM_BASE(&namcona1_workram)
	AM_RANGE(0x080000, 0x3f7fff) AM_READ(bogus_r)
	AM_RANGE(0x3f8000, 0x3fffff) AM_READ(mcu_mailbox_r)
	AM_RANGE(0x400000, 0xbfffff) AM_ROM AM_REGION(REGION_CPU1, 0x280000)	/* data */
	AM_RANGE(0xc00000, 0xdfffff) AM_ROM AM_REGION(REGION_CPU1, 0x080000)	/* code */
	AM_RANGE(0xe00000, 0xe00fff) AM_READ(namcona1_nvram_r)
	AM_RANGE(0xe40000, 0xe4000f) AM_READ(custom_key_r)
	AM_RANGE(0xe40010, 0xeffeff) AM_READ(bogus_r)
	AM_RANGE(0xefff00, 0xefffff) AM_READ(namcona1_vreg_r)
	AM_RANGE(0xf00000, 0xf01fff) AM_READ(namcona1_paletteram_r)
	AM_RANGE(0xf02000, 0xf3ffff) AM_READ(bogus_r)
	AM_RANGE(0xf40000, 0xf7ffff) AM_READ(namcona1_gfxram_r)
	AM_RANGE(0xf80000, 0xfeffff) AM_READ(bogus_r)
	AM_RANGE(0xff0000, 0xff7fff) AM_READ(namcona1_videoram_r)
	AM_RANGE(0xff8000, 0xffdfff) AM_READ(SMH_RAM)		/* spare videoram */
	AM_RANGE(0xffe000, 0xffefff) AM_READ(SMH_RAM)		/* scroll registers */
	AM_RANGE(0xfff000, 0xffffff) AM_READ(SMH_RAM)		/* spriteram */
ADDRESS_MAP_END

static ADDRESS_MAP_START( namcona1_mcu_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_RAM
	AM_RANGE(0x080000, 0x3f7fff) AM_WRITE(bogus_w)
	AM_RANGE(0x3f8000, 0x3fffff) AM_WRITE(mcu_mailbox_w_68k)
	AM_RANGE(0x400000, 0xdfffff) AM_WRITE(SMH_ROM) /* data + code */
	AM_RANGE(0xe00000, 0xe00fff) AM_WRITE(namcona1_nvram_w)
	AM_RANGE(0xe01000, 0xe3ffff) AM_WRITE(bogus_w)
	AM_RANGE(0xe40000, 0xe4000f) AM_WRITE(custom_key_w)
	AM_RANGE(0xe40010, 0xeffeff) AM_WRITE(bogus_w)
	AM_RANGE(0xefff00, 0xefffff) AM_WRITE(namcona1_vreg_w) AM_BASE(&namcona1_vreg)
	AM_RANGE(0xf00000, 0xf01fff) AM_WRITE(namcona1_paletteram_w) AM_BASE(&paletteram16)
	AM_RANGE(0xf02000, 0xf3ffff) AM_WRITE(bogus_w)
	AM_RANGE(0xf40000, 0xf7ffff) AM_WRITE(namcona1_gfxram_w)
	AM_RANGE(0xf80000, 0xfeffff) AM_WRITE(bogus_w)
	AM_RANGE(0xff0000, 0xff7fff) AM_WRITE(namcona1_videoram_w) AM_BASE(&videoram16)
	AM_RANGE(0xff8000, 0xffdfff) AM_WRITE(SMH_RAM) AM_BASE(&namcona1_sparevram)
	AM_RANGE(0xffe000, 0xffefff) AM_WRITE(SMH_RAM) AM_BASE(&namcona1_scroll)
	AM_RANGE(0xfff000, 0xffffff) AM_WRITE(SMH_RAM) AM_BASE(&spriteram16)
ADDRESS_MAP_END

/* ----- NA-1 MCU handling ----------------------------------- */

static READ16_HANDLER( na1mcu_shared_r )
{
	UINT16 data;

	data = namcona1_workram[offset];

#if 0
	if (offset >= 0x70000/2)
	{
		logerror("MD: %04x @ %x PC %x\n", ((data>>8)&0xff) | ((data<<8)&0xff00), offset*2, activecpu_get_pc());
	}
#endif

	return ((data>>8)&0xff) | ((data<<8)&0xff00);
}

static WRITE16_HANDLER( na1mcu_shared_w )
{
	mem_mask = ((mem_mask>>8)&0xff) | ((mem_mask<<8)&0xff00);
	data = ((data>>8)&0xff) | ((data<<8)&0xff00);

	COMBINE_DATA(&namcona1_workram[offset]);
}

static READ16_HANDLER(snd_r)
{
	return C140_r(machine,offset*2+1) | C140_r(machine,offset*2)<<8;
}

static WRITE16_HANDLER(snd_w)
{
	if (ACCESSING_LSB16)
	{
		C140_w(machine,(offset*2)+1, data);
	}

	if (ACCESSING_MSB16)
	{
		C140_w(machine,(offset*2), data>>8);
	}
}

static ADDRESS_MAP_START( namcona1_mcu_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000800, 0x000fff) AM_READWRITE(mcu_mailbox_r, mcu_mailbox_w_mcu)	// "Mailslot" communications ports
	AM_RANGE(0x001000, 0x001fff) AM_READWRITE(snd_r, snd_w)				// C140-alike sound chip
	AM_RANGE(0x002000, 0x002fff) AM_READWRITE(na1mcu_shared_r, na1mcu_shared_w)	// mirror of first page of shared work RAM
	AM_RANGE(0x003000, 0x00afff) AM_RAM						// there is a 32k RAM chip according to CGFM
	AM_RANGE(0x00c000, 0x00ffff) AM_ROM AM_REGION(REGION_CPU2, 0)			// internal ROM BIOS
	AM_RANGE(0x200000, 0x27ffff) AM_READWRITE(na1mcu_shared_r, na1mcu_shared_w)	// shared work RAM
ADDRESS_MAP_END

static UINT8 mcu_port4, mcu_port5, mcu_port6, mcu_port8;

// port 4: bit 3 (0x08) enables the 68000 (see the 68k launch code at c604 in swcourt's BIOS)
static READ8_HANDLER( port4_r )
{
	return mcu_port4;
}

static WRITE8_HANDLER( port4_w )
{
	if ((data & 0x08) && !(mcu_port4 & 0x08))
	{
		logerror("launching 68k, PC=%x\n", activecpu_get_pc());

		// reset and launch the 68k
		cpunum_set_input_line(Machine, 0, INPUT_LINE_RESET, CLEAR_LINE);
	}

	mcu_port4 = data;
}

// port 5: not sure yet, but MCU code requires this interaction at least
static READ8_HANDLER( port5_r )
{
	return mcu_port5;
}

static WRITE8_HANDLER( port5_w )
{
	mcu_port5 = data;

	// bit 0 must mirror bit 1 - this is checked at CCD3 in the C69 BIOS
	mcu_port5 &= 0xfe;
	mcu_port5 |= ((mcu_port5 & 0x2)>>1);
}

static READ8_HANDLER( port6_r )
{
	return 0;
}

static WRITE8_HANDLER( port6_w )
{
	mcu_port6 = data;
}

static READ8_HANDLER( port7_r )
{
	switch (mcu_port6 & 0xe0)
	{
		case 0x40:
			return readinputport(0);
			break;

  		case 0x60:
			return readinputport(1);
			break;

		case 0x20:
			return readinputport(4);
			break;

		case 0x00:
			return readinputport(3);
			break;
	}

	return 0xff;
}

static WRITE8_HANDLER( port7_w )
{
}

// port 8: bit 5 (0x20) toggles, watchdog?
static READ8_HANDLER( port8_r )
{
	return mcu_port8;
}

static WRITE8_HANDLER( port8_w )
{
	mcu_port8 = data;
}


static MACHINE_START( namcona1 )
{
	C140_set_base(0, namcona1_workram);
}

// for games with the MCU emulated, the MCU boots the 68000.  don't allow it before that.
static MACHINE_RESET( namcona1_mcu )
{
	cpunum_set_input_line(machine, 0, INPUT_LINE_RESET, ASSERT_LINE);

	mcu_port5 = 1;
}

// "encrypt" player 3 inputs
// each bit of player 3's inputs is split across one of the 8 analog input ports
// bit 6 => port 0
// bit 5 => port 1
// bit 4 => port 2
// bit 0 => port 3
// bit 1 => port 4
// bit 2 => port 5
// bit 3 => port 6
// bit 7 => port 7
static READ8_HANDLER( portana_r )
{
	static const UINT8 bitnum[8] = { 0x40, 0x20, 0x10, 0x01, 0x02, 0x04, 0x08, 0x80 };
	UINT8 port = readinputport(2);

	return (port & bitnum[offset>>1]) ? 0xff : 0x00;
}

static ADDRESS_MAP_START( namcona1_mcu_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(M37710_PORT4, M37710_PORT4) AM_READWRITE( port4_r, port4_w )
	AM_RANGE(M37710_PORT5, M37710_PORT5) AM_READWRITE( port5_r, port5_w )
	AM_RANGE(M37710_PORT6, M37710_PORT6) AM_READWRITE( port6_r, port6_w )
	AM_RANGE(M37710_PORT7, M37710_PORT7) AM_READWRITE( port7_r, port7_w )
	AM_RANGE(M37710_PORT8, M37710_PORT8) AM_READWRITE( port8_r, port8_w )
	AM_RANGE(0x10, 0x1f) AM_READ( portana_r )
ADDRESS_MAP_END

static INTERRUPT_GEN( namcona1_interrupt )
{
	int level = cpu_getiloops(); /* 0,1,2,3,4 */
	if( level==0 )
	{
		simulate_mcu();
	}
	if( mEnableInterrupts )
	{
		if( (namcona1_vreg[0x1a/2]&(1<<level))==0 )
		{
			cpunum_set_input_line(machine, 0, level+1, HOLD_LINE);
		}
	}
}

// MCU interrupts: IRQ 0 => process mail slot (probably set on mail slot write from 68k)
//                 IRQ 1 =>
//                 IRQ 2 =>

static INTERRUPT_GEN( mcu_interrupt )
{
	if (cpu_getiloops() == 0)
 		cpunum_set_input_line(machine, 1, M37710_LINE_IRQ1, HOLD_LINE);
	else if (cpu_getiloops() == 1)
	{
		cpunum_set_input_line(machine, 1, M37710_LINE_ADC, HOLD_LINE);
	}
}

static const struct C140interface C140_interface_typeA =
{
	C140_TYPE_ASIC219,
	0
};

/* cropped at sides */
static MACHINE_DRIVER_START( namcona1 )
	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, 50113000/4)
	MDRV_CPU_PROGRAM_MAP(namcona1_mcu_readmem,namcona1_mcu_writemem)
	MDRV_CPU_VBLANK_INT_HACK(namcona1_interrupt,5)

	MDRV_CPU_ADD(M37710, 50113000/4)
	MDRV_CPU_PROGRAM_MAP(namcona1_mcu_map, 0)
	MDRV_CPU_IO_MAP( namcona1_mcu_io_map, 0 )
	MDRV_CPU_VBLANK_INT_HACK(mcu_interrupt, 2)

	MDRV_NVRAM_HANDLER(namcosna1)
	MDRV_MACHINE_START(namcona1)
	MDRV_MACHINE_RESET(namcona1_mcu)
	MDRV_INTERLEAVE(40)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(38*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(8, 38*8-1-8, 4*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x1000)

	MDRV_VIDEO_START(namcona1)
	MDRV_VIDEO_UPDATE(namcona1)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(C140, 42667)
	MDRV_SOUND_CONFIG(C140_interface_typeA)
	MDRV_SOUND_ROUTE(0, "right", 1.00)
	MDRV_SOUND_ROUTE(1, "left", 1.00)
MACHINE_DRIVER_END


/* full-width */
static MACHINE_DRIVER_START( namcona1w )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(namcona1)

	/* video hardware */
	MDRV_SCREEN_MODIFY("main")
	MDRV_SCREEN_VISIBLE_AREA(0, 38*8-1-0, 4*8, 32*8-1)
MACHINE_DRIVER_END

static ADDRESS_MAP_START( namcona2_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_READ(SMH_RAM)		/* work RAM */
	AM_RANGE(0x080000, 0x3fffff) AM_READ(bogus_r)
	AM_RANGE(0x400000, 0xbfffff) AM_ROM AM_REGION(REGION_CPU1, 0x280000)	/* data */
	AM_RANGE(0xc00000, 0xdfffff) AM_ROM AM_REGION(REGION_CPU1, 0x080000)	/* code */
	AM_RANGE(0xe00000, 0xe00fff) AM_READ(namcona1_nvram_r)
	AM_RANGE(0xe40000, 0xe4000f) AM_READ(custom_key_r)
	AM_RANGE(0xe40010, 0xeffeff) AM_READ(bogus_r)
	AM_RANGE(0xefff00, 0xefffff) AM_READ(namcona1_vreg_r)
	AM_RANGE(0xf00000, 0xf01fff) AM_READ(namcona1_paletteram_r)
	AM_RANGE(0xf02000, 0xf3ffff) AM_READ(bogus_r)
	AM_RANGE(0xf40000, 0xf7ffff) AM_READ(namcona1_gfxram_r)
	AM_RANGE(0xf80000, 0xfeffff) AM_READ(bogus_r)
	AM_RANGE(0xff0000, 0xff7fff) AM_READ(namcona1_videoram_r)
	AM_RANGE(0xff8000, 0xffdfff) AM_READ(SMH_RAM)		/* spare videoram */
	AM_RANGE(0xffe000, 0xffefff) AM_READ(SMH_RAM)		/* scroll registers */
	AM_RANGE(0xfff000, 0xffffff) AM_READ(SMH_RAM)		/* spriteram */
ADDRESS_MAP_END

static ADDRESS_MAP_START( namcona2_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x000fff) AM_WRITE(namcona1_mcu_w) AM_BASE(&mcu_ram)
	AM_RANGE(0x000000, 0x07ffff) AM_WRITE(SMH_RAM) AM_BASE(&namcona1_workram)
	AM_RANGE(0x080000, 0x3f8007) AM_WRITE(bogus_w)
	AM_RANGE(0x3f8008, 0x3f8009) AM_WRITE(mcu_command_w)
	AM_RANGE(0x3f800a, 0x3fffff) AM_WRITE(bogus_w)
	AM_RANGE(0x400000, 0xdfffff) AM_WRITE(SMH_ROM) /* data + code */
	AM_RANGE(0xe00000, 0xe00fff) AM_WRITE(namcona1_nvram_w)
	AM_RANGE(0xe01000, 0xe3ffff) AM_WRITE(bogus_w)
	AM_RANGE(0xe40000, 0xe4000f) AM_WRITE(custom_key_w)
	AM_RANGE(0xe40010, 0xeffeff) AM_WRITE(bogus_w)
	AM_RANGE(0xefff00, 0xefffff) AM_WRITE(namcona1_vreg_w) AM_BASE(&namcona1_vreg)
	AM_RANGE(0xf00000, 0xf01fff) AM_WRITE(namcona1_paletteram_w) AM_BASE(&paletteram16)
	AM_RANGE(0xf02000, 0xf3ffff) AM_WRITE(bogus_w)
	AM_RANGE(0xf40000, 0xf7ffff) AM_WRITE(namcona1_gfxram_w)
	AM_RANGE(0xf80000, 0xfeffff) AM_WRITE(bogus_w)
	AM_RANGE(0xff0000, 0xff7fff) AM_WRITE(namcona1_videoram_w) AM_BASE(&videoram16)
	AM_RANGE(0xff8000, 0xffdfff) AM_WRITE(SMH_RAM) AM_BASE(&namcona1_sparevram)
	AM_RANGE(0xffe000, 0xffefff) AM_WRITE(SMH_RAM) AM_BASE(&namcona1_scroll)
	AM_RANGE(0xfff000, 0xffffff) AM_WRITE(SMH_RAM) AM_BASE(&spriteram16)
ADDRESS_MAP_END

static struct NAMCONAinterface NAMCONA_interface =
{
	NULL,
	0x70000/2
};

static MACHINE_DRIVER_START( namcona2 )
	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, 50113000/4)
	MDRV_CPU_PROGRAM_MAP(namcona2_readmem,namcona2_writemem)
	MDRV_CPU_VBLANK_INT_HACK(namcona1_interrupt,5)

	MDRV_NVRAM_HANDLER(namcosna1)
	MDRV_INTERLEAVE(40)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(38*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(8, 38*8-1-8, 4*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x1000)

	MDRV_VIDEO_START(namcona1)
	MDRV_VIDEO_UPDATE(namcona1)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(NAMCONA, 4*8000)
	MDRV_SOUND_CONFIG(NAMCONA_interface)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)
MACHINE_DRIVER_END

static void
init_namcona1( int gametype )
{
	UINT16 *pMem = (UINT16 *)memory_region( REGION_CPU1 );

	namcona1_gametype = gametype;
	mpBank0 = &pMem[0x80000/2];
	mpBank1 = mpBank0 +  0x200000/2;

	is_na2 = 0;
	mEnableInterrupts = 0;
}

static void
init_namcona2( int gametype )
{
	UINT16 *pMem = (UINT16 *)memory_region( REGION_CPU1 );

	namcona1_workram[0] = 0x0007; namcona1_workram[1] = 0xfffc; /* (?) stack */
	namcona1_workram[2] = 0x00c0; namcona1_workram[3] = 0x0000; /* reset vector */

	namcona1_gametype = gametype;
	mpBank0 = &pMem[0x80000/2];
	mpBank1 = mpBank0 +  0x200000/2;

	is_na2 = 1;
	mCoinCount[0] = mCoinCount[1] = mCoinCount[2] = mCoinCount[3] = 0;
	mCoinState = 0;
	mEnableInterrupts = 0;

	NAMCONA_interface.memory_base = namcona1_workram;
	if (namcona1_gametype == NAMCO_KNCKHEAD)
		NAMCONA_interface.metadata_offset = 0x10000/2;
	else
		NAMCONA_interface.metadata_offset = 0x70000/2;
}

static DRIVER_INIT( bkrtmaq ){		init_namcona1(NAMCO_BKRTMAQ); }
static DRIVER_INIT( cgangpzl ){	init_namcona1(NAMCO_CGANGPZL); }
static DRIVER_INIT( emeralda ){	init_namcona2(NAMCO_EMERALDA); } /* NA-2 Hardware */
static DRIVER_INIT( emeraldj ){	init_namcona1(NAMCO_EMERALDA); } /* NA-1 Hardware */
static DRIVER_INIT( exbania ){		init_namcona1(NAMCO_EXBANIA); }
static DRIVER_INIT( fa ){	        init_namcona1(NAMCO_FA); }
static DRIVER_INIT( knckhead ){	init_namcona2(NAMCO_KNCKHEAD); }
static DRIVER_INIT( numanath ){	init_namcona2(NAMCO_NUMANATH); }
static DRIVER_INIT( quiztou ){		init_namcona2(NAMCO_QUIZTOU); }
static DRIVER_INIT( swcourt ){		init_namcona1(NAMCO_SWCOURT); }
static DRIVER_INIT( tinklpit ){	init_namcona1(NAMCO_TINKLPIT); }
static DRIVER_INIT( xday2 ){		init_namcona2(NAMCO_XDAY2); }

ROM_START( bkrtmaq )
	ROM_REGION( 0xa80000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "mq1-ep0l.bin", 0x080001, 0x080000, CRC(f029bc57) SHA1(fdbf8b8b9f69d5755ca5197dda4f887b12dd66f4) ) /* 0xc00000 */
	ROM_LOAD16_BYTE( "mq1-ep0u.bin", 0x080000, 0x080000, CRC(4cff62b8) SHA1(5cac170dcfbeb3dcfa0840bdbe7541a9d2f44a14) )
	ROM_LOAD16_BYTE( "mq1-ep1l.bin", 0x180001, 0x080000, CRC(e3be6f4b) SHA1(75d9a4cff25e63a9d6c092aa6e241eccd1c61f91) )
	ROM_LOAD16_BYTE( "mq1-ep1u.bin", 0x180000, 0x080000, CRC(b44e31b2) SHA1(3d8c63789b98ada3663ba9e28c370815a9a9c3ed) )

	ROM_LOAD16_BYTE( "mq1-ma0l.bin", 0x280001, 0x100000, CRC(11fed35f) SHA1(511d98b6b42b330238a1874bca031b1892654a48) ) /* 0x400000 */
	ROM_LOAD16_BYTE( "mq1-ma0u.bin", 0x280000, 0x100000, CRC(23442ac0) SHA1(fac706f24045d51a2712f51530967140ea8e875f) )
	ROM_LOAD16_BYTE( "mq1-ma1l.bin", 0x480001, 0x100000, CRC(fe82205f) SHA1(860cc7a96ae3f848ce594077c1362e4e22a36908) )
	ROM_LOAD16_BYTE( "mq1-ma1u.bin", 0x480000, 0x100000, CRC(0cdb6bd0) SHA1(b8b398477c9654e96921110fb30c754240183897) )

	/* M37702 BIOS - labeled as Namco custom C69 */
	ROM_REGION16_LE( 0x4000, REGION_CPU2, 0 )
        ROM_LOAD( "c69.bin",      0x000000, 0x004000, CRC(349134d9) SHA1(61a4981fc2716c228b6121fedcbf1ed6f34dc2de) )
ROM_END

ROM_START( cgangpzl )
	ROM_REGION( 0xa80000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "cp2-ep0l.bin", 0x080001, 0x80000, CRC(8f5cdcc5) SHA1(925db3f3f16224bc28f97a57aba0ab2b51c5067c) ) /* 0xc00000 */
	ROM_LOAD16_BYTE( "cp2-ep0u.bin", 0x080000, 0x80000, CRC(3a816140) SHA1(613c367e08a0a20ec62e1938faab0128743b26f8) )

	/* M37702 BIOS - labeled as Namco custom C69 */
	ROM_REGION16_LE( 0x4000, REGION_CPU2, 0 )
        ROM_LOAD( "c69.bin",      0x000000, 0x004000, CRC(349134d9) SHA1(61a4981fc2716c228b6121fedcbf1ed6f34dc2de) )
ROM_END

ROM_START( cgangpzj )
	ROM_REGION( 0xa80000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "cp1-ep0l.bin", 0x080001, 0x80000, CRC(2825f7ba) SHA1(5f6f8df6bdf0f45656904411cdbb31fdcf8f3be0) ) /* 0xc00000 */
	ROM_LOAD16_BYTE( "cp1-ep0u.bin", 0x080000, 0x80000, CRC(94d7d6fc) SHA1(2460741e0dbb2ccff28f4fbc419a7507382467d2) )

	/* M37702 BIOS - labeled as Namco custom C69 */
	ROM_REGION16_LE( 0x4000, REGION_CPU2, 0 )
        ROM_LOAD( "c69.bin",      0x000000, 0x004000, CRC(349134d9) SHA1(61a4981fc2716c228b6121fedcbf1ed6f34dc2de) )
ROM_END

ROM_START( emeraldj ) /* NA-1 Game PCB, parent is NA-2 version listed below */
	ROM_REGION( 0xa80000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "em1-ep0lb.bin", 0x080001, 0x080000, CRC(fcd55293) SHA1(fdabf9d5f528c37196ac1e031b097618b4c887b5) ) /* 0xc00000 */
	ROM_LOAD16_BYTE( "em1-ep0ub.bin", 0x080000, 0x080000, CRC(a52f00d5) SHA1(85f95d2a69a2df2e9195f55583645c064b0b6fe6) )
	ROM_LOAD16_BYTE( "em1-ep1l.bin",  0x180001, 0x080000, CRC(373c1c59) SHA1(385cb3bc056b798878de890dbff97a8bdd48fe4e) )
	ROM_LOAD16_BYTE( "em1-ep1u.bin",  0x180000, 0x080000, CRC(4e969152) SHA1(2c89ae5d43585f479f16cf8278f8fc001e077e45) )

	/* M37702 BIOS - labeled as Namco custom C69 */
	ROM_REGION16_LE( 0x4000, REGION_CPU2, 0 )
        ROM_LOAD( "c69.bin",      0x000000, 0x004000, CRC(349134d9) SHA1(61a4981fc2716c228b6121fedcbf1ed6f34dc2de) )
ROM_END

ROM_START( emerldja ) /* NA-1 Game PCB, parent is NA-2 version listed below */
	ROM_REGION( 0xa80000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "em1-ep0l.bin", 0x080001, 0x080000, CRC(443f3fce) SHA1(35b6c834e5716c1e9b55f1e39f4e7336dbbe2d9b) ) /* 0xc00000 */
	ROM_LOAD16_BYTE( "em1-ep0u.bin", 0x080000, 0x080000, CRC(484a2a81) SHA1(1b60c18dfb2aebfd4aa8b2a85a1e90883a1f8e61) )
	ROM_LOAD16_BYTE( "em1-ep1l.bin", 0x180001, 0x080000, CRC(373c1c59) SHA1(385cb3bc056b798878de890dbff97a8bdd48fe4e) )
	ROM_LOAD16_BYTE( "em1-ep1u.bin", 0x180000, 0x080000, CRC(4e969152) SHA1(2c89ae5d43585f479f16cf8278f8fc001e077e45) )

	/* M37702 BIOS - labeled as Namco custom C69 */
	ROM_REGION16_LE( 0x4000, REGION_CPU2, 0 )
        ROM_LOAD( "c69.bin",      0x000000, 0x004000, CRC(349134d9) SHA1(61a4981fc2716c228b6121fedcbf1ed6f34dc2de) )
ROM_END

ROM_START( exvania )
	ROM_REGION( 0xa80000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "ex1-ep0l.bin", 0x080001, 0x080000, CRC(18c12015) SHA1(e4f3524e798545c434549719b377c8b5863f580f) ) /* 0xc00000 */
	ROM_LOAD16_BYTE( "ex1-ep0u.bin", 0x080000, 0x080000, CRC(07d054d1) SHA1(e2d2cb81acd309c519686572804648bef4cbd191) )

	ROM_LOAD16_BYTE( "ex1-ma0l.bin", 0x280001, 0x100000, CRC(17922cd4) SHA1(af92c2335e7110c0c5e712f3148c1534d22d3814) ) /* 0x400000 */
	ROM_LOAD16_BYTE( "ex1-ma0u.bin", 0x280000, 0x100000, CRC(93d66106) SHA1(c5d665db04ae0e8992ef46544e2cb7b0e27c8bfe) )
	ROM_LOAD16_BYTE( "ex1-ma1l.bin", 0x480001, 0x100000, CRC(e4bba6ed) SHA1(6483ef91e5a5b8ddd13a3d889936c39829fa50d6) )
	ROM_LOAD16_BYTE( "ex1-ma1u.bin", 0x480000, 0x100000, CRC(04e7c4b0) SHA1(78180d96cd1fae583617d4d227ed4ee24f2f9e29) )

	/* M37702 BIOS - labeled as Namco custom C69 */
	ROM_REGION16_LE( 0x4000, REGION_CPU2, 0 )
        ROM_LOAD( "c69.bin",      0x000000, 0x004000, CRC(349134d9) SHA1(61a4981fc2716c228b6121fedcbf1ed6f34dc2de) )
ROM_END

ROM_START( fghtatck )
	ROM_REGION( 0xa80000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "fa2-ep0l.bin", 0x080001, 0x080000, CRC(8996db9c) SHA1(ebbe7d4cb2960a346cfbdf38c77638d71b6ba20e) ) /* 0xc00000 */
	ROM_LOAD16_BYTE( "fa2-ep0u.bin", 0x080000, 0x080000, CRC(58d5e090) SHA1(950219d4e9bf440f92e3c8765f47e23a9019d2d1) )
	ROM_LOAD16_BYTE( "fa1-ep1l.bin", 0x180001, 0x080000, CRC(b23a5b01) SHA1(4ba9bc2102fffc93a5ff73a107d557fc0f3beefd) )
	ROM_LOAD16_BYTE( "fa1-ep1u.bin", 0x180000, 0x080000, CRC(de2eb129) SHA1(912993cab1c2edcaf986478f2ae22a2f10edf807) )

	ROM_LOAD16_BYTE( "fa1-ma0l.bin", 0x280001, 0x100000, CRC(a0a95e54) SHA1(da35f8a6a5bc9e2b5b6cacf8eb0d900ef1073a67) ) /* 0x400000 */
	ROM_LOAD16_BYTE( "fa1-ma0u.bin", 0x280000, 0x100000, CRC(1d0135bd) SHA1(2a7f8d09c213629a68376ce0379be61b37711d0a) )
	ROM_LOAD16_BYTE( "fa1-ma1l.bin", 0x480001, 0x100000, CRC(c4adf0a2) SHA1(4cc7adc68b1db7e725a973b31d52720bd7dc1140) )
	ROM_LOAD16_BYTE( "fa1-ma1u.bin", 0x480000, 0x100000, CRC(900297be) SHA1(57bb2078ff104c6f631c67219f80f8ede5ddbd09) )

	/* M37702 BIOS - labeled as Namco custom C69 */
	ROM_REGION16_LE( 0x4000, REGION_CPU2, 0 )
        ROM_LOAD( "c69.bin",      0x000000, 0x004000, CRC(349134d9) SHA1(61a4981fc2716c228b6121fedcbf1ed6f34dc2de) )
ROM_END

ROM_START( fa )
	ROM_REGION( 0xa80000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "fa1-ep0l.bin", 0x080001, 0x080000, CRC(182eee5c) SHA1(49769e3b72b59fc3e7b73364fe97168977dbe66b) ) /* 0xc00000 */
	ROM_LOAD16_BYTE( "fa1-ep0u.bin", 0x080000, 0x080000, CRC(7ea7830e) SHA1(79390943eea0b8029b2b8869233caf27228e776a) )
	ROM_LOAD16_BYTE( "fa1-ep1l.bin", 0x180001, 0x080000, CRC(b23a5b01) SHA1(4ba9bc2102fffc93a5ff73a107d557fc0f3beefd) )
	ROM_LOAD16_BYTE( "fa1-ep1u.bin", 0x180000, 0x080000, CRC(de2eb129) SHA1(912993cab1c2edcaf986478f2ae22a2f10edf807) )

	ROM_LOAD16_BYTE( "fa1-ma0l.bin", 0x280001, 0x100000, CRC(a0a95e54) SHA1(da35f8a6a5bc9e2b5b6cacf8eb0d900ef1073a67) ) /* 0x400000 */
	ROM_LOAD16_BYTE( "fa1-ma0u.bin", 0x280000, 0x100000, CRC(1d0135bd) SHA1(2a7f8d09c213629a68376ce0379be61b37711d0a) )
	ROM_LOAD16_BYTE( "fa1-ma1l.bin", 0x480001, 0x100000, CRC(c4adf0a2) SHA1(4cc7adc68b1db7e725a973b31d52720bd7dc1140) )
	ROM_LOAD16_BYTE( "fa1-ma1u.bin", 0x480000, 0x100000, CRC(900297be) SHA1(57bb2078ff104c6f631c67219f80f8ede5ddbd09) )

	/* M37702 BIOS - labeled as Namco custom C69 */
	ROM_REGION16_LE( 0x4000, REGION_CPU2, 0 )
        ROM_LOAD( "c69.bin",      0x000000, 0x004000, CRC(349134d9) SHA1(61a4981fc2716c228b6121fedcbf1ed6f34dc2de) )
ROM_END

ROM_START( swcourt )
	ROM_REGION( 0xa80000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "sc2-ep0l.4c",  0x080001, 0x080000, CRC(5053a02e) SHA1(8ab5a085969cef5e01be01d8f531233002ea5bff) ) /* 0xc00000 */
	ROM_LOAD16_BYTE( "sc2-ep0u.4f",  0x080000, 0x080000, CRC(7b3fc7fa) SHA1(f96c03a03339b7677b8dc8689d907f2c8895886c) )
	ROM_LOAD16_BYTE( "sc1-ep1l.bin", 0x180001, 0x080000, CRC(fb45cf5f) SHA1(6ded351daa9b39d0b8149100caefc4fa0c598e79) )
	ROM_LOAD16_BYTE( "sc1-ep1u.bin", 0x180000, 0x080000, CRC(1ce07b15) SHA1(b1b28cc480301c9ad642597c7cdd8e9cdec996a6) )

	ROM_LOAD16_BYTE( "sc1-ma0l.bin", 0x280001, 0x100000, CRC(3e531f5e) SHA1(6da56630bdfbb19f1639c539779c180d106f6ee2) ) /* 0x400000 */
	ROM_LOAD16_BYTE( "sc1-ma0u.bin", 0x280000, 0x100000, CRC(31e76a45) SHA1(5c278c167c1025c648ce2da2c3764645e96dcd55) )
	ROM_LOAD16_BYTE( "sc1-ma1l.bin", 0x480001, 0x100000, CRC(8ba3a4ec) SHA1(f881e7b4728f388d18450ba85e13e233071fbc88) )
	ROM_LOAD16_BYTE( "sc1-ma1u.bin", 0x480000, 0x100000, CRC(252dc4b7) SHA1(f1be6bd045495c7a0ecd97f01d1dc8ad341fecfd) )

	/* M37702 BIOS - labeled as Namco custom C69 */
	ROM_REGION16_LE( 0x4000, REGION_CPU2, 0 )
        ROM_LOAD( "c69.bin",      0x000000, 0x004000, CRC(349134d9) SHA1(61a4981fc2716c228b6121fedcbf1ed6f34dc2de) )
ROM_END

ROM_START( swcourtj )
	ROM_REGION( 0xa80000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "sc1-ep0l.4c",  0x080001, 0x080000, CRC(145111dd) SHA1(f8f74f77fb80af2ea37ea8ddbf02c1f3fcaf3fdb) ) /* 0xc00000 */
	ROM_LOAD16_BYTE( "sc1-ep0u.4f",  0x080000, 0x080000, CRC(c721c138) SHA1(5d30d66629d982b54c3bb62118be940dc7b69a6b) )
	ROM_LOAD16_BYTE( "sc1-ep1l.bin", 0x180001, 0x080000, CRC(fb45cf5f) SHA1(6ded351daa9b39d0b8149100caefc4fa0c598e79) )
	ROM_LOAD16_BYTE( "sc1-ep1u.bin", 0x180000, 0x080000, CRC(1ce07b15) SHA1(b1b28cc480301c9ad642597c7cdd8e9cdec996a6) )

	ROM_LOAD16_BYTE( "sc1-ma0l.bin", 0x280001, 0x100000, CRC(3e531f5e) SHA1(6da56630bdfbb19f1639c539779c180d106f6ee2) ) /* 0x400000 */
	ROM_LOAD16_BYTE( "sc1-ma0u.bin", 0x280000, 0x100000, CRC(31e76a45) SHA1(5c278c167c1025c648ce2da2c3764645e96dcd55) )
	ROM_LOAD16_BYTE( "sc1-ma1l.bin", 0x480001, 0x100000, CRC(8ba3a4ec) SHA1(f881e7b4728f388d18450ba85e13e233071fbc88) )
	ROM_LOAD16_BYTE( "sc1-ma1u.bin", 0x480000, 0x100000, CRC(252dc4b7) SHA1(f1be6bd045495c7a0ecd97f01d1dc8ad341fecfd) )

	/* M37702 BIOS - labeled as Namco custom C69 */
	ROM_REGION16_LE( 0x4000, REGION_CPU2, 0 )
        ROM_LOAD( "c69.bin",      0x000000, 0x004000, CRC(349134d9) SHA1(61a4981fc2716c228b6121fedcbf1ed6f34dc2de) )
ROM_END

ROM_START( tinklpit )
	ROM_REGION( 0xa80000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "tk1-ep0l.bin", 0x080001, 0x080000, CRC(fdccae42) SHA1(398384482ccb3eb08bfb9db495513272a5188d92) ) /* 0xc00000 */
	ROM_LOAD16_BYTE( "tk1-ep0u.bin", 0x080000, 0x080000, CRC(62cdb48c) SHA1(73c7b99b117b8dc567bc254b0ffcc117c9d42fb5) )
	ROM_LOAD16_BYTE( "tk1-ep1l.bin", 0x180001, 0x080000, CRC(7e90f104) SHA1(79e371426b2e32dc8f687e4d124d23c251198937) )
	ROM_LOAD16_BYTE( "tk1-ep1u.bin", 0x180000, 0x080000, CRC(9c0b70d6) SHA1(eac44d3470f4c2ddd9c41f82e6398bca0cc8a4fd) )

	ROM_LOAD16_BYTE( "tk1-ma0l.bin", 0x280001, 0x100000, CRC(c6b4e15d) SHA1(55252ba4d904b14940436f1b4dc5e2a6bd163bdf) ) /* 0x400000 */
	ROM_LOAD16_BYTE( "tk1-ma0u.bin", 0x280000, 0x100000, CRC(a3ad6f67) SHA1(54289eed5347defb5464ec5a610a6748909159f6) )
	ROM_LOAD16_BYTE( "tk1-ma1l.bin", 0x480001, 0x100000, CRC(61cfb92a) SHA1(eacf0e7557f33d552045f43a116ff08c533a2771) )
	ROM_LOAD16_BYTE( "tk1-ma1u.bin", 0x480000, 0x100000, CRC(54b77816) SHA1(9341d07858623e1920eaae7b2b90126c7057297e) )
	ROM_LOAD16_BYTE( "tk1-ma2l.bin", 0x680001, 0x100000, CRC(087311d2) SHA1(6fe50f9e08551e57d15a15b01e3822a6cb7c8352) )
	ROM_LOAD16_BYTE( "tk1-ma2u.bin", 0x680000, 0x100000, CRC(5ce20c2c) SHA1(7eaff21714bae44f8b21b6db98f055e04bfbae18) )

	/* M37702 BIOS - labeled as Namco custom C69 */
	ROM_REGION16_LE( 0x4000, REGION_CPU2, 0 )
        ROM_LOAD( "c69.bin",      0x000000, 0x004000, CRC(349134d9) SHA1(61a4981fc2716c228b6121fedcbf1ed6f34dc2de) )
ROM_END


/**************************************
     NA-2 Based games
**************************************/


ROM_START( emeralda ) /* NA-2 Game PCB, clones are NA-1 based; see games listed above */
	ROM_REGION( 0xa80000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "em2-ep0l.6c", 0x080001, 0x080000, CRC(ff1479dc) SHA1(ea945d97ed909be13fb6e062742c7142c0d96c31) ) /* 0xc00000 */
	ROM_LOAD16_BYTE( "em2-ep0u.6f", 0x080000, 0x080000, CRC(ffe750a2) SHA1(d10d31489ae364572d7517dd515a6af2182ac764) )
	ROM_LOAD16_BYTE( "em2-ep1l.7c", 0x180001, 0x080000, CRC(6c3e5b53) SHA1(72b941e28c7fda8cb81240a8226386fe55c14e2d) )
	ROM_LOAD16_BYTE( "em2-ep1u.7f", 0x180000, 0x080000, CRC(dee15a81) SHA1(474a264029bd77e4205773a7461dea695e65933f) )

	/* M37702 BIOS - labeled as Namco custom C70 */
	ROM_REGION16_LE( 0x4000, REGION_CPU2, 0 )
        ROM_LOAD( "c70.bin",      0x000000, 0x004000, NO_DUMP )
ROM_END

ROM_START( knckhead )
	ROM_REGION( 0xa80000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "kh2-ep0l.bin", 0x080001, 0x080000, CRC(b4b88077) SHA1(9af03d1832ad6c77222e18427f4afca330a41ce6) ) /* 0xc00000 */
	ROM_LOAD16_BYTE( "kh2-ep0u.bin", 0x080000, 0x080000, CRC(a578d97e) SHA1(9a5bb6649cca7b98daf538a66c813f61cca2e2ec) )
	ROM_LOAD16_BYTE( "kh1-ep1l.bin", 0x180001, 0x080000, CRC(27e6ab4e) SHA1(66f397cc2117c1e73652c4800c0937e6d8116380) )
	ROM_LOAD16_BYTE( "kh1-ep1u.bin", 0x180000, 0x080000, CRC(487b2434) SHA1(2d62db85ceac1fca61c39e4db92c96ae80ba3323) )

	ROM_LOAD16_BYTE( "kh1-ma0l.bin", 0x280001, 0x100000, CRC(7b2db5df) SHA1(ecc392c4683cf0718d986e73336b69952d324548) ) /* 0x400000 */
	ROM_LOAD16_BYTE( "kh1-ma0u.bin", 0x280000, 0x100000, CRC(6983228b) SHA1(5f3eeb780e9d91445b4c11da63d4ca580e654f34) )
	ROM_LOAD16_BYTE( "kh1-ma1l.bin", 0x480001, 0x100000, CRC(b24f93e6) SHA1(3d8951485dc8a2810da9ddf2f4896efa31779bf4) )
	ROM_LOAD16_BYTE( "kh1-ma1u.bin", 0x480000, 0x100000, CRC(18a60348) SHA1(298e0e0e7649e872791c3c99c81a19f273e9eb8a) )
	ROM_LOAD16_BYTE( "kh1-ma2l.bin", 0x680001, 0x100000, CRC(82064ee9) SHA1(0b984565d17e580f49fff982a1621ef90e14c064) )
	ROM_LOAD16_BYTE( "kh1-ma2u.bin", 0x680000, 0x100000, CRC(17fe8c3d) SHA1(88c45076477725faa5f8a23512e65a40385bb27d) )
	ROM_LOAD16_BYTE( "kh1-ma3l.bin", 0x880001, 0x100000, CRC(ad9a7807) SHA1(c40f18a68306e76acd89ccb3fc82b8106556912e) )
	ROM_LOAD16_BYTE( "kh1-ma3u.bin", 0x880000, 0x100000, CRC(efeb768d) SHA1(15d016244549f3ea0d19f5cfb04bcebd65ac6134) )

	/* M37702 BIOS - labeled as Namco custom C70 */
	ROM_REGION16_LE( 0x4000, REGION_CPU2, 0 )
        ROM_LOAD( "c70.bin",      0x000000, 0x004000, NO_DUMP )
ROM_END

ROM_START( knckhedj )
	ROM_REGION( 0xa80000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "kh1-ep0l.bin", 0x080001, 0x080000, CRC(94660bec) SHA1(42fa23f759cf66b05f30c2fc03a12fd14ae1f796) ) /* 0xc00000 */
	ROM_LOAD16_BYTE( "kh1-ep0u.bin", 0x080000, 0x080000, CRC(ad640d69) SHA1(62595a9d1d5952cbe3dd7266cfda9292be51d269) )
	ROM_LOAD16_BYTE( "kh1-ep1l.bin", 0x180001, 0x080000, CRC(27e6ab4e) SHA1(66f397cc2117c1e73652c4800c0937e6d8116380) )
	ROM_LOAD16_BYTE( "kh1-ep1u.bin", 0x180000, 0x080000, CRC(487b2434) SHA1(2d62db85ceac1fca61c39e4db92c96ae80ba3323) )

	ROM_LOAD16_BYTE( "kh1-ma0l.bin", 0x280001, 0x100000, CRC(7b2db5df) SHA1(ecc392c4683cf0718d986e73336b69952d324548) ) /* 0x400000 */
	ROM_LOAD16_BYTE( "kh1-ma0u.bin", 0x280000, 0x100000, CRC(6983228b) SHA1(5f3eeb780e9d91445b4c11da63d4ca580e654f34) )
	ROM_LOAD16_BYTE( "kh1-ma1l.bin", 0x480001, 0x100000, CRC(b24f93e6) SHA1(3d8951485dc8a2810da9ddf2f4896efa31779bf4) )
	ROM_LOAD16_BYTE( "kh1-ma1u.bin", 0x480000, 0x100000, CRC(18a60348) SHA1(298e0e0e7649e872791c3c99c81a19f273e9eb8a) )
	ROM_LOAD16_BYTE( "kh1-ma2l.bin", 0x680001, 0x100000, CRC(82064ee9) SHA1(0b984565d17e580f49fff982a1621ef90e14c064) )
	ROM_LOAD16_BYTE( "kh1-ma2u.bin", 0x680000, 0x100000, CRC(17fe8c3d) SHA1(88c45076477725faa5f8a23512e65a40385bb27d) )
	ROM_LOAD16_BYTE( "kh1-ma3l.bin", 0x880001, 0x100000, CRC(ad9a7807) SHA1(c40f18a68306e76acd89ccb3fc82b8106556912e) )
	ROM_LOAD16_BYTE( "kh1-ma3u.bin", 0x880000, 0x100000, CRC(efeb768d) SHA1(15d016244549f3ea0d19f5cfb04bcebd65ac6134) )

	/* M37702 BIOS - labeled as Namco custom C70 */
	ROM_REGION16_LE( 0x4000, REGION_CPU2, 0 )
        ROM_LOAD( "c70.bin",      0x000000, 0x004000, NO_DUMP )
ROM_END

ROM_START( numanath )
	ROM_REGION( 0xa80000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "nm2-ep0l.bin", 0x080001, 0x080000, CRC(f24414bb) SHA1(68b13dfdc2292afd5279edb891fe63972f991e7b) ) /* 0xc00000 */
	ROM_LOAD16_BYTE( "nm2-ep0u.bin", 0x080000, 0x080000, CRC(25c41616) SHA1(68ba67d3dd45f3bdddfa2fd21b574535306c1214) )
	ROM_LOAD16_BYTE( "nm1-ep1l.bin", 0x180001, 0x080000, CRC(4581dcb4) SHA1(1f46f98e63a7c9cdfde9e8ee2696a13c3f9bcc8e) )
	ROM_LOAD16_BYTE( "nm1-ep1u.bin", 0x180000, 0x080000, CRC(30cd589a) SHA1(74a14ec41fe4fc9f73e5357b0903f1199ed96337) )

	ROM_LOAD16_BYTE( "nm1-ma0l.bin", 0x280001, 0x100000, CRC(20faaa57) SHA1(9dbfc0dd48eec37b2c0715a5691c6e6f923fc7f7) ) /* 0x400000 */
	ROM_LOAD16_BYTE( "nm1-ma0u.bin", 0x280000, 0x100000, CRC(ed7c37f2) SHA1(829751af33754ade941f76982e196b494d56ab0a) )
	ROM_LOAD16_BYTE( "nm1-ma1l.bin", 0x480001, 0x100000, CRC(2232e3b4) SHA1(e9da3dc34eb2576c8a88e23cb9007129e885496d) )
	ROM_LOAD16_BYTE( "nm1-ma1u.bin", 0x480000, 0x100000, CRC(6cc9675c) SHA1(fec74da4479f2a088760efc6908e6acfaea3989f) )
	ROM_LOAD16_BYTE( "nm1-ma2l.bin", 0x680001, 0x100000, CRC(208abb39) SHA1(52d7247a71c6a14467f12f5270921bba1824cc3f) )
	ROM_LOAD16_BYTE( "nm1-ma2u.bin", 0x680000, 0x100000, CRC(03a3f204) SHA1(9cb0422c8ecc819d0cc8a65c29a228369d78d986) )
	ROM_LOAD16_BYTE( "nm1-ma3l.bin", 0x880001, 0x100000, CRC(42a539e9) SHA1(1c53a5a031648891ab7a37cf026c979404ce9589) )
	ROM_LOAD16_BYTE( "nm1-ma3u.bin", 0x880000, 0x100000, CRC(f79e2112) SHA1(8bb8639a9d3a5d3ac5c9bb78e72b3d76582a9c25) )

	/* M37702 BIOS - labeled as Namco custom C70 */
	ROM_REGION16_LE( 0x4000, REGION_CPU2, 0 )
        ROM_LOAD( "c70.bin",      0x000000, 0x004000, NO_DUMP )
ROM_END

ROM_START( numanatj )
	ROM_REGION( 0xa80000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "nm1-ep0l.bin", 0x080001, 0x080000, CRC(4398b898) SHA1(0d1517409ba181f796f7f413cac704c60085b505) ) /* 0xc00000 */
	ROM_LOAD16_BYTE( "nm1-ep0u.bin", 0x080000, 0x080000, CRC(be90aa79) SHA1(6884a8d72dd34c889527e8e653f5e5b4cf3fb5d6) )
	ROM_LOAD16_BYTE( "nm1-ep1l.bin", 0x180001, 0x080000, CRC(4581dcb4) SHA1(1f46f98e63a7c9cdfde9e8ee2696a13c3f9bcc8e) )
	ROM_LOAD16_BYTE( "nm1-ep1u.bin", 0x180000, 0x080000, CRC(30cd589a) SHA1(74a14ec41fe4fc9f73e5357b0903f1199ed96337) )

	ROM_LOAD16_BYTE( "nm1-ma0l.bin", 0x280001, 0x100000, CRC(20faaa57) SHA1(9dbfc0dd48eec37b2c0715a5691c6e6f923fc7f7) ) /* 0x400000 */
	ROM_LOAD16_BYTE( "nm1-ma0u.bin", 0x280000, 0x100000, CRC(ed7c37f2) SHA1(829751af33754ade941f76982e196b494d56ab0a) )
	ROM_LOAD16_BYTE( "nm1-ma1l.bin", 0x480001, 0x100000, CRC(2232e3b4) SHA1(e9da3dc34eb2576c8a88e23cb9007129e885496d) )
	ROM_LOAD16_BYTE( "nm1-ma1u.bin", 0x480000, 0x100000, CRC(6cc9675c) SHA1(fec74da4479f2a088760efc6908e6acfaea3989f) )
	ROM_LOAD16_BYTE( "nm1-ma2l.bin", 0x680001, 0x100000, CRC(208abb39) SHA1(52d7247a71c6a14467f12f5270921bba1824cc3f) )
	ROM_LOAD16_BYTE( "nm1-ma2u.bin", 0x680000, 0x100000, CRC(03a3f204) SHA1(9cb0422c8ecc819d0cc8a65c29a228369d78d986) )
	ROM_LOAD16_BYTE( "nm1-ma3l.bin", 0x880001, 0x100000, CRC(42a539e9) SHA1(1c53a5a031648891ab7a37cf026c979404ce9589) )
	ROM_LOAD16_BYTE( "nm1-ma3u.bin", 0x880000, 0x100000, CRC(f79e2112) SHA1(8bb8639a9d3a5d3ac5c9bb78e72b3d76582a9c25) )

	/* M37702 BIOS - labeled as Namco custom C70 */
	ROM_REGION16_LE( 0x4000, REGION_CPU2, 0 )
        ROM_LOAD( "c70.bin",      0x000000, 0x004000, NO_DUMP )
ROM_END

ROM_START( quiztou )
	ROM_REGION( 0xa80000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "qt1ep0l.6c", 0x080001, 0x080000, CRC(b680e543) SHA1(f10f38113a46c821d8e9d66f52d7311d9d52e595) ) /* 0xc00000 */
	ROM_LOAD16_BYTE( "qt1ep0u.6f", 0x080000, 0x080000, CRC(143c5e4d) SHA1(24c584986c97a5e6fe7e73f0e9af4af28ed20c4a) )
	ROM_LOAD16_BYTE( "qt1ep1l.7c", 0x180001, 0x080000, CRC(33a72242) SHA1(5d17f033878d28dbebba50931a549ccf84802c05) )
	ROM_LOAD16_BYTE( "qt1ep1u.7f", 0x180000, 0x080000, CRC(69f876cb) SHA1(d0c7e972a04c45d3ab34ef5be88614d6389189c6) )

	ROM_LOAD16_BYTE( "qt1ma0l.2c", 0x280001, 0x100000, CRC(5597f2b9) SHA1(747c4be867d4eb37ffab8303740729686a00b825) ) /* 0x400000 */
	ROM_LOAD16_BYTE( "qt1ma0u.2f", 0x280000, 0x100000, CRC(f0a4cb7d) SHA1(364e85af956e7cfc29c957da11574a4b389f7797) )
	ROM_LOAD16_BYTE( "qt1ma1l.3c", 0x480001, 0x100000, CRC(1b9ce7a6) SHA1(dac1da9dd8076f238211fed5c780b4b8bededf22) )
	ROM_LOAD16_BYTE( "qt1ma1u.3f", 0x480000, 0x100000, CRC(58910872) SHA1(c0acbd64e90672564c3839fd21870672aa32e439) )
	ROM_LOAD16_BYTE( "qt1ma2l.4c", 0x680001, 0x100000, CRC(94739917) SHA1(b5be5c9fd7223d3fb601f769cb80f56a5a586de0) )
	ROM_LOAD16_BYTE( "qt1ma2u.4f", 0x680000, 0x100000, CRC(6ba5b893) SHA1(071caed9cf261f1f8af7079875bd206177baef1a) )
	ROM_LOAD16_BYTE( "qt1ma3l.5c", 0x880001, 0x100000, CRC(aa9dc6ff) SHA1(c738f8c59bb5245874576c5bcf88c7138fa9a147) )
	ROM_LOAD16_BYTE( "qt1ma3u.5f", 0x880000, 0x100000, CRC(14a5a163) SHA1(1107f50e491bedeb4ab7ac3f32cfe47727274ba9) )

	/* M37702 BIOS - labeled as Namco custom C70 */
	ROM_REGION16_LE( 0x4000, REGION_CPU2, 0 )
        ROM_LOAD( "c70.bin",      0x000000, 0x004000, NO_DUMP )
ROM_END

ROM_START( xday2 )
	ROM_REGION( 0xa80000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "xds1-mpr0.4b", 0x080001, 0x080000, CRC(83539aaa) SHA1(42d97bb2daaf5ff48efac70f0ff37869c5ba177d) ) /* 0xc00000 */
	ROM_LOAD16_BYTE( "xds1-mpr1.8b", 0x080000, 0x080000, CRC(468b36de) SHA1(52817be9913a6938ce6add2834ba1a727b1d677e) )

	ROM_LOAD16_BYTE( "xds1-dat0.4b", 0x280001, 0x200000, CRC(42cecc8b) SHA1(7510f16b908dd0f7828887dcfa26c5e4643df66c) ) /* 0x400000 */
	ROM_LOAD16_BYTE( "xds1-dat1.8b", 0x280000, 0x200000, CRC(d250b7e8) SHA1(b99251ae8e25fae062d33e74ff800ab43fb308a2) )
	ROM_LOAD16_BYTE( "xds1-dat2.4c", 0x680001, 0x200000, CRC(99d72a08) SHA1(4615b43b9a81240ffee8b0f021037f554f4f1f24) )
	ROM_LOAD16_BYTE( "xds1-dat3.8c", 0x680000, 0x200000, CRC(8980acc4) SHA1(ecd94a3d3a38923e8e322cd8863671af26e30812) )

	/* M37702 BIOS - labeled as Namco custom C70 */
	ROM_REGION16_LE( 0x4000, REGION_CPU2, 0 )
        ROM_LOAD( "c70.bin",      0x000000, 0x004000, NO_DUMP )
ROM_END

// NA-1 (C69 MCU)
GAME( 1992,bkrtmaq,  0,        namcona1w, namcona1_quiz,bkrtmaq,  ROT0, "Namco", "Bakuretsu Quiz Ma-Q Dai Bouken (Japan)", 0 )
GAME( 1992,cgangpzl, 0,        namcona1w, namcona1_joy,	cgangpzl, ROT0, "Namco", "Cosmo Gang the Puzzle (US)", 0 )
GAME( 1992,cgangpzj, cgangpzl, namcona1w, namcona1_joy,	cgangpzl, ROT0, "Namco", "Cosmo Gang the Puzzle (Japan)", 0 )
GAME( 1992,exvania,  0,        namcona1,  namcona1_joy,	exbania,  ROT0, "Namco", "Exvania (Japan)", 0 )
GAME( 1992,fghtatck, 0,        namcona1,  namcona1_joy,	fa,       ROT90,"Namco", "Fighter & Attacker (US)", 0 )
GAME( 1992,fa,       fghtatck, namcona1,  namcona1_joy,	fa,       ROT90,"Namco", "F/A (Japan)", 0 )
GAME( 1992,swcourt,  0,        namcona1w, namcona1_joy,	swcourt,  ROT0, "Namco", "Super World Court (World)", 0 )
GAME( 1992,swcourtj, swcourt,  namcona1w, namcona1_joy,	swcourt,  ROT0, "Namco", "Super World Court (Japan)", 0 )
GAME( 1993,emeraldj, emeralda, namcona1w, namcona1_joy,	emeraldj, ROT0, "Namco", "Emeraldia (Japan Version B)", 0 ) /* Parent is below on NA-2 Hardware */
GAME( 1993,emerldja, emeralda, namcona1w, namcona1_joy,	emeraldj, ROT0, "Namco", "Emeraldia (Japan)", 0 ) /* Parent is below on NA-2 Hardware */
GAME( 1993,tinklpit, 0,        namcona1w, namcona1_joy,	tinklpit, ROT0, "Namco", "Tinkle Pit (Japan)", 0 )

// NA-2 (C70 MCU)
GAME( 1992,knckhead, 0,        namcona2,  namcona2_joy,	knckhead, ROT0, "Namco", "Knuckle Heads (World)", GAME_IMPERFECT_SOUND )
GAME( 1992,knckhedj, knckhead, namcona2,  namcona2_joy,	knckhead, ROT0, "Namco", "Knuckle Heads (Japan)", GAME_IMPERFECT_SOUND )
GAME( 1993,emeralda, 0,        namcona2,  namcona2_joy,	emeralda, ROT0, "Namco", "Emeraldia (World)", GAME_IMPERFECT_SOUND )
GAME( 1993,numanath, 0,        namcona2,  namcona2_joy,	numanath, ROT0, "Namco", "Numan Athletics (World)", GAME_IMPERFECT_SOUND )
GAME( 1993,numanatj, numanath, namcona2,  namcona2_joy,	numanath, ROT0, "Namco", "Numan Athletics (Japan)", GAME_IMPERFECT_SOUND )
GAME( 1993,quiztou,  0,        namcona2,  namcona2_quiz,quiztou,  ROT0, "Namco", "Nettou! Gekitou! Quiztou!! (Japan)", GAME_IMPERFECT_SOUND )
GAME( 1995,xday2,    0,        namcona2,  namcona2_joy, xday2,    ROT0, "Namco", "X-Day 2 (Japan)", GAME_IMPERFECT_SOUND|GAME_NOT_WORKING )
