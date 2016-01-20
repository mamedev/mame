// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Stroffolino, Carlos A. Lozano
/**********************************************************************

Legion
(c)1986 Nichibutsu

Terra Force
(c)1987 Nichibutsu

Kozure Ookami
(c)1987 Nichibutsu

Crazy Climber 2
(c)1988 Nichibutsu

Armed Formation
(c)1988 Nichibutsu

Based on the notes below I suspect several of the supported sets are
actually bootlegs.

68000 + Z80

TODO:
- identify and decap the NB1414M4 chip, it could be either a MCU or a fancy blitter chip;
- time over doesn't kill the player in Kozure Ookami, check (1) note;
- Fix Armed F and Tatakae Big Fighter text tilemap usage, they both doesn't use the NB1414M4
  (as shown by the per-game kludge)

Notes:
- the initial level color fade in effect in Armed F is confirmed on real HW, i.e. goes from
  red to blue;
- Crazy Climber 2 does use the 1414M4 chip, but the data used gets overwritten by the m68k code.
- sprites use a RAM clut for colors, used for color cycling effects. Examples are:
  - "2" logo in Crazy Climber 2 title screen;
  - ship rays on Armed F title screen;
  - gameplay in Armed F abuses of this effect (shots, player ship lights etc.);
  - Terra Force helmet during the intro;
  - Invincibility flickering of main character in Kozure Ookami;
- (1) Kozure Ookami timer over bug:
      010118: lea     $63510.l, A0
      01011E: tst.w   (A0) ;check time variable, in BCD format
      010120: beq     $10152
      010156: cmpi.w  #$4, D0
      01015A: beq     $f992
      01015E: cmpi.w  #$10, D0
      010162: beq     $f992
      010166: tst.b   $60626.l ;fails there, this location is never enabled!
      01016C: beq     $1017e
      01016E: btst    #$7, $60621.l ;check dsw2 ram copy bit 15 (debug feature?)
      010176: bne     $1017e
      010178: bra     $f9f0 ;timer over event occurs
  btanb perhaps? Currently patched to work, might also be that DSW2 bit 7 is actually a MCU bit ready flag, so it
  definitely needs PCB tests.


Stephh's notes (based on the games M68000 code and some tests) :

1a) 'legion'

  - The ROM test (code at 0x00046e) checks range 0x000102-0x01ffff (!)
    and reports an error if the checksum isn't correct.

  - 3 Dip Switches which are told to be unused have an effect if
    [0x062d53] != 0x00 (check code at 0x00d9b2).
    As this value is ALWAYS set to 0x00 via code at 0x0001d4, I've
    added a #define LEGION_HACK you need to change to 1 if you want
    get the benefits of these Dip Switches.
    Here is what these "unused" Dip Switch do :

      * 1st unused Dip Switch (DSW0 bit 6) remains unused
      * 2nd unused Dip Switch (DSW0 bit 7) determines if the "Invulnerability"
        Dip Switches will be tested
      * 3rd unused Dip Switch (DSW1 bit 6) gives invulnerability or not
        to player 1 (DSW0 bit 7 must be ON too !)
      * 4th unused Dip Switch (DSW1 bit 7) gives invulnerability or not
        to player 2 (DSW0 bit 7 must be ON too !)

    Also note bit 1 of 0x062d53 is also tested but I haven't been able
    to find its purpose (any more infos are welcome)


1b) 'legionjb' (bootleg set?)

  - The ROM test (code at 0x000466) checks range 0x000102-0x03ffff
    but NEVER reports an error if the checksum isn't correct due
    to the instruction at 0x000480 (see where it branches) :

    000466: 7000                     moveq   #$0, D0
    000468: 41FA FC98                lea     (-$368,PC), A0; ($102)
    00046C: D058                     add.w   (A0)+, D0
    00046E: B1FC 0004 0000           cmpa.l  #$40000, A0
    000474: 66F6                     bne     46c
    000476: 33C0 0006 2CAE           move.w  D0, $62cae.l
    00047C: B078 0100                cmp.w   $100.w, D0
    000480: 6600 0002                bne     484
    000484: 41FA FF86                lea     (-$7a,PC), A0; ($40c)

  - 3 Dip Switches which are told to be unused have an effect if
    [0x062d53] != 0x00 (check code at 0x00d7ea).
    As this value is ALWAYS set to 0x00 via code at 0x0001d4, I've
    added a #define LEGION_HACK you need to change to 1 if you want
    get the benefits of these Dip Switches.
    Here is what these "unused" Dip Switch do :

      * 1st unused Dip Switch (DSW0 bit 6) remains unused
      * 2nd unused Dip Switch (DSW0 bit 7) determines if the "Invulnerability"
        Dip Switches will be tested
      * 3rd unused Dip Switch (DSW1 bit 6) gives invulnerability or not
        to player 1 (DSW0 bit 7 must be ON too !)
      * 4th unused Dip Switch (DSW1 bit 7) gives invulnerability or not
        to player 2 (DSW0 bit 7 must be ON too !)

    Also note bit 1 of 0x062d53 is also tested but I haven't been able
    to find its purpose (any more infos are welcome)


2a) 'terraf'

  - The ROM test (code at 0x000292) ALWAYS displays "OK", but memory is
    in fact NEVER scanned ! Original behaviour or is the game a bootleg ?

    000292: 45F8 0000                lea     $0.w, A2
    000296: 303C 7FFF                move.w  #$7fff, D0
    00029A: 7200                     moveq   #$0, D1
    00029C: D29A                     add.l   (A2)+, D1
    00029E: 0C81 0000 0000           cmpi.l  #$0, D1
    0002A4: 4E71                     nop
    0002A6: 23FC 004F 004B 0006 83AA move.l  #$4f004b, $683aa.l
    0002B0: 4EF9 0000 0124           jmp     $124.l
    ...
    0002C2: 4EF9 0000 0124           jmp     $124.l


2b) 'terrafu'

  - The ROM test (code at 0x000292) NEVER displays "OK", but memory is
    in fact NEVER scanned ! Original behaviour or is the game a bootleg ?

    000292: 45F8 0000                lea     $0.w, A2
    000296: 303C 7FFF                move.w  #$7fff, D0
    00029A: 7200                     moveq   #$0, D1
    00029C: D29A                     add.l   (A2)+, D1
    00029E: 0C81 0000 0000           cmpi.l  #$0, D1
    0002A4: 661C                     bne     2c2
    0002A6: 23FC 004F 004B 0006 83AA move.l  #$4f004b, $683aa.l
    0002B0: 4EF9 0000 0124           jmp     $124.l
    ...
    0002C2: 4EF9 0000 0124           jmp     $124.l


3)  'kozure'

  - The ROM test (code at 0x004fac) checks range 0x000000-0x05ffff
    and reports an error if the checksum isn't correct.


4)  'cclimbr2'

  - The ROM test (code at 0x012f6e) checks ranges 0x000100-0x014fff,
    and 0x020000-0x024fff, and reports an error if the checksum isn't
    correct.


5)  'armedf'

  - The ROM test (code at 0x00df5e) checks ranges 0x000100-0x014fff,
    0x020000-0x024fff and 0x040000-0x04ffff, and reports an error if
    the checksum isn't correct.

========================================================================

 DIP locations verified for:
  -cclimbr2
  -legion
  -terraf

========================================================================

Tatakae! Big Fighter (c)1989 Nichibutsu

 based on armedf.c

 TODO:
 - scroll
 - controls
 - dips

    $80600($80000 ?? ) - $80fff  = shared ram with 8751 MCU

    controls :

    02E3E8: move.w  $8c000.l, $8064a.l
    02E3F2: move.w  $8c002.l, $8064c.l
    02E3FC: move.b  #$1, $80640.l
    02E404: move.w  $400000.l, D0
    02E40A: tst.b   $80640.l
    02E410: bne     2e3fc
    02E412: move.w  $80642.l, $8064e.l
    02E41C: move.w  $80644.l, $80650.l
    02E426: move.w  $80646.l, $80652.l - input 1 (80646)
    02E430: move.w  $80648.l, $80654.l
    02E43A: rts


------------------------------------------------------------------------
Tatakae! Big Fighter
Nichibutsu, 1989

This is a horizontal shoot'em-up similar to R-Type.

It appears this PCB is re-used? Sticker says PCB number is 1706 and (C) 1989
On the PCB under the sticker is written 1605 and (C) 1988


PCB Layout
----------


1605A-1 (1706-1)
-------------------------------------------------------------------
|                     2018                                        |
|              6.15F  2018                                        |
|  PROM.13H    5.13F                                              |
|                                                                 |
|                                                                 |
|                            5814                                 |
|                            5814                                 |
|                                                                 |
|                     5814                                        |
|                     5814                                        |
|                                           7.11C                 |
|                                                                 |
|                     6264   6264                                 |
|                     6264   6264                                 |
|                   ----------------                              |
|                   |1706-3        |                              |
|                   |              |                  PAL         |
|                   | PAL  8751    |                              |
|                   |          PAL |                              |
|                   |              |                              |
|                   |              |                  PAL         |
|                   | 2.IC4  4.IC5 |                              |
|                   | 1.IC2  3.IC3 |                              |
| DSW2              |              |                              |
| DSW1              |    68000     |    16MHz         PAL         |
|                   ----------------                              |
-------------------------------------------------------------------


1605B (1706-2)
-------------------------------------------------------------------
|                                                                 |
| 8.17K  Z80A                2018                                 |
|                            2018                                 |
| YM3812  2018               2018                                 |
| Y3014B                                                          |
|                                                                 |
|                                                                 |
|                                                                 |
|                                                                 |
|                                                                 |
|                                                                 |
|                                                                 |
|                                  2018                           |
|                          10.9D   2018                           |
|                           9.8D                          12.8A   |
|                           PAL                           11.6A   |
|                                                                 |
|                                                                 |
|                                                                 |
|                                                                 |
| 2018  2018                                                      |
| 2018  2018                                                      |
|                                                                 |
|               24MHz                                             |
|                                                                 |
-------------------------------------------------------------------


Notes:
      Horizontal Sync: 15.08kHz
        Vertical Sync: 60Hz
            68K Clock: 7.998MHz
            Z80 Clock: ? (unstable, probably 6MHz or less)


***********************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "sound/3812intf.h"
#include "includes/armedf.h"

#define LEGION_HACK 0


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

/*
    -x-- ---- ---- ---- trigger 1414M4 operation
    ---x ---- ---- ---- flip screen
    ---- x--- ---- ---- disable bg layer
    ---- -x-- ---- ---- disable fg layer
    ---- --x- ---- ---- disable sprite
    ---- ---x ---- ---- disable tx layer
    ---- ---- ---- --x- coin counter 1
    ---- ---- ---- ---x coin counter 0
*/

WRITE16_MEMBER(armedf_state::terraf_io_w)
{
	if(data & 0x4000 && ((m_vreg & 0x4000) == 0)) //0 -> 1 transition
		m_nb1414m4->exec((m_text_videoram[0] << 8) | (m_text_videoram[1] & 0xff),m_text_videoram.get(),m_fg_scrollx,m_fg_scrolly,m_tx_tilemap);


	COMBINE_DATA(&m_vreg);

	machine().bookkeeping().coin_counter_w(0, (data & 1) >> 0);
	machine().bookkeeping().coin_counter_w(1, (data & 2) >> 1);

	flip_screen_set(m_vreg & 0x1000);
}

WRITE16_MEMBER(armedf_state::terrafjb_io_w)
{
	if(data & 0x4000 && ((m_vreg & 0x4000) == 0)) //0 -> 1 transition
		m_extra->set_input_line(0, HOLD_LINE);

	COMBINE_DATA(&m_vreg);

	machine().bookkeeping().coin_counter_w(0, (data & 1) >> 0);
	machine().bookkeeping().coin_counter_w(1, (data & 2) >> 1);

	flip_screen_set(m_vreg & 0x1000);
}

WRITE16_MEMBER(armedf_state::bootleg_io_w)
{
	if(data & 0x4000 && ((m_vreg & 0x4000) == 0)) //0 -> 1 transition
	{
		// NOP
	}

	COMBINE_DATA(&m_vreg);

	machine().bookkeeping().coin_counter_w(0, (data & 1) >> 0);
	machine().bookkeeping().coin_counter_w(1, (data & 2) >> 1);

	flip_screen_set(m_vreg & 0x1000);
}

WRITE16_MEMBER(armedf_state::sound_command_w)
{
	if (ACCESSING_BITS_0_7)
		soundlatch_byte_w(space, 0, ((data & 0x7f) << 1) | 1);
}

READ8_MEMBER(armedf_state::soundlatch_clear_r)
{
	soundlatch_clear_byte_w(space, 0, 0);
	return 0;
}

WRITE16_MEMBER(armedf_state::irq_lv1_ack_w)
{
	m_maincpu->set_input_line(1, CLEAR_LINE);
}

WRITE16_MEMBER(armedf_state::irq_lv2_ack_w)
{
	m_maincpu->set_input_line(2, CLEAR_LINE);
}



/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( terraf_map, AS_PROGRAM, 16, armedf_state )
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x060000, 0x0603ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x060400, 0x063fff) AM_RAM
	AM_RANGE(0x064000, 0x064fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x068000, 0x069fff) AM_READWRITE8(nb1414m4_text_videoram_r,nb1414m4_text_videoram_w,0x00ff)
	AM_RANGE(0x06a000, 0x06a9ff) AM_RAM
	AM_RANGE(0x06c000, 0x06cfff) AM_RAM AM_SHARE("spr_pal_clut")
	AM_RANGE(0x070000, 0x070fff) AM_RAM_WRITE(armedf_fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0x074000, 0x074fff) AM_RAM_WRITE(armedf_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0x078000, 0x078001) AM_READ_PORT("P1")
	AM_RANGE(0x078002, 0x078003) AM_READ_PORT("P2")
	AM_RANGE(0x078004, 0x078005) AM_READ_PORT("DSW1")
	AM_RANGE(0x078006, 0x078007) AM_READ_PORT("DSW2")
//  AM_RANGE(0x07c000, 0x07c001) AM_WRITE(terraf_io_w) handled in DRIVER_INIT
	AM_RANGE(0x07c002, 0x07c003) AM_WRITE(armedf_bg_scrollx_w)
	AM_RANGE(0x07c004, 0x07c005) AM_WRITE(armedf_bg_scrolly_w)
	AM_RANGE(0x07c00a, 0x07c00b) AM_WRITE(sound_command_w)
	AM_RANGE(0x07c00c, 0x07c00d) AM_WRITENOP                    /* Watchdog ? cycle 0000 -> 0100 -> 0200 back to 0000 */
	AM_RANGE(0x07c00e, 0x07c00f) AM_WRITE(irq_lv1_ack_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( kozure_map, AS_PROGRAM, 16, armedf_state )
	AM_RANGE(0x060000, 0x060fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x061000, 0x063fff) AM_RAM
//  AM_RANGE(0x07c000, 0x07c001) AM_WRITE(kozure_io_w)
//  AM_RANGE(0x0c0000, 0x0c0001) AM_WRITENOP /* watchdog? */
//  AM_RANGE(0xffd000, 0xffd001) AM_WRITENOP /* passes crc ROM information to MCU, I guess */
	AM_IMPORT_FROM( terraf_map )
ADDRESS_MAP_END

static ADDRESS_MAP_START( cclimbr2_map, AS_PROGRAM, 16, armedf_state )
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x060000, 0x060fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x061000, 0x063fff) AM_RAM
	AM_RANGE(0x064000, 0x064fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x068000, 0x069fff) AM_READWRITE8(nb1414m4_text_videoram_r,nb1414m4_text_videoram_w,0x00ff)
	AM_RANGE(0x06a000, 0x06a9ff) AM_RAM
	AM_RANGE(0x06c000, 0x06cfff) AM_RAM AM_SHARE("spr_pal_clut")
	AM_RANGE(0x070000, 0x070fff) AM_RAM_WRITE(armedf_fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0x074000, 0x074fff) AM_RAM_WRITE(armedf_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0x078000, 0x078001) AM_READ_PORT("P1")
	AM_RANGE(0x078002, 0x078003) AM_READ_PORT("P2")
	AM_RANGE(0x078004, 0x078005) AM_READ_PORT("DSW1")
	AM_RANGE(0x078006, 0x078007) AM_READ_PORT("DSW2")
//  AM_RANGE(0x07c000, 0x07c001) AM_WRITE(io_w)
	AM_RANGE(0x07c002, 0x07c003) AM_WRITE(armedf_bg_scrollx_w)
	AM_RANGE(0x07c004, 0x07c005) AM_WRITE(armedf_bg_scrolly_w)
	AM_RANGE(0x07c00a, 0x07c00b) AM_WRITE(sound_command_w)
	AM_RANGE(0x07c00c, 0x07c00d) AM_WRITENOP /* Watchdog ? cycle 0000 -> 0100 -> 0200 back to 0000 */
	AM_RANGE(0x07c00e, 0x07c00f) AM_WRITE(irq_lv2_ack_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( legion_map, AS_PROGRAM, 16, armedf_state )
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x060000, 0x060fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x061000, 0x063fff) AM_RAM
	AM_RANGE(0x064000, 0x064fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x068000, 0x069fff) AM_READWRITE8(nb1414m4_text_videoram_r,nb1414m4_text_videoram_w,0x00ff)
	AM_RANGE(0x06a000, 0x06a9ff) AM_RAM
	AM_RANGE(0x06c000, 0x06cfff) AM_RAM AM_SHARE("spr_pal_clut")
	AM_RANGE(0x070000, 0x070fff) AM_RAM_WRITE(armedf_fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0x074000, 0x074fff) AM_RAM_WRITE(armedf_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0x078000, 0x078001) AM_READ_PORT("P1")
	AM_RANGE(0x078002, 0x078003) AM_READ_PORT("P2")
	AM_RANGE(0x078004, 0x078005) AM_READ_PORT("DSW1")
	AM_RANGE(0x078006, 0x078007) AM_READ_PORT("DSW2")
//  AM_RANGE(0x07c000, 0x07c001) AM_WRITE(legion_io_w)
	AM_RANGE(0x07c002, 0x07c003) AM_WRITE(armedf_bg_scrollx_w)
	AM_RANGE(0x07c004, 0x07c005) AM_WRITE(armedf_bg_scrolly_w)
	AM_RANGE(0x07c00a, 0x07c00b) AM_WRITE(sound_command_w)
	AM_RANGE(0x07c00c, 0x07c00d) AM_WRITENOP        /* Watchdog ? cycle 0000 -> 0100 -> 0200 back to 0000 */
	AM_RANGE(0x07c00e, 0x07c00f) AM_WRITE(irq_lv2_ack_w)
ADDRESS_MAP_END

WRITE8_MEMBER(armedf_state::legionjb_fg_scroll_w)
{
	if(offset >= 0xb && offset < 0xf)
		m_legion_cmd[offset-0xb] = data & 0xff;

	m_fg_scrollx = (m_legion_cmd[0x02] & 0xff) | ((m_legion_cmd[0x03] & 0x3) << 8);
	m_fg_scrolly = (m_legion_cmd[0x00] & 0xff) | ((m_legion_cmd[0x01] & 0x3) << 8);
}

static ADDRESS_MAP_START( legionjb_map, AS_PROGRAM, 16, armedf_state )
	AM_RANGE(0x040000, 0x04003f) AM_WRITE8(legionjb_fg_scroll_w,0x00ff)
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x060000, 0x060fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x061000, 0x063fff) AM_RAM
	AM_RANGE(0x064000, 0x064fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x068000, 0x069fff) AM_READWRITE8(armedf_text_videoram_r,armedf_text_videoram_w,0x00ff)
	AM_RANGE(0x06a000, 0x06a9ff) AM_RAM
	AM_RANGE(0x06c000, 0x06cfff) AM_RAM AM_SHARE("spr_pal_clut")
	AM_RANGE(0x070000, 0x070fff) AM_RAM_WRITE(armedf_fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0x074000, 0x074fff) AM_RAM_WRITE(armedf_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0x078000, 0x078001) AM_READ_PORT("P1")
	AM_RANGE(0x078002, 0x078003) AM_READ_PORT("P2")
	AM_RANGE(0x078004, 0x078005) AM_READ_PORT("DSW1")
	AM_RANGE(0x078006, 0x078007) AM_READ_PORT("DSW2")
//  AM_RANGE(0x07c000, 0x07c001) AM_WRITE(bootleg_io_w)
	AM_RANGE(0x07c002, 0x07c003) AM_WRITE(armedf_bg_scrollx_w)
	AM_RANGE(0x07c004, 0x07c005) AM_WRITE(armedf_bg_scrolly_w)
	AM_RANGE(0x07c00a, 0x07c00b) AM_WRITE(sound_command_w)
	AM_RANGE(0x07c00c, 0x07c00d) AM_WRITENOP      /* Watchdog ? cycle 0000 -> 0100 -> 0200 back to 0000 */
	AM_RANGE(0x07c00e, 0x07c00f) AM_WRITE(irq_lv2_ack_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( armedf_map, AS_PROGRAM, 16, armedf_state )
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x060000, 0x060fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x061000, 0x065fff) AM_RAM
	AM_RANGE(0x066000, 0x066fff) AM_RAM_WRITE(armedf_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0x067000, 0x067fff) AM_RAM_WRITE(armedf_fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0x068000, 0x069fff) AM_READWRITE8(armedf_text_videoram_r,armedf_text_videoram_w,0x00ff)
	AM_RANGE(0x06a000, 0x06afff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x06b000, 0x06bfff) AM_RAM AM_SHARE("spr_pal_clut")
	AM_RANGE(0x06c000, 0x06c001) AM_READ_PORT("P1")
	AM_RANGE(0x06c002, 0x06c003) AM_READ_PORT("P2")
	AM_RANGE(0x06c004, 0x06c005) AM_READ_PORT("DSW1")
	AM_RANGE(0x06c006, 0x06c007) AM_READ_PORT("DSW2")
	AM_RANGE(0x06c000, 0x06c7ff) AM_RAM
	AM_RANGE(0x06d000, 0x06d001) AM_WRITE(terraf_io_w)
	AM_RANGE(0x06d002, 0x06d003) AM_WRITE(armedf_bg_scrollx_w)
	AM_RANGE(0x06d004, 0x06d005) AM_WRITE(armedf_bg_scrolly_w)
	AM_RANGE(0x06d006, 0x06d007) AM_WRITE(armedf_fg_scrollx_w)
	AM_RANGE(0x06d008, 0x06d009) AM_WRITE(armedf_fg_scrolly_w)
	AM_RANGE(0x06d00a, 0x06d00b) AM_WRITE(sound_command_w)
	AM_RANGE(0x06d00c, 0x06d00d) AM_WRITENOP //watchdog
	AM_RANGE(0x06d00e, 0x06d00f) AM_WRITE(irq_lv1_ack_w)
ADDRESS_MAP_END

READ16_MEMBER(bigfghtr_state::latch_r)
{
	m_read_latch = 1;
	return 0;
}

WRITE16_MEMBER(bigfghtr_state::sharedram_w)
{
	COMBINE_DATA(&m_sharedram[offset]);

	switch(offset)
	{
		case 0x40/2:
			m_mcu_input_snippet = (data == 0x100);
			m_mcu_jsr_snippet = (data == 0x300);
			break;
	}
}

READ16_MEMBER(bigfghtr_state::sharedram_r)
{
	if(m_mcu_input_snippet)
	{
		switch(offset+0x600/2)
		{
			case 0x640/2:
				if(m_read_latch)
				{
					m_read_latch = 0;
					return machine().rand(); // TODO
				}
				break;

			case 0x642/2:
				return (ioport("DSW0")->read() & 0xffff) ^ 0xffff;

			case 0x644/2:
				return (ioport("DSW1")->read() & 0xffff) ^ 0xffff;

			case 0x646/2:
				return (ioport("P1")->read() & 0xffff) ^ 0xffff;

			case 0x648/2:
				return (ioport("P2")->read() & 0xffff) ^ 0xffff;
		}
	}

	if(m_mcu_jsr_snippet)
	{
		switch(offset+0x600/2)
		{
			case 0x640/2:
				if(m_read_latch)
				{
					m_read_latch = 0;
					return machine().rand(); // TODO
				}
				break;
			case 0x642/2:
				return (ioport("DSW0")->read() & 0xffff) ^ 0xffff;

			case 0x644/2:
				return (ioport("DSW1")->read() & 0xffff) ^ 0xffff;

			case 0x646/2:
				return (ioport("P1")->read() & 0xffff) ^ 0xffff;

			case 0x648/2:
				return (ioport("P2")->read() & 0xffff) ^ 0xffff;

			/*
			protection controls where the program code should jump to.

			example snippet:
			00DB2A: 41FA FE86                  lea     (-$17a,PC), A0; ($d9b2) ;base program vector
			00DB2E: 4DF9 0008 0E2A             lea     $80e2a.l, A6 ;base RAM vector, used by the i8751 to send the value, this value is added to the above A0
			00DB34: 3039 0008 0E62             move.w  $80e62.l, D0 ;number of snippets to execute
			00DB3A: 6100 00F0                  bsr     $dc2c
			*/

			/* bp daee, A0 = 0xdd02, A6 = 0x808ca, D0 = 0x80902 */
			//case 0x902/2:
			//  return 0x0001;
			//case (0x90a+0x24)/2:
			//  return 0x0001;
			//case (0x902+8)/2:
			//  return 0x0004; // 0xf86a

			/* bp db02, A0 = 0xdc2e, A6 = 0x80912, D0 = 0x8094a */
			//case 0x94a/2:
			//  return 1;
			//case (0x94a+8)/2:
			//  return 0x00dc; // 0xd62e

			/* bp db16, A0 = 0xda86, A6 = 0x80c22, D0 = 0x80c5a */
			//case 0xc5a/2:
			//  return 1;
			//case (0xc5a+8)/2:
			//  return 0x0288; // 0x345f4

			/* bp db2a, A0 = 0xd9b2, A6 = 0x80e2a, D0 = 0x80e62 */

			/* bp db3e, A0 = 0xd8da, A6 = 0x81132, D0 = 0x8116a */

			/* bp db52, A0 = 0xd806, A6 = 0x8133a, D0 = 0x81372 */

			/* bp db66, A0 = 0xd7aa, A6 = 0x81742, D0 = 0x8177a */

			/* bp db7a, A0 = 0xd746, A6 = 0x81b4a, D0 = 0x81b82 */

			/* bp db8e, A0 = 0xd672, A6 = 0x81f52, D0 = 0x81f8a */

			/* bp dba4, A0 = 0xd5ae, A6 = 0x8205a, D0 = 0x82092 */

			/* bp dbba, A0 = 0xd5be, A6 = 0x82862, D0 = 0x8289a */

			/* bp dbd0, A0 = 0xd512, A6 = 0x8296a, D0 = 0x829a2 */

			/* bp dbe6, A0 = 0xd466, A6 = 0x82d72, D0 = 0x82daa */

			/* bp dbfc, A0 = 0xd43e, A6 = 0x8357a, D0 = 0x835b2 */

			/* following is separated from the others, dunno why ... */
			/* bp dc14, A0 = 0xd3aa, A6 = 0x835a2, D0 = 0x835b2 */


			/*case 0x
			case 0x94a/2:
			    return 0x0002*4;
			case (0x90a+2*0x40)/2:
			case (0x90a+3*0x40)/2:
			    return 0x0003*4;
			case (0x90a+4*0x40)/2:
			    return 0x000c*4; // 0x13d74
			case (0x90a+5*0x40)/2:
			    return 0x000d*4; // 0x130f6
			case (0x90a+6*0x40)/2:
			    return 0x000e*4; // 0x1817e
			case (0x90a+7*0x40)/2:
			    return 0x0010*4; // 0x15924
			//case (0x90a+0x25)/2:
			//  return 2;*/
		}
	}

	return m_sharedram[offset];
}

static ADDRESS_MAP_START( bigfghtr_map, AS_PROGRAM, 16, bigfghtr_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x0805ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x080600, 0x083fff) AM_READWRITE(sharedram_r, sharedram_w) AM_SHARE("sharedram")
	AM_RANGE(0x084000, 0x085fff) AM_RAM //work ram
	AM_RANGE(0x086000, 0x086fff) AM_RAM_WRITE(armedf_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0x087000, 0x087fff) AM_RAM_WRITE(armedf_fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0x088000, 0x089fff) AM_READWRITE8(armedf_text_videoram_r,armedf_text_videoram_w,0x00ff)
	AM_RANGE(0x08a000, 0x08afff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x08b000, 0x08bfff) AM_RAM AM_SHARE("spr_pal_clut")
	AM_RANGE(0x08c000, 0x08c001) AM_READ_PORT("P1")
	AM_RANGE(0x08c002, 0x08c003) AM_READ_PORT("P2")
	AM_RANGE(0x08c004, 0x08c005) AM_READ_PORT("DSW0")
	AM_RANGE(0x08c006, 0x08c007) AM_READ_PORT("DSW1")
	AM_RANGE(0x08d000, 0x08d001) AM_WRITE(terraf_io_w)  //807b0
	AM_RANGE(0x08d002, 0x08d003) AM_WRITE(armedf_bg_scrollx_w)
	AM_RANGE(0x08d004, 0x08d005) AM_WRITE(armedf_bg_scrolly_w)
	AM_RANGE(0x08d006, 0x08d007) AM_WRITE(armedf_fg_scrollx_w)
	AM_RANGE(0x08d008, 0x08d009) AM_WRITE(armedf_fg_scrolly_w)
	AM_RANGE(0x08d00a, 0x08d00b) AM_WRITE(sound_command_w)
	AM_RANGE(0x08d00c, 0x08d00d) AM_WRITENOP //watchdog
	AM_RANGE(0x08d00e, 0x08d00f) AM_WRITE(irq_lv1_ack_w)

	AM_RANGE(0x400000, 0x400001) AM_READ(latch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, armedf_state )
	AM_RANGE(0x0000, 0xf7ff) AM_ROM
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cclimbr2_soundmap, AS_PROGRAM, 8, armedf_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xffff) AM_RAM
ADDRESS_MAP_END

READ8_MEMBER(armedf_state::blitter_txram_r)
{
	return m_text_videoram[offset];
}

WRITE8_MEMBER(armedf_state::blitter_txram_w)
{
	m_text_videoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(armedf_state::fg_scrollx_w)
{
	m_fg_scrollx = (data & 0xff) | (m_fg_scrollx & 0x300);
}

WRITE8_MEMBER(armedf_state::fg_scrolly_w)
{
	m_fg_scrolly = (data & 0xff) | (m_fg_scrolly & 0x300);
}

WRITE8_MEMBER(armedf_state::fg_scroll_msb_w)
{
	m_fg_scrolly = (((data & 0x03) >> 0) << 8) | (m_fg_scrolly & 0xff);
	m_fg_scrollx = (((data & 0x0c) >> 2) << 8) | (m_fg_scrollx & 0xff);
}


static ADDRESS_MAP_START( terrafjb_extraz80_map, AS_PROGRAM, 8, armedf_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x5fff) AM_READWRITE(blitter_txram_r,blitter_txram_w)
	AM_RANGE(0x8000, 0x87ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( terrafjb_extraz80_portmap, AS_IO, 8, armedf_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00,0x00) AM_WRITE(fg_scrollx_w)
	AM_RANGE(0x01,0x01) AM_WRITE(fg_scrolly_w)
	AM_RANGE(0x02,0x02) AM_WRITE(fg_scroll_msb_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap, AS_IO, 8, armedf_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x0, 0x1) AM_DEVWRITE("ymsnd", ym3812_device, write)
	AM_RANGE(0x2, 0x2) AM_DEVWRITE("dac1", dac_device, write_signed8)
	AM_RANGE(0x3, 0x3) AM_DEVWRITE("dac2", dac_device, write_signed8)
	AM_RANGE(0x4, 0x4) AM_READ(soundlatch_clear_r)
	AM_RANGE(0x6, 0x6) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

#define NIHON_SINGLE_JOYSTICK(_n_) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(_n_) \
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(_n_) \
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(_n_) \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(_n_) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

static INPUT_PORTS_START( armedf )
	PORT_START("P1")
	NIHON_SINGLE_JOYSTICK(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	NIHON_SINGLE_JOYSTICK(2)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x0200, IP_ACTIVE_LOW )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0xf800, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )                PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	/* SW1:3,4 defined in manual/test-mode as:
	PORT_DIPNAME( 0x04, 0x04, "1st Bonus Life" )                PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "20k" )
	PORT_DIPSETTING(    0x00, "40k" )
	PORT_DIPNAME( 0x08, 0x08, "2nd Bonus Life" )                PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "60k" )
	PORT_DIPSETTING(    0x00, "80k" )*/
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )           PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "20k then every 60k" )
	PORT_DIPSETTING(    0x04, "20k then every 80k" )
	PORT_DIPSETTING(    0x08, "40k then every 60k" )
	PORT_DIPSETTING(    0x00, "40k then every 80k" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )              PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:5,6") // not in the "test mode"
	PORT_DIPSETTING(    0x30, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, "3 Times" )
	PORT_DIPSETTING(    0x10, "5 Times" )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW2:7") // not in the "test mode"
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( legion )
	PORT_INCLUDE( armedf )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Bonus_Life ) )           PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "30k Then Every 100k" )
	PORT_DIPSETTING(    0x00, "50k Only" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:6" )                /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )                /* Listed as "Unused" */
#if LEGION_HACK // see notes
	PORT_DIPNAME( 0x80, 0x80, "Allow Invulnerability (Cheat)" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
#else
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )                /* Listed as "Unused" */
#endif

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x10, 0x10, "Coin Slots" )                    PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Common" )
	PORT_DIPSETTING(    0x00, "Individual" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
#if LEGION_HACK // see notes
	PORT_DIPNAME( 0x40, 0x40, "P1 Invulnerability (Cheat)")     PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "P2 Invulnerability (Cheat)")     PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
#else
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )                /* Listed as "Unused" */
#endif
INPUT_PORTS_END


static INPUT_PORTS_START( terraf )
	PORT_INCLUDE( armedf )

	PORT_MODIFY("DSW1")
	/* SW1:3,4 defined in manual/test-mode as:
	PORT_DIPNAME( 0x04, 0x04, "1st Bonus Life" )                PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "20k" )
	PORT_DIPSETTING(    0x00, "50k" )
	PORT_DIPNAME( 0x08, 0x08, "2nd Bonus Life" )                PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "60k" )
	PORT_DIPSETTING(    0x00, "90k" )*/
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )           PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "20k then every 60k" )
	PORT_DIPSETTING(    0x04, "20k then every 90k" )
	PORT_DIPSETTING(    0x08, "50k then every 60k" )
	PORT_DIPSETTING(    0x00, "50k then every 90k" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )


	PORT_MODIFY("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )                /* Listed as "Unused" */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, "Only 3 Times" )
	PORT_DIPSETTING(    0x40, "Only 5 Times" )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( kozure )
	PORT_INCLUDE( terraf )

	PORT_MODIFY("DSW1")
	/* SW1:3,4 defined in manual/test-mode as:
	PORT_DIPNAME( 0x04, 0x04, "1st Bonus Life" )                PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( None ) )
	PORT_DIPSETTING(    0x00, "50k" )
	PORT_DIPNAME( 0x08, 0x08, "2nd Bonus Life" )                PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "60k" )
	PORT_DIPSETTING(    0x00, "90k" )*/
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )           PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x08, "50k then every 60k" )
	PORT_DIPSETTING(    0x00, "50k then every 90k" )
	PORT_DIPSETTING(    0x0c, "Every 60k" )
	PORT_DIPSETTING(    0x04, "Every 90k" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )                /* Listed as "Unused" */

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Infinite Timer (Cheat)" )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cclimbr2 )
	PORT_INCLUDE( armedf )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_PLAYER(1)

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_MODIFY("DSW1")
	/* SW1:3,4 defined in manual/test-mode as:
	PORT_DIPNAME( 0x04, 0x04, "1st Bonus Life" )                    PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "30k" )
	PORT_DIPSETTING(    0x00, "60k" )
	PORT_DIPNAME( 0x08, 0x08, "2nd Bonus Life" )                    PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "70k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )*/
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )               PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "30K and 100k" )
	PORT_DIPSETTING(    0x08, "60k and 130k" )
	PORT_DIPSETTING(    0x04, "30k only" )
	PORT_DIPSETTING(    0x00, "60k only" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )               PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )                    /* Listed as "Unused" */

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )           PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, "3 Times" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )              PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Partial Invulnerability (Cheat)")    PORT_DIPLOCATION("SW2:7") /* Listed as "Unused" */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )                    /* Listed as "Unused" */
INPUT_PORTS_END

static INPUT_PORTS_START( bigfghtr )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0200, IP_ACTIVE_LOW )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0xf800, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "20k then every 60k" )
	PORT_DIPSETTING(    0x04, "20k then every 80k" )
	PORT_DIPSETTING(    0x08, "40k then every 60k" )
	PORT_DIPSETTING(    0x00, "40k then every 80k" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x30, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, "3 Times" )
	PORT_DIPSETTING(    0x10, "5 Times" )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout char_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout tile_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24,
			32+4, 32+0, 32+12, 32+8, 32+20, 32+16, 32+28, 32+24 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8
};

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 12, 8, RGN_FRAC(1,2)+12, RGN_FRAC(1,2)+8,
			20, 16, RGN_FRAC(1,2)+20, RGN_FRAC(1,2)+16, 28, 24, RGN_FRAC(1,2)+28, RGN_FRAC(1,2)+24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	64*8
};

static GFXDECODE_START( armedf )
	GFXDECODE_ENTRY( "gfx1", 0, char_layout,         0*16,  32 )
	GFXDECODE_ENTRY( "gfx2", 0, tile_layout,        64*16,  32 )
	GFXDECODE_ENTRY( "gfx3", 0, tile_layout,        96*16,  32 )
	GFXDECODE_ENTRY( "gfx4", 0, sprite_layout,  32*16,  32 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_START_MEMBER(armedf_state,armedf)
{
	save_item(NAME(m_old_mcu_mode));
	save_item(NAME(m_scroll_msb));
	save_item(NAME(m_waiting_msb));
	save_item(NAME(m_vreg));
	save_item(NAME(m_fg_scrollx));
	save_item(NAME(m_fg_scrolly));
	save_item(NAME(m_bg_scrollx));
	save_item(NAME(m_bg_scrolly));
}

MACHINE_RESET_MEMBER(armedf_state,armedf)
{
	m_old_mcu_mode = 0;
	m_scroll_msb = 0;
	m_waiting_msb = 0;
	m_vreg = 0;
	m_fg_scrollx = 0;
	m_fg_scrolly = 0;
	m_bg_scrollx = 0;
	m_bg_scrolly = 0;
}


static MACHINE_CONFIG_START( terraf, armedf_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz/2)   // 8mhz?
	MCFG_CPU_PROGRAM_MAP(terraf_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", armedf_state,  irq1_line_assert)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz/2)      // 4mhz?
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(armedf_state, irq0_line_hold,  XTAL_8MHz/2/512)    // ?

	MCFG_MACHINE_START_OVERRIDE(armedf_state,armedf)
	MCFG_MACHINE_RESET_OVERRIDE(armedf_state,armedf)

	MCFG_DEVICE_ADD("nb1414m4", NB1414M4, 0)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(12*8, (64-12)*8-1, 1*8, 31*8-1 )
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(armedf_state,terraf)
	MCFG_SCREEN_UPDATE_DRIVER(armedf_state, screen_update_armedf)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", armedf)

	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)

	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_8MHz/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( terrafjb, armedf_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz/2)   // 8mhz?
	MCFG_CPU_PROGRAM_MAP(terraf_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", armedf_state,  irq1_line_assert)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz/2)      // 4mhz?
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(armedf_state, irq0_line_hold,  XTAL_8MHz/2/512)    // ?

	MCFG_CPU_ADD("extra", Z80, XTAL_8MHz/2)         // 4mhz?
	MCFG_CPU_PROGRAM_MAP(terrafjb_extraz80_map)
	MCFG_CPU_IO_MAP(terrafjb_extraz80_portmap)

	MCFG_MACHINE_START_OVERRIDE(armedf_state,armedf)
	MCFG_MACHINE_RESET_OVERRIDE(armedf_state,armedf)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(12*8, (64-12)*8-1, 1*8, 31*8-1 )
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(armedf_state,terraf)
	MCFG_SCREEN_UPDATE_DRIVER(armedf_state, screen_update_armedf)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", armedf)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)

	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_8MHz/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( kozure, armedf_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz/2)   // 8mhz?
	MCFG_CPU_PROGRAM_MAP(kozure_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", armedf_state,  irq1_line_assert)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz/2)      // 4mhz?
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(armedf_state, irq0_line_hold,  XTAL_8MHz/2/512)    // ?

	MCFG_MACHINE_START_OVERRIDE(armedf_state,armedf)
	MCFG_MACHINE_RESET_OVERRIDE(armedf_state,armedf)

	MCFG_DEVICE_ADD("nb1414m4", NB1414M4, 0)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(12*8, (64-12)*8-1, 1*8, 31*8-1 ) // 320 x 240, trusted
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(armedf_state,terraf)
	MCFG_SCREEN_UPDATE_DRIVER(armedf_state, screen_update_armedf)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", armedf)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)

	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_8MHz/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( armedf, armedf_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz/2)   // 8mhz?
	MCFG_CPU_PROGRAM_MAP(armedf_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", armedf_state,  irq1_line_assert)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz/2)      // 4mhz?
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(armedf_state, irq0_line_hold,  XTAL_8MHz/2/512)    // ?

	MCFG_MACHINE_START_OVERRIDE(armedf_state,armedf)
	MCFG_MACHINE_RESET_OVERRIDE(armedf_state,armedf)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(12*8, (64-12)*8-1, 1*8, 31*8-1 )
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(armedf_state,armedf)
	MCFG_SCREEN_UPDATE_DRIVER(armedf_state, screen_update_armedf)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", armedf)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)

	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_8MHz/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( cclimbr2, armedf_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz/2)   // 8mhz?
	MCFG_CPU_PROGRAM_MAP(cclimbr2_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", armedf_state,  irq2_line_assert)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz/2)      // 4mhz?
	MCFG_CPU_PROGRAM_MAP(cclimbr2_soundmap)
	MCFG_CPU_IO_MAP(sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(armedf_state, irq0_line_hold,  XTAL_8MHz/2/512)    // ?

	MCFG_MACHINE_START_OVERRIDE(armedf_state,armedf)
	MCFG_MACHINE_RESET_OVERRIDE(armedf_state,armedf)

	MCFG_DEVICE_ADD("nb1414m4", NB1414M4, 0)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(armedf_state,terraf)
	MCFG_SCREEN_UPDATE_DRIVER(armedf_state, screen_update_armedf)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", armedf)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)

	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_8MHz/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( legion, armedf_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz/2)   // 8mhz?
	MCFG_CPU_PROGRAM_MAP(legion_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", armedf_state,  irq2_line_assert)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz/2)      // 4mhz?
	MCFG_CPU_PROGRAM_MAP(cclimbr2_soundmap)
	MCFG_CPU_IO_MAP(sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(armedf_state, irq0_line_hold,  XTAL_8MHz/2/512)    // ?

	MCFG_MACHINE_START_OVERRIDE(armedf_state,armedf)
	MCFG_MACHINE_RESET_OVERRIDE(armedf_state,armedf)

	MCFG_DEVICE_ADD("nb1414m4", NB1414M4, 0)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(armedf_state,terraf)
	MCFG_SCREEN_UPDATE_DRIVER(armedf_state, screen_update_armedf)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", armedf)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)

	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_8MHz/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( legionjb, armedf_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz/2)   // 8mhz?
	MCFG_CPU_PROGRAM_MAP(legionjb_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", armedf_state,  irq2_line_assert)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz/2)      // 4mhz?
	MCFG_CPU_PROGRAM_MAP(cclimbr2_soundmap)
	MCFG_CPU_IO_MAP(sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(armedf_state, irq0_line_hold,  XTAL_8MHz/2/512)    // ?

	MCFG_MACHINE_START_OVERRIDE(armedf_state,armedf)
	MCFG_MACHINE_RESET_OVERRIDE(armedf_state,armedf)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(armedf_state,terraf)
	MCFG_SCREEN_UPDATE_DRIVER(armedf_state, screen_update_armedf)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", armedf)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)

	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_8MHz/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_CONFIG_END

MACHINE_START_MEMBER(bigfghtr_state,bigfghtr)
{
	MACHINE_START_CALL_MEMBER(armedf);
	save_item(NAME(m_read_latch));
}

MACHINE_RESET_MEMBER(bigfghtr_state,bigfghtr)
{
	MACHINE_RESET_CALL_MEMBER(armedf);
	m_read_latch = 0;
}

static MACHINE_CONFIG_START( bigfghtr, bigfghtr_state )

	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz/2)   // verified
	MCFG_CPU_PROGRAM_MAP(bigfghtr_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", armedf_state,  irq1_line_assert)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz/2)      // 4mhz?
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(armedf_state, irq0_line_hold,  XTAL_8MHz/2/512)    // ?

	MCFG_MACHINE_START_OVERRIDE(bigfghtr_state,bigfghtr)
	MCFG_MACHINE_RESET_OVERRIDE(bigfghtr_state,bigfghtr)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(12*8, (64-12)*8-1, 1*8, 31*8-1 )
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(armedf_state,armedf)
	MCFG_SCREEN_UPDATE_DRIVER(armedf_state, screen_update_armedf)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", armedf)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)

	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_8MHz/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( legion )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 64K*8 for 68000 code */
	ROM_LOAD16_BYTE( "lg1.bin", 0x000001, 0x010000, CRC(c4aeb724) SHA1(b4c0383f3b1fa6b1d5bdab0f3a5293c89a82a474) )
	ROM_LOAD16_BYTE( "lg3.bin", 0x000000, 0x010000, CRC(777e4935) SHA1(225766940059b4c12e69332ea77eb618dbd1467b) )
	ROM_LOAD16_BYTE( "legion.1b", 0x020001, 0x010000, CRC(c306660a) SHA1(31c6b868ba07677b5110c577335873354bff596f) ) // lg2
	ROM_LOAD16_BYTE( "legion.1d", 0x020000, 0x010000, CRC(c2e45e1e) SHA1(95cc359145b1b03123262891feed358407ba105a) ) // lg4

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 code (sound) */
	ROM_LOAD( "legion.1h", 0x00000, 0x04000, CRC(2ca4f7f0) SHA1(7cf997af9dd74ced9d28c047069ccfb67d72e257) ) // lg9
	ROM_LOAD( "legion.1i", 0x04000, 0x08000, CRC(79f4a827) SHA1(25e4c1b5b8466627244b7226310e67e4261333b6) ) // lg10

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "lg8.bin", 0x00000, 0x08000, CRC(e0596570) SHA1(68ddc950efc55a16e6abc699e3bad18ea19d579f) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "legion.1e", 0x00000, 0x10000, CRC(a9d70faf) SHA1(8b8b60ae49c55e931d6838e863463f6b2bf7adb0) ) // lg5
	ROM_LOAD( "legion.1f", 0x18000, 0x08000, CRC(f018313b) SHA1(860bc9937202dc3a40c9fa7caad11c2c2aa19f5c) ) // lg6

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "legion.1l", 0x00000, 0x10000, CRC(29b8adaa) SHA1(10338ebe7324960683de1f796dd311ed662e42b4) ) // lg13

	ROM_REGION( 0x20000, "gfx4", 0 )
	ROM_LOAD( "legion.1k", 0x000000, 0x010000, CRC(ff5a0db9) SHA1(9308deb363d3b7686cc69485ec14201dd68f9a97) ) // lg12
	ROM_LOAD( "legion.1j", 0x010000, 0x010000, CRC(bae220c8) SHA1(392ae0fb0351dcad7b0e8e0ed4a1dc6e07f493df) ) // lg11

	ROM_REGION( 0x4000, "nb1414m4", 0 )    /* data for mcu/blitter */
	ROM_LOAD ( "lg7.bin", 0x0000, 0x4000, CRC(533e2b58) SHA1(a13ea4a530038760ffa87713903c59a932452717) )
ROM_END

ROM_START( legionj )
	ROM_REGION( 0x60000, "maincpu", ROMREGION_ERASEFF ) /* 64K*8 for 68000 code */
	ROM_LOAD16_BYTE( "legion.e1", 0x000001, 0x010000, CRC(977fa324) SHA1(04432ecb0cab61731e17bcf665ca66fe34b2d75c) )
	ROM_LOAD16_BYTE( "legion.e5", 0x000000, 0x010000, CRC(49e8e1b7) SHA1(ed0b38aae3f46f689fe9d2c96c383d375716e77e) )
	ROM_LOAD16_BYTE( "legion.1b", 0x020001, 0x010000, CRC(c306660a) SHA1(31c6b868ba07677b5110c577335873354bff596f) )
	ROM_LOAD16_BYTE( "legion.1d", 0x020000, 0x010000, CRC(c2e45e1e) SHA1(95cc359145b1b03123262891feed358407ba105a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 code (sound) */
	ROM_LOAD( "legion.1h", 0x00000, 0x04000, CRC(2ca4f7f0) SHA1(7cf997af9dd74ced9d28c047069ccfb67d72e257) )
	ROM_LOAD( "legion.1i", 0x04000, 0x08000, CRC(79f4a827) SHA1(25e4c1b5b8466627244b7226310e67e4261333b6) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "legion.1g", 0x00000, 0x08000, CRC(c50b0125) SHA1(83b5e9707152d97777fb65fa8820ba34ec2fac8d) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "legion.1e", 0x00000, 0x10000, CRC(a9d70faf) SHA1(8b8b60ae49c55e931d6838e863463f6b2bf7adb0) )
	ROM_LOAD( "legion.1f", 0x18000, 0x08000, CRC(f018313b) SHA1(860bc9937202dc3a40c9fa7caad11c2c2aa19f5c) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "legion.1l", 0x00000, 0x10000, CRC(29b8adaa) SHA1(10338ebe7324960683de1f796dd311ed662e42b4) )

	ROM_REGION( 0x20000, "gfx4", 0 )
	ROM_LOAD( "legion.1k", 0x000000, 0x010000, CRC(ff5a0db9) SHA1(9308deb363d3b7686cc69485ec14201dd68f9a97) )
	ROM_LOAD( "legion.1j", 0x010000, 0x010000, CRC(bae220c8) SHA1(392ae0fb0351dcad7b0e8e0ed4a1dc6e07f493df) )

	ROM_REGION( 0x4000, "nb1414m4", 0 )    /* data for mcu/blitter */
	ROM_LOAD ( "lg7.bin", 0x0000, 0x4000, CRC(533e2b58) SHA1(a13ea4a530038760ffa87713903c59a932452717) )
ROM_END

ROM_START( legionjb )
	ROM_REGION( 0x60000, "maincpu", ROMREGION_ERASEFF ) /* 64K*8 for 68000 code */
	ROM_LOAD16_BYTE( "legion.1a", 0x000001, 0x010000, CRC(8c0cda1d) SHA1(14b93d4fb4381ebc6a4ccdb480089bf69c6f474b) )
	ROM_LOAD16_BYTE( "legion.1c", 0x000000, 0x010000, CRC(21226660) SHA1(ee48812d6ec9d4dccc58684164916f91b71aabf2) )
	ROM_LOAD16_BYTE( "legion.1b", 0x020001, 0x010000, CRC(c306660a) SHA1(31c6b868ba07677b5110c577335873354bff596f) )
	ROM_LOAD16_BYTE( "legion.1d", 0x020000, 0x010000, CRC(c2e45e1e) SHA1(95cc359145b1b03123262891feed358407ba105a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 code (sound) */
	ROM_LOAD( "legion.1h", 0x00000, 0x04000, CRC(2ca4f7f0) SHA1(7cf997af9dd74ced9d28c047069ccfb67d72e257) )
	ROM_LOAD( "legion.1i", 0x04000, 0x08000, CRC(79f4a827) SHA1(25e4c1b5b8466627244b7226310e67e4261333b6) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "legion.1g", 0x00000, 0x08000, CRC(c50b0125) SHA1(83b5e9707152d97777fb65fa8820ba34ec2fac8d) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "legion.1e", 0x00000, 0x10000, CRC(a9d70faf) SHA1(8b8b60ae49c55e931d6838e863463f6b2bf7adb0) )
	ROM_LOAD( "legion.1f", 0x18000, 0x08000, CRC(f018313b) SHA1(860bc9937202dc3a40c9fa7caad11c2c2aa19f5c) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "legion.1l", 0x00000, 0x10000, CRC(29b8adaa) SHA1(10338ebe7324960683de1f796dd311ed662e42b4) )

	ROM_REGION( 0x20000, "gfx4", 0 )
	ROM_LOAD( "legion.1k", 0x000000, 0x010000, CRC(ff5a0db9) SHA1(9308deb363d3b7686cc69485ec14201dd68f9a97) )
	ROM_LOAD( "legion.1j", 0x010000, 0x010000, CRC(bae220c8) SHA1(392ae0fb0351dcad7b0e8e0ed4a1dc6e07f493df) )
ROM_END

ROM_START( terraf )
	ROM_REGION( 0x60000, "maincpu", ROMREGION_ERASEFF ) /* 64K*8 for 68000 code */
	ROM_LOAD16_BYTE( "8.6e", 0x00000, 0x10000, CRC(fd58fa06) SHA1(f1f5fbd153be5fd5669aada66134baedfeac5d32) )
	ROM_LOAD16_BYTE( "3.6h", 0x00001, 0x10000, CRC(54823a7d) SHA1(bdf67890428710470a622ea48383b3fae8de8cbd) )
	ROM_LOAD16_BYTE( "7.4e", 0x20000, 0x10000, CRC(fde8de7e) SHA1(6b0d27ec49c8c0609c110ad97938bec8c077ad18) )
	ROM_LOAD16_BYTE( "2.4h", 0x20001, 0x10000, CRC(db987414) SHA1(0a1734794c626cf9083d7854c9000c5daadfc3fd) )
	ROM_LOAD16_BYTE( "6.3e", 0x40000, 0x10000, CRC(a5bb8c3b) SHA1(36730b1a4a686de7251b80eb084a00f64bd7b40d) )
	ROM_LOAD16_BYTE( "1.3h", 0x40001, 0x10000, CRC(d2de6d28) SHA1(0371b5bf8e6105aaf8847c4b6d0d2d7d105079c1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 code (sound) */
	ROM_LOAD( "11.17k", 0x00000, 0x10000,  CRC(4407d475) SHA1(96e86c7ef4dc997812436f7d0ddea332b4e6cb2b) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "9.11e", 0x00000, 0x08000, CRC(bc6f7cbc) SHA1(20b8a34de4bfa0c2fdcd2f7743a0ab35141f4bf9) ) /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "5.15h", 0x00000, 0x10000, CRC(25d23dfd) SHA1(da32895c1aca403209b7fb181fa4fa23a8e74d32) ) /* foreground tiles */
	ROM_LOAD( "4.13h", 0x10000, 0x10000, CRC(b9b0fe27) SHA1(983c48239ba1524b517f89f281f2b70564bea1e9) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "15.8a", 0x00000, 0x10000, CRC(2144d8e0) SHA1(ed89da11abf3d79753b478603009970c2600ab60) ) /* background tiles */
	ROM_LOAD( "14.6a", 0x10000, 0x10000, CRC(744f5c9e) SHA1(696223a087bb575c7cfaba11e682b221ada461e4) )

	ROM_REGION( 0x20000, "gfx4", 0 )
	ROM_LOAD( "12.7d", 0x00000, 0x10000, CRC(2d1f2ceb) SHA1(77544e1c4bda06feac135a96bb76af7c79278dc0) ) /* sprites */
	ROM_LOAD( "13.9d", 0x10000, 0x10000, CRC(1d2f92d6) SHA1(e842c6bf95a5958a6ca2c85e68b9bc3cc15211a4) )

	ROM_REGION( 0x4000, "nb1414m4", 0 )    /* data for mcu/blitter */
	ROM_LOAD( "10.11c", 0x0000, 0x4000, CRC(ac705812) SHA1(65be46ee959d8478cb6dffb25e61f7742276997b) )

	ROM_REGION( 0x0100, "proms", 0 )    /* Unknown use */
	ROM_LOAD( "n82s129an.11j", 0x0000, 0x0100, CRC(81244757) SHA1(6324f63e571f0f7a0bb9eb97f9994809db79493f) ) /* N82S129AN or compatible labled "TF" */
ROM_END

ROM_START( terrafu ) /* Bootleg of the USA version?, uses some roms common to bootlegs that differ to the orginal board sets */
	ROM_REGION( 0x60000, "maincpu", ROMREGION_ERASEFF ) /* 64K*8 for 68000 code */
	ROM_LOAD16_BYTE( "tf-8.6e", 0x00000, 0x10000, CRC(fea6dd64) SHA1(682eae338ce14808f134897f594fae1c69e75a1a) )
	ROM_LOAD16_BYTE( "tf-3.6h", 0x00001, 0x10000, CRC(02f9d05a) SHA1(88985373bc3cffbc838e0b701ecd732a417975a1) )
	ROM_LOAD16_BYTE( "tf-7.4e", 0x20000, 0x10000, CRC(fde8de7e) SHA1(6b0d27ec49c8c0609c110ad97938bec8c077ad18) )
	ROM_LOAD16_BYTE( "tf-2.4h", 0x20001, 0x10000, CRC(db987414) SHA1(0a1734794c626cf9083d7854c9000c5daadfc3fd) )
	ROM_LOAD16_BYTE( "tf-6.3e", 0x40000, 0x08000, CRC(b91e9ba3) SHA1(33e5272d1691859a2bb1f340eb4bdfdd5d73a5d4) ) /* Half the size of World & Japan sets?? */
	ROM_LOAD16_BYTE( "tf-1.3h", 0x40001, 0x08000, CRC(d6e22375) SHA1(c84fc19700b65ee36b0c7d75cd7c97f86c7f719d) ) /* bootlegs use this size rom for 1 & 6  */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 code (sound) */
	ROM_LOAD( "tf-001.17k", 0x00000, 0x10000, CRC(eb6b4138) SHA1(04c53bf46d87a156d3fad86f051985d0df79bd20) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "9.11e", 0x00000, 0x08000, CRC(bc6f7cbc) SHA1(20b8a34de4bfa0c2fdcd2f7743a0ab35141f4bf9) ) /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "5.15h", 0x00000, 0x10000, CRC(25d23dfd) SHA1(da32895c1aca403209b7fb181fa4fa23a8e74d32) ) /* foreground tiles */
	ROM_LOAD( "4.13h", 0x10000, 0x10000, CRC(b9b0fe27) SHA1(983c48239ba1524b517f89f281f2b70564bea1e9) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "15.8a", 0x00000, 0x10000, CRC(2144d8e0) SHA1(ed89da11abf3d79753b478603009970c2600ab60) ) /* background tiles */
	ROM_LOAD( "14.6a", 0x10000, 0x10000, CRC(744f5c9e) SHA1(696223a087bb575c7cfaba11e682b221ada461e4) )

	ROM_REGION( 0x20000, "gfx4", 0 )
	ROM_LOAD( "tf-003.7d", 0x00000, 0x10000, CRC(d74085a1) SHA1(3f6ba85dbd6e48a502c115b2d322a586fc4f56c9) ) /* sprites */
	ROM_LOAD( "tf-002.9d", 0x10000, 0x10000, CRC(148aa0c5) SHA1(8d8a565540e91b384a9c154522501921b7da4d4e) )

	ROM_REGION( 0x4000, "nb1414m4", 0 )    /* data for mcu/blitter */
	ROM_LOAD( "10.11c", 0x0000, 0x4000, CRC(ac705812) SHA1(65be46ee959d8478cb6dffb25e61f7742276997b) )

	ROM_REGION( 0x0100, "proms", 0 )    /* Unknown use */
	ROM_LOAD( "n82s129an.11j", 0x0000, 0x0100, CRC(81244757) SHA1(6324f63e571f0f7a0bb9eb97f9994809db79493f) ) /* N82S129AN or compatible labled "TF" */
ROM_END

ROM_START( terrafj )
	ROM_REGION( 0x60000, "maincpu", ROMREGION_ERASEFF ) /* 64K*8 for 68000 code */
	ROM_LOAD16_BYTE( "tfj-8.bin", 0x00000, 0x10000, CRC(b11a6fa7) SHA1(7bb2b98be02d8913796a6d4fa20eed16226ce6b9) )
	ROM_LOAD16_BYTE( "tfj-3.bin", 0x00001, 0x10000, CRC(6c6aa7ed) SHA1(ee5fdeb5411034ce0fd1c883ee25bf1fe9a3ec52) )
	ROM_LOAD16_BYTE( "tfj-7.bin", 0x20000, 0x10000, CRC(fde8de7e) SHA1(6b0d27ec49c8c0609c110ad97938bec8c077ad18) )
	ROM_LOAD16_BYTE( "tfj-2.bin", 0x20001, 0x10000, CRC(db987414) SHA1(0a1734794c626cf9083d7854c9000c5daadfc3fd) )
	ROM_LOAD16_BYTE( "tfj-6.bin", 0x40000, 0x10000, CRC(4911dfbf) SHA1(91d3607e98e2e746cebddd20a10a033a5557c017) )
	ROM_LOAD16_BYTE( "tfj-1.bin", 0x40001, 0x10000, CRC(93063d9a) SHA1(58321e18865a0a72b74beab969b7ea5e010be16a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 code (sound) */
	ROM_LOAD( "11.17k", 0x00000, 0x10000,  CRC(4407d475) SHA1(96e86c7ef4dc997812436f7d0ddea332b4e6cb2b) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "9.11e", 0x00000, 0x08000, CRC(bc6f7cbc) SHA1(20b8a34de4bfa0c2fdcd2f7743a0ab35141f4bf9) ) /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "5.15h", 0x00000, 0x10000, CRC(25d23dfd) SHA1(da32895c1aca403209b7fb181fa4fa23a8e74d32) ) /* foreground tiles */
	ROM_LOAD( "4.13h", 0x10000, 0x10000, CRC(b9b0fe27) SHA1(983c48239ba1524b517f89f281f2b70564bea1e9) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "15.8a", 0x00000, 0x10000, CRC(2144d8e0) SHA1(ed89da11abf3d79753b478603009970c2600ab60) ) /* background tiles */
	ROM_LOAD( "14.6a", 0x10000, 0x10000, CRC(744f5c9e) SHA1(696223a087bb575c7cfaba11e682b221ada461e4) )

	ROM_REGION( 0x20000, "gfx4", 0 )
	ROM_LOAD( "tfj-12.7d", 0x00000, 0x10000, CRC(d74085a1) SHA1(3f6ba85dbd6e48a502c115b2d322a586fc4f56c9) ) /* sprites */
	ROM_LOAD( "tfj-13.9d", 0x10000, 0x10000, CRC(148aa0c5) SHA1(8d8a565540e91b384a9c154522501921b7da4d4e) )

	ROM_REGION( 0x4000, "nb1414m4", 0 )    /* data for mcu/blitter */
	ROM_LOAD( "10.11c", 0x0000, 0x4000, CRC(ac705812) SHA1(65be46ee959d8478cb6dffb25e61f7742276997b) )

	ROM_REGION( 0x0100, "proms", 0 )    /* Unknown use */
	ROM_LOAD( "n82s129an.11j", 0x0000, 0x0100, CRC(81244757) SHA1(6324f63e571f0f7a0bb9eb97f9994809db79493f) ) /* N82S129AN or compatible labled "TF" */
ROM_END


/*

CPU

Main cpu 68000,Z80B*
Sound cpu Z80
Sound ic ym3812
Osc 16 and 24 Mhz

Note:

The custom mcu of original was been replaced by a small daughterboard inversely
connected to the main cpu board.It has a Z80B*,which uses 2kb of ram (6116 SRAM x1),
and some standard TTL ics.

ROMs

tfb-1 to 3 and tfb-6 to 8 main program
tfb-10 Z80B* program (mcu hack)
the rest of roms (sound program and gfx data) are the same of existing set

All roms are 24128,24256,27512
Note

This romset comes from a bootleg/hacked pcb.Game differences from original are:
Company logo and copyright string removed.

*/

ROM_START( terrafjb )
	ROM_REGION( 0x60000, "maincpu", ROMREGION_ERASEFF ) /* 64K*8 for 68000 code */
	ROM_LOAD16_BYTE( "tfj-8.bin", 0x00000, 0x10000, CRC(b11a6fa7) SHA1(7bb2b98be02d8913796a6d4fa20eed16226ce6b9) )
	ROM_LOAD16_BYTE( "tfj-3.bin", 0x00001, 0x10000, CRC(6c6aa7ed) SHA1(ee5fdeb5411034ce0fd1c883ee25bf1fe9a3ec52) )
	ROM_LOAD16_BYTE( "tfj-7.bin", 0x20000, 0x10000, CRC(fde8de7e) SHA1(6b0d27ec49c8c0609c110ad97938bec8c077ad18) )
	ROM_LOAD16_BYTE( "tfj-2.bin", 0x20001, 0x10000, CRC(db987414) SHA1(0a1734794c626cf9083d7854c9000c5daadfc3fd) )
	ROM_LOAD16_BYTE( "tfb-6.bin", 0x40000, 0x08000, CRC(552c3c63) SHA1(82d930b7fcdf4879c4644c47ecce58fd2645f10f) )
	ROM_LOAD16_BYTE( "tfb-1.bin", 0x40001, 0x08000, CRC(6a0b94c7) SHA1(55fb32ab859bf51a1c79bf962bb677fa557216ed) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 code (sound) */
	ROM_LOAD( "tf-001.17k", 0x00000, 0x10000, CRC(eb6b4138) SHA1(04c53bf46d87a156d3fad86f051985d0df79bd20) )

	ROM_REGION( 0x4000, "extra", 0 )    /* z80 program (replacement mcu/blitter) */
	ROM_LOAD( "tfb-10.bin", 0x0000, 0x4000, CRC(3f9aa367) SHA1(8278fb357b2d68869e39efa01ff19005807b41f8) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "9.11e", 0x00000, 0x08000, CRC(bc6f7cbc) SHA1(20b8a34de4bfa0c2fdcd2f7743a0ab35141f4bf9) ) /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "5.15h", 0x00000, 0x10000, CRC(25d23dfd) SHA1(da32895c1aca403209b7fb181fa4fa23a8e74d32) ) /* foreground tiles */
	ROM_LOAD( "4.13h", 0x10000, 0x10000, CRC(b9b0fe27) SHA1(983c48239ba1524b517f89f281f2b70564bea1e9) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "15.8a", 0x00000, 0x10000, CRC(2144d8e0) SHA1(ed89da11abf3d79753b478603009970c2600ab60) ) /* background tiles */
	ROM_LOAD( "14.6a", 0x10000, 0x10000, CRC(744f5c9e) SHA1(696223a087bb575c7cfaba11e682b221ada461e4) )

	ROM_REGION( 0x20000, "gfx4", 0 )
	ROM_LOAD( "tfj-12.7d", 0x00000, 0x10000, CRC(d74085a1) SHA1(3f6ba85dbd6e48a502c115b2d322a586fc4f56c9) ) /* sprites */
	ROM_LOAD( "tfj-13.9d", 0x10000, 0x10000, CRC(148aa0c5) SHA1(8d8a565540e91b384a9c154522501921b7da4d4e) )

	ROM_REGION( 0x0100, "proms", 0 )    /* Unknown use */
	ROM_LOAD( "n82s129an.11j", 0x0000, 0x0100, CRC(81244757) SHA1(6324f63e571f0f7a0bb9eb97f9994809db79493f) ) /* N82S129AN or compatible labled "TF" */
ROM_END


/*

xtals are 16MHz and 24MHz
68000 @ 8.0MHz (16/2)
Z80 @ 4.0MHz (16/4)
YM3812 @ 4.0MHz (16/4)
hsync 15.0540khz
vsync 59.0702Hz

Note the blit data ROM is not present

This seems to be a bootleg of the Japanese version with the region warning screen hacked to just say "WAIT"

*/

ROM_START( terrafb )
	ROM_REGION( 0x60000, "maincpu", ROMREGION_ERASEFF ) /* 64K*8 for 68000 code */
	ROM_LOAD16_BYTE( "f-14.4s", 0x00000, 0x10000, CRC(8e5f557f) SHA1(3462a58146c3f33bf8686adbd2ead25dae3804a8) )
	ROM_LOAD16_BYTE( "f-11.3s", 0x00001, 0x10000, CRC(5320162a) SHA1(eaffafcaf146cdddb03f40f92ce23dfd096eb89e) )
	ROM_LOAD16_BYTE( "f-13.4p", 0x20000, 0x10000, CRC(a86951e0) SHA1(804cc6f143993f5a9d5f3798e971d7abfe94c3a8) )
	ROM_LOAD16_BYTE( "f-9.3p",  0x20001, 0x10000, CRC(58b5f43b) SHA1(9df77235c0b7ac5af4258c04bd90d0a86ccc86b0) )
	ROM_LOAD16_BYTE( "f-12.4m", 0x40000, 0x08000, CRC(4f0e1d76) SHA1(b8636acde7547358663b94bdc8d49b5cc6b596eb) )
	ROM_LOAD16_BYTE( "f-8.3m",  0x40001, 0x08000, CRC(d1014280) SHA1(5ee8d71d77b31b25cce2bf1953c0a5166313a857) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 code (sound) */
	ROM_LOAD( "f-1.1a", 0x00000, 0x10000, CRC(eb6b4138) SHA1(04c53bf46d87a156d3fad86f051985d0df79bd20) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "f-11.4g", 0x00000, 0x08000, CRC(bc6f7cbc) SHA1(20b8a34de4bfa0c2fdcd2f7743a0ab35141f4bf9) ) /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "f-6.3c", 0x00000, 0x10000, CRC(25d23dfd) SHA1(da32895c1aca403209b7fb181fa4fa23a8e74d32) ) /* foreground tiles */
	ROM_LOAD( "f-7.3e", 0x10000, 0x10000, CRC(b9b0fe27) SHA1(983c48239ba1524b517f89f281f2b70564bea1e9) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "f-4.9k", 0x00000, 0x10000, CRC(2144d8e0) SHA1(ed89da11abf3d79753b478603009970c2600ab60) ) /* background tiles */
	ROM_LOAD( "f-5.9m", 0x10000, 0x10000, CRC(744f5c9e) SHA1(696223a087bb575c7cfaba11e682b221ada461e4) )

	ROM_REGION( 0x20000, "gfx4", 0 )
	ROM_LOAD( "f-3.6l", 0x00000, 0x10000, CRC(d74085a1) SHA1(3f6ba85dbd6e48a502c115b2d322a586fc4f56c9) ) /* sprites */
	ROM_LOAD( "f-2.6j", 0x10000, 0x10000, CRC(148aa0c5) SHA1(8d8a565540e91b384a9c154522501921b7da4d4e) )
ROM_END

ROM_START( kozure )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 64K*8 for 68000 code */
	ROM_LOAD16_BYTE( "kozure8.6e", 0x00000, 0x10000, CRC(6bbfb1e6) SHA1(ffc8f835e28ff5f5b11f826b74ac2712c3018178) )
	ROM_LOAD16_BYTE( "kozure3.6h", 0x00001, 0x10000, CRC(f9178ec8) SHA1(68085b6030f4d1f89931263df68635b3b276e9f0) )
	ROM_LOAD16_BYTE( "kozure7.5e", 0x20000, 0x10000, CRC(a7ee09bb) SHA1(70ea2ab09b44e9188eb9500d9663d11e521caa1a) )
	ROM_LOAD16_BYTE( "kozure2.5h", 0x20001, 0x10000, CRC(236d820f) SHA1(e8784c0bbfe22e63a442a8eac18247b740f437a8) )
	ROM_LOAD16_BYTE( "kozure6.3e", 0x40000, 0x10000, CRC(9120e728) SHA1(af2ce368d66d01cbad136ae119b31b1701ad0595) )
	ROM_LOAD16_BYTE( "kozure1.3h", 0x40001, 0x10000, CRC(345fe7a5) SHA1(56ad809cf4a609447cce3e0181ff86e3f0e8966c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 code (sound) */
	ROM_LOAD( "kozure11.17k", 0x00000, 0x10000, CRC(dba51e2d) SHA1(49e799d39d298cd3e01602ae5a2d123dfbfa9134) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "kozure9.11e", 0x00000, 0x08000, CRC(e041356e) SHA1(3e4c8564e7a8c940bbe72db11759903aa295287f) )   /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "kozure5.15h", 0x00000, 0x20000, CRC(0b510258) SHA1(e7ec89faa574b755605ddb67d6c908a9f5f2d6ac) )   /* foreground tiles */
	ROM_LOAD( "kozure4.14h", 0x20000, 0x10000, CRC(fb8e13e6) SHA1(f2eafcf6d7362dc62e808f582a7bd2970e5e1ad1) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "kozure14.8a", 0x00000, 0x10000, CRC(94a9c3d0) SHA1(7a5d810ea370d158b2099c17f4d656fbd3deeac8) )   /* background tiles */

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "kozure12.8d", 0x00000, 0x20000, CRC(15f4021d) SHA1(b2ba6fda1a7bdaae97de4b0157b9b656b4385e08) )   /* sprites */
	ROM_LOAD( "kozure13.9d", 0x20000, 0x20000, CRC(b3b6c753) SHA1(9ad061cac9558320b5cfd1ac1ac8d7f1788270cc) )

	ROM_REGION( 0x4000, "nb1414m4", 0 )    /* data for mcu/blitter */
	ROM_LOAD( "kozure10.11c", 0x0000, 0x4000, CRC(f48be21d) SHA1(5d6db049f30cab98f672814a86a06609c1fa8fb4) )

	ROM_REGION( 0x0100, "proms", 0 )    /* Unknown use */
	ROM_LOAD( "n82s129an.11j", 0x0000, 0x0100, CRC(81244757) SHA1(6324f63e571f0f7a0bb9eb97f9994809db79493f) ) /* N82S129AN or compatible labled "TF" */
ROM_END

ROM_START( cclimbr2 )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 64K*8 for 68000 code */
	ROM_LOAD16_BYTE( "4.bin", 0x00000, 0x10000, CRC(7922ea14) SHA1(4395c1dfdeeba8173cce23b5453185c3ff976980) )
	ROM_LOAD16_BYTE( "1.bin", 0x00001, 0x10000, CRC(2ac7ed67) SHA1(5e9c3ca4f5b259ed7b08db8353be7d36ac947578) )
	ROM_LOAD16_BYTE( "6.bin", 0x20000, 0x10000, CRC(7905c992) SHA1(5e3ddb3b8127476b763578c3717ffe85aa6b342a) )
	ROM_LOAD16_BYTE( "5.bin", 0x20001, 0x10000, CRC(47be6c1e) SHA1(cbd928458087199e63020956c5b61925c3f055f4) )
	ROM_LOAD16_BYTE( "3.bin", 0x40000, 0x10000, CRC(1fb110d6) SHA1(a478096b4b075ff655d079e43151d6b8375f0caa) )
	ROM_LOAD16_BYTE( "2.bin", 0x40001, 0x10000, CRC(0024c15b) SHA1(0cd69a24139e878c09d4de37e4d102851765168f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 code (sound) */
	ROM_LOAD( "11.bin", 0x00000, 0x04000, CRC(fe0175be) SHA1(5c50fa07d8fa61d58a825bbc2cc5a7b85ff3e42e) )
	ROM_LOAD( "12.bin", 0x04000, 0x08000, CRC(5ddf18f2) SHA1(b66da5ad400d00b07160986e4841a309a3572bd1) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "10.bin", 0x00000, 0x08000, CRC(7f475266) SHA1(73d544731fcfd7266bca451880120c555d19ea5d) ) /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x00000, 0x10000, CRC(cbdd3906) SHA1(0525599a5981f1e25ec6faf008e547da7a9ee2cb) ) /* foreground tiles */
	ROM_LOAD( "8.bin",  0x10000, 0x10000, CRC(b2a613c0) SHA1(1d92b85a0dd4b7e533677c454ec23359867defda) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "17.bin", 0x00000, 0x10000, CRC(e24bb2d7) SHA1(8f4b8410b77a50ae735d72f2f37e63784ebc10d9) ) /* background tiles */
	ROM_LOAD( "18.bin", 0x10000, 0x10000, CRC(56834554) SHA1(6d579c32fb57eb4eddc062cb2cc78b546f6607b2) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "15.bin", 0x00000, 0x10000, CRC(4bf838be) SHA1(6b1d7448caf406e47268a1276225bb0619b80cc9) ) /* sprites */
	ROM_LOAD( "16.bin", 0x10000, 0x10000, CRC(21a265c5) SHA1(a2b3a1e5aa545030d933c0f058f4f9a18e1af1c9) )
	ROM_LOAD( "13.bin", 0x20000, 0x10000, CRC(6b6ec999) SHA1(7749ce435f497732bd1b6958974cd95e960fc9fe) )
	ROM_LOAD( "14.bin", 0x30000, 0x10000, CRC(f426a4ad) SHA1(facccb21ca73c560d3a38e05e677782516d5b0c0) )

	ROM_REGION( 0x4000, "nb1414m4", 0 )    /* data for mcu/blitter */
	ROM_LOAD( "9.bin",  0x0000, 0x4000, CRC(740d260f) SHA1(5b4487930c7a1fb0a796aec2243bec631b1b5104) )
ROM_END

ROM_START( cclimbr2a )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 64K*8 for 68000 code */
	ROM_LOAD16_BYTE( "4a.bin", 0x00000, 0x10000, CRC(e1d3192c) SHA1(6e0e6d731af401147f3c2566ddef3e169223eccd) )
	ROM_LOAD16_BYTE( "1a.bin", 0x00001, 0x10000, CRC(3ef84974) SHA1(04a6a2ac5b95d0a329ad7afc68b0ef480608c2d4) )
	ROM_LOAD16_BYTE( "6.bin",  0x20000, 0x10000, CRC(7905c992) SHA1(5e3ddb3b8127476b763578c3717ffe85aa6b342a) )
	ROM_LOAD16_BYTE( "5.bin",  0x20001, 0x10000, CRC(47be6c1e) SHA1(cbd928458087199e63020956c5b61925c3f055f4) )
	ROM_LOAD16_BYTE( "3.bin",  0x40000, 0x10000, CRC(1fb110d6) SHA1(a478096b4b075ff655d079e43151d6b8375f0caa) )
	ROM_LOAD16_BYTE( "2.bin",  0x40001, 0x10000, CRC(0024c15b) SHA1(0cd69a24139e878c09d4de37e4d102851765168f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 code (sound) */
	ROM_LOAD( "11.bin", 0x00000, 0x04000, CRC(fe0175be) SHA1(5c50fa07d8fa61d58a825bbc2cc5a7b85ff3e42e) )
	ROM_LOAD( "12.bin", 0x04000, 0x08000, CRC(5ddf18f2) SHA1(b66da5ad400d00b07160986e4841a309a3572bd1) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "10.bin", 0x00000, 0x08000, CRC(7f475266) SHA1(73d544731fcfd7266bca451880120c555d19ea5d) ) /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x00000, 0x10000, CRC(cbdd3906) SHA1(0525599a5981f1e25ec6faf008e547da7a9ee2cb) ) /* foreground tiles */
	ROM_LOAD( "8.bin",  0x10000, 0x10000, CRC(b2a613c0) SHA1(1d92b85a0dd4b7e533677c454ec23359867defda) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "17.bin", 0x00000, 0x10000, CRC(e24bb2d7) SHA1(8f4b8410b77a50ae735d72f2f37e63784ebc10d9) ) /* background tiles */
	ROM_LOAD( "18.bin", 0x10000, 0x10000, CRC(56834554) SHA1(6d579c32fb57eb4eddc062cb2cc78b546f6607b2) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "15.bin", 0x00000, 0x10000, CRC(4bf838be) SHA1(6b1d7448caf406e47268a1276225bb0619b80cc9) ) /* sprites */
	ROM_LOAD( "16.bin", 0x10000, 0x10000, CRC(21a265c5) SHA1(a2b3a1e5aa545030d933c0f058f4f9a18e1af1c9) )
	ROM_LOAD( "13.bin", 0x20000, 0x10000, CRC(6b6ec999) SHA1(7749ce435f497732bd1b6958974cd95e960fc9fe) )
	ROM_LOAD( "14.bin", 0x30000, 0x10000, CRC(f426a4ad) SHA1(facccb21ca73c560d3a38e05e677782516d5b0c0) )

	ROM_REGION( 0x4000, "nb1414m4", 0 )    /* data for mcu/blitter */
	ROM_LOAD( "9.bin",  0x0000, 0x4000, CRC(740d260f) SHA1(5b4487930c7a1fb0a796aec2243bec631b1b5104) )
ROM_END

ROM_START( armedf )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "06.3d", 0x00000, 0x10000, CRC(0f9015e2) SHA1(0dc9c2cda0220bb9d03048c0eac62bc512d30622) )
	ROM_LOAD16_BYTE( "01.3f", 0x00001, 0x10000, CRC(816ff7c5) SHA1(1013883f2389aba4b8f397aacc0d681807a31cd5) )
	ROM_LOAD16_BYTE( "07.5d", 0x20000, 0x10000, CRC(5b3144a5) SHA1(98dd45049f242480c9ac0903fb38dfb2b8906be7) )
	ROM_LOAD16_BYTE( "02.4f", 0x20001, 0x10000, CRC(fa10c29d) SHA1(9c772ff6f749a542248c2ca260ac3460ae4f0cc6) )
	ROM_LOAD16_BYTE( "af_08.rom", 0x40000, 0x10000, CRC(d1d43600) SHA1(1a473b4958a02a33c0a02e7e72a70a9ee0c68c50) )
	ROM_LOAD16_BYTE( "af_03.rom", 0x40001, 0x10000, CRC(bbe1fe2d) SHA1(f47be23c7564b106d636d49d5f1da47daecd31df) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 code (sound) */
	ROM_LOAD( "af_10.rom", 0x00000, 0x10000, CRC(c5eacb87) SHA1(33af84b48fbda26729975b02cfb70f23c0bce6a2) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "09.11c", 0x00000, 0x08000, CRC(5c6993d5) SHA1(3991851e5c4d2b3d72c372afdfbb710796874c15) ) /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "af_04.rom", 0x00000, 0x10000, CRC(44d3af4f) SHA1(0c2cb54357c314e43cec6f959fe9d4a2c8bc8834) ) /* foreground tiles */
	ROM_LOAD( "af_05.rom", 0x10000, 0x10000, CRC(92076cab) SHA1(f47424817373a6735da2b2049b53da5b38178cec) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "af_14.rom", 0x00000, 0x10000, CRC(8c5dc5a7) SHA1(758140ddb9e60fb3950fe58bf53c7aea769a1a94) ) /* background tiles */
	ROM_LOAD( "af_13.rom", 0x10000, 0x10000, CRC(136a58a3) SHA1(5481e3ce404881a0470f8740f0de6e42283bedf2) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "af_11.rom", 0x00000, 0x20000, CRC(b46c473c) SHA1(d8573225e2d8b133b3bdd4fa5a12d445c71d5e0e) ) /* sprites */
	ROM_LOAD( "af_12.rom", 0x20000, 0x20000, CRC(23cb6bfe) SHA1(34cb013827206bea71f5336b308ba92bee688506) )
ROM_END

ROM_START( armedff )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "af_06.rom", 0x00000, 0x10000, CRC(c5326603) SHA1(135a8d96d792cf9b55e39e21160ee419be76d28a) )
	ROM_LOAD16_BYTE( "af_01.rom", 0x00001, 0x10000, CRC(458e9542) SHA1(22e4281eaac6b9b04af324cc96b5f3e4d1cefe43) )
	ROM_LOAD16_BYTE( "af_07.rom", 0x20000, 0x10000, CRC(cc8517f5) SHA1(93e4d3707a48551af89cadd0e016ddb65285a005) )
	ROM_LOAD16_BYTE( "af_02.rom", 0x20001, 0x10000, CRC(214ef220) SHA1(0c32349afc31fbcd825695679540a024f1e1acb2) )
	ROM_LOAD16_BYTE( "af_08.rom", 0x40000, 0x10000, CRC(d1d43600) SHA1(1a473b4958a02a33c0a02e7e72a70a9ee0c68c50) )
	ROM_LOAD16_BYTE( "af_03.rom", 0x40001, 0x10000, CRC(bbe1fe2d) SHA1(f47be23c7564b106d636d49d5f1da47daecd31df) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 code (sound) */
	ROM_LOAD( "af_10.rom", 0x00000, 0x10000, CRC(c5eacb87) SHA1(33af84b48fbda26729975b02cfb70f23c0bce6a2) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "af_09.rom", 0x00000, 0x08000, CRC(7025e92d) SHA1(e590682092c25bbfb674afeccbfc0e613c51d188) ) /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "af_04.rom", 0x00000, 0x10000, CRC(44d3af4f) SHA1(0c2cb54357c314e43cec6f959fe9d4a2c8bc8834) ) /* foreground tiles */
	ROM_LOAD( "af_05.rom", 0x10000, 0x10000, CRC(92076cab) SHA1(f47424817373a6735da2b2049b53da5b38178cec) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "af_14.rom", 0x00000, 0x10000, CRC(8c5dc5a7) SHA1(758140ddb9e60fb3950fe58bf53c7aea769a1a94) ) /* background tiles */
	ROM_LOAD( "af_13.rom", 0x10000, 0x10000, CRC(136a58a3) SHA1(5481e3ce404881a0470f8740f0de6e42283bedf2) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "af_11.rom", 0x00000, 0x20000, CRC(b46c473c) SHA1(d8573225e2d8b133b3bdd4fa5a12d445c71d5e0e) ) /* sprites */
	ROM_LOAD( "af_12.rom", 0x20000, 0x20000, CRC(23cb6bfe) SHA1(34cb013827206bea71f5336b308ba92bee688506) )
ROM_END

ROM_START( skyrobo )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "3", 0x00000, 0x20000, CRC(02d8ba9f) SHA1(7622cc17561e5d1c069341b5f412f732f901d4a8) ) /* Rom location IC3 */
	ROM_LOAD16_BYTE( "1", 0x00001, 0x20000, CRC(fcfd9e2e) SHA1(c69b34653f04af8d488e323bc2db89656f76c332) ) /* Rom location IC2 */
	ROM_LOAD16_BYTE( "4", 0x40000, 0x20000, CRC(37ced4b7) SHA1(9ded66f795d3c0886f48e52de632e6edb8c57e84) ) /* Rom location IC5 */
	ROM_LOAD16_BYTE( "2", 0x40001, 0x20000, CRC(88d52f8e) SHA1(33b0d2b3cd38a13d8580694e7c50c059914eebe2) ) /* Rom location IC4 */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 code (sound) */
	ROM_LOAD( "8.17k", 0x00000, 0x10000, CRC(0aeab61e) SHA1(165e0ad58542b65383fef714578da21f62df7b74) )

	ROM_REGION( 0x10000, "mcu", 0 ) /* Intel C8751 read protected MCU */
	ROM_LOAD( "i8751.mcu", 0x00000, 0x1000, NO_DUMP )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "7", 0x00000, 0x08000, CRC(f556ef28) SHA1(2acb83cdf23356091056f2cfbbc2b9828ee25b6f) ) /* Rom location 11C */

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "5.13f", 0x00000, 0x20000, CRC(d440a29f) SHA1(9e6ea7c9903e5e3e8e10ac7680c6120e1aa27250) )
	ROM_LOAD( "6.15f", 0x20000, 0x10000, CRC(27469a76) SHA1(ebf2c60e1f70a589680c05adf10771ac2097b9d0) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "12.8a", 0x00000, 0x10000, CRC(a5694ea9) SHA1(ea94174495b3a65b3797932074a94df3b55fa0a2) )
	ROM_LOAD( "11.6a", 0x10000, 0x10000, CRC(10b74e2c) SHA1(e3ec68726e7f277dc2043424f2e4d863eb01b3dc) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "9.8d",  0x00000, 0x20000, CRC(fe67800e) SHA1(0d3c4c3cb185270260fa691a97cddf082d6a056e) )
	ROM_LOAD( "10.9d", 0x20000, 0x20000, CRC(dcb828c4) SHA1(607bc86580a6fe6e15e91131532b0eecd8b7a0cb) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tf.13h", 0x0000, 0x0100, CRC(81244757) SHA1(6324f63e571f0f7a0bb9eb97f9994809db79493f) ) /* Prom is a N82S129AN type */
ROM_END

ROM_START( bigfghtr )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "3.ic3", 0x00000, 0x20000, CRC(e1e1f291) SHA1(dbbd707be6250d9ffcba3fee265869b72f790e26) )
	ROM_LOAD16_BYTE( "1.ic2", 0x00001, 0x20000, CRC(1100d991) SHA1(3c79398804b3a26b3df0c5734b270c37e1ba6a60) )
	ROM_LOAD16_BYTE( "4.ic5", 0x40000, 0x20000, CRC(2464a83b) SHA1(00f5ac81bc33148daafeab757647b63894e0e0ca) )
	ROM_LOAD16_BYTE( "2.ic4", 0x40001, 0x20000, CRC(b47bbcd5) SHA1(811bd4bc8fb662abf4734ab51e24c863d5cc3df3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 code (sound) */
	ROM_LOAD( "8.17k", 0x00000, 0x10000, CRC(0aeab61e) SHA1(165e0ad58542b65383fef714578da21f62df7b74) )

	ROM_REGION( 0x10000, "mcu", 0 ) /* Intel C8751 read protected MCU */
	ROM_LOAD( "i8751.mcu", 0x00000, 0x1000, NO_DUMP )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "7.11c", 0x00000, 0x08000, CRC(1809e79f) SHA1(730547771f803857acb552a84a8bc21bd3bda33f) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "5.13f", 0x00000, 0x20000, CRC(d440a29f) SHA1(9e6ea7c9903e5e3e8e10ac7680c6120e1aa27250) )
	ROM_LOAD( "6.15f", 0x20000, 0x10000, CRC(27469a76) SHA1(ebf2c60e1f70a589680c05adf10771ac2097b9d0) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "12.8a", 0x00000, 0x10000, CRC(a5694ea9) SHA1(ea94174495b3a65b3797932074a94df3b55fa0a2) )
	ROM_LOAD( "11.6a", 0x10000, 0x10000, CRC(10b74e2c) SHA1(e3ec68726e7f277dc2043424f2e4d863eb01b3dc) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "9.8d",  0x00000, 0x20000, CRC(fe67800e) SHA1(0d3c4c3cb185270260fa691a97cddf082d6a056e) )
	ROM_LOAD( "10.9d", 0x20000, 0x20000, CRC(dcb828c4) SHA1(607bc86580a6fe6e15e91131532b0eecd8b7a0cb) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tf.13h", 0x0000, 0x0100, CRC(81244757) SHA1(6324f63e571f0f7a0bb9eb97f9994809db79493f) ) /* Prom is a N82S129AN type */
ROM_END

/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(armedf_state,terraf)
{
	m_scroll_type = 0;

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x07c000, 0x07c001, write16_delegate(FUNC(armedf_state::bootleg_io_w),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x07c006, 0x07c007, write16_delegate(FUNC(armedf_state::terraf_fg_scrolly_w),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x07c008, 0x07c009, write16_delegate(FUNC(armedf_state::terraf_fg_scrollx_w),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x0c0000, 0x0c0001, write16_delegate(FUNC(armedf_state::terraf_fg_scroll_msb_arm_w),this));
}

DRIVER_INIT_MEMBER(armedf_state,terrafu)
{
	m_scroll_type = 0;

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x07c000, 0x07c001, write16_delegate(FUNC(armedf_state::terraf_io_w),this));
}

DRIVER_INIT_MEMBER(armedf_state,terrafjb)
{
	m_scroll_type = 0;

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x07c000, 0x07c001, write16_delegate(FUNC(armedf_state::terrafjb_io_w),this));
}

DRIVER_INIT_MEMBER(armedf_state,armedf)
{
	m_scroll_type = 1;
}


DRIVER_INIT_MEMBER(armedf_state,kozure)
{
	UINT16 *ROM = (UINT16 *)memregion("maincpu")->base();

	/* patch "time over" bug, see notes on top. */
	ROM[0x1016c/2] = 0x4e71;
	/* ROM check at POST. */
	ROM[0x04fc6/2] = 0x4e71;
	m_scroll_type = 0;

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x07c000, 0x07c001, write16_delegate(FUNC(armedf_state::terraf_io_w),this));

}

DRIVER_INIT_MEMBER(armedf_state,legion)
{
#if LEGION_HACK
	/* This is a hack to allow you to use the extra features
	     of 3 of the "Unused" Dip Switches (see notes above). */
	UINT16 *ROM = (UINT16 *)memregion("maincpu")->base();
	RAM[0x0001d6 / 2] = 0x0001;
	/* To avoid checksum error */
	RAM[0x000488 / 2] = 0x4e71;
#endif

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x07c000, 0x07c001, write16_delegate(FUNC(armedf_state::terraf_io_w),this));

	m_scroll_type = 2;
}

DRIVER_INIT_MEMBER(armedf_state,legionjb)
{
#if LEGION_HACK
	/* This is a hack to allow you to use the extra features
	     of 3 of the "Unused" Dip Switches (see notes above). */
	UINT16 *RAM = (UINT16 *)memregion("maincpu")->base();
	RAM[0x0001d6/2] = 0x0001;
	/* No need to patch the checksum routine (see notes) ! */
#endif

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x07c000, 0x07c001, write16_delegate(FUNC(armedf_state::bootleg_io_w),this));

	m_scroll_type = 2;

	save_item(NAME(m_legion_cmd));
}

DRIVER_INIT_MEMBER(armedf_state,cclimbr2)
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x07c000, 0x07c001, write16_delegate(FUNC(armedf_state::terraf_io_w),this));

	m_scroll_type = 3;
}

DRIVER_INIT_MEMBER(bigfghtr_state,bigfghtr)
{
	m_scroll_type = 1;
}

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

/*     YEAR, NAME,    PARENT,   MACHINE,  INPUT,    INIT,                     MONITOR, COMPANY,                        FULLNAME, FLAGS */
GAME( 1987, legion,   0,        legion,   legion,   armedf_state,   legion,   ROT270, "Nichibutsu",                    "Legion - Spinner-87 (World ver 2.03)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, legionj,  legion,   legion,   legion,   armedf_state,   legion,   ROT270, "Nichibutsu",                    "Chouji Meikyuu Legion (Japan ver 1.05)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, legionjb, legion,   legionjb, legion,   armedf_state,   legionjb, ROT270, "bootleg",                       "Chouji Meikyuu Legion (Japan ver 1.05, bootleg)", MACHINE_SUPPORTS_SAVE) /* blitter protection removed */

GAME( 1987, terraf,   0,        terraf,   terraf,   armedf_state,   terrafu,  ROT0,   "Nichibutsu",                    "Terra Force", MACHINE_SUPPORTS_SAVE )
GAME( 1987, terrafu,  terraf,   terraf,   terraf,   armedf_state,   terrafu,  ROT0,   "Nichibutsu USA",                "Terra Force (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, terrafj,  terraf,   terraf,   terraf,   armedf_state,   terrafu,  ROT0,   "Nichibutsu Japan",              "Terra Force (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, terrafjb, terraf,   terrafjb, terraf,   armedf_state,   terrafjb, ROT0,   "bootleg",                       "Terra Force (Japan, bootleg with additional Z80)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, terrafb,  terraf,   terraf,   terraf,   armedf_state,   terraf,   ROT0,   "bootleg",                       "Terra Force (Japan, bootleg set 2)", MACHINE_SUPPORTS_SAVE )

GAME( 1987, kozure,   0,        kozure,   kozure,   armedf_state,   kozure,   ROT0,   "Nichibutsu",                    "Kozure Ookami (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1988, cclimbr2, 0,        cclimbr2, cclimbr2, armedf_state,   cclimbr2, ROT0,   "Nichibutsu",                    "Crazy Climber 2 (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, cclimbr2a,cclimbr2, cclimbr2, cclimbr2, armedf_state,   cclimbr2, ROT0,   "Nichibutsu",                    "Crazy Climber 2 (Japan, Harder)", MACHINE_SUPPORTS_SAVE  )

GAME( 1988, armedf,   0,        armedf,   armedf,   armedf_state,   armedf,   ROT270, "Nichibutsu",                    "Armed Formation", MACHINE_SUPPORTS_SAVE )
GAME( 1988, armedff,  armedf,   armedf,   armedf,   armedf_state,   armedf,   ROT270, "Nichibutsu (Fillmore license)", "Armed Formation (Fillmore license)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, skyrobo,  0,        bigfghtr, bigfghtr, bigfghtr_state, bigfghtr, ROT0,   "Nichibutsu",                    "Sky Robo", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1989, bigfghtr, skyrobo,  bigfghtr, bigfghtr, bigfghtr_state, bigfghtr, ROT0,   "Nichibutsu",                    "Tatakae! Big Fighter (Japan)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
