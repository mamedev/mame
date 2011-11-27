/***************************************************************************
                        WEC Le Mans 24  &   Hot Chase

                          (C)   1986 & 1988 Konami

                    driver by       Luca Elia (l.elia@tin.it)

- Note: press F2 to enter service mode -

---------------------------------------------------------------------------
                                TODO list
---------------------------------------------------------------------------
WEC Le Mans 24:
- The parallactic scrolling is sometimes wrong
Hot Chase:
- gameplay speed is VERY erratic
- Samples pitch is too low
- No zoom and rotation of the layers
Common Issues:
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
#include "video/konicdev.h"
#include "cpu/m6809/m6809.h"
#include "sound/2151intf.h"
#include "sound/k007232.h"
#include "wecleman.lh"
#include "includes/wecleman.h"


/***************************************************************************
                            Common Routines
***************************************************************************/

static READ16_HANDLER( wecleman_protection_r )
{
	wecleman_state *state = space->machine().driver_data<wecleman_state>();
	int blend, data0, data1, r0, g0, b0, r1, g1, b1;

	data0 = state->m_protection_ram[0];
	blend = state->m_protection_ram[2];
	data1 = state->m_protection_ram[1];
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

static WRITE16_HANDLER( wecleman_protection_w )
{
	wecleman_state *state = space->machine().driver_data<wecleman_state>();
	if (offset == 2) state->m_prot_state = data & 0x2000;
	if (!state->m_prot_state) COMBINE_DATA(state->m_protection_ram + offset);
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
static WRITE16_HANDLER( irqctrl_w )
{
	wecleman_state *state = space->machine().driver_data<wecleman_state>();
	if (ACCESSING_BITS_0_7)
	{
		// logerror("CPU #0 - PC = %06X - $140005 <- %02X (old value: %02X)\n",cpu_get_pc(&space->device()), data&0xFF, old_data&0xFF);

		// Bit 0 : SUBINT
		if ( (state->m_irqctrl & 1) && (!(data & 1)) )	// 1->0 transition
			cputag_set_input_line(space->machine(), "sub", 4, HOLD_LINE);

		// Bit 1 : NSUBRST
		if (data & 2)
			cputag_set_input_line(space->machine(), "sub", INPUT_LINE_RESET, CLEAR_LINE);
		else
			cputag_set_input_line(space->machine(), "sub", INPUT_LINE_RESET, ASSERT_LINE);

		// Bit 2 : SOUND-ON
		// Bit 3 : SOUNDRST
		// Bit 4 : SCR-HCNT
		// Bit 5 : SCR-VCNT
		// Bit 6 : TV-KILL
		state->m_irqctrl = data;	// latch the value
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
static WRITE16_HANDLER( selected_ip_w )
{
	wecleman_state *state = space->machine().driver_data<wecleman_state>();
	if (ACCESSING_BITS_0_7) state->m_selected_ip = data & 0xff;	// latch the value
}

/* $140021.b - Return the previously selected input port's value */
static READ16_HANDLER( selected_ip_r )
{
	wecleman_state *state = space->machine().driver_data<wecleman_state>();
	switch ( (state->m_selected_ip >> 5) & 3 )
	{																	// From WEC Le Mans Schems:
		case 0:  return input_port_read(space->machine(), "ACCEL");		// Accel - Schems: Accelevr
		case 1:  return ~0;												// ????? - Schems: Not Used
		case 2:  return input_port_read(space->machine(), "STEER");		// Wheel - Schems: Handlevr
		case 3:  return ~0;												// Table - Schems: Turnvr

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
static WRITE16_HANDLER( blitter_w )
{
	wecleman_state *state = space->machine().driver_data<wecleman_state>();
	COMBINE_DATA(&state->m_blitter_regs[offset]);

	/* do a blit if $80010.b has been written */
	if ( (offset == 0x10/2) && (ACCESSING_BITS_8_15) )
	{
		/* 80000.b = ?? usually 0 - other values: 02 ; 00 - ? logic function ? */
		/* 80001.b = ?? usually 0 - other values: 3f ; 01 - ? height ? */
		int minterm  = ( state->m_blitter_regs[0x0/2] & 0xFF00 ) >> 8;
		int list_len = ( state->m_blitter_regs[0x0/2] & 0x00FF ) >> 0;

		/* 80002.w = ?? always 0 - ? increment per horizontal line ? */
		/* no proof at all, it's always 0 */
		//int srcdisp = state->m_blitter_regs[0x2/2] & 0xFF00;
		//int destdisp = state->m_blitter_regs[0x2/2] & 0x00FF;

		/* 80004.l = source data address */
		int src  = ( state->m_blitter_regs[0x4/2] << 16 ) + state->m_blitter_regs[0x6/2];

		/* 80008.l = list of blits address */
		int list = ( state->m_blitter_regs[0x8/2] << 16 ) + state->m_blitter_regs[0xA/2];

		/* 8000C.l = destination address */
		int dest = ( state->m_blitter_regs[0xC/2] << 16 ) + state->m_blitter_regs[0xE/2];

		/* 80010.b = number of words to move */
		int size = ( state->m_blitter_regs[0x10/2] ) & 0x00FF;

		/* Word aligned transfers only ?? */
		src  &= (~1);   list &= (~1);    dest &= (~1);

		/* Two minterms / blit modes are used */
		if (minterm != 2)
		{
			/* One single blit */
			for ( ; size > 0 ; size--)
			{
				/* maybe slower than a memcpy but safer (and errors are logged) */
				space->write_word(dest, space->read_word(src));
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
				i = src + space->read_word(list+2);
				j = i + (size<<1);
				destptr = dest;

				for (; i<j; destptr+=2, i+=2)
					space->write_word(destptr, space->read_word(i));

				destptr = dest + 14;
				i = space->read_word(list) + state->m_spr_color_offs;
				space->write_word(destptr, i);

				dest += 16;
				list += 4;
			}

			/* hack for the blit to Sprites RAM - Sprite list end-marker */
			space->write_word(dest, 0xFFFF);
		}
	}
}


/***************************************************************************
                    WEC Le Mans 24 Main CPU Handlers
***************************************************************************/

static WRITE16_HANDLER( wecleman_soundlatch_w );

static ADDRESS_MAP_START( wecleman_map, AS_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM	// ROM (03c000-03ffff used as RAM sometimes!)
	AM_RANGE(0x040494, 0x040495) AM_WRITE(wecleman_videostatus_w) AM_BASE_MEMBER(wecleman_state, m_videostatus)	// cloud blending control (HACK)
	AM_RANGE(0x040000, 0x043fff) AM_RAM	// RAM
	AM_RANGE(0x060000, 0x060005) AM_WRITE(wecleman_protection_w) AM_BASE_MEMBER(wecleman_state, m_protection_ram)
	AM_RANGE(0x060006, 0x060007) AM_READ(wecleman_protection_r)	// MCU read
	AM_RANGE(0x080000, 0x080011) AM_RAM_WRITE(blitter_w) AM_BASE_MEMBER(wecleman_state, m_blitter_regs)	// Blitter
	AM_RANGE(0x100000, 0x103fff) AM_RAM_WRITE(wecleman_pageram_w) AM_BASE_MEMBER(wecleman_state, m_pageram)	// Background Layers
	AM_RANGE(0x108000, 0x108fff) AM_RAM_WRITE(wecleman_txtram_w) AM_BASE_MEMBER(wecleman_state, m_txtram)	// Text Layer
	AM_RANGE(0x110000, 0x110fff) AM_RAM_WRITE(wecleman_paletteram16_SSSSBBBBGGGGRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x124000, 0x127fff) AM_RAM AM_SHARE("share1")	// Shared with main CPU
	AM_RANGE(0x130000, 0x130fff) AM_RAM AM_BASE_MEMBER(wecleman_state, m_spriteram)	// Sprites
	AM_RANGE(0x140000, 0x140001) AM_WRITE(wecleman_soundlatch_w)	// To sound CPU
	AM_RANGE(0x140002, 0x140003) AM_WRITE(selected_ip_w)	// Selects accelerator / wheel / ..
	AM_RANGE(0x140004, 0x140005) AM_WRITE(irqctrl_w)	// Main CPU controls the other CPUs
	AM_RANGE(0x140006, 0x140007) AM_WRITENOP	// Watchdog reset
	AM_RANGE(0x140010, 0x140011) AM_READ_PORT("IN0")	// Coins + brake + gear
	AM_RANGE(0x140012, 0x140013) AM_READ_PORT("IN1")	// ??
	AM_RANGE(0x140014, 0x140015) AM_READ_PORT("DSWA")	// DSW 2
	AM_RANGE(0x140016, 0x140017) AM_READ_PORT("DSWB")	// DSW 1
	AM_RANGE(0x140020, 0x140021) AM_WRITEONLY	// Paired with writes to $140003
	AM_RANGE(0x140020, 0x140021) AM_READ(selected_ip_r)	// Accelerator or Wheel or ..
	AM_RANGE(0x140030, 0x140031) AM_WRITENOP	// toggles between 0 & 1 on hitting bumps and crashes (vibration?)
ADDRESS_MAP_END


/***************************************************************************
                        Hot Chase Main CPU Handlers
***************************************************************************/

static WRITE16_HANDLER( hotchase_soundlatch_w );

static ADDRESS_MAP_START( hotchase_map, AS_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x040000, 0x041fff) AM_RAM									// RAM
	AM_RANGE(0x060000, 0x063fff) AM_RAM									// RAM
	AM_RANGE(0x080000, 0x080011) AM_RAM_WRITE(blitter_w) AM_BASE_MEMBER(wecleman_state, m_blitter_regs)	// Blitter
	AM_RANGE(0x100000, 0x100fff) AM_DEVREADWRITE8("k051316_1", k051316_r, k051316_w, 0x00ff)	// Background
	AM_RANGE(0x101000, 0x10101f) AM_DEVWRITE8("k051316_1", k051316_ctrl_w, 0x00ff)	// Background Ctrl
	AM_RANGE(0x102000, 0x102fff) AM_DEVREADWRITE8("k051316_2", k051316_r, k051316_w, 0x00ff)	// Foreground
	AM_RANGE(0x103000, 0x10301f) AM_DEVWRITE8("k051316_2", k051316_ctrl_w, 0x00ff)	// Foreground Ctrl
	AM_RANGE(0x110000, 0x111fff) AM_RAM_WRITE(hotchase_paletteram16_SBGRBBBBGGGGRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x120000, 0x123fff) AM_RAM AM_SHARE("share1")					// Shared with sub CPU
	AM_RANGE(0x130000, 0x130fff) AM_RAM AM_BASE_MEMBER(wecleman_state, m_spriteram)	// Sprites
	AM_RANGE(0x140000, 0x140001) AM_WRITE(hotchase_soundlatch_w)	// To sound CPU
	AM_RANGE(0x140002, 0x140003) AM_WRITE(selected_ip_w)	// Selects accelerator / wheel /
	AM_RANGE(0x140004, 0x140005) AM_WRITE(irqctrl_w)	// Main CPU controls the other CPUs
	AM_RANGE(0x140006, 0x140007) AM_READNOP	// Watchdog reset
	AM_RANGE(0x140010, 0x140011) AM_READ_PORT("IN0")	// Coins + brake + gear
	AM_RANGE(0x140012, 0x140013) AM_READ_PORT("IN1")	// ?? bit 4 from sound cpu
	AM_RANGE(0x140014, 0x140015) AM_READ_PORT("DSW2")	// DSW 2
	AM_RANGE(0x140016, 0x140017) AM_READ_PORT("DSW1")	// DSW 1
	AM_RANGE(0x140020, 0x140021) AM_READ(selected_ip_r) AM_WRITENOP	// Paired with writes to $140003
	AM_RANGE(0x140022, 0x140023) AM_READNOP	// ??
	AM_RANGE(0x140030, 0x140031) AM_WRITENOP	// signal to cabinet vibration motors?
ADDRESS_MAP_END


/***************************************************************************
                    WEC Le Mans 24 Sub CPU Handlers
***************************************************************************/

static ADDRESS_MAP_START( wecleman_sub_map, AS_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM	// ROM
	AM_RANGE(0x060000, 0x060fff) AM_RAM AM_BASE_MEMBER(wecleman_state, m_roadram) AM_SIZE_MEMBER(wecleman_state, m_roadram_size)	// Road
	AM_RANGE(0x070000, 0x073fff) AM_RAM AM_SHARE("share1")	// RAM (Shared with main CPU)
ADDRESS_MAP_END


/***************************************************************************
                        Hot Chase Sub CPU Handlers
***************************************************************************/

static ADDRESS_MAP_START( hotchase_sub_map, AS_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM	// ROM
	AM_RANGE(0x020000, 0x020fff) AM_RAM AM_BASE_MEMBER(wecleman_state, m_roadram) AM_SIZE_MEMBER(wecleman_state, m_roadram_size)	// Road
	AM_RANGE(0x040000, 0x043fff) AM_RAM AM_SHARE("share1") // Shared with main CPU
	AM_RANGE(0x060000, 0x060fff) AM_RAM // a table, presumably road related
	AM_RANGE(0x061000, 0x06101f) AM_RAM // road vregs?
ADDRESS_MAP_END


/***************************************************************************
                    WEC Le Mans 24 Sound CPU Handlers
***************************************************************************/

/* 140001.b */
WRITE16_HANDLER( wecleman_soundlatch_w )
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_w(space, 0, data & 0xFF);
		cputag_set_input_line(space->machine(), "audiocpu", 0, HOLD_LINE);
	}
}

/* Protection - an external multiplyer connected to the sound CPU */
static READ8_HANDLER( multiply_r )
{
	wecleman_state *state = space->machine().driver_data<wecleman_state>();
	return (state->m_multiply_reg[0] * state->m_multiply_reg[1]) & 0xFF;
}

static WRITE8_HANDLER( multiply_w )
{
	wecleman_state *state = space->machine().driver_data<wecleman_state>();
	state->m_multiply_reg[offset] = data;
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

static WRITE8_DEVICE_HANDLER( wecleman_K00723216_bank_w )
{
	k007232_set_bank(device, 0, ~data&1 );	//* (wecleman062gre)
}

static ADDRESS_MAP_START( wecleman_sound_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM
	AM_RANGE(0x8500, 0x8500) AM_WRITENOP	// incresed with speed (global volume)?
	AM_RANGE(0x9000, 0x9000) AM_READ(multiply_r)	// Protection
	AM_RANGE(0x9000, 0x9001) AM_WRITE(multiply_w)	// Protection
	AM_RANGE(0x9006, 0x9006) AM_WRITENOP	// ?
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_r)	// From main CPU
	AM_RANGE(0xb000, 0xb00d) AM_DEVREADWRITE("konami", k007232_r, k007232_w)	// K007232 (Reading offset 5/b triggers the sample)
	AM_RANGE(0xc000, 0xc001) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0xf000, 0xf000) AM_DEVWRITE("konami", wecleman_K00723216_bank_w)	// Samples banking
ADDRESS_MAP_END


/***************************************************************************
                        Hot Chase Sound CPU Handlers
***************************************************************************/

/* 140001.b */
static WRITE16_HANDLER( hotchase_soundlatch_w )
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_w(space, 0, data & 0xFF);
		cputag_set_input_line(space->machine(), "audiocpu", M6809_IRQ_LINE, HOLD_LINE);
	}
}

static WRITE8_HANDLER( hotchase_sound_control_w )
{
	device_t *sound[3];

//  int reg[8];

	sound[0] = space->machine().device("konami1");
	sound[1] = space->machine().device("konami2");
	sound[2] = space->machine().device("konami3");

//  reg[offset] = data;

	switch (offset)
	{
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
			/* change volume
                offset 00000xxx----- channel select (0:channel 0, 1:channel 1)
                ++------ chip select ( 0:chip 1, 1:chip2, 2:chip3)
                data&0x0f left volume  (data>>4)&0x0f right volume
            */
		  k007232_set_volume( sound[offset>>1], offset&1,  (data&0x0f) * 0x08, (data>>4) * 0x08 );
		  break;

		case 0x06:	/* Bankswitch for chips 0 & 1 */
		{
			int bank0_a = (data >> 1) & 1;
			int bank1_a = (data >> 2) & 1;
			int bank0_b = (data >> 3) & 1;
			int bank1_b = (data >> 4) & 1;
			// bit 6: chip 2 - ch0 ?
			// bit 7: chip 2 - ch1 ?

			k007232_set_bank( sound[0], bank0_a, bank0_b );
			k007232_set_bank( sound[1], bank1_a, bank1_b );
		}
		break;

		case 0x07:	/* Bankswitch for chip 2 */
		{
			int bank2_a = (data >> 0) & 7;
			int bank2_b = (data >> 3) & 7;

			k007232_set_bank( sound[2], bank2_a, bank2_b );
		}
		break;
	}
}

/* Read and write handlers for one K007232 chip:
   even and odd register are mapped swapped */
static READ8_DEVICE_HANDLER( hotchase_k007232_r )
{
	return k007232_r(device, offset ^ 1);
}

static WRITE8_DEVICE_HANDLER( hotchase_k007232_w )
{
	k007232_w(device, offset ^ 1, data);
}

static ADDRESS_MAP_START( hotchase_sound_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x1000, 0x100d) AM_DEVREADWRITE("konami1", hotchase_k007232_r, hotchase_k007232_w)	// 3 x K007232
	AM_RANGE(0x2000, 0x200d) AM_DEVREADWRITE("konami2", hotchase_k007232_r, hotchase_k007232_w)
	AM_RANGE(0x3000, 0x300d) AM_DEVREADWRITE("konami3", hotchase_k007232_r, hotchase_k007232_w)
	AM_RANGE(0x4000, 0x4007) AM_WRITE(hotchase_sound_control_w)	// Sound volume, banking, etc.
	AM_RANGE(0x5000, 0x5000) AM_WRITENOP	// ? (written with 0 on IRQ, 1 on FIRQ)
	AM_RANGE(0x6000, 0x6000) AM_READ(soundlatch_r)	// From main CPU (Read on IRQ)
	AM_RANGE(0x7000, 0x7000) AM_WRITENOP	// Command acknowledge ?
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


/***************************************************************************
                        WEC Le Mans 24 Input Ports
***************************************************************************/

static INPUT_PORTS_START( wecleman )
	PORT_START("IN0")	/* $140011.b */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Shift") PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Brake")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")	/* Motor? - $140013.b */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 )	// right sw
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 )	// left sw
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE4 )	// thermo
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL )	// from sound cpu ?
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")	/* $140015.b */
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

	PORT_START("DSWB")	/* $140017.b */
	PORT_DIPNAME( 0x01, 0x01, "Speed Unit" )
	PORT_DIPSETTING(    0x01, "km/h" )
	PORT_DIPSETTING(    0x00, "mph" )
	PORT_DIPNAME( 0x02, 0x02, "Unknown B-1" )	// single
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown B-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )		// 66 seconds at the start
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )	// 64
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )		// 62
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )	// 60
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown B-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown B-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("ACCEL")	/* Accelerator - $140021.b (0) */
	PORT_BIT( 0xff, 0, IPT_PEDAL ) PORT_MINMAX(0,0x80) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

	PORT_START("STEER")	/* Steering Wheel - $140021.b (2) */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)
INPUT_PORTS_END


/***************************************************************************
                            Hot Chase Input Ports
***************************************************************************/

static INPUT_PORTS_START( hotchase )
	PORT_START("IN0")	/* $140011.b */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Shift") PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Brake")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")	/* Motor? - $140013.b */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 )	// right sw
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 )	// left sw
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE4 )	// thermo
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL )	// from sound cpu ?
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")	/* $140015.b */
	PORT_DIPNAME( 0x01, 0x01, "Speed Unit" )
	PORT_DIPSETTING(    0x01, "KM" )
	PORT_DIPSETTING(    0x00, "M.P.H." )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 2-1" )	// single (wheel related)
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Unknown 2-3&4" )	// Most likely Difficulty
	PORT_DIPSETTING(    0x18, "0" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x00, "c" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5" )	// single
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* wheel <-> brake ; accel -> start */
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )	// single (wheel<->brake)
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")	/* $140017.b */
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

	PORT_START("ACCEL")	/* Accelerator - $140021.b (0) */
	PORT_BIT( 0xff, 0, IPT_PEDAL ) PORT_MINMAX(0,0x80) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

	PORT_START("STEER")	/* Steering Wheel - $140021.b (2) */
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
	GFXDECODE_ENTRY( "gfx2", 0, wecleman_bg_layout,   0, 2048/8 )	// [0] bg + fg + txt
	GFXDECODE_ENTRY( "gfx3", 0, wecleman_road_layout, 0, 2048/8 )	// [1] road
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
	GFXDECODE_ENTRY( "gfx4", 0, hotchase_road_layout, 0x70*16, 16 )	// road
GFXDECODE_END


/***************************************************************************
                        WEC Le Mans 24 Hardware Definitions
***************************************************************************/


static TIMER_DEVICE_CALLBACK( wecleman_scanline )
{
	int scanline = param;

	if(scanline == 232) // vblank irq
		cputag_set_input_line(timer.machine(), "maincpu", 4, HOLD_LINE);
	else if(((scanline % 64) == 0)) // timer irq TODO: timings
		cputag_set_input_line(timer.machine(), "maincpu", 5, HOLD_LINE);
}

static TIMER_DEVICE_CALLBACK( hotchase_scanline )
{
	int scanline = param;

	if(scanline == 224) // vblank irq
		cputag_set_input_line(timer.machine(), "maincpu", 4, HOLD_LINE);
	else if(((scanline % 64) == 0)) // timer irq TODO: timings
		cputag_set_input_line(timer.machine(), "maincpu", 5, HOLD_LINE);
}


static MACHINE_RESET( wecleman )
{
	k007232_set_bank( machine.device("konami"), 0, 1 );
}

static MACHINE_CONFIG_START( wecleman, wecleman_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000)	/* Schems show 10MHz */
	MCFG_CPU_PROGRAM_MAP(wecleman_map)
	MCFG_TIMER_ADD_SCANLINE("scantimer", wecleman_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("sub", M68000, 10000000)	/* Schems show 10MHz */
	MCFG_CPU_PROGRAM_MAP(wecleman_sub_map)

	/* Schems: can be reset, no nmi, soundlatch, 3.58MHz */
	MCFG_CPU_ADD("audiocpu", Z80, 3579545)
	MCFG_CPU_PROGRAM_MAP(wecleman_sound_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_MACHINE_RESET(wecleman)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE(320 +16, 256)
	MCFG_SCREEN_VISIBLE_AREA(0 +8, 320-1 +8, 0 +8, 224-1 +8)
	MCFG_SCREEN_UPDATE(wecleman)

	MCFG_GFXDECODE(wecleman)

	MCFG_PALETTE_LENGTH(2048)

	MCFG_VIDEO_START(wecleman)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2151, 3579545)
	MCFG_SOUND_ROUTE(0, "mono", 0.85)
	MCFG_SOUND_ROUTE(1, "mono", 0.85)

	MCFG_SOUND_ADD("konami", K007232, 3579545)
	MCFG_SOUND_ROUTE(0, "mono", 0.20)
	MCFG_SOUND_ROUTE(1, "mono", 0.20)
MACHINE_CONFIG_END


/***************************************************************************
                        Hot Chase Hardware Definitions
***************************************************************************/

static INTERRUPT_GEN( hotchase_sound_timer )
{
	generic_pulse_irq_line(device, M6809_FIRQ_LINE);
}

static const k051316_interface hotchase_k051316_intf_0 =
{
	"gfx2", 1,
	4, FALSE, 0,
	1, -0xb0 / 2, -16,
	hotchase_zoom_callback_0
};

static const k051316_interface hotchase_k051316_intf_1 =
{
	"gfx3", 2,
	4, FALSE, 0,
	0, -0xb0 / 2, -16,
	hotchase_zoom_callback_1
};

static MACHINE_RESET( hotchase )
{
	int i;

	/* TODO: PCB reference clearly shows that the POST has random/filled data on the paletteram.
             For now let's fill everything with white colors until we have better info about it */
	for(i=0;i<0x2000/2;i++)
	{
		machine.generic.paletteram.u16[i] = 0xffff;
		palette_set_color_rgb(machine,i,0xff,0xff,0xff);
	}
}


static MACHINE_CONFIG_START( hotchase, wecleman_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000)	/* 10 MHz - PCB is drawn in one set's readme */
	MCFG_CPU_PROGRAM_MAP(hotchase_map)
	MCFG_TIMER_ADD_SCANLINE("scantimer", hotchase_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("sub", M68000, 10000000)	/* 10 MHz - PCB is drawn in one set's readme */
	MCFG_CPU_PROGRAM_MAP(hotchase_sub_map)

	MCFG_CPU_ADD("audiocpu", M6809, 3579545 / 2)	/* 3.579/2 MHz - PCB is drawn in one set's readme */
	MCFG_CPU_PROGRAM_MAP(hotchase_sound_map)
	MCFG_CPU_PERIODIC_INT( hotchase_sound_timer, 496 )

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_MACHINE_RESET(hotchase)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(320 +16, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 224-1)
	MCFG_SCREEN_UPDATE(hotchase)

	MCFG_GFXDECODE(hotchase)
	MCFG_PALETTE_LENGTH(2048*2)

	MCFG_VIDEO_START(hotchase)

	MCFG_K051316_ADD("k051316_1", hotchase_k051316_intf_0)
	MCFG_K051316_ADD("k051316_2", hotchase_k051316_intf_1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("konami1", K007232, 3579545)
	MCFG_SOUND_ROUTE(0, "mono", 0.20)
	MCFG_SOUND_ROUTE(1, "mono", 0.20)

	MCFG_SOUND_ADD("konami2", K007232, 3579545)
	MCFG_SOUND_ROUTE(0, "mono", 0.20)
	MCFG_SOUND_ROUTE(1, "mono", 0.20)

	MCFG_SOUND_ADD("konami3", K007232, 3579545)
	MCFG_SOUND_ROUTE(0, "mono", 0.20)
	MCFG_SOUND_ROUTE(1, "mono", 0.20)
MACHINE_CONFIG_END


/***************************************************************************
                        WEC Le Mans 24 ROM Definitions
***************************************************************************/

ROM_START( wecleman )

	ROM_REGION( 0x40000, "maincpu", 0 )	/* Main CPU Code */
	ROM_LOAD16_BYTE( "602f08.17h", 0x00000, 0x10000, CRC(493b79d3) SHA1(9625e3b65c211d5081d8ed8977de287eff100842) )
	ROM_LOAD16_BYTE( "602f11.23h", 0x00001, 0x10000, CRC(6bb4f1fa) SHA1(2cfb7885b42b49dab9892e8dfd54914b64eeab06) )
	ROM_LOAD16_BYTE( "602a09.18h", 0x20000, 0x10000, CRC(8a9d756f) SHA1(12605e86ce29e6300b5400720baac7b0293d9e66) )
	ROM_LOAD16_BYTE( "602a10.22h", 0x20001, 0x10000, CRC(569f5001) SHA1(ec2dd331a279083cf847fbbe71c017038a1d562a) )

	ROM_REGION( 0x10000, "sub", 0 )	/* Sub CPU Code */
	ROM_LOAD16_BYTE( "602a06.18a", 0x00000, 0x08000, CRC(e12c0d11) SHA1(991afd48bf1b2c303b975ce80c754e5972c39111) )
	ROM_LOAD16_BYTE( "602a07.20a", 0x00001, 0x08000, CRC(47968e51) SHA1(9b01b2c6a14dd80327a8f66a7f1994471a4bc38e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound CPU Code */
	ROM_LOAD( "602a01.6d",  0x00000, 0x08000, CRC(deafe5f1) SHA1(4cfbe2841233b1222c22160af7287b7a7821c3a0) )

	ROM_REGION( 0x200000 * 2, "gfx1", 0 )	/* x2, do not dispose, zooming sprites */
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
	ROM_LOAD( "602a31.26g", 0x000000, 0x08000, CRC(01fa40dd) SHA1(2b8aa97f5116f39ae6a8e46f109853d70e370884) )	// layers
	ROM_LOAD( "602a30.24g", 0x008000, 0x08000, CRC(be5c4138) SHA1(7aee2ee17ef3e37399a60d9b019cfa733acbf07b) )
	ROM_LOAD( "602a29.23g", 0x010000, 0x08000, CRC(f1a8d33e) SHA1(ed6531f2fd4ad6835a879e9a5600387d8cad6d17) )

	ROM_REGION( 0x0c000, "gfx3", 0 )	/* road */
	ROM_LOAD( "602a04.11e", 0x000000, 0x08000, CRC(ade9f359) SHA1(58db6be6217ed697827015e50e99e58602042a4c) )
	ROM_LOAD( "602a05.13e", 0x008000, 0x04000, CRC(f22b7f2b) SHA1(857389c57552c4e2237cb599f4c68c381430475e) )

	ROM_REGION( 0x40000, "konami", 0 )	/* Samples (Channel A 0x20000=Channel B) */
	ROM_LOAD( "602a03.10a", 0x00000, 0x20000, CRC(31392b01) SHA1(0424747bc2015c9c93afd20e6a23083c0dcc4fb7) )
	ROM_LOAD( "602a02.8a",  0x20000, 0x20000, CRC(e2be10ae) SHA1(109c31bf7252c83a062d259143cd8299681db778) )

	ROM_REGION( 0x04000, "user1", 0 )	/* extra data for road effects? */
	ROM_LOAD( "602a12.1a",  0x000000, 0x04000, CRC(77b9383d) SHA1(7cb970889677704d6324bb64aafc05326c4503ad) )

ROM_END

static void wecleman_unpack_sprites(running_machine &machine)
{
	const char *region       = "gfx1";	// sprites

	const UINT32 len = machine.region(region)->bytes();
	UINT8 *src     = machine.region(region)->base() + len / 2 - 1;
	UINT8 *dst     = machine.region(region)->base() + len - 1;

	while(dst > src)
	{
		UINT8 data = *src--;
		if( (data&0xf0) == 0xf0 ) data &= 0x0f;
		if( (data&0x0f) == 0x0f ) data &= 0xf0;
		*dst-- = data & 0xF;    *dst-- = data >> 4;
	}
}

static void bitswap(running_machine &machine,UINT8 *src,size_t len,int _14,int _13,int _12,int _11,int _10,int _f,int _e,int _d,int _c,int _b,int _a,int _9,int _8,int _7,int _6,int _5,int _4,int _3,int _2,int _1,int _0)
{
	UINT8 *buffer = auto_alloc_array(machine, UINT8, len);
	int i;

	memcpy(buffer,src,len);
	for (i = 0;i < len;i++)
	{
		src[i] =
			buffer[BITSWAP24(i,23,22,21,_14,_13,_12,_11,_10,_f,_e,_d,_c,_b,_a,_9,_8,_7,_6,_5,_4,_3,_2,_1,_0)];
	}
	auto_free(machine, buffer);
}

/* Unpack sprites data and do some patching */
static DRIVER_INIT( wecleman )
{
	wecleman_state *state = machine.driver_data<wecleman_state>();
	int i, len;
	UINT8 *RAM;
//  UINT16 *RAM1 = (UINT16 *) machine.region("maincpu")->base();   /* Main CPU patches */
//  RAM1[0x08c2/2] = 0x601e;    // faster self test

	/* Decode GFX Roms - Compensate for the address lines scrambling */

	/*  Sprites - decrypting the sprites nearly KILLED ME!
        It's been the main cause of the delay of this driver ...
        I hope you'll appreciate this effort!  */

	/* let's swap even and odd *pixels* of the sprites */
	RAM = machine.region("gfx1")->base();
	len = machine.region("gfx1")->bytes();
	for (i = 0; i < len; i ++)
	{
		/* TODO: could be wrong, colors have to be fixed.       */
		/* The only certain thing is that 87 must convert to f0 */
		/* otherwise stray lines appear, made of pens 7 & 8     */
		RAM[i] = BITSWAP8(RAM[i],7,0,1,2,3,4,5,6);
	}

	bitswap(machine, machine.region("gfx1")->base(), machine.region("gfx1")->bytes(),
			0,1,20,19,18,17,14,9,16,6,4,7,8,15,10,11,13,5,12,3,2);

	/* Now we can unpack each nibble of the sprites into a pixel (one byte) */
	wecleman_unpack_sprites(machine);

	/* Bg & Fg & Txt */
	bitswap(machine, machine.region("gfx2")->base(), machine.region("gfx2")->bytes(),
			20,19,18,17,16,15,12,7,14,4,2,5,6,13,8,9,11,3,10,1,0);

	/* Road */
	bitswap(machine, machine.region("gfx3")->base(), machine.region("gfx3")->bytes(),
			20,19,18,17,16,15,14,7,12,4,2,5,6,13,8,9,11,3,10,1,0);

	state->m_spr_color_offs = 0x40;
}


/***************************************************************************
                            Hot Chase ROM Definitions
***************************************************************************/

ROM_START( hotchase )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* Main Code */
	ROM_LOAD16_BYTE( "763k05", 0x000000, 0x010000, CRC(f34fef0b) SHA1(9edaf6da988348cb32d5686fe7a67fb92b1c9777) )
	ROM_LOAD16_BYTE( "763k04", 0x000001, 0x010000, CRC(60f73178) SHA1(49c919d09fa464b205d7eccce337349e3a633a14) )
	ROM_LOAD16_BYTE( "763k03", 0x020000, 0x010000, CRC(28e3a444) SHA1(106b22a3cbe8301eac2e46674a267b96e72ac72f) )
	ROM_LOAD16_BYTE( "763k02", 0x020001, 0x010000, CRC(9510f961) SHA1(45b1920cab08a0dacd044c851d4e7f0cb5772b46) )

	ROM_REGION( 0x20000, "sub", 0 )	/* Sub Code */
	ROM_LOAD16_BYTE( "763k07", 0x000000, 0x010000, CRC(ae12fa90) SHA1(7f76f09916fe152411b5af3c504ee7be07497ef4) )
	ROM_LOAD16_BYTE( "763k06", 0x000001, 0x010000, CRC(b77e0c07) SHA1(98bf492ac889d31419df706029fdf3d51b85c936) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound Code */
	ROM_LOAD( "763f01", 0x8000, 0x8000, CRC(4fddd061) SHA1(ff0aa18605612f6102107a6be1f93ae4c5edc84f) )

	ROM_REGION( 0x300000 * 2, "gfx1", 0 )	/* x2, do not dispose, zooming sprites */
	ROM_LOAD16_WORD_SWAP( "763e17", 0x000000, 0x080000, CRC(8db4e0aa) SHA1(376cb3cae110998f2f9df7e6cdd35c06732fea69) )
	ROM_LOAD16_WORD_SWAP( "763e20", 0x080000, 0x080000, CRC(a22c6fce) SHA1(174fb9c1706c092947bcce386831acd33a237046) )
	ROM_LOAD16_WORD_SWAP( "763e18", 0x100000, 0x080000, CRC(50920d01) SHA1(313c7ecbd154b3f4c96f25c29a7734a9b3facea4) )
	ROM_LOAD16_WORD_SWAP( "763e21", 0x180000, 0x080000, CRC(77e0e93e) SHA1(c8e415438a1f5ad79b10fd3ad5cb22de0d562e5d) )
	ROM_LOAD16_WORD_SWAP( "763e19", 0x200000, 0x080000, CRC(a2622e56) SHA1(0a0ed9713882b987518e6f06a02dba417c1f4f32) )
	ROM_LOAD16_WORD_SWAP( "763e22", 0x280000, 0x080000, CRC(967c49d1) SHA1(01979d216a9fd8085298445ac5f7870d1598db74) )

	ROM_REGION( 0x20000, "gfx2", 0 )	/* bg */
	ROM_LOAD( "763e14", 0x000000, 0x020000, CRC(60392aa1) SHA1(8499eb40a246587e24f6fd00af2eaa6d75ee6363) )

	ROM_REGION( 0x10000, "gfx3", 0 )	/* fg (patched) */
	ROM_LOAD( "763a13", 0x000000, 0x010000, CRC(8bed8e0d) SHA1(ccff330abc23fe499e76c16cab5783c3daf155dd) )

	ROM_REGION( 0x20000, "gfx4", 0 )	/* road */
	ROM_LOAD( "763e15", 0x000000, 0x020000, CRC(7110aa43) SHA1(639dc002cc1580f0530bb5bb17f574e2258d5954) )

	ROM_REGION( 0x40000, "konami1", 0 )	/* Samples, 2 banks */
	ROM_LOAD( "763e11", 0x000000, 0x040000, CRC(9d99a5a7) SHA1(96e37bbb259e0a91d124c26b6b1a9b70de2e19a4) )

	ROM_REGION( 0x40000, "konami2", 0 )	/* Samples, 2 banks */
	ROM_LOAD( "763e10", 0x000000, 0x040000, CRC(ca409210) SHA1(703d7619c4bd33d2ff5fad127d98c82906fede33) )

	ROM_REGION( 0x100000, "konami3", 0 )	/* Samples, 4 banks for each ROM */
	ROM_LOAD( "763e08", 0x000000, 0x080000, CRC(054a9a63) SHA1(45d7926c9e7af47c041ba9b733e334bccd730a6d) )
	ROM_LOAD( "763e09", 0x080000, 0x080000, CRC(c39857db) SHA1(64b135a9ccf9e1dd50789cdd5c6bc03da8decfd0) )

	ROM_REGION( 0x08000, "user1", 0 )	/* extra data for road effects? */
	ROM_LOAD( "763a12", 0x000000, 0x008000, CRC(05f1e553) SHA1(8aaeb7374bd93038c24e6470398936f22cabb0fe) )
ROM_END

/*      Important: you must leave extra space when listing sprite ROMs
    in a ROM module definition.  This routine unpacks each sprite nibble
    into a byte, doubling the memory consumption. */

static void hotchase_sprite_decode( running_machine &machine, int num16_banks, int bank_size )
{
	UINT8 *base, *temp;
	int i;

	base = machine.region("gfx1")->base();	// sprites
	temp = auto_alloc_array(machine, UINT8,  bank_size );

	for( i = num16_banks; i >0; i-- ){
		UINT8 *finish   = base + 2*bank_size*i;
		UINT8 *dest     = finish - 2*bank_size;

		UINT8 *p1 = temp;
		UINT8 *p2 = temp+bank_size/2;

		UINT8 data;

		memcpy (temp, base+bank_size*(i-1), bank_size);

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
	auto_free( machine, temp );
}

/* Unpack sprites data and do some patching */
static DRIVER_INIT( hotchase )
{
	wecleman_state *state = machine.driver_data<wecleman_state>();
//  UINT16 *RAM1 = (UINT16) machine.region("maincpu")->base(); /* Main CPU patches */
//  RAM[0x1140/2] = 0x0015; RAM[0x195c/2] = 0x601A; // faster self test

	UINT8 *RAM;

	/* Decode GFX Roms */

	/* Let's swap even and odd bytes of the sprites gfx roms */
	RAM = machine.region("gfx1")->base();

	/* Now we can unpack each nibble of the sprites into a pixel (one byte) */
	hotchase_sprite_decode(machine,3,0x80000*2);	// num banks, bank len

	/* Let's copy the second half of the fg layer gfx (charset) over the first */
	RAM = machine.region("gfx3")->base();
	memcpy(&RAM[0], &RAM[0x10000/2], 0x10000/2);

	state->m_spr_color_offs = 0;
}


/***************************************************************************
                                Game driver(s)
***************************************************************************/

GAMEL( 1986, wecleman, 0, wecleman, wecleman, wecleman, ROT0, "Konami", "WEC Le Mans 24", 0, layout_wecleman )
GAMEL( 1988, hotchase, 0, hotchase, hotchase, hotchase, ROT0, "Konami", "Hot Chase", 0, layout_wecleman )
