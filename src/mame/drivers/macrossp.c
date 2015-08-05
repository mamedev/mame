// license:BSD-3-Clause
// copyright-holders:David Haywood
/*** DRIVER INFO **************************************************************

Macross Plus                        (c)1996 Banpresto
Quiz Bisyoujo Senshi Sailor Moon    (c)1997 Banpresto

Driver by David Haywood

TODO:
 - what is the 'bios' rom for? it appears to be data tables and is very different between games but we don't map it anywhere
 - convert tilemaps to devices?

 68020 interrupts
 lev 1 : 0x64 : 0000 084c - unknown..
 lev 2 : 0x68 : 0000 0882 - unknown..
 lev 3 : 0x6c : 0000 08b0 - vblank?
 lev 4 : 0x70 : 001f 002a - x
 lev 5 : 0x74 : 001f 002a - x
 lev 6 : 0x78 : 001f 002a - x
 lev 7 : 0x7c : 001f 002a - x


**** README INFO **************************************************************

--- ROMSET: macrossp ---

Macross Plus
Banpresto Co., Ltd., 1996

PCB: BP964
CPU: MC68EC020FG25
SND: TMP68HC000N-16, OTTOR2 ES5506000102
OSC: 50.000MHz, 32.000MHz, 27.000MHz
RAM: TC55257CFL-85 x 8 (28 PIN TSOP), MCM62068AEJ25 x 12 (28 PIN SSOP), TC55328AJ-15 x 6 (28 PIN SSOP)
GFX: IKM-AA004   (208 PIN PQFP)
     IKM-AA0062  (208 PIN PQFP)
     IKM-AA005   x 2 (208 PIN PQFP)
DIPS: 2x 8-Position


DIPSW-1
------------------------------------------------------------------
    DipSwitch Title   | Function | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
------------------------------------------------------------------
                      | 1cn/1pl  |off|off|off|off|               |*
                      | 1cn/2pl  |on |off|off|off|               |
                      | 1cn/3pl  |off|on |off|off|               |
                      | 1cn/4pl  |on |on |off|off|               |
                      | 1cn/5pl  |off|off|on |off|               |
                      | 1cn/6pl  |on |off|on |off|               |
     Coin Chute A     | 1cn/7pl  |off|on |on |off|               |
                      | 2cn/1pl  |on |on |on |off|               |
                      | 2cn/3pl  |off|off|off|on |               |
                      | 2cn/5pl  |on |off|off|on |               |
                      | 3cn/1pl  |off|on |off|on |               |
                      | 3cn/2pl  |on |on |off|on |               |
                      | 3cn/4pl  |off|off|on |on |               |
                      | 4cn/1pl  |on |off|on |on |               |
                      | 4cn/3pl  |off|on |on |on |               |
                      |  Free    |on |on |on |on |               |
------------------------------------------------------------------
                      | 1cn/1pl  |               |off|off|off|off|*
                      | 1cn/2pl  |               |on |off|off|off|
                      | 1cn/3pl  |               |off|on |off|off|
                      | 1cn/4pl  |               |on |on |off|off|
                      | 1cn/5pl  |               |off|off|on |off|
                      | 1cn/6pl  |               |on |off|on |off|
     Coin Chute B     | 1cn/7pl  |               |off|on |on |off|
                      | 2cn/1pl  |               |on |on |on |off|
                      | 2cn/3pl  |               |off|off|off|on |
                      | 2cn/5pl  |               |on |off|off|on |
                      | 3cn/1pl  |               |off|on |off|on |
                      | 3cn/2pl  |               |on |on |off|on |
                      | 3cn/4pl  |               |off|off|on |on |
                      | 4cn/1pl  |               |on |off|on |on |
                      | 4cn/3pl  |               |off|on |on |on |
                      | 5cn/3pl  |               |on |on |on |on |
------------------------------------------------------------------


DIPSW-2
------------------------------------------------------------------
    DipSwitch Title   | Function | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
------------------------------------------------------------------
                      |  Normal  |off|off|                       |*
      Difficulty      |   Easy   |on |off|                       |
        Level         |   Hard   |off|on |                       |
                      |  V.Hard  |on |on |                       |
------------------------------------------------------------------
                      |     3    |       |off|off|               |*
    Players Count     |     4    |       |on |off|               |
                      |     5    |       |off|on |               |
                      |     2    |       |on |on |               |
------------------------------------------------------------------
      Demo Sound      |   Play   |               |off|           |*
                      |   Mute   |               |on |           |
------------------------------------------------------------------
      Screen Flip     |    Off   |                   |off|       |*
                      |    On    |                   |on |       |
------------------------------------------------------------------
Title Screen Language | Japanese |                       |off|   |*#
                      | English  |                       |on |   |
------------------------------------------------------------------
   Test / Game Mode   |   Game   |                           |off|*
                      |   Test   |                           |on |
------------------------------------------------------------------

* Denotes Factory Defualts
# English title page doesn't display the Japanese translation of the word "Macross"


ROMs:  (Filename = ROM label, extension also on ROM label)

TOP ROM PCB
-----------
BP964A-C.U1 \
BP964A-C.U2  |
BP964A-C.U3  |
BP964A-C.U4  > 27C040
BP964A.U19   |
BP964A.U20   |
BP964A.U21  /

BP964A.U9   \
BP964A.U10   |
BP964A.U11   |
BP964A.U12   |
BP964A.U13   |
BP964A.U14   > 32M (44 pin SOP - surface mounted)
BP964A.U15   |
BP964A.U16   |
BP964A.U17   |
BP964A.U18   |
BP964A.U24  /

ROMs:  (Filename = ROM Label)

MOTHERBOARD PCB
---------------
BP964A.U49  27C010

--- ROMSET: quizmoon ---

Quiz Bisyoujo Senshi Sailor Moon - Chiryoku Tairyoku Toki no Un -
(c)1997 Banpresto / Gazelle
BP965A

-----------
Motherboard
-----------
CPU  : MC68EC020FG25
Sound: TMP68HC000N-16, ENSONIQ OTTO R2 (ES5506)
OSC  : 50.000MHz (X1), 32.000MHz (X2), 27.000MHz (X3)

ROMs:
u49.bin - (ST 27c1001)

GALs (16V8B, not dumped):
u009.bin
u200.bin

Custom chips:
IKM-AA004 1633JF8433 JAPAN 9523YAA (U62, 208pin QFP)
IKM-AA005 1670F1541 JAPAN 9525EAI (U47&48, 208pin QFP)
IKM-AA006 1633JF8432 JAPAN 9525YAA (U31, 208pin QFP)

--------------
Mask ROM board
--------------
u5.bin - Main programs (TI 27c040)
u6.bin |
u7.bin |
u8.bin |
u1.bin | (ST 27c1001)
u2.bin |
u3.bin |
u4.bin /

u09.bin - Graphics (uPD23C32000GX)
u10.bin |
u11.bin |
u12.bin |
u13.bin |
u15.bin |
u17.bin / (uPD23C16000GX)

u20.bin - Sound programs (ST 27c1001)
u21.bin /

u24.bin - Samples (uPD23C32000GX)
u25.bin |
u26.bin |
u27.bin /


----------
PCB Layout
----------

MOTHERBOARD
|---------------------------------------------------------------|
|TA8205AH  GAL16V8B(3)    |----------------|BP964A_U49          |
|  VOL                    |----------------||-------|  TC55328  |
| TL074    ES5506                           |AA005  |  TC55328  |
|  D6376                                    |       |  TC55328  |
|               TC55257    |-------------|  |       |  TC55328  |
|               TC55257    |   68000     |  |-------|           |
|     GAL16V8B(2)          |             |                      |
|     50MHz     TC55257    |-------------|  |-------|           |
|               TC55257                     |AA005  |       |-| |
|                                           |       |       | | |
|               TC55257                     |       |       | | |
|      |-------|TC55257                     |-------|       | | |
|      |68EC020|                                            | | |
|      |       | GAL16V8B(1)                |-------|       | | |
|      |-------|                            |AA0062 |       | | |
|          MCM6206  MCM6206                 |       |       | | |
|          MCM6206  MCM6206                 |       |       | | |
|                                           |-------|       |-| |
|          MCM6206  MCM6206                                     |
|                                           |-------|  TC55328  |
|          MCM6206  MCM6206                 |AA004  |  TC55328  |
|          MCM6206  MCM6206                 |       |           |
|          MCM6206  MCM6206                 |       |   32MHz   |
|DSW1(8)                  |----------------||-------|   27MHz   |
|DSW2(8)                  |----------------| TC55257  TC55257   |
|---------------------------------------------------------------|
Notes:
      68020 clock 25.000MHz [50/2] (QFP100)
      68000 clock 16.000MHz [32/2] (SDIP64)
      ES5506 clock 16.000MHz [32/2] (PLCC68)
      VSync 60Hz
      HSync 15.83kHz
      TC55257     - Toshiba TC55257CFL-85 32k x8 SRAM (SOJ28)
      TC55328     - Toshiba TC55328AJ-15 32k x8 SRAM (SOP28)
      MCM6206     - Motorola MCM6206BAEJ25 32k x8 SRAM (SOJ28)
      GAL16V8B(1) - stamped 'U008' (DIP20)
      GAL16V8B(2) - stamped 'U200' (DIP20)
      GAL16V8B(3) - stamped 'U009' (DIP20)
      D6376       - NEC uPD6376 Audio 2-Channel 16-Bit D/A Converter (SOIC16)
      Custom ICs  -
                   IKM-AA004 1633JF8433 (QFP208)
                   IKM-AA0062 1633CF8461 (QFP208)
                   IKM-AA005 1670F1541 (x2, QFP208)

ROM Board
---------

MASK ROM BOARD
|----------------------------------------|
|  |----------------|                    |
|  |----------------|                    |
|                              U19       |
| *U27    *U26                           |
|                                        |
|                                        |
| *U25     U24        *U23    *U22       |
|                                        |
|                                    |-| |
|  U18     U17         U21     U20   | | |
|                                    | | |
|                                    | | |
|                                    | | |
|  U16     U15         U14     U13   | | |
|                                    | | |
|                                    | | |
|  U9      U10         U11     U12   | | |
|                                    |-| |
|                                        |
| *U5     *U6         *U7     *U8        |
|                                        |
|  U1      U2          U3      U4        |
|                                        |
|  |----------------|                    |
|  |----------------|                    |
|----------------------------------------|
Notes:
      * - These locations not populated
      ROMs at locations U1, U2, U3, U4, U19, U20 & U21 are 27C040 EPROMs
      All other ROMs are surface mounted 32MBit SOP44 MaskROMs

******************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/es5506.h"
#include "includes/macrossp.h"

/*** VARIOUS READ / WRITE HANDLERS *******************************************/

READ32_MEMBER(macrossp_state::macrossp_soundstatus_r)
{
	//  logerror("%08x read soundstatus\n", space.device().safe_pc());

	/* bit 1 is sound status */
	/* bit 0 unknown - it is expected to toggle, vblank? */

	m_snd_toggle ^= 1;

	return (m_sndpending << 1) | m_snd_toggle;
}

WRITE32_MEMBER(macrossp_state::macrossp_soundcmd_w)
{
	if (ACCESSING_BITS_16_31)
	{
		//logerror("%08x write soundcmd %08x (%08x)\n",space.device().safe_pc(),data,mem_mask);
		soundlatch_word_w(space, 0, data >> 16, 0xffff);
		m_sndpending = 1;
		m_audiocpu->set_input_line(2, HOLD_LINE);
		/* spin for a while to let the sound CPU read the command */
		space.device().execute().spin_until_time(attotime::from_usec(50));
	}
}

READ16_MEMBER(macrossp_state::macrossp_soundcmd_r)
{
	//  logerror("%06x read soundcmd\n",space.device().safe_pc());
	m_sndpending = 0;
	return soundlatch_word_r(space, offset, mem_mask);
}

WRITE16_MEMBER(macrossp_state::palette_fade_w)
{
	// 0xff is written a few times on startup
	if (data >> 8 != 0xff)
	{
		// range seems to be 40 (brightest) to 252 (darkest)
		UINT8 fade = ((data >> 8) - 40) / 212.0 * 255.0;
		m_screen->set_brightness(0xff - fade);
	}
}

/*** MEMORY MAPS *************************************************************/

static ADDRESS_MAP_START( macrossp_map, AS_PROGRAM, 32, macrossp_state )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM
	AM_RANGE(0x800000, 0x802fff) AM_RAM AM_SHARE("spriteram")
	/* SCR A Layer */
	AM_RANGE(0x900000, 0x903fff) AM_RAM_WRITE(macrossp_scra_videoram_w) AM_SHARE("scra_videoram")
	AM_RANGE(0x904200, 0x9043ff) AM_RAM AM_SHARE("scra_linezoom") /* W/O? */
	AM_RANGE(0x905000, 0x90500b) AM_RAM AM_SHARE("scra_videoregs") /* W/O? */
	/* SCR B Layer */
	AM_RANGE(0x908000, 0x90bfff) AM_RAM_WRITE(macrossp_scrb_videoram_w) AM_SHARE("scrb_videoram")
	AM_RANGE(0x90c200, 0x90c3ff) AM_RAM AM_SHARE("scrb_linezoom") /* W/O? */
	AM_RANGE(0x90d000, 0x90d00b) AM_RAM AM_SHARE("scrb_videoregs") /* W/O? */
	/* SCR C Layer */
	AM_RANGE(0x910000, 0x913fff) AM_RAM_WRITE(macrossp_scrc_videoram_w) AM_SHARE("scrc_videoram")
	AM_RANGE(0x914200, 0x9143ff) AM_RAM AM_SHARE("scrc_linezoom")/* W/O? */
	AM_RANGE(0x915000, 0x91500b) AM_RAM AM_SHARE("scrc_videoregs") /* W/O? */
	/* Text Layer */
	AM_RANGE(0x918000, 0x91bfff) AM_RAM_WRITE(macrossp_text_videoram_w) AM_SHARE("text_videoram")
	AM_RANGE(0x91c200, 0x91c3ff) AM_RAM AM_SHARE("text_linezoom") /* W/O? */
	AM_RANGE(0x91d000, 0x91d00b) AM_RAM AM_SHARE("text_videoregs") /* W/O? */

	AM_RANGE(0xa00000, 0xa03fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")

	AM_RANGE(0xb00000, 0xb00003) AM_READ_PORT("INPUTS")
	AM_RANGE(0xb00004, 0xb00007) AM_READ(macrossp_soundstatus_r) AM_WRITENOP // irq related?
	AM_RANGE(0xb00008, 0xb0000b) AM_WRITENOP    // irq related?
	AM_RANGE(0xb0000c, 0xb0000f) AM_READ_PORT("DSW") AM_WRITENOP
	AM_RANGE(0xb00010, 0xb00013) AM_WRITE16(palette_fade_w, 0x0000ffff)
	AM_RANGE(0xb00020, 0xb00023) AM_WRITENOP

	AM_RANGE(0xc00000, 0xc00003) AM_WRITE(macrossp_soundcmd_w)

	AM_RANGE(0xf00000, 0xf1ffff) AM_RAM AM_SHARE("mainram") /* Main Ram */
//  AM_RANGE(0xfe0000, 0xfe0003) AM_NOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( macrossp_sound_map, AS_PROGRAM, 16, macrossp_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x207fff) AM_RAM
	AM_RANGE(0x400000, 0x40007f) AM_DEVREADWRITE8("ensoniq", es5506_device, read, write, 0x00ff)
	AM_RANGE(0x600000, 0x600001) AM_READ(macrossp_soundcmd_r)
ADDRESS_MAP_END

/*** INPUT PORTS *************************************************************/

static INPUT_PORTS_START( macrossp )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED ) /* Unknown use */
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0000ffc0, IP_ACTIVE_LOW, IPT_UNUSED ) /* Unknown use */
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("DSW")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED ) /* Unknown use, but not dipswitches */
	PORT_DIPNAME( 0x000f0000, 0x000f0000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(          0x00020000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(          0x00050000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(          0x00080000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(          0x00040000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(          0x00010000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(          0x000f0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(          0x00030000, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(          0x00070000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(          0x000e0000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(          0x00060000, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(          0x000d0000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(          0x000c0000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(          0x000b0000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(          0x000a0000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(          0x00090000, "1 Coins/7 Credits" )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Free_Play) )
	PORT_DIPNAME( 0x00f00000, 0x00f00000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(          0x00200000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(          0x00500000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(          0x00800000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(          0x00400000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(          0x00100000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(          0x00f00000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(          0x00300000, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(          0x00700000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(          0x00e00000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(          0x00600000, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(          0x00d00000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(          0x00c00000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(          0x00b00000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(          0x00a00000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(          0x00900000, "1 Coins/7 Credits" )
	PORT_DIPNAME( 0x03000000, 0x03000000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(          0x02000000, DEF_STR( Easy ) )
	PORT_DIPSETTING(          0x03000000, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x01000000, DEF_STR( Hard ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c000000, 0x0c000000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(          0x00000000, "2" )
	PORT_DIPSETTING(          0x0c000000, "3" )
	PORT_DIPSETTING(          0x08000000, "4" )
	PORT_DIPSETTING(          0x04000000, "5" )
	PORT_DIPNAME( 0x10000000, 0x10000000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x10000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x20000000, 0x20000000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:6")  /* See above for dips listing.... also in Quiz game's test screens */
	PORT_DIPSETTING(          0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x40000000, 0x00000000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:7")  /* See title page for difference :-)  The Manual shows this as UNUSED */
	PORT_DIPSETTING(          0x40000000, DEF_STR( Japanese ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( English ) )
	PORT_SERVICE_DIPLOC(  0x80000000, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( quizmoon )
	PORT_INCLUDE(macrossp)

	PORT_MODIFY("INPUTS")
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Test ) )
	PORT_DIPSETTING(          0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Tilt ) )
	PORT_DIPSETTING(          0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )

	PORT_MODIFY("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x40000000, 0x40000000, "SW2:7" ) /* no Language dipswitch for this game */
INPUT_PORTS_END

/*** GFX DECODE **************************************************************/

static const gfx_layout macrossp_char16x16x4layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28, 32+0,32+4,32+8,32+12,32+16,32+20,32+24,32+28 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
		8*64, 9*64, 10*64,11*64,12*64,13*64,14*64,15*64},
	16*64
};

static const gfx_layout macrossp_char16x16x8layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0, 8, 16, 24, 32, 40, 48, 56, 64+0,64+8,64+16,64+24,64+32,64+40,64+48,64+56 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128,
		8*128, 9*128, 10*128,11*128,12*128,13*128,14*128,15*128},
	16*128
};

static GFXDECODE_START( macrossp )
	GFXDECODE_ENTRY( "gfx1", 0, macrossp_char16x16x8layout,   0x000, 0x20 ) /* 8bpp but 6bpp granularity */
	GFXDECODE_ENTRY( "gfx2", 0, macrossp_char16x16x8layout,   0x800, 0x20 ) /* 8bpp but 6bpp granularity */
	GFXDECODE_ENTRY( "gfx3", 0, macrossp_char16x16x8layout,   0x800, 0x20 ) /* 8bpp but 6bpp granularity */
	GFXDECODE_ENTRY( "gfx4", 0, macrossp_char16x16x8layout,   0x800, 0x20 ) /* 8bpp but 6bpp granularity */
	GFXDECODE_ENTRY( "gfx5", 0, macrossp_char16x16x4layout,   0x800, 0x80 )
GFXDECODE_END

/*** MACHINE DRIVER **********************************************************/

WRITE_LINE_MEMBER(macrossp_state::irqhandler)
{
	logerror("ES5506 irq %d\n", state);

	/* IRQ lines 1 & 4 on the sound 68000 are definitely triggered by the ES5506,
	but I haven't noticed the ES5506 ever assert the line - maybe only used when developing the game? */
	//  m_audiocpu->set_input_line(1, state ? ASSERT_LINE : CLEAR_LINE);
}

void macrossp_state::machine_start()
{
	save_item(NAME(m_sndpending));
	save_item(NAME(m_snd_toggle));
}

void macrossp_state::machine_reset()
{
	m_sndpending = 0;
	m_snd_toggle = 0;
}

static MACHINE_CONFIG_START( macrossp, macrossp_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68EC020, 50000000/2)   /* 25 MHz */
	MCFG_CPU_PROGRAM_MAP(macrossp_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", macrossp_state,  irq3_line_hold) // there are others ...

	MCFG_CPU_ADD("audiocpu", M68000, 32000000/2)    /* 16 MHz */
	MCFG_CPU_PROGRAM_MAP(macrossp_sound_map)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*16, 16*16)
	MCFG_SCREEN_VISIBLE_AREA(0*16, 24*16-1, 0*16, 15*16-1)
	MCFG_SCREEN_UPDATE_DRIVER(macrossp_state, screen_update_macrossp)
	MCFG_SCREEN_VBLANK_DRIVER(macrossp_state, screen_eof_macrossp)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", macrossp)

	MCFG_PALETTE_ADD("palette", 4096)
	MCFG_PALETTE_FORMAT(RGBX)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ensoniq", ES5506, 16000000)
	MCFG_ES5506_REGION0("ensoniq.0")
	MCFG_ES5506_REGION1("ensoniq.1")
	MCFG_ES5506_REGION2("ensoniq.2")
	MCFG_ES5506_REGION3("ensoniq.3")
	MCFG_ES5506_CHANNELS(1)               /* channels */
	MCFG_ES5506_IRQ_CB(WRITELINE(macrossp_state, irqhandler))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.1)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( quizmoon, macrossp )

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, 24*16-1, 0*8, 14*16-1)
MACHINE_CONFIG_END



/*** ROM LOADING *************************************************************/

ROM_START( macrossp )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "bp964a-c.u1", 0x000003, 0x080000, CRC(39da35e7) SHA1(022c902b0adf21090c650ce68e4b8b42498e1be6) )
	ROM_LOAD32_BYTE( "bp964a-c.u2", 0x000002, 0x080000, CRC(86d0ca6a) SHA1(8cc1b4a83cbba8b07e1343c5c20e2590d3ce471a) )
	ROM_LOAD32_BYTE( "bp964a-c.u3", 0x000001, 0x080000, CRC(fb895a7b) SHA1(547e5d3d43e503a15573748ab49a44e5569db1d7) )
	ROM_LOAD32_BYTE( "bp964a-c.u4", 0x000000, 0x080000, CRC(8c8b966c) SHA1(881fa2b7aefc3ea916924b715d2cd0ceaffe2d92) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "bp964a.u20", 0x000001, 0x080000, CRC(12960cbb) SHA1(7182c4b36849a5d34ddf388bf5f4485ed360fe84) )
	ROM_LOAD16_BYTE( "bp964a.u21", 0x000000, 0x080000, CRC(87bdd2fc) SHA1(c33f087ebca6e98db195404788ca8e0cc6663622) )

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD( "bp964a.u49", 0x000000, 0x020000, CRC(ad203f76) SHA1(3eb86eeeb020349dfd88ebc8b4fc9579d1cc50fb) )  // 'BIOS'

	ROM_REGION( 0x1000000, "gfx1", 0 ) /* sprites - 16x16x8 */
	ROM_LOAD32_BYTE( "bp964a.u9",  0x000003, 0x400000, CRC(bd51a70d) SHA1(3447ae9d368e4e33df2d4e2848b4fd5aa0fc6840) )
	ROM_LOAD32_BYTE( "bp964a.u10", 0x000002, 0x400000, CRC(ab84bba7) SHA1(d30876b2e45c4b78cda27d3c648100e60f739d9c) )
	ROM_LOAD32_BYTE( "bp964a.u11", 0x000001, 0x400000, CRC(b9ae1d0b) SHA1(bc541a8bd622c99cf5065b3a793f0b5f6420ac64) )
	ROM_LOAD32_BYTE( "bp964a.u12", 0x000000, 0x400000, CRC(8dda1052) SHA1(c374335e98859ae98ac392a7cdb44f15b4e1c23a) )

	ROM_REGION( 0x800000, "gfx2", 0 ) /* backgrounds - 16x16x8 */
	ROM_LOAD( "bp964a.u13", 0x000000, 0x400000, CRC(f4d3c5bf) SHA1(82522d276a6d49148da8a4fb11846a039429bcf8) )
	ROM_LOAD( "bp964a.u14", 0x400000, 0x400000, CRC(4f2dd1b2) SHA1(30a2c9fb26bca8bb27fbc5637878f99e7f6ad8f4) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* backgrounds - 16x16x8 */
	ROM_LOAD( "bp964a.u15", 0x000000, 0x400000, CRC(5b97a870) SHA1(16f3921649b28ecb6d628871214f972333bbeca4) )
	ROM_LOAD( "bp964a.u16", 0x400000, 0x400000, CRC(c8a0cd64) SHA1(2a30a4d4ec3f94631783eb08c62003b116bb2ee3) )

	ROM_REGION( 0x800000, "gfx4", 0 ) /* backgrounds - 16x16x8 */
	ROM_LOAD( "bp964a.u17", 0x000000, 0x400000, CRC(f2470876) SHA1(e683208432f71f3cc19ced245fa5b8a82466d19b) )
	ROM_LOAD( "bp964a.u18", 0x400000, 0x400000, CRC(52ef21f3) SHA1(08fb1969ad0ffd0c5bf11d3d5448a26112d562b0) )

	ROM_REGION( 0x400000, "gfx5", 0 ) /* foreground - 16x16x4 */
	ROM_LOAD( "bp964a.u19", 0x000000, 0x080000, CRC(19c7acd9) SHA1(b7631e74f359c5570c44addf46c3e96c80adc6c3) )

	ROM_REGION16_BE( 0x800000, "ensoniq.0", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "bp964a.u24", 0x000000, 0x400000, CRC(93f90336) SHA1(75daa2f8cedc732cf5ef98254f61748c94b94aea) )

	ROM_REGION16_BE( 0x400000, "ensoniq.1", 0 )
	ROM_COPY( "ensoniq.0", 0x400000, 0x000000, 0x400000 )
ROM_END

ROM_START( quizmoon )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "u1.bin",  0x000003, 0x020000, CRC(ea404553) SHA1(123bb8e399a5b54c43f4eb41d3e5f52c6947900f) )
	ROM_LOAD32_BYTE( "u2.bin",  0x000002, 0x020000, CRC(024eedff) SHA1(cbaa2b71980a2686e582331616dc36e34ecd9e67) )
	ROM_LOAD32_BYTE( "u3.bin",  0x000001, 0x020000, CRC(545b1d17) SHA1(f1b15260942482857c48b574ada1e2a3b728f395) )
	ROM_LOAD32_BYTE( "u4.bin",  0x000000, 0x020000, CRC(60b3d18c) SHA1(230342a084938fdbd2b4da23df2054391eab165b) )
	ROM_LOAD32_BYTE( "u5.bin",  0x200003, 0x080000, CRC(4cc65f5e) SHA1(eebad4c1bf761f08cacbf8c75e7f7bd421ee65ca) )
	ROM_LOAD32_BYTE( "u6.bin",  0x200002, 0x080000, CRC(d84b7c6c) SHA1(ba6ab34fb5c61aa1a97159b7aa3d89e978fb0538) )
	ROM_LOAD32_BYTE( "u7.bin",  0x200001, 0x080000, CRC(656b2125) SHA1(ac3874e71ec0aa4e77ac0d556e4572606ce673c7) )
	ROM_LOAD32_BYTE( "u8.bin",  0x200000, 0x080000, CRC(944df309) SHA1(ee85f6dbfe970b63943d01d9f8b491717a4d5a71) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "u20.bin", 0x000001, 0x020000, CRC(d7ad1ffb) SHA1(9d375285628b32296c93456a00bc005a3f40ce38) )
	ROM_LOAD16_BYTE( "u21.bin", 0x000000, 0x020000, CRC(6fc625c6) SHA1(542bc025cf0e37686eae5d6c80bc5e047d6389fd) )

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD( "u49.bin", 0x000000, 0x020000, CRC(1590ad81) SHA1(04fb8119d9eafc6d2a921700dfb11e9c8b705c88) )  // 'BIOS'

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "u9.bin",  0x0000003, 0x0400000, CRC(aaaf2ca9) SHA1(b9e59590daf4cdee4b1deeb6d4ecc80eb12a2e18) )
	ROM_LOAD32_BYTE( "u10.bin", 0x0000002, 0x0400000, CRC(f0349691) SHA1(623a680ad164d407be0af585a15540f0dca995a4) )
	ROM_LOAD32_BYTE( "u11.bin", 0x0000001, 0x0400000, CRC(893ab178) SHA1(ba68b9a3e81af4c2565715504ada35c7da3f135f) )
	ROM_LOAD32_BYTE( "u12.bin", 0x0000000, 0x0400000, CRC(39b731b8) SHA1(2bf1d083fc6d8058a0d26b29714945e8be0e2c79) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "u13.bin", 0x0000000, 0x0400000, CRC(3dcbb041) SHA1(fcff67113707fcf14d49538551724490498c0909) )

	ROM_REGION( 0x400000, "gfx3", 0 )
	ROM_LOAD( "u15.bin", 0x0000000, 0x0400000, CRC(b84224f0) SHA1(7163aec2cc118111b2c5d8deb61133d762a5d74c) )

	ROM_REGION( 0x0200000, "gfx4", 0 )
	ROM_LOAD( "u17.bin", 0x0000000, 0x0200000, CRC(ff93c949) SHA1(13917d73a6cb70d03d0335bd816bf6b094758d0b) )

	ROM_REGION( 0x400000, "gfx5", ROMREGION_ERASE00 )
	/* nothing on this game? */

	ROM_REGION16_BE( 0x800000, "ensoniq.0", 0 )
	ROM_LOAD16_BYTE( "u26.bin", 0x0000000, 0x0400000, CRC(6c8f30d4) SHA1(7e215589e4a52cbce7f2bb31b333f874a9f83d00) )
	ROM_LOAD16_BYTE( "u24.bin", 0x0000001, 0x0400000, CRC(5b12d0b1) SHA1(c5ddff2053148a1da0710a10f48689bf5c736ae4) )

	ROM_REGION16_BE( 0x400000, "ensoniq.1", 0 )
	ROM_COPY( "ensoniq.0", 0x400000, 0x000000, 0x400000 )

	ROM_REGION16_BE( 0x800000, "ensoniq.2", 0 )
	ROM_LOAD16_BYTE( "u27.bin", 0x0000000, 0x0400000, CRC(bd75d165) SHA1(2da770d15c812cbfdb4e3048d320071edffccfa1) )
	ROM_LOAD16_BYTE( "u25.bin", 0x0000001, 0x0400000, CRC(3b9689bc) SHA1(0857c3d3e9810f9468f7c17f8b795825c55a9f08) )

	ROM_REGION16_BE( 0x400000, "ensoniq.3", 0 )
	ROM_COPY( "ensoniq.2", 0x400000, 0x000000, 0x400000 )
ROM_END



WRITE32_MEMBER(macrossp_state::macrossp_speedup_w)
{
/*
PC :00018104 018104: addq.w  #1, $f1015a.l
PC :0001810A 01810A: cmp.w   $f10140.l, D0
PC :00018110 018110: beq     18104
*/

	COMBINE_DATA(&m_mainram[0x10158 / 4]);
	if (space.device().safe_pc() == 0x001810A) space.device().execute().spin_until_interrupt();
}

#ifdef UNUSED_FUNCTION
WRITE32_MEMBER(macrossp_state::quizmoon_speedup_w)
{
	COMBINE_DATA(&m_mainram[0x00020 / 4]);
	if (space.device().safe_pc() == 0x1cc) space.device().execute().spin_until_interrupt();
}
#endif

DRIVER_INIT_MEMBER(macrossp_state,macrossp)
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xf10158, 0xf1015b, write32_delegate(FUNC(macrossp_state::macrossp_speedup_w),this));
}

DRIVER_INIT_MEMBER(macrossp_state,quizmoon)
{
#ifdef UNUSED_FUNCTION
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xf00020, 0xf00023, write32_delegate(FUNC(macrossp_state::quizmoon_speedup_w),this));
#endif
}

GAME( 1996, macrossp, 0, macrossp, macrossp, macrossp_state, macrossp, ROT270, "MOSS / Banpresto", "Macross Plus", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1997, quizmoon, 0, quizmoon, quizmoon, macrossp_state, quizmoon, ROT0,   "Banpresto", "Quiz Bisyoujo Senshi Sailor Moon - Chiryoku Tairyoku Toki no Un", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
