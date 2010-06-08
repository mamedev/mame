/*

CPS3 Driver (preliminary)

Decryption by Andreas Naive

Driver by David Haywood
 with help from Tomasz Slanina and ElSemi

Sound emulation by Philip Bennett

SCSI code by ElSemi

ToDo: (in order or priority?)

Street Fighter 3 2nd Impact uses flipped tilemaps during flashing, emulate this.

Figure out proper IRQ10 generation:
 If we generate on DMA operations only then Warzard is OK, otherwise it hangs during attract
 HOWEVER, SFIII2 sometimes has messed up character profiles unless we also generate it periodicly.
 I think the corrupt background on some of the lighning effects may be realted to this + the DMA
 status flags.

Alpha Blending Effects
    These are actually palette manipulation effects, not true blending.  How the values are used is
    not currently 100% understood.  They are incorrect if you use player 2 in Warzard

Linezoom
    Is it used anywhere??

Palette DMA effects
    Verify them, they might not be 100% accurate at the moment

Verify Full Screen Zoom on real hardware
 Which is which, x & y registers, how far can it zoom etc.

Verify CRT registers
 Only SFIII2 changes them, for widescreen mode.  What other modes are possible?

Sprite positioning glitches
 Some sprites are still in the wrong places, seems the placement of zooming sprites is imperfect
 eg. warzard intro + cutscenes leave the left most 16 pixels uncovered because the sprite is positioned incorrectly,
     the same occurs in the sf games.  doesn't look like the origin is correct when zooming in all cases.

Gaps in Sprite Zooming
 probably cause by use of drawgfx instead of processing as a single large sprite, but could also be due to the
 positioning of each part of the sprite.  Warzard is confirmed to have gaps during some cutscenes on real hardware.

---

Capcom CP SYSTEM III Hardware Overview
Capcom, 1996-1999

From late 1996 to 1999 Capcom developed another hardware platform to rival the CPS2 System and called
it CP SYSTEM III. Only 6 games were produced....

Game                                                 Date   S/W Rev        CD Part#     Label         Cart Part#
----------------------------------------------------------------------------------------------------------------
JoJo no Kimyouna Bouken / JoJo's Venture             1998   98/12/02       CAP-JJK000   CAP-JJK-140   JJK98c00F
                                                            99/01/08                    CAP-JJK-160
                                                            99/01/28 **
JoJo no Kimyouna Bouken  Miraie no Isan
/ JoJo's Bizarre Adventure Heritage for the Future   1999   99/09/13       CAP-JJM000   CAP-JJM-110   JJM99900F
                                                            99/09/27 **

Street Fighter III New Generation                    1997   97/02/04       CAP-SF3000   CAP-SF3-3     SF397200F

Street Fighter III 2nd Impact Giant Attack           1997   97/09/30       CAP-3GA000   CAP-3GA-1     3GA97a00F

Street Fighter III 3rd Strike Fight for the Future   1999   99/05/12       CAP-33S000   CAP-33S-1     33S99400F
                                                            99/06/08                    CAP-33S-2

Warzard / Red Earth                                  1996   96/11/21       CAP-WZD000   CAP-WZD-5     WZD96a00F

** NOT DUMPED but known to exist

The Game Region / No CD Flags / Development flags etc. are controlled by a byte in the bios roms.  The CDs
contain revisions of the game code and are independant of the region.

The CP SYSTEM III comprises a main board with several custom ASICs, custom 72-pin SIMMs for program
and graphics storage (the same SIMMs are also used in some CPS2 titles), SCSI CDROM and CDROM disc,
and a plug-in security cart containing a boot ROM, an NVRAM and another custom ASIC containing vital
decryption information held by a [suicide] battery.

Not much is known about the actual CPU used in this system due to the extensive use of encryption,
and the volatile nature of the security information. There appears to be a custom Hitachi SH-2
CPU on the mainboard and there has been confirmed to be one in the cart. Tests were done by decrypting
the BIOS and code and running it on the PCB. It is known that neither of these CPU's will run standard
(i.e. unencrypted) SH2 code.

The security cart is thought to work like this....  the flashROM in the cart contains a program BIOS which is
decrypted by the CPU in the cart (the CPU has built-in decryption) then executed by that CPU to boot the BIOS
code.  Even though the code in the flashROM is encrypted, the cart can run it even if it is dead/suicided because
it has been discovered that the BIOS contains a hidden security menu allowing the cart to be loaded with the
security data. This proves the cart runs the BIOS even if it is dead. The special security menu is not
normally available but is likely accessed with a special key/button combination which is unknown ATM.
The cart contains a FM1208S NVRAM which appears to either be unused or holds game settings. Because the CPU
in the cart is always powered by a battery, it has stealth capability that allows it to continually monitor
the situation. If the custom CPU detects any tampering (generally things such as voltage fluctuation or
voltage dropping or even removal of the cart with the power on), it immediately erases the SRAM inside the
CPU (and thus the key) which effectively kills the security cart dead. This also suggests that the custom
Capcom CPU contains some additional internal code to initiate the boot process which is battery-backed
as well. It is known (from decapping it) that the CPU in the security cart does contain an amount of static
RAM for data storage and a SH2 core.

The main board uses the familiar Capcom SIMM modules to hold the data from the CDROM so that the life of
the CD drive is maximized. The SIMMs don't contain RAM, but instead TSOP48 surface mounted flashROMs
that can be updated with different games on bootup using a built-in software updating system.
The SIMMs that hold the program code are located in positions 1 & 2 and are 64MBit.
The SIMMs that hold the graphics are located in positions 3, 4, 5, 6 & 7 and are 128MBit.
The data in the SIMMs is not decrypted, it is merely taken directly from the CDROM and shuffled slightly
then programmed to the flashROMs. The SIMMs hold the entire contents of the CDROM.

To swap games requires the security cart for the game, it's CDROM disc and the correctly populated type
and number of SIMMs on the main board.
On first power-up after switching the cart and CD, you're presented with a screen asking if you want to
re-program the SIMMs with the new game. Pressing player 1 button 2 cancels it. Pressing player 1 button 1
allows it to proceed whereby you wait about 25 minutes then the game boots up almost immediately. On
subsequent power-ups, the game boots immediately.
If the CDROM is not present in the drive on a normal bootup, a message tells you to insert the CDROM.
Then you press button 1 to continue and the game boots immediately.
Note that not all of the SIMMs are populated on the PCB for each game. Some games have more, some less,
depending on game requirements, so flash times can vary per game. See the table below for details.

                                                     |----------- Required SIMM Locations & Types -----------|
Game                                                 1       2       3        4        5         6         7
--------------------------------------------------------------------------------------------------------------
JoJo's Venture                                       64MBit  64MBit  128MBit  128MBit  32MBit    -         -
JoJo's Bizarre Adventure                             64MBit  64MBit  128MBit  128MBit  128MBit   -         -
Street Fighter III New Generation                    64MBit  -       128MBit  128MBit  32MBit*   -         -
Street Fighter III 2nd Impact Giant Attack           64MBit  64MBit  128MBit  128MBit  128MBit   -         -
Street Fighter III 3rd Strike Fight for the Future   64MBit  64MBit  128MBit  128MBit  128MBit   128MBit   -
Warzard / Red Earth                                  64MBit  -       128MBit  128MBit  32MBit*   -         -

                                                     Notes:
                                                           - denotes not populated
                                                           * 32MBit SIMMs have only 2 FlashROMs populated on them.
                                                             128MBit SIMMs can also be used.
                                                           No game uses a SIMM at 7
                                                           See main board diagram below for SIMM locations.

Due to the built-in upgradability of the hardware, and the higher frame-rates the hardware seems to have,
it appears Capcom had big plans for this system and possibly intended to create many games on it, as they
did with CPS2. Unfortunately for Capcom, CP SYSTEM III was an absolute flop in the arcades so those plans
were cancelled. Possible reasons include...
- The games were essentially just 2D, and already there were many 3D games coming out onto the market that
  interested operators more than this.
- The cost of the system was quite expensive when compared to other games on the market.
- It is rumoured that the system was difficult to program for developers.
- These PCBs were not popular with operators because the security carts are extremely static-sensitive and most
  of them failed due to the decryption information being zapped by simple handling of the PCBs or by touching
  the security cart edge connector underneath the PCB while the security cart was plugged in, or by power
  fluctuations while flashing the SIMMs. You will know if your cart has been zapped because on bootup, you get
  a screen full of garbage coloured pixels instead of the game booting up, or just a black or single-coloured
  screen. You should also not touch the inside of the security cart because it will be immediately zapped
  when you touch it! The PCB can detect the presence of the security cart and if it is removed on a working game,
  the game will freeze immediately and it will also erase the security cart battery-backed data.


PCB Layouts
-----------

CAPCOM
CP SYSTEMIII
95682A-4 (older rev 95682A-3)
   |----------------------------------------------------------------------|
  |= J1             HM514260     |------------|      |  |  |  |  |        |
   |                             |CAPCOM      |      |  |  |  |  |        |
  |= J2     TA8201  TC5118160    |DL-2729 PPU |      |  |  |  |  |        |
   |                             |(QFP304)    |      |  |  |  |  |        |
|--|          VOL   TC5118160    |            |      |  |  |  |  |        |
|    LM833N                      |            |      S  S  S  S  S        |
|    LM833N         TC5118160    |------------|      I  I  I  I  I        |
|           TDA1306T                      |--------| M  M  M  M  M        |
|                   TC5118160  60MHz      |CAPCOM  | M  M  M  M  M       |-|
|                              42.9545MHz |DL-3329 | 7  6  5  4  3       | |
|           LM385                         |SSU     | |  |  |  |  |       | |
|J                         KM681002       |--------| |  |  |  |  |       | |
|A                         KM681002  62256 |-------| |  |  |  |  |       | |
|M                                         |DL3529 | |  |  |  |  |       | |
|M          MC44200FU                      |GLL2   | |  |  |  |  |       | |
|A                              3.6864MHz  |-------|                  CN6| |
|                                                             |  |       | |
|                               |--------|   |-|              |  |       | |
|                               |CAPCOM  |   | |   |-------|  |  |       | |
|        TD62064                |DL-2929 |   | |   |CAPCOM |  |  |       | |
|                               |IOU     |   | |   |DL-3429|  |  |       | |
|        TD62064                |--------|   | |   |GLL1   |  S  S       | |
|--|                            *HA16103FPJ  | |   |-------|  I  I       |-|
   |                                         | |CN5           M  M        |
   |                                         | |   |-------|  M  M        |
  |-|                        93C46           | |   |CAPCOM |  2  1        |
  | |      PS2501                            | |   |DL-2829|  |  | |-----||
  | |CN1                                     | |   |CCU    |  |  | |AMD  ||
  | |      PS2501                            | |   |-------|  |  | |33C93||
  |-|                                        |-|              |  | |-----||
   |   SW1                                         HM514260   |  |        |
   |----------------------------------------------------------------------|
Notes:
      TA8201     - Toshiba TA8201 18W BTL x 2-Channel Audio Power Amplifier
      PS2501     - NEC PS2501 High Isolation Voltage Single Transistor Type Multi Photocoupler (DIP16)
      TDA1306T   - Philips TDA1306T Noise Shaping Filter DAC (SOIC24). The clock (on pin 12) measures
                   14.3181667MHz (42.9545/3)
      MC44200FU  - Motorola MC44200FU Triple 8-bit Video DAC (QFP44)
      LM833N     - ST Microelectronics LM833N Low Noise Audio Dual Op-Amp (DIP8)
      TD62064    - Toshiba TD62064AP NPN 50V 1.5A Quad Darlington Driver (DIP16)
      HA16103FPJ - Hitachi HA16103FPJ Watchdog Timer (SOIC20)
                   *Note this IC is not populated on the rev -4 board
      93C46      - National Semiconductor NM93C46A 128bytes x8 Serial EEPROM (SOIC8)
                   Note this IC is covered by a plastic housing on the PCB. The chip is just a normal
                   (unsecured) EEPROM so why it was covered is not known.
      LM385      - National Semiconductor LM385 Adjustable Micropower Voltage Reference Diode (SOIC8)
      33C93      - AMD 33C93A-16 SCSI Controller (PLCC44)
      KM681002   - Samsung Electronics KM681002 128k x8 SRAM (SOJ32)
      62256      - 8k x8 SRAM (SOJ28)
      HM514260   - Hitachi HM514260CJ7 1M x16 DRAM (SOJ42)
      TC5118160  - Toshiba TC5118160BJ-60 256k x16 DRAM (SOJ42)
      SW1        - Push-button Test Switch
      VOL        - Master Volume Potentiometer
      J1/J2      - Optional RCA Left/Right Audio Out Connectors
      CN1        - 34-Pin Capcom Kick Button Harness Connector
      CN5        - Security Cartridge Slot
      CN6        - 4-Pin Power Connector and 50-pin SCSI Data Cable Connector
                   CDROM Drive is a CR504-KCM 4X SCSI drive manufactured By Panasonic / Matsushita
      SIMM 1-2   - 72-Pin SIMM Connector, holds single sided SIMMs containing 4x Fujitsu 29F016A
                   surface mounted TSOP48 FlashROMs
      SIMM 3-7   - 72-Pin SIMM Connector, holds double sided SIMMs containing 8x Fujitsu 29F016A
                   surface mounted TSOP48 FlashROMs

                   SIMM Layout -
                          |----------------------------------------------------|
                          |                                                    |
                          |   |-------|   |-------|   |-------|   |-------|    |
                          |   |Flash_A|   |Flash_B|   |Flash_C|   |Flash_D|    |
                          |   |-------|   |-------|   |-------|   |-------|    |
                          |-                                                   |
                           |-------------------------/\------------------------|
                           Notes:
                                  For SIMMs 1-2, Flash_A & Flash_C and regular pinout (Fujitsu 29F016A-90PFTN)
                                  Flash_B & Flash_D are reverse pinout (Fujitsu 29F016A-90PFTR)
                                  and are mounted upside down also so that pin1 lines up with
                                  the normal pinout of FlashROMs A & C.
                                  For SIMMs 3-7, the 8 FlashROMs are populated on both sides using a similar layout.

      Capcom Custom ASICs -
                           DL-2729 PPU SD10-505   (QFP304). Decapping reveals this is the main graphics chip.
                           DL-2829 CCU SD07-1514  (QFP208). Decapping reveals this to be a custom Toshiba ASIC.
                           DL-2929 IOU SD08-1513  (QFP208). This is the I/O controller.
                           DL-3329 SSU SD04-1536  (QFP144). This is might be the main CPU. It appears to be a SH2
                                                            variant with built-in encryption. It is clocked at
                                                            21.47725MHz (42.9545/2)
                           DL-3429 GLL1 SD06-1537 (QFP144). Unknown, possibly a DMA or bus controller.
                           DL-3529 GLL2 SD11-1755 (QFP80).  This might be the sound chip (it has 32k SRAM connected to it).


Connector Pinouts
-----------------

                       JAMMA Connector                                       Extra Button Connector
                       ---------------                                       ----------------------
                    PART SIDE    SOLDER SIDE                                       TOP    BOTTOM
                ----------------------------                               --------------------------
                      GND  01    A  GND                                        GND  01    02  GND
                      GND  02    B  GND                                        +5V  03    04  +5V
                      +5V  03    C  +5V                                       +12V  05    06  +12V
                      +5V  04    D  +5V                                             07    08
                       NC  05    E  NC                           Player 2 Button 4  09    10
                     +12V  06    F  +12V                                            11    12
                           07    H                                                  13    14
           Coin Counter 1  08    J  NC                           Player 1 Button 4  15    16
             Coin Lockout  09    K  Coin Lockout                 Player 1 Button 5  17    18
               Speaker (+) 10    L  Speaker (-)                  Player 1 Button 6  19    20
                       NC  11    M  NC                           Player 2 Button 5  21    22
                Video Red  12    N  Video Green                  Player 2 Button 6  23    24
               Video Blue  13    P  Video Composite Sync                            25    26
             Video Ground  14    R  Service Switch                                  27    28
                     Test  15    S  NC                                 Volume Down  29    30  Volume UP
                   Coin A  16    T  Coin B                                     GND  31    32  GND
           Player 1 Start  17    U  Player 2 Start                             GND  33    34  GND
              Player 1 Up  18    V  Player 2 Up
            Player 1 Down  19    W  Player 2 Down
            Player 1 Left  20    X  Player 2 Left
           Player 1 Right  21    Y  Player 2 Right
        Player 1 Button 1  22    Z  Player 2 Button 1
        Player 1 Button 2  23    a  Player 2 Button 2
        Player 1 Button 3  24    b  Player 2 Button 3
                       NC  25    c  NC
                       NC  26    d  NC
                      GND  27    e  GND
                      GND  28    f  GND


Security Cartridge PCB Layout
-----------------------------

CAPCOM 95682B-3 TORNADE
|------------------------------------------------|
|      BATTERY                                   |
|                          |-------|             |
|                          |CAPCOM |   29F400    |
|                          |DL-3229|   *28F400   |
|                          |SCU    |     *FM1208S|
| 74HC00                   |-------|             |
|               6.25MHz                    74F00 |
|---|     |-|                             |------|
    |     | |                             |
    |-----| |-----------------------------|
Notes:
      74F00        - 74F00 Quad 2-Input NAND Gate (SOIC14)
      74HC00       - Philips 74HC00N Quad 2-Input NAND Gate (DIP14)
      29F400       - Fujitsu 29F400TA-90PFTN 512k x8 FlashROM (TSOP48)
      Custom ASIC  - CAPCOM DL-3229 SCU (QFP144). Decapping reveals this is a Hitachi HD6417099 SH2 variant
                     with built-in encryption, clocked at 6.250MHz
      FM1208S      - RAMTRON FM1208S 4k (512bytes x8) Nonvolatile Ferroelectric RAM (SOIC24)
      28F400       - 28F400 SOP44 FlashROM (not populated)
      *            - These components located on the other side of the PCB
      The battery powers the CPU only. A small board containing some transistors is wired to the 74HC00
      to switch the CPU from battery power to main power to increase the life of the battery.

*/

#include "emu.h"
#include "cdrom.h"
#include "cpu/sh2/sh2.h"
#include "machine/intelfsh.h"
#include "includes/cps3.h"
#include "machine/wd33c93.h"

#define MASTER_CLOCK	42954500

/* load extracted cd content? */
#define LOAD_CD_CONTENT 1
#define DEBUG_PRINTF 0



#define DMA_XOR(a)		((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(1,2))


static UINT32* decrypted_bios;

static UINT32* decrypted_gamerom;
static UINT32 cram_gfxflash_bank;
static UINT32* cps3_nops;

static UINT32* tilemap20_regs_base;
static UINT32* tilemap30_regs_base;
static UINT32* tilemap40_regs_base;
static UINT32* tilemap50_regs_base;

static UINT32* cps3_0xc0000000_ram;
static UINT32* cps3_0xc0000000_ram_decrypted;

static UINT32* cps3_char_ram;

static UINT32* cps3_spriteram;
static UINT32* cps3_eeprom;
static UINT32* cps3_fullscreenzoom;

static UINT32 cps3_ss_pal_base = 0;
static UINT32* cps3_colourram;
static UINT32 cps3_unk_vidregs[0x20/4];
static UINT32 cps3_ss_bank_base = 0;

static UINT32 cps3_screenwidth;
//cdrom_file* cps3_cd;

static UINT32* cps3_mame_colours;//[0x20000]; // actual values to write to 32-bit bitmap

static bitmap_t *renderbuffer_bitmap;
static rectangle renderbuffer_clip;

static UINT8* cps3_user4region;
UINT8* cps3_user5region;
#define USER4REGION_LENGTH 0x800000*2
#define USER5REGION_LENGTH 0x800000*10

#define CPS3_TRANSPARENCY_NONE 0
#define CPS3_TRANSPARENCY_PEN 1
#define CPS3_TRANSPARENCY_PEN_INDEX 2
#define CPS3_TRANSPARENCY_PEN_INDEX_BLEND 3

INLINE void cps3_drawgfxzoom(bitmap_t *dest_bmp,const rectangle *clip,const gfx_element *gfx,
		unsigned int code,unsigned int color,int flipx,int flipy,int sx,int sy,
		int transparency,int transparent_color,
		int scalex, int scaley,bitmap_t *pri_buffer,UINT32 pri_mask)
{
	rectangle myclip;

//  UINT8 al;

//  al = (pdrawgfx_shadow_lowpri) ? 0 : 0x80;

	if (!scalex || !scaley) return;

// todo: reimplement this optimization!!
//  if (scalex == 0x10000 && scaley == 0x10000)
//  {
//      common_drawgfx(dest_bmp,gfx,code,color,flipx,flipy,sx,sy,clip,transparency,transparent_color,pri_buffer,pri_mask);
//      return;
//  }

	/*
    scalex and scaley are 16.16 fixed point numbers
    1<<15 : shrink to 50%
    1<<16 : uniform scale
    1<<17 : double to 200%
    */


	/* force clip to bitmap boundary */
	if(clip)
	{
		myclip.min_x = clip->min_x;
		myclip.max_x = clip->max_x;
		myclip.min_y = clip->min_y;
		myclip.max_y = clip->max_y;

		if (myclip.min_x < 0) myclip.min_x = 0;
		if (myclip.max_x >= dest_bmp->width) myclip.max_x = dest_bmp->width-1;
		if (myclip.min_y < 0) myclip.min_y = 0;
		if (myclip.max_y >= dest_bmp->height) myclip.max_y = dest_bmp->height-1;

		clip=&myclip;
	}


	/* 32-bit ONLY */
	{
		if( gfx )
		{
//          const pen_t *pal = &gfx->colortable[gfx->color_granularity * (color % gfx->total_colors)];
			UINT32 palbase = (gfx->color_granularity * color) & 0x1ffff;
			const pen_t *pal = &cps3_mame_colours[palbase];
			const UINT8 *source_base = gfx_element_get_data(gfx, code % gfx->total_elements);

			int sprite_screen_height = (scaley*gfx->height+0x8000)>>16;
			int sprite_screen_width = (scalex*gfx->width+0x8000)>>16;

			if (sprite_screen_width && sprite_screen_height)
			{
				/* compute sprite increment per screen pixel */
				int dx = (gfx->width<<16)/sprite_screen_width;
				int dy = (gfx->height<<16)/sprite_screen_height;

				int ex = sx+sprite_screen_width;
				int ey = sy+sprite_screen_height;

				int x_index_base;
				int y_index;

				if( flipx )
				{
					x_index_base = (sprite_screen_width-1)*dx;
					dx = -dx;
				}
				else
				{
					x_index_base = 0;
				}

				if( flipy )
				{
					y_index = (sprite_screen_height-1)*dy;
					dy = -dy;
				}
				else
				{
					y_index = 0;
				}

				if( clip )
				{
					if( sx < clip->min_x)
					{ /* clip left */
						int pixels = clip->min_x-sx;
						sx += pixels;
						x_index_base += pixels*dx;
					}
					if( sy < clip->min_y )
					{ /* clip top */
						int pixels = clip->min_y-sy;
						sy += pixels;
						y_index += pixels*dy;
					}
					if( ex > clip->max_x+1 )
					{ /* clip right */
						int pixels = ex-clip->max_x-1;
						ex -= pixels;
					}
					if( ey > clip->max_y+1 )
					{ /* clip bottom */
						int pixels = ey-clip->max_y-1;
						ey -= pixels;
					}
				}

				if( ex>sx )
				{ /* skip if inner loop doesn't draw anything */
					int y;

					/* case 0: no transparency */
					if (transparency == CPS3_TRANSPARENCY_NONE)
					{
						{
							for( y=sy; y<ey; y++ )
							{
								const UINT8 *source = source_base + (y_index>>16) * gfx->line_modulo;
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);

								int x, x_index = x_index_base;
								for( x=sx; x<ex; x++ )
								{
									dest[x] = pal[source[x_index>>16]];
									x_index += dx;
								}

								y_index += dy;
							}
						}
					}
					else if (transparency == CPS3_TRANSPARENCY_PEN)
					{
						{
							for( y=sy; y<ey; y++ )
							{
								const UINT8 *source = source_base + (y_index>>16) * gfx->line_modulo;
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);

								int x, x_index = x_index_base;
								for( x=sx; x<ex; x++ )
								{
									int c = source[x_index>>16];
									if( c != transparent_color ) dest[x] = pal[c];
									x_index += dx;
								}

								y_index += dy;
							}
						}
					}
					else if (transparency == CPS3_TRANSPARENCY_PEN_INDEX)
					{
						{
							for( y=sy; y<ey; y++ )
							{
								const UINT8 *source = source_base + (y_index>>16) * gfx->line_modulo;
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);

								int x, x_index = x_index_base;
								for( x=sx; x<ex; x++ )
								{
									int c = source[x_index>>16];
									if( c != transparent_color ) dest[x] = c | palbase;
									x_index += dx;
								}

								y_index += dy;
							}
						}
					}
					else if (transparency == CPS3_TRANSPARENCY_PEN_INDEX_BLEND)
					{
						{
							for( y=sy; y<ey; y++ )
							{
								const UINT8 *source = source_base + (y_index>>16) * gfx->line_modulo;
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);

								int x, x_index = x_index_base;
								for( x=sx; x<ex; x++ )
								{
									int c = source[x_index>>16];
									if( c != transparent_color )
									{
										/* blending isn't 100% understood */
										if (gfx->color_granularity == 64)
										{
											// OK for sfiii2 spotlight
											if (c&0x01) dest[x] |= 0x2000;
											if (c&0x02) dest[x] |= 0x4000;
											if (c&0x04) dest[x] |= 0x8000;
											if (c&0x08) dest[x] |= 0x10000;
											if (c&0xf0) dest[x] |= mame_rand(gfx->machine); // ?? not used?
										}
										else
										{
											// OK for jojo intro, and warzard swords, and various shadows in sf games
											if (c&0x01) dest[x] |= 0x8000;
											if (color&0x100) dest[x]|=0x10000;
										}
									}


									x_index += dx;
								}

								y_index += dy;
							}
						}
					}
				}
			}
		}
	}
}



static DIRECT_UPDATE_HANDLER( cps3_direct_handler );

/* Encryption */

static UINT32 cps3_key1, cps3_key2;
static int cps3_altEncryption; // sfiii2 has different encryption, data isn't encrypted outside of the bios

static UINT16 rotate_left(UINT16 value, int n)
{
   int aux = value>>(16-n);
   return ((value<<n)|aux)%0x10000;
}

static UINT16 rotxor(UINT16 val, UINT16 xorval)
{
	UINT16 res;

	res = val + rotate_left(val,2);

	res = rotate_left(res,4) ^ (res & (val ^ xorval));

	return res;
}

static UINT32 cps3_mask(UINT32 address, UINT32 key1, UINT32 key2)
{
	UINT16 val;

	address ^= key1;

	val = (address & 0xffff) ^ 0xffff;

	val = rotxor(val, key2 & 0xffff);

	val ^= (address >> 16) ^ 0xffff;

	val = rotxor(val, key2 >> 16);

	val ^= (address & 0xffff) ^ (key2 & 0xffff);

	return val | (val << 16);
}

static void cps3_decrypt_bios(running_machine *machine)
{
	int i;
	UINT32 *coderegion = (UINT32*)memory_region(machine, "user1");

	decrypted_bios = (UINT32*)memory_region(machine, "user1");

	for (i=0;i<0x80000;i+=4)
	{
		UINT32 dword = coderegion[i/4];
		UINT32 xormask = cps3_mask(i, cps3_key1, cps3_key2);
		decrypted_bios[i/4] = dword ^ xormask;
	}
#if 0
	/* Dump to file */
	{
		FILE *fp;
		const char *gamename = machine->gamedrv->name;
		char filename[256];
		sprintf(filename, "%s_bios.dump", gamename);

		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(decrypted_bios, 0x080000, 1, fp);
			fclose(fp);
		}
	}
#endif
}


static DRIVER_INIT( cps3 )
{
	int i;

	// cache pointers to regions
	cps3_user4region = memory_region(machine,"user4");
	cps3_user5region = memory_region(machine,"user5");

	if (!cps3_user4region) cps3_user4region = auto_alloc_array(machine, UINT8, USER4REGION_LENGTH);
	if (!cps3_user5region) cps3_user5region = auto_alloc_array(machine, UINT8, USER5REGION_LENGTH);

	// set strict verify
	sh2drc_set_options(devtag_get_device(machine, "maincpu"), SH2DRC_STRICT_VERIFY);

	cps3_decrypt_bios(machine);
	decrypted_gamerom = auto_alloc_array(machine, UINT32, 0x1000000/4);

	/* just some NOPs for the game to execute if it crashes and starts executing unmapped addresses
     - this prevents MAME from crashing */
	cps3_nops = auto_alloc(machine, UINT32);
	cps3_nops[0] = 0x00090009;


	cps3_0xc0000000_ram_decrypted = auto_alloc_array(machine, UINT32, 0x400/4);
	memory_set_direct_update_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), cps3_direct_handler);

	// flash roms

	for (i=0;i<48;i++)
		intelflash_init( machine, i, FLASH_FUJITSU_29F016A, NULL );

	cps3_eeprom = auto_alloc_array(machine, UINT32, 0x400/4);
}

static DRIVER_INIT( jojo )    { cps3_key1 = 0x02203ee3; cps3_key2 = 0x01301972; cps3_altEncryption = 0; DRIVER_INIT_CALL(cps3); }
static DRIVER_INIT( jojoba )  { cps3_key1 = 0x23323ee3; cps3_key2 = 0x03021972; cps3_altEncryption = 0; DRIVER_INIT_CALL(cps3); }
static DRIVER_INIT( sfiii )   { cps3_key1 = 0xb5fe053e; cps3_key2 = 0xfc03925a; cps3_altEncryption = 0; DRIVER_INIT_CALL(cps3); }
static DRIVER_INIT( sfiii2 )  { cps3_key1 = 0x00000000; cps3_key2 = 0x00000000; cps3_altEncryption = 1; DRIVER_INIT_CALL(cps3); }
static DRIVER_INIT( sfiii3 )  { cps3_key1 = 0xa55432b4; cps3_key2 = 0x0c129981; cps3_altEncryption = 0; DRIVER_INIT_CALL(cps3); }
static DRIVER_INIT( redearth ){ cps3_key1 = 0x9e300ab1; cps3_key2 = 0xa175b82c; cps3_altEncryption = 0; DRIVER_INIT_CALL(cps3); }


/* GFX decodes */


static const gfx_layout cps3_tiles16x16_layout =
{
	16,16,
	0x8000,
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 3*8,2*8,1*8,0*8,7*8,6*8,5*8,4*8,
	  11*8,10*8,9*8,8*8,15*8,14*8,13*8,12*8 },
	{ 0*128,1*128,2*128,3*128,4*128,5*128,6*128,7*128,8*128,9*128,10*128,11*128,12*128,13*128,14*128,15*128},
	8*256
};



static const gfx_layout cps3_tiles8x8_layout =
{
	8,8,
	0x400,
	4,
	{ /*8,9,10,11,*/ 0,1,2,3 },
	{ 20,16,4,0,52,48,36,32 },

	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};

static UINT32* cps3_ss_ram;

static void cps3_set_mame_colours(running_machine *machine, int colournum, UINT16 data, UINT32 fadeval )
{
	int r,g,b;
	UINT16* dst = (UINT16*)cps3_colourram;


	r = (data >> 0) & 0x1f;
	g = (data >> 5) & 0x1f;
	b = (data >> 10) & 0x1f;

	/* is this 100% correct? */
	if (fadeval!=0)
	{
		int fade;
		//printf("fadeval %08x\n",fadeval);

		fade = (fadeval & 0x3f000000)>>24;
		r = (r*fade)>>5;
		if (r>0x1f) r = 0x1f;

		fade = (fadeval & 0x003f0000)>>16;
		g = (g*fade)>>5;
		if (g>0x1f) g = 0x1f;

		fade = (fadeval & 0x0000003f)>>0;
		b = (b*fade)>>5;
		if (b>0x1f) b = 0x1f;

		data = (r <<0) | (g << 5) | (b << 10);
	}

	dst[colournum] = data;

	cps3_mame_colours[colournum] = (r << (16+3)) | (g << (8+3)) | (b << (0+3));

	if (colournum<0x10000) palette_set_color(machine,colournum,cps3_mame_colours[colournum]/* MAKE_RGB(r<<3,g<<3,b<<3)*/);//cps3_mame_colours[colournum]);
}


static VIDEO_START(cps3)
{
	cps3_ss_ram       = auto_alloc_array(machine, UINT32, 0x10000/4);
	memset(cps3_ss_ram, 0x00, 0x10000);
	state_save_register_global_pointer(machine, cps3_ss_ram, 0x10000/4);

	cps3_char_ram = auto_alloc_array(machine, UINT32, 0x800000/4);
	memset(cps3_char_ram, 0x00, 0x800000);
	state_save_register_global_pointer(machine, cps3_char_ram, 0x800000 /4);

	/* create the char set (gfx will then be updated dynamically from RAM) */
	machine->gfx[0] = gfx_element_alloc(machine, &cps3_tiles8x8_layout, (UINT8 *)cps3_ss_ram, machine->config->total_colors / 16, 0);

	//decode_ssram();

	/* create the char set (gfx will then be updated dynamically from RAM) */
	machine->gfx[1] = gfx_element_alloc(machine, &cps3_tiles16x16_layout, (UINT8 *)cps3_char_ram, machine->config->total_colors / 64, 0);
	machine->gfx[1]->color_granularity = 64;

	//decode_charram();

	cps3_mame_colours = auto_alloc_array(machine, UINT32, 0x80000/4);
	memset(cps3_mame_colours, 0x00, 0x80000);

	cps3_screenwidth = 384;

	// the renderbuffer can be twice the size of the screen, this allows us to handle framebuffer zoom values
	// between 0x00 and 0x80 (0x40 is normal, 0x80 would be 'view twice as much', 0x20 is 'view half as much')
	renderbuffer_bitmap = auto_bitmap_alloc(machine,512*2,224*2,machine->primary_screen->format());

	renderbuffer_clip.min_x = 0;
	renderbuffer_clip.max_x = cps3_screenwidth-1;
	renderbuffer_clip.min_y = 0;
	renderbuffer_clip.max_y = 224-1;

	bitmap_fill(renderbuffer_bitmap,&renderbuffer_clip,0x3f);

}

// the 0x400 bit in the tilemap regs is "draw it upside-down"  (bios tilemap during flashing, otherwise capcom logo is flipped)

static void cps3_draw_tilemapsprite_line(running_machine *machine, int tmnum, int drawline, bitmap_t *bitmap, const rectangle *cliprect )
{
	UINT32* tmapregs[4] = { tilemap20_regs_base, tilemap30_regs_base, tilemap40_regs_base, tilemap50_regs_base };
	UINT32* regs;
	int line;
	int scrolly;
	if (tmnum>3)
	{
		printf("cps3_draw_tilemapsprite_line Illegal tilemap number %d\n",tmnum);
		return;
	}
	regs = tmapregs[tmnum];

	scrolly =  ((regs[0]&0x0000ffff)>>0)+4;
	line = drawline+scrolly;
	line&=0x3ff;


	if (!(regs[1]&0x00008000)) return;

	{
		UINT32 mapbase =  (regs[2]&0x007f0000)>>16;
		UINT32 linebase=  (regs[2]&0x7f000000)>>24;
		int linescroll_enable = (regs[1]&0x00004000);

		int scrollx;
		int x;
		int tileline = (line/16)+1;
		int tilesubline = line % 16;
		rectangle clip;

		mapbase = mapbase << 10;
		linebase = linebase << 10;

		if (!linescroll_enable)
		{
			scrollx =  (regs[0]&0xffff0000)>>16;
		}
		else
		{
		//  printf("linebase %08x\n", linebase);

			scrollx =  (regs[0]&0xffff0000)>>16;
			scrollx+= (cps3_spriteram[linebase+((line+16-4)&0x3ff)]>>16)&0x3ff;

		}

//  zoombase    =  (layerregs[1]&0xffff0000)>>16;

		drawline&=0x3ff;

		if (drawline>cliprect->max_y+4) return;

		clip.min_x = cliprect->min_x;
		clip.max_x = cliprect->max_x;
		clip.min_y = drawline;
		clip.max_y = drawline;

		for (x=0;x<(cliprect->max_x/16)+2;x++)
		{

			UINT32 dat;
			int tileno;
			int colour;
			int bpp;
			int xflip,yflip;

			dat = cps3_spriteram[mapbase+((tileline&63)*64)+((x+scrollx/16)&63)];
			tileno = (dat & 0xffff0000)>>17;
			colour = (dat & 0x000001ff)>>0;
			bpp = (dat & 0x0000200)>>9;
			yflip  = (dat & 0x00000800)>>11;
			xflip  = (dat & 0x00001000)>>12;

			if (!bpp) machine->gfx[1]->color_granularity=256;
			else machine->gfx[1]->color_granularity=64;

			cps3_drawgfxzoom(bitmap,&clip,machine->gfx[1],tileno,colour,xflip,yflip,(x*16)-scrollx%16,drawline-tilesubline,CPS3_TRANSPARENCY_PEN_INDEX,0, 0x10000, 0x10000, NULL, 0);
		}
	}
}

static VIDEO_UPDATE(cps3)
{
	int y,x, count;
	attoseconds_t period = screen->frame_period().attoseconds;
	rectangle visarea = screen->visible_area();

	int bg_drawn[4] = { 0, 0, 0, 0 };

	UINT32 fullscreenzoomx, fullscreenzoomy;
	UINT32 fszx, fszy;

//  decode_ssram();
//  decode_charram();

	/* registers are normally 002a006f 01ef01c6
            widescreen mode = 00230076 026501c6
      only SFIII2 uses widescreen, I don't know exactly which register controls it */
	if (((cps3_fullscreenzoom[1]&0xffff0000)>>16)==0x0265)
	{
		if (cps3_screenwidth!=496)
		{
			cps3_screenwidth = 496;
			visarea.min_x = 0; visarea.max_x = 496-1;
			visarea.min_y = 0; visarea.max_y = 224-1;
			screen->configure(496, 224, visarea, period);
		}
	}
	else
	{
		if (cps3_screenwidth!=384)
		{
			cps3_screenwidth = 384;
			visarea.min_x = 0; visarea.max_x = 384-1;
			visarea.min_y = 0; visarea.max_y = 224-1;
			screen->configure(384, 224, visarea, period);
		}
	}

	fullscreenzoomx = cps3_fullscreenzoom[3] & 0x000000ff;
	fullscreenzoomy = cps3_fullscreenzoom[3] & 0x000000ff;
	/* clamp at 0x80, I don't know if this is accurate */
	if (fullscreenzoomx>0x80) fullscreenzoomx = 0x80;
	if (fullscreenzoomy>0x80) fullscreenzoomy = 0x80;

	fszx = (fullscreenzoomx<<16)/0x40;
	fszy = (fullscreenzoomy<<16)/0x40;

	renderbuffer_clip.min_x = 0;
	renderbuffer_clip.max_x = ((cps3_screenwidth*fszx)>>16)-1;
	renderbuffer_clip.min_y = 0;
	renderbuffer_clip.max_y = ((224*fszx)>>16)-1;

	bitmap_fill(renderbuffer_bitmap,&renderbuffer_clip,0);

	/* Sprites */
	{
		int i;

		//printf("Spritelist start:\n");
		for (i=0x00000/4;i<0x2000/4;i+=4)
		{
			int xpos =  	(cps3_spriteram[i+1]&0x03ff0000)>>16;
			int ypos =  	cps3_spriteram[i+1]&0x000003ff;
			int j;
			int gscroll =      (cps3_spriteram[i+0]&0x70000000)>>28;
			int length =    (cps3_spriteram[i+0]&0x01ff0000)>>16; // how many entries in the sprite table
			UINT32 start  =    (cps3_spriteram[i+0]&0x00007ff0)>>4;

			int whichbpp =     (cps3_spriteram[i+2]&0x40000000)>>30; // not 100% sure if this is right, jojo title / characters
			int whichpal =     (cps3_spriteram[i+2]&0x20000000)>>29;
			int global_xflip = (cps3_spriteram[i+2]&0x10000000)>>28;
			int global_yflip = (cps3_spriteram[i+2]&0x08000000)>>27;
			int global_alpha = (cps3_spriteram[i+2]&0x04000000)>>26; // alpha / shadow? set on sfiii2 shadows, and big black image in jojo intro
			int global_bpp =   (cps3_spriteram[i+2]&0x02000000)>>25;
			int global_pal =   (cps3_spriteram[i+2]&0x01ff0000)>>16;

			int gscrollx = (cps3_unk_vidregs[gscroll]&0x03ff0000)>>16;
			int gscrolly = (cps3_unk_vidregs[gscroll]&0x000003ff)>>0;
			start = (start * 0x100) >> 2;

			if ((cps3_spriteram[i+0]&0xf0000000) == 0x80000000)
				break;

			for (j=0;j<(length)*4;j+=4)
			{

				UINT32 value1 = 	(cps3_spriteram[start+j+0]);
				UINT32 value2 = 	(cps3_spriteram[start+j+1]);
				UINT32 value3 = 	(cps3_spriteram[start+j+2]);


				//UINT8* srcdata = (UINT8*)cps3_char_ram;
				//UINT32 sourceoffset = (value1 >>14)&0x7fffff;
				int count;

				UINT32 tileno = (value1&0xfffe0000)>>17;

				int xpos2 = (value2 & 0x03ff0000)>>16;
				int ypos2 = (value2 & 0x000003ff)>>0;
				int flipx = (value1 & 0x00001000)>>12;
				int flipy = (value1 & 0x00000800)>>11;
				int alpha = (value1 & 0x00000400)>>10; //? this one is used for alpha effects on warzard
				int bpp =   (value1 & 0x00000200)>>9;
				int pal =   (value1 & 0x000001ff);


				/* these are the sizes to actually draw */
				int ysizedraw2 = ((value3 & 0x7f000000)>>24);
				int xsizedraw2 = ((value3 & 0x007f0000)>>16);
				int xx,yy;

				static const int tilestable[4] = { 8,1,2,4 };
				int ysize2 = ((value3 & 0x0000000c)>>2);
				int xsize2 = ((value3 & 0x00000003)>>0);
				UINT32 xinc,yinc;

				if (ysize2==0)
				{
				//  printf("invalid sprite ysize of 0 tiles\n");
					continue;
				}

				if (xsize2==0) // xsize of 0 tiles seems to be a special command to draw tilemaps
				{
					int tilemapnum = ((value3 & 0x00000030)>>4);
					//int startline;// = value2 & 0x3ff;
					//int endline;
					//int height = (value3 & 0x7f000000)>>24;
					int uu;
					UINT32* tmapregs[4] = { tilemap20_regs_base, tilemap30_regs_base, tilemap40_regs_base, tilemap50_regs_base };
					UINT32* regs;
					regs = tmapregs[tilemapnum];
					//endline = value2;
					//startline = endline - height;

					//startline &=0x3ff;
					//endline &=0x3ff;

					//printf("tilemap draw %01x %02x %02x %02x\n",tilemapnum, value2, height, regs[0]&0x000003ff );

					//printf("tilemap draw %01x %d %d\n",tilemapnum, startline, endline );


					/* Urgh, the startline / endline seem to be direct screen co-ordinates regardless of fullscreen zoom
                       which probably means the fullscreen zoom is applied when rendering everything, not aftewards */
					//for (uu=startline;uu<endline+1;uu++)

					if (bg_drawn[tilemapnum]==0)
					{
						for (uu=0;uu<1023;uu++)
						{
							cps3_draw_tilemapsprite_line(screen->machine, tilemapnum, uu, renderbuffer_bitmap, &renderbuffer_clip );
						}
					}
					bg_drawn[tilemapnum] = 1;
				}
				else
				{
					ysize2 = tilestable[ysize2];
					xsize2 = tilestable[xsize2];

					xinc = ((xsizedraw2+1)<<16) / ((xsize2*0x10));
					yinc = ((ysizedraw2+1)<<16) / ((ysize2*0x10));

					xsize2-=1;
					ysize2-=1;

					flipx ^= global_xflip;
					flipy ^= global_yflip;

					if (!flipx) xpos2+=((xsizedraw2+1)/2);
					else xpos2-=((xsizedraw2+1)/2);

					ypos2+=((ysizedraw2+1)/2);

					if (!flipx) xpos2-= ((xsize2+1)*16*xinc)>>16;
					else  xpos2+= (xsize2*16*xinc)>>16;

					if (flipy) ypos2-= (ysize2*16*yinc)>>16;

					{
						count = 0;
						for (xx=0;xx<xsize2+1;xx++)
						{
							int current_xpos;

							if (!flipx) current_xpos = (xpos+xpos2+((xx*16*xinc)>>16));
							else current_xpos = (xpos+xpos2-((xx*16*xinc)>>16));
							//current_xpos +=  rand()&0x3ff;
							current_xpos += gscrollx;
							current_xpos += 1;
							current_xpos &=0x3ff;
							if (current_xpos&0x200) current_xpos-=0x400;

							for (yy=0;yy<ysize2+1;yy++)
							{
								int current_ypos;
								int actualpal;

								if (flipy) current_ypos = (ypos+ypos2+((yy*16*yinc)>>16));
								else current_ypos = (ypos+ypos2-((yy*16*yinc)>>16));

								current_ypos += gscrolly;
								current_ypos = 0x3ff-current_ypos;
								current_ypos -= 17;
								current_ypos &=0x3ff;

								if (current_ypos&0x200) current_ypos-=0x400;

								//if ( (whichbpp) && (machine->primary_screen->frame_number() & 1)) continue;

								/* use the palette value from the main list or the sublists? */
								if (whichpal)
								{
									actualpal = global_pal;
								}
								else
								{
									actualpal = pal;
								}

								/* use the bpp value from the main list or the sublists? */
								if (whichbpp)
								{
									if (!global_bpp) screen->machine->gfx[1]->color_granularity=256;
									else screen->machine->gfx[1]->color_granularity=64;
								}
								else
								{
									if (!bpp) screen->machine->gfx[1]->color_granularity=256;
									else screen->machine->gfx[1]->color_granularity=64;
								}

								{
									int realtileno = tileno+count;

									if (global_alpha || alpha)
									{
										cps3_drawgfxzoom(renderbuffer_bitmap,&renderbuffer_clip,screen->machine->gfx[1],realtileno,actualpal,0^flipx,0^flipy,current_xpos,current_ypos,CPS3_TRANSPARENCY_PEN_INDEX_BLEND,0,xinc,yinc, NULL, 0);
									}
									else
									{
										cps3_drawgfxzoom(renderbuffer_bitmap,&renderbuffer_clip,screen->machine->gfx[1],realtileno,actualpal,0^flipx,0^flipy,current_xpos,current_ypos,CPS3_TRANSPARENCY_PEN_INDEX,0,xinc,yinc, NULL, 0);
									}
									count++;
								}
							}
						}
					}
	//              */

				//  printf("cell %08x %08x %08x\n",value1, value2, value3);
				}
			}
		}
	}

	/* copy render bitmap with zoom */
	{
		UINT32 renderx,rendery;
		UINT32 srcx, srcy;
		UINT32* srcbitmap;
		UINT32* dstbitmap;


		srcy=0;
		for (rendery=0;rendery<224;rendery++)
		{
			dstbitmap = BITMAP_ADDR32(bitmap, rendery, 0);
			srcbitmap = BITMAP_ADDR32(renderbuffer_bitmap, srcy>>16, 0);
			srcx=0;

			for (renderx=0;renderx<cps3_screenwidth;renderx++)
			{
				dstbitmap[renderx] = cps3_mame_colours[srcbitmap[srcx>>16]&0x1ffff];
				srcx += fszx;
			}

			srcy += fszy;
		}
	}

	/* Draw the text layer */
	/* Copy the first 0x800 colours to be used for fg layer rendering */
//  for (offset=0;offset<0x200;offset++)
//  {
//      int palreadbase = (cps3_ss_pal_base << 9);
//      palette_set_color(machine,offset,cps3_mame_colours[palreadbase+offset]);
//  }

	// fg layer
	{
		// bank select? (sfiii2 intro)
		if (cps3_ss_bank_base & 0x01000000) count = 0x000;
		else count = 0x800;

		for (y=0;y<32;y++)
		{
			for (x=0;x<64;x++)
			{
				UINT32 data = cps3_ss_ram[count]; // +0x800 = 2nd bank, used on sfiii2 intro..
				UINT32 tile = (data >> 16) & 0x1ff;
				int pal = (data&0x003f) >> 1;
				int flipx = (data & 0x0080) >> 7;
				int flipy = (data & 0x0040) >> 6;
				pal += cps3_ss_pal_base << 5;
				tile+=0x200;

				cps3_drawgfxzoom(bitmap, cliprect, screen->machine->gfx[0],tile,pal,flipx,flipy,x*8,y*8,CPS3_TRANSPARENCY_PEN,0,0x10000,0x10000,NULL,0);
				count++;
			}
		}
	}
	return 0;
}

static READ32_HANDLER( cps3_ssram_r )
{
	if (offset>0x8000/4)
		return LITTLE_ENDIANIZE_INT32(cps3_ss_ram[offset]);
	else
		return cps3_ss_ram[offset];
}

static WRITE32_HANDLER( cps3_ssram_w )
{
	if (offset>0x8000/4)
	{
		// we only want to endian-flip the character data, the tilemap info is fine
		data = LITTLE_ENDIANIZE_INT32(data);
		mem_mask = LITTLE_ENDIANIZE_INT32(mem_mask);
		gfx_element_mark_dirty(space->machine->gfx[0], offset/16);
	}

	COMBINE_DATA(&cps3_ss_ram[offset]);
}

static WRITE32_HANDLER( cps3_0xc0000000_ram_w )
{
	COMBINE_DATA( &cps3_0xc0000000_ram[offset] );
	// store a decrypted copy
	cps3_0xc0000000_ram_decrypted[offset] = cps3_0xc0000000_ram[offset]^cps3_mask(offset*4+0xc0000000, cps3_key1, cps3_key2);
}



static DIRECT_UPDATE_HANDLER( cps3_direct_handler )
{
//  if(DEBUG_PRINTF) printf("address %04x\n",address);

	/* BIOS ROM */
	if (address < 0x80000)
	{
		direct->raw = direct->decrypted = memory_region(space->machine, "user1");
		return ~0;
	}
	/* RAM */
	else if (address >= 0x06000000 && address <= 0x06ffffff)
	{
		direct->decrypted = (UINT8*)decrypted_gamerom-0x06000000;
		direct->raw = (UINT8*)decrypted_gamerom-0x06000000;

		if (cps3_altEncryption) direct->raw = (UINT8*) cps3_user4region-0x06000000;


		return ~0;
	}
	else if (address >= 0xc0000000 && address <= 0xc00003ff)
	{
		//direct->decrypted = (void*)cps3_0xc0000000_ram_decrypted;
		direct->decrypted = (UINT8*)cps3_0xc0000000_ram_decrypted-0xc0000000;
		direct->raw = (UINT8*)cps3_0xc0000000_ram-0xc0000000;
		return ~0;
	}

	/* anything else falls through to NOPs */
	direct->decrypted = (UINT8*)cps3_nops-address;
	direct->raw = (UINT8*)cps3_nops-address;
	return ~0;
}

static UINT32 cram_bank = 0;

static WRITE32_HANDLER( cram_bank_w )
{
	if (ACCESSING_BITS_0_7)
	{
		// this seems to be related to accesses to the 0x04100000 region
		if (cram_bank != data)
		{
			cram_bank = data;
		//if(data&0xfffffff0)
		//bank_w 00000000, ffff0000
		//bank_w 00000001, ffff0000
		//bank_w 00000002, ffff0000
		//bank_w 00000003, ffff0000
		//bank_w 00000004, ffff0000
		//bank_w 00000005, ffff0000
		//bank_w 00000006, ffff0000
		//bank_w 00000007, ffff0000
		// during CHARACTER RAM test..
			if(DEBUG_PRINTF) printf("bank_w %08x, %08x\n",data,mem_mask);

		}
	}
	else
	{
		if(DEBUG_PRINTF) printf("bank_w LSB32 %08x, %08x\n",data,mem_mask);

	}
}

static READ32_HANDLER( cram_data_r )
{
	UINT32 fulloffset = (((cram_bank&0x7)*0x100000)/4) + offset;

	return LITTLE_ENDIANIZE_INT32(cps3_char_ram[fulloffset]);
}

static WRITE32_HANDLER( cram_data_w )
{
	UINT32 fulloffset = (((cram_bank&0x7)*0x100000)/4) + offset;
	mem_mask = LITTLE_ENDIANIZE_INT32(mem_mask);
	data = LITTLE_ENDIANIZE_INT32(data);
	COMBINE_DATA(&cps3_char_ram[fulloffset]);
	gfx_element_mark_dirty(space->machine->gfx[1], fulloffset/0x40);
}

/* FLASH ROM ACCESS */

static READ32_HANDLER( cps3_gfxflash_r )
{
	UINT32 result = 0;
	int flash1 = 8;
	int flash2 = 9;
	if (cram_gfxflash_bank&1) offset += 0x200000/4;

	flash1 += cram_gfxflash_bank&0x3e;
	flash2 += cram_gfxflash_bank&0x3e;

	if(DEBUG_PRINTF) printf("gfxflash_r\n");

	if (ACCESSING_BITS_24_31)	// GFX Flash 1
	{
		logerror("read GFX flash chip %d addr %02x\n", flash1-8, (offset<<1));
		result |= intelflash_read(flash1, (offset<<1) ) << 24;
	}
	if (ACCESSING_BITS_16_23)	// GFX Flash 2
	{
		logerror("read GFX flash chip %d addr %02x\n", flash2-8, (offset<<1));
		result |= intelflash_read(flash2, (offset<<1) ) << 16;
	}
	if (ACCESSING_BITS_8_15)	// GFX Flash 1
	{
		logerror("read GFX flash chip %d addr %02x\n", flash1-8, (offset<<1)+1);
		result |= intelflash_read(flash1, (offset<<1)+0x1 ) << 8;
	}
	if (ACCESSING_BITS_0_7)	// GFX Flash 2
	{
		logerror("read GFX flash chip %d addr %02x\n", flash2-8, (offset<<1)+1);
		result |= intelflash_read(flash2, (offset<<1)+0x1 ) << 0;
	}

	//printf("read GFX flash chips addr %02x returning %08x mem_mask %08x crambank %08x gfxbank %08x\n", offset*2, result,mem_mask,  cram_bank, cram_gfxflash_bank  );

	return result;
}

static WRITE32_HANDLER( cps3_gfxflash_w )
{
	int command;
	int flash1 = 8;
	int flash2 = 9;
	if (cram_gfxflash_bank&1) offset += 0x200000/4;

	flash1 += cram_gfxflash_bank&0x3e;
	flash2 += cram_gfxflash_bank&0x3e;


//  if(DEBUG_PRINTF) printf("cps3_gfxflash_w %08x %08x %08x\n", offset *2, data, mem_mask);


	if (ACCESSING_BITS_24_31)	// GFX Flash 1
	{
		command = (data >> 24) & 0xff;
		logerror("write to GFX flash chip %d addr %02x cmd %02x\n", flash1-8, (offset<<1), command);
		intelflash_write(flash1, (offset<<1), command);
	}
	if (ACCESSING_BITS_16_23)	// GFX Flash 2
	{
		command = (data >> 16) & 0xff;
		logerror("write to GFX flash chip %d addr %02x cmd %02x\n", flash2-8, (offset<<1), command);
		intelflash_write(flash2, (offset<<1), command);
	}
	if (ACCESSING_BITS_8_15)	// GFX Flash 1
	{
		command = (data >> 8) & 0xff;
		logerror("write to GFX flash chip %d addr %02x cmd %02x\n", flash1-8, (offset<<1)+1, command);
		intelflash_write(flash1, (offset<<1)+0x1, command);
	}
	if (ACCESSING_BITS_0_7)	// GFX Flash 2
	{
		command = (data >> 0) & 0xff;
		//if ( ((offset<<1)+1) != 0x555) printf("write to GFX flash chip %d addr %02x cmd %02x\n", flash1-8, (offset<<1)+1, command);
		intelflash_write(flash2, (offset<<1)+0x1, command);
	}

	/* make a copy in the linear memory region we actually use for drawing etc.  having it stored in interleaved flash roms isnt' very useful */
	{
		UINT32* romdata = (UINT32*)cps3_user5region;
		int real_offset = 0;
		UINT32 newdata;
		UINT8* ptr1 = (UINT8*)intelflash_getmemptr(flash1);
		UINT8* ptr2 = (UINT8*)intelflash_getmemptr(flash2);

		real_offset = ((cram_gfxflash_bank&0x3e) * 0x200000) + offset*4;

		newdata =((ptr1[((offset*2)&0xfffffffe)+0]<<8) |
			      (ptr1[((offset*2)&0xfffffffe)+1]<<24) |
                  (ptr2[((offset*2)&0xfffffffe)+0]<<0)  |
                  (ptr2[((offset*2)&0xfffffffe)+1]<<16));

//      printf("flashcrap %08x %08x %08x\n", offset *2, romdata[real_offset/4], newdata);
		romdata[real_offset/4] = newdata;
	}
}



static UINT32 cps3_flashmain_r(int base, UINT32 offset, UINT32 mem_mask)
{
	UINT32 result = 0;

	if (ACCESSING_BITS_24_31)	// Flash 1
	{
//      logerror("read flash chip %d addr %02x\n", base+0, offset*4 );
		result |= (intelflash_read(base+0, offset)<<24);
	}
	if (ACCESSING_BITS_16_23)	// Flash 1
	{
//      logerror("read flash chip %d addr %02x\n", base+1, offset*4 );
		result |= (intelflash_read(base+1, offset)<<16);
	}
	if (ACCESSING_BITS_8_15)	// Flash 1
	{
//      logerror("read flash chip %d addr %02x\n", base+2, offset*4 );
		result |= (intelflash_read(base+2, offset)<<8);
	}
	if (ACCESSING_BITS_0_7)	// Flash 1
	{
//      logerror("read flash chip %d addr %02x\n", base+3, offset*4 );
		result |= (intelflash_read(base+3, offset)<<0);
	}

//  if (base==4) logerror("read flash chips addr %02x returning %08x\n", offset*4, result );

	return result;
}



static READ32_HANDLER( cps3_flash1_r )
{
	UINT32 retvalue = cps3_flashmain_r(0, offset,mem_mask);

	if (cps3_altEncryption) return retvalue;

	retvalue = retvalue ^ cps3_mask(0x6000000+offset*4, cps3_key1, cps3_key2);
	return retvalue;
}

static READ32_HANDLER( cps3_flash2_r )
{
	UINT32 retvalue = cps3_flashmain_r(4, offset,mem_mask);

	if (cps3_altEncryption) return retvalue;

	retvalue = retvalue ^ cps3_mask(0x6800000+offset*4, cps3_key1, cps3_key2);
	return retvalue;
}

static void cps3_flashmain_w(running_machine *machine, int base, UINT32 offset, UINT32 data, UINT32 mem_mask)
{
	int command;
	if (ACCESSING_BITS_24_31)	// Flash 1
	{
		command = (data >> 24) & 0xff;
		logerror("write to flash chip %d addr %02x cmd %02x\n", base+0, offset, command);
		intelflash_write(base+0, offset, command);
	}
	if (ACCESSING_BITS_16_23)	// Flash 2
	{
		command = (data >> 16) & 0xff;
		logerror("write to flash chip %d addr %02x cmd %02x\n", base+1, offset, command);
		intelflash_write(base+1, offset, command);
	}
	if (ACCESSING_BITS_8_15)	// Flash 2
	{
		command = (data >> 8) & 0xff;
		logerror("write to flash chip %d addr %02x cmd %02x\n", base+2, offset, command);
		intelflash_write(base+2, offset, command);
	}
	if (ACCESSING_BITS_0_7)	// Flash 2
	{
		command = (data >> 0) & 0xff;
		logerror("write to flash chip %d addr %02x cmd %02x\n", base+3, offset, command);
		intelflash_write(base+3, offset, command);
	}

	/* copy data into regions to execute from */
	{
		UINT32* romdata =  (UINT32*)cps3_user4region;
		UINT32* romdata2 = (UINT32*)decrypted_gamerom;
		int real_offset = 0;
		UINT32 newdata;
		UINT8* ptr1 = (UINT8*)intelflash_getmemptr(base+0);
		UINT8* ptr2 = (UINT8*)intelflash_getmemptr(base+1);
		UINT8* ptr3 = (UINT8*)intelflash_getmemptr(base+2);
		UINT8* ptr4 = (UINT8*)intelflash_getmemptr(base+3);

		real_offset = offset * 4;

		if (base==4)
		{
			romdata+=0x800000/4;
			romdata2+=0x800000/4;
			real_offset += 0x800000;
		}

		newdata = (ptr1[offset]<<24) |
			      (ptr2[offset]<<16) |
                  (ptr3[offset]<<8) |
                  (ptr4[offset]<<0);

		//printf("%08x %08x %08x %08x %08x\n",offset, romdata2[offset], romdata[offset], newdata,  newdata^cps3_mask(0x6000000+real_offset, cps3_key1, cps3_key2)  );

		romdata[offset] = newdata;
		romdata2[offset] = newdata^cps3_mask(0x6000000+real_offset, cps3_key1, cps3_key2);
	}
}

static WRITE32_HANDLER( cps3_flash1_w )
{
	cps3_flashmain_w(space->machine,0,offset,data,mem_mask);
}

static WRITE32_HANDLER( cps3_flash2_w )
{
	cps3_flashmain_w(space->machine,4,offset,data,mem_mask);
}

static WRITE32_HANDLER( cram_gfxflash_bank_w )
{
	if (ACCESSING_BITS_24_31)
	{
		//printf("cram_gfxflash_bank_w MSB32 %08x\n",data);
/*
    SIMM 3 (Rom 30/31)
    cram_gfxflash_bank_w MSB32 00020000  // first half of gfx 0 + 1
    cram_gfxflash_bank_w MSB32 00030000  // 2nd half of gfx 0 + 1
    cram_gfxflash_bank_w MSB32 00040000  // first half of gfx 2 + 3
    cram_gfxflash_bank_w MSB32 00050000
    cram_gfxflash_bank_w MSB32 00060000  // first half of gfx 4 + 5
    cram_gfxflash_bank_w MSB32 00070000
    cram_gfxflash_bank_w MSB32 00080000  // first half of gfx 6 + 7
    cram_gfxflash_bank_w MSB32 00090000
    SIMM 4 (Rom 40/41)
    cram_gfxflash_bank_w MSB32 000a0000  // first half of gfx 8 + 9
    cram_gfxflash_bank_w MSB32 000b0000
    cram_gfxflash_bank_w MSB32 000c0000  // first half of gfx 10 + 11
    cram_gfxflash_bank_w MSB32 000d0000
    cram_gfxflash_bank_w MSB32 000e0000  // first half of gfx 12 + 13
    cram_gfxflash_bank_w MSB32 000f0000
    cram_gfxflash_bank_w MSB32 00100000  // first half of gfx 14 + 15
    cram_gfxflash_bank_w MSB32 00110000
    SIMM 5 (Rom 50/51)
    cram_gfxflash_bank_w MSB32 00120000  // first half of gfx 16 + 17
    cram_gfxflash_bank_w MSB32 00130000
    cram_gfxflash_bank_w MSB32 00140000  // first half of gfx 18 + 19
    cram_gfxflash_bank_w MSB32 00150000
    cram_gfxflash_bank_w MSB32 00160000  // first half of gfx 20 + 21
    cram_gfxflash_bank_w MSB32 00170000
    cram_gfxflash_bank_w MSB32 00180000  // first half of gfx 22 + 23
    cram_gfxflash_bank_w MSB32 00190000
    SIMM 6 (Rom 60/61)
    cram_gfxflash_bank_w MSB32 001a0000  // first half of gfx 24 + 25
    cram_gfxflash_bank_w MSB32 001b0000
    cram_gfxflash_bank_w MSB32 001c0000  // first half of gfx 26 + 27
    cram_gfxflash_bank_w MSB32 001d0000
    cram_gfxflash_bank_w MSB32 001e0000  // first half of gfx 28 + 29
    cram_gfxflash_bank_w MSB32 001f0000
    cram_gfxflash_bank_w MSB32 00200000  // first half of gfx 30 + 31
    cram_gfxflash_bank_w MSB32 00210000
    SIMM 7 (Rom 70/71) ** NOT USED (would follow on in sequence tho)

    */
		cram_gfxflash_bank = (data & 0xffff0000) >> 16;
		cram_gfxflash_bank-= 0x0002;// as with sound access etc. first 4 meg is 'special' and skipped
	}

	if (ACCESSING_BITS_0_7)
	{
	/*  if(DEBUG_PRINTF)*/ printf("cram_gfxflash_bank_LSB_w LSB32 %08x\n",data);
	}
}

// this seems to be dma active flags, and maybe vblank... not if it is anything else
static READ32_HANDLER( cps3_vbl_r )
{
	return 0x00000000;
}

static READ32_HANDLER( cps3_unk_io_r )
{
	//  warzard will crash before booting if you return anything here
	return 0xffffffff;
}

static READ32_HANDLER( cps3_40C0000_r )
{
	return 0x00000000;
}

static READ32_HANDLER( cps3_40C0004_r )
{
	return 0x00000000;
}

/* EEPROM access is a little odd, I think it accesses eeprom through some kind of
   additional interface, as these writes aren't normal for the type of eeprom we have */
static UINT16 cps3_current_eeprom_read;

static READ32_HANDLER( cps3_eeprom_r )
{
	int addr = offset*4;

	if (addr>=0x100 && addr<=0x17f)
	{
		if (ACCESSING_BITS_24_31) cps3_current_eeprom_read = (cps3_eeprom[offset-0x100/4] & 0xffff0000)>>16;
		else cps3_current_eeprom_read = (cps3_eeprom[offset-0x100/4] & 0x0000ffff)>>0;
		// read word to latch...
		return 0x00000000;
	}
	else if (addr == 0x200)
	{
		// busy flag / read data..
		if (ACCESSING_BITS_24_31) return 0;
		else
		{
			//if(DEBUG_PRINTF) printf("reading %04x from eeprom\n", cps3_current_eeprom_read);
			return cps3_current_eeprom_read;
		}
	}
	else
	{
	//  if(DEBUG_PRINTF) printf("unk read eeprom addr %04x, mask %08x\n", addr, mem_mask);
		return 0x00000000;
	}
	return 0x00000000;
}

static WRITE32_HANDLER( cps3_eeprom_w )
{
	int addr = offset*4;

	if (addr>=0x080 && addr<=0x0ff)
	{
		offset -= 0x80/4;
		COMBINE_DATA(&cps3_eeprom[offset]);
		// write word to storage

	}
	else if (addr>=0x180 && addr<=0x1ff)
	{
		// always 00000000 ? incrememnt access?
	}
	else
	{
	//  if(DEBUG_PRINTF) printf("unk write eeprom addr %04x, data %08x, mask %08x\n", addr, data, mem_mask);
	}

}

static READ32_HANDLER( cps3_cdrom_r )
{
	UINT32 retval = 0;

	if (ACCESSING_BITS_24_31)
	{
		retval |= ((UINT16)wd33c93_r(space,0))<<16;
	}

	if (ACCESSING_BITS_0_7)
	{
		retval |= (UINT16)wd33c93_r(space,1);
	}

	return retval;
}

static WRITE32_HANDLER( cps3_cdrom_w )
{
	if (ACCESSING_BITS_24_31)
	{
		wd33c93_w(space,0,(data & 0x00ff0000)>>16);
	}

	if (ACCESSING_BITS_0_7)
	{
		wd33c93_w(space,1,(data & 0x000000ff)>>0);
	}
}

static WRITE32_HANDLER( cps3_ss_bank_base_w )
{
	// might be scroll registers or something else..
	// used to display bank with 'insert coin' on during sfiii2 attract intro
	COMBINE_DATA(&cps3_ss_bank_base);

//  printf("cps3_ss_bank_base_w %08x %08x\n", data, mem_mask);
}

static WRITE32_HANDLER( cps3_ss_pal_base_w )
{
	 if(DEBUG_PRINTF) printf ("cps3_ss_pal_base_w %08x %08x\n", data, mem_mask);

	if(ACCESSING_BITS_24_31)
	{
		cps3_ss_pal_base = (data & 0x00ff0000)>>16;

		if (data & 0xff000000) printf("cps3_ss_pal_base MSB32 upper bits used %04x \n", data);
	}
	else
	{
	//  printf("cps3_ss_pal_base LSB32 used %04x \n", data);
	}
}

//<ElSemi> +0 X  +2 Y +4 unknown +6 enable (&0x8000) +8 low part tilemap base, high part linescroll base
//<ElSemi> (a word each)

static UINT32 paldma_source;
static UINT32 paldma_realsource;
static UINT32 paldma_dest;
static UINT32 paldma_fade;
static UINT32 paldma_other2;
static UINT32 paldma_length;

static WRITE32_HANDLER( cps3_palettedma_w )
{
	if (offset==0)
	{
		COMBINE_DATA(&paldma_source);
		paldma_realsource = (paldma_source<<1)-0x400000;
	}
	else if (offset==1)
	{
		COMBINE_DATA(&paldma_dest);
	}
	else if (offset==2)
	{
		COMBINE_DATA(&paldma_fade);
	}
	else if (offset==3)
	{
		COMBINE_DATA(&paldma_other2);

		if (ACCESSING_BITS_24_31)
		{
			paldma_length = (data & 0xffff0000)>>16;
		}
		if (ACCESSING_BITS_0_7)
		{
			if (data & 0x0002)
			{
				int i;
				UINT16* src = (UINT16*)cps3_user5region;
			//  if(DEBUG_PRINTF) printf("CPS3 pal dma start %08x (real: %08x) dest %08x fade %08x other2 %08x (length %04x)\n", paldma_source, paldma_realsource, paldma_dest, paldma_fade, paldma_other2, paldma_length);

				for (i=0;i<paldma_length;i++)
				{
					UINT16 coldata = src[BYTE_XOR_BE(((paldma_realsource>>1)+i))];

					//if (paldma_fade!=0) printf("%08x\n",paldma_fade);

					cps3_set_mame_colours(space->machine, (paldma_dest+i)^1, coldata, paldma_fade);
				}


				cputag_set_input_line(space->machine, "maincpu", 10, ASSERT_LINE);


			}
		}
	}

}

static UINT32 chardma_source;
static UINT32 chardma_other;

//static UINT8* current_table;
static UINT32 current_table_address;

static int cps3_rle_length = 0;

static int last_normal_byte = 0;


static UINT32 process_byte( running_machine *machine, UINT8 real_byte, UINT32 destination, int max_length )
{
	UINT8* dest       = (UINT8*)cps3_char_ram;

	//printf("process byte for destination %08x\n", destination);

	destination&=0x7fffff;

	if (real_byte&0x40)
	{
		int tranfercount = 0;

		//printf("Set RLE Mode\n");
		cps3_rle_length = (real_byte&0x3f)+1;

		//printf("RLE Operation (length %08x\n", cps3_rle_length );

		while (cps3_rle_length)
		{
			dest[((destination+tranfercount)&0x7fffff)^3] = (last_normal_byte&0x3f);
			gfx_element_mark_dirty(machine->gfx[1], ((destination+tranfercount)&0x7fffff)/0x100);
			//printf("RLE WRite Byte %08x, %02x\n", destination+tranfercount, real_byte);

			tranfercount++;
			cps3_rle_length--;
			max_length--;

			if ((destination+tranfercount) > 0x7fffff)  return max_length;


	//      if (max_length==0) return max_length; // this is meant to abort the transfer if we exceed dest length,, not working
		}
		return tranfercount;
	}
	else
	{
		//printf("Write Normal Data\n");
		dest[(destination&0x7fffff)^3] = real_byte;
		last_normal_byte = real_byte;
		gfx_element_mark_dirty(machine->gfx[1], (destination&0x7fffff)/0x100);
		return 1;
	}
}

static void cps3_do_char_dma( running_machine *machine, UINT32 real_source, UINT32 real_destination, UINT32 real_length )
{
	UINT8* sourcedata = (UINT8*)cps3_user5region;
	int length_remaining;

	last_normal_byte = 0;
	cps3_rle_length = 0;
	length_remaining = real_length;
	while (length_remaining)
	{
		UINT8 current_byte;

		current_byte = sourcedata[DMA_XOR(real_source)];
		real_source++;

		if (current_byte & 0x80)
		{
			UINT8 real_byte;
			UINT32 length_processed;
			current_byte &= 0x7f;

			real_byte = sourcedata[DMA_XOR((current_table_address+current_byte*2+0))];
			//if (real_byte&0x80) return;
			length_processed = process_byte( machine, real_byte, real_destination, length_remaining );
			length_remaining-=length_processed; // subtract the number of bytes the operation has taken
			real_destination+=length_processed; // add it onto the destination
			if (real_destination>0x7fffff) return;
			if (length_remaining<=0) return; // if we've expired, exit

			real_byte = sourcedata[DMA_XOR((current_table_address+current_byte*2+1))];
			//if (real_byte&0x80) return;
			length_processed = process_byte( machine, real_byte, real_destination, length_remaining );
			length_remaining-=length_processed; // subtract the number of bytes the operation has taken
			real_destination+=length_processed; // add it onto the destination
			if (real_destination>0x7fffff) return;
			if (length_remaining<=0) return;  // if we've expired, exit
		}
		else
		{
			UINT32 length_processed;
			length_processed = process_byte( machine, current_byte, real_destination, length_remaining );
			length_remaining-=length_processed; // subtract the number of bytes the operation has taken
			real_destination+=length_processed; // add it onto the destination
			if (real_destination>0x7fffff) return;
			if (length_remaining<=0) return;  // if we've expired, exit
		}

//      length_remaining--;
	}
}

static unsigned short lastb;
static unsigned short lastb2;
static UINT32 ProcessByte8(running_machine *machine,UINT8 b,UINT32 dst_offset)
{
	UINT8* destRAM = (UINT8*)cps3_char_ram;
	int l=0;

	if(lastb==lastb2)	//rle
	{
		int i;
		int rle=(b+1)&0xff;

		for(i=0;i<rle;++i)
		{
			destRAM[(dst_offset&0x7fffff)^3] = lastb;
			gfx_element_mark_dirty(machine->gfx[1], (dst_offset&0x7fffff)/0x100);

			dst_offset++;
			++l;
		}
		lastb2=0xffff;

		return l;
	}
	else
	{
		lastb2=lastb;
		lastb=b;
		destRAM[(dst_offset&0x7fffff)^3] = b;
		gfx_element_mark_dirty(machine->gfx[1], (dst_offset&0x7fffff)/0x100);
		return 1;
	}
}

static void cps3_do_alt_char_dma( running_machine *machine, UINT32 src, UINT32 real_dest, UINT32 real_length )
{
	UINT8* px = (UINT8*)cps3_user5region;
	UINT32 start = real_dest;
	UINT32 ds = real_dest;

	lastb=0xfffe;
	lastb2=0xffff;

	while(1)
	{
		int i;
		UINT8 ctrl=px[DMA_XOR(src)];
		++src;

		for(i=0;i<8;++i)
		{
			UINT8 p=px[DMA_XOR(src)];

			if(ctrl&0x80)
			{
				UINT8 real_byte;
				p&=0x7f;
				real_byte = px[DMA_XOR((current_table_address+p*2+0))];
				ds+=ProcessByte8(machine,real_byte,ds);
				real_byte = px[DMA_XOR((current_table_address+p*2+1))];
				ds+=ProcessByte8(machine,real_byte,ds);
			}
			else
			{
				ds+=ProcessByte8(machine,p,ds);
			}
			++src;
			ctrl<<=1;

			if((ds-start)>=real_length)
				return;
		}
	}
}

static void cps3_process_character_dma(running_machine *machine, UINT32 address)
{
	int i;

	//printf("charDMA start:\n");

	for (i = 0; i < 0x1000; i += 3)
	{
		UINT32 dat1 = LITTLE_ENDIANIZE_INT32(cps3_char_ram[i + 0 + (address)]);
		UINT32 dat2 = LITTLE_ENDIANIZE_INT32(cps3_char_ram[i + 1 + (address)]);
		UINT32 dat3 = LITTLE_ENDIANIZE_INT32(cps3_char_ram[i + 2 + (address)]);
		UINT32 real_source      = (dat3 << 1) - 0x400000;
		UINT32 real_destination =  dat2 << 3;
		UINT32 real_length      = (((dat1 & 0x001fffff) + 1) << 3);

		/* 0x01000000 is the end of list marker, 0x13131313 is our default fill */
		if ((dat1 == 0x01000000) || (dat1 == 0x13131313)) break;

        //printf("%08x %08x %08x real_source %08x (rom %d offset %08x) real_destination %08x, real_length %08x\n", dat1, dat2, dat3, real_source, real_source/0x800000, real_source%0x800000, real_destination, real_length);

		if  ((dat1 & 0x00e00000) == 0x00800000)
		{
			/* Sets a table used by the decompression routines */
			{
				/* We should probably copy this, but a pointer to it is fine for our purposes as the data doesn't change */
				current_table_address = real_source;
			}
			cputag_set_input_line(machine, "maincpu", 10, ASSERT_LINE);
		}
		else if  ((dat1 & 0x00e00000) == 0x00400000)
		{
			/* 6bpp DMA decompression
              - this is used for the majority of sprites and backgrounds */
			cps3_do_char_dma( machine, real_source, real_destination, real_length );
			cputag_set_input_line(machine, "maincpu", 10, ASSERT_LINE);

		}
		else if  ((dat1 & 0x00e00000) == 0x00600000)
		{
			/* 8bpp DMA decompression
              - this is used on SFIII NG Sean's Stage ONLY */
			cps3_do_alt_char_dma( machine, real_source, real_destination, real_length);
			cputag_set_input_line(machine, "maincpu", 10, ASSERT_LINE);
		}
		else
		{
			printf("Unknown DMA List Command Type\n"); // warzard uses command 0, uncompressed? but for what?
		}

	}
}

static WRITE32_HANDLER( cps3_characterdma_w )
{
	if(DEBUG_PRINTF) printf("chardma_w %08x %08x %08x\n", offset, data, mem_mask);

	if (offset==0)
	{
		//COMBINE_DATA(&chardma_source);
		if (ACCESSING_BITS_0_7)
		{
			chardma_source = data & 0x0000ffff;
		}
		if (ACCESSING_BITS_24_31)
		{
			if(DEBUG_PRINTF) printf("chardma_w accessing MSB32 of offset 0");
		}
	}
	else if (offset==1)
	{
		COMBINE_DATA(&chardma_other);

		if (ACCESSING_BITS_24_31)
		{
			if ((data>>16) & 0x0040)
			{
				UINT32 list_address;
				list_address = (chardma_source | ((chardma_other&0x003f0000)));

				//printf("chardma_w activated %08x %08x (address = cram %08x)\n", chardma_source, chardma_other, list_address*4 );
				cps3_process_character_dma(space->machine, list_address);
			}
			else
			{
				if(DEBUG_PRINTF) printf("chardma_w NOT activated %08x %08x\n", chardma_source, chardma_other );
			}

			if ((data>>16) & 0xff80)
				if(DEBUG_PRINTF) printf("chardma_w unknown bits in activate command %08x %08x\n", chardma_source, chardma_other );
		}
		else
		{
			if(DEBUG_PRINTF) printf("chardma_w LSB32 write to activate command %08x %08x\n", chardma_source, chardma_other );
		}
	}
}

static WRITE32_HANDLER( cps3_irq10_ack_w )
{
	cputag_set_input_line(space->machine, "maincpu", 10, CLEAR_LINE); return;
}

static WRITE32_HANDLER( cps3_irq12_ack_w )
{
	cputag_set_input_line(space->machine, "maincpu", 12, CLEAR_LINE); return;
}

static WRITE32_HANDLER( cps3_unk_vidregs_w )
{
	COMBINE_DATA(&cps3_unk_vidregs[offset]);
}

static READ32_HANDLER( cps3_colourram_r )
{
	UINT16* src = (UINT16*)cps3_colourram;

	return src[offset*2+1] | (src[offset*2+0]<<16);
}

static WRITE32_HANDLER( cps3_colourram_w )
{
//  COMBINE_DATA(&cps3_colourram[offset]);

	if (ACCESSING_BITS_24_31)
	{
		cps3_set_mame_colours(space->machine, offset*2, (data & 0xffff0000) >> 16, 0);
	}

	if (ACCESSING_BITS_0_7)
	{
		cps3_set_mame_colours(space->machine, offset*2+1, (data & 0x0000ffff) >> 0, 0);
	}
}

static UINT32* cps3_mainram;

/* there are more unknown writes, but you get the idea */
static ADDRESS_MAP_START( cps3_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0007ffff) AM_ROM AM_REGION("user1", 0) // Bios ROM
	AM_RANGE(0x02000000, 0x0207ffff) AM_RAM AM_BASE(&cps3_mainram) // Main RAM

	AM_RANGE(0x03000000, 0x030003ff) AM_RAM // 'FRAM' (SFIII memory test mode ONLY)

//  AM_RANGE(0x04000000, 0x0407dfff) AM_RAM AM_BASE(&cps3_spriteram)//AM_WRITEONLY // Sprite RAM (jojoba tests this size)
	AM_RANGE(0x04000000, 0x0407ffff) AM_RAM AM_BASE(&cps3_spriteram)//AM_WRITEONLY // Sprite RAM

	AM_RANGE(0x04080000, 0x040bffff) AM_READWRITE(cps3_colourram_r, cps3_colourram_w) AM_BASE(&cps3_colourram)  // Colour RAM (jojoba tests this size) 0x20000 colours?!

	// video registers of some kind probably
	AM_RANGE(0x040C0000, 0x040C0003) AM_READ(cps3_40C0000_r)//?? every frame
	AM_RANGE(0x040C0004, 0x040C0007) AM_READ(cps3_40C0004_r)//AM_READ(cps3_40C0004_r) // warzard reads this!
//  AM_RANGE(0x040C0008, 0x040C000b) AM_WRITENOP//??
    AM_RANGE(0x040C000c, 0x040C000f) AM_READ(cps3_vbl_r)// AM_WRITENOP/

	AM_RANGE(0x040C0000, 0x040C001f) AM_WRITE(cps3_unk_vidregs_w)
	AM_RANGE(0x040C0020, 0x040C002b) AM_WRITEONLY AM_BASE(&tilemap20_regs_base)
	AM_RANGE(0x040C0030, 0x040C003b) AM_WRITEONLY AM_BASE(&tilemap30_regs_base)
	AM_RANGE(0x040C0040, 0x040C004b) AM_WRITEONLY AM_BASE(&tilemap40_regs_base)
	AM_RANGE(0x040C0050, 0x040C005b) AM_WRITEONLY AM_BASE(&tilemap50_regs_base)

	AM_RANGE(0x040C0060, 0x040C007f) AM_RAM AM_BASE(&cps3_fullscreenzoom)


	AM_RANGE(0x040C0094, 0x040C009b) AM_WRITE(cps3_characterdma_w)


	AM_RANGE(0x040C00a0, 0x040C00af) AM_WRITE(cps3_palettedma_w)


	AM_RANGE(0x040C0084, 0x040C0087) AM_WRITE(cram_bank_w)
	AM_RANGE(0x040C0088, 0x040C008b) AM_WRITE(cram_gfxflash_bank_w)

	AM_RANGE(0x040e0000, 0x040e02ff) AM_READWRITE(cps3_sound_r, cps3_sound_w)

	AM_RANGE(0x04100000, 0x041fffff) AM_READWRITE(cram_data_r, cram_data_w)
	AM_RANGE(0x04200000, 0x043fffff) AM_READWRITE(cps3_gfxflash_r, cps3_gfxflash_w) // GFX Flash ROMS

	AM_RANGE(0x05000000, 0x05000003) AM_READ_PORT("INPUTS")
	AM_RANGE(0x05000004, 0x05000007) AM_READ_PORT("EXTRA")

	AM_RANGE(0x05000008, 0x0500000b) AM_WRITENOP // ?? every frame

	AM_RANGE(0x05000a00, 0x05000a1f) AM_READ( cps3_unk_io_r ) // ?? every frame

	AM_RANGE(0x05001000, 0x05001203) AM_READWRITE( cps3_eeprom_r, cps3_eeprom_w )

	AM_RANGE(0x05040000, 0x0504ffff) AM_READWRITE(cps3_ssram_r,cps3_ssram_w) // 'SS' RAM (Score Screen) (text tilemap + toles)
	//0x25050020
	AM_RANGE(0x05050020, 0x05050023) AM_WRITE( cps3_ss_bank_base_w )
	AM_RANGE(0x05050024, 0x05050027) AM_WRITE( cps3_ss_pal_base_w )

	AM_RANGE(0x05100000, 0x05100003) AM_WRITE( cps3_irq12_ack_w )
	AM_RANGE(0x05110000, 0x05110003) AM_WRITE( cps3_irq10_ack_w )

	AM_RANGE(0x05140000, 0x05140003) AM_READWRITE( cps3_cdrom_r, cps3_cdrom_w )

	AM_RANGE(0x06000000, 0x067fffff) AM_READWRITE( cps3_flash1_r, cps3_flash1_w ) /* Flash ROMs simm 1 */
	AM_RANGE(0x06800000, 0x06ffffff) AM_READWRITE( cps3_flash2_r, cps3_flash2_w ) /* Flash ROMs simm 2 */

	AM_RANGE(0xc0000000, 0xc00003ff) AM_RAM_WRITE( cps3_0xc0000000_ram_w ) AM_BASE(&cps3_0xc0000000_ram) /* Executes code from here */
ADDRESS_MAP_END




static INPUT_PORTS_START( cps3 )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED ) // nothing here?
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNUSED ) // nothing here?
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00020000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00fc0000, IP_ACTIVE_LOW, IPT_UNUSED ) // nothing here?
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) // nothing here?
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0000000, IP_ACTIVE_LOW, IPT_UNUSED ) // nothing here?

	PORT_START("EXTRA")
	PORT_BIT( 0x0001ffff, IP_ACTIVE_LOW, IPT_UNUSED ) // nothing here?
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0xffc00000, IP_ACTIVE_LOW, IPT_UNUSED ) // nothing here?
INPUT_PORTS_END

static INTERRUPT_GEN(cps3_vbl_interrupt)
{
	cpu_set_input_line(device,12, ASSERT_LINE);
}

static INTERRUPT_GEN(cps3_other_interrupt)
{
	// this seems to need to be periodic (see the life bar portraits in sfiii2
	// but also triggered on certain dma events (or warzard locks up in attract)
	// what is the REAL source of IRQ10??
	cpu_set_input_line(device,10, ASSERT_LINE);
}


//static sh2_cpu_core sh2cp_conf_slave  = { 1 };


static const SCSIConfigTable dev_table =
{
	1,                                      /* 1 SCSI device */
	{ { SCSI_ID_1, "cdrom", SCSI_DEVICE_CDROM } } /* SCSI ID 2, using CD 0, and it's a CD-ROM */
};

static const struct WD33C93interface scsi_intf =
{
	&dev_table,		/* SCSI device table */
	NULL			/* command completion IRQ */
};

static void cps3_exit(running_machine *machine)
{
	wd33c93_exit(&scsi_intf);
}

static MACHINE_START( cps3 )
{
	wd33c93_init(machine, &scsi_intf);
	add_exit_callback(machine, cps3_exit);
}

static MACHINE_RESET( cps3 )
{
	current_table_address = -1;
}





static void precopy_to_flash(running_machine *machine)
{
	UINT32* romdata = (UINT32*)cps3_user4region;
	int i;
	/* precopy program roms, ok, sfiii2 tests pass, others fail because of how the decryption affects testing */
	for (i=0;i<0x800000;i+=4)
	{
		UINT32 data;
		UINT8* ptr1 = (UINT8*)intelflash_getmemptr(0);
		UINT8* ptr2 = (UINT8*)intelflash_getmemptr(1);
		UINT8* ptr3 = (UINT8*)intelflash_getmemptr(2);
		UINT8* ptr4 = (UINT8*)intelflash_getmemptr(3);

		data = romdata[i/4];

		ptr1[i/4] = (data & 0xff000000)>>24;
		ptr2[i/4] = (data & 0x00ff0000)>>16;
		ptr3[i/4] = (data & 0x0000ff00)>>8;
		ptr4[i/4] = (data & 0x000000ff)>>0;
	}

	for (i=0;i<0x800000;i+=4)
	{
		UINT32 data;
		UINT8* ptr1 = (UINT8*)intelflash_getmemptr(4);
		UINT8* ptr2 = (UINT8*)intelflash_getmemptr(5);
		UINT8* ptr3 = (UINT8*)intelflash_getmemptr(6);
		UINT8* ptr4 = (UINT8*)intelflash_getmemptr(7);

		data = romdata[(0x800000+i)/4];

		ptr1[i/4] = (data & 0xff000000)>>24;
		ptr2[i/4] = (data & 0x00ff0000)>>16;
		ptr3[i/4] = (data & 0x0000ff00)>>8;
		ptr4[i/4] = (data & 0x000000ff)>>0;
	}

	/* precopy gfx roms, good, tests pass */
	{
		UINT32 thebase, len = USER5REGION_LENGTH;
		int flashnum = 8;

		romdata = (UINT32*)cps3_user5region;
		for (thebase = 0;thebase < len/2; thebase+=0x200000)
		{
		//  printf("flashnums %d. %d\n",flashnum, flashnum+1);

			for (i=0;i<0x200000;i+=2)
			{
				UINT8* ptr1 = (UINT8*)intelflash_getmemptr(flashnum);
				UINT8* ptr2 = (UINT8*)intelflash_getmemptr(flashnum+1);
				UINT32 dat = romdata[(thebase+i)/2];

				ptr1[BYTE_XOR_LE(i+1)] =  (dat&0xff000000)>>24;
				ptr2[BYTE_XOR_LE(i+1)] =  (dat&0x00ff0000)>>16;

				ptr1[BYTE_XOR_LE(i+0)] =  (dat&0x0000ff00)>>8;
				ptr2[BYTE_XOR_LE(i+0)] =  (dat&0x000000ff)>>0;
			}
			flashnum+=2;
		}
	}
}


// make a copy in the regions we execute code / draw gfx from
static void copy_from_nvram(running_machine *machine)
{
	UINT32* romdata = (UINT32*)cps3_user4region;
	UINT32* romdata2 = (UINT32*)decrypted_gamerom;
	int i;
	/* copy + decrypt program roms which have been loaded from flashroms/nvram */
	for (i=0;i<0x800000;i+=4)
	{
		UINT32 data;
		UINT8* ptr1 = (UINT8*)intelflash_getmemptr(0);
		UINT8* ptr2 = (UINT8*)intelflash_getmemptr(1);
		UINT8* ptr3 = (UINT8*)intelflash_getmemptr(2);
		UINT8* ptr4 = (UINT8*)intelflash_getmemptr(3);

		data = ((ptr1[i/4]<<24) | (ptr2[i/4]<<16) | (ptr3[i/4]<<8) | (ptr4[i/4]<<0));

	//  printf("%08x %08x %08x %08x\n",romdata[i/4],data, romdata2[i/4], data ^ cps3_mask(i+0x6000000, cps3_key1, cps3_key2));
		romdata[i/4] = data;
		romdata2[i/4] = data ^ cps3_mask(i+0x6000000, cps3_key1, cps3_key2);

	}

	romdata  += 0x800000/4;
	romdata2 += 0x800000/4;

	for (i=0;i<0x800000;i+=4)
	{
		UINT32 data;
		UINT8* ptr1 = (UINT8*)intelflash_getmemptr(4);
		UINT8* ptr2 = (UINT8*)intelflash_getmemptr(5);
		UINT8* ptr3 = (UINT8*)intelflash_getmemptr(6);
		UINT8* ptr4 = (UINT8*)intelflash_getmemptr(7);

		data = ((ptr1[i/4]<<24) | (ptr2[i/4]<<16) | (ptr3[i/4]<<8) | (ptr4[i/4]<<0));

	//  printf("%08x %08x %08x %08x\n",romdata[i/4],data, romdata2[i/4],  data ^ cps3_mask(i+0x6800000, cps3_key1, cps3_key2) );
		romdata[i/4] = data;
		romdata2[i/4] = data ^ cps3_mask(i+0x6800000, cps3_key1, cps3_key2);
	}

	/* copy gfx from loaded flashroms to user reigon 5, where it's used */
	{
		UINT32 thebase, len = USER5REGION_LENGTH;
		int flashnum = 8;
		int countoffset = 0;

		romdata = (UINT32*)cps3_user5region;
		for (thebase = 0;thebase < len/2; thebase+=0x200000)
		{
		//  printf("flashnums %d. %d\n",flashnum, flashnum+1);

			for (i=0;i<0x200000;i+=2)
			{
				UINT8* ptr1 = (UINT8*)intelflash_getmemptr(flashnum);
				UINT8* ptr2 = (UINT8*)intelflash_getmemptr(flashnum+1);
				UINT32 dat = (ptr1[i+0]<<8) |
					         (ptr1[i+1]<<24) |
							 (ptr2[i+0]<<0) |
							 (ptr2[i+1]<<16);

				//printf("%08x %08x\n",romdata[countoffset],dat);
				romdata[countoffset] = dat;

				countoffset++;
			}
			flashnum+=2;
		}
	}


	/*
    {
        FILE *fp;
        const char *gamename = machine->gamedrv->name;
        char filename[256];
        sprintf(filename, "%s_bios.dump", gamename);

        fp=fopen(filename, "w+b");
        if (fp)
        {
            fwrite(rom, 0x080000, 1, fp);
            fclose(fp);
        }
    }
    */

}

static NVRAM_HANDLER( cps3 )
{
	int i;

	if (read_or_write)
	{
		//printf("read_write\n");
		mame_fwrite(file, cps3_eeprom, 0x400);
		for (i=0;i<48;i++)
			nvram_handler_intelflash( machine, i, file, read_or_write );
	}
	else if (file)
	{
		//printf("file\n");
		mame_fread(file, cps3_eeprom, 0x400);
		for (i=0;i<48;i++)
			nvram_handler_intelflash( machine, i, file, read_or_write );

		copy_from_nvram(machine); // copy data from flashroms back into user regions + decrypt into regions we execute/draw from.
	}
	else
	{
		//printf("nothing?\n");
		precopy_to_flash(machine);  // attempt to copy data from user regions into flash roms (incase this is a NOCD set)
		copy_from_nvram(machine); // copy data from flashroms back into user regions + decrypt into regions we execute/draw from.
	}


}




static int cps3_dma_callback(UINT32 src, UINT32 dst, UINT32 data, int size)
{
	/*
      on the actual CPS3 hardware the SH2 DMA bypasses the encryption.

      to handle this in MAME we use this callback, and reverse the effect of the
      encryption that would otherwise be applied.  this allows us to avoid per-game,
      per-PC hacks.  this approach is however still a little messy.

    */

	/* I doubt this is endian safe.. needs checking / fixing */
	if (size==0)
	{
		if ((src&3)==0) data <<=24;
		if ((src&3)==1) data <<=16;
		if ((src&3)==2) data <<=8;
		if ((src&3)==3) data <<=0;
	}


	if (src<0x80000)
	{
		int offs = (src&0x07ffff)>>2;
		data = data ^ cps3_mask(offs*4, cps3_key1, cps3_key2);
	}
	else if (src>=0x6000000 && src<0x6800000)
	{
		int offs = (src&0x07fffff)>>2;
		if (!cps3_altEncryption) data = data ^ cps3_mask(0x6000000+offs*4, cps3_key1, cps3_key2);
	}
	else if (src>=0x6800000 && src<0x7000000)
	{
		int offs = (src&0x07fffff)>>2;
		if (!cps3_altEncryption) data = data ^ cps3_mask(0x6800000+offs*4, cps3_key1, cps3_key2);
	}
	else
	{
		//printf("%s :src %08x, dst %08x, returning %08x\n", cpuexec_describe_context(machine), src, dst, data);
	}

	/* I doubt this is endian safe.. needs checking / fixing */
	if (size==0)
	{
		if ((src&3)==0) data >>=24;
		if ((src&3)==1)	data >>=16;
		if ((src&3)==2)	data >>=8;
		if ((src&3)==3) data >>=0;

		data &=0x000000ff;
	}

	return data;
}



static const sh2_cpu_core sh2_conf_cps3 = {
	0, // master
	cps3_dma_callback
};

static MACHINE_DRIVER_START( cps3 )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", SH2, 6250000*4) // external clock is 6.25 Mhz, it sets the intenral multiplier to 4x (this should probably be handled in the core..)
	MDRV_CPU_PROGRAM_MAP(cps3_map)
	MDRV_CPU_VBLANK_INT("screen", cps3_vbl_interrupt)
	MDRV_CPU_PERIODIC_INT(cps3_other_interrupt,80) /* ?source? */
	MDRV_CPU_CONFIG(sh2_conf_cps3)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(XTAL_60MHz/8, 486, 0, 384, 259, 0, 224)
/*
    Measured clocks:
        V = 59.5992Hz
        H = 15.4335kHz
        H/V = 258.955 ~ 259 lines

    Possible video clocks:
        60MHz       / 15.4335kHz = 3887.647 / 8 = 485.956 ~ 486 -> likely
         42.9545MHz / 15.4445kHz = 2781.217 / 6 = 463.536 -> unlikely
*/

	MDRV_MACHINE_START(cps3)
	MDRV_MACHINE_RESET(cps3)
	MDRV_NVRAM_HANDLER( cps3 )
	MDRV_PALETTE_LENGTH(0x10000) // actually 0x20000 ...

	MDRV_VIDEO_START(cps3)
	MDRV_VIDEO_UPDATE(cps3)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("cps3", CPS3, MASTER_CLOCK / 3)
	MDRV_SOUND_ROUTE(1, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(0, "rspeaker", 1.0)
MACHINE_DRIVER_END


ROM_START( sfiii )
	ROM_REGION32_BE( 0x080000, "user1", 0 ) /* bios region */
	ROM_LOAD( "sfiii_usa.29f400.u2", 0x000000, 0x080000, CRC(fb172a8e) SHA1(48ebf59910f246835f7dc0c588da30f7a908072f) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "sf3000", 0, SHA1(606e62cc5f46275e366e7dbb412dbaeb7e54cd0c) )
ROM_END

ROM_START( sfiiij )
	ROM_REGION32_BE( 0x080000, "user1", 0 ) /* bios region */
	ROM_LOAD( "sfiii_japan.29f400.u2", 0x000000, 0x080000, CRC(74205250) SHA1(c3e83ace7121d32da729162662ec6b5285a31211) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "sf3000", 0, SHA1(606e62cc5f46275e366e7dbb412dbaeb7e54cd0c) )
ROM_END

ROM_START( sfiii2 )
	ROM_REGION32_BE( 0x080000, "user1", 0 ) /* bios region */
	ROM_LOAD( "sfiii2_usa.29f400.u2", 0x000000, 0x080000, CRC(75dd72e0) SHA1(5a12d6ea6734df5de00ecee6f9ef470749d2f242) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "3ga000", 0, SHA1(4e162885b0b3265a56e0265037bcf247e820f027) )
ROM_END

ROM_START( sfiii2j )
	ROM_REGION32_BE( 0x080000, "user1", 0 ) /* bios region */
	ROM_LOAD( "sfiii2_japan.29f400.u2", 0x000000, 0x080000, CRC(faea0a3e) SHA1(a03cd63bcf52e4d57f7a598c8bc8e243694624ec) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "3ga000", 0, SHA1(4e162885b0b3265a56e0265037bcf247e820f027) )
ROM_END

ROM_START( sfiii3 )
	ROM_REGION32_BE( 0x080000, "user1", 0 ) /* bios region */
	ROM_LOAD( "sfiii3_usa.29f400.u2", 0x000000, 0x080000, CRC(ecc545c1) SHA1(e39083820aae914fd8b80c9765129bedb745ceba) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "cap-33s-2", 0, SHA1(41b0e246db91cbfc3f8f0f62d981734feb4b4ab5) )
ROM_END

ROM_START( sfiii3a )
	ROM_REGION32_BE( 0x080000, "user1", 0 ) /* bios region */
	ROM_LOAD( "sfiii3_usa.29f400.u2", 0x000000, 0x080000, CRC(ecc545c1) SHA1(e39083820aae914fd8b80c9765129bedb745ceba) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "33s000", 0, SHA1(2f4a9006a31903114f9f9dc09465ae253e565c51) )
ROM_END

ROM_START( redearth )
	ROM_REGION32_BE( 0x080000, "user1", 0 ) /* bios region */
	ROM_LOAD( "warzard_euro.29f400.u2", 0x000000, 0x080000, CRC(02e0f336) SHA1(acc37e830dfeb9674f5a0fb24f4cc23217ae4ff5) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "wzd000", 0, SHA1(e5676752b08283dc4a98c3d7b759e8aa6dcd0679) )
ROM_END

ROM_START( warzard )
	ROM_REGION32_BE( 0x080000, "user1", 0 ) /* bios region */
	ROM_LOAD( "warzard_japan.29f400.u2", 0x000000, 0x080000, CRC(f8e2f0c6) SHA1(93d6a986f44c211fff014e55681eca4d2a2774d6) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "wzd000", 0, SHA1(e5676752b08283dc4a98c3d7b759e8aa6dcd0679) )
ROM_END

ROM_START( jojo )
	ROM_REGION32_BE( 0x080000, "user1", 0 ) /* bios region */
	ROM_LOAD( "jojo_japan.29f400.u2", 0x000000, 0x080000, CRC(02778f60) SHA1(a167f9ebe030592a0cdb0c6a3c75835c6a43be4c) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "cap-jjk-160", 0, SHA1(0f5c09171409213e191a607ee89ca3a91fe9c96a) )
ROM_END

ROM_START( jojoa )
	ROM_REGION32_BE( 0x080000, "user1", 0 ) /* bios region */
	ROM_LOAD( "jojo_japan.29f400.u2", 0x000000, 0x080000, CRC(02778f60) SHA1(a167f9ebe030592a0cdb0c6a3c75835c6a43be4c) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "jjk000", 0, SHA1(09869f6d8c032b527e02d815749dc8fab1289e86) )
ROM_END

ROM_START( jojoba )
	ROM_REGION32_BE( 0x080000, "user1", 0 ) /* bios region */
	ROM_LOAD( "jojoba_japan.29f400.u2",  0x000000, 0x080000,  CRC(3085478c) SHA1(055eab1fc42816f370a44b17fd7e87ffcb10e8b7) )

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "jjm000", 0, SHA1(0678a0baeb853dcff1d230c14f0873cc9f143d7b) )
ROM_END


/* NO CD sets - use NO CD BIOS roms */

ROM_START( sfiiin )
	ROM_REGION32_BE( 0x080000, "user1", 0 ) /* bios region */
	ROM_LOAD( "sfiii_asia_nocd.29f400.u2", 0x000000, 0x080000, CRC(73e32463) SHA1(45d144e533e4b20cc5a744ca4f618e288430c601) )

	ROM_REGION32_BE( USER4REGION_LENGTH, "user4", ROMREGION_ERASEFF ) /* cd content region */
	ROM_LOAD( "10",  0x0000000, 0x800000, CRC(e896dc27) SHA1(47623820c64b72e69417afcafaacdd2c318cde1c) )
	ROM_REGION16_BE( USER5REGION_LENGTH, "user5", ROMREGION_ERASEFF ) /* cd content region */
	ROM_LOAD( "30",  0x0000000, 0x800000, CRC(98c2d07c) SHA1(604ce4a16170847c10bc233a47a47a119ce170f7) )
	ROM_LOAD( "31",  0x0800000, 0x800000, CRC(7115a396) SHA1(b60a74259e3c223138e66e68a3f6457694a0c639) )
	ROM_LOAD( "40",  0x1000000, 0x800000, CRC(839f0972) SHA1(844e43fcc157b7c774044408bfe918c49e174cdb) )
	ROM_LOAD( "41",  0x1800000, 0x800000, CRC(8a8b252c) SHA1(9ead4028a212c689d7a25746fbd656dca6f938e8) )
	ROM_LOAD( "50",  0x2000000, 0x400000, CRC(58933dc2) SHA1(1f1723be676a817237e96b6e20263b935c59daae) )
ROM_END

ROM_START( sfiii2n )
	ROM_REGION32_BE( 0x080000, "user1", 0 ) /* bios region */
	ROM_LOAD( "sfiii2_asia_nocd.29f400.u2", 0x000000, 0x080000, CRC(fd297c0d) SHA1(4323deda2789f104b53f32a663196ec16de73215) )

	ROM_REGION32_BE( USER4REGION_LENGTH, "user4", ROMREGION_ERASEFF ) /* cd content region */
	ROM_LOAD( "10",  0x0000000, 0x800000, CRC(682b014a) SHA1(abd5785f4b7c89584d6d1cf6fb61a77d7224f81f) )
	ROM_LOAD( "20",  0x0800000, 0x800000, CRC(38090460) SHA1(aaade89b8ccdc9154f97442ca35703ec538fe8be) )
	ROM_REGION16_BE( USER5REGION_LENGTH, "user5", ROMREGION_ERASEFF ) /* cd content region */
	ROM_LOAD( "30",  0x0000000, 0x800000, CRC(77c197c0) SHA1(944381161462e65de7ae63a656658f3fbe44727a) )
	ROM_LOAD( "31",  0x0800000, 0x800000, CRC(7470a6f2) SHA1(850b2e20afe8a5a1f0d212d9abe002cb5cf14d22) )
	ROM_LOAD( "40",  0x1000000, 0x800000, CRC(01a85ced) SHA1(802df3274d5f767636b2785606e0558f6d3b9f13) )
	ROM_LOAD( "41",  0x1800000, 0x800000, CRC(fb346d74) SHA1(57570f101a170aa7e83e84e4b7cbdc63a11a486a) )
	ROM_LOAD( "50",  0x2000000, 0x800000, CRC(32f79449) SHA1(44b1f2a640ab4abc23ff47e0edd87fbd0b345c06) )
	ROM_LOAD( "51",  0x2800000, 0x800000, CRC(1102b8eb) SHA1(c7dd2ee3a3214c6ec47a03fe3e8c941775d57f76) )
ROM_END

ROM_START( sfiii3n )
	ROM_REGION32_BE( 0x080000, "user1", 0 ) /* bios region */
	ROM_LOAD( "sfiii3_japan_nocd.29f400.u2", 0x000000, 0x080000, CRC(1edc6366) SHA1(60b4b9adeb030a33059d74fdf03873029e465b52) )

	ROM_REGION32_BE( USER4REGION_LENGTH, "user4", ROMREGION_ERASEFF ) /* cd content region */
	ROM_LOAD( "10",  0x0000000, 0x800000, CRC(ba7f76b2) SHA1(6b396596dea009b34af17484919ae37eda53ec65) )
	ROM_LOAD( "20",  0x0800000, 0x800000, CRC(5ca8faba) SHA1(71c12638ae7fa38b362d68c3ccb4bb3ccd67f0e9) )
	ROM_REGION16_BE( USER5REGION_LENGTH, "user5", ROMREGION_ERASEFF ) /* cd content region */
	ROM_LOAD( "30",  0x0000000, 0x800000, CRC(b37cf960) SHA1(60310f95e4ecedee85846c08ccba71e286cda73b) )
	ROM_LOAD( "31",  0x0800000, 0x800000, CRC(450ec982) SHA1(1cb3626b8479997c4f1b29c41c81cac038fac31b) )
	ROM_LOAD( "40",  0x1000000, 0x800000, CRC(632c965f) SHA1(9a46b759f5dee35411fd6446c2457c084a6dfcd8) )
	ROM_LOAD( "41",  0x1800000, 0x800000, CRC(7a4c5f33) SHA1(f33cdfe247c7caf9d3d394499712f72ca930705e) )
	ROM_LOAD( "50",  0x2000000, 0x800000, CRC(8562358e) SHA1(8ed78f6b106659a3e4d94f38f8a354efcbdf3aa7) )
	ROM_LOAD( "51",  0x2800000, 0x800000, CRC(7baf234b) SHA1(38feb45d6315d771de5f9ae965119cb25bae2a1e) )
	ROM_LOAD( "60",  0x3000000, 0x800000, CRC(bc9487b7) SHA1(bc2cd2d3551cc20aa231bba425ff721570735eba) )
	ROM_LOAD( "61",  0x3800000, 0x800000, CRC(b813a1b1) SHA1(16de0ee3dfd6bf33d07b0ff2e797ebe2cfe6589e) )
ROM_END

ROM_START( sfiii3an )
	ROM_REGION32_BE( 0x080000, "user1", 0 ) /* bios region */
	ROM_LOAD( "sfiii3_japan_nocd.29f400.u2", 0x000000, 0x080000, CRC(1edc6366) SHA1(60b4b9adeb030a33059d74fdf03873029e465b52) )

	ROM_REGION32_BE( USER4REGION_LENGTH, "user4", ROMREGION_ERASEFF ) /* cd content region */
	ROM_LOAD( "10",  0x0000000, 0x800000, CRC(77233d39) SHA1(59c3f890fdc33a7d8dc91e5f9c4e7b7019acfb00) )
	ROM_LOAD( "20",  0x0800000, 0x800000, CRC(5ca8faba) SHA1(71c12638ae7fa38b362d68c3ccb4bb3ccd67f0e9) )
	ROM_REGION16_BE( USER5REGION_LENGTH, "user5", ROMREGION_ERASEFF ) /* cd content region */
	ROM_LOAD( "30",  0x0000000, 0x800000, CRC(b37cf960) SHA1(60310f95e4ecedee85846c08ccba71e286cda73b) )
	ROM_LOAD( "31",  0x0800000, 0x800000, CRC(450ec982) SHA1(1cb3626b8479997c4f1b29c41c81cac038fac31b) )
	ROM_LOAD( "40",  0x1000000, 0x800000, CRC(632c965f) SHA1(9a46b759f5dee35411fd6446c2457c084a6dfcd8) )
	ROM_LOAD( "41",  0x1800000, 0x800000, CRC(7a4c5f33) SHA1(f33cdfe247c7caf9d3d394499712f72ca930705e) )
	ROM_LOAD( "50",  0x2000000, 0x800000, CRC(8562358e) SHA1(8ed78f6b106659a3e4d94f38f8a354efcbdf3aa7) )
	ROM_LOAD( "51",  0x2800000, 0x800000, CRC(7baf234b) SHA1(38feb45d6315d771de5f9ae965119cb25bae2a1e) )
	ROM_LOAD( "60",  0x3000000, 0x800000, CRC(bc9487b7) SHA1(bc2cd2d3551cc20aa231bba425ff721570735eba) )
	ROM_LOAD( "61",  0x3800000, 0x800000, CRC(b813a1b1) SHA1(16de0ee3dfd6bf33d07b0ff2e797ebe2cfe6589e) )
ROM_END

ROM_START( jojon )
	ROM_REGION32_BE( 0x080000, "user1", 0 ) /* bios region */
	ROM_LOAD( "jojo_asia_nocd.29f400.u2", 0x000000, 0x080000, CRC(05b4f953) SHA1(c746c7bb5359acc9adced817cb4870b1912eaefd) )

	ROM_REGION32_BE( USER4REGION_LENGTH, "user4", ROMREGION_ERASEFF ) /* cd content region */
	ROM_LOAD( "10",  0x0000000, 0x800000, CRC(bc612872) SHA1(18930f2906176b54a9b8bec56f06dda3362eb418) )
	ROM_LOAD( "20",  0x0800000, 0x800000, CRC(0e1daddf) SHA1(34bb4e0fb86258095a7b20f60174453195f3735a) )
	ROM_REGION16_BE( USER5REGION_LENGTH, "user5", ROMREGION_ERASEFF ) /* cd content region */
	ROM_LOAD( "30",  0x0000000, 0x800000, CRC(1d99181b) SHA1(25c216de16cefac2d5892039ad23d07848a457e6) )
	ROM_LOAD( "31",  0x0800000, 0x800000, CRC(6889fbda) SHA1(53a51b993d319d81a604cdf70b224955eacb617e) )
	ROM_LOAD( "40",  0x1000000, 0x800000, CRC(8069f9de) SHA1(7862ee104a2e9034910dd592687b40ebe75fa9ce) )
	ROM_LOAD( "41",  0x1800000, 0x800000, CRC(9c426823) SHA1(1839dccc7943a44063e8cb2376cd566b24e8b797) )
	ROM_LOAD( "50",  0x2000000, 0x400000, CRC(1c749cc7) SHA1(23df741360476d8035c68247e645278fbab53b59) )
ROM_END

ROM_START( jojoan )
	ROM_REGION32_BE( 0x080000, "user1", 0 ) /* bios region */
	ROM_LOAD( "jojo_asia_nocd.29f400.u2", 0x000000, 0x080000, CRC(05b4f953) SHA1(c746c7bb5359acc9adced817cb4870b1912eaefd) )

	ROM_REGION32_BE( USER4REGION_LENGTH, "user4", ROMREGION_ERASEFF ) /* cd content region */
	ROM_LOAD( "10",  0x0000000, 0x800000, CRC(e40dc123) SHA1(517e7006349b5a8fd6c30910362583f48d009355) )
	ROM_LOAD( "20",  0x0800000, 0x800000, CRC(0571e37c) SHA1(1aa28ef6ea1b606a55d0766480b3ee156f0bca5a) )
	ROM_REGION16_BE( USER5REGION_LENGTH, "user5", ROMREGION_ERASEFF ) /* cd content region */
	ROM_LOAD( "30",  0x0000000, 0x800000, CRC(1d99181b) SHA1(25c216de16cefac2d5892039ad23d07848a457e6) )
	ROM_LOAD( "31",  0x0800000, 0x800000, CRC(6889fbda) SHA1(53a51b993d319d81a604cdf70b224955eacb617e) )
	ROM_LOAD( "40",  0x1000000, 0x800000, CRC(8069f9de) SHA1(7862ee104a2e9034910dd592687b40ebe75fa9ce) )
	ROM_LOAD( "41",  0x1800000, 0x800000, CRC(9c426823) SHA1(1839dccc7943a44063e8cb2376cd566b24e8b797) )
	ROM_LOAD( "50",  0x2000000, 0x400000, CRC(1c749cc7) SHA1(23df741360476d8035c68247e645278fbab53b59) )
ROM_END

ROM_START( jojoban )
	ROM_REGION32_BE( 0x080000, "user1", 0 ) /* bios region */
	ROM_LOAD( "jojoba_japan_nocd.29f400.u2", 0x000000, 0x080000, CRC(4dab19f5) SHA1(ba07190e7662937fc267f07285c51e99a45c061e) )

	ROM_REGION32_BE( USER4REGION_LENGTH, "user4", ROMREGION_ERASEFF ) /* cd content region */
	ROM_LOAD( "10",  0x000000, 0x800000, CRC(6e2490f6) SHA1(75cbf1e39ad6362a21c937c827e492d927b7cf39) )
	ROM_LOAD( "20",  0x800000, 0x800000, CRC(1293892b) SHA1(b1beafac1a9c4b6d0640658af8a3eb359e76eb25) )
	ROM_REGION16_BE( USER5REGION_LENGTH, "user5", ROMREGION_ERASEFF ) /* cd content region */
	ROM_LOAD( "30",  0x0000000, 0x800000, CRC(d25c5005) SHA1(93a19a14783d604bb42feffbe23eb370d11281e8) )
	ROM_LOAD( "31",  0x0800000, 0x800000, CRC(51bb3dba) SHA1(39e95a05882909820b3efa6a3b457b8574012638) )
	ROM_LOAD( "40",  0x1000000, 0x800000, CRC(94dc26d4) SHA1(5ae2815142972f322886eea4885baf2b82563ab1) )
	ROM_LOAD( "41",  0x1800000, 0x800000, CRC(1c53ee62) SHA1(e096bf3cb6fbc3d45955787b8f3213abcd76d120) )
	ROM_LOAD( "50",  0x2000000, 0x800000, CRC(36e416ed) SHA1(58d0e95cc13f39bc171165468ce72f4f17b8d8d6) )
	ROM_LOAD( "51",  0x2800000, 0x800000, CRC(eedf19ca) SHA1(a7660bf9ff87911afb4f83b64456245059986830) )
ROM_END

ROM_START( jojobane )
	ROM_REGION32_BE( 0x080000, "user1", 0 ) /* bios region */
	ROM_LOAD( "jojoba_euro_nocd.29f400.u2", 0x000000, 0x080000,  CRC(1ee2d679) SHA1(9e129b454a376606b3f7e8aec64de425cf9c635c) )

	ROM_REGION32_BE( USER4REGION_LENGTH, "user4", ROMREGION_ERASEFF ) /* cd content region */
	ROM_LOAD( "10",  0x000000, 0x800000, CRC(6e2490f6) SHA1(75cbf1e39ad6362a21c937c827e492d927b7cf39) )
	ROM_LOAD( "20",  0x800000, 0x800000, CRC(1293892b) SHA1(b1beafac1a9c4b6d0640658af8a3eb359e76eb25) )
	ROM_REGION16_BE( USER5REGION_LENGTH, "user5", ROMREGION_ERASEFF ) /* cd content region */
	ROM_LOAD( "30",  0x0000000, 0x800000, CRC(d25c5005) SHA1(93a19a14783d604bb42feffbe23eb370d11281e8) )
	ROM_LOAD( "31",  0x0800000, 0x800000, CRC(51bb3dba) SHA1(39e95a05882909820b3efa6a3b457b8574012638) )
	ROM_LOAD( "40",  0x1000000, 0x800000, CRC(94dc26d4) SHA1(5ae2815142972f322886eea4885baf2b82563ab1) )
	ROM_LOAD( "41",  0x1800000, 0x800000, CRC(1c53ee62) SHA1(e096bf3cb6fbc3d45955787b8f3213abcd76d120) )
	ROM_LOAD( "50",  0x2000000, 0x800000, CRC(36e416ed) SHA1(58d0e95cc13f39bc171165468ce72f4f17b8d8d6) )
	ROM_LOAD( "51",  0x2800000, 0x800000, CRC(eedf19ca) SHA1(a7660bf9ff87911afb4f83b64456245059986830) )
ROM_END


ROM_START( redeartn )
	ROM_REGION32_BE( 0x080000, "user1", 0 ) /* bios region */
	ROM_LOAD( "redearth_nocd.bios", 0x000000, 0x080000, NO_DUMP )

	ROM_REGION32_BE( USER4REGION_LENGTH, "user4", ROMREGION_ERASEFF ) /* cd content region */
	ROM_LOAD( "10",  0x000000, 0x800000, CRC(68188016) SHA1(93aaac08cb5566c33aabc16457085b0a36048019) )
	ROM_REGION16_BE( USER5REGION_LENGTH, "user5", ROMREGION_ERASEFF ) /* cd content region */
	ROM_LOAD( "30",  0x0000000, 0x800000, CRC(074cab4d) SHA1(4cb6cc9cce3b1a932b07058a5d723b3effa23fcf) )
	ROM_LOAD( "31",  0x0800000, 0x800000, CRC(14e2cad4) SHA1(9958a4e315e4476e4791a6219b93495413c7b751) )
	ROM_LOAD( "40",  0x1000000, 0x800000, CRC(72d98890) SHA1(f40926c52cb7a71b0ef0027a0ea38bbc7e8b31b0) )
	ROM_LOAD( "41",  0x1800000, 0x800000, CRC(88ccb33c) SHA1(1e7af35d186d0b4e45b6c27458ddb9cfddd7c9bc) )
	ROM_LOAD( "50",  0x2000000, 0x400000, CRC(2f5b44bd) SHA1(7ffdbed5b6899b7e31414a0828e04543d07435e4) )
ROM_END


/*****************************************************************************************
  CPS3 game region / special flag information
*****************************************************************************************/

/*****************************************************************************************

    JoJo's Venture

    XXXXXX 0
    JAPAN 1
    ASIA 2
    EURO 3
    USA 4
    HISPANIC 5
    BRAZIL 6
    OCEANIA 7

    DEVELOPMENT VERSION add 0x70 mask!

    UINT32 *rom =  (UINT32*)memory_region ( machine, "user1" );
    rom[0x1fec8/4]^=0x00000001; // region hack (clear jpn)

    rom[0x1fec8/4]^=0x00000004; // region
    rom[0x1fec8/4]^=0x00000070; // DEV mode
    rom[0x1fecc/4]^=0x01000000; // nocd

*****************************************************************************************/


/*****************************************************************************************

    JoJo's Bizarre Adventure: Heritage for the Future

    XXXXXX 0
    JAPAN 1
    ASIA 2
    EURO 3
    USA 4
    HISPANIC 5
    BRAZIL 6
    OCEANIA 7

    DEVELOPMENT VERSION add 0x70 mask!

    UINT32 *rom =  (UINT32*)memory_region ( machine, "user1" );
    rom[0x1fec8/4]^=0x00000001; // region (clear jpn)
    rom[0x1fec8/4]^=0x00000002; // region
    rom[0x1fec8/4]^=0x00000070; // DEV mode
    rom[0x1fecc/4]^=0x01000000; // nocd

*****************************************************************************************/

/*****************************************************************************************

    Red Earth / Warzard

    JAPAN 1
    ASIA 2
    EURO 3
    USA 4
    HISPANIC 5
    BRAZIL 6
    OCEANIA 7
    ASIA NCD 8

    UINT32 *rom =  (UINT32*)memory_region ( machine, "user1" );
    rom[0x1fed8/4]^=0x00000001; // clear region to 0 (invalid)
    rom[0x1fed8/4]^=0x00000008; // region 8 - ASIA NO CD - doesn't actually skip the CD
                                // test on startup, only during game, must be another flag
                                // somewhere too, and we don't have any actual NCD dumps
                                // to compare (or it expects SCSI to report there being
                                // no cd drive?)

*****************************************************************************************/

/*****************************************************************************************

    Street Fighter III: New Generation

    JAPAN 1
    ASIA NCD 2
    EURO 3
    USA 4
    HISPANIC 5
    BRAZIL 6
    OCEANIA 7
    ASIA 8

    // bios rom also lists korea, but game rom does not.

    UINT32 *rom =  (UINT32*)memory_region ( machine, "user1" );
    rom[0x1fec8/4]^=0x00000001; // region (clear region)
    rom[0x1fec8/4]^=0x00000008; // region
    rom[0x1fecc/4]^=0x01000000; // nocd - this ONLY skips the cd check in the bios test
                                // menu is region is ASIA NCD, otherwise it will report
                                // NG, Asia was probably the only NCD region for this

*****************************************************************************************/

/*****************************************************************************************

    Street Fighter III 2nd Impact

    JAPAN 1
    ASIA NCD 2
    EURO 3
    USA 4
    HISPANIC 5
    BRAZIL 6
    OCEANIA 7
    ASIA 8

    UINT32 *rom =  (UINT32*)memory_region ( machine, "user1" );
    rom[0x1fec8/4]^=0x00000001; // region (clear region)
    rom[0x1fec8/4]^=0x00000008; // region
    rom[0x1fecc/4]^=0x01000000; // nocd - this ONLY skips the cd check in the bios test
                                // menu is region is ASIA NCD, otherwise it will report
                                // NG, Asia was probably the only NCD region for this

*****************************************************************************************/


/*****************************************************************************************

    Street Fighter III 3rd Strike

    JAPAN 1
    ASIA 2
    EURO 3
    USA 4
    HISPANIC 5
    BRAZIL 6
    OCEANIA 7

    UINT32 *rom =  (UINT32*)memory_region ( machine, "user1" );
    rom[0x1fec8/4]^=0x00000004; // region (clear region)
    rom[0x1fec8/4]^=0x00000001; // region
    rom[0x1fecc/4]^=0x01000000; // nocd

*****************************************************************************************/



/* todo: use BIOS for the bios roms, having clones only for CD / No CD */

GAME( 1997, sfiii,   0,        cps3, cps3, sfiii,  ROT0,   "Capcom", "Street Fighter III: New Generation (USA, 970204)", GAME_IMPERFECT_GRAPHICS )
GAME( 1997, sfiiij,  sfiii,    cps3, cps3, sfiii,  ROT0,   "Capcom", "Street Fighter III: New Generation (Japan, 970204)", GAME_IMPERFECT_GRAPHICS )

GAME( 1997, sfiii2,  0,        cps3, cps3, sfiii2, ROT0,   "Capcom", "Street Fighter III 2nd Impact: Giant Attack (USA, 970930)", GAME_IMPERFECT_GRAPHICS )
GAME( 1997, sfiii2j, sfiii2,   cps3, cps3, sfiii2, ROT0,   "Capcom", "Street Fighter III 2nd Impact: Giant Attack (Japan, 970930)", GAME_IMPERFECT_GRAPHICS )

GAME( 1999, sfiii3,  0,        cps3, cps3, sfiii3, ROT0,   "Capcom", "Street Fighter III 3rd Strike: Fight for the Future (USA, 990608)", GAME_IMPERFECT_GRAPHICS )
GAME( 1999, sfiii3a, sfiii3,   cps3, cps3, sfiii3, ROT0,   "Capcom", "Street Fighter III 3rd Strike: Fight for the Future (USA, 990512)", GAME_IMPERFECT_GRAPHICS )

GAME( 1998, jojo,    0,        cps3, cps3, jojo, ROT0,   "Capcom", "JoJo's Venture / JoJo no Kimyouna Bouken (Japan, 990108)", GAME_IMPERFECT_GRAPHICS )
GAME( 1998, jojoa,   jojo,     cps3, cps3, jojo, ROT0,   "Capcom", "JoJo's Venture / JoJo no Kimyouna Bouken (Japan, 981202)", GAME_IMPERFECT_GRAPHICS )

GAME( 1999, jojoba,  0,        cps3, cps3, jojoba, ROT0,   "Capcom", "JoJo's Bizarre Adventure: Heritage for the Future / JoJo no Kimyouna Bouken: Miraie no Isan (Japan, 990913)", GAME_IMPERFECT_GRAPHICS )

GAME( 1996, redearth,0,        cps3, cps3, redearth, ROT0,   "Capcom", "Red Earth (Euro, 961121)", GAME_IMPERFECT_GRAPHICS )
GAME( 1996, warzard, redearth, cps3, cps3, redearth, ROT0,   "Capcom", "Warzard (Japan, 961121)", GAME_IMPERFECT_GRAPHICS )

/* NO-CD sets */

GAME( 1997, sfiiin,  sfiii,    cps3, cps3, sfiii, ROT0,   "Capcom", "Street Fighter III: New Generation (Asia, 970204, NO CD)", GAME_IMPERFECT_GRAPHICS )

GAME( 1997, sfiii2n, sfiii2,   cps3, cps3, sfiii2, ROT0,   "Capcom", "Street Fighter III 2nd Impact: Giant Attack (Asia, 970930, NO CD)", GAME_IMPERFECT_GRAPHICS )

GAME( 1999, sfiii3n, sfiii3,   cps3, cps3, sfiii3, ROT0,   "Capcom", "Street Fighter III 3rd Strike: Fight for the Future (Japan, 990608, NO CD)", GAME_IMPERFECT_GRAPHICS )
GAME( 1999, sfiii3an,sfiii3,   cps3, cps3, sfiii3, ROT0,   "Capcom", "Street Fighter III 3rd Strike: Fight for the Future (Japan, 990512, NO CD)", GAME_IMPERFECT_GRAPHICS )

GAME( 1998, jojon,   jojo,     cps3, cps3, jojo, ROT0,   "Capcom", "JoJo's Venture / JoJo no Kimyouna Bouken (Asia, 990108, NO CD)", GAME_IMPERFECT_GRAPHICS )
GAME( 1998, jojoan,  jojo,     cps3, cps3, jojo, ROT0,   "Capcom", "JoJo's Venture / JoJo no Kimyouna Bouken (Asia, 981202, NO CD)", GAME_IMPERFECT_GRAPHICS )

GAME( 1999, jojoban, jojoba,   cps3, cps3, jojoba, ROT0,   "Capcom", "JoJo's Bizarre Adventure: Heritage for the Future / JoJo no Kimyouna Bouken: Miraie no Isan (Japan, 990913, NO CD)", GAME_IMPERFECT_GRAPHICS )
GAME( 1999, jojobane,jojoba,   cps3, cps3, jojoba, ROT0,   "Capcom", "JoJo's Bizarre Adventure: Heritage for the Future / JoJo no Kimyouna Bouken: Miraie no Isan (Euro, 990913, NO CD)", GAME_IMPERFECT_GRAPHICS )

// We don't have any actual warzard / red earth no cd bios sets, but keep this here anyway
GAME( 1996, redeartn,redearth, cps3, cps3, redearth, ROT0,   "Capcom", "Red Earth (961121, NO CD)", GAME_NOT_WORKING )
