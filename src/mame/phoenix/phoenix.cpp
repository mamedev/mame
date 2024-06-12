// license:BSD-3-Clause
// copyright-holders:Richard Davies
/***************************************************************************

Phoenix hardware games

driver by Richard Davies

There are 2 Phoenix board families:
* 2 PCBs with 2 50-pin connectors. The 'Main board' hosts the 8085 CPU,
  the MN6221 sound chip, 2 PROMs, the volume trimmer, 1 bank of 8 dip switches
  and 3 amplifiers (1x LM380 + 2x LM324), the 'ROMs board' hosts the ROMs,
  the RAM and the oscillator. This board type is used by the Amstar original
  and some of the clones, some of which introduce a small PCB for credits'
  management (with 1 bank of 4 dip switch), while others substitute the MM8221
  with a piggyback PCB. These clones were manufactured between 1980 and 1981.
* Only 1 PCB, and 1 color PROM instead of 2. In most of these bootlegs the LM380
  isn't present and only the 2 LM324 remain. There are 2 banks of 4 switches
  and 1 bank of 8, though not all bootlegs have all the banks fitted, probably
  as a cost cutting measure. For the same reason, not all have the MN6221. These
  bootlegs were all built in 1981 and are an evolution of the first family,
  manufactured more cheaply thanks to some reengineering of the circuits.

Notes:
- Discrete sound emulation is in audio/phoenix.cpp,
  pleiads is using another sound driver, audio/pleiads.cpp


To Do:

Phoenix:
- Emulate the different sound system used at least by phoenixc2, griffono,
   and nextfase.
- Some of the bootlegs use MN6221AB instead of MN6221AA.
- Better documentation of the bootlegs.

Survival:

- Check background visible area.  When the background scrolls up, it
  currently shows below the top and bottom of the border of the play area.


Pleiads:

- Palette banking.  Controlled by 3 custom chips marked T-X, T-Y and T-Z.
  These chips are responsible for the protection as well.

***************************************************************************/

#include "emu.h"
#include "phoenix.h"
#include "phoenix_a.h"

#include "cpu/z80/z80.h"
#include "cpu/i8085/i8085.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "speaker.h"


void phoenix_state::phoenix_memory_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x4fff).bankr("bank1").w(FUNC(phoenix_state::phoenix_videoram_w)); // 2 pages selected by bit 0 of the video register
	map(0x5000, 0x53ff).w(FUNC(phoenix_state::phoenix_videoreg_w));
	map(0x5800, 0x5bff).w(FUNC(phoenix_state::phoenix_scroll_w));
	map(0x6000, 0x63ff).w("cust", FUNC(phoenix_sound_device::control_a_w));
	map(0x6800, 0x6bff).w("cust", FUNC(phoenix_sound_device::control_b_w));
	map(0x7000, 0x73ff).portr("IN0");                            // IN0 or IN1
	map(0x7800, 0x7bff).portr("DSW0");                           // DSW
}

void phoenix_state::pleiads_memory_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x4fff).bankr("bank1").w(FUNC(phoenix_state::phoenix_videoram_w)); // 2 pages selected by bit 0 of the video register
	map(0x5000, 0x53ff).w(FUNC(phoenix_state::pleiads_videoreg_w));
	map(0x5800, 0x5bff).w(FUNC(phoenix_state::phoenix_scroll_w));
	map(0x6000, 0x63ff).w(m_pleiads_custom, FUNC(pleiads_sound_device::control_a_w));
	map(0x6800, 0x6bff).w(m_pleiads_custom, FUNC(pleiads_sound_device::control_b_w));
	map(0x7000, 0x73ff).portr("IN0");                            // IN0 or IN1 + protection
	map(0x7800, 0x7bff).portr("DSW0");                           // DSW
}

void phoenix_state::survival_memory_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x4fff).bankr("bank1").w(FUNC(phoenix_state::phoenix_videoram_w)); // 2 pages selected by bit 0 of the video register
	map(0x5000, 0x53ff).w(FUNC(phoenix_state::phoenix_videoreg_w));
	map(0x5800, 0x5bff).w(FUNC(phoenix_state::phoenix_scroll_w));
	map(0x6800, 0x68ff).w("aysnd", FUNC(ay8910_device::address_w));
	map(0x6900, 0x69ff).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x7000, 0x73ff).r(FUNC(phoenix_state::survival_input_port_0_r));               // IN0 or IN1
	map(0x7800, 0x7bff).portr("DSW0");                           // DSW
}



static INPUT_PORTS_START( phoenix )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(phoenix_state, player_input_r)

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )            PORT_DIPLOCATION( "SW1:1,2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION( "SW1:3,4" )
	PORT_DIPSETTING(    0x00, "3K 30K" )
	PORT_DIPSETTING(    0x04, "4K 40K" )
	PORT_DIPSETTING(    0x08, "5K 50K" )
	PORT_DIPSETTING(    0x0c, "6K 60K" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Coinage ) )          PORT_DIPLOCATION( "SW1:5" )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION( "SW1:6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION( "SW1:7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("CAB")       // fake port for non-memory mapped dip switch
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION( "SW1:!8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )

	PORT_START("CTRL")      // fake port for multiplexed controls
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL                  PORT_CONDITION("CAB",0x01,EQUALS,0x01)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL PORT_CONDITION("CAB",0x01,EQUALS,0x01)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_2WAY PORT_COCKTAIL PORT_CONDITION("CAB",0x01,EQUALS,0x01)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL                  PORT_CONDITION("CAB",0x01,EQUALS,0x01)
INPUT_PORTS_END

static INPUT_PORTS_START( phoenixa )
	PORT_INCLUDE( phoenix )

	PORT_MODIFY("DSW0")
	// Coinage is backwards from phoenix (Amstar)
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Coinage ) )          PORT_DIPLOCATION( "SW1:5" )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( phoenixt )
	PORT_INCLUDE( phoenix )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION( "SW1:5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( phoenix3 )
	PORT_INCLUDE( phoenix )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION( "SW1:5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION( "SW1:6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Coinage ) )          PORT_DIPLOCATION( "SW1:7" )
	PORT_DIPSETTING(    0x40, "A - 2C/1C B - 1C/3C" )
	PORT_DIPSETTING(    0x00, "A - 1C/1C B - 1C/6C" )
INPUT_PORTS_END

static INPUT_PORTS_START( condor )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(phoenix_state, player_input_r)

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )            PORT_DIPLOCATION( "SW1:1,2" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION( "SW1:3,4" )
	PORT_DIPSETTING(    0x00, "Every 6000" )
	PORT_DIPSETTING(    0x08, "Every 10000" )
	PORT_DIPSETTING(    0x04, "Every 14000" )
	PORT_DIPSETTING(    0x0c, "Every 18000" )
	PORT_DIPNAME( 0x70, 0x30, "Fuel Consumption" )          PORT_DIPLOCATION( "SW1:5,6,7" )
	PORT_DIPSETTING(    0x00, "Slowest" )
	PORT_DIPSETTING(    0x10, "Slower" )
	PORT_DIPSETTING(    0x20, "Slow" )
	PORT_DIPSETTING(    0x30, "Bit Slow" )
	PORT_DIPSETTING(    0x40, "Bit Fast" )
	PORT_DIPSETTING(    0x50, "Fast" )
	PORT_DIPSETTING(    0x60, "Faster" )
	PORT_DIPSETTING(    0x70, "Fastest" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_B ) )           PORT_DIPLOCATION( "SW2:1,2,3,4" )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_6C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_8C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_A ) )           PORT_DIPLOCATION( "SW2:5,6,7,8" )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 2C_6C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 2C_8C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("CAB")       // fake port for non-memory mapped dip switch
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION( "SW1:!8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )

	PORT_START("CTRL")      // fake port for multiplexed controls
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL                  PORT_CONDITION("CAB",0x01,EQUALS,0x01)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL PORT_CONDITION("CAB",0x01,EQUALS,0x01)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_2WAY PORT_COCKTAIL PORT_CONDITION("CAB",0x01,EQUALS,0x01)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL                  PORT_CONDITION("CAB",0x01,EQUALS,0x01)
INPUT_PORTS_END

static INPUT_PORTS_START( falconz )
	PORT_INCLUDE( phoenix )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )            PORT_DIPLOCATION( "SW1:1,2" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION( "SW1:3,4" )
	PORT_DIPSETTING(    0x00, "3K 30K" )
	PORT_DIPSETTING(    0x04, "4K 40K" )
	PORT_DIPSETTING(    0x08, "5K 50K" )
	PORT_DIPSETTING(    0x0c, "6K 60K" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION( "SW1:5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION( "SW1:6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coinage ) )          PORT_DIPLOCATION( "SW1:7" )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( nextfase )
	PORT_INCLUDE( phoenix )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )            PORT_DIPLOCATION( "SW1:1,2" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION( "SW1:3,4" )
	PORT_DIPSETTING(    0x00, "3K 30K" )
	PORT_DIPSETTING(    0x04, "4K 40K" )
	PORT_DIPSETTING(    0x08, "5K 50K" )
	PORT_DIPSETTING(    0x0c, "6K 60K" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION( "SW1:5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Coinage ) )          PORT_DIPLOCATION( "SW1:6,7" )
	PORT_DIPSETTING(    0x00, "A - 1C/1C B - 1C/2C" )
	PORT_DIPSETTING(    0x20, "A - 2C/3C B - 1C/3C" )
	PORT_DIPSETTING(    0x40, "A - 1C/2C B - 1C/4C" )
	PORT_DIPSETTING(    0x60, "A - 2C/5C B - 1C/5C" )
INPUT_PORTS_END


static INPUT_PORTS_START( pleiads )
	PORT_INCLUDE( phoenix )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(phoenix_state, pleiads_protection_r)     // Protection. See 0x0552

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION( "SW1:7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )

	// Based on various sources, no Button 2 was present in Pleiads (Tehkan version)
	PORT_MODIFY("CTRL")     // fake port for multiplexed controls
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL                  PORT_CONDITION("CAB",0x01,EQUALS,0x01)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL PORT_CONDITION("CAB",0x01,EQUALS,0x01)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_2WAY PORT_COCKTAIL PORT_CONDITION("CAB",0x01,EQUALS,0x01)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )                                 PORT_CONDITION("CAB",0x01,EQUALS,0x01)
INPUT_PORTS_END

static INPUT_PORTS_START( pleiadbl )
	PORT_INCLUDE( phoenix )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(phoenix_state, pleiads_protection_r)     // Protection. See 0x0552

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION( "SW1:7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pleiadce )
	PORT_INCLUDE( pleiadbl )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION( "SW1:3,4" )
	PORT_DIPSETTING(    0x00, "7K 70K" )
	PORT_DIPSETTING(    0x04, "8K 80K" )
	PORT_DIPSETTING(    0x08, "9K 90K" )
	//PORT_DIPSETTING(    0x0c, "INVALID" )                   Sets bonus to A000
INPUT_PORTS_END

static INPUT_PORTS_START( cityatta )
	PORT_INCLUDE( pleiadbl )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )     PORT_DIPLOCATION( "SW1:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) )  PORT_DIPLOCATION( "SW1:4" )
	PORT_DIPSETTING(    0x00, "3K" )
	PORT_DIPSETTING(    0x08, "5K" )
INPUT_PORTS_END

static INPUT_PORTS_START( capitol )
	PORT_INCLUDE( pleiads )

	// Capitol has no Button 2 as Pleiads, but there is no protection
	PORT_MODIFY("IN0")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( survival )
	PORT_START("IN0")      // IN0
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP  )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT    )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )

	PORT_START("IN1")       // IN1
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_CUSTOM )    // comes from IN0 0-2
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN     ) PORT_COCKTAIL

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )            PORT_DIPLOCATION( "SW1:1,2" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION( "SW1:3,4" )
	PORT_DIPSETTING(    0x0c, "25000" )
	PORT_DIPSETTING(    0x08, "35000" )
	PORT_DIPSETTING(    0x04, "45000" )
	PORT_DIPSETTING(    0x00, "55000" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Free_Play ) )        PORT_DIPLOCATION( "SW1:5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Coinage ) )          PORT_DIPLOCATION( "SW1:6,7" )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_1C ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("CAB")       // fake port for non-memory mapped dip switch
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION( "SW1:8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	256,    // 256 characters
	2,  // 2 bits per pixel
	{ 256*8*8, 0 }, // the two bitplanes are separated
	{ 7, 6, 5, 4, 3, 2, 1, 0 }, // pretty straightforward layout
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 // every char takes 8 consecutive bytes
};

static GFXDECODE_START( gfx_phoenix )
	GFXDECODE_ENTRY( "bgtiles", 0, charlayout, 0, 32 )
	GFXDECODE_ENTRY( "fgtiles", 0, charlayout, 0, 32 )
GFXDECODE_END

static GFXDECODE_START( gfx_pleiads )
	GFXDECODE_ENTRY( "bgtiles", 0, charlayout, 0, 64 )
	GFXDECODE_ENTRY( "fgtiles", 0, charlayout, 0, 64 )
GFXDECODE_END


void phoenix_state::phoenix(machine_config &config)
{
	// basic machine hardware
	I8085A(config, m_maincpu, CPU_CLOCK);  // 2.75 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &phoenix_state::phoenix_memory_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	screen.set_screen_update(FUNC(phoenix_state::screen_update_phoenix));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_phoenix);
	PALETTE(config, m_palette, FUNC(phoenix_state::phoenix_palette), 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	tms36xx_device &tms(TMS36XX(config, "tms", 372));
	tms.set_subtype(tms36xx_device::subtype::MM6221AA);
	tms.set_decays(0.50, 0, 0, 1.05, 0, 0);
	tms.set_tune_speed(0.21);
	tms.add_route(ALL_OUTPUTS, "mono", 0.5);

	PHOENIX_SOUND(config, "cust").add_route(ALL_OUTPUTS, "mono", 0.4);

	DISCRETE(config, "discrete", 120000, phoenix_discrete).add_route(ALL_OUTPUTS, "mono", 0.6);
}

void phoenix_state::pleiads(machine_config &config)
{
	phoenix(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &phoenix_state::pleiads_memory_map);

	// video hardware
	m_gfxdecode->set_info(gfx_pleiads);

	m_palette->set_init(FUNC(phoenix_state::pleiads_palette));

	// sound hardware
	tms36xx_device &tms(TMS36XX(config.replace(), "tms", 247));
	tms.set_subtype(tms36xx_device::subtype::TMS3615);
	tms.set_decays(0.33, 0.33, 0, 0.33, 0, 0.33);
	// NOTE: it's unknown if the TMS3615 mixes more than one voice internally.
	// A wav taken from Pop Flamer sounds like there are at least no 'odd'
	// harmonics (5 1/3' and 2 2/3')
	tms.add_route(ALL_OUTPUTS, "mono", 0.75);

	config.device_remove("cust");
	PLEIADS_SOUND(config, "pleiads_custom").add_route(ALL_OUTPUTS, "mono", 0.40);

	config.device_remove("discrete");
}


// Same as Phoenix, but uses an AY8910 and an extra visible line (column)

void phoenix_state::survival(machine_config &config)
{
	// basic machine hardware
	i8085a_cpu_device &maincpu(I8085A(config, m_maincpu, CPU_CLOCK));  // 5.50 MHz
	maincpu.in_sid_func().set(FUNC(phoenix_state::survival_sid_callback));
	maincpu.set_addrmap(AS_PROGRAM, &phoenix_state::survival_memory_map);

	// video hardware

	/* schematics fairly identical to phoenix, however the interesting
	 * page is missing
	 */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	screen.set_screen_update(FUNC(phoenix_state::screen_update_phoenix));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_phoenix);
	PALETTE(config, m_palette, FUNC(phoenix_state::survival_palette), 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	// FIXME: check clock
	ay8910_device &aysnd(AY8910(config, "aysnd", XTAL(11'000'000)/4));
	aysnd.port_b_read_callback().set(FUNC(phoenix_state::survival_protection_r));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
}


// Uses a Z80
void phoenix_state::condor(machine_config &config)
{
	phoenix(config);

	// basic machine hardware
	// FIXME: Verify clock. This is most likely 11MHz/2
	Z80(config.replace(), m_maincpu, 11000000/4);    // 2.75 MHz???
	m_maincpu->set_addrmap(AS_PROGRAM, &phoenix_state::phoenix_memory_map);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( phoenix )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic45",         0x0000, 0x0800, CRC(9f68086b) SHA1(fc3cef299bf03bf0586c4047c6b96ca666846220) )
	ROM_LOAD( "ic46",         0x0800, 0x0800, CRC(273a4a82) SHA1(6f3019a074e73ff50ceb92f655fcf15659f34919) )
	ROM_LOAD( "ic47",         0x1000, 0x0800, CRC(3d4284b9) SHA1(6e69f8f0d537fe89140cd95d2398531d7e93d102) )
	ROM_LOAD( "ic48",         0x1800, 0x0800, CRC(cb5d9915) SHA1(49bcf55a5721cfcc02c3b811a4b601e35ea576db) )
	ROM_LOAD( "h5-ic49.5a",   0x2000, 0x0800, CRC(a105e4e7) SHA1(b35142a91b6b7fdf7535202671793393c9f4685f) )
	ROM_LOAD( "h6-ic50.6a",   0x2800, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) )
	ROM_LOAD( "h7-ic51.7a",   0x3000, 0x0800, CRC(2eab35b4) SHA1(849bf8273317cc869bdd67e50c68399ee8ece81d) )
	ROM_LOAD( "h8-ic52.8a",   0x3800, 0x0800, CRC(aff8e9c5) SHA1(e4164f85ec12d4d9bcbffba27ab1f51b3599f6d0) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23.3d",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "ic24.4d",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "b1-ic39.3b",   0x0000, 0x0800, CRC(53413e8f) SHA1(d772358505b973b10da840d204afb210c0c746ec) )
	ROM_LOAD( "b2-ic40.4b",   0x0800, 0x0800, CRC(0be2ba91) SHA1(af9243ee23377b632b9b7d0b84d341d06bf22480) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

ROM_START( phoenix2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic45-pg1.1a",  0x0000, 0x0800, CRC(5b8c55a8) SHA1(839c1ca9766f730ec3accd48db70f6429a9c3362) )
	ROM_LOAD( "ic46-pg2.2a",  0x0800, 0x0800, CRC(273a4a82) SHA1(6f3019a074e73ff50ceb92f655fcf15659f34919) )
	ROM_LOAD( "ic47-pg3.3a",  0x1000, 0x0800, CRC(cbbb8839) SHA1(b7f449374cac111081559e39646f973e7e99fd64) )
	ROM_LOAD( "ic48-pg4.4a",  0x1800, 0x0800, CRC(cb5d9915) SHA1(49bcf55a5721cfcc02c3b811a4b601e35ea576db) )
	ROM_LOAD( "ic49-pg5.5a",  0x2000, 0x0800, CRC(73bcd2e1) SHA1(773acb7317531572cb2b81c7375e0fe52127fa0a) )
	ROM_LOAD( "ic50-pg6.6a",  0x2800, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) )
	ROM_LOAD( "ic51-pg7.7a",  0x3000, 0x0800, CRC(2eab35b4) SHA1(849bf8273317cc869bdd67e50c68399ee8ece81d) )
	ROM_LOAD( "ic52-pg8.8a",  0x3800, 0x0800, CRC(aff8e9c5) SHA1(e4164f85ec12d4d9bcbffba27ab1f51b3599f6d0) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23-cg1.3d",  0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "ic24-cg2.4d",  0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "ic39-cg3.3b",  0x0000, 0x0800, CRC(53413e8f) SHA1(d772358505b973b10da840d204afb210c0c746ec) )
	ROM_LOAD( "ic40-cg4.4b",  0x0800, 0x0800, CRC(0be2ba91) SHA1(af9243ee23377b632b9b7d0b84d341d06bf22480) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

ROM_START( phoenixa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1-ic45.1a",    0x0000, 0x0800, CRC(c7a9b499) SHA1(cda61de47956b3603ff6e48556ce528b5f45deab) )
	ROM_LOAD( "2-ic46.2a",    0x0800, 0x0800, CRC(d0e6ae1b) SHA1(63c6df8365dcb8befa338e8479482e34a4259abf) )
	ROM_LOAD( "3-ic47.3a",    0x1000, 0x0800, CRC(64bf463a) SHA1(6cd876e80b85fbac6374ea1f26620c026ba9e99a) )
	ROM_LOAD( "4-ic48.4a",    0x1800, 0x0800, CRC(1b20fe62) SHA1(87d2da6b9bde9049825245ca4b994fc84543e8b9) )
	ROM_LOAD( "h5-ic49.5a",   0x2000, 0x0800, CRC(a105e4e7) SHA1(b35142a91b6b7fdf7535202671793393c9f4685f) )
	ROM_LOAD( "h6-ic50.6a",   0x2800, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) )
	ROM_LOAD( "h7-ic51.7a",   0x3000, 0x0800, CRC(2eab35b4) SHA1(849bf8273317cc869bdd67e50c68399ee8ece81d) )
	ROM_LOAD( "h8-ic52.8a",   0x3800, 0x0800, CRC(aff8e9c5) SHA1(e4164f85ec12d4d9bcbffba27ab1f51b3599f6d0) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23.3d",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "ic24.4d",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "b1-ic39.3b",   0x0000, 0x0800, CRC(53413e8f) SHA1(d772358505b973b10da840d204afb210c0c746ec) )
	ROM_LOAD( "b2-ic40.4b",   0x0800, 0x0800, CRC(0be2ba91) SHA1(af9243ee23377b632b9b7d0b84d341d06bf22480) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

ROM_START( phoenixb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1-ic45.1a",    0x0000, 0x0800, CRC(c7a9b499) SHA1(cda61de47956b3603ff6e48556ce528b5f45deab) )
	ROM_LOAD( "2-ic46.2a",    0x0800, 0x0800, CRC(d0e6ae1b) SHA1(63c6df8365dcb8befa338e8479482e34a4259abf) )
	ROM_LOAD( "3-ic47.3a",    0x1000, 0x0800, CRC(64bf463a) SHA1(6cd876e80b85fbac6374ea1f26620c026ba9e99a) )
	ROM_LOAD( "4-ic48.4a",    0x1800, 0x0800, CRC(1b20fe62) SHA1(87d2da6b9bde9049825245ca4b994fc84543e8b9) )
	ROM_LOAD( "phoenixc.49",  0x2000, 0x0800, CRC(1a1ce0d0) SHA1(c2825eef5d461e16ca2172daff94b3751be2f4dc) )
	ROM_LOAD( "h6-ic50.6a",   0x2800, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) )
	ROM_LOAD( "h7-ic51.7a",   0x3000, 0x0800, CRC(2eab35b4) SHA1(849bf8273317cc869bdd67e50c68399ee8ece81d) )
	ROM_LOAD( "h8-ic52.8a",   0x3800, 0x0800, CRC(aff8e9c5) SHA1(e4164f85ec12d4d9bcbffba27ab1f51b3599f6d0) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23.3d",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "ic24.4d",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "phoenixc.39",  0x0000, 0x0800, CRC(bb0525ed) SHA1(86db1c7584fb3846bfd47535e1585eeb7fbbb1fe) )
	ROM_LOAD( "phoenixc.40",  0x0800, 0x0800, CRC(4178aa4f) SHA1(5350f8f62cc7c223c38008bc83140b7a19147d81) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

ROM_START( phoenixdal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dal.a1",         0x0000, 0x0800, CRC(5b8c55a8) SHA1(839c1ca9766f730ec3accd48db70f6429a9c3362) )
	ROM_LOAD( "dal.a2",         0x0800, 0x0800, CRC(dbc942fa) SHA1(9fe224e6ced407289dfa571468259a021d942b7d) )
	ROM_LOAD( "dal.a3",         0x1000, 0x0800, CRC(cbbb8839) SHA1(b7f449374cac111081559e39646f973e7e99fd64) )
	ROM_LOAD( "dal.a4",         0x1800, 0x0800, CRC(228b76ad) SHA1(e1677b3f69ef842e27b2d74fd4e6f3520c4b6593) )
	ROM_LOAD( "d2716,dal.a5",         0x2000, 0x0800, CRC(1a1ce0d0) SHA1(c2825eef5d461e16ca2172daff94b3751be2f4dc) )
	ROM_LOAD( "h6-ic50.6a",   0x2800, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) ) // dal.a6
	ROM_LOAD( "h7-ic51.7a",   0x3000, 0x0800, CRC(2eab35b4) SHA1(849bf8273317cc869bdd67e50c68399ee8ece81d) ) // dal.a7
	ROM_LOAD( "dal.a8",   0x3800, 0x0800, CRC(0a0f92c0) SHA1(d28ce84ef86f852e1f10f5ea1e370dfd1751f477) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23.3d",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) ) // dal.d3
	ROM_LOAD( "ic24.4d",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) ) // dal.d4

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "dal.b3",   0x0000, 0x0800, CRC(bb0525ed) SHA1(86db1c7584fb3846bfd47535e1585eeb7fbbb1fe) )
	ROM_LOAD( "dal.b4",   0x0800, 0x0800, CRC(4178aa4f) SHA1(5350f8f62cc7c223c38008bc83140b7a19147d81) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

ROM_START( phoenixt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "phoenix.45",   0x0000, 0x0800, CRC(5b8c55a8) SHA1(839c1ca9766f730ec3accd48db70f6429a9c3362) )
	ROM_LOAD( "phoenix.46",   0x0800, 0x0800, CRC(dbc942fa) SHA1(9fe224e6ced407289dfa571468259a021d942b7d) )
	ROM_LOAD( "phoenix.47",   0x1000, 0x0800, CRC(cbbb8839) SHA1(b7f449374cac111081559e39646f973e7e99fd64) )
	ROM_LOAD( "phoenix.48",   0x1800, 0x0800, CRC(cb65eff8) SHA1(63e674d680972d3744d66b943e8546f3b77ee6d4) )
	ROM_LOAD( "phoenix.49",   0x2000, 0x0800, CRC(c8a5d6d6) SHA1(ef6ade323544e3edd4101609138ecf35e8cb9577) )
	ROM_LOAD( "h6-ic50.6a",   0x2800, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) )
	ROM_LOAD( "h7-ic51.7a",   0x3000, 0x0800, CRC(2eab35b4) SHA1(849bf8273317cc869bdd67e50c68399ee8ece81d) )
	ROM_LOAD( "phoenix.52",   0x3800, 0x0800, CRC(b9915263) SHA1(f61396077b23364b5b26f62c6923394d23a37eb3) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23.3d",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "ic24.4d",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "b1-ic39.3b",   0x0000, 0x0800, CRC(53413e8f) SHA1(d772358505b973b10da840d204afb210c0c746ec) )
	ROM_LOAD( "b2-ic40.4b",   0x0800, 0x0800, CRC(0be2ba91) SHA1(af9243ee23377b632b9b7d0b84d341d06bf22480) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

ROM_START( phoenixj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pn01.45",   0x0000, 0x0800, CRC(5b8c55a8) SHA1(839c1ca9766f730ec3accd48db70f6429a9c3362) )
	ROM_LOAD( "pn02.46",   0x0800, 0x0800, CRC(dbc942fa) SHA1(9fe224e6ced407289dfa571468259a021d942b7d) )
	ROM_LOAD( "pn03.47",   0x1000, 0x0800, CRC(cbbb8839) SHA1(b7f449374cac111081559e39646f973e7e99fd64) )
	ROM_LOAD( "pn04.48",   0x1800, 0x0800, CRC(dd41f22b) SHA1(cb3748e18a99e35d99b5f18cee2a9796dd5646f9) )
	ROM_LOAD( "pn05.49",   0x2000, 0x0800, CRC(1a1ce0d0) SHA1(c2825eef5d461e16ca2172daff94b3751be2f4dc) )
	ROM_LOAD( "pn06.50",   0x2800, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) )
	ROM_LOAD( "pn07.51",   0x3000, 0x0800, CRC(2eab35b4) SHA1(849bf8273317cc869bdd67e50c68399ee8ece81d) )
	ROM_LOAD( "pn08.52",   0x3800, 0x0800, CRC(b9915263) SHA1(f61396077b23364b5b26f62c6923394d23a37eb3) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "pn11.23",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "pn12.24",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "pn09.39",   0x0000, 0x0800, CRC(53413e8f) SHA1(d772358505b973b10da840d204afb210c0c746ec) )
	ROM_LOAD( "pn10.40",   0x0800, 0x0800, CRC(0be2ba91) SHA1(af9243ee23377b632b9b7d0b84d341d06bf22480) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "pn14.40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "pn13.41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

ROM_START( phoenix3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "phoenix3.45",  0x0000, 0x0800, CRC(a362cda0) SHA1(5ab38afaf92179c965533326574c773f6a63dbbb) )
	ROM_LOAD( "phoenix3.46",  0x0800, 0x0800, CRC(5748f486) SHA1(49e6fd836d26ec24105e95227b24cf668e8a470a) )
	ROM_LOAD( "phoenix.47",   0x1000, 0x0800, CRC(cbbb8839) SHA1(b7f449374cac111081559e39646f973e7e99fd64) )
	ROM_LOAD( "phoenix3.48",  0x1800, 0x0800, CRC(b5d97a4d) SHA1(d5d92c5e34431b2ded47e58608c459cc8cdd7937) )
	ROM_LOAD( "h5-ic49.5a",   0x2000, 0x0800, CRC(a105e4e7) SHA1(b35142a91b6b7fdf7535202671793393c9f4685f) )
	ROM_LOAD( "h6-ic50.6a",   0x2800, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) )
	ROM_LOAD( "h7-ic51.7a",   0x3000, 0x0800, CRC(2eab35b4) SHA1(849bf8273317cc869bdd67e50c68399ee8ece81d) )
	ROM_LOAD( "phoenix3.52",  0x3800, 0x0800, CRC(d2c5c984) SHA1(a9432f9aff8a2f5ca1d347443efc008a177d8ae0) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23.3d",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "ic24.4d",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "b1-ic39.3b",   0x0000, 0x0800, CRC(53413e8f) SHA1(d772358505b973b10da840d204afb210c0c746ec) )
	ROM_LOAD( "b2-ic40.4b",   0x0800, 0x0800, CRC(0be2ba91) SHA1(af9243ee23377b632b9b7d0b84d341d06bf22480) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

ROM_START( phoenixc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "phoenix.45",   0x0000, 0x0800, CRC(5b8c55a8) SHA1(839c1ca9766f730ec3accd48db70f6429a9c3362) )
	ROM_LOAD( "phoenix.46",   0x0800, 0x0800, CRC(dbc942fa) SHA1(9fe224e6ced407289dfa571468259a021d942b7d) )
	ROM_LOAD( "phoenix.47",   0x1000, 0x0800, CRC(cbbb8839) SHA1(b7f449374cac111081559e39646f973e7e99fd64) )
	ROM_LOAD( "phoenixc.48",  0x1800, 0x0800, CRC(5ae0b215) SHA1(f6dd86806fb9c467aaa63edf0cb4dbed9645e7c0) )
	ROM_LOAD( "phoenixc.49",  0x2000, 0x0800, CRC(1a1ce0d0) SHA1(c2825eef5d461e16ca2172daff94b3751be2f4dc) )
	ROM_LOAD( "h6-ic50.6a",   0x2800, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) )
	ROM_LOAD( "h7-ic51.7a",   0x3000, 0x0800, CRC(2eab35b4) SHA1(849bf8273317cc869bdd67e50c68399ee8ece81d) )
	ROM_LOAD( "phoenixc.52",  0x3800, 0x0800, CRC(8424d7c4) SHA1(1b5fa7d8be9e8750a4148dfefc17e96c86ed084d) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23.3d",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "ic24.4d",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "phoenixc.39",  0x0000, 0x0800, CRC(bb0525ed) SHA1(86db1c7584fb3846bfd47535e1585eeb7fbbb1fe) )
	ROM_LOAD( "phoenixc.40",  0x0800, 0x0800, CRC(4178aa4f) SHA1(5350f8f62cc7c223c38008bc83140b7a19147d81) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

/*
main PCB is marked: "PH2"
main PCB is labeled: "03814367" and "280402" and "N 132073"
ROMs PCB is marked: "PH1"
On top of main PCB there is a small piggyback PCB. it is a replacement for MN6221 Melody-Alarm Generator - sound. There are 3 PROMs (dumped) and a few 74xx logics.
This board was in a cocktail table cabinet manufactured by Model Racing and labeled "Thunderbird".
*/

ROM_START( phoenixc2 ) // verified main and ROMs PCBs and 2 PROMs
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "phoenix.45",   0x0000, 0x0800, CRC(5b8c55a8) SHA1(839c1ca9766f730ec3accd48db70f6429a9c3362) ) // 01.ic45
	ROM_LOAD( "phoenix.46",   0x0800, 0x0800, CRC(dbc942fa) SHA1(9fe224e6ced407289dfa571468259a021d942b7d) ) // 01.ic46
	ROM_LOAD( "phoenix.47",   0x1000, 0x0800, CRC(cbbb8839) SHA1(b7f449374cac111081559e39646f973e7e99fd64) ) // 01.ic47
	ROM_LOAD( "01.ic48",      0x1800, 0x0800, CRC(f28e16d8) SHA1(65a7592f9589bcd094ffc9b2b44ca3257bcf5e5c) )
	ROM_LOAD( "phoenixc.49",  0x2000, 0x0800, CRC(1a1ce0d0) SHA1(c2825eef5d461e16ca2172daff94b3751be2f4dc) ) // 01.1c49
	ROM_LOAD( "h6-ic50.6a",   0x2800, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) ) // 01.ic50
	ROM_LOAD( "h7-ic51.7a",   0x3000, 0x0800, CRC(2eab35b4) SHA1(849bf8273317cc869bdd67e50c68399ee8ece81d) ) // 01.ic51
	ROM_LOAD( "phoenixc.52",  0x3800, 0x0800, CRC(8424d7c4) SHA1(1b5fa7d8be9e8750a4148dfefc17e96c86ed084d) ) // 01.ic52

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23.3d",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) ) // 01.ic23
	ROM_LOAD( "ic24.4d",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) ) // 01.ic24

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "phoenixc.39",  0x0000, 0x0800, CRC(bb0525ed) SHA1(86db1c7584fb3846bfd47535e1585eeb7fbbb1fe) ) // 01.ic39
	ROM_LOAD( "phoenixc.40",  0x0800, 0x0800, CRC(4178aa4f) SHA1(5350f8f62cc7c223c38008bc83140b7a19147d81) ) // 01.ic40

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits - 01.ic40
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits - 01.ic41

	ROM_REGION( 0x0300, "plds", 0 )
	ROM_LOAD( "tbp24sa10n.183.bin",   0x0000, 0x0100, CRC(47f5e887) SHA1(786d5a599b919ce6f06fe5c568ab6fa6c87b1002) )
	ROM_LOAD( "tbp24sa10n.184.bin",   0x0100, 0x0100, CRC(931f3292) SHA1(94b0d65e909389a2c0c0aac2e16a55e60b14f3bc) )
	ROM_LOAD( "tbp24sa10n.185.bin",   0x0200, 0x0100, CRC(0a06bd1b) SHA1(8c5debc5502e88af8019266fedcbe4bad1e2e214) )
ROM_END

ROM_START( phoenixc3 ) // verified main and ROMs PCBs and 2 PROMs
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "phoenix.45",   0x0000, 0x0800, CRC(5b8c55a8) SHA1(839c1ca9766f730ec3accd48db70f6429a9c3362) )
	ROM_LOAD( "phoenix.46",   0x0800, 0x0800, CRC(dbc942fa) SHA1(9fe224e6ced407289dfa571468259a021d942b7d) )
	ROM_LOAD( "phoenix.47",   0x1000, 0x0800, CRC(cbbb8839) SHA1(b7f449374cac111081559e39646f973e7e99fd64) )
	ROM_LOAD( "4.a4",         0x1800, 0x0800, CRC(61514bed) SHA1(142d44fc0602e1de7dc93d7759d4c3ee99890f97) )
	ROM_LOAD( "phoenixc.49",  0x2000, 0x0800, CRC(1a1ce0d0) SHA1(c2825eef5d461e16ca2172daff94b3751be2f4dc) )
	ROM_LOAD( "h6-ic50.6a",   0x2800, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) )
	ROM_LOAD( "h7-ic51.7a",   0x3000, 0x0800, CRC(2eab35b4) SHA1(849bf8273317cc869bdd67e50c68399ee8ece81d) )
	ROM_LOAD( "phoenixc.52",  0x3800, 0x0800, CRC(8424d7c4) SHA1(1b5fa7d8be9e8750a4148dfefc17e96c86ed084d) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23.3d",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "ic24.4d",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "phoenixc.39",  0x0000, 0x0800, CRC(bb0525ed) SHA1(86db1c7584fb3846bfd47535e1585eeb7fbbb1fe) )
	ROM_LOAD( "phoenixc.40",  0x0800, 0x0800, CRC(4178aa4f) SHA1(5350f8f62cc7c223c38008bc83140b7a19147d81) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

/*
main PCB is marked: "PX 031081"
There is an additional daughter board to handle coins credits; it's clearly an addition made by an operator.
*/

ROM_START( phoenixc4 ) // verified main and ROMs PCBs and 2 PROMs
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "phoenix.45",   0x0000, 0x0800, CRC(5b8c55a8) SHA1(839c1ca9766f730ec3accd48db70f6429a9c3362) )
	ROM_LOAD( "phoenix.46",   0x0800, 0x0800, CRC(dbc942fa) SHA1(9fe224e6ced407289dfa571468259a021d942b7d) )
	ROM_LOAD( "phoenix.47",   0x1000, 0x0800, CRC(cbbb8839) SHA1(b7f449374cac111081559e39646f973e7e99fd64) )
	ROM_LOAD( "phoenixd.48",  0x1800, 0x0800, CRC(6e51f009) SHA1(f91ed67543a675be9337f2a73c179f3fbea2c33e) )
	ROM_LOAD( "phoenixc.49",  0x2000, 0x0800, CRC(1a1ce0d0) SHA1(c2825eef5d461e16ca2172daff94b3751be2f4dc) )
	ROM_LOAD( "cond06c.bin",  0x2800, 0x0800, CRC(8c83bff7) SHA1(3dfb090d7e3a9ae8da882b06e166c48555eaf77c) )
	ROM_LOAD( "vautor07.1m",  0x3000, 0x0800, CRC(079ac364) SHA1(55b17c069b191cd1752e78068ef683b33c094e56) )
	ROM_LOAD( "phoenixc.52",  0x3800, 0x0800, CRC(8424d7c4) SHA1(1b5fa7d8be9e8750a4148dfefc17e96c86ed084d) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23.3d",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "ic24.4d",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "phoenixd.3b",  0x0000, 0x0800, BAD_DUMP CRC(31c06c22) SHA1(41e13e2e14f098ef0c6985dbcd2d85f83eb3b44f) )
	ROM_LOAD( "phoenixc.40",  0x0800, 0x0800, CRC(4178aa4f) SHA1(5350f8f62cc7c223c38008bc83140b7a19147d81) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic41", 0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )
	ROM_RELOAD(               0x0000, 0x0100 )  // the dump had 2 identical proms with different names
ROM_END

ROM_START( phoenixgu ) // verified 2 PCBs, 2 PROMs
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "phoenix.45",   0x0000, 0x0800, CRC(5b8c55a8) SHA1(839c1ca9766f730ec3accd48db70f6429a9c3362) )
	ROM_LOAD( "phoenix.46",   0x0800, 0x0800, CRC(dbc942fa) SHA1(9fe224e6ced407289dfa571468259a021d942b7d) )
	ROM_LOAD( "phoenix.47",   0x1000, 0x0800, CRC(cbbb8839) SHA1(b7f449374cac111081559e39646f973e7e99fd64) )
	ROM_LOAD( "48.bin",       0x1800, 0x0800, CRC(75623a06) SHA1(f12cf24287cd8e78218b0fb2862e9d6f84797fbc) )
	ROM_LOAD( "phoenixc.49",  0x2000, 0x0800, CRC(1a1ce0d0) SHA1(c2825eef5d461e16ca2172daff94b3751be2f4dc) )
	ROM_LOAD( "h6-ic50.6a",   0x2800, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) )
	ROM_LOAD( "h7-ic51.7a",   0x3000, 0x0800, CRC(2eab35b4) SHA1(849bf8273317cc869bdd67e50c68399ee8ece81d) )
	ROM_LOAD( "phoenixc.52",  0x3800, 0x0800, CRC(8424d7c4) SHA1(1b5fa7d8be9e8750a4148dfefc17e96c86ed084d) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23.3d",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "ic24.4d",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "phoenixc.39",  0x0000, 0x0800, CRC(bb0525ed) SHA1(86db1c7584fb3846bfd47535e1585eeb7fbbb1fe) )
	ROM_LOAD( "phoenixc.40",  0x0800, 0x0800, CRC(4178aa4f) SHA1(5350f8f62cc7c223c38008bc83140b7a19147d81) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

ROM_START( phoenixha ) // verified 2 PROMs, number of boards unknown (probably 2)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic45",  0x0000, 0x0800, CRC(5b8c55a8) SHA1(839c1ca9766f730ec3accd48db70f6429a9c3362) ) // sldh
	ROM_LOAD( "ic46",  0x0800, 0x0800, CRC(dbc942fa) SHA1(9fe224e6ced407289dfa571468259a021d942b7d) ) // sldh
	ROM_LOAD( "ic47",  0x1000, 0x0800, CRC(cbbb8839) SHA1(b7f449374cac111081559e39646f973e7e99fd64) ) // sldh
	ROM_LOAD( "ic48",  0x1800, 0x0800, CRC(b2672265) SHA1(dfc635f2089b521c61b3b493e7c3aefcef65c8d0) ) // sldh
	ROM_LOAD( "ic49",  0x2000, 0x0800, CRC(1a1ce0d0) SHA1(c2825eef5d461e16ca2172daff94b3751be2f4dc) )
	ROM_LOAD( "ic50",  0x2800, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) )
	ROM_LOAD( "ic51",  0x3000, 0x0800, CRC(2eab35b4) SHA1(849bf8273317cc869bdd67e50c68399ee8ece81d) )
	ROM_LOAD( "ic52",  0x3800, 0x0800, CRC(f3f10ac5) SHA1(aebf35ea59197fff511096dbba7320a4e79216a7) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "ic24",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "ic39",  0x0000, 0x0800, CRC(bb0525ed) SHA1(86db1c7584fb3846bfd47535e1585eeb7fbbb1fe) )
	ROM_LOAD( "ic40",  0x0800, 0x0800, CRC(4178aa4f) SHA1(5350f8f62cc7c223c38008bc83140b7a19147d81) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "ic40_b",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "ic41_a",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

ROM_START( phoenixi ) // verified single PCB, single PROM
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0201.bin",  0x0000, 0x0800, CRC(c0f73929) SHA1(3cecf8341a5674165d2cae9b22ea5db26a9597de) )
	ROM_LOAD( "0202.bin",  0x0800, 0x0800, CRC(440d56e8) SHA1(b3147d5416cec8c00c7df40b878b826434121737) )
	ROM_LOAD( "0203.bin",  0x1000, 0x0800, CRC(750b059b) SHA1(6fbaa2ef4c7eef6f731a73b2d33a02fff21b318a) )
	ROM_LOAD( "0204.bin",  0x1800, 0x0800, CRC(e2d3271f) SHA1(4bf01aa5104bdc84066f8267766d397a39af7b3e) )
	ROM_LOAD( "0205.bin",  0x2000, 0x0800, CRC(1ff3a982) SHA1(66fb39e7abdf7a9c6e2eb01d41cfe9429781d6aa) )
	ROM_LOAD( "0206.bin",  0x2800, 0x0800, CRC(8c83bff7) SHA1(3dfb090d7e3a9ae8da882b06e166c48555eaf77c) )
	ROM_LOAD( "0207.bin",  0x3000, 0x0800, CRC(805ec2e8) SHA1(7e56fc9990eb99512078e2b1e2874fb33b0aa05c) )
	ROM_LOAD( "cond08c.bin",  0x3800, 0x0800, BAD_DUMP CRC(1edebb45) SHA1(2fdf061ee600e27a6ed512ea61a8d78307a7fb8a) ) // 0208.bin wasn't readable, but very probably matches the one from condor

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "0209.bin",  0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "0210.bin",  0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "0211.bin",  0x0000, 0x0800, CRC(53c52eb0) SHA1(19624ca359996b77d3c65ef78a7af90eeb092377) )
	ROM_LOAD( "0212.bin",  0x0800, 0x0800, CRC(eba42f0f) SHA1(378282cb2c4e10c23179ae3c605ae7bf691150f6) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "sn74s471n.bin",   0x0100, 0x0100, CRC(c68a49bc) SHA1(1a015b89ac0622e73bcebd76cf5132830fe0bfc1) )
ROM_END

ROM_START( fenixn ) // Fenix, bootleg from the Spanish company Niemer
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2201.bin", 0x0000, 0x0800, CRC(2afa8e2c) SHA1(e9cd02b730912365aae4172da881bddb7c7b16aa) )
	ROM_LOAD( "1202.bin", 0x0800, 0x0800, CRC(6527395e) SHA1(13799da4b5c4118b3fafe7b6145b0ebca89b9f00) )
	ROM_LOAD( "1203.bin", 0x1000, 0x0800, CRC(868e69ba) SHA1(bef3173f1c978a9b9d1597439d272186b466e069) )
	ROM_LOAD( "1204.bin", 0x1800, 0x0800, CRC(85d96d7c) SHA1(19bb1d0a6764b6982d3cb29e2681dee8648d5fda) )
	ROM_LOAD( "1205.bin", 0x2000, 0x0800, CRC(a105e4e7) SHA1(b35142a91b6b7fdf7535202671793393c9f4685f) )
	ROM_LOAD( "1206.bin", 0x2800, 0x0800, CRC(249a6f21) SHA1(1c83d9883120796cf3bf775ab6620c4850f52d3c) )
	ROM_LOAD( "1207.bin", 0x3000, 0x0800, CRC(2eab35b4) SHA1(849bf8273317cc869bdd67e50c68399ee8ece81d) )
	ROM_LOAD( "1208.bin", 0x3800, 0x0800, CRC(b90efc28) SHA1(61956880f64ae50c4796d4cf60a49c44a81aaa31) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "1211.bin", 0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "1212.bin", 0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "1209.bin", 0x0000, 0x0800, CRC(bb0525ed) SHA1(86db1c7584fb3846bfd47535e1585eeb7fbbb1fe) )
	ROM_LOAD( "1210.bin", 0x0800, 0x0800, CRC(0be2ba91) SHA1(af9243ee23377b632b9b7d0b84d341d06bf22480) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "prom1.bin", 0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "prom2.bin", 0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

ROM_START( condor )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cond01c.bin",  0x0000, 0x0800, CRC(c0f73929) SHA1(3cecf8341a5674165d2cae9b22ea5db26a9597de) )
	ROM_LOAD( "cond02c.bin",  0x0800, 0x0800, CRC(440d56e8) SHA1(b3147d5416cec8c00c7df40b878b826434121737) )
	ROM_LOAD( "cond03c.bin",  0x1000, 0x0800, CRC(750b059b) SHA1(6fbaa2ef4c7eef6f731a73b2d33a02fff21b318a) )
	ROM_LOAD( "cond04c.bin",  0x1800, 0x0800, CRC(ca55e1dd) SHA1(f3d8de56e54ec8651ab95af90ed122096c076c65) )
	ROM_LOAD( "cond05c.bin",  0x2000, 0x0800, CRC(1ff3a982) SHA1(66fb39e7abdf7a9c6e2eb01d41cfe9429781d6aa) )
	ROM_LOAD( "cond06c.bin",  0x2800, 0x0800, CRC(8c83bff7) SHA1(3dfb090d7e3a9ae8da882b06e166c48555eaf77c) )
	ROM_LOAD( "cond07c.bin",  0x3000, 0x0800, CRC(805ec2e8) SHA1(7e56fc9990eb99512078e2b1e2874fb33b0aa05c) )
	ROM_LOAD( "cond08c.bin",  0x3800, 0x0800, CRC(1edebb45) SHA1(2fdf061ee600e27a6ed512ea61a8d78307a7fb8a) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "cond09c.bin",  0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "cond10c.bin",  0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "cond11c.bin",  0x0000, 0x0800, CRC(53c52eb0) SHA1(19624ca359996b77d3c65ef78a7af90eeb092377) )
	ROM_LOAD( "cond12c.bin",  0x0800, 0x0800, CRC(eba42f0f) SHA1(378282cb2c4e10c23179ae3c605ae7bf691150f6) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

// PCB is marked: "NOVARMATIC" and "13200"

ROM_START( condorn ) // verified single PCB, single PROM
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",  0x0000, 0x0800, CRC(c0f73929) SHA1(3cecf8341a5674165d2cae9b22ea5db26a9597de) )
	ROM_LOAD( "2.bin",  0x0800, 0x0800, CRC(440d56e8) SHA1(b3147d5416cec8c00c7df40b878b826434121737) )
	ROM_LOAD( "3.bin",  0x1000, 0x0800, CRC(750b059b) SHA1(6fbaa2ef4c7eef6f731a73b2d33a02fff21b318a) )
	ROM_LOAD( "4.bin",  0x1800, 0x0800, CRC(78416372) SHA1(8e66c253ddae4449af2f4119f1cefa992792c779) )
	ROM_LOAD( "5.bin",  0x2000, 0x0800, CRC(1ff3a982) SHA1(66fb39e7abdf7a9c6e2eb01d41cfe9429781d6aa) )
	ROM_LOAD( "6.bin",  0x2800, 0x0800, CRC(8c83bff7) SHA1(3dfb090d7e3a9ae8da882b06e166c48555eaf77c) )
	ROM_LOAD( "7.bin",  0x3000, 0x0800, CRC(805ec2e8) SHA1(7e56fc9990eb99512078e2b1e2874fb33b0aa05c) )
	ROM_LOAD( "8.bin",  0x3800, 0x0800, CRC(1edebb45) SHA1(2fdf061ee600e27a6ed512ea61a8d78307a7fb8a) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "c.bin",  0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "d.bin",  0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "a.bin",  0x0000, 0x0800, CRC(cdd5ef12) SHA1(1eb4ff6a0d56640acf0de2d6e7b7070e5b2c90d4) )
	ROM_LOAD( "b.bin",  0x0800, 0x0800, CRC(eba42f0f) SHA1(378282cb2c4e10c23179ae3c605ae7bf691150f6) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "sn74s471n.bin",   0x0100, 0x0100, CRC(c68a49bc) SHA1(1a015b89ac0622e73bcebd76cf5132830fe0bfc1) )
ROM_END

// PCB marked "VA 250.291"
ROM_START( condorva )  // Single PCB, single PROM
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.1i", 0x0000, 0x0800, CRC(351984b6) SHA1(c46f988001dd3c4e5c8bc8d472558993ec33384e) )
	ROM_LOAD( "2.1h", 0x0800, 0x0800, CRC(cbf2d29e) SHA1(0897fd69cf7d4e15321b4b7bc8766ebb89b3385c) )
	ROM_LOAD( "3.1f", 0x1000, 0x0800, CRC(b6f3e6ec) SHA1(d1c71da30b00fdc3be7060bb41f37e0f59e01061) )
	ROM_LOAD( "4.1e", 0x1800, 0x0800, CRC(104cb55c) SHA1(b044b339e8908b3a8f45410f63a1a771fbb4a68c) )
	ROM_LOAD( "5.1d", 0x2000, 0x0800, CRC(1a1ce0d0) SHA1(c2825eef5d461e16ca2172daff94b3751be2f4dc) )
	ROM_LOAD( "6.1c", 0x2800, 0x0800, CRC(0de92138) SHA1(d6eba691d1da453cfd00f01a86d1421794df44e4) )
	ROM_LOAD( "7.1b", 0x3000, 0x0800, CRC(bbf12d54) SHA1(3c8ac7dd1abe22fd0469e0f374edc0fbd189019e) )
	ROM_LOAD( "7.1a", 0x3800, 0x0800, CRC(e9816604) SHA1(b8576fa0405db202dba203e97d853b14da1c6749) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "0a.2f", 0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "1a.2e", 0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "0b.4f", 0x0000, 0x0800, CRC(53c52eb0) SHA1(19624ca359996b77d3c65ef78a7af90eeb092377) )
	ROM_LOAD( "1b.4e", 0x0800, 0x0800, CRC(eba42f0f) SHA1(378282cb2c4e10c23179ae3c605ae7bf691150f6) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "sn74s471.b9", 0x0100, 0x0100, CRC(c68a49bc) SHA1(1a015b89ac0622e73bcebd76cf5132830fe0bfc1) )
ROM_END

ROM_START( falcon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "falcon.45",    0x0000, 0x0800, CRC(80382b6c) SHA1(47e24f04b5dd8aa8258ce324a0e4ef68a75dc168) )
	ROM_LOAD( "falcon.46",    0x0800, 0x0800, CRC(6a13193b) SHA1(760347695f1abc92cfe19ea7085e5aaf2dced383) )
	ROM_LOAD( "phoenix.47",   0x1000, 0x0800, CRC(cbbb8839) SHA1(b7f449374cac111081559e39646f973e7e99fd64) )
	ROM_LOAD( "falcon.48",    0x1800, 0x0800, CRC(084e9766) SHA1(844b83041c3cf60c51a045029624296f205847ab) )
	ROM_LOAD( "phoenixc.49",  0x2000, 0x0800, CRC(1a1ce0d0) SHA1(c2825eef5d461e16ca2172daff94b3751be2f4dc) )
	ROM_LOAD( "h6-ic50.6a",   0x2800, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) )
	ROM_LOAD( "falcon.51",    0x3000, 0x0800, CRC(6e82e400) SHA1(22e97f74ca7010bba4263ea844cb7b2c6da09ab7) )
	ROM_LOAD( "h8-ic52.8a",   0x3800, 0x0800, CRC(aff8e9c5) SHA1(e4164f85ec12d4d9bcbffba27ab1f51b3599f6d0) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23.3d",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "ic24.4d",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "b1-ic39.3b",   0x0000, 0x0800, CRC(53413e8f) SHA1(d772358505b973b10da840d204afb210c0c746ec) )
	ROM_LOAD( "b2-ic40.4b",   0x0800, 0x0800, CRC(0be2ba91) SHA1(af9243ee23377b632b9b7d0b84d341d06bf22480) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

ROM_START( vautour )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vautor01.1e",  0x0000, 0x0800, CRC(cd2807ee) SHA1(79b9769f212d25b9ccb5124e2aa632c964c14a0b) )
	ROM_LOAD( "phoenix.46",   0x0800, 0x0800, CRC(dbc942fa) SHA1(9fe224e6ced407289dfa571468259a021d942b7d) )
	ROM_LOAD( "phoenix.47",   0x1000, 0x0800, CRC(cbbb8839) SHA1(b7f449374cac111081559e39646f973e7e99fd64) )
	ROM_LOAD( "vautor04.1j",  0x1800, 0x0800, CRC(106262eb) SHA1(1e52ca66ea3542d86f2604f5aadc854ffe22fd89) )
	ROM_LOAD( "phoenixc.49",  0x2000, 0x0800, CRC(1a1ce0d0) SHA1(c2825eef5d461e16ca2172daff94b3751be2f4dc) )
	ROM_LOAD( "vautor06.1h",  0x2800, 0x0800, CRC(c90e3287) SHA1(696014738d3b87acb10175b021d9fd4885904a9f) )
	ROM_LOAD( "vautor07.1m",  0x3000, 0x0800, CRC(079ac364) SHA1(55b17c069b191cd1752e78068ef683b33c094e56) )
	ROM_LOAD( "vautor08.1n",  0x3800, 0x0800, CRC(1dbd937a) SHA1(efed9adad3d639c893b33071fd86c64800a7a32f) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23.3d",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "ic24.4d",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "vautor12.2h",  0x0000, 0x0800, CRC(8eff75c9) SHA1(d38a0e0c02ba680984dd8748a3c45ac55f81f127) )
	ROM_LOAD( "vautor11.2j",  0x0800, 0x0800, CRC(369e7476) SHA1(599d2fc3b298060d746e95c20a089ad37f685d5b) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

ROM_START( vautourza )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.e1",  0x0000, 0x0800, CRC(cd2807ee) SHA1(79b9769f212d25b9ccb5124e2aa632c964c14a0b) )
	ROM_LOAD( "2.f1",  0x0800, 0x0800, CRC(3699b11a) SHA1(7122685cbfcd75898eaa68f8c5bf87c11df59a3b) )
	ROM_LOAD( "3.h1",  0x1000, 0x0800, CRC(cbbb8839) SHA1(b7f449374cac111081559e39646f973e7e99fd64) )
	ROM_LOAD( "4.j1",  0x1800, 0x0800, CRC(106262eb) SHA1(1e52ca66ea3542d86f2604f5aadc854ffe22fd89) )
	ROM_LOAD( "5.k1",  0x2000, 0x0800, CRC(1a1ce0d0) SHA1(c2825eef5d461e16ca2172daff94b3751be2f4dc) )
	ROM_LOAD( "6.h1",  0x2800, 0x0800, CRC(1fcac707) SHA1(ea10a1c94d8cf49391a4d393ccef56ae3b9458b1) )
	ROM_LOAD( "7.m1",  0x3000, 0x0800, CRC(805ec2e8) SHA1(7e56fc9990eb99512078e2b1e2874fb33b0aa05c) )
	ROM_LOAD( "8.n1",  0x3800, 0x0800, CRC(1edebb45) SHA1(2fdf061ee600e27a6ed512ea61a8d78307a7fb8a) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "10.h2",     0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "9.j2",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "12.h4",  0x0000, 0x0800, CRC(8eff75c9) SHA1(d38a0e0c02ba680984dd8748a3c45ac55f81f127) )
	ROM_LOAD( "11.j4",  0x0800, 0x0800, CRC(369e7476) SHA1(599d2fc3b298060d746e95c20a089ad37f685d5b) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s135.m9",   0x0100, 0x0100, CRC(c68a49bc) SHA1(1a015b89ac0622e73bcebd76cf5132830fe0bfc1) )  // expanded in init (upper nibbles are the ic40 data, lower nibbles ic41 data)
ROM_END

ROM_START( falconz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "f45.bin",      0x0000, 0x0800, CRC(9158b43b) SHA1(222cbcfb3f95d09bb90148813541c2613d8b7e1c) )
	ROM_LOAD( "f46.bin",      0x0800, 0x0800, CRC(22ddb600) SHA1(9606d11722261990c34b788baae5dc7516ba52d6) )
	ROM_LOAD( "f47.bin",      0x1000, 0x0800, CRC(cb2838d9) SHA1(332e339475b17d17eeb43a524c5cb3bf9818837a) )
	ROM_LOAD( "f48.bin",      0x1800, 0x0800, CRC(552cf57a) SHA1(d9d36495f0cb25c1648e0807c8d37daf49bbf0d4) )
	ROM_LOAD( "f49.bin",      0x2000, 0x0800, CRC(1ff3a982) SHA1(66fb39e7abdf7a9c6e2eb01d41cfe9429781d6aa) )
	ROM_LOAD( "f50.bin",      0x2800, 0x0800, CRC(8c83bff7) SHA1(3dfb090d7e3a9ae8da882b06e166c48555eaf77c) )
	ROM_LOAD( "f51.bin",      0x3000, 0x0800, CRC(805ec2e8) SHA1(7e56fc9990eb99512078e2b1e2874fb33b0aa05c) )
	ROM_LOAD( "f52.bin",      0x3800, 0x0800, CRC(33f3af63) SHA1(f2e2ebdec205360a6fa8ce4bc8cdf15b82b02728) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23.3d",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "ic24.4d",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "f39.bin",      0x0000, 0x0800, CRC(53c52eb0) SHA1(19624ca359996b77d3c65ef78a7af90eeb092377) )
	ROM_LOAD( "f40.bin",      0x0800, 0x0800, CRC(eba42f0f) SHA1(378282cb2c4e10c23179ae3c605ae7bf691150f6) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

ROM_START( vautourz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vautour1.bin", 0x0000, 0x0800, CRC(a600f6a4) SHA1(f1aea5ec71da0d0664fb90b87602fe4c4eed2dbe) )
	ROM_LOAD( "vautour2.bin", 0x0800, 0x0800, CRC(3699b11a) SHA1(7122685cbfcd75898eaa68f8c5bf87c11df59a3b) )
	ROM_LOAD( "vautour3.bin", 0x1000, 0x0800, CRC(750b059b) SHA1(6fbaa2ef4c7eef6f731a73b2d33a02fff21b318a) )
	ROM_LOAD( "vautour4.bin", 0x1800, 0x0800, CRC(01a4bfde) SHA1(a740b8f43a226eb585ea538d41228a98392872d6) )
	ROM_LOAD( "vautour5.bin", 0x2000, 0x0800, CRC(1ff3a982) SHA1(66fb39e7abdf7a9c6e2eb01d41cfe9429781d6aa) )
	ROM_LOAD( "vautour6.bin", 0x2800, 0x0800, CRC(8c83bff7) SHA1(3dfb090d7e3a9ae8da882b06e166c48555eaf77c) )
	ROM_LOAD( "vautour7.bin", 0x3000, 0x0800, CRC(805ec2e8) SHA1(7e56fc9990eb99512078e2b1e2874fb33b0aa05c) )
	ROM_LOAD( "vautour8.bin", 0x3800, 0x0800, CRC(1edebb45) SHA1(2fdf061ee600e27a6ed512ea61a8d78307a7fb8a) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23.3d",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "ic24.4d",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "vautor12.2h",  0x0000, 0x0800, CRC(8eff75c9) SHA1(d38a0e0c02ba680984dd8748a3c45ac55f81f127) )
	ROM_LOAD( "vautor11.2j",  0x0800, 0x0800, CRC(369e7476) SHA1(599d2fc3b298060d746e95c20a089ad37f685d5b) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

/*
PCB is marked: "ORIO/G"
PCB is labeled: "FONECHIS"
*/

ROM_START( fenix ) // verified single PCB, single PROM
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0.1e",         0x0000, 0x0800, NO_DUMP ) // socket at location '1e' is empty.
	ROM_LOAD( "1.1f",         0x0800, 0x0800, CRC(3699b11a) SHA1(7122685cbfcd75898eaa68f8c5bf87c11df59a3b) )    // 1.1f
	ROM_LOAD( "2.1h",         0x1000, 0x0800, CRC(750b059b) SHA1(6fbaa2ef4c7eef6f731a73b2d33a02fff21b318a) )    // 2.1h
	ROM_LOAD( "3.1j",         0x1800, 0x0800, CRC(61b8a41b) SHA1(6dc7b23cee607042183ac13a0ecf408d88057513) )    //          96.386719%
	ROM_LOAD( "4.1k",         0x2000, 0x0800, CRC(1ff3a982) SHA1(66fb39e7abdf7a9c6e2eb01d41cfe9429781d6aa) )    // 4.1k
	ROM_LOAD( "5.1l",         0x2800, 0x0800, CRC(a210fe51) SHA1(0487d5bc835549cf2bfb8f26f665019490f643b7) )    //          82.812500%
	ROM_LOAD( "6.1m",         0x3000, 0x0800, CRC(805ec2e8) SHA1(7e56fc9990eb99512078e2b1e2874fb33b0aa05c) )    // 6.1m
	ROM_LOAD( "7.1n",         0x3800, 0x0800, CRC(1edebb45) SHA1(2fdf061ee600e27a6ed512ea61a8d78307a7fb8a) )    // 7.1n

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "9.2h",         0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )    // 9.2h
	ROM_LOAD( "8.2j",         0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )    // 8.2j

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "11.3h",        0x0000, 0x0800, CRC(8eff75c9) SHA1(d38a0e0c02ba680984dd8748a3c45ac55f81f127) )    // 11.3h
	ROM_LOAD( "10.3j",        0x0800, 0x0800, CRC(369e7476) SHA1(599d2fc3b298060d746e95c20a089ad37f685d5b) )    // 10.3j

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END



ROM_START( avefenix )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4101-8516.rom",   0x0000, 0x0800, CRC(5bc2e2fe) SHA1(4d625a3bd423cc1329a6311b87683cb7c0a9374f) )
	ROM_LOAD( "4102-2716.rom",   0x0800, 0x0800, CRC(dcf2cc3e) SHA1(adffb23ffab4f23d9da40a23e92aa08446d3dc7d) )
	ROM_LOAD( "4103-8516.rom",   0x1000, 0x0800, CRC(cbbb8839) SHA1(b7f449374cac111081559e39646f973e7e99fd64) )
	ROM_LOAD( "4104-8516.rom",   0x1800, 0x0800, CRC(8380a581) SHA1(1b56f3e44de93d12008a049c2c71fc23627299e0) )
	ROM_LOAD( "4105-8516.rom",   0x2000, 0x0800, CRC(cfa8cb51) SHA1(79a7de61927a602bd06d87a1314276929e613cd3) )
	ROM_LOAD( "4106-8516.rom",   0x2800, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) )
	ROM_LOAD( "4107-8516.rom",   0x3000, 0x0800, CRC(2eab35b4) SHA1(849bf8273317cc869bdd67e50c68399ee8ece81d) )
	ROM_LOAD( "4108-8516.rom",   0x3800, 0x0800, CRC(f15c439d) SHA1(6b80276b4ddc9989adb2981f018d5c9c55b06430) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "41011-8516.rom",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "41012-8516.rom",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "4109-8516.rom",   0x0000, 0x0800, CRC(53413e8f) SHA1(d772358505b973b10da840d204afb210c0c746ec) )
	ROM_LOAD( "41010-8516.rom",   0x0800, 0x0800, CRC(0be2ba91) SHA1(af9243ee23377b632b9b7d0b84d341d06bf22480) )

	ROM_REGION( 0x0200, "proms", 0 ) // NOT verified on this set
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

ROM_START( avefenixrf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "601-ic45.a1",    0x0000, 0x0800, CRC(b04260e9) SHA1(a275ba596e620b1e4dd73792cc9b11fbc55723cc) )
	ROM_LOAD( "6002-ic46.a2",   0x0800, 0x0800, CRC(25a2e4bd) SHA1(a1188c4d1d059852b59ccfe16dd9a305d7b26299) )
	ROM_LOAD( "0003-ic47.a3",   0x1000, 0x0800, CRC(cbbb8839) SHA1(b7f449374cac111081559e39646f973e7e99fd64) )
	ROM_LOAD( "6004-ic48.a4",   0x1800, 0x0800, CRC(4b7701b4) SHA1(2330802d4016450985f85937fd58e2a71e77c719) )
	ROM_LOAD( "6005-ic49.a5",   0x2000, 0x0800, CRC(1ab92ef9) SHA1(ff5a263865895f8534f8535fbcb398af8512cbfe) )
	ROM_LOAD( "0006-ic50.a6",   0x2800, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) )
	ROM_LOAD( "6007-ic51.a7",   0x3000, 0x0800, CRC(2eab35b4) SHA1(849bf8273317cc869bdd67e50c68399ee8ece81d) )
	ROM_LOAD( "f008-ic52.a8",   0x3800, 0x0800, CRC(3719fc84) SHA1(70691d55a86cdad21938a8af2a84ab9cfdc8d76a) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "0011-ic23.d3",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "0012-ic24.d4",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "0009-ic39.b3",   0x0000, 0x0800, CRC(bb0525ed) SHA1(86db1c7584fb3846bfd47535e1585eeb7fbbb1fe) )
	ROM_LOAD( "0010-ic40.b4",   0x0800, 0x0800, CRC(4178aa4f) SHA1(5350f8f62cc7c223c38008bc83140b7a19147d81) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END


ROM_START( avefenixl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "01_ic45.a1",   0x0000, 0x0800, CRC(2c53998c) SHA1(6adaea6c88ebbbbf11d78bbbb35c4ed2f4e7e531) )
	ROM_LOAD( "02_ic46.a2",   0x0800, 0x0800, CRC(fea2435c) SHA1(f02bf68074dbfcfa259b98d16a8d942ddd71409a) )
	ROM_LOAD( "03_ic47.a3",   0x1000, 0x0800, CRC(cbbb8839) SHA1(b7f449374cac111081559e39646f973e7e99fd64) )
	ROM_LOAD( "04_ic48.a4",   0x1800, 0x0800, CRC(90a02a45) SHA1(ec3033100d5ed21948bba9fca8754fb6d725d83d) )
	ROM_LOAD( "05_ic49.a5",   0x2000, 0x0800, CRC(74b1cf66) SHA1(38f9915b239c30f45567e165e9320558f1197ff9) )
	ROM_LOAD( "06_ic50.a6",   0x2800, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) )
	ROM_LOAD( "07_ic51.a7",   0x3000, 0x0800, CRC(2eab35b4) SHA1(849bf8273317cc869bdd67e50c68399ee8ece81d) )
	ROM_LOAD( "08_ic52.a8",   0x3800, 0x0800, CRC(f15c439d) SHA1(6b80276b4ddc9989adb2981f018d5c9c55b06430) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "11_ic23.d3",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "12_ic24.d4",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "09_ic39.b3",   0x0000, 0x0800, CRC(bb0525ed) SHA1(86db1c7584fb3846bfd47535e1585eeb7fbbb1fe) )
	ROM_LOAD( "10_ic40.b4",   0x0800, 0x0800, CRC(4178aa4f) SHA1(5350f8f62cc7c223c38008bc83140b7a19147d81) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

// PCB is marked: "003 LATO A" on component side and "003 LATO B" on solder side. (In Italian "lato" means "side")

ROM_START( griffon )  // verified single PCB, single PROM
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "griffon0.a5",  0x0000, 0x0800, CRC(c0f73929) SHA1(3cecf8341a5674165d2cae9b22ea5db26a9597de) )
	ROM_LOAD( "griffon1.a6",  0x0800, 0x0800, CRC(3cc33e4a) SHA1(45d16334f245cc185e18f63062e08627e9bd06bb) )
	ROM_LOAD( "griffon2.a7",  0x1000, 0x0800, CRC(750b059b) SHA1(6fbaa2ef4c7eef6f731a73b2d33a02fff21b318a) )
	// ROM_LOAD( "griffon3.a8",  0x1800, 0x0800, CRC(3a6188eb) SHA1(f343d7a6dc836d118621e9a143799e33658a3005) ) // suspected bitrot, new dump has been confirmed on 2 different PCBs
	ROM_LOAD( "griffon3.a8",  0x1800, 0x0800, CRC(5e49f5b5) SHA1(288314d24b3e8f507c43db6c64a7777e97215ed1) )
	ROM_LOAD( "griffon4.a9",  0x2000, 0x0800, CRC(87a45ceb) SHA1(0788dd57eac3047e34a50e730df37059616abc1a) )
	ROM_LOAD( "griffon5.a10", 0x2800, 0x0800, CRC(8c83bff7) SHA1(3dfb090d7e3a9ae8da882b06e166c48555eaf77c) )
	ROM_LOAD( "griffon6.a11", 0x3000, 0x0800, CRC(805ec2e8) SHA1(7e56fc9990eb99512078e2b1e2874fb33b0aa05c) )
	ROM_LOAD( "griffon7.a12", 0x3800, 0x0800, CRC(55e68cb1) SHA1(b19de884fb3c988599772a76c0c5229e76241e6d) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23.3d",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "ic24.4d",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "griffon.d7",   0x0000, 0x0800, CRC(53c52eb0) SHA1(19624ca359996b77d3c65ef78a7af90eeb092377) )
	ROM_LOAD( "griffon.d8",   0x0800, 0x0800, CRC(eba42f0f) SHA1(378282cb2c4e10c23179ae3c605ae7bf691150f6) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "sn74s471n.bin",   0x0100, 0x0100, CRC(c68a49bc) SHA1(1a015b89ac0622e73bcebd76cf5132830fe0bfc1) )
ROM_END

/*There is no MN6221AA Melody-Alarm Generator, it is substituted by a small piggyback sound PCB with some 74xx logic
  and 2x PROM 63S140N main PCB is marked: "003 LATO A" and "003 LATO B" sound PCB is marked: "LC" and "NW"*/

ROM_START( griffono )  // verified single PCB with piggyback sound PCB instead of MM6221AA, single PROM
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "griffon00.5a",    0x0000, 0x0800, CRC(c0f73929) SHA1(3cecf8341a5674165d2cae9b22ea5db26a9597de) )
	ROM_LOAD( "griffon01.6a",    0x0800, 0x0800, CRC(22ec3403) SHA1(528a0ee3e78a43e57cc7666818cb300144d410ae) )
	ROM_LOAD( "griffon02.7a",    0x1000, 0x0800, CRC(750b059b) SHA1(6fbaa2ef4c7eef6f731a73b2d33a02fff21b318a) )
	ROM_LOAD( "griffon03.8a",    0x1800, 0x0800, CRC(73c90019) SHA1(7cc45e57d3566aa0b474f4bbfbaddfc4372473c4) )
	ROM_LOAD( "griffon04.9a",    0x2000, 0x0800, CRC(87a45ceb) SHA1(0788dd57eac3047e34a50e730df37059616abc1a) )
	ROM_LOAD( "griffon05.10a",   0x2800, 0x0800, CRC(8c83bff7) SHA1(3dfb090d7e3a9ae8da882b06e166c48555eaf77c) )
	ROM_LOAD( "griffon06.11a",   0x3000, 0x0800, CRC(805ec2e8) SHA1(7e56fc9990eb99512078e2b1e2874fb33b0aa05c) )
	ROM_LOAD( "griffon07-0.12a", 0x3800, 0x0800, CRC(7fbdd2fa) SHA1(13f3a6b5bd2191b6d40ae224f77200c7779ee8eb) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "griffonb7.7b",    0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "griffonb8.8b",    0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "griffond7-0.7d",  0x0000, 0x0800, CRC(074eb660) SHA1(5f0cd92e22b6948dffbf56e5ea57cdda9b3e2c51) )
	ROM_LOAD( "griffond8.8d",    0x0800, 0x0800, CRC(bee48e0c) SHA1(a129510a6bddafe3de3f921fca34ecda03a1c283) )

	ROM_REGION( 0x0200, "soundproms", 0 ) // currently not used by the emulation
	ROM_LOAD( "63s140n.1",   0x0000, 0x0100, CRC(73d5f0c2) SHA1(5ca482ab68ffd52d16c532c72c1a2ed693648995) )
	ROM_LOAD( "63s140n.2",   0x0100, 0x0100, CRC(c790b283) SHA1(50b14ffbf69851995ab9fe54d0ea58dc7a30a9ba) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "sn74s471n.11k",   0x0100, 0x0100, CRC(c68a49bc) SHA1(1a015b89ac0622e73bcebd76cf5132830fe0bfc1) )
ROM_END

/*
  There is no MN6221AA Melody-Alarm Generator.
  There is a EPSON Electronics 7910CG Multi-Melody IC instead.
*/
ROM_START( nextfase )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nf01.bin",   0x0000, 0x0800, CRC(b31ce820) SHA1(dfdb17995a14b66d2571c2c8de481d2792f9ce6a) )
	ROM_LOAD( "nf02.bin",   0x0800, 0x0800, CRC(891d21e1) SHA1(bea01962c0706c00eae42920bb2b3bfdb7e80d89) )
	ROM_LOAD( "nf03.bin",   0x1000, 0x0800, CRC(2ab7389d) SHA1(c0bc0c235cae4a8e880237196ea1718f8c1d0123) )
	ROM_LOAD( "nf04.bin",   0x1800, 0x0800, CRC(590d3c36) SHA1(89e87f207cdb9a7f5624170c09626ef85ede3969) )
	ROM_LOAD( "nf05.bin",   0x2000, 0x0800, CRC(3527f247) SHA1(0cccbc3e15d7603deaec845581983bfbcc4d4560) )
	ROM_LOAD( "nf06.bin",   0x2800, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) )
	ROM_LOAD( "nf07.bin",   0x3000, 0x0800, CRC(2eab35b4) SHA1(849bf8273317cc869bdd67e50c68399ee8ece81d) )
	ROM_LOAD( "nf08.bin",   0x3800, 0x0800, CRC(04c2323f) SHA1(4d820464f57e4f59acc46ea3264dba3cb9c501a1) )

	ROM_REGION( 0x800, "melody", 0 ) // Epson 7910CG Multi-Melody IC
	ROM_LOAD( "7910cg", 0x000, 0x800, NO_DUMP ) // actual size unknown, needs decapping

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "nf11.bin",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "nf12.bin",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "nf09.bin",   0x0000, 0x0800, CRC(bacbfa88) SHA1(bf378a729726db01448f2cc4820f06e17659d674) )
	ROM_LOAD( "nf10.bin",   0x0800, 0x0800, CRC(3143a9ee) SHA1(371bb314dc9e4ec6ed469eb81391061296c547ec) )

	// PROMs were missing from this dump, these might not be correct
	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

ROM_START( phoenixs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic45.1_a1",   0x0000, 0x0800, CRC(5b8c55a8) SHA1(839c1ca9766f730ec3accd48db70f6429a9c3362) )
	ROM_LOAD( "ic46.2_a2",   0x0800, 0x0800, CRC(dbc942fa) SHA1(9fe224e6ced407289dfa571468259a021d942b7d) )
	ROM_LOAD( "ic47.3_a3",   0x1000, 0x0800, CRC(cbbb8839) SHA1(b7f449374cac111081559e39646f973e7e99fd64) )
	ROM_LOAD( "ic48.4_a4",   0x1800, 0x0800, CRC(25c8b83f) SHA1(1c47b6ad14560927ffd50217d489ff00f8276f23) )
	ROM_LOAD( "ic49.5_a5",   0x2000, 0x0800, CRC(1a1ce0d0) SHA1(c2825eef5d461e16ca2172daff94b3751be2f4dc) )
	ROM_LOAD( "ic50.6_a6",   0x2800, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) )
	ROM_LOAD( "ic51.7_a7",   0x3000, 0x0800, CRC(2eab35b4) SHA1(849bf8273317cc869bdd67e50c68399ee8ece81d) )
	ROM_LOAD( "ic52.8_a8",   0x3800, 0x0800, CRC(3657f69b) SHA1(2251efa057ad312d270016236a01932d2bb3a1be) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23.11_d3",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "ic24.12_d4",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "ic39.9_b3",   0x0000, 0x0800, CRC(14ccdf63) SHA1(42827f2150fae82523475428eaa9db3c824b94dd) )
	ROM_LOAD( "ic40.10_b4",  0x0800, 0x0800, CRC(eba42f0f) SHA1(378282cb2c4e10c23179ae3c605ae7bf691150f6) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40",   0x0000, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // palette low bits
	ROM_LOAD( "mmi6301.ic41",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END

ROM_START( phoenixbl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pn1-2716.a1",  0x0000, 0x0800, CRC(5b8c55a8) SHA1(839c1ca9766f730ec3accd48db70f6429a9c3362) )
	ROM_LOAD( "14-2716.a2",   0x0800, 0x0800, CRC(d00ba009) SHA1(c30b9a803f828a9510e60b02b3fcf352436dc5d8) )
	ROM_LOAD( "3-2716.a3",    0x1000, 0x0800, CRC(cbbb8839) SHA1(b7f449374cac111081559e39646f973e7e99fd64) )
	ROM_LOAD( "4-2716.a4",    0x1800, 0x0800, CRC(95534db2) SHA1(3d20c6c786299a96308f76f4d32a16b2bf35fe0d) )
	ROM_LOAD( "5-2716.a5",    0x2000, 0x0800, CRC(1a1ce0d0) SHA1(c2825eef5d461e16ca2172daff94b3751be2f4dc) )
	ROM_LOAD( "6-2716.a6",    0x2800, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) )
	ROM_LOAD( "7-2716.a7",    0x3000, 0x0800, CRC(2eab35b4) SHA1(849bf8273317cc869bdd67e50c68399ee8ece81d) )
	ROM_LOAD( "8-2716.a8",    0x3800, 0x0800, CRC(89690582) SHA1(96dddb3dccb6e768fe28fbd4db24a3e81b81ecc3) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "11-2716.d3",   0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "12-2716.d4",   0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "9-2716.b3",    0x0000, 0x0800, CRC(14ccdf63) SHA1(42827f2150fae82523475428eaa9db3c824b94dd) )
	ROM_LOAD( "10-2716.b4",   0x0800, 0x0800, CRC(eba42f0f) SHA1(378282cb2c4e10c23179ae3c605ae7bf691150f6) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6301.ic40", 0x0000, 0x0100, BAD_DUMP CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )  // not dumped on this set
	ROM_LOAD( "mmi6301.ic41", 0x0100, 0x0100, BAD_DUMP CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // not dumped on this set
ROM_END

ROM_START( phoenixass ) // Uses MN6221AB melody chip (Greensleeves) instead of MN6221AA
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "00-811.ic45",   0x0000, 0x0800, CRC(5b8c55a8) SHA1(839c1ca9766f730ec3accd48db70f6429a9c3362) )
	ROM_LOAD( "01-811.ic46",   0x0800, 0x0800, CRC(dbc942fa) SHA1(9fe224e6ced407289dfa571468259a021d942b7d) )
	ROM_LOAD( "02-811.ic47",   0x1000, 0x0800, CRC(cbbb8839) SHA1(b7f449374cac111081559e39646f973e7e99fd64) )
	ROM_LOAD( "03-811.ic48",   0x1800, 0x0800, CRC(1e2e2fc7) SHA1(b181411d1f7c11ee27e4410d20bd509b21dd7242) ) // a second PCB was found with the only difference being the copyright being changed from 'Assa' to 'Matto'
	ROM_LOAD( "04-811.ic49",   0x2000, 0x0800, CRC(1a1ce0d0) SHA1(c2825eef5d461e16ca2172daff94b3751be2f4dc) )
	ROM_LOAD( "05-811.ic50",   0x2800, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) )
	ROM_LOAD( "06-811.ic51",   0x3000, 0x0800, CRC(2eab35b4) SHA1(849bf8273317cc869bdd67e50c68399ee8ece81d) )
	ROM_LOAD( "07-811.ic52",   0x3800, 0x0800, CRC(15a02d87) SHA1(df69d99747dd8b42187e4a4258edfae8e89663d0) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "10-811.ic23",      0x0000, 0x0800, CRC(3c7e623f) SHA1(e7ff5fc371664af44785c079e92eeb2d8530187b) )
	ROM_LOAD( "11-811.ic24",      0x0800, 0x0800, CRC(59916d3b) SHA1(71aec70a8e096ed1f0c2297b3ae7dca1b8ecc38d) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "08-811.ic39",   0x0000, 0x0800, CRC(bb0525ed) SHA1(86db1c7584fb3846bfd47535e1585eeb7fbbb1fe) )
	ROM_LOAD( "09-811.ic40",   0x0800, 0x0800, CRC(4178aa4f) SHA1(5350f8f62cc7c223c38008bc83140b7a19147d81) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "dm74s287n.ic41",   0x0000, 0x0100, CRC(7c9f2e00) SHA1(372293748b0d4254d2884bafe4f9f33fbf0c03a6) )  // palette low bits // slightly different to other sets (note IC positions reversed)
	ROM_LOAD( "dm74s287n.ic40",   0x0100, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )  // palette high bits
ROM_END


ROM_START( pleiads )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic47.r1",      0x0000, 0x0800, CRC(960212c8) SHA1(52a3232e99920805ce9e195b8a6338ae7044dd18) )
	ROM_LOAD( "ic48.r2",      0x0800, 0x0800, CRC(b254217c) SHA1(312a33cca09d5d2d18992f28eb051230a90db6e3) )
	ROM_LOAD( "ic47.bin",     0x1000, 0x0800, CRC(87e700bb) SHA1(0f352b5461da957c564920fd1da83bc81f41ffb9) ) // IC 49 on real board
	ROM_LOAD( "ic48.bin",     0x1800, 0x0800, CRC(2d5198d0) SHA1(6bfdc6c965199c5d4d687fe35dda057ec38cd8e0) ) // IC 50 on real board
	ROM_LOAD( "ic51.r5",      0x2000, 0x0800, CRC(49c629bc) SHA1(fd7937d0c114c8d9c1efaa9918ae3df2af41f032) )
	ROM_LOAD( "ic50.bin",     0x2800, 0x0800, CRC(f1a8a00d) SHA1(5c183e3a73fa882ffec3cb9219fb5619e625591a) ) // IC 52 on real board
	ROM_LOAD( "ic53.r7",      0x3000, 0x0800, CRC(b5f07fbc) SHA1(2ae687c84732942e69ad4dfb7a4ac1b97b77487a) )
	ROM_LOAD( "ic52.bin",     0x3800, 0x0800, CRC(b1b5a8a6) SHA1(7e4ef298c8ddefc7dc0cbf94a9c9f36a4b807ba0) ) // IC 54 on real board

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23.bin",     0x0000, 0x0800, CRC(4e30f9e7) SHA1(da023a94725dc40107cd97e4decfd4dc0f9f00ee) ) // IC 45 on real board
	ROM_LOAD( "ic24.bin",     0x0800, 0x0800, CRC(5188fc29) SHA1(421dedc674c6dde7abf01412df035a8eb8e6db9b) ) // IC 44 on real board

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "ic39.bin",     0x0000, 0x0800, CRC(85866607) SHA1(cd240bd056f761b2f9e2142049434f02cae3e315) ) // IC 27 on real board
	ROM_LOAD( "ic40.bin",     0x0800, 0x0800, CRC(a841d511) SHA1(8349008ab1d8ef08775b54170c37deb1d391fffc) ) // IC 26 on real board

	ROM_REGION( 0x800, "melody", 0 ) // Epson 7910E Multi-Melody IC (according to Centuri version manual)
	ROM_LOAD( "7910e", 0x000, 0x800, NO_DUMP ) // actual size unknown, needs decapping

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "7611-5.33",    0x0000, 0x0100, CRC(e38eeb83) SHA1(252880d80425b2e697146e76efdc6cb9f3ba0378) )   // palette low bits
	ROM_LOAD( "7611-5.26",    0x0100, 0x0100, CRC(7a1bcb1e) SHA1(bdfab316ea26e2063879e7aa78b6ae2b55eb95c8) )   // palette high bits
ROM_END

ROM_START( pleiadsb2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic47.r1",      0x0000, 0x0800, CRC(fa98cb73) SHA1(d01138536e90a0a92d3e356fe354648e431a106c) ) // sldh
	ROM_LOAD( "ic48.r2",      0x0800, 0x0800, CRC(b254217c) SHA1(312a33cca09d5d2d18992f28eb051230a90db6e3) )
	ROM_LOAD( "ic47.bin",     0x1000, 0x0800, CRC(0951829e) SHA1(03c0598dfe248ce14683ff18e59adb2e72731336) ) // sldh
	ROM_LOAD( "ic48.bin",     0x1800, 0x0800, CRC(4972f5ce) SHA1(9175cc924c335d01ee47f3771276cdc90028fcc5) ) // sldh
	ROM_LOAD( "ic51.r5",      0x2000, 0x0800, CRC(49c629bc) SHA1(fd7937d0c114c8d9c1efaa9918ae3df2af41f032) )
	ROM_LOAD( "ic50.bin",     0x2800, 0x0800, CRC(f1a8a00d) SHA1(5c183e3a73fa882ffec3cb9219fb5619e625591a) )
	ROM_LOAD( "ic53.r7",      0x3000, 0x0800, CRC(037b319c) SHA1(2ff7a7777a63326e2abca2d1881df33a8e3f8561) ) // sldh
	ROM_LOAD( "ic52.bin",     0x3800, 0x0800, CRC(b3db08c2) SHA1(d5b1b77dcf2d76498f30d5f880635f5acfac7dfd) ) // sldh

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23.bin",     0x0000, 0x0800, CRC(4e30f9e7) SHA1(da023a94725dc40107cd97e4decfd4dc0f9f00ee) )
	ROM_LOAD( "ic24.bin",     0x0800, 0x0800, CRC(5188fc29) SHA1(421dedc674c6dde7abf01412df035a8eb8e6db9b) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "ic39.bin",     0x0000, 0x0800, CRC(85866607) SHA1(cd240bd056f761b2f9e2142049434f02cae3e315) )
	ROM_LOAD( "ic40.bin",     0x0800, 0x0800, CRC(a841d511) SHA1(8349008ab1d8ef08775b54170c37deb1d391fffc) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "7611-5.26",    0x0000, 0x0100, CRC(7a1bcb1e) SHA1(bdfab316ea26e2063879e7aa78b6ae2b55eb95c8) )   // palette low bits
	ROM_LOAD( "7611-5.33",    0x0100, 0x0100, CRC(e38eeb83) SHA1(252880d80425b2e697146e76efdc6cb9f3ba0378) )   // palette high bits
ROM_END

ROM_START( pleiadbl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic45.bin",     0x0000, 0x0800, CRC(93fc2958) SHA1(d8723c4f4376e035e655f69352c1765fdbf4a602) )
	ROM_LOAD( "ic46.bin",     0x0800, 0x0800, CRC(e2b5b8cd) SHA1(514ab2b24fc1d6d1fd64e74470b601ba9a11f36f) )
	ROM_LOAD( "ic47.bin",     0x1000, 0x0800, CRC(87e700bb) SHA1(0f352b5461da957c564920fd1da83bc81f41ffb9) )
	ROM_LOAD( "ic48.bin",     0x1800, 0x0800, CRC(2d5198d0) SHA1(6bfdc6c965199c5d4d687fe35dda057ec38cd8e0) )
	ROM_LOAD( "ic49.bin",     0x2000, 0x0800, CRC(9dc73e63) SHA1(8a2de6666fecead7071285125b16641b50249adc) )
	ROM_LOAD( "ic50.bin",     0x2800, 0x0800, CRC(f1a8a00d) SHA1(5c183e3a73fa882ffec3cb9219fb5619e625591a) )
	ROM_LOAD( "ic51.bin",     0x3000, 0x0800, CRC(6f56f317) SHA1(d7e6b0b1c58b741de3504640bcc23e86d1a134a0) )
	ROM_LOAD( "ic52.bin",     0x3800, 0x0800, CRC(b1b5a8a6) SHA1(7e4ef298c8ddefc7dc0cbf94a9c9f36a4b807ba0) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic23.bin",     0x0000, 0x0800, CRC(4e30f9e7) SHA1(da023a94725dc40107cd97e4decfd4dc0f9f00ee) )
	ROM_LOAD( "ic24.bin",     0x0800, 0x0800, CRC(5188fc29) SHA1(421dedc674c6dde7abf01412df035a8eb8e6db9b) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "ic39.bin",     0x0000, 0x0800, CRC(85866607) SHA1(cd240bd056f761b2f9e2142049434f02cae3e315) )
	ROM_LOAD( "ic40.bin",     0x0800, 0x0800, CRC(a841d511) SHA1(8349008ab1d8ef08775b54170c37deb1d391fffc) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "7611-5.33",    0x0000, 0x0100, CRC(e38eeb83) SHA1(252880d80425b2e697146e76efdc6cb9f3ba0378) )   // palette low bits
	ROM_LOAD( "7611-5.26",    0x0100, 0x0100, CRC(7a1bcb1e) SHA1(bdfab316ea26e2063879e7aa78b6ae2b55eb95c8) )   // palette high bits
ROM_END

ROM_START( pleiadce )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pleiades.47",  0x0000, 0x0800, CRC(711e2ba0) SHA1(62d9108b9066d3e2b99c712daf2b9412704970cc) )
	ROM_LOAD( "pleiades.48",  0x0800, 0x0800, CRC(93a36943) SHA1(7cb4a9e8b60e28415df8401373ff4e595eaab7f5) )
	ROM_LOAD( "ic47.bin",     0x1000, 0x0800, CRC(87e700bb) SHA1(0f352b5461da957c564920fd1da83bc81f41ffb9) )
	ROM_LOAD( "pleiades.50",  0x1800, 0x0800, CRC(5a9beba0) SHA1(e9cf03c88d8db2a7cf97877a103cfdd1fe3f459e) )
	ROM_LOAD( "pleiades.51",  0x2000, 0x0800, CRC(1d828719) SHA1(54857a3de9f4c9c5f18b0d46cf428b4171f839e9) )
	ROM_LOAD( "ic50.bin",     0x2800, 0x0800, CRC(f1a8a00d) SHA1(5c183e3a73fa882ffec3cb9219fb5619e625591a) )
	ROM_LOAD( "pleiades.53",  0x3000, 0x0800, CRC(037b319c) SHA1(2ff7a7777a63326e2abca2d1881df33a8e3f8561) )
	ROM_LOAD( "pleiades.54",  0x3800, 0x0800, CRC(ca264c7c) SHA1(3a6adfaa935a1a11cb62e73b9f43b228b711c2da) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "pleiades.45",  0x0000, 0x0800, CRC(8dbd3785) SHA1(700cb9eb8ea64be99d843910cebcd29d601ab2e9) )
	ROM_LOAD( "pleiades.44",  0x0800, 0x0800, CRC(0db3e436) SHA1(cd1825775b0a10df66d2ccc01cb4b6a9a3d2141a) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "ic39.bin",     0x0000, 0x0800, CRC(85866607) SHA1(cd240bd056f761b2f9e2142049434f02cae3e315) )
	ROM_LOAD( "ic40.bin",     0x0800, 0x0800, CRC(a841d511) SHA1(8349008ab1d8ef08775b54170c37deb1d391fffc) )

	ROM_REGION( 0x800, "melody", 0 )  // Epson 7910E Multi-Melody IC (according to manual)
	ROM_LOAD( "7910e", 0x000, 0x800, NO_DUMP ) // actual size unknown, needs decapping

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "7611-5.33",    0x0000, 0x0100, CRC(e38eeb83) SHA1(252880d80425b2e697146e76efdc6cb9f3ba0378) )   // palette low bits
	ROM_LOAD( "7611-5.26",    0x0100, 0x0100, CRC(7a1bcb1e) SHA1(bdfab316ea26e2063879e7aa78b6ae2b55eb95c8) )   // palette high bits
ROM_END

// TIM-8001 + TIM-8002 PCBs. Sound section seems more similar to Phoenix (MM6221AA instead of TMS3615, etc)
ROM_START( pleiadsgmp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic45",   0x0000, 0x0800, CRC(6a6dcabf) SHA1(cec3669674feb1ea86248d181f83f60a6affe68d) )
	ROM_LOAD( "0.ic46", 0x0800, 0x0800, CRC(c82557b4) SHA1(d3ed8c5282bd75a65f8f6ecc096ab9668a8cdedd) )
	ROM_LOAD( "3.ic47", 0x1000, 0x0800, CRC(3b29aec5) SHA1(b90b55fdc799db672558e2f7c6b05a958bf33a2c) )
	ROM_LOAD( "4.ic48", 0x1800, 0x0800, CRC(10df5159) SHA1(73fc143c19d5ee5de8db5e6135ebeb7b43f1da05) )
	ROM_LOAD( "1.ic49", 0x2000, 0x0800, CRC(263576e7) SHA1(4c8f4ef413e96f3ce11e0eefdc41dbc11d8e5d20) )
	ROM_LOAD( "2.ic50", 0x2800, 0x0800, CRC(aa4d93ec) SHA1(1cd09a485c863ea8885a90f012c350310477fa0b) )
	ROM_LOAD( "ic51",   0x3000, 0x0800, CRC(b5f07fbc) SHA1(2ae687c84732942e69ad4dfb7a4ac1b97b77487a) )
	ROM_LOAD( "8.ic52", 0x3800, 0x0800, CRC(b3db08c2) SHA1(d5b1b77dcf2d76498f30d5f880635f5acfac7dfd) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "ic24.bin", 0x0000, 0x0800, CRC(5188fc29) SHA1(421dedc674c6dde7abf01412df035a8eb8e6db9b) )
	ROM_LOAD( "ic23.bin", 0x0800, 0x0800, CRC(4e30f9e7) SHA1(da023a94725dc40107cd97e4decfd4dc0f9f00ee) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "ic39.bin", 0x0000, 0x0800, CRC(85866607) SHA1(cd240bd056f761b2f9e2142049434f02cae3e315) )
	ROM_LOAD( "ic40.bin", 0x0800, 0x0800, CRC(a841d511) SHA1(8349008ab1d8ef08775b54170c37deb1d391fffc) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "cpu41.bin", 0x0000, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )
	ROM_LOAD( "cpu40.bin", 0x0100, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )
ROM_END

ROM_START( pleiadsi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1 2716.bin",    0x0000, 0x0800, CRC(9bbef607) SHA1(e563b72294fdbf4ff5bf748d0765af9e86e119bd) ) // unique to the Irecsa sets
	ROM_LOAD( "2 2716.bin",    0x0800, 0x0800, CRC(e2b5b8cd) SHA1(514ab2b24fc1d6d1fd64e74470b601ba9a11f36f) )
	ROM_LOAD( "3 2716.bin",    0x1000, 0x0800, CRC(87e700bb) SHA1(0f352b5461da957c564920fd1da83bc81f41ffb9) )
	ROM_LOAD( "4 2716.bin",    0x1800, 0x0800, CRC(ca14fe4a) SHA1(fbd60b8f98275193df6633b3589106c4e392fb62) ) // unique to this set
	ROM_LOAD( "5 2716.bin",    0x2000, 0x0800, CRC(9dc73e63) SHA1(8a2de6666fecead7071285125b16641b50249adc) )
	ROM_LOAD( "6 2716.bin",    0x2800, 0x0800, CRC(f1a8a00d) SHA1(5c183e3a73fa882ffec3cb9219fb5619e625591a) )
	ROM_LOAD( "7 2716.bin",    0x3000, 0x0800, CRC(6f56f317) SHA1(d7e6b0b1c58b741de3504640bcc23e86d1a134a0) )
	ROM_LOAD( "8 2716.bin",    0x3800, 0x0800, CRC(ca264c7c) SHA1(3a6adfaa935a1a11cb62e73b9f43b228b711c2da) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "11 2716.bin",   0x0000, 0x0800, CRC(8dbd3785) SHA1(700cb9eb8ea64be99d843910cebcd29d601ab2e9) )
	ROM_LOAD( "12 2716.bin",   0x0800, 0x0800, CRC(0db3e436) SHA1(cd1825775b0a10df66d2ccc01cb4b6a9a3d2141a) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "9 2716.bin",    0x0000, 0x0800, CRC(85866607) SHA1(cd240bd056f761b2f9e2142049434f02cae3e315) )
	ROM_LOAD( "10 2716.bin",   0x0800, 0x0800, CRC(a841d511) SHA1(8349008ab1d8ef08775b54170c37deb1d391fffc) )

	ROM_REGION( 0x0200, "proms", 0 ) // not present in dump, assuming to be the same, matches screenshots, note reverse order to most sets, same as pleiadsb2.
	ROM_LOAD( "7611-5.26",     0x0000, 0x0100, CRC(7a1bcb1e) SHA1(bdfab316ea26e2063879e7aa78b6ae2b55eb95c8) )   // palette low bits
	ROM_LOAD( "7611-5.33",     0x0100, 0x0100, CRC(e38eeb83) SHA1(252880d80425b2e697146e76efdc6cb9f3ba0378) )   // palette high bits
ROM_END

// This set shows an Irecsa patent number on the title screen.
ROM_START( pleiadsia )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.ic47",        0x0000, 0x0800, CRC(9bbef607) SHA1(e563b72294fdbf4ff5bf748d0765af9e86e119bd) )
	ROM_LOAD( "2.ic46",        0x0800, 0x0800, CRC(e2b5b8cd) SHA1(514ab2b24fc1d6d1fd64e74470b601ba9a11f36f) )
	ROM_LOAD( "3.ic49",        0x1000, 0x0800, CRC(87e700bb) SHA1(0f352b5461da957c564920fd1da83bc81f41ffb9) )
	ROM_LOAD( "4.ic50",        0x1800, 0x0800, CRC(3e0d3a6e) SHA1(dba3432b8060fed967bcc64025a5fa5a78f2edf3) )
	ROM_LOAD( "5.ic51",        0x2000, 0x0800, CRC(65a58c6d) SHA1(b8c2724099928c96674068a52ef26530f391ded5) )
	ROM_LOAD( "6.ic52",        0x2800, 0x0800, CRC(44ab59c1) SHA1(518a296558998729e44602267e525da2e1c84e39) )
	ROM_LOAD( "7.ic53",        0x3000, 0x0800, CRC(6f56f317) SHA1(d7e6b0b1c58b741de3504640bcc23e86d1a134a0) )
	ROM_LOAD( "8.ic54",        0x3800, 0x0800, CRC(ca264c7c) SHA1(3a6adfaa935a1a11cb62e73b9f43b228b711c2da) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "10.ic45",       0x0000, 0x0800, CRC(8dbd3785) SHA1(700cb9eb8ea64be99d843910cebcd29d601ab2e9) )
	ROM_LOAD( "9.ic44",        0x0800, 0x0800, CRC(0db3e436) SHA1(cd1825775b0a10df66d2ccc01cb4b6a9a3d2141a) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "r10.c26",       0x0000, 0x0800, CRC(85866607) SHA1(cd240bd056f761b2f9e2142049434f02cae3e315) )
	ROM_LOAD( "r9.ic27",       0x0800, 0x0800, CRC(a841d511) SHA1(8349008ab1d8ef08775b54170c37deb1d391fffc) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "a_hm7611.ic26", 0x0000, 0x0100, CRC(7a1bcb1e) SHA1(bdfab316ea26e2063879e7aa78b6ae2b55eb95c8) )   // palette low bits
	ROM_LOAD( "b_hm7611.ic33", 0x0100, 0x0100, CRC(304cf3f1) SHA1(2821577b792a9a9b0ddded4ba8046a2cef1e7d1f) )   // palette high bits
ROM_END

ROM_START( pleiadsn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",     0x0000, 0x0800, CRC(c013515f) SHA1(c44db1c615c11ace997c0065762020827bf9ef7e) )
	ROM_LOAD( "2.bin",     0x0800, 0x0800, CRC(b254217c) SHA1(312a33cca09d5d2d18992f28eb051230a90db6e3) )
	ROM_LOAD( "3.bin",     0x1000, 0x0800, CRC(3b29aec5) SHA1(b90b55fdc799db672558e2f7c6b05a958bf33a2c) )
	ROM_LOAD( "4.bin",     0x1800, 0x0800, CRC(1fbde4d7) SHA1(b358649288108159a426dba3940c627c2d2aeb01) )
	ROM_LOAD( "5.bin",     0x2000, 0x0800, BAD_DUMP CRC(9dc73e63) SHA1(8a2de6666fecead7071285125b16641b50249adc) )  // the best of 50 different dumps. the device is clearly damaged.
	ROM_LOAD( "6.bin",     0x2800, 0x0800, CRC(f1a8a00d) SHA1(5c183e3a73fa882ffec3cb9219fb5619e625591a) )
	ROM_LOAD( "7.bin",     0x3000, 0x0800, CRC(b5f07fbc) SHA1(2ae687c84732942e69ad4dfb7a4ac1b97b77487a) )
	ROM_LOAD( "8.bin",     0x3800, 0x0800, CRC(b3db08c2) SHA1(d5b1b77dcf2d76498f30d5f880635f5acfac7dfd) )

	ROM_REGION( 0x1000, "bgtiles", 0 ) // these are straight (colors match the real machine)
	ROM_LOAD( "11.bin",    0x0000, 0x0800, CRC(4e30f9e7) SHA1(da023a94725dc40107cd97e4decfd4dc0f9f00ee) )
	ROM_LOAD( "12.bin",    0x0800, 0x0800, CRC(72d511fc) SHA1(a12485698ad35ba3a8c72bb9401c0cf522ffc73c) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "9.bin",     0x0000, 0x0800, CRC(85866607) SHA1(cd240bd056f761b2f9e2142049434f02cae3e315) )
	ROM_LOAD( "10.bin",    0x0800, 0x0800, CRC(a841d511) SHA1(8349008ab1d8ef08775b54170c37deb1d391fffc) )

	ROM_REGION( 0x0200, "proms", 0 ) // proms borrowed from phoenix, reverse order.
	ROM_LOAD( "hm3-7611.bin", 0x0000, 0x0100, BAD_DUMP CRC(e38eeb83) SHA1(252880d80425b2e697146e76efdc6cb9f3ba0378) )   // palette low bits
	ROM_LOAD( "mb7052.ic41",  0x0100, 0x0100, BAD_DUMP CRC(7a1bcb1e) SHA1(bdfab316ea26e2063879e7aa78b6ae2b55eb95c8) )   // palette high bits
ROM_END

// Famaresa "580" PCB set (580-001 and 580-002).
ROM_START( pleiadss )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pl45.bin",     0x0000, 0x0800, CRC(e2528599) SHA1(4647c62a2c6047238ad2855cf71b9e079ac4b4c7) )
	ROM_LOAD( "pl46.bin",     0x0800, 0x0800, CRC(b254217c) SHA1(312a33cca09d5d2d18992f28eb051230a90db6e3) )
	ROM_LOAD( "pl47.bin",     0x1000, 0x0800, CRC(3b29aec5) SHA1(b90b55fdc799db672558e2f7c6b05a958bf33a2c) )
	ROM_LOAD( "pl48.bin",     0x1800, 0x0800, CRC(e74ccdeb) SHA1(1f82a98dc766e6d90b8fa76daa5403151f20d1fb) )
	ROM_LOAD( "pl49.bin",     0x2000, 0x0800, CRC(24f9c3e8) SHA1(2da3f65b21a274245222e617237e47438f79f592) )
	ROM_LOAD( "pl50.bin",     0x2800, 0x0800, CRC(f1a8a00d) SHA1(5c183e3a73fa882ffec3cb9219fb5619e625591a) )
	ROM_LOAD( "pl51.bin",     0x3000, 0x0800, CRC(b5f07fbc) SHA1(2ae687c84732942e69ad4dfb7a4ac1b97b77487a) )
	ROM_LOAD( "pl52.bin",     0x3800, 0x0800, CRC(b3db08c2) SHA1(d5b1b77dcf2d76498f30d5f880635f5acfac7dfd) )

	ROM_REGION( 0x1000, "bgtiles", 0 ) // these are swapped compared to other sets (colours match screenshots with this configuration)
	ROM_LOAD( "pl24.bin",     0x0000, 0x0800, CRC(5188fc29) SHA1(421dedc674c6dde7abf01412df035a8eb8e6db9b) )
	ROM_LOAD( "pl23.bin",     0x0800, 0x0800, CRC(4e30f9e7) SHA1(da023a94725dc40107cd97e4decfd4dc0f9f00ee) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "pl39.bin",     0x0000, 0x0800, CRC(71635678) SHA1(5c4f7aa57308352514f99d3613df75f7826a65df) ) // this differs very slightly from others, check how
	ROM_LOAD( "pl40.bin",     0x0800, 0x0800, CRC(a841d511) SHA1(8349008ab1d8ef08775b54170c37deb1d391fffc) )

	ROM_REGION( 0x0200, "proms", 0 ) // proms from phoenix, reverse order, like capitol (colours match screenshots with this configuration)
	ROM_LOAD( "ic41.prm",     0x0000, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )
	ROM_LOAD( "ic40.prm",     0x0100, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )
ROM_END

/* PCB silkscreened as "H-P6 NO.085318"
   On screen game title shows just "Attack", but the cabinet, manual and legal registry name (number A/20/00241) is "City Attack" */
ROM_START( cityatta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1-2716.bin",     0x0000, 0x0800, CRC(c013515f) SHA1(c44db1c615c11ace997c0065762020827bf9ef7e) )
	ROM_LOAD( "2-2716.bin",     0x0800, 0x0800, CRC(b254217c) SHA1(312a33cca09d5d2d18992f28eb051230a90db6e3) )
	ROM_LOAD( "3-2716.bin",     0x1000, 0x0800, CRC(87e700bb) SHA1(0f352b5461da957c564920fd1da83bc81f41ffb9) )
	ROM_LOAD( "4-2716.bin",     0x1800, 0x0800, CRC(bcda59ae) SHA1(04dd7139ef8b337b3e1dccddc52280eb6905b179) )
	ROM_LOAD( "5-2716.bin",     0x2000, 0x0800, CRC(49c629bc) SHA1(fd7937d0c114c8d9c1efaa9918ae3df2af41f032) )
	ROM_LOAD( "6-2716.bin",     0x2800, 0x0800, CRC(f1a8a00d) SHA1(5c183e3a73fa882ffec3cb9219fb5619e625591a) )
	ROM_LOAD( "7-2716.bin",     0x3000, 0x0800, CRC(b5f07fbc) SHA1(2ae687c84732942e69ad4dfb7a4ac1b97b77487a) )
	ROM_LOAD( "8-2716.bin",     0x3800, 0x0800, CRC(b1b5a8a6) SHA1(7e4ef298c8ddefc7dc0cbf94a9c9f36a4b807ba0) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "11-2716.bin",    0x0000, 0x0800, CRC(6694a80f) SHA1(754076f26d783eb6327b9c5c1e3a3f7814e22a76) )
	ROM_LOAD( "12-2716.bin",    0x0800, 0x0800, CRC(5188fc29) SHA1(421dedc674c6dde7abf01412df035a8eb8e6db9b) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "10-2716.bin",    0x0000, 0x0800, CRC(85866607) SHA1(cd240bd056f761b2f9e2142049434f02cae3e315) )
	ROM_LOAD( "9-2716.bin",     0x0800, 0x0800, CRC(a841d511) SHA1(8349008ab1d8ef08775b54170c37deb1d391fffc) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "ic41a-7611.bin", 0x0000, 0x0100, CRC(e38eeb83) SHA1(252880d80425b2e697146e76efdc6cb9f3ba0378) ) // palette low bits
	ROM_LOAD( "ic40a-7611.bin", 0x0100, 0x0100, CRC(7a1bcb1e) SHA1(bdfab316ea26e2063879e7aa78b6ae2b55eb95c8) ) // palette high bits
ROM_END

ROM_START( capitol )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cp1.45",       0x0000, 0x0800, CRC(0922905b) SHA1(501342b0162bba43570b1cbefa1ada6302a54017) )
	ROM_LOAD( "cp2.46",       0x0800, 0x0800, CRC(4f168f45) SHA1(8d268dad54a2cf7081f22a29a3e025174ae462e7) )
	ROM_LOAD( "cp3.47",       0x1000, 0x0800, CRC(3975e0b0) SHA1(d509398aa8b90c7c451d9e60bc1ca6488b1563a9) )
	ROM_LOAD( "cp4.48",       0x1800, 0x0800, CRC(da49caa8) SHA1(274c8dcabb9f43a2cbf9682b849ffd2bc8cf6496) )
	ROM_LOAD( "cp5.49",       0x2000, 0x0800, CRC(38e4362b) SHA1(c5aeb8b7d49b3da760904a16b57604e782cf29fc) )
	ROM_LOAD( "cp6.50",       0x2800, 0x0800, CRC(aaf798eb) SHA1(660774db4195aaa499569804a2304e969f168cdf) )
	ROM_LOAD( "cp7.51",       0x3000, 0x0800, CRC(eaadf14c) SHA1(753a46317e98b1ae63f88f5c3e70ff1c7ec04286) )
	ROM_LOAD( "cp8.52",       0x3800, 0x0800, CRC(d3fe2af4) SHA1(f0c9bfc17ba6f55fbe95136da40a3de775aa46d2) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "cp11.23",      0x0000, 0x0800, CRC(9b0bbb8d) SHA1(cde7c0140e773fe28e97e36486d4e048710f6004) )
	ROM_LOAD( "cp12.24",      0x0800, 0x0800, CRC(39949e66) SHA1(21a204f22f04c5808538b21d49ebc6b7cb7625e8) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "cp9.39",       0x0000, 0x0800, CRC(04f7d19a) SHA1(9b8e55ad0e374d4e7538e82c7b3f081b3bb98124) )
	ROM_LOAD( "cp10.40",      0x0800, 0x0800, CRC(4807408f) SHA1(4aa81e934a65e9986b194e9a9bab99f6c85ff7a5) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "ic41.prm",     0x0000, 0x0100, CRC(e176b768) SHA1(e2184dd495ed579f10b6da0b78379e02d7a6229f) )
	ROM_LOAD( "ic40.prm",     0x0100, 0x0100, CRC(79350b25) SHA1(57411be4c1d89677f7919ae295446da90612c8a8) )
ROM_END

ROM_START( survival )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g959-32a_45.u45", 0x0000, 0x0800, CRC(0bc53541) SHA1(0d1bcf226b89d0cfe0864aab8126b276273a23c2) )
	ROM_LOAD( "g959-33a_46.u46", 0x0800, 0x0800, CRC(726e9428) SHA1(515c6278ece9bf39827b9c886a1a900e274bd272) )
	ROM_LOAD( "g959-34a_47.u47", 0x1000, 0x0800, CRC(78f166ff) SHA1(ad079372067319dcfad4a00e437d34a999ff9161) )
	ROM_LOAD( "g959-35a_48.u48", 0x1800, 0x0800, CRC(59dbe099) SHA1(b7a60302510c72d61b397f7d0f615cfe1762ef5f) )
	ROM_LOAD( "g959-36a_49.u49", 0x2000, 0x0800, CRC(bd5e586e) SHA1(8e53e5de7bf73cb56fbd6c18017f09cbbbb17769) )
	ROM_LOAD( "g959-37a_50.u50", 0x2800, 0x0800, CRC(b2de1094) SHA1(c8b35b9dd57bd50f837087f10c50f2af04cea823) )
	ROM_LOAD( "g959-38a_51.u51", 0x3000, 0x0800, CRC(131c4440) SHA1(c2610d8fbf63110767037c384b6776cfe981da4c) )
	ROM_LOAD( "g959-39a_52.u52", 0x3800, 0x0800, CRC(213bc910) SHA1(a03b3ca8185e929898cc32ea2d5e944c4c897d0d) )

	ROM_REGION( 0x1000, "bgtiles", 0 )
	ROM_LOAD( "g959-42-23.u23",  0x0000, 0x0800, CRC(3d1ce38d) SHA1(48b94027467ba360c08a7f56bb75474e6859381f) )
	ROM_LOAD( "g959-43-24.u24",  0x0800, 0x0800, CRC(cd150da9) SHA1(642264c0eed34ae2b9f1156d261786361f296d1a) )

	ROM_REGION( 0x1000, "fgtiles", 0 )
	ROM_LOAD( "g959-40-39.u39",  0x0000, 0x0800, CRC(41dee996) SHA1(55a792504b2ffc1236eb3f427dee38893e1ca5b7) )
	ROM_LOAD( "g959-41-40.u40",  0x0800, 0x0800, CRC(a255d6dc) SHA1(1b2f635f4392d0df1cbd527dcf6cf662b2a1014e) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s129n.u40",  0x0000, 0x0100, CRC(b3e20669) SHA1(4f01c5d74fc8abe748dd88e4513edf52b977ee32) )   // palette low bits
	ROM_LOAD( "n82s129n.u41",  0x0100, 0x0100, CRC(abddf69a) SHA1(e22c380a94fb491bec95c4f4c2d4f072839c09cf) )   // palette high bits
ROM_END


void phoenix_state::init_coindsw()
{
	// additional inputs for coinage
	m_maincpu->space(AS_PROGRAM).install_read_port(0x5000, 0x5000, "DSW1");
}

void phoenix_state::init_oneprom()
{
	uint8_t *rgn = memregion("proms")->base();

	// expand the 8-bit PROM into the same layout as the 4-bit PROMs used by most versions of the game
	for (int i = 0; i < 0x100; i++)
	{
		rgn[i] = (rgn[i + 0x100] & 0xf0) >> 4;
		rgn[i + 0x100] &= 0x0f;
	}
}

void phoenix_state::init_oneprom_coindsw()
{
	init_coindsw();

	init_oneprom();
}


  /*** Phoenix (& clones) ***/
GAME( 1980, phoenix,    0,       phoenix,  phoenix,  phoenix_state, empty_init,           ROT90, "Amstar",                                 "Phoenix (Amstar, set 1)",                                    MACHINE_SUPPORTS_SAVE )
GAME( 1980, phoenix2,   phoenix, phoenix,  phoenix,  phoenix_state, empty_init,           ROT90, "Amstar",                                 "Phoenix (Amstar, set 2)",                                    MACHINE_SUPPORTS_SAVE )
GAME( 1980, phoenixa,   phoenix, phoenix,  phoenixa, phoenix_state, empty_init,           ROT90, "Amstar (Centuri license)",               "Phoenix (Centuri, set 1)",                                   MACHINE_SUPPORTS_SAVE )
GAME( 1980, phoenixb,   phoenix, phoenix,  phoenixa, phoenix_state, empty_init,           ROT90, "Amstar (Centuri license)",               "Phoenix (Centuri, set 2)",                                   MACHINE_SUPPORTS_SAVE )
GAME( 1980, phoenixt,   phoenix, phoenix,  phoenixt, phoenix_state, empty_init,           ROT90, "Amstar (Taito license)",                 "Phoenix (Taito)",                                            MACHINE_SUPPORTS_SAVE )
GAME( 1980, phoenixj,   phoenix, phoenix,  phoenixt, phoenix_state, empty_init,           ROT90, "Amstar (Taito Japan license)",           "Phoenix (Taito Japan)",                                      MACHINE_SUPPORTS_SAVE )
GAME( 1980, phoenix3,   phoenix, phoenix,  phoenix3, phoenix_state, empty_init,           ROT90, "bootleg (T.P.N.)",                       "Phoenix (T.P.N. bootleg)",                                   MACHINE_SUPPORTS_SAVE )
GAME( 1980, phoenixdal, phoenix, phoenix,  phoenixt, phoenix_state, empty_init,           ROT90, "bootleg (D&L)",                          "Phoenix (D&L bootleg)",                                      MACHINE_SUPPORTS_SAVE )
GAME( 1981, phoenixc,   phoenix, phoenix,  phoenixt, phoenix_state, empty_init,           ROT90, "bootleg? (Irecsa / G.G.I. Corporation)", "Phoenix (Irecsa / G.G.I. Corporation)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1981, phoenixc2,  phoenix, phoenix,  phoenixt, phoenix_state, empty_init,           ROT90, "bootleg (G.G.I. Corporation)",           "Phoenix (G.G.I. Corporation, set 1)",                        MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, phoenixc3,  phoenix, phoenix,  phoenixt, phoenix_state, empty_init,           ROT90, "bootleg (G.G.I. Corporation)",           "Phoenix (G.G.I. Corporation, set 2)",                        MACHINE_SUPPORTS_SAVE )
GAME( 1981, phoenixc4,  phoenix, phoenix,  phoenixt, phoenix_state, empty_init,           ROT90, "bootleg (G.G.I. Corporation)",           "Phoenix (G.G.I. Corporation, set 3)",                        MACHINE_SUPPORTS_SAVE )
GAME( 1981, phoenixgu,  phoenix, phoenix,  phoenixt, phoenix_state, empty_init,           ROT90, "bootleg? (G. Universal Video)",          "Phoenix (G. Universal Video bootleg)",                       MACHINE_SUPPORTS_SAVE )
GAME( 1981, phoenixi,   phoenix, condor,   condor,   phoenix_state, init_oneprom_coindsw, ROT90, "bootleg (IDI)",                          "Phoenix (IDI bootleg)",                                      MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE ) // Needs correct color PROM decode
GAME( 1980, fenixn,     phoenix, phoenix,  phoenix,  phoenix_state, empty_init,           ROT90, "bootleg (Niemer)",                       "Fenix (Niemer bootleg of Phoenix)",                          MACHINE_SUPPORTS_SAVE )
GAME( 1981, condor,     phoenix, condor,   condor,   phoenix_state, init_coindsw,         ROT90, "bootleg (Sidam)",                        "Condor (Sidam bootleg of Phoenix)",                          MACHINE_SUPPORTS_SAVE )
GAME( 1981, condorn,    phoenix, condor,   condor,   phoenix_state, init_oneprom_coindsw, ROT90, "bootleg (S C Novar)",                    "Condor (S C Novar bootleg of Phoenix)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1980, condorva,   phoenix, condor,   phoenix,  phoenix_state, init_oneprom,         ROT90, "bootleg (Valadon Automation)",           "Condor (Valadon Automation bootleg of Phoenix)",             MACHINE_SUPPORTS_SAVE )
// the following 2 were common bootlegs in England & France respectively
GAME( 1980, falcon,     phoenix, phoenix,  phoenixt, phoenix_state, empty_init,           ROT90, "bootleg",                                "Falcon (bootleg of Phoenix) (8085A CPU)",                    MACHINE_SUPPORTS_SAVE )
GAME( 1980, vautour,    phoenix, phoenix,  phoenixt, phoenix_state, empty_init,           ROT90, "bootleg (Jeutel)",                       "Vautour (bootleg of Phoenix) (8085A CPU)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1980, falconz,    phoenix, condor,   falconz,  phoenix_state, empty_init,           ROT90, "bootleg (Digimatic)",                    "Falcon (bootleg of Phoenix) (Z80 CPU)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1980, vautourz,   phoenix, condor,   condor,   phoenix_state, init_coindsw,         ROT90, "bootleg",                                "Vautour (bootleg of Phoenix) (Z80 CPU)",                     MACHINE_SUPPORTS_SAVE )
GAME( 1980, vautourza,  phoenix, condor ,  phoenixt, phoenix_state, init_oneprom,         ROT90, "bootleg (Jeutel)",                       "Vautour (bootleg of Phoenix) (Z80 CPU, single PROM)",        MACHINE_SUPPORTS_SAVE )

// German bootlegs
GAME( 1981, phoenixha,  phoenix, phoenix,  phoenixt, phoenix_state, empty_init,           ROT90, "bootleg (Hellomat Automaten)",           "Phoenix (Hellomat Automaten bootleg)",                       MACHINE_SUPPORTS_SAVE )

// fenix is an Italian bootleg based on vautourz (also an unrelated Spanish bootleg from Niemer)
GAME( 1980, fenix,      phoenix, condor,   condor,   phoenix_state, init_oneprom_coindsw, ROT90, "bootleg (Orio)",                         "Fenix (Orio bootleg of Phoenix)",                            MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1980, griffon,    phoenix, condor,   condor,   phoenix_state, init_oneprom_coindsw, ROT90, "bootleg (Videotron)",                    "Griffon (Videotron bootleg of Phoenix)",                     MACHINE_SUPPORTS_SAVE )
GAME( 1980, griffono,   phoenix, condor,   condor,   phoenix_state, init_oneprom_coindsw, ROT90, "bootleg (Olympia)",                      "Griffon (Olympia bootleg of Phoenix)",                       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
// nextfase is a Spanish bootleg
GAME( 1981, nextfase,   phoenix, phoenix,  nextfase, phoenix_state, empty_init,           ROT90, "bootleg (Petaco S.A.)",                  "Next Fase (bootleg of Phoenix)",                             MACHINE_SUPPORTS_SAVE )
  // as is this
GAME( 1981, phoenixs,   phoenix, phoenix,  phoenix,  phoenix_state, empty_init,           ROT90, "bootleg (Sonic)",                        "Phoenix (Sonic, Spanish bootleg)",                           MACHINE_SUPPORTS_SAVE )
GAME( 1981, phoenixbl,  phoenix, phoenix,  phoenix,  phoenix_state, empty_init,           ROT90, "bootleg",                                "Phoenix (bootleg)",                                          MACHINE_SUPPORTS_SAVE )
GAME( 1981, phoenixass, phoenix, phoenix,  phoenix,  phoenix_state, empty_init,           ROT90, "bootleg (Assa)",                         "Phoenix (Assa, Spanish bootleg)",                            MACHINE_SUPPORTS_SAVE )
GAME( 1980, avefenix,   phoenix, phoenix,  phoenix,  phoenix_state, empty_init,           ROT90, "bootleg (Video Game)",                   "Ave Fenix (Electrogame, Spanish bootleg of Phoenix)",        MACHINE_SUPPORTS_SAVE ) // Electrogame (Barcelona) made the dedicated cabinet. Ingame shows 'Video Game', as Electrogame changed its name to Video Game S.A.
GAME( 1980, avefenixrf, phoenix, phoenix,  phoenix,  phoenix_state, empty_init,           ROT90, "bootleg (Recreativos Franco S.A.)",      "Ave Fenix (Recreativos Franco, Spanish bootleg of Phoenix)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, avefenixl,  phoenix, phoenix,  phoenix,  phoenix_state, empty_init,           ROT90, "bootleg (Laguna)",                       "Ave Fenix (Laguna, Spanish bootleg of Phoenix)",             MACHINE_SUPPORTS_SAVE )

  /*** Pleiads (& clones) ***/
GAME( 1981, pleiads,    0,       pleiads,  pleiads,  phoenix_state, empty_init,           ROT90, "Tehkan",                                 "Pleiads (Tehkan)",                                           MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1981, pleiadsb2,  pleiads, pleiads,  pleiads,  phoenix_state, empty_init,           ROT90, "bootleg (ESG)",                          "Pleiads (bootleg set 2)",                                    MACHINE_SUPPORTS_SAVE )
GAME( 1981, pleiadbl,   pleiads, pleiads,  pleiadbl, phoenix_state, empty_init,           ROT90, "bootleg",                                "Pleiads (bootleg set 1)",                                    MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1981, pleiadce,   pleiads, pleiads,  pleiadce, phoenix_state, empty_init,           ROT90, "Tehkan (Centuri license)",               "Pleiads (Centuri)",                                          MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1981, pleiadsgmp, pleiads, phoenix,  pleiadce, phoenix_state, empty_init,           ROT90, "bootleg (GMP Games)",                    "Pleiads (GMP Games)",                                        MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1981, pleiadsi,   pleiads, pleiads,  pleiadce, phoenix_state, empty_init,           ROT90, "bootleg (Irecsa)",                       "Pleiads (Irecsa, set 1)",                                    MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1981, pleiadsia,  pleiads, pleiads,  pleiadce, phoenix_state, empty_init,           ROT90, "bootleg (Irecsa)",                       "Pleiads (Irecsa, set 2)",                                    MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1981, pleiadsn,   pleiads, phoenix,  pleiadce, phoenix_state, empty_init,           ROT90, "bootleg (Niemer S.A.)",                  "Pleiads (Niemer S.A.)",                                      MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1981, pleiadss,   pleiads, phoenix,  pleiadce, phoenix_state, empty_init,           ROT90, "bootleg (Famaresa)",                     "Pleiads (Famaresa, Spanish bootleg)",                        MACHINE_SUPPORTS_SAVE ) // colours match PCB (but are ugly)
GAME( 1981, cityatta,   pleiads, pleiads,  cityatta, phoenix_state, empty_init,           ROT90, "bootleg (Petaco S.A.)",                  "City Attack (Petaco S.A., bootleg of Pleiads)",              MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE ) // Colors are bad, as seen on the screenshot from https://www.recreativas.org/city-attack-454-petaco
GAME( 1981, capitol,    pleiads, phoenix,  capitol,  phoenix_state, empty_init,           ROT90, "bootleg? (Universal Video Spiel)",       "Capitol",                                                    MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )

  /*** Others ***/
GAME( 1982, survival,   0,       survival, survival, phoenix_state, empty_init,           ROT90, "Rock-Ola",                               "Survival",                                                   MACHINE_SUPPORTS_SAVE ) // colors match PCB
