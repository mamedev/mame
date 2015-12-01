// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************
                        WEC Le Mans 24  &   Hot Chase

                          (C)   1986 & 1988 Konami

                    driver by       Luca Elia (l.elia@tin.it)

- Note: press F2 to enter service mode -

---------------------------------------------------------------------------
                                TODO list
---------------------------------------------------------------------------
WEC Le Mans 24:
- The parallactic scrolling is sometimes wrong (related to v-cnt bit enabled?)
Hot Chase:
- Sound BGMs are regressed (hiccups badly);
- Samples pitch is too low, for instance game over speech;
Common Issues:
- Too many hacks with protection/blitter/colors. 
  Additionally, there's a bug report that claims that current arrangement is broken for later levels in WEC Le Mans.
  007643 / 007645 could do with a rewrite, in short.
- One ROM unused (32K in hotchase, 16K in wecleman)
- Incomplete DSWs
- Sprite ram is not cleared by the game and no sprite list end-marker
  is written. We cope with that with an hack in the Blitter but there
  must be a register to do the trick



----------------------------------------------------------------------
Hardware                Main    Sub             Sound   Sound Chips
----------------------------------------------------------------------
[WEC Le Mans 24]        68000   68000   Z-80    YM2151 YM3012 1x007232

[Hot Chase]             68000   68000   68B09E                3x007232

[CPU PCB GX763 350861B]
    007641  007770  3x007232  051550

[VID PCB GX763 350860A AI AM-1]
    007634  007635  3x051316  007558  007557
----------------------------------------------------------------------


----------------------------------------------------------------
Main CPU                     [WEC Le Mans 24]     [Hot Chase]
----------------------------------------------------------------
ROM                R         000000-03ffff        <
Work RAM           RW        040000-043fff        040000-063fff*
?                  RW        060000-060007        -
Blitter             W        080000-080011        <
Page RAM           RW        100000-103fff        -
Text RAM           RW        108000-108fff        -
Palette RAM        RW        110000-110fff        110000-111fff**
Shared RAM         RW        124000-127fff        120000-123fff
Sprites RAM        RW        130000-130fff        <
Input Ports        RW        1400xx-1400xx        <
Background         RW                             100000-100fff
Background Ctrl     W        -                    101000-10101f
Foreground         RW        -                    102000-102fff
Foreground Ctrl     W        -                    103000-10301f

* weird    ** only half used


----------------------------------------------------------------
Sub CPU                      [WEC Le Mans 24]     [Hot Chase]
----------------------------------------------------------------

ROM                R         000000-00ffff        000000-01ffff
Work RAM           RW        -                    060000-060fff
Road RAM           RW        060000-060fff        020000-020fff
Shared RAM         RW        070000-073fff        040000-043fff


---------------------------------------------------------------------------
                                Game code
                            [WEC Le Mans 24]
---------------------------------------------------------------------------

                    Interesting locations (main cpu)
                    --------------------------------

There's some 68000 assembly code in ASCII around d88 :-)

040000+
7-9                             *** hi score/10 (BCD 3 bytes) ***
b-d                             *** score/10 (BCD 3 bytes) ***
1a,127806               <- 140011.b
1b,127807               <- 140013.b
1c,127808               <- 140013.b
1d,127809               <- 140015.b
1e,12780a               <- 140017.b
1f                      <- 140013.b
30                              *** credits ***
3a,3b,3c,3d             <-140021.b
3a = accelerator   3b = ??   3c = steering   3d = table

d2.w                    -> 108f24 fg y scroll
112.w                   -> 108f26 bg y scroll

16c                             influences 140031.b
174                             screen address
180                             input port selection (->140003.b ->140021.b)
181                     ->140005.b
185                             bit 7 high -> must copy sprite data to 130000
1dc+(1da).w             ->140001.b

40a.w,c.w               *** time (BCD) ***
411                             EF if brake, 0 otherwise
416                             ?
419                             gear: 0=lo,1=hi
41e.w                   speed related ->127880
424.w                   speed BCD
43c.w                   accel?
440.w                   level?

806.w                   scrollx related
80e.w                   scrolly related

c08.b                   routine select: 1>1e1a4 2>1e1ec 3>1e19e other>1e288 (map screen)

117a.b                  selected letter when entering name in hi-scores
117e.w                  cycling color in hi-scores

12c0.w                  ?time,pos,len related?
12c2.w
12c4.w
12c6.w

1400-1bff               color data (0000-1023 chars)
1c00-23ff               color data (1024-2047 sprites?)

2400                    Sprite data: 40 entries x  4 bytes =  100
2800                    Sprite data: 40 entries x 40 bytes = 1000
3800                    Sprite data: 40 entries x 10 bytes =  400

                    Interesting routines (main cpu)
                    -------------------------------

804                     mem test
818                     end mem test (cksums at 100, addresses at A90)
82c                     other cpu test
a0c                     rom test
c1a                     prints string (a1)
1028                    end test
204c                    print 4*3 box of chars to a1, made up from 2 2*6 (a0)=0xLR (Left,Righ index)
4e62                    raws in the fourth page of chars
6020                    test screen (print)
60d6                    test screen
62c4                    motor test?
6640                    print input port values ( 6698 = scr_disp.w,ip.b,bit.b[+/-] )

819c                    prepares sprite data
8526                    blitter: 42400->130000
800c                    8580    sprites setup on map screen

1833a                   cycle cols on hi-scores
18514                   hiscores: main loop
185e8                   hiscores: wheel selects letter

TRAP#0                  prints string: A0-> addr.l, attr.w, (char.b)*, 0

IRQs                    [1,3,6]  602
IRQs                    [2,7]    1008->12dc      ORI.W    #$2700,(A7) RTE
IRQs                    [4]      1004->124c
IRQs                    [5]      106c->1222      calls sequence: $3d24 $1984 $28ca $36d2 $3e78


                    Interesting locations (sub cpu)
                    -------------------------------

                    Interesting routines (sub cpu)
                    ------------------------------

1028    'wait for command' loop.
1138    lev4 irq
1192    copies E0*4 bytes: (a1)+ -> (a0)+


---------------------------------------------------------------------------
                                 Game code
                                [Hot Chase]
---------------------------------------------------------------------------

This game has been probably coded by the same programmers of WEC Le Mans 24
It shares some routines and there is the (hidden?) string "WEC 2" somewhere

                            Main CPU                Sub CPU

Interrupts: [1, 7]          FFFFFFFF                FFFFFFFF
Interrupts: [2,3,4,5,6]     221c                    1288

Self Test:
 0] pause,120002==55,pause,120002==AA,pause,120002==CC, (on error set bit d7.0)
 6] 60000-63fff(d7.1),40000-41fff(d7.2)
 8] 40000/2<-chksum 0-20000(e/o);40004/6<-chksum 20000-2ffff(e/o) (d7.3456)
 9] chksums from sub cpu: even->40004   odd->(40006)    (d7.78)
 A] 110000-111fff(even)(d7.9),102000-102fff(odd)(d7.a)
 C] 100000-100fff(odd)(d7.b),pause,pause,pause
10] 120004==0(d7.c),120006==0(d7.d),130000-1307ff(first $A of every $10 bytes only)(d7.e),pause
14] 102000<-hw screen+(d7==0)? jmp 1934/1000
15] 195c start of game


                    Interesting locations (main cpu)
                    --------------------------------

60024.b                 <- !140017.b (DSW 1 - coinage)
60025.b                 <- !140015.b (DSW 2 - options)
6102c.w                 *** time ***

                    Interesting routines (main cpu)
                    -------------------------------

18d2                    (d7.d6)?print BAD/OK to (a5)+, jmp(D0)
1d58                    print d2.w to (a4)+, jmp(a6)
580c                    writes at 60000
61fc                    print test strings
18cbe                   print "game over"

Revisions:

05-05-2002 David Haywood(Haze)
- improved Wec Le Mans steering

05-01-2002 Hiromitsu Shioya(Shica)
- fixed Hot Chase volume and sound interrupt

xx-xx-2003 Acho A. Tang
[Wec Le Mans 24]
- generalized blitter to support Wec Le Mans
- emulated custom alpha blending chip used for protection
- fixed game color and sound volume
- added shadows and sprite-to-sprite priority
- added curbs effect
- modified zoom equation to close tile gaps
- fixed a few tile glitches
- converted driver to use RGB direct
- cloud transition(needs verification from board owners)
- fixed sound banking
- source clean-up

TODO:
- check dust color on title screen(I don't think it should be black)
- check brake light(LED) support
- check occational off-pitch music and samples(sound interrupt related?)

* Sprite, road and sky drawings do not support 32-bit color depth.
  Certain sprites with incorrect z-value still pop in front of closer
  billboards and some appear a few pixels off the ground. They could be
  the game's intrinsic flaws. (reference: www.system16.com)

[Hot Chase]
- shared changes with Wec Le Mans
- removed junk tiles during introduction(needs verification)

* Special thanks to Luca Elia for bringing us so many enjoyable games.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6809/m6809.h"
#include "sound/2151intf.h"
#include "wecleman.lh"
#include "includes/wecleman.h"


/***************************************************************************
                            Common Routines
***************************************************************************/

READ16_MEMBER(wecleman_state::wecleman_protection_r)
{
	int blend, data0, data1, r0, g0, b0, r1, g1, b1;

	data0 = m_protection_ram[0];
	blend = m_protection_ram[2];
	data1 = m_protection_ram[1];
	blend &= 0x3ff;

	// a precalculated table will take an astronomical 4096^2(colors) x 1024(steps) x 2(word) bytes
	r0 = data0;  g0 = data0;  b0 = data0;
	r0 &= 0xf;   g0 &= 0xf0;  b0 &= 0xf00;
	r1 = data1;  g1 = data1;  b1 = data1;
	r1 &= 0xf;   g1 &= 0xf0;  b1 &= 0xf00;
	r1 -= r0;    g1 -= g0;    b1 -= b0;
	r1 *= blend; g1 *= blend; b1 *= blend;
	r1 >>= 10;   g1 >>= 10;   b1 >>= 10;
	r0 += r1;    g0 += g1;    b0 += b1;
	g0 &= 0xf0;  b0 &= 0xf00;

	r0 |= g0;
	r0 |= b0;

	return(r0);
}

WRITE16_MEMBER(wecleman_state::wecleman_protection_w)
{
	if (offset == 2) m_prot_state = data & 0x2000;
	if (!m_prot_state) COMBINE_DATA(m_protection_ram + offset);
}



/* 140005.b (WEC Le Mans 24 Schematics)

 COMMAND
 ___|____
|   CK  8|--/        7
| LS273 7| TV-KILL   6
|       6| SCR-VCNT  5
|       5| SCR-HCNT  4
|   5H  4| SOUND-RST 3
|       3| SOUND-ON  2
|       2| NSUBRST   1
|       1| SUBINT    0
|__CLR___|
    |
  NEXRES

 Schems: SUBRESET does a RST+HALT
         Sub CPU IRQ 4 generated by SUBINT, no other IRQs
*/
WRITE16_MEMBER(wecleman_state::irqctrl_w)
{
	if (ACCESSING_BITS_0_7)
	{
		// logerror("CPU #0 - PC = %06X - $140005 <- %02X (old value: %02X)\n",space.device().safe_pc(), data&0xFF, old_data&0xFF);

		// Bit 0 : SUBINT
		if ( (m_irqctrl & 1) && (!(data & 1)) ) // 1->0 transition
			m_subcpu->set_input_line(4, HOLD_LINE);

		// Bit 1 : NSUBRST
		m_subcpu->set_input_line(INPUT_LINE_RESET,  (data & 2) ? CLEAR_LINE : ASSERT_LINE);

		// Bit 2 : SOUND-ON: send a interrupt to sound CPU, 0 -> 1 transition
		if ( (m_irqctrl & 4) && (!(data & 4)) )
		{
			if(m_sound_hw_type == 0) // wec le mans
				m_audiocpu->set_input_line(0, HOLD_LINE);
			else // hot chase
			{
				m_hotchase_sound_hs = false;
				// TODO: ASSERT_LINE here?
				m_audiocpu->set_input_line(M6809_IRQ_LINE, HOLD_LINE);
			}
		}
		// Bit 3 : SOUNDRST, pc=0x18ea in Hot Chase POST, 1 -> 0 -> 1
		m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 8) ? CLEAR_LINE : ASSERT_LINE);
		// Bit 4 : SCR-HCNT
		// Bit 5 : SCR-VCNT: active in WEC Le Mans, disabled in Hot Chase (where's the latch anyway?)
		// Bit 6 : TV-KILL: active low, disables screen.
		m_irqctrl = data;   // latch the value
	}
}

/* 140003.b (usually paired with a write to 140021.b)

    Bit:

    7-------        ?
    -65-----        input selection (0-3)
    ---43---        ?
    -----2--        start light
    ------10        ? out 1/2

*/
WRITE16_MEMBER(wecleman_state::selected_ip_w)
{
	if (ACCESSING_BITS_0_7) m_selected_ip = data & 0xff;    // latch the value
}

/* $140021.b - Return the previously selected input port's value */
READ16_MEMBER(wecleman_state::selected_ip_r)
{
	switch ( (m_selected_ip >> 5) & 3 )
	{                                                                   // From WEC Le Mans Schems:
		case 0:  return ioport("ACCEL")->read();        // Accel - Schems: Accelevr
		case 1:  return ~0;                                             // ????? - Schems: Not Used
		case 2:  return ioport("STEER")->read();        // Wheel - Schems: Handlevr
		case 3:  return ~0;                                             // Table - Schems: Turnvr

		default: return ~0;
	}
}

/* Word Blitter - Copies data around (Work RAM, Sprite RAM etc.)
                  It's fed with a list of blits to do

    Offset:

    00.b            ? Number of words - 1 to add to address per transfer
    01.b            ? logic function / blit mode
    02.w            ? (always 0)
    04.l            Source address (Base address of source data)
    08.l            List of blits address
    0c.l            Destination address
    01.b            ? Number of transfers
    10.b            Triggers the blit
    11.b            Number of words per transfer

    The list contains 4 bytes per blit:


    Offset:

    00.w            ?
    02.w            offset from Base address


    Note:

    Hot Chase explicitly copies color information from sprite parameters back to list[4n+1](byte ptr)
    and that tips me off where the colors are actually encoded. List[4n+0] is believed to hold the
    sprites' depth value. Wec Le Mans will z-sort the sprites before writing them to video RAM but
    the order is not always right. It is possible the video hardware performs additional sorting.

    The color code in the original sprite encoding has special meanings on the other hand. I'll take
    a shortcut by manually copying list[0] and list[1] to sprite RAM for further process.
*/
WRITE16_MEMBER(wecleman_state::blitter_w)
{
	COMBINE_DATA(&m_blitter_regs[offset]);

	/* do a blit if $80010.b has been written */
	if ( (offset == 0x10/2) && (ACCESSING_BITS_8_15) )
	{
		/* 80000.b = ?? usually 0 - other values: 02 ; 00 - ? logic function ? */
		/* 80001.b = ?? usually 0 - other values: 3f ; 01 - ? height ? */
		int minterm  = ( m_blitter_regs[0x0/2] & 0xFF00 ) >> 8;
		int list_len = ( m_blitter_regs[0x0/2] & 0x00FF ) >> 0;

		/* 80002.w = ?? always 0 - ? increment per horizontal line ? */
		/* no proof at all, it's always 0 */
		//int srcdisp = m_blitter_regs[0x2/2] & 0xFF00;
		//int destdisp = m_blitter_regs[0x2/2] & 0x00FF;

		/* 80004.l = source data address */
		int src  = ( m_blitter_regs[0x4/2] << 16 ) + m_blitter_regs[0x6/2];

		/* 80008.l = list of blits address */
		int list = ( m_blitter_regs[0x8/2] << 16 ) + m_blitter_regs[0xA/2];

		/* 8000C.l = destination address */
		int dest = ( m_blitter_regs[0xC/2] << 16 ) + m_blitter_regs[0xE/2];

		/* 80010.b = number of words to move */
		int size = ( m_blitter_regs[0x10/2] ) & 0x00FF;

		/* Word aligned transfers only ?? */
		src  &= (~1);   list &= (~1);    dest &= (~1);

		/* Two minterms / blit modes are used */
		if (minterm != 2)
		{
			/* One single blit */
			for ( ; size > 0 ; size--)
			{
				/* maybe slower than a memcpy but safer (and errors are logged) */
				space.write_word(dest, space.read_word(src));
				src += 2;
				dest += 2;
			}
		}
		else
		{
			/* Number of blits in the list */
			for ( ; list_len > 0 ; list_len-- )
			{
				int i, j, destptr;

				/* Read offset of source from the list of blits */
				i = src + space.read_word(list+2);
				j = i + (size<<1);
				destptr = dest;

				for (; i<j; destptr+=2, i+=2)
					space.write_word(destptr, space.read_word(i));

				destptr = dest + 14;
				i = space.read_word(list) + m_spr_color_offs;
				space.write_word(destptr, i);

				dest += 16;
				list += 4;
			}

			/* hack for the blit to Sprites RAM - Sprite list end-marker */
			space.write_word(dest, 0xFFFF);
		}
	}
}


/***************************************************************************
                    WEC Le Mans 24 Main CPU Handlers
***************************************************************************/

static ADDRESS_MAP_START( wecleman_map, AS_PROGRAM, 16, wecleman_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM // ROM (03c000-03ffff used as RAM sometimes!)
	AM_RANGE(0x040494, 0x040495) AM_WRITE(wecleman_videostatus_w) AM_SHARE("videostatus")   // cloud blending control (HACK)
	AM_RANGE(0x040000, 0x043fff) AM_RAM // RAM
	AM_RANGE(0x060000, 0x060005) AM_WRITE(wecleman_protection_w) AM_SHARE("protection_ram")
	AM_RANGE(0x060006, 0x060007) AM_READ(wecleman_protection_r) // MCU read
	AM_RANGE(0x080000, 0x080011) AM_RAM_WRITE(blitter_w) AM_SHARE("blitter_regs")   // Blitter
	AM_RANGE(0x100000, 0x103fff) AM_RAM_WRITE(wecleman_pageram_w) AM_SHARE("pageram")   // Background Layers
	AM_RANGE(0x108000, 0x108fff) AM_RAM_WRITE(wecleman_txtram_w) AM_SHARE("txtram") // Text Layer
	AM_RANGE(0x110000, 0x110fff) AM_RAM_WRITE(wecleman_paletteram16_SSSSBBBBGGGGRRRR_word_w) AM_SHARE("paletteram")
	AM_RANGE(0x124000, 0x127fff) AM_RAM AM_SHARE("share1")  // Shared with main CPU
	AM_RANGE(0x130000, 0x130fff) AM_RAM AM_SHARE("spriteram")   // Sprites
	AM_RANGE(0x140000, 0x140001) AM_WRITE(wecleman_soundlatch_w)    // To sound CPU
	AM_RANGE(0x140002, 0x140003) AM_WRITE(selected_ip_w)    // Selects accelerator / wheel / ..
	AM_RANGE(0x140004, 0x140005) AM_WRITE(irqctrl_w)    // Main CPU controls the other CPUs
	AM_RANGE(0x140006, 0x140007) AM_WRITENOP    // Watchdog reset
	AM_RANGE(0x140010, 0x140011) AM_READ_PORT("IN0")    // Coins + brake + gear
	AM_RANGE(0x140012, 0x140013) AM_READ_PORT("IN1")    // ??
	AM_RANGE(0x140014, 0x140015) AM_READ_PORT("DSWA")   // DSW 2
	AM_RANGE(0x140016, 0x140017) AM_READ_PORT("DSWB")   // DSW 1
	AM_RANGE(0x140020, 0x140021) AM_WRITEONLY   // Paired with writes to $140003
	AM_RANGE(0x140020, 0x140021) AM_READ(selected_ip_r) // Accelerator or Wheel or ..
	AM_RANGE(0x140030, 0x140031) AM_WRITENOP    // toggles between 0 & 1 on hitting bumps and crashes (vibration?)
ADDRESS_MAP_END


/***************************************************************************
                        Hot Chase Main CPU Handlers
***************************************************************************/



static ADDRESS_MAP_START( hotchase_map, AS_PROGRAM, 16, wecleman_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x040000, 0x041fff) AM_RAM                                 // RAM
	AM_RANGE(0x060000, 0x063fff) AM_RAM                                 // RAM
	AM_RANGE(0x080000, 0x080011) AM_RAM_WRITE(blitter_w) AM_SHARE("blitter_regs")   // Blitter
	AM_RANGE(0x100000, 0x100fff) AM_DEVREADWRITE8("k051316_1", k051316_device, read, write, 0x00ff) // Background
	AM_RANGE(0x101000, 0x10101f) AM_DEVWRITE8("k051316_1", k051316_device, ctrl_w, 0x00ff)   // Background Ctrl
	AM_RANGE(0x102000, 0x102fff) AM_DEVREADWRITE8("k051316_2", k051316_device, read, write, 0x00ff) // Foreground
	AM_RANGE(0x103000, 0x10301f) AM_DEVWRITE8("k051316_2", k051316_device, ctrl_w, 0x00ff)   // Foreground Ctrl
	AM_RANGE(0x110000, 0x111fff) AM_RAM_WRITE(hotchase_paletteram16_SBGRBBBBGGGGRRRR_word_w) AM_SHARE("paletteram")
	AM_RANGE(0x120000, 0x123fff) AM_RAM AM_SHARE("share1")                  // Shared with sub CPU
	AM_RANGE(0x130000, 0x130fff) AM_RAM AM_SHARE("spriteram")   // Sprites
	AM_RANGE(0x140000, 0x140001) AM_WRITE(hotchase_soundlatch_w)    // To sound CPU
	AM_RANGE(0x140002, 0x140003) AM_WRITE(selected_ip_w)    // Selects accelerator / wheel /
	AM_RANGE(0x140004, 0x140005) AM_WRITE(irqctrl_w)    // Main CPU controls the other CPUs
	AM_RANGE(0x140006, 0x140007) AM_READNOP // Watchdog reset
	AM_RANGE(0x140010, 0x140011) AM_READ_PORT("IN0")    // Coins + brake + gear
	AM_RANGE(0x140012, 0x140013) AM_READ_PORT("IN1")    // ?? bit 4 from sound cpu
	AM_RANGE(0x140014, 0x140015) AM_READ_PORT("DSW2")   // DSW 2
	AM_RANGE(0x140016, 0x140017) AM_READ_PORT("DSW1")   // DSW 1
	AM_RANGE(0x140020, 0x140021) AM_READ(selected_ip_r) AM_WRITENOP // Paired with writes to $140003
	AM_RANGE(0x140022, 0x140023) AM_READNOP // read and written at $601c0, unknown purpose
	AM_RANGE(0x140030, 0x140031) AM_WRITENOP    // signal to cabinet vibration motors?
ADDRESS_MAP_END


/***************************************************************************
                    WEC Le Mans 24 Sub CPU Handlers
***************************************************************************/

static ADDRESS_MAP_START( wecleman_sub_map, AS_PROGRAM, 16, wecleman_state )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM // ROM
	AM_RANGE(0x060000, 0x060fff) AM_RAM AM_SHARE("roadram") // Road
	AM_RANGE(0x070000, 0x073fff) AM_RAM AM_SHARE("share1")  // RAM (Shared with main CPU)
ADDRESS_MAP_END


/***************************************************************************
                        Hot Chase Sub CPU Handlers
***************************************************************************/

static ADDRESS_MAP_START( hotchase_sub_map, AS_PROGRAM, 16, wecleman_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM // ROM
	AM_RANGE(0x020000, 0x020fff) AM_RAM AM_SHARE("roadram") // Road
	AM_RANGE(0x040000, 0x043fff) AM_RAM AM_SHARE("share1") // Shared with main CPU
	AM_RANGE(0x060000, 0x060fff) AM_RAM // a table, presumably road related
	AM_RANGE(0x061000, 0x06101f) AM_RAM // road vregs?
ADDRESS_MAP_END


/***************************************************************************
                    WEC Le Mans 24 Sound CPU Handlers
***************************************************************************/

/* 140001.b */
WRITE16_MEMBER(wecleman_state::wecleman_soundlatch_w)
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_byte_w(space, 0, data & 0xFF);
	}
}

/* Protection - an external multiplyer connected to the sound CPU */
READ8_MEMBER(wecleman_state::multiply_r)
{
	return (m_multiply_reg[0] * m_multiply_reg[1]) & 0xFF;
}

WRITE8_MEMBER(wecleman_state::multiply_w)
{
	m_multiply_reg[offset] = data;
}

/*      K007232 registers reminder:

[Ch A]  [Ch B]  [Meaning]
00      06      address step    (low  byte)
01      07      address step    (high byte, max 1)
02      08      sample address  (low  byte)
03      09      sample address  (mid  byte)
04      0a      sample address  (high byte, max 1 -> max rom size: $20000)
05      0b      Reading this byte triggers the sample

[Ch A & B]
0c              volume
0d              play sample once or looped (2 channels -> 2 bits (0&1))

** sample playing ends when a byte with bit 7 set is reached **/

WRITE8_MEMBER(wecleman_state::wecleman_volume_callback)
{
	m_k007232->set_volume(0, (data >> 4) * 0x11, 0);
	m_k007232->set_volume(1, 0, (data & 0x0f) * 0x11);
}

WRITE8_MEMBER(wecleman_state::wecleman_K00723216_bank_w)
{
	m_k007232->set_bank(0, ~data&1 );  //* (wecleman062gre)
}

static ADDRESS_MAP_START( wecleman_sound_map, AS_PROGRAM, 8, wecleman_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM
	AM_RANGE(0x8500, 0x8500) AM_WRITENOP    // incresed with speed (global volume)?
	AM_RANGE(0x9000, 0x9000) AM_READ(multiply_r)    // Protection
	AM_RANGE(0x9000, 0x9001) AM_WRITE(multiply_w)   // Protection
	AM_RANGE(0x9006, 0x9006) AM_WRITENOP    // ?
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_byte_r) // From main CPU
	AM_RANGE(0xb000, 0xb00d) AM_DEVREADWRITE("k007232", k007232_device, read, write) // K007232 (Reading offset 5/b triggers the sample)
	AM_RANGE(0xc000, 0xc001) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0xf000, 0xf000) AM_WRITE(wecleman_K00723216_bank_w)    // Samples banking
ADDRESS_MAP_END


/***************************************************************************
                        Hot Chase Sound CPU Handlers
***************************************************************************/

/* 140001.b */
WRITE16_MEMBER(wecleman_state::hotchase_soundlatch_w)
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_byte_w(space, 0, data & 0xFF);
	}
}

WRITE8_MEMBER(wecleman_state::hotchase_sound_control_w)
{
//  int reg[8];


//  reg[offset] = data;

	switch (offset)
	{
		/* change volume
		    offset 00000xxx----- channel select (0:channel 0, 1:channel 1)
		    ++------ chip select ( 0:chip 1, 1:chip2, 2:chip3)
		    data&0x0f left volume  (data>>4)&0x0f right volume
			*/
		case 0x0:
		case 0x1:
			m_k007232_1->set_volume( offset&1,  (data&0x0f) * 0x08, (data>>4) * 0x08 );
			break;
		case 0x2:
		case 0x3:
			m_k007232_2->set_volume( offset&1,  (data&0x0f) * 0x08, (data>>4) * 0x08 );
			break;
		case 0x4:
		case 0x5:
			m_k007232_3->set_volume( offset&1,  (data&0x0f) * 0x08, (data>>4) * 0x08 );
			break;

		case 0x06:  /* Bankswitch for chips 0 & 1 */
		{
			int bank0_a = (data >> 1) & 1;
			int bank1_a = (data >> 2) & 1;
			int bank0_b = (data >> 3) & 1;
			int bank1_b = (data >> 4) & 1;
			// bit 6: chip 2 - ch0 ?
			// bit 7: chip 2 - ch1 ?

			m_k007232_1->set_bank( bank0_a, bank0_b );
			m_k007232_2->set_bank( bank1_a, bank1_b );
		}
		break;

		case 0x07:  /* Bankswitch for chip 2 */
		{
			int bank2_a = (data >> 0) & 7;
			int bank2_b = (data >> 3) & 7;

			m_k007232_3->set_bank( bank2_a, bank2_b );
		}
		break;
	}
}

WRITE8_MEMBER(wecleman_state::hotchase_sound_hs_w)
{
	m_hotchase_sound_hs = true;
}

/* Read and write handlers for one K007232 chip:
   even and odd register are mapped swapped */
READ8_MEMBER(wecleman_state::hotchase_1_k007232_r)
{
	return m_k007232_1->read(space, offset ^ 1);
}

WRITE8_MEMBER(wecleman_state::hotchase_1_k007232_w)
{
	m_k007232_1->write(space, offset ^ 1, data);
}

READ8_MEMBER(wecleman_state::hotchase_2_k007232_r)
{
	return m_k007232_2->read(space, offset ^ 1);
}

WRITE8_MEMBER(wecleman_state::hotchase_2_k007232_w)
{
	m_k007232_2->write(space, offset ^ 1, data);
}

READ8_MEMBER(wecleman_state::hotchase_3_k007232_r)
{
	return m_k007232_3->read(space, offset ^ 1);
}

WRITE8_MEMBER(wecleman_state::hotchase_3_k007232_w)
{
	m_k007232_3->write(space, offset ^ 1, data);
}

static ADDRESS_MAP_START( hotchase_sound_map, AS_PROGRAM, 8, wecleman_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x1000, 0x100d) AM_READWRITE(hotchase_1_k007232_r, hotchase_1_k007232_w)   // 3 x K007232
	AM_RANGE(0x2000, 0x200d) AM_READWRITE(hotchase_2_k007232_r, hotchase_2_k007232_w)
	AM_RANGE(0x3000, 0x300d) AM_READWRITE(hotchase_3_k007232_r, hotchase_3_k007232_w)
	AM_RANGE(0x4000, 0x4007) AM_WRITE(hotchase_sound_control_w) // Sound volume, banking, etc.
	AM_RANGE(0x5000, 0x5000) AM_WRITENOP   // 0 at start of IRQ service, 1 at end (irq mask?)
	AM_RANGE(0x6000, 0x6000) AM_READ(soundlatch_byte_r) // From main CPU (Read on IRQ)
	AM_RANGE(0x7000, 0x7000) AM_WRITE(hotchase_sound_hs_w)    // ACK signal to main CPU
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


/***************************************************************************
                        WEC Le Mans 24 Input Ports
***************************************************************************/

static INPUT_PORTS_START( wecleman )
	PORT_START("IN0")   /* $140011.b */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Shift") PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Brake")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")   /* Motor? - $140013.b */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Right SW")  // right sw
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Left SW")  // left sw
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME("Thermo SW")  // thermo
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL )   // from sound cpu ?
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")  /* $140015.b */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "No Coin B" )
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSWB")  /* $140017.b */
	PORT_DIPNAME( 0x01, 0x01, "Speed Unit" )
	PORT_DIPSETTING(    0x01, "km/h" )
	PORT_DIPSETTING(    0x00, "mph" )
	PORT_DIPNAME( 0x02, 0x02, "Unknown B-1" )   // single
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown B-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )     // 66 seconds at the start
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )   // 64
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )     // 62
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )  // 60
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown B-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown B-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("ACCEL") /* Accelerator - $140021.b (0) */
	PORT_BIT( 0xff, 0, IPT_PEDAL ) PORT_MINMAX(0,0x80) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

	PORT_START("STEER") /* Steering Wheel - $140021.b (2) */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)
INPUT_PORTS_END


/***************************************************************************
                            Hot Chase Input Ports
***************************************************************************/

CUSTOM_INPUT_MEMBER(wecleman_state::hotchase_sound_status_r)
{
	return m_hotchase_sound_hs;
}

static INPUT_PORTS_START( hotchase )
	PORT_START("IN0")   /* $140011.b */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Shift") PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Brake")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")   /* Motor? - $140013.b */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Right SW")   // right sw
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Left SW")  // left sw
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME("Thermo SW")  // thermo
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) // from sound cpu
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, wecleman_state,hotchase_sound_status_r, NULL)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")  /* $140015.b */
	PORT_DIPNAME( 0x01, 0x01, "Speed Unit" )
	PORT_DIPSETTING(    0x01, "KM" )
	PORT_DIPSETTING(    0x00, "M.P.H." )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 2-1" )   // single (wheel related)
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Unknown 2-3&4" ) // Most likely Difficulty
	PORT_DIPSETTING(    0x18, "0" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x00, "c" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5" )   // single
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* wheel <-> brake ; accel -> start */
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )   // single (wheel<->brake)
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")  /* $140017.b */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/99 Credits" )

	PORT_START("ACCEL") /* Accelerator - $140021.b (0) */
	PORT_BIT( 0xff, 0, IPT_PEDAL ) PORT_MINMAX(0,0x80) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

	PORT_START("STEER") /* Steering Wheel - $140021.b (2) */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)
INPUT_PORTS_END


/***************************************************************************
                            WEC Le Mans 24 Graphics Layout
***************************************************************************/

static const gfx_layout wecleman_bg_layout =
{
	8,8,
	8*0x8000*3/(8*8*3),
	3,
	{ 0,0x8000*8,0x8000*8*2 },
	{0,7,6,5,4,3,2,1},
	{0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8},
	8*8
};

static const UINT32 wecleman_road_layout_xoffset[64] =
{
		0,7,6,5,4,3,2,1,
		8,15,14,13,12,11,10,9,
		16,23,22,21,20,19,18,17,
		24,31,30,29,28,27,26,25,

		0+32,7+32,6+32,5+32,4+32,3+32,2+32,1+32,
		8+32,15+32,14+32,13+32,12+32,11+32,10+32,9+32,
		16+32,23+32,22+32,21+32,20+32,19+32,18+32,17+32,
		24+32,31+32,30+32,29+32,28+32,27+32,26+32,25+32
};

/* We draw the road, made of 512 pixel lines, using 64x1 tiles */
static const gfx_layout wecleman_road_layout =
{
	64,1,
	8*0x4000*3/(64*1*3),
	3,
	{ 0x4000*8*2,0x4000*8*1,0x4000*8*0 },
	EXTENDED_XOFFS,
	{0},
	64*1,
	wecleman_road_layout_xoffset,
	NULL
};

static GFXDECODE_START( wecleman )
	// "gfx1" holds sprite, which are not decoded here
	GFXDECODE_ENTRY( "gfx2", 0, wecleman_bg_layout,   0, 2048/8 )   // [0] bg + fg + txt
	GFXDECODE_ENTRY( "gfx3", 0, wecleman_road_layout, 0, 2048/8 )   // [1] road
GFXDECODE_END


/***************************************************************************
                            Hot Chase Graphics Layout
***************************************************************************/

static const UINT32 hotchase_road_layout_xoffset[64] =
{
		0*4,0*4,1*4,1*4,2*4,2*4,3*4,3*4,4*4,4*4,5*4,5*4,6*4,6*4,7*4,7*4,
		8*4,8*4,9*4,9*4,10*4,10*4,11*4,11*4,12*4,12*4,13*4,13*4,14*4,14*4,15*4,15*4,
		16*4,16*4,17*4,17*4,18*4,18*4,19*4,19*4,20*4,20*4,21*4,21*4,22*4,22*4,23*4,23*4,
		24*4,24*4,25*4,25*4,26*4,26*4,27*4,27*4,28*4,28*4,29*4,29*4,30*4,30*4,31*4,31*4
};

/* We draw the road, made of 512 pixel lines, using 64x1 tiles */
/* tiles are doubled horizontally */
static const gfx_layout hotchase_road_layout =
{
	64,1,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	EXTENDED_XOFFS,
	{0},
	32*4,
	hotchase_road_layout_xoffset,
	NULL
};

static GFXDECODE_START( hotchase )
	// "gfx1" holds sprite, which are not decoded here
	// "gfx2" and 3 are for the 051316
	GFXDECODE_ENTRY( "gfx4", 0, hotchase_road_layout, 0x70*16, 16 ) // road
GFXDECODE_END


/***************************************************************************
                        WEC Le Mans 24 Hardware Definitions
***************************************************************************/


TIMER_DEVICE_CALLBACK_MEMBER(wecleman_state::wecleman_scanline)
{
	int scanline = param;

	if(scanline == 232) // vblank irq
		m_maincpu->set_input_line(4, HOLD_LINE);
	else if(((scanline % 64) == 0)) // timer irq TODO: wrong place maybe? Could do with blitter chip irq (007643/007645?) or "V-CNT" signal.
		m_maincpu->set_input_line(5, HOLD_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(wecleman_state::hotchase_scanline)
{
	int scanline = param;

	if(scanline == 224) // vblank irq
		m_maincpu->set_input_line(4, HOLD_LINE);
}


MACHINE_RESET_MEMBER(wecleman_state,wecleman)
{
	m_k007232->set_bank( 0, 1 );
}

static MACHINE_CONFIG_START( wecleman, wecleman_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000)   /* Schems show 10MHz */
	MCFG_CPU_PROGRAM_MAP(wecleman_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", wecleman_state, wecleman_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("sub", M68000, 10000000)   /* Schems show 10MHz */
	MCFG_CPU_PROGRAM_MAP(wecleman_sub_map)

	/* Schems: can be reset, no nmi, soundlatch, 3.58MHz */
	MCFG_CPU_ADD("audiocpu", Z80, 3579545)
	MCFG_CPU_PROGRAM_MAP(wecleman_sound_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_MACHINE_RESET_OVERRIDE(wecleman_state,wecleman)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320 +16, 256)
	MCFG_SCREEN_VISIBLE_AREA(0 +8, 320-1 +8, 0 +8, 224-1 +8)
	MCFG_SCREEN_UPDATE_DRIVER(wecleman_state, screen_update_wecleman)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", wecleman)

	MCFG_PALETTE_ADD("palette", 2048)

	MCFG_VIDEO_START_OVERRIDE(wecleman_state,wecleman)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", 3579545)
	MCFG_SOUND_ROUTE(0, "mono", 0.85)
	MCFG_SOUND_ROUTE(1, "mono", 0.85)

	MCFG_SOUND_ADD("k007232", K007232, 3579545)
	MCFG_K007232_PORT_WRITE_HANDLER(WRITE8(wecleman_state, wecleman_volume_callback))
	MCFG_SOUND_ROUTE(0, "mono", 0.20)
	MCFG_SOUND_ROUTE(1, "mono", 0.20)
MACHINE_CONFIG_END


/***************************************************************************
                        Hot Chase Hardware Definitions
***************************************************************************/

INTERRUPT_GEN_MEMBER(wecleman_state::hotchase_sound_timer)
{
	device.execute().set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
}

MACHINE_RESET_MEMBER(wecleman_state,hotchase)
{
	int i;

	/* TODO: PCB reference clearly shows that the POST has random/filled data on the paletteram.
	         For now let's fill everything with white colors until we have better info about it */
	for(i=0;i<0x2000/2;i++)
	{
		m_generic_paletteram_16[i] = 0xffff;
		m_palette->set_pen_color(i,0xff,0xff,0xff);
	}
}


static MACHINE_CONFIG_START( hotchase, wecleman_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000)   /* 10 MHz - PCB is drawn in one set's readme */
	MCFG_CPU_PROGRAM_MAP(hotchase_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", wecleman_state, hotchase_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("sub", M68000, 10000000)   /* 10 MHz - PCB is drawn in one set's readme */
	MCFG_CPU_PROGRAM_MAP(hotchase_sub_map)

	MCFG_CPU_ADD("audiocpu", M6809, 3579545 / 2)    /* 3.579/2 MHz - PCB is drawn in one set's readme */
	MCFG_CPU_PROGRAM_MAP(hotchase_sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(wecleman_state, hotchase_sound_timer,  496)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_MACHINE_RESET_OVERRIDE(wecleman_state,hotchase)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320 +16, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(wecleman_state, screen_update_hotchase)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", hotchase)
	MCFG_PALETTE_ADD("palette", 2048*2)

	MCFG_VIDEO_START_OVERRIDE(wecleman_state, hotchase)

	MCFG_DEVICE_ADD("k051316_1", K051316, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K051316_OFFSETS(-0xb0 / 2, -16)
	MCFG_K051316_WRAP(1)
	MCFG_K051316_CB(wecleman_state, hotchase_zoom_callback_1)

	MCFG_DEVICE_ADD("k051316_2", K051316, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K051316_OFFSETS(-0xb0 / 2, -16)
	MCFG_K051316_CB(wecleman_state, hotchase_zoom_callback_2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("k007232_1", K007232, 3579545)
	// SLEV not used, volume control is elsewhere
	MCFG_SOUND_ROUTE(0, "mono", 0.20)
	MCFG_SOUND_ROUTE(1, "mono", 0.20)

	MCFG_SOUND_ADD("k007232_2", K007232, 3579545)
	// SLEV not used, volume control is elsewhere
	MCFG_SOUND_ROUTE(0, "mono", 0.20)
	MCFG_SOUND_ROUTE(1, "mono", 0.20)

	MCFG_SOUND_ADD("k007232_3", K007232, 3579545)
	// SLEV not used, volume control is elsewhere
	MCFG_SOUND_ROUTE(0, "mono", 0.20)
	MCFG_SOUND_ROUTE(1, "mono", 0.20)
MACHINE_CONFIG_END


/***************************************************************************
                        WEC Le Mans 24 ROM Definitions
***************************************************************************/

ROM_START( wecleman )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* Main CPU Code */
	ROM_LOAD16_BYTE( "602f08.17h", 0x00000, 0x10000, CRC(493b79d3) SHA1(9625e3b65c211d5081d8ed8977de287eff100842) )
	ROM_LOAD16_BYTE( "602f11.23h", 0x00001, 0x10000, CRC(6bb4f1fa) SHA1(2cfb7885b42b49dab9892e8dfd54914b64eeab06) )
	ROM_LOAD16_BYTE( "602a09.18h", 0x20000, 0x10000, CRC(8a9d756f) SHA1(12605e86ce29e6300b5400720baac7b0293d9e66) )
	ROM_LOAD16_BYTE( "602a10.22h", 0x20001, 0x10000, CRC(569f5001) SHA1(ec2dd331a279083cf847fbbe71c017038a1d562a) )

	ROM_REGION( 0x10000, "sub", 0 ) /* Sub CPU Code */
	ROM_LOAD16_BYTE( "602a06.18a", 0x00000, 0x08000, CRC(e12c0d11) SHA1(991afd48bf1b2c303b975ce80c754e5972c39111) )
	ROM_LOAD16_BYTE( "602a07.20a", 0x00001, 0x08000, CRC(47968e51) SHA1(9b01b2c6a14dd80327a8f66a7f1994471a4bc38e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU Code */
	ROM_LOAD( "602a01.6d",  0x00000, 0x08000, CRC(deafe5f1) SHA1(4cfbe2841233b1222c22160af7287b7a7821c3a0) )

	ROM_REGION( 0x200000 * 2, "gfx1", 0 )   /* x2, do not dispose, zooming sprites */
	ROM_LOAD( "602a25.12e", 0x000000, 0x20000, CRC(0eacf1f9) SHA1(b4dcd457e68175ffee3da4aff23a241fe33eb500) )
	ROM_LOAD( "602a26.14e", 0x020000, 0x20000, CRC(2182edaf) SHA1(5ae4223a76b3c0be8f66458707f2e6f63fba0b13) )
	ROM_LOAD( "602a27.15e", 0x040000, 0x20000, CRC(b22f08e9) SHA1(1ba99bc4e00e206507e9bfafc989208d6ae6f8a3) )
	ROM_LOAD( "602a28.17e", 0x060000, 0x20000, CRC(5f6741fa) SHA1(9c81634f502da8682673b3b87efe0497af8abbd7) )
	ROM_LOAD( "602a21.6e",  0x080000, 0x20000, CRC(8cab34f1) SHA1(264df01460f44cd5ccdf3c8bd2d3f327874b69ea) )
	ROM_LOAD( "602a22.7e",  0x0a0000, 0x20000, CRC(e40303cb) SHA1(da943437ea2e208ea477f35bb05f77412ecdf9ac) )
	ROM_LOAD( "602a23.9e",  0x0c0000, 0x20000, CRC(75077681) SHA1(32ad10e9e32779c36bb50b402f5c6d941e293942) )
	ROM_LOAD( "602a24.10e", 0x0e0000, 0x20000, CRC(583dadad) SHA1(181ebe87095d739a5903c17ec851864e2275f571) )
	ROM_LOAD( "602a17.12c", 0x100000, 0x20000, CRC(31612199) SHA1(dff58ec3f7d98bfa7e9405f0f23647ff4ecfee62) )
	ROM_LOAD( "602a18.14c", 0x120000, 0x20000, CRC(3f061a67) SHA1(be57c38410c5635311d26afc44b3065e42fa12b7) )
	ROM_LOAD( "602a19.15c", 0x140000, 0x20000, CRC(5915dbc5) SHA1(61ab123c8a4128a18d7eb2cae99ad58203f03ffc) )
	ROM_LOAD( "602a20.17c", 0x160000, 0x20000, CRC(f87e4ef5) SHA1(4c2f0d036925a7ccd32aef3ca12b960a27247bc3) )
	ROM_LOAD( "602a13.6c",  0x180000, 0x20000, CRC(5d3589b8) SHA1(d146cb8511cfe825bdfe8296c7758545542a0faa) )
	ROM_LOAD( "602a14.7c",  0x1a0000, 0x20000, CRC(e3a75f6c) SHA1(80b20323e3560316ffbdafe4fd2f81326e103045) )
	ROM_LOAD( "602a15.9c",  0x1c0000, 0x20000, CRC(0d493c9f) SHA1(02690a1963cadd469bd67cb362384923916900a1) )
	ROM_LOAD( "602a16.10c", 0x1e0000, 0x20000, CRC(b08770b3) SHA1(41871e9261d08fd372b7deb72d939973fb694b54) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "602a31.26g", 0x000000, 0x08000, CRC(01fa40dd) SHA1(2b8aa97f5116f39ae6a8e46f109853d70e370884) )   // layers
	ROM_LOAD( "602a30.24g", 0x008000, 0x08000, CRC(be5c4138) SHA1(7aee2ee17ef3e37399a60d9b019cfa733acbf07b) )
	ROM_LOAD( "602a29.23g", 0x010000, 0x08000, CRC(f1a8d33e) SHA1(ed6531f2fd4ad6835a879e9a5600387d8cad6d17) )

	ROM_REGION( 0x0c000, "gfx3", 0 )    /* road */
	ROM_LOAD( "602a04.11e", 0x000000, 0x08000, CRC(ade9f359) SHA1(58db6be6217ed697827015e50e99e58602042a4c) )
	ROM_LOAD( "602a05.13e", 0x008000, 0x04000, CRC(f22b7f2b) SHA1(857389c57552c4e2237cb599f4c68c381430475e) )   // may also exist as 32KB with one half empty

	ROM_REGION( 0x40000, "k007232", 0 )  /* Samples (Channel A 0x20000=Channel B) */
	ROM_LOAD( "602a03.10a", 0x00000, 0x20000, CRC(31392b01) SHA1(0424747bc2015c9c93afd20e6a23083c0dcc4fb7) )
	ROM_LOAD( "602a02.8a",  0x20000, 0x20000, CRC(e2be10ae) SHA1(109c31bf7252c83a062d259143cd8299681db778) )

	ROM_REGION( 0x04000, "user1", 0 )   /* extra data for road effects? */
	ROM_LOAD( "602a12.1a",  0x000000, 0x04000, CRC(77b9383d) SHA1(7cb970889677704d6324bb64aafc05326c4503ad) )
ROM_END

ROM_START( weclemana )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* Main CPU Code */
	// I doubt these labels are correct, or one set of roms is bad.
	ROM_LOAD16_BYTE( "602f08.17h", 0x00000, 0x10000, CRC(43241265) SHA1(3da1ed0d15b03845c07f07ec6838ce160d81633d) ) // only 17h and 23h differ slightly from parent
	ROM_LOAD16_BYTE( "602f11.23h", 0x00001, 0x10000, CRC(3ea7dae0) SHA1(d33d67f4cc65a7680e5f43407136b75512a10230) ) // "
	ROM_LOAD16_BYTE( "602a09.18h", 0x20000, 0x10000, CRC(8a9d756f) SHA1(12605e86ce29e6300b5400720baac7b0293d9e66) )
	ROM_LOAD16_BYTE( "602a10.22h", 0x20001, 0x10000, CRC(569f5001) SHA1(ec2dd331a279083cf847fbbe71c017038a1d562a) )

	ROM_REGION( 0x10000, "sub", 0 ) /* Sub CPU Code */
	ROM_LOAD16_BYTE( "602a06.18a", 0x00000, 0x08000, CRC(e12c0d11) SHA1(991afd48bf1b2c303b975ce80c754e5972c39111) )
	ROM_LOAD16_BYTE( "602a07.20a", 0x00001, 0x08000, CRC(47968e51) SHA1(9b01b2c6a14dd80327a8f66a7f1994471a4bc38e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU Code */
	ROM_LOAD( "602a01.6d",  0x00000, 0x08000, CRC(deafe5f1) SHA1(4cfbe2841233b1222c22160af7287b7a7821c3a0) )

	ROM_REGION( 0x200000 * 2, "gfx1", 0 )   /* x2, do not dispose, zooming sprites */
	ROM_LOAD( "602a25.12e", 0x000000, 0x20000, CRC(0eacf1f9) SHA1(b4dcd457e68175ffee3da4aff23a241fe33eb500) )
	ROM_LOAD( "602a26.14e", 0x020000, 0x20000, CRC(2182edaf) SHA1(5ae4223a76b3c0be8f66458707f2e6f63fba0b13) )
	ROM_LOAD( "602a27.15e", 0x040000, 0x20000, CRC(b22f08e9) SHA1(1ba99bc4e00e206507e9bfafc989208d6ae6f8a3) )
	ROM_LOAD( "602a28.17e", 0x060000, 0x20000, CRC(5f6741fa) SHA1(9c81634f502da8682673b3b87efe0497af8abbd7) )
	ROM_LOAD( "602a21.6e",  0x080000, 0x20000, CRC(8cab34f1) SHA1(264df01460f44cd5ccdf3c8bd2d3f327874b69ea) )
	ROM_LOAD( "602a22.7e",  0x0a0000, 0x20000, CRC(e40303cb) SHA1(da943437ea2e208ea477f35bb05f77412ecdf9ac) )
	ROM_LOAD( "602a23.9e",  0x0c0000, 0x20000, CRC(75077681) SHA1(32ad10e9e32779c36bb50b402f5c6d941e293942) )
	ROM_LOAD( "602a24.10e", 0x0e0000, 0x20000, CRC(583dadad) SHA1(181ebe87095d739a5903c17ec851864e2275f571) )
	ROM_LOAD( "602a17.12c", 0x100000, 0x20000, CRC(31612199) SHA1(dff58ec3f7d98bfa7e9405f0f23647ff4ecfee62) )
	ROM_LOAD( "602a18.14c", 0x120000, 0x20000, CRC(3f061a67) SHA1(be57c38410c5635311d26afc44b3065e42fa12b7) )
	ROM_LOAD( "602a19.15c", 0x140000, 0x20000, CRC(5915dbc5) SHA1(61ab123c8a4128a18d7eb2cae99ad58203f03ffc) )
	ROM_LOAD( "602a20.17c", 0x160000, 0x20000, CRC(f87e4ef5) SHA1(4c2f0d036925a7ccd32aef3ca12b960a27247bc3) )
	ROM_LOAD( "602a13.6c",  0x180000, 0x20000, CRC(5d3589b8) SHA1(d146cb8511cfe825bdfe8296c7758545542a0faa) )
	ROM_LOAD( "602a14.7c",  0x1a0000, 0x20000, CRC(e3a75f6c) SHA1(80b20323e3560316ffbdafe4fd2f81326e103045) )
	ROM_LOAD( "602a15.9c",  0x1c0000, 0x20000, CRC(0d493c9f) SHA1(02690a1963cadd469bd67cb362384923916900a1) )
	ROM_LOAD( "602a16.10c", 0x1e0000, 0x20000, CRC(b08770b3) SHA1(41871e9261d08fd372b7deb72d939973fb694b54) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "602a31.26g", 0x000000, 0x08000, CRC(01fa40dd) SHA1(2b8aa97f5116f39ae6a8e46f109853d70e370884) )   // layers
	ROM_LOAD( "602a30.24g", 0x008000, 0x08000, CRC(be5c4138) SHA1(7aee2ee17ef3e37399a60d9b019cfa733acbf07b) )
	ROM_LOAD( "602a29.23g", 0x010000, 0x08000, CRC(f1a8d33e) SHA1(ed6531f2fd4ad6835a879e9a5600387d8cad6d17) )

	ROM_REGION( 0x0c000, "gfx3", 0 )    /* road */
	ROM_LOAD( "602a04.11e", 0x000000, 0x08000, CRC(ade9f359) SHA1(58db6be6217ed697827015e50e99e58602042a4c) )
	ROM_LOAD( "602a05.13e", 0x008000, 0x04000, CRC(f22b7f2b) SHA1(857389c57552c4e2237cb599f4c68c381430475e) )   // may also exist as 32KB with one half empty

	ROM_REGION( 0x40000, "k007232", 0 )  /* Samples (Channel A 0x20000=Channel B) */
	ROM_LOAD( "602a03.10a", 0x00000, 0x20000, CRC(31392b01) SHA1(0424747bc2015c9c93afd20e6a23083c0dcc4fb7) )
	ROM_LOAD( "602a02.8a",  0x20000, 0x20000, CRC(e2be10ae) SHA1(109c31bf7252c83a062d259143cd8299681db778) )

	ROM_REGION( 0x04000, "user1", 0 )   /* extra data for road effects? */
	ROM_LOAD( "602a12.1a",  0x000000, 0x04000, CRC(77b9383d) SHA1(7cb970889677704d6324bb64aafc05326c4503ad) )
ROM_END
/*
early set V.1.26
rom labels faded out, all other roms match
*/
ROM_START( weclemanb )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* Main CPU Code */
	ROM_LOAD16_BYTE( "17h", 0x00000, 0x10000, CRC(66901326) SHA1(672aab497e9b94843451e016de6ca6d3c358362e) )
	ROM_LOAD16_BYTE( "23h", 0x00001, 0x10000, CRC(d9d492f4) SHA1(12c177fa5cc541be86431f314e96a4f3a74f95c6) )
	ROM_LOAD16_BYTE( "602a09.18h", 0x20000, 0x10000, CRC(8a9d756f) SHA1(12605e86ce29e6300b5400720baac7b0293d9e66) )
	ROM_LOAD16_BYTE( "602a10.22h", 0x20001, 0x10000, CRC(569f5001) SHA1(ec2dd331a279083cf847fbbe71c017038a1d562a) )

	ROM_REGION( 0x10000, "sub", 0 ) /* Sub CPU Code */
	ROM_LOAD16_BYTE( "602a06.18a", 0x00000, 0x08000, CRC(e12c0d11) SHA1(991afd48bf1b2c303b975ce80c754e5972c39111) )
	ROM_LOAD16_BYTE( "602a07.20a", 0x00001, 0x08000, CRC(47968e51) SHA1(9b01b2c6a14dd80327a8f66a7f1994471a4bc38e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU Code */
	ROM_LOAD( "602a01.6d",  0x00000, 0x08000, CRC(deafe5f1) SHA1(4cfbe2841233b1222c22160af7287b7a7821c3a0) )

	ROM_REGION( 0x200000 * 2, "gfx1", 0 )   /* x2, do not dispose, zooming sprites */
	ROM_LOAD( "602a25.12e", 0x000000, 0x20000, CRC(0eacf1f9) SHA1(b4dcd457e68175ffee3da4aff23a241fe33eb500) )
	ROM_LOAD( "602a26.14e", 0x020000, 0x20000, CRC(2182edaf) SHA1(5ae4223a76b3c0be8f66458707f2e6f63fba0b13) )
	ROM_LOAD( "602a27.15e", 0x040000, 0x20000, CRC(b22f08e9) SHA1(1ba99bc4e00e206507e9bfafc989208d6ae6f8a3) )
	ROM_LOAD( "602a28.17e", 0x060000, 0x20000, CRC(5f6741fa) SHA1(9c81634f502da8682673b3b87efe0497af8abbd7) )
	ROM_LOAD( "602a21.6e",  0x080000, 0x20000, CRC(8cab34f1) SHA1(264df01460f44cd5ccdf3c8bd2d3f327874b69ea) )
	ROM_LOAD( "602a22.7e",  0x0a0000, 0x20000, CRC(e40303cb) SHA1(da943437ea2e208ea477f35bb05f77412ecdf9ac) )
	ROM_LOAD( "602a23.9e",  0x0c0000, 0x20000, CRC(75077681) SHA1(32ad10e9e32779c36bb50b402f5c6d941e293942) )
	ROM_LOAD( "602a24.10e", 0x0e0000, 0x20000, CRC(583dadad) SHA1(181ebe87095d739a5903c17ec851864e2275f571) )
	ROM_LOAD( "602a17.12c", 0x100000, 0x20000, CRC(31612199) SHA1(dff58ec3f7d98bfa7e9405f0f23647ff4ecfee62) )
	ROM_LOAD( "602a18.14c", 0x120000, 0x20000, CRC(3f061a67) SHA1(be57c38410c5635311d26afc44b3065e42fa12b7) )
	ROM_LOAD( "602a19.15c", 0x140000, 0x20000, CRC(5915dbc5) SHA1(61ab123c8a4128a18d7eb2cae99ad58203f03ffc) )
	ROM_LOAD( "602a20.17c", 0x160000, 0x20000, CRC(f87e4ef5) SHA1(4c2f0d036925a7ccd32aef3ca12b960a27247bc3) )
	ROM_LOAD( "602a13.6c",  0x180000, 0x20000, CRC(5d3589b8) SHA1(d146cb8511cfe825bdfe8296c7758545542a0faa) )
	ROM_LOAD( "602a14.7c",  0x1a0000, 0x20000, CRC(e3a75f6c) SHA1(80b20323e3560316ffbdafe4fd2f81326e103045) )
	ROM_LOAD( "602a15.9c",  0x1c0000, 0x20000, CRC(0d493c9f) SHA1(02690a1963cadd469bd67cb362384923916900a1) )
	ROM_LOAD( "602a16.10c", 0x1e0000, 0x20000, CRC(b08770b3) SHA1(41871e9261d08fd372b7deb72d939973fb694b54) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "602a31.26g", 0x000000, 0x08000, CRC(01fa40dd) SHA1(2b8aa97f5116f39ae6a8e46f109853d70e370884) )   // layers
	ROM_LOAD( "602a30.24g", 0x008000, 0x08000, CRC(be5c4138) SHA1(7aee2ee17ef3e37399a60d9b019cfa733acbf07b) )
	ROM_LOAD( "602a29.23g", 0x010000, 0x08000, CRC(f1a8d33e) SHA1(ed6531f2fd4ad6835a879e9a5600387d8cad6d17) )

	ROM_REGION( 0x0c000, "gfx3", 0 )    /* road */
	ROM_LOAD( "602a04.11e", 0x000000, 0x08000, CRC(ade9f359) SHA1(58db6be6217ed697827015e50e99e58602042a4c) )
	ROM_LOAD( "602a05.13e", 0x008000, 0x04000, CRC(f22b7f2b) SHA1(857389c57552c4e2237cb599f4c68c381430475e) )   // may also exist as 32KB with one half empty

	ROM_REGION( 0x40000, "k007232", 0 )  /* Samples (Channel A 0x20000=Channel B) */
	ROM_LOAD( "602a03.10a", 0x00000, 0x20000, CRC(31392b01) SHA1(0424747bc2015c9c93afd20e6a23083c0dcc4fb7) )
	ROM_LOAD( "602a02.8a",  0x20000, 0x20000, CRC(e2be10ae) SHA1(109c31bf7252c83a062d259143cd8299681db778) )

	ROM_REGION( 0x04000, "user1", 0 )   /* extra data for road effects? */
	ROM_LOAD( "602a12.1a",  0x000000, 0x04000, CRC(77b9383d) SHA1(7cb970889677704d6324bb64aafc05326c4503ad) )
ROM_END

void wecleman_state::wecleman_unpack_sprites()
{
	const char *region       = "gfx1";  // sprites

	const UINT32 len = memregion(region)->bytes();
	UINT8 *src     = memregion(region)->base() + len / 2 - 1;
	UINT8 *dst     = memregion(region)->base() + len - 1;

	while(dst > src)
	{
		UINT8 data = *src--;
		if( (data&0xf0) == 0xf0 ) data &= 0x0f;
		if( (data&0x0f) == 0x0f ) data &= 0xf0;
		*dst-- = data & 0xF;    *dst-- = data >> 4;
	}
}

void wecleman_state::bitswap(UINT8 *src,size_t len,int _14,int _13,int _12,int _11,int _10,int _f,int _e,int _d,int _c,int _b,int _a,int _9,int _8,int _7,int _6,int _5,int _4,int _3,int _2,int _1,int _0)
{
	dynamic_buffer buffer(len);
	int i;

	memcpy(&buffer[0],src,len);
	for (i = 0;i < len;i++)
	{
		src[i] =
			buffer[BITSWAP24(i,23,22,21,_14,_13,_12,_11,_10,_f,_e,_d,_c,_b,_a,_9,_8,_7,_6,_5,_4,_3,_2,_1,_0)];
	}
}

/* Unpack sprites data and do some patching */
DRIVER_INIT_MEMBER(wecleman_state,wecleman)
{
	int i, len;
	UINT8 *RAM;
//  UINT16 *RAM1 = (UINT16 *) memregion("maincpu")->base();   /* Main CPU patches */
//  RAM1[0x08c2/2] = 0x601e;    // faster self test

	/* Decode GFX Roms - Compensate for the address lines scrambling */

	/*  Sprites - decrypting the sprites nearly KILLED ME!
	    It's been the main cause of the delay of this driver ...
	    I hope you'll appreciate this effort!  */

	/* let's swap even and odd *pixels* of the sprites */
	RAM = memregion("gfx1")->base();
	len = memregion("gfx1")->bytes();
	for (i = 0; i < len; i ++)
	{
		/* TODO: could be wrong, colors have to be fixed.       */
		/* The only certain thing is that 87 must convert to f0 */
		/* otherwise stray lines appear, made of pens 7 & 8     */
		RAM[i] = BITSWAP8(RAM[i],7,0,1,2,3,4,5,6);
	}

	bitswap(memregion("gfx1")->base(), memregion("gfx1")->bytes(),
			0,1,20,19,18,17,14,9,16,6,4,7,8,15,10,11,13,5,12,3,2);

	/* Now we can unpack each nibble of the sprites into a pixel (one byte) */
	wecleman_unpack_sprites();

	/* Bg & Fg & Txt */
	bitswap(memregion("gfx2")->base(), memregion("gfx2")->bytes(),
			20,19,18,17,16,15,12,7,14,4,2,5,6,13,8,9,11,3,10,1,0);

	/* Road */
	bitswap(memregion("gfx3")->base(), memregion("gfx3")->bytes(),
			20,19,18,17,16,15,14,7,12,4,2,5,6,13,8,9,11,3,10,1,0);

	m_spr_color_offs = 0x40;
	m_sound_hw_type = 0;
}


/***************************************************************************
                            Hot Chase ROM Definitions
***************************************************************************/

ROM_START( hotchase )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* Main Code */
	ROM_LOAD16_BYTE( "763k05", 0x000000, 0x010000, CRC(f34fef0b) SHA1(9edaf6da988348cb32d5686fe7a67fb92b1c9777) )
	ROM_LOAD16_BYTE( "763k04", 0x000001, 0x010000, CRC(60f73178) SHA1(49c919d09fa464b205d7eccce337349e3a633a14) )
	ROM_LOAD16_BYTE( "763k03", 0x020000, 0x010000, CRC(28e3a444) SHA1(106b22a3cbe8301eac2e46674a267b96e72ac72f) )
	ROM_LOAD16_BYTE( "763k02", 0x020001, 0x010000, CRC(9510f961) SHA1(45b1920cab08a0dacd044c851d4e7f0cb5772b46) )

	ROM_REGION( 0x20000, "sub", 0 ) /* Sub Code */
	ROM_LOAD16_BYTE( "763k07", 0x000000, 0x010000, CRC(ae12fa90) SHA1(7f76f09916fe152411b5af3c504ee7be07497ef4) )
	ROM_LOAD16_BYTE( "763k06", 0x000001, 0x010000, CRC(b77e0c07) SHA1(98bf492ac889d31419df706029fdf3d51b85c936) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound Code */
	ROM_LOAD( "763f01", 0x8000, 0x8000, CRC(4fddd061) SHA1(ff0aa18605612f6102107a6be1f93ae4c5edc84f) )

	ROM_REGION( 0x300000 * 2, "gfx1", 0 )   /* x2, do not dispose, zooming sprites */
	ROM_LOAD16_WORD_SWAP( "763e17", 0x000000, 0x080000, CRC(8db4e0aa) SHA1(376cb3cae110998f2f9df7e6cdd35c06732fea69) )
	ROM_LOAD16_WORD_SWAP( "763e20", 0x080000, 0x080000, CRC(a22c6fce) SHA1(174fb9c1706c092947bcce386831acd33a237046) )
	ROM_LOAD16_WORD_SWAP( "763e18", 0x100000, 0x080000, CRC(50920d01) SHA1(313c7ecbd154b3f4c96f25c29a7734a9b3facea4) )
	ROM_LOAD16_WORD_SWAP( "763e21", 0x180000, 0x080000, CRC(77e0e93e) SHA1(c8e415438a1f5ad79b10fd3ad5cb22de0d562e5d) )
	ROM_LOAD16_WORD_SWAP( "763e19", 0x200000, 0x080000, CRC(a2622e56) SHA1(0a0ed9713882b987518e6f06a02dba417c1f4f32) )
	ROM_LOAD16_WORD_SWAP( "763e22", 0x280000, 0x080000, CRC(967c49d1) SHA1(01979d216a9fd8085298445ac5f7870d1598db74) )

	ROM_REGION( 0x20000, "k051316_1", 0 )    /* bg */
	ROM_LOAD( "763e14", 0x000000, 0x020000, CRC(60392aa1) SHA1(8499eb40a246587e24f6fd00af2eaa6d75ee6363) )

	ROM_REGION( 0x08000, "k051316_2", 0 )    /* fg */
	/* first half empty - PCB silkscreen reads "27256/27512" */
	ROM_LOAD( "763a13", 0x000000, 0x008000, CRC(8bed8e0d) SHA1(ccff330abc23fe499e76c16cab5783c3daf155dd) )
	ROM_CONTINUE( 0x000000, 0x008000 )

	ROM_REGION( 0x20000, "gfx4", 0 )    /* road */
	ROM_LOAD( "763e15", 0x000000, 0x020000, CRC(7110aa43) SHA1(639dc002cc1580f0530bb5bb17f574e2258d5954) )

	ROM_REGION( 0x40000, "k007232_1", 0 ) /* Samples, 2 banks */
	ROM_LOAD( "763e11", 0x000000, 0x040000, CRC(9d99a5a7) SHA1(96e37bbb259e0a91d124c26b6b1a9b70de2e19a4) )

	ROM_REGION( 0x40000, "k007232_2", 0 ) /* Samples, 2 banks */
	ROM_LOAD( "763e10", 0x000000, 0x040000, CRC(ca409210) SHA1(703d7619c4bd33d2ff5fad127d98c82906fede33) )

	ROM_REGION( 0x100000, "k007232_3", 0 )    /* Samples, 4 banks for each ROM */
	ROM_LOAD( "763e08", 0x000000, 0x080000, CRC(054a9a63) SHA1(45d7926c9e7af47c041ba9b733e334bccd730a6d) )
	ROM_LOAD( "763e09", 0x080000, 0x080000, CRC(c39857db) SHA1(64b135a9ccf9e1dd50789cdd5c6bc03da8decfd0) )

	ROM_REGION( 0x08000, "user1", 0 )   /* extra data for road effects? */
	ROM_LOAD( "763a12", 0x000000, 0x008000, CRC(05f1e553) SHA1(8aaeb7374bd93038c24e6470398936f22cabb0fe) )
ROM_END

/*

Hot Chase
Konami 1988



         E08D E08B    E09D E09B  E10D E10B
         E08A E08C    E09A E09C  E10A E10C

GX763 350861

               E09      E10        E11
               E08      07232      07232
               07232   3.579MHz         2128
                               6809     P01.R10
      SW1
      SW2                               2128  2128
               6264 6264                6264  6264
                                        R02.P14 R03.R14
                            07770       R04.P16 R05.R16
      2018-45  D06.E18 D07.H18   10MHz
      2018-45  68000-10         07641   68000-10

GX763 350860

 051316 PSAC    051316 PSAC  A13.H5 A14.J5
                                                2018-45 2018-R6
                    2018-45
                    2018-45                     07558
 051316 PSAC        2018-45                            A12.R13

                             A15.H14

    A23.B17                            07634
                                                     07635
               2018-45 2018-45
               2018-45 2018-45         07557
               2018-45 2018-45                       25.2MHz
               2018-45 2018-45


Left EPROM board

                                   E19A.A8 E19B.A7 E19C.A6 E19D.A5
E22E.B12 E22F.B11 E22G.B10 E22H.B9 E19E.B8 E19F.B7 E19G.B6 E19H.B5
                                   E22A.D9 E22B.D7 E22C.D6 E22D.D5

Right EPROM board

E21E E21F E21G E21H E17A E17B E17C E17D E18A E18B E18C E18D
E20E E20F E20G E20H E17E E17F E17G E17H E18E E18F E18G E18H
                    E20A E20B E20C E20D E21A E21B E21C E21D

*/

// uses EPROM sub-boards in place of some of the MASK roms, different program too
ROM_START( hotchasea )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* Main Code */
	ROM_LOAD16_BYTE( "763r05.r16", 0x000000, 0x010000, CRC(c880d5e4) SHA1(3c3ab3ad5496cfbc8de76620eedc06601ee7a8c7) )
	ROM_LOAD16_BYTE( "763r04.p16", 0x000001, 0x010000, CRC(b732ee2c) SHA1(b3d73cf5039980ac74927eef656326515fd2026b) )
	ROM_LOAD16_BYTE( "763r03.r14", 0x020000, 0x010000, CRC(13dd71de) SHA1(4b86b81ef79e0e92a1e458010b0b9574183a9c29) )
	ROM_LOAD16_BYTE( "763r02.p14", 0x020001, 0x010000, CRC(6cd1a18e) SHA1(0ddfe6a46e95052534325f228b7f0faba121950e) )

	ROM_REGION( 0x20000, "sub", 0 ) /* Sub Code */
	ROM_LOAD16_BYTE( "763d07.h18", 0x000000, 0x010000, CRC(ae12fa90) SHA1(7f76f09916fe152411b5af3c504ee7be07497ef4) )
	ROM_LOAD16_BYTE( "763d06.e18", 0x000001, 0x010000, CRC(b77e0c07) SHA1(98bf492ac889d31419df706029fdf3d51b85c936) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound Code */
	ROM_LOAD( "763p01.r10", 0x8000, 0x8000, CRC(15dbca7b) SHA1(ac0c965b72a8579a3b60dbadfb942248d2cff2d8) )

	ROM_REGION( 0x300000 * 2, "gfx1", 0 )   /* x2, do not dispose, zooming sprites */
	ROM_LOAD16_BYTE( "763e17a", 0x000000, 0x010000, CRC(8542d7d7) SHA1(a7c8aa7d8e0cabdc5269eb7adff944aaa0f819b6) )
	ROM_LOAD16_BYTE( "763e17e", 0x000001, 0x010000, CRC(4b4d919c) SHA1(0364eb74da8db7238888274d12011de876662d5a) )
	ROM_LOAD16_BYTE( "763e17b", 0x020000, 0x010000, CRC(ba9d7e72) SHA1(3af618087dcc66552ffabaf655f97b20e597122c) )
	ROM_LOAD16_BYTE( "763e17f", 0x020001, 0x010000, CRC(582400bb) SHA1(9479e45087d908c9b20392dba2a752a7ec1482e2) )
	ROM_LOAD16_BYTE( "763e17c", 0x040000, 0x010000, CRC(0ed292f8) SHA1(8c161e73c7f27925377799f67585b888bade6d82) )
	ROM_LOAD16_BYTE( "763e17g", 0x040001, 0x010000, CRC(35b27ed7) SHA1(e17e7674ee210ff340482a16dce3439b55c29f72) )
	ROM_LOAD16_BYTE( "763e17d", 0x060000, 0x010000, CRC(0166d00d) SHA1(e58f6deabc5743f6610252242f97bd5e973316ae) )
	ROM_LOAD16_BYTE( "763e17h", 0x060001, 0x010000, CRC(e5b8e8e6) SHA1(ae1349977559ff24dcd1678d6fd3a3e118612d07) )
	ROM_LOAD16_BYTE( "763e20a", 0x080000, 0x010000, CRC(256fe63c) SHA1(414325f2ff9abc411e2401dddd216e1a4de8a01e) )
	ROM_LOAD16_BYTE( "763e20e", 0x080001, 0x010000, CRC(ee8ca7e1) SHA1(fee544d6508f4106176f39e3765961e9f80fe620) )
	ROM_LOAD16_BYTE( "763e20b", 0x0a0000, 0x010000, CRC(b6714c24) SHA1(88f6437e181f36b7e44f1c70872314d8c0cc30e7) )
	ROM_LOAD16_BYTE( "763e20f", 0x0a0001, 0x010000, CRC(9dbc4b21) SHA1(31559903707a4f8ba3b044e8aad928de38403dcf) )
	ROM_LOAD16_BYTE( "763e20c", 0x0c0000, 0x010000, CRC(5173ad9b) SHA1(afe82c69f7036c7595f1a56b22176ba202b00b5c) )
	ROM_LOAD16_BYTE( "763e20g", 0x0c0001, 0x010000, CRC(b8c77f99) SHA1(e3bea1481c5b1c4733130651f9cf18587d3efc46) )
	ROM_LOAD16_BYTE( "763e20d", 0x0e0000, 0x010000, CRC(4ebdba32) SHA1(ac7daa291c82f75b09faf7bc5f6257870cc46061) )
	ROM_LOAD16_BYTE( "763e20h", 0x0e0001, 0x010000, CRC(0a428654) SHA1(551026f6f57d38aedd3498cce33af7bb2cf07184) )
	ROM_LOAD16_BYTE( "763e18a", 0x100000, 0x010000, CRC(09748099) SHA1(1821e2067b9a50a0638c8d105c617f4030d61877) )
	ROM_LOAD16_BYTE( "763e18e", 0x100001, 0x010000, CRC(049d4fcf) SHA1(aed18297677a3bb0b7197f59ea329aef9b678c01) )
	ROM_LOAD16_BYTE( "763e18b", 0x120000, 0x010000, CRC(ed0c3369) SHA1(84f336546dee01fec31c9c256ee00a9f8448cea4) )
	ROM_LOAD16_BYTE( "763e18f", 0x120001, 0x010000, CRC(b596a9ce) SHA1(dea0fe1c3386b5f0d19df4467f42d077678ae220) )
	ROM_LOAD16_BYTE( "763e18c", 0x140000, 0x010000, CRC(5a278291) SHA1(05c529baa68ef5877a28901c6f221e3d3593735f) )
	ROM_LOAD16_BYTE( "763e18g", 0x140001, 0x010000, CRC(aa7263cd) SHA1(b2acf66c02faf7777c5cb947aaf8e038f29c0f2e) )
	ROM_LOAD16_BYTE( "763e18d", 0x160000, 0x010000, CRC(b0b79a71) SHA1(46d0f17b7a6e4fb94ac9b8335bc598339d7707a5) )
	ROM_LOAD16_BYTE( "763e18h", 0x160001, 0x010000, CRC(a18b9127) SHA1(890971d2922a59ff4beea00238e710c8d3e0f19d) )
	ROM_LOAD16_BYTE( "763e21a", 0x180000, 0x010000, CRC(60788c29) SHA1(4faaa192d07f6acac0e7d11676146ecd0e71541f) )
	ROM_LOAD16_BYTE( "763e21e", 0x180001, 0x010000, CRC(844799ff) SHA1(8dc3ae3bb30ecb4e921a5b2068d3cd9421577844) )
	ROM_LOAD16_BYTE( "763e21b", 0x1a0000, 0x010000, CRC(1eefed61) SHA1(9c09dbff073d63384bf1ec9df4db4833afa33826) )
	ROM_LOAD16_BYTE( "763e21f", 0x1a0001, 0x010000, CRC(3aacfb10) SHA1(fb3eebf1f0850ed2f8f02cd4b5b564524e391afd) )
	ROM_LOAD16_BYTE( "763e21c", 0x1c0000, 0x010000, CRC(97e48b37) SHA1(864c73f48d839c2afeecec99605be6111f450ddd) )
	ROM_LOAD16_BYTE( "763e21g", 0x1c0001, 0x010000, CRC(74fefb12) SHA1(7746918c3ea8981c9cb2ead79a252939ba8bde3f) )
	ROM_LOAD16_BYTE( "763e21d", 0x1e0000, 0x010000, CRC(dd41569e) SHA1(065ee2de9ad6980788807cb563feccef1c3d1b9d) )
	ROM_LOAD16_BYTE( "763e21h", 0x1e0001, 0x010000, CRC(7ea52bf6) SHA1(2be93f88ccdea989b05beca13ebbfb77626ea41f) )
	ROM_LOAD16_BYTE( "763e19a", 0x200000, 0x010000, CRC(8c912c46) SHA1(e314edc39c32471c6fa2969e7c5c771c19fda88c) )
	ROM_LOAD16_BYTE( "763e19e", 0x200001, 0x010000, CRC(0eb34787) SHA1(9b8145dae210a177585e672fce30339b39c3c0f3) )
	ROM_LOAD16_BYTE( "763e19b", 0x220000, 0x010000, CRC(79960729) SHA1(f5c20ed7683aad8a435c292fbd5a1acc2a97ecee) )
	ROM_LOAD16_BYTE( "763e19f", 0x220001, 0x010000, CRC(1764ec5f) SHA1(4f7a0a3667087523a1ccdfc2d0e54a520f1216b3) )
	ROM_LOAD16_BYTE( "763e19c", 0x240000, 0x010000, CRC(f13377ac) SHA1(89f8d730cb457cc9cf55049b7002514302b2b04f) )
	ROM_LOAD16_BYTE( "763e19g", 0x240001, 0x010000, CRC(f2102e89) SHA1(41ff5d8904618a77c7c3c78c52c6f1b9c5a318fd) )
	ROM_LOAD16_BYTE( "763e19d", 0x260000, 0x010000, CRC(0b2a19f4) SHA1(3689b2c1f6227224fbcecc0d2470048a99510794) )
	ROM_LOAD16_BYTE( "763e19h", 0x260001, 0x010000, CRC(cd6d08a5) SHA1(ce13a8bba84f24e7d1fb25254e2e95f591fe1d67) )
	ROM_LOAD16_BYTE( "763e22a", 0x280000, 0x010000, CRC(16eec250) SHA1(f50375f207575e9d280285aca493902afbb7d729) )
	ROM_LOAD16_BYTE( "763e22e", 0x280001, 0x010000, CRC(c184b1c0) SHA1(d765e6eb2631b77dff5331840ac2a99cf1250362) )
	ROM_LOAD16_BYTE( "763e22b", 0x2a0000, 0x010000, CRC(1afe4b0c) SHA1(ce5a855291b443c1e16dbf54730960600c754fee) )
	ROM_LOAD16_BYTE( "763e22f", 0x2a0001, 0x010000, CRC(61f27c98) SHA1(d80af1a3e424c8dbab4fd21d494a0580ab96cd8d) )
	ROM_LOAD16_BYTE( "763e22c", 0x2c0000, 0x010000, CRC(c19b4b63) SHA1(93708b8769c44d5b93dcd2928a0d1120cc52c6ee) )
	ROM_LOAD16_BYTE( "763e22g", 0x2c0001, 0x010000, CRC(5bcbaf29) SHA1(621aa19606a15abb1539c07a033b32fc374a2d6a) )
	ROM_LOAD16_BYTE( "763e22d", 0x2e0000, 0x010000, CRC(fd5b669d) SHA1(fd5d82886708187e53c204c574ee252fc8a793af) )
	ROM_LOAD16_BYTE( "763e22h", 0x2e0001, 0x010000, CRC(9a9f45d8) SHA1(24fa9425b00441fff124eae7b40df7e65c920323) )

	ROM_REGION( 0x20000, "k051316_1", 0 )    /* bg */
	ROM_LOAD( "763a14", 0x000000, 0x020000, CRC(60392aa1) SHA1(8499eb40a246587e24f6fd00af2eaa6d75ee6363) )

	ROM_REGION( 0x08000, "k051316_2", 0 )    /* fg */
	/* first half empty - PCB silkscreen reads "27256/27512" */
	ROM_LOAD( "763a13", 0x000000, 0x008000, CRC(8bed8e0d) SHA1(ccff330abc23fe499e76c16cab5783c3daf155dd) )
	ROM_CONTINUE( 0x000000, 0x008000 )

	ROM_REGION( 0x20000, "gfx4", 0 )    /* road */
	ROM_LOAD( "763a15", 0x000000, 0x020000, CRC(7110aa43) SHA1(639dc002cc1580f0530bb5bb17f574e2258d5954) )

	ROM_REGION( 0x40000, "k007232_1", 0 ) /* Samples, 2 banks */
	ROM_LOAD( "763e11a", 0x000000, 0x010000, CRC(a60a93c8) SHA1(ce319f2b30c82f66fee0bab65d091ef4bef58a89) )
	ROM_LOAD( "763e11b", 0x010000, 0x010000, CRC(7750feb5) SHA1(e0900b8af400a50a22907ffa514847003cef342d) )
	ROM_LOAD( "763e11c", 0x020000, 0x010000, CRC(78b89bf8) SHA1(b74427e363a486d4be003df39f4ca8d10b9d5fc9) )
	ROM_LOAD( "763e11d", 0x030000, 0x010000, CRC(5f38d054) SHA1(ce0c87a7b7c0806e09cce5d48a651f12f790dd27) )

	ROM_REGION( 0x40000, "k007232_2", 0 ) /* Samples, 2 banks */
	ROM_LOAD( "763e10a", 0x000000, 0x010000, CRC(2b1cbefc) SHA1(f23fb943c277a05f2aa4d25de692d1fb3bb752ac) )
	ROM_LOAD( "763e10b", 0x010000, 0x010000, CRC(8209c950) SHA1(944c2afb4cfc67bd243de499f5ca6a7982980f45) )
	ROM_LOAD( "763e10c", 0x020000, 0x010000, CRC(b91d6c07) SHA1(ef90457cb495750c5793cd1716d0dc44d33d0a95) )
	ROM_LOAD( "763e10d", 0x030000, 0x010000, CRC(5b465d20) SHA1(66f10b58873e738f5539b960468c7f92d07c28bc) )

	ROM_REGION( 0x100000, "k007232_3", 0 )    /* Samples, 4 banks for each ROM */
	ROM_LOAD( "763e08a", 0x000000, 0x020000, CRC(02e4e7ef) SHA1(1622e4d85a333acae6e5f9304a037389bfeb71dc) )
	ROM_LOAD( "763e08b", 0x020000, 0x020000, CRC(94edde2f) SHA1(b124f83f271dab710d5ecb67a70cac7b4b41931c) )
	ROM_LOAD( "763e08c", 0x040000, 0x020000, CRC(b1ab1529) SHA1(962ad45fdccf6431e05eaec65d1b2f7842bddc02) )
	ROM_LOAD( "763e08d", 0x060000, 0x020000, CRC(ee8d14db) SHA1(098ba4f27b8cbb0ce017b28e5b69d6a3d2efa1df) )

	ROM_LOAD( "763e09a", 0x080000, 0x020000, CRC(1e6628ec) SHA1(9d24da1d32cb39dcbe3d0633045054d398ca8eb8) )
	ROM_LOAD( "763e09b", 0x0a0000, 0x020000, CRC(f0c2feb8) SHA1(9454d45a97dc2e823baf68dce85acce8e82a18f2) )
	ROM_LOAD( "763e09c", 0x0c0000, 0x020000, CRC(a0ade3e4) SHA1(1c94cede76f9350769a589625fadaee855c38866) )
	ROM_LOAD( "763e09d", 0x0e0000, 0x020000, CRC(c74e484d) SHA1(dd7ef64c30443847c638291f6cd2b45a5f0b2310) )

	ROM_REGION( 0x08000, "user1", 0 )   /* extra data for road effects? */
	ROM_LOAD( "763a12", 0x000000, 0x008000, CRC(05f1e553) SHA1(8aaeb7374bd93038c24e6470398936f22cabb0fe) )

	ROM_REGION( 0x200, "user2", 0 )
	ROM_LOAD( "763a23.b17", 0x00000, 0x200, CRC(81c30352) SHA1(20700aed065929835ef5c3b564a6f531f0a8fedf) )
ROM_END


/*      Important: you must leave extra space when listing sprite ROMs
    in a ROM module definition.  This routine unpacks each sprite nibble
    into a byte, doubling the memory consumption. */

void wecleman_state::hotchase_sprite_decode( int num16_banks, int bank_size )
{
	UINT8 *base;
	int i;

	base = memregion("gfx1")->base(); // sprites
	dynamic_buffer temp( bank_size );

	for( i = num16_banks; i >0; i-- ){
		UINT8 *finish   = base + 2*bank_size*i;
		UINT8 *dest     = finish - 2*bank_size;

		UINT8 *p1 = &temp[0];
		UINT8 *p2 = &temp[bank_size/2];

		UINT8 data;

		memcpy (&temp[0], base+bank_size*(i-1), bank_size);

		do {
			data = *p1++;
			if( (data&0xf0) == 0xf0 ) data &= 0x0f;
			if( (data&0x0f) == 0x0f ) data &= 0xf0;
			*dest++ = data >> 4;
			*dest++ = data & 0xF;
			data = *p1++;
			if( (data&0xf0) == 0xf0 ) data &= 0x0f;
			if( (data&0x0f) == 0x0f ) data &= 0xf0;
			*dest++ = data >> 4;
			*dest++ = data & 0xF;


			data = *p2++;
			if( (data&0xf0) == 0xf0 ) data &= 0x0f;
			if( (data&0x0f) == 0x0f ) data &= 0xf0;
			*dest++ = data >> 4;
			*dest++ = data & 0xF;
			data = *p2++;
			if( (data&0xf0) == 0xf0 ) data &= 0x0f;
			if( (data&0x0f) == 0x0f ) data &= 0xf0;
			*dest++ = data >> 4;
			*dest++ = data & 0xF;
		} while( dest<finish );
	}
}

/* Unpack sprites data and do some patching */
DRIVER_INIT_MEMBER(wecleman_state,hotchase)
{
//  UINT16 *RAM1 = (UINT16) memregion("maincpu")->base(); /* Main CPU patches */
//  RAM[0x1140/2] = 0x0015; RAM[0x195c/2] = 0x601A; // faster self test

	/* Now we can unpack each nibble of the sprites into a pixel (one byte) */
	hotchase_sprite_decode(3,0x80000*2);  // num banks, bank len

	m_spr_color_offs = 0;
	m_sound_hw_type = 1;
}


/***************************************************************************
                                Game driver(s)
***************************************************************************/

GAMEL( 1986, wecleman,  0,        wecleman, wecleman, wecleman_state, wecleman, ROT0, "Konami", "WEC Le Mans 24 (v2.00, set 1)", 0, layout_wecleman )
GAMEL( 1986, weclemana, wecleman, wecleman, wecleman, wecleman_state, wecleman, ROT0, "Konami", "WEC Le Mans 24 (v2.00, set 2)", 0, layout_wecleman ) // 1988 release (maybe date hacked?)
GAMEL( 1986, weclemanb, wecleman, wecleman, wecleman, wecleman_state, wecleman, ROT0, "Konami", "WEC Le Mans 24 (v1.26)", 0, layout_wecleman )
// a version 1.21 is known to exist too, see https://www.youtube.com/watch?v=4l8vYJi1OeU

GAMEL( 1988, hotchase,  0,        hotchase, hotchase, wecleman_state, hotchase, ROT0, "Konami", "Hot Chase (set 1)", 0, layout_wecleman )
GAMEL( 1988, hotchasea, hotchase, hotchase, hotchase, wecleman_state, hotchase, ROT0, "Konami", "Hot Chase (set 2)", 0, layout_wecleman )
