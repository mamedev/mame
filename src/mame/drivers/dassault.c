/***************************************************************************

  Thunder Zone / Desert Assault (c) 1991 Data East Corporation

  Thunder Zone       (World 2 players, 2 sets)
  Thunder Zone       (Japan 2 players)
  Desert Assault     (USA 4 players)
  Desert Assault     (USA 2 players)

  Emulation by Bryan McPhail, mish@tendril.co.uk

PCB DE-0344-3 (Main board)
 Data East 59 (68000 Main CPU)
 Data East 55 x 2
 Data East 45 (H6280 Sound CPU)
   YM2203C + YM3014B
   YM2151 + YM3012
   OKI M6295 x 2
 32.220MHz OSC
 DSW 8-switch (2 populated, 1 unpopulated)

PCB DE-0345-1 (Sub board)
 Data East 59 (68000 Sub CPU)
 Data East 55 x 2
 28MHz OSC


Stephh's notes (based on the games M68000 code and some tests) :

0) all games

  - If the "Flip Screen" Dip Switch is correct, screen flipping is handled by
    a write to 0x220000.w (dassault_control_1[0]) which isn't supported yet.
  - I can't confirm the "Demo Sounds" Dip Switch, but the routine is the same
    in all games (but different addresses) and it writes something to 0x180000.w
    (see 'WRITE16_HANDLER( dassault_sound_w )').


1) 'thndzone'

  - "Max Players" Dip Switch set to "2" :

      * COIN1    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
      * COIN2    : adds coin(s)/credit(s) depending on "Coin B" Dip Switch
      * COIN3    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
      * COIN4    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
      * SERVICE1 : adds 4 coins/credits

      * START1   : starts a game for player 1
      * START2   : starts a game for player 2

      * BUTTON1n : "fire"
      * BUTTON2n : "nuke"

  - "Max Players" Dip Switch set to "4" :

      * COIN1    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
      * COIN2    : adds coin(s)/credit(s) depending on "Coin B" Dip Switch
      * COIN3    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
      * COIN4    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
      * SERVICE1 : adds 4 coins/credits

      * START1   : starts a game for player 1
      * START2   : starts a game for player 2

      * BUTTON1n : "fire" + starts a game for player n
      * BUTTON2n : "nuke"


2) 'dassault'

  - "Max Players" Dip Switch set to "2" :

      * COIN1    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
      * COIN2    : adds coin(s)/credit(s) depending on "Coin B" Dip Switch
      * COIN3    : NO EFFECT !
      * COIN4    : NO EFFECT !
      * SERVICE1 : adds 1 coin/credit

      * START1   : starts a game for player 1
      * START2   : starts a game for player 2

      * BUTTON1n : "fire"
      * BUTTON2n : "nuke"

  - "Max Players" Dip Switch set to "3" :

      * COIN1    : adds coin(s)/credit(s) for player 1 depending on "Coin A" Dip Switch
      * COIN2    : adds coin(s)/credit(s) for player 2 depending on "Coin A" Dip Switch
      * COIN3    : adds coin(s)/credit(s) for player 3 depending on "Coin A" Dip Switch
      * COIN4    : adds coin(s)/credit(s) for FAKE player 4 depending on "Coin A" Dip Switch
      * SERVICE1 : adds 1 coin/credit for all players (including FAKE player 4 !)

      * START1   : NO EFFECT !
      * START2   : NO EFFECT !

      * BUTTON1n : "fire" + starts a game for player n
      * BUTTON2n : "nuke"

  - "Max Players" Dip Switch set to "4" :

      * COIN1    : adds coin(s)/credit(s) for player 1 depending on "Coin A" Dip Switch
      * COIN2    : adds coin(s)/credit(s) for player 2 depending on "Coin A" Dip Switch
      * COIN3    : adds coin(s)/credit(s) for player 3 depending on "Coin A" Dip Switch
      * COIN4    : adds coin(s)/credit(s) for player 4 depending on "Coin A" Dip Switch
      * SERVICE1 : adds 1 coin/credit for all players

      * START1   : NO EFFECT !
      * START2   : NO EFFECT !

      * BUTTON1n : "fire" + starts a game for player n
      * BUTTON2n : "nuke"


3) 'dassault4'

  - always 4 players :

      * COIN1    : adds coin(s)/credit(s) for player 1 depending on "Coinage" Dip Switch
      * COIN2    : adds coin(s)/credit(s) for player 2 depending on "Coinage" Dip Switch
      * COIN3    : adds coin(s)/credit(s) for player 3 depending on "Coinage" Dip Switch
      * COIN4    : adds coin(s)/credit(s) for player 4 depending on "Coinage" Dip Switch
      * SERVICE1 : adds 1 coin/credit

      * NO START1 !
      * NO START2 !

      * BUTTON1n : "fire" + starts a game for player n
      * BUTTON2n : "nuke"


2008-08
Dip locations verified with US conversion kit manual.

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/h6280/h6280.h"
#include "includes/dassault.h"
#include "sound/2203intf.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "video/deco16ic.h"
#include "video/decocomn.h"
#include "video/decospr.h"

/**********************************************************************************/

READ16_MEMBER(dassault_state::dassault_control_r)
{
	switch (offset << 1)
	{
		case 0: /* Player 1 & Player 2 joysticks & fire buttons */
			return ioport("P1_P2")->read();

		case 2: /* Player 3 & Player 4 joysticks & fire buttons */
			return ioport("P3_P4")->read();

		case 4: /* Dip 1 (stored at 0x3f8035) */
			return ioport("DSW1")->read();

		case 6: /* Dip 2 (stored at 0x3f8034) */
			return ioport("DSW2")->read();

		case 8: /* VBL, Credits */
			return ioport("SYSTEM")->read();
	}

	return 0xffff;
}

WRITE16_MEMBER(dassault_state::dassault_control_w)
{
	coin_counter_w(machine(), 0, data & 1);
	if (data & 0xfffe)
		logerror("Coin cointrol %04x\n", data);
}

READ16_MEMBER(dassault_state::dassault_sub_control_r)
{
	return ioport("VBLANK1")->read();
}

WRITE16_MEMBER(dassault_state::dassault_sound_w)
{
	soundlatch_byte_w(space, 0, data & 0xff);
	device_set_input_line(m_audiocpu, 0, HOLD_LINE); /* IRQ1 */
}

/* The CPU-CPU irq controller is overlaid onto the end of the shared memory */
READ16_MEMBER(dassault_state::dassault_irq_r)
{
	switch (offset)
	{
	case 0: device_set_input_line(m_maincpu, 5, CLEAR_LINE); break;
	case 1: device_set_input_line(m_subcpu, 6, CLEAR_LINE); break;
	}
	return m_shared_ram[(0xffc / 2) + offset]; /* The values probably don't matter */
}

WRITE16_MEMBER(dassault_state::dassault_irq_w)
{
	switch (offset)
	{
	case 0: device_set_input_line(m_maincpu, 5, ASSERT_LINE); break;
	case 1: device_set_input_line(m_subcpu, 6, ASSERT_LINE); break;
	}

	COMBINE_DATA(&m_shared_ram[(0xffc / 2) + offset]); /* The values probably don't matter */
}

WRITE16_MEMBER(dassault_state::shared_ram_w)
{
	COMBINE_DATA(&m_shared_ram[offset]);
}

READ16_MEMBER(dassault_state::shared_ram_r)
{
	return m_shared_ram[offset];
}

/**********************************************************************************/

static ADDRESS_MAP_START( dassault_map, AS_PROGRAM, 16, dassault_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM

	AM_RANGE(0x100000, 0x103fff) AM_RAM_DEVWRITE_LEGACY("deco_common", decocomn_nonbuffered_palette_w) AM_SHARE("paletteram")

	AM_RANGE(0x140004, 0x140007) AM_WRITENOP /* ? */
	AM_RANGE(0x180000, 0x180001) AM_WRITE(dassault_sound_w)

	AM_RANGE(0x1c0000, 0x1c000f) AM_READ(dassault_control_r)
	AM_RANGE(0x1c000a, 0x1c000b) AM_DEVWRITE_LEGACY("deco_common", decocomn_priority_w)
	AM_RANGE(0x1c000c, 0x1c000d) AM_DEVWRITE("spriteram2", buffered_spriteram16_device, write)
	AM_RANGE(0x1c000e, 0x1c000f) AM_WRITE(dassault_control_w)

	AM_RANGE(0x200000, 0x201fff) AM_DEVREADWRITE_LEGACY("tilegen1", deco16ic_pf1_data_r, deco16ic_pf1_data_w)
	AM_RANGE(0x202000, 0x203fff) AM_DEVREADWRITE_LEGACY("tilegen1", deco16ic_pf2_data_r, deco16ic_pf2_data_w)
	AM_RANGE(0x212000, 0x212fff) AM_WRITEONLY AM_SHARE("pf2_rowscroll")
	AM_RANGE(0x220000, 0x22000f) AM_DEVWRITE_LEGACY("tilegen1", deco16ic_pf_control_w)

	AM_RANGE(0x240000, 0x240fff) AM_DEVREADWRITE_LEGACY("tilegen2", deco16ic_pf1_data_r, deco16ic_pf1_data_w)
	AM_RANGE(0x242000, 0x242fff) AM_DEVREADWRITE_LEGACY("tilegen2", deco16ic_pf2_data_r, deco16ic_pf2_data_w)
	AM_RANGE(0x252000, 0x252fff) AM_WRITEONLY AM_SHARE("pf4_rowscroll")
	AM_RANGE(0x260000, 0x26000f) AM_DEVWRITE_LEGACY("tilegen2", deco16ic_pf_control_w)

	AM_RANGE(0x3f8000, 0x3fbfff) AM_RAM AM_SHARE("ram") /* Main ram */
	AM_RANGE(0x3fc000, 0x3fcfff) AM_RAM AM_SHARE("spriteram2") /* Spriteram (2nd) */
	AM_RANGE(0x3feffc, 0x3fefff) AM_READWRITE(dassault_irq_r, dassault_irq_w)
	AM_RANGE(0x3fe000, 0x3fefff) AM_READWRITE(shared_ram_r, shared_ram_w) AM_SHARE("shared_ram") /* Shared ram */
ADDRESS_MAP_END

static ADDRESS_MAP_START( dassault_sub_map, AS_PROGRAM, 16, dassault_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM

	AM_RANGE(0x100000, 0x100001) AM_DEVWRITE("spriteram", buffered_spriteram16_device, write)
	AM_RANGE(0x100002, 0x100007) AM_WRITENOP /* ? */
	AM_RANGE(0x100004, 0x100005) AM_READ(dassault_sub_control_r)

	AM_RANGE(0x3f8000, 0x3fbfff) AM_RAM AM_SHARE("ram2") /* Sub cpu ram */
	AM_RANGE(0x3fc000, 0x3fcfff) AM_RAM AM_SHARE("spriteram") /* Sprite ram */
	AM_RANGE(0x3feffc, 0x3fefff) AM_READWRITE(dassault_irq_r, dassault_irq_w)
	AM_RANGE(0x3fe000, 0x3fefff) AM_READWRITE(shared_ram_r, shared_ram_w)
ADDRESS_MAP_END

/******************************************************************************/

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, dassault_state )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x100000, 0x100001) AM_DEVREADWRITE_LEGACY("ym1", ym2203_r, ym2203_w)
	AM_RANGE(0x110000, 0x110001) AM_DEVREADWRITE_LEGACY("ym2", ym2151_r, ym2151_w)
	AM_RANGE(0x120000, 0x120001) AM_DEVREADWRITE("oki1", okim6295_device, read, write)
	AM_RANGE(0x130000, 0x130001) AM_DEVREADWRITE("oki2", okim6295_device, read, write)
	AM_RANGE(0x140000, 0x140001) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x1f0000, 0x1f1fff) AM_RAMBANK("bank8")
	AM_RANGE(0x1fec00, 0x1fec01) AM_WRITE_LEGACY(h6280_timer_w)
	AM_RANGE(0x1ff400, 0x1ff403) AM_WRITE_LEGACY(h6280_irq_status_w)
ADDRESS_MAP_END

/**********************************************************************************/

static INPUT_PORTS_START( common )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P3_P4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN4 )

	PORT_START("VBLANK1") /* Cpu 1 vblank */
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

static INPUT_PORTS_START( thndzone )
	PORT_INCLUDE( common )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )			// Adds 4 credits/coins !
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "2 Coins to Start, 1 to Continue" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" )	/* OFF & Not to be changed, according to manual */
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:2" )	/* OFF & Not to be changed, according to manual */
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )	/* OFF & Not to be changed, according to manual */
	PORT_DIPNAME( 0x20, 0x20, "Max Players" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )	/* OFF & Not to be changed, according to manual */
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")	// Check code at 0x001490
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dassault )
	PORT_INCLUDE( common )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "2 Coins to Start, 1 to Continue" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, "Max Players" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x10, "4" )
//  PORT_DIPSETTING(    0x00, "4 (buggy)" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")	// Check code at 0x0014bc
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dassault4 )
	PORT_INCLUDE( common )

	PORT_MODIFY("P1_P2")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_6C ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "2 Coins to Start, 1 to Continue" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")	// Check code at 0x0014a4
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/**********************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ 32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
		0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};

static GFXDECODE_START( dassault )
	/* "gfx1" is copied to "gfx2" at runtime */
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,     0,  32 )	/* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,     0,  32 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout,   512,  32 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx4", 0, tilelayout,  0/*1024*/,  64 )	/* Sprites 16x16 */
	GFXDECODE_ENTRY( "gfx5", 0, tilelayout,  0/*2048*/,  64 )	/* Sprites 16x16 */
GFXDECODE_END

/**********************************************************************************/

static void sound_irq(device_t *device, int state)
{
	dassault_state *driver_state = device->machine().driver_data<dassault_state>();
	device_set_input_line(driver_state->m_audiocpu, 1, state);
}

WRITE8_MEMBER(dassault_state::sound_bankswitch_w)
{

	/* the second OKIM6295 ROM is bank switched */
	m_oki2->set_bank_base((data & 1) * 0x40000);
}

static const ym2151_interface ym2151_config =
{
	DEVCB_LINE(sound_irq),
	DEVCB_DRIVER_MEMBER(dassault_state,sound_bankswitch_w)
};

/**********************************************************************************/

static const decocomn_interface dassault_decocomn_intf =
{
	"screen",
};

static int dassault_bank_callback( const int bank )
{
	return ((bank >> 4) & 0xf) << 12;
}

static const deco16ic_interface dassault_deco16ic_tilegen1_intf =
{
	"screen",
	0, 1,
	0x0f, 0x0f,	/* trans masks (default values) */
	0, 16, /* color base (default values) */
	0x0f, 0x0f, /* color masks (default values) */
	dassault_bank_callback,
	dassault_bank_callback,
	0,1,
};

static const deco16ic_interface dassault_deco16ic_tilegen2_intf =
{
	"screen",
	0, 1,
	0x0f, 0x0f,	/* trans masks (default values) */
	0, 16, /* color base (default values) */
	0x0f, 0x0f,	/* color masks (default values) */
	dassault_bank_callback,
	dassault_bank_callback,
	0,2,
};


static MACHINE_CONFIG_START( dassault, dassault_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_28MHz/2)	/* 14MHz - Accurate */
	MCFG_CPU_PROGRAM_MAP(dassault_map)
	MCFG_CPU_VBLANK_INT("screen", irq4_line_hold)

	MCFG_CPU_ADD("sub", M68000, XTAL_28MHz/2)	/* 14MHz - Accurate */
	MCFG_CPU_PROGRAM_MAP(dassault_sub_map)
	MCFG_CPU_VBLANK_INT("screen", irq5_line_hold)

	MCFG_CPU_ADD("audiocpu", H6280, XTAL_32_22MHz/8)	/* Accurate */
	MCFG_CPU_PROGRAM_MAP(sound_map)

//  MCFG_QUANTUM_TIME(attotime::from_hz(8400)) /* 140 CPU slices per frame */
	MCFG_QUANTUM_PERFECT_CPU("maincpu") // I was seeing random lockups.. let's see if this helps

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_STATIC(dassault)

	MCFG_VIDEO_START(dassault)

	MCFG_GFXDECODE(dassault)
	MCFG_PALETTE_LENGTH(4096)

	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")
	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram2")

	MCFG_DECOCOMN_ADD("deco_common", dassault_decocomn_intf)

	MCFG_DECO16IC_ADD("tilegen1", dassault_deco16ic_tilegen1_intf)
	MCFG_DECO16IC_ADD("tilegen2", dassault_deco16ic_tilegen2_intf)

	MCFG_DEVICE_ADD("spritegen1", DECO_SPRITE, 0)
	decospr_device::set_gfx_region(*device, 3);

	MCFG_DEVICE_ADD("spritegen2", DECO_SPRITE, 0)
	decospr_device::set_gfx_region(*device, 4);


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ym1", YM2203, XTAL_32_22MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.40)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.40)

	MCFG_SOUND_ADD("ym2", YM2151, XTAL_32_22MHz/9)
	MCFG_SOUND_CONFIG(ym2151_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.45)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.45)

	MCFG_OKIM6295_ADD("oki1", XTAL_32_22MHz/32, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified - was 1.023924MHz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MCFG_OKIM6295_ADD("oki2", XTAL_32_22MHz/16, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified - was 2.047848MHz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.25)
MACHINE_CONFIG_END

/**********************************************************************************/

ROM_START( thndzone )
	ROM_REGION(0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE("gz01-1.a15", 0x00000, 0x20000, CRC(20250da6) SHA1(2d01d59b67a2ecc2ddc88eded43f451931a0a33b) )
	ROM_LOAD16_BYTE("gz03-1.a17", 0x00001, 0x20000, CRC(3595fad0) SHA1(5d61776cdf2274cb26ea06ce97c35f5ce7f27e66) )
	ROM_LOAD16_BYTE("gt00.a14",   0x40000, 0x20000, CRC(b7277175) SHA1(ffb19c4dd12e0391f01de57c46a7998885fe22bf) ) /* Same data as GS00.A14 */
	ROM_LOAD16_BYTE("gt02.a16",   0x40001, 0x20000, CRC(cde31e35) SHA1(0219845308c9f46e73b0504bd2aefa2fa74f388e) ) /* Same data as GS02.A16 */

	ROM_REGION(0x80000, "sub", 0 ) /* 68000 code (Sub cpu) */
	ROM_LOAD16_BYTE("gz10-1.a12", 0x00000, 0x20000, CRC(811d86d7) SHA1(94971acad2c648b7a65ca54d9315414ed3b94f24) )
	ROM_LOAD16_BYTE("gz08-1.a9",  0x00001, 0x20000, CRC(8f61ab1e) SHA1(df4e7db889915eca39ed4e1a4b5fcae9cd1a9882) )
	ROM_LOAD16_BYTE("gt11-1.a14", 0x40000, 0x20000, CRC(80cb23de) SHA1(d52426460eea2285c57cfc3fe37aa6dc79990e25) ) /* Same data as GS11.A14 */
	ROM_LOAD16_BYTE("gt09-1.a11", 0x40001, 0x20000, CRC(0a8fa7e1) SHA1(330ae9602b5f56b5dc4961a41991b64412a59880) ) /* Same data as GS09.A11 */

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "gt04.f18",    0x00000, 0x10000, CRC(81c29ebf) SHA1(1b241277a8e35cdeaeb120970d14a09d33032459) ) /* Same data for all regions, different label */

	ROM_REGION(0x020000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "gt05.h11", 0x000000, 0x10000, CRC(0aae996a) SHA1(d37a12b057e9934212362d7eafa575c961819a27) ) /* Same data for all regions, different label */
	ROM_LOAD16_BYTE( "gt06.h12", 0x000001, 0x10000, CRC(4efdf03d) SHA1(835d22829c6d0f4efc76801b449f9a779f460f1c) ) /* Same data for all regions, different label */

	ROM_REGION(0x120000, "gfx2", 0 )
	ROM_LOAD( "maj-02.h14", 0x000000, 0x100000, CRC(383bbc37) SHA1(c537ab147a2770ce28ee185b08dd62d35249bfa9) )
	/* Other 0x20000 filled in later */

	ROM_REGION(0x200000, "gfx3", 0 )
	ROM_LOAD( "maj-01.c18", 0x000000, 0x100000, CRC(9840a204) SHA1(096c351769da5184c3d9a05495370134acc9507a) )
	ROM_LOAD( "maj-00.c17", 0x100000, 0x100000, CRC(87ea8d16) SHA1(db47123aa2ebbb800cfc5cfcf50309bc39cadbcd) )

	ROM_REGION( 0x400000, "gfx4", 0 ) /* sprites chip 1 */
	ROM_LOAD( "maj-04.r1",  0x000000, 0x80000, CRC(36e49b19) SHA1(bfbc45b635bf3d46ff8b8a514a3f352bf3a95535) )
	ROM_LOAD( "maj-05.r2",  0x080000, 0x80000, CRC(80fc71cc) SHA1(65b15afbe5d628051b012777d486b6ce92a3795c) )
	ROM_LOAD( "maj-06.r3",  0x100000, 0x80000, CRC(2e7a684b) SHA1(cffeda1a816dad30d6b1cb12458661188d625d40) )
	ROM_LOAD( "maj-07.r5",  0x180000, 0x80000, CRC(3acc1f78) SHA1(87ec65b4f54a66370754534d03f4c9217531b42f) )
	ROM_LOAD( "maj-08.s6",  0x200000, 0x80000, CRC(1958a36d) SHA1(466a30dcd2ea13028272ed2187f890ee20d6636b) )
	ROM_LOAD( "maj-09.s8",  0x280000, 0x80000, CRC(c21087a1) SHA1(b769c5f2f9b9c525d121902fe9557a6bfc077b99) )
	ROM_LOAD( "maj-10.s9",  0x300000, 0x80000, CRC(a02fa641) SHA1(14b999a441964e612700bf21945a948eaebb253e) )
	ROM_LOAD( "maj-11.s11", 0x380000, 0x80000, CRC(dabe9305) SHA1(44d69fe55e674de7f4c610d295d4528d4b2eb150) )

	ROM_REGION( 0x80000, "gfx5", 0 ) /* sprites chip 2 */
	ROM_LOAD16_BYTE( "gt12.n1", 0x000000, 0x20000, CRC(9a86a015) SHA1(968576b8422393ab9a93d98c15428b1c11417b3d) )
	ROM_LOAD16_BYTE( "gt13.n2", 0x000001, 0x20000, CRC(f4709905) SHA1(697842a3d7bc2588c77833c3af8938e6f0b1238d) )
	ROM_LOAD16_BYTE( "gt14.n3", 0x040000, 0x20000, CRC(750fc523) SHA1(ef8794359ff3a44a97ab402821fbe205a0be8f6a) )
	ROM_LOAD16_BYTE( "gt15.n5", 0x040001, 0x20000, CRC(f14edd3d) SHA1(802d576df6dac2c9bf99f963f1955fc3a7ffdac0) )

	ROM_REGION(0x40000, "oki1", 0 ) /* Oki samples */
	ROM_LOAD( "gt07.h15",  0x00000,  0x20000,  CRC(750b7e5d) SHA1(d33b17a1d8c9b05d5c1daf0c80fed6381e04b167) ) /* Same data as GS07.H15 */

	ROM_REGION(0x80000, "oki2", 0 ) /* Extra Oki samples */
	ROM_LOAD( "maj-03.h16", 0x00000, 0x80000,  CRC(31dcfac3) SHA1(88c7fc139f871991defbc8dc2c9c66b150dd6f6f) )	/* banked */

	ROM_REGION( 0x1000, "proms", 0 )
	ROM_LOAD( "mb7128y.10m", 0x00000,  0x800,  CRC(bde780a2) SHA1(94ea9fe6c3a421e976d077e67f564ca5c37a5e88) )	/* Priority?  Unused */
	ROM_LOAD( "mb7128y.16p", 0x00800,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )	/* Timing??  Unused */
	/* Above prom also at 16s and 17s */

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "pal16r8a 1h",  0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.7c",  0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.7d",  0x0400, 0x0104, CRC(199e83fd) SHA1(ebb5d66f29935b0a58e79b0db30611b5dce328a6) ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.7e",  0x0600, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.7l",  0x0800, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.8e",  0x0a00, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.9d",  0x0c00, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.10c", 0x0e00, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( thndzonea )
	ROM_REGION(0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE("gz01.a15", 0x00000, 0x20000, CRC(15e8c328) SHA1(8876b5fde77604c2fe4654271ceb341a8fa460c1) )
	ROM_LOAD16_BYTE("gz03.a17", 0x00001, 0x20000, CRC(aab5c86e) SHA1(c3560b15360ddf14e8444d9f70724e698b2bd42f) )
	ROM_LOAD16_BYTE("gt00.a14", 0x40000, 0x20000, CRC(b7277175) SHA1(ffb19c4dd12e0391f01de57c46a7998885fe22bf) ) /* Same data as GS00.A14 */
	ROM_LOAD16_BYTE("gt02.a16", 0x40001, 0x20000, CRC(cde31e35) SHA1(0219845308c9f46e73b0504bd2aefa2fa74f388e) ) /* Same data as GS02.A16 */

	ROM_REGION(0x80000, "sub", 0 ) /* 68000 code (Sub cpu) */
	ROM_LOAD16_BYTE("gz10.a12",   0x00000, 0x20000, CRC(79f919e9) SHA1(b6793173e310b1df07cf3e9209da1fbec3a8a05b) )
	ROM_LOAD16_BYTE("gz08.a9",    0x00001, 0x20000, CRC(d47d7836) SHA1(8a5d3e8b89f5dfd6bac83f7b093ddb03d5ecef73) )
	ROM_LOAD16_BYTE("gt11-1.a14", 0x40000, 0x20000, CRC(80cb23de) SHA1(d52426460eea2285c57cfc3fe37aa6dc79990e25) ) /* Same data as GS11.A14 */
	ROM_LOAD16_BYTE("gt09-1.a11", 0x40001, 0x20000, CRC(0a8fa7e1) SHA1(330ae9602b5f56b5dc4961a41991b64412a59880) ) /* Same data as GS09.A11 */

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "gt04.f18",    0x00000, 0x10000, CRC(81c29ebf) SHA1(1b241277a8e35cdeaeb120970d14a09d33032459) ) /* Same data for all regions, different label */

	ROM_REGION(0x020000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "gt05.h11", 0x000000, 0x10000, CRC(0aae996a) SHA1(d37a12b057e9934212362d7eafa575c961819a27) ) /* Same data for all regions, different label */
	ROM_LOAD16_BYTE( "gt06.h12", 0x000001, 0x10000, CRC(4efdf03d) SHA1(835d22829c6d0f4efc76801b449f9a779f460f1c) ) /* Same data for all regions, different label */

	ROM_REGION(0x120000, "gfx2", 0 )
	ROM_LOAD( "maj-02.h14", 0x000000, 0x100000, CRC(383bbc37) SHA1(c537ab147a2770ce28ee185b08dd62d35249bfa9) )
	/* Other 0x20000 filled in later */

	ROM_REGION(0x200000, "gfx3", 0 )
	ROM_LOAD( "maj-01.c18", 0x000000, 0x100000, CRC(9840a204) SHA1(096c351769da5184c3d9a05495370134acc9507a) )
	ROM_LOAD( "maj-00.c17", 0x100000, 0x100000, CRC(87ea8d16) SHA1(db47123aa2ebbb800cfc5cfcf50309bc39cadbcd) )

	ROM_REGION( 0x400000, "gfx4", 0 ) /* sprites chip 1 */
	ROM_LOAD( "maj-04.r1",  0x000000, 0x80000, CRC(36e49b19) SHA1(bfbc45b635bf3d46ff8b8a514a3f352bf3a95535) )
	ROM_LOAD( "maj-05.r2",  0x080000, 0x80000, CRC(80fc71cc) SHA1(65b15afbe5d628051b012777d486b6ce92a3795c) )
	ROM_LOAD( "maj-06.r3",  0x100000, 0x80000, CRC(2e7a684b) SHA1(cffeda1a816dad30d6b1cb12458661188d625d40) )
	ROM_LOAD( "maj-07.r5",  0x180000, 0x80000, CRC(3acc1f78) SHA1(87ec65b4f54a66370754534d03f4c9217531b42f) )
	ROM_LOAD( "maj-08.s6",  0x200000, 0x80000, CRC(1958a36d) SHA1(466a30dcd2ea13028272ed2187f890ee20d6636b) )
	ROM_LOAD( "maj-09.s8",  0x280000, 0x80000, CRC(c21087a1) SHA1(b769c5f2f9b9c525d121902fe9557a6bfc077b99) )
	ROM_LOAD( "maj-10.s9",  0x300000, 0x80000, CRC(a02fa641) SHA1(14b999a441964e612700bf21945a948eaebb253e) )
	ROM_LOAD( "maj-11.s11", 0x380000, 0x80000, CRC(dabe9305) SHA1(44d69fe55e674de7f4c610d295d4528d4b2eb150) )

	ROM_REGION( 0x80000, "gfx5", 0 ) /* sprites chip 2 */
	ROM_LOAD16_BYTE( "gt12.n1", 0x000000, 0x20000, CRC(9a86a015) SHA1(968576b8422393ab9a93d98c15428b1c11417b3d) )
	ROM_LOAD16_BYTE( "gt13.n2", 0x000001, 0x20000, CRC(f4709905) SHA1(697842a3d7bc2588c77833c3af8938e6f0b1238d) )
	ROM_LOAD16_BYTE( "gt14.n3", 0x040000, 0x20000, CRC(750fc523) SHA1(ef8794359ff3a44a97ab402821fbe205a0be8f6a) )
	ROM_LOAD16_BYTE( "gt15.n5", 0x040001, 0x20000, CRC(f14edd3d) SHA1(802d576df6dac2c9bf99f963f1955fc3a7ffdac0) )

	ROM_REGION(0x40000, "oki1", 0 ) /* Oki samples */
	ROM_LOAD( "gt07.h15",  0x00000,  0x20000,  CRC(750b7e5d) SHA1(d33b17a1d8c9b05d5c1daf0c80fed6381e04b167) ) /* Same data as GS07.H15 */

	ROM_REGION(0x80000, "oki2", 0 ) /* Extra Oki samples */
	ROM_LOAD( "maj-03.h16", 0x00000, 0x80000,  CRC(31dcfac3) SHA1(88c7fc139f871991defbc8dc2c9c66b150dd6f6f) )	/* banked */

	ROM_REGION( 0x2000, "proms", 0 )
	ROM_LOAD( "mb7128y.10m", 0x00000,  0x800,  CRC(bde780a2) SHA1(94ea9fe6c3a421e976d077e67f564ca5c37a5e88) )	/* Priority?  Unused */
	ROM_LOAD( "mb7128y.16p", 0x00800,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )	/* Unknown,  Unused */
	ROM_LOAD( "mb7128y.16s", 0x01000,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )	/* Unknown,  Unused */
	ROM_LOAD( "mb7128y.17s", 0x01800,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )	/* Unknown,  Unused - Data identical for 3 proms! */

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "pal16r8a 1h",  0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.7c",  0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.7d",  0x0400, 0x0104, CRC(199e83fd) SHA1(ebb5d66f29935b0a58e79b0db30611b5dce328a6) ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.7e",  0x0600, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.7l",  0x0800, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.8e",  0x0a00, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.9d",  0x0c00, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.10c", 0x0e00, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( thndzonej )
	ROM_REGION(0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE("gu01.a15", 0x00000, 0x20000, CRC(eb28f8e8) SHA1(834f89db3ef48a71d20c0ec3a0c2231e115d7f48) )
	ROM_LOAD16_BYTE("gu03.a17", 0x00001, 0x20000, CRC(9ad2b431) SHA1(c2fb88b4d2df93e3f787fe49c240573e1bc2844e) )
	ROM_LOAD16_BYTE("gu00.a14", 0x40000, 0x20000, CRC(fca9e84f) SHA1(a0ecf99eace7357b05da8f8fe06b9bbf7d16d95a) )
	ROM_LOAD16_BYTE("gu02.a16", 0x40001, 0x20000, CRC(b6026bae) SHA1(673b7f7a432580ec1780d1efa2b48184af428698) )

	ROM_REGION(0x80000, "sub", 0 ) /* 68000 code (Sub cpu) */
	ROM_LOAD16_BYTE("gu10.a12", 0x00000, 0x20000, CRC(8042e87d) SHA1(dc69b13fc06d94a2bc5569e96931e6d9496bd44f) )
	ROM_LOAD16_BYTE("gu08.a9",  0x00001, 0x20000, CRC(c8895bfa) SHA1(6a5421bd926e0aa86c81e345f2dfe5265bd3add2) )
	ROM_LOAD16_BYTE("gu11.a14", 0x40000, 0x20000, CRC(c0d6eb82) SHA1(44070e6d37f5327cf7f647e44ea49a1fe6844e5e) )
	ROM_LOAD16_BYTE("gu09.a11", 0x40001, 0x20000, CRC(42de13a7) SHA1(f948d31e368499fd8c35da0c7dd7519cfbd4b5f7) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "gu04.f18",    0x00000, 0x10000, CRC(81c29ebf) SHA1(1b241277a8e35cdeaeb120970d14a09d33032459) ) /* Same data for all regions, different label */

	ROM_REGION(0x020000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "gu05.h11", 0x000000, 0x10000, CRC(0aae996a) SHA1(d37a12b057e9934212362d7eafa575c961819a27) ) /* Same data for all regions, different label */
	ROM_LOAD16_BYTE( "gu06.h12", 0x000001, 0x10000, CRC(4efdf03d) SHA1(835d22829c6d0f4efc76801b449f9a779f460f1c) ) /* Same data for all regions, different label */

	ROM_REGION(0x120000, "gfx2", 0 )
	ROM_LOAD( "maj-02.h14", 0x000000, 0x100000, CRC(383bbc37) SHA1(c537ab147a2770ce28ee185b08dd62d35249bfa9) )
	/* Other 0x20000 filled in later */

	ROM_REGION(0x200000, "gfx3", 0 )
	ROM_LOAD( "maj-01.c18", 0x000000, 0x100000, CRC(9840a204) SHA1(096c351769da5184c3d9a05495370134acc9507a) )
	ROM_LOAD( "maj-00.c17", 0x100000, 0x100000, CRC(87ea8d16) SHA1(db47123aa2ebbb800cfc5cfcf50309bc39cadbcd) )

	ROM_REGION( 0x400000, "gfx4", 0 ) /* sprites chip 1 */
	ROM_LOAD( "maj-04.r1",  0x000000, 0x80000, CRC(36e49b19) SHA1(bfbc45b635bf3d46ff8b8a514a3f352bf3a95535) )
	ROM_LOAD( "maj-05.r2",  0x080000, 0x80000, CRC(80fc71cc) SHA1(65b15afbe5d628051b012777d486b6ce92a3795c) )
	ROM_LOAD( "maj-06.r3",  0x100000, 0x80000, CRC(2e7a684b) SHA1(cffeda1a816dad30d6b1cb12458661188d625d40) )
	ROM_LOAD( "maj-07.r5",  0x180000, 0x80000, CRC(3acc1f78) SHA1(87ec65b4f54a66370754534d03f4c9217531b42f) )
	ROM_LOAD( "maj-08.s6",  0x200000, 0x80000, CRC(1958a36d) SHA1(466a30dcd2ea13028272ed2187f890ee20d6636b) )
	ROM_LOAD( "maj-09.s8",  0x280000, 0x80000, CRC(c21087a1) SHA1(b769c5f2f9b9c525d121902fe9557a6bfc077b99) )
	ROM_LOAD( "maj-10.s9",  0x300000, 0x80000, CRC(a02fa641) SHA1(14b999a441964e612700bf21945a948eaebb253e) )
	ROM_LOAD( "maj-11.s11", 0x380000, 0x80000, CRC(dabe9305) SHA1(44d69fe55e674de7f4c610d295d4528d4b2eb150) )

	ROM_REGION( 0x80000, "gfx5", 0 ) /* sprites chip 2 */
	/* Although the other Mask ROMs on the PCB are MAJ-xx, for the Japan version, these 4 are actually MAL-xx */
	ROM_LOAD16_BYTE( "mal-12.n1", 0x000000, 0x20000, NO_DUMP ) /* Mask ROM - Need to verify if these are the same or different as the other sets */
	ROM_LOAD16_BYTE( "mal-13.n2", 0x000001, 0x20000, NO_DUMP ) /* Mask ROM - Need to verify if these are the same or different as the other sets */
	ROM_LOAD16_BYTE( "mal-14.n3", 0x040000, 0x20000, NO_DUMP ) /* Mask ROM - Need to verify if these are the same or different as the other sets */
	ROM_LOAD16_BYTE( "mal-15.n5", 0x040001, 0x20000, NO_DUMP ) /* Mask ROM - Need to verify if these are the same or different as the other sets */
	ROM_LOAD16_BYTE( "gt12.n1", 0x000000, 0x20000, CRC(9a86a015) SHA1(968576b8422393ab9a93d98c15428b1c11417b3d) ) /* REMOVE when MAL-12.N1 is dumped & added */
	ROM_LOAD16_BYTE( "gt13.n2", 0x000001, 0x20000, CRC(f4709905) SHA1(697842a3d7bc2588c77833c3af8938e6f0b1238d) ) /* REMOVE when MAL-13.N2 is dumped & added */
	ROM_LOAD16_BYTE( "gt14.n3", 0x040000, 0x20000, CRC(750fc523) SHA1(ef8794359ff3a44a97ab402821fbe205a0be8f6a) ) /* REMOVE when MAL-14.N3 is dumped & added */
	ROM_LOAD16_BYTE( "gt15.n5", 0x040001, 0x20000, CRC(f14edd3d) SHA1(802d576df6dac2c9bf99f963f1955fc3a7ffdac0) ) /* REMOVE when MAL-15.N5 is dumped & added */

	ROM_REGION(0x40000, "oki1", 0 ) /* Oki samples */
	/* This rom is also a Mask ROM label MAL-07 and _NOT_ MAJ-07 */
	ROM_LOAD( "mal-07.h15",  0x00000,  0x20000, NO_DUMP ) /* Mask ROM - Need to verify if these are the same or different as the other sets */
	ROM_LOAD( "gs07.h15",  0x00000,  0x20000,  CRC(750b7e5d) SHA1(d33b17a1d8c9b05d5c1daf0c80fed6381e04b167) ) /* REMOVE when MAL-07.H15 is dumped & added */

	ROM_REGION(0x80000, "oki2", 0 ) /* Extra Oki samples */
	ROM_LOAD( "maj-03.h16", 0x00000, 0x80000,  CRC(31dcfac3) SHA1(88c7fc139f871991defbc8dc2c9c66b150dd6f6f) )	/* banked */

	ROM_REGION( 0x2000, "proms", 0 )
	ROM_LOAD( "mb7128y.10m", 0x00000,  0x800,  CRC(bde780a2) SHA1(94ea9fe6c3a421e976d077e67f564ca5c37a5e88) )	/* Priority?  Unused */
	ROM_LOAD( "mb7128y.16p", 0x00800,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )	/* Unknown,  Unused */
	ROM_LOAD( "mb7128y.16s", 0x01000,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )	/* Unknown,  Unused */
	ROM_LOAD( "mb7128y.17s", 0x01800,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )	/* Unknown,  Unused - Data identical for 3 proms! */

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "pal16r8a 1h",  0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.7c",  0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.7d",  0x0400, 0x0104, CRC(199e83fd) SHA1(ebb5d66f29935b0a58e79b0db30611b5dce328a6) ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.7e",  0x0600, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.7l",  0x0800, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.8e",  0x0a00, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.9d",  0x0c00, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.10c", 0x0e00, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( dassault )
	ROM_REGION(0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE("01.a15",   0x00000, 0x20000, CRC(14f17ea7) SHA1(0bb8b7dba05f1ea42e68838861f0d4c263eac6b3) )
	ROM_LOAD16_BYTE("03.a17",   0x00001, 0x20000, CRC(bed1b90c) SHA1(c100f89b69025e2ff885b35a733abc627da98a07) )
	ROM_LOAD16_BYTE("gs00.a14", 0x40000, 0x20000, CRC(b7277175) SHA1(ffb19c4dd12e0391f01de57c46a7998885fe22bf) ) /* Same data as GT00.A14 */
	ROM_LOAD16_BYTE("gs02.a16", 0x40001, 0x20000, CRC(cde31e35) SHA1(0219845308c9f46e73b0504bd2aefa2fa74f388e) ) /* Same data as GT02.A16 */

	ROM_REGION(0x80000, "sub", 0 ) /* 68000 code (Sub cpu) */
	ROM_LOAD16_BYTE("hc10-1.a12", 0x00000, 0x20000, CRC(ac5ac770) SHA1(bf6640900c2f9c8091168bf106edf85350c34652) )
	ROM_LOAD16_BYTE("hc08-1.a9",  0x00001, 0x20000, CRC(864dca56) SHA1(0967f613684b539d10b67e4f6033c890e2134ea2) )
	ROM_LOAD16_BYTE("gs11.a14",   0x40000, 0x20000, CRC(80cb23de) SHA1(d52426460eea2285c57cfc3fe37aa6dc79990e25) ) /* Same data as GT11-1.A14 */
	ROM_LOAD16_BYTE("gs09.a11",   0x40001, 0x20000, CRC(0a8fa7e1) SHA1(330ae9602b5f56b5dc4961a41991b64412a59880) ) /* Same data as GT09-1.A11 */

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "gs04.f18",    0x00000, 0x10000, CRC(81c29ebf) SHA1(1b241277a8e35cdeaeb120970d14a09d33032459) ) /* Same data for all regions, different label */

	ROM_REGION(0x020000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "gs05.h11", 0x000000, 0x10000, CRC(0aae996a) SHA1(d37a12b057e9934212362d7eafa575c961819a27) ) /* Same data for all regions, different label */
	ROM_LOAD16_BYTE( "gs06.h12", 0x000001, 0x10000, CRC(4efdf03d) SHA1(835d22829c6d0f4efc76801b449f9a779f460f1c) ) /* Same data for all regions, different label */

	ROM_REGION(0x120000, "gfx2", 0 )
	ROM_LOAD( "maj-02.h14", 0x000000, 0x100000, CRC(383bbc37) SHA1(c537ab147a2770ce28ee185b08dd62d35249bfa9) )
	/* Other 0x20000 filled in later */

	ROM_REGION(0x200000, "gfx3", 0 )
	ROM_LOAD( "maj-01.c18", 0x000000, 0x100000, CRC(9840a204) SHA1(096c351769da5184c3d9a05495370134acc9507a) )
	ROM_LOAD( "maj-00.c17", 0x100000, 0x100000, CRC(87ea8d16) SHA1(db47123aa2ebbb800cfc5cfcf50309bc39cadbcd) )

	ROM_REGION( 0x400000, "gfx4", 0 ) /* sprites chip 1 */
	ROM_LOAD( "maj-04.r1",  0x000000, 0x80000, CRC(36e49b19) SHA1(bfbc45b635bf3d46ff8b8a514a3f352bf3a95535) )
	ROM_LOAD( "maj-05.r2",  0x080000, 0x80000, CRC(80fc71cc) SHA1(65b15afbe5d628051b012777d486b6ce92a3795c) )
	ROM_LOAD( "maj-06.r3",  0x100000, 0x80000, CRC(2e7a684b) SHA1(cffeda1a816dad30d6b1cb12458661188d625d40) )
	ROM_LOAD( "maj-07.r5",  0x180000, 0x80000, CRC(3acc1f78) SHA1(87ec65b4f54a66370754534d03f4c9217531b42f) )
	ROM_LOAD( "maj-08.s6",  0x200000, 0x80000, CRC(1958a36d) SHA1(466a30dcd2ea13028272ed2187f890ee20d6636b) )
	ROM_LOAD( "maj-09.s8",  0x280000, 0x80000, CRC(c21087a1) SHA1(b769c5f2f9b9c525d121902fe9557a6bfc077b99) )
	ROM_LOAD( "maj-10.s9",  0x300000, 0x80000, CRC(a02fa641) SHA1(14b999a441964e612700bf21945a948eaebb253e) )
	ROM_LOAD( "maj-11.s11", 0x380000, 0x80000, CRC(dabe9305) SHA1(44d69fe55e674de7f4c610d295d4528d4b2eb150) )

	ROM_REGION( 0x80000, "gfx5", 0 ) /* sprites chip 2 */
	ROM_LOAD16_BYTE( "gs12.n1", 0x000000, 0x20000, CRC(9a86a015) SHA1(968576b8422393ab9a93d98c15428b1c11417b3d) )
	ROM_LOAD16_BYTE( "gs13.n2", 0x000001, 0x20000, CRC(f4709905) SHA1(697842a3d7bc2588c77833c3af8938e6f0b1238d) )
	ROM_LOAD16_BYTE( "gs14.n3", 0x040000, 0x20000, CRC(750fc523) SHA1(ef8794359ff3a44a97ab402821fbe205a0be8f6a) )
	ROM_LOAD16_BYTE( "gs15.n5", 0x040001, 0x20000, CRC(f14edd3d) SHA1(802d576df6dac2c9bf99f963f1955fc3a7ffdac0) )

	ROM_REGION(0x40000, "oki1", 0 ) /* Oki samples */
	ROM_LOAD( "gs07.h15",  0x00000,  0x20000,  CRC(750b7e5d) SHA1(d33b17a1d8c9b05d5c1daf0c80fed6381e04b167) )

	ROM_REGION(0x80000, "oki2", 0 ) /* Extra Oki samples */
	ROM_LOAD( "maj-03.h16", 0x00000, 0x80000,  CRC(31dcfac3) SHA1(88c7fc139f871991defbc8dc2c9c66b150dd6f6f) )	/* banked */

	ROM_REGION( 0x2000, "proms", 0 )
	ROM_LOAD( "mb7128y.10m", 0x00000,  0x800,  CRC(bde780a2) SHA1(94ea9fe6c3a421e976d077e67f564ca5c37a5e88) )	/* Priority?  Unused */
	ROM_LOAD( "mb7128y.16p", 0x00800,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )	/* Unknown,  Unused */
	ROM_LOAD( "mb7128y.16s", 0x01000,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )	/* Unknown,  Unused */
	ROM_LOAD( "mb7128y.17s", 0x01800,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )	/* Unknown,  Unused - Data identical for 3 proms! */

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "pal16r8a 1h",  0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.7c",  0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.7d",  0x0400, 0x0104, CRC(199e83fd) SHA1(ebb5d66f29935b0a58e79b0db30611b5dce328a6) ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.7e",  0x0600, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.7l",  0x0800, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.8e",  0x0a00, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.9d",  0x0c00, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.10c", 0x0e00, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( dassault4 )
	ROM_REGION(0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE("gs01.a15", 0x00000, 0x20000, CRC(8613634d) SHA1(69b64e54fde3b5f1ee3435d7327b84e7a7d43f6d) )
	ROM_LOAD16_BYTE("gs03.a17", 0x00001, 0x20000, CRC(ea860bd4) SHA1(6e4e2d004433ad5842b4bc895eaa8f55bd1ee168) )
	ROM_LOAD16_BYTE("gs00.a14", 0x40000, 0x20000, CRC(b7277175) SHA1(ffb19c4dd12e0391f01de57c46a7998885fe22bf) ) /* Same data as GT00.A14 */
	ROM_LOAD16_BYTE("gs02.a16", 0x40001, 0x20000, CRC(cde31e35) SHA1(0219845308c9f46e73b0504bd2aefa2fa74f388e) ) /* Same data as GT02.A16 */

	ROM_REGION(0x80000, "sub", 0 ) /* 68000 code (Sub cpu) */
	ROM_LOAD16_BYTE("gs10.a12", 0x00000, 0x20000, CRC(285f72a3) SHA1(d01972aec500805ca1abed14983064cd14e942d4) )
	ROM_LOAD16_BYTE("gs08.a9",  0x00001, 0x20000, CRC(16691ede) SHA1(dc481dfc6104833a6fd18be6275e77ecc0510165) )
	ROM_LOAD16_BYTE("gs11.a14", 0x40000, 0x20000, CRC(80cb23de) SHA1(d52426460eea2285c57cfc3fe37aa6dc79990e25) ) /* Same data as GT11-1.A14 */
	ROM_LOAD16_BYTE("gs09.a11", 0x40001, 0x20000, CRC(0a8fa7e1) SHA1(330ae9602b5f56b5dc4961a41991b64412a59880) ) /* Same data as GT09-1.A11 */

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "gs04.f18",    0x00000, 0x10000, CRC(81c29ebf) SHA1(1b241277a8e35cdeaeb120970d14a09d33032459) ) /* Same data for all regions, different label */

	ROM_REGION(0x020000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "gs05.h11", 0x000000, 0x10000, CRC(0aae996a) SHA1(d37a12b057e9934212362d7eafa575c961819a27) ) /* Same data for all regions, different label */
	ROM_LOAD16_BYTE( "gs06.h12", 0x000001, 0x10000, CRC(4efdf03d) SHA1(835d22829c6d0f4efc76801b449f9a779f460f1c) ) /* Same data for all regions, different label */

	ROM_REGION(0x120000, "gfx2", 0 )
	ROM_LOAD( "maj-02.h14", 0x000000, 0x100000, CRC(383bbc37) SHA1(c537ab147a2770ce28ee185b08dd62d35249bfa9) )
	/* Other 0x20000 filled in later */

	ROM_REGION(0x200000, "gfx3", 0 )
	ROM_LOAD( "maj-01.c18", 0x000000, 0x100000, CRC(9840a204) SHA1(096c351769da5184c3d9a05495370134acc9507a) )
	ROM_LOAD( "maj-00.c17", 0x100000, 0x100000, CRC(87ea8d16) SHA1(db47123aa2ebbb800cfc5cfcf50309bc39cadbcd) )

	ROM_REGION( 0x400000, "gfx4", 0 ) /* sprites chip 1 */
	ROM_LOAD( "maj-04.r1",  0x000000, 0x80000, CRC(36e49b19) SHA1(bfbc45b635bf3d46ff8b8a514a3f352bf3a95535) )
	ROM_LOAD( "maj-05.r2",  0x080000, 0x80000, CRC(80fc71cc) SHA1(65b15afbe5d628051b012777d486b6ce92a3795c) )
	ROM_LOAD( "maj-06.r3",  0x100000, 0x80000, CRC(2e7a684b) SHA1(cffeda1a816dad30d6b1cb12458661188d625d40) )
	ROM_LOAD( "maj-07.r5",  0x180000, 0x80000, CRC(3acc1f78) SHA1(87ec65b4f54a66370754534d03f4c9217531b42f) )
	ROM_LOAD( "maj-08.s6",  0x200000, 0x80000, CRC(1958a36d) SHA1(466a30dcd2ea13028272ed2187f890ee20d6636b) )
	ROM_LOAD( "maj-09.s8",  0x280000, 0x80000, CRC(c21087a1) SHA1(b769c5f2f9b9c525d121902fe9557a6bfc077b99) )
	ROM_LOAD( "maj-10.s9",  0x300000, 0x80000, CRC(a02fa641) SHA1(14b999a441964e612700bf21945a948eaebb253e) )
	ROM_LOAD( "maj-11.s11", 0x380000, 0x80000, CRC(dabe9305) SHA1(44d69fe55e674de7f4c610d295d4528d4b2eb150) )

	ROM_REGION( 0x80000, "gfx5", 0 ) /* sprites chip 2 */
	ROM_LOAD16_BYTE( "gs12.n1", 0x000000, 0x20000, CRC(9a86a015) SHA1(968576b8422393ab9a93d98c15428b1c11417b3d) )
	ROM_LOAD16_BYTE( "gs13.n2", 0x000001, 0x20000, CRC(f4709905) SHA1(697842a3d7bc2588c77833c3af8938e6f0b1238d) )
	ROM_LOAD16_BYTE( "gs14.n3", 0x040000, 0x20000, CRC(750fc523) SHA1(ef8794359ff3a44a97ab402821fbe205a0be8f6a) )
	ROM_LOAD16_BYTE( "gs15.n5", 0x040001, 0x20000, CRC(f14edd3d) SHA1(802d576df6dac2c9bf99f963f1955fc3a7ffdac0) )

	ROM_REGION(0x40000, "oki1", 0 ) /* Oki samples */
	ROM_LOAD( "gs07.h15",  0x00000,  0x20000,  CRC(750b7e5d) SHA1(d33b17a1d8c9b05d5c1daf0c80fed6381e04b167) )

	ROM_REGION(0x80000, "oki2", 0 ) /* Extra Oki samples */
	ROM_LOAD( "maj-03.h16", 0x00000, 0x80000,  CRC(31dcfac3) SHA1(88c7fc139f871991defbc8dc2c9c66b150dd6f6f) )	/* banked */

	ROM_REGION( 0x2000, "proms", 0 )
	ROM_LOAD( "mb7128y.10m", 0x00000,  0x800,  CRC(bde780a2) SHA1(94ea9fe6c3a421e976d077e67f564ca5c37a5e88) )	/* Priority?  Unused */
	ROM_LOAD( "mb7128y.16p", 0x00800,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )	/* Unknown,  Unused */
	ROM_LOAD( "mb7128y.16s", 0x01000,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )	/* Unknown,  Unused */
	ROM_LOAD( "mb7128y.17s", 0x01800,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )	/* Unknown,  Unused - Data identical for 3 proms! */

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "pal16r8a 1h",  0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.7c",  0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.7d",  0x0400, 0x0104, CRC(199e83fd) SHA1(ebb5d66f29935b0a58e79b0db30611b5dce328a6) ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.7e",  0x0600, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.7l",  0x0800, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.8e",  0x0a00, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.9d",  0x0c00, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b.10c", 0x0e00, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

/**********************************************************************************/


DRIVER_INIT_MEMBER(dassault_state,dassault)
{
	const UINT8 *src = machine().root_device().memregion("gfx1")->base();
	UINT8 *dst = machine().root_device().memregion("gfx2")->base();
	UINT8 *tmp = auto_alloc_array(machine(), UINT8, 0x80000);

	/* Playfield 4 also has access to the char graphics, make things easier
    by just copying the chars to both banks (if I just used a different gfx
    bank then the colours would be wrong). */
	memcpy(tmp + 0x000000, dst + 0x80000, 0x80000);
	memcpy(dst + 0x090000, tmp + 0x00000, 0x80000);
	memcpy(dst + 0x080000, src + 0x00000, 0x10000);
	memcpy(dst + 0x110000, src + 0x10000, 0x10000);

	auto_free(machine(), tmp);
}

DRIVER_INIT_MEMBER(dassault_state,thndzone)
{
	const UINT8 *src = machine().root_device().memregion("gfx1")->base();
	UINT8 *dst = machine().root_device().memregion("gfx2")->base();
	UINT8 *tmp = auto_alloc_array(machine(), UINT8, 0x80000);

	/* Playfield 4 also has access to the char graphics, make things easier
    by just copying the chars to both banks (if I just used a different gfx
    bank then the colours would be wrong). */
	memcpy(tmp + 0x000000, dst + 0x80000, 0x80000);
	memcpy(dst + 0x090000, tmp + 0x00000, 0x80000);
	memcpy(dst + 0x080000, src + 0x00000, 0x10000);
	memcpy(dst + 0x110000, src + 0x10000, 0x10000);

	auto_free(machine(), tmp);
}

/**********************************************************************************/

GAME( 1991, thndzone,  0,        dassault, thndzone,  dassault_state, thndzone, ROT0, "Data East Corporation", "Thunder Zone (Rev 1, World)", GAME_SUPPORTS_SAVE )
GAME( 1991, thndzonea, thndzone, dassault, thndzone,  dassault_state, thndzone, ROT0, "Data East Corporation", "Thunder Zone (World)", GAME_SUPPORTS_SAVE )
GAME( 1991, thndzonej, thndzone, dassault, thndzone,  dassault_state, thndzone, ROT0, "Data East Corporation", "Thunder Zone (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1991, dassault,  thndzone, dassault, dassault,  dassault_state, dassault, ROT0, "Data East Corporation", "Desert Assault (US)", GAME_SUPPORTS_SAVE )
GAME( 1991, dassault4, thndzone, dassault, dassault4, dassault_state, dassault, ROT0, "Data East Corporation", "Desert Assault (US 4 Players)", GAME_SUPPORTS_SAVE )
