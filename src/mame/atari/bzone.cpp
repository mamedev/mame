// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Nicola Salmoria
/***************************************************************************

    Atari Battlezone hardware

    Games supported:
        * Battlezone
        * Bradley Trainer
        * Red Baron
        * Desert Wars (Spanish bootleg of Battle Zone)

    Known bugs:
        * None at this time on the Atari sets (Desert Wars isn't working)

****************************************************************************

    Battlezone memory map (preliminary)

    0000-03ff RAM
    0800      IN0
    0a00      IN1
    0c00      IN2

    1200      Vector generator start (write)
    1400      Watchdog Clear
    1600      Vector generator reset (write)

    1800      Mathbox Status register
    1810      Mathbox value (lo-byte)
    1818      Mathbox value (hi-byte)
    1820-182f POKEY I/O
    1828      Control inputs
    1860-187f Mathbox RAM

    2000-27ff Vector generator RAM (2K)
    2800-2fff Vector generator RAM / Vector ROM (2K)
    3000-37ff Vector Generator ROM (4K)
    5000-5fff Program ROM (4K)
    6000-7fff Program ROM (8K)

    Battlezone settings:

    0 = OFF  1 = ON  X = Don't Care  $ = Atari suggests

    ** IMPORTANT - BITS are INVERTED in the game itself **

    TOP 8 SWITCH DIP
    87654321
    --------
    XXXXXX11   Free Play
    XXXXXX10   1 coin for 2 plays
    XXXXXX01   1 coin for 1 play
    XXXXXX00   2 coins for 1 play
    XXXX11XX   Right coin mech x 1
    XXXX10XX   Right coin mech x 4
    XXXX01XX   Right coin mech x 5
    XXXX00XX   Right coin mech x 6
    XXX1XXXX   Center (or Left) coin mech x 1
    XXX0XXXX   Center (or Left) coin mech x 2
    111XXXXX   No bonus coin
    110XXXXX   For every 2 coins inserted, game logic adds 1 more
    101XXXXX   For every 4 coins inserted, game logic adds 1 more
    100XXXXX   For every 4 coins inserted, game logic adds 2 more
    011XXXXX   For every 5 coins inserted, game logic adds 1 more

    BOTTOM 8 SWITCH DIP
    87654321
    --------
    XXXXXX11   Game starts with 2 tanks
    XXXXXX10   Game starts with 3 tanks  $
    XXXXXX01   Game starts with 4 tanks
    XXXXXX00   Game starts with 5 tanks
    XXXX11XX   Missile appears after 5,000 points
    XXXX10XX   Missile appears after 10,000 points  $
    XXXX01XX   Missile appears after 20,000 points
    XXXX00XX   Missile appears after 30,000 points
    XX11XXXX   No bonus tank
    XX10XXXX   Bonus tanks at 15,000 and 100,000 points  $
    XX01XXXX   Bonus tanks at 25,000 and 100,000 points
    XX00XXXX   Bonus tanks at 50,000 and 100,000 points
    11XXXXXX   English language
    10XXXXXX   French language
    01XXXXXX   German language
    00XXXXXX   Spanish language

    4 SWITCH DIP

    XX11   All coin mechanisms register on one coin counter
    XX01   Left and center coin mechanisms on one coin counter, right on second
    XX10   Center and right coin mechanisms on one coin counter, left on second
    XX00   Each coin mechanism has it's own counter

****************************************************************************

    Red Baron memory map (preliminary)

    0000-03ff RAM (1K)
    0800      COIN_IN
    0a00      IN1
    0c00      IN2

    1000      Coin Counter
    1200      Vector generator start (write)
    1400      Watchdog Clear
    1600      Vector generator reset (write)

    1800      Mathbox Status register
    1802      Button inputs
    1804      Mathbox value (lo-byte)
    1806      Mathbox value (hi-byte)
    1808      Red Baron Sound (bit 1 selects joystick pot to read also)
    1810-181f POKEY I/O
    1818      Joystick inputs
    1860-187f Mathbox RAM

    2000-27ff Vector generator RAM (2K)
    2800-2fff Vector generator RAM / Vector ROM (2K)
    3000-37ff Vector generator ROM (4K)
    4800-7fff Program ROM (14K)

    RED BARON DIP SWITCH SETTINGS
    Donated by Dana Colbert


    $=Default
    "K" = 1,000

    Switch at position P10
                                      8    7    6    5    4    3    2    1
                                    _________________________________________
    English                        $|    |    |    |    |    |    |Off |Off |
    Spanish                         |    |    |    |    |    |    |Off | On |
    French                          |    |    |    |    |    |    | On |Off |
    German                          |    |    |    |    |    |    | On | On |
                                    |    |    |    |    |    |    |    |    |
     Bonus airplane granted at:     |    |    |    |    |    |    |    |    |
    Bonus at 2K, 10K and 30K        |    |    |    |    |Off |Off |    |    |
    Bonus at 4K, 15K and 40K       $|    |    |    |    |Off | On |    |    |
    Bonus at 6K, 20K and 50K        |    |    |    |    | On |Off |    |    |
    No bonus airplanes              |    |    |    |    | On | On |    |    |
                                    |    |    |    |    |    |    |    |    |
    2 aiplanes per game             |    |    |Off |Off |    |    |    |    |
    3 airplanes per game           $|    |    |Off | On |    |    |    |    |
    4 airplanes per game            |    |    | On |Off |    |    |    |    |
    5 airplanes per game            |    |    | On | On |    |    |    |    |
                                    |    |    |    |    |    |    |    |    |
    1-play minimum                 $|    |Off |    |    |    |    |    |    |
    2-play minimum                  |    | On |    |    |    |    |    |    |
                                    |    |    |    |    |    |    |    |    |
    Self-adj. game difficulty: on  $|Off |    |    |    |    |    |    |    |
    Self-adj. game difficulty: off  | On |    |    |    |    |    |    |    |
                                    -----------------------------------------

      If self-adjusting game difficulty feature is
    turned on, the program strives to maintain the
    following average game lengths (in seconds):

                                            Airplanes per game:
         Bonus airplane granted at:          2   3     4     5
    2,000, 10,000 and 30,000 points         90  105$  120   135
    4,000, 15,000 and 40,000 points         75   90   105   120
    6,000, 20,000 and 50,000 points         60   75    90   105
                 No bonus airplanes         45   60    75    90



    Switch at position M10
                                      8    7    6    5    4    3    2    1
                                    _________________________________________
        50  PER PLAY                |    |    |    |    |    |    |    |    |
     Straight 25  Door:             |    |    |    |    |    |    |    |    |
    No Bonus Coins                  |Off |Off |Off |Off |Off |Off | On | On |
    Bonus $1= 3 plays               |Off | On | On |Off |Off |Off | On | On |
    Bonus $1= 3 plays, 75 = 2 plays |Off |Off | On |Off |Off |Off | On | On |
                                    |    |    |    |    |    |    |    |    |
     25 /$1 Door or 25 /25 /$1 Door |    |    |    |    |    |    |    |    |
    No Bonus Coins                  |Off |Off |Off |Off |Off | On | On | On |
    Bonus $1= 3 plays               |Off | On | On |Off |Off | On | On | On |
    Bonus $1= 3 plays, 75 = 2 plays |Off |Off | On |Off |Off | On | On | On |
                                    |    |    |    |    |    |    |    |    |
        25  PER PLAY                |    |    |    |    |    |    |    |    |
     Straight 25  Door:             |    |    |    |    |    |    |    |    |
    No Bonus Coins                  |Off |Off |Off |Off |Off |Off | On |Off |
    Bonus 50 = 3 plays              |Off |Off | On |Off |Off |Off | On |Off |
    Bonus $1= 5 plays               |Off | On |Off |Off |Off |Off | On |Off |
                                    |    |    |    |    |    |    |    |    |
     25 /$1 Door or 25 /25 /$1 Door |    |    |    |    |    |    |    |    |
    No Bonus Coins                  |Off |Off |Off |Off |Off | On | On |Off |
    Bonus 50 = 3 plays              |Off |Off | On |Off |Off | On | On |Off |
    Bonus $1= 5 plays               |Off | On |Off |Off |Off | On | On |Off |
                                    -----------------------------------------

    Switch at position L11
                                                          1    2    3    4
                                                        _____________________
    All 3 mechs same denomination                       | On | On |    |    |
    Left and Center same, right different denomination  | On |Off |    |    |
    Right and Center same, left different denomination  |Off | On |    |    |
    All different denominations                         |Off |Off |    |    |
                                                        ---------------------


    2008-07
    Dip locations added from the notes above (factory settings for bzone
    from the manual).

***************************************************************************/

#include "emu.h"
#include "bzone.h"

#include "cpu/m6502/m6502.h"
#include "machine/watchdog.h"
#include "video/avgdvg.h"
#include "video/vector.h"
#include "sound/ay8910.h" // for Desert Wars
#include "sound/pokey.h"

#include "speaker.h"

#include "bzone.lh"
#include "redbaron.lh"


/*************************************
 *
 *  Save state registration
 *
 *************************************/

void bzone_state::machine_start()
{
	save_item(NAME(m_analog_data));
	m_startled.resolve();
}


void redbaron_state::machine_start()
{
	bzone_state::machine_start();
	save_item(NAME(m_rb_input_select));
}


void redbaron_state::machine_reset()
{
	earom_control_w(0);
}



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

INTERRUPT_GEN_MEMBER(bzone_state::bzone_interrupt)
{
	if (ioport("IN0")->read() & 0x10)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}



/*************************************
 *
 *  Battlezone input ports
 *
 *************************************/

int bzone_state::clock_r()
{
	return (m_maincpu->total_cycles() & 0x100) ? 1 : 0;
}


void bzone_state::bzone_coin_counter_w(offs_t offset, uint8_t data)
{
	machine().bookkeeping().coin_counter_w(offset,data);
}



/*************************************
 *
 *  Red Baron input ports
 *
 *************************************/

uint8_t redbaron_state::redbaron_joy_r()
{
	return m_fake_ports[m_rb_input_select ? 0 : 1]->read();
}

void redbaron_state::redbaron_joysound_w(uint8_t data)
{
	m_rb_input_select = data & 1;
	m_redbaronsound->sounds_w(data);
}



/*************************************
 *
 *  Red Baron EAROM
 *
 *************************************/

uint8_t redbaron_state::earom_read()
{
	return m_earom->data();
}

void redbaron_state::earom_write(offs_t offset, uint8_t data)
{
	m_earom->set_address((offset ^ 0x20) & 0x3f);
	m_earom->set_data(data);
}

void redbaron_state::earom_control_w(uint8_t data)
{
	// CK = EDB0, C1 = /EDB2, C2 = EDB1, CS1 = EDB3, /CS2 = GND
	m_earom->set_control(BIT(data, 3), 1, !BIT(data, 2), BIT(data, 1));
	m_earom->set_clk(BIT(data, 0));
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void bzone_state::bzone_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x03ff).ram();
	map(0x0800, 0x0800).portr("IN0");
	map(0x0a00, 0x0a00).portr("DSW0");
	map(0x0c00, 0x0c00).portr("DSW1");
	map(0x1000, 0x1000).w(FUNC(bzone_state::bzone_coin_counter_w));
	map(0x1200, 0x1200).w("avg", FUNC(avg_device::go_w));
	map(0x1400, 0x1400).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x1600, 0x1600).w("avg", FUNC(avg_device::reset_w));
	map(0x1800, 0x1800).r(m_mathbox, FUNC(mathbox_device::status_r));
	map(0x1810, 0x1810).r(m_mathbox, FUNC(mathbox_device::lo_r));
	map(0x1818, 0x1818).r(m_mathbox, FUNC(mathbox_device::hi_r));
	map(0x1820, 0x182f).rw("pokey", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x1840, 0x1840).w(FUNC(bzone_state::bzone_sounds_w));
	map(0x1860, 0x187f).w(m_mathbox, FUNC(mathbox_device::go_w));
	map(0x2000, 0x2fff).ram();
	map(0x3000, 0x7fff).rom();
}

void bzone_state::bradley_map(address_map &map)
{
	bzone_map(map);
	map(0x0400, 0x07ff).ram();
	map(0x1808, 0x1808).portr("1808");
	map(0x1809, 0x1809).portr("1809");
	map(0x180a, 0x180a).r(FUNC(bzone_state::analog_data_r));
	map(0x1848, 0x1850).w(FUNC(bzone_state::analog_select_w));
}

//TODO: AY8910 hook-up isn't correct. Are there discrete sounds, too?
void bzone_state::dsrtwars_map(address_map &map)
{
	map(0x0000, 0x03ff).ram();
	map(0x0800, 0x0800).portr("IN0");
	map(0x0a00, 0x0a00).portr("DSW0");
	map(0x0c00, 0x0c00).portr("DSW1");
	map(0x1000, 0x1000).w(FUNC(bzone_state::bzone_coin_counter_w));
	map(0x1200, 0x1200).w("avg", FUNC(avg_device::go_w));
	map(0x1400, 0x1400).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x1600, 0x1600).w("avg", FUNC(avg_device::reset_w));
	map(0x1800, 0x1800).r(m_mathbox, FUNC(mathbox_device::status_r));
	map(0x1810, 0x1810).r(m_mathbox, FUNC(mathbox_device::lo_r));
	map(0x1818, 0x1818).r(m_mathbox, FUNC(mathbox_device::hi_r));
	map(0x1820, 0x1821).w("aysnd", FUNC(ay8910_device::data_address_w)); // TODO: this is more complicated
	map(0x1828, 0x1828).portr("IN3");
	map(0x1860, 0x187f).w(m_mathbox, FUNC(mathbox_device::go_w));
	map(0x2000, 0x2fff).ram();
	map(0x3000, 0x7fff).rom();
	map(0xf800, 0xffff).rom().region("maincpu", 0x8000);
}

void redbaron_state::redbaron_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x03ff).ram();
	map(0x0800, 0x0800).portr("IN0");
	map(0x0a00, 0x0a00).portr("DSW0");
	map(0x0c00, 0x0c00).portr("DSW1");
	map(0x1000, 0x1000).nopw();        // coin out - Manual states this is "Coin Counter"
	map(0x1200, 0x1200).w("avg", FUNC(avg_device::go_w));
	map(0x1400, 0x1400).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x1600, 0x1600).w("avg", FUNC(avg_device::reset_w));
	map(0x1800, 0x1800).r("mathbox", FUNC(mathbox_device::status_r));
	map(0x1802, 0x1802).portr("IN4");
	map(0x1804, 0x1804).r("mathbox", FUNC(mathbox_device::lo_r));
	map(0x1806, 0x1806).r("mathbox", FUNC(mathbox_device::hi_r));
	map(0x1808, 0x1808).w(FUNC(redbaron_state::redbaron_joysound_w));  // and select joystick pot also
	map(0x180a, 0x180a).nopw();                // sound reset, yet to do
	map(0x180c, 0x180c).nopr().w(FUNC(redbaron_state::earom_control_w));
	map(0x1810, 0x181f).rw("pokey", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x1820, 0x185f).rw(FUNC(redbaron_state::earom_read), FUNC(redbaron_state::earom_write));
	map(0x1860, 0x187f).nopr().w("mathbox", FUNC(mathbox_device::go_w));
	map(0x2000, 0x2fff).ram();
	map(0x3000, 0x7fff).rom();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

#define BZONEIN0\
	PORT_START("IN0")\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )\
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )\
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )\
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Diagnostic Step") \
	/* bit 6 is the VG HALT bit. We set it to "low" */\
	/* per default (busy vector processor). */\
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("avg", FUNC(avg_device::done_r))\
	/* bit 7 is tied to a 3kHz clock */\
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(bzone_state::clock_r))


#define BZONEDSW0\
	PORT_START("DSW0")\
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) ) PORT_DIPLOCATION("M10:1,2")\
	PORT_DIPSETTING(    0x00, "2" )\
	PORT_DIPSETTING(    0x01, "3" )\
	PORT_DIPSETTING(    0x02, "4" )\
	PORT_DIPSETTING(    0x03, "5" )\
	PORT_DIPNAME( 0x0c, 0x04, "Missile appears at" ) PORT_DIPLOCATION("M10:3,4")\
	PORT_DIPSETTING(    0x00, "5000" )\
	PORT_DIPSETTING(    0x04, "10000" )\
	PORT_DIPSETTING(    0x08, "20000" )\
	PORT_DIPSETTING(    0x0c, "30000" )\
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("M10:5,6")\
	PORT_DIPSETTING(    0x10, "15k and 100k" )\
	PORT_DIPSETTING(    0x20, "25k and 100k" )\
	PORT_DIPSETTING(    0x30, "50k and 100k" )\
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )\
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("M10:7,8")\
	PORT_DIPSETTING(    0x00, DEF_STR( English ))\
	PORT_DIPSETTING(    0x40, DEF_STR( German ))\
	PORT_DIPSETTING(    0x80, DEF_STR( French ))\
	PORT_DIPSETTING(    0xc0, DEF_STR( Spanish ))

#define BZONEDSW1\
	PORT_START("DSW1")\
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("P10:1,2")\
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )\
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )\
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )\
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )\
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("P10:3,4")\
	PORT_DIPSETTING(    0x00, "*1" )\
	PORT_DIPSETTING(    0x04, "*4" )\
	PORT_DIPSETTING(    0x08, "*5" )\
	PORT_DIPSETTING(    0x0c, "*6" )\
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("P10:5")\
	PORT_DIPSETTING(    0x00, "*1" )\
	PORT_DIPSETTING(    0x10, "*2" )\
	PORT_DIPNAME( 0xe0, 0x00, "Bonus Coins" ) PORT_DIPLOCATION("P10:6,7,8")\
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )\
	PORT_DIPSETTING(    0x20, "3 credits/2 coins" )\
	PORT_DIPSETTING(    0x40, "5 credits/4 coins" )\
	PORT_DIPSETTING(    0x60, "6 credits/4 coins" )\
	PORT_DIPSETTING(    0x80, "6 credits/5 coins" )

#define BZONEADJ \
	PORT_START("R11") \
	PORT_ADJUSTER( 40, "R11 - Engine Frequency" )

static INPUT_PORTS_START( bzone )
	BZONEIN0
	BZONEDSW0
	BZONEDSW1

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	BZONEADJ
INPUT_PORTS_END


static INPUT_PORTS_START( redbaron )
	BZONEIN0

	PORT_START("DSW0")
	// See the table above if you are really interested
	PORT_DIPNAME( 0xff, 0xfd, DEF_STR( Coinage ) ) PORT_DIPLOCATION("M10:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(    0xfd, DEF_STR( Normal ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Language ) ) PORT_DIPLOCATION("P10:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( German ) )
	PORT_DIPSETTING(    0x01, DEF_STR( French ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Spanish ) )
	PORT_DIPSETTING(    0x03, DEF_STR( English ) )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("P10:3,4")
	PORT_DIPSETTING(    0x0c, "2k 10k 30k" )
	PORT_DIPSETTING(    0x08, "4k 15k 40k" )
	PORT_DIPSETTING(    0x04, "6k 20k 50k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Lives ) ) PORT_DIPLOCATION("P10:5,6")
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, "One Play Minimum" ) PORT_DIPLOCATION("P10:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Self Adjust Diff" ) PORT_DIPLOCATION("P10:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	/* IN3 - the real machine reads either the X or Y axis from this port.
	   Instead, we use the two fake 5 & 6 ports and bank-switch the proper
	   value based on the lsb of the byte written to the sound port */
	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY

	PORT_START("IN4")   // Misc controls
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	// These 2 are fake - they are bank-switched from reads to IN3
	// Red Baron doesn't seem to use the full 0-255 range.
	PORT_START("FAKE1") // IN5
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(64,192) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)

	PORT_START("FAKE2") // IN6
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(64,192) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)
INPUT_PORTS_END


static INPUT_PORTS_START( bradley )
	BZONEIN0
	BZONEDSW0
	BZONEDSW1

	PORT_START("IN3")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("1808")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Armor Piercing (Single Shot)") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("High Explosive (Single Shot)") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Armor Piercing (Low Rate)") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("High Explosive (Low Rate)") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Armor Piercing (High Rate)") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("High Explosive (High Rate)") PORT_CODE(KEYCODE_C)

	PORT_START("1809")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Select TOW Missiles") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("7.62 mm Machine Gun") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Magnification Toggle") PORT_CODE(KEYCODE_M) PORT_TOGGLE
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("AN0")   // analog 0 = turret rotation
	PORT_BIT( 0xff, 0x88, IPT_AD_STICK_X ) PORT_MINMAX(0x48,0xc8) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)

	PORT_START("AN1")   // analog 1 = turret elevation
	PORT_BIT( 0xff, 0x86, IPT_AD_STICK_Y ) PORT_MINMAX(0x46,0xc6) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)

	PORT_START("AN2")   // analog 2 = shell firing range hack removed, now uses Z
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Z ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE

	BZONEADJ
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void bzone_state::bzone_base(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, BZONE_MASTER_CLOCK / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &bzone_state::bzone_map);
	m_maincpu->set_periodic_int(FUNC(bzone_state::bzone_interrupt), attotime::from_hz(BZONE_CLOCK_3KHZ / 12));

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	VECTOR(config, "vector");
	SCREEN(config, m_screen, SCREEN_TYPE_VECTOR);
	m_screen->set_refresh_hz(BZONE_CLOCK_3KHZ / 12 / 6);
	m_screen->set_size(400, 300);
	m_screen->set_visarea(0, 580, 0, 400);
	m_screen->set_screen_update("vector", FUNC(vector_device::screen_update));

	avg_device &avg(AVG_BZONE(config, "avg", 0));
	avg.set_vector("vector");
	avg.set_memory(m_maincpu, AS_PROGRAM, 0x2000);

	// Drivers
	MATHBOX(config, m_mathbox, 0);
}

void bzone_state::bzone(machine_config &config)
{
	bzone_base(config);

	// sound hardware
	bzone_audio(config);
}

void bzone_state::bradley(machine_config &config)
{
	bzone(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &bzone_state::bradley_map);
}

void bzone_state::dsrtwars(machine_config &config)
{
	bzone_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &bzone_state::dsrtwars_map);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 1'000'000)); // unknown clock
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.85);
	aysnd.port_a_read_callback().set_ioport("IN3");
}

void redbaron_state::redbaron(machine_config &config)
{
	bzone_base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &redbaron_state::redbaron_map);

	ER2055(config, m_earom);

	// video hardware
	m_screen->set_refresh_hz(BZONE_CLOCK_3KHZ / 12 / 4);
	m_screen->set_visarea(0, 520, 0, 400);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	pokey_device &pokey(POKEY(config, "pokey", 1500000));
	pokey.allpot_r().set(FUNC(redbaron_state::redbaron_joy_r));
	pokey.add_route(ALL_OUTPUTS, "mono", 1.0);

	REDBARON(config, m_redbaronsound, 0);
	m_redbaronsound->add_route(ALL_OUTPUTS, "mono", 0.50);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

/* Battle Zone

For the Analog Vec Gen A035742 PCB:

The -01 revision uses PROMs, -02 uses ROMs

Rom Component Equivalents & Locations:

-01 P.C. Boards     -02 P.C. Boards
---------------------------------------
036415-01 (A3)
                    036421-01 (A3)
036418-01 (E3)


036416-01 (B/C3)
                    036422-01 (B/C3)
036419-01 (F/H3)

*/

ROM_START( bzone ) // Analog Vec Gen A035742-02
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "036414-02.e1",  0x5000, 0x0800, CRC(13de36d5) SHA1(40e356ddc5c042bc1ce0b71f51e8b6de72daf1e4) )
	ROM_LOAD( "036413-01.h1",  0x5800, 0x0800, CRC(5d9d9111) SHA1(42638cff53a9791a0f18d316f62a0ea8eea4e194) )
	ROM_LOAD( "036412-01.j1",  0x6000, 0x0800, CRC(ab55cbd2) SHA1(6bbb8316d9f8588ea0893932f9174788292b8edc) )
	ROM_LOAD( "036411-01.k1",  0x6800, 0x0800, CRC(ad281297) SHA1(54c5e06b2e69eb731a6c9b1704e4340f493e7ea5) )
	ROM_LOAD( "036410-01.lm1", 0x7000, 0x0800, CRC(0b7bfaa4) SHA1(33ae0f68b4e2eae9f3aecbee2d0b29003ce460b2) )
	ROM_LOAD( "036409-01.n1",  0x7800, 0x0800, CRC(1e14e919) SHA1(448fab30535e6fad7e0ab4427bc06bbbe075e797) )
	// Vector Generator ROMs
	ROM_LOAD( "036422-01.bc3", 0x3000, 0x0800, CRC(7414177b) SHA1(147d97a3b475e738ce00b1a7909bbd787ad06eda) )
	ROM_LOAD( "036421-01.a3",  0x3800, 0x0800, CRC(8ea8f939) SHA1(b71e0ab0e220c3e64dc2b094c701fb1a960b64e4) )  // 036421-01e.a3 same contents

	// AVG PROM
	ROM_REGION( 0x100, "avg:prom", 0 )
	ROM_LOAD( "036408-01.k7",   0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )

	// Mathbox PROMs
	ROM_REGION( 0x20, "user2", 0 )
	ROM_LOAD( "036174-01.b1",   0x0000, 0x0020, CRC(8b04f921) SHA1(317b3397482f13b2d1bc21f296d3b3f9a118787b) )

	ROM_REGION32_BE( 0x400, "user3", 0 )
	ROMX_LOAD( "036175-01.m1", 0, 0x100, CRC(2af82e87) SHA1(3816835a9ccf99a76d246adf204989d9261bb065), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "036176-01.l1", 0, 0x100, CRC(b31f6e24) SHA1(ce5f8ca34d06a5cfa0076b47400e61e0130ffe74), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "036177-01.k1", 1, 0x100, CRC(8119b847) SHA1(c4fbaedd4ce1ad6a4128cbe902b297743edb606a), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "036178-01.j1", 1, 0x100, CRC(09f5a4d5) SHA1(d6f2ac07ca9ee385c08831098b0dcaf56808993b), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "036179-01.h1", 2, 0x100, CRC(823b61ae) SHA1(d99a839874b45f64e14dae92a036e47a53705d16), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "036180-01.f1", 2, 0x100, CRC(276eadd5) SHA1(55718cd8ec4bcf75076d5ef0ee1ed2551e19d9ba), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
ROM_END


ROM_START( bzonea ) // Analog Vec Gen A035742-02
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "036414-01.e1",  0x5000, 0x0800, CRC(efbc3fa0) SHA1(6d284fab34b09dde8aa0df7088711d4723f07970) )
	ROM_LOAD( "036413-01.h1",  0x5800, 0x0800, CRC(5d9d9111) SHA1(42638cff53a9791a0f18d316f62a0ea8eea4e194) )
	ROM_LOAD( "036412-01.j1",  0x6000, 0x0800, CRC(ab55cbd2) SHA1(6bbb8316d9f8588ea0893932f9174788292b8edc) )
	ROM_LOAD( "036411-01.k1",  0x6800, 0x0800, CRC(ad281297) SHA1(54c5e06b2e69eb731a6c9b1704e4340f493e7ea5) )
	ROM_LOAD( "036410-01.lm1", 0x7000, 0x0800, CRC(0b7bfaa4) SHA1(33ae0f68b4e2eae9f3aecbee2d0b29003ce460b2) )
	ROM_LOAD( "036409-01.n1",  0x7800, 0x0800, CRC(1e14e919) SHA1(448fab30535e6fad7e0ab4427bc06bbbe075e797) )
	// Vector Generator ROMs
	ROM_LOAD( "036422-01.bc3", 0x3000, 0x0800, CRC(7414177b) SHA1(147d97a3b475e738ce00b1a7909bbd787ad06eda) )
	ROM_LOAD( "036421-01.a3",  0x3800, 0x0800, CRC(8ea8f939) SHA1(b71e0ab0e220c3e64dc2b094c701fb1a960b64e4) )  // 036421-01e.a3 same contents

	// AVG PROM
	ROM_REGION( 0x100, "avg:prom", 0 )
	ROM_LOAD( "036408-01.k7",   0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )

	// Mathbox PROMs
	ROM_REGION( 0x20, "user2", 0 )
	ROM_LOAD( "036174-01.b1",   0x0000, 0x0020, CRC(8b04f921) SHA1(317b3397482f13b2d1bc21f296d3b3f9a118787b) )

	ROM_REGION32_BE( 0x400, "user3", 0 )
	ROMX_LOAD( "036175-01.m1", 0, 0x100, CRC(2af82e87) SHA1(3816835a9ccf99a76d246adf204989d9261bb065), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "036176-01.l1", 0, 0x100, CRC(b31f6e24) SHA1(ce5f8ca34d06a5cfa0076b47400e61e0130ffe74), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "036177-01.k1", 1, 0x100, CRC(8119b847) SHA1(c4fbaedd4ce1ad6a4128cbe902b297743edb606a), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "036178-01.j1", 1, 0x100, CRC(09f5a4d5) SHA1(d6f2ac07ca9ee385c08831098b0dcaf56808993b), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "036179-01.h1", 2, 0x100, CRC(823b61ae) SHA1(d99a839874b45f64e14dae92a036e47a53705d16), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "036180-01.f1", 2, 0x100, CRC(276eadd5) SHA1(55718cd8ec4bcf75076d5ef0ee1ed2551e19d9ba), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
ROM_END


ROM_START( bzonec ) // cocktail version
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "bz1g4800",   0x4800, 0x0800, CRC(e228dd64) SHA1(247c788b4ccadf6c1e9201ad4f31d55c0036ff0f) )
	ROM_LOAD( "bz1f5000",   0x5000, 0x0800, CRC(dddfac9a) SHA1(e6f2761902e1ffafba437a1117e9ba40f116087d) )
	ROM_LOAD( "bz1e5800",   0x5800, 0x0800, CRC(7e00e823) SHA1(008e491a8074dac16e56c3aedec32d4b340158ce) )
	ROM_LOAD( "bz1d6000",   0x6000, 0x0800, CRC(c0f8c068) SHA1(66fff6b493371f0015c21b06b94637db12deced2) )
	ROM_LOAD( "bz1c6800",   0x6800, 0x0800, CRC(5adc64bd) SHA1(4574e4fe375d4ab3151a988235efa11e8744e2c6) )
	ROM_LOAD( "bz1b7000",   0x7000, 0x0800, CRC(ed8a860e) SHA1(316a3c4870ba44bb3e9cb9fc5200eb081318facf) )
	ROM_LOAD( "bz1a7800",   0x7800, 0x0800, CRC(04babf45) SHA1(a59da5ff49fc398ca4a948e28f05250af776b898) )
	// Vector Generator ROMs
	ROM_LOAD( "036422-01.bc3",  0x3000, 0x0800, CRC(7414177b) SHA1(147d97a3b475e738ce00b1a7909bbd787ad06eda) )  // bz3a3000
	ROM_LOAD( "bz3b3800",   0x3800, 0x0800, CRC(76cf57f6) SHA1(1b8f3fcd664ed04ce60d94fdf27e56b20d52bdbd) )

	// AVG PROM
	ROM_REGION( 0x100, "avg:prom", 0 )
	ROM_LOAD( "036408-01.k7",   0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )

	// Mathbox PROMs
	ROM_REGION( 0x20, "user2", 0 )
	ROM_LOAD( "036174-01.b1",   0x0000, 0x0020, CRC(8b04f921) SHA1(317b3397482f13b2d1bc21f296d3b3f9a118787b) )

	ROM_REGION32_BE( 0x400, "user3", 0 )
	ROMX_LOAD( "036175-01.m1", 0, 0x100, CRC(2af82e87) SHA1(3816835a9ccf99a76d246adf204989d9261bb065), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "036176-01.l1", 0, 0x100, CRC(b31f6e24) SHA1(ce5f8ca34d06a5cfa0076b47400e61e0130ffe74), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "036177-01.k1", 1, 0x100, CRC(8119b847) SHA1(c4fbaedd4ce1ad6a4128cbe902b297743edb606a), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "036178-01.j1", 1, 0x100, CRC(09f5a4d5) SHA1(d6f2ac07ca9ee385c08831098b0dcaf56808993b), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "036179-01.h1", 2, 0x100, CRC(823b61ae) SHA1(d99a839874b45f64e14dae92a036e47a53705d16), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "036180-01.f1", 2, 0x100, CRC(276eadd5) SHA1(55718cd8ec4bcf75076d5ef0ee1ed2551e19d9ba), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
ROM_END


ROM_START( bradley )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "btc1.bin",   0x4000, 0x0800, CRC(0bb8e049) SHA1(158517ff9a4e8ae7270ccf7eab87bf77427a4a8c) )
	ROM_LOAD( "btd1.bin",   0x4800, 0x0800, CRC(9e0566d4) SHA1(f14aa5c3d14136c5e9a317004f82d44a8d5d6815) )
	ROM_LOAD( "bte1.bin",   0x5000, 0x0800, CRC(64ee6a42) SHA1(33d0713ed2a1f4c1c443dce1f053321f2c279293) )
	ROM_LOAD( "bth1.bin",   0x5800, 0x0800, CRC(baab67be) SHA1(77ad1935bf252b401bb6bbb57bd2ed66a85f0a6d) )
	ROM_LOAD( "btj1.bin",   0x6000, 0x0800, CRC(036adde4) SHA1(16a9fcf98a2aa287e0b7a665b88c9c67377a1203) )
	ROM_LOAD( "btk1.bin",   0x6800, 0x0800, CRC(f5c2904e) SHA1(f2cbf720c4f5ce0fc912dbc2f0445cb2c51ffac1) )
	ROM_LOAD( "btlm.bin",   0x7000, 0x0800, CRC(7d0313bf) SHA1(17e3d8df62b332cf889133f1943c8f27308df027) )
	ROM_LOAD( "btn1.bin",   0x7800, 0x0800, CRC(182c8c64) SHA1(511af60d86551291d2dc28442970b4863c62624a) )
	// Vector Generator ROMs
	ROM_LOAD( "btb3.bin",   0x3000, 0x0800, CRC(88206304) SHA1(6a2e2ff35a929acf460f244db7968f3978b1d239) )
	ROM_LOAD( "bta3.bin",   0x3800, 0x0800, CRC(d669d796) SHA1(ad606882320cd13612c7962d4718680fe5a35dd3) )

	// AVG PROM
	ROM_REGION( 0x100, "avg:prom", 0 )
	ROM_LOAD( "036408-01.k7",   0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )

	// Mathbox PROMs
	ROM_REGION( 0x20, "user2", 0 )
	ROM_LOAD( "036174-01.b1",   0x0000, 0x0020, CRC(8b04f921) SHA1(317b3397482f13b2d1bc21f296d3b3f9a118787b) )

	ROM_REGION32_BE( 0x400, "user3", 0 )
	ROMX_LOAD( "036175-01.m1", 0, 0x100, CRC(2af82e87) SHA1(3816835a9ccf99a76d246adf204989d9261bb065), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "036176-01.l1", 0, 0x100, CRC(b31f6e24) SHA1(ce5f8ca34d06a5cfa0076b47400e61e0130ffe74), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "036177-01.k1", 1, 0x100, CRC(8119b847) SHA1(c4fbaedd4ce1ad6a4128cbe902b297743edb606a), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "036178-01.j1", 1, 0x100, CRC(09f5a4d5) SHA1(d6f2ac07ca9ee385c08831098b0dcaf56808993b), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "036179-01.h1", 2, 0x100, CRC(823b61ae) SHA1(d99a839874b45f64e14dae92a036e47a53705d16), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "036180-01.f1", 2, 0x100, CRC(276eadd5) SHA1(55718cd8ec4bcf75076d5ef0ee1ed2551e19d9ba), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
ROM_END

/* Desert Wars uses a separate sound board with an AY-3-8910 and one EPROM.
   The PCB is labeled as "ANDRA S.A. 300900-A".
   ____________________________________________________________
  |      __________                           __________      |
  |     |SN74LS175N                          |_74LS00PC|      |
  |                    _____________________                  |
  |      __________   | ·················· |         ___      |
  |     |SN74LS175N   | ····CONNECTOR····· |        |  |      |
  |                   |____________________|        | <-74LS138PC
  |                                                 |__|      |
  |  _____________________                                 __ |
  | | AY-3-8910          |        __________  __________  | | |
  | |                    |       |SN74LS240N||_74LS00PC|  |_| |
  | |____________________|                                    |
  |                                                           |
  |   __________  __________                                  |
  |  |_74LS74AN| |_CD4040BE|      _____________               |
  |                              |EPROM 2716  |               |
  |   __________                 |            |               |
  |  |_74LS00PC|                 |____________|               |
  |                                                           |
  |___________________________________________________________|

   Also, it uses a blue overlay instead of the usual green.
*/
ROM_START( dsrtwars ) // Desert Wars
	ROM_REGION( 0x8800, "maincpu", 0 )
	ROM_LOAD( "300600_b.e1",       0x5000, 0x0800, CRC(13de36d5) SHA1(40e356ddc5c042bc1ce0b71f51e8b6de72daf1e4) )
	ROM_LOAD( "300600_b.h1",       0x5800, 0x0800, CRC(5d9d9111) SHA1(42638cff53a9791a0f18d316f62a0ea8eea4e194) )
	ROM_LOAD( "300600_b_dxc.j1",   0x6000, 0x0800, CRC(8d45383f) SHA1(704c2a5fefc109fd79d28b870bdf83f82b89ffdc) )
	ROM_LOAD( "300600_b_kx.k1",    0x6800, 0x0800, CRC(6e481957) SHA1(920d28641058a6c79e1b16d9bab66ffddd87d903) )
	ROM_LOAD( "300600_b.m1",       0x7000, 0x0800, CRC(ace7fbc8) SHA1(52e923aface37fcaf959b2506d186c8e3bf45bb9) )
	ROM_LOAD( "300600_b_n10.n1",   0x7800, 0x1000, CRC(bb2f8479) SHA1(996253b5ce4d300eaf2c0960bb48b9a7d375256c) ) // 2532
	// Vector Generator ROMs
	ROM_LOAD( "300600_b_gen_b.b3", 0x3000, 0x0800, CRC(9a25b1d4) SHA1(092fbc0d86f71a08881f8ff33793e8077236a57f) )
	ROM_LOAD( "300600_b_gen_a.a3", 0x3800, 0x0800, CRC(909550da) SHA1(a7a1c48c470e53088f0b6e8393d5fee38e8e8892) )

	// AVG PROM
	ROM_REGION( 0x100, "avg:prom", 0 )
	ROM_LOAD( "300600_b_vr_74s287.k7", 0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )

	// Mathbox PROMs
	ROM_REGION( 0x020, "user2", 0 )
	ROM_LOAD( "300800_b_am27s19.b1", 0x000, 0x020, CRC(8b04f921) SHA1(317b3397482f13b2d1bc21f296d3b3f9a118787b) )

	ROM_REGION32_BE( 0x400, "user3", 0 )
	ROMX_LOAD( "300800_b_74s287.m1", 0, 0x100, CRC(2af82e87) SHA1(3816835a9ccf99a76d246adf204989d9261bb065), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "300800_b_74s287.l1", 0, 0x100, CRC(b31f6e24) SHA1(ce5f8ca34d06a5cfa0076b47400e61e0130ffe74), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "300800_b_74s287.k1", 1, 0x100, CRC(8119b847) SHA1(c4fbaedd4ce1ad6a4128cbe902b297743edb606a), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "300800_b_74s287.j1", 1, 0x100, CRC(09f5a4d5) SHA1(d6f2ac07ca9ee385c08831098b0dcaf56808993b), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "300800_b_74s287.h1", 2, 0x100, CRC(823b61ae) SHA1(d99a839874b45f64e14dae92a036e47a53705d16), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "300800_b_74s287.f1", 2, 0x100, CRC(276eadd5) SHA1(55718cd8ec4bcf75076d5ef0ee1ed2551e19d9ba), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))

	ROM_REGION( 0x800, "music", 0 )
	ROM_LOAD( "300900_b_am27s19.b1", 0x000, 0x800, CRC(1b6e9f6f) SHA1(1ced9958021b29400e4681fbcb76228fd1b74ab9) ) // TODO: where's this mapped?
ROM_END


/* Red Baron

For the Analog Vec Gen A035742 PCB:

The -01 revision uses PROMs, -02 uses ROMs

Rom Component Equivalents & Locations:

-01 P.C. Boards     -02 P.C. Boards
---------------------------------------
037005-01 (A3)
                    037007-01 (A3)
037003-01 (E3)


037004-01 (B/C3)
                    037006-01 (B/C3)
037002-01 (F/H3)

Program rom locations as same as redbarona listed below
*/


ROM_START( redbaron ) // Analog Vec Gen A035742-02 Rev. C+
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "037587-01.fh1", 0x4800, 0x0800, CRC(60f23983) SHA1(7a9e5380bf49bf50a2d8ab0e0bd1ba3ac8efde24) ) // == 037001-1E + 036999-1E
	ROM_CONTINUE(              0x5800, 0x0800 )
	ROM_LOAD( "037000-01.e1",  0x5000, 0x0800, CRC(69bed808) SHA1(27d99efc74113cdcbbf021734b8a5a5fdb78c04c) )
	ROM_LOAD( "036998-01.j1",  0x6000, 0x0800, CRC(d1104dd7) SHA1(0eab47cb45ede9dcc4dd7498dcf3a8d8194460b4) )
	ROM_LOAD( "036997-01.k1",  0x6800, 0x0800, CRC(7434acb4) SHA1(c950c4c12ab556b5051ad356ab4a0ed6b779ba1f) )
	ROM_LOAD( "036996-01.lm1", 0x7000, 0x0800, CRC(c0e7589e) SHA1(c1aedc95966afffd860d7e0009d5a43e8b292036) )
	ROM_LOAD( "036995-01.n1",  0x7800, 0x0800, CRC(ad81d1da) SHA1(8bd66e5f34fc1c75f31eb6b168607e52aa3aa4df) )
	// Vector Generator ROMs
	ROM_LOAD( "037006-01.bc3", 0x3000, 0x0800, CRC(9fcffea0) SHA1(69b76655ee75742fcaa0f39a4a1cf3aa58088343) )
	ROM_LOAD( "037007-01.a3",  0x3800, 0x0800, CRC(60250ede) SHA1(9c48952bd69863bee0c6dce09f3613149e0151ef) )

	// AVG PROM
	ROM_REGION( 0x100, "avg:prom", 0 )
	ROM_LOAD( "036408-01.k7", 0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) ) // 74S287N or compatible bprom like the 82S129

	// Mathbox PROMs
	ROM_REGION( 0x20, "user2", 0 )
	ROM_LOAD( "036174-01.a1", 0x0000, 0x0020, CRC(8b04f921) SHA1(317b3397482f13b2d1bc21f296d3b3f9a118787b) ) // 74S288 or compatible bprom like the 82S123

	ROM_REGION32_BE( 0x400, "user3", 0 )
	ROMX_LOAD( "036175-01.e1", 0, 0x100, CRC(2af82e87) SHA1(3816835a9ccf99a76d246adf204989d9261bb065), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "036176-01.f1", 0, 0x100, CRC(b31f6e24) SHA1(ce5f8ca34d06a5cfa0076b47400e61e0130ffe74), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "036177-01.h1", 1, 0x100, CRC(8119b847) SHA1(c4fbaedd4ce1ad6a4128cbe902b297743edb606a), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "036178-01.j1", 1, 0x100, CRC(09f5a4d5) SHA1(d6f2ac07ca9ee385c08831098b0dcaf56808993b), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "036179-01.k1", 2, 0x100, CRC(823b61ae) SHA1(d99a839874b45f64e14dae92a036e47a53705d16), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "036180-01.l1", 2, 0x100, CRC(276eadd5) SHA1(55718cd8ec4bcf75076d5ef0ee1ed2551e19d9ba), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))

	// Address decoding PROM (located on the AUX PCB) - currently not used
	ROM_REGION( 0x20, "prom", 0 )
	ROM_LOAD( "036464-01.a5", 0x0000, 0x0020, CRC(42875b18) SHA1(10ba29f3c8c8e581eb275a85574c746272ebb865) ) // 74S288 or compatible bprom like the 82S123
ROM_END

ROM_START( redbarona ) // Analog Vec Gen A035742-02
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "037001-01e.e1",  0x4800, 0x0800, CRC(b9486a6a) SHA1(76cf42569c4ef0a2ad7171e3c766c1a815a62a0e) )
	ROM_LOAD( "037000-01e.fh1", 0x5000, 0x0800, CRC(69bed808) SHA1(27d99efc74113cdcbbf021734b8a5a5fdb78c04c) )
	ROM_LOAD( "036999-01e.j1",  0x5800, 0x0800, CRC(48d49819) SHA1(caf1521ae7bbf3afa91069dae62201748b482a50) )
	ROM_LOAD( "036998-01e.k1",  0x6000, 0x0800, CRC(d1104dd7) SHA1(0eab47cb45ede9dcc4dd7498dcf3a8d8194460b4) )
	ROM_LOAD( "036997-01e.lm1", 0x6800, 0x0800, CRC(7434acb4) SHA1(c950c4c12ab556b5051ad356ab4a0ed6b779ba1f) )
	ROM_LOAD( "036996-01e.n1",  0x7000, 0x0800, CRC(c0e7589e) SHA1(c1aedc95966afffd860d7e0009d5a43e8b292036) )
	ROM_LOAD( "036995-01e.p1",  0x7800, 0x0800, CRC(ad81d1da) SHA1(8bd66e5f34fc1c75f31eb6b168607e52aa3aa4df) )
	// Vector Generator ROMs
	ROM_LOAD( "037006-01e.bc3", 0x3000, 0x0800, CRC(9fcffea0) SHA1(69b76655ee75742fcaa0f39a4a1cf3aa58088343) )
	ROM_LOAD( "037007-01e.a3",  0x3800, 0x0800, CRC(60250ede) SHA1(9c48952bd69863bee0c6dce09f3613149e0151ef) )

	// AVG PROM
	ROM_REGION( 0x100, "avg:prom", 0 )
	ROM_LOAD( "036408-01.k7", 0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) ) // 74S287N or compatible bprom like the 82S129

	// Mathbox PROMs
	ROM_REGION( 0x20, "user2", 0 )
	ROM_LOAD( "036174-01.a1", 0x0000, 0x0020, CRC(8b04f921) SHA1(317b3397482f13b2d1bc21f296d3b3f9a118787b) ) // 74S288 or compatible bprom like the 82S123

	ROM_REGION32_BE( 0x400, "user3", 0 )
	ROMX_LOAD( "036175-01.e1", 0, 0x100, CRC(2af82e87) SHA1(3816835a9ccf99a76d246adf204989d9261bb065), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "036176-01.f1", 0, 0x100, CRC(b31f6e24) SHA1(ce5f8ca34d06a5cfa0076b47400e61e0130ffe74), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "036177-01.h1", 1, 0x100, CRC(8119b847) SHA1(c4fbaedd4ce1ad6a4128cbe902b297743edb606a), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "036178-01.j1", 1, 0x100, CRC(09f5a4d5) SHA1(d6f2ac07ca9ee385c08831098b0dcaf56808993b), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))
	ROMX_LOAD( "036179-01.k1", 2, 0x100, CRC(823b61ae) SHA1(d99a839874b45f64e14dae92a036e47a53705d16), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(3))
	ROMX_LOAD( "036180-01.l1", 2, 0x100, CRC(276eadd5) SHA1(55718cd8ec4bcf75076d5ef0ee1ed2551e19d9ba), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(3))

	// Address decoding PROM (located on the AUX PCB) - currently not used
	ROM_REGION( 0x20, "prom", 0 )
	ROM_LOAD( "036464-01.a5", 0x0000, 0x0020, CRC(42875b18) SHA1(10ba29f3c8c8e581eb275a85574c746272ebb865) ) // 74S288 or compatible bprom like the 82S123
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

uint8_t bzone_state::analog_data_r()
{
	return m_analog_data;
}


void bzone_state::analog_select_w(offs_t offset, uint8_t data)
{
	static const char *const analog_port[] = { "AN0", "AN1", "AN2" };

	if (offset <= 2)
		m_analog_data = ioport(analog_port[offset])->read();
}


/*************************************
 *
 *  Game drivers
 *
 *************************************/
//    YEAR  NAME       PARENT    MACHINE   INPUT     CLASS           INIT        ROT   COMPANY  FULLNAME                        FLAGS                                        LAYOUT
GAMEL(1980, bzone,     0,        bzone,    bzone,    bzone_state,    empty_init, ROT0, "Atari", "Battle Zone (rev 2)",          MACHINE_SUPPORTS_SAVE,                       layout_bzone )
GAMEL(1980, bzonea,    bzone,    bzone,    bzone,    bzone_state,    empty_init, ROT0, "Atari", "Battle Zone (rev 1)",          MACHINE_SUPPORTS_SAVE,                       layout_bzone )
GAMEL(1980, bzonec,    bzone,    bzone,    bzone,    bzone_state,    empty_init, ROT0, "Atari", "Battle Zone (cocktail)",       MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL, layout_bzone )
GAME( 1980, bradley,   0,        bradley,  bradley,  bzone_state,    empty_init, ROT0, "Atari", "Bradley Trainer",              MACHINE_SUPPORTS_SAVE )
GAMEL(1980, redbaron,  0,        redbaron, redbaron, redbaron_state, empty_init, ROT0, "Atari", "Red Baron (revised hardware)", MACHINE_SUPPORTS_SAVE,                       layout_redbaron )
GAMEL(1980, redbarona, redbaron, redbaron, redbaron, redbaron_state, empty_init, ROT0, "Atari", "Red Baron",                    MACHINE_SUPPORTS_SAVE,                       layout_redbaron )

// Not from Atari
GAMEL(1981, dsrtwars,  bzone,    dsrtwars, bzone,    bzone_state,    empty_init, ROT0, "bootleg (Andra S.A.)", "Desert Wars (bootleg of Battle Zone)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_bzone )
