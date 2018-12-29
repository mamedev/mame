// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*******************************************************************************

    Irem M107 games:

    Air Assault / Fire Barrel               (c) 1993 Irem Corporation
    Dream Soccer '94                        (c) 1994 Data East Corporation
    Kick for the Goal                       (c) 1994 Jaleco
    World PK Soccer                         (c) 1995 Jaleco


    Graphics glitches in both games.

    Emulation by Bryan McPhail, mish@tendril.co.uk


To Do:
  Hook up inputs and EEPROM in Kick for the Goal / World PK Soccer

2008-08
Dip locations have been added assuming that the layout is the same as the
m92 boards (and earlier Irem boards). However, it would be nice to have them
confirmed for m107 games as well.

*******************************************************************************/

#include "emu.h"
#include "includes/m107.h"
#include "includes/iremipt.h"

#include "cpu/nec/nec.h"
#include "machine/gen_latch.h"
#include "machine/irem_cpu.h"
#include "sound/ym2151.h"
#include "sound/iremga20.h"
#include "speaker.h"

/*****************************************************************************/

void m107_state::machine_start()
{
}

/*****************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(m107_state::scanline_interrupt)
{
	int scanline = param;

	/* raster interrupt */
	if (scanline == m_raster_irq_position)
	{
		m_screen->update_partial(scanline);
		m_upd71059c->ir2_w(1);
	}
	else
	{
		/* VBLANK interrupt */
		if (scanline == m_screen->visible_area().max_y + 1)
		{
			m_screen->update_partial(scanline);
			m_upd71059c->ir0_w(1);
			m_upd71059c->ir2_w(0);
		}
		else
		{
			m_upd71059c->ir0_w(0);
		}

	}
}


/*****************************************************************************/

WRITE8_MEMBER(m107_state::coincounter_w)
{
	machine().bookkeeping().coin_counter_w(0,data & 0x01);
	machine().bookkeeping().coin_counter_w(1,data & 0x02);
}

WRITE8_MEMBER(m107_state::bankswitch_w)
{
	m_mainbank->set_entry((data & 0x06) >> 1);
	if (data & 0xf9)
		logerror("%05x: bankswitch %04x\n", m_maincpu->pc(), data);
}

WRITE16_MEMBER(m107_state::sound_reset_w)
{
	m_soundcpu->set_input_line(INPUT_LINE_RESET, (data) ? CLEAR_LINE : ASSERT_LINE);
}

/*****************************************************************************/

void m107_state::main_map(address_map &map)
{
	map(0xd0000, 0xdffff).ram().w(FUNC(m107_state::vram_w)).share("vram_data");
	map(0xe0000, 0xeffff).ram(); /* System ram */
	map(0xf8000, 0xf8fff).ram().share("spriteram");
	map(0xf9000, 0xf9fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xffff0, 0xfffff).rom().region("maincpu", 0x7fff0);
}

// Not bankswitched
void m107_state::firebarr_map(address_map &map)
{
	map(0x00000, 0xbffff).rom();
	main_map(map);
}

// Bankswitched
void m107_state::dsoccr94_map(address_map &map)
{
	map(0x00000, 0x9ffff).rom();
	map(0xa0000, 0xbffff).bankr("mainbank");
	main_map(map);
}

void m107_state::main_portmap(address_map &map)
{
	map(0x00, 0x01).portr("P1_P2");
	map(0x02, 0x03).portr("COINS_DSW3");
	map(0x04, 0x05).portr("DSW");
	map(0x06, 0x07).portr("P3_P4");
	map(0x08, 0x08).r("soundlatch2", FUNC(generic_latch_8_device::read));   // answer from sound CPU
	map(0x00, 0x00).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x02, 0x02).w(FUNC(m107_state::coincounter_w));
	map(0x04, 0x05).nopw(); /* ??? 0008 */
	map(0x40, 0x43).rw(m_upd71059c, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x80, 0x9f).w(FUNC(m107_state::control_w));
	map(0xa0, 0xaf).nopw(); /* Written with 0's in interrupt */
	map(0xb0, 0xb1).w(FUNC(m107_state::spritebuffer_w));
	map(0xc0, 0xc3).nopr(); /* Only wpksoc: ticket related? */
	map(0xc0, 0xc1).w(FUNC(m107_state::sound_reset_w));
}

void m107_state::dsoccr94_io_map(address_map &map)
{
	main_portmap(map);
	map(0x06, 0x06).w(FUNC(m107_state::bankswitch_w));
}

/* same as M107 but with an extra i/o board */
WRITE16_MEMBER(m107_state::wpksoc_output_w)
{
	/*
	x--- ---- ?
	---- --xx lamps
	*/
	if(data & 0x7c)
		popmessage("%04x",data);
}

void m107_state::wpksoc_map(address_map &map)
{
	main_map(map);
	map(0x00000, 0x7ffff).rom();
	map(0xf0000, 0xf0001).portr("WPK_DSW0");
	map(0xf0002, 0xf0003).portr("WPK_DSW1");
	map(0xf0004, 0xf0005).portr("WPK_DSW2");
}

void m107_state::wpksoc_io_map(address_map &map)
{
	main_portmap(map);
	map(0x22, 0x23).w(FUNC(m107_state::wpksoc_output_w));
	map(0xc0, 0xc1).portr("WPK_IN0");
	map(0xc2, 0xc3).portr("WPK_IN1");
}

/******************************************************************************/

void m107_state::sound_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0xa0000, 0xa3fff).ram();
	map(0xa8000, 0xa803f).rw("irem", FUNC(iremga20_device::irem_ga20_r), FUNC(iremga20_device::irem_ga20_w)).umask16(0x00ff);
	map(0xa8040, 0xa8043).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write)).umask16(0x00ff);
	map(0xa8044, 0xa8044).rw("soundlatch", FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::acknowledge_w));
	map(0xa8046, 0xa8046).w("soundlatch2", FUNC(generic_latch_8_device::write));
	map(0xffff0, 0xfffff).rom().region("soundcpu", 0x1fff0);
}

/******************************************************************************/

static INPUT_PORTS_START( m107_2player )
	PORT_START("P1_P2")
	IREM_GENERIC_JOYSTICKS_3_BUTTONS(1, 2)

	PORT_START("COINS_DSW3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen") //this is sprite flag on Irem M92, if this is active low then Dream Soccer '94 is unplayably slow

	/* DIP switch bank 3 */
	PORT_DIPUNKNOWN_DIPLOC( 0x0100, 0x0100, "SW3:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0200, "SW3:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0400, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0800, "SW3:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "SW3:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW3:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "SW3:8" )

	PORT_START("DSW")
	/* Dip switch bank 1 */
	PORT_DIPUNKNOWN_DIPLOC( 0x0001, 0x0001, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0002, 0x0002, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0004, 0x0004, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0008, 0x0008, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, 0x0010, "SW1:5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" )
	/* Dip switch bank 2 */
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0200, "SW2:2" )
	PORT_DIPNAME( 0x0400, 0x0400, "Coin Slots" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Common" )
	PORT_DIPSETTING(      0x0000, "Separate" )
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	/* Coin Mode 1 */
	IREM_COIN_MODE_1_NEW_HIGH
	/* Coin Mode 2 */
	IREM_COIN_MODE_2_HIGH

	PORT_START("P3_P4")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( m107_3player )
	PORT_INCLUDE(m107_2player)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0200, "2 Players" )
	PORT_DIPSETTING(      0x0000, "3 Players" )

	PORT_MODIFY("P3_P4")
	IREM_INPUT_PLAYER_3
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( m107_4player )
	PORT_INCLUDE(m107_3player)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0200, "2 Players" )
	PORT_DIPSETTING(      0x0000, "4 Players" )

	PORT_MODIFY("P3_P4")
	IREM_INPUT_PLAYER_4
INPUT_PORTS_END

/******************************************************************************/

static INPUT_PORTS_START( firebarr )
	PORT_INCLUDE(m107_2player)

	PORT_MODIFY("COINS_DSW3")
	PORT_DIPNAME( 0x0c00, 0x0800, "Rapid Fire" ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(      0x0000, "Button 1 Normal, Button 3 Rapid Fire" )
	PORT_DIPSETTING(      0x0400, "Button 1 Rapid Fire, Button 3 No Function" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x1000, 0x0000, "Continuous Play" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0001, "5" )
INPUT_PORTS_END


static INPUT_PORTS_START( dsoccr94 )
	PORT_INCLUDE(m107_4player)

	PORT_MODIFY("COINS_DSW3")
	PORT_DIPNAME( 0x0300, 0x0300, "Player Power" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(      0x0000, "500" )
	PORT_DIPSETTING(      0x0300, "1000" )
	PORT_DIPSETTING(      0x0100, "1500" )
	PORT_DIPSETTING(      0x0200, "2000" )
	/* Manual says not to use these SW3:3-8 */

	PORT_MODIFY("DSW")
	/* Dip switch bank 1 */
	PORT_DIPNAME( 0x0003, 0x0003, "Time" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, "1:30" )
	PORT_DIPSETTING(      0x0003, "2:00" )
	PORT_DIPSETTING(      0x0002, "2:30" )
	PORT_DIPSETTING(      0x0001, "3:00" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Game Mode" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, "Match Mode" )
	PORT_DIPSETTING(      0x0000, "Power Mode" )
/*
   Match Mode: Winner advances to the next game.  Game Over for the loser
   Power Mode: The Players can play the game until their respective powers run
               out, reguardless of whether they win or lose the game.
               Player 2 can join in any time during the game
               Player power (time) can be adjusted by dip switch #3
*/
	PORT_DIPNAME( 0x0020, 0x0020, "Starting Button" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, "Button 1" )
	PORT_DIPSETTING(      0x0020, "Start Button" )
INPUT_PORTS_END

static INPUT_PORTS_START( wpksoc )
	PORT_START("P1_P2")
	PORT_BIT(0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COINS_DSW3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen") //this is sprite flag on Irem M92, if this is active low then Dream Soccer '94 is unplayably slow
	PORT_DIPNAME( 0x0100, 0x0000, "DSW3" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( On ) )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0000, "DSW1" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, "DSW2" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( On ) )

	PORT_START("P3_P4")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("WPK_DSW0")
	PORT_DIPNAME( 0x0001, 0x0000, "WPK_DSW0-0" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, "WPK_DSW0-1" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( On ) )

	PORT_START("WPK_DSW1")
	PORT_DIPNAME( 0x0001, 0x0000, "WPK_DSW1-0" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x00f0, 0x0000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0xa0^0xf0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0^0xf0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xc0^0xf0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xd0^0xf0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xe0^0xf0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10^0xf0, "2 Coins to Start/1 to Continue")
	PORT_DIPSETTING(    0x30^0xf0, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x20^0xf0, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0^0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40^0xf0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x90^0xf0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80^0xf0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x70^0xf0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x60^0xf0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50^0xf0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00^0xf0, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0100, 0x0000, "WPK_DSW1-1" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( On ) )

	PORT_START("WPK_DSW2")
	PORT_DIPNAME( 0x0001, 0x0000, "WPK_DSW2-0" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x00f0, 0x0000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(    0xa0^0xf0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0^0xf0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xc0^0xf0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xd0^0xf0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xe0^0xf0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10^0xf0, "2 Coins to Start/1 to Continue")
	PORT_DIPSETTING(    0x30^0xf0, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x20^0xf0, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0^0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40^0xf0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x90^0xf0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80^0xf0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x70^0xf0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x60^0xf0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50^0xf0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00^0xf0, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0100, 0x0000, "WPK_DSW2-1" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( On ) )

	PORT_START("WPK_IN0")
	PORT_DIPNAME( 0x0001, 0x0000, "WPK_IN0-0" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0001, DEF_STR( On ) )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN ) // motor status?
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, "WPK_IN0-1" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( On ) )

	PORT_START("WPK_IN1")
	PORT_DIPNAME( 0x0001, 0x0000, "WPK_IN1-0" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, "WPK_IN1-1" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 8, 0, 24, 16 },
	{ STEP8(0,1) },
	{ STEP8(0,32) },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ STEP8(0,1), STEP8(16*8,1) },
	{ STEP16(0,8) },
	32*8
};

static const gfx_layout spritelayout2 =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ STEP8(8,1), STEP8(0,1) },
	{ STEP16(0,16) },
	32*8
};

static GFXDECODE_START( gfx_m107 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 128 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0, 128 )
GFXDECODE_END

static GFXDECODE_START( gfx_firebarr )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 128 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout2,0, 128 )
GFXDECODE_END

/***************************************************************************/

void m107_state::firebarr(machine_config &config)
{
	/* basic machine hardware */
	V33(config, m_maincpu, XTAL(28'000'000)/2);    /* NEC V33, 28MHz clock */
	m_maincpu->set_addrmap(AS_PROGRAM, &m107_state::firebarr_map);
	m_maincpu->set_addrmap(AS_IO, &m107_state::main_portmap);
	m_maincpu->set_irq_acknowledge_callback("upd71059c", FUNC(pic8259_device::inta_cb));

	V35(config, m_soundcpu, XTAL(14'318'181));
	m_soundcpu->set_addrmap(AS_PROGRAM, &m107_state::sound_map);
	m_soundcpu->set_decryption_table(rtypeleo_decryption_table);

	PIC8259(config, m_upd71059c, 0);
	m_upd71059c->out_int_callback().set_inputline(m_maincpu, 0);

	TIMER(config, "scantimer").configure_scanline(FUNC(m107_state::scanline_interrupt), "screen", 0, 1);

	/* video hardware */
	BUFFERED_SPRITERAM16(config, "spriteram");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(512, 256);
	m_screen->set_visarea(80, 511-112, 8, 247); /* 320 x 240 */
	m_screen->set_screen_update(FUNC(m107_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_firebarr);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	generic_latch_8_device &soundlatch(GENERIC_LATCH_8(config, "soundlatch"));
	soundlatch.data_pending_callback().set_inputline(m_soundcpu, NEC_INPUT_LINE_INTP1);
	soundlatch.set_separate_acknowledge(true);

	GENERIC_LATCH_8(config, "soundlatch2").data_pending_callback().set(m_upd71059c, FUNC(pic8259_device::ir3_w));

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(14'318'181)/4));
	ymsnd.irq_handler().set_inputline(m_soundcpu, NEC_INPUT_LINE_INTP0);
	ymsnd.add_route(0, "lspeaker", 0.40);
	ymsnd.add_route(1, "rspeaker", 0.40);

	iremga20_device &ga20(IREMGA20(config, "irem", XTAL(14'318'181)/4));
	ga20.add_route(0, "lspeaker", 1.0);
	ga20.add_route(1, "rspeaker", 1.0);
}

void m107_state::dsoccr94(machine_config &config)
{
	firebarr(config);

	/* basic machine hardware */
	m_maincpu->set_clock(20000000/2);  /* NEC V33, Could be 28MHz clock? */
	m_maincpu->set_addrmap(AS_PROGRAM, &m107_state::dsoccr94_map);
	m_maincpu->set_addrmap(AS_IO, &m107_state::dsoccr94_io_map);

	m_soundcpu->set_decryption_table(dsoccr94_decryption_table);

	/* video hardware */
	m_gfxdecode->set_info(gfx_m107);
}


void m107_state::wpksoc(machine_config &config)
{
	firebarr(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &m107_state::wpksoc_map);
	m_maincpu->set_addrmap(AS_IO, &m107_state::wpksoc_io_map);

	m_soundcpu->set_decryption_table(leagueman_decryption_table);
}

void m107_state::airass(machine_config &config)
{
	firebarr(config);
	m_gfxdecode->set_info(gfx_m107);

	m_soundcpu->set_decryption_table(gunforce_decryption_table);
}

/***************************************************************************/


ROM_START( airass )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "f4-a-h0-etc.h0", 0x000001, 0x40000, CRC(038f2cbd) SHA1(79db9f4cc96d32ae9b9526111078bdb87f5711ce) ) /* IC59 */
	ROM_LOAD16_BYTE( "f4-a-l0-etc.l0", 0x000000, 0x40000, CRC(d3eb7842) SHA1(4f8c48f6d42ff222d28397e747fafddb025f21b1) ) /* IC61 */
	ROM_LOAD16_BYTE( "f4-a-h1-ss.h1",  0x080001, 0x20000, CRC(4cb1c9ae) SHA1(9e372f9a6e21a80fce1ce94290ba638f0659f056) ) /* IC60 */
	ROM_LOAD16_BYTE( "f4-a-l1-ss.l1",  0x080000, 0x20000, CRC(1ddd192d) SHA1(5805d33bd73d2c2c1529420d29582df1c43b62c2) ) /* IC62 */

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "f4-b-sh0-c.sh0", 0x00001, 0x10000, CRC(31c05c0d) SHA1(9e4d4cd35cf4d725c26836610f0bf36a40fb1617) ) /* IC31 */
	ROM_LOAD16_BYTE( "f4-b-sl0-c.sl0", 0x00000, 0x10000, CRC(60a0d33a) SHA1(11668b44ff4d85b6f23278c5eb6a142e129dec38) ) /* IC37 */

	ROM_REGION( 0x200000, "gfx1", 0 )   /* chars */
	ROM_LOAD16_BYTE( "w45.c00", 0x000000, 0x100000, CRC(2aab419e) SHA1(bc55d3d52ae9d89b9f2b38493d3fce2710a95837) ) /* IC29 */
	ROM_LOAD16_BYTE( "w46.c10", 0x000001, 0x100000, CRC(d6e5c910) SHA1(40bcf895a7379aa3c65cc58c0b5dbec5e391ab6c) ) /* IC28 */

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites */
	ROM_LOAD( "w47.000", 0x000000, 0x100000, CRC(72e1a253) SHA1(ac7e28eadb365dbb6e2246ae4a9b9ae9bcb6ccee) ) /* IC11 */
	ROM_LOAD( "w48.010", 0x100000, 0x100000, CRC(1746b7f6) SHA1(b0faa60516e656dfce19bc1f2d72281342adc8a4) ) /* IC12 */
	ROM_LOAD( "w49.020", 0x200000, 0x100000, CRC(17b5caf2) SHA1(df38f9a625226c96ac921182ef975e598d9bc245) ) /* IC13 */
	ROM_LOAD( "w50.030", 0x300000, 0x100000, CRC(63e4bec3) SHA1(252b4493e1bc368021389e65295036523c401ad4) ) /* IC14 */

	ROM_REGION( 0x40000, "sprtable", 0 )   /* sprite tables */
	ROM_LOAD16_BYTE( "f4-b-drh-.drh", 0x000001, 0x20000, CRC(12001372) SHA1(a5346d8a741cd1a93aa289562bb56d2fc40c1bbb) ) /* IC10 */
	ROM_LOAD16_BYTE( "f4-b-drl-.drl", 0x000000, 0x20000, CRC(08cb7533) SHA1(9e0d8f8498bddfa1c6135abbab4465e9eeb033fe) ) /* IC1  */

	ROM_REGION( 0x80000, "irem", 0 )    /* ADPCM samples */
	ROM_LOAD( "w96.da0", 0x000000, 0x80000, CRC(7a493e2e) SHA1(f6a8bacbe25760c86bdd8e8bb6d052ff15718eef) ) /* IC24 */
ROM_END

ROM_START( firebarr )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "f4-a-h0-c.h0", 0x000001, 0x40000, CRC(2aa5676e) SHA1(7f51c462c58b63fa4f34ec9dd2ee69c932ebf718) ) /* IC59 @ 9d */
	ROM_LOAD16_BYTE( "f4-a-l0-c.l0", 0x000000, 0x40000, CRC(42f75d59) SHA1(eba3a02874d608ecb8c93160c8f0b4c8bb8061d2) ) /* IC61 @ 9f */
	ROM_LOAD16_BYTE( "f4-a-h1-c.h1", 0x080001, 0x20000, CRC(bb7f6968) SHA1(366747672aac939454d9915cda5277b0438f063b) ) /* IC60 @ 9e */
	ROM_LOAD16_BYTE( "f4-a-l1-c.l1", 0x080000, 0x20000, CRC(9d57edd6) SHA1(16122829b61aa3aee88aeb6634831e8cf95eaee0) ) /* IC62 @ 9h */

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "f4-b-sh0-b.sh0", 0x00001, 0x10000, CRC(30a8e232) SHA1(d4695aed35a1aa796b2872e58a6014e8b28bc154) ) /* IC31 */
	ROM_LOAD16_BYTE( "f4-b-sl0-b.sl0", 0x00000, 0x10000, CRC(204b5f1f) SHA1(f0386500773cd7cca93f0e8e740db29182320c70) ) /* IC37 */

	ROM_REGION( 0x200000, "gfx1", 0 )   /* chars */
	ROM_LOAD16_BYTE( "f4-c00.c00", 0x000000, 0x80000, CRC(50cab384) SHA1(66e88a1dfa943e0d49c2e186ac2f6cbf5cfe0864) ) /* IC29 */
	ROM_LOAD16_BYTE( "f4-c10.c10", 0x000001, 0x80000, CRC(330c6df2) SHA1(f199d959385398adb6b86ec8ec5de8b40899597c) ) /* IC28 */
	ROM_LOAD16_BYTE( "f4-c01.c01", 0x100000, 0x80000, CRC(12a698c8) SHA1(74d21768bac70e8cb7e1a6737f758f33869b6af9) ) /* IC21 */
	ROM_LOAD16_BYTE( "f4-c11.c11", 0x100001, 0x80000, CRC(3f9add18) SHA1(840339a1f33d68c555e42618dd436788639b1edf) ) /* IC20 */

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites */
	ROM_LOAD16_BYTE( "f4-000.000", 0x000000, 0x80000, CRC(920deee9) SHA1(6341eeccdad97fde5337f32f317ddc94f6b8d07a) ) /* IC11 */
	ROM_LOAD16_BYTE( "f4-001.001", 0x000001, 0x80000, CRC(e5725eaf) SHA1(c884d69742484a7c07eb0c7882a33d90b240529e) ) /* IC2  */
	ROM_LOAD16_BYTE( "f4-010.010", 0x100000, 0x80000, CRC(3505d185) SHA1(1330c18eaadb3e23d6205f3912015cb9ca5f3590) ) /* IC12 */
	ROM_LOAD16_BYTE( "f4-011.011", 0x100001, 0x80000, CRC(1912682f) SHA1(d0234877aabf94df7f6a6091e38247954725e1f3) ) /* IC3  */
	ROM_LOAD16_BYTE( "f4-020.020", 0x200000, 0x80000, CRC(ec130b8e) SHA1(6a4562f3e39d02f97f3b917e4a51f48b6f43a4c8) ) /* IC13 */
	ROM_LOAD16_BYTE( "f4-021.021", 0x200001, 0x80000, CRC(8dd384dc) SHA1(dee79d0d48762b98c20c88ba6617de5e939f596d) ) /* IC4  */
	ROM_LOAD16_BYTE( "f4-030.030", 0x300000, 0x80000, CRC(7e7b30cd) SHA1(eca9d2a5d9f9deebb565456018126bc37a1de1d8) ) /* IC14 */
	ROM_LOAD16_BYTE( "f4-031.031", 0x300001, 0x80000, CRC(83ac56c5) SHA1(47e1063c71d5570fecf8591c2cb7c74fd45199f5) ) /* IC5  */

	ROM_REGION( 0x40000, "sprtable", 0 )   /* sprite tables */
	ROM_LOAD16_BYTE( "f4-b-drh-.drh", 0x000001, 0x20000, CRC(12001372) SHA1(a5346d8a741cd1a93aa289562bb56d2fc40c1bbb) ) /* IC10 */
	ROM_LOAD16_BYTE( "f4-b-drl-.drl", 0x000000, 0x20000, CRC(08cb7533) SHA1(9e0d8f8498bddfa1c6135abbab4465e9eeb033fe) ) /* IC1  */

	ROM_REGION( 0x80000, "irem", 0 )    /* ADPCM samples */
	ROM_LOAD( "f4-b-da0.da0", 0x000000, 0x80000, CRC(7a493e2e) SHA1(f6a8bacbe25760c86bdd8e8bb6d052ff15718eef) ) /* IC24 (== w96.da0 from Air Assault) */
ROM_END

ROM_START( dsoccr94 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "a3-4p_h0-c-0.ic59", 0x000001, 0x040000, CRC(d01d3fd7) SHA1(925dff999252bf3b920bc0f427744e1464620fe8) )
	ROM_LOAD16_BYTE( "a3-4p_l0-c-0.ic61", 0x000000, 0x040000, CRC(8af0afe2) SHA1(423c77d392a79cdaed66ad8c13039450d34d3f6d) )
	ROM_LOAD16_BYTE( "a3_h1-c-0.ic60",    0x080001, 0x040000, CRC(6109041b) SHA1(063898a88f8a6a9f1510aa55e53a39f037b02903) )
	ROM_LOAD16_BYTE( "a3_l1-c-0.ic62",    0x080000, 0x040000, CRC(97a01f6b) SHA1(e188e28f880f5f3f4d7b49eca639d643989b1468) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "a3-sh0-c-0.ic31", 0x00001, 0x10000, CRC(23fe6ffc) SHA1(896377961cafc19e44d9d889f9fbfdbaedd556da) )
	ROM_LOAD16_BYTE( "a3-sl0-c-0.ic37", 0x00000, 0x10000, CRC(768132e5) SHA1(1bb64516eb58d3b246f08e1c07f091e78085689f) )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* chars */
	ROM_LOAD16_BYTE( "ds_c00.ic29", 0x000000, 0x100000, CRC(2d31d418) SHA1(6cd0e362bc2e3f2b20d96ee97a04bff46ee3016a) ) /* mask ROMs with no "official" ROM label */
	ROM_LOAD16_BYTE( "ds_c10.ic28", 0x000001, 0x100000, CRC(57f7bcd3) SHA1(a38e7cdfdea72d882fba414cae391ba09443e73c) )
	ROM_LOAD16_BYTE( "ds_c01.ic21", 0x200000, 0x100000, CRC(9d31a464) SHA1(1e38ac296f64d77fabfc0d5f7921a9b7a8424875) )
	ROM_LOAD16_BYTE( "ds_c11.ic20", 0x200001, 0x100000, CRC(a372e79f) SHA1(6b0889cfc2970028832566e25257927ddc461ea6) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites */
	ROM_LOAD( "ds_000.ic11", 0x000000, 0x100000, CRC(366b3e29) SHA1(cb016dcbdc6e8ea56c28c00135263666b07df991) ) /* mask ROMs with no "official" ROM label */
	ROM_LOAD( "ds_010.ic12", 0x100000, 0x100000, CRC(28a4cc40) SHA1(7f4e1ef995eaadf1945ee22ab3270cb8a21c601d) )
	ROM_LOAD( "ds_020.ic13", 0x200000, 0x100000, CRC(5a310f7f) SHA1(21969e4247c8328d27118d00604096deaf6700af) )
	ROM_LOAD( "ds_030.ic14", 0x300000, 0x100000, CRC(328b1f45) SHA1(4cbbd4d9be4fc151d426175bdbd35d8481bf2966) )

	ROM_REGION( 0x100000, "irem", 0 )    /* ADPCM samples */
	ROM_LOAD( "ds_da0.ic24", 0x000000, 0x100000, CRC(67fc52fd) SHA1(5771e948115af8fe4a6d3f448c03a2a9b42b6f20) )
ROM_END

ROM_START( dsoccr94k )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ic59_h0.bin",    0x000001, 0x040000, CRC(7b26d8a3) SHA1(03b4a5f0c7bd72bee24065feb22b837b373d936c) )
	ROM_LOAD16_BYTE( "ic61_l0.bin",    0x000000, 0x040000, CRC(b13f0ff4) SHA1(01d4241019abb16364090b2d00b93864d228ab98) )
	ROM_LOAD16_BYTE( "ic60_h1.bin",    0x080001, 0x040000, CRC(6109041b) SHA1(063898a88f8a6a9f1510aa55e53a39f037b02903) )
	ROM_LOAD16_BYTE( "ic62_l1.bin",    0x080000, 0x040000, CRC(97a01f6b) SHA1(e188e28f880f5f3f4d7b49eca639d643989b1468) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "a3-sh0-c-0.ic31", 0x00001, 0x10000, CRC(23fe6ffc) SHA1(896377961cafc19e44d9d889f9fbfdbaedd556da) )
	ROM_LOAD16_BYTE( "a3-sl0-c-0.ic37", 0x00000, 0x10000, CRC(768132e5) SHA1(1bb64516eb58d3b246f08e1c07f091e78085689f) )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* chars */
	ROM_LOAD16_BYTE( "ds_c00.ic29", 0x000000, 0x100000, CRC(2d31d418) SHA1(6cd0e362bc2e3f2b20d96ee97a04bff46ee3016a) ) /* mask ROMs with no "official" ROM label */
	ROM_LOAD16_BYTE( "ds_c10.ic28", 0x000001, 0x100000, CRC(57f7bcd3) SHA1(a38e7cdfdea72d882fba414cae391ba09443e73c) )
	ROM_LOAD16_BYTE( "ds_c01.ic21", 0x200000, 0x100000, CRC(9d31a464) SHA1(1e38ac296f64d77fabfc0d5f7921a9b7a8424875) )
	ROM_LOAD16_BYTE( "ds_c11.ic20", 0x200001, 0x100000, CRC(a372e79f) SHA1(6b0889cfc2970028832566e25257927ddc461ea6) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites */
	ROM_LOAD( "ds_000.ic11", 0x000000, 0x100000, CRC(366b3e29) SHA1(cb016dcbdc6e8ea56c28c00135263666b07df991) ) /* mask ROMs with no "official" ROM label */
	ROM_LOAD( "ds_010.ic12", 0x100000, 0x100000, CRC(28a4cc40) SHA1(7f4e1ef995eaadf1945ee22ab3270cb8a21c601d) )
	ROM_LOAD( "ds_020.ic13", 0x200000, 0x100000, CRC(5a310f7f) SHA1(21969e4247c8328d27118d00604096deaf6700af) )
	ROM_LOAD( "ds_030.ic14", 0x300000, 0x100000, CRC(328b1f45) SHA1(4cbbd4d9be4fc151d426175bdbd35d8481bf2966) )

	ROM_REGION( 0x100000, "irem", 0 )    /* ADPCM samples */
	ROM_LOAD( "ds_da0.ic24", 0x000000, 0x100000, CRC(67fc52fd) SHA1(5771e948115af8fe4a6d3f448c03a2a9b42b6f20) )
ROM_END


ROM_START( wpksoc )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pk-h0-eur-d.h0", 0x000001, 0x040000, CRC(b4917788) SHA1(673294c518eaf28354fa6a3058f9325c6d9ddde6) )
	ROM_LOAD16_BYTE( "pk-l0-eur-d.l0", 0x000000, 0x040000, CRC(03816bae) SHA1(832e2ec722b41d41626fec583fc11e9ff62cdaa0) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "pk-sh0.sh0", 0x00001, 0x10000, CRC(1145998c) SHA1(cdb2a428e0f35302b81696dab02d3dd2c433f6e5) )
	ROM_LOAD16_BYTE( "pk-sl0.sl0", 0x00000, 0x10000, CRC(542ee1c7) SHA1(b934adeecbba17cf96b06a2b1dc1ceaebdf9ad10) )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* chars */
	ROM_LOAD16_BYTE( "pk-c00-os.c00", 0x000000, 0x80000, CRC(42ae3d73) SHA1(e4777066155c9882695ebff0412bd879b8d6f716) )
	ROM_LOAD16_BYTE( "pk-c10-os.c10", 0x000001, 0x80000, CRC(86acf45c) SHA1(3b3d2abcf8000161a37d5e2619df529533aea47d) )
	ROM_LOAD16_BYTE( "pk-c01-os.c01", 0x100000, 0x80000, CRC(b0d33f87) SHA1(f2c0e3a10615c6861a3f6fd82a3f066e8e264233) )
	ROM_LOAD16_BYTE( "pk-c11-os.c11", 0x100001, 0x80000, CRC(19de7d63) SHA1(6d0633e412b47accaecc887a5c39f542eda49e81) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites */
	ROM_LOAD16_BYTE( "pk-000.000", 0x000000, 0x80000, CRC(165ce027) SHA1(3510b323c683ade4dd7307b539072bb342b6796d) )
	ROM_LOAD16_BYTE( "pk-001.001", 0x000001, 0x80000, CRC(e2745147) SHA1(99026525449c2ca84e054a7d633c400e0e836461) )
	ROM_LOAD16_BYTE( "pk-010.010", 0x100000, 0x80000, CRC(6c171b73) SHA1(a99c9f012f21373daea08d28554cc36170f4e1fa) )
	ROM_LOAD16_BYTE( "pk-011.011", 0x100001, 0x80000, CRC(471c0bf4) SHA1(1cace5ffd5db91850662de929cb9086dc154d662) )
	ROM_LOAD16_BYTE( "pk-020.020", 0x200000, 0x80000, CRC(c886dad1) SHA1(9b58a2f108547c3f55399932a7e56031c5658737) )
	ROM_LOAD16_BYTE( "pk-021.021", 0x200001, 0x80000, CRC(91e877ff) SHA1(3df095632728ab16ab229d592ab12d3df44b2629) )
	ROM_LOAD16_BYTE( "pk-030.030", 0x300000, 0x80000, CRC(3390621c) SHA1(4138c690666f78b1c5cf83d815ed6b37239a94b4) )
	ROM_LOAD16_BYTE( "pk-031.031", 0x300001, 0x80000, CRC(4d322804) SHA1(b5e2b40e3ce83b6f97b2b57edaa79df6968d0997) )

	ROM_REGION( 0x100000, "irem", 0 )    /* ADPCM samples */
	ROM_LOAD( "pk-da0.da0", 0x000000, 0x80000, CRC(26a34cf4) SHA1(a8a7cd91cdc6d644ee02ca16e7fdc8debf8f3a5f) )
ROM_END

ROM_START( kftgoal )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pk-h0-usa-d.h0", 0x000001, 0x040000, CRC(aed4cde0) SHA1(2fe04bf93c353108b82a0b6017229e9b0f451b06) )
	ROM_LOAD16_BYTE( "pk-l0-usa-d.l0", 0x000000, 0x040000, CRC(39fe30d2) SHA1(e0c117da4fe9c779dd534ee0d09685aeb5f579c6) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "pk-sh0.sh0", 0x00001, 0x10000, CRC(1145998c) SHA1(cdb2a428e0f35302b81696dab02d3dd2c433f6e5) )
	ROM_LOAD16_BYTE( "pk-sl0.sl0", 0x00000, 0x10000, CRC(542ee1c7) SHA1(b934adeecbba17cf96b06a2b1dc1ceaebdf9ad10) )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* chars */
	ROM_LOAD16_BYTE( "pk-c00-os.c00", 0x000000, 0x80000, CRC(42ae3d73) SHA1(e4777066155c9882695ebff0412bd879b8d6f716) )
	ROM_LOAD16_BYTE( "pk-c10-os.c10", 0x000001, 0x80000, CRC(86acf45c) SHA1(3b3d2abcf8000161a37d5e2619df529533aea47d) )
	ROM_LOAD16_BYTE( "pk-c01-os.c01", 0x100000, 0x80000, CRC(b0d33f87) SHA1(f2c0e3a10615c6861a3f6fd82a3f066e8e264233) )
	ROM_LOAD16_BYTE( "pk-c11-os.c11", 0x100001, 0x80000, CRC(19de7d63) SHA1(6d0633e412b47accaecc887a5c39f542eda49e81) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites */
	ROM_LOAD16_BYTE( "pk-000-usa.000", 0x000000, 0x80000, CRC(72e905ab) SHA1(5f47f0db0a19311cb74c39ea9d86f6909b926aa8) )
	ROM_LOAD16_BYTE( "pk-001-usa.001", 0x000001, 0x80000, CRC(eec4f43c) SHA1(93133389701c8752fc288f3f82da2646446804ca) )
	ROM_LOAD16_BYTE( "pk-010-usa.010", 0x100000, 0x80000, CRC(b3339d73) SHA1(1f59591a24434cf7d754d211c1a20591c1d7234c) )
	ROM_LOAD16_BYTE( "pk-011-usa.011", 0x100001, 0x80000, CRC(bab2b7cf) SHA1(53997c5dc204a4f510624dcdef949e859ad79d23) )
	ROM_LOAD16_BYTE( "pk-020-usa.020", 0x200000, 0x80000, CRC(740a0bef) SHA1(89782d6d76e0cbd99047dc9a4d3c00bbab0d6bce) )
	ROM_LOAD16_BYTE( "pk-021-usa.021", 0x200001, 0x80000, CRC(f44208a6) SHA1(e6436bacebca786de1521ce7a207aca686e312a0) )
	ROM_LOAD16_BYTE( "pk-030-usa.030", 0x300000, 0x80000, CRC(8eceef50) SHA1(e39a2420a6259a8571a71fd3f9b003b0e0abea3b) )
	ROM_LOAD16_BYTE( "pk-031-usa.031", 0x300001, 0x80000, CRC(8aa7dc04) SHA1(8aebdf50a832acf00fcfebb35ab49a06d13bc444) )

	ROM_REGION( 0x100000, "irem", 0 )    /* ADPCM samples */
	ROM_LOAD( "pk-da0.da0", 0x000000, 0x80000, BAD_DUMP CRC(26a34cf4) SHA1(a8a7cd91cdc6d644ee02ca16e7fdc8debf8f3a5f) ) //clearly taken from World PK Soccer, it says "World PK Soccer" at title screen

	ROM_REGION( 0x2000, "eeprom", 0 ) /* ST M28C64C-20PI EEPROM */
	ROM_LOAD( "st-m28c64c.eeprom", 0x000, 0x2000, CRC(8e0c8b7c) SHA1(0b57290d709e6d54ce1bb3a5c01b80590203c1dd) )
ROM_END

/***************************************************************************/

void m107_state::init_firebarr()
{
	m_spritesystem = 1;
}

void m107_state::init_wpksoc()
{
	m_spritesystem = 0;
}

void m107_state::init_dsoccr94()
{
	uint8_t *ROM = memregion("maincpu")->base();

	m_mainbank->configure_entries(0, 4, &ROM[0x80000], 0x20000);

	init_wpksoc();
}

/***************************************************************************/

GAME( 1993, airass,    0,        airass,   firebarr, m107_state, init_firebarr, ROT270, "Irem", "Air Assault (World)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // possible location test, but sound code is newer than Japan version
GAME( 1993, firebarr,  airass,   firebarr, firebarr, m107_state, init_firebarr, ROT270, "Irem", "Fire Barrel (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1994, dsoccr94,  0,        dsoccr94, dsoccr94, m107_state, init_dsoccr94, ROT0,   "Irem (Data East Corporation license)", "Dream Soccer '94 (World, M107 hardware)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1994, dsoccr94k, dsoccr94, dsoccr94, dsoccr94, m107_state, init_dsoccr94, ROT0,   "Irem (Data East Corporation license)", "Dream Soccer '94 (Korea, M107 hardware)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // default team selected is Korea, so likely a Korean set

GAME( 1995, wpksoc,    0,        wpksoc,   wpksoc,   m107_state, init_wpksoc,   ROT0,   "Jaleco", "World PK Soccer",   MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1994, kftgoal,   wpksoc,   wpksoc,   wpksoc,   m107_state, init_wpksoc,   ROT0,   "Jaleco", "Kick for the Goal", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
