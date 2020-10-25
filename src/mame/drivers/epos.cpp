// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

    Epos games

    driver by Zsolt Vasvari


    Notes:

    - To walk in IGMO, hold down button 2.
    - Super Glob seems like a later revision of The Glob, the most obvious
      difference being an updated service mode.
    - These games don't have cocktail mode.
    - The CPU clock divisor 4 was derived using the timing loop used to split
      the screen in the middle.  This loop takes roughly 24200 cycles, giving
      2500 + (24200 - 2500) * 2 * 60 = 2754000 = 2.75MHz for the CPU speed,
      assuming 60 fps and a 2500 cycle VBLANK period.
      This also matches the IGMO schematic, as it is the /CLK signal, which is
      derived from the 11MHz xtal by dividing it down by two 74LS193 chips, one
      (U92) dividing the clock by 2, and another (U91) having 3 taps, further
      dividing the already divided clock by 2 (/CLK), 4 (/PLOAD) and 8 (CLOCK).
      The CLOCK signal drives the AY.
    - I think theglob2 is earlier than theglob.  They only differ in one routine,
      but it appears to be a bug fix.  Also, theglob3 appears to be even older.

    To Do:

    - Super Blob uses a busy loop during the color test to split the screen
      between the two palettes.  This effect is not emulated, but since both
      halves of the palette are identical, this is not an issue.  See $039c.
      The other games have a different color test, not using the busy loop.

    - dealer/beastf/revngr84: "PSG registers not OK" in service mode thru
      sound menu, internal ay8910 not right?

***************************************************************************/

#include "emu.h"
#include "includes/epos.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/watchdog.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "speaker.h"


void epos_state::dealer_decrypt_rom(offs_t offset, uint8_t data)
{
	if (offset & 0x04)
		m_counter = (m_counter + 1) & 0x03;
	else
		m_counter = (m_counter - 1) & 0x03;

	//logerror("PC %08x: ctr=%04x\n",m_maincpu->pc(), m_counter);

	membank("bank1")->set_entry(m_counter);

	// is the 2nd bank changed by the counter or it always uses the 1st key?
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void epos_state::epos_map(address_map &map)
{
	map(0x0000, 0x77ff).rom();
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xffff).ram().share("videoram");
}

void epos_state::dealer_map(address_map &map)
{
	map(0x0000, 0x5fff).bankr("bank1");
	map(0x6000, 0x6fff).bankr("bank2");
	map(0x7000, 0x7fff).ram().share("nvram");
	map(0x8000, 0xffff).ram().share("videoram");
}


/*************************************
 *
 *  Main CPU port handlers
 *
 *************************************/

void epos_state::epos_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("DSW").w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x01, 0x01).portr("SYSTEM").w(FUNC(epos_state::port_1_w));
	map(0x02, 0x02).portr("INPUTS").w("aysnd", FUNC(ay8910_device::data_w));
	map(0x03, 0x03).portr("UNK");
	map(0x06, 0x06).w("aysnd", FUNC(ay8910_device::address_w));
}

void epos_state::dealer_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x0f).w(FUNC(epos_state::dealer_pal_w));
	map(0x10, 0x13).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x20, 0x24).w(FUNC(epos_state::dealer_decrypt_rom));
	map(0x34, 0x34).w("aysnd", FUNC(ay8910_device::data_w));
	map(0x38, 0x38).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x3c, 0x3c).w("aysnd", FUNC(ay8910_device::address_w));
	map(0x40, 0x40).w("watchdog", FUNC(watchdog_timer_device::reset_w));
}

uint8_t epos_state::i8255_porta_r()
{
	uint8_t data = 0xff;

	if (!BIT(m_input_multiplex, 0))
		data &= m_inputs[0]->read();

	if (!BIT(m_input_multiplex, 1))
		data &= m_inputs[1]->read();

	return data;
}

/*
   ROMs U01-U03 are checked with the same code in a loop.
   There's a separate ROM check for banked U04 at 30F3.
   It looks like dealer/revenger uses ppi8255 to control bankswitching.
*/
void epos_state::i8255_portc_w(uint8_t data)
{
	membank("bank2")->set_entry(data & 0x01);
	m_input_multiplex = (data >> 5) & 3;
}

uint8_t epos_state::ay_porta_mpx_r()
{
	return (m_ay_porta_multiplex ? 0xFF : ioport("DSW")->read());
}

void epos_state::flip_screen_w(uint8_t data)
{
	flip_screen_set(BIT(data, 7));
	// bit 6: ay8910 port A/B multiplexer read
	m_ay_porta_multiplex = BIT(data, 6);
}


/*************************************
 *
 *  Port definitions
 *
 *************************************/

/* I think the upper two bits of port 1 are used as a simple form of protection,
   so that ROMs couldn't be simply swapped.  Each game checks these bits and halts
   the processor if an unexpected value is read. */

static INPUT_PORTS_START( megadon )
	// There are odd port mappings (old=new)
	// 02=10, 04=40, 08=02, 10=20, 20=04, 40=08
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x50, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x50, "6" )
	PORT_DIPNAME( 0x02, 0x00, "Fuel Consumption" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x02, "Fast" )
	PORT_DIPNAME( 0x20, 0x20, "Enemy Fire Rate" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x20, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x04, 0x00, "Rotation" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x08, 0x08, "ERG" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x00, "Game Mode" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Arcade" )
	PORT_DIPSETTING(    0x80, "Contest" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_SERVICE_NO_TOGGLE(0x10, IP_ACTIVE_LOW)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_CUSTOM )   /* this has to be HI */
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_CUSTOM )   /* this has to be HI */

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( suprglob )
	// There are odd port mappings (old=new)
	// 02=10, 04=40, 08=20, 10=02, 20=04, 40=08
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x50, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x50, "6" )
	PORT_DIPNAME( 0x26, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x22, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x24, "7" )
	PORT_DIPSETTING(    0x26, "8" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "10000 + Difficulty * 10000" )
	PORT_DIPSETTING(    0x08, "90000 + Difficulty * 10000" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_SERVICE_NO_TOGGLE(0x10, IP_ACTIVE_LOW)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM )   /* this has to be LO */
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_CUSTOM )   /* this has to be HI */

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( igmo )
	// There are odd port mappings (old=new)
	// 02=10, 04=40, 08=20, 10=02, 20=04, 40=08
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x50, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x50, "6" )
	PORT_DIPNAME( 0x22, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x02, "40000" )
	PORT_DIPSETTING(    0x20, "60000" )
	PORT_DIPSETTING(    0x22, "80000" )
	PORT_DIPNAME( 0x8c, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "4" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x84, "6" )
	PORT_DIPSETTING(    0x88, "7" )
	PORT_DIPSETTING(    0x8c, "8" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_SERVICE_NO_TOGGLE(0x10, IP_ACTIVE_LOW)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_CUSTOM )   /* this has to be HI */
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_CUSTOM )   /* this has to be HI */

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( catapult )
	PORT_INCLUDE( igmo )

	// There are odd port mappings (old=new)
	// 02=08, 04=20, 08=40, 10=02, 20=10, 40=04
	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x50, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x50, "6" )
	PORT_DIPNAME( 0x22, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x22, "4" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x04, "40000" )
	PORT_DIPSETTING(    0x08, "60000" )
	PORT_DIPSETTING(    0x0c, "80000" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( eeekk )
	// There are odd port mappings (old=new)
	// 02=10, 04=40, 08=02, 10=20, 20=04, 40=08
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x50, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x50, "6" )
	PORT_DIPNAME( 0x26, 0x06, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x00, "1 (Easy)" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x22, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x24, "7" )
	PORT_DIPSETTING(    0x26, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, "Extra Life Range" ) PORT_DIPLOCATION("SW1:7") // exact points value varies by 10000 for every level of difficulty chosen via the dips above
	PORT_DIPSETTING(    0x08, "100000 - 170000 points" )
	PORT_DIPSETTING(    0x00, "20000 - 90000 points" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_SERVICE_NO_TOGGLE(0x10, IP_ACTIVE_LOW)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM )   /* this has to be LO */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM )   /* this has to be LO */

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( dealer )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INPUTS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME( "Draw" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_STAND )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME( "Play" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE )
INPUT_PORTS_END

static INPUT_PORTS_START( beastf )
	PORT_INCLUDE( dealer )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_MODIFY("INPUTS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

MACHINE_START_MEMBER(epos_state,epos)
{
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_counter));
	save_item(NAME(m_input_multiplex));
	save_item(NAME(m_ay_porta_multiplex));
}

void epos_state::machine_reset()
{
	m_palette_bank = 0;
	m_counter = 0;
	m_input_multiplex = 3;
	m_ay_porta_multiplex = 0;
}


MACHINE_START_MEMBER(epos_state,dealer)
{
	uint8_t *ROM = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 4, &ROM[0x0000], 0x10000);
	membank("bank2")->configure_entries(0, 2, &ROM[0x6000], 0x1000);

	membank("bank1")->set_entry(0);
	membank("bank2")->set_entry(0);

	MACHINE_START_CALL_MEMBER(epos);
}

void epos_state::epos(machine_config &config) /* EPOS TRISTAR 8000 PCB */
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(11'000'000)/4);    /* 2.75 MHz schematics confirm 11MHz XTAL (see notes) */
	m_maincpu->set_addrmap(AS_PROGRAM, &epos_state::epos_map);
	m_maincpu->set_addrmap(AS_IO, &epos_state::epos_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(epos_state::irq0_line_hold));

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(272, 241);
	screen.set_visarea(0, 271, 0, 235);
	screen.set_screen_update(FUNC(epos_state::screen_update));

	PALETTE(config, m_palette, FUNC(epos_state::epos_palette), 32);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AY8912(config, "aysnd", XTAL(11'000'000)/16).add_route(ALL_OUTPUTS, "mono", 1.0); /*  0.6875 MHz, confirmed from schematics */
}


void epos_state::dealer(machine_config &config) /* EPOS TRISTAR 9000 PCB */
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(22'118'400)/8);    /* 2.7648 MHz (measured) */
	m_maincpu->set_addrmap(AS_PROGRAM, &epos_state::dealer_map);
	m_maincpu->set_addrmap(AS_IO, &epos_state::dealer_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(epos_state::irq0_line_hold));

	i8255_device &ppi(I8255A(config, "ppi8255"));
	ppi.in_pa_callback().set(FUNC(epos_state::i8255_porta_r));
	ppi.out_pc_callback().set(FUNC(epos_state::i8255_portc_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	MCFG_MACHINE_START_OVERRIDE(epos_state,dealer)

	// RAM-based palette instead of prom
	PALETTE(config, m_palette, palette_device::BLACK, 32);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(272, 241);
	screen.set_visarea(0, 271, 0, 235);
	screen.set_screen_update(FUNC(epos_state::screen_update));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, "aysnd", XTAL(22'118'400)/32)); /* 0.6912 MHz (measured) */
	aysnd.add_route(ALL_OUTPUTS, "mono", 1.0);
	aysnd.port_a_read_callback().set(FUNC(epos_state::ay_porta_mpx_r));
	// port a writes?
	aysnd.port_b_write_callback().set(FUNC(epos_state::flip_screen_w)); // flipscreen and ay port a multiplex control
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

// Tristar 8000 boards:
ROM_START( megadon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2732u10b.bin",   0x0000, 0x1000, CRC(af8fbe80) SHA1(2d7857616462112fe17343a9357ee51d8f965a0f) )
	ROM_LOAD( "2732u09b.bin",   0x1000, 0x1000, CRC(097d1e73) SHA1(b6141155b2c63c33a367dd18fe53ff9f01b99380) )
	ROM_LOAD( "2732u08b.bin",   0x2000, 0x1000, CRC(526784da) SHA1(7d9f43dc6975a018bec95982029ce7ac9f675869) )
	ROM_LOAD( "2732u07b.bin",   0x3000, 0x1000, CRC(5b060910) SHA1(98a719bf0ffe8010437565de681aaefa647d9a6c) )
	ROM_LOAD( "2732u06b.bin",   0x4000, 0x1000, CRC(8ac8af6d) SHA1(53c123f0e9f0443737c39c01dbdb685189cffa92) )
	ROM_LOAD( "2732u05b.bin",   0x5000, 0x1000, CRC(052bb603) SHA1(eb74a9563f44cca50dc2c475e4a376ed14e4f56f) )
	ROM_LOAD( "2732u04b.bin",   0x6000, 0x1000, CRC(9b8b7e92) SHA1(051ad9a8ba51740a865e3c95a738658b30bbbe60) )
	ROM_LOAD( "2716u11b.bin",   0x7000, 0x0800, CRC(599b8b61) SHA1(e687c6f475a0fead3e47f05b1d1b3b29cf4a83a1) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "74s288.bin",     0x0000, 0x0020, CRC(c779ea99) SHA1(7702ae3684579950b36274ea91d4267c96faeeb8) )
ROM_END


ROM_START( catapult )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "co3223.u10",     0x0000, 0x1000, CRC(50abcfd2) SHA1(13ce04addc7bcaa1ec6659da26b1c13ed9dc28f9) )
	ROM_LOAD( "co3223.u09",     0x1000, 0x1000, CRC(fd5a9a1c) SHA1(512374e8450459537ba2cc41e7d0178052445316) )
	ROM_LOAD( "co3223.u08",     0x2000, 0x1000, BAD_DUMP CRC(4bfc36f3) SHA1(b916805eed40cfeff0c1b0cb3cdcbcc6e362a236)  ) /* BADADDR xxxx-xxxxxxx */
	ROM_LOAD( "co3223.u07",     0x3000, 0x1000, CRC(4113bb99) SHA1(3cebb874dae211d75082209e913d4afa4f621de1) )
	ROM_LOAD( "co3223.u06",     0x4000, 0x1000, CRC(966bb9f5) SHA1(1a217c6f7a88c58e0deae0290bc5ddd2789d18eb) )
	ROM_LOAD( "co3223.u05",     0x5000, 0x1000, CRC(65f9fb9a) SHA1(63b616a736d9e39a8f2f76889fd7c5fe4128a966) )
	ROM_LOAD( "co3223.u04",     0x6000, 0x1000, CRC(648453bc) SHA1(8e4538aedad4d32bd046aad474dbcc689ee8fe53) )
	ROM_LOAD( "co3223.u11",     0x7000, 0x0800, CRC(08fb8c28) SHA1(0b08cc2727a54e0ad7472234be0f637b46bc3253) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "co3223.u66",     0x0000, 0x0020, CRC(e7de76a7) SHA1(101ce85459a59c0d01ce3ea96480f1f8413a788e) )
ROM_END


ROM_START( suprglob )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u10",            0x0000, 0x1000, CRC(c0141324) SHA1(a54bd71da233eb22f45da630693fddd5a0bcf25b) )
	ROM_LOAD( "u9",             0x1000, 0x1000, CRC(58be8128) SHA1(534f0a093b3ff577a2a5461498bc11ce14dc6d97) )
	ROM_LOAD( "u8",             0x2000, 0x1000, CRC(6d088c16) SHA1(0929ea1b58eab997b5d9c9642f8b47557a4045f1) )
	ROM_LOAD( "u7",             0x3000, 0x1000, CRC(b2768203) SHA1(9de52f4dbe6a46ea1b9b7f9cf70378211d372353) )
	ROM_LOAD( "u6",             0x4000, 0x1000, CRC(976c8f46) SHA1(120c76eff8c04ccb5ad945c4333e8c9de0cbc3af) )
	ROM_LOAD( "u5",             0x5000, 0x1000, CRC(340f5290) SHA1(2e5fa0c41d1626e5a435f2c55eec0bcdcb004223) )
	ROM_LOAD( "u4",             0x6000, 0x1000, CRC(173bd589) SHA1(25690a0c3cd0e017f8d220d8fbf2eaeb86f05fc5) )
	ROM_LOAD( "u11",            0x7000, 0x0800, CRC(d45b740d) SHA1(54c15f378b6d91ea1aba0a51921178bb15854079) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.u66",     0x0000, 0x0020, CRC(f4f6ddc5) SHA1(cab915acbefb5f451f538dd538bf9b3dd14bb1f5) )
ROM_END


ROM_START( theglob )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "globu10.bin",    0x0000, 0x1000, CRC(08fdb495) SHA1(739efa676b5a3df36a6061382aeb8c2d495ba23f) )
	ROM_LOAD( "globu9.bin",     0x1000, 0x1000, CRC(827cd56c) SHA1(3aedc1cefb463cf6b31befb33e50c832dc2e3941) )
	ROM_LOAD( "globu8.bin",     0x2000, 0x1000, CRC(d1219966) SHA1(571349f9c978fdcf826a0c66c3fb11a9e27b240a) )
	ROM_LOAD( "globu7.bin",     0x3000, 0x1000, CRC(b1649da7) SHA1(1509d48a72e545195e45d1170cdb113c6aecc8d9) )
	ROM_LOAD( "globu6.bin",     0x4000, 0x1000, CRC(b3457e67) SHA1(1347bdf085ad69879f9a9e7e4ed1ca4869e8e8cd) )
	ROM_LOAD( "globu5.bin",     0x5000, 0x1000, CRC(89d582cd) SHA1(f331c7a2fce606153992abb312c5406251a7fb3b) )
	ROM_LOAD( "globu4.bin",     0x6000, 0x1000, CRC(7ee9fdeb) SHA1(a8e0dd5d1cdcff132edc0eb182b66656ce244fa1) )
	ROM_LOAD( "globu11.bin",    0x7000, 0x0800, CRC(9e05dee3) SHA1(751799b23f0e664f59d3785b438ec3ae9f5bab2c) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.u66",     0x0000, 0x0020, CRC(f4f6ddc5) SHA1(cab915acbefb5f451f538dd538bf9b3dd14bb1f5) )
ROM_END


ROM_START( theglob2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "611293.u10",     0x0000, 0x1000, CRC(870af7ce) SHA1(f901619663313a72997f30ccecdeac8294fe200e) )
	ROM_LOAD( "611293.u9",      0x1000, 0x1000, CRC(a3679782) SHA1(fbc26ae98e2bf10272d61159b084d78a6f410374) )
	ROM_LOAD( "611293.u8",      0x2000, 0x1000, CRC(67499d1a) SHA1(dce7041df5ed1847e0ffc82672d09e00b16de3a9) )
	ROM_LOAD( "611293.u7",      0x3000, 0x1000, CRC(55e53aac) SHA1(20a428db287e8b7fb55cb9fe1a1ed0196481114c) )
	ROM_LOAD( "611293.u6",      0x4000, 0x1000, CRC(c64ad743) SHA1(572ff6acb9b2281581974646e96699d7d2388aff) )
	ROM_LOAD( "611293.u5",      0x5000, 0x1000, CRC(f93c3203) SHA1(8cb88b5202e99d206eccf7d25e168cf23acee19b) )
	ROM_LOAD( "611293.u4",      0x6000, 0x1000, CRC(ceea0018) SHA1(511430539429ef0e5368f7b605f2e680ca9038bc) )
	ROM_LOAD( "611293.u11",     0x7000, 0x0800, CRC(6ac83f9b) SHA1(b1e8482ec04107f0e595a714b7c0f70571aca6e5) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.u66",     0x0000, 0x0020, CRC(f4f6ddc5) SHA1(cab915acbefb5f451f538dd538bf9b3dd14bb1f5) )
ROM_END


ROM_START( theglob3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "theglob3.u10",   0x0000, 0x1000, CRC(969cfaf6) SHA1(b63226b8694640d6452bca12755780d1b52d1d3c) )
	ROM_LOAD( "theglob3.u9",    0x1000, 0x1000, CRC(8e6c010a) SHA1(ec9627742ce52eb29bbafc9d0555d16ac7146f2e) )
	ROM_LOAD( "theglob3.u8",    0x2000, 0x1000, CRC(1c1ca5c8) SHA1(6e5f9d7f9f016a72003433375c806c5f921ed423) )
	ROM_LOAD( "theglob3.u7",    0x3000, 0x1000, CRC(a54b9d22) SHA1(3db96d1f55642ecf1ebc76387cac76e8f9721919) )
	ROM_LOAD( "theglob3.u6",    0x4000, 0x1000, CRC(5a6f82a9) SHA1(ea92ad949373e8b1f06c65f243ceedad2fdcd934) )
	ROM_LOAD( "theglob3.u5",    0x5000, 0x1000, CRC(72f935db) SHA1(d7023cf5f16a77a42590a9c97c2690ac0e3d282a) )
	ROM_LOAD( "theglob3.u4",    0x6000, 0x1000, CRC(81db53ad) SHA1(a1e4aa8e08ca0f585b3638a3849a465977d44af0) )
	ROM_LOAD( "theglob3.u11",   0x7000, 0x0800, CRC(0e2e6359) SHA1(f231637ad4c997406989cf5a701d26c95e69171e) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.u66",     0x0000, 0x0020, CRC(f4f6ddc5) SHA1(cab915acbefb5f451f538dd538bf9b3dd14bb1f5) )
ROM_END


ROM_START( igmo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u10_igmo_i05134.u10", 0x0000, 0x1000, CRC(a9f691a4) SHA1(e3f2dc41bd8760fc52e99b7e9faa12c7cf51ffe0) )
	ROM_LOAD( "u9_igmo_i05134.u9",   0x1000, 0x1000, CRC(3c133c97) SHA1(002b5aff6b947b6a9cbabeed5be798c1ddf2bda1) )
	ROM_LOAD( "u8_igmo_i05134.u8",   0x2000, 0x1000, CRC(5692f8d8) SHA1(6ab50775dff49330a85fbfb2d4d4c3a2e54df3d1) )
	ROM_LOAD( "u7_igmo_i05134.u7",   0x3000, 0x1000, CRC(630ae2ed) SHA1(0c293b6192e703b16ed20c277c706ae90773f477) )
	ROM_LOAD( "u6_igmo_i05134.u6",   0x4000, 0x1000, CRC(d3f20e1d) SHA1(c0e0b542ac020adc085ec90c2462c6544098447e) )
	ROM_LOAD( "u5_igmo_i05134.u5",   0x5000, 0x1000, CRC(e26bb391) SHA1(ba0e44c02fbb36e18e0d779d46bb992e6aba6cf1) )
	ROM_LOAD( "u4_igmo_i05134.u4",   0x6000, 0x1000, CRC(762a4417) SHA1(7fed5221950e3e1ce41c0b4ded44597a242a0177) )
	ROM_LOAD( "u11_igmo_i05134.u11", 0x7000, 0x0800, CRC(8c675837) SHA1(2725729693960b53ea01ebffa0a81df2cd425890) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.u66",          0x0000, 0x0020, CRC(1ba03ffe) SHA1(f5692c06ae6d20c010430c8d08f5c60e78d36dc9) )
ROM_END


ROM_START( eeekk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u10_e12063.u10", 0x0000, 0x1000, CRC(edd05de2) SHA1(25dfa7ad2e29b1ca9ce9bb36bf1a573baabb4d5b) )
	ROM_LOAD( "u9_e12063.u9",   0x1000, 0x1000, CRC(6f57114a) SHA1(417b910a4343da026426b4cfd0a83b9142c87353) )
	ROM_LOAD( "u8_e12063.u8",   0x2000, 0x1000, CRC(bcb0ebbd) SHA1(a2a00dedee12d6006817021e98fb44b2339127a0) )
	ROM_LOAD( "u7_e12063.u7",   0x3000, 0x1000, CRC(a0df8f77) SHA1(ee2afed25ab32bf09b14e8638d03b6e2f8e6b337) )
	ROM_LOAD( "u6_e12063.u6",   0x4000, 0x1000, CRC(61953b0a) SHA1(67bcb4286e39cdda20684a4f580392468b08800e) )
	ROM_LOAD( "u5_e12063.u5",   0x5000, 0x1000, CRC(4c22c6d9) SHA1(94a8fc951994746f8ccfb77d80f8b98fde8a6f33) )
	ROM_LOAD( "u4_e12063.u4",   0x6000, 0x1000, CRC(3d341208) SHA1(bc4d2567df2779d97e718376c4bf682ba459c01e) )
	ROM_LOAD( "u11_e12063.u11", 0x7000, 0x0800, CRC(417faff0) SHA1(7965155ee32694ea9a10245db73d8beef229408c) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "74s288.u66",     0x0000, 0x0020, CRC(f2078c38) SHA1(7bfd49932a6fd8840514b7af93a64cedb248122d) )
ROM_END


// Tristar 9000 boards:
ROM_START( dealer )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "u1.bin",         0x0000, 0x2000, CRC(e06f3563) SHA1(0d58cd1f2e1ca89adb9c64d7dd520bb1f2d50f1a) )
	ROM_LOAD( "u2.bin",         0x2000, 0x2000, CRC(726bbbd6) SHA1(3538f3d655899c2a0f984c43fb7545ea4be1b231) )
	ROM_LOAD( "u3.bin",         0x4000, 0x2000, CRC(ab721455) SHA1(a477da0590e0431172baae972e765473e19dcbff) )
	ROM_LOAD( "u4.bin",         0x6000, 0x2000, CRC(ddb903e4) SHA1(4c06a2048b1c6989c363b110a17c33180025b9c8) )

	ROM_REGION( 0x1000, "nvram", 0)
	ROM_LOAD( "dealer.nv", 0, 0x1000, CRC(a6f88459) SHA1(1deda2a71433c97fe3e5cb39defc285f4fa9c9b8) )
ROM_END

/*

Revenger 84 - EPOS

EPOS TRISTAR 9000
+-------------------------------+
|                               |
|  DSW                          |
|             4116 4116         |
| 8910  Z80A  4116 4116         |
|             4116 4116         |
|  6116  BAT  4116 4116         |
|  6116       4116 4116         |
|  U4         4116 4116  74S189 |
|  U3         4116 4116  74S189 |
|  U2 PAL.U13 4116 4116         |
|  U1 PAL10L8 8255  DM74S288N   |
|                     22.1184MHz|
|                               |
|                      LM384N   |
|                        VOL    |
+--|28 pin Connector|-----------+

  CPU: Z80A        2.764800 MHz [22.1184MHz/8]
Sound: AY-3-8910   0.691200 MHz [22.1184MHz/32]
 XTAL: 22.1184MHz
  DSW: 8 position dipswitch bank
  BAT: Battery (battery backed up RAM for high scores)
  VOL: Volume Pot

28 Pin non-JAMMA connector

 4 ROMs at U1 through U4 are 2764 type
 BPROM at U60 is a DM74S288N
 2 pals, U12 is a PAL10L8, U13 PAL type is unknown (markings scrubbed off)

4116 RAS is = 2.764800 MHz [22.1184MHz/8]
4116 CAS is = 1.382400 MHz [22.1184MHz/16]

*/

ROM_START( revngr84 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "u_1__revenger__r06254__=c=_epos_corp.m5l2764k.u1",  0x0000, 0x2000, CRC(308f231f) SHA1(cf06695601bd0387e4fcb64d9b34143323e98b07) ) /* labeled as "U 1 // REVENGER // R06254 // (C) EPOS CORP" (hand written R06254 over R06124) */
	ROM_LOAD( "u_2__revenger__r06254__=c=_epos_corp.m5l2764k.u2",  0x2000, 0x2000, CRC(e80bbfb4) SHA1(9302beaef8bbb7376b6a20e9ee5adbcf60d66dd8) ) /* labeled as "U 2 // REVENGER // R06254 // (C) EPOS CORP" (hand written R06254 over R06124) */
	ROM_LOAD( "u_3__revenger__r06254__=c=_epos_corp.m5l2764k.u3",  0x4000, 0x2000, CRC(d9270929) SHA1(a95034b5387a40e02f04bdfa79e1d8e65dad30fe) ) /* labeled as "U 3 // REVENGER // R06254 // (C) EPOS CORP" (hand written R06254 over R06124) */
	ROM_LOAD( "u_4__revenger__r06254__=c=_epos_corp.m5l2764k.u4",  0x6000, 0x2000, CRC(d6e6cfa8) SHA1(f10131bb2e9d088c7b6d6a5d5520073d78ad69cc) ) /* labeled as "U 4 // REVENGER // R06254 // (C) EPOS CORP" (hand written R06254 over R06124) */

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "dm74s288n.u60", 0x0000, 0x0020, CRC(be2b0641) SHA1(26982903b6d942af8e0a526412d8e01978d76420) ) // unknown purpose

	ROM_REGION( 0x1000, "nvram", 0)
	ROM_LOAD( "revngr84.nv", 0, 0x1000, CRC(a4417770) SHA1(92eded82db0810e7818d2f52a0497032f390fcc1) )
ROM_END

ROM_START( revenger )
	ROM_REGION( 0x40000, "maincpu", 0 )
	// these roms probably had the same "U x // REVENGER // R06124 // (C) EPOS CORP" printed labels as the newer set above, but without the hand-penned "25" in r06254 written over the printed "12" of r06124 as above
	ROM_LOAD( "r06124.u1",    0x0000, 0x2000, BAD_DUMP CRC(fad1a2a5) SHA1(a31052c91fe67e2e90441abc40b6483f921ecfe3) )
	ROM_LOAD( "r06124.u2",    0x2000, 0x2000, BAD_DUMP CRC(a8e0ee7b) SHA1(f6f78e8ce40eab07de461b364876c1eb4a78d96e) )
	ROM_LOAD( "r06124.u3",    0x4000, 0x2000, BAD_DUMP CRC(cca414a5) SHA1(1c9dd3ff63d57e9452e63083cdbd7f5d693bb686) )
	ROM_LOAD( "r06124.u4",    0x6000, 0x2000, BAD_DUMP CRC(0b81c303) SHA1(9022d18dec11312eb4bb471c22b563f5f897b4f7) )

	ROM_REGION( 0x0020, "proms", 0 ) /* this PROM not included in this dump, but assumed to be the same as above set */
	ROM_LOAD( "dm74s288n.u60", 0x0000, 0x0020, CRC(be2b0641) SHA1(26982903b6d942af8e0a526412d8e01978d76420) ) // unknown purpose

	ROM_REGION( 0x1000, "nvram", 0)
	ROM_LOAD( "revngr84.nv", 0, 0x1000, CRC(a4417770) SHA1(92eded82db0810e7818d2f52a0497032f390fcc1) )
ROM_END

ROM_START( beastf )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "u_1__beastie__feastie__b09084.m5l2764k.u1",  0x0000, 0x2000, CRC(820d4019) SHA1(e953aaeeb626776dd86c521066b553d054ae4422) ) /* labeled as "U 1 // BEASTIE // FEASTIE // B09084" */
	ROM_LOAD( "u_2__beastie__feastie__b09084.m5l2764k.u2",  0x2000, 0x2000, CRC(967405d8) SHA1(dd763be909e6966521b01ee878df9cef865c3b30) ) /* labeled as "U 2 // BEASTIE // FEASTIE // B09084" */
	ROM_LOAD( "u_3__beastie__feastie__b09084.m5l2764k.u3",  0x4000, 0x2000, CRC(3edb5381) SHA1(14c236045e6df7a475c32222652860689d4f68ce) ) /* labeled as "U 3 // BEASTIE // FEASTIE // B09084" */
	ROM_LOAD( "u_4__beastie__feastie__b09084.m5l2764k.u4",  0x6000, 0x2000, CRC(c8cd9640) SHA1(72da881b903ead873cc3f4df27646d1ffdd63c1c) ) /* labeled as "U 4 // BEASTIE // FEASTIE // B09084" */

	ROM_REGION( 0x1000, "nvram", 0)
	ROM_LOAD( "beastf.nv", 0, 0x1000, CRC(98017b09) SHA1(0e2b2071bb47fc179d5bc36ef9431a9d2727d36a) )
ROM_END

void epos_state::init_dealer()
{
	uint8_t *rom = memregion("maincpu")->base();

	/* Key 0 */
	for (int A = 0; A < 0x8000; A++)
		rom[A] = bitswap<8>(rom[A] ^ 0xbd, 2,6,4,0,5,7,1,3 );

	/* Key 1 */
	for (int A = 0; A < 0x8000; A++)
		rom[A + 0x10000] = bitswap<8>(rom[A], 7,5,4,6,3,2,1,0 );

	/* Key 2 */
	for (int A = 0; A < 0x8000; A++)
		rom[A + 0x20000] = bitswap<8>(rom[A] ^ 1, 7,6,5,4,3,0,2,1 );

	/* Key 3 */
	for (int A = 0; A < 0x8000; A++)
		rom[A + 0x30000] = bitswap<8>(rom[A] ^ 1, 7,5,4,6,3,0,2,1 );

	/*
	    there is not enough data to determine key 3.
	    the code in question is this:

	    [this is the data as decrypted by Key 1]
	    2F58: 55 5C 79
	    2F5B: 55 F7 79
	    2F5E: 55 CD 79

	    it must become

	    2F58: 32 3e 78  ld (793e),a
	    2F5B: 32 xx 78  ld (79xx),a
	    2F5E: 32 xx 78  ld (79xx),a

	    the obvious solution is a combination of key 1 and key 2.
	*/
}


/*************************************
 *
 *  Game drivers
 *
 *************************************/

/* EPOS TRISTAR 8000 PCB based */
GAME( 1982, megadon,  0,        epos,   megadon,  epos_state, empty_init, ROT270, "Epos Corporation (Photar Industries license)", "Megadon", MACHINE_SUPPORTS_SAVE )
GAME( 1982, catapult, 0,        epos,   catapult, epos_state, empty_init, ROT270, "Epos Corporation", "Catapult",           MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* bad rom, hold f2 for test mode */
GAME( 1983, suprglob, 0,        epos,   suprglob, epos_state, empty_init, ROT270, "Epos Corporation", "Super Glob",         MACHINE_SUPPORTS_SAVE )
GAME( 1983, theglob,  suprglob, epos,   suprglob, epos_state, empty_init, ROT270, "Epos Corporation", "The Glob",           MACHINE_SUPPORTS_SAVE )
GAME( 1983, theglob2, suprglob, epos,   suprglob, epos_state, empty_init, ROT270, "Epos Corporation", "The Glob (earlier)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, theglob3, suprglob, epos,   suprglob, epos_state, empty_init, ROT270, "Epos Corporation", "The Glob (set 3)",   MACHINE_SUPPORTS_SAVE )
GAME( 1984, igmo,     0,        epos,   igmo,     epos_state, empty_init, ROT270, "Epos Corporation", "IGMO",               MACHINE_SUPPORTS_SAVE )
GAME( 1983, eeekk,    0,        epos,   eeekk,    epos_state, empty_init, ROT270, "Epos Corporation", "Eeekk!",             MACHINE_SUPPORTS_SAVE )

/* EPOS TRISTAR 9000 PCB based */
GAME( 1984, dealer,   0,        dealer, dealer,   epos_state, init_dealer, ROT270, "Epos Corporation", "The Dealer",           MACHINE_SUPPORTS_SAVE )
GAME( 1984, revngr84, 0,        dealer, beastf,   epos_state, init_dealer, ROT270, "Epos Corporation", "Revenger '84 (newer)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, revenger, revngr84, dealer, beastf,   epos_state, init_dealer, ROT270, "Epos Corporation", "Revenger '84 (older)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1984, beastf,   suprglob, dealer, beastf,   epos_state, init_dealer, ROT270, "Epos Corporation", "Beastie Feastie",      MACHINE_SUPPORTS_SAVE )
