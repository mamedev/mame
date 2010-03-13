/***************************************************************************

  Namco System 11 - Arcade PSX Hardware
  =====================================
  Driver by smf & Ryan Holtz
  Board notes by The Guru
  Sound and MCU hookup by R. Belmont
  Thanks to the original Zinc Team.

  Issues:
   Random draw list corruption in soul edge v2 & dunkmania.
   soul edge, dunk mania & prime goal ex try to access joypads/memory cards. It is unknown what they would do if they found one.
   Pocket Racer can't enter service menus with proper C76 hookup, runs 60 FPS when it looks like it should be 30

Known Dumps
-----------

Game       Description                             CPU board           Mother board             Daughter board          Keycus   ROM0L
--------------------------------------------------------------------------------------------------------------------------------------
tekken     Tekken (TE4/VER.C)                      COH-100             SYSTEM11 MOTHER PCB      SYSTEM11 ROM8 PCB       none     5
tekkena    Tekken (TE2/VER.C)                      COH-100             "                        "                       "        "
tekkenb    Tekken (TE2/VER.B)                      COH-100             "                        "                       "        "
tekkenc    Tekken (TE1/VER.B)                      COH-100             "                        "                       "        "
tekken2    Tekken 2 Ver.B (TES3/VER.B)             COH-100             SYSTEM11 MOTHER PCB      SYSTEM11 ROM8 PCB       C406     6
tekken2a   Tekken 2 Ver.B (TES2/VER.B)             COH-100             "                        "                       "        "
tekken2b   Tekken 2 (TES2/VER.A)                   COH-100             "                        "                       "        "
souledge   Soul Edge Ver. II (SO4/VER.C)           COH-100 / COH-110   SYSTEM11 MOTHER PCB      SYSTEM11 ROM8 PCB       C409     6
souledga   Soul Edge Ver. II (SO3/VER.C)           COH-100 / COH-110   "                        "                       "        "
souledgb   Soul Edge (SO3/VER.A)                   COH-100 / COH-110   "                        "                       "        "
souledgc   Soul Edge (SO1/VER.A)                   COH-100 / COH-110   "                        "                       "        "
dunkmnia   Dunk Mania (DM2/VER.C)                  COH-100 / COH-110   SYSTEM11 MOTHER PCB      SYSTEM11 ROM8 PCB       C410     5
dunkmnic   Dunk Mania (DM1/VER.C)                  COH-100 / COH-110   SYSTEM11 MOTHER PCB      SYSTEM11 ROM8 PCB       C410     5
xevi3dg    Xevious 3D/G (XV31/VER.A)               COH-100 / COH-110   SYSTEM11 MOTHER PCB      SYSTEM11 ROM8 PCB       C430     5
primglex   Prime Goal EX (PG1/VER.A)               COH-100 / COH-110   SYSTEM11 MOTHER PCB      SYSTEM11 ROM8 PCB       C411     6
danceyes   Dancing Eyes (DC1/VER.A)                COH-100 / COH-110   SYSTEM11 MOTHER PCB      SYSTEM11 ROM8 PCB       C431     5
pocketrc   Pocket Racer (PKR1/VER.B)               COH-110             SYSTEM11 MOTHER PCB      SYSTEM11 ROM8 PCB       C432     5
starswep   Star Sweep (STP1/VER.A)                 COH-100 / COH-110   SYSTEM11 MOTHER(B) PCB                           C442     -
myangel3   Kosodate Quiz My Angel 3 (KQT1/VER.A)   COH-110             SYSTEM11 MOTHER(B) PCB   SYSTEM11 ROM8(64) PCB   C443     2
ptblnk2a   Point Blank 2 (GNB3/VER.A)              COH-100 / COH-110   SYSTEM11 MOTHER PCB      SYSTEM11 ROM8(64) PCB   C443     2


Not Dumped Yet
--------------
Point Blank 3           (C) Namco, 2001
Soul Edge Ver. III      (C) Namco, 1996
Tekken Ver.C            (C) Namco, 1995
Tekken II Ver.C         (C) Namco, 1995

If you can help with the remaining undumped S11 games, please contact http://www.mameworld.net/gurudumps/comments.html


The Namco System 11 system comprises 3 PCB's....
MOTHER PCB- This is the main PCB. It holds all sound circuitry, sound ROMs, program ROMs, shared RAM, bank-switching logic,
            controller/input logic (including sound CPU).
            There are two known revisions of this PCB. The differences are not known, though they're probably almost identical except
            for some minor component shuffling. The 2nd revision is used only by Kosodate Quiz My Angel 3 and Star Sweep.
CPU PCB   - There are two known revisions of this PCB. Any game can use either PCB. Contains main CPU/RAM and GPU/Video RAM
            The differences are only in the RAM type, one uses 4x 16MBit chips compared to the other that uses 2x 32MBit chips.
ROM PCB   - There are two known revisions of this PCB. They're mostly identical except one uses all 32MBit SOP44 MASKROMs and the other
            uses 64MBit SOP44 MASKROMs. The 64MBit ROM board also has space for a PAL and a KEYCUS.

Each game has a multi-letter code assigned to it which is printed on a small sticker and placed on the bottom side of the MOTHER PCB.
This code is then proceeded by a number (1, 2, 3 & 4 seen so far), then 'Ver.' then A/B/C/D/E which denotes the software
revision, and in some cases a sub-revision such as 1 or 2 (usually only listed in the test mode).
The first 1 denotes a Japanese version, other numbers denote a World version.
For World versions, only the main program changes (and sometimes only some of the program ROMs change), the rest of the (graphics) ROMs
use the Japanese version ROMs.
See the Main PCB and ROM Daughterboard PCB texts below for more details.

1st Revision
SYSTEM11 MOTHER PCB 8645960103 (8645970103)
  |----------------------------------------|
  | LA4705        |----|  (CONN1)          |
|-|               |C384| KEYCUS   S11MOT3A |
|  VOL      4558  |----| AT28C16           |
|             LC78815   S11MOT2A           |
|                                          |
|J                                  PRG.2L |
|A     WAVE.8K    |-----| 2061ASC-1 PRG.2K |
|M                |C352 |           PRG.2J |
|M                |     |           PRG.2F |
|A                |-----|                  |
|                                          |
|                         |-------|        |
|-|   |-----|             |       |        |
  |   | C76 |   SPROG.6D  | C195  | 61C256 |
|-|   |     |             |       | 61C256 |
|4    |-----|   S11MOT1A  |-------|        |
|8                                         |
|W    DSW(2)                               |
|A                                         |
|Y            (CONN2)                      |
|-|   PQ30RV21                             |
  |----------------------------------------|
Notes:
      C76      - Sound CPU with 16k internal BIOS, a Namco-badged custom, actually a Mitsubishi M37702 MCU (QFP80)
      C195     - Namco custom (QFP160)
      C384     - Namco custom (QFP48)
      C352     - Namco custom C352 PCM sound chip (QFP100)
      AT28C16  - 2k x8 EEPROM (DIP28)
      61C256   - 32k x8 SRAM (x2, SOJ28)
      2061ASC-1- IC Designs 2061ASC-1 programmable clock generator (SOIC16)
      S11MOT*  - Standard System 11 PALs (DIP20)
      WAVE.8K  - Sound samples, 42 pin DIP MASKROM, either 16MBit or 32MBit. If 32MBit, it is programmed in Byte Mode.
      SPROG.6D - Sound program, Intel PA28F200BX 2MBit Flash ROM (SOP44)
      PRG.2*   - Main program, Intel E28F008SA 8MBit Flash ROM (TSOP40)
      CONN1    - for connection of the ROM Board
      CONN2    - for connection of the CPU board
      KEYCUS   - DIP28 protection chip (see table below, DIP32)
      PQ3RV21  - Sharp PQ30RV21 5V to 3.3V Voltage Regulator
      LC78815  - Sanyo LM78815 2-Channel 16-Bit D/A Converter (SOIC20)
      4558     - Op Amp (SOIC8)
      LA4705   - Sanyo LA4705 15W 2-Channel Power Amplifier (SIP18)
      48WAY    - Namco 48 way edge connector used for extra controls and to output the 2nd speaker when set to stereo mode.

      There is a REV B Mother board 'SYSTEM11 MOTHER(B) PCB' with slightly different
      component placings. It's not known if it has different components also.

                                Game Code
      Game                      Sticker      KEYCUS
      ---------------------------------------------
      Dancing Eyes              DC           C431
      Don Bear Near
      Dunk Mania                DM           C410
      Kosodate Quiz My Angel 3  KQT          C443
      Prime Goal EX             PG           C411
      Pocket Racer              PKR          C432
      Point Blank 2             GNB          C443
      Point Blank 3
      Soul Edge                 SO           C409
      Soul Edge Ver. II         SO           C409
      Soul Edge Ver. III
      Star Sweep                STP          C442
      Tekken                    TE           none
      Tekken Ver.B              TE           none
      Tekken Ver.C
      Tekken II                 TES          C406
      Tekken II Ver.B           TES          C406
      Tekken II Ver.C
      Xevious 3D/G              XV3          C430



ROM Boards
----------

SYSTEM11 ROM8 PCB 8645960202 (8645970202)
|----------------------------------------|
|               (CONN1)                  |
|ROM3L.9               74F139    ROM3U.1 |
|                                        |
|ROM1L.8     ROM0L.6       ROM2U.4       |
|      ROM2L.7      ROM0U.5       ROM1U.3|
|                                        |
|----------------------------------------|
Notes:
      This ROM board is wired to accept a maximum of 8x 8Bit 32MBit SOP44 MASK ROMs.


SYSTEM11 ROM8(64) PCB 8645960500 (8645970500)
|----------------------------------------|
|KEYCUS.7       (CONN1)       PAL16V8H.1 |
|                                        |
|            *PRG3U.IC10                 |
|PRG2U.8     PRG0U.5       PRG1L.3       |
|      PRG1U.6      PRG2L.4       PRG0L.2|
|                 *PRG3L.IC9             |
|----------------------------------------|
Notes:
      This ROM board is wired to accept a maximum of 8x 8Bit 64MBit SOP44 MASK ROMs.
      There is room for a PLCC44 KEYCUS IC (usually a CPLD, but not populated) and a PLCC20
      IC type PAL16V8H (populated and labelled 'ROM8 DEC0')
      * - These ROMs are on the other side of the PCB.

CPU Board
---------

Revision 1
GP-11  COH-100  1-655-543-12
|-------------------------------------|
|        |---------|     KM48V514BJ-6 |
|        |SONY     |     KM48V514BJ-6 |
|        |CXD8530AQ|     KM48V514BJ-6 |
|        |         |     KM48V514BJ-6 |
|        |---------|     KM48V514BJ-6 |
|             67.737MHz  KM48V514BJ-6 |
|        |---------|     KM48V514BJ-6 |
|(CONN2) |SONY     |     KM48V514BJ-6 |
|        |CXD8538Q |                  |
|        |         |                  |
|        |---------| 53.69MHz         |
|                       D482445LGW-A70|
|  SONY     D482445LGW-A70            |
|  CXD2923AR            D482445LGW-A70|
|           D482445LGW-A70            |
|-------------------------------------|
Notes:
      CXD8530AQ - Sony CXD8530AQ Main CPU (QFP208)
                  CXD8530BQ may be used instead of CXD8530AQ
      CXD8538Q  - Sony CXD8538Q GPU (QFP208)
      CXD2923AR - Sony CXD2923AR SPU (QFP80)

Revision 2
GP-13  COH-110  S-XMB 1-660-276-11
|-------------------------------------|
|        |---------|     KM48V514BJ-6 |
|        |SONY     |     KM48V514BJ-6 |
|        |CXD8530CQ|     KM48V514BJ-6 |
|        |         |     KM48V514BJ-6 |
|        |---------|     KM48V514BJ-6 |
|             67.737MHz  KM48V514BJ-6 |
|        |---------|     KM48V514BJ-6 |
|(CONN2) |SONY     |     KM48V514BJ-6 |
|        |CXD8561Q |                  |
|        |         |                  |
|        |---------| 53.69MHz         |
|                                     |
|    KM4132G271Q-12                   |
|    KM4132G271Q-12        XC44200FUB |
|                                     |
|-------------------------------------|
Notes:
      The 2nd revision CPU board (GP-13 COH-110) uses 2x 32MBit RAMs instead of
      the 4x D482445LGW-A70 RAMs and the 2 main SONY IC's are updated revisions,
      though the functionality of them is identical. The 2 types of CPU boards can be
      used with any System 11 motherboard, and any System 11 game.

Gun Board (Used only with Point Blank 2 so far)
---------

System11 GUN I/F PCB 8645960701 (8645970701)
|-------------------------------------------|
|    |-------|    |-------|    |-------|    |
|    |S11GUN3|    |S11GUN2|    |S11GUN1|    |
|    |       |    |       |    |       |    |
|    |       |    |       |    |       |    |
|    |-------|    |-------|    |-------|    |
|                                           |
|                                           |
|                                           |
|      SLA4060                              |
|J3                     |---------|         |
|                       | S11GUN0 |         |
|                       |         |         |
|                       |         |         |
|                       |         |  AV9170 |
|                       |---------|         |
|                                           |
|                   J2                      |
|                                           |
|                   J1                      |
|-------------------------------------------|
Notes:
      SLA4060 - Sanken Electric Co. NPN Darlington Transistor Array (SIP12)
      J1      - 96 pin connector joining to the mother board (connector below the PCB)
      J2      - 96 pin connector joining to the CPU board (connector above the PCB)
      J3      - 10 pin connector joining to the gun via a 24V solenoid driver board (for the gun recoil)
      S11GUN0 - PLCC84 FPGA (not populated)
      S11GUN1 - Altera Max EPM7128STC100-10 EPLD (QFP100, not populated)
      S11GUN2 - Altera Max EPM7128STC100-10 EPLD (QFP100, labelled 'S11GUN2)
      S11GUN3 - Altera Max EPM7128STC100-10 EPLD (QFP100, not populated)
      AV9170  - Integrated Circuit Systems Inc. AV9170 Clock Synchronizer and Multiplier, Voltage Controlled Oscillator (SOIC8)

***************************************************************************/

#include "emu.h"
#include "deprecat.h"
#include "cpu/mips/psx.h"
#include "cpu/m37710/m37710.h"
#include "includes/psx.h"
#include "machine/at28c16.h"
#include "sound/c352.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine *machine, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", cpuexec_describe_context(machine), buf );
	}
}

static UINT32 *namcos11_sharedram;
static UINT32 *namcos11_keycus;
static size_t namcos11_keycus_size;

static WRITE32_HANDLER( keycus_w )
{
	verboselog( space->machine, 1, "keycus_w( %08x, %08x, %08x )\n", offset, data, mem_mask );
	COMBINE_DATA( &namcos11_keycus[ offset ] );
}

/* tekken 2 */
static READ32_HANDLER( keycus_c406_r )
{
	/* todo: verify behaviour */
	UINT32 data;

	data = namcos11_keycus[ offset ];
	switch( offset )
	{
	case 0:
		data = ( data & 0xffff0000 ) |
			( ( namcos11_keycus[ 0 ] >> 8 ) & 0xf000 ) |
			( ( namcos11_keycus[ 0 ] >> 16 ) & 0xf00 ) |
			( ( namcos11_keycus[ 1 ] >> 8 ) & 0xf0 ) |
			( ( namcos11_keycus[ 1 ] >> 8 ) & 0xf );
		break;
	}
	verboselog( space->machine, 1, "keycus_c406_r( %08x, %08x, %08x )\n", offset, data, mem_mask );
	return data;
}

/* soul edge */
static READ32_HANDLER( keycus_c409_r )
{
	/* todo: verify behaviour */
	UINT32 data;

	data = namcos11_keycus[ offset ];
	switch( offset )
	{
	case 3:
		data = ( data & 0x0000ffff ) | 0x000f0000;
		break;
	}
	verboselog( space->machine, 1, "keycus_c409_r( %08x, %08x, %08x )\n", offset, data, mem_mask );
	return data;
}

/* dunk mania */
static READ32_HANDLER( keycus_c410_r )
{
	UINT32 data;
	UINT32 n_value;

	if( ( namcos11_keycus[ 0 ] & 0x0000ffff ) != 0xfffe )
	{
		n_value = namcos11_keycus[ 0 ] & 0x0000ffff;
	}
	else
	{
		n_value = 410;
	}

	data = namcos11_keycus[ offset ];
	switch( offset )
	{
	case 0:
		data = ( data & 0x0000ffff ) |
			( ( n_value / 1 ) % 10 );
		break;
	case 1:
		data =
			( ( ( n_value / 10 ) % 10 ) << 24 ) |
			( ( ( n_value / 100 ) % 10 ) << 0 ) |
			( ( ( n_value / 1000 ) % 10 ) << 8 ) |
			( ( ( n_value / 10000 ) % 10 ) << 16 );
		break;
	}
	verboselog( space->machine, 1, "keycus_c410_r( %08x, %08x, %08x )\n", offset, data, mem_mask );
	return data;
}

/* prime goal ex */
static READ32_HANDLER( keycus_c411_r )
{
	UINT32 data;
	UINT32 n_value;

	data = namcos11_keycus[ offset ];
	if( ( namcos11_keycus[ 1 ] & 0x0000ffff ) == 0x00007256 )
	{
		n_value = namcos11_keycus[ 5 ] & 0x0000ffff;
	}
	else
	{
		n_value = 411;
	}

	switch( offset )
	{
	case 0:
		data = ( data & 0xffff0000 ) | ( ( ( n_value / 10 ) % 10 ) << 8 ) | ( ( n_value / 1 ) % 10 );
		break;
	case 1:
		data = ( data & 0xffff0000 ) | ( ( ( n_value / 1000 ) % 10 ) << 8 ) | ( ( n_value / 100 ) % 10 );
		break;
	case 4:
		data = ( data & 0xffff0000 ) | ( ( n_value / 10000 ) % 10 );
		break;
	}

	verboselog( space->machine, 1, "keycus_c411_r( %08x, %08x, %08x )\n", offset, data, mem_mask );
	return data;
}

/* xevious 3d/g */
static READ32_HANDLER( keycus_c430_r )
{
	UINT32 data;
	UINT16 n_value;

	if( ( namcos11_keycus[ 2 ] & 0x0000ffff ) == 0x0000e296 )
	{
		n_value = namcos11_keycus[ 0 ] & 0x0000ffff;
	}
	else
	{
		n_value = 430;
	}

	data = namcos11_keycus[ offset ];
	switch( offset )
	{
	case 0:
		data = ( data & 0x0000ffff ) |
			( ( ( n_value / 10000 ) % 10 ) << 16 );
		break;
	case 2:
		data =
			( ( ( n_value / 1000 ) % 10 ) << 8 ) |
			( ( ( n_value / 100 ) % 10 ) << 0 ) |
			( ( ( n_value / 10 ) % 10 ) << 24 ) |
			( ( ( n_value / 1 ) % 10 ) << 16 );
		break;
	}
	verboselog( space->machine, 1, "keycus_c430_r( %08x, %08x, %08x )\n", offset, data, mem_mask );
	return data;
}

/* dancing eyes */
static READ32_HANDLER( keycus_c431_r )
{
	UINT32 data;
	UINT16 n_value;

	if( ( namcos11_keycus[ 0 ] & 0x0000ffff ) == 0x00009e61 )
	{
		n_value = namcos11_keycus[ 6 ] & 0x0000ffff;
	}
	else
	{
		n_value = 431;
	}

	data = namcos11_keycus[ offset ];
	switch( offset )
	{
	case 0:
		data = ( data & 0xffff0000 ) | ( ( ( n_value / 10 ) % 10 ) << 8 ) | ( ( n_value / 1 ) % 10 );
		break;
	case 2:
		data = ( data & 0xffff0000 ) | ( ( ( n_value / 1000 ) % 10 ) << 8 ) | ( ( n_value / 100 ) % 10 );
		break;
	case 4:
		data = ( data & 0xffff0000 ) | ( ( n_value / 10000 ) % 10 );
		break;
	}
	verboselog( space->machine, 1, "keycus_c431_r( %08x, %08x, %08x )\n", offset, data, mem_mask );
	return data;
}

/* pocket racer */
static READ32_HANDLER( keycus_c432_r )
{
	UINT32 data;
	UINT16 n_value;

	if( ( namcos11_keycus[ 3 ] & 0x0000ffff ) == 0x00002f15 )
	{
		n_value = namcos11_keycus[ 1 ] & 0x0000ffff;
	}
	else
	{
		n_value = 432;
	}

	data = namcos11_keycus[ offset ];
	switch( offset )
	{
	case 1:
		data = ( data & 0xffff0000 ) | ( ( ( n_value / 10 ) % 10 ) << 8 ) | ( ( n_value / 1 ) % 10 );
		break;
	case 2:
		data = ( data & 0xffff0000 ) | ( ( ( n_value / 1000 ) % 10 ) << 8 ) | ( ( n_value / 100 ) % 10 );
		break;
	case 3:
		data = ( data & 0xffff0000 ) | ( ( ( n_value / 100000 ) % 10 ) << 8 ) | ( ( n_value / 10000 ) % 10 );
		break;
	}

	verboselog( space->machine, 1, "keycus_c432_r( %08x, %08x, %08x )\n", offset, data, mem_mask );
	return data;
}

/* star sweep */
static READ32_HANDLER( keycus_c442_r )
{
	/* todo: verify behaviour */
	UINT32 data;

	data = namcos11_keycus[ offset ];

	switch( offset )
	{
	case 0:
		if( ( data & 0xffff0000 ) == 0x00210000 )
		{
			data = ( data & 0x0000ffff ) | 0xc4420000;
		}
		break;
	}
	verboselog( space->machine, 1, "keycus_c442_r( %08x, %08x, %08x )\n", offset, data, mem_mask );
	return data;
}

/* kosodate quiz my angel 3 / point blank 2 */
static READ32_HANDLER( keycus_c443_r )
{
	/* todo: verify behaviour */
	UINT32 data;

	data = namcos11_keycus[ offset ];

	switch( offset )
	{
	case 0:
		if( ( data & 0x0000ffff ) == 0x00000020 )
		{
			data = ( data & 0xffff0000 ) | 0x00005678;
		}
		if( ( data & 0xffff0000 ) == 0xa9870000 )
		{
			data = ( data & 0x0000ffff ) | 0x56580000;
		}
		if( ( data & 0xffff0000 ) == 0xa9880000 ) /* ptblnk2a */
		{
			data = ( data & 0x0000ffff ) | 0xc4430000;
		}
		if( ( data & 0xffff0000 ) == 0xffff0000 ) /* myangel3 */
		{
			data = ( data & 0x0000ffff ) | 0xc4430000;
		}
		break;
	}
	verboselog( space->machine, 1, "keycus_c443_r( %08x, %08x, %08x )\n", offset, data, mem_mask );
	return data;
}

static INTERRUPT_GEN( namcos11_vblank )
{
	if( strcmp( device->machine->gamedrv->name, "pocketrc" ) == 0 )
	{
		if( g_p_n_psxram[ 0x12c74 / 4 ] == 0x1440fff9 )
		{
			g_p_n_psxram[ 0x12c74 / 4 ] = 0;
		}
		if( g_p_n_psxram[ 0x64694 / 4 ] == 0x1443000c )
		{
			g_p_n_psxram[ 0x64694 / 4 ] = 0;
		}
	}

	psx_vblank(device);
}

static UINT32 m_n_bankoffset;

INLINE void bankswitch_rom8( const address_space *space, const char *bank, int n_data )
{
	memory_set_bank( space->machine, bank, ( ( n_data & 0xc0 ) >> 4 ) + ( n_data & 0x03 ) );
}

static const char * const bankname[] = { "bank1", "bank2", "bank3", "bank4", "bank5", "bank6", "bank7", "bank8" };

static WRITE32_HANDLER( bankswitch_rom32_w )
{
	verboselog( space->machine, 2, "bankswitch_rom32_w( %08x, %08x, %08x )\n", offset, data, mem_mask );

	if( ACCESSING_BITS_0_15 )
	{
		bankswitch_rom8( space, bankname[offset * 2], data & 0xffff );
	}
	if( ACCESSING_BITS_16_31 )
	{
		bankswitch_rom8( space, bankname[offset * 2 + 1], data >> 16 );
	}
}

static WRITE32_HANDLER( bankswitch_rom64_upper_w )
{
	verboselog( space->machine, 2, "bankswitch_rom64_upper_w( %08x, %08x, %08x )\n", offset, data, mem_mask );

	if( ACCESSING_BITS_0_15 )
	{
		m_n_bankoffset = 0;
	}
	if( ACCESSING_BITS_16_31 )
	{
		m_n_bankoffset = 16;
	}
}

INLINE void bankswitch_rom64( const address_space *space, const char *bank, int n_data )
{
	/* todo: verify behaviour */
	memory_set_bank( space->machine,  bank, ( ( ( ( n_data & 0xc0 ) >> 3 ) + ( n_data & 0x07 ) ) ^ m_n_bankoffset ) );
}

static WRITE32_HANDLER( bankswitch_rom64_w )
{
	verboselog( space->machine, 2, "bankswitch_rom64_w( %08x, %08x, %08x )\n", offset, data, mem_mask );

	if( ACCESSING_BITS_0_15 )
	{
		bankswitch_rom64( space, bankname[offset * 2], data & 0xffff );
	}
	if( ACCESSING_BITS_16_31 )
	{
		bankswitch_rom64( space, bankname[offset * 2 + 1], data >> 16 );
	}
}

static WRITE32_HANDLER( lightgun_w )
{
	if( ACCESSING_BITS_0_15 )
	{
		output_set_value( "led0", !( data & 0x08 ) );
		output_set_value( "led1", !( data & 0x04 ) );
		output_set_value( "recoil0", !( data & 0x02 ) );
		output_set_value( "recoil1", !( data & 0x01 ) );

		verboselog( space->machine, 1, "lightgun_w: outputs (%08x %08x)\n", data, mem_mask );
	}
	if( ACCESSING_BITS_16_31 )
	{
		verboselog( space->machine, 2, "lightgun_w: start reading (%08x %08x)\n", data, mem_mask );
	}
}

static READ32_HANDLER( lightgun_r )
{
	UINT32 data = 0;
	switch( offset )
	{
	case 0:
		data = input_port_read(space->machine,  "GUN1X" );
		break;
	case 1:
		data = ( input_port_read(space->machine,  "GUN1Y" ) ) | ( ( input_port_read(space->machine,  "GUN1Y" ) + 1 ) << 16 );
		break;
	case 2:
		data = input_port_read(space->machine,  "GUN2X" );
		break;
	case 3:
		data = ( input_port_read(space->machine,  "GUN2Y" ) ) | ( ( input_port_read(space->machine,  "GUN2Y" ) + 1 ) << 16 );
		break;
	}
	verboselog( space->machine, 2, "lightgun_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
	return data;
}

static ADDRESS_MAP_START( namcos11_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM	AM_SHARE("share1") AM_BASE(&g_p_n_psxram) AM_SIZE(&g_n_psxramsize) /* ram */
	AM_RANGE(0x1f800000, 0x1f8003ff) AM_RAM /* scratchpad */
	AM_RANGE(0x1f801000, 0x1f801007) AM_WRITENOP
	AM_RANGE(0x1f801008, 0x1f80100b) AM_RAM /* ?? */
	AM_RANGE(0x1f80100c, 0x1f80102f) AM_WRITENOP
	AM_RANGE(0x1f801010, 0x1f801013) AM_READNOP
	AM_RANGE(0x1f801014, 0x1f801017) AM_READNOP
	AM_RANGE(0x1f801040, 0x1f80105f) AM_READWRITE(psx_sio_r, psx_sio_w)
	AM_RANGE(0x1f801060, 0x1f80106f) AM_WRITENOP
	AM_RANGE(0x1f801070, 0x1f801077) AM_READWRITE(psx_irq_r, psx_irq_w)
	AM_RANGE(0x1f801080, 0x1f8010ff) AM_READWRITE(psx_dma_r, psx_dma_w)
	AM_RANGE(0x1f801100, 0x1f80112f) AM_READWRITE(psx_counter_r, psx_counter_w)
	AM_RANGE(0x1f801810, 0x1f801817) AM_READWRITE(psx_gpu_r, psx_gpu_w)
	AM_RANGE(0x1f801820, 0x1f801827) AM_READWRITE(psx_mdec_r, psx_mdec_w)
	AM_RANGE(0x1f801c00, 0x1f801dff) AM_NOP
	AM_RANGE(0x1f802020, 0x1f802033) AM_RAM /* ?? */
	AM_RANGE(0x1f802040, 0x1f802043) AM_WRITENOP
	AM_RANGE(0x1fa04000, 0x1fa0ffff) AM_RAM AM_BASE(&namcos11_sharedram) /* shared ram with C76 */
	AM_RANGE(0x1fa20000, 0x1fa2ffff) AM_WRITE(keycus_w) AM_BASE(&namcos11_keycus) AM_SIZE(&namcos11_keycus_size) /* keycus */
	AM_RANGE(0x1fa30000, 0x1fa30fff) AM_DEVREADWRITE8("at28c16", at28c16_r, at28c16_w, 0x00ff00ff) /* eeprom */
	AM_RANGE(0x1fb00000, 0x1fb00003) AM_WRITENOP /* ?? */
	AM_RANGE(0x1fbf6000, 0x1fbf6003) AM_WRITENOP /* ?? */
	AM_RANGE(0x1fc00000, 0x1fffffff) AM_ROM AM_SHARE("share2") AM_REGION("user1", 0) /* bios */
	AM_RANGE(0x80000000, 0x803fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0x9fc00000, 0x9fffffff) AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xa0000000, 0xa03fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0xbfc00000, 0xbfffffff) AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xfffe0130, 0xfffe0133) AM_WRITENOP
ADDRESS_MAP_END

static READ16_HANDLER( c76_shared_r )
{
	UINT16 *share16 = (UINT16 *)namcos11_sharedram;

	return share16[offset];
}

static WRITE16_HANDLER( c76_shared_w )
{
	UINT16 *share16 = (UINT16 *)namcos11_sharedram;

	COMBINE_DATA(&share16[offset]);
}

static UINT8 su_83;

static READ16_HANDLER( c76_speedup_r )
{
	if ((cpu_get_pc(space->cpu) == 0xc153) && (!(su_83 & 0xff00)))
	{
		cpu_spinuntil_int(space->cpu);
	}

	return su_83;
}

static WRITE16_HANDLER( c76_speedup_w )
{
	COMBINE_DATA(&su_83);
}

static READ16_HANDLER( c76_inputs_r )
{
//  logerror("'c76' Read port %d @ %06X\n", offset, cpu_get_pc(space->cpu));

	switch (offset)
	{
		case 0:
			return input_port_read(space->machine, "PLAYER4");

		case 1:
			return input_port_read(space->machine, "SWITCH");

		case 2:
			return input_port_read(space->machine, "PLAYER1");

		case 3:
			return input_port_read(space->machine, "PLAYER2");
	}

	return 0xff;
}

ADDRESS_MAP_START( c76_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x002000, 0x002fff) AM_DEVREADWRITE("c352", c352_r, c352_w )
	AM_RANGE(0x001000, 0x001007) AM_READ( c76_inputs_r )
	AM_RANGE(0x004000, 0x00bfff) AM_READWRITE( c76_shared_r, c76_shared_w )
	AM_RANGE(0x00c000, 0x00ffff) AM_ROM AM_REGION("c76", 0x40000)
	AM_RANGE(0x080000, 0x0fffff) AM_ROM AM_REGION("c76", 0)
	AM_RANGE(0x200000, 0x27ffff) AM_ROM AM_REGION("c76", 0)
	AM_RANGE(0x280000, 0x2fffff) AM_ROM AM_REGION("c76", 0)
	AM_RANGE(0x300000, 0x300001) AM_WRITENOP
	AM_RANGE(0x301000, 0x301001) AM_WRITENOP
ADDRESS_MAP_END

static READ8_HANDLER(dac7_r)		// bit 7
{
	return input_port_read_safe(space->machine, "PLAYER3", 0xff)&0x80;
}

static READ8_HANDLER(dac6_r)		// bit 3
{
	return (input_port_read_safe(space->machine, "PLAYER3", 0xff)<<4)&0x80;
}

static READ8_HANDLER(dac5_r)		// bit 2
{
	return (input_port_read_safe(space->machine, "PLAYER3", 0xff)<<5)&0x80;
}

static READ8_HANDLER(dac4_r)		// bit 1
{
	return (input_port_read_safe(space->machine, "PLAYER3", 0xff)<<6)&0x80;
}

static READ8_HANDLER(dac3_r)		// bit 0
{
	return (input_port_read_safe(space->machine, "PLAYER3", 0xff)<<7)&0x80;
}

static READ8_HANDLER(dac2_r)		// bit 4
{
	return (input_port_read_safe(space->machine, "PLAYER3", 0xff)<<3)&0x80;
}

static READ8_HANDLER(dac1_r)		// bit 5
{
	return (input_port_read_safe(space->machine, "PLAYER3", 0xff)<<2)&0x80;
}

static READ8_HANDLER(dac0_r)		// bit 6
{
	return (input_port_read_safe(space->machine, "PLAYER3", 0xff)<<1)&0x80;
}

static READ8_HANDLER(pocketrc_gas_r)
{
	return input_port_read(space->machine, "GAS");
}

static READ8_HANDLER(pocketrc_steer_r)
{
	return input_port_read(space->machine, "STEERING");
}

ADDRESS_MAP_START( c76_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(M37710_ADC7_L, M37710_ADC7_L) AM_READ(dac7_r)
	AM_RANGE(M37710_ADC6_L, M37710_ADC6_L) AM_READ(dac6_r)
	AM_RANGE(M37710_ADC5_L, M37710_ADC5_L) AM_READ(dac5_r)
	AM_RANGE(M37710_ADC4_L, M37710_ADC4_L) AM_READ(dac4_r)
	AM_RANGE(M37710_ADC3_L, M37710_ADC3_L) AM_READ(dac3_r)
	AM_RANGE(M37710_ADC2_L, M37710_ADC2_L) AM_READ(dac2_r)
	AM_RANGE(M37710_ADC1_L, M37710_ADC1_L) AM_READ(dac1_r)
	AM_RANGE(M37710_ADC0_L, M37710_ADC0_L) AM_READ(dac0_r)

	AM_RANGE(M37710_ADC0_H, M37710_ADC7_H) AM_READNOP
ADDRESS_MAP_END

INTERRUPT_GEN( c76_interrupt )
{
	switch (cpu_getiloops(device))
	{
		case 0:
			cpu_set_input_line(device, M37710_LINE_IRQ0, HOLD_LINE);
			break;

		case 1:
			cpu_set_input_line(device, M37710_LINE_IRQ2, HOLD_LINE);
			break;

		case 2:
			cpu_set_input_line(device, M37710_LINE_ADC, HOLD_LINE);
			break;
	}
}

static const struct
{
	const char *s_name;
	read32_space_func keycus_r;
	int n_daughterboard;
} namcos11_config_table[] =
{
	{ "tekken", NULL, 32 },
	{ "tekkena", NULL, 32 },
	{ "tekkenb", NULL, 32 },
	{ "tekkenc", NULL, 32 },
	{ "tekken2", keycus_c406_r, 32 },
	{ "tekken2a", keycus_c406_r, 32 },
	{ "tekken2b", keycus_c406_r, 32 },
	{ "souledge", keycus_c409_r, 32 },
	{ "souledga", keycus_c409_r, 32 },
	{ "souledgb", keycus_c409_r, 32 },
	{ "souledgc", keycus_c409_r, 32 },
	{ "dunkmnia", keycus_c410_r, 32 },
	{ "dunkmnic", keycus_c410_r, 32 },
	{ "xevi3dg",  keycus_c430_r, 32 },
	{ "primglex", keycus_c411_r, 32 },
	{ "danceyes", keycus_c431_r, 32 },
	{ "pocketrc", keycus_c432_r, 32 },
	{ "starswep", keycus_c442_r, 0 },
	{ "myangel3", keycus_c443_r, 64 },
	{ "ptblnk2a", keycus_c443_r, 64 },
	{ NULL, NULL }
};

static DRIVER_INIT( namcos11 )
{
	int n_game;

	memory_install_readwrite16_handler(cputag_get_address_space(machine, "c76", ADDRESS_SPACE_PROGRAM), 0x82, 0x83, 0, 0, c76_speedup_r, c76_speedup_w);

	if( strcmp( machine->gamedrv->name, "pocketrc" ) == 0 )
	{
		memory_install_read8_handler(cputag_get_address_space(machine, "c76", ADDRESS_SPACE_IO), M37710_ADC0_L, M37710_ADC0_L, 0, 0, pocketrc_steer_r);
		memory_install_read8_handler(cputag_get_address_space(machine, "c76", ADDRESS_SPACE_IO), M37710_ADC1_L, M37710_ADC1_L, 0, 0, pocketrc_gas_r);
	}

	psx_driver_init(machine);

	n_game = 0;
	while( namcos11_config_table[ n_game ].s_name != NULL )
	{
		if( strcmp( machine->gamedrv->name, namcos11_config_table[ n_game ].s_name ) == 0 )
		{
			if( namcos11_config_table[ n_game ].keycus_r != NULL )
			{
				memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1fa20000, 0x1fa2ffff, 0, 0, namcos11_config_table[ n_game ].keycus_r );
			}
			if( namcos11_config_table[ n_game ].n_daughterboard != 0 )
			{
				int bank;
				UINT32 len = memory_region_length( machine, "user2" );
				UINT8 *rgn = memory_region( machine, "user2" );

				memory_install_read_bank(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f000000, 0x1f0fffff, 0, 0, "bank1" );
				memory_install_read_bank(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f100000, 0x1f1fffff, 0, 0, "bank2" );
				memory_install_read_bank(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f200000, 0x1f2fffff, 0, 0, "bank3" );
				memory_install_read_bank(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f300000, 0x1f3fffff, 0, 0, "bank4" );
				memory_install_read_bank(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f400000, 0x1f4fffff, 0, 0, "bank5" );
				memory_install_read_bank(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f500000, 0x1f5fffff, 0, 0, "bank6" );
				memory_install_read_bank(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f600000, 0x1f6fffff, 0, 0, "bank7" );
				memory_install_read_bank(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f700000, 0x1f7fffff, 0, 0, "bank8" );

				for( bank = 0; bank < 8; bank++ )
				{
					memory_configure_bank(machine, bankname[bank], 0, len / ( 1024 * 1024 ), rgn, 1024 * 1024 );
					memory_set_bank(machine, bankname[bank], 0 );
				}

				if( namcos11_config_table[ n_game ].n_daughterboard == 32 )
				{
					memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1fa10020, 0x1fa1002f, 0, 0, bankswitch_rom32_w );
				}
				if( namcos11_config_table[ n_game ].n_daughterboard == 64 )
				{
					m_n_bankoffset = 0;
					memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f080000, 0x1f080003, 0, 0, bankswitch_rom64_upper_w );
					memory_nop_read(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1fa10020, 0x1fa1002f, 0, 0 );
					memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1fa10020, 0x1fa1002f, 0, 0, bankswitch_rom64_w );
					state_save_register_global(machine,  m_n_bankoffset );
				}
			}
			else
			{
				memory_nop_write(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1fa10020, 0x1fa1002f, 0, 0 );
			}
			break;
		}
		n_game++;
	}

	if( strcmp( machine->gamedrv->name, "ptblnk2a" ) == 0 )
	{
		memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f788000, 0x1f788003, 0, 0, lightgun_w );
		memory_install_read32_handler (cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f780000, 0x1f78000f, 0, 0, lightgun_r );
	}
}

static MACHINE_RESET( namcos11 )
{
	memset( namcos11_keycus, 0, namcos11_keycus_size );
	psx_machine_init(machine);
}

static MACHINE_DRIVER_START( coh100 )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",  PSXCPU, XTAL_67_7376MHz )
	MDRV_CPU_PROGRAM_MAP( namcos11_map)
	MDRV_CPU_VBLANK_INT("screen", namcos11_vblank)

	MDRV_CPU_ADD("c76", M37702, 16384000)
	MDRV_CPU_PROGRAM_MAP(c76_map)
	MDRV_CPU_IO_MAP(c76_io_map)
	MDRV_CPU_VBLANK_INT_HACK(c76_interrupt, 3)

	MDRV_MACHINE_RESET( namcos11 )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE( 60 )
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE( 1024, 1024 )
	MDRV_SCREEN_VISIBLE_AREA( 0, 639, 0, 479 )

	MDRV_PALETTE_LENGTH( 65536 )

	MDRV_PALETTE_INIT( psx )
	MDRV_VIDEO_START( psx_type1 )
	MDRV_VIDEO_UPDATE( psx )

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MDRV_SOUND_ADD("c352", C352, 16384000)
	MDRV_SOUND_ROUTE(0, "rspeaker", 1.00)
	MDRV_SOUND_ROUTE(1, "lspeaker", 1.00)
	MDRV_SOUND_ROUTE(2, "rspeaker", 1.00)
	MDRV_SOUND_ROUTE(3, "lspeaker", 1.00)

	MDRV_AT28C16_ADD( "at28c16", NULL )
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( coh110 )
	MDRV_IMPORT_FROM( coh100 )
	MDRV_VIDEO_START( psx_type2 )

	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_REFRESH_RATE( 30 )
MACHINE_DRIVER_END

static INPUT_PORTS_START( namcos11 )
	PORT_START( "SWITCH" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_DIPNAME( 0x02, 0x02, "DIP1 (Test)" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, "DIP2 (Freeze)" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "TEST" )
	PORT_DIPNAME( 0x80, 0x00, "TDIP8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "TDIP7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "TDIP6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "TDIP5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "TDIP4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "TDIP3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "TDIP2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x00, "TDIP1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )

	PORT_START( "PLAYER1" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START( "PLAYER2" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START( "PLAYER3" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER( 3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER( 3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER( 3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER( 3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START3 )

	PORT_START( "PLAYER4" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER( 4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER( 4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER( 4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER( 4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 4 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 4 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )
INPUT_PORTS_END

static INPUT_PORTS_START( tekken )
	PORT_INCLUDE( namcos11 )

	PORT_MODIFY( "SWITCH" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY( "PLAYER1" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY( "PLAYER2" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY( "PLAYER3" )
	PORT_BIT( 0xcf, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 )

	PORT_MODIFY( "PLAYER4" )
	PORT_BIT( 0xcf, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER( 2 )
INPUT_PORTS_END

static INPUT_PORTS_START( souledge )
	PORT_INCLUDE( namcos11 )

	PORT_MODIFY( "SWITCH" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY( "PLAYER3" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY( "PLAYER4" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( myangel3 )
	PORT_INCLUDE( namcos11 )

	PORT_MODIFY( "SWITCH" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY( "PLAYER1" )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_MODIFY( "PLAYER2" )
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_MODIFY( "PLAYER3" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY( "PLAYER4" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( ptblnk2a )
	PORT_INCLUDE( namcos11 )

	PORT_MODIFY( "PLAYER1" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY( "PLAYER2" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY( "PLAYER3" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY( "PLAYER4" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "GUN1X" )
	PORT_BIT( 0xffff, 0x022f, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0xd8, 0x387 ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 15 ) PORT_PLAYER( 1 )

	PORT_START( "GUN1Y" )
	PORT_BIT( 0xffff, 0x00a8, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0x2c, 0x11b ) PORT_SENSITIVITY( 50 ) PORT_KEYDELTA( 15 ) PORT_PLAYER( 1 )

	PORT_START( "GUN2X" )
	PORT_BIT( 0xffff, 0x022f, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0xd8, 0x387 ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 15 ) PORT_PLAYER( 2 )

	PORT_START( "GUN2Y" )
	PORT_BIT( 0xffff, 0x00a8, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0x2c, 0x11b ) PORT_SENSITIVITY( 50 ) PORT_KEYDELTA( 15 ) PORT_PLAYER( 2 )
INPUT_PORTS_END

static INPUT_PORTS_START( pocketrc )
	PORT_INCLUDE( namcos11 )

	PORT_MODIFY( "SWITCH" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY( "PLAYER1" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY( "PLAYER2" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY( "PLAYER3" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY( "PLAYER4" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "STEERING" )
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX( 0x00, 0xff ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 3 ) PORT_CENTERDELTA( 128 ) PORT_PLAYER( 1 ) PORT_REVERSE

	PORT_START( "GAS" )
	PORT_BIT( 0x00ff, 0x0000, IPT_PEDAL ) PORT_MINMAX( 0x0000, 0x00ff ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 15 ) PORT_PLAYER( 1 ) PORT_REVERSE
INPUT_PORTS_END

ROM_START( danceyes )
	ROM_REGION32_LE( 0x0400000, "user1", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "dc1vera.2l",   0x0000000, 0x100000, CRC(b164ad67) SHA1(62a7f9bc7fb9c218e5031598512dbd1e013283b3) )
	ROM_LOAD16_BYTE( "dc1vera.2j",   0x0000001, 0x100000, CRC(28e4cb3d) SHA1(17923c66725da1f8e77b7c08d8017160bba53eb9) )
	ROM_LOAD16_BYTE( "dc1vera.2k",   0x0200000, 0x100000, CRC(bdd9484e) SHA1(2f85e0ac4b12b2cf4c9717fad745d2d68c27d39a) )
	ROM_LOAD16_BYTE( "dc1vera.2f",   0x0200001, 0x100000, CRC(25a2f06f) SHA1(400806a262681cf908ac16e039686b5a0d5fd58e) )

	ROM_REGION32_LE( 0x1000000, "user2", 0 ) /* main data */
	ROM_LOAD16_BYTE( "dc1rom0l.ic5", 0x0000000, 0x200000, CRC(8b5b4b13) SHA1(9681e29ad95a2cc555d0fbb558a0865a87a77268) )
	ROM_LOAD16_BYTE( "dc1rom0u.ic6", 0x0000001, 0x200000, CRC(93ca9bd0) SHA1(c7d2ecbeb451bd57097fb39f21dc347b86a2b838) )
	ROM_LOAD16_BYTE( "dc1rom1l.ic3", 0x0400000, 0x200000, CRC(380e0282) SHA1(33389e3b15b93fb939991b8d7dc8b182ba88e78a) )
	ROM_LOAD16_BYTE( "dc1rom1u.ic8", 0x0400001, 0x200000, CRC(47d966a7) SHA1(3b6f004136bd9d75dbef846f417ca34a56893d61) )
	ROM_LOAD16_BYTE( "dc1rom2l.ic4", 0x0800000, 0x200000, CRC(8f130220) SHA1(7f086d502178c27511c1142254381a9a2a999e8d) )
	ROM_LOAD16_BYTE( "dc1rom2u.ic7", 0x0800001, 0x200000, CRC(24514dc6) SHA1(f9f7d6a45cbd51513cc038f6321a30c1a72b7a58) )
	ROM_LOAD16_BYTE( "dc1rom3l.ic1", 0x0c00000, 0x200000, CRC(a76bcd4c) SHA1(817abdc43158b7aaac329c3ea17782277acb36a4) )
	ROM_LOAD16_BYTE( "dc1rom3u.ic9", 0x0c00001, 0x200000, CRC(1405d123) SHA1(3d7be5558358740f5a0a3a3022543cf5aca4cf24) )

	ROM_REGION16_LE( 0x100000, "c76", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "dc1sprog.6d",  0x0000000, 0x040000, CRC(96cd7788) SHA1(68a5a53a5fc50e2b6b684c99d27d81e3a8c56287) )
	ROM_LOAD( "c76.bin",      0x040000, 0x004000, CRC(399faac7) SHA1(ceb184ef0486caf715dd997101999785f67a40b8) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "dc1wave.8k",   0x000000, 0x400000, CRC(8ba0f6a7) SHA1(e9868debd808e92b196d1baeeeae9c4855356a01) )
	ROM_RELOAD( 0x800000, 0x400000 )
ROM_END

ROM_START( dunkmnia )
	ROM_REGION32_LE( 0x0400000, "user1", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "dm2verc.2l",   0x0000000, 0x100000, CRC(f6a6c46e) SHA1(5552c3a8fb0ef18e6f0bca8d81ec68b904824a66) )
	ROM_LOAD16_BYTE( "dm2verc.2j",   0x0000001, 0x100000, CRC(1df539ce) SHA1(89ac436c685cf26899168c73fdaccee778fd7d2d) )
	ROM_LOAD16_BYTE( "dm1verc.2k",   0x0200000, 0x100000, CRC(c8d72f78) SHA1(30341301f0d1cdcb14f3e0672ec1165c0583fbc8) )
	ROM_LOAD16_BYTE( "dm1verc.2f",   0x0200001, 0x100000, CRC(d379dfa9) SHA1(142cb70b5ea060c961c5bc60a624643b5ec390df) )

	ROM_REGION32_LE( 0x1000000, "user2", 0 ) /* main data */
	ROM_LOAD16_BYTE( "dm1rom0l.ic5", 0x0000000, 0x200000, CRC(4bb5d71d) SHA1(7d61211c7a6d1f6593604390fe99206a6a8cc7b3) )
	ROM_CONTINUE( 0x0000000, 0x200000 ) /* first & second half identical */
	ROM_LOAD16_BYTE( "dm1rom0u.ic6", 0x0000001, 0x200000, CRC(c16b47c5) SHA1(0fb2c5bc4674b3366762127c6333fb3a837b4de2) )
	ROM_CONTINUE( 0x0000001, 0x200000 ) /* first & second half identical */
	ROM_LOAD16_BYTE( "dm1rom1l.ic3", 0x0400000, 0x200000, CRC(20dd3294) SHA1(b2fd5075b6281ac7bfc2681fc282f9ebaa089af5) )
	ROM_CONTINUE( 0x0400000, 0x200000 ) /* first & second half identical */
	ROM_LOAD16_BYTE( "dm1rom1u.ic8", 0x0400001, 0x200000, CRC(01e905d3) SHA1(430b2ae0c67265b6acc8aa4dd50f6144929993f8) )
	ROM_CONTINUE( 0x0400001, 0x200000 ) /* first & second half identical */

	ROM_REGION16_LE( 0x100000, "c76", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "dm1sprog.6d",  0x0000000, 0x040000, CRC(de1cbc78) SHA1(855ebece1841f50ae324d7d6b8b18ab6f657d28e) )
	ROM_LOAD( "c76.bin",      0x040000, 0x004000, CRC(399faac7) SHA1(ceb184ef0486caf715dd997101999785f67a40b8) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "dm1wave.8k",   0x000000, 0x400000, CRC(4891d53e) SHA1(a1fee060e94d3219174b5974517f4fd3be32aaa5) )
	ROM_RELOAD( 0x800000, 0x400000 )
ROM_END

ROM_START( dunkmniaj )
	ROM_REGION32_LE( 0x0400000, "user1", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "dm1verc.2l",   0x0000000, 0x100000, CRC(6c81654a) SHA1(00e84cc41b3dd49a8f0d3a364ea4a5d2662c45ff) )
	ROM_LOAD16_BYTE( "dm1verc.2j",   0x0000001, 0x100000, CRC(10329b7e) SHA1(d214764e90c7d79abea01580e79092e34a58b695) )
	ROM_LOAD16_BYTE( "dm1verc.2k",   0x0200000, 0x100000, CRC(c8d72f78) SHA1(30341301f0d1cdcb14f3e0672ec1165c0583fbc8) )
	ROM_LOAD16_BYTE( "dm1verc.2f",   0x0200001, 0x100000, CRC(d379dfa9) SHA1(142cb70b5ea060c961c5bc60a624643b5ec390df) )

	ROM_REGION32_LE( 0x1000000, "user2", 0 ) /* main data */
	ROM_LOAD16_BYTE( "dm1rom0l.ic5", 0x0000000, 0x200000, CRC(4bb5d71d) SHA1(7d61211c7a6d1f6593604390fe99206a6a8cc7b3) )
	ROM_CONTINUE( 0x0000000, 0x200000 ) /* first & second half identical */
	ROM_LOAD16_BYTE( "dm1rom0u.ic6", 0x0000001, 0x200000, CRC(c16b47c5) SHA1(0fb2c5bc4674b3366762127c6333fb3a837b4de2) )
	ROM_CONTINUE( 0x0000001, 0x200000 ) /* first & second half identical */
	ROM_LOAD16_BYTE( "dm1rom1l.ic3", 0x0400000, 0x200000, CRC(20dd3294) SHA1(b2fd5075b6281ac7bfc2681fc282f9ebaa089af5) )
	ROM_CONTINUE( 0x0400000, 0x200000 ) /* first & second half identical */
	ROM_LOAD16_BYTE( "dm1rom1u.ic8", 0x0400001, 0x200000, CRC(01e905d3) SHA1(430b2ae0c67265b6acc8aa4dd50f6144929993f8) )
	ROM_CONTINUE( 0x0400001, 0x200000 ) /* first & second half identical */

	ROM_REGION16_LE( 0x100000, "c76", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "dm1sprog.6d",  0x0000000, 0x040000, CRC(de1cbc78) SHA1(855ebece1841f50ae324d7d6b8b18ab6f657d28e) )
	ROM_LOAD( "c76.bin",      0x040000, 0x004000, CRC(399faac7) SHA1(ceb184ef0486caf715dd997101999785f67a40b8) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "dm1wave.8k",   0x800000, 0x400000, CRC(4891d53e) SHA1(a1fee060e94d3219174b5974517f4fd3be32aaa5) )
ROM_END

ROM_START( myangel3 )
	ROM_REGION32_LE( 0x0400000, "user1", 0 ) /* main prg */
	ROM_LOAD( "kqt1vera.1j",         0x0000000, 0x200000, CRC(df7aef8a) SHA1(d4ff144bcdecc1d4a3b834d0b9c182609ad9b260) )
	ROM_LOAD( "kqt1vera.1l",         0x0200000, 0x200000, CRC(ffc51c01) SHA1(bba2c2c1ad31039c7dc7413e51e7fc317451e1e3) )

	ROM_REGION32_LE( 0x2000000, "user2", 0 ) /* main data */
	ROM_LOAD16_BYTE( "kqt1prg0l.ic2", 0x0000000, 0x800000, CRC(d67eee66) SHA1(1842e987ae406ac2cf4c173aaaa73b5f67f4fd3d) )
	ROM_LOAD16_BYTE( "kqt1prg0u.ic5", 0x0000001, 0x800000, CRC(4d1c7bf3) SHA1(6bf9712264a82920f7898896643d05a6b425e74a) )
	ROM_LOAD16_BYTE( "kqt1prg1l.ic3", 0x1000000, 0x800000, CRC(298d8eeb) SHA1(c421b1bdd5fd46c026a41e2cec47cafd1a69d33d) )
	ROM_LOAD16_BYTE( "kqt1prg1u.ic6", 0x1000001, 0x800000, CRC(911783db) SHA1(1005fc9b38e212844e397150a6f98f43ad88d4b9) )

	ROM_REGION16_LE( 0x100000, "c76", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "kqt1sprog.7e", 0x0000000, 0x040000, CRC(bb1888a6) SHA1(4db07738079725413cdba7eb75252ee71ae50a66) )
	ROM_LOAD( "c76.bin",      0x040000, 0x004000, CRC(399faac7) SHA1(ceb184ef0486caf715dd997101999785f67a40b8) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "kqt1wave.8k",  0x000000, 0x400000, CRC(92ca8e4f) SHA1(48d6bdfcc5de1c280afa36c3f0dd6d4177771355) )
	ROM_RELOAD( 0x800000, 0x400000 )
ROM_END

ROM_START( pocketrc )
	ROM_REGION32_LE( 0x0400000, "user1", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "pkr1verb.2l",  0x000000, 0x100000, CRC(300d906a) SHA1(f521858f78284d69e6a6ec2c35a468c87dd0628c) )
	ROM_LOAD16_BYTE( "pkr1verb.2j",  0x000001, 0x100000, CRC(d5f47526) SHA1(b1d5b6f714510d0cfcc9b32708287755f9f27ead) )

	ROM_REGION32_LE( 0x1000000, "user2", 0 ) /* main data */
	ROM_LOAD16_BYTE( "pkr1rom0l.ic5", 0x000000, 0x200000, CRC(6c9b074c) SHA1(885f342bd178e4146e1f75259206f6625c0b3c18) )
	ROM_LOAD16_BYTE( "pkr1rom0u.ic6", 0x000001, 0x200000, CRC(a55c0906) SHA1(3b6abfa877f88a4d96222d98af02498b0c777af6) )

	ROM_REGION16_LE( 0x100000, "c76", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "pkr1verb.6d",  0x000000, 0x040000, CRC(9bf08992) SHA1(fca7943f7bcf0ee758fa63fbdef8f7456b9e46cb) )
	ROM_LOAD( "c76.bin",      0x040000, 0x004000, CRC(399faac7) SHA1(ceb184ef0486caf715dd997101999785f67a40b8) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "pkr1wave.8k",  0x000000, 0x400000, CRC(72517c46) SHA1(d0dcc750fe8eca9e965e7c366ac39a42ffd76557) )
	ROM_RELOAD( 0x800000, 0x400000 )
ROM_END

ROM_START( primglex )
	ROM_REGION32_LE( 0x0400000, "user1", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "pg1vera.2l",   0x0000000, 0x100000, CRC(fc15fd1a) SHA1(6ca5ebdc096cab3296dc7c1f675d78dfc7c69a05) )
	ROM_LOAD16_BYTE( "pg1vera.2j",   0x0000001, 0x100000, CRC(79955553) SHA1(ad2dca38b06a835f8241fae0a5fa18d5874cebe4) )

	ROM_REGION32_LE( 0x1000000, "user2", 0 ) /* main data */
	ROM_LOAD16_BYTE( "pg1rom0u.ic5", 0x0000000, 0x200000, CRC(2a503f2f) SHA1(206b9c9204be22241d2a3e017b96c3a103f5a976) )
	ROM_CONTINUE( 0x0000000, 0x200000 ) /* first & second half identical */
	ROM_LOAD16_BYTE( "pg1rom0l.ic6", 0x0000001, 0x200000, CRC(54cef992) SHA1(5ba81353b1dddc1c6640fc4c15df81535e7a6ae8) )
	ROM_CONTINUE( 0x0000001, 0x200000 ) /* first & second half identical */
	ROM_LOAD16_BYTE( "pg1rom1u.ic3", 0x0400000, 0x200000, CRC(1ee41152) SHA1(d240e6ba820aa2aa4f12380c255f624f91aed564) )
	ROM_CONTINUE( 0x0400000, 0x200000 ) /* first & second half identical */
	ROM_LOAD16_BYTE( "pg1rom1l.ic8", 0x0400001, 0x200000, CRC(59b5a71c) SHA1(ddc1f0a5488466166c21fd0c84ab2b4cf04316bf) )
	ROM_CONTINUE( 0x0400001, 0x200000 ) /* first & second half identical */

	ROM_REGION16_LE( 0x100000, "c76", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "pg1sprog.6d",  0x0000000, 0x040000, CRC(e7c3396d) SHA1(12bbb8ebcaab1b40462a12917dd9b58bd9ab8663) )
	ROM_LOAD( "c76.bin",      0x040000, 0x004000, CRC(399faac7) SHA1(ceb184ef0486caf715dd997101999785f67a40b8) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "pg1wave.8k",   0x0000000, 0x400000, CRC(fc9ad9eb) SHA1(ce5bb2288ed8cf1348825c39423cbb99d9324b9c) )
	ROM_RELOAD( 0x800000, 0x400000 )
ROM_END

ROM_START( ptblnk2a )
	ROM_REGION32_LE( 0x0400000, "user1", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "gnb3vera.2l",  0x0000000, 0x100000, CRC(57ad719a) SHA1(f22a02d33c7c23cccffb8ce2e3aca26b07ecac0a) )
	ROM_LOAD16_BYTE( "gnb3vera.2j",  0x0000001, 0x100000, CRC(0378af98) SHA1(601444b5a0935a4b69b5ada618aaf1bc6bb12a3b) )
	ROM_LOAD16_BYTE( "gnb3vera.2k",  0x0200000, 0x100000, CRC(e6335e4e) SHA1(9067f05d848c1c8a88967a3c6552d2d24e80672b) )
	ROM_LOAD16_BYTE( "gnb3vera.2f",  0x0200001, 0x100000, CRC(2bb7eb6d) SHA1(d1b1e031a28443140ac8652dfd77a65a042b67fc) )

	ROM_REGION32_LE( 0x2000000, "user2", 0 ) /* main data */
	ROM_LOAD16_BYTE( "gnb1prg0l.ic2", 0x000000, 0x800000, CRC(78746037) SHA1(d130ca1153a730e3c967945248f00662f9fab304) )
	ROM_LOAD16_BYTE( "gnb1prg0u.ic5", 0x000001, 0x800000, CRC(697d3279) SHA1(40302780f7494d9413888b2d1da38bd14a9a444f) )

	ROM_REGION16_LE( 0x100000, "c76", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "gnb1vera.6d",  0x0000000, 0x040000, CRC(6461ae77) SHA1(1377b716a69ef9d4d2e48083d23f22bd5c103c00) )
	ROM_LOAD( "c76.bin",      0x040000, 0x004000, CRC(399faac7) SHA1(ceb184ef0486caf715dd997101999785f67a40b8) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "gnb1wave.8k",  0x0000000, 0x400000, CRC(4e19d9d6) SHA1(0a92c987536999a789663a30c787950ab6995128) )
	ROM_RELOAD( 0x800000, 0x400000 )
ROM_END

ROM_START( souledge )
	ROM_REGION32_LE( 0x0400000, "user1", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "so4verc.2l",   0x0000000, 0x100000, CRC(12b8ae0d) SHA1(31571023d5b77ebcd4103b8cac5ba710a3d570a0) )
	ROM_LOAD16_BYTE( "so4verc.2j",   0x0000001, 0x100000, CRC(938262b0) SHA1(e806883e32c473a3c2bb07849126631f6d66fa66) )
	ROM_LOAD16_BYTE( "so1verc.2k",   0x0200000, 0x100000, CRC(1789e399) SHA1(2e11869124cdfa7a66107b0e642bdc72ee0c306a) )
	ROM_LOAD16_BYTE( "so1verc.2f",   0x0200001, 0x100000, CRC(8cffe1c3) SHA1(d54a0b1d55f33db2890bfa70c411cca3e446fccf) )

	ROM_REGION32_LE( 0x1000000, "user2", 0 ) /* main data */
	ROM_LOAD16_BYTE( "so1rom0u.ic5", 0x0000000, 0x200000, CRC(e364d673) SHA1(70fd58fb912939c57e3a5fadd01c1467df08d4ac) )
	ROM_LOAD16_BYTE( "so1rom0l.ic6", 0x0000001, 0x200000, CRC(9c5b0858) SHA1(f3ac726f1167551beea7edc46e43b826b7baaf62) )
	ROM_LOAD16_BYTE( "so1rom1u.ic3", 0x0400000, 0x200000, CRC(8f9d8c5b) SHA1(ac1da70854eee344a645749f564366ceac571767) )
	ROM_LOAD16_BYTE( "so1rom1l.ic8", 0x0400001, 0x200000, CRC(4406ef16) SHA1(c86f199fdb1db23e5944ca51e6518b9cd0dafb71) )
	ROM_LOAD16_BYTE( "so1rom2u.ic4", 0x0800000, 0x200000, CRC(b4baa886) SHA1(0432692a4d71a3f1b47707efb6858927744940e4) )
	ROM_LOAD16_BYTE( "so1rom2l.ic7", 0x0800001, 0x200000, CRC(37c1f66e) SHA1(13a8a73fce142ea5ebe3f0c1050e44a027ab42a6) )
	ROM_LOAD16_BYTE( "so1rom3u.ic1", 0x0c00000, 0x200000, CRC(f11bd521) SHA1(baf936dec58cebfeef1c74f95e455b2fe74eb982) )
	ROM_LOAD16_BYTE( "so1rom3l.ic9", 0x0c00001, 0x200000, CRC(84465bcc) SHA1(d8be888d41cfe194c3a1853d9146d3a74ef7bab1) )

	ROM_REGION16_LE( 0x100000, "c76", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "so1sprogc.6d", 0x0000000, 0x040000, CRC(2bbc118c) SHA1(4168a9aa525f1f0ce6cf6e14cfe4c118c4c0d773) )
	ROM_LOAD( "c76.bin",      0x040000, 0x004000, CRC(399faac7) SHA1(ceb184ef0486caf715dd997101999785f67a40b8) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "so1wave.8k",   0x000000, 0x400000, CRC(0e68836b) SHA1(c392b370a807803c7ab060105861253e1b407f49) )
	ROM_RELOAD( 0x800000, 0x400000 )
ROM_END

ROM_START( souledgea )
	ROM_REGION32_LE( 0x0400000, "user1", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "so3verc.2l",   0x0000000, 0x100000, CRC(c90e343b) SHA1(83bd2fc67493c34b408b13148f6a2feba9a44ed0) )
	ROM_LOAD16_BYTE( "so3verc.2j",   0x0000001, 0x100000, CRC(b7466db5) SHA1(c7a4c66e61812fb6de582fe555755293d78771c7) )
	ROM_LOAD16_BYTE( "so1verc.2k",   0x0200000, 0x100000, CRC(1789e399) SHA1(2e11869124cdfa7a66107b0e642bdc72ee0c306a) )
	ROM_LOAD16_BYTE( "so1verc.2f",   0x0200001, 0x100000, CRC(8cffe1c3) SHA1(d54a0b1d55f33db2890bfa70c411cca3e446fccf) )

	ROM_REGION32_LE( 0x1000000, "user2", 0 ) /* main data */
	ROM_LOAD16_BYTE( "so1rom0u.ic5", 0x0000000, 0x200000, CRC(e364d673) SHA1(70fd58fb912939c57e3a5fadd01c1467df08d4ac) )
	ROM_LOAD16_BYTE( "so1rom0l.ic6", 0x0000001, 0x200000, CRC(9c5b0858) SHA1(f3ac726f1167551beea7edc46e43b826b7baaf62) )
	ROM_LOAD16_BYTE( "so1rom1u.ic3", 0x0400000, 0x200000, CRC(8f9d8c5b) SHA1(ac1da70854eee344a645749f564366ceac571767) )
	ROM_LOAD16_BYTE( "so1rom1l.ic8", 0x0400001, 0x200000, CRC(4406ef16) SHA1(c86f199fdb1db23e5944ca51e6518b9cd0dafb71) )
	ROM_LOAD16_BYTE( "so1rom2u.ic4", 0x0800000, 0x200000, CRC(b4baa886) SHA1(0432692a4d71a3f1b47707efb6858927744940e4) )
	ROM_LOAD16_BYTE( "so1rom2l.ic7", 0x0800001, 0x200000, CRC(37c1f66e) SHA1(13a8a73fce142ea5ebe3f0c1050e44a027ab42a6) )
	ROM_LOAD16_BYTE( "so1rom3u.ic1", 0x0c00000, 0x200000, CRC(f11bd521) SHA1(baf936dec58cebfeef1c74f95e455b2fe74eb982) )
	ROM_LOAD16_BYTE( "so1rom3l.ic9", 0x0c00001, 0x200000, CRC(84465bcc) SHA1(d8be888d41cfe194c3a1853d9146d3a74ef7bab1) )

	ROM_REGION16_LE( 0x100000, "c76", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "so1sprogc.6d", 0x0000000, 0x040000, CRC(2bbc118c) SHA1(4168a9aa525f1f0ce6cf6e14cfe4c118c4c0d773) )
	ROM_LOAD( "c76.bin",      0x040000, 0x004000, CRC(399faac7) SHA1(ceb184ef0486caf715dd997101999785f67a40b8) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "so1wave.8k",   0x000000, 0x400000, CRC(0e68836b) SHA1(c392b370a807803c7ab060105861253e1b407f49) )
	ROM_RELOAD( 0x800000, 0x400000 )
ROM_END

ROM_START( souledge1 )
	ROM_REGION32_LE( 0x0400000, "user1", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "so3vera.2l",   0x0000000, 0x100000, CRC(19b39096) SHA1(9188cd1fd9b15e5545eb41ea2768a8bd42113379) )
	ROM_LOAD16_BYTE( "so3vera.2j",   0x0000001, 0x100000, CRC(09eda46f) SHA1(24d04d2ba51af508ddc0656e8bb5e1335b08cc8a) )
	ROM_LOAD16_BYTE( "so1vera.2k",   0x0200000, 0x100000, CRC(29bdc6bb) SHA1(9047792c1a21a0001024939f1c6a8c3d86948973) )
	ROM_LOAD16_BYTE( "so1vera.2f",   0x0200001, 0x100000, CRC(c035b71b) SHA1(38719a75193774b124d845460c0c03d36849719d) )

	ROM_REGION32_LE( 0x1000000, "user2", 0 ) /* main data */
	ROM_LOAD16_BYTE( "so1rom0u.ic5", 0x0000000, 0x200000, CRC(e364d673) SHA1(70fd58fb912939c57e3a5fadd01c1467df08d4ac) )
	ROM_LOAD16_BYTE( "so1rom0l.ic6", 0x0000001, 0x200000, CRC(9c5b0858) SHA1(f3ac726f1167551beea7edc46e43b826b7baaf62) )
	ROM_LOAD16_BYTE( "so1rom1u.ic3", 0x0400000, 0x200000, CRC(8f9d8c5b) SHA1(ac1da70854eee344a645749f564366ceac571767) )
	ROM_LOAD16_BYTE( "so1rom1l.ic8", 0x0400001, 0x200000, CRC(4406ef16) SHA1(c86f199fdb1db23e5944ca51e6518b9cd0dafb71) )
	ROM_LOAD16_BYTE( "so1rom2u.ic4", 0x0800000, 0x200000, CRC(b4baa886) SHA1(0432692a4d71a3f1b47707efb6858927744940e4) )
	ROM_LOAD16_BYTE( "so1rom2l.ic7", 0x0800001, 0x200000, CRC(37c1f66e) SHA1(13a8a73fce142ea5ebe3f0c1050e44a027ab42a6) )
	ROM_LOAD16_BYTE( "so1rom3u.ic1", 0x0c00000, 0x200000, CRC(f11bd521) SHA1(baf936dec58cebfeef1c74f95e455b2fe74eb982) )
	ROM_LOAD16_BYTE( "so1rom3l.ic9", 0x0c00001, 0x200000, CRC(84465bcc) SHA1(d8be888d41cfe194c3a1853d9146d3a74ef7bab1) )

	ROM_REGION16_LE( 0x100000, "c76", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "so1sprog.6d",  0x0000000, 0x040000, CRC(f6f682b7) SHA1(a64e19be3f6e630b8c34f34b46b95aadfabd3f63) )
	ROM_LOAD( "c76.bin",      0x040000, 0x004000, CRC(399faac7) SHA1(ceb184ef0486caf715dd997101999785f67a40b8) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "so1wave.8k",   0x800000, 0x400000, CRC(0e68836b) SHA1(c392b370a807803c7ab060105861253e1b407f49) )
ROM_END

ROM_START( souledge1j )
	ROM_REGION32_LE( 0x0400000, "user1", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "so1vera.2l",   0x0000000, 0x100000, CRC(bafb94c8) SHA1(92461ce74a537a9801a024280e18fc3d0a5e6e5c) )
	ROM_LOAD16_BYTE( "so1vera.2j",   0x0000001, 0x100000, CRC(abe2d28e) SHA1(e9d858c8f8651b04bc72eb6de423da4925e94250) )
	ROM_LOAD16_BYTE( "so1vera.2k",   0x0200000, 0x100000, CRC(29bdc6bb) SHA1(9047792c1a21a0001024939f1c6a8c3d86948973) )
	ROM_LOAD16_BYTE( "so1vera.2f",   0x0200001, 0x100000, CRC(c035b71b) SHA1(38719a75193774b124d845460c0c03d36849719d) )

	ROM_REGION32_LE( 0x1000000, "user2", 0 ) /* main data */
	ROM_LOAD16_BYTE( "so1rom0u.ic5", 0x0000000, 0x200000, CRC(e364d673) SHA1(70fd58fb912939c57e3a5fadd01c1467df08d4ac) )
	ROM_LOAD16_BYTE( "so1rom0l.ic6", 0x0000001, 0x200000, CRC(9c5b0858) SHA1(f3ac726f1167551beea7edc46e43b826b7baaf62) )
	ROM_LOAD16_BYTE( "so1rom1u.ic3", 0x0400000, 0x200000, CRC(8f9d8c5b) SHA1(ac1da70854eee344a645749f564366ceac571767) )
	ROM_LOAD16_BYTE( "so1rom1l.ic8", 0x0400001, 0x200000, CRC(4406ef16) SHA1(c86f199fdb1db23e5944ca51e6518b9cd0dafb71) )
	ROM_LOAD16_BYTE( "so1rom2u.ic4", 0x0800000, 0x200000, CRC(b4baa886) SHA1(0432692a4d71a3f1b47707efb6858927744940e4) )
	ROM_LOAD16_BYTE( "so1rom2l.ic7", 0x0800001, 0x200000, CRC(37c1f66e) SHA1(13a8a73fce142ea5ebe3f0c1050e44a027ab42a6) )
	ROM_LOAD16_BYTE( "so1rom3u.ic1", 0x0c00000, 0x200000, CRC(f11bd521) SHA1(baf936dec58cebfeef1c74f95e455b2fe74eb982) )
	ROM_LOAD16_BYTE( "so1rom3l.ic9", 0x0c00001, 0x200000, CRC(84465bcc) SHA1(d8be888d41cfe194c3a1853d9146d3a74ef7bab1) )

	ROM_REGION16_LE( 0x100000, "c76", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "so1sprog.6d",  0x0000000, 0x040000, CRC(f6f682b7) SHA1(a64e19be3f6e630b8c34f34b46b95aadfabd3f63) )
	ROM_LOAD( "c76.bin",      0x040000, 0x004000, CRC(399faac7) SHA1(ceb184ef0486caf715dd997101999785f67a40b8) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "so1wave.8k",   0x000000, 0x400000, CRC(0e68836b) SHA1(c392b370a807803c7ab060105861253e1b407f49) )
	ROM_RELOAD( 0x800000, 0x400000 )
ROM_END

ROM_START( starswep )
	ROM_REGION32_LE( 0x0400000, "user1", 0 ) /* main prg */
	ROM_LOAD( "stp1vera.1j",         0x0000000, 0x200000, CRC(ef83e126) SHA1(f721b43358cedad0f28af5d2b292b44043fd47a0) )
	ROM_LOAD( "stp1vera.1l",         0x0200000, 0x200000, CRC(0ee7fe1e) SHA1(8c2f5b0e7b49dbe0e8105bf55c493acd46a4f59d) )

	ROM_REGION32_LE( 0x0100000, "user2", ROMREGION_ERASE00 ) /* main data */

	ROM_REGION16_LE( 0x100000, "c76", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "stp1sprog.7e", 0x0000000, 0x040000, CRC(08aaaf6a) SHA1(51c913a39ff7c154aef8bb10139cc8b92eb4756a) )
	ROM_LOAD( "c76.bin",      0x040000, 0x004000, CRC(399faac7) SHA1(ceb184ef0486caf715dd997101999785f67a40b8) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "stp1wave.8k",  0x000000, 0x400000, CRC(18f30e92) SHA1(b3819455856298527a7224495f541145aecf23dd) )
	ROM_RELOAD( 0x800000, 0x400000 )
ROM_END

ROM_START( tekken )
	ROM_REGION32_LE( 0x0400000, "user1", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "te4verc.2l",   0x0000000, 0x100000, CRC(7ecb7892) SHA1(7837f2b3dbfc6b4a153ea652e8a3fd89c4daa26e) )
	ROM_LOAD16_BYTE( "te4verc.2j",   0x0000001, 0x100000, CRC(eea3365d) SHA1(d13df90833aac48f9d9d20cddefb81f90ebab249) )
	ROM_LOAD16_BYTE( "te1verb.2k",   0x0200000, 0x100000, CRC(b9860b29) SHA1(678889fc5c70bf66f0bd9864a20636ffb620ed0d) )
	ROM_LOAD16_BYTE( "te1verb.2f",   0x0200001, 0x100000, CRC(3dc01aad) SHA1(266f346fa575c42b635bc469798f5aade9821e20) )

	ROM_REGION32_LE( 0x1000000, "user2", 0 ) /* main data */
	ROM_LOAD16_BYTE( "te1rom0l.ic5", 0x0000000, 0x200000, CRC(03786e09) SHA1(de2b9e19ace43c424d100dc5d3207217f66d6479) )
	ROM_LOAD16_BYTE( "te1rom0u.ic6", 0x0000001, 0x200000, CRC(75d91051) SHA1(1c7958162315576c3881dcc684b85710f7f19cd6) )
	ROM_LOAD16_BYTE( "te1rom1l.ic3", 0x0400000, 0x200000, CRC(81416f8e) SHA1(b42ff08ee84491c57a7c87bb767db7e2ec7a26c2) )
	ROM_LOAD16_BYTE( "te1rom1u.ic8", 0x0400001, 0x200000, CRC(fa7ba433) SHA1(e222c4b1631c09e58546446a1e82c16fca936f1d) )
	ROM_LOAD16_BYTE( "te1rom2l.ic4", 0x0800000, 0x200000, CRC(41d77846) SHA1(eeab049135c02a255899fe37e225c1111b2fbb7d) )
	ROM_LOAD16_BYTE( "te1rom2u.ic7", 0x0800001, 0x200000, CRC(a678987e) SHA1(c62c00ce5cf4d001723c999b2bc3dbb90283def1) )

	ROM_REGION16_LE( 0x100000, "c76", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "te1sprog.6d",  0x0000000, 0x040000, CRC(849587e9) SHA1(94c6a757b24758a866a41bd8acd46aa46844f74b) )
	ROM_LOAD( "c76.bin",      0x040000, 0x004000, CRC(399faac7) SHA1(ceb184ef0486caf715dd997101999785f67a40b8) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "te1wave.8k",   0x0000000, 0x200000, CRC(fce6c57a) SHA1(7fb8c69452c92c59a940a2b69d0d73ef7aefcb82) )
ROM_END

ROM_START( tekkena )
	ROM_REGION32_LE( 0x0400000, "user1", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "te2verc.2l",   0x0000000, 0x100000, CRC(a24c8c57) SHA1(bf69f17310201e38aa17fb4484cdb480f26d876e) )
	ROM_LOAD16_BYTE( "te2verc.2j",   0x0000001, 0x100000, CRC(3224c298) SHA1(8afe80af0eef52f96bf2ddb2482b06692a8f047d) )
	ROM_LOAD16_BYTE( "te1verb.2k",   0x0200000, 0x100000, CRC(b9860b29) SHA1(678889fc5c70bf66f0bd9864a20636ffb620ed0d) )
	ROM_LOAD16_BYTE( "te1verb.2f",   0x0200001, 0x100000, CRC(3dc01aad) SHA1(266f346fa575c42b635bc469798f5aade9821e20) )

	ROM_REGION32_LE( 0x1000000, "user2", 0 ) /* main data */
	ROM_LOAD16_BYTE( "te1rom0l.ic5", 0x0000000, 0x200000, CRC(03786e09) SHA1(de2b9e19ace43c424d100dc5d3207217f66d6479) )
	ROM_LOAD16_BYTE( "te1rom0u.ic6", 0x0000001, 0x200000, CRC(75d91051) SHA1(1c7958162315576c3881dcc684b85710f7f19cd6) )
	ROM_LOAD16_BYTE( "te1rom1l.ic3", 0x0400000, 0x200000, CRC(81416f8e) SHA1(b42ff08ee84491c57a7c87bb767db7e2ec7a26c2) )
	ROM_LOAD16_BYTE( "te1rom1u.ic8", 0x0400001, 0x200000, CRC(fa7ba433) SHA1(e222c4b1631c09e58546446a1e82c16fca936f1d) )
	ROM_LOAD16_BYTE( "te1rom2l.ic4", 0x0800000, 0x200000, CRC(41d77846) SHA1(eeab049135c02a255899fe37e225c1111b2fbb7d) )
	ROM_LOAD16_BYTE( "te1rom2u.ic7", 0x0800001, 0x200000, CRC(a678987e) SHA1(c62c00ce5cf4d001723c999b2bc3dbb90283def1) )

	ROM_REGION16_LE( 0x100000, "c76", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "te1sprog.6d",  0x0000000, 0x040000, CRC(849587e9) SHA1(94c6a757b24758a866a41bd8acd46aa46844f74b) )
	ROM_LOAD( "c76.bin",      0x040000, 0x004000, CRC(399faac7) SHA1(ceb184ef0486caf715dd997101999785f67a40b8) )

	ROM_REGION( 0x0200000, "c352", 0 ) /* samples */
	ROM_LOAD( "te1wave.8k",   0x0000000, 0x200000, CRC(fce6c57a) SHA1(7fb8c69452c92c59a940a2b69d0d73ef7aefcb82) )
ROM_END

ROM_START( tekkenb )
	ROM_REGION32_LE( 0x0400000, "user1", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "te2verb.2l",   0x0000000, 0x100000, CRC(246cfbdd) SHA1(c1bffe3c463e9eff6dcaf9937da72bff8ff33e4e) )
	ROM_LOAD16_BYTE( "te2verb.2j",   0x0000001, 0x100000, CRC(dfa83e47) SHA1(a29d852c1b6a52c043248d7e5af04067dfa3eb40) )
	ROM_LOAD16_BYTE( "te1verb.2k",   0x0200000, 0x100000, CRC(b9860b29) SHA1(678889fc5c70bf66f0bd9864a20636ffb620ed0d) )
	ROM_LOAD16_BYTE( "te1verb.2f",   0x0200001, 0x100000, CRC(3dc01aad) SHA1(266f346fa575c42b635bc469798f5aade9821e20) )

	ROM_REGION32_LE( 0x1000000, "user2", 0 ) /* main data */
	ROM_LOAD16_BYTE( "te1rom0l.ic5", 0x0000000, 0x200000, CRC(03786e09) SHA1(de2b9e19ace43c424d100dc5d3207217f66d6479) )
	ROM_LOAD16_BYTE( "te1rom0u.ic6", 0x0000001, 0x200000, CRC(75d91051) SHA1(1c7958162315576c3881dcc684b85710f7f19cd6) )
	ROM_LOAD16_BYTE( "te1rom1l.ic3", 0x0400000, 0x200000, CRC(81416f8e) SHA1(b42ff08ee84491c57a7c87bb767db7e2ec7a26c2) )
	ROM_LOAD16_BYTE( "te1rom1u.ic8", 0x0400001, 0x200000, CRC(fa7ba433) SHA1(e222c4b1631c09e58546446a1e82c16fca936f1d) )
	ROM_LOAD16_BYTE( "te1rom2l.ic4", 0x0800000, 0x200000, CRC(41d77846) SHA1(eeab049135c02a255899fe37e225c1111b2fbb7d) )
	ROM_LOAD16_BYTE( "te1rom2u.ic7", 0x0800001, 0x200000, CRC(a678987e) SHA1(c62c00ce5cf4d001723c999b2bc3dbb90283def1) )

	ROM_REGION16_LE( 0x100000, "c76", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "te1sprog.6d",  0x0000000, 0x040000, CRC(849587e9) SHA1(94c6a757b24758a866a41bd8acd46aa46844f74b) )
	ROM_LOAD( "c76.bin",      0x040000, 0x004000, CRC(399faac7) SHA1(ceb184ef0486caf715dd997101999785f67a40b8) )

	ROM_REGION( 0x0200000, "c352", 0 ) /* samples */
	ROM_LOAD( "te1wave.8k",   0x0000000, 0x200000, CRC(fce6c57a) SHA1(7fb8c69452c92c59a940a2b69d0d73ef7aefcb82) )
ROM_END

ROM_START( tekkenc )
	ROM_REGION32_LE( 0x0400000, "user1", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "te1verb.2l",   0x0000000, 0x100000, CRC(4291afee) SHA1(2e04a6c786544176e2b7e22b5f469e3548896b19) )
	ROM_LOAD16_BYTE( "te1verb.2j",   0x0000001, 0x100000, CRC(5c534705) SHA1(2430b5c36de419822de0283c006c5af2e7cd95ef) )
	ROM_LOAD16_BYTE( "te1verb.2k",   0x0200000, 0x100000, CRC(b9860b29) SHA1(678889fc5c70bf66f0bd9864a20636ffb620ed0d) )
	ROM_LOAD16_BYTE( "te1verb.2f",   0x0200001, 0x100000, CRC(3dc01aad) SHA1(266f346fa575c42b635bc469798f5aade9821e20) )

	ROM_REGION32_LE( 0x1000000, "user2", 0 ) /* main data */
	ROM_LOAD16_BYTE( "te1rom0l.ic5", 0x0000000, 0x200000, CRC(03786e09) SHA1(de2b9e19ace43c424d100dc5d3207217f66d6479) )
	ROM_LOAD16_BYTE( "te1rom0u.ic6", 0x0000001, 0x200000, CRC(75d91051) SHA1(1c7958162315576c3881dcc684b85710f7f19cd6) )
	ROM_LOAD16_BYTE( "te1rom1l.ic3", 0x0400000, 0x200000, CRC(81416f8e) SHA1(b42ff08ee84491c57a7c87bb767db7e2ec7a26c2) )
	ROM_LOAD16_BYTE( "te1rom1u.ic8", 0x0400001, 0x200000, CRC(fa7ba433) SHA1(e222c4b1631c09e58546446a1e82c16fca936f1d) )
	ROM_LOAD16_BYTE( "te1rom2l.ic4", 0x0800000, 0x200000, CRC(41d77846) SHA1(eeab049135c02a255899fe37e225c1111b2fbb7d) )
	ROM_LOAD16_BYTE( "te1rom2u.ic7", 0x0800001, 0x200000, CRC(a678987e) SHA1(c62c00ce5cf4d001723c999b2bc3dbb90283def1) )

	ROM_REGION16_LE( 0x100000, "c76", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "te1sprog.6d",  0x0000000, 0x040000, CRC(849587e9) SHA1(94c6a757b24758a866a41bd8acd46aa46844f74b) )
	ROM_LOAD( "c76.bin",      0x040000, 0x004000, CRC(399faac7) SHA1(ceb184ef0486caf715dd997101999785f67a40b8) )

	ROM_REGION( 0x0200000, "c352", 0 ) /* samples */
	ROM_LOAD( "te1wave.8k",   0x0000000, 0x200000, CRC(fce6c57a) SHA1(7fb8c69452c92c59a940a2b69d0d73ef7aefcb82) )
ROM_END

ROM_START( tekken2 )
	ROM_REGION32_LE( 0x0400000, "user1", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "tes3verb.2l",  0x0000000, 0x100000, CRC(4692075f) SHA1(d048a92040ceb57ef7462bebc2c1112b964570ec) )
	ROM_LOAD16_BYTE( "tes3verb.2j",  0x0000001, 0x100000, CRC(db3ec640) SHA1(fc9f475232ea77abd2eb7e2e09314281264e9d38) )
	ROM_LOAD16_BYTE( "tes1verb.2k",  0x0200000, 0x100000, CRC(668ca712) SHA1(67100db4c6a3ca62d3f62f6fcef974ce017e2c9e) )
	ROM_LOAD16_BYTE( "tes1verb.2f",  0x0200001, 0x100000, CRC(c4f66a0a) SHA1(1b3dd33d7e6d9122826bf8be0dbbc088e4cc41e8) )

	ROM_REGION32_LE( 0x1000000, "user2", 0 ) /* main data */
	ROM_LOAD16_BYTE( "tes1rom0l.ic6", 0x0000000, 0x200000, CRC(fc904ede) SHA1(cea378ba86f94beadb3d67685f1b8c141f478abe) )
	ROM_LOAD16_BYTE( "tes1rom0u.ic5", 0x0000001, 0x200000, CRC(57b38f5d) SHA1(edb4beab47b1339a5b1bc8071086abfcba57722e) )
	ROM_LOAD16_BYTE( "tes1rom1l.ic8", 0x0400000, 0x200000, CRC(aa48f04b) SHA1(f7383d2b3a84c4e649a27c0ad1e6af4702ec0a17) )
	ROM_LOAD16_BYTE( "tes1rom1u.ic3", 0x0400001, 0x200000, CRC(b147c543) SHA1(c4b18c218999ec73d04c92e06fb3e6165ceebf2b) )
	ROM_LOAD16_BYTE( "tes1rom2l.ic7", 0x0800000, 0x200000, CRC(b08da52c) SHA1(31fe2021d0fe37c16555650dd10d26ed80d9b493) )
	ROM_LOAD16_BYTE( "tes1rom2u.ic4", 0x0800001, 0x200000, CRC(8a1561b8) SHA1(ebc02c9e7033d54aefb5034c97a3c8cd749b5600) )
	ROM_LOAD16_BYTE( "tes1rom3l.ic9", 0x0c00000, 0x200000, CRC(d5ac0f18) SHA1(342d063f7974bd1f90b5ca4832dfa4fbc9605453) )
	ROM_LOAD16_BYTE( "tes1rom3u.ic1", 0x0c00001, 0x200000, CRC(44ed509d) SHA1(27e26aaf5ce72ab686f3f05743b1d91b5334b4e0) )

	ROM_REGION16_LE( 0x100000, "c76", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "tes1sprog.6d", 0x0000000, 0x040000, CRC(af18759f) SHA1(aabd7d1384925781d37f860605a5d4622e0fc2e4) )
	ROM_LOAD( "c76.bin",      0x040000, 0x004000, CRC(399faac7) SHA1(ceb184ef0486caf715dd997101999785f67a40b8) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "tes1wave.8k",  0x800000, 0x400000, CRC(34a34eab) SHA1(8e83a579abdcd419dc5cff8aa4c1d7e6c3add773) )
ROM_END

ROM_START( tekken2a )
	ROM_REGION32_LE( 0x0400000, "user1", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "tes2verb.2l",  0x0000000, 0x100000, CRC(aa9a4503) SHA1(e6fdd34216591510593bbda0232ddc0fcd80e80b) )
	ROM_LOAD16_BYTE( "tes2verb.2j",  0x0000001, 0x100000, CRC(63706d8c) SHA1(740cfa8b422fb663bcb412b3389da33c7f9f13be) )
	ROM_LOAD16_BYTE( "tes1verb.2k",  0x0200000, 0x100000, CRC(668ca712) SHA1(67100db4c6a3ca62d3f62f6fcef974ce017e2c9e) )
	ROM_LOAD16_BYTE( "tes1verb.2f",  0x0200001, 0x100000, CRC(c4f66a0a) SHA1(1b3dd33d7e6d9122826bf8be0dbbc088e4cc41e8) )

	ROM_REGION32_LE( 0x1000000, "user2", 0 ) /* main data */
	ROM_LOAD16_BYTE( "tes1rom0l.ic6", 0x0000000, 0x200000, CRC(fc904ede) SHA1(cea378ba86f94beadb3d67685f1b8c141f478abe) )
	ROM_LOAD16_BYTE( "tes1rom0u.ic5", 0x0000001, 0x200000, CRC(57b38f5d) SHA1(edb4beab47b1339a5b1bc8071086abfcba57722e) )
	ROM_LOAD16_BYTE( "tes1rom1l.ic8", 0x0400000, 0x200000, CRC(aa48f04b) SHA1(f7383d2b3a84c4e649a27c0ad1e6af4702ec0a17) )
	ROM_LOAD16_BYTE( "tes1rom1u.ic3", 0x0400001, 0x200000, CRC(b147c543) SHA1(c4b18c218999ec73d04c92e06fb3e6165ceebf2b) )
	ROM_LOAD16_BYTE( "tes1rom2l.ic7", 0x0800000, 0x200000, CRC(b08da52c) SHA1(31fe2021d0fe37c16555650dd10d26ed80d9b493) )
	ROM_LOAD16_BYTE( "tes1rom2u.ic4", 0x0800001, 0x200000, CRC(8a1561b8) SHA1(ebc02c9e7033d54aefb5034c97a3c8cd749b5600) )
	ROM_LOAD16_BYTE( "tes1rom3l.ic9", 0x0c00000, 0x200000, CRC(d5ac0f18) SHA1(342d063f7974bd1f90b5ca4832dfa4fbc9605453) )
	ROM_LOAD16_BYTE( "tes1rom3u.ic1", 0x0c00001, 0x200000, CRC(44ed509d) SHA1(27e26aaf5ce72ab686f3f05743b1d91b5334b4e0) )

	ROM_REGION16_LE( 0x100000, "c76", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "tes1sprog.6d", 0x0000000, 0x040000, CRC(af18759f) SHA1(aabd7d1384925781d37f860605a5d4622e0fc2e4) )
	ROM_LOAD( "c76.bin",      0x040000, 0x004000, CRC(399faac7) SHA1(ceb184ef0486caf715dd997101999785f67a40b8) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "tes1wave.8k",  0x800000, 0x400000, CRC(34a34eab) SHA1(8e83a579abdcd419dc5cff8aa4c1d7e6c3add773) )
ROM_END

ROM_START( tekken2b )
	ROM_REGION32_LE( 0x0400000, "user1", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "tes2vera.2l",  0x0000000, 0x100000, CRC(8bb82bf0) SHA1(ac4e0077dff4c46ea2435903c410590f91cafe7d) )
	ROM_LOAD16_BYTE( "tes2vera.2j",  0x0000001, 0x100000, CRC(4e02f921) SHA1(15339c2626033912947d33e5f59a109e607be0bf) )
	ROM_LOAD16_BYTE( "tes1vera.2k",  0x0200000, 0x100000, CRC(78e2ce1a) SHA1(fb242725dc72fa234bd7df81cec57fe010cf58f3) )
	ROM_LOAD16_BYTE( "tes1vera.2f",  0x0200001, 0x100000, CRC(fbb0b146) SHA1(08b11ac0fbfeed62910c5cb5ff7b5939ecbca142) )

	ROM_REGION32_LE( 0x1000000, "user2", 0 ) /* main data */
	ROM_LOAD16_BYTE( "tes1rom0l.ic6", 0x0000000, 0x200000, CRC(fc904ede) SHA1(cea378ba86f94beadb3d67685f1b8c141f478abe) )
	ROM_LOAD16_BYTE( "tes1rom0u.ic5", 0x0000001, 0x200000, CRC(57b38f5d) SHA1(edb4beab47b1339a5b1bc8071086abfcba57722e) )
	ROM_LOAD16_BYTE( "tes1rom1l.ic8", 0x0400000, 0x200000, CRC(aa48f04b) SHA1(f7383d2b3a84c4e649a27c0ad1e6af4702ec0a17) )
	ROM_LOAD16_BYTE( "tes1rom1u.ic3", 0x0400001, 0x200000, CRC(b147c543) SHA1(c4b18c218999ec73d04c92e06fb3e6165ceebf2b) )
	ROM_LOAD16_BYTE( "tes1rom2l.ic7", 0x0800000, 0x200000, CRC(b08da52c) SHA1(31fe2021d0fe37c16555650dd10d26ed80d9b493) )
	ROM_LOAD16_BYTE( "tes1rom2u.ic4", 0x0800001, 0x200000, CRC(8a1561b8) SHA1(ebc02c9e7033d54aefb5034c97a3c8cd749b5600) )
	ROM_LOAD16_BYTE( "tes1rom3l.ic9", 0x0c00000, 0x200000, CRC(d5ac0f18) SHA1(342d063f7974bd1f90b5ca4832dfa4fbc9605453) )
	ROM_LOAD16_BYTE( "tes1rom3u.ic1", 0x0c00001, 0x200000, CRC(44ed509d) SHA1(27e26aaf5ce72ab686f3f05743b1d91b5334b4e0) )

	ROM_REGION16_LE( 0x100000, "c76", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "tes1sprog.6d", 0x0000000, 0x040000, CRC(af18759f) SHA1(aabd7d1384925781d37f860605a5d4622e0fc2e4) )
	ROM_LOAD( "c76.bin",      0x040000, 0x004000, CRC(399faac7) SHA1(ceb184ef0486caf715dd997101999785f67a40b8) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "tes1wave.8k",  0x800000, 0x400000, CRC(34a34eab) SHA1(8e83a579abdcd419dc5cff8aa4c1d7e6c3add773) )
ROM_END

ROM_START( xevi3dg )
	ROM_REGION32_LE( 0x0400000, "user1", 0 ) /* main prg */
	ROM_LOAD16_BYTE( "xv31vera.2l",  0x0000000, 0x100000, CRC(419e0f13) SHA1(876ae81c9fde18cdec792f3dfee682761ac99730) )
	ROM_LOAD16_BYTE( "xv31vera.2j",  0x0000001, 0x100000, CRC(df95373a) SHA1(65cf85ad4dcbc8f0376ac46ee0c1037d79099843) )
	ROM_LOAD16_BYTE( "xv31vera.2k",  0x0200000, 0x100000, CRC(3d58138e) SHA1(9203d6bdc2d968de818d5f465523cc030217dcf8) )
	ROM_LOAD16_BYTE( "xv31vera.2f",  0x0200001, 0x100000, CRC(9e8780a2) SHA1(83148d55456b2b92969f7ac2bdb2d492bf969895) )

	ROM_REGION32_LE( 0x1000000, "user2", 0 ) /* main data */
	ROM_LOAD16_BYTE( "xv31rom0l.ic5", 0x0000000, 0x200000, CRC(24e1e262) SHA1(84df49b22a8a36284da771944a8390672a0c32bf) )
	ROM_LOAD16_BYTE( "xv31rom0u.ic6", 0x0000001, 0x200000, CRC(cae38ef3) SHA1(2dfe0b31969091975e8d8c8188ce7dd007e4a0f3) )
	ROM_LOAD16_BYTE( "xv31rom1l.ic3", 0x0400000, 0x200000, CRC(46b4cb72) SHA1(c3360c4fdb71ffcbccec3f4ad8d7963b08822e26) )
	ROM_LOAD16_BYTE( "xv31rom1u.ic8", 0x0400001, 0x200000, CRC(be0eb5d1) SHA1(f1a0adcb7c65bbba723fe09b07280b0b924d6b19) )
	ROM_LOAD16_BYTE( "xv31rom2l.ic4", 0x0800000, 0x200000, CRC(8403a277) SHA1(35193211351494a086d8422e3b0b71a8d3a262a6) )
	ROM_LOAD16_BYTE( "xv31rom2u.ic7", 0x0800001, 0x200000, CRC(ecf70432) SHA1(bec128a215e0aef66e9a8707bb0d1eb7b098a356) )

	ROM_REGION16_LE( 0x100000, "c76", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "xv31sprog.6d", 0x0000000, 0x040000, CRC(e50b856a) SHA1(631da4f60c9ce08387fca26a70481a2fdacf9765) )
	ROM_LOAD( "c76.bin",      0x040000, 0x004000, CRC(399faac7) SHA1(ceb184ef0486caf715dd997101999785f67a40b8) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* samples */
	ROM_LOAD( "xv31wave.8k",  0x000000, 0x400000, CRC(14f25ddd) SHA1(4981cf1017432ff85b768ec88c36f535df30b783) )
	ROM_RELOAD( 0x800000, 0x400000 )
ROM_END

GAME( 1994, tekken,    0,        coh100, tekken,   namcos11, ROT0, "Namco", "Tekken (TE4/VER.C)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1994, tekkena,   tekken,   coh100, tekken,   namcos11, ROT0, "Namco", "Tekken (TE2/VER.C)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1994, tekkenb,   tekken,   coh100, tekken,   namcos11, ROT0, "Namco", "Tekken (TE2/VER.B)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1994, tekkenc,   tekken,   coh100, tekken,   namcos11, ROT0, "Namco", "Tekken (Japan, TE1/VER.B)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1995, tekken2,   0,        coh100, tekken,   namcos11, ROT0, "Namco", "Tekken 2 Ver.B (TES3/VER.B)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1995, tekken2a,  tekken2,  coh100, tekken,   namcos11, ROT0, "Namco", "Tekken 2 Ver.B (TES2/VER.B)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1995, tekken2b,  tekken2,  coh100, tekken,   namcos11, ROT0, "Namco", "Tekken 2 (TES2/VER.A)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1996, souledge,  0,        coh110, souledge, namcos11, ROT0, "Namco", "Soul Edge Ver. II (SO4/VER.C)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1995, souledgea, souledge, coh110, souledge, namcos11, ROT0, "Namco", "Soul Edge Ver. II (SO3/VER.C)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND )
GAME( 1995, souledge1, souledge, coh110, souledge, namcos11, ROT0, "Namco", "Soul Edge (SO3/VER.A)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND )
GAME( 1995, souledge1j,souledge, coh110, souledge, namcos11, ROT0, "Namco", "Soul Edge (Japan, SO1/VER.A)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND )
GAME( 1995, dunkmnia,  0,        coh110, namcos11, namcos11, ROT0, "Namco", "Dunk Mania (DM2/VER.C)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1995, dunkmniaj, dunkmnia, coh110, namcos11, namcos11, ROT0, "Namco", "Dunk Mania (Japan, DM1/VER.C)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1995, xevi3dg,   0,        coh110, namcos11, namcos11, ROT0, "Namco", "Xevious 3D/G (Japan, XV31/VER.A)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1996, primglex,  0,        coh110, tekken,   namcos11, ROT0, "Namco", "Prime Goal EX (Japan, PG1/VER.A)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1996, danceyes,  0,        coh110, namcos11, namcos11, ROT0, "Namco", "Dancing Eyes (Japan, DC1/VER.A)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1996, pocketrc,  0,        coh110, pocketrc, namcos11, ROT0, "Namco", "Pocket Racer (Japan, PKR1/VER.B)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1997, starswep,  0,        coh110, namcos11, namcos11, ROT0, "Axela/Namco", "Star Sweep (Japan, STP1/VER.A)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, myangel3,  0,        coh110, myangel3, namcos11, ROT0, "Namco", "Kosodate Quiz My Angel 3 (Japan, KQT1/VER.A)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, ptblnk2a,  ptblank2, coh110, ptblnk2a, namcos11, ROT0, "Namco", "Point Blank 2 (GNB3/VER.A)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
