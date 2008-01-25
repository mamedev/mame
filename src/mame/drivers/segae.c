/*

Sega System E
-- this is a modified version of the sms.c from HazeMD
 it really needs some improvements


 Sega System 'E' is a piece of hardware used for a couple of Arcade Games
 produced by Sega in the Mid 80's, its roughly based on their Sega Master System
 home console unit, using the same '315-5124' VDP (actually in this case 2 of
 them)

 An interesting feature of the system is that the CPU is contained on the ROM
 board, the MAIN System E board contains the Graphics processor, this opens the
 possibility for using processors other than the Standard Z80 to run the main
 game code on; several games have an encrypted Z80 module instead.

 Also interesting is each VDP has double the Video RAM found on the SMS console
 this is banked through Port Writes, the System also allows for the Video RAM
 to be written directly, bypassing the usual procedure of writing to it via the
 '315-5124' data port, it can not however be read directly, the same area used
 for writing is used to access banked ROMs when reading

 Pretty much everything on this hardware is done through port accesses, the
 main memory map consists of simply ROM, BANKED ROM / BANKED RAM, RAM

********************************************************************************

    ROMs + CPU Board (32kb ROMs)

    IC 07 (Good)    IC 05 (Good)
    IC 04 (Good)    IC 03 (Good)
    IC 02 (Good)

    (834-5803) MAIN Board (8kb RAMs)

    IC 49 (Good)    IC 55 (Good)    System RAM (0xc000 - 0xffff)
    IC 03 (Good)    IC 07 (Good)    Front Layer VRAM (Bank 1)   Port F7 -0------
    IC 04 (Good)    IC 08 (Good)    Front Layer VRAM (Bank 2)   Port F7 -1------
    IC 01 (Good)    IC 05 (Good)    Back Layer VRAM (Bank 1)    Port F7 0-------
    IC 02 (Good)    IC 06 (Good)    Back Layer VRAM (Bank 2)    Port F7 1-------
    (or at least this is how it appears from HangOnJr's RAMs Test)

    2x (315-5124)'s here too, these are the VDP chips

    PORTS (to be completed)

    0xba - 0xbb r/w     Back Layer VDP
    0xbe - 0xbf r/w     Front Layer VDP

    0xf7 w/o            Banking Controls

    0xe0 r/o            Inputs (Coins, Start Btns)
    0xe1 r/o            Controls (Transformer)

    0xf2 - 0xf3 r/o     Dipswitches

    0xf8 r/o            Analog Input (Hang On Jr)

    0x7e r/o            V Counter (vertical beam pos in scanlines)
    0x7f r/o            H Counter (horizontal beam pos in 'pixel clock cycles')

********************************************************************************

Sega System E Hardware Overview
Sega, 1985-1988

This PCB is essentially a Sega Master System home console unit, but using
two '315-5124' VDPs and extra RAM.
The CPU is located on a plug-in board that also holds all of the EPROMs.

The games that run on this hardware include....
Hang-On Jr.             1985
Transformer/Astro Flash 1986
Riddle of Pythagoras    1986
Opa Opa                 1987
Fantasy Zone 2          1988
Tetris                  1988

PCB Layout
----------
834-5803 (C)Sega 1985
|-----------------------------------------------------|
|         D4168      D4168      D4168       D4168     |
|                                                     |
|         D4168      D4168      D4168       D4168     |
|                                                     |
|                                                     |
|    SW1                                              |
|CN1                                                  |
|    SW2                                              |
|                                                     |
|   LED            |---|             |---|            |
|                  | 3 |             | 3 |            |
|                  | 1 |             | 1 |            |
|                  | 5 |             | 5 |            |
|                  | | |             | | |         CN3|
|                  | 5 |             | 5 |     8255   |
|CN4               | 1 |             | 1 |            |
|                  | 2 |             | 2 |            |
|                  | 4 |             | 4 |            |
|                  |---|             |---|            |
|               |--------ROM-BOARD-(above)---------|  |
|               |                                  |  |
|               |CN2                   10.7386MHz  |  |
|               |         D4168                    |  |
|  VOL          |         D4168                    |  |
| LA4460        |----------------------------------|  |
|-----------------------------------------------------|
Notes:
      315-5124 VDP clock - 10.7386MHz
      SN76496 clock      - 3.579533MHz [10.7386/3]
      D4168              - 8k x8 SRAM
      VSync              - 60Hz
      HSync              - 15.58kHz
      CN1                - Connector used for standard controls
      CN2                - connector for CPU/ROM PCB
      CN3                - Connector used for special controls (via a small plug-in interface PCB)
      CN4                - Connector for power

ROM Daughterboard
-----------------
834-6592-01
|--------------------------------------------|
|                                            |
|    |---|                                   |
|C   |   |                           IC6     |
|N   |Z80|                                   |
|2   |   |                                   |
|    |   |   IC2   IC3   IC4   IC5        IC7|
|    |---|                                   |
|     IC1             PAD1 PAD2     PAD3 PAD4|
|--------------------------------------------|
Notes:
       IC1: Z80 clock - 5.3693MHz [10.7386/2]
            On some games this is replaced with a NEC MC-8123 Encrypted CPU Module.
            The clock speed is the same. The MC-8123 contains a Z80 core, plus a RAM chip
            and battery. When the battery dies, the program can no longer be decrypted
            and the PCB does not boot up at all. The battery can not be changed because the
            MC-8123 is sealed, so there is no way to access it.

 IC2 - IC5: 27C256 EPROM (DIP28)

       IC6: 74LS139

       IC7: 27C256 EPROM (DIP28)

    PAD1-4: These are jumper pads used to configure the ROM board for use with the
            Z80 or with the MC8123 CPU.
            PAD1 - Ties Z80 pin 24 (WAIT) to pin1 of the EPROMs at IC2, 3, 4 & 5
            PAD2 - Ties CN2 pin B21 to pin1 of the EPROMs at IC2, 3, 4 & 5
            PAD3 - Ties CN2 pin B21 to pin 2 of the 74LS139 @ IC6
            PAD4 - Ties Z80 pin 24 (WAIT) to pin 2 of the 74LS139 @ IC6

            The pads are configured like this..... (U=Upper, L=Lower)

                                                 |----|      |----|
                                                 |IC6 |      |IC7 |
                                                 |  12|------|22  |
                                                 |    |      |    |
                       IC2   IC3    IC4   IC5    |   1|------|27  |
                       PIN1  PIN1   PIN1  PIN1   |   2|--|   |    |
                        O-----O--+---O------O    |----|  |   |----|
                                 |                       |
                                 |         |----|        |
                              O--+----O    |    O    |---O
            CN2    Z80      PAD1U   PAD2U  |  PAD3U  | PAD4U
            B21    PIN24    PAD1L   PAD2L  |  PAD3L  | PAD4L
             O       O--4.7k--O       O----|    O----|   O
             |                |       |                  |
             |                |-------|------------------|
             |                        |
             |------------------------|

            When using a regular Z80B (and thus, unencrypted code):
            PAD1 - Open
            PAD2 - Open
            PAD3 - Shorted
            PAD4 - Open

            When using an encrypted CPU module (MC-8123):
            PAD1 - Open
            PAD2 - Shorted
            PAD3 - Open
            PAD4 - Open
            Additionally, a wire must be tied from CN2 pin B22 to the side
            of PAD3 nearest IC6 (i.e. PAD3U).

ROMs:
-----

Game                     IC2         IC3         IC4         IC5         IC7
---------------------------------------------------------------------------------
Hang-On Jr.              EPR-?       EPR-?       EPR-?       EPR-?       EPR-?     Hello, Sega Part Numbers....!?
Transformer              EPR-7350    EPR-?       EPR-7348    EPR-7347    EPR-?     Ditto
           /Astro Flash  EPR-7350    EPR-7349    EPR-7348    EPR-7347    EPR-7723
Riddle of Pythagoras     EPR-10422   EPR-10423   EPR-10424   EPR-10425   EPR-10426
Opa Opa                  EPR-11220   EPR-11221   EPR-11222   EPR-11223   EPR-11224
Fantasy Zone 2           EPR-11412   EPR-11413   EPR-11414   EPR-11415   EPR-11416
Tetris                   -           -           EPR-12211   EPR-12212   EPR-12213

A System E PCB can run all of the games simply by swapping the EPROMs plus CPU.
Well, in theory anyway. To run the not-encrypted games, just swap EPROMs and they will work.

To run the encrypted games, use a double sized EPROM in IC7 (i.e. a 27C512)
and program the decrypted opcodes to the lower half and the decrypted data to the upper half,
then connect the highest address pin of the EPROM (A15 pin 1) to the M1 pin on the Z80.
This method has been tested and does not actually work. An update on this may follow....


System E PCB Pinout
-------------------

CN1
---

+12V             1A  1B  Coin switch 1
Coin switch 2    2A  2B  Test switch
Service switch   3A  3B
                 4A  4B  1P start
2P start         5A  5B  1P up
1P down          6A  6B  1P left
1P right         7A  7B  1P button 1
1P button 2      8A  8B
                 9A  9B  2P up
2P down          10A 10B 2P left
2P RIGHT         11A 11B 2P button 1
2P button 2      12A 12B
                 13A 13B Video RED
                 14A 14B Video GREEN
                 15A 15B Video BLUE
                 16A 16B Video SYNC
                 17A 17B
                 18A 18B
Speaker [+]      19A 19B
Speaker [-]      20A 20B
Coin counter GND 21A 21B
GND              22A 22B Coin counter 1
                 23A 23B Coin counter 2
                 24A 24B
                 25A 25B
CN4
---

+5V  1A 1B +5V
+5V  2A 2B +5V
     3A 3B
GND  4A 4B GND
GND  5A 5B GND
+12V 6A 6B +12V
+12V 7A 7B +12V
GND  8A 8B GND


 Game Notes:
 Riddle of Pythagoras is interesting, it looks like Sega might have planned it
 as a two player game, there is prelimiary code for 2 player support which
 never gets executed, see code around 0x0E95.  Theres also quite a bit of
 pointless code here and there.  Some Interesting Memory Locations

 C000 : level - value (00-0x32)
 C001 : level - display (00-0x50, BCD coded)
 C003 : credits (00-0x99, BCD coded)
 C005 : DSWA put here (coinage, left and right nibbles for left and right slot
        - freeplay when 0x0f or 0xf0)
 C006 : DSWB put here
  bits 0 and 1 : lives ("02", "03", "04", "98")
  bit 3 : difficulty
  bits 5 and 6 : bonus lives ("50K, 100K, 200K, 500K", "100K, 200K, 500K", "100K,
                               200K, 500K, 99999999", "none")
 C009 : lives (for player 1)
 C00A : lives (for player 2)
 C00B : bonus lives counter

 E20B-E20E : score (00000000-0x99999999, BCD coded)
 E215-E218 : hi-score (00000000-0x99999999, BCD coded)

 E543 : bit 0 : ON = player 1 one still has lives
        bit 1 : ON = player 2 one still has lives
        bit 2 : ON = player 1 is the current player - OFF = player 2 is the
         current player

 E572 : table with L. slot infos (5 bytes wide)
 E577 : table with R. slot infos (5 bytes wide)

Known issues:

sometimes hangonjr has corrupt gfx when you start a game, I don't know why

todo:

tidy up, add save states, clean up so we can use it with hazemd again
add emulation of vdp bugs, use mame's new screen timing system instead
covert megatech / megaplay drivers to use new code etc. etc.

*/

#include "driver.h"
#include "deprecat.h"
#include "sound/sn76496.h"
#include "machine/mc8123.h"
#include "machine/segacrpt.h"
#include "segae.h"


//static UINT8* sms_rom;
static UINT8* sms_mainram;
//static UINT8* smsgg_backupram;
static TIMER_CALLBACK( sms_scanline_timer_callback );
static struct sms_vdp *vdp2;
static struct sms_vdp *vdp1;

static struct sms_vdp *md_sms_vdp;

/* All Accesses to VRAM go through here for safety */
#define SMS_VDP_VRAM(address) chip->vram[(address)&0x3fff]

#ifdef UNUSED_FUNCTION
static ADDRESS_MAP_START( sms_readmem, ADDRESS_SPACE_PROGRAM, 8 )
//  AM_RANGE(0x0000 , 0xbfff) AM_READ(MRA8_ROM)
//  AM_RANGE(0xc000 , 0xdfff) AM_READ(MRA8_RAM) AM_MIRROR(0x2000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sms_writemem, ADDRESS_SPACE_PROGRAM, 8 )
//  AM_RANGE(0x0000 , 0xbfff) AM_WRITE(MWA8_ROM)
//  AM_RANGE(0xc000 , 0xdfff) AM_WRITE(MWA8_RAM) AM_MIRROR(0x2000)
ADDRESS_MAP_END
#endif

/* we have to fill in the ROM addresses for systeme due to the encrypted games */
static ADDRESS_MAP_START( systeme_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)				/* Fixed ROM */
	AM_RANGE(0x8000, 0xbfff) AM_READ(MRA8_BANK1)				/* Banked ROM */

//  AM_RANGE(0x0000 , 0xbfff) AM_READ(MRA8_ROM)
//  AM_RANGE(0xc000 , 0xdfff) AM_READ(MRA8_RAM) AM_MIRROR(0x2000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( systeme_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(MWA8_ROM)				/* Fixed ROM */

//  AM_RANGE(0x0000 , 0xbfff) AM_WRITE(MWA8_ROM)
//  AM_RANGE(0xc000 , 0xdfff) AM_WRITE(MWA8_RAM) AM_MIRROR(0x2000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sms_readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
ADDRESS_MAP_END

static ADDRESS_MAP_START( sms_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
ADDRESS_MAP_END

/* Precalculated tables for H/V counters.  Note the position the counter 'jumps' is marked with with
   an empty comment */
static const UINT8 hc_256[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,    0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,    0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,    0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,    0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,    0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,    0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,    0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,    0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,    0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,    0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,    0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9,    0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9,    0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9,    0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,/**/0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
    0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3,    0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,
    0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3,    0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9,
    0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3,    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9,
    0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3,    0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9,
    0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3,    0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3,    0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
    0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};


static const UINT8 vc_ntsc_192[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,    0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,    0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,    0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a,    0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,    0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a,    0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,    0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,    0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a,    0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a,    0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,    0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,    0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca,    0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,/**/0xd5, 0xd6, 0xd7, 0xd8, 0xd9,
    0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4,    0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4,    0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
    0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

static const UINT8 vc_ntsc_224[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,    0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,    0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,    0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a,    0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,    0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a,    0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,    0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,    0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a,    0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a,    0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,    0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,    0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca,    0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,    0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,/**/0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4,    0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
    0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

static const UINT8 vc_ntsc_240[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05
};



static const UINT8 vc_pal_192[] =
{
    0x00, 0x01, 0x02,    0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12,    0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22,    0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32,    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42,    0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52,    0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62,    0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72,    0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82,    0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92,    0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2,    0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2,    0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2,    0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2,    0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2,    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2,/**/0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6,
    0xc7, 0xc8, 0xc9,    0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    0xd7, 0xd8, 0xd9,    0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
    0xe7, 0xe8, 0xe9,    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
    0xf7, 0xf8, 0xf9,    0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};


static const UINT8 vc_pal_224[] =
{
    0x00, 0x01, 0x02,    0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12,    0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22,    0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32,    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42,    0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52,    0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62,    0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72,    0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82,    0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92,    0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2,    0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2,    0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2,    0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2,    0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2,    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2,    0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    0x00, 0x01, 0x02,/**/0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    0xd7, 0xd8, 0xd9,    0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
    0xe7, 0xe8, 0xe9,    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
    0xf7, 0xf8, 0xf9,    0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

static const UINT8 vc_pal_240[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,    0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,    0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,    0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a,    0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,    0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a,    0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,    0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,    0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a,    0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a,    0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,    0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,    0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca,    0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,    0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,    0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa,    0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,/**/0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1,    0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
    0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1,    0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
    0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

static const struct
{
	UINT8 sms2_name[40];
	int sms2_valid;
	int sms2_height;
	int sms2_tilemap_height;
	const UINT8* sms_vcounter_table;
	const UINT8* sms_hcounter_table;

} sms_mode_table[] =
{
	/* NTSC Modes */
	{ "Graphic 1 (NTSC)",         0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Text (NTSC)",              0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Graphic 2 (NTSC)",         0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 1+2 (NTSC)" ,         0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Multicolor (NTSC)",        0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 1+3 (NTSC)",          0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 2+3 (NTSC)",          0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 1+2+3 (NTSC)",        0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 4 (NTSC)",            1, 192, 224, vc_ntsc_192, hc_256 },
	{ "Invalid Text (NTSC)",      0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 4 (NTSC)",            1, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 4 (224-line) (NTSC)", 1, 224, 256, vc_ntsc_224, hc_256 },
	{ "Mode 4 (NTSC)",            1, 192, 224, vc_ntsc_192, hc_256 },
	{ "Invalid Text (NTSC)",      0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 4 (240-line) (NTSC)", 1, 240, 256, vc_ntsc_240, hc_256 },
	{ "Mode 4 (NTSC)",            1, 192, 244, vc_ntsc_192, hc_256 },

	/* Pal Modes (different Vcounter) */
	{ "Graphic 1 (PAL)",         0, 192, 224, vc_pal_192, hc_256 },
	{ "Text (PAL)",              0, 192, 224, vc_pal_192, hc_256 },
	{ "Graphic 2 (PAL)",         0, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 1+2 (PAL)" ,         0, 192, 224, vc_pal_192, hc_256 },
	{ "Multicolor (PAL)",        0, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 1+3 (PAL)",          0, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 2+3 (PAL)",          0, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 1+2+3 (PAL)",        0, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 4 (PAL)",            1, 192, 224, vc_pal_192, hc_256 },
	{ "Invalid Text (PAL)",      0, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 4 (PAL)",            1, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 4 (224-line) (PAL)", 1, 224, 256, vc_pal_224, hc_256 },
	{ "Mode 4 (PAL)",            1, 192, 224, vc_pal_192, hc_256 },
	{ "Invalid Text (PAL)",      0, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 4 (240-line) (PAL)", 1, 240, 256, vc_pal_240, hc_256 },
	{ "Mode 4 (PAL)",            1, 192, 244, vc_pal_192, hc_256 }
};

enum
{
	SMS_VDP = 0,  // SMS1 VDP
	SMS2_VDP = 1, // SMS2 VDP, or Game Gear VDP running in SMS2 Mode
	GG_VDP = 2,   // Game Gear VDP running in Game Gear Mode
	GEN_VDP = 3   // Genesis VDP running in SMS2 Mode
};

static int sms_vdp_null_irq_callback(int status)
{
	return -1;
}

static int sms_vdp_cpu0_irq_callback(int status)
{
	if (status==1)
		cpunum_set_input_line(Machine, 0,0,HOLD_LINE);
	else
		cpunum_set_input_line(Machine, 0,0,CLEAR_LINE);

	return 0;
}

static int sms_vdp_cpu1_irq_callback(int status)
{
	if (status==1)
		cpunum_set_input_line(Machine, 1,0,HOLD_LINE);
	else
		cpunum_set_input_line(Machine, 1,0,CLEAR_LINE);

	return 0;
}


static int sms_vdp_cpu2_irq_callback(int status)
{
	if (status==1)
		cpunum_set_input_line(Machine, 2,0,HOLD_LINE);
	else
		cpunum_set_input_line(Machine, 2,0,CLEAR_LINE);

	return 0;
}




struct sms_vdp
{
	UINT8 chip_id;

	UINT8  cmd_pend;
	UINT8  cmd_part1;
	UINT8  cmd_part2;
	UINT16 addr_reg;
	UINT8  cmd_reg;
	UINT8  regs[0x10];
	UINT8  readbuf;
	UINT8* vram;
	UINT8* cram;
	UINT8  writemode;
	mame_bitmap* r_bitmap;
	UINT8* tile_renderline;
	UINT8* sprite_renderline;

	UINT8 sprite_collision;
	UINT8 sprite_overflow;

	UINT8  yscroll;
	UINT8  hint_counter;

	UINT8 frame_irq_pending;
	UINT8 line_irq_pending;

	UINT8 vdp_type;

	UINT8 gg_cram_latch; // gamegear specific.

	/* below are MAME specific, to make things easier */
	UINT8 screen_mode;
	UINT8 is_pal;
	int sms_scanline_counter;
	int sms_total_scanlines;
	int sms_framerate;
	emu_timer* sms_scanline_timer;
	UINT16* cram_mamecolours; // for use on RGB_DIRECT screen
	int	 (*set_irq)(int state);

};



static void *start_vdp(running_machine *machine, int type)
{
	struct sms_vdp *chip;

	chip = auto_malloc(sizeof(*chip));
	memset(chip, 0, sizeof(*chip));

	chip->vdp_type = type;

	chip->set_irq = sms_vdp_null_irq_callback;

	chip->cmd_pend = 0;
	chip->cmd_part1 = 0;
	chip->cmd_part2 = 0;
	chip->addr_reg = 0;
	chip->cmd_reg = 0;

	chip->regs[0x0] = 0x06; // mode 4
	chip->regs[0x1] = 0x18; // mode 4
	chip->regs[0x2] = 0;
	chip->regs[0x3] = 0;
	chip->regs[0x4] = 0;
	chip->regs[0x5] = 0;
	chip->regs[0x6] = 0;
	chip->regs[0x7] = 0;
	chip->regs[0x8] = 0;
	chip->regs[0x9] = 0;
	chip->regs[0xa] = 0;
	/* b-f don't matter */
	chip->readbuf = 0;
	chip->vram = auto_malloc(0x4000);
	memset(chip->vram,0x00,0x4000);

	//printf("%d\n", (*chip->set_irq)(200));

	if (chip->vdp_type==GG_VDP)
	{
		chip->cram = auto_malloc(0x0040);
		memset(chip->cram,0x00,0x0040);
		chip->cram_mamecolours = auto_malloc(0x0080);
		memset(chip->cram_mamecolours,0x00,0x0080);
		chip->gg_cram_latch = 0;
	}
	else
	{
		chip->cram = auto_malloc(0x0020);
		memset(chip->cram,0x00,0x0020);
		chip->cram_mamecolours = auto_malloc(0x0040);
		memset(chip->cram_mamecolours,0x00,0x0040);
	}

	chip->tile_renderline = auto_malloc(256+8);
	memset(chip->tile_renderline,0x00,256+8);

	chip->sprite_renderline = auto_malloc(256+32);
	memset(chip->sprite_renderline,0x00,256+32);

	chip->writemode = 0;
	chip->r_bitmap = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,machine->screen[0].format);

	chip->sms_scanline_timer = timer_alloc(sms_scanline_timer_callback, chip);

	return chip;
}

/* stop timer and clear ram.. used on megatech when we switch between genesis and sms mode */
void segae_md_sms_stop_scanline_timer(void)
{
	timer_adjust(md_sms_vdp->sms_scanline_timer,  attotime_never, 0, attotime_never);
	memset(md_sms_vdp->vram,0x00,0x4000);
}


#ifdef UNUSED_FUNCTION
static READ8_HANDLER( z80_unmapped_r )
{
	printf("unmapped z80 read %04x\n",offset);
	return 0;
}

static WRITE8_HANDLER( z80_unmapped_w )
{
	printf("unmapped z80 write %04x\n",offset);
}
#endif

static UINT8 vcounter_r(struct sms_vdp *chip)
{
//  return vc_pal_224[sms_scanline_counter%(sizeof vc_pal_224)];
	UINT8 retvalue;
	int scanline = chip->sms_scanline_counter%chip->sms_total_scanlines;

	retvalue = sms_mode_table[chip->screen_mode].sms_vcounter_table[scanline];

	return retvalue;
	//printf("retvalue %d\n";
}


static UINT8 vdp_data_r(struct sms_vdp *chip)
{
	UINT8 retdata = chip->readbuf;
	chip->readbuf = SMS_VDP_VRAM(chip->addr_reg);
	chip->addr_reg++; chip->addr_reg&=0x3fff;
	return retdata;
}

static void vdp_data_w(UINT8 data, struct sms_vdp* chip)
{
	/* data writes clear the pending flag */
	chip->cmd_pend = 0;

	if (chip->writemode==0)
	{ /* Write to VRAM */
		SMS_VDP_VRAM(chip->addr_reg)=data;
		chip->addr_reg++; chip->addr_reg&=0x3fff;
		chip->readbuf = data; // quirk of the VDP
	}
	else if (chip->writemode==1)
	{
		if (chip->vdp_type==GG_VDP)
		{
			if (!(chip->addr_reg&1))
			{ /* Even address, value latched */
				chip->gg_cram_latch = data;
			}
			else
			{
				chip->cram[(chip->addr_reg&0x3e)+1]=data;
				chip->cram[(chip->addr_reg&0x3e)+0]=chip->gg_cram_latch;

				/* Set Colour */
				{
					UINT16 palword;
					UINT8 r,g,b;

					palword = ((chip->cram[(chip->addr_reg&0x3e)+1])<<8)|(chip->cram[(chip->addr_reg&0x3e)+0]);

					//printf("addr %04x palword %04x\n", chip->addr_reg&0x3f, palword);

					r = (palword & 0x000f)>>0;
					g = (palword & 0x00f0)>>4;
					b = (palword & 0x0f00)>>8;
					palette_set_color_rgb(Machine,(chip->addr_reg&0x3e)/2, pal4bit(r), pal4bit(g), pal4bit(b));
					chip->cram_mamecolours[(chip->addr_reg&0x3e)/2]=(b<<1)|(g<<6)|(r<<11);
				}
			}
		}
		else
		{
			chip->cram[chip->addr_reg&0x1f]=data;

			/* Set Colour */
			{
				UINT8 r,g,b;
				r = (data & 0x03)>>0;
				g = (data & 0x0c)>>2;
				b = (data & 0x30)>>4;
				palette_set_color_rgb(Machine,chip->addr_reg&0x1f, pal2bit(r), pal2bit(g), pal2bit(b));
				chip->cram_mamecolours[chip->addr_reg&0x1f]=(b<<3)|(g<<8)|(r<<13);
			}

		}

		chip->addr_reg++; chip->addr_reg&=0x3fff;
		chip->readbuf = data; // quirk of the VDP

	}

}

static UINT8 vdp_ctrl_r(struct sms_vdp *chip)
{
	UINT8 retvalue;

	retvalue = (chip->frame_irq_pending<<7) |
	   	       (chip->sprite_overflow<<6) |
	           (chip->sprite_collision<<5);

	chip->cmd_pend = 0;
	chip->frame_irq_pending = 0;
	chip->line_irq_pending = 0;
	chip->sprite_collision = 0;
	chip->sprite_overflow = 0;

	(chip->set_irq)(0); // clear IRQ;


	return retvalue;
}

/* check me */
static void vdp_update_code_addr_regs(struct sms_vdp *chip)
{
	chip->addr_reg = ((chip->cmd_part2&0x3f)<<8) | chip->cmd_part1;
	chip->cmd_reg = (chip->cmd_part2&0xc0)>>6;
}

static void vdp_set_register(struct sms_vdp *chip)
{
	UINT8 reg = chip->cmd_part2&0x0f;
	chip->regs[reg] = chip->cmd_part1;

	//if(reg==0) printf("setting reg 0 to %02x\n",chip->cmd_part1);

	//if (reg>0xa) printf("Invalid register write to register %01x\n",reg);

	if(reg==1)
	{
		if ((chip->regs[0x1]&0x20) && chip->frame_irq_pending)
		{
			(chip->set_irq)(1); // set IRQ;
		}
		else
		{
			(chip->set_irq)(0); // clear IRQ;
		}
	}

	if(reg==0)
	{
		if ((chip->regs[0x0]&0x10) && chip->line_irq_pending)
		{
			(chip->set_irq)(1); // set IRQ;
		}
		else
		{
			(chip->set_irq)(0); // clear IRQ;
		}
	}


//  printf("VDP: setting register %01x to %02x\n",reg, chip->cmd_part1);
}

static void vdp_ctrl_w(UINT8 data, struct sms_vdp *chip)
{
	if (chip->cmd_pend)
	{ /* Part 2 of a command word write */
		chip->cmd_pend = 0;
		chip->cmd_part2 = data;
		vdp_update_code_addr_regs(chip);

		switch (chip->cmd_reg)
		{
			case 0x0: /* VRAM read mode */
				chip->readbuf = SMS_VDP_VRAM(chip->addr_reg);
				chip->addr_reg++; chip->addr_reg&=0x3fff;
				chip->writemode = 0;
				break;

			case 0x1: /* VRAM write mode */
				chip->writemode = 0;
				break;

			case 0x2: /* REG setting */
				vdp_set_register(chip);
				chip->writemode = 0;
				break;

			case 0x3: /* CRAM write mode */
				chip->writemode = 1;
				break;
		}
	}
	else
	{ /* Part 1 of a command word write */
		chip->cmd_pend = 1;
		chip->cmd_part1 = data;
		vdp_update_code_addr_regs(chip);
	}
}

/* for the Genesis */

READ8_HANDLER( md_sms_vdp_vcounter_r )
{
	return vcounter_r(md_sms_vdp);
}

READ8_HANDLER( md_sms_vdp_data_r )
{
	return vdp_data_r(md_sms_vdp);
}

WRITE8_HANDLER( md_sms_vdp_data_w )
{
	vdp_data_w(data, md_sms_vdp);
}

READ8_HANDLER( md_sms_vdp_ctrl_r )
{
	return vdp_ctrl_r(md_sms_vdp);
}

WRITE8_HANDLER( md_sms_vdp_ctrl_w )
{
	vdp_ctrl_w(data, md_sms_vdp);
}


/* Read / Write Handlers - call other functions */

READ8_HANDLER( sms_vcounter_r )
{
	return vcounter_r(vdp1);
}

READ8_HANDLER( sms_vdp_data_r )
{
	return vdp_data_r(vdp1);
}

WRITE8_HANDLER( sms_vdp_data_w )
{
	vdp_data_w(data, vdp1);
}

READ8_HANDLER( sms_vdp_ctrl_r )
{
	return vdp_ctrl_r(vdp1);
}

WRITE8_HANDLER( sms_vdp_ctrl_w )
{
	vdp_ctrl_w(data, vdp1);
}

WRITE8_HANDLER( sms_sn76496_w )
{
	SN76496_0_w(0, data & 0xff);
}

static void draw_tile_line(int drawxpos, int tileline, UINT16 tiledata, UINT8* linebuf, struct sms_vdp* chip)
{
	int xx;
	UINT32 gfxdata;
	UINT16 gfx_base = (tiledata & 0x01ff)<<5;
	UINT8  flipx = (tiledata & 0x0200)>>9;
	UINT8  flipy = (tiledata & 0x0400)>>10;
	UINT8  pal   = (tiledata & 0x0800)>>11;
	UINT8  pri   = (tiledata & 0x1000)>>12;

	if (flipy)
	{
		gfx_base+=(7-tileline)*4;
	}
	else
	{
		gfx_base+=tileline*4;
	}

	gfxdata = (SMS_VDP_VRAM(gfx_base)<<24)|(SMS_VDP_VRAM(gfx_base+1)<<16)|(SMS_VDP_VRAM(gfx_base+2)<<8)|(SMS_VDP_VRAM(gfx_base+3)<<0);

	for (xx=0;xx<8;xx++)
	{
		UINT8 pixel;

		if (flipx)
		{
			pixel = (( (gfxdata>>(0+xx)  ) &0x00000001)<<3)|
		            (( (gfxdata>>(8+xx)  ) &0x00000001)<<2)|
		            (( (gfxdata>>(16+xx) ) &0x00000001)<<1)|
		            (( (gfxdata>>(24+xx) ) &0x00000001)<<0);
		}
		else
		{
			pixel = (( (gfxdata>>(7-xx)  ) &0x00000001)<<3)|
			        (( (gfxdata>>(15-xx) ) &0x00000001)<<2)|
			        (( (gfxdata>>(23-xx) ) &0x00000001)<<1)|
			        (( (gfxdata>>(31-xx) ) &0x00000001)<<0);
		}

		pixel += pal*0x10;

		if (!pri) linebuf[drawxpos+xx] = pixel;
		else
		{
			if (pixel&0xf)
				linebuf[drawxpos+xx] = pixel|0x80;
			else
				linebuf[drawxpos+xx] = pixel;

		}
	}
}

static void sms_render_spriteline(int scanline, struct sms_vdp* chip)
{
	int spritenum;
	int height = 8;
	int width = 8;
	int max_sprites = 8;
	int visible_line = 0;

	UINT16 table_base = (chip->regs[0x5]&0x7e) << 7;
	UINT8 pattern_bit = (chip->regs[0x6]&0x04) >> 2; // high bit of the tile # (because spriteram can only contain an 8-bit tile #)


	memset(chip->sprite_renderline, 0, 256+32);

	for (spritenum = 0;spritenum<64;spritenum++)
	{
		int xpos,ypos,num;
		/*
        00: yyyyyyyyyyyyyyyy
        10: yyyyyyyyyyyyyyyy
        20: yyyyyyyyyyyyyyyy
        30: yyyyyyyyyyyyyyyy
        40: ????????????????
        50: ????????????????
        60: ????????????????
        70: ????????????????
        80: xnxnxnxnxnxnxnxn
        90: xnxnxnxnxnxnxnxn
        A0: xnxnxnxnxnxnxnxn
        B0: xnxnxnxnxnxnxnxn
        C0: xnxnxnxnxnxnxnxn
        D0: xnxnxnxnxnxnxnxn
        E0: xnxnxnxnxnxnxnxn
        F0: xnxnxnxnxnxnxnxn
        */

		ypos = SMS_VDP_VRAM(table_base+spritenum);
		xpos = SMS_VDP_VRAM(table_base+0x80+spritenum*2+0);
		num  = SMS_VDP_VRAM(table_base+0x80+spritenum*2+1)|(pattern_bit<<8);

		if (chip->regs[0x1]&0x2)
		{
			num &=0x1fe;
			height=16;
		}
		else height = 8;


		xpos+=16; // allow room either side for clipping (avoids xdrawpos of -8 if bit below is set)

		if (chip->regs[0x0]&0x08) xpos-=8;

		if ((sms_mode_table[chip->screen_mode].sms2_height)==192)
		{
			if (ypos == 0xd0)
				return;
		}

		ypos++;

		num <<= 5;
		//num+=((scanline-ypos)&0x7)*4;

		visible_line = 0;

		if (ypos<=scanline && ypos+height>scanline)
		{
			visible_line = 1;
			num+=((scanline-ypos)&(height-1))*4;

		}
		else if (ypos+height>0x100)
		{
			if (scanline< ypos+height-0x100)
			{
				visible_line = 1;
				num+=((scanline-ypos)&(height-1))*4;

			}
		}

		if (visible_line)
		{
			int xx;
			UINT32 gfxdata;

			gfxdata = (SMS_VDP_VRAM(num&0x3fff)<<24)|(SMS_VDP_VRAM((num+1)&0x3fff)<<16)|(SMS_VDP_VRAM((num+2)&0x3fff)<<8)|(SMS_VDP_VRAM((num+3)&0x3fff)<<0);


			for (xx=0;xx<8;xx++)
			{
				UINT8 pixel = (( (gfxdata>>(0+xx)  ) &0x00000001)<<3)|
				              (( (gfxdata>>(8+xx)  ) &0x00000001)<<2)|
				              (( (gfxdata>>(16+xx) ) &0x00000001)<<1)|
				              (( (gfxdata>>(24+xx) ) &0x00000001)<<0);

				if (pixel)
				{
					if (!chip->sprite_renderline[xpos+((width-1)-xx)])
					{
						chip->sprite_renderline[xpos+((width-1)-xx)] = pixel;
					}
					else
					{
						chip->sprite_collision = 1;
					}
				}
			}

			max_sprites--;

			if (max_sprites==0)
			{
				chip->sprite_overflow = 1;
				return;
			}

		}
	}
}

static void sms_render_tileline(int scanline, struct sms_vdp* chip)
{
	int column = 0;
	int count = 32;
	int drawxpos;

	UINT8 xscroll = chip->regs[0x8];
	UINT8 yscroll = chip->yscroll;
	UINT16 table_base = (chip->regs[0x2]&0x0e)<<10;
	UINT16 our_base;
	UINT8  our_line = (scanline+yscroll) & 0x7;

	/* In 224 and 240 line modes the table base is different */
	if ((sms_mode_table[chip->screen_mode].sms2_height)!=192)
	{
		table_base &=0x3700; table_base|=0x700;
	}

	if ((chip->regs[0x0]&0x40) && scanline < 16)
	{
		xscroll = 0;
	}

//  xscroll = 0;

//  table_base -= 0x0100;

	our_base = table_base+(((scanline+yscroll)%sms_mode_table[chip->screen_mode].sms2_tilemap_height)>>3)*64;// + (yscroll>>3)*32;

	our_base &=0x3fff;

	memset(chip->tile_renderline, (chip->regs[0x7]&0x0f)+0x10, 256+8);

	drawxpos = xscroll&0x7;
	column = 32-(xscroll>>3);

	do
	{
		UINT16 tiledata = (SMS_VDP_VRAM((our_base+(column&0x1f)*2+1)&0x3fff) << 8) |
		                  (SMS_VDP_VRAM((our_base+(column&0x1f)*2+0)&0x3fff) << 0);

//      UINT16 pattern = ((column+((scanline>>3)*32)) & 0x01ff)<<5;

		draw_tile_line(drawxpos, our_line, tiledata, chip->tile_renderline, chip);

		drawxpos+=8;
		column++;
		column&=0x1f;
		count--;
	} while (count);

}

static void sms_copy_to_renderbuffer(int scanline, struct sms_vdp* chip)
{
	int x;
	UINT16* lineptr = BITMAP_ADDR16(chip->r_bitmap, scanline, 0);

	for (x=0;x<256;x++)
	{
		UINT8 dat = chip->tile_renderline[x];
		UINT8 col;


		col = (chip->regs[0x7]&0x0f)+0x10;
		lineptr[x] = chip->cram_mamecolours[col];


		if ((x<8 && (chip->regs[0x0]&0x20)) || !(chip->regs[0x1]&0x40))
			continue;


		if (!(dat & 0x80))
		{
			lineptr[x] = chip->cram_mamecolours[dat&0x1f];
			if ((dat&0xf)==0x0) lineptr[x]|=0x8000;

		}

		if (chip->sprite_renderline[x+16])
		{
			lineptr[x] =  chip->cram_mamecolours[chip->sprite_renderline[x+16]+0x10];
		}

		if (dat & 0x80)
		{
			lineptr[x] = chip->cram_mamecolours[dat&0x1f];
			if ((dat&0xf)==0x0) lineptr[x]|=0x8000;
		}

	}

}

static void sms_draw_scanline(int scanline, struct sms_vdp* chip)
{

	if (scanline>=0 && scanline<sms_mode_table[chip->screen_mode].sms2_height)
	{
		sms_render_spriteline(scanline, chip);
		sms_render_tileline(scanline, chip);
		sms_copy_to_renderbuffer(scanline, chip);

	}
}


static TIMER_CALLBACK( sms_scanline_timer_callback )
{
	/* This function is called at the very start of every scanline starting at the very
       top-left of the screen.  The first scanline is scanline 0 (we set scanline to -1 in
       VIDEO_EOF) */

	/* Compensate for some rounding errors

       When the counter reaches 314 (or whatever the max lines is) we should have reached the
       end of the frame, however due to rounding errors in the timer calculation we're not quite
       there.  Let's assume we are still in the previous scanline for now.

       The position to get the H position also has to compensate for a few errors
    */
//  printf("num %d\n",num );
	struct sms_vdp *chip = ptr;

	if (chip->sms_scanline_counter<(chip->sms_total_scanlines-1))
	{
		chip->sms_scanline_counter++;
		timer_adjust(chip->sms_scanline_timer, ATTOTIME_IN_HZ(chip->sms_framerate * chip->sms_total_scanlines), 0, attotime_zero);

		if (chip->sms_scanline_counter>sms_mode_table[chip->screen_mode].sms2_height)
		{
			chip->hint_counter=chip->regs[0xa];
		}

		if (chip->sms_scanline_counter==0)
		{
			chip->hint_counter=chip->regs[0xa];
		}

		if (chip->sms_scanline_counter<=192)
		{
			chip->hint_counter--;

			if (chip->hint_counter==0xff)
			{
				//if (chip->chip_id==2) printf("irq triggerd on scanline %d %d\n", vdp1->sms_scanline_counter, vdp2->sms_scanline_counter);

				chip->line_irq_pending = 1;
				chip->hint_counter=chip->regs[0xa];
				if (chip->regs[0x0]&0x10)
				{
					(chip->set_irq)(1); // set IRQ;
				}
				else
				{
					(chip->set_irq)(0); // clear IRQ;
				}
			}

		}


		sms_draw_scanline(chip->sms_scanline_counter, chip);

		//if(sms_scanline_counter==0) chip->sprite_collision = 0;

		if (chip->sms_scanline_counter==sms_mode_table[chip->screen_mode].sms2_height+1 )
		{
			chip->frame_irq_pending = 1;
			if (chip->regs[0x1]&0x20)
			{
				(chip->set_irq)(1); // set IRQ;
			}
			else
			{
				(chip->set_irq)(0); // clear IRQ;
			}
		}
	}
	else
	{	/* if we're called passed the total number of scanlines then assume we're still on the last one.
           this can happen due to rounding errors */
		chip->sms_scanline_counter = chip->sms_total_scanlines-1;
	}
}

#ifdef UNUSED_FUNCTION
static void show_tiles(struct sms_vdp* chip)
{
	int x,y,xx,yy;
	UINT16 count = 0;

	for (y=0;y<16;y++)
	{
		for (x=0;x<32;x++)
		{
			for (yy=0;yy<8;yy++)
			{
				int drawypos = y*8+yy;
				UINT16* lineptr = BITMAP_ADDR16(chip->r_bitmap, drawypos, 0);

				UINT32 gfxdata = (SMS_VDP_VRAM(count)<<24)|(SMS_VDP_VRAM(count+1)<<16)|(SMS_VDP_VRAM(count+2)<<8)|(SMS_VDP_VRAM(count+3)<<0);

				for (xx=0;xx<8;xx++)
				{
					int drawxpos = x*8+xx;

					UINT8 pixel = (( (gfxdata>>(0+xx)  ) &0x00000001)<<3)|
					              (( (gfxdata>>(8+xx)  ) &0x00000001)<<2)|
					              (( (gfxdata>>(16+xx) ) &0x00000001)<<1)|
					              (( (gfxdata>>(24+xx) ) &0x00000001)<<0);

					lineptr[drawxpos] = chip->cram_mamecolours[pixel+16];

				}


				count+=4;count&=0x3fff;
			}
		}
	}
}
#endif

/*
 Register $00 - Mode Control No. 1

 D7 - 1= Disable vertical scrolling for columns 24-31
 D6 - 1= Disable horizontal scrolling for rows 0-1
 D5 - 1= Mask column 0 with overscan color from register #7
 D4 - (IE1) 1= Line interrupt enable
 D3 - (EC) 1= Shift sprites left by 8 pixels
 D2 - (M4) 1= Use Mode 4, 0= Use TMS9918 modes (selected with M1, M2, M3)
 D1 - (M2) Must be 1 for M1/M3 to change screen height in Mode 4.
      Otherwise has no effect.
 D0 - 1= No sync, display is monochrome, 0= Normal display

 Bits 0 and 5 have no effect on the GameGear in either mode, while bit 6
 has no effect in GG mode but works normally in SMS mode.
 */

 /*
  Register $01 - Mode Control No. 2

  D7 - No effect
  D6 - (BLK) 1= Display visible, 0= display blanked.
  D5 - (IE0) 1= Frame interrupt enable.
  D4 - (M1) Selects 224-line screen for Mode 4 if M2=1, else has no effect.
  D3 - (M3) Selects 240-line screen for Mode 4 if M2=1, else has no effect.
  D2 - No effect
  D1 - Sprites are 1=16x16,0=8x8 (TMS9918), Sprites are 1=8x16,0=8x8 (Mode 4)
  D0 - Sprite pixels are doubled in size.

 Even though some games set bit 7, it does nothing.
 */

static void end_of_frame(struct sms_vdp *chip)
{
	UINT8 m1 = (chip->regs[0x1]&0x10)>>4;
	UINT8 m2 = (chip->regs[0x0]&0x02)>>1;
	UINT8 m3 = (chip->regs[0x1]&0x08)>>3;
	UINT8 m4 = (chip->regs[0x0]&0x04)>>2;
	UINT8 m5 = chip->is_pal;
	chip->screen_mode = m1|(m2<<1)|(m3<<2)|(m4<<3)|(m5<<4);

	if (chip->vdp_type!=GG_VDP) /* In GG mode the Game Gear resolution is fixed */
	{
		rectangle visarea;

		visarea.min_x = 0;
		visarea.max_x = 256-1;
		visarea.min_y = 0;
		visarea.max_y = sms_mode_table[chip->screen_mode].sms2_height-1;

		if (chip->chip_id==3) video_screen_configure(0, 256, 256, &visarea, HZ_TO_ATTOSECONDS(chip->sms_framerate));

	}
	else /* 160x144 */
	{
		rectangle visarea;
		visarea.min_x = (256-160)/2;
		visarea.max_x = (256-160)/2+160-1;
		visarea.min_y = (192-144)/2;
		visarea.max_y = (192-144)/2+144-1;
	}



//  printf("Mode: %s is ok\n", sms_mode_table[chip->screen_mode].sms2_name);

	chip->sms_scanline_counter = -1;
	chip->yscroll = chip->regs[0x9]; // this can't change mid-frame
	timer_adjust(chip->sms_scanline_timer, attotime_zero, 0, attotime_zero);
}

#ifdef UNUSED_FUNCTION
VIDEO_EOF(sms)
{
	end_of_frame(vdp1);
	//if (SMS_PAUSE_BUTTON) cpunum_set_input_line(machine, 0,INPUT_LINE_NMI,PULSE_LINE); // not on systeme!!!
}
#endif

static VIDEO_START(sms)
{

//  vdp->is_pal = 1;
//  vdp->sms_total_scanlines = 313;
//  vdp->sms_framerate = 50;
}

#ifdef UNUSED_FUNCTION
MACHINE_RESET(sms)
{
	timer_adjust(vdp1->sms_scanline_timer, attotime_zero, 0, attotime_zero);
}
#endif

/* Sega System E */

/*******************************************************************************
 Input Ports
********************************************************************************
 mostly unknown for the time being
*******************************************************************************/

	/* The Coinage is similar to Sega System 1 and C2, but
    it seems that Free Play is not used in all games
    (in fact, the only playable game that use it is
    Riddle of Pythagoras) */

#define SEGA_COIN_A \
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4") \
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x05, "2 Coins/1 Credit 5/3 6/4" ) \
	PORT_DIPSETTING(    0x04, "2 Coins/1 Credit, 4/3" ) \
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit, 5/6" ) \
	PORT_DIPSETTING(    0x02, "1 Coin/1 Credit, 4/5" ) \
	PORT_DIPSETTING(    0x01, "1 Coin/1 Credit, 2/3" ) \
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )

#define SEGA_COIN_B \
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8") \
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x50, "2 Coins/1 Credit 5/3 6/4" ) \
	PORT_DIPSETTING(    0x40, "2 Coins/1 Credit, 4/3" ) \
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x30, "1 Coin/1 Credit, 5/6" ) \
	PORT_DIPSETTING(    0x20, "1 Coin/1 Credit, 4/5" ) \
	PORT_DIPSETTING(    0x10, "1 Coin/1 Credit, 2/3" ) \
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )


static INPUT_PORTS_START( transfrm ) /* Used By Transformer */
	PORT_START_TAG("DSW0")	/* Read from Port 0xf2 */
	SEGA_COIN_A
	SEGA_COIN_B

	PORT_START_TAG("DSW1")	/* Read from Port 0xf3 */
	PORT_DIPNAME( 0x01, 0x00, "1 Player Only" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "10k, 30k, 50k and 70k" )
	PORT_DIPSETTING(    0x30, "20k, 60k, 100k and 140k"  )
	PORT_DIPSETTING(    0x10, "30k, 80k, 130k and 180k" )
	PORT_DIPSETTING(    0x00, "50k, 150k and 250k" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START_TAG("IN0")	/* Read from Port 0xe0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START2 )

	PORT_START_TAG("IN1")	/* Read from Port 0xe1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP  ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( fantzn2 ) /* Used By Fantasy Zone 2 */
	PORT_START_TAG("DSW0")	/* Read from Port 0xf2 */
	SEGA_COIN_A
	SEGA_COIN_B

	PORT_START_TAG("DSW1")	/* Read from Port 0xf3 */
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:1" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x30, 0x30, "Timer" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "90" )	/* 210 seconds */
	PORT_DIPSETTING(    0x30, "80" )	/* 180 seconds */
	PORT_DIPSETTING(    0x10, "70" )	/* 150 seconds */
	PORT_DIPSETTING(    0x00, "60" )	/* 120 seconds */
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START_TAG("IN0")	/* Read from Port 0xe0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START2 )

	PORT_START_TAG("IN1")	/* Read from Port 0xe1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP  ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( opaopa ) /* Used By Opa Opa */
	PORT_START_TAG("DSW0")	/* Read from Port 0xf2 */
	SEGA_COIN_A
	SEGA_COIN_B

	PORT_START_TAG("DSW1")	/* Read from Port 0xf3 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "0x30 Unknown" )
	PORT_DIPSETTING(    0x20, "0x20 Unknown" )
	PORT_DIPSETTING(    0x10, "0x10 Unknown" )
	PORT_DIPSETTING(    0x00, "0x00 Unknown" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START_TAG("IN0")	/* Read from Port 0xe0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START2 )

	PORT_START_TAG("IN1")	/* Read from Port 0xe1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START_TAG("IN2")	/* Read from Port 0xe2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( tetrisse ) /* Used By Tetris */
	PORT_START_TAG("DSW0")	/* Read from Port 0xf2 */
	SEGA_COIN_A
	SEGA_COIN_B

	PORT_START_TAG("DSW1")	/* Read from Port 0xf3 */
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:1" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:4" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START_TAG("IN0")	/* Read from Port 0xe0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START2 )

	PORT_START_TAG("IN1")	/* Read from Port 0xe1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP  ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( hangonjr ) /* Used By Hang On Jr */
	PORT_START_TAG("DSW0")	/* Read from Port 0xf2 */
	SEGA_COIN_A
	SEGA_COIN_B

	PORT_START_TAG("DSW1")	/* Read from Port 0xf3 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1") /* Supose to be demo sound but has no effect */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Enemies" ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )	/*  These three dips seems to be unused */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START_TAG("IN0")	/* Read from Port 0xe0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START_TAG("IN1")	/* Read from Port 0xe1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START_TAG("IN2")	/* Read from Port 0xf8 */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)

	PORT_START_TAG("IN3")  /* Read from Port 0xf8 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(20)
INPUT_PORTS_END

static INPUT_PORTS_START( ridleofp ) /* Used By Riddle Of Pythagoras */
	PORT_START_TAG("DSW0")	/* Read from Port 0xf2 */
	SEGA_COIN_A
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	SEGA_COIN_B
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_START_TAG("DSW1")	/* Read from Port 0xf3 */
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "100 (Cheat)")
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPNAME( 0x08, 0x08, "Ball Speed" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )  // difficult (on datasheet)
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:6,7") /* Values came from an original dipsheet */
	PORT_DIPSETTING(    0x60, "50K 100K 200K 1M 2M 10M 20M 50M" )
	PORT_DIPSETTING(    0x40, "100K 200K 1M 2M 10M 20M 50M" )
	PORT_DIPSETTING(    0x20, "200K 1M 2M 10M 20M 50M" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START_TAG("IN0")	/* Read from Port 0xe0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // Would Be IPT_START2 but the code doesn't use it

	PORT_START_TAG("IN1")	/* Port 0xe1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START_TAG("IN2")	/* Read from Port 0xf8 */
	PORT_BIT( 0x0fff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(60) PORT_KEYDELTA(125)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON2 )	/* is this used in the game? */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START_TAG("IN3")	/* Read from Port 0xf8 */
	PORT_BIT( 0x0fff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(60) PORT_KEYDELTA(125) PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
INPUT_PORTS_END



ROM_START( hangonjr )
	ROM_REGION( 0x30000, REGION_CPU1, 0 )
	ROM_LOAD( "rom5.ic7",	0x00000, 0x08000, CRC(d63925a7) SHA1(699f222d9712fa42651c753fe75d7b60e016d3ad) ) /* Fixed Code */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "rom4.ic5",	0x10000, 0x08000, CRC(ee3caab3) SHA1(f583cf92c579d1ca235e8b300e256ba58a04dc90) )
	ROM_LOAD( "rom3.ic4",	0x18000, 0x08000, CRC(d2ba9bc9) SHA1(85cf2a801883bf69f78134fc4d5075134f47dc03) )
	ROM_LOAD( "rom2.ic3",	0x20000, 0x08000, CRC(e14da070) SHA1(f8781f65be5246a23c1f492905409775bbf82ea8) )
	ROM_LOAD( "rom1.ic2",	0x28000, 0x08000, CRC(3810cbf5) SHA1(c8d5032522c0c903ab3d138f62406a66e14a5c69) )
ROM_END

ROM_START( ridleofp )
	ROM_REGION( 0x30000, REGION_CPU1, 0 )
	ROM_LOAD( "epr10426.bin",	0x00000, 0x08000, CRC(4404c7e7) SHA1(555f44786976a009d96a6395c9173929ad6138a7) ) /* Fixed Code */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "epr10425.bin",	0x10000, 0x08000, CRC(35964109) SHA1(a7bc64a87b23139b0edb9c3512f47dcf73feb854) )
	ROM_LOAD( "epr10424.bin",	0x18000, 0x08000, CRC(fcda1dfa) SHA1(b8497b04de28fc0d6b7cb0206ad50948cff07840) )
	ROM_LOAD( "epr10423.bin",	0x20000, 0x08000, CRC(0b87244f) SHA1(c88041614735a9b6cba1edde0a11ed413e115361) )
	ROM_LOAD( "epr10422.bin",	0x28000, 0x08000, CRC(14781e56) SHA1(f15d9d89e1ebff36c3867cfc8f0bdf7f6b3c96bc) )
ROM_END

ROM_START( transfrm )
	ROM_REGION( 0x30000, REGION_CPU1, 0 )
	ROM_LOAD( "ic7.top",	0x00000, 0x08000, CRC(ccf1d123) SHA1(5ade9b00e2a36d034fafdf1902d47a9a00e96fc4) ) /* Fixed Code */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "epr-7347.ic5",	0x10000, 0x08000, CRC(df0f639f) SHA1(a09a9841b66de246a585be63d911b9a42a323503) )
	ROM_LOAD( "epr-7348.ic4",	0x18000, 0x08000, CRC(0f38ea96) SHA1(d4d421c5d93832e2bc1f22f39dffb6b80f2750bd) )
	ROM_LOAD( "ic3.top",		0x20000, 0x08000, CRC(9d485df6) SHA1(b25f04803c8f7188021f3039aa13aac80d480823) )
	ROM_LOAD( "epr-7350.ic2",	0x28000, 0x08000, CRC(0052165d) SHA1(cf4b5dffa54238e513515b3fc90faa7ce0b65d34) )
ROM_END

ROM_START( astrofl )
	ROM_REGION( 0x50000, REGION_CPU1, 0 )
	ROM_LOAD( "epr-7723.ic7",	0x00000, 0x08000, CRC(66061137) SHA1(cb6a2c7864f9f87bbedfd4b1448ad6c2de65d6ca) ) /* encrypted */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "epr-7347.ic5",	0x10000, 0x08000, CRC(df0f639f) SHA1(a09a9841b66de246a585be63d911b9a42a323503) )
	ROM_LOAD( "epr-7348.ic4",	0x18000, 0x08000, CRC(0f38ea96) SHA1(d4d421c5d93832e2bc1f22f39dffb6b80f2750bd) )
	ROM_LOAD( "epr-7349.ic3",	0x20000, 0x08000, CRC(f8c352d5) SHA1(e59565ab6928c67706c6f82f6ea9a64cdfc65a21) )
	ROM_LOAD( "epr-7350.ic2",	0x28000, 0x08000, CRC(0052165d) SHA1(cf4b5dffa54238e513515b3fc90faa7ce0b65d34) )
ROM_END


ROM_START( tetrisse )
	ROM_REGION( 0x30000, REGION_CPU1, 0 )
	ROM_LOAD( "epr12213.7",	0x00000, 0x08000, CRC(ef3c7a38) SHA1(cbb91aef330ab1a37d3e21ecf1d008143d0dd7ec) ) /* Fixed Code */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "epr12212.5",	0x10000, 0x08000, CRC(28b550bf) SHA1(445922a62e8a7360335c754ad70dabbe0208207b) )
	ROM_LOAD( "epr12211.4",	0x18000, 0x08000, CRC(5aa114e9) SHA1(f9fc7fe4d0444a264185e74d2abc8475f0976534) )
	/* ic3 unpopulated */
	/* ic2 unpopulated */
ROM_END


ROM_START( fantzn2 )
	ROM_REGION( 0x50000, REGION_CPU1, 0 )
	ROM_LOAD( "epr-11416.ic7",	0x00000, 0x08000, CRC(76db7b7b) SHA1(d60e2961fc893dcb4445aed5f67515cbd25b610f) )	/* encrypted */

	ROM_LOAD( "epr-11415.ic5",	0x10000, 0x10000, CRC(57b45681) SHA1(1ae6d0d58352e246a4ec4e1ce02b0417257d5d20) )
	ROM_LOAD( "epr-11413.ic3",	0x20000, 0x10000, CRC(a231dc85) SHA1(45b94fdbde28c02e88546178ef3e8f9f3a96ab86) )
	ROM_LOAD( "epr-11414.ic4",	0x30000, 0x10000, CRC(6f7a9f5f) SHA1(b53aa2eded781c80466a79b7d81383b9a875d0be) )
	ROM_LOAD( "epr-11412.ic2",	0x40000, 0x10000, CRC(b14db5af) SHA1(04c7fb659385438b3d8f9fb66800eb7b6373bda9) )

	ROM_REGION( 0x2000, REGION_USER1, 0 ) /* MC8123 key */
	ROM_LOAD( "317-0057.key",  0x0000, 0x2000, CRC(ee43d0f0) SHA1(72cb75a4d8352fe372db12046a59ea044360d5c3) )
ROM_END

ROM_START( opaopa )
	ROM_REGION( 0x50000, REGION_CPU1, 0 )
	ROM_LOAD( "epr11224.ic7",	0x00000, 0x08000, CRC(024b1244) SHA1(59a522ac3d98982cc4ddb1c81f9584d3da453649) ) /* encrypted */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "epr11223.ic5",	0x10000, 0x08000, CRC(6bc41d6e) SHA1(8997a4ac2a9704f1400d0ec16b259ee496a7efef) ) /* encrypted */
	ROM_LOAD( "epr11222.ic4",	0x18000, 0x08000, CRC(395c1d0a) SHA1(1594bad13e78c5fad4db644cd85a6bac1eaddbad) ) /* encrypted */
	ROM_LOAD( "epr11221.ic3",	0x20000, 0x08000, CRC(4ca132a2) SHA1(cb4e4c01b6ab070eef37c0603190caafe6236ccd) ) /* encrypted */
	ROM_LOAD( "epr11220.ic2",	0x28000, 0x08000, CRC(a165e2ef) SHA1(498ff4c5d3a2658567393378c56be6ed86ac0384) ) /* encrypted */

	ROM_REGION( 0x2000, REGION_USER1, 0 ) /* MC8123 key */
	ROM_LOAD( "317-0042.key",  0x0000, 0x2000, CRC(d6312538) SHA1(494ac7f080775c21dc7d369e6ea78f3299e6975a) )
ROM_END


static UINT8* vdp2_vram_bank0;
static UINT8* vdp2_vram_bank1;

static UINT8* vdp1_vram_bank0;
static UINT8* vdp1_vram_bank1;
static UINT8 f7_bank_value;

static MACHINE_RESET(systeme)
{
	timer_adjust(vdp1->sms_scanline_timer, attotime_zero, 0, attotime_zero);
	timer_adjust(vdp2->sms_scanline_timer, attotime_zero, 0, attotime_zero);
}

MACHINE_RESET(megatech_md_sms)
{
	timer_adjust(md_sms_vdp->sms_scanline_timer, attotime_zero, 0, attotime_zero);
}

MACHINE_RESET(megatech_bios)
{
	timer_adjust(vdp1->sms_scanline_timer, attotime_zero, 0, attotime_zero);
}

static VIDEO_EOF(systeme)
{
	end_of_frame(vdp1);
	end_of_frame(vdp2);
}


VIDEO_EOF(megatech_md_sms)
{
	end_of_frame(md_sms_vdp);
}

VIDEO_EOF(megatech_bios)
{
	end_of_frame(vdp1);
}

VIDEO_UPDATE(megatech_md_sms)
{
	int x,y;

	for (y=0;y<224;y++)
	{
		UINT16* lineptr = BITMAP_ADDR16(bitmap, y, 0);
		UINT16* srcptr =  BITMAP_ADDR16(md_sms_vdp->r_bitmap, y, 0);

		for (x=0;x<256;x++)
		{
			lineptr[x]=srcptr[x]&0x7fff;
		}
	}

	return 0;
}


VIDEO_UPDATE(megatech_bios)
{
	int x,y;

	for (y=0;y<224;y++)
	{
		UINT16* lineptr = BITMAP_ADDR16(bitmap, y, 0);
		UINT16* srcptr =  BITMAP_ADDR16(vdp1->r_bitmap, y, 0);

		for (x=0;x<256;x++)
		{
			lineptr[x]=srcptr[x]&0x7fff;
		}
	}

	return 0;
}

static VIDEO_UPDATE(systeme)
{
//  show_tiles();
	int x,y;

	for (y=0;y<192;y++)
	{
		UINT16* lineptr = BITMAP_ADDR16(bitmap, y, 0);
		UINT16* srcptr =  BITMAP_ADDR16(vdp1->r_bitmap, y, 0);

		for (x=0;x<256;x++)
		{
			lineptr[x]=srcptr[x]&0x7fff;
		}

	}

	for (y=0;y<192;y++)
	{
		UINT16* lineptr = BITMAP_ADDR16(bitmap, y, 0);
		UINT16* srcptr =  BITMAP_ADDR16(vdp2->r_bitmap, y, 0);

		for (x=0;x<256;x++)
		{
			if(!(srcptr[x]&0x8000)) lineptr[x]=srcptr[x]&0x7fff;
		}
	}


	return 0;
}


static MACHINE_DRIVER_START( systeme )
	MDRV_CPU_ADD_TAG("z80", Z80, 10738600/2) /* correct for hangonjr, and astroflash/transformer at least  */
	MDRV_CPU_PROGRAM_MAP(systeme_readmem,systeme_writemem)
	MDRV_CPU_IO_MAP(sms_readport,sms_writeport)

	/* IRQ handled via the timers */

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION) // Vblank handled manually.
	MDRV_MACHINE_RESET(systeme)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB15)

	MDRV_SCREEN_SIZE(256, 256)
//  MDRV_SCREEN_VISIBLE_AREA(0, 255, 0, 223)
	MDRV_SCREEN_VISIBLE_AREA(0, 255, 0, 191)


	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(sms)
	MDRV_VIDEO_UPDATE(systeme) /* Copies a bitmap */
	MDRV_VIDEO_EOF(systeme) /* Used to Sync the timing */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(SN76496, 3579540)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD(SN76496, 3579540)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


static READ8_HANDLER( sms_vdp_2_data_r )
{
	return vdp_data_r(vdp2);
}

static WRITE8_HANDLER( sms_vdp_2_data_w )
{
	vdp_data_w(data, vdp2);
}

static READ8_HANDLER( sms_vdp_2_ctrl_r )
{
	return vdp_ctrl_r(vdp2);
}

static WRITE8_HANDLER( sms_vdp_2_ctrl_w )
{
	vdp_ctrl_w(data, vdp2);
}

static WRITE8_HANDLER( segasyse_videoram_w )
{
	if (f7_bank_value & 0x20)
	{ // to vdp1 vram
		if (f7_bank_value & 0x80)
		{
			vdp1_vram_bank0[offset] = data;
		}
		else
		{
			vdp1_vram_bank1[offset] = data;
		}
	}
	else
	{ // to vdp2 vram
		if (f7_bank_value & 0x40)
		{
			vdp2_vram_bank0[offset] = data;
		}
		else
		{
			vdp2_vram_bank1[offset] = data;
		}
	}

}

static WRITE8_HANDLER( systeme_bank_w )
{
	int rombank;
	f7_bank_value = data;

	rombank = data & 0x0f;

	if (f7_bank_value&0x80)
	{
		vdp1->vram = vdp1_vram_bank1;
	}
	else
	{
		vdp1->vram = vdp1_vram_bank0;
	}

	if (f7_bank_value&0x40)
	{
		vdp2->vram = vdp2_vram_bank1;
	}
	else
	{
		vdp2->vram = vdp2_vram_bank0;
	}

	//memcpy(sms_rom+0x8000, memory_region(REGION_USER1)+0x10000+rombank*0x4000, 0x4000);
	memory_set_bank(1, rombank);

}

static WRITE8_HANDLER( sms_sn76496_2_w )
{
	SN76496_1_w(0, data & 0xff);
}

static void init_ports_systeme(void)
{
	/* INIT THE PORTS *********************************************************************************************/

	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0x7e, 0x7e, 0, 0, sms_vcounter_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0x7e, 0x7e, 0, 0, sms_sn76496_w);


	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0x7b, 0x7b, 0, 0, sms_sn76496_2_w);

	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0x7f, 0x7f, 0, 0, sms_sn76496_w);

	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xba, 0xba, 0, 0, sms_vdp_data_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0xba, 0xba, 0, 0, sms_vdp_data_w);
	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xbb, 0xbb, 0, 0, sms_vdp_ctrl_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0xbb, 0xbb, 0, 0, sms_vdp_ctrl_w);


	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xbe, 0xbe, 0, 0, sms_vdp_2_data_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0xbe, 0xbe, 0, 0, sms_vdp_2_data_w);
	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xbf, 0xbf, 0, 0, sms_vdp_2_ctrl_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0xbf, 0xbf, 0, 0, sms_vdp_2_ctrl_w);


	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xe0, 0xe0, 0, 0, input_port_2_r);
	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xe1, 0xe1, 0, 0, input_port_3_r);
	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xe2, 0xe2, 0, 0, input_port_4_r);
	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xf2, 0xf2, 0, 0, input_port_0_r);
	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xf3, 0xf3, 0, 0, input_port_1_r);


	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0xf7, 0xf7, 0, 0, systeme_bank_w );

}



static void init_systeme_map(void)
{
	/* INIT THE MEMMAP / BANKING *********************************************************************************/

	/* catch any addresses that don't get mapped */
//  memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0x0000, 0xffff, 0, 0, z80_unmapped_r);
//  memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x0000, 0xffff, 0, 0, z80_unmapped_w);

	/* fixed rom bank area */
//  sms_rom = auto_malloc(0xc000);
//  memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0x0000, 0xbfff, 0, 0, MRA8_BANK1);
//  memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x0000, 0x7fff, 0, 0, MWA8_UNMAP);
//  memory_set_bankptr( 1, sms_rom );

	memory_configure_bank(1, 0, 16, memory_region(REGION_CPU1) + 0x10000, 0x4000);

	/* alternate way of accessing video ram */
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x8000, 0xbfff, 0, 0, segasyse_videoram_w);


//  memcpy(sms_rom, memory_region(REGION_USER1), 0x8000);

	/* main ram area */
	sms_mainram = auto_malloc(0x4000);
	memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0xc000, 0xffff, 0, 0, MRA8_BANK2);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xc000, 0xffff, 0, 0, MWA8_BANK2);
	memory_set_bankptr( 2, sms_mainram );
	memset(sms_mainram,0x00,0x4000);

	init_ports_systeme();
}

void init_for_megadrive(void)
{
	md_sms_vdp = start_vdp(Machine, GEN_VDP);
	md_sms_vdp->set_irq = sms_vdp_cpu1_irq_callback;
	md_sms_vdp->is_pal = 0;
	md_sms_vdp->sms_total_scanlines = 262;
	md_sms_vdp->sms_framerate = 60;
	md_sms_vdp->chip_id = 3;
}



DRIVER_INIT( megatech_bios )
{
//  init_systeme_map();

	vdp1 = start_vdp(machine, SMS2_VDP);
	vdp1->set_irq = sms_vdp_cpu2_irq_callback;
	vdp1->is_pal = 0;
	vdp1->sms_total_scanlines = 262;
	vdp1->sms_framerate = 60;
	vdp1->chip_id = 1;

	vdp1_vram_bank0 = vdp1->vram;
	vdp1_vram_bank1 = auto_malloc(0x4000);
}

static DRIVER_INIT( segasyse )
{
	init_systeme_map();

	vdp1 = start_vdp(machine, SMS2_VDP);
//  vdp1->set_irq = sms_vdp_cpu0_irq_callback;
	vdp1->is_pal = 0;
	vdp1->sms_total_scanlines = 262;
	vdp1->sms_framerate = 60;
	vdp1->chip_id = 1;

	vdp1_vram_bank0 = vdp1->vram;
	vdp1_vram_bank1 = auto_malloc(0x4000);


	vdp2 = start_vdp(machine, SMS2_VDP);
	vdp2->set_irq = sms_vdp_cpu0_irq_callback;
	vdp2->is_pal = 0;
	vdp2->sms_total_scanlines = 262;
	vdp2->sms_framerate = 60;
	vdp2->chip_id = 2;

	vdp2_vram_bank0 = vdp2->vram;
	vdp2_vram_bank1 = auto_malloc(0x4000);
}

/*- Hang On Jr. Specific -*/
static UINT8 port_fa_last;		/* Last thing written to port 0xfa (control related) */

static READ8_HANDLER (segae_hangonjr_port_f8_r)
{
	UINT8 temp;

	temp = 0;

	if (port_fa_last == 0x08)  /* 0000 1000 */ /* Angle */
		temp = readinputport(4);

	if (port_fa_last == 0x09)  /* 0000 1001 */ /* Accel */
		temp = readinputport(5);

	return temp;
}

static WRITE8_HANDLER (segae_hangonjr_port_fa_w)
{
	/* Seems to write the same pattern again and again bits ---- xx-x used */
	port_fa_last = data;
}

/*- Riddle of Pythagoras Specific -*/

static int port_to_read,last1,last2,diff1,diff2;

static READ8_HANDLER (segae_ridleofp_port_f8_r)
{
	switch (port_to_read)
	{
		default:
		case 0:	return diff1 & 0xff;
		case 1:	return diff1 >> 8;
		case 2:	return diff2 & 0xff;
		case 3:	return diff2 >> 8;
	}
}

static WRITE8_HANDLER (segae_ridleofp_port_fa_w)
{
	/* 0x10 is written before reading the dial (hold counters?) */
	/* 0x03 is written after reading the dial (reset counters?) */

	port_to_read = (data & 0x0c) >> 2;

	if (data & 1)
	{
		int curr = readinputport(4);
		diff1 = ((curr - last1) & 0x0fff) | (curr & 0xf000);
		last1 = curr;
	}
	if (data & 2)
	{
		int curr = readinputport(5) & 0x0fff;
		diff2 = ((curr - last2) & 0x0fff) | (curr & 0xf000);
		last2 = curr;
	}
}

static DRIVER_INIT( ridleofp )
{
	DRIVER_INIT_CALL(segasyse);

	memory_install_read8_handler(0, ADDRESS_SPACE_IO, 0xf8, 0xf8, 0, 0, segae_ridleofp_port_f8_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0xfa, 0xfa, 0, 0, segae_ridleofp_port_fa_w);
}


static DRIVER_INIT( hangonjr )
{
	DRIVER_INIT_CALL(segasyse);

	memory_install_read8_handler(0, ADDRESS_SPACE_IO, 0xf8, 0xf8, 0, 0, segae_hangonjr_port_f8_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0xfa, 0xfa, 0, 0, segae_hangonjr_port_fa_w);
}

static DRIVER_INIT( opaopa )
{
	DRIVER_INIT_CALL(segasyse);

	mc8123_decrypt_rom(0, memory_region(REGION_USER1), 1, 8);
}

static DRIVER_INIT( fantzn2 )
{
	DRIVER_INIT_CALL(segasyse);

	mc8123_decrypt_rom(0, memory_region(REGION_USER1), 0, 0);
}

static DRIVER_INIT( astrofl )
{
	DRIVER_INIT_CALL(segasyse);

	astrofl_decode();
}

GAME( 1985, hangonjr, 0,        systeme, hangonjr, hangonjr, ROT0,  "Sega", "Hang-On Jr.", 0 )
GAME( 1986, transfrm, 0,        systeme, transfrm, segasyse, ROT0,  "Sega", "Transformer", 0 )
GAME( 1986, astrofl,  transfrm, systeme, transfrm, astrofl,  ROT0,  "Sega", "Astro Flash (Japan)", 0 )
GAME( 1986, ridleofp, 0,        systeme, ridleofp, ridleofp, ROT90, "Sega / Nasco", "Riddle of Pythagoras (Japan)", 0 )
GAME( 1987, opaopa,   0,        systeme, opaopa,   opaopa,   ROT0,  "Sega", "Opa Opa (MC-8123, 317-0042)", 0 )
GAME( 1988, fantzn2,  0,        systeme, fantzn2,  fantzn2,  ROT0,  "Sega", "Fantasy Zone 2 (MC-8123, 317-0057)", 0 )
GAME( 1988, tetrisse, 0,        systeme, tetrisse, segasyse, ROT0,  "Sega", "Tetris (Japan, System E)", 0 )
