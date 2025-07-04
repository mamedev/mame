// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/*

    United Amusements PC-Engine based hardware
    Driver by Mariusz Wojcieszek
    Thanks for Charles MacDonald for hardware docs

 Overview

 The system consists of a stock PC-Engine console, JAMMA interface board,
 and several cables to connect certain pins from the PCE backplane connector
 and pad connector to the interface board.

 History

 In 1989 United Amusements (a large operator of arcades in the US at that
 time) developed a JAMMA interface for the PC-Engine with NEC's blessing. NEC
 pulled funding for the project before mass production began, and it never
 took off.

 Driver notes:
 - game time is controlled using software loop - with current clock it takes lots
   of time until credit expires. Should Z80 clock be raised?
 - tone played by jamma if board is not emulated

 A kit owned by collector has following contents:
 -x1 NOS boxed NEC arcade unit with all connectors
 -x1 NOS boxed Keith Courage (custom/prototype PCB) with NOS marquee
 -x1 Pacland standard Hucard WITH arcade cabinet marquee (this proves that while this is a standard card, it was sold by United Amusements with the intention of being used in the arcade unit)
 -x1 Alien Crush standard Hucard with arcade "how to play" placard. Same as above.... proof this was sold for the arcade unit and not as a home card
 -x1 Ninja Warriors standard Hucard
 -x1 Victory Run standardd Hucard
 -x1 Plexiglass control panel overlay (used)

In the August 1989 issue of Vending Times magazine:
https://archive.org/details/VendingTimesVOL29NO10August1989Clearscan/page/n69
https://archive.org/details/VendingTimesVOL29NO10August1989Clearscan/page/n99
there was a list of all of the available UA produced HU-Cards about to be released.
- Legendary Axe
- Victory Run
- Keith Courage in the Alpha Zones
- World Class Baseball
- Power Golf
- Blazing Lazers
- Dungeon Explorer
- Alien Crush
- China Warrior
- Military Madness
- JJ and Jeff
and said that six to ten more new game cards would be available by the end of the year.
It also shows game marquees for Victory Run and Power League.

In the February 1990 issue of Video Games & Computer Entertainment magazine, there was a list of
all of the available UA produced Hu-Cards (at the current time of the article was published).
The article mentions that the UA Hu-Cards were not compatible with the TG-16 gaming console.
- Keith Courage in the Alpha Zones
- World Class Baseball
- Blazing Lazers
- Alien Crush
- China Warrior

Blazing Lazers dump notes:
----------------------------------------------------------------------------
Game software
----------------------------------------------------------------------------

An EPROM-based HuCard manufactured by NEC (PCB ID: PWD-703) is used to
store the game program. It supports four uPD27C2001 (256Kx8) EPROMs as
well as 27C010s for total ROM capacity of 1MB down to 128K in various
configurations.

Jumper settings:

J1
Pos. 1 : Map IC1 to 000000-03FFFF (256K)
Pos. 2 : Map IC1 to 000000-01FFFF (128K)

J2
Pos. 1 : Map IC2 to 040000-07FFFF (256K)
Pos. 2 : Map IC2 to 080000-09FFFF (128K)
Pos. 3 : Map IC2 to 020000-03FFFF (128K)

J3
Pos. 1 : Map IC3 to 080000-0BFFFF (256K)
Pos. 2 : Map IC3 to 080000-09FFFF (128K)
Pos. 3 : Map IC3 to 040000-05FFFF (128K)

J4
Pos. 1 : Map IC4 to 0C0000-0FFFFF (256K)
Pos. 2 : Map IC4 to 0A0000-0BFFFF (128K)
Pos. 3 : Map IC4 to 060000-07FFFF (128K)

The board came with three EPROMs in the following configuration:

IC1 is a 27C010 mapped to 000000-01FFFF
IC2 is a 27C010 mapped to 020000-03FFFF
IC3 is a 27C010 mapped to 080000-09FFFF
IC4 is unpopulated

The software was a modified version of "Blazing Lazers", identical to the
USA retail version with the region check (offsets 0 through $18) patched
with NOP to remove it. The interface board is connected to a PC-Engine
console rather than a TurboGrafx-16, which would cause the region check to
fail.


Keith Courage In Alpha Zones: dump was made from PC-Engine game dump of US version,
 with region check nop'ed out.
Alien Crush & Pac_Land: dumps made from PC-Engine dumps of JP versions
*/

#include "emu.h"
#include "pcecommn.h"

#include "cpu/h6280/h6280.h"
#include "cpu/z80/z80.h"
#include "video/huc6260.h"
#include "video/huc6270.h"
#include "sound/discrete.h"

#include "screen.h"
#include "speaker.h"


namespace {

class uapce_state : public pce_common_state
{
public:
	uapce_state(const machine_config &mconfig, device_type type, const char *tag)
		: pce_common_state(mconfig, type, tag),
		m_discrete(*this, "discrete") { }

	void uapce(machine_config &config);

private:
	uint8_t m_jamma_if_control_latch = 0;
	void jamma_if_control_latch_w(uint8_t data);
	uint8_t jamma_if_control_latch_r();
	uint8_t jamma_if_read_dsw(offs_t offset);
	virtual uint8_t joy_read() override;
	virtual void machine_reset() override ATTR_COLD;
	required_device<discrete_device> m_discrete;

	void pce_io(address_map &map) ATTR_COLD;
	void pce_mem(address_map &map) ATTR_COLD;
	void z80_map(address_map &map) ATTR_COLD;
};

#define UAPCE_SOUND_EN  NODE_10
#define UAPCE_TONE_752  NODE_11

static DISCRETE_SOUND_START(uapce_discrete)
	DISCRETE_INPUT_LOGIC(UAPCE_SOUND_EN)
	DISCRETE_SQUAREWFIX(UAPCE_TONE_752, UAPCE_SOUND_EN, 752, DEFAULT_TTL_V_LOGIC_1, 50, DEFAULT_TTL_V_LOGIC_1, 0)   // 752Hz
	DISCRETE_OUTPUT(UAPCE_TONE_752, 100)
DISCRETE_SOUND_END


void uapce_state::jamma_if_control_latch_w(uint8_t data)
{
	uint8_t diff = data ^ m_jamma_if_control_latch;
	m_jamma_if_control_latch = data;

/*  D7 : Controls relay which connects the PCE R-AUDIO output to the common audio path.
    (1= Relay closed, 0= Relay open) */
	machine().sound().system_mute(!BIT(data, 7));

/* D6 : Output to JAMMA connector KEY pin. Connected to /RESET on the PCE backplane connector.
    (1= /RESET not asserted, 0= /RESET asserted) */

	if ( diff & 0x40 )
	{
		m_maincpu->set_input_line(INPUT_LINE_RESET, (data & 0x40) ? CLEAR_LINE : ASSERT_LINE);
	}

/* D5 : Connected to a TIP31 which may control the coin meter:
      B = Latched D5
      C = JAMMA pin 'z'
      E = Ground

   Pin 'z' is a normally ground connection, but on this board it is isolated from ground.
   The wiring harness also has the corresponding wire separate from the others. */
	machine().bookkeeping().coin_counter_w(0, BIT(data,5));

/* D4 : Connects the START1 switch input from the JAMMA connector to the
    "RUN" key input of the control pad multiplexer.
    (1= "RUN" input connected to START1 switch, 0= "RUN" input always '1') */

/* D3 : Controls the RESET input of timer B of a 556 IC, enabling a
      752 Hz (D-3) square wave to be output on the common audio path.
      (1= Tone output ON, 0= Tone output OFF) */

	m_discrete->write(UAPCE_SOUND_EN, BIT(data,3));

/* D2 : Not latched, though software writes to this bit like it is. */

/* D1 : Not latched. */

/* D0 : Not latched. */
}

uint8_t uapce_state::jamma_if_control_latch_r()
{
	return m_jamma_if_control_latch & 0x08;
}

uint8_t uapce_state::jamma_if_read_dsw(offs_t offset)
{
	uint8_t dsw_val;

	dsw_val = ioport("DSW" )->read();

	if ( BIT( offset, 7 ) == 0 )
	{
		dsw_val >>= 7;
	}
	else if ( BIT( offset, 6 ) == 0 )
	{
		dsw_val >>= 6;
	}
	else if ( BIT( offset, 5 ) == 0 )
	{
		dsw_val >>= 5;
	}
	else if ( BIT( offset, 4 ) == 0 )
	{
		dsw_val >>= 4;
	}
	else if ( BIT( offset, 3 ) == 0 )
	{
		dsw_val >>= 3;
	}
	else if ( BIT( offset, 2 ) == 0 )
	{
		dsw_val >>= 2;
	}
	else if ( BIT( offset, 1 ) == 0 )
	{
		dsw_val >>= 1;
	}
	else if ( BIT( offset, 0 ) == 0 )
	{
		dsw_val >>= 0;
	}

	return dsw_val & 1;
}

uint8_t uapce_state::joy_read()
{
	if ( m_jamma_if_control_latch & 0x10 )
	{
		return ioport("JOY" )->read();
	}
	else
	{
		return ioport("JOY" )->read() | 0x08;
	}
}

void uapce_state::machine_reset()
{
	m_jamma_if_control_latch = 0;
}

void uapce_state::z80_map(address_map &map)
{
	map(0x0000, 0x07FF).rom();
	map(0x0800, 0x0FFF).ram();
	map(0x1000, 0x17FF).w(FUNC(uapce_state::jamma_if_control_latch_w));
	map(0x1800, 0x1FFF).r(FUNC(uapce_state::jamma_if_read_dsw));
	map(0x2000, 0x27FF).portr("COIN");
	map(0x2800, 0x2FFF).r(FUNC(uapce_state::jamma_if_control_latch_r));
}


static INPUT_PORTS_START( uapce )
	PCE_STANDARD_INPUT_PORT_P1

	PORT_START( "DSW" )
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x38, 0x10, "Time" )
	PORT_DIPSETTING(    0x00, "Untimed Play" )
	PORT_DIPSETTING(    0x08, "1 minute Timed Play" )
	PORT_DIPSETTING(    0x10, "3 minute Timed Play" )
	PORT_DIPSETTING(    0x18, "5 minute Timed Play" )
	PORT_DIPSETTING(    0x20, "7 minute Timed Play" )
	PORT_DIPSETTING(    0x28, "10 minute Timed Play" )
	PORT_DIPSETTING(    0x30, "12 minute Timed Play" )
	PORT_DIPSETTING(    0x38, "20 minute Timed Play" )
	PORT_DIPNAME( 0x40, 0x40, "Buy-In Feature" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "COIN" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
INPUT_PORTS_END

void uapce_state::pce_mem(address_map &map)
{
	map(0x000000, 0x09FFFF).rom();
	map(0x1F0000, 0x1F1FFF).ram().mirror(0x6000);
	map(0x1FE000, 0x1FE3FF).rw("huc6270", FUNC(huc6270_device::read), FUNC(huc6270_device::write));
	map(0x1FE400, 0x1FE7FF).rw(m_huc6260, FUNC(huc6260_device::read), FUNC(huc6260_device::write));
}

void uapce_state::pce_io(address_map &map)
{
	map(0x00, 0x03).rw("huc6270", FUNC(huc6270_device::read), FUNC(huc6270_device::write));
}


void uapce_state::uapce(machine_config &config)
{
	/* basic machine hardware */
	H6280(config, m_maincpu, PCE_MAIN_CLOCK/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &uapce_state::pce_mem);
	m_maincpu->set_addrmap(AS_IO, &uapce_state::pce_io);
	m_maincpu->port_in_cb().set(FUNC(uapce_state::pce_joystick_r));
	m_maincpu->port_out_cb().set(FUNC(uapce_state::pce_joystick_w));
	m_maincpu->add_route(0, "speaker", 0.5, 0);
	m_maincpu->add_route(1, "speaker", 0.5, 1);

	z80_device &sub(Z80(config, "sub", 1400000));
	sub.set_addrmap(AS_PROGRAM, &uapce_state::z80_map);

	config.set_maximum_quantum(attotime::from_hz(60));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(PCE_MAIN_CLOCK, huc6260_device::WPF, 64, 64 + 1024 + 64, huc6260_device::LPF, 18, 18 + 242);
	screen.set_screen_update(FUNC(pce_common_state::screen_update));
	screen.set_palette("huc6260");

	HUC6260(config, m_huc6260, PCE_MAIN_CLOCK);
	m_huc6260->next_pixel_data().set("huc6270", FUNC(huc6270_device::next_pixel));
	m_huc6260->time_til_next_event().set("huc6270", FUNC(huc6270_device::time_until_next_event));
	m_huc6260->vsync_changed().set("huc6270", FUNC(huc6270_device::vsync_changed));
	m_huc6260->hsync_changed().set("huc6270", FUNC(huc6270_device::hsync_changed));

	huc6270_device &huc6270(HUC6270(config, "huc6270", 0));
	huc6270.set_vram_size(0x10000);
	huc6270.irq().set_inputline(m_maincpu, 0);

	SPEAKER(config, "speaker", 2).front();

	DISCRETE(config, m_discrete, uapce_discrete).add_route(0, "speaker", 1.00, 1);
}

ROM_START(blazlaz)
	ROM_REGION( 0x0a0000, "maincpu", 0 )
	ROM_LOAD( "ic1.bin", 0x000000, 0x020000, CRC(c86d44fe) SHA1(070512918e4d305db48bbda374eaff2d121909c5) )
	ROM_LOAD( "ic2.bin", 0x020000, 0x020000, CRC(fb813309) SHA1(60d1ea45717a04d776b6837377467e81431f9bc6) )
	ROM_LOAD( "ic3.bin", 0x080000, 0x020000, CRC(d30a2ecf) SHA1(8328b303e2ffeb694b472719e59044d41471725f) )

	ROM_REGION( 0x800, "sub", 0 )
	ROM_LOAD( "u1.bin", 0x0000, 0x800, CRC(f5e538a9) SHA1(19ac9525c9ad6bea1789cc9e63cdb7fe949867d9) )
ROM_END

ROM_START(keith)
	ROM_REGION( 0x0a0000, "maincpu", 0 )
	ROM_LOAD( "keith.ic1", 0x000000, 0x020000, BAD_DUMP CRC(d49d4e0d) SHA1(4ff8e7025dd1023e54a0f5bac52d7cecb7cb1430) )
	ROM_LOAD( "keith.ic2", 0x020000, 0x020000, BAD_DUMP CRC(133e5c8b) SHA1(8ebacd7b3887d10b28d8c672396c8850e8c0cdaf) )

	ROM_REGION( 0x800, "sub", 0 )
	ROM_LOAD( "u1.bin", 0x0000, 0x800, CRC(f5e538a9) SHA1(19ac9525c9ad6bea1789cc9e63cdb7fe949867d9) )
ROM_END

ROM_START(aliencr)
	ROM_REGION( 0x0a0000, "maincpu", 0 )
	ROM_LOAD( "aliencr.bin", 0x000000, 0x040000, CRC(60edf4e1) SHA1(a1e6ad82b66a82c25e0a9e25fb8be370f49c8c2d) )

	ROM_REGION( 0x800, "sub", 0 )
	ROM_LOAD( "u1.bin", 0x0000, 0x800, CRC(f5e538a9) SHA1(19ac9525c9ad6bea1789cc9e63cdb7fe949867d9) )
ROM_END

ROM_START(paclandp)
	ROM_REGION( 0x0a0000, "maincpu", 0 )
	ROM_LOAD( "paclandp.bin", 0x000000, 0x040000, CRC(14fad3ba) SHA1(fc0166da82ed3cf4a4e06fc6c73fd3184ba8bb3b) )

	ROM_REGION( 0x800, "sub", 0 )
	ROM_LOAD( "u1.bin", 0x0000, 0x800, CRC(f5e538a9) SHA1(19ac9525c9ad6bea1789cc9e63cdb7fe949867d9) )
ROM_END

} // anonymous namespace


GAME( 1989, blazlaz, 0, uapce, uapce, uapce_state, init_pce_common, ROT0, "Hudson Soft", "Blazing Lazers (United Amusements PC Engine)",                         MACHINE_IMPERFECT_SOUND )
GAME( 1989, keith,   0, uapce, uapce, uapce_state, init_pce_common, ROT0, "Hudson Soft", "Keith Courage In Alpha Zones (United Amusements PC Engine)",           MACHINE_IMPERFECT_SOUND )
GAME( 1989, aliencr, 0, uapce, uapce, uapce_state, init_pce_common, ROT0, "Hudson Soft", "Alien Crush (United Amusements PC Engine)",                            MACHINE_IMPERFECT_SOUND )
GAME( 1989, paclandp,0, uapce, uapce, uapce_state, init_pce_common, ROT0, "Namco",       "Pac-Land (United Amusements PC Engine)",                               MACHINE_IMPERFECT_SOUND )
