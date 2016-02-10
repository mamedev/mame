// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Shanghai 3           (c)1993 Sunsoft     (68000     AY8910 OKI6295)
Hebereke no Popoon   (c)1994 Sunsoft     (68000 Z80 YM3438 OKI6295)
Blocken              (c)1994 KID / Visco (68000 Z80 YM3438 OKI6295)

These games use the custom blitter GA9201 KA01-0249 (120pin IC)

driver by Nicola Salmoria

TODO:
all games:
- Blitter needs to be device-ized
shangha3:
- The zoom used for the "100" floating score when you remove tiles is very
  rough.
heberpop:
- Unknown writes to sound ports 40/41
blocken:
- incomplete zoom support, and missing rotation support. Setting the game in
  Game Mode B shows a decent test case for it by starting a play.
- attract mode tries to read at 0x80000-0xfffff area, returning 0 in there
  freezes the demo play for some frames (MT #00985). For now I've returned $ff,
  but needs HW tests to check out what lies in there (maybe a ROM mirror).
- how to play screen is bogus, it basically loses sync pretty soon.

Notes:
- a Blocken PCB shot shows a 48 MHz xtal, game is definitely too slow at
  8 MHz (noticeable thru colour cycling effects)
- Confirmed OSC is 48MHz and OKI resonator is 1.056MHz.

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "sound/2612intf.h"
#include "includes/shangha3.h"


/* this looks like a simple protection check */
/*
write    read
78 78 -> 0
9b 10 -> 1
9b 20 -> 3
9b 40 -> 7
9b 80 -> f
08    -> e
10    -> c
20    -> 8
40    -> 0
*/
READ16_MEMBER(shangha3_state::shangha3_prot_r)
{
	static const int result[] = { 0x0,0x1,0x3,0x7,0xf,0xe,0xc,0x8,0x0};

	logerror("PC %04x: read 20004e\n",space.device().safe_pc());

	return result[m_prot_count++ % 9];
}

WRITE16_MEMBER(shangha3_state::shangha3_prot_w)
{
	logerror("PC %04x: write %02x to 20004e\n",space.device().safe_pc(),data);
}

WRITE16_MEMBER(shangha3_state::shangha3_coinctrl_w)
{
	if (ACCESSING_BITS_8_15)
	{
		machine().bookkeeping().coin_lockout_w(0,~data & 0x0400);
		machine().bookkeeping().coin_lockout_w(1,~data & 0x0400);
		machine().bookkeeping().coin_counter_w(0,data & 0x0100);
		machine().bookkeeping().coin_counter_w(1,data & 0x0200);
	}
}

WRITE16_MEMBER(shangha3_state::heberpop_coinctrl_w)
{
	if (ACCESSING_BITS_0_7)
	{
		/* the sound ROM bank is selected by the main CPU! */
		m_oki->set_bank_base((data & 0x08) ? 0x40000 : 0x00000);

		machine().bookkeeping().coin_lockout_w(0,~data & 0x04);
		machine().bookkeeping().coin_lockout_w(1,~data & 0x04);
		machine().bookkeeping().coin_counter_w(0,data & 0x01);
		machine().bookkeeping().coin_counter_w(1,data & 0x02);
	}
}

WRITE16_MEMBER(shangha3_state::blocken_coinctrl_w)
{
	if (ACCESSING_BITS_0_7)
	{
		/* the sound ROM bank is selected by the main CPU! */
		m_oki->set_bank_base(((data >> 4) & 3) * 0x40000);

		machine().bookkeeping().coin_lockout_w(0,~data & 0x04);
		machine().bookkeeping().coin_lockout_w(1,~data & 0x04);
		machine().bookkeeping().coin_counter_w(0,data & 0x01);
		machine().bookkeeping().coin_counter_w(1,data & 0x02);
	}
}


WRITE16_MEMBER(shangha3_state::heberpop_sound_command_w)
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_byte_w(space, 0, data & 0xff);
		m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);  /* RST 38h */
	}
}

WRITE16_MEMBER(shangha3_state::irq_ack_w)
{
	m_maincpu->set_input_line(4, CLEAR_LINE);
}

static ADDRESS_MAP_START( shangha3_map, AS_PROGRAM, 16, shangha3_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x100fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x200000, 0x200001) AM_READ_PORT("INPUTS")
	AM_RANGE(0x200002, 0x200003) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x200008, 0x200009) AM_WRITE(blitter_go_w)
	AM_RANGE(0x20000a, 0x20000b) AM_WRITE(irq_ack_w)
	AM_RANGE(0x20000c, 0x20000d) AM_WRITE(shangha3_coinctrl_w)
	AM_RANGE(0x20001e, 0x20001f) AM_DEVREAD8("aysnd", ay8910_device, data_r, 0x00ff)
	AM_RANGE(0x20002e, 0x20002f) AM_DEVWRITE8("aysnd", ay8910_device, data_w, 0x00ff)
	AM_RANGE(0x20003e, 0x20003f) AM_DEVWRITE8("aysnd", ay8910_device, address_w, 0x00ff)
	AM_RANGE(0x20004e, 0x20004f) AM_READWRITE(shangha3_prot_r,shangha3_prot_w)
	AM_RANGE(0x20006e, 0x20006f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x300000, 0x30ffff) AM_RAM AM_SHARE("ram") /* gfx & work ram */
	AM_RANGE(0x340000, 0x340001) AM_WRITE(flipscreen_w)
	AM_RANGE(0x360000, 0x360001) AM_WRITE(gfxlist_addr_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( heberpop_map, AS_PROGRAM, 16, shangha3_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x100fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x200000, 0x200001) AM_READ_PORT("INPUTS")
	AM_RANGE(0x200002, 0x200003) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x200004, 0x200005) AM_READ_PORT("DSW")
	AM_RANGE(0x200008, 0x200009) AM_WRITE(blitter_go_w)
	AM_RANGE(0x20000a, 0x20000b) AM_WRITE(irq_ack_w)
	AM_RANGE(0x20000c, 0x20000d) AM_WRITE(heberpop_coinctrl_w)
	AM_RANGE(0x20000e, 0x20000f) AM_WRITE(heberpop_sound_command_w)
	AM_RANGE(0x300000, 0x30ffff) AM_RAM AM_SHARE("ram") /* gfx & work ram */
	AM_RANGE(0x340000, 0x340001) AM_WRITE(flipscreen_w)
	AM_RANGE(0x360000, 0x360001) AM_WRITE(gfxlist_addr_w)
	AM_RANGE(0x800000, 0xb7ffff) AM_ROM AM_REGION("gfx1", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( blocken_map, AS_PROGRAM, 16, shangha3_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x100001) AM_READ_PORT("INPUTS")
	AM_RANGE(0x100002, 0x100003) AM_READ_PORT("SYSTEM") AM_WRITENOP // w -> unknown purpose
	AM_RANGE(0x100004, 0x100005) AM_READ_PORT("DSW")
	AM_RANGE(0x100008, 0x100009) AM_WRITE(blitter_go_w)
	AM_RANGE(0x10000a, 0x10000b) AM_READNOP AM_WRITE(irq_ack_w) // r -> unknown purpose (value doesn't matter, left-over?)
	AM_RANGE(0x10000c, 0x10000d) AM_WRITE(blocken_coinctrl_w)
	AM_RANGE(0x10000e, 0x10000f) AM_WRITE(heberpop_sound_command_w)
	AM_RANGE(0x200000, 0x200fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x300000, 0x30ffff) AM_RAM AM_SHARE("ram") /* gfx & work ram */
	AM_RANGE(0x340000, 0x340001) AM_WRITE(flipscreen_w)
	AM_RANGE(0x360000, 0x360001) AM_WRITE(gfxlist_addr_w)
	AM_RANGE(0x800000, 0xb7ffff) AM_ROM AM_REGION("gfx1", 0)
ADDRESS_MAP_END


static ADDRESS_MAP_START( heberpop_sound_map, AS_PROGRAM, 8, shangha3_state )
	AM_RANGE(0x0000, 0xf7ff) AM_ROM
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( heberpop_sound_io_map, AS_IO, 8, shangha3_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ymsnd", ym3438_device, read, write)
	AM_RANGE(0x80, 0x80) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xc0, 0xc0) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( shangha3 )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE(0x0020, IP_ACTIVE_LOW)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW1") /* Dipswitch locations assigned as per service mode */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWA:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2") /* Dipswitch locations assigned as per service mode */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Base Time" )         PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x04, "70 sec" )
	PORT_DIPSETTING(    0x0c, "80 sec" )
	PORT_DIPSETTING(    0x08, "90 sec" )
	PORT_DIPSETTING(    0x00, "100 sec" )
	PORT_DIPNAME( 0x30, 0x30, "Additional Time" )       PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x10, "4 sec" )
	PORT_DIPSETTING(    0x30, "5 sec" )
	PORT_DIPSETTING(    0x20, "6 sec" )
	PORT_DIPSETTING(    0x00, "7 sec" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( heberpop )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") /* vblank?? has to toggle */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") /* vblank?? has to toggle */

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE(0x0020, IP_ACTIVE_LOW)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") /* Dipswitch locations assigned as per service mode */
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Allow Diagonal Moves" )      PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( blocken )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") /* vblank?? has to toggle */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") /* vblank?? has to toggle */

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE )  /* keeping this pressed on boot generates "BAD DIPSW" */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") /* Dipswitch locations assigned as per service mode */
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0006, 0x0006, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Game Type" )         PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, "A" )
	PORT_DIPSETTING(      0x0000, "B" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Players ) )      PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0030, "1" )
	PORT_DIPSETTING(      0x0020, "2" )
	PORT_DIPSETTING(      0x0010, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x0200, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0f00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x0d00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0b00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0900, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 1C_7C ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4,
			9*4, 8*4, 11*4, 10*4, 13*4, 12*4, 15*4, 14*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8
};

static GFXDECODE_START( shangha3 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 128 )
GFXDECODE_END


static MACHINE_CONFIG_START( shangha3, shangha3_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_48MHz/3) // TMP68HC000N-16
	MCFG_CPU_PROGRAM_MAP(shangha3_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", shangha3_state,  irq4_line_assert)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
//  MCFG_SCREEN_SIZE(24*16, 16*16)
//  MCFG_SCREEN_VISIBLE_AREA(0*16, 24*16-1, 1*16, 15*16-1)
	MCFG_SCREEN_RAW_PARAMS(XTAL_48MHz/6,512,0,24*16,263,1*16,15*16) /* refresh rate is unknown */

	MCFG_SCREEN_UPDATE_DRIVER(shangha3_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", shangha3)

	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(RRRRRGGGGGBBBBBx)
	MCFG_PALETTE_ENABLE_SHADOWS()

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, XTAL_48MHz/32) // 1.5MHz
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW2"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_OKIM6295_ADD("oki", XTAL_1_056MHz, OKIM6295_PIN7_HIGH) // pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( heberpop, shangha3_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_48MHz/3) // TMP68HC000N-16 like the others??
	MCFG_CPU_PROGRAM_MAP(heberpop_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", shangha3_state,  irq4_line_assert)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_48MHz/8)  /* 6 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(heberpop_sound_map)
	MCFG_CPU_IO_MAP(heberpop_sound_io_map)  /* NMI triggered by YM3438 */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
//  MCFG_SCREEN_SIZE(24*16, 16*16)
//  MCFG_SCREEN_VISIBLE_AREA(0*16, 24*16-1, 1*16, 15*16-1)
	MCFG_SCREEN_RAW_PARAMS(XTAL_48MHz/6,512,0,24*16,263,1*16,15*16) /* refresh rate is unknown */

	MCFG_SCREEN_UPDATE_DRIVER(shangha3_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", shangha3)

	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(RRRRRGGGGGBBBBBx)
	MCFG_PALETTE_ENABLE_SHADOWS()

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3438, XTAL_48MHz/6) /* 8 MHz? */
	MCFG_YM2612_IRQ_HANDLER(INPUTLINE("audiocpu", INPUT_LINE_NMI))
	MCFG_SOUND_ROUTE(0, "mono", 0.40)
	MCFG_SOUND_ROUTE(1, "mono", 0.40)

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( blocken, shangha3_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_48MHz/3) // TMP68HC000N-16
	MCFG_CPU_PROGRAM_MAP(blocken_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", shangha3_state,  irq4_line_assert)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_48MHz/8)   /* 6 MHz? */
	MCFG_CPU_PROGRAM_MAP(heberpop_sound_map)
	MCFG_CPU_IO_MAP(heberpop_sound_io_map)  /* NMI triggered by YM3438 */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
//  MCFG_SCREEN_SIZE(24*16, 16*16)
//  MCFG_SCREEN_VISIBLE_AREA(0*16, 24*16-1, 1*16, 15*16-1)
	MCFG_SCREEN_RAW_PARAMS(XTAL_48MHz/6,512,0,24*16,263,1*16,15*16) /* refresh rate is unknown */

	MCFG_SCREEN_UPDATE_DRIVER(shangha3_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", shangha3)

	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(RRRRRGGGGGBBBBBx)
	MCFG_PALETTE_ENABLE_SHADOWS()

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3438, XTAL_48MHz/6) /* 8 MHz? */
	MCFG_YM2612_IRQ_HANDLER(INPUTLINE("audiocpu", INPUT_LINE_NMI))
	MCFG_SOUND_ROUTE(0, "mono", 0.40)
	MCFG_SOUND_ROUTE(1, "mono", 0.40)

	MCFG_OKIM6295_ADD("oki", XTAL_1_056MHz, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/


/*

Shanghai 3  (c)  1993 Sunsoft

SUN04C
+-------------------------------------+
|        M6295 IC75                   |
|    1.056MHz  SW2                    |
| VOL  YM2149F SW1                    |
|                                     |
|                                     |
|J                    M M             |
|A                    2 2             |
|M        M                           |
|M        1    M    +------+     IC48*|
|A             3    |GA9201|     IC47*|
|         M         | KA01 |     IC46*|
|         1    M    | 0249 |     IC44*|
|              3    +------+          |
|    68000                        I   |
| I I                             C   |
| C C 48MHz                       4   |
| 3 2                             3   |
+-------------------------------------+

   CPU: TMP68HC000-16
 Sound: YM2149F, OKI M6295
 Video: GA9201 KA01-0249 (QFP120)
   OSC: 48MHz, 1.056MHz (resonator)
Memory: M1 = TMM2018AP-45 (2K x 8 SRAM)
        M2 = LH52B256D-70LL (32K x 8 SRAM)
        M3 = TC514280BJL-70 (256K x 4 DRAM)
 Other: SW1 & SW2 - 8-position dipswitch
        VOL - Volume pot

* = unpopulated 32 pin rom sockets silkscreened 27C040

NOTE: For the "World" set, it differs from the US set (besides the US set having the data repeated) by 2 bytes.

  0xB8B == 0x00 for world, 0x12 for US set (flag to show FBI warning screen)
0x32800 == 0xB9 for world, 0xCB for US set (checksum adjustment)

*/

ROM_START( shangha3 ) /* PCB labeled SUN04C - Has two additional tiles sets to choose from. */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ic3",  0x0000, 0x40000, CRC(4a9cdcfd) SHA1(c27b767ef2de90b36095b49baae9fa514f461c2c) ) /* ST M27C2001 EPROM with no label */
	ROM_LOAD16_BYTE( "ic2",  0x0001, 0x40000, CRC(714bfdbc) SHA1(0ce611624e8a5e28cba5443b63b8872eed9f68fc) ) /* ST M27C2001 EPROM with no label */

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "s3j_char-a1.ic43", 0x0000, 0x200000, CRC(2dbf9d17) SHA1(dd94ddc4bb02ab544aa3f89b614afc46678cc48d) ) /* 42pin MASK ROM */
	ROM_LOAD( "27c4000.ic44", 0x200000, 0x080000, CRC(6344ffb7) SHA1(06bc5bcf94973ec152e7abf9cc658ef319eb4b65) ) // korean fonts, vs mode how to play etc? (probably for Korean program roms we don't have, but was on World board)

	ROM_REGION( 0x40000, "oki", 0 ) /* samples for M6295 */
	ROM_LOAD( "s3j_v10.ic75", 0x0000, 0x40000, CRC(f0cdc86a) SHA1(b1017a9841a56e0f5d2714f550f64ed1f4e238e6) )
ROM_END

ROM_START( shangha3u ) /* PCB labeled SUN04C - Shows FBI "Winners Don't Use Drugs" splash screen (once). Has two additional tiles sets to choose from. */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ic3.ic3",  0x0000, 0x80000, CRC(53ef4988) SHA1(63f098d95865928a553e945fe60dea79aa16c603) ) /* ST M27C4001 EPROM labeled IC3 */
	ROM_LOAD16_BYTE( "ic2.ic2",  0x0001, 0x80000, CRC(fdea0232) SHA1(8983a646412df01b6bc66994700796e7b7fcbb61) ) /* ST M27C4001 EPROM labeled IC2 */
	/* both program ROMs are double sized with the identical halves */

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "s3j_char-a1.ic43", 0x0000, 0x200000, CRC(2dbf9d17) SHA1(dd94ddc4bb02ab544aa3f89b614afc46678cc48d) ) /* 42pin MASK ROM */

	ROM_REGION( 0x80000, "oki", 0 ) /* samples for M6295 */
	ROM_LOAD( "ic75.ic75", 0x0000, 0x80000, CRC(a8136d8c) SHA1(8028bda5642c2546c1ac8da78dbff4084829f03b) ) /* 27C4001 with 1st & 2nd halves == s3j_v10.ic75 */
ROM_END

ROM_START( shangha3j ) /* PCB labeled SUN04C */
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s3j_v11.ic3",  0x0000, 0x40000, CRC(e98ce9c8) SHA1(359e117aebb644d7b235add7e71ed6891243d451) )
	ROM_LOAD16_BYTE( "s3j_v11.ic2",  0x0001, 0x40000, CRC(09174620) SHA1(1d1639c07895f715facfe153fbdb6ae0f3cdd876) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "s3j_char-a1.ic43", 0x0000, 0x200000, CRC(2dbf9d17) SHA1(dd94ddc4bb02ab544aa3f89b614afc46678cc48d) ) /* 42pin MASK ROM */

	ROM_REGION( 0x40000, "oki", 0 ) /* samples for M6295 */
	ROM_LOAD( "s3j_v10.ic75", 0x0000, 0x40000, CRC(f0cdc86a) SHA1(b1017a9841a56e0f5d2714f550f64ed1f4e238e6) )
ROM_END

ROM_START( heberpop )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hbpic31.bin",  0x0000, 0x80000, CRC(c430d264) SHA1(4be12b1fa90da09047db3a31171ffda8ab8bd851) )
	ROM_LOAD16_BYTE( "hbpic32.bin",  0x0001, 0x80000, CRC(bfa555a8) SHA1(754f581554022b98ba8e78ee96f846faa2cedc69) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "hbpic34.bin",  0x0000, 0x10000, CRC(0cf056c6) SHA1(9992cd3879d9a57fcb784fc1e11d6b6d87e5a366) )

	ROM_REGION( 0x380000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD( "hbpic98.bin",  0x000000, 0x80000, CRC(a599100a) SHA1(f2e517256a42b3fa4a047bbe742d714f568cc117) )
	ROM_LOAD( "hbpic99.bin",  0x080000, 0x80000, CRC(fb8bb12f) SHA1(78c1fec1371d312e113d92803dd59acc36604989) )
	ROM_LOAD( "hbpic100.bin", 0x100000, 0x80000, CRC(05a0f765) SHA1(4f44cf367c3697eb6c245297c9d05160d7d94e24) )
	ROM_LOAD( "hbpic101.bin", 0x180000, 0x80000, CRC(151ba025) SHA1(b6ebe60872957a2625e666d53a5a4bc941a1f21c) )
	ROM_LOAD( "hbpic102.bin", 0x200000, 0x80000, CRC(2b5e341a) SHA1(c7ad2dafb3433296c117978434e1699290267891) )
	ROM_LOAD( "hbpic103.bin", 0x280000, 0x80000, CRC(efa0e745) SHA1(fc1d52d35b3c902d8b25403b0e13f86a04039bc4) )
	ROM_LOAD( "hbpic104.bin", 0x300000, 0x80000, CRC(bb896bbb) SHA1(4311876628beb82cbacdab4d055c3738e74241b0) )

	ROM_REGION( 0x80000, "oki", 0 ) /* samples for M6295 */
	ROM_LOAD( "hbpic53.bin",  0x0000, 0x80000, CRC(a4483aa0) SHA1(be301d8ac6d69f5c3fdbcb85bd557090e46da1ff) )
ROM_END

ROM_START( blocken ) /* PCB labeled KID-07 */
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ic31j.bin",    0x0000, 0x20000, CRC(ec8de2a3) SHA1(09a6b8c1b656b17ab3d1fc057902487e4f94cf02) )
	ROM_LOAD16_BYTE( "ic32j.bin",    0x0001, 0x20000, CRC(79b96240) SHA1(c1246bd4b91fa45c581a8fdf90cc6beb85adf8ec) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ic34.bin",     0x0000, 0x10000, CRC(23e446ff) SHA1(82c03b45b337696b0f8293c446544d7ee080d415) )

	ROM_REGION( 0x380000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD( "ic98j.bin",    0x000000, 0x80000, CRC(35dda273) SHA1(95850d12ca1557c14bc471ddf925aaf423313ff0) )
	ROM_LOAD( "ic99j.bin",    0x080000, 0x80000, CRC(ce43762b) SHA1(e1c51ea0b54b5febdee127619e15f1cda650cb4c) )
	/* 100000-1fffff empty */
	ROM_LOAD( "ic100j.bin",   0x200000, 0x80000, CRC(a34786fd) SHA1(7d4879cbaa055c2ddbe6d20dd946bf0e3e069d4d) )
	/* 280000-37ffff empty */

	ROM_REGION( 0x80000, "samples", 0 ) /* samples for M6295 */
	ROM_LOAD( "ic53.bin",     0x0000, 0x80000, CRC(86108c56) SHA1(aa405fa2eec5cc178ef6226f229a12dac09504f0) )

	ROM_REGION( 0x100000, "oki", 0 )
	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_COPY( "samples", 0x000000, 0x000000, 0x020000)
	ROM_COPY( "samples", 0x000000, 0x020000, 0x020000)
	ROM_COPY( "samples", 0x000000, 0x040000, 0x020000)
	ROM_COPY( "samples", 0x020000, 0x060000, 0x020000)
	ROM_COPY( "samples", 0x000000, 0x080000, 0x020000)
	ROM_COPY( "samples", 0x040000, 0x0a0000, 0x020000)
	ROM_COPY( "samples", 0x000000, 0x0c0000, 0x020000)
	ROM_COPY( "samples", 0x060000, 0x0e0000, 0x020000)
ROM_END



DRIVER_INIT_MEMBER(shangha3_state,shangha3)
{
	m_do_shadows = 1;

	save_item(NAME(m_prot_count));
}

DRIVER_INIT_MEMBER(shangha3_state,heberpop)
{
	m_do_shadows = 0;
}

GAME( 1993, shangha3,  0,        shangha3, shangha3, shangha3_state, shangha3, ROT0, "Sunsoft", "Shanghai III (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, shangha3u, shangha3, shangha3, shangha3, shangha3_state, shangha3, ROT0, "Sunsoft", "Shanghai III (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, shangha3j, shangha3, shangha3, shangha3, shangha3_state, shangha3, ROT0, "Sunsoft", "Shanghai III (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, heberpop,  0,        heberpop, heberpop, shangha3_state, heberpop, ROT0, "Sunsoft / Atlus", "Hebereke no Popoon (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, blocken,   0,        blocken,  blocken,  shangha3_state, heberpop, ROT0, "Visco / KID", "Blocken (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
