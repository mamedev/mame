// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                 -= Paradise / Target Ball / Torus =-

                 driver by   Luca Elia (l.elia@tin.it)


CPU          :  Z8400B
Video Chips  :  TPC1024AFN-084C
Sound Chips  :  2 x AR17961 (OKI M6295) (only 1 in Torus)

---------------------------------------------------------------------------
Year + Game          Board#
---------------------------------------------------------------------------
94  Paradise         YS-1600
94  Paradise Deluxe  YS-1604
95  Target Ball      YS-2002
96  Target Ball '96  YS-2002
96  Penky            YS951004
96  Torus            YS-0402? Looks identical
98  Mad Ball         YS-0402
---------------------------------------------------------------------------

Notes:

paradise: I'm not sure it's working correctly:

- The high scores table can't be entered !?


penky: we need to delay the irqs at startup or it won't boot. Either one of
       ports 0x2003.r or 0x2005.w starts up the irq timer (confirmed via trojan)

Alternate dipswitch settings for Penky as found in scanned Pins & Dip manual:

DIPSW-A
--------------------------------------------------------------------
    DipSwitch Title   |  Function  | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
--------------------------------------------------------------------
                      |   70 Sec   |off|off|                       |*
      Game Time       |   60 Sec   |on |off|                       |
                      |   50 Sec   |off|on |                       |
                      |   40 Sec   |on |on |                       |
--------------------------------------------------------------------
     Strip-Tease      |     On     |       |off|                   |*
                      |     Off    |       |on |                   |
--------------------------------------------------------------------
                      |    Easy    |           |off|off|           |
      Difficulty      |   Normal   |           |on |off|           |*
                      |    Hard    |           |off|on |           |
                      | Very Hard  |           |on |on |           |
--------------------------------------------------------------------
                      |    99%     |                   |off|off|   |*
 Minimum Percentage to|    90%     |                   |on |off|   |
 Complete for Win or  |    80%     |                   |off|on |   |
majority @ end of time|    70%     |                   |on |on |   |
--------------------------------------------------------------------
      Not Used                                                 |off|*
--------------------------------------------------------------------

DIPSW-B
--------------------------------------------------------------------
    DipSwitch Title   |  Function  | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
--------------------------------------------------------------------
                      | 1cn / 1pl  |off|off|                       |*
        Coinage       | 1cn / 2pl  |on |off|                       |
                      | 1cn / 3pl  |off|on |                       |
                      | 2cn / 3pl  |on |on |                       |
--------------------------------------------------------------------
      Not Used                             |off|off|off|           |*
--------------------------------------------------------------------
  Competition Mode    | 1Bout 1Win |                   |off|       |*
                      |3Bouts 2Wins|                   |on |       |
--------------------------------------------------------------------
    Demo Sounds       |    Yes     |                       |off|   |*
                      |     No     |                       |on |   |
--------------------------------------------------------------------
       TV Test        |    Game    |                           |off|*
    (Slide Show)      |    Test    |                           |on |
--------------------------------------------------------------------

2015/06/03 - System11
Amended above comments - bonus round is triggered by percentage when you
beat a stage.  Added 2 clones.  Amended date to 1994 based on Distributor
sticker from Jan 95 and factory sticker 94*41.

***************************************************************************/

#include "emu.h"
#include "includes/paradise.h"

#include "cpu/z80/z80.h"
#include "speaker.h"


/***************************************************************************

                                Memory Maps

***************************************************************************/

WRITE8_MEMBER(paradise_state::rombank_w)
{
	int bank = data;
	int bank_n = memregion("maincpu")->bytes() / 0x4000;

	if (bank >= bank_n)
	{
		logerror("PC %04X - invalid rom bank %x\n", m_maincpu->pc(), bank);
		bank %= bank_n;
	}

	membank("prgbank")->set_entry(bank);
}

WRITE8_MEMBER(paradise_state::paradise_okibank_w)
{
	if (data & ~0x02)
		logerror("%s: unknown oki bank bits %02X\n", machine().describe_context(), data);

	m_oki2->set_rom_bank((data & 0x02) >> 1);
}

WRITE8_MEMBER(paradise_state::torus_coin_counter_w)
{
	machine().bookkeeping().coin_counter_w(0, data ^ 0xff);
}

void paradise_state::base_map(address_map &map)
{
	map(0x0000, 0x7fff).rom(); /* ROM */
	map(0x8000, 0xbfff).bankr("prgbank");    /* ROM (banked) */
	map(0xc000, 0xc7ff).ram().w(FUNC(paradise_state::vram_2_w)).share("vram_2"); /* Background */
	map(0xc800, 0xcfff).ram().w(FUNC(paradise_state::vram_1_w)).share("vram_1"); /* Midground */
	map(0xd000, 0xd7ff).ram().w(FUNC(paradise_state::vram_0_w)).share("vram_0"); /* Foreground */
}

void paradise_state::paradise_map(address_map &map)
{
	base_map(map);
	map(0xd800, 0xd8ff).ram(); // RAM
	map(0xd900, 0xe0ff).ram().share("spriteram");   // Sprites
	map(0xe100, 0xffff).ram(); // RAM
}

void paradise_state::tgtball_map(address_map &map)
{
	base_map(map);
	map(0xd800, 0xd8ff).ram(); // RAM
	map(0xd900, 0xd9ff).ram().share("spriteram");   // Sprites
	map(0xda00, 0xffff).ram(); // RAM
}

void paradise_state::torus_map(address_map &map)
{
	base_map(map);
	map(0xd800, 0xdfff).ram().share("spriteram");   // Sprites
	map(0xe000, 0xffff).ram(); // RAM
}

void paradise_state::torus_io_map(address_map &map)
{
	map(0x0000, 0x17ff).ram().w(FUNC(paradise_state::palette_w)).share("paletteram");    // Palette
	map(0x1800, 0x1800).w(FUNC(paradise_state::priority_w));  // Layers priority
	map(0x2001, 0x2001).w(FUNC(paradise_state::flipscreen_w));    // Flip Screen
	map(0x2004, 0x2004).w(FUNC(paradise_state::palbank_w));   // Layers palette bank
	map(0x2006, 0x2006).w(FUNC(paradise_state::rombank_w));   // ROM bank
	map(0x2010, 0x2010).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));  // OKI 0
	map(0x2020, 0x2020).portr("DSW1");
	map(0x2021, 0x2021).portr("DSW2");
	map(0x2022, 0x2022).portr("P1");
	map(0x2023, 0x2023).portr("P2");
	map(0x2024, 0x2024).portr("SYSTEM");
	map(0x8000, 0xffff).ram().w(FUNC(paradise_state::pixmap_w)).share("videoram");   // Pixmap
}

void paradise_state::paradise_io_map(address_map &map)
{
	torus_io_map(map);
	map(0x2007, 0x2007).w(FUNC(paradise_state::paradise_okibank_w));   // OKI 1 samples bank
	map(0x2030, 0x2030).rw(m_oki2, FUNC(okim6295_device::read), FUNC(okim6295_device::write));  // OKI 1
}


/***************************************************************************

                                Input Ports

***************************************************************************/

/***************************************************************************
                                Paradise
***************************************************************************/

static INPUT_PORTS_START( paradise )
	PORT_START("DSW1")  /* port $2020 */
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x08, "Fill Area" )         PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "75%" )
	PORT_DIPSETTING(    0x08, "80%" )
	PORT_DIPSETTING(    0x04, "85%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x60, 0x20, "Time" )          PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x00, "45" )            /* Listed as  90 Secs */
	PORT_DIPSETTING(    0x20, "60" )            /* Listed as 120 Secs */
	PORT_DIPSETTING(    0x40, "75" )            /* Listed as 150 Secs */
	PORT_DIPSETTING(    0x60, "90" )            /* Listed as 180 Secs */
	PORT_DIPNAME( 0x80, 0x80, "Sound Test" )        PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  /* port $2021 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Slide Show" )        PORT_DIPLOCATION("SW2:8") /* Player1 button used to advance one time through ALL backgrounds */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P1")    /* port $2022 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    // alias for button1?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    // alias for button1?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")    /* port $2023 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    // alias for button1?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    // alias for button1?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")    /* port $2024 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( para2dx )
	PORT_INCLUDE(paradise)

	PORT_MODIFY("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" ) /* No Slide Show */
INPUT_PORTS_END


static INPUT_PORTS_START( tgtball )
	PORT_START("DSW1")  /* port $2020 */
	PORT_DIPNAME( 0x03, 0x02, "Time for 1 Player" )     PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "1:00" )
	PORT_DIPSETTING(    0x02, "1:20" )
	PORT_DIPSETTING(    0x01, "1:40" )
	PORT_DIPSETTING(    0x00, "2:00" )
	PORT_DIPNAME( 0x0c, 0x08, "Bonus Time?" )       PORT_DIPLOCATION("SW1:3,4") /* Difficulty or Bonus (time)? */
	PORT_DIPSETTING(    0x0c, "15" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "25" )
	PORT_DIPSETTING(    0x00, "30" )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x40, 0x40, "Balls Sequence Length" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x80, 0x80, "Game Goal" )         PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "Target Score" )
	PORT_DIPSETTING(    0x00, "Balls Sequence" )

	PORT_START("DSW2")  /* port $2021 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x08, "Time for 2 Players"  )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "2:00" )
	PORT_DIPSETTING(    0x08, "2:40" )
	PORT_DIPSETTING(    0x04, "3:20" )
	PORT_DIPSETTING(    0x00, "4:00" )
	PORT_DIPNAME( 0x10, 0x10, "Vs. Matches" )       PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Slide Show" )        PORT_DIPLOCATION("SW2:8") /* Player1 button used to advance one time through ALL backgrounds */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P1")    /* port $2022 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")    /* port $2023 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")    /* port $2024 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( penky )
	PORT_START("DSW1")  /* port $2020 */
	PORT_DIPNAME( 0x03, 0x03, "Time" )          PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "0:40" )
	PORT_DIPSETTING(    0x01, "0:50" )
	PORT_DIPSETTING(    0x02, "1:00" )
	PORT_DIPSETTING(    0x03, "1:10" )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x30, 0x30, "Fill % to Win" )     PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "Majority at Time or 99.9%" )
	PORT_DIPSETTING(    0x20, "Majority at Time or 90%" )
	PORT_DIPSETTING(    0x10, "Majority at Time or 85%" )
	PORT_DIPSETTING(    0x00, "Majority at Time or 80%" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:7") /* One of these likely disables the nude pics */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:8") /* One of these likely disables the nude pics */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  /* port $2021 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x10, 0x10, "Vs. Matches" )       PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Slide Show" )        PORT_DIPLOCATION("SW2:8") /* Player1 button used to advance one time through the backgrounds */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P1")    /* port $2022 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    // alias for button1?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    // alias for button1?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")    /* port $2023 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    // alias for button1?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    // alias for button1?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")    /* port $2024 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( penkyi )
	PORT_INCLUDE(penky)

	PORT_MODIFY("DSW1")  /* port $2020 */
	PORT_DIPNAME( 0x03, 0x03, "Time" )          PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "0:30" )
	PORT_DIPSETTING(    0x01, "0:40" )
	PORT_DIPSETTING(    0x02, "0:50" )
	PORT_DIPSETTING(    0x03, "1:00" )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode" )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "Normal" )
	PORT_DIPSETTING(    0x00, "Redemption" ) // gives out tickets

	PORT_MODIFY("DSW2")  /* port $2021 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Points/Tickets" )      PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "50.000 points/1 ticket" )
	PORT_DIPSETTING(    0x08, "100.000 points/1 ticket" )
	PORT_DIPSETTING(    0x04, "150.000 points/1 ticket" )
	PORT_DIPSETTING(    0x00, "200.000 points/1 ticket" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:6") // Demo sounds in the parent
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( torus )
	PORT_START("DSW1")  /* port $2020 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Dropping Speed" )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, "Fast" )
	PORT_DIPSETTING(    0x04, "Faster" )
	PORT_DIPSETTING(    0x00, "Fastest" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  /* port $2021 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Slide Show" )    /* Player1 Button to pull the blinds down in sections, continuous loop */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P1")    /* port $2022 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")    /* port $2023 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")    /* port $2024 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( madball )
	PORT_START("DSW1")  /* port $2020 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Controls ) ) PORT_DIPLOCATION("SW1:6") /* Spinner controls currently NOT hooked up */
	PORT_DIPSETTING(    0x20, "Spinner" )
	PORT_DIPSETTING(    0x00, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Slide Show" ) PORT_DIPLOCATION("SW1:8") /* Use P1 button to advance */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  /* port $2021 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P1")    /* port $2022 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")    /* port $2023 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")    /* port $2024 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************

                                Graphics Layouts

***************************************************************************/

static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP8(0,4) },
	{ STEP8(0,4*8) },
	8*8*4
};

static const gfx_layout layout_8x8x8 =
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{ STEP4(RGN_FRAC(1,2),1), STEP4(RGN_FRAC(0,2),1) },
	{ STEP8(0,4) },
	{ STEP8(0,4*8) },
	8*8*4
};

static const gfx_layout layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,2),
	8,
	{ STEP4(RGN_FRAC(1,2),1), STEP4(RGN_FRAC(0,2),1) },
	{ STEP8(8*8*4*0,4), STEP8(8*8*4*1,4) },
	{ STEP8(8*8*4*0,4*8), STEP8(8*8*4*2,4*8) },
	16*16*4
};

static const gfx_layout torus_layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,2),
	8,
	{ STEP4(RGN_FRAC(1,2),1), STEP4(RGN_FRAC(0,2),1) },
	{ STEP8(0,4),STEP8(4*8,4) },
	{ STEP16(0,8*8) },
	128*8
};

static GFXDECODE_START( gfx_paradise )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x8, 0x100, 1  ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_8x8x4,   0x400, 16 ) // [1] Background
	GFXDECODE_ENTRY( "gfx3", 0, layout_8x8x8,   0x300, 1  ) // [2] Midground
	GFXDECODE_ENTRY( "gfx4", 0, layout_8x8x8,   0x000, 1  ) // [3] Foreground
GFXDECODE_END

static GFXDECODE_START( gfx_torus )
	GFXDECODE_ENTRY( "gfx1", 0, torus_layout_16x16x8, 0x100, 1  ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_8x8x4,        0x400, 16 ) // [1] Background
	GFXDECODE_ENTRY( "gfx3", 0, layout_8x8x8,        0x300, 1  ) // [2] Midground
	GFXDECODE_ENTRY( "gfx4", 0, layout_8x8x8,        0x000, 1  ) // [3] Foreground
GFXDECODE_END

static GFXDECODE_START( gfx_madball )
	GFXDECODE_ENTRY( "gfx1", 0, torus_layout_16x16x8, 0x500, 1  ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_8x8x4,        0x400, 16 ) // [1] Background
	GFXDECODE_ENTRY( "gfx3", 0, layout_8x8x8,        0x300, 1  ) // [2] Midground
	GFXDECODE_ENTRY( "gfx4", 0, layout_8x8x8,        0x000, 1  ) // [3] Foreground
GFXDECODE_END

/***************************************************************************

                                Machine Drivers

***************************************************************************/

void paradise_state::machine_start()
{
	int bank_n = memregion("maincpu")->bytes() / 0x4000;

	membank("prgbank")->configure_entries(0, bank_n, memregion("maincpu")->base(), 0x4000);

	save_item(NAME(m_palbank));
	save_item(NAME(m_priority));
	save_item(NAME(m_irq_count));
}

void paradise_state::machine_reset()
{
	m_palbank = 0;
	m_priority = 0;
	m_irq_count = 0;
}

INTERRUPT_GEN_MEMBER(paradise_state::irq)
{
	if (m_irq_count<300)
		m_irq_count++;
	else
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);
}

void paradise_state::paradise(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(12'000'000)/2);          /* Z8400B - 6mhz Verified */
	m_maincpu->set_addrmap(AS_PROGRAM, &paradise_state::paradise_map);
	m_maincpu->set_addrmap(AS_IO, &paradise_state::paradise_io_map);
	m_maincpu->set_periodic_int(FUNC(paradise_state::irq), attotime::from_hz(4*54));    /* No nmi routine, timing is confirmed (i.e. three timing irqs for each vblank irq */


	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(54); /* 54 verified */
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */    /* we're using PORT_VBLANK */);
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0+16, 256-1-16);
	m_screen->set_screen_update(FUNC(paradise_state::screen_update_paradise));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_paradise);
	PALETTE(config, m_palette).set_entries(0x800 + 16);


	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki1", XTAL(12'000'000)/12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.50);    /* verified on pcb */

	OKIM6295(config, m_oki2, XTAL(12'000'000)/12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.50); /* verified on pcb */
}

void paradise_state::tgtball(machine_config &config)
{
	paradise(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &paradise_state::tgtball_map);
}

void paradise_state::torus(machine_config &config)
{
	paradise(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &paradise_state::torus_map);
	m_maincpu->set_addrmap(AS_IO, &paradise_state::torus_io_map);

	m_gfxdecode->set_info(gfx_torus);
	m_screen->set_screen_update(FUNC(paradise_state::screen_update_torus));

	config.device_remove("oki2");
}

void paradise_state::madball(machine_config &config)
{
	torus(config);

	m_gfxdecode->set_info(gfx_madball);

	m_screen->set_screen_update(FUNC(paradise_state::screen_update_madball));
}

void paradise_state::penky(machine_config &config)
{
	paradise(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &paradise_state::torus_map);
	m_maincpu->set_addrmap(AS_IO, &paradise_state::torus_io_map);
}


void paradise_state::penkyi(machine_config &config)
{
	penky(config);

	// TODO add ticket dispenser

	config.device_remove("oki2");
}

/***************************************************************************

                                ROMs Loading

***************************************************************************/

/***************************************************************************

                          Paradise / Paradise Deluxe

(c) Yun Sung  year unknown

Another porn qix like game

YS-1600 / YS-1604
  CPU: Z8400B PS (Z80 6Mhz)
Sound: 2 x AR17961 or AD-65 (OKI M6295)
Video: TPC1020AFN-084C
  OSC: 12.000MHz & 4.000MHz

YS-1604
|---------------------------------------------------------|
|  AD-65  AD-65   Z80              4MHz       YUNSUNG.110 |
| YUNSUNG.113   YUNSUNG.128                   YUNSUNG.111 |
| YUNSUNG.85     6264                                     |
|                                             YUNSUNG.92  |
|                                             YUNSUNG.93  |
|                                             YUNSUNG.94  |
|                6116                                     |
|                6116              6116                   |
|J               6116                                     |
|A                                                        |
|M                                            6116        |
|M                                            6116        |
|A                            |-------|       6116        |
|                             | TI    |                   |
|                             |TPC1020|                   |
|DSW1(8)                      | 084C  |                   |
|         12MHz               |-------|                   |
|                           4464                          |
|                           4464     YUNSUNG.114          |
|                           4464     YUNSUNG.115          |
|DSW2(8)                    4464                          |
|                                                         |
|---------------------------------------------------------|

Year is assumed to be 1994 based on 94*41 factory sticker and 19/1/95
operator sticker on a revised version PCB.  The year is not shown but
must be >= 1994, since the development system (cross compiler?) they
used left a "1994.8-1989" in the rom

Paradise (set 2): No board ID, although some tracks which are wire hacks on
 YS-1600 PCB are corrected on this version. All roms are labeled as "A-19"
 Except for the program rom all other data matches Paradise (set 1).
 NOTE: on this version of the PCB the above listed 4MHz OSC is replaced with
       a 3.579545MHz OSC

The Escape version might be the newest revision due to improved slot machine
 animation on 'girl select' compared to other sets. Except for the program
 rom all other data matches Paradise (set 1).
***************************************************************************/

ROM_START( paradise )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* Z80 Code */
	ROM_LOAD( "u128", 0x00000, 0x40000, CRC(8e5b5a24) SHA1(a4e559d9329f8a7a9d12cd90d98d0525958085d8) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT)  /* 16x16x8 Sprites */
	ROM_LOAD( "u114", 0x00000, 0x40000, CRC(c748ba3b) SHA1(ad23bda4e001ca539f849c1ca256de5daf7c233b) )
	ROM_LOAD( "u115", 0x40000, 0x40000, CRC(0d517bbb) SHA1(5bf7c5036f3d660901e26f14baaea1a3c0327dfe) )

	ROM_REGION( 0x20000, "gfx2", ROMREGION_INVERT)  /* 8x8x4 Background */
	ROM_LOAD( "u94", 0x00000, 0x20000, CRC(e3a99209) SHA1(5db79dc1a38d93b458b043499a58516285c65aa8) )

	ROM_REGION( 0x100000, "gfx3", ROMREGION_INVERT) /* 8x8x8 Foreground */
	ROM_LOAD( "u92", 0x00000, 0x80000, CRC(633d24f0) SHA1(26b25ec1014fba1a3d0d2bdba0c867c57034647d) )
	ROM_LOAD( "u93", 0x80000, 0x80000, CRC(bbf5c632) SHA1(9d31e136f014c2dd7dd988c3aee0adfcfea91bc9) )

	ROM_REGION( 0x40000, "gfx4", ROMREGION_INVERT)  /* 8x8x8 Midground */
	ROM_LOAD( "u110", 0x00000, 0x20000, CRC(9807a7e6) SHA1(30e2a741a93954cfe672c61c93a990d0c3b25145) )
	ROM_LOAD( "u111", 0x20000, 0x20000, CRC(bc9f93f0) SHA1(dd4cfc849a0c0f918ac0dfeb7f00a67aae5a1c13) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "u85", 0x00000, 0x40000, CRC(bf3c3065) SHA1(54dd7ffea2fb3f31ed575e982b82691cddc2581a) )

	ROM_REGION( 0x80000, "oki2", 0 )    /* Samples (banked) */
	ROM_LOAD( "u113", 0x00000, 0x80000, CRC(53de6025) SHA1(c94b3778b57ff7f46ce4cff661841019fb187d5d) )
ROM_END

ROM_START( paradisea )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* Z80 Code */
	ROM_LOAD( "a-19.u128", 0x00000, 0x40000, CRC(d47ecb7e) SHA1(74e7a33f2fc4c7c830c53c50541c3d0efd152e98) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT)  /* 16x16x8 Sprites */
	ROM_LOAD( "a-19.u114", 0x00000, 0x40000, CRC(c748ba3b) SHA1(ad23bda4e001ca539f849c1ca256de5daf7c233b) )
	ROM_LOAD( "a-19.u115", 0x40000, 0x40000, CRC(0d517bbb) SHA1(5bf7c5036f3d660901e26f14baaea1a3c0327dfe) )

	ROM_REGION( 0x20000, "gfx2", ROMREGION_INVERT)  /* 8x8x4 Background */
	ROM_LOAD( "a-19.u94", 0x00000, 0x20000, CRC(e3a99209) SHA1(5db79dc1a38d93b458b043499a58516285c65aa8) )

	ROM_REGION( 0x100000, "gfx3", ROMREGION_INVERT) /* 8x8x8 Foreground */
	ROM_LOAD( "a-19.u92", 0x00000, 0x80000, CRC(633d24f0) SHA1(26b25ec1014fba1a3d0d2bdba0c867c57034647d) )
	ROM_LOAD( "a-19.u93", 0x80000, 0x80000, CRC(bbf5c632) SHA1(9d31e136f014c2dd7dd988c3aee0adfcfea91bc9) )

	ROM_REGION( 0x40000, "gfx4", ROMREGION_INVERT)  /* 8x8x8 Midground */
	ROM_LOAD( "a-19.u110", 0x00000, 0x20000, CRC(9807a7e6) SHA1(30e2a741a93954cfe672c61c93a990d0c3b25145) )
	ROM_LOAD( "a-19.u111", 0x20000, 0x20000, CRC(bc9f93f0) SHA1(dd4cfc849a0c0f918ac0dfeb7f00a67aae5a1c13) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "a-19.u85", 0x00000, 0x40000, CRC(bf3c3065) SHA1(54dd7ffea2fb3f31ed575e982b82691cddc2581a) )

	ROM_REGION( 0x80000, "oki2", 0 )    /* Samples (banked) */
	ROM_LOAD( "a-19.u113", 0x00000, 0x80000, CRC(53de6025) SHA1(c94b3778b57ff7f46ce4cff661841019fb187d5d) )
ROM_END

ROM_START( paradisee ) /* YS-1600 PCB. All labels are simply labeled "Escape" */
	ROM_REGION( 0x40000, "maincpu", 0 )     /* Z80 Code */
	ROM_LOAD( "escape.u128", 0x00000, 0x40000, CRC(19b4e854) SHA1(7d7292017df67b7ed3a3e0059334866890c58b83) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT)  /* 16x16x8 Sprites */
	ROM_LOAD( "escape.u114", 0x00000, 0x40000, CRC(c748ba3b) SHA1(ad23bda4e001ca539f849c1ca256de5daf7c233b) )
	ROM_LOAD( "escape.u115", 0x40000, 0x40000, CRC(0d517bbb) SHA1(5bf7c5036f3d660901e26f14baaea1a3c0327dfe) )

	ROM_REGION( 0x20000, "gfx2", ROMREGION_INVERT)  /* 8x8x4 Background */
	ROM_LOAD( "escape.u94", 0x00000, 0x20000, CRC(e3a99209) SHA1(5db79dc1a38d93b458b043499a58516285c65aa8) )

	ROM_REGION( 0x100000, "gfx3", ROMREGION_INVERT) /* 8x8x8 Foreground */
	ROM_LOAD( "escape.u92", 0x00000, 0x80000, CRC(633d24f0) SHA1(26b25ec1014fba1a3d0d2bdba0c867c57034647d) )
	ROM_LOAD( "escape.u93", 0x80000, 0x80000, CRC(bbf5c632) SHA1(9d31e136f014c2dd7dd988c3aee0adfcfea91bc9) )

	ROM_REGION( 0x40000, "gfx4", ROMREGION_INVERT)  /* 8x8x8 Midground */
	ROM_LOAD( "escape.u110", 0x00000, 0x20000, CRC(9807a7e6) SHA1(30e2a741a93954cfe672c61c93a990d0c3b25145) )
	ROM_LOAD( "escape.u111", 0x20000, 0x20000, CRC(bc9f93f0) SHA1(dd4cfc849a0c0f918ac0dfeb7f00a67aae5a1c13) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "escape.u85", 0x00000, 0x40000, CRC(bf3c3065) SHA1(54dd7ffea2fb3f31ed575e982b82691cddc2581a) )

	ROM_REGION( 0x80000, "oki2", 0 )    /* Samples (banked) */
	ROM_LOAD( "escape.u113", 0x00000, 0x80000, CRC(53de6025) SHA1(c94b3778b57ff7f46ce4cff661841019fb187d5d) )
ROM_END

ROM_START( paradlx )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* Z80 Code */
	ROM_LOAD( "8.u128", 0x00000, 0x40000, CRC(3a45ac9e) SHA1(24e1b508ef582c8429e09929fea387f3a137f0e3) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT)  /* 16x16x8 Sprites */
	ROM_LOAD( "6.u114", 0x00000, 0x40000, CRC(d0341838) SHA1(fa400486968bd6b5a805fb79a970bb280ee24662) )
	ROM_LOAD( "7.u115", 0x40000, 0x40000, CRC(a6231efd) SHA1(2f484ce2081c692b48dbfd98e152b7a74de9c414) )

	ROM_REGION( 0x40000, "gfx2", ROMREGION_INVERT)  /* 8x8x4 Background */
	ROM_LOAD( "5.u94", 0x00000, 0x40000, CRC(70560945) SHA1(f5f1f1779178cb3d1bb4789a135cd49a0d0fd99b) )

	ROM_REGION( 0x100000, "gfx3", ROMREGION_INVERT) /* 8x8x8 Foreground */
	ROM_LOAD( "3.u92", 0x00000, 0x80000, CRC(c61aa37b) SHA1(8f4235a6ff47209b5982aa1c143f3c877bfd1bae) )
	ROM_LOAD( "4.u93", 0x80000, 0x80000, CRC(658f855d) SHA1(73a9377633b53869c47c443898914b70238b591a) )

	ROM_REGION( 0x40000, "gfx4", ROMREGION_INVERT)  /* 8x8x8 Midground */
	ROM_LOAD( "1.u110", 0x00000, 0x20000, CRC(6b7f9bb9) SHA1(fd150c8e5a560bff49c993b0b703d84f775ea0b0) )
	ROM_LOAD( "2.u111", 0x20000, 0x20000, CRC(eb291f96) SHA1(096f09894f4a319c30daa7a3051798902d4fd1eb) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "10.u85", 0x00000, 0x40000, CRC(ed20133e) SHA1(a6ab1ab3ca075a3b03fe96cd32a5c186ee26d095) )

	ROM_REGION( 0x80000, "oki2", 0 )    /* Samples (banked) */
	ROM_LOAD( "9.u113", 0x00000, 0x80000, CRC(9c5337f0) SHA1(4d7a8069be4551aad9d7d32d835dcf91be079359) )
ROM_END

ROM_START( para2dx )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* Z80 Code */
	ROM_LOAD( "pdx2_u128.bin", 0x00000, 0x40000, CRC(4cbd22e1) SHA1(ad69663109d3127f6472797ec8763097da94b7d4) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT)  /* 16x16x8 Sprites */
	ROM_LOAD( "pdx2_u114.bin", 0x00000, 0x40000, CRC(3634b086) SHA1(6d079efb7be4fbe51d95d1f6b2c44dafdacb6016) )
	ROM_LOAD( "pdx2_u115.bin", 0x40000, 0x40000, CRC(404409f4) SHA1(0763da81a1eb57037edd816e49a56dc8609fa502) )

	ROM_REGION( 0x40000, "gfx2", ROMREGION_INVERT)  /* 8x8x4 Background */
	ROM_LOAD( "pdx2_u094.bin", 0x00000, 0x40000,  CRC(87c4521b) SHA1(3ebd1e475e6125e9361b21160736103471c7aa2b) )

	ROM_REGION( 0x100000, "gfx3", ROMREGION_INVERT) /* 8x8x8 Foreground */
	ROM_LOAD( "pdx2_u092.bin", 0x00000, 0x80000, CRC(d6797812) SHA1(b2d463b5932501382abcbbd911c492b6671c3cf7) )
	ROM_LOAD( "pdx2_u093.bin", 0x80000, 0x80000, CRC(7644b8e9) SHA1(5f570c565523748afddc37f9ddd276c83f6b7d19) )

	ROM_REGION( 0x40000, "gfx4", ROMREGION_INVERT)  /* 8x8x8 Midground */
	ROM_LOAD( "pdx2_u110.bin", 0x00000, 0x20000, CRC(59e828d1) SHA1(fd76c5a74e1be22bde52bcfbce179dc73591bec3) )
	ROM_LOAD( "pdx2_u111.bin", 0x20000, 0x20000, CRC(619dd972) SHA1(c64d256d21da5821e27b1cd55351ad2a9b141f47) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "pdx2_u085.bin", 0x00000, 0x40000, CRC(398b842e) SHA1(933759d2907640e85f11f532096ee1a912f67b53) )

	ROM_REGION( 0x80000, "oki2", 0 )    /* Samples (banked) */
	ROM_LOAD( "pdx2_u113.bin", 0x00000, 0x80000, CRC(9c5337f0) SHA1(4d7a8069be4551aad9d7d32d835dcf91be079359) )
ROM_END

/***************************************************************************

                          Target Ball

Yun Sung, 1995

PCB Layout
----------

YS-2002 YUNSUNG
|---------------------------------------------------------|
|  M6295  M6295   Z80              4MHz       YUNSUNG.110 |
| YUNSUNG.113   YUNSUNG.128                   YUNSUNG.111 |
| YUNSUNG.85     6264                                     |
|                                             YUNSUNG.92  |
|                                             YUNSUNG.93  |
|                                                         |
|                6116                                     |
|                6116              6116                   |
|J               6116                                     |
|A                                                        |
|M                                            6116        |
|M                                            6116        |
|A                            |-------|                   |
|                             | ACTEL |                   |
|                             |A1020B |                   |
|DSW1(8)                      |PLCC84 |                   |
|         12MHz               |-------|                   |
|                           4464                          |
|                           4464     YUNSUNG.114          |
|                           4464     YUNSUNG.115          |
|DSW2(8)                    4464                          |
|                                                         |
|---------------------------------------------------------|
Notes:
      Z80 clock: 6.000MHz
     6295 clock: 1.000MHz (both), sample rate = 1000000/132 (both)
          VSync: 54Hz

 note even with these settings game runs slightly faster in Mame than real PCB


***************************************************************************/

ROM_START( tgtbal96 ) /* mainly a title screen hack?? But original Yun Sung PCB and rom labels */
	ROM_REGION( 0x40000, "maincpu", 0 )     /* Z80 Code */
	ROM_LOAD( "bc7.u128", 0x00000, 0x40000, CRC(3ae07ee5) SHA1(830890e5fe93fa85f306df06c5b84c7f2aa266c8) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT)  /* 16x16x8 Sprites */
	ROM_LOAD( "yunsung.u114", 0x00000, 0x40000, CRC(3dbe1872) SHA1(754f90123a3944ca548fc66ee65a93615155bf30) )
	ROM_LOAD( "yunsung.u115", 0x40000, 0x40000, CRC(30f49dac) SHA1(b70d37973bd03069c48641d6c0804be6f9aa6553) )

	ROM_REGION( 0x20000, "gfx2", ROMREGION_ERASEFF) /* 8x8x4 Background */
	/* not for this game? */

	ROM_REGION( 0x100000, "gfx3", ROMREGION_INVERT) /* 8x8x8 Foreground */
	ROM_LOAD( "bc10.u92", 0x00000, 0x80000, CRC(2f511f93) SHA1(4d4b543e981855fdd42b9902c8d85a56dc2bb8a6) ) /* Slide show reveals the title screen added a newer copyright and the '96 */
	ROM_LOAD( "bc11.u93", 0x80000, 0x80000, CRC(c5acf1e0) SHA1(6306a231cfe6fb5ebc86ea7adf122331b8b830ca) ) /* otherwise the graphics appear to be the same as the tgtballn below */

	ROM_REGION( 0x100000, "gfx4", ROMREGION_INVERT) /* 8x8x8 Midground */
	ROM_LOAD( "bc8.u110", 0x00000, 0x80000,  CRC(f97d754e) SHA1(0fb32d77d79ee0f438bafbfb09836278b587acca) )
	ROM_LOAD( "bc9.u111", 0x80000, 0x80000,  CRC(ee0728c0) SHA1(6a2a782f744c7d9318a63acc060046de01fc9ee5) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "yunsung.u85", 0x00000, 0x20000, CRC(cdf3336b) SHA1(98029d6d5d8ffb3b24ae2bcf950618a7d5b404c3) )

	ROM_REGION( 0x80000, "oki2", 0 )    /* Samples (banked) */
	ROM_LOAD( "yunsung.u113", 0x00000, 0x40000, CRC(150a6cc6) SHA1(b435fcf8ba48006f506db6b63ba54a30a6b3eade) )
ROM_END

ROM_START( tgtballn )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* Z80 Code */
	ROM_LOAD( "rom7.u128", 0x00000, 0x40000, CRC(8dbeab12) SHA1(7181c23459990aecbe2d13377aaf19f65108eac6) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT)  /* 16x16x8 Sprites */
	ROM_LOAD( "yunsung.u114", 0x00000, 0x40000, CRC(3dbe1872) SHA1(754f90123a3944ca548fc66ee65a93615155bf30) )
	ROM_LOAD( "yunsung.u115", 0x40000, 0x40000, CRC(30f49dac) SHA1(b70d37973bd03069c48641d6c0804be6f9aa6553) )

	ROM_REGION( 0x20000, "gfx2", ROMREGION_ERASEFF) /* 8x8x4 Background */
	/* not for this game? */

	ROM_REGION( 0x100000, "gfx3", ROMREGION_INVERT) /* 8x8x8 Foreground */
	ROM_LOAD( "rom2.u92", 0x00000, 0x80000, CRC(fe4004ec) SHA1(fde782665445ad465b8f8fb95df5f60cd24016ad) )
	ROM_LOAD( "rom1.u93", 0x80000, 0x80000, CRC(aef17762) SHA1(3dd8924695b67eec0f25549dbe2461b927268b8f) )

	ROM_REGION( 0x100000, "gfx4", ROMREGION_INVERT) /* 8x8x8 Midground */
	ROM_LOAD( "rom4.u110", 0x00000, 0x80000,  CRC(0a5abf62) SHA1(6900d598764300c81c90f5a7efb294639178bee6) )
	ROM_LOAD( "rom3.u111", 0x80000, 0x80000,  CRC(94822bbf) SHA1(9fa6595eb819f163b58181926c276346cfa5c332) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "yunsung.u85", 0x00000, 0x20000, CRC(cdf3336b) SHA1(98029d6d5d8ffb3b24ae2bcf950618a7d5b404c3) )

	ROM_REGION( 0x80000, "oki2", 0 )    /* Samples (banked) */
	ROM_LOAD( "yunsung.u113", 0x00000, 0x40000, CRC(150a6cc6) SHA1(b435fcf8ba48006f506db6b63ba54a30a6b3eade) )
ROM_END

ROM_START( tgtball )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* Z80 Code */
	ROM_LOAD( "yunsung.u128", 0x00000, 0x40000, CRC(cb0f3d46) SHA1(b56c4abbd4248074c1559a0f1902d2ea11cb01a8) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT)  /* 16x16x8 Sprites */
	ROM_LOAD( "yunsung.u114", 0x00000, 0x40000, CRC(3dbe1872) SHA1(754f90123a3944ca548fc66ee65a93615155bf30) )
	ROM_LOAD( "yunsung.u115", 0x40000, 0x40000, CRC(30f49dac) SHA1(b70d37973bd03069c48641d6c0804be6f9aa6553) )

	ROM_REGION( 0x20000, "gfx2", ROMREGION_ERASEFF) /* 8x8x4 Background */
	/* not for this game? */

	ROM_REGION( 0x100000, "gfx3", ROMREGION_INVERT) /* 8x8x8 Foreground */
	ROM_LOAD( "yunsung.u92", 0x00000, 0x80000, CRC(bcf206a9) SHA1(0db2cee21c025b7b8d2d5b898c7231c77e36904d) )
	ROM_LOAD( "yunsung.u93", 0x80000, 0x80000, CRC(64edb93c) SHA1(94f8d4fd159c682d952d6a4c38dc50f2c0c0824d) )

	ROM_REGION( 0x100000, "gfx4", ROMREGION_INVERT) /* 8x8x8 Midground */
	ROM_LOAD( "yunsung.u110", 0x00000, 0x80000, CRC(c209201e) SHA1(ba1cb3a204f689f9a3636834628d2265927e34f7) )
	ROM_LOAD( "yunsung.u111", 0x80000, 0x80000, CRC(82334337) SHA1(4b2a07196027b190366131cd7b8eca87a1bd0b1c) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "yunsung.u85", 0x00000, 0x20000, CRC(cdf3336b) SHA1(98029d6d5d8ffb3b24ae2bcf950618a7d5b404c3) )

	ROM_REGION( 0x80000, "oki2", 0 )    /* Samples (banked) */
	ROM_LOAD( "yunsung.u113", 0x00000, 0x40000, CRC(150a6cc6) SHA1(b435fcf8ba48006f506db6b63ba54a30a6b3eade) )
ROM_END

/***************************************************************************

                          Penky
Yun Sung, 1996

YS951004
  CPU: Z8400B PS (Z80 6Mhz)
Sound: OKI M6295 x 2
Video: Actel A1020A PL84C
  OSC: 12.000MHz & 4.000MHz

YS951004
+--------------------------------------------+
|    M6295 M6295   Z80    4MHz    U110  U111 |
|VOL     U113*    U128                  U92  |
|        U85     6264                   U93  |
|                                       U94  |
|               6116                         |
|J                     +-------+             |
|A                     | Actel |  6116       |
|M              6116   |A1020A |             |
|M                     | PL84C |             |
|A              6116   +-------+             |
|       12MHz                                |
| DSW1                                  6116 |
|                     4464              6116 |
|                     4464  U114        6116 |
| DSW2                4464  U115             |
|                     4464                   |
+--------------------------------------------+

U113 is not populated on this PCB

Notes, the clocks should be the same as other boards of this era/type. IE:
      Z80 clock: 6.000MHz
     6295 clock: 1.000MHz (both), sample rate = 1000000/132 (both)

***************************************************************************/

ROM_START( penky )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* Z80 Code */
	ROM_LOAD( "yunsung.u128", 0x00000, 0x40000, CRC(57baeada) SHA1(360fd2d352b201e57436ed9c9f0510a052452738) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_INVERT) /* 16x16x8 Sprites */
	ROM_LOAD( "yunsung.u114", 0x00000, 0x80000, CRC(cb6b1cfd) SHA1(22406f70fc2ad839d5ca4d00d503a2857b295cf5) )
	ROM_LOAD( "yunsung.u115", 0x80000, 0x80000, CRC(55c5ff90) SHA1(f68a22628b9da77c3e301fa57bf673c572760869) )

	ROM_REGION( 0x20000, "gfx2", ROMREGION_INVERT)  /* 8x8x4 Background */
	ROM_LOAD( "yunsung.u94", 0x00000, 0x20000, CRC(58b31c0e) SHA1(eea9a0c17737ce071895f818499edee7790d98f7) )

	ROM_REGION( 0x100000, "gfx3", ROMREGION_INVERT) /* 8x8x8 Foreground */
	ROM_LOAD( "yunsung.u92", 0x00000, 0x80000, CRC(31993a6c) SHA1(8cdcae52472768f40dc7cbefaa459982d008deaa) )
	ROM_LOAD( "yunsung.u93", 0x80000, 0x80000, CRC(b570dc0c) SHA1(1f55681412db144e2d5cbb7a89783edc5059add7) )

	ROM_REGION( 0x100000, "gfx4", ROMREGION_INVERT) /* 8x8x8 Midground */
	ROM_LOAD( "yunsung.u110", 0x00000, 0x80000, CRC(ba3173a1) SHA1(6667bced70eb6be9853239feb69d4b30daf2d0c1) )
	ROM_LOAD( "yunsung.u111", 0x80000, 0x80000, CRC(9223ef85) SHA1(f8da8fc5c8178165e8142eb52889b4ef1c710e24) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "yunsung.u85", 0x00000, 0x40000, CRC(c664d0cc) SHA1(52d5122407e727d4c98bc6f2f939534de4b725ae) )

	ROM_REGION( 0x80000, "oki2", ROMREGION_ERASE00 )    /* Samples (banked) */
	/* not populated for this game */
ROM_END

ROM_START( penkyi )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* Z80 Code */
	ROM_LOAD( "u128", 0x00000, 0x80000, CRC(17c8c97c) SHA1(8f5a88670f64ae5591b4ac1b6ddd7aa7db60e042) ) // 27C040, but 1st and 2nd half identical

	ROM_REGION( 0x100000, "gfx1", ROMREGION_INVERT) /* 16x16x8 Sprites */
	ROM_LOAD( "u114", 0x00000, 0x80000, CRC(593e7b15) SHA1(bf2719e86bb23b2f149b6721fd3e8131b388ceca) ) // 27C040
	ROM_LOAD( "u115", 0x80000, 0x80000, CRC(29449fa2) SHA1(6aae7967952d3ed1a95201b4f467f3b73e8df4f6) ) // 27C040

	ROM_REGION( 0x20000, "gfx2", ROMREGION_INVERT)  /* 8x8x4 Background */
	ROM_LOAD( "u94", 0x00000, 0x10000, CRC(d45bac24) SHA1(fc869647873f29bb44f4d58333fdb023d99028de) ) // 27C512, half size of the one in the penky set, mostly 0 filled anyway

	ROM_REGION( 0x100000, "gfx3", ROMREGION_INVERT) /* 8x8x8 Foreground */
	ROM_LOAD( "u92", 0x00000, 0x80000, CRC(31993a6c) SHA1(8cdcae52472768f40dc7cbefaa459982d008deaa) ) // 27C040
	ROM_LOAD( "u93", 0x80000, 0x80000, CRC(b570dc0c) SHA1(1f55681412db144e2d5cbb7a89783edc5059add7) ) // 27C040

	ROM_REGION( 0x100000, "gfx4", ROMREGION_INVERT) /* 8x8x8 Midground */
	ROM_LOAD( "u110", 0x00000, 0x80000, CRC(c6501e3a) SHA1(f6fa7925a395a226714c4f5536866bc87c1bf0ca) ) // 27C040
	ROM_LOAD( "u111", 0x80000, 0x80000, CRC(de405c6f) SHA1(715e111438d4cbecc435a519ae370842f5531163) ) // 27C040

	ROM_REGION( 0x80000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "u85", 0x00000, 0x80000, CRC(452578cd) SHA1(a86ce33df0a5dc9d58233820689d52943844a7ea) )// 27C040, but 1st and 2nd half identical

	ROM_REGION( 0x80000, "oki2", ROMREGION_ERASE00 )    /* Samples (banked) */
	/* not populated for this game */
ROM_END

/*

Yun Sung Torus (c) 1996

PCB Layout
----------

Looks identical to the YS-0402 but has no number printed on the PCB
  For vertical games, Yun Sung would take a known board reprogram the
  Actel A1020B but not give the PCB a specific number.  This was also
  done for Yun Sung's game Paparazzi.  See yunsun16.c driver

+----------------------------------------+
|    M6295 Z80 4MHz                      |
|          U1                            |
|     U    6264                          |
|VOL  2                                  |
|     8                                  |
|                                        |
|J DSW1   12MHz                          |
|A                                       |
|M DSW2       +-------+                  |
|M            | ACTEL |                  |
|A            |A1020B |                  |
|             |PLCC84 |                  |
| U66         +-------+ 6116             |
| U67                   6116             |
| U92                   6116        62256|
| U93                   6116        62256|
| U105                  6116        62256|
| U106                  6116        62256|
+----------------------------------------+

Z80A
AD-65 (OKI M6295)
Actel A1020B PLC84C (or compatible QuickLogic QL12X16B-XPL84C)
OSC: 12.000 MHz, 4.000Mhz

RAM 4 Hyundai HY62256ALP-70
    1 Hyundai HY6264LP-10 (by program rom & Z80A)
    6 UMC UM6116K-3
DSW 2 8-switch DIP

All roms had a Yun Sung label with no other ID markings or numbers

*/

ROM_START( torus )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* Z80 Code */
	ROM_LOAD( "yunsung.u1", 0x00000, 0x10000, CRC(55d3ef3e) SHA1(195463271fdb3f9f5c19068efd1c99105f761fe9) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT)  /* 16x16x8 Sprites */
	ROM_LOAD( "yunsung.u67", 0x00000, 0x40000, CRC(5b60ce9f) SHA1(d5c091145e0bae7cd776e642ea17895d086ed2b0) )
	ROM_LOAD( "yunsung.u66", 0x40000, 0x40000, CRC(4caa0c50) SHA1(a971b6e87cd1162cf370d39cfeafefbb1557e14e) )

	ROM_REGION( 0x20000, "gfx2", ROMREGION_ERASEFF) /* 8x8x4 Background */
	/* not for this game */

	ROM_REGION( 0x100000, "gfx3", ROMREGION_INVERT) /* 8x8x8 Foreground */
	ROM_LOAD( "yunsung.u105", 0x00000, 0x80000, CRC(67c5ba1a) SHA1(0e39752ddc5ee9469647140a3fc9e6bb69d6afa1) )
	ROM_LOAD( "yunsung.u106", 0x80000, 0x80000, CRC(efb105e9) SHA1(7bfe6ff64b25797dd524a7077def5669f25f16ec) )

	ROM_REGION( 0x40000, "gfx4", ROMREGION_INVERT)  /* 8x8x8 Midground */
	ROM_LOAD( "yunsung.u93", 0x00000, 0x20000, CRC(ee914caf) SHA1(42f3d760a4c14658ac2eb0ba7f54fb9916368b50) )
	ROM_LOAD( "yunsung.u92", 0x20000, 0x20000, CRC(aff1dab9) SHA1(ae488abd605c1e78b8b73452a2c1391cc0fe6b00) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "yunsung.u28", 0x00000, 0x40000, CRC(12d84839) SHA1(840d82253c0651ebe6799ea2bb5bae334e963e12) )
ROM_END

/*

Yun Sung Mad Ball (c) 1997-1998

PCB Layout
----------

YS-0402 YUNSUNG
+----------------------------------------+
|    M6295 Z80 4MHz                      |
|          U1                            |
|     U    6264                          |
|VOL  2                                  |
|     8                                  |
|                                        |
|J DSW1   12MHz                          |
|A                                       |
|M DSW2       +-------+                  |
|M            | ACTEL |                  |
|A            |A1020B |                  |
|             |PLCC84 |                  |
| U66         +-------+ 6116             |
| U67                   6116             |
| U92                   6116        62256|
| U93                   6116        62256|
| U105                  6116        62256|
| U106                  6116        62256|
+----------------------------------------+

Z80A
AD-65 (OKI M6295)
Actel A1020B PLC84C (or compatible QuickLogic QL12X16B-XPL84C)
OSC: 12.000 MHz, 4.000Mhz

RAM 4 Hyundai HY62256ALP-70
    1 Hyundai HY6264LP-10 (by P rom & Z80A)
    6 UMC UM6116K-3
DSW 2 8-switch DIP

P.u1  Intel i27C010A - Program (next to Z80A)
s.u28 ST M27C4001    - Sound (next to AD-65)

1.u66  ST M27C2001
2.u67  ST M27C2001
3.u92  TI 27C040
4.u93  TI 27C040
5.u105 TI 27C040
6.u106 TI 27C040

All roms read with manufacturer's IDs and routines

NOTE: A version of Mad Ball has been seen with a YS-1302 daughtercard that plugs in through the Z80 processor:

YS-1302 Spinner Daughtercard
+--------------------------+
|   [Connector to Main]    |
| CN1 R   Z8400B           |
| CN2 S   GAL16V8  74LS669 |
| CD4584  74LS374  74LS669 |
| 74LS669 74LS374  74LS669 |
+--------------------------+

CN1 & CN2 are 4 pin headers for spinner controls
RS is a 4.7K 5 pin resistor pack

*/

ROM_START( madball ) /* Models in swimsuits only, no nudity */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* Z80 Code */
	ROM_LOAD( "p.u1", 0x00000, 0x20000, CRC(73008425) SHA1(6eded60fd5c637a63783247c858d999d5974d378) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT)  /* 16x16x8 Sprites */
	ROM_LOAD( "2.u67", 0x00000, 0x40000, CRC(1f3a6cd5) SHA1(7a17549f2fff003605d91703c84a398488b2f74c) )
	ROM_LOAD( "1.u66", 0x40000, 0x40000, CRC(8637c7b4) SHA1(e0026e48f0e8f3554a5b448e0d1f9d1c5551dbfb) )

	ROM_REGION( 0x20000, "gfx2", ROMREGION_ERASEFF) /* 8x8x4 Background */
	/* not for this game */

	ROM_REGION( 0x100000, "gfx3", ROMREGION_INVERT) /* 8x8x8 Foreground */
	ROM_LOAD( "5.u105", 0x00000, 0x80000, CRC(f26aac1e) SHA1(50ad34ee70bf45fa4e1dc9281b83bcdd7c7db3f8) )
	ROM_LOAD( "6.u106", 0x80000, 0x80000, CRC(27b78907) SHA1(ab6645457adc0d17b141e366aac7e00e8ce4296b) )

	ROM_REGION( 0x100000, "gfx4", ROMREGION_INVERT) /* 8x8x8 Midground */
	ROM_LOAD( "4.u93", 0x80000, 0x80000, CRC(c3be56ad) SHA1(9cfa0b38c60798deccca74dc6b0ce0826ff7f467) )
	ROM_LOAD( "3.u92", 0x00000, 0x80000, CRC(846019a6) SHA1(571bfa299e13b96ca263bd7e62c760bdbe3438bd) )

	ROM_REGION( 0x80000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "s.u28", 0x00000, 0x80000, CRC(78f02584) SHA1(70542e126db73a573db9ef41399d3a07fb7ea94b) )
ROM_END

ROM_START( madballn ) /* Even numbered stages show topless models.  Is nudity controlled by a dipswitch? */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* Z80 Code */
	ROM_LOAD( "u1.bin", 0x00000, 0x20000, CRC(531fa919) SHA1(0eafc663b9ad50d0dfc5491fe96c9bcf30483991) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT)  /* 16x16x8 Sprites */
	ROM_LOAD( "2.u67", 0x00000, 0x40000, CRC(1f3a6cd5) SHA1(7a17549f2fff003605d91703c84a398488b2f74c) )
	ROM_LOAD( "1.u66", 0x40000, 0x40000, CRC(8637c7b4) SHA1(e0026e48f0e8f3554a5b448e0d1f9d1c5551dbfb) )

	ROM_REGION( 0x20000, "gfx2", ROMREGION_ERASEFF) /* 8x8x4 Background */
	/* not for this game */

	ROM_REGION( 0x100000, "gfx3", ROMREGION_INVERT) /* 8x8x8 Foreground */
	ROM_LOAD( "u105.bin", 0x00000, 0x80000, CRC(d75faa62) SHA1(95badf932e8a8084e67aa7df8d6cb2cb2917d5fc) )
	ROM_LOAD( "u106.bin", 0x80000, 0x80000, CRC(04b8f7a5) SHA1(97555880f200d0ecc521f8c76bcaa4a0f0eb1aa9) )

	ROM_REGION( 0x100000, "gfx4", ROMREGION_INVERT) /* 8x8x8 Midground */
	ROM_LOAD( "u93.bin", 0x80000, 0x80000, CRC(f07a5fe6) SHA1(0b1117d8ff0f2a6c953ab1988065b75a33e2c949) )
	ROM_LOAD( "u92.bin", 0x00000, 0x80000, CRC(7ed233ab) SHA1(8a4bc31741b4e6e1c03974f9b00f747a29c78ebf) )

	ROM_REGION( 0x80000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "s.u28", 0x00000, 0x80000, CRC(78f02584) SHA1(70542e126db73a573db9ef41399d3a07fb7ea94b) )
ROM_END

void paradise_state::init_paradise()
{
	m_sprite_inc = 0x20;
}

// Inverted flipscreen and sprites are packed in less memory (same number though)
void paradise_state::init_tgtball()
{
	m_sprite_inc = 4;
	m_maincpu->space(AS_IO).install_write_handler(0x2001, 0x2001, write8_delegate(FUNC(paradise_state::tgtball_flipscreen_w),this));

}

void paradise_state::init_torus()
{
	m_sprite_inc = 4;
	m_maincpu->space(AS_IO).install_write_handler(0x2070, 0x2070, write8_delegate(FUNC(paradise_state::torus_coin_counter_w),this));
}


/***************************************************************************

                                Game Drivers

***************************************************************************/

GAME( 1994,  paradise, 0,         paradise, paradise, paradise_state, init_paradise, ROT90, "Yun Sung", "Paradise (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1994,  paradisea, paradise, paradise, paradise, paradise_state, init_paradise, ROT90, "Yun Sung", "Paradise (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1994,  paradisee, paradise, paradise, paradise, paradise_state, init_paradise, ROT90, "Yun Sung (Escape license)", "Paradise (Escape)", MACHINE_SUPPORTS_SAVE )
GAME( 199?,  paradlx,  0,         paradise, paradise, paradise_state, init_paradise, ROT90, "Yun Sung", "Paradise Deluxe", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // year not shown, but should be >=1994
GAME( 199?,  para2dx,  0,         paradise, para2dx,  paradise_state, init_paradise, ROT90, "Yun Sung", "Paradise 2 Deluxe", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // year not shown, but should be >=1994
GAME( 1996,  tgtbal96, 0,         tgtball,  tgtball,  paradise_state, init_tgtball,  ROT0,  "Yun Sung", "Target Ball '96", MACHINE_SUPPORTS_SAVE ) // With nudity
GAME( 1995,  tgtball,  tgtbal96,  tgtball,  tgtball,  paradise_state, init_tgtball,  ROT0,  "Yun Sung", "Target Ball", MACHINE_SUPPORTS_SAVE )
GAME( 1995,  tgtballn, tgtbal96,  tgtball,  tgtball,  paradise_state, init_tgtball,  ROT0,  "Yun Sung", "Target Ball (With Nudity)", MACHINE_SUPPORTS_SAVE )
GAME( 1996,  penky,    0,         penky,    penky,    paradise_state, init_tgtball,  ROT0,  "Yun Sung", "Penky", MACHINE_SUPPORTS_SAVE )
GAME( 1996,  penkyi,   penky,     penkyi,   penkyi,   paradise_state, init_tgtball,  ROT0,  "Yun Sung (Impeuropex license)", "Penky (Italian)", MACHINE_SUPPORTS_SAVE )
GAME( 1996,  torus,    0,         torus,    torus,    paradise_state, init_torus,    ROT90, "Yun Sung", "Torus", MACHINE_SUPPORTS_SAVE )
GAME( 1998,  madball,  0,         madball,  madball,  paradise_state, init_tgtball,  ROT0,  "Yun Sung", "Mad Ball V2.0", MACHINE_SUPPORTS_SAVE )
GAME( 1997,  madballn, madball,   madball,  madball,  paradise_state, init_tgtball,  ROT0,  "Yun Sung", "Mad Ball V2.0 (With Nudity)", MACHINE_SUPPORTS_SAVE )
