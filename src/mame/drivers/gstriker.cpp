// license:BSD-3-Clause
// copyright-holders:Farfetch'd, David Haywood
/*** DRIVER INFO **************************************************************

Grand Striker, V Goal Soccer, World Cup '94
driver by Farfetch'd and David Haywood

Grand Striker (c)199?  Human
V Goal Soccer (c)199?  Tecmo (2 sets)
Tecmo World Cup '94 (c) 1994 Tecmo

******************************************************************************

    Hardware notes

Both games seem to be similar hardware, V Goal Soccer doesn't work.
the hardware is also quite similar to several other Video System games.

In particular, the sound hardware is identical to aerofgt (including the
memory mapping of the Z80, it's really just a romswap), and the sprite chip
(Fujitsu CG10103) is the same used in several Video System games (see the notes
in the video).

Grand Striker has an IRQ2 which is probably network related.

DSWs need correctly mapping, they're just commented for the moment.

TODO:
Finish hooking up the inputs
Tilemap scrolling/rotation/zooming or whatever effect it needs
Priorities are wrong. I suspect they need sprite orthogonality
Missing mixer registers (mainly layer enable/disable)

******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "includes/gstriker.h"
#include "sound/2610intf.h"

/*** README INFO **************************************************************

*** ROMSET: gstriker

Grand Striker
Human 1993

This game runs on Video Systems h/w.

PCB Nos: TW-107 94V-0
         LD01-A
CPU    : MC68000P10
SND    : Zilog Z0840006PSC (Z80), YM2610, YM3016-D
OSC    : 14.31818 MHz, 20.000MHz
XTAL   : 8.000MHz
DIPs   : 8 position (x2)
RAM    : 6264 (x12), 62256 (x4), CY7C195 (x1), 6116 (x3)
PALs   : 16L8 labelled S204A (near Z80)
         16L8 labelled S205A (near VS920A)
         16L8 labelled S201A \
                       S202A  |
                       S203A /  (Near 68000)


Other  :

MC68B50P (located next to 68000)
Fujitsu MB3773 (8 pin DIP)
Fujitsu MB605E53U (160 pin PQFP, located near U2 & U4) (screen tilemap)
Fujitsu CG10103 145 (160 pin PQFP, located near U25) (sprites)
VS9209 (located near DIPs)
VS920A (located near U79) (score tilemap)

ROMs:
human-1.u58 27C240   - Main Program
human-2.u79 27C1024  - ? (near VS920A)
human-3.u87 27C010   - Sound Program
human-4.u6      27C240   - ?, maybe region specific gfx
scrgs101.u25    23C16000 - GFX
scrgs102.u24    23C16000 - GFX
scrgs103.u23    23C16000 - GFX
scrgs104.u22    23C16000 - GFX
scrgs105.u2     23C16000 - GFX   \
scrgs105.u4     23C16000 - GFX   / note, contents of these are identical.
scrgs106.u93    232001   - Sounds
scrgs107.u99    23c8000  - Sounds

*** ROMSET: vgoalsoc

V Goal Soccer
Tecmo 199x?

This game runs on Video Systems h/w.

PCB No: VSIS-20V3, Tecmo No. VG63
CPU: MC68HC000P16
SND: Zilog Z0840006PSC (Z80), YM2610, YM3016-D
OSC: 14.31818 MHz (Near Z80), 32.000MHz (Near 68000), 20.000MHz (Near MCU)
DIPs: 8 position (x2)
RAM: LH5168 (x12), KM62256 (x4), CY7C195 (x1), LH5116 (x3)
PALs: 16L8 labelled S2032A (near Z80)
      16L8 labelled S2036A (near U104)
 4 x  16L8 labelled S2031A \
                    S2033A  |
                    S2034A  |  (Near 68000)
                    S2035A /


Other:

Hitachi H8/325  HD6473258P10 (Micro-controller, located next to 68000)
Fujitsu MB3773 (8 pin DIP)
Fujitsu MB605E53U (160 pin PQFP, located near U17 & U20)
Fujitsu CG10103 145 (160 pin PQFP, located next to VS9210)
VS9210 (located near U11 & U12)
VS9209 (located near DIPs)
VS920A (located near U48) (score tilemap)

ROMs:
c16_u37.u37 27C4002  - Main Program
c16_u48.u48 27C1024  - ?
c16_u65.u65 27C2001  - Sound Program
c13_u86.u86 HN62302  - Sounds
c13_u104.104    HN624116 - Sounds
c13_u20.u20     HN62418  - GFX   \
c13_u17.u17     HN62418  - GFX   / note, contents of these are identical.
c13_u11.u11     HN624116 - GFX
c13_u12.u12     HN624116 - GFX


*** ROMSET: vgoalsca

Tecmo V Goal Soccer (c)1994? Tecmo

CPU: 68000, Z80
Sound: YM2610
Other: VS9209, VS920A, VS9210, VS920B, HD6473258P10, CG10103, CY7C195,

X1: 20
X2: 32
X3: 14.31818

Note: Same hardware as Tecmo World Cup '94, minus one VS9209 chip.

*** ROMSET: twrldc94

Tecmo World Cup 94
Tecmo 1994

VSIS-20V3

   6264
   6264            H8/320         SW    SW
   6264            20MHz  13  6264
   6264   ?        68000-16   6264
   6264
   6264    ?
   6264
   6264
   6264
   6264

   U11         6264
   U12         6264
   U13
   U14         11


   U17-20                U104
           6264 6264
                      U86
   U17-20    ?                 YM2610
                   12   Z80

Frequencies: 68k is XTAL_32MHZ/2
             z80 is XTAL_20MHz/4

******************************************************************************/

void gstriker_state::machine_start()
{
	membank("soundbank")->configure_entries(0, 8, memregion("audiocpu")->base(), 0x8000);

	save_item(NAME(m_dmmy_8f_ret));
	save_item(NAME(m_pending_command));
}

/*** MISC READ / WRITE HANDLERS **********************************************/

READ16_MEMBER(gstriker_state::dmmy_8f)
{
	m_dmmy_8f_ret = ~m_dmmy_8f_ret;
	return m_dmmy_8f_ret;
}

/*** SOUND RELATED ***********************************************************/


WRITE16_MEMBER(gstriker_state::sound_command_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_pending_command = 1;
		soundlatch_byte_w(space, offset, data & 0xff);
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
}

#if 0
READ16_MEMBER(gstriker_state::pending_command_r)
{
	return m_pending_command;
}
#endif

WRITE8_MEMBER(gstriker_state::sh_pending_command_clear_w)
{
	m_pending_command = 0;
}

WRITE8_MEMBER(gstriker_state::sh_bankswitch_w)
{
	membank("soundbank")->set_entry(data & 0x07);
}

/*** GFX DECODE **************************************************************/

static const gfx_layout gs_8x8x4_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};


static const gfx_layout gs_16x16x4_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28,
	32+0,32+4,32+8,32+12,32+16,32+20,32+24,32+28
	},

	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
		8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64
	},
	16*64
};

static GFXDECODE_START( gstriker )
	GFXDECODE_ENTRY( "gfx1", 0, gs_8x8x4_layout,     0, 256 )
	GFXDECODE_ENTRY( "gfx2", 0, gs_16x16x4_layout,   0, 256 )
	GFXDECODE_ENTRY( "gfx3", 0, gs_16x16x4_layout,   0, 256 )

GFXDECODE_END


/*** MEMORY LAYOUTS **********************************************************/



static ADDRESS_MAP_START( gstriker_map, AS_PROGRAM, 16, gstriker_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_DEVREADWRITE("zoomtilemap", mb60553_zooming_tilemap_device,  vram_r, vram_w )
	AM_RANGE(0x140000, 0x141fff) AM_RAM AM_SHARE("cg10103_m_vram")
	AM_RANGE(0x180000, 0x180fff) AM_DEVREADWRITE("texttilemap", vs920a_text_tilemap_device,  vram_r, vram_w )
	AM_RANGE(0x181000, 0x181fff) AM_DEVREADWRITE("zoomtilemap", mb60553_zooming_tilemap_device,  line_r, line_w )
	AM_RANGE(0x1c0000, 0x1c0fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette") AM_MIRROR(0x00f000)

	AM_RANGE(0x200000, 0x20000f) AM_DEVREADWRITE("zoomtilemap", mb60553_zooming_tilemap_device,  regs_r, regs_w )
	AM_RANGE(0x200040, 0x20005f) AM_RAM AM_SHARE("mixerregs1")
	AM_RANGE(0x200060, 0x20007f) AM_RAM AM_SHARE("mixerregs2")
	AM_RANGE(0x200080, 0x200081) AM_READ_PORT("P1")
	AM_RANGE(0x200082, 0x200083) AM_READ_PORT("P2")
	AM_RANGE(0x200084, 0x200085) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x200086, 0x200087) AM_READ_PORT("DSW1")
	AM_RANGE(0x200088, 0x200089) AM_READ_PORT("DSW2")
	AM_RANGE(0x20008e, 0x20008f) AM_READ(dmmy_8f)
	AM_RANGE(0x2000a0, 0x2000a1) AM_WRITE(sound_command_w)

	AM_RANGE(0xffc000, 0xffffff) AM_RAM AM_SHARE("work_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, gstriker_state )
	AM_RANGE(0x0000, 0x77ff) AM_ROM
	AM_RANGE(0x7800, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("soundbank")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io_map, AS_IO, 8, gstriker_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ymsnd", ym2610_device, read, write)
	AM_RANGE(0x04, 0x04) AM_WRITE(sh_bankswitch_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(sh_pending_command_clear_w)
	AM_RANGE(0x0c, 0x0c) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END



/*** INPUT PORTS *************************************************************/

static INPUT_PORTS_START( gstriker_generic )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE2 )             // "Test"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN) // vbl?
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */

	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)   // "Spare"
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)   // "Spare"
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
INPUT_PORTS_END

static INPUT_PORTS_START( gstriker )
	PORT_INCLUDE( gstriker_generic )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0010, 0x0000, "2 Players VS CPU Game" )     // "Cooperation Coin"
	PORT_DIPSETTING(      0x0010, "1 Credit" )
	PORT_DIPSETTING(      0x0000, "2 Credits" )
	PORT_DIPNAME( 0x0020, 0x0000, "Player VS Player Game" )     // "Competitive Coin"
	PORT_DIPSETTING(      0x0020, "1 Credit" )
	PORT_DIPSETTING(      0x0000, "2 Credits" )
	PORT_DIPNAME( 0x0040, 0x0040, "New Challenger" )            /* unknown purpose */
	PORT_DIPSETTING(      0x0040, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Maximum Players" )           // "Cabinet Type"
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0080, "2" )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0006, 0x0006, "Player(s) VS CPU Time" )     // "Tournament  Time"
	PORT_DIPSETTING(      0x0006, "1:30" )
	PORT_DIPSETTING(      0x0004, "2:00" )
	PORT_DIPSETTING(      0x0002, "3:00" )
	PORT_DIPSETTING(      0x0000, "4:00" )
	PORT_DIPNAME( 0x0018, 0x0018, "Player VS Player Time" )     // "Competitive Time"
	PORT_DIPSETTING(      0x0018, "2:00" )
	PORT_DIPSETTING(      0x0010, "3:00" )
	PORT_DIPSETTING(      0x0008, "4:00" )
	PORT_DIPSETTING(      0x0000, "5:00" )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Demo_Sounds ) )      // "Demo Sound"
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Communication Mode" )            // "Master/Slave"
	PORT_DIPSETTING(      0x0040, "Master" )
	PORT_DIPSETTING(      0x0000, "Slave" )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )                   // "Self Test Mode"
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
INPUT_PORTS_END

static INPUT_PORTS_START( twrldc94 )
	PORT_INCLUDE( gstriker_generic )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_6C ) )

	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_6C ) )

	PORT_DIPNAME( 0x00c0, 0x00c0, "Play Time" )
	PORT_DIPSETTING(      0x0000, "P v CPU 1:00, P v P 1:30" )
	PORT_DIPSETTING(      0x00c0, "P v CPU 1:30, P v P 2:00" )
	PORT_DIPSETTING(      0x0040, "P v CPU 2:00, P v P 2:30" )
	PORT_DIPSETTING(      0x0080, "P v CPU 2:30, P v P 3:00" )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )

	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Show Configuration" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Countdown" )
	PORT_DIPSETTING(      0x0010, "54 sec" )
	PORT_DIPSETTING(      0x0000, "60 sec" )
	PORT_DIPNAME( 0x0020, 0x0020, "Start credit" )
	PORT_DIPSETTING(      0x0020, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
INPUT_PORTS_END

static INPUT_PORTS_START( vgoalsoc )
	PORT_INCLUDE( gstriker_generic )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_6C ) )

	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_6C ) )

	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR (Unknown) ) // Probably difficulty
	PORT_DIPSETTING(      0x0080, "A" )
	PORT_DIPSETTING(      0x00c0, "B" )
	PORT_DIPSETTING(      0x0040, "C" )
	PORT_DIPSETTING(      0x0000, "D" )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0003, 0x0003, "Player VS CPU Time" ) // no coperative
	PORT_DIPSETTING(      0x0002, "1:00" )
	PORT_DIPSETTING(      0x0003, "1:30" )
	PORT_DIPSETTING(      0x0001, "2:00" )
	PORT_DIPSETTING(      0x0000, "2:30" )
	PORT_DIPNAME( 0x000c, 0x000c, "Player VS Player Time" )
	PORT_DIPSETTING(      0x0008, "1:30" )
	PORT_DIPSETTING(      0x000c, "2:00" )
	PORT_DIPSETTING(      0x0004, "2:30" )
	PORT_DIPSETTING(      0x0000, "3:00" )
	PORT_DIPNAME( 0x0010, 0x0010, "Countdown" )
	PORT_DIPSETTING(      0x0010, "54 sec" )
	PORT_DIPSETTING(      0x0000, "60 sec" )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "DWS2:6" )        // hangs at POST
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Start credit" )
	PORT_DIPSETTING(      0x0080, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
INPUT_PORTS_END

/*** MACHINE DRIVER **********************************************************/

static MACHINE_CONFIG_START( gstriker, gstriker_state )
	MCFG_CPU_ADD("maincpu", M68000, 10000000)
	MCFG_CPU_PROGRAM_MAP(gstriker_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gstriker_state,  irq1_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80,8000000/2) /* 4 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_io_map)

	MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(5000) /* hand-tuned, it needs a bit */)
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(gstriker_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", gstriker)
	MCFG_PALETTE_ADD("palette", 0x800)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)


	MCFG_DEVICE_ADD("zoomtilemap", MB60553, 0)
	MCFG_MB60553_GFXDECODE("gfxdecode")
	MCFG_MB60553_GFX_REGION(1)

	MCFG_DEVICE_ADD("texttilemap", VS920A, 0)
	MCFG_VS920A_GFXDECODE("gfxdecode")
	MCFG_VS920A_GFX_REGION(0)


	MCFG_DEVICE_ADD("vsystem_spr", VSYSTEM_SPR, 0)
	MCFG_VSYSTEM_SPR_SET_GFXREGION(2)
	MCFG_VSYSTEM_SPR_SET_PALMASK(0x1f)
	MCFG_VSYSTEM_SPR_SET_TRANSPEN(0)
	MCFG_VSYSTEM_SPR_GFXDECODE("gfxdecode")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2610, 8000000)
	MCFG_YM2610_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "lspeaker",  0.25)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(1, "lspeaker",  1.0)
	MCFG_SOUND_ROUTE(2, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( twc94, gstriker )
	MCFG_CPU_REPLACE("maincpu", M68000, 16000000)
	MCFG_CPU_PROGRAM_MAP(gstriker_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gstriker_state,  irq1_line_hold)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( vgoal, gstriker )
	MCFG_CPU_REPLACE("maincpu", M68000, 16000000)
	MCFG_CPU_PROGRAM_MAP(gstriker_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gstriker_state,  irq1_line_hold)

	MCFG_DEVICE_MODIFY("vsystem_spr")
	MCFG_VSYSTEM_SPR_SET_TRANSPEN(0xf) // different vs. the other games, find register
MACHINE_CONFIG_END




/*** ROM LOADING *************************************************************/

ROM_START( gstriker )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "human-1.u58",  0x00000, 0x80000, CRC(45cf4857) SHA1(8133a9a7bdd547cc3d69140a68a1a5a7341e9f5b) )

	ROM_REGION( 0x40000, "audiocpu", 0 )
	ROM_LOAD( "human-3_27c1001.u87",  0x00000, 0x20000, CRC(2f28c01e) SHA1(63829ad7969d197b2f2c87cb88bdb9e9880ed2d6) )

	ROM_REGION( 0x20000, "gfx1", 0 ) // score tilemap
	ROM_LOAD( "human-2_27c1024.u79",  0x00000, 0x20000, CRC(a981993b) SHA1(ed92c7581d2b84a8628744dd5f8a2266c45dcd5b) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // scroll tilemap
	ROM_LOAD( "human_scr-gs-105_m531602c-44_3405356.u2",  0x00000, 0x200000, CRC(d584b568) SHA1(64c5e4fdbb859873e51f62d8f5314598108270ef) )
	ROM_LOAD( "human_scr-gs-105_m531602c-44_3405356.u4",  0x00000, 0x200000, CRC(d584b568) SHA1(64c5e4fdbb859873e51f62d8f5314598108270ef) ) // same content, dif pos on board

	ROM_REGION( 0x1000000, "gfx3", 0 )
	ROM_LOAD( "human_scr-gs-101_m531602c-40_3405351.u25", 0x000000, 0x200000, CRC(becaea24) SHA1(e96fca863f49f50992f56c7defa5a69599608785) )
	ROM_LOAD( "human_scr-gs-102_m531602c-41_3405355.u24", 0x200000, 0x200000, CRC(0dae7aba) SHA1(304f336994be33fa8239c13e6fd9967c06f97d5c) )
	ROM_LOAD( "human_scr-gs-103_m531602c-42_3405353.u23", 0x400000, 0x200000, CRC(3448fe92) SHA1(c4c2d2d5610795aff6633b0601ff484897598904) )
	ROM_LOAD( "human_scr-gs-104_m531602c-43_3405354.u22", 0x600000, 0x200000, CRC(0ac33e5a) SHA1(9d7717d80f2c6817bac3fad50c39e04f0aa94255) )
	ROM_LOAD( "human-4_27c240.u6",   0xf80000, 0x080000, CRC(a990f9bb) SHA1(7ce31d4c650eb244e2ab285f253a98d6613b7dc8) ) // extra european team flags

	ROM_REGION( 0x40000, "ymsnd.deltat", 0 )
	ROM_LOAD( "human_scr-gs-106_m532001b-16_3402370.u93", 0x00000, 0x040000, CRC(93c9868c) SHA1(dcecb34e46405155e35aaf134b8547430d23f5a7) )

	ROM_REGION( 0x100000, "ymsnd", 0 )
	ROM_LOAD( "scrgs107.u99", 0x00000, 0x100000, CRC(ecc0a01b) SHA1(239e832b7d22925460a8f44eb82e782cd13aba49) )

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "pal16l8.s201a.u52",   0x0000, 0x0104, CRC(724faf0f) SHA1(755fad09d188af58efce733a9f1256b1abc7c360) )
	ROM_LOAD( "pal16l8.s202a.u74",   0x0200, 0x0104, CRC(ad5c4722) SHA1(0aad71b73c6674e15596b7de59160a5156a4118d) )
	ROM_LOAD( "pal16l8.s203a.u75",   0x0400, 0x0104, CRC(ad197e2d) SHA1(e0691b79b8433285a0bafea1d52b0166f6417c20) )
	ROM_LOAD( "pal16l8.s204a.u89",   0x0600, 0x0104, CRC(eb997577) SHA1(504a2499c8a96c74607d06aefb0a062612a78b38) )
	ROM_LOAD( "pal16l8.s205a.u109",  0x0800, 0x0104, CRC(0d644e59) SHA1(bb8f4ab47d7bc9b9b37f636f8fa9c419f17630ad) )
ROM_END

ROM_START( gstrikera )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "human-1_27c4002.u58",  0x00000, 0x80000, CRC(7cf45320) SHA1(4127c93fe5f863cecf0a005c66129c0eb660f5dd) )

	ROM_REGION( 0x40000, "audiocpu", 0 )
	ROM_LOAD( "human-3_27c1001.u87",  0x00000, 0x20000, CRC(2f28c01e) SHA1(63829ad7969d197b2f2c87cb88bdb9e9880ed2d6) )

	ROM_REGION( 0x20000, "gfx1", 0 ) // score tilemap
	ROM_LOAD( "human-2_27c1024.u79",  0x00000, 0x20000, CRC(a981993b) SHA1(ed92c7581d2b84a8628744dd5f8a2266c45dcd5b) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // scroll tilemap
	ROM_LOAD( "human_scr-gs-105_m531602c-44_3405356.u2",  0x00000, 0x200000, CRC(d584b568) SHA1(64c5e4fdbb859873e51f62d8f5314598108270ef) )
	ROM_LOAD( "human_scr-gs-105_m531602c-44_3405356.u4",  0x00000, 0x200000, CRC(d584b568) SHA1(64c5e4fdbb859873e51f62d8f5314598108270ef) ) // same content, dif pos on board

	ROM_REGION( 0x1000000, "gfx3", 0 )
	ROM_LOAD( "human_scr-gs-101_m531602c-40_3405351.u25", 0x000000, 0x200000, CRC(becaea24) SHA1(e96fca863f49f50992f56c7defa5a69599608785) )
	ROM_LOAD( "human_scr-gs-102_m531602c-41_3405355.u24", 0x200000, 0x200000, CRC(0dae7aba) SHA1(304f336994be33fa8239c13e6fd9967c06f97d5c) )
	ROM_LOAD( "human_scr-gs-103_m531602c-42_3405353.u23", 0x400000, 0x200000, CRC(3448fe92) SHA1(c4c2d2d5610795aff6633b0601ff484897598904) )
	ROM_LOAD( "human_scr-gs-104_m531602c-43_3405354.u22", 0x600000, 0x200000, CRC(0ac33e5a) SHA1(9d7717d80f2c6817bac3fad50c39e04f0aa94255) )
	ROM_LOAD( "human-4_27c240.u6",   0xf80000, 0x080000, CRC(a990f9bb) SHA1(7ce31d4c650eb244e2ab285f253a98d6613b7dc8) ) // extra european team flags

	ROM_REGION( 0x40000, "ymsnd.deltat", 0 )
	ROM_LOAD( "human_scr-gs-106_m532001b-16_3402370.u93", 0x00000, 0x040000, CRC(93c9868c) SHA1(dcecb34e46405155e35aaf134b8547430d23f5a7) )

	ROM_REGION( 0x100000, "ymsnd", 0 )
	ROM_LOAD( "scrgs107.u99", 0x00000, 0x100000, CRC(ecc0a01b) SHA1(239e832b7d22925460a8f44eb82e782cd13aba49) )

		/* PALs were protected on this version, used the ones from the "gstriker" set */
	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "pal16l8.s201a.u52",   0x0000, 0x0104, CRC(724faf0f) SHA1(755fad09d188af58efce733a9f1256b1abc7c360) )
	ROM_LOAD( "pal16l8.s202a.u74",   0x0200, 0x0104, CRC(ad5c4722) SHA1(0aad71b73c6674e15596b7de59160a5156a4118d) )
	ROM_LOAD( "pal16l8.s203a.u75",   0x0400, 0x0104, CRC(ad197e2d) SHA1(e0691b79b8433285a0bafea1d52b0166f6417c20) )
	ROM_LOAD( "pal16l8.s204a.u89",   0x0600, 0x0104, CRC(eb997577) SHA1(504a2499c8a96c74607d06aefb0a062612a78b38) )
	ROM_LOAD( "pal16l8.s205a.u109",  0x0800, 0x0104, CRC(0d644e59) SHA1(bb8f4ab47d7bc9b9b37f636f8fa9c419f17630ad) )
ROM_END

ROM_START( gstrikerj )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "human1.u58",  0x00000, 0x80000, CRC(dce0549c) SHA1(5805a81ddae6bec5b6cc47dc1dbcbe2a81d2c033) )

	ROM_REGION( 0x40000, "audiocpu", 0 )
	ROM_LOAD( "human3.u87",  0x00000, 0x20000, CRC(2f28c01e) SHA1(63829ad7969d197b2f2c87cb88bdb9e9880ed2d6) )

	ROM_REGION( 0x20000, "gfx1", 0 ) // score tilemap
	ROM_LOAD( "human2.u79",  0x00000, 0x20000, CRC(9ad17eb3) SHA1(614b2630e02745f675b1791a514a90131264d545) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // scroll tilemap
	ROM_LOAD( "human_scr-gs-105_m531602c-44_3405356.u2",  0x00000, 0x200000, CRC(d584b568) SHA1(64c5e4fdbb859873e51f62d8f5314598108270ef) )
	ROM_LOAD( "human_scr-gs-105_m531602c-44_3405356.u4",  0x00000, 0x200000, CRC(d584b568) SHA1(64c5e4fdbb859873e51f62d8f5314598108270ef) ) // same content, dif pos on board

	ROM_REGION( 0x1000000, "gfx3", 0 )
	ROM_LOAD( "human_scr-gs-101_m531602c-40_3405351.u25", 0x000000, 0x200000, CRC(becaea24) SHA1(e96fca863f49f50992f56c7defa5a69599608785) )
	ROM_LOAD( "human_scr-gs-102_m531602c-41_3405355.u24", 0x200000, 0x200000, CRC(0dae7aba) SHA1(304f336994be33fa8239c13e6fd9967c06f97d5c) )
	ROM_LOAD( "human_scr-gs-103_m531602c-42_3405353.u23", 0x400000, 0x200000, CRC(3448fe92) SHA1(c4c2d2d5610795aff6633b0601ff484897598904) )
	ROM_LOAD( "human_scr-gs-104_m531602c-43_3405354.u22", 0x600000, 0x200000, CRC(0ac33e5a) SHA1(9d7717d80f2c6817bac3fad50c39e04f0aa94255) )
	// u6 is NOT populated on the JPN version

	ROM_REGION( 0x40000, "ymsnd.deltat", 0 )
	ROM_LOAD( "human_scr-gs-106_m532001b-16_3402370.u93", 0x00000, 0x040000, CRC(93c9868c) SHA1(dcecb34e46405155e35aaf134b8547430d23f5a7) )

	ROM_REGION( 0x100000, "ymsnd", 0 )
	ROM_LOAD( "scrgs107.u99", 0x00000, 0x100000, CRC(ecc0a01b) SHA1(239e832b7d22925460a8f44eb82e782cd13aba49) )

		/* PALs were protected on this version, used the ones from the "gstriker" set */
	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "pal16l8.s201a.u52",   0x0000, 0x0104, CRC(724faf0f) SHA1(755fad09d188af58efce733a9f1256b1abc7c360) )
	ROM_LOAD( "pal16l8.s202a.u74",   0x0200, 0x0104, CRC(ad5c4722) SHA1(0aad71b73c6674e15596b7de59160a5156a4118d) )
	ROM_LOAD( "pal16l8.s203a.u75",   0x0400, 0x0104, CRC(ad197e2d) SHA1(e0691b79b8433285a0bafea1d52b0166f6417c20) )
	ROM_LOAD( "pal16l8.s204a.u89",   0x0600, 0x0104, CRC(eb997577) SHA1(504a2499c8a96c74607d06aefb0a062612a78b38) )
	ROM_LOAD( "pal16l8.s205a.u109",  0x0800, 0x0104, CRC(0d644e59) SHA1(bb8f4ab47d7bc9b9b37f636f8fa9c419f17630ad) )
ROM_END

ROM_START( vgoalsoc )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "c16_u37.u37",  0x00000, 0x80000, CRC(18c05440) SHA1(0fc78ee0ba6d7817d4a93a80f668f193c352c00d) )

	ROM_REGION( 0x40000, "audiocpu", 0 )
	ROM_LOAD( "c16_u65.u65",  0x000000, 0x040000, CRC(2f7bf23c) SHA1(1a1a06f57bbac59807679e3762cb2f23ab1ad35e) )

	ROM_REGION( 0x20000, "mcu", 0 )
	ROM_LOAD( "vgoalsoc_hd6473258p10", 0x00000, 0x20000, NO_DUMP )

	ROM_REGION( 0x20000, "gfx1", 0 ) // score tilemap
	ROM_LOAD( "c16_u48.u48",  0x000000, 0x020000, CRC(ca059e7f) SHA1(2fa48b0fec1210575f3a1ecee7d2aec0af3fa9c4) )

	ROM_REGION( 0x100000, "gfx2", 0 ) // screen tilemap
	ROM_LOAD( "c13_u20.u20",  0x000000, 0x100000, CRC(bc6e07e8) SHA1(3f164165a2eed909aaf38d1ae23a622482d39f96) )
	ROM_LOAD( "c13_u17.u17",  0x000000, 0x100000, CRC(bc6e07e8) SHA1(3f164165a2eed909aaf38d1ae23a622482d39f96) ) // same content, dif pos on board

	ROM_REGION( 0x400000, "gfx3", 0 )
	ROM_LOAD( "c13_u11.u11",  0x000000, 0x200000, CRC(76d09f27) SHA1(ffef83954426f9e56bbe2d98b32cea675c063fab) )
	ROM_LOAD( "c13_u12.u12",  0x200000, 0x200000, CRC(a3874419) SHA1(c9fa283106ada3419e311f400fcf4251b32318c4) )

	ROM_REGION( 0x40000, "ymsnd.deltat", 0 )
	ROM_LOAD( "c13_u86.u86",  0x000000, 0x040000, CRC(4b76a162) SHA1(38dcb7536662f5f520e59f3ff746b42e9df789d2) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "c13_u104.104", 0x000000, 0x200000, CRC(8437b6f8) SHA1(79f183dcbf3cde5c77e086e4fdd8341809396e37) )
ROM_END

ROM_START( vgoalsca )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "vgoalc16.u37", 0x00000, 0x80000, CRC(775ef300) SHA1(d0ab1c13a19ce646c6edfc25a0c0994989560cbc) )

	ROM_REGION( 0x40000, "audiocpu", 0 )
	ROM_LOAD( "c16_u65.u65",  0x000000, 0x040000, CRC(2f7bf23c) SHA1(1a1a06f57bbac59807679e3762cb2f23ab1ad35e) )

	ROM_REGION( 0x20000, "mcu", 0 )
	ROM_LOAD( "vgoalsoc_hd6473258p10", 0x00000, 0x20000, NO_DUMP )

	ROM_REGION( 0x20000, "gfx1", 0 ) // fixed tile
	ROM_LOAD( "c16_u48.u48",  0x000000, 0x020000, CRC(ca059e7f) SHA1(2fa48b0fec1210575f3a1ecee7d2aec0af3fa9c4) )

	ROM_REGION( 0x100000, "gfx2", 0 ) // scroll tile
	ROM_LOAD( "c13_u20.u20",  0x000000, 0x100000, CRC(bc6e07e8) SHA1(3f164165a2eed909aaf38d1ae23a622482d39f96) )
	ROM_LOAD( "c13_u17.u17",  0x000000, 0x100000, CRC(bc6e07e8) SHA1(3f164165a2eed909aaf38d1ae23a622482d39f96) ) // same content, dif pos on board

	ROM_REGION( 0x400000, "gfx3", 0 )
	ROM_LOAD( "c13_u11.u11",  0x000000, 0x200000, CRC(76d09f27) SHA1(ffef83954426f9e56bbe2d98b32cea675c063fab) )
	ROM_LOAD( "c13_u12.u12",  0x200000, 0x200000, CRC(a3874419) SHA1(c9fa283106ada3419e311f400fcf4251b32318c4) )

	ROM_REGION( 0x40000, "ymsnd.deltat", 0 )
	ROM_LOAD( "c13_u86.u86",  0x000000, 0x040000, CRC(4b76a162) SHA1(38dcb7536662f5f520e59f3ff746b42e9df789d2) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "c13_u104.104", 0x000000, 0x200000, CRC(8437b6f8) SHA1(79f183dcbf3cde5c77e086e4fdd8341809396e37) )
ROM_END

ROM_START( twrldc94 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "13.u37",           0x00000, 0x80000, CRC(42adb463) SHA1(ec7bcb684489b56f81ab851a9d8f42d54679363b) )

	ROM_REGION( 0x40000, "audiocpu", 0 )
	ROM_LOAD( "12.u65",           0x000000, 0x040000, CRC(f316e7fc) SHA1(a2215605518e7293774735371c65abcead99bd88) )

	ROM_REGION( 0x20000, "mcu", 0 )
	ROM_LOAD( "twcup94_hd6473258p10", 0x00000, 0x20000, NO_DUMP )

	ROM_REGION( 0x20000, "gfx1", 0 ) // fixed tile
	ROM_LOAD( "11.u48",           0x000000, 0x020000, CRC(37d6dcb6) SHA1(679dd8b615497fff23c4638d413b5d4a724d3f2a) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // scroll tile
	ROM_LOAD( "u17",          0x000000, 0x200000, CRC(a5e40a61) SHA1(a2cb452fb069862570870653b29b045d12caf062) )
	ROM_LOAD( "u20",          0x000000, 0x200000, CRC(a5e40a61) SHA1(a2cb452fb069862570870653b29b045d12caf062) )

	ROM_REGION( 0x800000, "gfx3", 0 )
	ROM_LOAD( "u11",          0x000000, 0x200000, CRC(dd93fd45) SHA1(26491815b5443fe6d8b1ef4d795c5151fd75c101) )
	ROM_LOAD( "u12",          0x200000, 0x200000, CRC(8e3c9bd2) SHA1(bfd23157c836148a3860ccea5191f656fdd98ef4) )
	ROM_LOAD( "u13",          0x400000, 0x200000, CRC(8db6b3a9) SHA1(9422cd5d6fb57a7eaa7a13bdf4ccee1f8b57f773) )
	ROM_LOAD( "u14",          0x600000, 0x200000, CRC(89739c31) SHA1(29cd779bfe93448fb6cbfe6f8e3661dd659c0d21) )

	ROM_REGION( 0x40000, "ymsnd.deltat", 0 )
	ROM_LOAD( "u86",          0x000000, 0x040000, CRC(775f45dc) SHA1(1a740dd880d9f873e93dfc096fbcae1784b4f522) )

	ROM_REGION( 0x100000, "ymsnd", 0 )
	ROM_LOAD( "u104",         0x000000, 0x100000, CRC(df07d0af) SHA1(356560e164ff222bc9004fe202f829c93244a6c9) )
ROM_END

ROM_START( twrldc94a )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "twrdc94a_13.u37",           0x00000, 0x80000, CRC(08f314ee) SHA1(3fca5050f5bcd60533d3bd9dea81ba631a98bfd6) )

	ROM_REGION( 0x40000, "audiocpu", 0 )
	ROM_LOAD( "twrdc94a_12.u65",           0x000000, 0x040000, CRC(c131f5a4) SHA1(d8cc7c463ad628f6f052489a73b97f998532738d) )

	ROM_REGION( 0x20000, "mcu", 0 )
	ROM_LOAD( "twcup94_hd6473258p10", 0x00000, 0x20000, NO_DUMP )

	ROM_REGION( 0x20000, "gfx1", 0 ) // fixed tile
	ROM_LOAD( "twrdc94a_11.u48",           0x000000, 0x020000, CRC(37d6dcb6) SHA1(679dd8b615497fff23c4638d413b5d4a724d3f2a) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // scroll tile
	ROM_LOAD( "u17",          0x000000, 0x200000, CRC(a5e40a61) SHA1(a2cb452fb069862570870653b29b045d12caf062) )
	ROM_LOAD( "u20",          0x000000, 0x200000, CRC(a5e40a61) SHA1(a2cb452fb069862570870653b29b045d12caf062) )

	ROM_REGION( 0x800000, "gfx3", 0 )
	ROM_LOAD( "u11",          0x000000, 0x200000, CRC(dd93fd45) SHA1(26491815b5443fe6d8b1ef4d795c5151fd75c101) )
	ROM_LOAD( "u12",          0x200000, 0x200000, CRC(8e3c9bd2) SHA1(bfd23157c836148a3860ccea5191f656fdd98ef4) )
	ROM_LOAD( "u13",          0x400000, 0x200000, CRC(8db6b3a9) SHA1(9422cd5d6fb57a7eaa7a13bdf4ccee1f8b57f773) )
	ROM_LOAD( "u14",          0x600000, 0x200000, CRC(89739c31) SHA1(29cd779bfe93448fb6cbfe6f8e3661dd659c0d21) )

	ROM_REGION( 0x40000, "ymsnd.deltat", 0 )
	ROM_LOAD( "u86",          0x000000, 0x040000, CRC(775f45dc) SHA1(1a740dd880d9f873e93dfc096fbcae1784b4f522) )

	ROM_REGION( 0x100000, "ymsnd", 0 )
	ROM_LOAD( "u104",         0x000000, 0x100000, CRC(df07d0af) SHA1(356560e164ff222bc9004fe202f829c93244a6c9) )
ROM_END


/******************************************************************************************
Simple protection check concept.The M68k writes a command and the MCU
returns the PC at address 0xffc000.
The problem is that only the concept is easy,beating this protection requires a good
amount of time without a trojan...

Misc Notes:
-Protection routine is at 0x890
-An original feature of this game is that if you enter into service mode the game gives you
the possibility to test various stuff on a pre-registered play such as the speed or
the zooming.To use it,you should use Player 2 Start button to show the test screens
or to advance into the tests.
******************************************************************************************/
#define PC(_num_)\
m_work_ram[0x000/2] = (_num_ & 0xffff0000) >> 16;\
m_work_ram[0x002/2] = (_num_ & 0x0000ffff) >> 0;


WRITE16_MEMBER(gstriker_state::twrldc94_mcu_w)
{
	m_mcu_data = data & 0xff;
}

READ16_MEMBER(gstriker_state::twrldc94_mcu_r)
{
	return m_mcu_data;
}

WRITE16_MEMBER(gstriker_state::twrldc94_prot_reg_w)
{
	m_prot_reg[1] = m_prot_reg[0];
	m_prot_reg[0] = data & 0xff;

	if( ((m_prot_reg[1] & 2) == 2) && ((m_prot_reg[0] & 2) == 0) )
	{
		switch( m_gametype )
		{
			case 1:
				switch(m_mcu_data)
				{
					#define NULL_SUB 0x0000828E
					case 0x53: PC(0x0000a4c); break; // boot -> main loop

					/*
					    68 and 62 could be sprite or sound changes, or ?
					    68(),61()
					    if( !carry )
					    {
					        68(),65()
					    }
					    else
					    {
					        62(),72()
					    }
					*/
					case 0x68: PC(NULL_SUB); break; // time up doesn't block long enough for pk shootout
					case 0x61: PC(0x0003AF4); break; // after time up, pk shootout???
					case 0x65: PC(0x0003F26); break;

					// 62->72
					case 0x62: PC(NULL_SUB); break; // after lose shootout, continue ???
					case 0x72: PC(0x000409E); break; // game over

					/*
					    Attract mode is pre programmed loop called from main
					    that runs through top11->demoplay
					    (NOTE: sprites for demo play are being drawn at 0x141000,
					    this address is used in a few places, and there's some activity
					    further up around 0x1410b0.)

					    The loop begins with three prot calls:
					    one always present (may be diversion to 0x0010DC8 unreachable code
					    and prot cases 6a,79,6f) and two alternating calls.
					    The loop is 6e -> [6b|69] -> top11 -> (4 segment)playdemo

					    These are the likely suspects for attract mode:
					    0x0010E28 red tecmo on black
					    0x0010EEC bouncing ball and player with game title
					    0x00117A2 single segment demo play with player sprites at 0x140000
					    0x001120A sliding display of player photos
					    0x0010DC8 unreachable code at end of attract loop with cases 6a,79,6f

					*/
					case 0x6e: PC(0x0010E28); break; // loop
					case 0x6b: PC(0x0010EEC); break; // attract even
					case 0x69: PC(0x001120A); break; // attract odd

					// In "continue" screen
					// if( w@FFE078 & 80) 75
					// *** after 75 beq
					case 0x75: PC(NULL_SUB); break;

					// unreachable code at end of attract loop 6a->79->6f
					case 0x6a: PC(NULL_SUB); break;
					case 0x79: PC(NULL_SUB); break;
					case 0x6f: PC(NULL_SUB); break;

					default:
						popmessage("Unknown MCU CMD %04x",m_mcu_data);
						PC(NULL_SUB);
						break;
				}
				break;

			case 2:
				switch(m_mcu_data)
				{
					case 0x53: PC(0x00000a5c); break; // POST

					default:
						popmessage("Unknown MCU CMD %04x",m_mcu_data);
						PC(NULL_SUB);
						break;
				}
				break;


			case 3:
				switch(m_mcu_data)
				{
					case 0x33: PC(0x00063416); break; // *after game over, is this right?
					case 0x3d: PC(0x0006275C); break; // after sprite ram init, team select
					case 0x42: PC(0x0006274E); break; // after press start, init sprite ram
					case 0x43: PC(0x0006a000); break; // POST
					case 0x50: PC(0x00001900); break; // enter main loop
					case 0x65: PC(0x0006532C); break; // results
					case 0x70: PC(0x00063416); break; // *attract loop ends, what should happen after "standings" display?
					case 0x74: PC(0x000650D8); break; // after time up, show scores and continue
					case 0x79: PC(0x0006072E); break; // after select, start match

					default:
						popmessage("Unknown MCU CMD %04x",m_mcu_data);
						PC(0x00000586); // rts
						break;
				}
				break;
		}
	}
}

READ16_MEMBER(gstriker_state::twrldc94_prot_reg_r)
{
	// bit 0 is for debugging vgoalsoc?
	// Setting it results in a hang with a digit displayed on screen
	// For twrldc94, it just disables sound.

	return m_prot_reg[0];
}

/*
    vgoalsoc uses a set of programmable timers.
    There is a code implementation for at 00065F00 that appears to have
    been RTSed out.
    I'm guessing it was replaced with an external implementation.

    This does indicate though that the protection could be performing
    other more complicated functions.

    The tick count is usually set to 0x3c => it's driven off vblank?
*/
//m_work_ram[ (0xffe900 - 0xffc00) ]
#define COUNTER1_ENABLE m_work_ram[0x2900/2] >> 8
#define COUNTER2_ENABLE (m_work_ram[0x2900/2] & 0xff)
#define TICK_1 m_work_ram[0x2908/2]
#define TICKCOUNT_1 m_work_ram[0x290a/2]
#define TICK_2 m_work_ram[0x290c/2]
#define TICKCOUNT_3 m_work_ram[0x290e/2]
#define COUNTER_1 m_work_ram[0x2928/2]
#define COUNTER_2 m_work_ram[0x292a/2]
READ16_MEMBER(gstriker_state::vbl_toggle_r)
{
	return 0xff;
}

WRITE16_MEMBER(gstriker_state::vbl_toggle_w)
{
	if( COUNTER1_ENABLE == 1 )
	{
		TICK_1 = (TICK_1 - 1) & 0xff;   // 8bit
		if( TICK_1 <= 0 )
		{
			TICK_1 = TICKCOUNT_1;
			COUNTER_1 = (COUNTER_1 - 1);// & 0xff; has to be 16bit for continue timer.
		}
	}

	if( COUNTER2_ENABLE == 2 )
	{
		TICK_2  = (TICK_2 - 1) & 0xff;
		if( TICK_2 <= 0 )
		{
			TICK_2 = TICKCOUNT_3;
			COUNTER_2 = (COUNTER_2 - 1);// & 0xff;
		}
	}
}

void gstriker_state::mcu_init()
{
	m_dmmy_8f_ret = 0xFFFF;
	m_pending_command = 0;
	m_mcu_data = 0;

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x20008a, 0x20008b, write16_delegate(FUNC(gstriker_state::twrldc94_mcu_w),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x20008a, 0x20008b, read16_delegate(FUNC(gstriker_state::twrldc94_mcu_r),this));

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x20008e, 0x20008f, write16_delegate(FUNC(gstriker_state::twrldc94_prot_reg_w),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x20008e, 0x20008f, read16_delegate(FUNC(gstriker_state::twrldc94_prot_reg_r),this));

	save_item(NAME(m_mcu_data));
	save_item(NAME(m_prot_reg));
}

DRIVER_INIT_MEMBER(gstriker_state,twrldc94)
{
	m_gametype = 1;
	mcu_init();
}

DRIVER_INIT_MEMBER(gstriker_state,twrldc94a)
{
	m_gametype = 2;
	mcu_init();
}

DRIVER_INIT_MEMBER(gstriker_state,vgoalsoc)
{
	m_gametype = 3;
	mcu_init();

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x200090, 0x200091, write16_delegate(FUNC(gstriker_state::vbl_toggle_w),this)); // vblank toggle
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x200090, 0x200091, read16_delegate(FUNC(gstriker_state::vbl_toggle_r),this));
}

/*** GAME DRIVERS ************************************************************/

GAME( 1993, gstriker, 0,        gstriker, gstriker, driver_device, 0,        ROT0, "Human", "Grand Striker", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1993, gstrikera, gstriker, gstriker, gstriker, driver_device, 0,        ROT0, "Human", "Grand Striker (Americas)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1993, gstrikerj, gstriker, gstriker, gstriker, driver_device, 0,        ROT0, "Human", "Grand Striker (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )


/* Similar, but not identical hardware, appear to be protected by an MCU :-( */
GAME( 1994, vgoalsoc, 0,        vgoal,    vgoalsoc, gstriker_state, vgoalsoc,   ROT0, "Tecmo", "V Goal Soccer (Europe)",         MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // has ger/hol/arg/bra/ita/eng/spa/fra
GAME( 1994, vgoalsca, vgoalsoc, vgoal,    vgoalsoc, gstriker_state, vgoalsoc,   ROT0, "Tecmo", "V Goal Soccer (US/Japan/Korea)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // has ger/hol/arg/bra/ita/kor/usa/jpn
GAME( 1994, twrldc94, 0,        twc94,    twrldc94, gstriker_state, twrldc94,   ROT0, "Tecmo", "Tecmo World Cup '94 (set 1)",    MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, twrldc94a,twrldc94, twc94,    twrldc94, gstriker_state, twrldc94a,  ROT0, "Tecmo", "Tecmo World Cup '94 (set 2)",    MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
