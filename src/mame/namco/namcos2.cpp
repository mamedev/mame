// license:BSD-3-Clause
// copyright-holders:K.Wilkins
/***************************************************************************

Namco System II driver by K.Wilkins  (Jun1998, Oct1999)
Email: mame@esplexo.co.uk

How to calibrate the guns:
  - hold 9, press f2
  - shoot at the targets it shows using the crosshair
  - after you've shot the target press 9 again to move onto the next one
  - once you get back to the start again press f2 to exit.

How to calibrate the steering in Dirt Fox and Final Lap 2/3:
  - hold 9, press f2
  - verify that controls are set to zero then press f2 to exit.

Final Lap Notes:
    1..4 cabinets may be wired together.
    To move through self test options, press gas pedal and change gear shift from low to high
    To change an option, move gear shift from low to high without touching the gas pedal

TODO:
    - Verify below still occur

    General
    - sprite/tilemap orthogonality needed

    Final Lap & Final Lap 2:
    - by default, the graphics are way too bright if compared to original PCB/monitor output (just an overall gamma issue?)

    Finest Hour:
    - roz plane colors are bad in-game

    Final Lap 3:
    - uses unaligned 32x32 sprites, which aren't handled correctly in video/namcos2.cpp yet

    Suzuka 8 Hours II
    - some sprite cropping issues

    Valkyrie no Densetsu
    - gives ADSMISS error on startup
       Does a checksum on area 0x181000 - 0x183fff, in 0x20 bytes block chunks. Game doesn't init it properly so you either have to go into service menu and do
       an "all data clear" or play once to get rid of the message.

    Metal Hawk
    - ROZ wraparound isn't implemented (see large battleship in 2nd stage)

    Burning Force (+ maybe others)
    - POSIRQ is off-by-one, but adjusting it makes other cases worse
      (because some layers are line-buffered and some aren't, and we need proper scroll/data latch times for each layer type?)

The Namco System II board is a 5 ( only 4 are emulated ) CPU system. The
complete system consists of two boards: CPU + GRAPHICS. It contains a large
number of custom ASICs to perform graphics operations, there is no
documentation available for these parts.

The system is extremely powerful and flexible. A standard CPU board is coupled
with a number of different graphics boards to produce a system.



CPU Board details
=================

CPU BOARD - Master/Slave CPU, Sound CPU, I/O CPU, Serial I/F CPU
            Text/Scrolling layer generation and video pixel generator.
            Sound Generation.

CPU1 - Master CPU               (68K)
CPU2 - Slave CPU                (68K)
CPU3 - Sound/IO engine          (6809)
CPU4 - IO Microcontroller       (63705) Dips/input ports
CPU5 - Serial I/F Controller    (??? - Not emulated)

The 4 CPU's are all connected via a central 2KByte dual port SRAM. The two
68000s are on one side and the 6809/63705 are on the other side.

Each 68000 has its own private bus area AND a common shared area between the
two devices, which is where the video ram/dual port/Sprite Generation etc
logic sits.

So far only 1 CPU board variant has been identified, unlike the GFX board...

All sound circuitry is contained on the CPU board, it consists of:
    YM2151
    C140 (24 Channel Stereo PCM Sample player)

The CPU board also contains the frame timing and video image generation
circuitry along with text/scroll planes and the palette generator. The system
has 8192 pens of which 4096+2048 are displayable at any given time. These
pens reference a 24 bit colour lookup table (R8:G8:B8).

The text/tile plane generator has the following capabilities:

2 x Static tile planes (36x28 tiles)
4 x Scrolling tile planes (64x64 tiles)

Each plane has its own colour index (8 total) that is used alongside the
pen number to be looked up in the pen index and generate a 24 bit pixel. Each
plane also has its own priority level.

The video image generator receives a pixel stream from the graphics board
which contains:

        PEN NUMBER
        COLOUR BANK
        PIXEL PRIORITY

This stream is then combined with the stream from the text plane pixel
generator with the highest priority pixel being displayed on screen.


Graphics Board details
======================

There are several variants of graphics board with unique capabilities
separate memory map definition. The PCB outputs a pixel stream to the
main PCB board via one of the system connectors.


ROZ(A):    1 256x256 ROZ plane composed of 8x8 tiles
ROZ(B):    2 ROZ planes, composed of 16x16 tiles (same as Namco NB2)
Sprite(A): 128 Sprites displayable, but 16 banks of 128 sprites
Sprite(B): (same as Namco NB2)
Roadway:   tiles and road attributes in RAM

                          ROZ  Sprites  Roadway
Standard Namco System 2   (A)  (A)      n/a
Final Lap (1/2/3)         n/a  (A)      yes
Metal Hawk                (B)  (A)      no
Steel Gunner 2            n/a  (B)      no
Suzuka (1/2)              n/a  (B)      yes
Lucky&Wild                (B)  (B)      yes


Memory Map
==========

The Dual 68000 Shared memory map area is shown below, this is taken from the memory
decoding pal from the Cosmo Gang board.


#############################################################
#                                                           #
#       MASTER 68000 PRIVATE MEMORY AREA (MAIN PCB)         #
#                                                           #
#############################################################
# Function                         Address        R/W  DATA #
#############################################################
Program ROM                        000000-03FFFF  R    D00-D15

Program RAM                        100000-10FFFF  R/W  D00-D15

EEPROM                             180000-183FFF  R/W  D00-D07

Interrupt Controller C148          1C0000-1FFFFF  R/W  D00-D02
    ????????                       1C0XXX
    ????????                       1C2XXX
    ????????                       1C4XXX
    Master/Slave IRQ level         1C6XXX              D00-D02
    EXIRQ level                    1C8XXX              D00-D02
    POSIRQ level                   1CAXXX              D00-D02
    SCIRQ level                    1CCXXX              D00-D02
    VBLANK IRQ level               1CEXXX              D00-D02
    ????????                       1D0XXX
    ????????                       1D4000 trigger master/slave INT?

    Acknowledge Master/Slave IRQ   1D6XXX ack master/slave INT
    Acknowledge EXIRQ              1D8XXX
    Acknowledge POSIRQ             1DAXXX
    Acknowledge SCIRQ              1DCXXX
    Acknowledge VBLANK IRQ         1DEXXX

    EEPROM Ready status            1E0XXX         R    D01
    Sound CPU Reset control        1E2XXX           W  D01
    Slave 68000 & IO CPU Reset     1E4XXX           W  D01
    Watchdog reset kicker          1E6XXX           W



#############################################################
#                                                           #
#        SLAVE 68000 PRIVATE MEMORY AREA (MAIN PCB)         #
#                                                           #
#############################################################
# Function                         Address        R/W  DATA #
#############################################################
Program ROM                        000000-03FFFF  R    D00-D15

Program RAM                        100000-10FFFF  R/W  D00-D15

Interrupt Controller C148          1C0000-1FFFFF  R/W  D00-D02
    ????????                       1C0XXX
    ????????                       1C2XXX
    ????????                       1C4XXX
    Master/Slave IRQ level         1C6XXX              D00-D02
    EXIRQ level                    1C8XXX              D00-D02
    POSIRQ level                   1CAXXX              D00-D02
    SCIRQ level                    1CCXXX              D00-D02
    VBLANK IRQ level               1CEXXX              D00-D02
    ????????                       1D0XXX
    Acknowledge Master/Slave IRQ   1D6XXX
    Acknowledge EXIRQ              1D8XXX
    Acknowledge POSIRQ             1DAXXX
    Acknowledge SCIRQ              1DCXXX
    Acknowledge VBLANK IRQ         1DEXXX
    Watchdog reset kicker          1E6XXX           W




#############################################################
#                                                           #
#           SHARED 68000 MEMORY AREA (MAIN PCB)             #
#                                                           #
#############################################################
# Function                         Address        R/W  DATA #
#############################################################
Data ROMS 0-1                      200000-2FFFFF  R    D00-D15

Data ROMS 2-3                      300000-3FFFFF  R    D00-D15

Screen memory for text planes      400000-41FFFF  R/W  D00-D15

Screen control registers           420000-43FFFF  R/W  D00-D15

    Scroll plane 0 - X offset      42XX02           W  D00-D11
    Scroll plane 0 - X flip        42XX02           W  D15

    ??????                         42XX04           W  D14-D15

    Scroll plane 0 - Y offset      42XX06           W  D00-D11
    Scroll plane 0 - Y flip        42XX06           W  D15

    ??????                         42XX08           W  D14-D15

    Scroll plane 1 - X offset      42XX0A           W  D00-D11
    Scroll plane 1 - X flip        42XX0A           W  D15

    ??????                         42XX0C           W  D14-D15

    Scroll plane 1 - Y offset      42XX0E           W  D00-D11
    Scroll plane 1 - Y flip        42XX0E           W  D15

    ??????                         42XX10           W  D14-D15

    Scroll plane 2 - X offset      42XX12           W  D00-D11
    Scroll plane 2 - X flip        42XX12           W  D15

    ??????                         42XX14           W  D14-D15

    Scroll plane 2 - Y offset      42XX16           W  D00-D11
    Scroll plane 2 - Y flip        42XX16           W  D15

    ??????                         42XX18           W  D14-D15

    Scroll plane 3 - X offset      42XX1A           W  D00-D11
    Scroll plane 3 - X flip        42XX1A           W  D15

    ??????                         42XX1C           W  D14-D15

    Scroll plane 3 - Y offset      42XX1E           W  D00-D11
    Scroll plane 3 - Y flip        42XX1E           W  D15

    Scroll plane 0 priority        42XX20           W  D00-D02
    Scroll plane 1 priority        42XX22           W  D00-D02
    Scroll plane 2 priority        42XX24           W  D00-D02
    Scroll plane 3 priority        42XX26           W  D00-D02
    Text plane 0 priority          42XX28           W  D00-D02
    Text plane 1 priority          42XX2A           W  D00-D02

    Scroll plane 0 colour          42XX30           W  D00-D03
    Scroll plane 1 colour          42XX32           W  D00-D03
    Scroll plane 2 colour          42XX34           W  D00-D03
    Scroll plane 3 colour          42XX36           W  D00-D03
    Text plane 0 colour            42XX38           W  D00-D03
    Text plane 1 colour            42XX3A           W  D00-D03

Screen palette control/data        440000-45FFFF  R/W  D00-D15
    RED   ROZ/Sprite pens 8x256    440000-440FFF
    GREEN                          441000-441FFF
    BLUE                           442000-442FFF
    Control registers              443000-44300F  R/W  D00-D15
    RED   ROZ/Sprite pens 8x256    444000-444FFF
    GREEN                          445000-445FFF
    BLUE                           446000-446FFF
                                                                   447000-447FFF
    RED   Text plane pens 8x256    448000-448FFF
    GREEN                          449000-449FFF
    BLUE                           44A000-44AFFF
                                                                   44B000-44BFFF
    RED   Unused pens 8x256        44C000-44CFFF
    GREEN                          44D000-44DFFF
    BLUE                           44E000-44EFFF

Dual port memory                   460000-47FFFF  R/W  D00-D07

Serial comms processor             480000-49FFFF

Serial comms processor - Data      4A0000-4BFFFF



#############################################################
#                                                           #
#           SHARED 68000 MEMORY AREA (GFX PCB)              #
#             (STANDARD NAMCO SYSTEM 2 BOARD)               #
#                                                           #
#############################################################
# Function                         Address        R/W  DATA #
#############################################################
Sprite RAM - 16 banks x 128 spr.   C00000-C03FFF  R/W  D00-D15

Sprite bank select                 C40000           W  D00-D03
Rotate colour bank select                           W  D08-D11
Rotate priority level                               W  D12-D14

Rotate/Zoom RAM (ROZ)              C80000-CBFFFF  R/W  D00-D15

Rotate/Zoom - Down dy     (8:8)    CC0000         R/W  D00-D15
Rotate/Zoom - Right dy    (8.8)    CC0002         R/W  D00-D15
Rotate/Zoom - Down dx     (8.8)    CC0004         R/W  D00-D15
Rotate/Zoom - Right dx    (8.8)    CC0006         R/W  D00-D15
Rotate/Zoom - Start Ypos  (12.4)   CC0008         R/W  D00-D15
Rotate/Zoom - Start Xpos  (12.4)   CC000A         R/W  D00-D15
Rotate/Zoom control                CC000E         R/W  D00-D15

Key generator/Security device      D00000-D0000F  R/W  D00-D15



#############################################################
#                                                           #
#           SHARED 68000 MEMORY AREA (GFX PCB)              #
#           (METAL HAWK PCB - DUAL ROZ PLANES)              #
#                                                           #
#############################################################
# Function                         Address        R/W  DATA #
#############################################################
Sprite RAM - 16 banks x 128 spr.   C00000-C03FFF  R/W  D00-D15

Rotate/Zoom RAM (ROZ1)             C40000-C47FFF  R/W  D00-D15

Rotate/Zoom RAM (ROZ2)             C48000-C4FFFF  R/W  D00-D15

Rotate/Zoom1 - Down dy     (8:8)   D00000         R/W  D00-D15
Rotate/Zoom1 - Right dy    (8.8)   D00002         R/W  D00-D15
Rotate/Zoom1 - Down dx     (8.8)   D00004         R/W  D00-D15
Rotate/Zoom1 - Right dx    (8.8)   D00006         R/W  D00-D15
Rotate/Zoom1 - Start Ypos  (12.4)  D00008         R/W  D00-D15
Rotate/Zoom1 - Start Xpos  (12.4)  D0000A         R/W  D00-D15
Rotate/Zoom1 - control             D0000E         R/W  D00-D15

Rotate/Zoom2 - Down dy     (8:8)   D00010         R/W  D00-D15
Rotate/Zoom2 - Right dy    (8.8)   D00012         R/W  D00-D15
Rotate/Zoom2 - Down dx     (8.8)   D00014         R/W  D00-D15
Rotate/Zoom2 - Right dx    (8.8)   D00016         R/W  D00-D15
Rotate/Zoom2 - Start Ypos  (12.4)  D00018         R/W  D00-D15
Rotate/Zoom2 - Start Xpos  (12.4)  D0001A         R/W  D00-D15
Rotate/Zoom2 - control             D0001E         R/W  D00-D15

Sprite bank select ?               E00000           W  D00-D15


#############################################################
#                                                           #
#           SHARED 68000 MEMORY AREA (GFX PCB)              #
#           (FINAL LAP PCB)                                 #
#                                                           #
#############################################################
# Function                         Address        R/W  DATA #
#############################################################
Sprite RAM - ?? banks x ??? spr.   800000-80FFFF  R/W  D00-D15
Sprite bank select ?               840000           W  D00-D15
Road RAM for tile layout           880000-88FFFF  R/W  D00-D15
Road RAM for tiles gfx data        890000-897FFF  R/W  D00-D15
Road Generator controls            89F000-89FFFF  R/W  D00-D15
Key generator/Security device      A00000-A0000F  R/W  D00-D15



All interrupt handling is done on the 68000s by two identical custom devices (C148),
this device takes the level based signals and encodes them into the 3 bit encoded
form for the 68000 CPU. The master CPU C148 also controls the reset for the slave
CPU and MCU which are common. The C148 only has the lower 3 data bits connected.

C148 Features
-------------
3 Bit output port
3 Bit input port
3 Chip selects
68000 Interrupt encoding/handling
Data strobe control
Bus arbitration
Reset output
Watchdog


C148pin     Master CPU      Slave CPU
-------------------------------------
YBNK        VBLANK          VBLANK
IRQ4        SCIRQ           SCIRQ       (Serial comms IC Interrupt)
IRQ3        POSIRQ          POSIRQ      (Comes from C116, pixel generator, line based position interrupt?)
IRQ2        EXIRQ           EXIRQ       (Goes to video board but does not appear to be connected)
IRQ1        SCPUIRQ         MCPUIRQ     (Master/Slave interrupts)

OP0         SSRES                       (Sound CPU reset - 6809 only)
OP1
OP2

IP0         EEPROM BUSY
IP1
IP2



Protection
----------
The Chip at $d00000 seems to be heavily involved in protection, some games lock
or reset if it doesn't return the correct values.
rthun2 is sprinkled with reads to $d00006 which look like they are being used as
random numbers. rthun2 also checks the response value after a number is written.
Device takes clock and vblank. Only output is reset.

This chip is based on the graphics board.


Palette
-------

0x800 (2048) colours

Ram test does:

$440000-$442fff     Object ???
$444000-$446fff     Char   ???
$448000-$44afff     Roz    ???
$44c000-$44efff

$448000-$4487ff     Red??
$448800-$448fff     Green??
$449000-$4497ff     Blue??


Steel Gunner 2
--------------
Again this board has a different graphics layout, also the protection checks
are done at $a00000 as opposed to $d00000 on a standard board. Similar
$a00000 checks have been seen on the Final Lap boards.


Custom Chips Notes (moved to here from Stroff's old namcoic.c)
==================

System 21 here presumably refers to the Winning Run PCB, not the later games?

Custom Chips:                       Final Lap   Assault     LuckyWld    System21    NA1/2       NB1/2
    C45     Land Generator            *                       *
    C65     I/O Controller (older)    *           *
    C67     TMS320C25 (DSP int ROM)
    C68     I/O Controller (newer)                            *           *
    C70                                                                               *
    C95                               *           *
    C102    ROZ:Memory Access Control             *
    C106    OBJ:X-Axis Zoom Control   *           *
    C107    Land Line Buffer          *
    C116    Screen Waveform Generator *           *           *                                   *
    C121    Glue logic for the 6809   *           *           *
    C123    GFX:Tile Mem Decoder      *           *           *                                   *
    C134    OBJ:Address Generator     *           *
    C135    OBJ:Line matching         *           *
    C137    Clock Generator IC        *           *           *           *                       *
    C138                                                                  *
    C139    Serial I/F Controller     *           *           *           *
    C140    24 Channel PCM            *           *           *
    C145    GFX:Tile Memory Access    *           *           *                                   *
    C146    OBJ:Line Buf Steering     *           *
    C148    CPU Bus Manager           *           *           *           *
    C149    Mouse/Trackball Decoder   *           *           *           *
    C156    Pixel Stream Combo        *           *           *                                   *
    C160    Control                                                                               *
    C165                                                                  *
    C169    ROZ(B)                                            *                                   *
    C187                                                      *           *                       *
    C210                                                                              *
    C215                                                                              *
    C218                                                                              *
    C219                                                                              *
    C329    CPU?                                                                                  *
    C347    GfxObj                                                                                *
    C352    PCM                                                                                   *
    C355    Motion Obj(B)                                     *           *                       *
    C373    LAND-related                                      *
    C382                                                                                          *
    C383                                                                                          *
    C384    GFX(3)                                                                                *
    C385                                                                                          *
    C390    Key Custom                                                                            *


General Support
---------------
C65  - This is the I/O Microcontroller, handles all input/output devices. 63705 uC, CPU4 in Namco System2.
C137 - Takes System clock and generates all sub-system clocks, doesn't need emulation, not accessed via CPU
C139 - Serial Interface Controller
C148 - Does some Memory Decode, Interrupt Handling, 3 bit PIO port, Bus Controller
C149 - Does decoding of mouse/trackball input streams for the I/O Controller. (Offset Square wave)


Tile Fields Static/Scrolled
---------------------------
Combination of these two devices and associated RAM & TileGFX produces a pixel stream that is fed
into the Pixel stream decoder.

C145 - Tile Screen Memory Access controller
C123 - Tile Memory decoder Part 1, converts X,Y,Tile into character ROM address index


Pixel Stream Decode
-------------------
These two devices take the pixel streams from the tilefield generator and the associated graphics board
and combine them to form an RGB data stream that is fed to the monitor.

C156 - Pixel stream combiner
Takes tile field & graphics board streams and generates the prioritized pixel, then does the lookup to
go from palettised to 24bit RGB pixel.

C116 - Screen Waveform Generator
Takes RGB24 pixel stream from C156 and generates the waveform signals for the monitor, also generates
the line interrupt and controls screen blanking,shift, etc.

Object Control
--------------
C106 - Generates memory output clocks to generate X-Axis Zoom for Line Buffer Writes
C134 - Object Memory Address Generator. Sequences the sprite memory contents to the hardware.
C135 - Checks is object is displayed on Current output line.
C146 - Steers the Decode Object Pixel data to the correct line buffer A or B

ROZ
---
C102 - Controls CPU access to ROZ Memory Area.

***************************************************************************/

#include "emu.h"
#include "namcos2.h"

#include "cpu/m68000/m68000.h"
#include "cpu/m6805/m6805.h"
#include "cpu/m6809/m6809.h"
#include "machine/nvram.h"
#include "sound/ymopm.h"
#include "speaker.h"

#include "finallap.lh"

/* Define clocks based on actual OSC on the PCB */

#define MAIN_OSC_CLOCK      XTAL(49'152'000)
#define M68K_CPU_CLOCK      (MAIN_OSC_CLOCK / 4)        /* 12.288MHz clock for 68000 (Master & Slave) */
#define M68B09_CPU_CLOCK    (MAIN_OSC_CLOCK / 24)       /* 2.048MHz clock for 68B09 sound CPU */
#define C65_CPU_CLOCK       (MAIN_OSC_CLOCK / 24)       /* 2.048MHz clock for 63705 (or 63B05) I/O CPU */
#define C68_CPU_CLOCK       (MAIN_OSC_CLOCK / 6)        /* 8.192MHz clock for 37450 I/O CPU */
#define YM2151_SOUND_CLOCK  XTAL(3'579'545)            /* 3.579545MHz FM clock */
#define C140_SOUND_CLOCK    (MAIN_OSC_CLOCK / 384 / 6)  /* 21.333kHz C140 clock (was 8000000/374 or 21.390kHz) */


/*************************************************************/
/* 68000/6809/63705 Shared memory area - DUAL PORT Memory    */
/*************************************************************/

uint16_t namcos2_state::dpram_word_r(offs_t offset)
{
	return m_dpram[offset];
}

void namcos2_state::dpram_word_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if( ACCESSING_BITS_0_7 )
	{
		m_dpram[offset] = data & 0xff;

		/* Note:  Outputs for the other gun games pass through here as well, but I couldn't find the offsets. */
		/* Steel Gunner 1 & 2 have 6 "damage lamps" (three on each side) as well as gun recoils. */
	}
}

void gollygho_state::dpram_word_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if( ACCESSING_BITS_0_7 )
	{
		m_dpram[offset] = data & 0xff;

		// TODO : This is a hack! should be output ports MCU side, not probing into DPRAM content
		switch( offset )
		{
			case 0xc0/2:
			{
				int on = BIT(data, 7);
				// output diorama0-5
				/*
				Golly Ghost:
				0 = toybox
				1 = bathroom
				2 = bureau
				3 = refrigerator
				4 = porch
				5 = backlight

				Bubble Trouble:
				0 = shell
				1 = trapdoor
				2 = ship
				3 = chest
				4 = unused?
				5 = backlight
				*/

				for (int i = 0; i < 5; i++)
					m_out_diorama[i] = on & BIT(data, i);
				m_out_diorama[5] = on;

				// output gun recoil
				m_out_gun_recoil[0] = on & BIT(data, 5);
				m_out_gun_recoil[1] = on & BIT(data, 6);

				break;
			}

			case 0xc2/2:
				// unknown; 0x00 or 0x01 - probably lights up guns
				break;

			case 0xc4/2: case 0xc6/2: case 0xc8/2: case 0xca/2:
			{
				// output 7segs
				// 6/9 have no roof/tail, so presume 7448
				static const uint8_t ls48_map[0x10] =
					{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 };

				int group = (offset * 2) - 0xc4;
				m_out_digit[group | 0] = ls48_map[data >> 4 & 0xf];
				m_out_digit[group | 1] = ls48_map[data & 0xf];
				break;
			}

			default:
				break;
		}
	}
}


uint8_t namcos2_state::dpram_byte_r(offs_t offset)
{
	return m_dpram[offset];
}

void namcos2_state::dpram_byte_w(offs_t offset, uint8_t data)
{
	m_dpram[offset] = data;
}

/*************************************************************/
/* SHARED 68000 CPU Memory declarations                      */
/*************************************************************/

/*  ROM0   = $200000-$2fffff
    ROM1   = $300000-$3fffff
    SCR    = $400000-$41ffff
    SCRDT  = $420000-$43ffff
    PALET  = $440000-$45ffff
    DPCS   = $460000-$47ffff
    SCOM   = $480000-$49ffff
    SCOMDT = $4a0000-$4bffff

    0xc00000 ONWARDS are unverified memory locations on the video board
*/

void namcos2_state::namcos2_68k_default_cpu_board_am(address_map &map)
{
	map(0x200000, 0x3fffff).rom().region("data_rom", 0);
	map(0x400000, 0x40ffff).mirror(0x010000).rw(m_c123tmap, FUNC(namco_c123tmap_device::videoram16_r), FUNC(namco_c123tmap_device::videoram16_w));
	map(0x420000, 0x42003f).rw(m_c123tmap, FUNC(namco_c123tmap_device::control16_r), FUNC(namco_c123tmap_device::control16_w));
	map(0x440000, 0x44ffff).r(FUNC(namcos2_state::c116_r)).w(m_c116, FUNC(namco_c116_device::write)).umask16(0x00ff).cswidth(16);
	map(0x460000, 0x460fff).mirror(0x00f000).rw(FUNC(namcos2_state::dpram_word_r), FUNC(namcos2_state::dpram_word_w));
	map(0x480000, 0x483fff).rw(m_sci, FUNC(namco_c139_device::ram_r), FUNC(namco_c139_device::ram_w));
	map(0x4a0000, 0x4a000f).m(m_sci, FUNC(namco_c139_device::regs_map));
}

/*************************************************************/

void namcos2_state::common_default_am(address_map &map)
{
	namcos2_68k_default_cpu_board_am(map);
	map(0xc00000, 0xc03fff).ram().share("spriteram");
	map(0xc40000, 0xc40001).rw(FUNC(namcos2_state::gfx_ctrl_r), FUNC(namcos2_state::gfx_ctrl_w));
	map(0xc80000, 0xc9ffff).ram().w(m_ns2roz, FUNC(namcos2_roz_device::rozram_word_w)).share("rozram");
	map(0xcc0000, 0xcc000f).ram().share("rozctrl");
	map(0xd00000, 0xd0000f).rw(FUNC(namcos2_state::namcos2_68k_key_r), FUNC(namcos2_state::namcos2_68k_key_w));
}

void namcos2_state::master_common_am(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x180000, 0x183fff).rw(FUNC(namcos2_state::eeprom_r), FUNC(namcos2_state::eeprom_w)).umask16(0x00ff);
	map(0x1c0000, 0x1fffff).m(m_master_intc, FUNC(namco_c148_device::map));
}

void namcos2_state::slave_common_am(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x13ffff).ram();
	map(0x1c0000, 0x1fffff).m(m_slave_intc, FUNC(namco_c148_device::map));
}

void namcos2_state::master_default_am(address_map &map)
{
	common_default_am(map);
	master_common_am(map);
}

void namcos2_state::slave_default_am(address_map &map)
{
	common_default_am(map);
	slave_common_am(map);
}


/*************************************************************/

void namcos2_state::common_finallap_am(address_map &map)
{
	namcos2_68k_default_cpu_board_am(map);
	map(0x300000, 0x33ffff).r(FUNC(namcos2_state::namcos2_finallap_prot_r));
	map(0x800000, 0x80ffff).ram().share("spriteram");
	map(0x840000, 0x840001).rw(FUNC(namcos2_state::gfx_ctrl_r), FUNC(namcos2_state::gfx_ctrl_w));
	map(0x880000, 0x89ffff).rw(m_c45_road, FUNC(namco_c45_road_device::read), FUNC(namco_c45_road_device::write));
	map(0x8c0000, 0x8c0001).nopw();
}

void namcos2_state::master_finallap_am(address_map &map)
{
	common_finallap_am(map);
	master_common_am(map);
}

void namcos2_state::slave_finallap_am(address_map &map)
{
	common_finallap_am(map);
	slave_common_am(map);
}

/*************************************************************/

void namcos2_state::common_sgunner_am(address_map &map)
{
	namcos2_68k_default_cpu_board_am(map);
	map(0x800000, 0x8141ff).rw(m_c355spr, FUNC(namco_c355spr_device::spriteram_r), FUNC(namco_c355spr_device::spriteram_w));
	map(0x818000, 0x818001).nopw();
	map(0xa00000, 0xa0000f).rw(FUNC(namcos2_state::namcos2_68k_key_r), FUNC(namcos2_state::namcos2_68k_key_w));
}

void namcos2_state::master_sgunner_am(address_map &map)
{
	common_sgunner_am(map);
	master_common_am(map);
}

void namcos2_state::slave_sgunner_am(address_map &map)
{
	common_sgunner_am(map);
	slave_common_am(map);
}

/*************************************************************/

void namcos2_state::common_metlhawk_am(address_map &map)
{
	namcos2_68k_default_cpu_board_am(map);
	map(0xc00000, 0xc03fff).ram().share("spriteram");
	map(0xc40000, 0xc4ffff).rw(m_c169roz, FUNC(namco_c169roz_device::videoram_r), FUNC(namco_c169roz_device::videoram_w));
	map(0xd00000, 0xd0001f).rw(m_c169roz, FUNC(namco_c169roz_device::control_r), FUNC(namco_c169roz_device::control_w));
	map(0xe00000, 0xe00001).rw(FUNC(namcos2_state::gfx_ctrl_r), FUNC(namcos2_state::gfx_ctrl_w)); /* ??? */
}

void namcos2_state::master_metlhawk_am(address_map &map)
{
	common_metlhawk_am(map);
	master_common_am(map);
}

void namcos2_state::slave_metlhawk_am(address_map &map)
{
	common_metlhawk_am(map);
	slave_common_am(map);
}

/*************************************************************/

void namcos2_state::common_suzuka8h_am(address_map &map)
{
	namcos2_68k_default_cpu_board_am(map);
	map(0x800000, 0x8141ff).rw(m_c355spr, FUNC(namco_c355spr_device::spriteram_r), FUNC(namco_c355spr_device::spriteram_w));
	map(0x818000, 0x818001).noprw(); /* enable? */
	map(0x81a000, 0x81a001).nopw(); /* enable? - or maybe sprite DMA / buffering which is currently done automatically by setting m_c355spr->set_buffer(1); */
	map(0x840000, 0x840001).nopr();
	map(0x900000, 0x900007).rw(m_c355spr, FUNC(namco_c355spr_device::position_r), FUNC(namco_c355spr_device::position_w));
	map(0xa00000, 0xa1ffff).rw(m_c45_road, FUNC(namco_c45_road_device::read), FUNC(namco_c45_road_device::write));
	map(0xf00000, 0xf00007).rw(FUNC(namcos2_state::namcos2_68k_key_r), FUNC(namcos2_state::namcos2_68k_key_w));
}

void namcos2_state::common_suzuka8h_roz_am(address_map &map)
{
	map(0xc00000, 0xc0ffff).noprw(); // no ROZ hardware implemented in PCB
	map(0xd00000, 0xd0001f).noprw(); // ^^
}

void namcos2_state::master_suzuka8h_am(address_map &map)
{
	common_suzuka8h_am(map);
	master_common_am(map);
	common_suzuka8h_roz_am(map);
}

void namcos2_state::slave_suzuka8h_am(address_map &map)
{
	common_suzuka8h_am(map);
	slave_common_am(map);
	common_suzuka8h_roz_am(map);
}

void namcos2_state::common_luckywld_roz_am(address_map &map)
{
	map(0xc00000, 0xc0ffff).rw(m_c169roz, FUNC(namco_c169roz_device::videoram_r), FUNC(namco_c169roz_device::videoram_w));
	map(0xd00000, 0xd0001f).rw(m_c169roz, FUNC(namco_c169roz_device::control_r), FUNC(namco_c169roz_device::control_w));
}

void namcos2_state::master_luckywld_am(address_map &map)
{
	common_suzuka8h_am(map);
	master_common_am(map);
	common_luckywld_roz_am(map);
}

void namcos2_state::slave_luckywld_am(address_map &map)
{
	common_suzuka8h_am(map);
	slave_common_am(map);
	common_luckywld_roz_am(map);
}

/*************************************************************/
/* 6809 SOUND CPU Memory declarations                        */
/*************************************************************/

void namcos2_state::sound_default_am(address_map &map)
{
	map(0x0000, 0x3fff).bankr("audiobank"); /* banked */
	map(0x4000, 0x4001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x5000, 0x51ff).mirror(0x0e00).rw(m_c140, FUNC(c140_device::c140_r), FUNC(c140_device::c140_w));
	map(0x6000, 0x61ff).mirror(0x0e00).rw(m_c140, FUNC(c140_device::c140_r), FUNC(c140_device::c140_w)); // mirrored
	map(0x7000, 0x77ff).mirror(0x0800).rw(FUNC(namcos2_state::dpram_byte_r), FUNC(namcos2_state::dpram_byte_w)).share("dpram");
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xbfff).nopw(); /* Amplifier enable on 1st write */
	map(0xc000, 0xc001).w(FUNC(namcos2_state::sound_bankselect_w));
	map(0xd001, 0xd001).nopw(); /* Watchdog */
	map(0xe000, 0xe000).nopw();
	map(0xd000, 0xffff).rom().region("audiocpu", 0x01000);
}

void namcos2_state::c140_default_am(address_map &map)
{
	map.global_mask(0x7fffff); // bit 23-24 not connected
	map(0x000000, 0x7fffff).r(FUNC(namcos2_state::c140_rom_r));
}

/*************************************************************/
/*                                                           */
/*  NAMCO SYSTEM 2 PORT MACROS                               */
/*                                                           */
/*  Below are the port definition macros that should be used */
/*  as the basis for defining a port set for a Namco System2 */
/*  game.                                                    */
/*                                                           */
/*************************************************************/

#define NAMCOS2_MCU_PORT_B_DEFAULT \
	PORT_START("MCUB")      /* 63B05Z0 - PORT B */ \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

#define NAMCOS2_MCU_PORT_C_DEFAULT \
	PORT_START("MCUC")      /* 63B05Z0 - PORT C & SCI */ \
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED ) \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 ) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Service Button") \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

#define NAMCOS2_MCU_ANALOG_PORT_DEFAULT \
	PORT_START("AN0")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 0 */ \
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) \
	PORT_START("AN1")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 1 */ \
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) \
	PORT_START("AN2")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 2 */ \
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) \
	PORT_START("AN3")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 3 */ \
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) \
	PORT_START("AN4")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 4 */ \
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) \
	PORT_START("AN5")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 5 */ \
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) \
	PORT_START("AN6")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 6 */ \
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) \
	PORT_START("AN7")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 7 */ \
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

#define NAMCOS2_MCU_PORT_H_DEFAULT \
	PORT_START("MCUH")      /* 63B05Z0 - PORT H */ \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )

#define NAMCOS2_MCU_DIPSW_DEFAULT \
	PORT_START("DSW")       /* 63B05Z0 - $2000 DIP SW */ \
	PORT_DIPNAME( 0x01, 0x01, "Video Display") \
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) ) \
	PORT_DIPSETTING(    0x00, "Frozen" ) \
	PORT_DIPNAME( 0x02, 0x02, "$2000-1") \
	PORT_DIPSETTING(    0x02, "H" ) \
	PORT_DIPSETTING(    0x00, "L" ) \
	PORT_DIPNAME( 0x04, 0x04, "$2000-2") \
	PORT_DIPSETTING(    0x04, "H" ) \
	PORT_DIPSETTING(    0x00, "L" ) \
	PORT_DIPNAME( 0x08, 0x08, "$2000-3") \
	PORT_DIPSETTING(    0x08, "H" ) \
	PORT_DIPSETTING(    0x00, "L" ) \
	PORT_DIPNAME( 0x10, 0x10, "$2000-4") \
	PORT_DIPSETTING(    0x10, "H" ) \
	PORT_DIPSETTING(    0x00, "L" ) \
	PORT_DIPNAME( 0x20, 0x20, "$2000-5") \
	PORT_DIPSETTING(    0x20, "H" ) \
	PORT_DIPSETTING(    0x00, "L" ) \
	PORT_DIPNAME( 0x40, 0x40, "$2000-6") \
	PORT_DIPSETTING(    0x40, "H" ) \
	PORT_DIPSETTING(    0x00, "L" ) \
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

#define NAMCOS2_MCU_DIAL_DEFAULT \
	PORT_START("MCUDI0")        /* 63B05Z0 - $3000 */ \
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) \
	PORT_START("MCUDI1")        /* 63B05Z0 - $3001 */ \
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) \
	PORT_START("MCUDI2")        /* 63B05Z0 - $3002 */ \
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) \
	PORT_START("MCUDI3")        /* 63B05Z0 - $3003 */ \
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

/*************************************************************/
/*                                                           */
/*  NAMCO SYSTEM 2 PORT DEFINITIONS                          */
/*                                                           */
/*  There is a standard port definition defined that will    */
/*  work for most games, if you wish to produce a special    */
/*  definition for a particular game then see the assault    */
/*  and dirtfox definitions for examples of how to construct */
/*  a special port definition                                */
/*                                                           */
/*  The default definitions includes only the following list */
/*  of connections :                                         */
/*    2 Joysticks, 6 Buttons, 1 Service, 1 Advance           */
/*    2 Start                                                */
/*                                                           */
/*************************************************************/

static INPUT_PORTS_START( base )
	NAMCOS2_MCU_PORT_B_DEFAULT
	NAMCOS2_MCU_PORT_C_DEFAULT
	NAMCOS2_MCU_ANALOG_PORT_DEFAULT
	NAMCOS2_MCU_PORT_H_DEFAULT
	NAMCOS2_MCU_DIPSW_DEFAULT
	NAMCOS2_MCU_DIAL_DEFAULT
INPUT_PORTS_END

static INPUT_PORTS_START( kyukaidk )
	PORT_INCLUDE( base )

	PORT_MODIFY("DSW")
	/* Must be 'L' for correct C140 output */
	PORT_DIPNAME( 0x20, 0x00, "$2000-5" )
	PORT_DIPSETTING(    0x20, "H" )
	PORT_DIPSETTING(    0x00, "L" )
INPUT_PORTS_END

static INPUT_PORTS_START( gollygho )
	PORT_START("MCUB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	NAMCOS2_MCU_PORT_C_DEFAULT

	PORT_START("AN0")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_REVERSE
	PORT_START("AN1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_REVERSE
	PORT_START("AN2")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_PLAYER(2) PORT_REVERSE
	PORT_START("AN3")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_PLAYER(2) PORT_REVERSE
	PORT_START("AN4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MCUH")      /* 63B05Z0 - PORT H */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	NAMCOS2_MCU_DIPSW_DEFAULT
	NAMCOS2_MCU_DIAL_DEFAULT
INPUT_PORTS_END

static INPUT_PORTS_START( bubbletr )
	PORT_START("MCUB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	NAMCOS2_MCU_PORT_C_DEFAULT

	// crosshair is required for gun calibration (hold 9, press F2)
	PORT_START("AN0")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, -1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(8)
	PORT_START("AN1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(8)
	PORT_START("AN2")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, -1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_PLAYER(2)
	PORT_START("AN3")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_PLAYER(2)
	PORT_START("AN4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MCUH")      /* 63B05Z0 - PORT H */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	NAMCOS2_MCU_DIPSW_DEFAULT
	NAMCOS2_MCU_DIAL_DEFAULT
INPUT_PORTS_END

static INPUT_PORTS_START( finallap )
	PORT_START("MCUB")      /* 63B05Z0 - PORT B */
	PORT_DIPNAME( 0x01, 0x01, "PortB 0x01")
	PORT_DIPSETTING(    0x01, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	/**
	 * Each player can have one of four car types, selected by attaching a
	 * particular special connector (included with the game) to the PCB.
	 *
	 * (see also Car Type(B) below in Port H)
	 */
	PORT_DIPNAME( 0x02, 0x02, "Car Type(A)")
	PORT_DIPSETTING(    0x02, "McLaren/Williams" )
	PORT_DIPSETTING(    0x00, "Lotus/March" )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("MCUC")      /* 63B05Z0 - PORT C & SCI */
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE(0x40, IP_ACTIVE_LOW)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("AN0")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 0 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN1")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 1 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN2")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 2 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN3")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 3 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN4")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 4 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN5")       /* Steering Wheel */        /* sensitivity, delta, min, max */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)
	PORT_START("AN6")       /* Brake Pedal */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30)
	PORT_START("AN7")       /* Accelerator Pedal */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15)

	PORT_START("MCUH")      /* 63B05Z0 - PORT H */
	PORT_DIPNAME( 0x01, 0x01, "PortH 0x01")
	PORT_DIPSETTING(    0x01, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x02, 0x02, "Automatic Car Select")
	PORT_DIPSETTING(    0x02, DEF_STR(No) )
	PORT_DIPSETTING(    0x00, DEF_STR(Yes) )
	PORT_DIPNAME( 0x04, 0x04, "PortH 0x04")
	PORT_DIPSETTING(    0x04, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x08, 0x08, "Enable Onscreen Diagnostics")
	PORT_DIPSETTING(    0x08, DEF_STR(No) )
	PORT_DIPSETTING(    0x00, DEF_STR(Yes) )
	PORT_DIPNAME( 0x10, 0x10, "PortH 0x10")
	PORT_DIPSETTING(    0x10, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_TOGGLE PORT_NAME("Shift Gear")
	PORT_DIPNAME( 0x40, 0x40, "PortH 0x40")
	PORT_DIPSETTING(    0x40, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x80, 0x80, "Car Type(B)")
	PORT_DIPSETTING(    0x00, "McLaren/March" )
	PORT_DIPSETTING(    0x80, "Williams/Lotus" )

	NAMCOS2_MCU_DIPSW_DEFAULT /* 63B05Z0 - $2000 DIP SW */

	NAMCOS2_MCU_DIAL_DEFAULT  /* 63B05Z0 - $3000 */
INPUT_PORTS_END

static INPUT_PORTS_START( finalap3 )
	PORT_START("MCUB")      /* 63B05Z0 - PORT B */
	PORT_DIPNAME( 0x01, 0x01, "PortB 0x01")
	PORT_DIPSETTING(    0x01, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x02, 0x02, "Car Type(A)")
	PORT_DIPSETTING(    0x02, "McLaren/Williams" )
	PORT_DIPSETTING(    0x00, "Lotus/March" )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("MCUC")      /* 63B05Z0 - PORT C & SCI */
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE(0x40, IP_ACTIVE_LOW)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("AN0")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 0 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN1")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 1 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN2")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 2 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN3")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 3 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN4")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 4 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN5")       /* Steering Wheel */        /* sensitivity, delta, min, max */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)
	PORT_START("AN6")       /* Brake Pedal */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30)
	PORT_START("AN7")       /* Accelerator Pedal */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15)

	PORT_START("MCUH")      /* 63B05Z0 - PORT H */
	PORT_DIPNAME( 0x01, 0x01, "PortH 0x01")
	PORT_DIPSETTING(    0x01, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x02, 0x02, "Automatic Car Select")
	PORT_DIPSETTING(    0x02, DEF_STR(No) )
	PORT_DIPSETTING(    0x00, DEF_STR(Yes) )
	PORT_DIPNAME( 0x04, 0x04, "PortH 0x04")
	PORT_DIPSETTING(    0x04, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x08, 0x08, "Enable Onscreen Diagnostics")
	PORT_DIPSETTING(    0x08, DEF_STR(No) )
	PORT_DIPSETTING(    0x00, DEF_STR(Yes) )
	PORT_DIPNAME( 0x10, 0x10, "PortH 0x10")
	PORT_DIPSETTING(    0x10, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_TOGGLE PORT_NAME("Shift Gear")
	PORT_DIPNAME( 0x40, 0x40, "PortH 0x40")
	PORT_DIPSETTING(    0x40, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x80, 0x80, "Car Type(B)")
	PORT_DIPSETTING(    0x00, "McLaren/March" )
	PORT_DIPSETTING(    0x80, "Williams/Lotus" )

	NAMCOS2_MCU_DIPSW_DEFAULT /* 63B05Z0 - $2000 DIP SW */

	NAMCOS2_MCU_DIAL_DEFAULT  /* 63B05Z0 - $3000 */
INPUT_PORTS_END

static INPUT_PORTS_START( fourtrax )
	PORT_START("MCUB")      /* 63B05Z0 - PORT B */
	PORT_DIPNAME( 0x01, 0x01, "PortB 0x01")
	PORT_DIPSETTING(    0x01, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x02, 0x02, "PortB 0x02")
	PORT_DIPSETTING(    0x02, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x04, 0x04, "PortB 0x04")
	PORT_DIPSETTING(    0x04, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x08, 0x08, "PortB 0x08")
	PORT_DIPSETTING(    0x08, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x10, 0x10, "PortB 0x10")
	PORT_DIPSETTING(    0x10, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x20, 0x20, "PortB 0x20")
	PORT_DIPSETTING(    0x20, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("MCUC")      /* 63B05Z0 - PORT C & SCI */
	PORT_DIPNAME( 0x01, 0x01, "PortC 0x01")
	PORT_DIPSETTING(    0x01, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x02, 0x02, "PortC 0x02")
	PORT_DIPSETTING(    0x02, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x04, 0x04, "PortC 0x04")
	PORT_DIPSETTING(    0x04, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x08, 0x08, "PortC 0x08")
	PORT_DIPSETTING(    0x08, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Service Button")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("AN0")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 0 2 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN1")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 1 3 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN2")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 2 4 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN3")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 3 5 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN4")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 4 6 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN5")       /* Steering Wheel 7 */      /* sensitivity, delta, min, max */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(75) PORT_KEYDELTA(50)
	PORT_START("AN6")       /* Brake Pedal 8 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30)
	PORT_START("AN7")       /* Accelerator Pedal 9 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15)

	PORT_START("MCUH")      /* 63B05Z0 - PORT H */
	PORT_DIPNAME( 0x01, 0x01, "PortH 0x01")
	PORT_DIPSETTING(    0x01, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x02, 0x02, "PortH 0x02")
	PORT_DIPSETTING(    0x02, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x04, 0x04, "PortH 0x04")
	PORT_DIPSETTING(    0x04, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x08, 0x08, "PortH 0x08 (onscreen diagnostics)")
	PORT_DIPSETTING(    0x08, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x10, 0x10, "PortH 0x10")
	PORT_DIPSETTING(    0x10, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Gear Shift") PORT_TOGGLE
	PORT_DIPNAME( 0x40, 0x40, "PortH 0x40")
	PORT_DIPSETTING(    0x40, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x80, 0x80, "PortH 0x80")
	PORT_DIPSETTING(    0x80, "H" )
	PORT_DIPSETTING(    0x00, "L" )

	NAMCOS2_MCU_DIPSW_DEFAULT
	NAMCOS2_MCU_DIAL_DEFAULT
INPUT_PORTS_END

static INPUT_PORTS_START( assault )
	PORT_START("MCUB")      /* 63B05Z0 - PORT B */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	NAMCOS2_MCU_PORT_C_DEFAULT
	NAMCOS2_MCU_ANALOG_PORT_DEFAULT

	PORT_START("MCUH")      /* 63B05Z0 - PORT H */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_4WAY PORT_PLAYER(1)

	NAMCOS2_MCU_DIPSW_DEFAULT

	/* These are tagged this way to make easier reads by tag */
	PORT_START("MCUDI0")    /* 63B05Z0 - $3000 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_START("MCUDI1")    /* 63B05Z0 - $3001 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("MCUDI2")    /* 63B05Z0 - $3002 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("MCUDI3")    /* 63B05Z0 - $3003 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( suzuka )
	PORT_START("MCUB")      /* 63B05Z0 - PORT B */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	NAMCOS2_MCU_PORT_C_DEFAULT

	PORT_START("AN0")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 0 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN1")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 1 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN2")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 2 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN3")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 3 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN4")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 4 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN5") /* Steering Wheel */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(50) PORT_KEYDELTA(100)
	PORT_START("AN6")       /* Brake pedal */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0x7f) PORT_SENSITIVITY(100) PORT_KEYDELTA(30)
	PORT_START("AN7")       /* Accelerator pedal */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15)

	PORT_START("MCUH")      /* 63B05Z0 - PORT H */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")       /* 63B05Z0 - $2000 DIP SW */
	PORT_DIPNAME( 0x01, 0x01, "Video Display")
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Frozen" )
	PORT_DIPNAME( 0x02, 0x02, "$2000-1")
	PORT_DIPSETTING(    0x02, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x04, 0x04, "$2000-2")
	PORT_DIPSETTING(    0x04, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x08, 0x08, "$2000-3")
	PORT_DIPSETTING(    0x08, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x10, 0x10, "$2000-4")
	PORT_DIPSETTING(    0x10, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x20, 0x20, "$2000-5")
	PORT_DIPSETTING(    0x20, "H" )
	PORT_DIPSETTING(    0x00, "L" )
	PORT_DIPNAME( 0x40, 0x40, "System is Status Monitor")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	NAMCOS2_MCU_DIAL_DEFAULT
INPUT_PORTS_END

/* note, even with perfectly calibrated gun settings the on-screen cursor won't align with the MAME cursor
   because the game assumes the screen space isn't a linear mapping to the gun.  This is especially noticeable
   with the P2 cursor.  You should turn off the MAME cursor using the 'Crosshair Options' in the MAME internal
   TAB menu.

   This game has a rather unique control setup, 2 lightguns, plus steering hardware.  Defaults have been set
   up to avoid a large number of duplicate buttons.  Using these settings (with -mouse) the following mapping is
   given

     Z, X - Steer Car
     Ctrl - Accelerate
     Alt - Brake
     Mouse - Aim Gun
     Left Shift - Fire Gun
 */

static INPUT_PORTS_START( luckywld )
	PORT_START("MCUB")      /* 63B05Z0 - PORT B */
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	NAMCOS2_MCU_PORT_C_DEFAULT

	PORT_START("AN0")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 0 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_PLAYER(2)
	PORT_START("AN2")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(8)
	PORT_START("AN3")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_PLAYER(2)
	PORT_START("AN4")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(8)
	PORT_START("AN5")       /* Steering Wheel */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_CODE(INPUT_CODE_INVALID) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X)
	PORT_START("AN6")       /* Brake pedal */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0x7f) PORT_SENSITIVITY(100) PORT_KEYDELTA(30)
	PORT_START("AN7")       /* Accelerator pedal */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0x7f) PORT_SENSITIVITY(100) PORT_KEYDELTA(15)

	PORT_START("MCUH")      /* 63B05Z0 - PORT H */
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Fire") PORT_CODE(KEYCODE_RSHIFT) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Fire") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	NAMCOS2_MCU_DIPSW_DEFAULT
	NAMCOS2_MCU_DIAL_DEFAULT

	PORT_MODIFY("DSW")
	// this applies to both the World and Japan sets
	PORT_DIPNAME( 0x40, 0x40, "Show Winners Don't Use Drugs")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( sgunner )
	PORT_START("MCUB")      /* 63B05Z0 - PORT B */
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	NAMCOS2_MCU_PORT_C_DEFAULT

	PORT_START("AN0")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 0 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN4")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(8)
	PORT_START("AN5")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_PLAYER(2)
	PORT_START("AN6")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(8)
	PORT_START("AN7")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_PLAYER(2)

	PORT_START("MCUH")      /* 63B05Z0 - PORT H */
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	NAMCOS2_MCU_DIPSW_DEFAULT
	NAMCOS2_MCU_DIAL_DEFAULT
INPUT_PORTS_END


static INPUT_PORTS_START( dirtfox )
	PORT_START("MCUB")      /* 63B05Z0 - PORT B */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Gear Shift Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 )  PORT_NAME("Gear Shift Up")

	NAMCOS2_MCU_PORT_C_DEFAULT

	PORT_START("AN0")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 0 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN1")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 1 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN2")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 2 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN3")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 3 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN4")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 4 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN5")       /* Steering Wheel */
	PORT_BIT( 0xff,  0x80, IPT_PADDLE ) PORT_SENSITIVITY(70) PORT_KEYDELTA(50)
	PORT_START("AN6")       /* Brake pedal */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0x7f) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_NAME("Brake")
	PORT_START("AN7")       /* Accelerator pedal */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0x7f) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_NAME("Accelerator")

	PORT_START("MCUH")      /* 63B05Z0 - PORT H */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	NAMCOS2_MCU_DIPSW_DEFAULT
	NAMCOS2_MCU_DIAL_DEFAULT
INPUT_PORTS_END

static INPUT_PORTS_START( metlhawk )
	PORT_START("MCUB")      /* 63B05Z0 - PORT B */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	NAMCOS2_MCU_PORT_C_DEFAULT

	PORT_START("AN0")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 0 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN1")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 1 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN2")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 2 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN3")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 3 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN4")       /* 63B05Z0 - 8 CHANNEL ANALOG - CHANNEL 4 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("AN5")       /* Joystick Y */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(16)
	PORT_START("AN6")       /* Joystick X */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(16)
	PORT_START("AN7")       /* Lever */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Z ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(16)

	PORT_START("MCUH")      /* 63B05Z0 - PORT H */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	NAMCOS2_MCU_DIPSW_DEFAULT
	NAMCOS2_MCU_DIAL_DEFAULT
INPUT_PORTS_END


/*************************************************************/
/* Namco System II - Graphics Declarations                   */
/*************************************************************/

static const gfx_layout obj_layout =
{
	32,32,
	RGN_FRAC(1,1),  /* number of sprites */
	8,      /* bits per pixel */
	{ STEP8(0,4) },
	{ /* x offsets */
		STEP4(4*8*0,1),STEP4(4*8*1,1),
		STEP4(4*8*2,1),STEP4(4*8*3,1),
		STEP4(4*8*4,1),STEP4(4*8*5,1),
		STEP4(4*8*6,1),STEP4(4*8*7,1)
	},
	{ STEP32(0,4*8*8) },
	32*32*8 /* sprite offset */
};

static const gfx_layout metlhawk_sprite_layout =
{
	32,32,
	RGN_FRAC(1,1), /* number of sprites */
	8, /* bits per pixel */
	{ STEP8(0,1) },
	{ STEP32(0,8) },
	{ STEP32(0,8*32) },
	32*32*8
};

static const gfx_layout metlhawk_sprite_layout_swapped =
{
	32,32,
	RGN_FRAC(1,1), /* number of sprites */
	8, /* bits per pixel */
	{ STEP8(0,1) },
	{ STEP32(0,8*32) },
	{ STEP32(0,8) },
	32*32*8
};

static GFXDECODE_START( gfx_metlhawk )
	GFXDECODE_ENTRY( "sprite", 0x000000, metlhawk_sprite_layout,         0, 16 )
	GFXDECODE_ENTRY( "sprite", 0x000000, metlhawk_sprite_layout_swapped, 0, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_namcos2 )
	GFXDECODE_ENTRY( "sprite", 0x000000, obj_layout, 0, 16 )
GFXDECODE_END

/* end */

/******************************************

Master clock = 49.152MHz

68000 Measured at  84ns = 12.4MHz   BUT 49.152MHz/4 = 12.288MHz = 81ns
6809  Measured at 343ns = 2.915 MHz BUT 49.152MHz/16 = 3.072MHz = 325ns
63B05 Measured at 120ns = 8.333 MHz BUT 49.152MHz/6 = 8.192MHz = 122ns

I've corrected all frequencies to be multiples of integer divisions of
the 49.152MHz master clock. Additionally the 6305 looks to have an
internal divider.

Soooo;

68000   = 12288000
6809    =  3072000
63B05Z0 =  2048000

The interrupts to CPU4 has been measured at 60Hz (16.5mS period) on a
logic analyser. This interrupt is wired to port PA1 which is configured
via software as INT1

*******************************************/

/*************************************************************/
/*                                                           */
/*  NAMCO SYSTEM 2 MACHINE DEFINTIONS                        */
/*                                                           */
/*************************************************************/

void namcos2_state::configure_c116_standard(machine_config &config)
{
	NAMCO_C116(config, m_c116);
	m_c116->enable_shadows();
}

void namcos2_state::configure_c148_standard(machine_config &config)
{
	NAMCO_C148(config, m_master_intc, 0, m_maincpu, true);
	m_master_intc->link_c148_device(m_slave_intc);
	m_master_intc->out_ext1_callback().set(FUNC(namcos2_state::sound_reset_w));
	m_master_intc->out_ext2_callback().set(FUNC(namcos2_state::system_reset_w));

	NAMCO_C148(config, m_slave_intc, 0, m_slave, false);
	m_slave_intc->link_c148_device(m_master_intc);
}

void namcos2_state::configure_c65_standard(machine_config &config)
{
	NAMCOC65(config, m_c65, C65_CPU_CLOCK);
	m_c65->in_pb_callback().set_ioport("MCUB");
	m_c65->in_pc_callback().set_ioport("MCUC");
	m_c65->in_ph_callback().set_ioport("MCUH");
	m_c65->in_pdsw_callback().set_ioport("DSW");
	m_c65->di0_in_cb().set_ioport("MCUDI0");
	m_c65->di1_in_cb().set_ioport("MCUDI1");
	m_c65->di2_in_cb().set_ioport("MCUDI2");
	m_c65->di3_in_cb().set_ioport("MCUDI3");
	m_c65->an0_in_cb().set_ioport("AN0");
	m_c65->an1_in_cb().set_ioport("AN1");
	m_c65->an2_in_cb().set_ioport("AN2");
	m_c65->an3_in_cb().set_ioport("AN3");
	m_c65->an4_in_cb().set_ioport("AN4");
	m_c65->an5_in_cb().set_ioport("AN5");
	m_c65->an6_in_cb().set_ioport("AN6");
	m_c65->an7_in_cb().set_ioport("AN7");
	m_c65->dp_in_callback().set(FUNC(namcos2_state::dpram_byte_r));
	m_c65->dp_out_callback().set(FUNC(namcos2_state::dpram_byte_w));
}

void namcos2_state::configure_c68_standard(machine_config &config)
{
	NAMCOC68(config, m_c68, C68_CPU_CLOCK);
	m_c68->in_pb_callback().set_ioport("MCUB");
	m_c68->in_pc_callback().set_ioport("MCUC");
	m_c68->in_ph_callback().set_ioport("MCUH");
	m_c68->in_pdsw_callback().set_ioport("DSW");
	m_c68->di0_in_cb().set_ioport("MCUDI0");
	m_c68->di1_in_cb().set_ioport("MCUDI1");
	m_c68->di2_in_cb().set_ioport("MCUDI2");
	m_c68->di3_in_cb().set_ioport("MCUDI3");
	m_c68->an0_in_cb().set_ioport("AN0");
	m_c68->an1_in_cb().set_ioport("AN1");
	m_c68->an2_in_cb().set_ioport("AN2");
	m_c68->an3_in_cb().set_ioport("AN3");
	m_c68->an4_in_cb().set_ioport("AN4");
	m_c68->an5_in_cb().set_ioport("AN5");
	m_c68->an6_in_cb().set_ioport("AN6");
	m_c68->an7_in_cb().set_ioport("AN7");
	m_c68->dp_in_callback().set(FUNC(namcos2_state::dpram_byte_r));
	m_c68->dp_out_callback().set(FUNC(namcos2_state::dpram_byte_w));
}

void namcos2_state::configure_common_standard(machine_config &config)
{
	M68000(config, m_maincpu, M68K_CPU_CLOCK); /* 12.288MHz (49.152MHz OSC/4) */
	TIMER(config, "scantimer").configure_scanline(FUNC(namcos2_state::screen_scanline), "screen", 0, 1);

	M68000(config, m_slave, M68K_CPU_CLOCK); /*  12.288MHz (49.152MHz OSC/4) */

	MC6809E(config, m_audiocpu, M68B09_CPU_CLOCK); /* 2.048MHz (49.152MHz OSC/24) - Sound handling */
	m_audiocpu->set_periodic_int(FUNC(namcos2_state::irq0_line_hold), attotime::from_hz(2*60));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	NAMCO_C139(config, m_sci, 0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MAIN_OSC_CLOCK/8, 384, 0*8, 36*8, 264, 0*8, 28*8);
	m_screen->set_palette(m_c116);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	C140(config, m_c140, C140_SOUND_CLOCK); /* 21.333kHz */
	m_c140->set_addrmap(0, &namcos2_state::c140_default_am);
	m_c140->int1_callback().set_inputline(m_audiocpu, M6809_FIRQ_LINE);
}

// TODO: temp
TIMER_DEVICE_CALLBACK_MEMBER(namcos2_state::screen_scanline)
{
	int scanline = param;
	int cur_posirq = get_pos_irq_scanline();

	if (scanline == 200) // triggering this a bit before Vblank allows the Assault Plus mode select screen to work without overclocking the IO MCU, exact timings unknown.
	{
		if (m_c65)
			m_c65->ext_interrupt(HOLD_LINE);

		if (m_c68)
			m_c68->ext_interrupt(ASSERT_LINE);
	}

	if(scanline == 240)
	{
		m_master_intc->vblank_irq_trigger();
		m_slave_intc->vblank_irq_trigger();

	}

	if(scanline == cur_posirq)
	{
		m_master_intc->pos_irq_trigger();
		m_slave_intc->pos_irq_trigger();
		// TODO: should be when video registers are updated (and/or latched) but that makes things worse
		m_screen->update_partial(m_update_to_line_before_posirq ? param-1 : param);
	}
}

void namcos2_state::configure_c123tmap_standard(machine_config &config)
{
	NAMCO_C123TMAP(config, m_c123tmap);
	m_c123tmap->set_palette(m_c116);
	m_c123tmap->set_tile_callback(namco_c123tmap_device::c123_tilemap_delegate(&namcos2_state::TilemapCB, this));
	m_c123tmap->set_color_base(16*256);
}

void namcos2_state::configure_c169roz_standard(machine_config &config)
{
	NAMCO_C169ROZ(config, m_c169roz);
	m_c169roz->set_palette(m_c116);
	m_c169roz->set_is_namcofl(false);
	m_c169roz->set_ram_words(0x10000/2);
	m_c169roz->set_color_base(0*256);
}

void namcos2_state::configure_c355spr_standard(machine_config &config)
{
	NAMCO_C355SPR(config, m_c355spr);
	m_c355spr->set_screen(m_screen);
	m_c355spr->set_palette(m_c116);
	m_c355spr->set_scroll_offsets(0x26, 0x19);
	m_c355spr->set_tile_callback(namco_c355spr_device::c355_obj_code2tile_delegate());
	m_c355spr->set_palxor(0x0);
	m_c355spr->set_color_base(0);
}

void namcos2_state::configure_c45road_standard(machine_config &config)
{
	NAMCO_C45_ROAD(config, m_c45_road);
	m_c45_road->set_palette(m_c116);
	m_c45_road->set_xoffset(-72);
}

void namcos2_state::configure_namcos2_sprite_standard(machine_config &config)
{
	NAMCOS2_SPRITE(config, m_ns2sprite, 0);
	m_ns2sprite->set_gfxdecode_tag("gfxdecode");
	m_ns2sprite->set_spriteram_tag("spriteram");
}

void namcos2_state::configure_namcos2_roz_standard(machine_config &config)
{
	NAMCOS2_ROZ(config, m_ns2roz, 0);
	m_ns2roz->set_palette(m_c116);
	m_ns2roz->set_rozram_tag("rozram");
	m_ns2roz->set_rozctrl_tag("rozctrl");
}

void namcos2_state::base_noio(machine_config &config)
{
	configure_common_standard(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos2_state::master_default_am);

	m_slave->set_addrmap(AS_PROGRAM, &namcos2_state::slave_default_am);

	m_audiocpu->set_addrmap(AS_PROGRAM, &namcos2_state::sound_default_am);

	config.set_maximum_quantum(attotime::from_hz(12000)); /* CPU slices per frame */

	configure_c148_standard(config);
	configure_c116_standard(config);

	m_screen->set_screen_update(FUNC(namcos2_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_c116, gfx_namcos2);

	configure_namcos2_sprite_standard(config);
	configure_c123tmap_standard(config);
	configure_namcos2_roz_standard(config);

	m_c140->add_route(0, "lspeaker", 0.75);
	m_c140->add_route(1, "rspeaker", 0.75);

	YM2151(config, "ymsnd", YM2151_SOUND_CLOCK).add_route(0, "lspeaker", 0.80).add_route(1, "rspeaker", 0.80); /* 3.579545MHz */
}


void namcos2_state::base(machine_config &config)
{
	base_noio(config);
	configure_c65_standard(config);
}

void namcos2_state::base_c68(machine_config &config)
{
	base_noio(config);
	configure_c68_standard(config);
}

void namcos2_state::base2(machine_config &config)
{
	base(config);

	m_c140->reset_routes();
	m_c140->add_route(0, "lspeaker", 1.0);
	m_c140->add_route(1, "rspeaker", 1.0);
}

void namcos2_state::assaultp(machine_config &config)
{
	base2(config);

	config.set_maximum_quantum(attotime::from_hz(12000*8)); /* CPU slices per frame - boosted (along with MCU speed) so that the Mode Select works */
}

void namcos2_state::base3(machine_config &config)
{
	base(config);

	m_c140->reset_routes();
	m_c140->add_route(0, "lspeaker", 0.45);
	m_c140->add_route(1, "rspeaker", 0.45);

	YM2151(config.replace(), "ymsnd", YM2151_SOUND_CLOCK).add_route(0, "lspeaker", 1.0).add_route(1, "rspeaker", 1.0); /* 3.579545MHz */
}


void namcos2_state::finallap_noio(machine_config &config)
{
	configure_common_standard(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos2_state::master_finallap_am);

	m_slave->set_addrmap(AS_PROGRAM, &namcos2_state::slave_finallap_am);

	m_audiocpu->set_addrmap(AS_PROGRAM, &namcos2_state::sound_default_am);

	config.set_maximum_quantum(attotime::from_hz(6000)); /* CPU slices per frame */

	configure_c148_standard(config);
	configure_c116_standard(config);

	m_screen->set_screen_update(FUNC(namcos2_state::screen_update_finallap));

	GFXDECODE(config, m_gfxdecode, m_c116, gfx_namcos2);

	configure_namcos2_sprite_standard(config);
	configure_c123tmap_standard(config);
	configure_c45road_standard(config);

	m_c140->add_route(0, "lspeaker", 0.75);
	m_c140->add_route(1, "rspeaker", 0.75);

	YM2151(config, "ymsnd", YM2151_SOUND_CLOCK).add_route(0, "lspeaker", 0.80).add_route(1, "rspeaker", 0.80); /* 3.579545MHz */
}

void namcos2_state::base_fl(machine_config &config)
{
	finallap_noio(config);
	configure_c65_standard(config);
}

void namcos2_state::finallap(machine_config &config)
{
	base_fl(config);

	NAMCOS2_SPRITE_FINALLAP(config.replace(), m_ns2sprite, 0);
	m_ns2sprite->set_gfxdecode_tag("gfxdecode");
	m_ns2sprite->set_spriteram_tag("spriteram");
}


void namcos2_state::finallap_c68(machine_config &config)
{
	finallap_noio(config);
	configure_c68_standard(config);
}


// finalap2 has different mangle
void namcos2_state::finalap2(machine_config &config)
{
	base_fl(config);

	m_c123tmap->set_tile_callback(namco_c123tmap_device::c123_tilemap_delegate(&namcos2_state::TilemapCB_finalap2, this));
}

void namcos2_state::finalap3(machine_config &config)
{
	finallap_c68(config);

	m_c123tmap->set_tile_callback(namco_c123tmap_device::c123_tilemap_delegate(&namcos2_state::TilemapCB_finalap2, this));
}


void namcos2_state::sgunner(machine_config &config)
{
	configure_common_standard(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos2_state::master_sgunner_am);

	m_slave->set_addrmap(AS_PROGRAM, &namcos2_state::slave_sgunner_am);

	m_audiocpu->set_addrmap(AS_PROGRAM, &namcos2_state::sound_default_am);

	configure_c65_standard(config);

	config.set_maximum_quantum(attotime::from_hz(6000)); /* CPU slices per frame */

	configure_c148_standard(config);
	configure_c116_standard(config);

	m_screen->set_screen_update(FUNC(namcos2_state::screen_update_sgunner));

	configure_c355spr_standard(config);
	configure_c123tmap_standard(config);

	MCFG_VIDEO_START_OVERRIDE(namcos2_state, sgunner)

	m_c140->add_route(0, "lspeaker", 0.75);
	m_c140->add_route(1, "rspeaker", 0.75);

	YM2151(config, "ymsnd", YM2151_SOUND_CLOCK).add_route(0, "lspeaker", 0.80).add_route(1, "rspeaker", 0.80); /* 3.579545MHz */
}

void namcos2_state::sgunner2(machine_config &config)
{
	configure_common_standard(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos2_state::master_sgunner_am);

	m_slave->set_addrmap(AS_PROGRAM, &namcos2_state::slave_sgunner_am);

	m_audiocpu->set_addrmap(AS_PROGRAM, &namcos2_state::sound_default_am);

	configure_c68_standard(config);

	config.set_maximum_quantum(attotime::from_hz(6000)); /* CPU slices per frame */

	configure_c148_standard(config);
	configure_c116_standard(config);

	m_screen->set_screen_update(FUNC(namcos2_state::screen_update_sgunner));

	configure_c355spr_standard(config);
	configure_c123tmap_standard(config);

	MCFG_VIDEO_START_OVERRIDE(namcos2_state, sgunner)

	m_c140->add_route(0, "lspeaker", 0.75);
	m_c140->add_route(1, "rspeaker", 0.75);

	YM2151(config, "ymsnd", YM2151_SOUND_CLOCK).add_route(0, "lspeaker", 0.80).add_route(1, "rspeaker", 0.80); /* 3.579545MHz */
}

void namcos2_state::suzuka8h(machine_config &config)
{
	configure_common_standard(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos2_state::master_suzuka8h_am);

	m_slave->set_addrmap(AS_PROGRAM, &namcos2_state::slave_suzuka8h_am);

	m_audiocpu->set_addrmap(AS_PROGRAM, &namcos2_state::sound_default_am);

	configure_c68_standard(config);

	config.set_maximum_quantum(attotime::from_hz(6000)); /* CPU slices per frame */

	configure_c148_standard(config);
	configure_c116_standard(config);

	m_screen->set_screen_update(FUNC(namcos2_state::screen_update_luckywld));
	m_screen->screen_vblank().set(m_c355spr, FUNC(namco_c355spr_device::vblank));

	configure_c355spr_standard(config);
	m_c355spr->set_buffer(1);

	configure_c123tmap_standard(config);
	configure_c45road_standard(config);

	MCFG_VIDEO_START_OVERRIDE(namcos2_state, luckywld)

	m_c140->add_route(0, "lspeaker", 0.75);
	m_c140->add_route(1, "rspeaker", 0.75);

	YM2151(config, "ymsnd", YM2151_SOUND_CLOCK).add_route(0, "lspeaker", 0.80).add_route(1, "rspeaker", 0.80); /* 3.579545MHz */
}

void namcos2_state::luckywld(machine_config &config)
{
	suzuka8h(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &namcos2_state::master_luckywld_am);

	m_slave->set_addrmap(AS_PROGRAM, &namcos2_state::slave_luckywld_am);

	configure_c169roz_standard(config);
	m_c169roz->set_tile_callback(namco_c169roz_device::c169_tilemap_delegate(&namcos2_state::RozCB_luckywld, this));
}

void namcos2_state::metlhawk(machine_config &config)
{
	configure_common_standard(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos2_state::master_metlhawk_am);

	m_slave->set_addrmap(AS_PROGRAM, &namcos2_state::slave_metlhawk_am);

	m_audiocpu->set_addrmap(AS_PROGRAM, &namcos2_state::sound_default_am);

	configure_c65_standard(config);

	config.set_maximum_quantum(attotime::from_hz(6000)); /* CPU slices per frame */

	configure_c148_standard(config);
	configure_c116_standard(config);

	m_screen->set_screen_update(FUNC(namcos2_state::screen_update_metlhawk));

	GFXDECODE(config, m_gfxdecode, m_c116, gfx_metlhawk);

	NAMCOS2_SPRITE_METALHAWK(config, m_ns2sprite, 0);
	m_ns2sprite->set_gfxdecode_tag("gfxdecode");
	m_ns2sprite->set_spriteram_tag("spriteram");

	configure_c123tmap_standard(config);
	configure_c169roz_standard(config);
	m_c169roz->set_tile_callback(namco_c169roz_device::c169_tilemap_delegate(&namcos2_state::RozCB_metlhawk, this));

	MCFG_VIDEO_START_OVERRIDE(namcos2_state, metlhawk)

	m_c140->add_route(0, "lspeaker", 1.0);
	m_c140->add_route(1, "rspeaker", 1.0);

	YM2151(config, "ymsnd", YM2151_SOUND_CLOCK).add_route(0, "lspeaker", 0.80).add_route(1, "rspeaker", 0.80); /* 3.579545MHz */
//  ymsnd.irq_handler().set_inputline("audiocpu", 1);
}


/*************************************************************/
/* Namco System II - ROM Declarations                        */
/*************************************************************/

#define NAMCOS2_GFXROM_LOAD_128K(romname,start,chksum)\
	ROM_LOAD(romname       , (start + 0x000000), 0x020000, chksum )\
	ROM_RELOAD(              (start + 0x020000), 0x020000 )\
	ROM_RELOAD(              (start + 0x040000), 0x020000 )\
	ROM_RELOAD(              (start + 0x060000), 0x020000 )

#define NAMCOS2_GFXROM_LOAD_256K(romname,start,chksum)\
	ROM_LOAD(romname       , (start + 0x000000), 0x040000, chksum )\
	ROM_RELOAD(              (start + 0x040000), 0x040000 )

#define NAMCOS2_SPRROM_LOAD_128K(romname,start,chksum)\
	ROM_LOAD32_BYTE(romname, (start + 0x000000), 0x020000, chksum )\
	ROM_RELOAD(              (start + 0x080000), 0x020000 )\
	ROM_RELOAD(              (start + 0x100000), 0x020000 )\
	ROM_RELOAD(              (start + 0x180000), 0x020000 )

#define NAMCOS2_SPRROM_LOAD_256K(romname,start,chksum)\
	ROM_LOAD32_BYTE(romname, (start + 0x000000), 0x040000, chksum )\
	ROM_RELOAD(              (start + 0x100000), 0x040000 )

#define NAMCOS2_DATA_LOAD_E_128K(romname,start,chksum)\
	ROM_LOAD16_BYTE(romname, (start + 0x000000), 0x020000, chksum )\
	ROM_RELOAD(              (start + 0x040000), 0x020000 )\
	ROM_RELOAD(              (start + 0x080000), 0x020000 )\
	ROM_RELOAD(              (start + 0x0c0000), 0x020000 )

#define NAMCOS2_DATA_LOAD_O_128K(romname,start,chksum)\
	ROM_LOAD16_BYTE(romname, (start + 0x000001), 0x020000, chksum )\
	ROM_RELOAD(              (start + 0x040001), 0x020000 )\
	ROM_RELOAD(              (start + 0x080001), 0x020000 )\
	ROM_RELOAD(              (start + 0x0c0001), 0x020000 )

#define NAMCOS2_DATA_LOAD_E_256K(romname,start,chksum)\
	ROM_LOAD16_BYTE(romname, (start + 0x000000), 0x040000, chksum )\
	ROM_RELOAD(              (start + 0x080000), 0x040000 )

#define NAMCOS2_DATA_LOAD_O_256K(romname,start,chksum)\
	ROM_LOAD16_BYTE(romname, (start + 0x000001), 0x040000, chksum )\
	ROM_RELOAD(              (start + 0x080001), 0x040000 )

#define NAMCOS2_DATA_LOAD_E_512K(romname,start,chksum)\
	ROM_LOAD16_BYTE(romname, (start + 0x000000), 0x080000, chksum )

#define NAMCOS2_DATA_LOAD_O_512K(romname,start,chksum)\
	ROM_LOAD16_BYTE(romname, (start + 0x000001), 0x080000, chksum )


/* ASSAULT (NAMCO) */
ROM_START( assault )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "at2mp0b.bin",  0x000000, 0x010000, CRC(801f71c5) SHA1(b07366a7b5cc4f3dd12fc0455c55146680f9f3f7) )
	ROM_LOAD16_BYTE( "at2mp1b.bin",  0x000001, 0x010000, CRC(72312d4f) SHA1(cbb1d430937dc201293885e177eb18535d8b2884) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "at1sp0.bin",  0x000000, 0x010000, CRC(0de2a0da) SHA1(5f6ce61f2ad1e50f0eac3a115c511151bbb00fd2) )
	ROM_LOAD16_BYTE( "at1sp1.bin",  0x000001, 0x010000, CRC(02d15fbe) SHA1(7c1bc845a06188ebe2efdb5880562beae11a7b4f) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "at1snd0.bin",  0x000000, 0x020000, CRC(1d1ffe12) SHA1(5a49bf1eef981df69abac28132af65670ba01118) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65b.bin",  0x000000, 0x008000, CRC(e9f2922a) SHA1(5767d2f85e1eb3de19192e73b02221f28b1fbb83) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	NAMCOS2_SPRROM_LOAD_128K( "atobj0.bin",  0x000003, CRC(22240076) SHA1(916fc0e6b338a6dda84399df910c3c9463e6b915) )
	NAMCOS2_SPRROM_LOAD_128K( "atobj1.bin",  0x000002, CRC(2284a8e8) SHA1(80f9143e08f9f8ff3e937312a8ce76855a1929ad) )
	NAMCOS2_SPRROM_LOAD_128K( "atobj2.bin",  0x000001, CRC(51425476) SHA1(12a2fb1b61adfa4c21a5af4f206ffe48a045a953) )
	NAMCOS2_SPRROM_LOAD_128K( "atobj3.bin",  0x000000, CRC(791f42ce) SHA1(95583130abe2e6f9ad3e96288d811b4abc3d44b3) )
	NAMCOS2_SPRROM_LOAD_128K( "atobj4.bin",  0x200003, CRC(4782e1b0) SHA1(3d9f4b9eb711fb47e424cd57f7183f49f5dd6ec4) )
	NAMCOS2_SPRROM_LOAD_128K( "atobj5.bin",  0x200002, CRC(f5d158cf) SHA1(f05f44915afe3c17fff0b85a0364f70e79b25428) )
	NAMCOS2_SPRROM_LOAD_128K( "atobj6.bin",  0x200001, CRC(12f6a569) SHA1(e3051de0961f34e15b8642fa769deac3cb0c8305) )
	NAMCOS2_SPRROM_LOAD_128K( "atobj7.bin",  0x200000, CRC(06a929f2) SHA1(65308972a27ab4a649fd08414a89e6f97a09240e) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "atchr0.bin",  0x000000, CRC(6f8e968a) SHA1(b771359a3b08c1aeeb248eff325b19238bb88bf8) )
	NAMCOS2_GFXROM_LOAD_128K( "atchr1.bin",  0x080000, CRC(88cf7cbe) SHA1(df7565cb36658311b784d5406dc6f83141af58a9) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "atroz0.bin",  0x000000, CRC(8c247a97) SHA1(e06dc0b46fa06f41d2017ec8113baf5c4ba832ab) )
	NAMCOS2_GFXROM_LOAD_128K( "atroz1.bin",  0x080000, CRC(e44c475b) SHA1(64768692a8d38377fa1109cd9f7c86b31c01e13c) )
	NAMCOS2_GFXROM_LOAD_128K( "atroz2.bin",  0x100000, CRC(770f377f) SHA1(3a7d4f4b9eb01d4366119f34b0d7d7db2f5a2471) )
	NAMCOS2_GFXROM_LOAD_128K( "atroz3.bin",  0x180000, CRC(01d93d0b) SHA1(47dfda4720f0d4cd41633fd0291b82a54a4eb310) )
	NAMCOS2_GFXROM_LOAD_128K( "atroz4.bin",  0x200000, CRC(f96feab5) SHA1(c861df1f66de046ca714087630f5a1e2f3f30361) )
	NAMCOS2_GFXROM_LOAD_128K( "atroz5.bin",  0x280000, CRC(da2f0d9e) SHA1(a7fbe4748c67906a1ab2902d3b75a21219a677c0) )
	NAMCOS2_GFXROM_LOAD_128K( "atroz6.bin",  0x300000, CRC(9089e477) SHA1(929d7d81c05aa148212e22969d147296811d433c) )
	NAMCOS2_GFXROM_LOAD_128K( "atroz7.bin",  0x380000, CRC(62b2783a) SHA1(d1ba043a2ec9390c2f281f53de6e59bd936ea6f6) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "atshape.bin",  0x000000, CRC(dfcad82b) SHA1(9c3826b8dc36fa0d71c0de7f8be3479d9a025803) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data ROMs */
	NAMCOS2_DATA_LOAD_E_128K( "at1dat0.13s",  0x000000, CRC(844890f4) SHA1(1be30760acd81fae836301d81d6adbb3e5941373) )
	NAMCOS2_DATA_LOAD_O_128K( "at1dat1.13p",  0x000000, CRC(21715313) SHA1(97c6edae6a5f1df434f1dcf7be307b5e006e72a6) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "atvoi1.bin",  0x000000, 0x080000, CRC(d36a649e) SHA1(30173f32c6ec9dda6b8946baa14266e828b0324e) )
ROM_END

/* ASSAULT (JAPAN) */
ROM_START( assaultj )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "at1_mp0.bin",  0x000000, 0x010000, CRC(2d3e5c8c) SHA1(e43081a94c0d35fcb9ef48e248be59f0dd390b9b) )
	ROM_LOAD16_BYTE( "at1_mp1.bin",  0x000001, 0x010000, CRC(851cec3a) SHA1(75afae79e1b5843d765677b457f38bed8eebf07c) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "at1sp0.bin",  0x000000, 0x010000, CRC(0de2a0da) SHA1(5f6ce61f2ad1e50f0eac3a115c511151bbb00fd2) )
	ROM_LOAD16_BYTE( "at1sp1.bin",  0x000001, 0x010000, CRC(02d15fbe) SHA1(7c1bc845a06188ebe2efdb5880562beae11a7b4f) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "at1snd0.bin",  0x000000, 0x020000, CRC(1d1ffe12) SHA1(5a49bf1eef981df69abac28132af65670ba01118) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65b.bin",  0x000000, 0x008000, CRC(e9f2922a) SHA1(5767d2f85e1eb3de19192e73b02221f28b1fbb83) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	NAMCOS2_SPRROM_LOAD_128K( "atobj0.bin",  0x000003, CRC(22240076) SHA1(916fc0e6b338a6dda84399df910c3c9463e6b915) )
	NAMCOS2_SPRROM_LOAD_128K( "atobj1.bin",  0x000002, CRC(2284a8e8) SHA1(80f9143e08f9f8ff3e937312a8ce76855a1929ad) )
	NAMCOS2_SPRROM_LOAD_128K( "atobj2.bin",  0x000001, CRC(51425476) SHA1(12a2fb1b61adfa4c21a5af4f206ffe48a045a953) )
	NAMCOS2_SPRROM_LOAD_128K( "atobj3.bin",  0x000000, CRC(791f42ce) SHA1(95583130abe2e6f9ad3e96288d811b4abc3d44b3) )
	NAMCOS2_SPRROM_LOAD_128K( "atobj4.bin",  0x200003, CRC(4782e1b0) SHA1(3d9f4b9eb711fb47e424cd57f7183f49f5dd6ec4) )
	NAMCOS2_SPRROM_LOAD_128K( "atobj5.bin",  0x200002, CRC(f5d158cf) SHA1(f05f44915afe3c17fff0b85a0364f70e79b25428) )
	NAMCOS2_SPRROM_LOAD_128K( "atobj6.bin",  0x200001, CRC(12f6a569) SHA1(e3051de0961f34e15b8642fa769deac3cb0c8305) )
	NAMCOS2_SPRROM_LOAD_128K( "atobj7.bin",  0x200000, CRC(06a929f2) SHA1(65308972a27ab4a649fd08414a89e6f97a09240e) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "atchr0.bin",  0x000000, CRC(6f8e968a) SHA1(b771359a3b08c1aeeb248eff325b19238bb88bf8) )
	NAMCOS2_GFXROM_LOAD_128K( "atchr1.bin",  0x080000, CRC(88cf7cbe) SHA1(df7565cb36658311b784d5406dc6f83141af58a9) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "atroz0.bin",  0x000000, CRC(8c247a97) SHA1(e06dc0b46fa06f41d2017ec8113baf5c4ba832ab) )
	NAMCOS2_GFXROM_LOAD_128K( "atroz1.bin",  0x080000, CRC(e44c475b) SHA1(64768692a8d38377fa1109cd9f7c86b31c01e13c) )
	NAMCOS2_GFXROM_LOAD_128K( "atroz2.bin",  0x100000, CRC(770f377f) SHA1(3a7d4f4b9eb01d4366119f34b0d7d7db2f5a2471) )
	NAMCOS2_GFXROM_LOAD_128K( "atroz3.bin",  0x180000, CRC(01d93d0b) SHA1(47dfda4720f0d4cd41633fd0291b82a54a4eb310) )
	NAMCOS2_GFXROM_LOAD_128K( "atroz4.bin",  0x200000, CRC(f96feab5) SHA1(c861df1f66de046ca714087630f5a1e2f3f30361) )
	NAMCOS2_GFXROM_LOAD_128K( "atroz5.bin",  0x280000, CRC(da2f0d9e) SHA1(a7fbe4748c67906a1ab2902d3b75a21219a677c0) )
	NAMCOS2_GFXROM_LOAD_128K( "atroz6.bin",  0x300000, CRC(9089e477) SHA1(929d7d81c05aa148212e22969d147296811d433c) )
	NAMCOS2_GFXROM_LOAD_128K( "atroz7.bin",  0x380000, CRC(62b2783a) SHA1(d1ba043a2ec9390c2f281f53de6e59bd936ea6f6) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "atshape.bin",  0x000000, CRC(dfcad82b) SHA1(9c3826b8dc36fa0d71c0de7f8be3479d9a025803) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data ROMs */
	NAMCOS2_DATA_LOAD_E_128K( "at1dat0.13s",  0x000000, CRC(844890f4) SHA1(1be30760acd81fae836301d81d6adbb3e5941373) )
	NAMCOS2_DATA_LOAD_O_128K( "at1dat1.13p",  0x000000, CRC(21715313) SHA1(97c6edae6a5f1df434f1dcf7be307b5e006e72a6) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "atvoi1.bin",  0x000000, 0x080000, CRC(d36a649e) SHA1(30173f32c6ec9dda6b8946baa14266e828b0324e) )
ROM_END

/* ASSAULT PLUS (NAMCO) */
ROM_START( assaultp )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "mpr0.bin",    0x000000, 0x010000, CRC(97519f9f) SHA1(760580c0865931f18f4b0b4f609379c0dbeff977) )
	ROM_LOAD16_BYTE( "mpr1.bin",    0x000001, 0x010000, CRC(c7f437c7) SHA1(f56d908ed557192cd6d583eccd5ded33807e73a9) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "at1sp0.bin",  0x000000, 0x010000, CRC(0de2a0da) SHA1(5f6ce61f2ad1e50f0eac3a115c511151bbb00fd2) )
	ROM_LOAD16_BYTE( "at1sp1.bin",  0x000001, 0x010000, CRC(02d15fbe) SHA1(7c1bc845a06188ebe2efdb5880562beae11a7b4f) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "at1snd0.bin",  0x000000, 0x020000, CRC(1d1ffe12) SHA1(5a49bf1eef981df69abac28132af65670ba01118) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65b.bin",  0x000000, 0x008000, CRC(e9f2922a) SHA1(5767d2f85e1eb3de19192e73b02221f28b1fbb83) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	NAMCOS2_SPRROM_LOAD_128K( "atobj0.bin",  0x000003, CRC(22240076) SHA1(916fc0e6b338a6dda84399df910c3c9463e6b915) )
	NAMCOS2_SPRROM_LOAD_128K( "atobj1.bin",  0x000002, CRC(2284a8e8) SHA1(80f9143e08f9f8ff3e937312a8ce76855a1929ad) )
	NAMCOS2_SPRROM_LOAD_128K( "atobj2.bin",  0x000001, CRC(51425476) SHA1(12a2fb1b61adfa4c21a5af4f206ffe48a045a953) )
	NAMCOS2_SPRROM_LOAD_128K( "atobj3.bin",  0x000000, CRC(791f42ce) SHA1(95583130abe2e6f9ad3e96288d811b4abc3d44b3) )
	NAMCOS2_SPRROM_LOAD_128K( "atobj4.bin",  0x200003, CRC(4782e1b0) SHA1(3d9f4b9eb711fb47e424cd57f7183f49f5dd6ec4) )
	NAMCOS2_SPRROM_LOAD_128K( "atobj5.bin",  0x200002, CRC(f5d158cf) SHA1(f05f44915afe3c17fff0b85a0364f70e79b25428) )
	NAMCOS2_SPRROM_LOAD_128K( "atobj6.bin",  0x200001, CRC(12f6a569) SHA1(e3051de0961f34e15b8642fa769deac3cb0c8305) )
	NAMCOS2_SPRROM_LOAD_128K( "atobj7.bin",  0x200000, CRC(06a929f2) SHA1(65308972a27ab4a649fd08414a89e6f97a09240e) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "atchr0.bin",  0x000000, CRC(6f8e968a) SHA1(b771359a3b08c1aeeb248eff325b19238bb88bf8) )
	NAMCOS2_GFXROM_LOAD_128K( "atchr1.bin",  0x080000, CRC(88cf7cbe) SHA1(df7565cb36658311b784d5406dc6f83141af58a9) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "atroz0.bin",  0x000000, CRC(8c247a97) SHA1(e06dc0b46fa06f41d2017ec8113baf5c4ba832ab) )
	NAMCOS2_GFXROM_LOAD_128K( "atroz1.bin",  0x080000, CRC(e44c475b) SHA1(64768692a8d38377fa1109cd9f7c86b31c01e13c) )
	NAMCOS2_GFXROM_LOAD_128K( "atroz2.bin",  0x100000, CRC(770f377f) SHA1(3a7d4f4b9eb01d4366119f34b0d7d7db2f5a2471) )
	NAMCOS2_GFXROM_LOAD_128K( "atroz3.bin",  0x180000, CRC(01d93d0b) SHA1(47dfda4720f0d4cd41633fd0291b82a54a4eb310) )
	NAMCOS2_GFXROM_LOAD_128K( "atroz4.bin",  0x200000, CRC(f96feab5) SHA1(c861df1f66de046ca714087630f5a1e2f3f30361) )
	NAMCOS2_GFXROM_LOAD_128K( "atroz5.bin",  0x280000, CRC(da2f0d9e) SHA1(a7fbe4748c67906a1ab2902d3b75a21219a677c0) )
	NAMCOS2_GFXROM_LOAD_128K( "atroz6.bin",  0x300000, CRC(9089e477) SHA1(929d7d81c05aa148212e22969d147296811d433c) )
	NAMCOS2_GFXROM_LOAD_128K( "atroz7.bin",  0x380000, CRC(62b2783a) SHA1(d1ba043a2ec9390c2f281f53de6e59bd936ea6f6) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "atshape.bin",  0x000000, CRC(dfcad82b) SHA1(9c3826b8dc36fa0d71c0de7f8be3479d9a025803) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data ROMs */
	NAMCOS2_DATA_LOAD_E_128K( "at1dat0.13s",  0x000000, CRC(844890f4) SHA1(1be30760acd81fae836301d81d6adbb3e5941373) )
	NAMCOS2_DATA_LOAD_O_128K( "at1dat1.13p",  0x000000, CRC(21715313) SHA1(97c6edae6a5f1df434f1dcf7be307b5e006e72a6) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "atvoi1.bin",  0x000000, 0x080000, CRC(d36a649e) SHA1(30173f32c6ec9dda6b8946baa14266e828b0324e) )
ROM_END

/* BURNING FORCE */
ROM_START( burnforc )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "bu1_mpr0c.bin",  0x000000, 0x020000, CRC(cc5864c6) SHA1(278b02b634ca07b066a817ed38a8557bcb13aa78) )
	ROM_LOAD16_BYTE( "bu1_mpr1c.bin",  0x000001, 0x020000, CRC(3e6b4b1b) SHA1(c765e1ba5873239c4d440678baaac6e95a410339) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "bu1_spr0.bin",  0x000000, 0x010000, CRC(17022a21) SHA1(331fb1a8e97f21bd5e0a5889fc960b1d187f1ee1) )
	ROM_LOAD16_BYTE( "bu1_spr1.bin",  0x000001, 0x010000, CRC(5255f8a5) SHA1(e2253b98a1b7b7c761ea71c201320711dc7d2e32) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "bu1_snd0.bin",  0x000000, 0x020000, CRC(fabb1150) SHA1(61e3133adf8d593419f70a854ff91dd5011de3b3) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "bu_obj-0.bin",  0x000003, 0x80000, CRC(24c919a1) SHA1(ddf5bfbf1bbe2a10d6708b618b77f1d6d7862372) )
	ROM_LOAD32_BYTE( "bu_obj-1.bin",  0x000002, 0x80000, CRC(5bcb519b) SHA1(1d2979a4bed7e952ec77d3a5891a6412044d5f49) )
	ROM_LOAD32_BYTE( "bu_obj-2.bin",  0x000001, 0x80000, CRC(509dd5d0) SHA1(68a9054fcde7b677f529ef4db6a8b29750649a2a) )
	ROM_LOAD32_BYTE( "bu_obj-3.bin",  0x000000, 0x80000, CRC(270a161e) SHA1(e26092b6950e2adba34f0c5c08179b83fcd86949) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "bu_chr-0.bin",  0x000000, CRC(c2109f73) SHA1(5f09aa9afb027850f21175614c24071db8c754b5) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_chr-1.bin",  0x080000, CRC(67d6aa67) SHA1(ac7791e4984c6e736ab12e538d856e4fc63383b4) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_chr-2.bin",  0x100000, CRC(52846eff) SHA1(f925512f382a51040401d5833015cce17b8eb1fd) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_chr-3.bin",  0x180000, CRC(d1326d7f) SHA1(fe2138ce42cf19a214b07f1e7f59cd313c4f7775) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_chr-4.bin",  0x200000, CRC(81a66286) SHA1(650273269cba96f223413ec04928408378170f62) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_chr-5.bin",  0x280000, CRC(629aa67f) SHA1(f626b2faf8a1eed1a59124e74fbc05cd99dfb1d9) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "bu_roz-0.bin",  0x000000, CRC(65fefc83) SHA1(cf11e62476ca41f76b685011f94234769e37bf5f) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_roz-1.bin",  0x080000, CRC(979580c2) SHA1(cac8342fd4c41084ce6f578206a9c20d399babde) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_roz-2.bin",  0x100000, CRC(548b6ad8) SHA1(170113dc6f61ad737f72f58a29ea61347f819ddb) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_roz-3.bin",  0x180000, CRC(a633cea0) SHA1(c729679e95fcb73edd9a309b90c40e0e6a12777e) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_roz-4.bin",  0x200000, CRC(1b1f56a6) SHA1(11164fce62724fdd3074abf1c364e299ad092b8e) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_roz-5.bin",  0x280000, CRC(4b864b0e) SHA1(a0b23f49fc85d39bc9dc950ac763fe12454dcdb1) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_roz-6.bin",  0x300000, CRC(38bd25ba) SHA1(8eb6ba969660ab62102ded2f0c39837fd90f3f7d) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "bu_shape.bin",  0x000000,CRC(80a6b722) SHA1(2c24327a890310c5e8086dc6821627108a88c62e) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data ROMs */
	NAMCOS2_DATA_LOAD_E_128K( "bu1_dat0.13s",  0x000000, CRC(e0a9d92f) SHA1(15042e6d7b31bec08ccdf36e89fdb4b6fb62fa4b) )
	NAMCOS2_DATA_LOAD_O_128K( "bu1_dat1.13p",  0x000000, CRC(5fe54b73) SHA1(a5d4895f0a4523be20de40ccaa74f8fad0d5df7d) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "bu_voi-1.bin",  0x000000, 0x080000, CRC(99d8a239) SHA1(1ebc586048e757ac0ac68dc9cc171f4849e67cef) )
ROM_END

ROM_START( burnforco )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "bu1_mpr0.bin",  0x000000, 0x020000, CRC(096b73e2) SHA1(ed96ca336dd0ce368f6b07cf1a80c55cf16919c9) )
	ROM_LOAD16_BYTE( "bu1_mpr1.bin",  0x000001, 0x020000, CRC(7ead4cbf) SHA1(ff8cf722e57fbe9e035cc542180806fe045206f5) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "bu1_spr0.bin",  0x000000, 0x010000, CRC(17022a21) SHA1(331fb1a8e97f21bd5e0a5889fc960b1d187f1ee1) )
	ROM_LOAD16_BYTE( "bu1_spr1.bin",  0x000001, 0x010000, CRC(5255f8a5) SHA1(e2253b98a1b7b7c761ea71c201320711dc7d2e32) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "bu1_snd0.bin",  0x000000, 0x020000, CRC(fabb1150) SHA1(61e3133adf8d593419f70a854ff91dd5011de3b3) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "bu_obj-0.bin",  0x000003, 0x80000, CRC(24c919a1) SHA1(ddf5bfbf1bbe2a10d6708b618b77f1d6d7862372) )
	ROM_LOAD32_BYTE( "bu_obj-1.bin",  0x000002, 0x80000, CRC(5bcb519b) SHA1(1d2979a4bed7e952ec77d3a5891a6412044d5f49) )
	ROM_LOAD32_BYTE( "bu_obj-2.bin",  0x000001, 0x80000, CRC(509dd5d0) SHA1(68a9054fcde7b677f529ef4db6a8b29750649a2a) )
	ROM_LOAD32_BYTE( "bu_obj-3.bin",  0x000000, 0x80000, CRC(270a161e) SHA1(e26092b6950e2adba34f0c5c08179b83fcd86949) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "bu_chr-0.bin",  0x000000, CRC(c2109f73) SHA1(5f09aa9afb027850f21175614c24071db8c754b5) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_chr-1.bin",  0x080000, CRC(67d6aa67) SHA1(ac7791e4984c6e736ab12e538d856e4fc63383b4) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_chr-2.bin",  0x100000, CRC(52846eff) SHA1(f925512f382a51040401d5833015cce17b8eb1fd) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_chr-3.bin",  0x180000, CRC(d1326d7f) SHA1(fe2138ce42cf19a214b07f1e7f59cd313c4f7775) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_chr-4.bin",  0x200000, CRC(81a66286) SHA1(650273269cba96f223413ec04928408378170f62) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_chr-5.bin",  0x280000, CRC(629aa67f) SHA1(f626b2faf8a1eed1a59124e74fbc05cd99dfb1d9) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "bu_roz-0.bin",  0x000000, CRC(65fefc83) SHA1(cf11e62476ca41f76b685011f94234769e37bf5f) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_roz-1.bin",  0x080000, CRC(979580c2) SHA1(cac8342fd4c41084ce6f578206a9c20d399babde) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_roz-2.bin",  0x100000, CRC(548b6ad8) SHA1(170113dc6f61ad737f72f58a29ea61347f819ddb) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_roz-3.bin",  0x180000, CRC(a633cea0) SHA1(c729679e95fcb73edd9a309b90c40e0e6a12777e) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_roz-4.bin",  0x200000, CRC(1b1f56a6) SHA1(11164fce62724fdd3074abf1c364e299ad092b8e) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_roz-5.bin",  0x280000, CRC(4b864b0e) SHA1(a0b23f49fc85d39bc9dc950ac763fe12454dcdb1) )
	NAMCOS2_GFXROM_LOAD_128K( "bu_roz-6.bin",  0x300000, CRC(38bd25ba) SHA1(8eb6ba969660ab62102ded2f0c39837fd90f3f7d) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "bu_shape.bin",  0x000000,CRC(80a6b722) SHA1(2c24327a890310c5e8086dc6821627108a88c62e) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data ROMs */
	NAMCOS2_DATA_LOAD_E_128K( "bu1_dat0.13s",  0x000000, CRC(e0a9d92f) SHA1(15042e6d7b31bec08ccdf36e89fdb4b6fb62fa4b) )
	NAMCOS2_DATA_LOAD_O_128K( "bu1_dat1.13p",  0x000000, CRC(5fe54b73) SHA1(a5d4895f0a4523be20de40ccaa74f8fad0d5df7d) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "bu_voi-1.bin",  0x000000, 0x080000, CRC(99d8a239) SHA1(1ebc586048e757ac0ac68dc9cc171f4849e67cef) )
ROM_END

/* COSMO GANG THE VIDEO (USA) */
ROM_START( cosmogng )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "co2mpr0.bin",  0x000000, 0x020000, CRC(2632c209) SHA1(a03136c18eff4e3d69dda955a3c774f2829393ec) )
	ROM_LOAD16_BYTE( "co2mpr1.bin",  0x000001, 0x020000, CRC(65840104) SHA1(71dfd379a985779e11e9e321ee998df21d03c30e) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "co1spr0.bin",  0x000000, 0x020000, CRC(bba2c28f) SHA1(5adf4eb9d24833e462a240253ab9ac0205cb6501) )
	ROM_LOAD16_BYTE( "co1spr1.bin",  0x000001, 0x020000, CRC(c029b459) SHA1(b92c0d24449b8db352bbbd782dc43af0640479e5) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "co2_s0",  0x000000, 0x020000, CRC(4ca59338) SHA1(95916977ab4e0704216984ceed66df7b9e1fdf44) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "co1obj0.bin",  0x000003, 0x80000, CRC(5df8ce0c) SHA1(afb9fb6e048af5aed8976192b847c0674c5e5ce1) )
	ROM_LOAD32_BYTE( "co1obj1.bin",  0x000002, 0x80000, CRC(3d152497) SHA1(70c6725cacf86ba4d4b9dbeed7a1e04df9301228) )
	ROM_LOAD32_BYTE( "co1obj2.bin",  0x000001, 0x80000, CRC(4e50b6ee) SHA1(0fd4c19fa77ba6774237c760ac1096d4806248dd) )
	ROM_LOAD32_BYTE( "co1obj3.bin",  0x000000, 0x80000, CRC(7beed669) SHA1(92e5eb2a8de3ff71c002807f31581a79a5db5422) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "co1chr0.bin",  0x000000, 0x80000, CRC(ee375b3e) SHA1(e7cc3a137450a79c6068c3bf2c15149f6f6dd18a) )
	ROM_LOAD( "co1chr1.bin",  0x080000, 0x80000, CRC(0149de65) SHA1(36d42e56251c850e26ee2253cebf62682ac7516f) )
	ROM_LOAD( "co1chr2.bin",  0x100000, 0x80000, CRC(93d565a0) SHA1(c6e993dc77c5a30daee5de4363454e2ccf3bd02d) )
	ROM_LOAD( "co1chr3.bin",  0x180000, 0x80000, CRC(4d971364) SHA1(579f7e37bfa37b8b152ac44cdfe712c2ec2cda8b) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	ROM_LOAD( "co1roz0.bin",  0x000000, 0x80000, CRC(2bea6951) SHA1(273e3e8ccb042ec794b1709d6626603c8a39a73a) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	ROM_LOAD( "co1sha0.bin",  0x000000, 0x80000, CRC(063a70cc) SHA1(c3179d55d57c47d3fef49d45e45b88c4d8250548) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data ROMs */
	NAMCOS2_DATA_LOAD_E_128K( "co1dat0.13s",  0x000000, CRC(b53da2ae) SHA1(a7fe63668d50928d5d2e2249a5f377c7e8dfc6a5) )
	NAMCOS2_DATA_LOAD_O_128K( "co1dat1.13p",  0x000000, CRC(d21ad10b) SHA1(dcf2d4cc048ea57507952a9a35390af7de5cfe34) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "co2voi1.bin",  0x000000, 0x080000, CRC(5a301349) SHA1(e333ea5955a66ac8d7c94cd50047efaf6fa95b15) )
	ROM_LOAD16_BYTE( "co2voi2.bin",  0x100000, 0x080000, CRC(a27cb45a) SHA1(08ccaaf43369e8358e31b213877829bdfd61479e) )

	ROM_REGION( 0x2000, "user2", 0 ) /* zoom */
	ROM_LOAD( "04544191.6n", 0, 0x2000, CRC(90db1bf6) SHA1(dbb9e50a8efc3b4012fcf587cc87da9ef42a1b80) )
ROM_END

/* COSMO GANG THE VIDEO (JAPAN) */
ROM_START( cosmogngj )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "co1mpr0.bin",  0x000000, 0x020000, CRC(d1b4c8db) SHA1(d14974edc458b95cca02defef073804f1af0f5de) )
	ROM_LOAD16_BYTE( "co1mpr1.bin",  0x000001, 0x020000, CRC(2f391906) SHA1(d97e58eecaca84127caa3bc55d6aa256628e35d7) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "co1spr0.bin",  0x000000, 0x020000, CRC(bba2c28f) SHA1(5adf4eb9d24833e462a240253ab9ac0205cb6501) )
	ROM_LOAD16_BYTE( "co1spr1.bin",  0x000001, 0x020000, CRC(c029b459) SHA1(b92c0d24449b8db352bbbd782dc43af0640479e5) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "co1snd0.bin",  0x000000, 0x020000, CRC(6bfa619f) SHA1(71e57adf320034d6cacd9f4a9e93891c8143542f) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "co1obj0.bin",  0x000003, 0x80000, CRC(5df8ce0c) SHA1(afb9fb6e048af5aed8976192b847c0674c5e5ce1) )
	ROM_LOAD32_BYTE( "co1obj1.bin",  0x000002, 0x80000, CRC(3d152497) SHA1(70c6725cacf86ba4d4b9dbeed7a1e04df9301228) )
	ROM_LOAD32_BYTE( "co1obj2.bin",  0x000001, 0x80000, CRC(4e50b6ee) SHA1(0fd4c19fa77ba6774237c760ac1096d4806248dd) )
	ROM_LOAD32_BYTE( "co1obj3.bin",  0x000000, 0x80000, CRC(7beed669) SHA1(92e5eb2a8de3ff71c002807f31581a79a5db5422) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "co1chr0.bin",  0x000000, 0x80000, CRC(ee375b3e) SHA1(e7cc3a137450a79c6068c3bf2c15149f6f6dd18a) )
	ROM_LOAD( "co1chr1.bin",  0x080000, 0x80000, CRC(0149de65) SHA1(36d42e56251c850e26ee2253cebf62682ac7516f) )
	ROM_LOAD( "co1chr2.bin",  0x100000, 0x80000, CRC(93d565a0) SHA1(c6e993dc77c5a30daee5de4363454e2ccf3bd02d) )
	ROM_LOAD( "co1chr3.bin",  0x180000, 0x80000, CRC(4d971364) SHA1(579f7e37bfa37b8b152ac44cdfe712c2ec2cda8b) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	ROM_LOAD( "co1roz0.bin",  0x000000, 0x80000, CRC(2bea6951) SHA1(273e3e8ccb042ec794b1709d6626603c8a39a73a) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	ROM_LOAD( "co1sha0.bin",  0x000000, 0x80000, CRC(063a70cc) SHA1(c3179d55d57c47d3fef49d45e45b88c4d8250548) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data ROMs */
	NAMCOS2_DATA_LOAD_E_128K( "co1dat0.13s",  0x000000, CRC(b53da2ae) SHA1(a7fe63668d50928d5d2e2249a5f377c7e8dfc6a5) )
	NAMCOS2_DATA_LOAD_O_128K( "co1dat1.13p",  0x000000, CRC(d21ad10b) SHA1(dcf2d4cc048ea57507952a9a35390af7de5cfe34) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "co1voi1.bin",  0x000000, 0x080000, CRC(b5ba8f15) SHA1(9e54b9ba1cd44353782adf337376dff9eec4e937) )
	ROM_LOAD16_BYTE( "co1voi2.bin",  0x100000, 0x080000, CRC(b566b105) SHA1(b5530b0f3dea0135f28419044aee923d855f382c) )

	ROM_REGION( 0x2000, "user2", 0 ) /* zoom */
	ROM_LOAD( "04544191.6n", 0, 0x2000, CRC(90db1bf6) SHA1(dbb9e50a8efc3b4012fcf587cc87da9ef42a1b80) )
ROM_END

/* DIRT FOX (JAPAN) */
ROM_START( dirtfoxj )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "df1_mpr0.bin",  0x000000, 0x020000, CRC(8386c820) SHA1(a30f4e1b34d3a8294b42960fee6753c4a157e0a0) )
	ROM_LOAD16_BYTE( "df1_mpr1.bin",  0x000001, 0x020000, CRC(51085728) SHA1(04ce7bd625f50bdf8f204d89b873072e5f48de20) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "df1_spr0.bin",  0x000000, 0x020000, CRC(d4906585) SHA1(b205663f60f682f2e645c2a0118c7bf14d2b0651) )
	ROM_LOAD16_BYTE( "df1_spr1.bin",  0x000001, 0x020000, CRC(7d76cf57) SHA1(cd13deb8131f87fd9bbf5caa8b3d6f88bc8d4d7d) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "df1_snd0.bin",  0x000000, 0x020000, CRC(66b4f3ab) SHA1(b165b2b222c0ce62a16f09e4a0741ca8459b624d) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "df1_obj0.bin",  0x000003, 0x80000, CRC(b6bd1a68) SHA1(38677b54cd257411db499ba03b9176422797bf64) )
	ROM_LOAD32_BYTE( "df1_obj1.bin",  0x000002, 0x80000, CRC(05421dc1) SHA1(d538bb33b1ec1a3ad0feaa75d69a7a327c7dc6fa) )
	ROM_LOAD32_BYTE( "df1_obj2.bin",  0x000001, 0x80000, CRC(9390633e) SHA1(91d1a7f2c981c893e4c5d0c6c7199646b86bd1e0) )
	ROM_LOAD32_BYTE( "df1_obj3.bin",  0x000000, 0x80000, CRC(c8447b33) SHA1(1f62af3a8b16915adf993ed675cba368f13d4acf) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "df1_chr0.bin",  0x000000, CRC(4b10e4ed) SHA1(b3c56f712b05837590d25dfa7535b0f63cbd61c5) )
	NAMCOS2_GFXROM_LOAD_128K( "df1_chr1.bin",  0x080000, CRC(8f63f3d6) SHA1(4432b611550e6890bc351a0db7a90deabe489824) )
	NAMCOS2_GFXROM_LOAD_128K( "df1_chr2.bin",  0x100000, CRC(5a1b852a) SHA1(84cbfc04614ed85fdf0efe5ab10d1b6e86bea028) )
	NAMCOS2_GFXROM_LOAD_128K( "df1_chr3.bin",  0x180000, CRC(28570676) SHA1(2a0c90839fda6153f6fe42a759d51293998034e2) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	NAMCOS2_GFXROM_LOAD_256K( "df1_roz0.bin",  0x000000, CRC(a6129f94) SHA1(5c6f36dce703d985704676948999c81374ac1856) )
	NAMCOS2_GFXROM_LOAD_256K( "df1_roz1.bin",  0x080000, CRC(c8e7ce73) SHA1(7e113a56c6c115c251bf8b17d763c8955757edd2) )
	NAMCOS2_GFXROM_LOAD_256K( "df1_roz2.bin",  0x100000, CRC(c598e923) SHA1(67b8d74c7a44a896d842271842bb5d2b26d565be) )
	NAMCOS2_GFXROM_LOAD_256K( "df1_roz3.bin",  0x180000, CRC(5a38b062) SHA1(ed80b2abcd8d271a42426f8282a9d413c2329705) )
	NAMCOS2_GFXROM_LOAD_256K( "df1_roz4.bin",  0x200000, CRC(e196d2e8) SHA1(bc2756c37209766681b31fcef867462a888b2deb) )
	NAMCOS2_GFXROM_LOAD_256K( "df1_roz5.bin",  0x280000, CRC(1f8a1a3c) SHA1(e0c381685135857fb5b81e5e1259f1bec018e2c1) )
	NAMCOS2_GFXROM_LOAD_256K( "df1_roz6.bin",  0x300000, CRC(7f3a1ed9) SHA1(df62bc3a236046b73b296cb396f6e7ee6861de09) )
	NAMCOS2_GFXROM_LOAD_256K( "df1_roz7.bin",  0x380000, CRC(dd546ae8) SHA1(a5b50b4e8027027ec272ed5c71c7bff4b03e9f92) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "df1_sha.bin",  0x000000, CRC(9a7c9a9b) SHA1(06221ae8d3f6bebbb5a7ab2eaaf35b9922389115) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data ROMs */
	NAMCOS2_DATA_LOAD_E_256K( "df1_dat0.13s",  0x000000, CRC(f5851c85) SHA1(e99c05891622cdaab394630b7b2678968e6761d7) )
	NAMCOS2_DATA_LOAD_O_256K( "df1_dat1.13p",  0x000000, CRC(1a31e46b) SHA1(4be7115893b27d6a3dc38c97dcb41eafebb423cd) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "df1_voi1.bin",  0x000000, 0x080000, CRC(15053904) SHA1(b8ca7e5e53249dbee8284ce1e5c0e6438e64b2cf) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "nvram",  0x000000, 0x2000, CRC(4b9f7b06) SHA1(384496d2d80a48d31084dc316ebae3a5c1aa1ab9) )
ROM_END

/* DRAGON SABER */
ROM_START( dsaber )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "do2 mpr0.mpr0",  0x000000, 0x020000, CRC(a4c9ff34) SHA1(bf9310f1ab2f45ec01d6fa19e7d2e1d2a802dd2a) )
	ROM_LOAD16_BYTE( "do2 mpr1.mpr1",  0x000001, 0x020000, CRC(2a89e794) SHA1(d9548e14a915b7bfab87292f03bdfd884438b49b) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "do1 spr0.spr0",  0x000000, 0x010000, CRC(013faf80) SHA1(2cb086e9bb6a551e9fc185c40b976989f758cc57) )
	ROM_LOAD16_BYTE( "do1 spr1.spr1",  0x000001, 0x010000, CRC(c36242bb) SHA1(776f0ab4b2798bc7fc293e27a455675c01ff3132) )

	ROM_REGION( 0x040000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "do1 snd0.snd0",  0x000000, 0x020000, CRC(af5b1ff8) SHA1(fc2a104788f081e4d2ddd22c586e541d6c5b5d69) )
	ROM_LOAD( "do1 snd1.snd1",  0x020000, 0x020000, CRC(c4ca6f3f) SHA1(829a053451be07b296fb4d97818d59eb1e68c807) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "do obj-0a.obj0",  0x000003, 0x80000, CRC(f08c6648) SHA1(ac5221ba159f2390060cbbb7d9cd8148c7bb4a02) )
	ROM_LOAD32_BYTE( "do obj-1a.obj1",  0x000002, 0x80000, CRC(34e0810d) SHA1(679d9b82879cff5197a5098e5dc724c85373b9dc) )
	ROM_LOAD32_BYTE( "do obj-2a.obj2",  0x000001, 0x80000, CRC(bccdabf3) SHA1(d079d89083ac6e71ac8926792d0d7cdcebc848a9) )
	ROM_LOAD32_BYTE( "do obj-3a.obj3",  0x000000, 0x80000, CRC(2a60a4b8) SHA1(5923e08121ad27629bd917d890e037e888e6d356) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "do chr-0a.chr0",  0x000000, 0x80000, CRC(c6058df6) SHA1(13bacad6d593aa5533161e410e22f351c77f29c4) )
	ROM_LOAD( "do chr-1a.chr1",  0x080000, 0x80000, CRC(67aaab36) SHA1(3abb7e226badcfe016325d42c40f06ee020124e3) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	ROM_LOAD( "roz0.bin",  0x000000, 0x80000, CRC(32aab758) SHA1(a3220c2a02b9d8bdd95004d36d2aa5ddf57adfbb) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	ROM_LOAD( "shape.bin",  0x000000, 0x80000, CRC(698e7a3e) SHA1(4d41bf0242626ca1448d1f650c84b5987a7f6597) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data ROMs */
	NAMCOS2_DATA_LOAD_E_128K( "do1 dat0.13s",  0x000000, CRC(3e53331f) SHA1(3dd4c133f587361f30ab1b890f5b05749d5838e3) )
	NAMCOS2_DATA_LOAD_O_128K( "do1 dat1.13p",  0x000000, CRC(d5427f11) SHA1(af8d8153dc60044616a6b0571831c53c09fefda1) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "do voi-1a.voice1",  0x000000, 0x080000, CRC(dadf6a57) SHA1(caba21fc6b62d140f6d8231411ce82ae0ad2837a) )
	ROM_LOAD16_BYTE( "do voi-2a.voice2",  0x100000, 0x080000, CRC(81078e01) SHA1(adc70506b21b9a12eadd2f3fd1e920c2eb27c36e) )

	ROM_REGION( 0x0500, "plds", 0 )
	ROM_LOAD( "pal16l8a.4g", 0x0000, 0x0104, CRC(660e1655) SHA1(ffb43238c5ffa3fa831975bc3cde72334c4c2540) )
	ROM_LOAD( "pal16l8a.5f", 0x0200, 0x0104, CRC(18f43c22) SHA1(72849c5b842678bb9037541d26d4c99cdf879982) )
	ROM_LOAD( "pal12l10.8d", 0x0400, 0x0040, CRC(e2379249) SHA1(ad4cdf2e0fd1304a135022eeafa2f61c5f5789cd) )
ROM_END


/* DRAGON SABER */
ROM_START( dsabera )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "mpr0.bin",  0x000000, 0x020000, CRC(45309ddc) SHA1(53b63046b0e5178695c998a0956667f4cf4204f2) )
	ROM_LOAD16_BYTE( "mpr1.bin",  0x000001, 0x020000, CRC(cbfc4cba) SHA1(2037119f28af2b6a4af9e2019506ad7fccfa3113) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "do1 spr0.spr0",  0x000000, 0x010000, CRC(013faf80) SHA1(2cb086e9bb6a551e9fc185c40b976989f758cc57) )
	ROM_LOAD16_BYTE( "do1 spr1.spr1",  0x000001, 0x010000, CRC(c36242bb) SHA1(776f0ab4b2798bc7fc293e27a455675c01ff3132) )

	ROM_REGION( 0x040000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "do1 snd0.snd0",  0x000000, 0x020000, CRC(af5b1ff8) SHA1(fc2a104788f081e4d2ddd22c586e541d6c5b5d69) )
	ROM_LOAD( "do1 snd1.snd1",  0x020000, 0x020000, CRC(c4ca6f3f) SHA1(829a053451be07b296fb4d97818d59eb1e68c807) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "do obj-0a.obj0",  0x000003, 0x80000, CRC(f08c6648) SHA1(ac5221ba159f2390060cbbb7d9cd8148c7bb4a02) )
	ROM_LOAD32_BYTE( "do obj-1a.obj1",  0x000002, 0x80000, CRC(34e0810d) SHA1(679d9b82879cff5197a5098e5dc724c85373b9dc) )
	ROM_LOAD32_BYTE( "do obj-2a.obj2",  0x000001, 0x80000, CRC(bccdabf3) SHA1(d079d89083ac6e71ac8926792d0d7cdcebc848a9) )
	ROM_LOAD32_BYTE( "do obj-3a.obj3",  0x000000, 0x80000, CRC(2a60a4b8) SHA1(5923e08121ad27629bd917d890e037e888e6d356) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "do chr-0a.chr0",  0x000000, 0x80000, CRC(c6058df6) SHA1(13bacad6d593aa5533161e410e22f351c77f29c4) )
	ROM_LOAD( "do chr-1a.chr1",  0x080000, 0x80000, CRC(67aaab36) SHA1(3abb7e226badcfe016325d42c40f06ee020124e3) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	ROM_LOAD( "roz0.bin",  0x000000, 0x80000, CRC(32aab758) SHA1(a3220c2a02b9d8bdd95004d36d2aa5ddf57adfbb) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	ROM_LOAD( "shape.bin",  0x000000, 0x80000, CRC(698e7a3e) SHA1(4d41bf0242626ca1448d1f650c84b5987a7f6597) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data ROMs */
	NAMCOS2_DATA_LOAD_E_128K( "do1 dat0.13s",  0x000000, CRC(3e53331f) SHA1(3dd4c133f587361f30ab1b890f5b05749d5838e3) )
	NAMCOS2_DATA_LOAD_O_128K( "do1 dat1.13p",  0x000000, CRC(d5427f11) SHA1(af8d8153dc60044616a6b0571831c53c09fefda1) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "do voi-1a.voice1",  0x000000, 0x080000, CRC(dadf6a57) SHA1(caba21fc6b62d140f6d8231411ce82ae0ad2837a) )
	ROM_LOAD16_BYTE( "do voi-2a.voice2",  0x100000, 0x080000, CRC(81078e01) SHA1(adc70506b21b9a12eadd2f3fd1e920c2eb27c36e) )

	ROM_REGION( 0x0500, "plds", 0 )
	ROM_LOAD( "pal16l8a.4g", 0x0000, 0x0104, CRC(660e1655) SHA1(ffb43238c5ffa3fa831975bc3cde72334c4c2540) )
	ROM_LOAD( "pal16l8a.5f", 0x0200, 0x0104, CRC(18f43c22) SHA1(72849c5b842678bb9037541d26d4c99cdf879982) )
	ROM_LOAD( "pal12l10.8d", 0x0400, 0x0040, CRC(e2379249) SHA1(ad4cdf2e0fd1304a135022eeafa2f61c5f5789cd) )
ROM_END

/* DRAGON SABER (JAPAN) */
ROM_START( dsaberj )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "do1 mpr0b.mor0",  0x000000, 0x020000, CRC(2898e791) SHA1(9f4fd4899897cfb7c8f918abc2070e1ac4169f5d) )
	ROM_LOAD16_BYTE( "do1 mpr1b.mpr1",  0x000001, 0x020000, CRC(5fa9778e) SHA1(3beeb73a42e6afdf866a88618aa03fe8934ec2aa) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "do1 spr0.spr0",  0x000000, 0x010000, CRC(013faf80) SHA1(2cb086e9bb6a551e9fc185c40b976989f758cc57) )
	ROM_LOAD16_BYTE( "do1 spr1.spr1",  0x000001, 0x010000, CRC(c36242bb) SHA1(776f0ab4b2798bc7fc293e27a455675c01ff3132) )

	ROM_REGION( 0x040000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "do1 snd0.snd0",  0x000000, 0x020000, CRC(af5b1ff8) SHA1(fc2a104788f081e4d2ddd22c586e541d6c5b5d69) )
	ROM_LOAD( "do1 snd1.snd1",  0x020000, 0x020000, CRC(c4ca6f3f) SHA1(829a053451be07b296fb4d97818d59eb1e68c807) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "do obj-0a.obj0",  0x000003, 0x80000, CRC(f08c6648) SHA1(ac5221ba159f2390060cbbb7d9cd8148c7bb4a02) )
	ROM_LOAD32_BYTE( "do obj-1a.obj1",  0x000002, 0x80000, CRC(34e0810d) SHA1(679d9b82879cff5197a5098e5dc724c85373b9dc) )
	ROM_LOAD32_BYTE( "do obj-2a.obj2",  0x000001, 0x80000, CRC(bccdabf3) SHA1(d079d89083ac6e71ac8926792d0d7cdcebc848a9) )
	ROM_LOAD32_BYTE( "do obj-3a.obj3",  0x000000, 0x80000, CRC(2a60a4b8) SHA1(5923e08121ad27629bd917d890e037e888e6d356) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "do chr-0a.chr0",  0x000000, 0x80000, CRC(c6058df6) SHA1(13bacad6d593aa5533161e410e22f351c77f29c4) )
	ROM_LOAD( "do chr-1a.chr1",  0x080000, 0x80000, CRC(67aaab36) SHA1(3abb7e226badcfe016325d42c40f06ee020124e3) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	ROM_LOAD( "roz0.bin",  0x000000, 0x80000, CRC(32aab758) SHA1(a3220c2a02b9d8bdd95004d36d2aa5ddf57adfbb) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	ROM_LOAD( "shape.bin",  0x000000, 0x80000, CRC(698e7a3e) SHA1(4d41bf0242626ca1448d1f650c84b5987a7f6597) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data ROMs */
	NAMCOS2_DATA_LOAD_E_128K( "do1 dat0.13s",  0x000000, CRC(3e53331f) SHA1(3dd4c133f587361f30ab1b890f5b05749d5838e3) )
	NAMCOS2_DATA_LOAD_O_128K( "do1 dat1.13p",  0x000000, CRC(d5427f11) SHA1(af8d8153dc60044616a6b0571831c53c09fefda1) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "do voi-1a.voice1",  0x000000, 0x080000, CRC(dadf6a57) SHA1(caba21fc6b62d140f6d8231411ce82ae0ad2837a) )
	ROM_LOAD16_BYTE( "do voi-2a.voice2",  0x100000, 0x080000, CRC(81078e01) SHA1(adc70506b21b9a12eadd2f3fd1e920c2eb27c36e) )

	ROM_REGION( 0x0500, "plds", 0 )
	ROM_LOAD( "pal16l8a.4g", 0x0000, 0x0104, CRC(660e1655) SHA1(ffb43238c5ffa3fa831975bc3cde72334c4c2540) )
	ROM_LOAD( "pal16l8a.5f", 0x0200, 0x0104, CRC(18f43c22) SHA1(72849c5b842678bb9037541d26d4c99cdf879982) )
	ROM_LOAD( "pal12l10.8d", 0x0400, 0x0040, CRC(e2379249) SHA1(ad4cdf2e0fd1304a135022eeafa2f61c5f5789cd) )
ROM_END

/* FINAL LAP (REV E) */
ROM_START( finallap )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "fl2mp0e",  0x000000, 0x010000, CRC(ed805674) SHA1(1d3f391a736303677c69438e5dd0919d0df9aec7) )
	ROM_LOAD16_BYTE( "fl2mp1e",  0x000001, 0x010000, CRC(4c1d523b) SHA1(2aa096008f115c0c45bc930c8c7d514e824686f1) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "fl1-sp0",  0x000000, 0x010000, CRC(2c5ff15d) SHA1(762b49016e8688a3b323b85bc4009b1745f86a3f) )
	ROM_LOAD16_BYTE( "fl1-sp1",  0x000001, 0x010000, CRC(ea9d1a2e) SHA1(37f96306c49475bf45157f768c9173a057bade20) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "fl1-s0b",  0x000000, 0x020000, CRC(f5d76989) SHA1(05c45ccc60fe833cee04f8300620bea5fecc110f) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	/* no external MCU ROM? previously loaded type C, but the game predates it */

	ROM_REGION( 0x200000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "obj-0b",  0x000003, 0x80000, CRC(c6986523) SHA1(1a4b0e95ade6314850b6e44f2debda0ab6e91397) )
	ROM_LOAD32_BYTE( "obj-1b",  0x000002, 0x80000, CRC(6af7d284) SHA1(c74f975c301ff15040be1b38359624ec9c83ac76) )
	ROM_LOAD32_BYTE( "obj-2b",  0x000001, 0x80000, CRC(de45ca8d) SHA1(f476ff1719f60d721d55fd1e40e465f48e7ed019) )
	ROM_LOAD32_BYTE( "obj-3b",  0x000000, 0x80000, CRC(dba830a2) SHA1(5bd899b39458978dd419bf01082782a02b2d9c20) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c0",  0x000000, CRC(cd9d2966) SHA1(39671f846542ba6ae47764674509127cf73e3d71) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c1",  0x080000, CRC(b0efec87) SHA1(6d042f35942c2bdbf5aeb31358d3837ee9c8e5fa) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c2",  0x100000, CRC(263b8e31) SHA1(e2665edd89f7bb8c699e61df6de6a2e8e0698092) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c3",  0x180000, CRC(c2c56743) SHA1(3f338599ffd2e13452cdc3dbf3697fd8aa04bcd2) )
	NAMCOS2_GFXROM_LOAD_128K( "fl2-c4",  0x200000, CRC(83c77a50) SHA1(a3c19de25819586bdecae46326836090e80a92ee) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c5",  0x280000, CRC(ab89da77) SHA1(5cbead4289269a832c0d44d0ded79b58fcc4cc17) )
	NAMCOS2_GFXROM_LOAD_128K( "fl2-c6",  0x300000, CRC(239bd9a0) SHA1(729abe89bea31b4e21161c69579df775b1cba6fe) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "fl2-sha",  0x000000, CRC(5fda0b6d) SHA1(92c0410e159977ea73a8e8c0cb1321c3056f6c2f) )

	ROM_REGION16_BE( 0x200000, "data_rom", ROMREGION_ERASEFF ) /* Shared data ROMs */
	/* No DAT files present in ZIP archive */

	ROM_REGION( 0x100, "c45_road:clut", 0 ) /* PROM for road colors */
	ROM_LOAD( "fl1-3.5b", 0, 0x100, CRC(d179d99a) SHA1(4e64f284c74d2b77f893bd28aaa6489084056aa2) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	NAMCOS2_DATA_LOAD_E_128K( "fl1-v1",  0x000000, CRC(86b21996) SHA1(833ffde729199c81e472fb88ed5b7f4ce08a83d6) )
	NAMCOS2_DATA_LOAD_E_128K( "fl1-v2",  0x100000, CRC(6a164647) SHA1(3162457beccccdb416994ebd32fb83b13eb719e0) )
ROM_END

/* FINAL LAP (revision D) */
ROM_START( finallapd )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "fl2-mp0d",  0x000000, 0x010000, CRC(3576d3aa) SHA1(4928cfc5c929de8cc5ec12ea87c678188d314c39) )
	ROM_LOAD16_BYTE( "fl2-mp1d",  0x000001, 0x010000, CRC(22d3906d) SHA1(63f3076eeefca57daf427633f82fdea353a52da2) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "fl1-sp0",  0x000000, 0x010000, CRC(2c5ff15d) SHA1(762b49016e8688a3b323b85bc4009b1745f86a3f) )
	ROM_LOAD16_BYTE( "fl1-sp1",  0x000001, 0x010000, CRC(ea9d1a2e) SHA1(37f96306c49475bf45157f768c9173a057bade20) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "fl1-s0b",  0x000000, 0x020000, CRC(f5d76989) SHA1(05c45ccc60fe833cee04f8300620bea5fecc110f) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	/* no external MCU ROM? previously loaded type C, but the game predates it */

	ROM_REGION( 0x200000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "obj-0b",  0x000003, 0x80000, CRC(c6986523) SHA1(1a4b0e95ade6314850b6e44f2debda0ab6e91397) )
	ROM_LOAD32_BYTE( "obj-1b",  0x000002, 0x80000, CRC(6af7d284) SHA1(c74f975c301ff15040be1b38359624ec9c83ac76) )
	ROM_LOAD32_BYTE( "obj-2b",  0x000001, 0x80000, CRC(de45ca8d) SHA1(f476ff1719f60d721d55fd1e40e465f48e7ed019) )
	ROM_LOAD32_BYTE( "obj-3b",  0x000000, 0x80000, CRC(dba830a2) SHA1(5bd899b39458978dd419bf01082782a02b2d9c20) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c0",  0x000000, CRC(cd9d2966) SHA1(39671f846542ba6ae47764674509127cf73e3d71) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c1",  0x080000, CRC(b0efec87) SHA1(6d042f35942c2bdbf5aeb31358d3837ee9c8e5fa) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c2",  0x100000, CRC(263b8e31) SHA1(e2665edd89f7bb8c699e61df6de6a2e8e0698092) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c3",  0x180000, CRC(c2c56743) SHA1(3f338599ffd2e13452cdc3dbf3697fd8aa04bcd2) )
	NAMCOS2_GFXROM_LOAD_128K( "fl2-c4",  0x200000, CRC(83c77a50) SHA1(a3c19de25819586bdecae46326836090e80a92ee) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c5",  0x280000, CRC(ab89da77) SHA1(5cbead4289269a832c0d44d0ded79b58fcc4cc17) )
	NAMCOS2_GFXROM_LOAD_128K( "fl2-c6",  0x300000, CRC(239bd9a0) SHA1(729abe89bea31b4e21161c69579df775b1cba6fe) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 )                 /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "fl2-sha",  0x000000, CRC(5fda0b6d) SHA1(92c0410e159977ea73a8e8c0cb1321c3056f6c2f) )

	ROM_REGION16_BE( 0x200000, "data_rom", ROMREGION_ERASEFF ) /* Shared data ROMs */
	/* No DAT files present in ZIP archive */

	ROM_REGION( 0x100, "c45_road:clut", 0 ) /* PROM for road colors */
	ROM_LOAD( "fl1-3.5b", 0, 0x100, CRC(d179d99a) SHA1(4e64f284c74d2b77f893bd28aaa6489084056aa2) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	NAMCOS2_DATA_LOAD_E_128K( "fl1-v1",  0x000000, CRC(86b21996) SHA1(833ffde729199c81e472fb88ed5b7f4ce08a83d6) )
	NAMCOS2_DATA_LOAD_E_128K( "fl1-v2",  0x100000, CRC(6a164647) SHA1(3162457beccccdb416994ebd32fb83b13eb719e0) )
ROM_END

/* FINAL LAP (revision C) */
ROM_START( finallapc )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "fl2-mp0c",  0x000000, 0x010000, CRC(f667f2c9) SHA1(79096bd600c001b21020ddf468e55c34dad9f1eb) )
	ROM_LOAD16_BYTE( "fl2-mp1c",  0x000001, 0x010000, CRC(b8615d33) SHA1(f72ce75047cf41d47684d6657aa3bd1b00f68056) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "fl1-sp0",  0x000000, 0x010000, CRC(2c5ff15d) SHA1(762b49016e8688a3b323b85bc4009b1745f86a3f) )
	ROM_LOAD16_BYTE( "fl1-sp1",  0x000001, 0x010000, CRC(ea9d1a2e) SHA1(37f96306c49475bf45157f768c9173a057bade20) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "fl1-s0",  0x000000, 0x020000, CRC(1f8ff494) SHA1(10a90291159e29a905c6d438aa2bc98ccf906a69) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	/* no external MCU ROM? previously loaded type C, but the game predates it */

	ROM_REGION( 0x200000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "obj-0b",  0x000003, 0x80000, CRC(c6986523) SHA1(1a4b0e95ade6314850b6e44f2debda0ab6e91397) )
	ROM_LOAD32_BYTE( "obj-1b",  0x000002, 0x80000, CRC(6af7d284) SHA1(c74f975c301ff15040be1b38359624ec9c83ac76) )
	ROM_LOAD32_BYTE( "obj-2b",  0x000001, 0x80000, CRC(de45ca8d) SHA1(f476ff1719f60d721d55fd1e40e465f48e7ed019) )
	ROM_LOAD32_BYTE( "obj-3b",  0x000000, 0x80000, CRC(dba830a2) SHA1(5bd899b39458978dd419bf01082782a02b2d9c20) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c0",  0x000000, CRC(cd9d2966) SHA1(39671f846542ba6ae47764674509127cf73e3d71) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c1",  0x080000, CRC(b0efec87) SHA1(6d042f35942c2bdbf5aeb31358d3837ee9c8e5fa) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c2",  0x100000, CRC(263b8e31) SHA1(e2665edd89f7bb8c699e61df6de6a2e8e0698092) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c3",  0x180000, CRC(c2c56743) SHA1(3f338599ffd2e13452cdc3dbf3697fd8aa04bcd2) )
	NAMCOS2_GFXROM_LOAD_128K( "fl2-c4",  0x200000, CRC(83c77a50) SHA1(a3c19de25819586bdecae46326836090e80a92ee) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c5",  0x280000, CRC(ab89da77) SHA1(5cbead4289269a832c0d44d0ded79b58fcc4cc17) )
	NAMCOS2_GFXROM_LOAD_128K( "fl2-c6",  0x300000, CRC(239bd9a0) SHA1(729abe89bea31b4e21161c69579df775b1cba6fe) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "fl2-sha",  0x000000, CRC(5fda0b6d) SHA1(92c0410e159977ea73a8e8c0cb1321c3056f6c2f) )

	ROM_REGION16_BE( 0x200000, "data_rom", ROMREGION_ERASEFF ) /* Shared data ROMs */
	/* No DAT files present in ZIP archive */

	ROM_REGION( 0x100, "c45_road:clut", 0 ) /* PROM for road colors */
	ROM_LOAD( "fl1-3.5b", 0, 0x100, CRC(d179d99a) SHA1(4e64f284c74d2b77f893bd28aaa6489084056aa2) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	NAMCOS2_DATA_LOAD_E_128K( "fl1-v1",  0x000000, CRC(86b21996) SHA1(833ffde729199c81e472fb88ed5b7f4ce08a83d6) )
	NAMCOS2_DATA_LOAD_E_128K( "fl1-v2",  0x100000, CRC(6a164647) SHA1(3162457beccccdb416994ebd32fb83b13eb719e0) )
ROM_END

/* FINAL LAP (Rev C - Japan) */
ROM_START( finallapjc )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "fl1_mp0c.bin",  0x000000, 0x010000, CRC(63cd7304) SHA1(f9342a03b6e3c599946fec1690f34be2d0fe0307) )
	ROM_LOAD16_BYTE( "fl1_mp1c.bin",  0x000001, 0x010000, CRC(cc9c5fb6) SHA1(7da82f5c8c1574c92432ee003c8433b7dde9d393) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "fl1-sp0",  0x000000, 0x010000, CRC(2c5ff15d) SHA1(762b49016e8688a3b323b85bc4009b1745f86a3f) )
	ROM_LOAD16_BYTE( "fl1-sp1",  0x000001, 0x010000, CRC(ea9d1a2e) SHA1(37f96306c49475bf45157f768c9173a057bade20) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "fl1-s0b",  0x000000, 0x020000, CRC(f5d76989) SHA1(05c45ccc60fe833cee04f8300620bea5fecc110f) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	/* no external MCU ROM? previously loaded type C, but the game predates it */

	ROM_REGION( 0x200000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "obj-0b",  0x000003, 0x80000, CRC(c6986523) SHA1(1a4b0e95ade6314850b6e44f2debda0ab6e91397) )
	ROM_LOAD32_BYTE( "obj-1b",  0x000002, 0x80000, CRC(6af7d284) SHA1(c74f975c301ff15040be1b38359624ec9c83ac76) )
	ROM_LOAD32_BYTE( "obj-2b",  0x000001, 0x80000, CRC(de45ca8d) SHA1(f476ff1719f60d721d55fd1e40e465f48e7ed019) )
	ROM_LOAD32_BYTE( "obj-3b",  0x000000, 0x80000, CRC(dba830a2) SHA1(5bd899b39458978dd419bf01082782a02b2d9c20) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c0",  0x000000, CRC(cd9d2966) SHA1(39671f846542ba6ae47764674509127cf73e3d71) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c1",  0x080000, CRC(b0efec87) SHA1(6d042f35942c2bdbf5aeb31358d3837ee9c8e5fa) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c2",  0x100000, CRC(263b8e31) SHA1(e2665edd89f7bb8c699e61df6de6a2e8e0698092) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c3",  0x180000, CRC(c2c56743) SHA1(3f338599ffd2e13452cdc3dbf3697fd8aa04bcd2) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c4",  0x200000, CRC(cdc1de2e) SHA1(701dcc4fc25e7b6118947a5accabd43c0475ebe9) ) // Japan specific (was labeled fl2 - seems like fl1 is correct, fl2 is the english version?)
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c5",  0x280000, CRC(ab89da77) SHA1(5cbead4289269a832c0d44d0ded79b58fcc4cc17) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c6",  0x300000, CRC(8e78a3c3) SHA1(d1cd7fad038d52430f933b3ee2440554d2aeade9) ) // Japan specific (was labeled fl2 - seems like fl1 is correct, fl2 is the english version?)

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "fl1_sha.bin",  0x000000, CRC(b7e1c7a3) SHA1(b82f9b340d95b80a12286647adba8c139b4d081a) )

	ROM_REGION16_BE( 0x200000, "data_rom", ROMREGION_ERASEFF ) /* Shared data ROMs */
	/* No DAT files present in ZIP archive */

	ROM_REGION( 0x100, "c45_road:clut", 0 ) /* PROM for road colors */
	ROM_LOAD( "fl1-3.5b", 0, 0x100, CRC(d179d99a) SHA1(4e64f284c74d2b77f893bd28aaa6489084056aa2) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	NAMCOS2_DATA_LOAD_E_128K( "fl1-v1",  0x000000, CRC(86b21996) SHA1(833ffde729199c81e472fb88ed5b7f4ce08a83d6) )
	NAMCOS2_DATA_LOAD_E_128K( "fl1-v2",  0x100000, CRC(6a164647) SHA1(3162457beccccdb416994ebd32fb83b13eb719e0) )
ROM_END

/* FINAL LAP  (REV B - JAPAN) */
ROM_START( finallapjb )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "fl1_mp0b.bin",  0x000000, 0x010000, CRC(870a482a) SHA1(776afe0206385732f9bcad4a75399ad7cc8fcda1) )
	ROM_LOAD16_BYTE( "fl1_mp1b.bin",  0x000001, 0x010000, CRC(af52c991) SHA1(23d440cc0f53872739fce1d0837ef906a5e75f98) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "fl1-sp0",  0x000000, 0x010000, CRC(2c5ff15d) SHA1(762b49016e8688a3b323b85bc4009b1745f86a3f) )
	ROM_LOAD16_BYTE( "fl1-sp1",  0x000001, 0x010000, CRC(ea9d1a2e) SHA1(37f96306c49475bf45157f768c9173a057bade20) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "fl1-s0",  0x000000, 0x020000, CRC(1f8ff494) SHA1(10a90291159e29a905c6d438aa2bc98ccf906a69) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	/* no external MCU ROM? previously loaded type C, but the game predates it */

	ROM_REGION( 0x200000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "obj-0b",  0x000003, 0x80000, CRC(c6986523) SHA1(1a4b0e95ade6314850b6e44f2debda0ab6e91397) )
	ROM_LOAD32_BYTE( "obj-1b",  0x000002, 0x80000, CRC(6af7d284) SHA1(c74f975c301ff15040be1b38359624ec9c83ac76) )
	ROM_LOAD32_BYTE( "obj-2b",  0x000001, 0x80000, CRC(de45ca8d) SHA1(f476ff1719f60d721d55fd1e40e465f48e7ed019) )
	ROM_LOAD32_BYTE( "obj-3b",  0x000000, 0x80000, CRC(dba830a2) SHA1(5bd899b39458978dd419bf01082782a02b2d9c20) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c0",  0x000000, CRC(cd9d2966) SHA1(39671f846542ba6ae47764674509127cf73e3d71) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c1",  0x080000, CRC(b0efec87) SHA1(6d042f35942c2bdbf5aeb31358d3837ee9c8e5fa) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c2",  0x100000, CRC(263b8e31) SHA1(e2665edd89f7bb8c699e61df6de6a2e8e0698092) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c3",  0x180000, CRC(c2c56743) SHA1(3f338599ffd2e13452cdc3dbf3697fd8aa04bcd2) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c4",  0x200000, CRC(cdc1de2e) SHA1(701dcc4fc25e7b6118947a5accabd43c0475ebe9) ) // Japan specific (was labeled fl2 - seems like fl1 is correct, fl2 is the english version?)
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c5",  0x280000, CRC(ab89da77) SHA1(5cbead4289269a832c0d44d0ded79b58fcc4cc17) )
	NAMCOS2_GFXROM_LOAD_128K( "fl1-c6",  0x300000, CRC(8e78a3c3) SHA1(d1cd7fad038d52430f933b3ee2440554d2aeade9) ) // Japan specific (was labeled fl2 - seems like fl1 is correct, fl2 is the english version?)

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "fl1_sha.bin",  0x000000, CRC(b7e1c7a3) SHA1(b82f9b340d95b80a12286647adba8c139b4d081a) )

	ROM_REGION16_BE( 0x200000, "data_rom", ROMREGION_ERASEFF ) /* Shared data ROMs */
	/* No DAT files present in ZIP archive */

	ROM_REGION( 0x100, "c45_road:clut", 0 ) /* PROM for road colors */
	ROM_LOAD( "fl1-3.5b", 0, 0x100, CRC(d179d99a) SHA1(4e64f284c74d2b77f893bd28aaa6489084056aa2) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	NAMCOS2_DATA_LOAD_E_128K( "fl1-v1",  0x000000, CRC(86b21996) SHA1(833ffde729199c81e472fb88ed5b7f4ce08a83d6) )
	NAMCOS2_DATA_LOAD_E_128K( "fl1-v2",  0x100000, CRC(6a164647) SHA1(3162457beccccdb416994ebd32fb83b13eb719e0) )
ROM_END

ROM_START( finalap2 )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "fls2mp0b",  0x000000, 0x020000, CRC(97b48aae) SHA1(03774b642c5a8186e443ad4138a9e86a931a2199) )
	ROM_LOAD16_BYTE( "fls2mp1b",  0x000001, 0x020000, CRC(c9f3e0e7) SHA1(4127e373239e4ca31b5fa8b71d8f10b7d7ed93c2) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "fls1sp0b",  0x000000, 0x020000, CRC(8bf15d9c) SHA1(b6c14a9d06e99d03636fd6eb2163a18e2bbcc4b1) )
	ROM_LOAD16_BYTE( "fls1sp1b",  0x000001, 0x020000, CRC(c1a31086) SHA1(55317b72a219ffbfe00bf62ad2a635790d56f84e) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "flss0",  0x000000, 0x020000, CRC(c07cc10a) SHA1(012f19a8014a77fdf0409241c0223b2c0c247357) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "fl2obj0",  0x000003, 0x80000, CRC(3657dd7a) SHA1(8f286ec0642b09ff42bf0dbd784ae257d4ab278a) )
	ROM_LOAD32_BYTE( "fl2obj2",  0x000002, 0x80000, CRC(8ac933fd) SHA1(b158df2ec55f49ec05861075c8d7bd265361dab0) )
	ROM_LOAD32_BYTE( "fl2obj4",  0x000001, 0x80000, CRC(e7b989e6) SHA1(485e8148510edd1645f5b4fbbc9a53e8bf1c3e5f) )
	ROM_LOAD32_BYTE( "fl2obj6",  0x000000, 0x80000, CRC(4936583d) SHA1(0145e89fdb5db28cb8f8ce59572729e83d8fad7c) )
	ROM_LOAD32_BYTE( "fl2obj1",  0x200003, 0x80000, CRC(3cebf419) SHA1(bfdf1b768920e55850173a5bcd1007608e1a4f56) )
	ROM_LOAD32_BYTE( "fl2obj3",  0x200002, 0x80000, CRC(0959ed55) SHA1(00e640d449cb47da0e65baa798743395c7a1f632) )
	ROM_LOAD32_BYTE( "fl2obj5",  0x200001, 0x80000, CRC(d74ae0d3) SHA1(96c9798378da7bdc127ed7d02a4dd14dfd142550) )
	ROM_LOAD32_BYTE( "fl2obj7",  0x200000, 0x80000, CRC(5ca68c93) SHA1(fa326992338843ccfa458a5b85ba58537da666d0) )

	ROM_REGION( 0x200000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "fls2chr0",  0x000000, 0x40000, CRC(7bbda499) SHA1(cf6ff072a40063cbe41eae1f60b29447a0020926) )
	ROM_LOAD( "fls2chr1",  0x040000, 0x40000, CRC(ac8940e5) SHA1(449687d38cf830445df713ed4d675ed94ca5b375) )
	ROM_LOAD( "fls2chr2",  0x080000, 0x40000, CRC(1756173d) SHA1(c912163979098387aea9a0580e9ca55c1f7275f3) )
	ROM_LOAD( "fls2chr3",  0x0c0000, 0x40000, CRC(69032785) SHA1(cfcd12bea730f724444188c206adcdb5e755eb7d) )
	ROM_LOAD( "fls2chr4",  0x100000, 0x40000, CRC(8216cf42) SHA1(79820435584d769b63649b554574486dbcd6f468) )
	ROM_LOAD( "fls2chr5",  0x140000, 0x40000, CRC(dc3e8e1c) SHA1(a7968cfa0ca2639364507b42526f10cf1b2000f4) )
	ROM_LOAD( "fls2chr6",  0x180000, 0x40000, CRC(1ef4bdde) SHA1(ceb36c021450efa4cb0fee278fa0b9d65f7d1f05) )
	ROM_LOAD( "fls2chr7",  0x1c0000, 0x40000, CRC(53dafcde) SHA1(f9d9460349b34bda95b8c206af7ce2347c951214) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_256K( "fls2sha",  0x000000, CRC(f7b40a85) SHA1(a458a1cc0dae757fe8a15cb5f5ae46d3c033df00) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data ROMs */
	NAMCOS2_DATA_LOAD_E_256K( "fls2dat0.13s",  0x000000, CRC(f1af432c) SHA1(c514261a49ceb5c3ba0246519ba5d02e9a20d950) )
	NAMCOS2_DATA_LOAD_O_256K( "fls2dat1.13p",  0x000000, CRC(8719533e) SHA1(98d2767da6f7f67da7af15e8cfed95adb04b7427) )

	ROM_REGION( 0x100, "c45_road:clut", 0 ) /* PROM for road colors */
	ROM_LOAD( "fl1-3.5b", 0, 0x100, CRC(d179d99a) SHA1(4e64f284c74d2b77f893bd28aaa6489084056aa2) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "flsvoi1",  0x000000, 0x080000, CRC(590be52f) SHA1(9ef2728dd533979b6019b422fc4961a6085428b4) )
	ROM_LOAD16_BYTE( "flsvoi2",  0x100000, 0x080000, CRC(204b3c27) SHA1(80cd13bfe2a4b3039b4a120b905674e46b8b3b9c) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "finalap2.nv",  0x000000, 0x2000, CRC(c7ae5d0a) SHA1(9527e44accec0ec9d1990138d1b0bfc71957cc8a) )
ROM_END

/* FINAL LAP 2 (Japan) */
ROM_START( finalap2j )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "fls1_mp0.bin",  0x000000, 0x020000, CRC(05ea8090) SHA1(edd3d8780e9f849f45fff1593821aed6b7711a57) )
	ROM_LOAD16_BYTE( "fls1_mp1.bin",  0x000001, 0x020000, CRC(fb189f50) SHA1(9436aea727adf9e11e8061d0ded4e4b00df90b70) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "fls1sp0b",  0x000000, 0x020000, CRC(8bf15d9c) SHA1(b6c14a9d06e99d03636fd6eb2163a18e2bbcc4b1) )
	ROM_LOAD16_BYTE( "fls1sp1b",  0x000001, 0x020000, CRC(c1a31086) SHA1(55317b72a219ffbfe00bf62ad2a635790d56f84e) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "flss0",  0x000000, 0x020000, CRC(c07cc10a) SHA1(012f19a8014a77fdf0409241c0223b2c0c247357) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "fl2obj0",  0x000003, 0x80000, CRC(3657dd7a) SHA1(8f286ec0642b09ff42bf0dbd784ae257d4ab278a) )
	ROM_LOAD32_BYTE( "fl2obj2",  0x000002, 0x80000, CRC(8ac933fd) SHA1(b158df2ec55f49ec05861075c8d7bd265361dab0) )
	ROM_LOAD32_BYTE( "fl2obj4",  0x000001, 0x80000, CRC(e7b989e6) SHA1(485e8148510edd1645f5b4fbbc9a53e8bf1c3e5f) )
	ROM_LOAD32_BYTE( "fl2obj6",  0x000000, 0x80000, CRC(4936583d) SHA1(0145e89fdb5db28cb8f8ce59572729e83d8fad7c) )
	ROM_LOAD32_BYTE( "fl2obj1",  0x200003, 0x80000, CRC(3cebf419) SHA1(bfdf1b768920e55850173a5bcd1007608e1a4f56) )
	ROM_LOAD32_BYTE( "fl2obj3",  0x200002, 0x80000, CRC(0959ed55) SHA1(00e640d449cb47da0e65baa798743395c7a1f632) )
	ROM_LOAD32_BYTE( "fl2obj5",  0x200001, 0x80000, CRC(d74ae0d3) SHA1(96c9798378da7bdc127ed7d02a4dd14dfd142550) )
	ROM_LOAD32_BYTE( "fl2obj7",  0x200000, 0x80000, CRC(5ca68c93) SHA1(fa326992338843ccfa458a5b85ba58537da666d0) )

	// The Japanese version should not be using the same ROMs as the World version here, causes corrupt text in attract mode should probably be fls1
	ROM_REGION( 0x200000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "fls2chr0",  0x000000, 0x40000, BAD_DUMP CRC(7bbda499) SHA1(cf6ff072a40063cbe41eae1f60b29447a0020926) )
	ROM_LOAD( "fls2chr1",  0x040000, 0x40000, BAD_DUMP CRC(ac8940e5) SHA1(449687d38cf830445df713ed4d675ed94ca5b375) )
	ROM_LOAD( "fls2chr2",  0x080000, 0x40000, BAD_DUMP CRC(1756173d) SHA1(c912163979098387aea9a0580e9ca55c1f7275f3) )
	ROM_LOAD( "fls2chr3",  0x0c0000, 0x40000, BAD_DUMP CRC(69032785) SHA1(cfcd12bea730f724444188c206adcdb5e755eb7d) )
	ROM_LOAD( "fls2chr4",  0x100000, 0x40000, BAD_DUMP CRC(8216cf42) SHA1(79820435584d769b63649b554574486dbcd6f468) )
	ROM_LOAD( "fls2chr5",  0x140000, 0x40000, BAD_DUMP CRC(dc3e8e1c) SHA1(a7968cfa0ca2639364507b42526f10cf1b2000f4) )
	ROM_LOAD( "fls2chr6",  0x180000, 0x40000, BAD_DUMP CRC(1ef4bdde) SHA1(ceb36c021450efa4cb0fee278fa0b9d65f7d1f05) )
	ROM_LOAD( "fls2chr7",  0x1c0000, 0x40000, BAD_DUMP CRC(53dafcde) SHA1(f9d9460349b34bda95b8c206af7ce2347c951214) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_256K( "fls2sha",  0x000000, BAD_DUMP CRC(f7b40a85) SHA1(a458a1cc0dae757fe8a15cb5f5ae46d3c033df00) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data ROMs */
	NAMCOS2_DATA_LOAD_E_256K( "fls2dat0.13s",  0x000000, CRC(f1af432c) SHA1(c514261a49ceb5c3ba0246519ba5d02e9a20d950) )
	NAMCOS2_DATA_LOAD_O_256K( "fls2dat1.13p",  0x000000, CRC(8719533e) SHA1(98d2767da6f7f67da7af15e8cfed95adb04b7427) )

	ROM_REGION( 0x100, "c45_road:clut", 0 ) /* PROM for road colors */
	ROM_LOAD( "fl1-3.5b", 0, 0x100, CRC(d179d99a) SHA1(4e64f284c74d2b77f893bd28aaa6489084056aa2) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "flsvoi1",  0x000000, 0x080000, CRC(590be52f) SHA1(9ef2728dd533979b6019b422fc4961a6085428b4) )
	ROM_LOAD16_BYTE( "flsvoi2",  0x100000, 0x080000, CRC(204b3c27) SHA1(80cd13bfe2a4b3039b4a120b905674e46b8b3b9c) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "finalap2.nv",  0x000000, 0x2000, CRC(c7ae5d0a) SHA1(9527e44accec0ec9d1990138d1b0bfc71957cc8a) )
ROM_END

/* FINAL LAP 3 WORLD (REV. C) */
ROM_START( finalap3 ) // this set displays MOTION (Ver. 3) in the test mode menu
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "flt2_mpr0c.11d",  0x000000, 0x020000, CRC(9ff361ff) SHA1(0a197f5a3e9a03455cd5afe6979587baf757973f) )
	ROM_LOAD16_BYTE( "flt2_mpr1c.13d",  0x000001, 0x020000, CRC(17efb7f2) SHA1(74940950b8b1a3353a130846937c9f651be3f335) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "flt1_sp0.11k",  0x000000, 0x020000, CRC(e804ced1) SHA1(b31e1fddd202ba503b28455a270e8d45a348dc2e) )
	ROM_LOAD16_BYTE( "flt1_sp1.13k",  0x000001, 0x020000, CRC(3a2b24ee) SHA1(e4f16f30516dab13cc8b1cb6c80ec1df129f8851) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "flt1_snd0.7j",  0x000000, 0x020000, CRC(60b72aed) SHA1(f12e157ae5e9f373ba1d75012b869b1c4ad9fb00) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "flt_obj-0.4c",  0x000003, 0x80000, CRC(eab19ec6) SHA1(2859e88b94aa873f3b6ba22790f2211f3e172dd1) )
	ROM_LOAD32_BYTE( "flt_obj-2.4a",  0x000002, 0x80000, CRC(2a3b7ded) SHA1(455d9d6cf7d497687f93af899fc20bbff6129391) )
	ROM_LOAD32_BYTE( "flt_obj-4.8c",  0x000001, 0x80000, CRC(84aa500c) SHA1(087c0089478a270154f50f3b0f001428e80d74c7) )
	ROM_LOAD32_BYTE( "flt_obj-6.8a",  0x000000, 0x80000, CRC(33118e63) SHA1(126cc034909e05da953a1a67d6c0f18f5304b407) )
	ROM_LOAD32_BYTE( "flt_obj-1.2c",  0x200003, 0x80000, CRC(4ef37a51) SHA1(2f43691cfcd852773ae5e1d879f556f232bae877) )
	ROM_LOAD32_BYTE( "flt_obj-3.2a",  0x200002, 0x80000, CRC(b86dc7cd) SHA1(25402d7111c1277a618b313d1244c1a567ce458a) )
	ROM_LOAD32_BYTE( "flt_obj-5.5c",  0x200001, 0x80000, CRC(6a53e603) SHA1(6087c694e0e30a98c84227991d9c2e9c39c3e9ca) )
	ROM_LOAD32_BYTE( "flt_obj-7.6a",  0x200000, 0x80000, CRC(b52a85e2) SHA1(1eea10eb20ae56309397238a52e9ea0756912412) )

	ROM_REGION( 0x200000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "flt2_chr-0.11n", 0x000000, 0x40000, CRC(5954f270) SHA1(6f26365d89f38d4ab477908f32823e06f1a84e09) )
	ROM_LOAD( "fltchr-1.11p",   0x040000, 0x40000, CRC(2e68d13c) SHA1(46bb0628da1f97e0f6865f37e53a01d2e8391255) )
	ROM_LOAD( "flt2_chr-2.11r", 0x080000, 0x40000, CRC(98f3b190) SHA1(4858dab070c0c46d37148312664c6a616765c240) )
	ROM_LOAD( "fltchr-3.11s",   0x0c0000, 0x40000, CRC(e9b05a1f) SHA1(6b7e08f4f535fb99692920612f751a49c3cd529e) )
	ROM_LOAD( "fltchr-4.9n",    0x100000, 0x40000, CRC(5ae43767) SHA1(f3e24dd5ba2cffffd616ddee628e423aa0aec1d2) )
	ROM_LOAD( "fltchr-5.9p",    0x140000, 0x40000, CRC(b5f4e780) SHA1(a7d64d150121eda5d82f5651a06a11683220577a) )
	ROM_LOAD( "fltchr-6.9r",    0x180000, 0x40000, CRC(4b0baea2) SHA1(a75ba5294f06ddbe170988073b8f4a74a7cbcee1) )
	ROM_LOAD( "fltchr-7.9s",    0x1c0000, 0x40000, CRC(85db9e94) SHA1(918f414c1dd51f7451a9a491ba1d60f5f9a38c3e) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_256K( "flt2_sha.7n",  0x000000, CRC(6986565b) SHA1(df95f2ad5d1e938551d33153f2dcc4711ffd0eba) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data ROMs */
	NAMCOS2_DATA_LOAD_E_128K( "flt1_d0.13s",  0x000000, CRC(80004966) SHA1(112b2a9b0ea792d5dbff1b9cf904da788aeede29) )
	NAMCOS2_DATA_LOAD_O_128K( "flt1_d1.13p",  0x000000, CRC(a2e93e8c) SHA1(9c8a5431a79153a70eb6939d16e0a5a6be235e75) )

	ROM_REGION( 0x100, "c45_road:clut", 0 ) /* PROM for road colors */
	ROM_LOAD( "fl1_3.5b", 0, 0x100, CRC(d179d99a) SHA1(4e64f284c74d2b77f893bd28aaa6489084056aa2) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "flt_voi-1.3m",  0x000000, 0x080000, CRC(4fc7c0ba) SHA1(bbfd1764fd79087bba5e6199e8916c28bed4d3f4) )
	ROM_LOAD16_BYTE( "flt_voi-2.3l",  0x100000, 0x080000, CRC(409c62df) SHA1(0c2f088168f1f92f2f767ea47522c0e8f4a10265) )

	ROM_REGION( 0x2000, "user2", 0 ) /* zoom */
	ROM_LOAD( "04544191.6r", 0, 0x2000, CRC(90db1bf6) SHA1(dbb9e50a8efc3b4012fcf587cc87da9ef42a1b80) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "finalap3.nv",  0x000000, 0x2000, CRC(efbc6274) SHA1(f542012e467027b7bd5d7102096ff91d8c9adee3) )
ROM_END


/* FINAL LAP 3 WORLD */
ROM_START( finalap3a )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "flt2-mp0.11d",  0x000000, 0x020000, CRC(22082168) SHA1(d3090a91bda70fb46d3b7846d32db36c1f42d5cc) )
	ROM_LOAD16_BYTE( "flt2-mp1.13d",  0x000001, 0x020000, CRC(2ec21977) SHA1(fe8ded6402da710255e09cdb22ba1ef5acd78518) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "flt1_sp0.11k",  0x000000, 0x020000, CRC(e804ced1) SHA1(b31e1fddd202ba503b28455a270e8d45a348dc2e) )
	ROM_LOAD16_BYTE( "flt1_sp1.13k",  0x000001, 0x020000, CRC(3a2b24ee) SHA1(e4f16f30516dab13cc8b1cb6c80ec1df129f8851) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "flt1_snd0.7j",  0x000000, 0x020000, CRC(60b72aed) SHA1(f12e157ae5e9f373ba1d75012b869b1c4ad9fb00) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "flt_obj-0.4c",  0x000003, 0x80000, CRC(eab19ec6) SHA1(2859e88b94aa873f3b6ba22790f2211f3e172dd1) )
	ROM_LOAD32_BYTE( "flt_obj-2.4a",  0x000002, 0x80000, CRC(2a3b7ded) SHA1(455d9d6cf7d497687f93af899fc20bbff6129391) )
	ROM_LOAD32_BYTE( "flt_obj-4.8c",  0x000001, 0x80000, CRC(84aa500c) SHA1(087c0089478a270154f50f3b0f001428e80d74c7) )
	ROM_LOAD32_BYTE( "flt_obj-6.8a",  0x000000, 0x80000, CRC(33118e63) SHA1(126cc034909e05da953a1a67d6c0f18f5304b407) )
	ROM_LOAD32_BYTE( "flt_obj-1.2c",  0x200003, 0x80000, CRC(4ef37a51) SHA1(2f43691cfcd852773ae5e1d879f556f232bae877) )
	ROM_LOAD32_BYTE( "flt_obj-3.2a",  0x200002, 0x80000, CRC(b86dc7cd) SHA1(25402d7111c1277a618b313d1244c1a567ce458a) )
	ROM_LOAD32_BYTE( "flt_obj-5.5c",  0x200001, 0x80000, CRC(6a53e603) SHA1(6087c694e0e30a98c84227991d9c2e9c39c3e9ca) )
	ROM_LOAD32_BYTE( "flt_obj-7.6a",  0x200000, 0x80000, CRC(b52a85e2) SHA1(1eea10eb20ae56309397238a52e9ea0756912412) )

	ROM_REGION( 0x200000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "flt2_chr-0.11n", 0x000000, 0x40000, CRC(5954f270) SHA1(6f26365d89f38d4ab477908f32823e06f1a84e09) )
	ROM_LOAD( "fltchr-1.11p",   0x040000, 0x40000, CRC(2e68d13c) SHA1(46bb0628da1f97e0f6865f37e53a01d2e8391255) )
	ROM_LOAD( "flt2_chr-2.11r", 0x080000, 0x40000, CRC(98f3b190) SHA1(4858dab070c0c46d37148312664c6a616765c240) )
	ROM_LOAD( "fltchr-3.11s",   0x0c0000, 0x40000, CRC(e9b05a1f) SHA1(6b7e08f4f535fb99692920612f751a49c3cd529e) )
	ROM_LOAD( "fltchr-4.9n",    0x100000, 0x40000, CRC(5ae43767) SHA1(f3e24dd5ba2cffffd616ddee628e423aa0aec1d2) )
	ROM_LOAD( "fltchr-5.9p",    0x140000, 0x40000, CRC(b5f4e780) SHA1(a7d64d150121eda5d82f5651a06a11683220577a) )
	ROM_LOAD( "fltchr-6.9r",    0x180000, 0x40000, CRC(4b0baea2) SHA1(a75ba5294f06ddbe170988073b8f4a74a7cbcee1) )
	ROM_LOAD( "fltchr-7.9s",    0x1c0000, 0x40000, CRC(85db9e94) SHA1(918f414c1dd51f7451a9a491ba1d60f5f9a38c3e) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_256K( "flt2_sha.7n",  0x000000, CRC(6986565b) SHA1(df95f2ad5d1e938551d33153f2dcc4711ffd0eba) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data ROMs */
	NAMCOS2_DATA_LOAD_E_128K( "flt1_d0.13s",  0x000000, CRC(80004966) SHA1(112b2a9b0ea792d5dbff1b9cf904da788aeede29) )
	NAMCOS2_DATA_LOAD_O_128K( "flt1_d1.13p",  0x000000, CRC(a2e93e8c) SHA1(9c8a5431a79153a70eb6939d16e0a5a6be235e75) )

	ROM_REGION( 0x100, "c45_road:clut", 0 ) /* PROM for road colors */
	ROM_LOAD( "fl1_3.5b", 0, 0x100, CRC(d179d99a) SHA1(4e64f284c74d2b77f893bd28aaa6489084056aa2) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "flt_voi-1.3m",  0x000000, 0x080000, CRC(4fc7c0ba) SHA1(bbfd1764fd79087bba5e6199e8916c28bed4d3f4) )
	ROM_LOAD16_BYTE( "flt_voi-2.3l",  0x100000, 0x080000, CRC(409c62df) SHA1(0c2f088168f1f92f2f767ea47522c0e8f4a10265) )

	ROM_REGION( 0x2000, "user2", 0 ) /* zoom */
	ROM_LOAD( "04544191.6r", 0, 0x2000, CRC(90db1bf6) SHA1(dbb9e50a8efc3b4012fcf587cc87da9ef42a1b80) )

	ROM_REGION( 0x20000, "unknown", 0 ) /* unknown rom */
	ROM_LOAD( "341.bin", 0x00000, 0x20000, CRC(8c90ca97) SHA1(dce2a680a5bc213f2f48d4baffc86ea27fe90209) ) // was read as 27c010

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "finalap3.nv",  0x000000, 0x2000, CRC(efbc6274) SHA1(f542012e467027b7bd5d7102096ff91d8c9adee3) )
ROM_END


/* FINAL LAP 3 JAPAN */
ROM_START( finalap3j )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "flt_mp0.11d",  0x000000, 0x020000, CRC(2f2a997a) SHA1(02c805b0727d55bd7782d3352d3563c490694fdb) )
	ROM_LOAD16_BYTE( "flt_mp1.13d",  0x000001, 0x020000, CRC(b505ca0b) SHA1(2dd6c264806a32466df78a0bc1f44e2e2796f81e) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "flt1_sp0.11k",  0x000000, 0x020000, CRC(e804ced1) SHA1(b31e1fddd202ba503b28455a270e8d45a348dc2e) )
	ROM_LOAD16_BYTE( "flt1_sp1.13k",  0x000001, 0x020000, CRC(3a2b24ee) SHA1(e4f16f30516dab13cc8b1cb6c80ec1df129f8851) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "flt1_snd0.7j",  0x000000, 0x020000, CRC(60b72aed) SHA1(f12e157ae5e9f373ba1d75012b869b1c4ad9fb00) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "flt_obj-0.4c",  0x000003, 0x80000, CRC(eab19ec6) SHA1(2859e88b94aa873f3b6ba22790f2211f3e172dd1) )
	ROM_LOAD32_BYTE( "flt_obj-2.4a",  0x000002, 0x80000, CRC(2a3b7ded) SHA1(455d9d6cf7d497687f93af899fc20bbff6129391) )
	ROM_LOAD32_BYTE( "flt_obj-4.8c",  0x000001, 0x80000, CRC(84aa500c) SHA1(087c0089478a270154f50f3b0f001428e80d74c7) )
	ROM_LOAD32_BYTE( "flt_obj-6.8a",  0x000000, 0x80000, CRC(33118e63) SHA1(126cc034909e05da953a1a67d6c0f18f5304b407) )
	ROM_LOAD32_BYTE( "flt_obj-1.2c",  0x200003, 0x80000, CRC(4ef37a51) SHA1(2f43691cfcd852773ae5e1d879f556f232bae877) )
	ROM_LOAD32_BYTE( "flt_obj-3.2a",  0x200002, 0x80000, CRC(b86dc7cd) SHA1(25402d7111c1277a618b313d1244c1a567ce458a) )
	ROM_LOAD32_BYTE( "flt_obj-5.5c",  0x200001, 0x80000, CRC(6a53e603) SHA1(6087c694e0e30a98c84227991d9c2e9c39c3e9ca) )
	ROM_LOAD32_BYTE( "flt_obj-7.6a",  0x200000, 0x80000, CRC(b52a85e2) SHA1(1eea10eb20ae56309397238a52e9ea0756912412) )

	ROM_REGION( 0x200000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "fltchr-0.11n", 0x000000, 0x40000, CRC(97ed5b62) SHA1(ce076ae71c6b2950be2a303829072d59732315df) )
	ROM_LOAD( "fltchr-1.11p", 0x040000, 0x40000, CRC(2e68d13c) SHA1(46bb0628da1f97e0f6865f37e53a01d2e8391255) )
	ROM_LOAD( "fltchr-2.11r", 0x080000, 0x40000, CRC(43c3abf8) SHA1(de66bcdb3e419725b7e7d9ae0c95e13ee99ec5c9) )
	ROM_LOAD( "fltchr-3.11s", 0x0c0000, 0x40000, CRC(e9b05a1f) SHA1(6b7e08f4f535fb99692920612f751a49c3cd529e) )
	ROM_LOAD( "fltchr-4.9n",  0x100000, 0x40000, CRC(5ae43767) SHA1(f3e24dd5ba2cffffd616ddee628e423aa0aec1d2) )
	ROM_LOAD( "fltchr-5.9p",  0x140000, 0x40000, CRC(b5f4e780) SHA1(a7d64d150121eda5d82f5651a06a11683220577a) )
	ROM_LOAD( "fltchr-6.9r",  0x180000, 0x40000, CRC(4b0baea2) SHA1(a75ba5294f06ddbe170988073b8f4a74a7cbcee1) )
	ROM_LOAD( "fltchr-7.9s",  0x1c0000, 0x40000, CRC(85db9e94) SHA1(918f414c1dd51f7451a9a491ba1d60f5f9a38c3e) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_256K( "flt_sha.7n",  0x000000, CRC(211bbd83) SHA1(17502830d1af1e2cfbc17e2f3bb303f2a0c27e68) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data ROMs */
	NAMCOS2_DATA_LOAD_E_128K( "flt1_d0.13s",  0x000000, CRC(80004966) SHA1(112b2a9b0ea792d5dbff1b9cf904da788aeede29) )
	NAMCOS2_DATA_LOAD_O_128K( "flt1_d1.13p",  0x000000, CRC(a2e93e8c) SHA1(9c8a5431a79153a70eb6939d16e0a5a6be235e75) )

	ROM_REGION( 0x100, "c45_road:clut", 0 ) /* PROM for road colors */
	ROM_LOAD( "fl1_3.5b", 0, 0x100, CRC(d179d99a) SHA1(4e64f284c74d2b77f893bd28aaa6489084056aa2) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "flt_voi-1.3m",  0x000000, 0x080000, CRC(4fc7c0ba) SHA1(bbfd1764fd79087bba5e6199e8916c28bed4d3f4) )
	ROM_LOAD16_BYTE( "flt_voi-2.3l",  0x100000, 0x080000, CRC(409c62df) SHA1(0c2f088168f1f92f2f767ea47522c0e8f4a10265) )

	ROM_REGION( 0x2000, "user2", 0 ) /* zoom */
	ROM_LOAD( "04544191.6r", 0, 0x2000, CRC(90db1bf6) SHA1(dbb9e50a8efc3b4012fcf587cc87da9ef42a1b80) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "finalap3.nv",  0x000000, 0x2000, CRC(efbc6274) SHA1(f542012e467027b7bd5d7102096ff91d8c9adee3) )
ROM_END

/* FINAL LAP 3 JAPAN (REV. C) */
ROM_START( finalap3jc )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "flt1_mp0c.11d",  0x000000, 0x020000, CRC(ebe1bff8) SHA1(dbfb2ba770dbbaa44d8b83c05365a97ddd18908d) )
	ROM_LOAD16_BYTE( "flt1_mp1c.13d",  0x000001, 0x020000, CRC(61099bb8) SHA1(c0e5a1bf15f5746d58ee8c510459fe76dde5a5a9) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "flt1_sp0.11k",  0x000000, 0x020000, CRC(e804ced1) SHA1(b31e1fddd202ba503b28455a270e8d45a348dc2e) )
	ROM_LOAD16_BYTE( "flt1_sp1.13k",  0x000001, 0x020000, CRC(3a2b24ee) SHA1(e4f16f30516dab13cc8b1cb6c80ec1df129f8851) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "flt1_snd0.7j",  0x000000, 0x020000, CRC(60b72aed) SHA1(f12e157ae5e9f373ba1d75012b869b1c4ad9fb00) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "flt_obj-0.4c",  0x000003, 0x80000, CRC(eab19ec6) SHA1(2859e88b94aa873f3b6ba22790f2211f3e172dd1) )
	ROM_LOAD32_BYTE( "flt_obj-2.4a",  0x000002, 0x80000, CRC(2a3b7ded) SHA1(455d9d6cf7d497687f93af899fc20bbff6129391) )
	ROM_LOAD32_BYTE( "flt_obj-4.8c",  0x000001, 0x80000, CRC(84aa500c) SHA1(087c0089478a270154f50f3b0f001428e80d74c7) )
	ROM_LOAD32_BYTE( "flt_obj-6.8a",  0x000000, 0x80000, CRC(33118e63) SHA1(126cc034909e05da953a1a67d6c0f18f5304b407) )
	ROM_LOAD32_BYTE( "flt_obj-1.2c",  0x200003, 0x80000, CRC(4ef37a51) SHA1(2f43691cfcd852773ae5e1d879f556f232bae877) )
	ROM_LOAD32_BYTE( "flt_obj-3.2a",  0x200002, 0x80000, CRC(b86dc7cd) SHA1(25402d7111c1277a618b313d1244c1a567ce458a) )
	ROM_LOAD32_BYTE( "flt_obj-5.5c",  0x200001, 0x80000, CRC(6a53e603) SHA1(6087c694e0e30a98c84227991d9c2e9c39c3e9ca) )
	ROM_LOAD32_BYTE( "flt_obj-7.6a",  0x200000, 0x80000, CRC(b52a85e2) SHA1(1eea10eb20ae56309397238a52e9ea0756912412) )

	ROM_REGION( 0x200000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "flt_chr-0.11n", 0x000000, 0x40000, CRC(97ed5b62) SHA1(ce076ae71c6b2950be2a303829072d59732315df) )
	ROM_LOAD( "flt_chr-1.11p", 0x040000, 0x40000, CRC(2e68d13c) SHA1(46bb0628da1f97e0f6865f37e53a01d2e8391255) )
	ROM_LOAD( "flt_chr-2.11r", 0x080000, 0x40000, CRC(43c3abf8) SHA1(de66bcdb3e419725b7e7d9ae0c95e13ee99ec5c9) )
	ROM_LOAD( "flt_chr-3.11s", 0x0c0000, 0x40000, CRC(e9b05a1f) SHA1(6b7e08f4f535fb99692920612f751a49c3cd529e) )
	ROM_LOAD( "flt_chr-4.9n",  0x100000, 0x40000, CRC(5ae43767) SHA1(f3e24dd5ba2cffffd616ddee628e423aa0aec1d2) )
	ROM_LOAD( "flt_chr-5.9p",  0x140000, 0x40000, CRC(b5f4e780) SHA1(a7d64d150121eda5d82f5651a06a11683220577a) )
	ROM_LOAD( "flt_chr-6.9r",  0x180000, 0x40000, CRC(4b0baea2) SHA1(a75ba5294f06ddbe170988073b8f4a74a7cbcee1) )
	ROM_LOAD( "flt_chr-7.9s",  0x1c0000, 0x40000, CRC(85db9e94) SHA1(918f414c1dd51f7451a9a491ba1d60f5f9a38c3e) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_256K( "flt_sha.7n",  0x000000, CRC(211bbd83) SHA1(17502830d1af1e2cfbc17e2f3bb303f2a0c27e68) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data ROMs */
	NAMCOS2_DATA_LOAD_E_128K( "flt1_d0.13s",  0x000000, CRC(80004966) SHA1(112b2a9b0ea792d5dbff1b9cf904da788aeede29) )
	NAMCOS2_DATA_LOAD_O_128K( "flt1_d1.13p",  0x000000, CRC(a2e93e8c) SHA1(9c8a5431a79153a70eb6939d16e0a5a6be235e75) )

	ROM_REGION( 0x100, "c45_road:clut", 0 ) /* PROM for road colors */
	ROM_LOAD( "fl1_3.5b", 0, 0x100, CRC(d179d99a) SHA1(4e64f284c74d2b77f893bd28aaa6489084056aa2) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "flt_voi-1.3m",  0x000000, 0x080000, CRC(4fc7c0ba) SHA1(bbfd1764fd79087bba5e6199e8916c28bed4d3f4) )
	ROM_LOAD16_BYTE( "flt_voi-2.3l",  0x100000, 0x080000, CRC(409c62df) SHA1(0c2f088168f1f92f2f767ea47522c0e8f4a10265) )

	ROM_REGION( 0x2000, "user2", 0 ) /* zoom */
	ROM_LOAD( "04544191.6r", 0, 0x2000, CRC(90db1bf6) SHA1(dbb9e50a8efc3b4012fcf587cc87da9ef42a1b80) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "finalap3.nv",  0x000000, 0x2000, CRC(efbc6274) SHA1(f542012e467027b7bd5d7102096ff91d8c9adee3) )
ROM_END

ROM_START( finalap3bl ) // bootleg set
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "fl3-mp0.11d",  0x000000, 0x020000, CRC(af11f52e) SHA1(7be160bc4babe780399747efdca0f61a8b4fca93) )
	ROM_LOAD16_BYTE( "fl3-mp1.13d",  0x000001, 0x020000, CRC(da9b1b48) SHA1(58b15a29759e511367d994a5ef1fae3f5cd4fa82) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "flt1sp0",  0x000000, 0x020000, CRC(e804ced1) SHA1(b31e1fddd202ba503b28455a270e8d45a348dc2e) )
	ROM_LOAD16_BYTE( "flt1sp1",  0x000001, 0x020000, CRC(3a2b24ee) SHA1(e4f16f30516dab13cc8b1cb6c80ec1df129f8851) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "flt1_snd0.7j",  0x000000, 0x020000, CRC(60b72aed) SHA1(f12e157ae5e9f373ba1d75012b869b1c4ad9fb00) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "fltobj0",  0x000003, 0x80000, CRC(eab19ec6) SHA1(2859e88b94aa873f3b6ba22790f2211f3e172dd1) )
	ROM_LOAD32_BYTE( "fltobj2",  0x000002, 0x80000, CRC(2a3b7ded) SHA1(455d9d6cf7d497687f93af899fc20bbff6129391) )
	ROM_LOAD32_BYTE( "fltobj4",  0x000001, 0x80000, CRC(84aa500c) SHA1(087c0089478a270154f50f3b0f001428e80d74c7) )
	ROM_LOAD32_BYTE( "fltobj6",  0x000000, 0x80000, CRC(33118e63) SHA1(126cc034909e05da953a1a67d6c0f18f5304b407) )
	ROM_LOAD32_BYTE( "fltobj1",  0x200003, 0x80000, CRC(4ef37a51) SHA1(2f43691cfcd852773ae5e1d879f556f232bae877) )
	ROM_LOAD32_BYTE( "fltobj3",  0x200002, 0x80000, CRC(b86dc7cd) SHA1(25402d7111c1277a618b313d1244c1a567ce458a) )
	ROM_LOAD32_BYTE( "fltobj5",  0x200001, 0x80000, CRC(6a53e603) SHA1(6087c694e0e30a98c84227991d9c2e9c39c3e9ca) )
	ROM_LOAD32_BYTE( "fltobj7",  0x200000, 0x80000, CRC(b52a85e2) SHA1(1eea10eb20ae56309397238a52e9ea0756912412) )

	ROM_REGION( 0x200000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "flt2_chr-0.bin", 0x000000, 0x40000, CRC(5954f270) SHA1(6f26365d89f38d4ab477908f32823e06f1a84e09) )
	ROM_LOAD( "fltchr-1.11p",   0x040000, 0x40000, CRC(2e68d13c) SHA1(46bb0628da1f97e0f6865f37e53a01d2e8391255) )
	ROM_LOAD( "flt2_chr-2.bin", 0x080000, 0x40000, CRC(98f3b190) SHA1(4858dab070c0c46d37148312664c6a616765c240) )
	ROM_LOAD( "fltchr-3.11s",   0x0c0000, 0x40000, CRC(e9b05a1f) SHA1(6b7e08f4f535fb99692920612f751a49c3cd529e) )
	ROM_LOAD( "fltchr-4.9n",    0x100000, 0x40000, CRC(5ae43767) SHA1(f3e24dd5ba2cffffd616ddee628e423aa0aec1d2) )
	ROM_LOAD( "fltchr-5.9p",    0x140000, 0x40000, CRC(b5f4e780) SHA1(a7d64d150121eda5d82f5651a06a11683220577a) )
	ROM_LOAD( "fltchr-6.9r",    0x180000, 0x40000, CRC(4b0baea2) SHA1(a75ba5294f06ddbe170988073b8f4a74a7cbcee1) )
	ROM_LOAD( "fltchr-7.9s",    0x1c0000, 0x40000, CRC(85db9e94) SHA1(918f414c1dd51f7451a9a491ba1d60f5f9a38c3e) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_256K( "flt2_sha.bin",  0x000000, CRC(6986565b) SHA1(df95f2ad5d1e938551d33153f2dcc4711ffd0eba) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "flt1d0.13s",  0x000000, CRC(80004966) SHA1(112b2a9b0ea792d5dbff1b9cf904da788aeede29) )
	NAMCOS2_DATA_LOAD_O_128K( "flt1d1.13p",  0x000000, CRC(a2e93e8c) SHA1(9c8a5431a79153a70eb6939d16e0a5a6be235e75) )

	ROM_REGION( 0x100, "c45_road:clut", 0 ) /* prom for road colors */
	ROM_LOAD( "fl1-3.5b", 0, 0x100, CRC(d179d99a) SHA1(4e64f284c74d2b77f893bd28aaa6489084056aa2) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "fltvoi1",  0x000000, 0x080000, CRC(4fc7c0ba) SHA1(bbfd1764fd79087bba5e6199e8916c28bed4d3f4) )
	ROM_LOAD16_BYTE( "fltvoi2",  0x100000, 0x080000, CRC(409c62df) SHA1(0c2f088168f1f92f2f767ea47522c0e8f4a10265) )

	ROM_REGION( 0x2000, "user2", 0 ) /* zoom */
	ROM_LOAD( "04544191.6r", 0, 0x2000, CRC(90db1bf6) SHA1(dbb9e50a8efc3b4012fcf587cc87da9ef42a1b80) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration and machine ID code that passes protection */
	ROM_LOAD( "finalap3bl.nv",  0x000000, 0x2000, CRC(60226586) SHA1(d66afd1149c3c95cbb0108337c530cab78327d97) )
ROM_END

/* FINEST HOUR */
ROM_START( finehour )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "fh1_mp0.bin",  0x000000, 0x020000, CRC(355d9119) SHA1(e8d834bbcfc9efa4bec9d5d38a0223bcea9e55de) )
	ROM_LOAD16_BYTE( "fh1_mp1.bin",  0x000001, 0x020000, CRC(647eb621) SHA1(441bcb492982db81ceb035cf81970eaa9cc1886c) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "fh1_sp0.bin",  0x000000, 0x020000, CRC(aa6289e9) SHA1(d20a62481c3ea386a9eee952388d9cb7137e07de) )
	ROM_LOAD16_BYTE( "fh1_sp1.bin",  0x000001, 0x020000, CRC(8532d5c7) SHA1(b32416e444393c0ba685a9d11bb7fba7a53f63db) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "fh1_sd0.bin",  0x000000, 0x020000, CRC(059a9cfd) SHA1(6b9ba6c270c90b382bd4c656e0ddca076a04a919) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "fh1_ob0.bin",  0x000003, 0x80000, CRC(b1fd86f1) SHA1(5504ca1a83c329a19d5632b9ac40cfa7e8ced304) )
	ROM_LOAD32_BYTE( "fh1_ob1.bin",  0x000002, 0x80000, CRC(519c44ce) SHA1(f4b033d1caac1944a870d94a06a40aad332a75db) )
	ROM_LOAD32_BYTE( "fh1_ob2.bin",  0x000001, 0x80000, CRC(9c5de4fa) SHA1(ead6e53d3fd7adc6f1cb4971a0858ff0098e9897) )
	ROM_LOAD32_BYTE( "fh1_ob3.bin",  0x000000, 0x80000, CRC(54d4edce) SHA1(1cf090b215f62528d13a8de6936be96bfe7d343a) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_256K( "fh1_ch0.bin",  0x000000, CRC(516900d1) SHA1(f3d95fa4c060a37164a6e3a87b056d032f3d1f6c) )
	NAMCOS2_GFXROM_LOAD_256K( "fh1_ch1.bin",  0x080000, CRC(964d06bd) SHA1(bd9a30bde66f680fdc45cb8823fac9672075a7bb) )
	NAMCOS2_GFXROM_LOAD_256K( "fh1_ch2.bin",  0x100000, CRC(fbb9449e) SHA1(291d4678e9972fd3ad2822bef90c57548d284482) )
	NAMCOS2_GFXROM_LOAD_256K( "fh1_ch3.bin",  0x180000, CRC(c18eda8a) SHA1(5e0cba196716049f3045cfe3a3f9571f9be16992) )
	NAMCOS2_GFXROM_LOAD_256K( "fh1_ch4.bin",  0x200000, CRC(80dd188a) SHA1(6d4f029983e98acee612c0f394675f7dd41208c7) )
	NAMCOS2_GFXROM_LOAD_256K( "fh1_ch5.bin",  0x280000, CRC(40969876) SHA1(6f444e7fc658ce557d0f54498614a2c05e14f280) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "fh1_rz0.bin",  0x000000, CRC(6c96c5c1) SHA1(a69321ccb17497671352ab5f4d8f331668f982d2) )
	NAMCOS2_GFXROM_LOAD_128K( "fh1_rz1.bin",  0x080000, CRC(44699eb9) SHA1(9f289fccfd56692e0dda5331160a21b6ed824d7d) )
	NAMCOS2_GFXROM_LOAD_128K( "fh1_rz2.bin",  0x100000, CRC(5ec14abf) SHA1(088b4da0f20286e72c7e02a6708ce46823f40237) )
	NAMCOS2_GFXROM_LOAD_128K( "fh1_rz3.bin",  0x180000, CRC(9f5a91b2) SHA1(962d09eca92a146bfb754aba575309530658fddb) )
	NAMCOS2_GFXROM_LOAD_128K( "fh1_rz4.bin",  0x200000, CRC(0b4379e6) SHA1(3455211a9a3c8412609fdbb98aa8911db3a47e09) )
	NAMCOS2_GFXROM_LOAD_128K( "fh1_rz5.bin",  0x280000, CRC(e034e560) SHA1(3b5ccdd93e2729875305a7016b3b9de237526358) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_256K( "fh1_sha.bin",  0x000000, CRC(15875eb0) SHA1(9225df6b01897938488461ebf0717e6d7b81d562) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "fh1_dt0.13s",  0x000000, CRC(2441c26f) SHA1(429c8f12f7bef3e62153ac8b9e00ea870db36a7b) )
	NAMCOS2_DATA_LOAD_O_128K( "fh1_dt1.13p",  0x000000, CRC(48154deb) SHA1(d6fde316f90bc847f60fa8d997504da34337ffa4) )
	NAMCOS2_DATA_LOAD_E_128K( "fh1_dt2.13r",  0x100000, CRC(12453ba4) SHA1(26ad0da6e56ece6f1ba0b0cf23d2fdae2ce24100) )
	NAMCOS2_DATA_LOAD_O_128K( "fh1_dt3.13n",  0x100000, CRC(50bab9da) SHA1(9c18e5fb810123f9d17042212e0878172e220d2a) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "fh1_vo1.bin",  0x000000, 0x080000, CRC(07560fc7) SHA1(76f3855f5a4567dc65d513e37072072c2a011e7e) )
ROM_END

/*

Four Trax
Namco 1989

The video board and CPU board are the same type as those used on Final Lap.

PCB Layouts
-----------

Top Board

8618961200
(8618963200)
|--------------------------------------------------------------------|
| TA7630  C116     M5M5178                                 49.152MHz |
|  TL084  TL084    M5M5178                                      137  |
| LC7880 YM3012    M5M5178   FX_CHR-7.9S  FX_CHR-3.11S   FX_DAT0.13S |
| LA4700     156         123 FX_CHR-6.9R  FX2_CH2.11R    FX_DAT2.13R |
|                            FX_CHR-5.9P  FX_CHR-1.11P   FX_DAT1.13P |
|140              FX_SHA.7N  FX_CHR-4.9N  FX_CHR-0.11N   FX_DAT3.13N |
|      3N*                                                           |
|  FX_VOI-1.3M        145     84256                                  |
|J     3L*                    84256       65256          65256       |
|A                                                                   |
|M   8464       2018                      FX2_SP0.11K    FX2_SP1.13K |
|M              2018                                                 |
|A   YM2151                  FX1_SD0.7J                              |
|          121  68B09           7G*            68000         148     |
|          SYS87B-1(PAL)   8464                                      |
|               SYS87B-2B(PAL)                                       |
|        SYS2C65C.3F                                                 |
|N                                                                   |
|A  DSW(8)    C65                                                    |
|M 3.579545MHz           MB8422                                      |
|C   149           LB1760     SYS87B-3(PAL)     65256       65256    |
|O                 LB1760          139                               |
|4                                  HN58C65  FX2_MP0.11D  FX2_MP1.13D|
|8         MB3771 PC900 6N137   M5M5179               68000      148 |
|--------------------------------------------------------------------|
Notes:
      68000 clocks - 12.288MHz [49.152/4]
      68B09 clock  - 2.048MHz [49.152/24]
      YM2151 clock - 3.579545MHz
      C65 clock    - 2.048MHz [49.152/24], 8.192MHz [49.152/6]
      C140 clocks  - 1.365333334MHz [49.152/36], 42.6667kHz, 85.3334kHz, 128.000kHz [49.152/384],
                     256.000kHz [49.152/192], 512.000kHz [49.152/96], 4.096MHz [49.152/12],
                     2.730666667MHz [49.152/18], 2.048MHz [49.152/24], 12.288MHz [49.152/4],
                     8.192MHz [49.152/6]
      *            - Unpopulated socket

      Namco customs
      -------------
                    C116 (QFP80)
                    156  (QFP64)
                    123  (QFP80)
                    145  (QFP80)
                    148  (QFP64, x2)
                    139  (QFP64)
                    C65  (QFP80). This is a 63B05 MCU
                    149  (DIP28)
                    137  (DIP28)
                    140  (QFP120)
                    121  (QFP64)

      Measurements
      ------------
      HSync - 15.9398kHz
      VSync - 60.6061Hz
      OSC1  - 49.15200MHz
      OSC2  - 3.57962MHz


Bottom Board

V56 VIDEO
2272961200
(2272963100)
|--------------------------------------------------------------------|
|                                                                    |
|                TMM2063     135                                     |
|                                              137                   |
| FL1-1(PAL)     TMM2063                                             |
|                                     LH5762                         |
|                TMM2063                                             |
|                                                                    |
|                TMM2063                                             |
|                                          2018             2018     |
|                134                                                 |
|                           J203           2018             2018     |
|                                                                    |
|                                                                    |
|   45                                                               |
|                                                                    |
|                                                         FL1-2(PAL) |
|                                                                    |
|                                                                    |
|                        4H*      5H*                                |
|                                                145                 |
|                        4F*      5F*                                |
|                                                                    |
|  MB81464  MB81464                                                  |
|                                                                    |
|  MB81464  MB81464                                                  |
|                    2018    107                                     |
|                                                                    |
|                    2018    FX1-1.5B                                |
|--------------------------------------------------------------------|
Notes:
      FX1-1.5B - PROM type MB7118
      *        - Unpopulated socket

      Namco customs:
                    45   (QFP80)
                    107  (SDIP64)
                    145  (QFP80)
                    134  (NDIP28)
                    135  (NDIP28)
                    137  (NDIP28)


ROM Board (mounts onto J203 of VIDEO board)

2294961301
(2294963201)
|--------------------------------------|
|      JP1       J2                    |
| 74F244  74F139    74F244             |
|                                      |
| 74F244  74F244    74F244             |
|               J1                     |
|                                      |
|FX_OBJ-12.8A     FX_OBJ-8.8C          |
|                                      |
|FX_OBJ-13.7A     FX_OBJ-9.7C          |
|                                      |
|FX_OBJ-14.6A     FX_OBJ10.6C          |
|                                      |
|FX_OBJ15.5A      FX_OBJ-11.5C         |
|                                      |
|                                      |
|                                      |
|FX_OBJ-4.4A      FX_OBJ-0.4C          |
|                                      |
|FX_OBJ-5.3A      FX_OBJ-1.3C          |
|                                      |
|FX_OBJ6.2A       FX_OBJ2.2C           |
|                                      |
|FX_OBJ7.1A       FX_OBJ-3.1C          |
|                                      |
|                                      |
|--------------------------------------|
Notes: (All ICs shown)
      JP1 - jumper, set to 2M. Alt setting is 4M
      J1  - 32 pin connector joining to VIDEO board
      J2  - 2 pin connector tied with 1 wire to a logic chip on the VIDEO board. If the connector
            is unplugged, all of the bike sprites are bad. The scenary and spectator objects are not affected.
      All ROMs are 2MBit. ROMs at 1A, 2A, 5A, 2C and 6C are 27C2001 EPROMs, all others
      are 2M 32 pin mask ROMs.
*/

ROM_START( fourtrax )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "fx2_mp0.11d",  0x000000, 0x020000, CRC(f147cd6b) SHA1(7cdadd68d55dd8fa9b19cbee1434d9266ae1f4b9) )
	ROM_LOAD16_BYTE( "fx2_mp1.13d",  0x000001, 0x020000, CRC(8af4a309) SHA1(538076359cfd08e99e42c05bcccd95df71856696) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "fx2_sp0.11k", 0x000000, 0x020000, CRC(48548e78) SHA1(b3a9de8682fe63c1c3ecab3e3f9380a884efd4af) )
	ROM_LOAD16_BYTE( "fx2_sp1.13k", 0x000001, 0x020000, CRC(d2861383) SHA1(36be5a8c8a19f35f9a9bd3ef725a83c5e58ccbe0) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "fx1_sd0.7j", 0x000000, 0x020000, CRC(acccc934) SHA1(98f1a823ba7e3f258a73d5780953f9339d438e1a) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "fx_obj-0.4c",  0x000003, 0x040000, CRC(1aa60ffa) SHA1(1fa625a52c763b8db718af14e9f3cc3e076ff83b) )
	ROM_LOAD32_BYTE( "fx_obj-1.3c",  0x100003, 0x040000, CRC(7509bc09) SHA1(823d8d884afc685dda26c1256c2d241c7f626f9e) )
	ROM_LOAD32_BYTE( "fx_obj-2.2c",  0x200003, 0x040000, CRC(243affc7) SHA1(738d62960e79b95079b2208ec48fa0f3738c7611) )
	ROM_LOAD32_BYTE( "fx_obj-3.1c",  0x300003, 0x040000, CRC(b7e5d17d) SHA1(3d8ea7cbf33b595ddf739024e8d0fccd5f9e073b) )
	ROM_LOAD32_BYTE( "fx_obj-4.4a",  0x000002, 0x040000, CRC(30add52a) SHA1(ff782d9dca96967233e435c3dd7d69ffde45db43) )
	ROM_LOAD32_BYTE( "fx_obj-5.3a",  0x100002, 0x040000, CRC(e3cd2776) SHA1(6155e9ad90b8a885125c8a76e9c068247e7693ae) )
	ROM_LOAD32_BYTE( "fx_obj-6.2a",  0x200002, 0x040000, CRC(a2d5ce4a) SHA1(bbe9df3914632a573a95fcba76442404d149fb9d) )
	ROM_LOAD32_BYTE( "fx_obj-7.1a",  0x300002, 0x040000, CRC(4d91c929) SHA1(97470a4ad7b28df83c632bfc8c309b24701275fe) )
	ROM_LOAD32_BYTE( "fx_obj-8.8c",  0x000001, 0x040000, CRC(b165acab) SHA1(86bd2cc22e25ddbf73e62426762aa72205868660) )
	ROM_LOAD32_BYTE( "fx_obj-9.7c",  0x100001, 0x040000, CRC(90f0735b) SHA1(2adbe72c6547075c0cc0386789cc1b8c1a0bc84f) )
	ROM_LOAD32_BYTE( "fx_obj-10.6c", 0x200001, 0x040000, CRC(7a01e86f) SHA1(5fde10e53cb192df0f3873cd6d59c725430948f5) )
	ROM_LOAD32_BYTE( "fx_obj-11.5c", 0x300001, 0x040000, CRC(514b3fe5) SHA1(19562ba2ac04a16d335bdc81b34d929f7ff9161c) )
	ROM_LOAD32_BYTE( "fx_obj-12.8a", 0x000000, 0x040000, CRC(f5e23b78) SHA1(99896bd7c6663e3f57cb5d206964b81b5d64c8b6) )
	ROM_LOAD32_BYTE( "fx_obj-13.7a", 0x100000, 0x040000, CRC(04a25007) SHA1(0c33450b0d6907754dbf1914849d1630baa824bd) )
	ROM_LOAD32_BYTE( "fx_obj-14.6a", 0x200000, 0x040000, CRC(c1658c77) SHA1(ec689d0e5cf95085c193aa8949c6ec6e7243338b) )
	ROM_LOAD32_BYTE( "fx_obj-15.5a", 0x300000, 0x040000, CRC(2bc909b3) SHA1(29c668d6d12ccdee25e97373bc4786894858d463) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-0.11n",  0x000000, CRC(6658c1c3) SHA1(64b5466e0f94cf5f3cb92915a26331748f67041a) )
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-1.11p",  0x080000, CRC(3a888943) SHA1(6540b417003045dfa1401a1ff04ac55b86cc177f) )
	NAMCOS2_GFXROM_LOAD_128K( "fx2_chr-2.11r", 0x100000, CRC(fdf1e86b) SHA1(91a61c10a9e92f8bbc26ffc9cb72deb31378a843) )
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-3.11s",  0x180000, CRC(47fa7e61) SHA1(cc2eed81ddb4f942dd7a07e474760e4608eb4da0) )
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-4.9n",   0x200000, CRC(c720c5f5) SHA1(f68f369bbefe01c770314ea597dd88587638c62a) )
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-5.9p",   0x280000, CRC(9eacdbc8) SHA1(ca4061afc9e61eeb543f2a3740812abf6a1049bc) )
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-6.9r",   0x300000, CRC(c3dba42e) SHA1(2b5a8fabec11ccd44156ecfccf86fc713845d262) )
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-7.9s",   0x380000, CRC(c009f3ae) SHA1(394beed29bda97f4f5ba532bc0bd22177154746b) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "fx_sha.7n", 0x000000, CRC(f7aa4af7) SHA1(b18ffda9e35beb0f072825e2899691be370f33b1) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_256K( "fx_dat0.13s", 0x000000, CRC(63abf69b) SHA1(6776991eeff434bf9720f49ad6e62c37fc7ddf40) )
	NAMCOS2_DATA_LOAD_O_256K( "fx_dat1.13p", 0x000000, CRC(725bed14) SHA1(bbf886ac7f8c2c7857bc0b5d7f8fc7e63e8e9559) )
	NAMCOS2_DATA_LOAD_E_256K( "fx_dat2.13r", 0x100000, CRC(71e4a5a0) SHA1(a0188c920a43c5e69e25464627094b6b6ed26a59) )
	NAMCOS2_DATA_LOAD_O_256K( "fx_dat3.13n", 0x100000, CRC(605725f7) SHA1(b94ce0ec37f879a5e46a097058cb2dd57e2281f1) )

	ROM_REGION( 0x100, "c45_road:clut", 0 ) /* prom for road colors */
	ROM_LOAD( "fx1_1.5b", 0, 0x100, CRC(85ffd753) SHA1(7dbc8c295204877f41289141a146aa4f5f9f9c96) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "fx_voi-1.3m", 0x000000, 0x080000, CRC(6173364f) SHA1(cc426f49b7e87b11f1f51e8e10db7cad87ffb44d) )
ROM_END

ROM_START( fourtraxj )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "fx1_mp0.11d",  0x000000, 0x020000, CRC(35a690b1) SHA1(0e5157d0b1b54aaac22f725e712bb61104f199e3) )
	ROM_LOAD16_BYTE( "fx1_mp1.13d",  0x000001, 0x020000, CRC(005bda8b) SHA1(2f258a30ff307e06b715321e48b3b649bce6648a) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "fx1_sp0.11k", 0x000000, 0x020000, CRC(48548e78) SHA1(b3a9de8682fe63c1c3ecab3e3f9380a884efd4af) ) // same content as fx2 set, different label
	ROM_LOAD16_BYTE( "fx1_sp1.13k", 0x000001, 0x020000, CRC(d2861383) SHA1(36be5a8c8a19f35f9a9bd3ef725a83c5e58ccbe0) ) // same content as fx2 set, different label

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "fx1_sd0.7j", 0x000000, 0x020000, CRC(acccc934) SHA1(98f1a823ba7e3f258a73d5780953f9339d438e1a) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "fx_obj-0.4c",  0x000003, 0x040000, CRC(1aa60ffa) SHA1(1fa625a52c763b8db718af14e9f3cc3e076ff83b) )
	ROM_LOAD32_BYTE( "fx_obj-1.3c",  0x100003, 0x040000, CRC(7509bc09) SHA1(823d8d884afc685dda26c1256c2d241c7f626f9e) )
	ROM_LOAD32_BYTE( "fx_obj-2.2c",  0x200003, 0x040000, CRC(243affc7) SHA1(738d62960e79b95079b2208ec48fa0f3738c7611) )
	ROM_LOAD32_BYTE( "fx_obj-3.1c",  0x300003, 0x040000, CRC(b7e5d17d) SHA1(3d8ea7cbf33b595ddf739024e8d0fccd5f9e073b) )
	ROM_LOAD32_BYTE( "fx_obj-4.4a",  0x000002, 0x040000, CRC(30add52a) SHA1(ff782d9dca96967233e435c3dd7d69ffde45db43) )
	ROM_LOAD32_BYTE( "fx_obj-5.3a",  0x100002, 0x040000, CRC(e3cd2776) SHA1(6155e9ad90b8a885125c8a76e9c068247e7693ae) )
	ROM_LOAD32_BYTE( "fx_obj-6.2a",  0x200002, 0x040000, CRC(a2d5ce4a) SHA1(bbe9df3914632a573a95fcba76442404d149fb9d) )
	ROM_LOAD32_BYTE( "fx_obj-7.1a",  0x300002, 0x040000, CRC(4d91c929) SHA1(97470a4ad7b28df83c632bfc8c309b24701275fe) )
	ROM_LOAD32_BYTE( "fx_obj-8.8c",  0x000001, 0x040000, CRC(b165acab) SHA1(86bd2cc22e25ddbf73e62426762aa72205868660) )
	ROM_LOAD32_BYTE( "fx_obj-9.7c",  0x100001, 0x040000, CRC(90f0735b) SHA1(2adbe72c6547075c0cc0386789cc1b8c1a0bc84f) )
	ROM_LOAD32_BYTE( "fx_obj-10.6c", 0x200001, 0x040000, CRC(7a01e86f) SHA1(5fde10e53cb192df0f3873cd6d59c725430948f5) )
	ROM_LOAD32_BYTE( "fx_obj-11.5c", 0x300001, 0x040000, CRC(514b3fe5) SHA1(19562ba2ac04a16d335bdc81b34d929f7ff9161c) )
	ROM_LOAD32_BYTE( "fx_obj-12.8a", 0x000000, 0x040000, CRC(f5e23b78) SHA1(99896bd7c6663e3f57cb5d206964b81b5d64c8b6) )
	ROM_LOAD32_BYTE( "fx_obj-13.7a", 0x100000, 0x040000, CRC(04a25007) SHA1(0c33450b0d6907754dbf1914849d1630baa824bd) )
	ROM_LOAD32_BYTE( "fx_obj-14.6a", 0x200000, 0x040000, CRC(c1658c77) SHA1(ec689d0e5cf95085c193aa8949c6ec6e7243338b) )
	ROM_LOAD32_BYTE( "fx_obj-15.5a", 0x300000, 0x040000, CRC(2bc909b3) SHA1(29c668d6d12ccdee25e97373bc4786894858d463) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-0.11n", 0x000000, CRC(6658c1c3) SHA1(64b5466e0f94cf5f3cb92915a26331748f67041a) )
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-1.11p", 0x080000, CRC(3a888943) SHA1(6540b417003045dfa1401a1ff04ac55b86cc177f) )
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-2.11r", 0x100000, CRC(179e4ec6) SHA1(2936ef847c5a9e527f7198f65a21eee7ac36891c) )
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-3.11s", 0x180000, CRC(47fa7e61) SHA1(cc2eed81ddb4f942dd7a07e474760e4608eb4da0) )
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-4.9n",  0x200000, CRC(c720c5f5) SHA1(f68f369bbefe01c770314ea597dd88587638c62a) )
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-5.9p",  0x280000, CRC(9eacdbc8) SHA1(ca4061afc9e61eeb543f2a3740812abf6a1049bc) )
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-6.9r",  0x300000, CRC(c3dba42e) SHA1(2b5a8fabec11ccd44156ecfccf86fc713845d262) )
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-7.9s",  0x380000, CRC(c009f3ae) SHA1(394beed29bda97f4f5ba532bc0bd22177154746b) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "fx_sha.7n", 0x000000, CRC(f7aa4af7) SHA1(b18ffda9e35beb0f072825e2899691be370f33b1) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_256K( "fx_dat0.13s", 0x000000, CRC(63abf69b) SHA1(6776991eeff434bf9720f49ad6e62c37fc7ddf40) )
	NAMCOS2_DATA_LOAD_O_256K( "fx_dat1.13p", 0x000000, CRC(725bed14) SHA1(bbf886ac7f8c2c7857bc0b5d7f8fc7e63e8e9559) )
	NAMCOS2_DATA_LOAD_E_256K( "fx_dat2.13r", 0x100000, CRC(71e4a5a0) SHA1(a0188c920a43c5e69e25464627094b6b6ed26a59) )
	NAMCOS2_DATA_LOAD_O_256K( "fx_dat3.13n", 0x100000, CRC(605725f7) SHA1(b94ce0ec37f879a5e46a097058cb2dd57e2281f1) )

	ROM_REGION( 0x100, "c45_road:clut", 0 ) /* prom for road colors */
	ROM_LOAD( "fx1_1.5b", 0, 0x100, CRC(85ffd753) SHA1(7dbc8c295204877f41289141a146aa4f5f9f9c96) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "fx_voi-1.3m", 0x000000, 0x080000, CRC(6173364f) SHA1(cc426f49b7e87b11f1f51e8e10db7cad87ffb44d) )
ROM_END

/* This is a strange set, it's based on the fx2 set, but with one of the 68k pair modified (21 bytes changed) and a unique GFX ROM

   Changes seem to be related to the trackside advertising banners.  This was around the same time Super Monaco GP was forced to
   remove 'real' advertising banners, so could be related.

   The changed graphic ROM has tiles for an additional 'awaiting entry' screen, but it is unclear where they are used,
   the same tiles in the fx2 set are blank, assuming that one isn't a bad dump.

   Previous dumps of the program / sub CPUs were 0x00 filled instead of 0xff filled in the 2nd half due to a dumping error
*/

ROM_START( fourtraxa )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "fx4_mpr-0a.11d",  0x000000, 0x020000, CRC(f147cd6b) SHA1(7cdadd68d55dd8fa9b19cbee1434d9266ae1f4b9) ) // == fx2_mp0.11d
	ROM_LOAD16_BYTE( "fx4_mpr-1a.13d",  0x000001, 0x020000, CRC(d1138c85) SHA1(32bf68ae36f72b84f3c3df28425147b6aaac1edf) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "fx1_sp0.11k", 0x000000, 0x020000, CRC(48548e78) SHA1(b3a9de8682fe63c1c3ecab3e3f9380a884efd4af) ) // same content as fx2 set, different label
	ROM_LOAD16_BYTE( "fx1_sp1.13k", 0x000001, 0x020000, CRC(d2861383) SHA1(36be5a8c8a19f35f9a9bd3ef725a83c5e58ccbe0) ) // same content as fx2 set, different label

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "fx1_sd0.7j", 0x000000, 0x020000, CRC(acccc934) SHA1(98f1a823ba7e3f258a73d5780953f9339d438e1a) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "fx_obj-0.4c",  0x000003, 0x040000, CRC(1aa60ffa) SHA1(1fa625a52c763b8db718af14e9f3cc3e076ff83b) )
	ROM_LOAD32_BYTE( "fx_obj-1.3c",  0x100003, 0x040000, CRC(7509bc09) SHA1(823d8d884afc685dda26c1256c2d241c7f626f9e) )
	ROM_LOAD32_BYTE( "fx_obj-2.2c",  0x200003, 0x040000, CRC(243affc7) SHA1(738d62960e79b95079b2208ec48fa0f3738c7611) )
	ROM_LOAD32_BYTE( "fx_obj-3.1c",  0x300003, 0x040000, CRC(b7e5d17d) SHA1(3d8ea7cbf33b595ddf739024e8d0fccd5f9e073b) )
	ROM_LOAD32_BYTE( "fx_obj-4.4a",  0x000002, 0x040000, CRC(30add52a) SHA1(ff782d9dca96967233e435c3dd7d69ffde45db43) )
	ROM_LOAD32_BYTE( "fx_obj-5.3a",  0x100002, 0x040000, CRC(e3cd2776) SHA1(6155e9ad90b8a885125c8a76e9c068247e7693ae) )
	ROM_LOAD32_BYTE( "fx_obj-6.2a",  0x200002, 0x040000, CRC(a2d5ce4a) SHA1(bbe9df3914632a573a95fcba76442404d149fb9d) )
	ROM_LOAD32_BYTE( "fx_obj-7.1a",  0x300002, 0x040000, CRC(4d91c929) SHA1(97470a4ad7b28df83c632bfc8c309b24701275fe) )
	ROM_LOAD32_BYTE( "fx_obj-8.8c",  0x000001, 0x040000, CRC(b165acab) SHA1(86bd2cc22e25ddbf73e62426762aa72205868660) )
	ROM_LOAD32_BYTE( "fx_obj-9.7c",  0x100001, 0x040000, CRC(90f0735b) SHA1(2adbe72c6547075c0cc0386789cc1b8c1a0bc84f) )
	ROM_LOAD32_BYTE( "fx_obj-10.6c", 0x200001, 0x040000, CRC(7a01e86f) SHA1(5fde10e53cb192df0f3873cd6d59c725430948f5) )
	ROM_LOAD32_BYTE( "fx_obj-11.5c", 0x300001, 0x040000, CRC(514b3fe5) SHA1(19562ba2ac04a16d335bdc81b34d929f7ff9161c) )
	ROM_LOAD32_BYTE( "fx_obj-12.8a", 0x000000, 0x040000, CRC(f5e23b78) SHA1(99896bd7c6663e3f57cb5d206964b81b5d64c8b6) )
	ROM_LOAD32_BYTE( "fx_obj-13.7a", 0x100000, 0x040000, CRC(04a25007) SHA1(0c33450b0d6907754dbf1914849d1630baa824bd) )
	ROM_LOAD32_BYTE( "fx_obj-14.6a", 0x200000, 0x040000, CRC(c1658c77) SHA1(ec689d0e5cf95085c193aa8949c6ec6e7243338b) )
	ROM_LOAD32_BYTE( "fx_obj-15.5a", 0x300000, 0x040000, CRC(2bc909b3) SHA1(29c668d6d12ccdee25e97373bc4786894858d463) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-0.11n",   0x000000, CRC(6658c1c3) SHA1(64b5466e0f94cf5f3cb92915a26331748f67041a) )
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-1.11p",   0x080000, CRC(3a888943) SHA1(6540b417003045dfa1401a1ff04ac55b86cc177f) )
	NAMCOS2_GFXROM_LOAD_128K( "fx4_chr-2a.11r", 0x100000, CRC(a5d1ab10) SHA1(8624ef0911c57155b46e184d10cb122d4a41a2de) ) // unique to this set, mask ROM intentionally replaced
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-3.11s",   0x180000, CRC(47fa7e61) SHA1(cc2eed81ddb4f942dd7a07e474760e4608eb4da0) )
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-4.9n",    0x200000, CRC(c720c5f5) SHA1(f68f369bbefe01c770314ea597dd88587638c62a) )
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-5.9p",    0x280000, CRC(9eacdbc8) SHA1(ca4061afc9e61eeb543f2a3740812abf6a1049bc) )
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-6.9r",    0x300000, CRC(c3dba42e) SHA1(2b5a8fabec11ccd44156ecfccf86fc713845d262) )
	NAMCOS2_GFXROM_LOAD_128K( "fx_chr-7.9s",    0x380000, CRC(c009f3ae) SHA1(394beed29bda97f4f5ba532bc0bd22177154746b) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "fx_sha.7n", 0x000000, CRC(f7aa4af7) SHA1(b18ffda9e35beb0f072825e2899691be370f33b1) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_256K( "fx_dat0.13s", 0x000000, CRC(63abf69b) SHA1(6776991eeff434bf9720f49ad6e62c37fc7ddf40) )
	NAMCOS2_DATA_LOAD_O_256K( "fx_dat1.13p", 0x000000, CRC(725bed14) SHA1(bbf886ac7f8c2c7857bc0b5d7f8fc7e63e8e9559) )
	NAMCOS2_DATA_LOAD_E_256K( "fx_dat2.13r", 0x100000, CRC(71e4a5a0) SHA1(a0188c920a43c5e69e25464627094b6b6ed26a59) )
	NAMCOS2_DATA_LOAD_O_256K( "fx_dat3.13n", 0x100000, CRC(605725f7) SHA1(b94ce0ec37f879a5e46a097058cb2dd57e2281f1) )

	ROM_REGION( 0x100, "c45_road:clut", 0 ) /* prom for road colors */
	ROM_LOAD( "fx1_1.5b", 0, 0x100, CRC(85ffd753) SHA1(7dbc8c295204877f41289141a146aa4f5f9f9c96) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "fx_voi-1.3m", 0x000000, 0x080000, CRC(6173364f) SHA1(cc426f49b7e87b11f1f51e8e10db7cad87ffb44d) )
ROM_END

/* MARVEL LAND (JAPAN) */
ROM_START( marvland )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "mv1-mpr0.bin",  0x000000, 0x010000, CRC(8369120f) SHA1(58cf481bf97f74a91ecc5ff77696528441b41b04) )
	ROM_LOAD16_BYTE( "mv1-mpr1.bin",  0x000001, 0x010000, CRC(6d5442cc) SHA1(8cdaf6e1ec735740ace78393df2d867a213a4725) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "mv1-spr0.bin",  0x000000, 0x010000, CRC(c3909925) SHA1(bf76cb77c38787574bf75caf868700316894895c) )
	ROM_LOAD16_BYTE( "mv1-spr1.bin",  0x000001, 0x010000, CRC(1c5599f5) SHA1(6bdf11da4e2a56c6bb6011977b045d9537d0597f) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "mv1-snd0.bin",  0x000000, 0x020000, CRC(51b8ccd7) SHA1(5aacb020c12d9a3c43c098f3abd8358bc18acc64) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	NAMCOS2_SPRROM_LOAD_256K( "mv1-obj0.bin",  0x000003, CRC(73a29361) SHA1(fc8ac9a063c1f18ae619ddca3062491774c86040) )
	NAMCOS2_SPRROM_LOAD_256K( "mv1-obj1.bin",  0x000002, CRC(abbe4a99) SHA1(7f8df4b40236b97a0dce984698308647d5803244) )
	NAMCOS2_SPRROM_LOAD_256K( "mv1-obj2.bin",  0x000001, CRC(753659e0) SHA1(2662acf7bec528c7ac4181f62154581e304eea82) )
	NAMCOS2_SPRROM_LOAD_256K( "mv1-obj3.bin",  0x000000, CRC(d1ce7339) SHA1(a89a0ef39b6ac3fdaf6a2b3c04fd048827fcdb13) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_256K( "mv1-chr0.bin",  0x000000, CRC(1c7e8b4f) SHA1(b9d61895d9c9302c5cb5f7bb7f045b2014c12317) )
	NAMCOS2_GFXROM_LOAD_256K( "mv1-chr1.bin",  0x080000, CRC(01e4cafd) SHA1(27c911d6d4501233094826cf1b4b809b832d6d9f) )
	NAMCOS2_GFXROM_LOAD_256K( "mv1-chr2.bin",  0x100000, CRC(198fcc6f) SHA1(b28f97d58fb2365843bbc3764cb59bfb9d5dfd92) )
	NAMCOS2_GFXROM_LOAD_256K( "mv1-chr3.bin",  0x180000, CRC(ed6f22a5) SHA1(62ab2a2746abbbed533a5b519bbb0d603030cdca) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "mv1-roz0.bin",  0x000000, CRC(7381a5a9) SHA1(0515630f124725adfd21575b390209833bb6a6ef) )
	NAMCOS2_GFXROM_LOAD_128K( "mv1-roz1.bin",  0x080000, CRC(e899482e) SHA1(caa511baba1805c485503353efdade9e218f2ba5) )
	NAMCOS2_GFXROM_LOAD_128K( "mv1-roz2.bin",  0x100000, CRC(de141290) SHA1(c1daa6c01ba592138cffef02edfa0928f2232079) )
	NAMCOS2_GFXROM_LOAD_128K( "mv1-roz3.bin",  0x180000, CRC(e310324d) SHA1(936c7aeace677ed51a720e4ae96cdac0f4984a9b) )
	NAMCOS2_GFXROM_LOAD_128K( "mv1-roz4.bin",  0x200000, CRC(48ddc5a9) SHA1(c524b18d7b4526227d5b99d7e4a4582ce2ecd373) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_256K( "mv1-sha.bin",  0x000000, CRC(a47db5d3) SHA1(110e26412aa84f229773049112709be457b7a6ff) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "mv1-dat0.13s",  0x000000, CRC(e15f412e) SHA1(d3ff006d4577540a690c912e94897a1b638ac265) )
	NAMCOS2_DATA_LOAD_O_128K( "mv1-dat1.13p",  0x000000, CRC(73e1545a) SHA1(a04034e56fef69fb2a2eb88f2f392c376e52d00d) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "mv1-voi1.bin",  0x000000, 0x080000, CRC(de5cac09) SHA1(2d73e54c4f159e52db2c403a59d6c137cce6f53e) )
ROM_END

/* MARVEL LAND (USA) */
ROM_START( marvlandup )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "mv2_mpr0",  0x000000, 0x020000, CRC(d8b14fee) SHA1(8b5615106426efad45c651f1d6b9a6e3238bc242) )
	ROM_LOAD16_BYTE( "mv2_mpr1",  0x000001, 0x020000, CRC(29ff2738) SHA1(9f493f32ae1c4e7ef48d7e208c63a222636bda06) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "mv2_spr0",  0x000000, 0x010000, CRC(aa418f29) SHA1(413798d9c1d98cfcadb045a5436aaee61ce6718f) )
	ROM_LOAD16_BYTE( "mv2_spr1",  0x000001, 0x010000, CRC(dbd94def) SHA1(56a8d7acd483bc4d12c8bc5b7e90ffdb132be670) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "mv2_snd0",  0x000000, 0x020000, CRC(a5b99162) SHA1(cafe8d1dae1e981c7ff9b70076b3e1d52cd806f7) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	NAMCOS2_SPRROM_LOAD_256K( "mv1-obj0.bin",  0x000003, CRC(73a29361) SHA1(fc8ac9a063c1f18ae619ddca3062491774c86040) )
	NAMCOS2_SPRROM_LOAD_256K( "mv1-obj1.bin",  0x000002, CRC(abbe4a99) SHA1(7f8df4b40236b97a0dce984698308647d5803244) )
	NAMCOS2_SPRROM_LOAD_256K( "mv1-obj2.bin",  0x000001, CRC(753659e0) SHA1(2662acf7bec528c7ac4181f62154581e304eea82) )
	NAMCOS2_SPRROM_LOAD_256K( "mv1-obj3.bin",  0x000000, CRC(d1ce7339) SHA1(a89a0ef39b6ac3fdaf6a2b3c04fd048827fcdb13) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_256K( "mv1-chr0.bin",  0x000000, CRC(1c7e8b4f) SHA1(b9d61895d9c9302c5cb5f7bb7f045b2014c12317) )
	NAMCOS2_GFXROM_LOAD_256K( "mv1-chr1.bin",  0x080000, CRC(01e4cafd) SHA1(27c911d6d4501233094826cf1b4b809b832d6d9f) )
	NAMCOS2_GFXROM_LOAD_256K( "mv1-chr2.bin",  0x100000, CRC(198fcc6f) SHA1(b28f97d58fb2365843bbc3764cb59bfb9d5dfd92) )
	NAMCOS2_GFXROM_LOAD_256K( "mv1-chr3.bin",  0x180000, CRC(ed6f22a5) SHA1(62ab2a2746abbbed533a5b519bbb0d603030cdca) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "mv1-roz0.bin",  0x000000, CRC(7381a5a9) SHA1(0515630f124725adfd21575b390209833bb6a6ef) )
	NAMCOS2_GFXROM_LOAD_128K( "mv1-roz1.bin",  0x080000, CRC(e899482e) SHA1(caa511baba1805c485503353efdade9e218f2ba5) )
	NAMCOS2_GFXROM_LOAD_128K( "mv1-roz2.bin",  0x100000, CRC(de141290) SHA1(c1daa6c01ba592138cffef02edfa0928f2232079) )
	NAMCOS2_GFXROM_LOAD_128K( "mv1-roz3.bin",  0x180000, CRC(e310324d) SHA1(936c7aeace677ed51a720e4ae96cdac0f4984a9b) )
	NAMCOS2_GFXROM_LOAD_128K( "mv1-roz4.bin",  0x200000, CRC(48ddc5a9) SHA1(c524b18d7b4526227d5b99d7e4a4582ce2ecd373) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_256K( "mv1-sha.bin",  0x000000, CRC(a47db5d3) SHA1(110e26412aa84f229773049112709be457b7a6ff) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "mv2_dat0.13s",  0x000000, CRC(62e6318b) SHA1(5c26bc054298f2a0980f7e82ba1844bc9f5eff98) )
	NAMCOS2_DATA_LOAD_O_128K( "mv2_dat1.13p",  0x000000, CRC(8a6902ca) SHA1(272e8699b872b3a50d72090b43c57493e6642bfe) )
	NAMCOS2_DATA_LOAD_E_128K( "mv2_dat2.13r",  0x100000, CRC(f5c6408c) SHA1(568fb08d0763dc91674d708fa2d15ca952956145) )
	NAMCOS2_DATA_LOAD_O_128K( "mv2_dat3.13n",  0x100000, CRC(6df76955) SHA1(fcfb520399acdd3776f66944121d6980552d3100) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "mv1-voi1.bin",  0x000000, 0x080000, BAD_DUMP CRC(de5cac09) SHA1(2d73e54c4f159e52db2c403a59d6c137cce6f53e) ) // either undumped, or PCB was wrongly populated with JP samples ROM?
ROM_END

/* METAL HAWK */
ROM_START( metlhawk )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "mh2mp0c.11d",  0x000000, 0x020000, CRC(cd7dae6e) SHA1(73600e44923446ce5a6aa692aebfc1ba5a6491bc) )
	ROM_LOAD16_BYTE( "mh2mp1c.13d",  0x000001, 0x020000, CRC(e52199fd) SHA1(0c626b89f2dd9fb91654b759f2a902a28268ac84) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "mh1sp0f.11k",  0x000000, 0x010000, CRC(2c141fea) SHA1(b767bbb69843df9916a042cae5a8ff745e30b9d3) )
	ROM_LOAD16_BYTE( "mh1sp1f.13k",  0x000001, 0x010000, CRC(8ccf98e0) SHA1(c4a0b573553fd609751e6f227f7ee3f28f78f589) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "mh1s0.7j",  0x000000, 0x020000, CRC(79e054cf) SHA1(934a15a1bdb2751f28cc23471d1c5e5b9af80815) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	NAMCOS2_SPRROM_LOAD_256K( "mhobj-0.5d", 0x000000, CRC(52ae6620) SHA1(0d08109db975d26caa06ac47b71ac8d26a993f9c) )
	NAMCOS2_SPRROM_LOAD_256K( "mhobj-1.5b", 0x000001, CRC(2c2a1291) SHA1(b644586cf623f9a113dbf3a8d951e62507e93179) )
	NAMCOS2_SPRROM_LOAD_256K( "mhobj-2.6d", 0x000002, CRC(6221b927) SHA1(caa106a47bc9e24fb90752175dc5156f7249d12a) )
	NAMCOS2_SPRROM_LOAD_256K( "mhobj-3.6b", 0x000003, CRC(fd09f2f1) SHA1(4ef5aef0fab89699cb6007103c286c54bd91b66e) )
	NAMCOS2_SPRROM_LOAD_256K( "mhobj-4.5c", 0x200000, CRC(e3590e1a) SHA1(9afffa54a63e676f5d78a01c76ca50cd795dd6e9) )
	NAMCOS2_SPRROM_LOAD_256K( "mhobj-5.5a", 0x200001, CRC(b85c0d07) SHA1(e1ae542c0e884ef454ba57ecdfd007b85f2dc59d) )
	NAMCOS2_SPRROM_LOAD_256K( "mhobj-6.6c", 0x200002, CRC(90c4523d) SHA1(c6f84da3187ebb747445b1b7499acf5adc0f39d8) )
	NAMCOS2_SPRROM_LOAD_256K( "mhobj-7.6a", 0x200003, CRC(f00edb39) SHA1(08b9037a014dc35ac53df6cd552adf8b36efad12) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "mhchr-0.11n",  0x000000, CRC(e2da1b14) SHA1(95adcd7814fc77ad8b6c208c8da86b1557c5ec22) )
	NAMCOS2_GFXROM_LOAD_128K( "mhchr-1.11p",  0x080000, CRC(023c78f9) SHA1(d11fa58c2429fa1db2081e7d653a01577d0850ac) )
	NAMCOS2_GFXROM_LOAD_128K( "mhchr-2.11r",  0x100000, CRC(ece47e91) SHA1(a07e056f85a9207d6cc8e844a1afe6622bfe5f9b) )
	NAMCOS2_GFXROM_LOAD_128K( "mh1c3.11s",    0x180000, CRC(9303aa7f) SHA1(1c7ed8d16a706f47bbd2970afd36b467ce19e9ee) )

	ROM_REGION( 0x200000, "c169roz", 0 ) /* ROZ Tiles */
	ROM_LOAD( "mhr0z-0.2d",  0x000000, 0x40000, CRC(30ade98f) SHA1(6bf3d2f729e9f5e2c3f1a69845afbbfcafbf2d56) )
	ROM_LOAD( "mhr0z-1.2c",  0x040000, 0x40000, CRC(a7fff42a) SHA1(5e13a11b8277842f6e1ae3a7f772f51ab07d0bfe) )
	ROM_LOAD( "mhr0z-2.2b",  0x080000, 0x40000, CRC(6abec820) SHA1(bcacda96f6e69046dc59125374f169753ddda683) )
	ROM_LOAD( "mhr0z-3.2a",  0x0c0000, 0x40000, CRC(d53cec6d) SHA1(433112d199c92fbc5df90d5b2d7b2e46665d144a) )
	ROM_LOAD( "mhr0z-4.3d",  0x100000, 0x40000, CRC(922213e2) SHA1(cbb6ac73c1a48b328c2c1e5999f6d4f194bbaa76) )
	ROM_LOAD( "mhr0z-5.3c",  0x140000, 0x40000, CRC(78418a54) SHA1(af5bbc0c2643a25000c8f25bbdd773bfe08ae362) )
	ROM_LOAD( "mhr0z-6.3b",  0x180000, 0x40000, CRC(6c74977e) SHA1(ccdc1922a3e759eec0b68d1a7fde271dde54d9b2) )
	ROM_LOAD( "mhr0z-7.3a",  0x1c0000, 0x40000, CRC(68a19cbd) SHA1(99759f7a670e41d92aba36c68fd66d74250445a3) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape (tiles) */
	NAMCOS2_GFXROM_LOAD_128K( "mh1sha.7n",  0x000000, CRC(6ac22294) SHA1(6b2cd7589691e6d01f16b162db7c928767750bd3) )

	ROM_REGION( 0x80000, "c169roz:mask", 0 ) /* Mask shape (ROZ) */
	ROM_LOAD( "mh-rzsh.bin",    0x000000, 0x40000, CRC(5090b1cf) SHA1(b814f8309a6133c6ece5f20161ebd02a981da66f) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "mh1d0.13s",  0x000000, CRC(8b178ac7) SHA1(210d31baf0aaba1af5efc15ec05714123f669030) )
	NAMCOS2_DATA_LOAD_O_128K( "mh1d1.13p",  0x000000, CRC(10684fd6) SHA1(1e39d32dcf7ab9a146aa01f47e2737142874eede) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "mhvoi-1.bin",  0x000000, 0x080000, CRC(2723d137) SHA1(f67334f8e456ae9e6aee39f0cf5e73449838f37f) )
	ROM_LOAD16_BYTE( "mhvoi-2.bin",  0x100000, 0x080000, CRC(dbc92d91) SHA1(a8c50f607d5283c8bd9688d2149b811e7ddb77dd) )

	ROM_REGION( 0x2000, "user2", 0 ) /* sprite zoom lookup table */
	ROM_LOAD( "mh5762.7p",    0x00000,  0x002000, CRC(90db1bf6) SHA1(dbb9e50a8efc3b4012fcf587cc87da9ef42a1b80) )

	ROM_REGION( 0x0500, "plds", 0 )
	ROM_LOAD( "ampal16l8a-sys87b-1.4g", 0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "ampal16l8a-sys87b-2.5e", 0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal12l10-sys87b-3.8d",   0x0400, 0x0040, CRC(d3ae64a6) SHA1(8e56f447908246e84d5a79df1a1cd3d5c8a040fb) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "metlhawk.nv",  0x000000, 0x2000, CRC(547cb0dc) SHA1(965ce5f8a42f5268cda2e77e4be115ccf57f1952) )
ROM_END

/* METAL HAWK (Japan) */
ROM_START( metlhawkj )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "mh1mpr0f.11d",  0x000000, 0x020000, CRC(e22dfb6e) SHA1(41a8d30081bc7206aae655626dcef01e8f744905) )
	ROM_LOAD16_BYTE( "mh1mpr1f.13d",  0x000001, 0x020000, CRC(d139a7b7) SHA1(ba68221ca6f91ab619e9b2c478d8988abfc86c8c) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "mh1sp0f.11k",  0x000000, 0x010000, CRC(2c141fea) SHA1(b767bbb69843df9916a042cae5a8ff745e30b9d3) )
	ROM_LOAD16_BYTE( "mh1sp1f.13k",  0x000001, 0x010000, CRC(8ccf98e0) SHA1(c4a0b573553fd609751e6f227f7ee3f28f78f589) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "mh1s0.7j",  0x000000, 0x020000, CRC(79e054cf) SHA1(934a15a1bdb2751f28cc23471d1c5e5b9af80815) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	NAMCOS2_SPRROM_LOAD_256K( "mhobj-0.5d", 0x000000, CRC(52ae6620) SHA1(0d08109db975d26caa06ac47b71ac8d26a993f9c) )
	NAMCOS2_SPRROM_LOAD_256K( "mhobj-1.5b", 0x000001, CRC(2c2a1291) SHA1(b644586cf623f9a113dbf3a8d951e62507e93179) )
	NAMCOS2_SPRROM_LOAD_256K( "mhobj-2.6d", 0x000002, CRC(6221b927) SHA1(caa106a47bc9e24fb90752175dc5156f7249d12a) )
	NAMCOS2_SPRROM_LOAD_256K( "mhobj-3.6b", 0x000003, CRC(fd09f2f1) SHA1(4ef5aef0fab89699cb6007103c286c54bd91b66e) )
	NAMCOS2_SPRROM_LOAD_256K( "mhobj-4.5c", 0x200000, CRC(e3590e1a) SHA1(9afffa54a63e676f5d78a01c76ca50cd795dd6e9) )
	NAMCOS2_SPRROM_LOAD_256K( "mhobj-5.5a", 0x200001, CRC(b85c0d07) SHA1(e1ae542c0e884ef454ba57ecdfd007b85f2dc59d) )
	NAMCOS2_SPRROM_LOAD_256K( "mhobj-6.6c", 0x200002, CRC(90c4523d) SHA1(c6f84da3187ebb747445b1b7499acf5adc0f39d8) )
	NAMCOS2_SPRROM_LOAD_256K( "mhobj-7.6a", 0x200003, CRC(f00edb39) SHA1(08b9037a014dc35ac53df6cd552adf8b36efad12) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "mhchr-0.11n",  0x000000, CRC(e2da1b14) SHA1(95adcd7814fc77ad8b6c208c8da86b1557c5ec22) )
	NAMCOS2_GFXROM_LOAD_128K( "mhchr-1.11p",  0x080000, CRC(023c78f9) SHA1(d11fa58c2429fa1db2081e7d653a01577d0850ac) )
	NAMCOS2_GFXROM_LOAD_128K( "mhchr-2.11r",  0x100000, CRC(ece47e91) SHA1(a07e056f85a9207d6cc8e844a1afe6622bfe5f9b) )
	NAMCOS2_GFXROM_LOAD_128K( "mh1c3.11s",    0x180000, CRC(9303aa7f) SHA1(1c7ed8d16a706f47bbd2970afd36b467ce19e9ee) )

	ROM_REGION( 0x200000, "c169roz", 0 ) /* ROZ Tiles */
	ROM_LOAD( "mhr0z-0.2d",  0x000000, 0x40000, CRC(30ade98f) SHA1(6bf3d2f729e9f5e2c3f1a69845afbbfcafbf2d56) )
	ROM_LOAD( "mhr0z-1.2c",  0x040000, 0x40000, CRC(a7fff42a) SHA1(5e13a11b8277842f6e1ae3a7f772f51ab07d0bfe) )
	ROM_LOAD( "mhr0z-2.2b",  0x080000, 0x40000, CRC(6abec820) SHA1(bcacda96f6e69046dc59125374f169753ddda683) )
	ROM_LOAD( "mhr0z-3.2a",  0x0c0000, 0x40000, CRC(d53cec6d) SHA1(433112d199c92fbc5df90d5b2d7b2e46665d144a) )
	ROM_LOAD( "mhr0z-4.3d",  0x100000, 0x40000, CRC(922213e2) SHA1(cbb6ac73c1a48b328c2c1e5999f6d4f194bbaa76) )
	ROM_LOAD( "mhr0z-5.3c",  0x140000, 0x40000, CRC(78418a54) SHA1(af5bbc0c2643a25000c8f25bbdd773bfe08ae362) )
	ROM_LOAD( "mhr0z-6.3b",  0x180000, 0x40000, CRC(6c74977e) SHA1(ccdc1922a3e759eec0b68d1a7fde271dde54d9b2) )
	ROM_LOAD( "mhr0z-7.3a",  0x1c0000, 0x40000, CRC(68a19cbd) SHA1(99759f7a670e41d92aba36c68fd66d74250445a3) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape (tiles) */
	NAMCOS2_GFXROM_LOAD_128K( "mh1sha.7n",  0x000000, CRC(6ac22294) SHA1(6b2cd7589691e6d01f16b162db7c928767750bd3) )

	ROM_REGION( 0x80000, "c169roz:mask", 0 ) /* Mask shape (ROZ) */
	ROM_LOAD( "mh-rzsh.bin",    0x000000, 0x40000, CRC(5090b1cf) SHA1(b814f8309a6133c6ece5f20161ebd02a981da66f) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "mh1d0.13s",  0x000000, CRC(8b178ac7) SHA1(210d31baf0aaba1af5efc15ec05714123f669030) )
	NAMCOS2_DATA_LOAD_O_128K( "mh1d1.13p",  0x000000, CRC(10684fd6) SHA1(1e39d32dcf7ab9a146aa01f47e2737142874eede) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "mhvoi-1.bin",  0x000000, 0x080000, CRC(2723d137) SHA1(f67334f8e456ae9e6aee39f0cf5e73449838f37f) )
	ROM_LOAD16_BYTE( "mhvoi-2.bin",  0x100000, 0x080000, CRC(dbc92d91) SHA1(a8c50f607d5283c8bd9688d2149b811e7ddb77dd) )

	ROM_REGION( 0x2000, "user2", 0 ) /* sprite zoom lookup table */
	ROM_LOAD( "mh5762.7p",    0x00000,  0x002000, CRC(90db1bf6) SHA1(dbb9e50a8efc3b4012fcf587cc87da9ef42a1b80) )

	ROM_REGION( 0x0500, "plds", 0 )
	ROM_LOAD( "ampal16l8a-sys87b-1.4g", 0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "ampal16l8a-sys87b-2.5e", 0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal12l10-sys87b-3.8d",   0x0400, 0x0040, CRC(d3ae64a6) SHA1(8e56f447908246e84d5a79df1a1cd3d5c8a040fb) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "metlhawk.nv",  0x000000, 0x2000, CRC(547cb0dc) SHA1(965ce5f8a42f5268cda2e77e4be115ccf57f1952) )
ROM_END

/* MIRAI NINJA */
ROM_START( mirninja )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "mn_mpr0e.bin",  0x000000, 0x010000, CRC(fa75f977) SHA1(15f8ce9417d663ea659e2c35d5b318c5e275f997) )
	ROM_LOAD16_BYTE( "mn_mpr1e.bin",  0x000001, 0x010000, CRC(58ddd464) SHA1(1b0c0023f7130c8b4cdc207ed32582f107953b51) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "mn1_spr0.bin",  0x000000, 0x010000, CRC(3f1a17be) SHA1(0d6a4e26235f44db4ad217b859c3d215f4e9b423) )
	ROM_LOAD16_BYTE( "mn1_spr1.bin",  0x000001, 0x010000, CRC(2bc66f60) SHA1(7b778ee3a24f57d43c9bcffbdb77cf8be2463c2d) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "mn_snd0.bin",  0x000000, 0x020000, CRC(6aa1ae84) SHA1(2186f93c4ccc4c202fa14d80b440060237659fc5) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65b.bin",  0x000000, 0x008000, CRC(e9f2922a) SHA1(5767d2f85e1eb3de19192e73b02221f28b1fbb83) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	NAMCOS2_SPRROM_LOAD_128K( "mn_obj0.bin",  0x000003, CRC(6bd1e290) SHA1(11e5f7adef0d7a519246c6d88f9371e49a6b49e9) )
	NAMCOS2_SPRROM_LOAD_128K( "mn_obj1.bin",  0x000002, CRC(5e8503be) SHA1(e03e13e70932b65e1bd560f685eda107f00a8bb6) )
	NAMCOS2_SPRROM_LOAD_128K( "mn_obj2.bin",  0x000001, CRC(a96d9b45) SHA1(5ad32ef08c38bff368590e0549c4b4552af5c2c8) )
	NAMCOS2_SPRROM_LOAD_128K( "mn_obj3.bin",  0x000000, CRC(0086ef8b) SHA1(cd282868e9f05a305816cec6043d31bfa26314b3) )
	NAMCOS2_SPRROM_LOAD_128K( "mn_obj4.bin",  0x200003, CRC(b3f48755) SHA1(d3b4a0b5d9939dad9b63a85e86afe5aa26dc9849) )
	NAMCOS2_SPRROM_LOAD_128K( "mn_obj5.bin",  0x200002, CRC(c21e995b) SHA1(03022f11f314f1a6a568cf75850117c98b7c0ce1) )
	NAMCOS2_SPRROM_LOAD_128K( "mn_obj6.bin",  0x200001, CRC(a052c582) SHA1(eadf07df0e7e13c6e51672860aad4c22b5dcc853) )
	NAMCOS2_SPRROM_LOAD_128K( "mn_obj7.bin",  0x200000, CRC(1854c6f5) SHA1(f49d18655d05ea9abf1dded17abc61855dba61ef) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "mn_chr0.bin",  0x000000, CRC(4f66df26) SHA1(7ca1215cb33b9c0898fc17721618a3129d751722) )
	NAMCOS2_GFXROM_LOAD_128K( "mn_chr1.bin",  0x080000, CRC(f5de5ea7) SHA1(58ba4a5cca631e53b685db342697625c9c9ea50c) )
	NAMCOS2_GFXROM_LOAD_128K( "mn_chr2.bin",  0x100000, CRC(9ff61924) SHA1(27537743b2df32eb492ec933faabd149e3283256) )
	NAMCOS2_GFXROM_LOAD_128K( "mn_chr3.bin",  0x180000, CRC(ba208bf5) SHA1(924478a44155707b79698518901fba4e21485740) )
	NAMCOS2_GFXROM_LOAD_128K( "mn_chr4.bin",  0x200000, CRC(0ef00aff) SHA1(01bf3753d11a3e5ea41fd205d4384f6949ad1c01) )
	NAMCOS2_GFXROM_LOAD_128K( "mn_chr5.bin",  0x280000, CRC(4cd9d377) SHA1(188e1a88dbd4f6aedd6fbe5e22d4f3a0a88dec3a) )
	NAMCOS2_GFXROM_LOAD_128K( "mn_chr6.bin",  0x300000, CRC(114aca76) SHA1(d2c6bdfdd0e42cd0c6f99517321c2105e5fc780d) )
	NAMCOS2_GFXROM_LOAD_128K( "mn_chr7.bin",  0x380000, CRC(2d5705d3) SHA1(690a50b3950a1cf9c27461aa3722c3f1f6a90c87) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "mn_roz0.bin",  0x000000, CRC(677a4f25) SHA1(8ca64833189c75c3f4efd022dbddc54dc2632ec1) )
	NAMCOS2_GFXROM_LOAD_128K( "mn_roz1.bin",  0x080000, CRC(f00ae572) SHA1(cd7f28b2ba03a0bb4d5702ffa36b1140560c9541) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "mn_sha.bin",  0x000000, CRC(c28af90f) SHA1(8b7f95375eb32c3e30c2a55b7f543235f56d3a13) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "mn1_dat0.13s",  0x000000, CRC(104bcca8) SHA1(e8368d0dc51bf0653143bf2261d7ed5b54d92941) )
	NAMCOS2_DATA_LOAD_O_128K( "mn1_dat1.13p",  0x000000, CRC(d2a918fb) SHA1(032b7a7bcc60c41325e7b35df9a932e68cdd0788) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "mn_voi1.bin",  0x000000, 0x080000, CRC(2ca3573c) SHA1(b2af101730de4ccc68acc1ed143c21a8c81f64db) )
	ROM_LOAD16_BYTE( "mn_voi2.bin",  0x100000, 0x080000, CRC(466c3b47) SHA1(9c282ffda8b0620ae60789c81c6e36c086a9a335) )
ROM_END

ROM_START( mirninjaa )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "mn1_mpr0",  0x000000, 0x010000, CRC(6d061fd6) SHA1(42d197edc8c8020dc5020ddb187bac6625acc41f) )
	ROM_LOAD16_BYTE( "mn1_mpr1",  0x000001, 0x010000, CRC(2ece6323) SHA1(1769b0a05e657dcf271f25f4be7452811af6691f) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "mn1_spr0.bin",  0x000000, 0x010000, CRC(3f1a17be) SHA1(0d6a4e26235f44db4ad217b859c3d215f4e9b423) )
	ROM_LOAD16_BYTE( "mn1_spr1.bin",  0x000001, 0x010000, CRC(2bc66f60) SHA1(7b778ee3a24f57d43c9bcffbdb77cf8be2463c2d) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "mn_snd0.bin",  0x000000, 0x020000, CRC(6aa1ae84) SHA1(2186f93c4ccc4c202fa14d80b440060237659fc5) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65b.bin",  0x000000, 0x008000, CRC(e9f2922a) SHA1(5767d2f85e1eb3de19192e73b02221f28b1fbb83) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	NAMCOS2_SPRROM_LOAD_128K( "mn_obj0.bin",  0x000003, CRC(6bd1e290) SHA1(11e5f7adef0d7a519246c6d88f9371e49a6b49e9) )
	NAMCOS2_SPRROM_LOAD_128K( "mn_obj1.bin",  0x000002, CRC(5e8503be) SHA1(e03e13e70932b65e1bd560f685eda107f00a8bb6) )
	NAMCOS2_SPRROM_LOAD_128K( "mn_obj2.bin",  0x000001, CRC(a96d9b45) SHA1(5ad32ef08c38bff368590e0549c4b4552af5c2c8) )
	NAMCOS2_SPRROM_LOAD_128K( "mn_obj3.bin",  0x000000, CRC(0086ef8b) SHA1(cd282868e9f05a305816cec6043d31bfa26314b3) )
	NAMCOS2_SPRROM_LOAD_128K( "mn_obj4.bin",  0x200003, CRC(b3f48755) SHA1(d3b4a0b5d9939dad9b63a85e86afe5aa26dc9849) )
	NAMCOS2_SPRROM_LOAD_128K( "mn_obj5.bin",  0x200002, CRC(c21e995b) SHA1(03022f11f314f1a6a568cf75850117c98b7c0ce1) )
	NAMCOS2_SPRROM_LOAD_128K( "mn_obj6.bin",  0x200001, CRC(a052c582) SHA1(eadf07df0e7e13c6e51672860aad4c22b5dcc853) )
	NAMCOS2_SPRROM_LOAD_128K( "mn_obj7.bin",  0x200000, CRC(1854c6f5) SHA1(f49d18655d05ea9abf1dded17abc61855dba61ef) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "mn_chr0.bin",  0x000000, CRC(4f66df26) SHA1(7ca1215cb33b9c0898fc17721618a3129d751722) )
	NAMCOS2_GFXROM_LOAD_128K( "mn_chr1.bin",  0x080000, CRC(f5de5ea7) SHA1(58ba4a5cca631e53b685db342697625c9c9ea50c) )
	NAMCOS2_GFXROM_LOAD_128K( "mn_chr2.bin",  0x100000, CRC(9ff61924) SHA1(27537743b2df32eb492ec933faabd149e3283256) )
	NAMCOS2_GFXROM_LOAD_128K( "mn_chr3.bin",  0x180000, CRC(ba208bf5) SHA1(924478a44155707b79698518901fba4e21485740) )
	NAMCOS2_GFXROM_LOAD_128K( "mn_chr4.bin",  0x200000, CRC(0ef00aff) SHA1(01bf3753d11a3e5ea41fd205d4384f6949ad1c01) )
	NAMCOS2_GFXROM_LOAD_128K( "mn_chr5.bin",  0x280000, CRC(4cd9d377) SHA1(188e1a88dbd4f6aedd6fbe5e22d4f3a0a88dec3a) )
	NAMCOS2_GFXROM_LOAD_128K( "mn_chr6.bin",  0x300000, CRC(114aca76) SHA1(d2c6bdfdd0e42cd0c6f99517321c2105e5fc780d) )
	NAMCOS2_GFXROM_LOAD_128K( "mn_chr7.bin",  0x380000, CRC(2d5705d3) SHA1(690a50b3950a1cf9c27461aa3722c3f1f6a90c87) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "mn_roz0.bin",  0x000000, CRC(677a4f25) SHA1(8ca64833189c75c3f4efd022dbddc54dc2632ec1) )
	NAMCOS2_GFXROM_LOAD_128K( "mn_roz1.bin",  0x080000, CRC(f00ae572) SHA1(cd7f28b2ba03a0bb4d5702ffa36b1140560c9541) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "mn_sha.bin",  0x000000, CRC(c28af90f) SHA1(8b7f95375eb32c3e30c2a55b7f543235f56d3a13) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "mn1_dat0.13s",  0x000000, CRC(104bcca8) SHA1(e8368d0dc51bf0653143bf2261d7ed5b54d92941) )
	NAMCOS2_DATA_LOAD_O_128K( "mn1_dat1.13p",  0x000000, CRC(d2a918fb) SHA1(032b7a7bcc60c41325e7b35df9a932e68cdd0788) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "mn_voi1.bin",  0x000000, 0x080000, CRC(2ca3573c) SHA1(b2af101730de4ccc68acc1ed143c21a8c81f64db) )
	ROM_LOAD16_BYTE( "mn_voi2.bin",  0x100000, 0x080000, CRC(466c3b47) SHA1(9c282ffda8b0620ae60789c81c6e36c086a9a335) )
ROM_END

/* ORDYNE */
ROM_START( ordyne )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "or2_mp0.mpr0",  0x000000, 0x020000, CRC(31a1742b) SHA1(6fbb6ca90829a8187dc256201e1e70bb583cb4fd) )
	ROM_LOAD16_BYTE( "or2_mp1.mpr1",  0x000001, 0x020000, CRC(c80c6b73) SHA1(183f894efe19d03d18546736ec53d24d32d24edb) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "or1_sp0.spr0",  0x000000, 0x010000, CRC(01ef6638) SHA1(403abd548014b671658aa7d40414b09c095c831d) )
	ROM_LOAD16_BYTE( "or1_sp1.spr1",  0x000001, 0x010000, CRC(b632adc3) SHA1(3d301d4ce9bf835c0f4c8ef883dba3fc4e3a5f12) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "or1_sd.snd0",  0x000000, 0x020000, CRC(c41e5d22) SHA1(7cb98d8068ddf92aadf0228b977df2ce1d866466) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2_c65b.3f",  0x000000, 0x008000, CRC(e9f2922a) SHA1(5767d2f85e1eb3de19192e73b02221f28b1fbb83) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-0.obj0",  0x000003, CRC(67b2b9e4) SHA1(4e589c28ed23224e40d3c68055ada0136cbf94cb) )
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-1.obj1",  0x000002, CRC(8a54fa5e) SHA1(8f71a79dc3bdf8ea4f11cfc31338836ab0c695a5) )
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-2.obj2",  0x000001, CRC(a2c1cca0) SHA1(f2e8b1b09751695c18bbfbdbe1765e3802833850) )
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-3.obj3",  0x000000, CRC(e0ad292c) SHA1(15c1198134e5aa1ea399ad628f478b3440dd6111) )
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-4.obj4",  0x200003, CRC(7aefba59) SHA1(99b827fabcd32dc665653b8aeda3498f97d5cebb) )
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-5.obj5",  0x200002, CRC(e4025be9) SHA1(64ac7f782d7143becffd0c6318008b2f764ae9a3) )
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-6.obj6",  0x200001, CRC(e284c30c) SHA1(f77a3cd81ac1d0fb06317db51b576f1aaa9bbec7) )
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-7.obj7",  0x200000, CRC(262b7112) SHA1(1f275eeb621d28a2efb3be9dad76606eeba78e8b) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "or_chr-0.chr0",  0x000000, CRC(e7c47934) SHA1(c86791ea11a72cd2b59293dca74fa08ec86edc80) )
	NAMCOS2_GFXROM_LOAD_128K( "or_chr-1.chr1",  0x080000, CRC(874b332d) SHA1(307ce6ad8013dbd0528a8c5d736bff21a34c4318) )
	NAMCOS2_GFXROM_LOAD_128K( "or_chr-3.chr3",  0x180000, CRC(5471a834) SHA1(91bd76eb6e03d6fdf636381a5ce1123291bd49ba) )
	NAMCOS2_GFXROM_LOAD_128K( "or_chr-5.chr5",  0x280000, CRC(a7d3a296) SHA1(ad258825371548e719c7fa0fc5a4813bf34172d5) )
	NAMCOS2_GFXROM_LOAD_128K( "or_chr-6.chr6",  0x300000, CRC(3adc09c8) SHA1(5302796c7a7f3621e2a068b5a3932f0020785836) )
	NAMCOS2_GFXROM_LOAD_128K( "or2_chr7.chr7",  0x380000, CRC(8c0d2ab7) SHA1(5e0e61623fcbaac261763fbb42079f32d1ac963c) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "or_roz-0.roz0",  0x000000, CRC(c88a9e6b) SHA1(58d73615d6d3446407dd192b7ae36dcdc1dced24) )
	NAMCOS2_GFXROM_LOAD_128K( "or_roz-1.roz1",  0x080000, CRC(c20cc749) SHA1(93142454f428ec3386bc94f1b8f7300f8e0414bc) )
	NAMCOS2_GFXROM_LOAD_128K( "or_roz-2.roz2",  0x100000, CRC(148c9866) SHA1(e2cf7f76a6e0adc22e8b0c347cf5bb9d913b6b0c) )
	NAMCOS2_GFXROM_LOAD_128K( "or_roz-3.roz3",  0x180000, CRC(4e727b7e) SHA1(272ffb137bdfabef83d6180c8667ec7e08265ea6) )
	NAMCOS2_GFXROM_LOAD_128K( "or_roz-4.roz4",  0x200000, CRC(17b04396) SHA1(410dd04538f5d9d147dfefc658b0640a771e1032) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "or_shape.shape",  0x000000, CRC(7aec9dee) SHA1(2e774e4cff69173b4524d6b25a6a449ba5498a6a) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "or1_d0.13s",  0x000000, CRC(de214f7a) SHA1(59883c7886b403306c30e51d7f49225483792650) )
	NAMCOS2_DATA_LOAD_O_128K( "or1_d1.13p",  0x000000, CRC(25e3e6c8) SHA1(ad093f15edaea71f6c7226c6e4f3c2130d418013) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "or_voi1.voice1",  0x000000, 0x080000, CRC(369e0bca) SHA1(2a921bb373dd043bd7b2a30e5e46ec3b8b3b5c8d) )
	ROM_LOAD16_BYTE( "or_voi2.voice2",  0x100000, 0x080000, CRC(9f4cd7b5) SHA1(10941dd5ab3846c0cb2543655944eaec742f8f21) )

	ROM_REGION( 0x2000, "user2", 0 ) /* sprite zoom lookup table */
	ROM_LOAD( "lh5762.6n",    0x00000,  0x002000, CRC(90db1bf6) SHA1(dbb9e50a8efc3b4012fcf587cc87da9ef42a1b80) )
ROM_END

/* ORDYNE */
ROM_START( ordyneje )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "or1_mp0e.mpr0",  0x000000, 0x020000, CRC(5e2f9052) SHA1(0bd5c2f3a1834beeddce5561833a36da9439d30c) )
	ROM_LOAD16_BYTE( "or1_mp1e.mpr1",  0x000001, 0x020000, CRC(367a8fcf) SHA1(af981338679939e99cd0733e7658a7d78c5e7ec3) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "or1_sp0.spr0",  0x000000, 0x010000, CRC(01ef6638) SHA1(403abd548014b671658aa7d40414b09c095c831d) )
	ROM_LOAD16_BYTE( "or1_sp1.spr1",  0x000001, 0x010000, CRC(b632adc3) SHA1(3d301d4ce9bf835c0f4c8ef883dba3fc4e3a5f12) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "or1_sd.snd0",  0x000000, 0x020000, CRC(c41e5d22) SHA1(7cb98d8068ddf92aadf0228b977df2ce1d866466) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2_c65b.3f",  0x000000, 0x008000, CRC(e9f2922a) SHA1(5767d2f85e1eb3de19192e73b02221f28b1fbb83) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-0.obj0",  0x000003, CRC(67b2b9e4) SHA1(4e589c28ed23224e40d3c68055ada0136cbf94cb) )
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-1.obj1",  0x000002, CRC(8a54fa5e) SHA1(8f71a79dc3bdf8ea4f11cfc31338836ab0c695a5) )
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-2.obj2",  0x000001, CRC(a2c1cca0) SHA1(f2e8b1b09751695c18bbfbdbe1765e3802833850) )
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-3.obj3",  0x000000, CRC(e0ad292c) SHA1(15c1198134e5aa1ea399ad628f478b3440dd6111) )
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-4.obj4",  0x200003, CRC(7aefba59) SHA1(99b827fabcd32dc665653b8aeda3498f97d5cebb) )
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-5.obj5",  0x200002, CRC(e4025be9) SHA1(64ac7f782d7143becffd0c6318008b2f764ae9a3) )
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-6.obj6",  0x200001, CRC(e284c30c) SHA1(f77a3cd81ac1d0fb06317db51b576f1aaa9bbec7) )
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-7.obj7",  0x200000, CRC(262b7112) SHA1(1f275eeb621d28a2efb3be9dad76606eeba78e8b) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "or_chr-0.chr0",  0x000000, CRC(e7c47934) SHA1(c86791ea11a72cd2b59293dca74fa08ec86edc80) )
	NAMCOS2_GFXROM_LOAD_128K( "or_chr-1.chr1",  0x080000, CRC(874b332d) SHA1(307ce6ad8013dbd0528a8c5d736bff21a34c4318) )
	NAMCOS2_GFXROM_LOAD_128K( "or_chr-3.chr3",  0x180000, CRC(5471a834) SHA1(91bd76eb6e03d6fdf636381a5ce1123291bd49ba) )
	NAMCOS2_GFXROM_LOAD_128K( "or_chr-5.chr5",  0x280000, CRC(a7d3a296) SHA1(ad258825371548e719c7fa0fc5a4813bf34172d5) )
	NAMCOS2_GFXROM_LOAD_128K( "or_chr-6.chr6",  0x300000, CRC(3adc09c8) SHA1(5302796c7a7f3621e2a068b5a3932f0020785836) )
	NAMCOS2_GFXROM_LOAD_128K( "or1_ch7e.chr7",  0x380000, CRC(8c0d2ab7) SHA1(5e0e61623fcbaac261763fbb42079f32d1ac963c) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "or_roz-0.roz0",  0x000000, CRC(c88a9e6b) SHA1(58d73615d6d3446407dd192b7ae36dcdc1dced24) )
	NAMCOS2_GFXROM_LOAD_128K( "or_roz-1.roz1",  0x080000, CRC(c20cc749) SHA1(93142454f428ec3386bc94f1b8f7300f8e0414bc) )
	NAMCOS2_GFXROM_LOAD_128K( "or_roz-2.roz2",  0x100000, CRC(148c9866) SHA1(e2cf7f76a6e0adc22e8b0c347cf5bb9d913b6b0c) )
	NAMCOS2_GFXROM_LOAD_128K( "or_roz-3.roz3",  0x180000, CRC(4e727b7e) SHA1(272ffb137bdfabef83d6180c8667ec7e08265ea6) )
	NAMCOS2_GFXROM_LOAD_128K( "or_roz-4.roz4",  0x200000, CRC(17b04396) SHA1(410dd04538f5d9d147dfefc658b0640a771e1032) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "or_shape.shape",  0x000000, CRC(7aec9dee) SHA1(2e774e4cff69173b4524d6b25a6a449ba5498a6a) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "or1_d0.13s",  0x000000, CRC(de214f7a) SHA1(59883c7886b403306c30e51d7f49225483792650) )
	NAMCOS2_DATA_LOAD_O_128K( "or1_d1.13p",  0x000000, CRC(25e3e6c8) SHA1(ad093f15edaea71f6c7226c6e4f3c2130d418013) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "or_voi1.voice1",  0x000000, 0x080000, CRC(369e0bca) SHA1(2a921bb373dd043bd7b2a30e5e46ec3b8b3b5c8d) )
	ROM_LOAD16_BYTE( "or_voi2.voice2",  0x100000, 0x080000, CRC(9f4cd7b5) SHA1(10941dd5ab3846c0cb2543655944eaec742f8f21) )

	ROM_REGION( 0x2000, "user2", 0 ) /* sprite zoom lookup table */
	ROM_LOAD( "lh5762.6n",    0x00000,  0x002000, CRC(90db1bf6) SHA1(dbb9e50a8efc3b4012fcf587cc87da9ef42a1b80) )
ROM_END

/* ORDYNE */
ROM_START( ordynej )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "or1_mp0.mpr0",  0x000000, 0x020000, CRC(f5929ed3) SHA1(7926c736904646e2918a708e33c7f9b4aa75d929) )
	ROM_LOAD16_BYTE( "or1_mp1.mpr1",  0x000001, 0x020000, CRC(c1c8c1e2) SHA1(153ebfdbc2bc73ea4f4628ebba4d90530b7ecd9d) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "or1_sp0.spr0",  0x000000, 0x010000, CRC(01ef6638) SHA1(403abd548014b671658aa7d40414b09c095c831d) )
	ROM_LOAD16_BYTE( "or1_sp1.spr1",  0x000001, 0x010000, CRC(b632adc3) SHA1(3d301d4ce9bf835c0f4c8ef883dba3fc4e3a5f12) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "or1_sd.snd0",  0x000000, 0x020000, CRC(c41e5d22) SHA1(7cb98d8068ddf92aadf0228b977df2ce1d866466) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2_c65b.3f",  0x000000, 0x008000, CRC(e9f2922a) SHA1(5767d2f85e1eb3de19192e73b02221f28b1fbb83) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-0.obj0",  0x000003, CRC(67b2b9e4) SHA1(4e589c28ed23224e40d3c68055ada0136cbf94cb) )
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-1.obj1",  0x000002, CRC(8a54fa5e) SHA1(8f71a79dc3bdf8ea4f11cfc31338836ab0c695a5) )
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-2.obj2",  0x000001, CRC(a2c1cca0) SHA1(f2e8b1b09751695c18bbfbdbe1765e3802833850) )
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-3.obj3",  0x000000, CRC(e0ad292c) SHA1(15c1198134e5aa1ea399ad628f478b3440dd6111) )
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-4.obj4",  0x200003, CRC(7aefba59) SHA1(99b827fabcd32dc665653b8aeda3498f97d5cebb) )
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-5.obj5",  0x200002, CRC(e4025be9) SHA1(64ac7f782d7143becffd0c6318008b2f764ae9a3) )
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-6.obj6",  0x200001, CRC(e284c30c) SHA1(f77a3cd81ac1d0fb06317db51b576f1aaa9bbec7) )
	NAMCOS2_SPRROM_LOAD_128K( "or_obj-7.obj7",  0x200000, CRC(262b7112) SHA1(1f275eeb621d28a2efb3be9dad76606eeba78e8b) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "or_chr-0.chr0",  0x000000, CRC(e7c47934) SHA1(c86791ea11a72cd2b59293dca74fa08ec86edc80) )
	NAMCOS2_GFXROM_LOAD_128K( "or_chr-1.chr1",  0x080000, CRC(874b332d) SHA1(307ce6ad8013dbd0528a8c5d736bff21a34c4318) )
	NAMCOS2_GFXROM_LOAD_128K( "or_chr-3.chr3",  0x180000, CRC(5471a834) SHA1(91bd76eb6e03d6fdf636381a5ce1123291bd49ba) )
	NAMCOS2_GFXROM_LOAD_128K( "or_chr-5.chr5",  0x280000, CRC(a7d3a296) SHA1(ad258825371548e719c7fa0fc5a4813bf34172d5) )
	NAMCOS2_GFXROM_LOAD_128K( "or_chr-6.chr6",  0x300000, CRC(3adc09c8) SHA1(5302796c7a7f3621e2a068b5a3932f0020785836) )
	NAMCOS2_GFXROM_LOAD_128K( "or_chr-7.chr7",  0x380000, CRC(f050a152) SHA1(e8d868c15f6319a4d4434ba889f1dfd160056c23) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "or_roz-0.roz0",  0x000000, CRC(c88a9e6b) SHA1(58d73615d6d3446407dd192b7ae36dcdc1dced24) )
	NAMCOS2_GFXROM_LOAD_128K( "or_roz-1.roz1",  0x080000, CRC(c20cc749) SHA1(93142454f428ec3386bc94f1b8f7300f8e0414bc) )
	NAMCOS2_GFXROM_LOAD_128K( "or_roz-2.roz2",  0x100000, CRC(148c9866) SHA1(e2cf7f76a6e0adc22e8b0c347cf5bb9d913b6b0c) )
	NAMCOS2_GFXROM_LOAD_128K( "or_roz-3.roz3",  0x180000, CRC(4e727b7e) SHA1(272ffb137bdfabef83d6180c8667ec7e08265ea6) )
	NAMCOS2_GFXROM_LOAD_128K( "or_roz-4.roz4",  0x200000, CRC(17b04396) SHA1(410dd04538f5d9d147dfefc658b0640a771e1032) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "or_shape.shape",  0x000000, CRC(7aec9dee) SHA1(2e774e4cff69173b4524d6b25a6a449ba5498a6a) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "or1_d0.13s",  0x000000, CRC(de214f7a) SHA1(59883c7886b403306c30e51d7f49225483792650) )
	NAMCOS2_DATA_LOAD_O_128K( "or1_d1.13p",  0x000000, CRC(25e3e6c8) SHA1(ad093f15edaea71f6c7226c6e4f3c2130d418013) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "or_voi1.voice1",  0x000000, 0x080000, CRC(369e0bca) SHA1(2a921bb373dd043bd7b2a30e5e46ec3b8b3b5c8d) )
	ROM_LOAD16_BYTE( "or_voi2.voice2",  0x100000, 0x080000, CRC(9f4cd7b5) SHA1(10941dd5ab3846c0cb2543655944eaec742f8f21) )

	ROM_REGION( 0x2000, "user2", 0 ) /* sprite zoom lookup table */
	ROM_LOAD( "lh5762.6n",    0x00000,  0x002000, CRC(90db1bf6) SHA1(dbb9e50a8efc3b4012fcf587cc87da9ef42a1b80) )
ROM_END

/* PHELIOS */
ROM_START( phelios )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "ps2_mpr0.mpr0",  0x000000, 0x020000, CRC(28f04e1a) SHA1(06cb053f7281ec9cb59319a4aa1262c128b369c8) )
	ROM_LOAD16_BYTE( "ps2_mpr1.mpr1",  0x000001, 0x020000, CRC(2546501a) SHA1(288cdfdf16feb61c9b49ab660efdf48020583274) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "ps2_spr0.spr0",  0x000000, 0x010000, CRC(e9c6987e) SHA1(325532b065999276e4629b10c27d7df083e81c33) )
	ROM_LOAD16_BYTE( "ps2_spr1.spr1",  0x000001, 0x010000, CRC(02b074fb) SHA1(4025c3dd83e26ac87de9f2f73753d1cb1211c1ae) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "ps2_snd0.snd0",  0x000000, 0x020000, CRC(da694838) SHA1(e1a20aedd18c49dcd12717e8ce479a5a441c10e6) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2_c65c.3f",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	NAMCOS2_SPRROM_LOAD_256K( "ps_obj-0.obj0",  0x000003, CRC(f323db2b) SHA1(fa3c42c618da06af161ad3f8aa1283e9c4bd63c0) )
	NAMCOS2_SPRROM_LOAD_256K( "ps_obj-1.obj1",  0x000002, CRC(faf7c2f5) SHA1(d0d33eddaf5de2b639717db83a85f441d81a5924) )
	NAMCOS2_SPRROM_LOAD_256K( "ps_obj-2.obj2",  0x000001, CRC(828178ba) SHA1(d35ab8020ebaad0b4c0b24fa9edc2886b713a8ed) )
	NAMCOS2_SPRROM_LOAD_256K( "ps_obj-3.obj3",  0x000000, CRC(e84771c8) SHA1(17bede39d8b703005b288f3596c4aaca266fa06b) )
	NAMCOS2_SPRROM_LOAD_256K( "ps_obj-4.obj4",  0x200003, CRC(81ea86c6) SHA1(27400b4e1d53c47bc6b821439c294d879bf58ae4) )
	NAMCOS2_SPRROM_LOAD_256K( "ps_obj-5.obj5",  0x200002, CRC(aaebd51a) SHA1(7acb88cfcaf7e7c41de171ed5952e1d0a13ef302) )
	NAMCOS2_SPRROM_LOAD_256K( "ps_obj-6.obj6",  0x200001, CRC(032ea497) SHA1(89f4deed8fa076683abc1f2e961ceb920ab9848d) )
	NAMCOS2_SPRROM_LOAD_256K( "ps_obj-7.obj7",  0x200000, CRC(f6183b36) SHA1(d1fec216e88f6a07f03d1e458a105548d0376ef3) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "ps_chr-0.chr0",  0x000000, CRC(668b6670) SHA1(35bdac5478cee37b82a8a5367a2a08c70014131d) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_chr-1.chr1",  0x080000, CRC(80c30742) SHA1(e9307b6a4a6a749a97f76355fb3785945e303289) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_chr-2.chr2",  0x100000, CRC(f4e11bf7) SHA1(4506fdc83956d7e58bee1a6661da9fe12f1317a9) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_chr-3.chr3",  0x180000, CRC(97a93dc5) SHA1(9e89271a0bcb201122465d8f3b980b490e1e7350) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_chr-4.chr4",  0x200000, CRC(81d965bf) SHA1(c7a2349c1ed34a5375d8643a3a373adffab554f4) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_chr-5.chr5",  0x280000, CRC(8ca72d35) SHA1(e3af2d3772468bd26b552a3f15c8a9dd562b1361) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_chr-6.chr6",  0x300000, CRC(da3543a9) SHA1(085ac3a7e71db8cc60fc343d3a6b009a81f751ee) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "ps_roz-0.roz0",  0x000000, CRC(68043d7e) SHA1(ee81e0f3fc1cd0aa903ad670aa7832c972613a80) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_roz-1.roz1",  0x080000, CRC(029802b4) SHA1(b9966d13f80faa5befa43cfa9628afd6a4ff783e) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_roz-2.roz2",  0x100000, CRC(bf0b76dc) SHA1(28f01dc16ee8818291f89cfc03b126c6cd7ced72) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_roz-3.roz3",  0x180000, CRC(9c821455) SHA1(259f2cb4a7de69e751644511f66611392e60f59d) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_roz-4.roz4",  0x200000, CRC(63a39b7a) SHA1(2fa8ced5beb7574cb4add0245f97282ce9149691) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_roz-5.roz5",  0x280000, CRC(fc5a99d0) SHA1(e61d45347ec0ee6db27edd6da797e810052803a2) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_roz-6.roz6",  0x300000, CRC(a2a17587) SHA1(44a58735e6efdfa815576ed134a1295db49430fb) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "ps_shape.shape",  0x000000, CRC(58e26fcf) SHA1(9edc3b54964f923ed07ee878520ccf3f188f04ad) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "ps2_dat0.13s",  0x000000, CRC(ee4194b0) SHA1(a0c2a807db70164ed761e5ad04301e5ae1173e7a) )
	NAMCOS2_DATA_LOAD_O_128K( "ps2_dat1.13p",  0x000000, CRC(5b22d714) SHA1(f6cb1fe661f7691269840245f06875845fd6eb33) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "ps_voi-1.voice1",  0x000000, 0x080000, CRC(f67376ed) SHA1(b54257aad34c6ad03d5b040e6a5dda94a48b6780) )
ROM_END

/* PHELIOS (Japan) */
ROM_START( pheliosj )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "ps1_mpr0.mpr0",  0x000000, 0x020000, CRC(bfbe96c6) SHA1(e597fc66b058aca4f55c38fbaec0abcad4ed6547) )
	ROM_LOAD16_BYTE( "ps1_mpr1.mpr1",  0x000001, 0x020000, CRC(f5c0f883) SHA1(9fd8b05a568a024e049bedc3c1c1893cb5bd249f) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "ps1_spr0.spr0",  0x000000, 0x010000, CRC(e9c6987e) SHA1(325532b065999276e4629b10c27d7df083e81c33) ) /* Same DATA as World set, but labeled PS1 */
	ROM_LOAD16_BYTE( "ps1_spr1.spr1",  0x000001, 0x010000, CRC(02b074fb) SHA1(4025c3dd83e26ac87de9f2f73753d1cb1211c1ae) ) /* Same DATA as World set, but labeled PS1 */

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "ps1_snd0.snd0",  0x000000, 0x020000, CRC(da694838) SHA1(e1a20aedd18c49dcd12717e8ce479a5a441c10e6) ) /* Same DATA as World set, but labeled PS1 */

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2_c65c.3f",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	NAMCOS2_SPRROM_LOAD_256K( "ps_obj-0.obj0",  0x000003, CRC(f323db2b) SHA1(fa3c42c618da06af161ad3f8aa1283e9c4bd63c0) )
	NAMCOS2_SPRROM_LOAD_256K( "ps_obj-1.obj1",  0x000002, CRC(faf7c2f5) SHA1(d0d33eddaf5de2b639717db83a85f441d81a5924) )
	NAMCOS2_SPRROM_LOAD_256K( "ps_obj-2.obj2",  0x000001, CRC(828178ba) SHA1(d35ab8020ebaad0b4c0b24fa9edc2886b713a8ed) )
	NAMCOS2_SPRROM_LOAD_256K( "ps_obj-3.obj3",  0x000000, CRC(e84771c8) SHA1(17bede39d8b703005b288f3596c4aaca266fa06b) )
	NAMCOS2_SPRROM_LOAD_256K( "ps_obj-4.obj4",  0x200003, CRC(81ea86c6) SHA1(27400b4e1d53c47bc6b821439c294d879bf58ae4) )
	NAMCOS2_SPRROM_LOAD_256K( "ps_obj-5.obj5",  0x200002, CRC(aaebd51a) SHA1(7acb88cfcaf7e7c41de171ed5952e1d0a13ef302) )
	NAMCOS2_SPRROM_LOAD_256K( "ps_obj-6.obj6",  0x200001, CRC(032ea497) SHA1(89f4deed8fa076683abc1f2e961ceb920ab9848d) )
	NAMCOS2_SPRROM_LOAD_256K( "ps_obj-7.obj7",  0x200000, CRC(f6183b36) SHA1(d1fec216e88f6a07f03d1e458a105548d0376ef3) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "ps_chr-0.chr0",  0x000000, CRC(668b6670) SHA1(35bdac5478cee37b82a8a5367a2a08c70014131d) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_chr-1.chr1",  0x080000, CRC(80c30742) SHA1(e9307b6a4a6a749a97f76355fb3785945e303289) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_chr-2.chr2",  0x100000, CRC(f4e11bf7) SHA1(4506fdc83956d7e58bee1a6661da9fe12f1317a9) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_chr-3.chr3",  0x180000, CRC(97a93dc5) SHA1(9e89271a0bcb201122465d8f3b980b490e1e7350) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_chr-4.chr4",  0x200000, CRC(81d965bf) SHA1(c7a2349c1ed34a5375d8643a3a373adffab554f4) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_chr-5.chr5",  0x280000, CRC(8ca72d35) SHA1(e3af2d3772468bd26b552a3f15c8a9dd562b1361) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_chr-6.chr6",  0x300000, CRC(da3543a9) SHA1(085ac3a7e71db8cc60fc343d3a6b009a81f751ee) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "ps_roz-0.roz0",  0x000000, CRC(68043d7e) SHA1(ee81e0f3fc1cd0aa903ad670aa7832c972613a80) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_roz-1.roz1",  0x080000, CRC(029802b4) SHA1(b9966d13f80faa5befa43cfa9628afd6a4ff783e) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_roz-2.roz2",  0x100000, CRC(bf0b76dc) SHA1(28f01dc16ee8818291f89cfc03b126c6cd7ced72) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_roz-3.roz3",  0x180000, CRC(9c821455) SHA1(259f2cb4a7de69e751644511f66611392e60f59d) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_roz-4.roz4",  0x200000, CRC(63a39b7a) SHA1(2fa8ced5beb7574cb4add0245f97282ce9149691) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_roz-5.roz5",  0x280000, CRC(fc5a99d0) SHA1(e61d45347ec0ee6db27edd6da797e810052803a2) )
	NAMCOS2_GFXROM_LOAD_128K( "ps_roz-6.roz6",  0x300000, CRC(a2a17587) SHA1(44a58735e6efdfa815576ed134a1295db49430fb) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "ps_shape.shape",  0x000000, CRC(58e26fcf) SHA1(9edc3b54964f923ed07ee878520ccf3f188f04ad) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "ps1_dat0.13s",  0x000000, CRC(ee4194b0) SHA1(a0c2a807db70164ed761e5ad04301e5ae1173e7a) ) /* Same DATA as World set, but labeled PS1 */
	NAMCOS2_DATA_LOAD_O_128K( "ps1_dat1.13p",  0x000000, CRC(5b22d714) SHA1(f6cb1fe661f7691269840245f06875845fd6eb33) ) /* Same DATA as World set, but labeled PS1 */

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "ps_voi-1.voice1",  0x000000, 0x080000, CRC(f67376ed) SHA1(b54257aad34c6ad03d5b040e6a5dda94a48b6780) )
ROM_END

/* ROLLING THUNDER 2 */
ROM_START( rthun2 )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "rts2_mpr0.11d",  0x000000, 0x020000, CRC(e09a3549) SHA1(027fe87c98a497c50d12c810b9c7e7216f985dca) )
	ROM_LOAD16_BYTE( "rts2_mpr1.13d",  0x000001, 0x020000, CRC(09573bff) SHA1(b75e036419f95967d5d95c14f1e08aa0c2a05d8a) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "rts2_spr0.11k",  0x000000, 0x010000, CRC(54c22ac5) SHA1(747df2362839e6af15bdbf3298f9ea1c6e25f76a) )
	ROM_LOAD16_BYTE( "rts2_spr1.13k",  0x000001, 0x010000, CRC(060eb393) SHA1(e8f7dd163df16747a74713a6cadd1d52c09b8036) )

	ROM_REGION( 0x040000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "rts1_snd0.7j",  0x000000, 0x020000, CRC(55b7562a) SHA1(47b12206ec4a709769351f3f5b4a1c5ebb98b416) )
	ROM_LOAD( "rts1_snd1.7g",  0x020000, 0x020000, CRC(00445a4f) SHA1(2e136e3c38e4a1b69f80a19e07555f3269b7beb1) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.3f",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites, on sub PCB */
	ROM_LOAD32_BYTE( "rts_obj0.5b",  0x000003, 0x80000, CRC(e5cb82c1) SHA1(2dc1922ecfd9e52af8c4a1edac1df343be64b499) )
	ROM_LOAD32_BYTE( "rts_obj1.4b",  0x000002, 0x80000, CRC(19ebe9fd) SHA1(6d7991a52a707f710c809eb44f1dfa4873369c17) )
	ROM_LOAD32_BYTE( "rts_obj2.5d",  0x000001, 0x80000, CRC(455c4a2f) SHA1(9d7944b41e98f990423d315365106890e2c5ae77) )
	ROM_LOAD32_BYTE( "rts_obj3.4d",  0x000000, 0x80000, CRC(fdcae8a9) SHA1(a1e1057b3263ee9af9e2d861cf8879f51265805e) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "rts_chr0.11n",  0x000000, 0x80000, CRC(6f0e9a68) SHA1(873296778104eff11b828273abf7f6ca461c055a) )
	ROM_LOAD( "rts_chr1.11p",  0x080000, 0x80000, CRC(15e44adc) SHA1(fead0b2d693f9b6267895d8339bb250f5c77fb4d) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles, on sub PCB */
	ROM_LOAD( "rts_roz0.1a",  0x000000, 0x80000, CRC(482d0554) SHA1(95b99d1db5851b83b2af4deda2b61635a0562604) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	ROM_LOAD( "rts_sha0.7n",  0x000000, 0x80000, CRC(cf58fbbe) SHA1(fbe3b2f0c3267b298993d6238d97b119e13e07f6) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "rts1_data0.13s",  0x000000, CRC(0baf44ee) SHA1(5135d634f76893adb26a32976a69e2d47e2385c6) )
	NAMCOS2_DATA_LOAD_O_128K( "rts1_data1.13p",  0x000000, CRC(58a8daac) SHA1(c13ae8fc25b748a006c6db5b4b7ae593738544e8) )
	NAMCOS2_DATA_LOAD_E_128K( "rts1_data2.13r",  0x100000, CRC(8e850a2a) SHA1(e5230e80a23ca6d09c2c53f443ecf70cc74075d7) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "rts_voi1.3m",  0x000000, 0x080000, CRC(e42027cd) SHA1(fa3a81118c7f112289c27023236dec2e9cbc78b5) )
	ROM_LOAD16_BYTE( "rts_voi2.3l",  0x100000, 0x080000, CRC(0c4c2b66) SHA1(7723cbef755439a66d026015596fe1547ccd65b1) )
ROM_END

/* ROLLING THUNDER 2 (Japan) */
ROM_START( rthun2j )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "rts1_mpr0.11d",  0x000000, 0x020000, CRC(2563b9ee) SHA1(c6a4305f88ca5d796f3ba4f36af54fed51c16b75) )
	ROM_LOAD16_BYTE( "rts1_mpr1.13d",  0x000001, 0x020000, CRC(14c4c564) SHA1(a826176fef65c53518fdbc7b14c7a1a65c821c8c) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "rts1_spr0.11k",  0x000000, 0x010000, CRC(f8ef5150) SHA1(92fddf08b97210afe8d47386fe73078ffc00bd90) )
	ROM_LOAD16_BYTE( "rts1_spr1.13k",  0x000001, 0x010000, CRC(52ed3a48) SHA1(21a9f0be29a7b121f1a8ca802af3a5ebf2c49cc0) )

	ROM_REGION( 0x040000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "rts1_snd0.7j",  0x000000, 0x020000, CRC(55b7562a) SHA1(47b12206ec4a709769351f3f5b4a1c5ebb98b416) )
	ROM_LOAD( "rts1_snd1.7g",  0x020000, 0x020000, CRC(00445a4f) SHA1(2e136e3c38e4a1b69f80a19e07555f3269b7beb1) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.3f",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites, on sub PCB */
	ROM_LOAD32_BYTE( "rts_obj0.5b",  0x000003, 0x80000, CRC(e5cb82c1) SHA1(2dc1922ecfd9e52af8c4a1edac1df343be64b499) )
	ROM_LOAD32_BYTE( "rts_obj1.4b",  0x000002, 0x80000, CRC(19ebe9fd) SHA1(6d7991a52a707f710c809eb44f1dfa4873369c17) )
	ROM_LOAD32_BYTE( "rts_obj2.5d",  0x000001, 0x80000, CRC(455c4a2f) SHA1(9d7944b41e98f990423d315365106890e2c5ae77) )
	ROM_LOAD32_BYTE( "rts_obj3.4d",  0x000000, 0x80000, CRC(fdcae8a9) SHA1(a1e1057b3263ee9af9e2d861cf8879f51265805e) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "rts_chr0.11n",  0x000000, 0x80000, CRC(6f0e9a68) SHA1(873296778104eff11b828273abf7f6ca461c055a) )
	ROM_LOAD( "rts_chr1.11p",  0x080000, 0x80000, CRC(15e44adc) SHA1(fead0b2d693f9b6267895d8339bb250f5c77fb4d) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles, on sub PCB */
	ROM_LOAD( "rts_roz0.1a",  0x000000, 0x80000, CRC(482d0554) SHA1(95b99d1db5851b83b2af4deda2b61635a0562604) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	ROM_LOAD( "rts_sha0.7n",  0x000000, 0x80000, CRC(cf58fbbe) SHA1(fbe3b2f0c3267b298993d6238d97b119e13e07f6) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "rts1_data0.13s",  0x000000, CRC(0baf44ee) SHA1(5135d634f76893adb26a32976a69e2d47e2385c6) )
	NAMCOS2_DATA_LOAD_O_128K( "rts1_data1.13p",  0x000000, CRC(58a8daac) SHA1(c13ae8fc25b748a006c6db5b4b7ae593738544e8) )
	NAMCOS2_DATA_LOAD_E_128K( "rts1_data2.13r",  0x100000, CRC(8e850a2a) SHA1(e5230e80a23ca6d09c2c53f443ecf70cc74075d7) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "rts_voi1.3m",  0x000000, 0x080000, CRC(e42027cd) SHA1(fa3a81118c7f112289c27023236dec2e9cbc78b5) )
	ROM_LOAD16_BYTE( "rts_voi2.3l",  0x100000, 0x080000, CRC(0c4c2b66) SHA1(7723cbef755439a66d026015596fe1547ccd65b1) )

	/* stuff below isn't used but loaded because it was on the board .. */
	ROM_REGION( 0x0950, "plds", 0 )
	ROM_LOAD( "pal12l10.8d",  0x0000, 0x0040, CRC(d3ae64a6) SHA1(8e56f447908246e84d5a79df1a1cd3d5c8a040fb) )
	ROM_LOAD( "plhs18p8a.2p", 0x0200, 0x0149, CRC(28c634a4) SHA1(4df8ce2a3216a6b609921e102ec558efc8a735b9) )
	ROM_LOAD( "plhs18p8a.4g", 0x0400, 0x0149, CRC(1932dd5e) SHA1(af6ee16fbd53c68c74dec4c3f6cb8e91b10e9249) )
	ROM_LOAD( "plhs18p8a.5f", 0x0600, 0x0149, CRC(ab2fd9c2) SHA1(a70a00b00533bf1208c4c2633136e2fb01f77711) )
	ROM_LOAD( "pal16l8.9d",   0x0800, 0x0104, NO_DUMP ) // Protected
ROM_END

/* STEEL GUNNER */
ROM_START( sgunner )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "sn2mpr0b.11d",  0x000000, 0x020000, CRC(4bb33394) SHA1(0c93e851d2b7ecfdc63593569baae2ea51a8dbc8) )
	ROM_LOAD16_BYTE( "sn2mpr1b.13d",  0x000001, 0x020000, CRC(d8b47334) SHA1(7aec721a06121b39e15de41b7f90b1653e43b034) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "sn1spr0.11k", 0x000000, 0x010000, CRC(4638b512) SHA1(8a075f9739870ca90d5f1ac01ece6c0efe2f5f1a) )
	ROM_LOAD16_BYTE( "sn1spr1.13k", 0x000001, 0x010000, CRC(e8b1ee73) SHA1(b459cb197807eb7cbc4ca34a1f52d7f46e23e37c) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "sn1_snd0.8j",  0x000000, 0x020000, CRC(bdf36d44) SHA1(46ee7c1b320fec3296e2483d6a68a1f11bcf196d) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "c355spr", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "sn_obj0.8c",   0x000000, 0x80000, CRC(bbae38f7) SHA1(7a40ade13307791f5c5d300882f9a38e18c411d6) )
	ROM_LOAD32_BYTE( "sn_obj1.12c",  0x000001, 0x80000, CRC(4dfacb51) SHA1(1b5ae37f7ee12b791ce80422bd7472aa38c41ddd) )
	ROM_LOAD32_BYTE( "sn_obj2.10c",  0x000002, 0x80000, CRC(313a308f) SHA1(0773a567cf649394cd6fcdd6fba0c4575220a582) )
	ROM_LOAD32_BYTE( "sn_obj3.14c",  0x000003, 0x80000, CRC(d7c340f6) SHA1(4215d9ef38aea2dbf14febedfadd658ce03bbcdf) )
	ROM_LOAD32_BYTE( "sn_obj4.9c",   0x200000, 0x80000, CRC(82fdaa06) SHA1(494cc639bbf4032bb83fc9ad5a1db9dae0d8714b) )
	ROM_LOAD32_BYTE( "sn_obj5.13c",  0x200001, 0x80000, CRC(8700a8a4) SHA1(90909e089405546e9634183969974af4a8cdc9eb) )
	ROM_LOAD32_BYTE( "sn_obj6.11c",  0x200002, 0x80000, CRC(9c6504f7) SHA1(0dc2960ec5b5ce75e06d0f84917286f360e98316) )
	ROM_LOAD32_BYTE( "sn_obj7.15c",  0x200003, 0x80000, CRC(cd1356c0) SHA1(7a21f315442857716eac813adc29cc4f7e28bee8) )

	ROM_REGION( 0x200000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "sn_chr0.11n",  0x000000, 0x80000, CRC(b433c37b) SHA1(514dcffd0f20faae0f5297b68d8946cfbc54e493) )
	ROM_LOAD( "sn_chr1.11p",  0x080000, 0x80000, CRC(b7dd41f9) SHA1(2119bca16cdb55df2416222b66272f681abd0359) )

	ROM_REGION( 0x400000, "s2roz", ROMREGION_ERASEFF ) /* ROZ Tiles */
	/* NO ROZ ROMS PRESENT */

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	ROM_LOAD( "sn_sha0.8n",  0x000000, 0x80000, CRC(01e20999) SHA1(5f925b5b43aa8889e077f7854a89d0731052605d) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "sn1_dat0.13s",  0x000000, CRC(72bfeca8) SHA1(88a2f8959d803611b2f2e219cb8ff085a37d01fe) )
	NAMCOS2_DATA_LOAD_O_128K( "sn1_dat1.13p",  0x000000, CRC(99b3e653) SHA1(d7e29ad4e059f5d5e03386d903428c879d591459) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "sn_voi1.3m",  0x000000, 0x080000, CRC(464e616d) SHA1(7279a2af64bdf76972bcf326611e6bff57a9cd39) )
	ROM_LOAD16_BYTE( "sn_voi2.3l",  0x100000, 0x080000, CRC(8c3251b5) SHA1(fa364c8462f490c636605262c5492a6a9b00e5b1) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "sgunner.nv",  0x000000, 0x2000, CRC(106026f8) SHA1(e4be6701d4eef6c18406593c6dee10644f29a15b) )
ROM_END

/* STEEL GUNNER (Japan) */
ROM_START( sgunnerj )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "sn1mpr0.11d", 0x000000, 0x020000, CRC(f60116d7) SHA1(40357b469157c23f0f9d7feaa6a28617e65c0a30) )
	ROM_LOAD16_BYTE( "sn1mpr1.13d", 0x000001, 0x020000, CRC(23942fc9) SHA1(25adb572f3f010b5e171e3b9d63aef28588c7ca4) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "sn1spr0.11k", 0x000000, 0x010000, CRC(4638b512) SHA1(8a075f9739870ca90d5f1ac01ece6c0efe2f5f1a) )
	ROM_LOAD16_BYTE( "sn1spr1.13k", 0x000001, 0x010000, CRC(e8b1ee73) SHA1(b459cb197807eb7cbc4ca34a1f52d7f46e23e37c) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "sn1_snd0.8j",  0x000000, 0x020000, CRC(bdf36d44) SHA1(46ee7c1b320fec3296e2483d6a68a1f11bcf196d) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "c355spr", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "sn_obj0.8c",   0x000000, 0x80000, CRC(bbae38f7) SHA1(7a40ade13307791f5c5d300882f9a38e18c411d6) )
	ROM_LOAD32_BYTE( "sn_obj1.12c",  0x000001, 0x80000, CRC(4dfacb51) SHA1(1b5ae37f7ee12b791ce80422bd7472aa38c41ddd) )
	ROM_LOAD32_BYTE( "sn_obj2.10c",  0x000002, 0x80000, CRC(313a308f) SHA1(0773a567cf649394cd6fcdd6fba0c4575220a582) )
	ROM_LOAD32_BYTE( "sn_obj3.14c",  0x000003, 0x80000, CRC(d7c340f6) SHA1(4215d9ef38aea2dbf14febedfadd658ce03bbcdf) )
	ROM_LOAD32_BYTE( "sn_obj4.9c",   0x200000, 0x80000, CRC(82fdaa06) SHA1(494cc639bbf4032bb83fc9ad5a1db9dae0d8714b) )
	ROM_LOAD32_BYTE( "sn_obj5.13c",  0x200001, 0x80000, CRC(8700a8a4) SHA1(90909e089405546e9634183969974af4a8cdc9eb) )
	ROM_LOAD32_BYTE( "sn_obj6.11c",  0x200002, 0x80000, CRC(9c6504f7) SHA1(0dc2960ec5b5ce75e06d0f84917286f360e98316) )
	ROM_LOAD32_BYTE( "sn_obj7.15c",  0x200003, 0x80000, CRC(cd1356c0) SHA1(7a21f315442857716eac813adc29cc4f7e28bee8) )

	ROM_REGION( 0x200000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "sn_chr0.11n",  0x000000, 0x80000, CRC(b433c37b) SHA1(514dcffd0f20faae0f5297b68d8946cfbc54e493) )
	ROM_LOAD( "sn_chr1.11p",  0x080000, 0x80000, CRC(b7dd41f9) SHA1(2119bca16cdb55df2416222b66272f681abd0359) )

	ROM_REGION( 0x400000, "s2roz", ROMREGION_ERASEFF ) /* ROZ Tiles */
	/* NO ROZ ROMS PRESENT */

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	ROM_LOAD( "sn_sha0.8n",  0x000000, 0x80000, CRC(01e20999) SHA1(5f925b5b43aa8889e077f7854a89d0731052605d) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "sn1_dat0.13s",  0x000000, CRC(72bfeca8) SHA1(88a2f8959d803611b2f2e219cb8ff085a37d01fe) )
	NAMCOS2_DATA_LOAD_O_128K( "sn1_dat1.13p",  0x000000, CRC(99b3e653) SHA1(d7e29ad4e059f5d5e03386d903428c879d591459) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "sn_voi1.3m",  0x000000, 0x080000, CRC(464e616d) SHA1(7279a2af64bdf76972bcf326611e6bff57a9cd39) )
	ROM_LOAD16_BYTE( "sn_voi2.3l",  0x100000, 0x080000, CRC(8c3251b5) SHA1(fa364c8462f490c636605262c5492a6a9b00e5b1) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "sgunner.nv",  0x000000, 0x2000, CRC(106026f8) SHA1(e4be6701d4eef6c18406593c6dee10644f29a15b) )
ROM_END

/* STEEL GUNNER 2 */
ROM_START( sgunner2 )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "sns2mpr0.bin",  0x000000, 0x020000, CRC(f1a44039) SHA1(c8e73c786a69780d9c66c5d599713976d541e36c) )
	ROM_LOAD16_BYTE( "sns2mpr1.bin",  0x000001, 0x020000, CRC(9184c4db) SHA1(ef3208157d6e5278a99baca0b31f0d43e5ffd89a) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "sns_spr0.bin",  0x000000, 0x010000, CRC(e5e40ed0) SHA1(7bd8cd9030ddb640613420388c26220d120a1244) )
	ROM_LOAD16_BYTE( "sns_spr1.bin",  0x000001, 0x010000, CRC(3a85a5e9) SHA1(8528520c9b0fa4a38fbf0e3935418403c6d055a2) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "sns_snd0.bin",  0x000000, 0x020000, CRC(f079cd32) SHA1(da507390b3e56ba4ad0f6e877d5ac82009b7bd32) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "c355spr", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "sns_obj0.bin",  0x000000, 0x80000, CRC(c762445c) SHA1(108170c9a5c82c23c1ac09f91195137ca05989f4) )
	ROM_LOAD32_BYTE( "sns_obj1.bin",  0x000001, 0x80000, CRC(e9e379d8) SHA1(01b5f5946e746a5265d230273c99a41910fd9e6f) )
	ROM_LOAD32_BYTE( "sns_obj2.bin",  0x000002, 0x80000, CRC(0d076f6c) SHA1(247da0514c3809350ce308334e601f1689a7449f) )
	ROM_LOAD32_BYTE( "sns_obj3.bin",  0x000003, 0x80000, CRC(0fb01e8b) SHA1(50190313da2ab673364e9d94e1b5b03e3c84f57c) )
	ROM_LOAD32_BYTE( "sns_obj4.bin",  0x200000, 0x80000, CRC(0b1be894) SHA1(aaa9fb2f11610458bf685a9124c4889acc63fdc5) )
	ROM_LOAD32_BYTE( "sns_obj5.bin",  0x200001, 0x80000, CRC(416b14e1) SHA1(bb4bc871a9c5ebc28e15a16267dd446f494c922e) )
	ROM_LOAD32_BYTE( "sns_obj6.bin",  0x200002, 0x80000, CRC(c2e94ed2) SHA1(213f57e1a4c8e8ba3c8cbd212431ff7a44d0ffc1) )
	ROM_LOAD32_BYTE( "sns_obj7.bin",  0x200003, 0x80000, CRC(fc1f26af) SHA1(1f0c36587bc9f80a39b49b6fd43d1773b2f49361) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "sns_chr0.bin",  0x000000, 0x80000, CRC(cdc42b61) SHA1(20cdd5a81ce4612f9eecd8f057d2e22e5baeb216) )
	ROM_LOAD( "sns_chr1.bin",  0x080000, 0x80000, CRC(42d4cbb7) SHA1(2ba52987b59f2ae327b341c696fb4a4bbdccb093) )
	ROM_LOAD( "sns_chr2.bin",  0x100000, 0x80000, CRC(7dbaa14e) SHA1(6df4fbe85560d2c1624ac2d6c80d6f7827954775) )
	ROM_LOAD( "sns_chr3.bin",  0x180000, 0x80000, CRC(b562ff72) SHA1(6b74bca0555e51d7b15fc5d8fe865900646acbc6) )

	ROM_REGION( 0x400000, "s2roz", ROMREGION_ERASEFF ) /* ROZ Tiles */
	/* NO ROZ ROMS PRESENT IN ZIP */

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	ROM_LOAD( "sns_sha0.bin",  0x000000, 0x80000, CRC(0374fd67) SHA1(2f09536ef4f8e1aa719108e73642feb376d6efff) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "sns_dat0.13s",  0x000000, CRC(48295d93) SHA1(2b314128d5ed7d30895967dbd2ecd8f1dfdc61ca) )
	NAMCOS2_DATA_LOAD_O_128K( "sns_dat1.13p",  0x000000, CRC(b44cc656) SHA1(7126ef28bf174c6c560469fa6dc4cfb439811850) )
	NAMCOS2_DATA_LOAD_E_128K( "sns_dat2.13r",  0x100000, CRC(ca2ae645) SHA1(8addc8ed8244d0ff4c03909e865e3f15934963f1) )
	NAMCOS2_DATA_LOAD_O_128K( "sns_dat3.13n",  0x100000, CRC(203bb018) SHA1(36e20ab81ed69a941e13518ff2ae52acd6b22a78) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "sns_voi1.bin",  0x000000, 0x080000, CRC(219c97f7) SHA1(d4b1d81e3d0e2585bc2fa305c0d80beef15b2a9f) )
	ROM_LOAD16_BYTE( "sns_voi2.bin",  0x100000, 0x080000, CRC(562ec86b) SHA1(c9874c7e1f38c5b38d21f45a82028651cf9089a5) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "sgunner2.nv",  0x000000, 0x2000, CRC(57a521c6) SHA1(d60b4f6f099b7f9fb1e575c5f9a74397986c6dac) )
ROM_END

/* STEEL GUNNER 2 (Japan) */
ROM_START( sgunner2j )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "sns1mpr0a.bin",  0x000000, 0x020000, CRC(e7216ad7) SHA1(ebc8e31ee6909d89bc4f40889e80f3c8b46dcd85) )
	ROM_LOAD16_BYTE( "sns1mpr1a.bin",  0x000001, 0x020000, CRC(6caef2ee) SHA1(8003517ff8b74480147786af3abdc7a78ba9d379) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "sns_spr0.bin",  0x000000, 0x010000, CRC(e5e40ed0) SHA1(7bd8cd9030ddb640613420388c26220d120a1244) )
	ROM_LOAD16_BYTE( "sns_spr1.bin",  0x000001, 0x010000, CRC(3a85a5e9) SHA1(8528520c9b0fa4a38fbf0e3935418403c6d055a2) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "sns_snd0.bin",  0x000000, 0x020000, CRC(f079cd32) SHA1(da507390b3e56ba4ad0f6e877d5ac82009b7bd32) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "c355spr", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "sns_obj0.bin",  0x000000, 0x80000, CRC(c762445c) SHA1(108170c9a5c82c23c1ac09f91195137ca05989f4) )
	ROM_LOAD32_BYTE( "sns_obj1.bin",  0x000001, 0x80000, CRC(e9e379d8) SHA1(01b5f5946e746a5265d230273c99a41910fd9e6f) )
	ROM_LOAD32_BYTE( "sns_obj2.bin",  0x000002, 0x80000, CRC(0d076f6c) SHA1(247da0514c3809350ce308334e601f1689a7449f) )
	ROM_LOAD32_BYTE( "sns_obj3.bin",  0x000003, 0x80000, CRC(0fb01e8b) SHA1(50190313da2ab673364e9d94e1b5b03e3c84f57c) )
	ROM_LOAD32_BYTE( "sns_obj4.bin",  0x200000, 0x80000, CRC(0b1be894) SHA1(aaa9fb2f11610458bf685a9124c4889acc63fdc5) )
	ROM_LOAD32_BYTE( "sns_obj5.bin",  0x200001, 0x80000, CRC(416b14e1) SHA1(bb4bc871a9c5ebc28e15a16267dd446f494c922e) )
	ROM_LOAD32_BYTE( "sns_obj6.bin",  0x200002, 0x80000, CRC(c2e94ed2) SHA1(213f57e1a4c8e8ba3c8cbd212431ff7a44d0ffc1) )
	ROM_LOAD32_BYTE( "sns_obj7.bin",  0x200003, 0x80000, CRC(fc1f26af) SHA1(1f0c36587bc9f80a39b49b6fd43d1773b2f49361) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "sns_chr0.bin",  0x000000, 0x80000, CRC(cdc42b61) SHA1(20cdd5a81ce4612f9eecd8f057d2e22e5baeb216) )
	ROM_LOAD( "sns_chr1.bin",  0x080000, 0x80000, CRC(42d4cbb7) SHA1(2ba52987b59f2ae327b341c696fb4a4bbdccb093) )
	ROM_LOAD( "sns_chr2.bin",  0x100000, 0x80000, CRC(7dbaa14e) SHA1(6df4fbe85560d2c1624ac2d6c80d6f7827954775) )
	ROM_LOAD( "sns_chr3.bin",  0x180000, 0x80000, CRC(b562ff72) SHA1(6b74bca0555e51d7b15fc5d8fe865900646acbc6) )

	ROM_REGION( 0x400000, "s2roz", ROMREGION_ERASEFF ) /* ROZ Tiles */
	/* NO ROZ ROMS PRESENT IN ZIP */

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	ROM_LOAD( "sns_sha0.bin",  0x000000, 0x80000, CRC(0374fd67) SHA1(2f09536ef4f8e1aa719108e73642feb376d6efff) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "sns_dat0.13s",  0x000000, CRC(48295d93) SHA1(2b314128d5ed7d30895967dbd2ecd8f1dfdc61ca) )
	NAMCOS2_DATA_LOAD_O_128K( "sns_dat1.13p",  0x000000, CRC(b44cc656) SHA1(7126ef28bf174c6c560469fa6dc4cfb439811850) )
	NAMCOS2_DATA_LOAD_E_128K( "sns_dat2.13r",  0x100000, CRC(ca2ae645) SHA1(8addc8ed8244d0ff4c03909e865e3f15934963f1) )
	NAMCOS2_DATA_LOAD_O_128K( "sns_dat3.13n",  0x100000, CRC(203bb018) SHA1(36e20ab81ed69a941e13518ff2ae52acd6b22a78) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "sns_voi1.bin",  0x000000, 0x080000, CRC(219c97f7) SHA1(d4b1d81e3d0e2585bc2fa305c0d80beef15b2a9f) )
	ROM_LOAD16_BYTE( "sns_voi2.bin",  0x100000, 0x080000, CRC(562ec86b) SHA1(c9874c7e1f38c5b38d21f45a82028651cf9089a5) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "sgunner2j.nv",  0x000000, 0x2000, CRC(014bccf9) SHA1(b6437fadf3e71df7a71fde9ec7ffc95fe6c057b3) )
ROM_END

/* SUPER WORLD STADIUM */
ROM_START( sws )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "ss1_mpr0.11d",  0x000000, 0x020000, CRC(d12bd020) SHA1(bae6250746ab1dbfffbf06a6099fa6aaa1680e98) )
	ROM_LOAD16_BYTE( "ss1_mpr1.13d",  0x000001, 0x020000, CRC(e9ae14ce) SHA1(76a82219c0f663637f5cfcd8607186a972e68218) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "sst1spr0.bin",  0x000000, 0x020000, CRC(9777ee2f) SHA1(95b769402cea53c7f557aab97e0bea61edfd4a35) )
	ROM_LOAD16_BYTE( "sst1spr1.bin",  0x000001, 0x020000, CRC(27a35c69) SHA1(cb23a357f6f608cb9afdeca9ea11d123b69272ef) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "sst1snd0.bin",  0x000000, 0x020000, CRC(8fc45114) SHA1(d0dc5c8af8733f1ac7cbe46f0f6cfbe188124d42) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "ss1_obj0.5b",  0x000003, 0x80000, CRC(9bd6add1) SHA1(34595987670d7f64ba18a840e98667b96ae5e4bf) )
	ROM_LOAD32_BYTE( "ss1_obj1.4b",  0x000002, 0x80000, CRC(a9db3d02) SHA1(63ff1ebc3fe27cd58fda0133bade9ca177ad2d89) )
	ROM_LOAD32_BYTE( "ss1_obj2.5d",  0x000001, 0x80000, CRC(b4a73ced) SHA1(9d8476fb3db7fd2fce124dab09094f0ce0057116) )
	ROM_LOAD32_BYTE( "ss1_obj3.4d",  0x000000, 0x80000, CRC(0a832b36) SHA1(56879a208e106105b8c0add3c7f7a69ce1ecbd9a) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "ss1_chr0.11n",  0x000000, 0x80000, CRC(ab0141de) SHA1(b63f5a1ca23a56f8da71741f64d410a323ff277f) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	ROM_LOAD( "ss_roz0.bin",  0x000000, 0x80000, CRC(40ce9a58) SHA1(240433e4fe31d333bcc8890c0a8e9ac84086fb07) )
	ROM_LOAD( "ss_roz1.bin",  0x080000, 0x80000, CRC(c98902ff) SHA1(fef954e90552a09fa4c9a8e4acc18c1e0e762900) )
	ROM_LOAD( "ss1_roz2.1c", 0x100000, 0x80000, CRC(b603e1a1) SHA1(bfe7ec0425c390390b3e3d17a9a57bca69346f98) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	ROM_LOAD( "ss1_sha0.7n",  0x000000, 0x80000,  CRC(fea6952c) SHA1(3bf27ee1e7e4c5ee0d53f28d49ef063b3f8064ba) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_256K( "ss1_dat0.13s",  0x000000, CRC(6a360f91) SHA1(22597c6bf7c597cf554a27182b4748de43a87b0a)  )
	NAMCOS2_DATA_LOAD_O_256K( "ss1_dat1.13p",  0x000000, CRC(ab1e487d) SHA1(b40ea6c28dd9adae4939f69fcbf53414ae4703c6) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "ss_voi1.bin",  0x000000, 0x080000, CRC(503e51b7) SHA1(2e159fcc9bb0bef9a3476ae233bc8d61fabbb4bd) )
ROM_END

/* SUPER WORLD STADIUM 92 */
ROM_START( sws92 )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "sss1mpr0.bin",  0x000000, 0x020000, CRC(dbea0210) SHA1(573c170cb2b32ed9c0c711d5b7484850d860bf88) )
	ROM_LOAD16_BYTE( "sss1mpr1.bin",  0x000001, 0x020000, CRC(b5e6469a) SHA1(d07010fd9a1318a08cdf14fe99e0855ff42639cb) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "sst1spr0.bin",  0x000000, 0x020000, CRC(9777ee2f) SHA1(95b769402cea53c7f557aab97e0bea61edfd4a35) )
	ROM_LOAD16_BYTE( "sst1spr1.bin",  0x000001, 0x020000, CRC(27a35c69) SHA1(cb23a357f6f608cb9afdeca9ea11d123b69272ef) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "sst1snd0.bin",  0x000000, 0x020000, CRC(8fc45114) SHA1(d0dc5c8af8733f1ac7cbe46f0f6cfbe188124d42) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "sss_obj0.bin",  0x000003, 0x80000, CRC(375e8f1f) SHA1(b737bcceb498a66593d06ef102958bea90032106) )
	ROM_LOAD32_BYTE( "sss_obj1.bin",  0x000002, 0x80000, CRC(675c1014) SHA1(b960a1f72cddc5e369ab7063678e5548b508e376) )
	ROM_LOAD32_BYTE( "sss_obj2.bin",  0x000001, 0x80000, CRC(bdc55f1c) SHA1(fa4f454406eb7e21daed16fedba2adcdb0fb6247) )
	ROM_LOAD32_BYTE( "sss_obj3.bin",  0x000000, 0x80000, CRC(e32ac432) SHA1(a8572adb38e72cb72b5d4ba8968a300c675465ba) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "sss_chr0.bin",  0x000000, 0x80000, CRC(1d2876f2) SHA1(00b3113ea3e4f316f5bf2d3164cfe98d326f66bd) )
	ROM_LOAD( "sss_chr6.bin",  0x300000, 0x80000, CRC(354f0ed2) SHA1(ea6370a460ef4319b6425dc0c8de687a9edbccdb) )
	ROM_LOAD( "sss_chr7.bin",  0x380000, 0x80000, CRC(4032f4c1) SHA1(4d6e3391e03864abfa16002bb883b636b0f3e574) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	ROM_LOAD( "ss_roz0.bin",  0x000000, 0x80000, CRC(40ce9a58) SHA1(240433e4fe31d333bcc8890c0a8e9ac84086fb07) )
	ROM_LOAD( "ss_roz1.bin",  0x080000, 0x80000, CRC(c98902ff) SHA1(fef954e90552a09fa4c9a8e4acc18c1e0e762900) )
	ROM_LOAD( "sss_roz2.bin", 0x100000, 0x80000, CRC(c9855c10) SHA1(40a3fe1204f20b8295e790bc7cb6706fb46d4d51) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	ROM_LOAD( "sss_sha0.bin",  0x000000, 0x80000, CRC(b71a731a) SHA1(29eab0ad5618f54e6c790d20f7dc895afae6c709) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_256K( "sss1dat0.13s",  0x000000, CRC(db3e6aec) SHA1(928960e3dc9c8225e695d12e9b18fbb7f151c151) )
	NAMCOS2_DATA_LOAD_O_256K( "sss1dat1.13p",  0x000000, CRC(463b5ba8) SHA1(029dce2e7ee50181392b6ef409bbd192105fb065) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "ss_voi1.bin",  0x000000, 0x080000, CRC(503e51b7) SHA1(2e159fcc9bb0bef9a3476ae233bc8d61fabbb4bd) )
ROM_END

/* SUPER WORLD STADIUM 92 */
ROM_START( sws92g )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "ssg1mpr0.bin",  0x000000, 0x020000, CRC(5596c535) SHA1(e2fd8e158662058636b804d6ce7caba8658675c2) )
	ROM_LOAD16_BYTE( "ssg1mpr1.bin",  0x000001, 0x020000, CRC(3289ef0c) SHA1(028eb28d824332bc59a552c2a9e5013073bb0949) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "sst1spr0.bin",  0x000000, 0x020000, CRC(9777ee2f) SHA1(95b769402cea53c7f557aab97e0bea61edfd4a35) )
	ROM_LOAD16_BYTE( "sst1spr1.bin",  0x000001, 0x020000, CRC(27a35c69) SHA1(cb23a357f6f608cb9afdeca9ea11d123b69272ef) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "sst1snd0.bin",  0x000000, 0x020000, CRC(8fc45114) SHA1(d0dc5c8af8733f1ac7cbe46f0f6cfbe188124d42) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "sss_obj0.bin",  0x000003, 0x80000, CRC(375e8f1f) SHA1(b737bcceb498a66593d06ef102958bea90032106) )
	ROM_LOAD32_BYTE( "sss_obj1.bin",  0x000002, 0x80000, CRC(675c1014) SHA1(b960a1f72cddc5e369ab7063678e5548b508e376) )
	ROM_LOAD32_BYTE( "sss_obj2.bin",  0x000001, 0x80000, CRC(bdc55f1c) SHA1(fa4f454406eb7e21daed16fedba2adcdb0fb6247) )
	ROM_LOAD32_BYTE( "sss_obj3.bin",  0x000000, 0x80000, CRC(e32ac432) SHA1(a8572adb38e72cb72b5d4ba8968a300c675465ba) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "sss_chr0.bin",  0x000000, 0x80000, CRC(1d2876f2) SHA1(00b3113ea3e4f316f5bf2d3164cfe98d326f66bd) )
	ROM_LOAD( "sss_chr6.bin",  0x300000, 0x80000, CRC(354f0ed2) SHA1(ea6370a460ef4319b6425dc0c8de687a9edbccdb) )
	ROM_LOAD( "sss_chr7.bin",  0x380000, 0x80000, CRC(4032f4c1) SHA1(4d6e3391e03864abfa16002bb883b636b0f3e574) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	ROM_LOAD( "ss_roz0.bin",  0x000000, 0x80000, CRC(40ce9a58) SHA1(240433e4fe31d333bcc8890c0a8e9ac84086fb07) )
	ROM_LOAD( "ss_roz1.bin",  0x080000, 0x80000, CRC(c98902ff) SHA1(fef954e90552a09fa4c9a8e4acc18c1e0e762900) )
	ROM_LOAD( "sss_roz2.bin", 0x100000, 0x80000, CRC(c9855c10) SHA1(40a3fe1204f20b8295e790bc7cb6706fb46d4d51) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	ROM_LOAD( "sss_sha0.bin",  0x000000, 0x80000, CRC(b71a731a) SHA1(29eab0ad5618f54e6c790d20f7dc895afae6c709) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_256K( "sss1dat0.13s",  0x000000, CRC(db3e6aec) SHA1(928960e3dc9c8225e695d12e9b18fbb7f151c151) )
	NAMCOS2_DATA_LOAD_O_256K( "sss1dat1.13p",  0x000000, CRC(463b5ba8) SHA1(029dce2e7ee50181392b6ef409bbd192105fb065) )
	NAMCOS2_DATA_LOAD_E_256K( "ssg1dat2.13r",  0x080000, CRC(754128aa) SHA1(459ffb08bcd905644d6019e5b25870dcb1e2b418) )
	NAMCOS2_DATA_LOAD_O_256K( "ssg1dat3.13n",  0x080000, CRC(cb3fed01) SHA1(68887d46fd07cd8fb67d58f37e74a6aefdae4328) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "ss_voi1.bin",  0x000000, 0x080000, CRC(503e51b7) SHA1(2e159fcc9bb0bef9a3476ae233bc8d61fabbb4bd) )
ROM_END

/* SUPER WORLD STADIUM 93 */
ROM_START( sws93 )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "sst1mpr0.bin",  0x000000, 0x020000, CRC(bd2679bc) SHA1(6916d2bdeb99aea5779697c825acf9a5ade9cb08) )
	ROM_LOAD16_BYTE( "sst1mpr1.bin",  0x000001, 0x020000, CRC(9132e220) SHA1(dbeb9171c29c2fa33104adee8ff71b2635ae435b) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "sst1spr0.bin",  0x000000, 0x020000, CRC(9777ee2f) SHA1(95b769402cea53c7f557aab97e0bea61edfd4a35) )
	ROM_LOAD16_BYTE( "sst1spr1.bin",  0x000001, 0x020000, CRC(27a35c69) SHA1(cb23a357f6f608cb9afdeca9ea11d123b69272ef) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "sst1snd0.bin",  0x000000, 0x020000, CRC(8fc45114) SHA1(d0dc5c8af8733f1ac7cbe46f0f6cfbe188124d42) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "sst_obj0.bin",  0x000003, 0x80000, CRC(4089dfd7) SHA1(d37fb08d03a4d3f87b10a8e73bbb1817543396ff) )
	ROM_LOAD32_BYTE( "sst_obj1.bin",  0x000002, 0x80000, CRC(cfbc25c7) SHA1(7b6459bda373d1025db6bc8df671d73d6c0963b9) )
	ROM_LOAD32_BYTE( "sst_obj2.bin",  0x000001, 0x80000, CRC(61ed3558) SHA1(af1785e909f61db4ad1b250a7064ad07d886edd5) )
	ROM_LOAD32_BYTE( "sst_obj3.bin",  0x000000, 0x80000, CRC(0e3bc05d) SHA1(9b7dd60074a17d75633c9e804d62e9a7a94e0698) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "sst_chr0.bin",  0x000000, 0x80000, CRC(3397850d) SHA1(2c06810bc3769b7d7d8d02a8f9aa27b0cbb06b6b) )
	ROM_LOAD( "sss_chr6.bin",  0x300000, 0x80000, CRC(354f0ed2) SHA1(ea6370a460ef4319b6425dc0c8de687a9edbccdb) )
	ROM_LOAD( "sst_chr7.bin",  0x380000, 0x80000, CRC(e0abb763) SHA1(6c60a4986562bad0da6b7009a5f5165a81f801d7) )

	ROM_REGION( 0x400000, "s2roz", 0 ) /* ROZ Tiles */
	ROM_LOAD( "ss_roz0.bin",  0x000000, 0x80000, CRC(40ce9a58) SHA1(240433e4fe31d333bcc8890c0a8e9ac84086fb07) )
	ROM_LOAD( "ss_roz1.bin",  0x080000, 0x80000, CRC(c98902ff) SHA1(fef954e90552a09fa4c9a8e4acc18c1e0e762900) )
	ROM_LOAD( "sss_roz2.bin", 0x100000, 0x80000, CRC(c9855c10) SHA1(40a3fe1204f20b8295e790bc7cb6706fb46d4d51) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	ROM_LOAD( "sst_sha0.bin", 0x000000, 0x80000, CRC(4f64d4bd) SHA1(0eb5311448cfd91b1e139b64b2f35b5179237e58) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_512K( "sst1dat0.13s",  0x000000, CRC(b99c9656) SHA1(ac9e6bf46204dad70caf0d75614a20af0269a07f) )
	NAMCOS2_DATA_LOAD_O_512K( "sst1dat1.13p",  0x000000, CRC(60cf6281) SHA1(c02a5bf8f4f94cbe8b0448c9457af53cd1c043d0) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "ss_voi1.bin",  0x000000, 0x080000, CRC(503e51b7) SHA1(2e159fcc9bb0bef9a3476ae233bc8d61fabbb4bd) )
ROM_END

/* SUZUKA 8 HOURS (World?) */
ROM_START( suzuka8h )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "eh2-mp0c.bin",  0x000000, 0x020000, CRC(9b9271ac) SHA1(92bb911a4f4705e5ee6962010b16e74aeeb78943) )
	ROM_LOAD16_BYTE( "eh2-mp1c.bin",  0x000001, 0x020000, CRC(24fdd4bc) SHA1(fd4eec21e24afe26281d41de7f9db6767ccf26a0) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "eh1-sp0.bin",  0x000000, 0x020000, CRC(4a8c4709) SHA1(c08b618296088d6f4b4906817a0eea8d79448e98) )
	ROM_LOAD16_BYTE( "eh1-sp1.bin",  0x000001, 0x020000, CRC(2256b14e) SHA1(5bf45cf90d9b6ca349cdb26c68a80008880e107d) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "eh1-snd0.bin",  0x000000, 0x020000, CRC(36748d3c) SHA1(30bf3310b513c7fcee28a908563459e154c5292a) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "c355spr", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "eh1-obj0.bin",  0x000000, 0x80000, CRC(864b6816) SHA1(72d831b631afb2848578bd49cd7d3e12a78644b4) )
	ROM_LOAD32_BYTE( "eh1-obj1.bin",  0x000001, 0x80000, CRC(d4921c35) SHA1(fe1b3997c3298e58919fa5602b94bd121439d5bc) )
	ROM_LOAD32_BYTE( "eh1-obj2.bin",  0x000002, 0x80000, CRC(966d3f19) SHA1(997669cce56350cd7ed02eec0a88696469435490) )
	ROM_LOAD32_BYTE( "eh1-obj3.bin",  0x000003, 0x80000, CRC(7d253cbe) SHA1(8ff32b7807e233dd6ea6454e744bf6efacd27181) )
	ROM_LOAD32_BYTE( "eh1-obj4.bin",  0x200000, 0x80000, CRC(cde13867) SHA1(071d5ea4b11c78d671e30f43d8d09e9b8314a4db) )
	ROM_LOAD32_BYTE( "eh1-obj5.bin",  0x200001, 0x80000, CRC(9f210546) SHA1(a86cb90788a0cf381b73771a9a95b5d876b43f83) )
	ROM_LOAD32_BYTE( "eh1-obj6.bin",  0x200002, 0x80000, CRC(6019fc8c) SHA1(f3eb74fe0df2efbfeeaf4f8b43c93f104009da0b) )
	ROM_LOAD32_BYTE( "eh1-obj7.bin",  0x200003, 0x80000, CRC(0bd966b8) SHA1(70d6b12139b2563a646da7d11c554f2f6ffb3559) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "eh2-chr0.bin",  0x000000, 0x80000, CRC(b2450fd2) SHA1(4aafb2c96b15e01364eb61ad0a71929c730e30a4) )
	ROM_LOAD( "eh2-chr1.bin",  0x080000, 0x80000, CRC(57204651) SHA1(72cd7bd761a2b0fd12b3c3214098439d85438db8) )
	ROM_LOAD( "eh1-chr2.bin",  0x100000, 0x80000, CRC(8150f644) SHA1(bf41d43938dbae6fd3162caebe8ffd4ec4bbd169) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	ROM_LOAD( "eh2-sha0.bin",  0x000000, 0x80000, CRC(7f24619c) SHA1(0d19ab621cb42c5315be9a6b56f6711fc46ffff7) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_256K( "eh1-d0.13s",  0x000000, CRC(b43e5dfa) SHA1(7e24ee46169532cdb5c504239f7961f8d17e86e4) )
	NAMCOS2_DATA_LOAD_O_256K( "eh1-d1.13p",  0x000000, CRC(9825d5bf) SHA1(720f0e90c69a2e0c48889d510a15102768226a67) )
	NAMCOS2_DATA_LOAD_O_256K( "eh1-d3.13n",  0x100000, CRC(f46d301f) SHA1(70797fd584735844539553efcad53e11239ec10e) )

	ROM_REGION( 0x100, "c45_road:clut", 0 ) /* prom for road colors */
	ROM_LOAD( "ehs1_landdt.10w", 0, 0x100, CRC(cde7e8a6) SHA1(860273daf2e649418746adf50a67ae33f9f3740c) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "eh1-voi1.bin",  0x000000, 0x080000, CRC(71e534d3) SHA1(2981de315e660b878673b5d3816886e96f0556d6) )
	ROM_LOAD16_BYTE( "eh1-voi2.bin",  0x100000, 0x080000, CRC(3e20df8e) SHA1(7f1d57a5a73c45c69f0afd137a630bf07c4e7a9c) )
ROM_END /* suzuka8h */

/* SUZUKA 8 HOURS (Japan) */
ROM_START( suzuka8hj )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "eh1-mp0b.bin",  0x000000, 0x020000, CRC(2850f469) SHA1(65142451db8ddbd1500f4bb5d5a6b2ede5871dd5) )
	ROM_LOAD16_BYTE( "eh1-mp1b.bin",  0x000001, 0x020000, CRC(be83eb2c) SHA1(f54ca4469050f734e4c2d7d681e3e51cc067f2c2) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "eh1-sp0.bin",  0x000000, 0x020000, CRC(4a8c4709) SHA1(c08b618296088d6f4b4906817a0eea8d79448e98) )
	ROM_LOAD16_BYTE( "eh1-sp1.bin",  0x000001, 0x020000, CRC(2256b14e) SHA1(5bf45cf90d9b6ca349cdb26c68a80008880e107d) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "eh1-snd0.bin",  0x000000, 0x020000, CRC(36748d3c) SHA1(30bf3310b513c7fcee28a908563459e154c5292a) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "c355spr", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "eh1-obj0.bin",  0x000000, 0x80000, CRC(864b6816) SHA1(72d831b631afb2848578bd49cd7d3e12a78644b4) )
	ROM_LOAD32_BYTE( "eh1-obj1.bin",  0x000001, 0x80000, CRC(d4921c35) SHA1(fe1b3997c3298e58919fa5602b94bd121439d5bc) )
	ROM_LOAD32_BYTE( "eh1-obj2.bin",  0x000002, 0x80000, CRC(966d3f19) SHA1(997669cce56350cd7ed02eec0a88696469435490) )
	ROM_LOAD32_BYTE( "eh1-obj3.bin",  0x000003, 0x80000, CRC(7d253cbe) SHA1(8ff32b7807e233dd6ea6454e744bf6efacd27181) )
	ROM_LOAD32_BYTE( "eh1-obj4.bin",  0x200000, 0x80000, CRC(cde13867) SHA1(071d5ea4b11c78d671e30f43d8d09e9b8314a4db) )
	ROM_LOAD32_BYTE( "eh1-obj5.bin",  0x200001, 0x80000, CRC(9f210546) SHA1(a86cb90788a0cf381b73771a9a95b5d876b43f83) )
	ROM_LOAD32_BYTE( "eh1-obj6.bin",  0x200002, 0x80000, CRC(6019fc8c) SHA1(f3eb74fe0df2efbfeeaf4f8b43c93f104009da0b) )
	ROM_LOAD32_BYTE( "eh1-obj7.bin",  0x200003, 0x80000, CRC(0bd966b8) SHA1(70d6b12139b2563a646da7d11c554f2f6ffb3559) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "eh1-chr0.bin",  0x000000, 0x80000, CRC(bc90ebef) SHA1(592ca134cc018e87214f72a97979cbf9425cfffd) )
	ROM_LOAD( "eh1-chr1.bin",  0x080000, 0x80000, CRC(61395018) SHA1(4ffb8323b5671f99bb420881f84be58de7d79d07) )
	ROM_LOAD( "eh1-chr2.bin",  0x100000, 0x80000, CRC(8150f644) SHA1(bf41d43938dbae6fd3162caebe8ffd4ec4bbd169) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	ROM_LOAD( "eh1-sha0.bin",  0x000000, 0x80000, CRC(39585cf9) SHA1(8cc18b5745ab2cf50d4df0a17fc1a57771db28ab) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_256K( "eh1-d0.13s",  0x000000, CRC(b43e5dfa) SHA1(7e24ee46169532cdb5c504239f7961f8d17e86e4) )
	NAMCOS2_DATA_LOAD_O_256K( "eh1-d1.13p",  0x000000, CRC(9825d5bf) SHA1(720f0e90c69a2e0c48889d510a15102768226a67) )
	NAMCOS2_DATA_LOAD_O_256K( "eh1-d3.13n",  0x100000, CRC(f46d301f) SHA1(70797fd584735844539553efcad53e11239ec10e) )

	ROM_REGION( 0x100, "c45_road:clut", 0 ) /* prom for road colors */
	ROM_LOAD( "ehs1_landdt.10w", 0, 0x100, CRC(cde7e8a6) SHA1(860273daf2e649418746adf50a67ae33f9f3740c) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "eh1-voi1.bin",  0x000000, 0x080000, CRC(71e534d3) SHA1(2981de315e660b878673b5d3816886e96f0556d6) )
	ROM_LOAD16_BYTE( "eh1-voi2.bin",  0x100000, 0x080000, CRC(3e20df8e) SHA1(7f1d57a5a73c45c69f0afd137a630bf07c4e7a9c) )
ROM_END /* suzuk8hj */

/* SUZUKA 8 HOURS 2 */
ROM_START( suzuk8h2 )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "ehs2_mp0b.11d",  0x000000, 0x020000, CRC(ade97f90) SHA1(8674da5b72d9c9dc09004cb5458b4e87a2b33459) )
	ROM_LOAD16_BYTE( "ehs2_mp1b.13d",  0x000001, 0x020000, CRC(19744a66) SHA1(90a9699d988eca89633def190836bc3862b8e49e) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "ehs1-sp0.11k",  0x000000, 0x020000, CRC(9ca967bc) SHA1(a4d19ae72d0d8e96c1fa5f7e9d8e002275e7be32) )
	ROM_LOAD16_BYTE( "ehs1-sp1.13k",  0x000001, 0x020000, CRC(f25bfaaa) SHA1(829936a9d19c4a8818684ce263e9ae5cbc7d5ca4) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "ehs1-snd0.7j",  0x000000, 0x020000, CRC(fc95993b) SHA1(e8ecd6dc37634bdd030b8cea0b69818fba5a9388) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "c355spr", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "ehs1-obj0.3p",  0x000000, 0x80000, CRC(a0acf307) SHA1(6d79d2dd00da4f8f0462245f42a9d88b6ad632b1) )
	ROM_LOAD32_BYTE( "ehs1-obj1.3w",  0x000001, 0x80000, CRC(ca780b44) SHA1(d16263851c165f5958b0a2ad1ba199058a8d56d5) )
	ROM_LOAD32_BYTE( "ehs1-obj2.3t",  0x000002, 0x80000, CRC(83b45afe) SHA1(10a4b88b36f8d037cbb611cb273613b1d45e8eb5) )
	ROM_LOAD32_BYTE( "ehs1-obj3.3y",  0x000003, 0x80000, CRC(360c03a8) SHA1(969b1a96833ab2db3d610a2b3793fc1e038b24d8) )
	ROM_LOAD32_BYTE( "ehs1-obj4.3s",  0x200000, 0x80000, CRC(4e503ca5) SHA1(248af1cba2cad2b6e3c53c7c7673165789c5f4d5) )
	ROM_LOAD32_BYTE( "ehs1-obj5.3x",  0x200001, 0x80000, CRC(5405f2d9) SHA1(c2b592abf72f4de22a3863909be579820c8fe5b1) )
	ROM_LOAD32_BYTE( "ehs1-obj6.3j",  0x200002, 0x80000, CRC(f5fc8b23) SHA1(d7cd4596cd6991db72c371d835051cc8001f30b3) )
	ROM_LOAD32_BYTE( "ehs1-obj7.3z",  0x200003, 0x80000, CRC(da6bf51b) SHA1(b9b49b983f76989067c4763fd88bfa11bbf5d064) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "ehs1-chr0.11n", 0x000000, 0x80000, CRC(844efe0d) SHA1(032a2d268bbab60706d911ab42206b5329e1abba) )
	ROM_LOAD( "ehs1-chr1.11p", 0x080000, 0x80000, CRC(e8480a6d) SHA1(247a7ab2f5270a87c6a05b12f01b884fb05dabd2) )
	ROM_LOAD( "ehs1-chr2.11r", 0x100000, 0x80000, CRC(ace2d871) SHA1(1a10f571268d83c70fe00fad2f1ce2b48fb764b1) )
	ROM_LOAD( "ehs1-chr3.11s", 0x180000, 0x80000, CRC(c1680818) SHA1(c8c9fd02b1155924fdc1e930fe176225a3b840a1) )
	ROM_LOAD( "ehs1-chr4.9n",  0x200000, 0x80000, CRC(82e8c1d5) SHA1(4c6d234a875f108623638960258a0cd530cab6da) )
	ROM_LOAD( "ehs1-chr5.9p",  0x280000, 0x80000, CRC(9448537c) SHA1(99b9215907ed8658f19bec1153dc1d48bbfd7753) )
	ROM_LOAD( "ehs1-chr6.9r",  0x300000, 0x80000, CRC(2d1c01ad) SHA1(1ed79e22b964fe648d22e43b78c1c3b5a7d5f8c8) )
	ROM_LOAD( "ehs1-chr7.9s",  0x380000, 0x80000, CRC(18dd8676) SHA1(59b9a07f4a980fd920a29b8a90ef54c8c3b53e97) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	ROM_LOAD( "ehs1-sha0.7n",  0x000000, 0x80000, CRC(0f0e2dbf) SHA1(a4575fbdc868ba959d23204be7d659d40d8db4c2) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_512K( "ehs1-dat0.13s",  0x000000, CRC(12a202fb) SHA1(dca1b88cb524d54a699d4f6003854cfc980e084b) )
	NAMCOS2_DATA_LOAD_O_512K( "ehs1-dat1.13p",  0x000000, CRC(91790905) SHA1(988af294959dbb8636f808347b44897a133d3203) )
	NAMCOS2_DATA_LOAD_E_512K( "ehs1-dat2.13r",  0x100000, CRC(087da1f3) SHA1(e9c4ba0383e883502c0f45ae6e6d5daba4eccb01) )
	NAMCOS2_DATA_LOAD_O_512K( "ehs1-dat3.13n",  0x100000, CRC(85aecb3f) SHA1(00ab6104dee0cd0fbdb0235b88b41e4d26794f98) )

	ROM_REGION( 0x100, "c45_road:clut", 0 ) /* prom for road colors */
	ROM_LOAD( "ehs1-landdt.10w", 0, 0x100, CRC(cde7e8a6) SHA1(860273daf2e649418746adf50a67ae33f9f3740c) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "ehs1-voi1.3m",  0x000000, 0x080000, CRC(bf94eb42) SHA1(61bb36550a58ffb8ad0ab8f5b51eddd7824ae8bc) )
	ROM_LOAD16_BYTE( "ehs1-voi2.3l",  0x100000, 0x080000, CRC(0e427604) SHA1(ebb15f53713c24731f6ebdc37ece88587cce5616) )
ROM_END /* suzuk8h2 */

/* SUZUKA 8 HOURS 2 Japan */
ROM_START( suzuk8h2j )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "ehs1_mp0b.11d",  0x000000, 0x020000, CRC(ae40d445) SHA1(a6ce8eb691d2b20a120a2c8ce04dbc925fceaad5) )
	ROM_LOAD16_BYTE( "ehs1_mp1b.13d",  0x000001, 0x020000, CRC(9d5b0d43) SHA1(cc8d288bb91703221697484aa41417c041d6a77f) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "ehs1-sp0.11k",  0x000000, 0x020000, CRC(9ca967bc) SHA1(a4d19ae72d0d8e96c1fa5f7e9d8e002275e7be32) )
	ROM_LOAD16_BYTE( "ehs1-sp1.13k",  0x000001, 0x020000, CRC(f25bfaaa) SHA1(829936a9d19c4a8818684ce263e9ae5cbc7d5ca4) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "ehs1-snd0.7j",  0x000000, 0x020000, CRC(fc95993b) SHA1(e8ecd6dc37634bdd030b8cea0b69818fba5a9388) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "c355spr", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "ehs1-obj0.3p",  0x000000, 0x80000, CRC(a0acf307) SHA1(6d79d2dd00da4f8f0462245f42a9d88b6ad632b1) )
	ROM_LOAD32_BYTE( "ehs1-obj1.3w",  0x000001, 0x80000, CRC(ca780b44) SHA1(d16263851c165f5958b0a2ad1ba199058a8d56d5) )
	ROM_LOAD32_BYTE( "ehs1-obj2.3t",  0x000002, 0x80000, CRC(83b45afe) SHA1(10a4b88b36f8d037cbb611cb273613b1d45e8eb5) )
	ROM_LOAD32_BYTE( "ehs1-obj3.3y",  0x000003, 0x80000, CRC(360c03a8) SHA1(969b1a96833ab2db3d610a2b3793fc1e038b24d8) )
	ROM_LOAD32_BYTE( "ehs1-obj4.3s",  0x200000, 0x80000, CRC(4e503ca5) SHA1(248af1cba2cad2b6e3c53c7c7673165789c5f4d5) )
	ROM_LOAD32_BYTE( "ehs1-obj5.3x",  0x200001, 0x80000, CRC(5405f2d9) SHA1(c2b592abf72f4de22a3863909be579820c8fe5b1) )
	ROM_LOAD32_BYTE( "ehs1-obj6.3j",  0x200002, 0x80000, CRC(f5fc8b23) SHA1(d7cd4596cd6991db72c371d835051cc8001f30b3) )
	ROM_LOAD32_BYTE( "ehs1-obj7.3z",  0x200003, 0x80000, CRC(da6bf51b) SHA1(b9b49b983f76989067c4763fd88bfa11bbf5d064) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "ehs1-chr0.11n", 0x000000, 0x80000, CRC(844efe0d) SHA1(032a2d268bbab60706d911ab42206b5329e1abba) )
	ROM_LOAD( "ehs1-chr1.11p", 0x080000, 0x80000, CRC(e8480a6d) SHA1(247a7ab2f5270a87c6a05b12f01b884fb05dabd2) )
	ROM_LOAD( "ehs1-chr2.11r", 0x100000, 0x80000, CRC(ace2d871) SHA1(1a10f571268d83c70fe00fad2f1ce2b48fb764b1) )
	ROM_LOAD( "ehs1-chr3.11s", 0x180000, 0x80000, CRC(c1680818) SHA1(c8c9fd02b1155924fdc1e930fe176225a3b840a1) )
	ROM_LOAD( "ehs1-chr4.9n",  0x200000, 0x80000, CRC(82e8c1d5) SHA1(4c6d234a875f108623638960258a0cd530cab6da) )
	ROM_LOAD( "ehs1-chr5.9p",  0x280000, 0x80000, CRC(9448537c) SHA1(99b9215907ed8658f19bec1153dc1d48bbfd7753) )
	ROM_LOAD( "ehs1-chr6.9r",  0x300000, 0x80000, CRC(2d1c01ad) SHA1(1ed79e22b964fe648d22e43b78c1c3b5a7d5f8c8) )
	ROM_LOAD( "ehs1-chr7.9s",  0x380000, 0x80000, CRC(18dd8676) SHA1(59b9a07f4a980fd920a29b8a90ef54c8c3b53e97) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	ROM_LOAD( "ehs1-sha0.7n",  0x000000, 0x80000, CRC(0f0e2dbf) SHA1(a4575fbdc868ba959d23204be7d659d40d8db4c2) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_512K( "ehs1-dat0.13s",  0x000000, CRC(12a202fb) SHA1(dca1b88cb524d54a699d4f6003854cfc980e084b) )
	NAMCOS2_DATA_LOAD_O_512K( "ehs1-dat1.13p",  0x000000, CRC(91790905) SHA1(988af294959dbb8636f808347b44897a133d3203) )
	NAMCOS2_DATA_LOAD_E_512K( "ehs1-dat2.13r",  0x100000, CRC(087da1f3) SHA1(e9c4ba0383e883502c0f45ae6e6d5daba4eccb01) )
	NAMCOS2_DATA_LOAD_O_512K( "ehs1-dat3.13n",  0x100000, CRC(85aecb3f) SHA1(00ab6104dee0cd0fbdb0235b88b41e4d26794f98) )

	ROM_REGION( 0x100, "c45_road:clut", 0 ) /* prom for road colors */
	ROM_LOAD( "ehs1-landdt.10w", 0, 0x100, CRC(cde7e8a6) SHA1(860273daf2e649418746adf50a67ae33f9f3740c) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "ehs1-voi1.3m",  0x000000, 0x080000, CRC(bf94eb42) SHA1(61bb36550a58ffb8ad0ab8f5b51eddd7824ae8bc) )
	ROM_LOAD16_BYTE( "ehs1-voi2.3l",  0x100000, 0x080000, CRC(0e427604) SHA1(ebb15f53713c24731f6ebdc37ece88587cce5616) )
ROM_END /* suzuk8h2j */

/* LEGEND OF THE VALKYRIE */
ROM_START( valkyrie )
	ROM_REGION( 0x040000, "maincpu", 0 )     /* Master CPU */
	ROM_LOAD16_BYTE( "wd1mpr0.bin",  0x000000, 0x020000, CRC(94111a2e) SHA1(bd0dfe362fc98c1e94422a898b4daf6a08ef7982) )
	ROM_LOAD16_BYTE( "wd1mpr1.bin",  0x000001, 0x020000, CRC(57b5051c) SHA1(de399381823251e2ee45f51c96be3e93ed763f94) )

	ROM_REGION( 0x040000, "slave", 0 )   /* Slave CPU */
	ROM_LOAD16_BYTE( "wd1spr0.bin",  0x000000, 0x010000, CRC(b2398321) SHA1(13b76d8756a5e47d7469f72ef5fd2837b3e24b3e) )
	ROM_LOAD16_BYTE( "wd1spr1.bin",  0x000001, 0x010000, CRC(38dba897) SHA1(22254186906b1accfb043172e6d0129d23d83b02) )

	ROM_REGION( 0x020000, "audiocpu", 0 )    /* Sound CPU (Banked) */
	ROM_LOAD( "wd1snd0.bin",  0x000000, 0x020000, CRC(d0fbf58b) SHA1(bc149eefcf5566a8e37ae60a62f5037a6e0f10bd) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 )          /* Sprites */
	NAMCOS2_SPRROM_LOAD_256K( "wdobj0.bin",  0x000003, CRC(e8089451) SHA1(f4d05df0015de01ec570f5f89ea11592204e4fe2) )
	NAMCOS2_SPRROM_LOAD_256K( "wdobj1.bin",  0x000002, CRC(7ca65666) SHA1(39d792abf5a1a5f3906cb6ab4626f4a5b20cb081) )
	NAMCOS2_SPRROM_LOAD_256K( "wdobj2.bin",  0x000001, CRC(7c159407) SHA1(ed5472eb9df7990b8d80ff5a587e41d138f48db8) )
	NAMCOS2_SPRROM_LOAD_256K( "wdobj3.bin",  0x000000, CRC(649f8760) SHA1(3ac7eac5197b4d377686d68d80ab29562768c202) )
	NAMCOS2_SPRROM_LOAD_256K( "wdobj4.bin",  0x200003, CRC(7ca39ae7) SHA1(3db34eb7f8c819c7b30c3b61b5a13b8f990b27f8) )
	NAMCOS2_SPRROM_LOAD_256K( "wdobj5.bin",  0x200002, CRC(9ead2444) SHA1(0ba541b518be22460c267d35b050594b7cedb954) )
	NAMCOS2_SPRROM_LOAD_256K( "wdobj6.bin",  0x200001, CRC(9fa2ea21) SHA1(89cefc286cf4de7f6e32dc6dc689835a21bea2ed) )
	NAMCOS2_SPRROM_LOAD_256K( "wdobj7.bin",  0x200000, CRC(66e07a36) SHA1(2f84128bbdc9dcfd783d3a85cb47a92087e71272) )

	ROM_REGION( 0x400000, "c123tmap", 0 )          /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "wdchr0.bin",  0x000000, CRC(debb0116) SHA1(ba4a9a166e22cf4930574eeadd127520ff7192b4) )
	NAMCOS2_GFXROM_LOAD_128K( "wdchr1.bin",  0x080000, CRC(8a1431e8) SHA1(ef3c81ea92f2541387760c24395ddf7a02f907f2) )
	NAMCOS2_GFXROM_LOAD_128K( "wdchr2.bin",  0x100000, CRC(62f75f69) SHA1(bcfd34c3c450cd05eb6a080307dc962687ccc799) )
	NAMCOS2_GFXROM_LOAD_128K( "wdchr3.bin",  0x180000, CRC(cc43bbe7) SHA1(9b01ac19feac60a68cd7ffcf3cab927cefa0e23a) )
	NAMCOS2_GFXROM_LOAD_128K( "wdchr4.bin",  0x200000, CRC(2f73d05e) SHA1(76341fd8c5e4216430eb09ded38ac554b5777ee5) )
	NAMCOS2_GFXROM_LOAD_128K( "wdchr5.bin",  0x280000, CRC(b632b2ec) SHA1(914cb9a0dd42067b93e47c8cf847d588f361854f) )

	ROM_REGION( 0x400000, "s2roz", 0 )    /* ROZ Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "wdroz0.bin",  0x000000, CRC(f776bf66) SHA1(2b134d5f9ede1205260a2109fdcd0d5c47b397ba) )
	NAMCOS2_GFXROM_LOAD_128K( "wdroz1.bin",  0x080000, CRC(c1a345c3) SHA1(3d8c2ab457b81485014a83b4d9e8a80fe487ddef) )
	NAMCOS2_GFXROM_LOAD_128K( "wdroz2.bin",  0x100000, CRC(28ffb44a) SHA1(207343e76dd02cbfe7575edd9b18074f68bb6768) )
	NAMCOS2_GFXROM_LOAD_128K( "wdroz3.bin",  0x180000, CRC(7e77b46d) SHA1(db0ba244fb878120a4f06af4666ddc4cc430c97a) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 )    /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "wdshape.bin",  0x000000, CRC(3b5e0249) SHA1(259bbabf57a8ad739c646c56aad6d0b92e10adcd) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 )  /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "wd1dat0.13s",  0x000000, CRC(ea209f48) SHA1(5e73a745dc2faaa4ce6c633d4072d41e9e494276) )
	NAMCOS2_DATA_LOAD_O_128K( "wd1dat1.13p",  0x000000, CRC(04b48ada) SHA1(aa046f8856bdd5b56d481c2c12ad2808c6517a5f) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 )    /* Sound voices */
	NAMCOS2_DATA_LOAD_E_256K( "wd1voi1.bin",  0x000000, CRC(f1ace193) SHA1(dd13bdf4b99c6bf4e356d623ff2e3da72db331dd) )
	NAMCOS2_DATA_LOAD_E_128K( "wd1voi2.bin",  0x100000, CRC(e95c5cf3) SHA1(4bfc7303bde23bcf6739c7877dd87671c33135bc) )
ROM_END

/* KYUUKAI DOUCHUUKI */
ROM_START( kyukaidk )
	ROM_REGION( 0x040000, "maincpu", 0 )     /* Master CPU */
	ROM_LOAD16_BYTE( "ky1_mp0b.bin", 0x000000, 0x010000, CRC(d1c992c8) SHA1(f3d49159ca3cbec7b905e6d877b8b76d023dd621) )
	ROM_LOAD16_BYTE( "ky1_mp1b.bin", 0x000001, 0x010000, CRC(723553af) SHA1(ae611c699c276521a064b79c74fcc0ecd76a22cf) )

	ROM_REGION( 0x040000, "slave", 0 )   /* Slave CPU */
	ROM_LOAD16_BYTE( "ky1_sp0.bin",  0x000000, 0x010000, CRC(4b4d2385) SHA1(e67e97108c63f1236e80045474a45ede386170f8) )
	ROM_LOAD16_BYTE( "ky1_sp1.bin",  0x000001, 0x010000, CRC(bd3368cd) SHA1(4fc9d3f5182fd7db7e6003c15cdec1dbcdff5384) )

	ROM_REGION( 0x020000, "audiocpu", 0 )    /* Sound CPU (Banked) */
	ROM_LOAD( "ky1_s0.bin",   0x000000, 0x020000, CRC(27aea3e9) SHA1(c9ee7eaef1885fd8642595fa46833e1b473e66b5) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 )          /* Sprites */
	ROM_LOAD32_BYTE( "ky1_o0.bin",  0x000003, 0x80000, CRC(ebec5132) SHA1(8d2dec3f1cd27c203899bb715a9983fff7ab820d) )
	ROM_LOAD32_BYTE( "ky1_o1.bin",  0x000002, 0x80000, CRC(fde7e5ae) SHA1(e17822f885977e10b6d1524a3d97fa9640472f8a) )
	ROM_LOAD32_BYTE( "ky1_o2.bin",  0x000001, 0x80000, CRC(2a181698) SHA1(bad62c6c59b4362d6815749b5622e321b6051ea4) )
	ROM_LOAD32_BYTE( "ky1_o3.bin",  0x000000, 0x80000, CRC(71fcd3a6) SHA1(8f2ba9c1d4fabbac07d1c80dfc3580cc67594071) )

	ROM_REGION( 0x400000, "c123tmap", 0 )          /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "ky1_c0.bin",  0x000000, CRC(7bd69a2d) SHA1(21402395eaacd4c25e5f023ea48a206b818b9c25) )
	NAMCOS2_GFXROM_LOAD_128K( "ky1_c1.bin",  0x080000, CRC(66a623fe) SHA1(35404b3aa05263dda6be7c1903ea77677e3c58cb) )
	NAMCOS2_GFXROM_LOAD_128K( "ky1_c2.bin",  0x100000, CRC(e84b3dfd) SHA1(742c88c187c14301afaabd0d2d8eba11185aaef1) )
	NAMCOS2_GFXROM_LOAD_128K( "ky1_c3.bin",  0x180000, CRC(69e67c86) SHA1(c835ad3e6908acd96b299826c2caa61255958f8c) )

	ROM_REGION( 0x400000, "s2roz", 0 )    /* ROZ Tiles */
	NAMCOS2_GFXROM_LOAD_256K( "ky1_r0.bin",  0x000000, CRC(9213e8c4) SHA1(178ea4db500adbc945bab0b6f1a014e89c077d77) )
	NAMCOS2_GFXROM_LOAD_256K( "ky1_r1.bin",  0x080000, CRC(97d1a641) SHA1(d1708dcc668ad126ee34f7f84cbac9a56a617d7b) )
	NAMCOS2_GFXROM_LOAD_256K( "ky1_r2.bin",  0x100000, CRC(39b58792) SHA1(1c30a04a370f56d2b548bf19c551acc95316693b) )
	NAMCOS2_GFXROM_LOAD_256K( "ky1_r3.bin",  0x180000, CRC(90c60d92) SHA1(b581d5f4d6c353604e0c9bcb27252a44b1b4e91c) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 )    /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "ky1_sha.bin",  0x000000, CRC(380a20d7) SHA1(9627d2e0192779d50bfb25d5c19976275818c310) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 )  /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "ky1_d0.13s",   0x000000, CRC(c9cf399d) SHA1(90cba42781b1e03ecc3f5b802d740ace6b88baaa) )
	NAMCOS2_DATA_LOAD_O_128K( "ky1_d1.13p",   0x000000, CRC(6d4f21b9) SHA1(f234a785f61969be684d2a4aed59616f125d72fc) )
	NAMCOS2_DATA_LOAD_E_128K( "ky1_d2.13r",   0x100000, CRC(eb6d19c8) SHA1(c9fdb33fe191d3c4d284db7cbb05d852551a998d) )
	NAMCOS2_DATA_LOAD_O_128K( "ky1_d3.13n",   0x100000, CRC(95674701) SHA1(9a8832837b9a3f8b75437717ea84d86261bfce59) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 )    /* Sound voices */
	ROM_LOAD16_BYTE( "ky1_v1.bin", 0x000000, 0x080000, CRC(5ff81aec) SHA1(0535eda474de0a4aa3b48649b04afe2b7a8619c9) )
ROM_END

/* KYUUKAI DOUCHUUKI (OLD) */
ROM_START( kyukaidko )
	ROM_REGION( 0x040000, "maincpu", 0 )     /* Master CPU */
	ROM_LOAD16_BYTE( "ky1_mp0.bin",  0x000000, 0x010000, CRC(01978a19) SHA1(dac40f6629ee025733ac160d83523488c99794c7) )
	ROM_LOAD16_BYTE( "ky1_mp1.bin",  0x000001, 0x010000, CRC(b40717a7) SHA1(10a0e4eaa2502f51f34949007e6cc9d3bf031554) )

	ROM_REGION( 0x040000, "slave", 0 )   /* Slave CPU */
	ROM_LOAD16_BYTE( "ky1_sp0.bin",  0x000000, 0x010000, CRC(4b4d2385) SHA1(e67e97108c63f1236e80045474a45ede386170f8) )
	ROM_LOAD16_BYTE( "ky1_sp1.bin",  0x000001, 0x010000, CRC(bd3368cd) SHA1(4fc9d3f5182fd7db7e6003c15cdec1dbcdff5384) )

	ROM_REGION( 0x020000, "audiocpu", 0 )    /* Sound CPU (Banked) */
	ROM_LOAD( "ky1_s0.bin",   0x000000, 0x020000, CRC(27aea3e9) SHA1(c9ee7eaef1885fd8642595fa46833e1b473e66b5) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "sys2c65c.bin",  0x000000, 0x008000, CRC(a5b2a4ff) SHA1(068bdfcc71a5e83706e8b23330691973c1c214dc) )

	ROM_REGION( 0x400000, "sprite", 0 )          /* Sprites */
	ROM_LOAD32_BYTE( "ky1_o0.bin",  0x000003, 0x80000, CRC(ebec5132) SHA1(8d2dec3f1cd27c203899bb715a9983fff7ab820d) )
	ROM_LOAD32_BYTE( "ky1_o1.bin",  0x000002, 0x80000, CRC(fde7e5ae) SHA1(e17822f885977e10b6d1524a3d97fa9640472f8a) )
	ROM_LOAD32_BYTE( "ky1_o2.bin",  0x000001, 0x80000, CRC(2a181698) SHA1(bad62c6c59b4362d6815749b5622e321b6051ea4) )
	ROM_LOAD32_BYTE( "ky1_o3.bin",  0x000000, 0x80000, CRC(71fcd3a6) SHA1(8f2ba9c1d4fabbac07d1c80dfc3580cc67594071) )

	ROM_REGION( 0x400000, "c123tmap", 0 )          /* Tiles */
	NAMCOS2_GFXROM_LOAD_128K( "ky1_c0.bin",  0x000000, CRC(7bd69a2d) SHA1(21402395eaacd4c25e5f023ea48a206b818b9c25) )
	NAMCOS2_GFXROM_LOAD_128K( "ky1_c1.bin",  0x080000, CRC(66a623fe) SHA1(35404b3aa05263dda6be7c1903ea77677e3c58cb) )
	NAMCOS2_GFXROM_LOAD_128K( "ky1_c2.bin",  0x100000, CRC(e84b3dfd) SHA1(742c88c187c14301afaabd0d2d8eba11185aaef1) )
	NAMCOS2_GFXROM_LOAD_128K( "ky1_c3.bin",  0x180000, CRC(69e67c86) SHA1(c835ad3e6908acd96b299826c2caa61255958f8c) )

	ROM_REGION( 0x400000, "s2roz", 0 )    /* ROZ Tiles */
	NAMCOS2_GFXROM_LOAD_256K( "ky1_r0.bin",  0x000000, CRC(9213e8c4) SHA1(178ea4db500adbc945bab0b6f1a014e89c077d77) )
	NAMCOS2_GFXROM_LOAD_256K( "ky1_r1.bin",  0x080000, CRC(97d1a641) SHA1(d1708dcc668ad126ee34f7f84cbac9a56a617d7b) )
	NAMCOS2_GFXROM_LOAD_256K( "ky1_r2.bin",  0x100000, CRC(39b58792) SHA1(1c30a04a370f56d2b548bf19c551acc95316693b) )
	NAMCOS2_GFXROM_LOAD_256K( "ky1_r3.bin",  0x180000, CRC(90c60d92) SHA1(b581d5f4d6c353604e0c9bcb27252a44b1b4e91c) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 )    /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "ky1_sha.bin",  0x000000, CRC(380a20d7) SHA1(9627d2e0192779d50bfb25d5c19976275818c310) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 )  /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "ky1_d0.13s",   0x000000, CRC(c9cf399d) SHA1(90cba42781b1e03ecc3f5b802d740ace6b88baaa) )
	NAMCOS2_DATA_LOAD_O_128K( "ky1_d1.13p",   0x000000, CRC(6d4f21b9) SHA1(f234a785f61969be684d2a4aed59616f125d72fc) )
	NAMCOS2_DATA_LOAD_E_128K( "ky1_d2.13r",   0x100000, CRC(eb6d19c8) SHA1(c9fdb33fe191d3c4d284db7cbb05d852551a998d) )
	NAMCOS2_DATA_LOAD_O_128K( "ky1_d3.13n",   0x100000, CRC(95674701) SHA1(9a8832837b9a3f8b75437717ea84d86261bfce59) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 )    /* Sound voices */
	ROM_LOAD16_BYTE( "ky1_v1.bin", 0x000000, 0x080000, CRC(5ff81aec) SHA1(0535eda474de0a4aa3b48649b04afe2b7a8619c9) )
ROM_END

/* GOLLY GHOST */
ROM_START( gollygho )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "gl2mpr0.11d", 0x000000, 0x010000, CRC(e5d48bb9) SHA1(53e790b9f11a8befc6e3b79b7af2e97500562e20) )
	ROM_LOAD16_BYTE( "gl2mpr1.13d", 0x000001, 0x010000, CRC(584ef971) SHA1(6ebeba593f056823311181831824cc9ef26ef6ea) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "gl1spr0.11k", 0x000000, 0x010000, CRC(a108136f) SHA1(5a90d9d77a0c98b590c8979df25821331c669c09) )
	ROM_LOAD16_BYTE( "gl1spr1.13k", 0x000001, 0x010000, CRC(da8443b7) SHA1(99bb873c253da65422b872783f28eb13993b0e2f) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "gl1snd0.7j",  0x000000, 0x020000, CRC(008bce72) SHA1(4197a492639a80457fdb7851ffc95587dfd64320) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "gl1edr0c.ic7", 0x0000, 0x8000, CRC(db60886f) SHA1(a1183c058c0470a4ef8b0f69a3637b1640c5b5a4) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "gl1obj0.5b",  0x000003, 0x40000, CRC(6809d267) SHA1(8a0f636067974e51659bd05a3c17819c630d70e3) )
	ROM_LOAD32_BYTE( "gl1obj1.4b",  0x000002, 0x40000, CRC(ae4304d4) SHA1(e3db507acf2ab9392060fc603bb95492d0251adc) )
	ROM_LOAD32_BYTE( "gl1obj2.5d",  0x000001, 0x40000, CRC(9f2e9eb0) SHA1(d3b001286a5ede58860505bb2f48a755cc661f1f) )
	ROM_LOAD32_BYTE( "gl1obj3.4d",  0x000000, 0x40000, CRC(3a85f3c2) SHA1(013148cc0174d39bb16a71cce01c0dc7044d2f42) )

	ROM_REGION( 0x60000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "gl1chr0.11n",  0x00000, 0x20000, CRC(1a7c8abd) SHA1(59ddc278c46e545bbc3d66e84810c40aaf703d9a) )
	ROM_LOAD( "gl1chr1.11p",  0x20000, 0x20000, CRC(36aa0fbc) SHA1(47dc10e689843962b51097c6ca27e00a221d2b48) )
	ROM_LOAD( "gl1chr2.11r",  0x40000, 0x10000, CRC(6c1964ba) SHA1(1aec2ce598751f4f0ba575f4a944647e831f6a87) )

	ROM_REGION( 0x400000, "s2roz", ROMREGION_ERASEFF ) /* ROZ Tiles */
	/* All ROZ ROM sockets unpopulated on PCB */

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	NAMCOS2_GFXROM_LOAD_128K( "gl1sha0.7n", 0x000000, CRC(8886f6f5) SHA1(3b311c5061449e1bbde1a1006fd967a6154326b8) )

	ROM_REGION16_BE( 0x200000, "data_rom", ROMREGION_ERASEFF ) /* Shared data roms */
	/* All DAT ROM sockets unpopulated on PCB */

	ROM_REGION16_BE( 0x2000, "user2", 0 ) /* sprite zoom */
	ROM_LOAD( "04544191.6n",  0x000000, 0x002000, CRC(90db1bf6) SHA1(dbb9e50a8efc3b4012fcf587cc87da9ef42a1b80) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "gl1voi1.3m",  0x000000, 0x080000, CRC(0eca0efb) SHA1(4e8e1b3118ee0b76c34dd6631047080ba1fcf576) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "gollygho.nv",  0x000000, 0x2000, CRC(b7e67b9d) SHA1(bb6e2d2cac1a2f3c26fa1327db0eb53b521005a3) )
ROM_END

/* BUBBLE TROUBLE (World) */
ROM_START( bubbletr ) /* All labels were hand written and included the rom size, checksum & date. All roms had the rev "A" added */
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "bt2-mpr0b.bin",   0x000000, 0x020000, CRC(26fbfce3) SHA1(532a8bc7f961ddb0ba2af5a6912a4898f84b144d) ) /* dated 6/9 */
	ROM_LOAD16_BYTE( "bt2-mpr1b.bin",   0x000001, 0x020000, CRC(21f42ab2) SHA1(c563ff6b81d036c9044125c45b1cf4da0f5495d9) ) /* dated 6/9 */

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "bt1-spr0.11k",    0x000000, 0x010000, CRC(b507b00a) SHA1(e7dcb36accf91e8b5b5321e65e6fd6bca7c6dce2) ) /* dated 6/8 */
	ROM_LOAD16_BYTE( "bt1-spr1.13k",    0x000001, 0x010000, CRC(4f35540f) SHA1(519510de81e566ed74b80987860778201f6ac785) ) /* dated 6/8 */

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "bt1-snd0.7j",  0x000000, 0x020000, CRC(46a5c625) SHA1(ccc7098f76a0bc39c9fe578d575e6002eca172b5) ) /* dated 6/8 */

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "bt1edr0a.ic7", 0x0000, 0x8000, CRC(155b02fc) SHA1(191683c19f756ac150b8e037f46a6daca1a082fa) ) /* dated 4/24 */

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "bt1-obj0.5b",  0x000003, 0x80000, CRC(16b5dc04) SHA1(57cc4b7907442f922102fbd61e470c149f0379ac) ) /* dated 4/24 */
	ROM_LOAD32_BYTE( "bt1-obj1.4b",  0x000002, 0x80000, CRC(ae37a969) SHA1(524a8ef68a62f9168d356e6cd37a72a888ced202) ) /* dated 4/24 */
	ROM_LOAD32_BYTE( "bt1-obj2.5d",  0x000001, 0x80000, CRC(75f74871) SHA1(75c47a2132e21a2d82000bca137929bffecc96ef) ) /* dated 4/24 */
	ROM_LOAD32_BYTE( "bt1-obj3.4d",  0x000000, 0x80000, CRC(7fb23c05) SHA1(da97c595a3338021c1bc46f9668e1ec6c7985cea) ) /* dated 4/24 */

	ROM_REGION( 0x200000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "bt1-chr0.11n",  0x00000, 0x80000,  CRC(11574c30) SHA1(6e85dd1448961b89a13e8cf905b24a69d182edd8) ) /* dated 4/24 */
	/* no chr1, chr2 missing or just not needed? probably just not needed as I see no tile glitches */

	ROM_REGION( 0x400000, "s2roz", ROMREGION_ERASEFF ) /* ROZ Tiles */
	/* All ROZ ROM sockets unpopulated on PCB */

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	ROM_LOAD( "bt1-sha0.7n",   0x000000, 0x80000, CRC(dc4664df) SHA1(59818b14e74ee9b15a66e850658e4697d78b28d9) ) /* dated 4/24 */

	ROM_REGION16_BE( 0x200000, "data_rom", ROMREGION_ERASEFF ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "bt1_dat0.13s",   0x000000, CRC(1001a14e) SHA1(7017a33f0447fb6013d4e246dcdfcd064af87812) ) /* dated 4/24 */
	NAMCOS2_DATA_LOAD_O_128K( "bt1_dat1.13p",   0x000000, CRC(7de6a839) SHA1(e6a3fd5b789dc061ec504570984cf61a6af7818f) ) /* dated 4/24 */

	ROM_REGION16_BE( 0x2000, "user2", 0 ) /* sprite zoom */
	ROM_LOAD( "04544191.6n",  0x000000, 0x002000, CRC(90db1bf6) SHA1(dbb9e50a8efc3b4012fcf587cc87da9ef42a1b80) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "bt1-voi1.3m",  0x000000, 0x080000,  CRC(08b3a089) SHA1(5023c2c0d0a94f0a2f98605d9b93d2d6ce626aa8) ) /* dated 6/8 */

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "bubbletr.nv",  0x000000, 0x2000, CRC(75ace624) SHA1(1acd2af0c825e50c542db975e1fbda8cbb009f1d) )
ROM_END

/* BUBBLE TROUBLE (Japan) */
ROM_START( bubbletrj )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "bt1-mpr0c.11d",   0x000000, 0x020000, CRC(64eb3496) SHA1(30cdc116e350270f0303e5d9dddf6c3c8bccbc04) )
	ROM_LOAD16_BYTE( "bt1-mpr1c.13d",   0x000001, 0x020000, CRC(26785bce) SHA1(ce83a3f2a8adc1a9c3fd9e677565a649a8168801) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "bt1-spr0.11k",    0x000000, 0x010000, CRC(b507b00a) SHA1(e7dcb36accf91e8b5b5321e65e6fd6bca7c6dce2) )
	ROM_LOAD16_BYTE( "bt1-spr1.13k",    0x000001, 0x010000, CRC(4f35540f) SHA1(519510de81e566ed74b80987860778201f6ac785) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "bt1-snd0.7j",  0x000000, 0x020000, CRC(46a5c625) SHA1(ccc7098f76a0bc39c9fe578d575e6002eca172b5) )

	ROM_REGION( 0x8000, "c65mcu:external", ROMREGION_ERASE00 ) /* I/O MCU */
	ROM_LOAD( "bt1edr0a.ic7", 0x0000, 0x8000, CRC(155b02fc) SHA1(191683c19f756ac150b8e037f46a6daca1a082fa) )

	ROM_REGION( 0x400000, "sprite", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "bt1-obj0.5b",  0x000003, 0x80000, CRC(16b5dc04) SHA1(57cc4b7907442f922102fbd61e470c149f0379ac) )
	ROM_LOAD32_BYTE( "bt1-obj1.4b",  0x000002, 0x80000, CRC(ae37a969) SHA1(524a8ef68a62f9168d356e6cd37a72a888ced202) )
	ROM_LOAD32_BYTE( "bt1-obj2.5d",  0x000001, 0x80000, CRC(75f74871) SHA1(75c47a2132e21a2d82000bca137929bffecc96ef) )
	ROM_LOAD32_BYTE( "bt1-obj3.4d",  0x000000, 0x80000, CRC(7fb23c05) SHA1(da97c595a3338021c1bc46f9668e1ec6c7985cea) )

	ROM_REGION( 0x200000, "c123tmap", 0 ) /* Tiles */
	ROM_LOAD( "bt1-chr0.11n",  0x00000, 0x80000,  CRC(11574c30) SHA1(6e85dd1448961b89a13e8cf905b24a69d182edd8) )
	/* no chr1, chr2 missing or just not needed? probably just not needed as I see no tile glitches */

	ROM_REGION( 0x400000, "s2roz", ROMREGION_ERASEFF ) /* ROZ Tiles */
	/* All ROZ ROM sockets unpopulated on PCB */

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* Mask shape */
	ROM_LOAD( "bt1-sha0.7n",   0x000000, 0x80000, CRC(dc4664df) SHA1(59818b14e74ee9b15a66e850658e4697d78b28d9) )

	ROM_REGION16_BE( 0x200000, "data_rom", ROMREGION_ERASEFF ) /* Shared data roms */
	NAMCOS2_DATA_LOAD_E_128K( "bt1_dat0.13s",   0x000000, CRC(1001a14e) SHA1(7017a33f0447fb6013d4e246dcdfcd064af87812) )
	NAMCOS2_DATA_LOAD_O_128K( "bt1_dat1.13p",   0x000000, CRC(7de6a839) SHA1(e6a3fd5b789dc061ec504570984cf61a6af7818f) )

	ROM_REGION16_BE( 0x2000, "user2", 0 ) /* sprite zoom */
	ROM_LOAD( "04544191.6n",  0x000000, 0x002000, CRC(90db1bf6) SHA1(dbb9e50a8efc3b4012fcf587cc87da9ef42a1b80) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "bt1-voi1.3m",  0x000000, 0x080000,  CRC(08b3a089) SHA1(5023c2c0d0a94f0a2f98605d9b93d2d6ce626aa8) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "bubbletr.nv",  0x000000, 0x2000, CRC(75ace624) SHA1(1acd2af0c825e50c542db975e1fbda8cbb009f1d) )
ROM_END


/*
Lucky & Wild
Namco, 1989

PCB Layout
----------

Top Board

8618963905  (8618961805)
|--------------------------------------------------------------------|
| TA7630  C116     M5M5178                                 49.152MHz |
|  TL084  TL084    M5M5178                                      137  |
| LC7881           M5M5178       9S*      LW1_CHR-3.11S  LW1_DAT0.13S|
| LA4700     156         123     9R*      LW1_CHR-2.11R  LW1_DAT2.13R|
|                           LW1_CHR-5.9P  LW1_CHR-1.11P  LW1_DAT1.13P|
|140           LW1_SHA-0.7N LW1_CHR-4.9N  LW1_CHR-0.11N  LW1_DAT3.13N|
|      3N*                                                           |
|  LW1_VOI-1.3M          145  84256                                  |
|J LW1_VOI-2.3L               84256       65256          65256       |
|A                                                                   |
|M   8464       2018                      LW1SP0.11K     LW1SP1.13K  |
|M              2018                                                 |
|A   YM2151                  LW1SND0.7J                              |
|          121  6809            7G*            68000         148     |
|          SYS87B-1(PAL)   8464                                      |
|               SYS87B-2B(PAL)                                       |
|        3F(C68PRG)*                                                 |
|                                                                    |
|   DSW(8)    C68                                                    |
| 3.579545MHz            CY7C132                                     |
|    149                      SYS87B-3(PAL)     62256       62256    |
|                                  139                               |
|                                     HN58C65  LW2MP0.11D  LW2MP1.13D|
|                               M5M5179               68000      148 |
|--------------------------------------------------------------------|
Notes:
      68000 clock : xx MHz
      6809 clock  : xx MHz
      YM2151 clock: xx MHz
      *           : Unpopulated socket

      Namco customs:
                    C116 (QFP80)
                    156  (QFP64)
                    123  (QFP80)
                    145  (QFP80)
                    148  (QFP64, x2)
                    139  (QFP64)
                    C68  (QFP80)
                    149  (DIP28)
                    137  (DIP28)
                    140  (QFP120)
                    121  (QFP64)


Bottom Board

SYSTEM2 VIDEO(E)
8618962600
8618962599
8618962400
(8618964200)

|--------------------------------------------------------------------|
|                            33.300MHz   137                         |
|   G272DEC(PAL)    KEYCUS   TC15832       TC15832     LW1_ROZ-0.23B |
|                            TC15832       TC15832     LW1_ROZ-1.23C |
| CY7C199                    TC15832       TC15832     LW1_ROZ-2.23E |
| CY7C199                    TC15832       TC15832 G272PRI(PAL)  23J*|
|                            TC15832       TC15832               23K*|
|                            TC15832       TC15832               23L*|
|                            TC15832       TC15832               23P*|
| C355       187             TC15832       TC15832               23R*|
|                                                                    |
|                                                      DSW2(6)       |
|                                                                    |
| LW1_OBJ-0.3P                                                       |
|                                                                    |
| LW1_OBJ-4.3P                                                       |
|                                                                    |
| LW1_OBJ-2.3P                                                       |
|                                                                    |
| LW1_OBJ-6.3P                                                       |
|                                                                    |
| LW1_OBJ-1.3P            LW1_LD8.10W                                |
|                                                                    |
| LW1_OBJ-5.3P                                169                    |
|                                                                    |
| LW1_OBJ-3.3P    45                  373                            |
|                                             84256                  |
| LW1_OBJ-7.3P                                84256   LW1_RZS-0.20Z  |
|DSW1(6)                                      84256                  |
|--------------------------------------------------------------------|
Notes:
      KEYCUS      : socket not populated
      LW1_LD8.10W : PROM type MB7118
      *           : Unpopulated socket

      Namco customs:
                    45   (QFP80)
                    C355 (QFP160)
                    187  (QFP120)
                    137  (DIP28)
                    169  (QFP120)
                    373  (SDIP64)
*/

ROM_START( luckywld )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "lw2mp0.11d",  0x000000, 0x020000, CRC(368306bb) SHA1(e529ff2cc8baed8fd9cb1c010ad0c9f46c5e5f97) )
	ROM_LOAD16_BYTE( "lw2mp1.13d",  0x000001, 0x020000, CRC(9be3a4b8) SHA1(238977abf558db6d91f9d8b289288bdb65715529) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "lw1sp0.11k",  0x000000, 0x020000, CRC(1eed12cb) SHA1(a978c0da0157ec0b8a7ba7cb6b6732fe89bac615) )
	ROM_LOAD16_BYTE( "lw1sp1.13k",  0x000001, 0x020000, CRC(535033bc) SHA1(2ab10bc9adebd93e385e02f211c5705863a49c9a) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "lw1snd0.7j",  0x000000, 0x020000, CRC(cc83c6b6) SHA1(6ac6848f7ac72feb756c5d76de9ea7d26814cb6e) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "c355spr", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "lw1obj0.3p",  0x000000, 0x80000, CRC(21485830) SHA1(e55a1f6df90c17b9c49e2b08c423b9be86996659) )
	ROM_LOAD32_BYTE( "lw1obj1.3w",  0x000001, 0x80000, CRC(d6437a82) SHA1(0aad3242828ed7dce65db75cad196c44ddd55ba8) )
	ROM_LOAD32_BYTE( "lw1obj2.3t",  0x000002, 0x80000, CRC(ceb6f516) SHA1(943dfe3bcf71a4885ce0ff33aaf81b2a49cf0b70) )
	ROM_LOAD32_BYTE( "lw1obj3.3y",  0x000003, 0x80000, CRC(5d32c7e9) SHA1(d684b68afaeacbbc734d55c1a970dc94f3459972) )
	ROM_LOAD32_BYTE( "lw1obj4.3s",  0x200000, 0x80000, CRC(0050458a) SHA1(605ea055b1934f83ca5ffaa532d0ae85ca56aefa) )
	ROM_LOAD32_BYTE( "lw1obj5.3x",  0x200001, 0x80000, CRC(cbc08f46) SHA1(2ece63a0544b39439255f0e0866a8675b3466643) )
	ROM_LOAD32_BYTE( "lw1obj6.3u",  0x200002, 0x80000, CRC(29740c88) SHA1(4078a5084256653a9c8ff72a7e2c652b0fbca425) )
	ROM_LOAD32_BYTE( "lw1obj7.3z",  0x200003, 0x80000, CRC(8cbd62b4) SHA1(c6605ae2629b34f036e440573b2bb68e26aced9b) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* 8x8 Tiles */
	ROM_LOAD( "lw1chr0.11n", 0x000000, 0x80000, CRC(a0da15fd) SHA1(d772f712f0c150fdeb5aafb84f27a1495ad3492c) )
	ROM_LOAD( "lw1chr1.11p", 0x080000, 0x80000, CRC(89102445) SHA1(74d4e51a3540f72cc90fad04c7f0622930e0f854) )
	ROM_LOAD( "lw1chr2.11r", 0x100000, 0x80000, CRC(c749b778) SHA1(f1e30f5269eced00a09af40717634ec65e64f06b) )
	ROM_LOAD( "lw1chr3.11s", 0x180000, 0x80000, CRC(d76f9578) SHA1(07ac3331c0dead612e305f850aceab4f31fa89f8) )
	ROM_LOAD( "lw1chr4.9n",  0x200000, 0x80000, CRC(2f8ab45e) SHA1(2c7e5597ebf020aa88349c6d5a419ba0eee2db90) )
	ROM_LOAD( "lw1chr5.9p",  0x280000, 0x80000, CRC(c9acbe61) SHA1(3a42ec1fcdf994bd9a309fdb48da01af4afb0a3f) )

	ROM_REGION( 0x400000, "c169roz", 0 ) /* 16x16 Tiles */
	ROM_LOAD( "lw1roz1.23c", 0x080000, 0x80000, CRC(74e98793) SHA1(973dce838d9fbdf112429ab20edcdfbf07c3a9dc) )
	ROM_LOAD( "lw1roz2.23e", 0x000000, 0x80000, CRC(1ef46d82) SHA1(2234585875bccdff74fa3f66d2d25d4b419c3bfe) )
	ROM_LOAD( "lw1roz0.23b", 0x1c0000, 0x80000, CRC(a14079c9) SHA1(631f9b63488fd3194ebb41c9c7ec39d576b098d7) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* 8x8 shape */
	ROM_LOAD( "lw1sha0.7n",  0x000000, 0x80000, CRC(e3a65196) SHA1(c983a6d16dc10f0acd5f4ef5e148271f46ffbf28) )

	ROM_REGION( 0x80000, "c169roz:mask", 0 ) /* 16x16 shape */
	ROM_LOAD( "lw1rzs0.20z", 0x000000, 0x80000, CRC(a1071537) SHA1(bb8a97b82066d7838471e258d1c3c716ede7572c) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	ROM_LOAD16_BYTE( "lw1dat0.13s",  0x000000, 0x80000, CRC(5d387d47) SHA1(e40ef52d1403fa630294d736f35a6924fa4e762e) )
	ROM_LOAD16_BYTE( "lw1dat1.13p",  0x000001, 0x80000, CRC(7ba94476) SHA1(cd4a964050e706f783450ace277e67586bf69e61) )
	ROM_LOAD16_BYTE( "lw1dat2.13r",  0x100000, 0x80000, CRC(eeba7c62) SHA1(6468518d3a5499b3f9a066488d83252cfc804d69) )
	ROM_LOAD16_BYTE( "lw1dat3.13n",  0x100001, 0x80000, CRC(ec3b36ea) SHA1(734549ada73a687377134051fa906b489ffd0dc4) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "lw1voi1.3m",  0x000000, 0x080000, CRC(b3e57993) SHA1(ff7071fc2e2c00f0cf819860c2a9be353474920a) )
	ROM_LOAD16_BYTE( "lw1voi2.3l",  0x100000, 0x080000, CRC(cd8b86a2) SHA1(54bbc91e995ea0c33874ce6fe5c3f014e173da07) )

	ROM_REGION( 0x100, "c45_road:clut", 0 ) /* prom for road colors */
	ROM_LOAD( "lw1ld8.10w", 0, 0x100, CRC(29058c73) SHA1(4916d6bdb7f78e6803698cab32d1586ea457dfc8) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration - see notes with inputs */
	ROM_LOAD( "luckywld.nv",  0x000000, 0x2000, CRC(0c185d2a) SHA1(6a4876361731df423515a0f3b37ba1496d6b1964) )
ROM_END

/* LUCKY & WILD (Japan) */
ROM_START( luckywldj )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* Master CPU */
	ROM_LOAD16_BYTE( "lw1mpr0.11d", 0x000000, 0x020000, CRC(7dce8ba6) SHA1(6cd307e5d88038eda6e061113d7ef2c059da915e) )
	ROM_LOAD16_BYTE( "lw1mpr1.13d", 0x000001, 0x020000, CRC(ce3b0f37) SHA1(e9edb953eae21465ea231de9d6f2e7f1072c4440) )

	ROM_REGION( 0x040000, "slave", 0 ) /* Slave CPU */
	ROM_LOAD16_BYTE( "lw1sp0.11k",  0x000000, 0x020000, CRC(1eed12cb) SHA1(a978c0da0157ec0b8a7ba7cb6b6732fe89bac615) )
	ROM_LOAD16_BYTE( "lw1sp1.13k",  0x000001, 0x020000, CRC(535033bc) SHA1(2ab10bc9adebd93e385e02f211c5705863a49c9a) )

	ROM_REGION( 0x020000, "audiocpu", 0 ) /* Sound CPU (Banked) */
	ROM_LOAD( "lw1snd0.7j",  0x000000, 0x020000, CRC(cc83c6b6) SHA1(6ac6848f7ac72feb756c5d76de9ea7d26814cb6e) )

	ROM_REGION( 0x8000, "c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x400000, "c355spr", 0 ) /* Sprites */
	ROM_LOAD32_BYTE( "lw1obj0.3p",  0x000000, 0x80000, CRC(21485830) SHA1(e55a1f6df90c17b9c49e2b08c423b9be86996659) )
	ROM_LOAD32_BYTE( "lw1obj1.3w",  0x000001, 0x80000, CRC(d6437a82) SHA1(0aad3242828ed7dce65db75cad196c44ddd55ba8) )
	ROM_LOAD32_BYTE( "lw1obj2.3t",  0x000002, 0x80000, CRC(ceb6f516) SHA1(943dfe3bcf71a4885ce0ff33aaf81b2a49cf0b70) )
	ROM_LOAD32_BYTE( "lw1obj3.3y",  0x000003, 0x80000, CRC(5d32c7e9) SHA1(d684b68afaeacbbc734d55c1a970dc94f3459972) )
	ROM_LOAD32_BYTE( "lw1obj4.3s",  0x200000, 0x80000, CRC(0050458a) SHA1(605ea055b1934f83ca5ffaa532d0ae85ca56aefa) )
	ROM_LOAD32_BYTE( "lw1obj5.3x",  0x200001, 0x80000, CRC(cbc08f46) SHA1(2ece63a0544b39439255f0e0866a8675b3466643) )
	ROM_LOAD32_BYTE( "lw1obj6.3u",  0x200002, 0x80000, CRC(29740c88) SHA1(4078a5084256653a9c8ff72a7e2c652b0fbca425) )
	ROM_LOAD32_BYTE( "lw1obj7.3z",  0x200003, 0x80000, CRC(8cbd62b4) SHA1(c6605ae2629b34f036e440573b2bb68e26aced9b) )

	ROM_REGION( 0x400000, "c123tmap", 0 ) /* 8x8 Tiles */
	ROM_LOAD( "lw1chr0.11n", 0x000000, 0x80000, CRC(a0da15fd) SHA1(d772f712f0c150fdeb5aafb84f27a1495ad3492c) )
	ROM_LOAD( "lw1chr1.11p", 0x080000, 0x80000, CRC(89102445) SHA1(74d4e51a3540f72cc90fad04c7f0622930e0f854) )
	ROM_LOAD( "lw1chr2.11r", 0x100000, 0x80000, CRC(c749b778) SHA1(f1e30f5269eced00a09af40717634ec65e64f06b) )
	ROM_LOAD( "lw1chr3.11s", 0x180000, 0x80000, CRC(d76f9578) SHA1(07ac3331c0dead612e305f850aceab4f31fa89f8) )
	ROM_LOAD( "lw1chr4.9n",  0x200000, 0x80000, CRC(2f8ab45e) SHA1(2c7e5597ebf020aa88349c6d5a419ba0eee2db90) )
	ROM_LOAD( "lw1chr5.9p",  0x280000, 0x80000, CRC(c9acbe61) SHA1(3a42ec1fcdf994bd9a309fdb48da01af4afb0a3f) )

	ROM_REGION( 0x400000, "c169roz", 0 ) /* 16x16 Tiles */
	ROM_LOAD( "lw1roz1.23c", 0x080000, 0x80000, CRC(74e98793) SHA1(973dce838d9fbdf112429ab20edcdfbf07c3a9dc) )
	ROM_LOAD( "lw1roz2.23e", 0x000000, 0x80000, CRC(1ef46d82) SHA1(2234585875bccdff74fa3f66d2d25d4b419c3bfe) )
	ROM_LOAD( "lw1roz0.23b", 0x1c0000, 0x80000, CRC(a14079c9) SHA1(631f9b63488fd3194ebb41c9c7ec39d576b098d7) )

	ROM_REGION( 0x080000, "c123tmap:mask", 0 ) /* 8x8 shape */
	ROM_LOAD( "lw1sha0.7n",  0x000000, 0x80000, CRC(e3a65196) SHA1(c983a6d16dc10f0acd5f4ef5e148271f46ffbf28) )

	ROM_REGION( 0x80000, "c169roz:mask", 0 ) /* 16x16 shape */
	ROM_LOAD( "lw1rzs0.20z", 0x000000, 0x80000, CRC(a1071537) SHA1(bb8a97b82066d7838471e258d1c3c716ede7572c) )

	ROM_REGION16_BE( 0x200000, "data_rom", 0 ) /* Shared data roms */
	ROM_LOAD16_BYTE( "lw1dat0.13s",  0x000000, 0x80000, CRC(5d387d47) SHA1(e40ef52d1403fa630294d736f35a6924fa4e762e) )
	ROM_LOAD16_BYTE( "lw1dat1.13p",  0x000001, 0x80000, CRC(7ba94476) SHA1(cd4a964050e706f783450ace277e67586bf69e61) )
	ROM_LOAD16_BYTE( "lw1dat2.13r",  0x100000, 0x80000, CRC(eeba7c62) SHA1(6468518d3a5499b3f9a066488d83252cfc804d69) )
	ROM_LOAD16_BYTE( "lw1dat3.13n",  0x100001, 0x80000, CRC(ec3b36ea) SHA1(734549ada73a687377134051fa906b489ffd0dc4) )

	ROM_REGION16_BE( 0x200000, "c140", ROMREGION_ERASE00 ) /* Sound voices */
	ROM_LOAD16_BYTE( "lw1voi1.3m",  0x000000, 0x080000, CRC(b3e57993) SHA1(ff7071fc2e2c00f0cf819860c2a9be353474920a) )
	ROM_LOAD16_BYTE( "lw1voi2.3l",  0x100000, 0x080000, CRC(cd8b86a2) SHA1(54bbc91e995ea0c33874ce6fe5c3f014e173da07) )

	ROM_REGION( 0x100, "c45_road:clut", 0 ) /* prom for road colors */
	ROM_LOAD( "lw1ld8.10w", 0, 0x100, CRC(29058c73) SHA1(4916d6bdb7f78e6803698cab32d1586ea457dfc8) )

	ROM_REGION( 0x2000, "nvram", 0 ) /* default settings, including calibration - see notes with inputs */
	ROM_LOAD( "luckywld.nv",  0x000000, 0x2000, CRC(0c185d2a) SHA1(6a4876361731df423515a0f3b37ba1496d6b1964) )
ROM_END


void namcos2_state::init_assault()
{
	m_gametype = NAMCOS2_ASSAULT;
}

void namcos2_state::init_assaultj()
{
	m_gametype = NAMCOS2_ASSAULT_JP;
}

void namcos2_state::init_assaultp()
{
	m_gametype = NAMCOS2_ASSAULT_PLUS;
}

void namcos2_state::init_burnforc()
{
	m_gametype = NAMCOS2_BURNING_FORCE;
	m_update_to_line_before_posirq = true; // prevents bad line on horizon
}

void namcos2_state::init_cosmogng()
{
	m_gametype = NAMCOS2_COSMO_GANG;
}

void namcos2_state::init_dsaber()
{
	m_gametype = NAMCOS2_DRAGON_SABER;
}

void namcos2_state::init_dsaberj()
{
	m_gametype = NAMCOS2_DRAGON_SABER;
}

void namcos2_state::init_dirtfoxj()
{
	m_gametype = NAMCOS2_DIRT_FOX_JP;
}

void namcos2_state::init_finallap()
{
	m_gametype = NAMCOS2_FINAL_LAP;

	save_item(NAME(m_finallap_prot_count));
	m_finallap_prot_count = 0;
}

void namcos2_state::init_finalap2()
{
	m_gametype = NAMCOS2_FINAL_LAP_2;

	save_item(NAME(m_finallap_prot_count));
	m_finallap_prot_count = 0;
}

void namcos2_state::init_finalap3()
{
	m_gametype = NAMCOS2_FINAL_LAP_3;

	save_item(NAME(m_finallap_prot_count));
	m_finallap_prot_count = 0;
}

uint16_t namcos2_state::finalap3bl_prot_r()
{
	// code at 0x3f22 expects this to be 0x4d00 or it sets a value in NVRAM which prevents booting
	// address 0x180020 (0x10 in NVRAM) must also be 0x6b (machine ID code first byte) or the same will occur
	// for the 2nd issue we use a default NVRAM with this set
	return 0x4d00;
}

void namcos2_state::init_finalap3bl()
{
	init_finalap3();
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x3ffff0, 0x3ffff1, read16smo_delegate(*this, FUNC(namcos2_state::finalap3bl_prot_r)));
}

void namcos2_state::init_finehour()
{
	m_gametype = NAMCOS2_FINEST_HOUR;
}

void namcos2_state::init_fourtrax()
{
	m_gametype = NAMCOS2_FOUR_TRAX;
}

void namcos2_state::init_kyukaidk()
{
	m_gametype = NAMCOS2_KYUUKAI_DOUCHUUKI;
}

void namcos2_state::init_marvland()
{
	m_gametype = NAMCOS2_MARVEL_LAND;
}

void namcos2_state::init_metlhawk()
{
	/* unscramble sprites */
	uint8_t *data = memregion("sprite")->base();
	int size = memregion("sprite")->bytes();
	for (int i=0; i<size; i+=32*32)
	{
		for (int j=0; j<32*32; j+=32*4)
		{
			for (int k=0; k<32; k+=4)
			{
				uint8_t v;
				int a;

				a = i+j+k+32;
				v = data[a];
				data[a]   = data[a+3];
				data[a+3] = data[a+2];
				data[a+2] = data[a+1];
				data[a+1] = v;

				a += 32;
				v = data[a];
				data[a]   = data[a+2];
				data[a+2] = v;
				v = data[a+1];
				data[a+1] = data[a+3];
				data[a+3] = v;

				a += 32;
				data[a]   = data[a+1];
				data[a+1] = data[a+2];
				data[a+2] = data[a+3];
				data[a+3] = v;

				a = i+j+k;
				for (int l=0; l<4; l++)
				{
					v = data[a+l+32];
					data[a+l+32] = data[a+l+32*3];
					data[a+l+32*3] = v;
				} /* next l */
			} /* next k */
		} /* next j */
	} /* next i */

	m_gametype = NAMCOS2_METAL_HAWK;
}

void namcos2_state::init_mirninja()
{
	m_gametype = NAMCOS2_MIRAI_NINJA;
}

void namcos2_state::init_ordyne()
{
	m_gametype = NAMCOS2_ORDYNE;
}

void namcos2_state::init_phelios()
{
	m_gametype = NAMCOS2_PHELIOS;
}

void namcos2_state::init_rthun2()
{
	m_gametype = NAMCOS2_ROLLING_THUNDER_2;
}

void namcos2_state::init_rthun2j()
{
	m_gametype = NAMCOS2_ROLLING_THUNDER_2;
}

void namcos2_state::init_sgunner2()
{
	m_gametype = NAMCOS2_STEEL_GUNNER_2;
}

void namcos2_state::init_sws()
{
	m_gametype = NAMCOS2_SUPER_WSTADIUM;
}

void namcos2_state::init_sws92()
{
	m_gametype = NAMCOS2_SUPER_WSTADIUM_92;
}

void namcos2_state::init_sws92g()
{
	m_gametype = NAMCOS2_SUPER_WSTADIUM_92T;
}

void namcos2_state::init_sws93()
{
	m_gametype = NAMCOS2_SUPER_WSTADIUM_93;
}

void namcos2_state::init_suzuka8h()
{
	m_gametype = NAMCOS2_SUZUKA_8_HOURS;
}

void namcos2_state::init_suzuk8h2()
{
	m_gametype = NAMCOS2_SUZUKA_8_HOURS_2;
	m_update_to_line_before_posirq = true; // needed for tunnels, see 2nd attract demo
}

void namcos2_state::init_valkyrie()
{
	m_gametype = NAMCOS2_VALKYRIE;
}

void namcos2_state::init_gollygho()
{
	m_gametype = NAMCOS2_GOLLY_GHOST;
}

void namcos2_state::init_bubbletr()
{
	m_gametype = NAMCOS2_BUBBLE_TROUBLE;
}



void namcos2_state::init_luckywld()
{
	uint8_t *pData = (uint8_t *)memregion( "c169roz:mask" )->base();
	int i;
	for( i=0; i<32*0x4000; i++ )
	{ /* unscramble gfx mask */
		int code = pData[i];
		pData[i] = bitswap<8>(code, 0, 1, 2, 3, 4, 5, 6, 7);
	}
	m_gametype = NAMCOS2_LUCKY_AND_WILD;
}

/* Based on the dumped BIOS versions it looks like Namco changed the BIOS rom */
/* from sys2c65b to sys2c65c sometime between 1988 and 1990 as mirai ninja    */
/* and metal hawk have the B version and dragon saber has the C version       */

/*     YEAR, NAME,       PARENT,   MACHINE,  INPUT,    STATE,         INIT,          MONITOR,COMPANY, FULLNAME */
GAMEL( 1987, finallap,   0,        finallap, finallap, namcos2_state, init_finallap, ROT0,   "Namco", "Final Lap (Rev E)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE, layout_finallap )
GAMEL( 1987, finallapd,  finallap, finallap, finallap, namcos2_state, init_finallap, ROT0,   "Namco", "Final Lap (Rev D)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE, layout_finallap )
GAMEL( 1987, finallapc,  finallap, finallap, finallap, namcos2_state, init_finallap, ROT0,   "Namco", "Final Lap (Rev C)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE, layout_finallap )
GAMEL( 1987, finallapjc, finallap, finallap, finallap, namcos2_state, init_finallap, ROT0,   "Namco", "Final Lap (Japan, Rev C)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE, layout_finallap )
GAMEL( 1987, finallapjb, finallap, finallap, finallap, namcos2_state, init_finallap, ROT0,   "Namco", "Final Lap (Japan, Rev B)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE, layout_finallap )

GAME(  1988, assault,    0,        base2,    assault,  namcos2_state, init_assault,  ROT90,  "Namco", "Assault (Rev B)", MACHINE_SUPPORTS_SAVE )
GAME(  1988, assaultj,   assault,  base2,    assault,  namcos2_state, init_assaultj, ROT90,  "Namco", "Assault (Japan)", MACHINE_SUPPORTS_SAVE )
GAME(  1988, assaultp,   assault,  assaultp, assault,  namcos2_state, init_assaultp, ROT90,  "Namco", "Assault Plus (Japan)", MACHINE_SUPPORTS_SAVE )

GAME(  1988, metlhawk,   0,        metlhawk, metlhawk, namcos2_state, init_metlhawk, ROT90,  "Namco", "Metal Hawk (Rev C)", MACHINE_SUPPORTS_SAVE )
GAME(  1988, metlhawkj,  metlhawk, metlhawk, metlhawk, namcos2_state, init_metlhawk, ROT90,  "Namco", "Metal Hawk (Japan, Rev F)", MACHINE_SUPPORTS_SAVE )

GAME(  1988, ordyne,     0,        base,     base,     namcos2_state, init_ordyne,   ROT180, "Namco", "Ordyne (World)", MACHINE_SUPPORTS_SAVE )
GAME(  1988, ordyneje,   ordyne,   base,     base,     namcos2_state, init_ordyne,   ROT180, "Namco", "Ordyne (Japan, English Version)", MACHINE_SUPPORTS_SAVE )
GAME(  1988, ordynej,    ordyne,   base,     base,     namcos2_state, init_ordyne,   ROT180, "Namco", "Ordyne (Japan)", MACHINE_SUPPORTS_SAVE )

GAME(  1988, mirninja,   0,        base,     base,     namcos2_state, init_mirninja, ROT0,   "Namco", "Mirai Ninja (Japan, set 1)", MACHINE_SUPPORTS_SAVE )
GAME(  1988, mirninjaa,  mirninja, base,     base,     namcos2_state, init_mirninja, ROT0,   "Namco", "Mirai Ninja (Japan, set 2)", MACHINE_SUPPORTS_SAVE )

GAME(  1988, phelios,    0,        base2,    base,     namcos2_state, init_phelios,  ROT90,  "Namco", "Phelios", MACHINE_SUPPORTS_SAVE )
GAME(  1988, pheliosj,   phelios,  base2,    base,     namcos2_state, init_phelios,  ROT90,  "Namco", "Phelios (Japan)", MACHINE_SUPPORTS_SAVE )

GAME(  1989, dirtfoxj,   0,        base2,    dirtfox,  namcos2_state, init_dirtfoxj, ROT90,  "Namco", "Dirt Fox (Japan)", MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE )

GAMEL( 1989, fourtrax,   0,        base_fl,  fourtrax, namcos2_state, init_fourtrax, ROT0,   "Namco", "Four Trax (World)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE, layout_finallap )
GAMEL( 1989, fourtraxj,  fourtrax, base_fl,  fourtrax, namcos2_state, init_fourtrax, ROT0,   "Namco", "Four Trax (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE, layout_finallap )
GAMEL( 1989, fourtraxa,  fourtrax, base_fl,  fourtrax, namcos2_state, init_fourtrax, ROT0,   "Namco (Atari license?)", "Four Trax (US?, censored banners)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE, layout_finallap ) // boards using the ROM code FX4 were produced for Atari? there's no US region warning or Atari copyright tho, modded version of the World code

GAME(  1989, valkyrie,   0,        base3,    base,     namcos2_state, init_valkyrie, ROT90,  "Namco", "Valkyrie no Densetsu (Japan)", MACHINE_SUPPORTS_SAVE )

GAME(  1989, finehour,   0,        base2,    base,     namcos2_state, init_finehour, ROT0,   "Namco", "Finest Hour (Japan)", MACHINE_SUPPORTS_SAVE )

GAME(  1989, burnforc,   0,        base3,    base,     namcos2_state, init_burnforc, ROT0,   "Namco", "Burning Force (Japan, new version (Rev C))", MACHINE_SUPPORTS_SAVE )
GAME(  1989, burnforco,  burnforc, base3,    base,     namcos2_state, init_burnforc, ROT0,   "Namco", "Burning Force (Japan, old version)", MACHINE_SUPPORTS_SAVE )

GAME(  1989, marvland,   0,        base,     base,     namcos2_state, init_marvland, ROT0,   "Namco", "Marvel Land (Japan)", MACHINE_SUPPORTS_SAVE )
GAME(  1989, marvlandup, marvland, base,     base,     namcos2_state, init_marvland, ROT0,   "Namco", "Marvel Land (US, prototype)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // or World as no WDUD logo. Game is incomplete and ends abruptly at World 3-4

GAME(  1990, kyukaidk,   0,        base,     kyukaidk, namcos2_state, init_kyukaidk, ROT0,   "Namco", "Kyuukai Douchuuki (Japan, new version (Rev B))", MACHINE_SUPPORTS_SAVE )
GAME(  1990, kyukaidko,  kyukaidk, base,     kyukaidk, namcos2_state, init_kyukaidk, ROT0,   "Namco", "Kyuukai Douchuuki (Japan, old version)", MACHINE_SUPPORTS_SAVE )

GAME(  1990, dsaber,     0,        base3,    base,     namcos2_state, init_dsaber,   ROT90,  "Namco", "Dragon Saber (World, DO2)", MACHINE_SUPPORTS_SAVE )
GAME(  1990, dsabera,    dsaber,   base3,    base,     namcos2_state, init_dsaber,   ROT90,  "Namco", "Dragon Saber (World, older?)", MACHINE_SUPPORTS_SAVE )
GAME(  1990, dsaberj,    dsaber,   base3,    base,     namcos2_state, init_dsaberj,  ROT90,  "Namco", "Dragon Saber (Japan, Rev B)", MACHINE_SUPPORTS_SAVE )

GAMEL( 1990, finalap2,   0,        finalap2, finallap, namcos2_state, init_finalap2, ROT0,   "Namco", "Final Lap 2", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE, layout_finallap )
GAMEL( 1990, finalap2j,  finalap2, finalap2, finallap, namcos2_state, init_finalap2, ROT0,   "Namco", "Final Lap 2 (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE, layout_finallap )

GAME(  1990, gollygho,   0,        base,     gollygho, gollygho_state,init_gollygho, ROT180, "Namco", "Golly! Ghost!", MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )

GAME(  1990, rthun2,     0,        base3,    base,     namcos2_state, init_rthun2,   ROT0,   "Namco", "Rolling Thunder 2", MACHINE_SUPPORTS_SAVE )
GAME(  1990, rthun2j,    rthun2,   base3,    base,     namcos2_state, init_rthun2j,  ROT0,   "Namco", "Rolling Thunder 2 (Japan)", MACHINE_SUPPORTS_SAVE )

GAME(  1990, sgunner,    0,        sgunner,  sgunner,  namcos2_state, init_sgunner2, ROT0,   "Namco", "Steel Gunner (Rev B)", MACHINE_SUPPORTS_SAVE )
GAME(  1990, sgunnerj,   sgunner,  sgunner,  sgunner,  namcos2_state, init_sgunner2, ROT0,   "Namco", "Steel Gunner (Japan)", MACHINE_SUPPORTS_SAVE )

// The C68 I/O MCU contains a 1991 copyright, so anything after this point is potentially using that instead of C65, games before this point can't be using it

GAME(  1991, sgunner2,   0,        sgunner2, sgunner,  namcos2_state, init_sgunner2, ROT0,   "Namco", "Steel Gunner 2 (US)", MACHINE_SUPPORTS_SAVE )
GAME(  1991, sgunner2j,  sgunner2, sgunner2, sgunner,  namcos2_state, init_sgunner2, ROT0,   "Namco", "Steel Gunner 2 (Japan, Rev A)", MACHINE_SUPPORTS_SAVE )

GAME(  1991, cosmogng,   0,        base,     base,     namcos2_state, init_cosmogng, ROT90,  "Namco", "Cosmo Gang the Video (US)", MACHINE_SUPPORTS_SAVE )
GAME(  1991, cosmogngj,  cosmogng, base,     base,     namcos2_state, init_cosmogng, ROT90,  "Namco", "Cosmo Gang the Video (Japan)", MACHINE_SUPPORTS_SAVE )

GAME(  1992, bubbletr,   0,        base,     bubbletr, gollygho_state,init_bubbletr, ROT180, "Namco", "Bubble Trouble - Golly! Ghost! 2 (World, Rev B)", MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(  1992, bubbletrj,  bubbletr, base,     bubbletr, gollygho_state,init_bubbletr, ROT180, "Namco", "Bubble Trouble - Golly! Ghost! 2 (Japan, Rev C)", MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )

GAMEL( 1992, finalap3,   0,        finalap3, finalap3, namcos2_state, init_finalap3, ROT0,   "Namco", "Final Lap 3 (World, Rev C)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE, layout_finallap )
GAMEL( 1992, finalap3a,  finalap3, finalap3, finalap3, namcos2_state, init_finalap3, ROT0,   "Namco", "Final Lap 3 (World, set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE, layout_finallap )
GAMEL( 1992, finalap3j,  finalap3, finalap3, finalap3, namcos2_state, init_finalap3, ROT0,   "Namco", "Final Lap 3 (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE, layout_finallap )
GAMEL( 1992, finalap3jc, finalap3, finalap3, finalap3, namcos2_state, init_finalap3, ROT0,   "Namco", "Final Lap 3 (Japan, Rev C)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE, layout_finallap )
GAMEL( 1992, finalap3bl, finalap3, finalap3, finalap3, namcos2_state, init_finalap3bl,ROT0,  "Namco", "Final Lap 3 (bootleg)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE, layout_finallap )

GAME(  1992, luckywld,   0,        luckywld, luckywld, namcos2_state, init_luckywld, ROT0,   "Namco", "Lucky & Wild", MACHINE_SUPPORTS_SAVE )
GAME(  1992, luckywldj,  luckywld, luckywld, luckywld, namcos2_state, init_luckywld, ROT0,   "Namco", "Lucky & Wild (Japan)", MACHINE_SUPPORTS_SAVE )

GAME(  1992, suzuka8h,   0,        suzuka8h, suzuka,   namcos2_state, init_suzuka8h, ROT0,   "Namco", "Suzuka 8 Hours (World, Rev C)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE )
GAME(  1992, suzuka8hj,  suzuka8h, suzuka8h, suzuka,   namcos2_state, init_suzuka8h, ROT0,   "Namco", "Suzuka 8 Hours (Japan, Rev B)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE )

GAME(  1992, sws,        0,        base_c68, base,     namcos2_state, init_sws,      ROT0,   "Namco", "Super World Stadium (Japan)", MACHINE_SUPPORTS_SAVE )

GAME(  1992, sws92,      0,        base_c68, base,     namcos2_state, init_sws92,    ROT0,   "Namco", "Super World Stadium '92 (Japan)", MACHINE_SUPPORTS_SAVE )
GAME(  1992, sws92g,     sws92,    base_c68, base,     namcos2_state, init_sws92g,   ROT0,   "Namco", "Super World Stadium '92 Gekitouban (Japan)", MACHINE_SUPPORTS_SAVE )

GAME(  1993, suzuk8h2,   0,        suzuka8h, suzuka,   namcos2_state, init_suzuk8h2, ROT0,   "Namco", "Suzuka 8 Hours 2 (World, Rev B)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE )
GAME(  1993, suzuk8h2j,  suzuk8h2, suzuka8h, suzuka,   namcos2_state, init_suzuk8h2, ROT0,   "Namco", "Suzuka 8 Hours 2 (Japan, Rev B)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE )

GAME(  1993, sws93,      0,        base_c68, base,     namcos2_state, init_sws93,    ROT0,   "Namco", "Super World Stadium '93 (Japan)", MACHINE_SUPPORTS_SAVE ) // this appears to have a subtitle, what is it?
