// license:BSD-3-Clause
// copyright-holders:David Haywood, R.Belmont

/*
    These use a 6502 derived CPU under a glob
    The CPU die is marked 'ELAN EU3A14'

    There is a second glob surrounded by TSOP48 pads
    this contains the ROM

    Known to be on this hardware

    name                          PCB ID      ROM width   TSOP pads   ROM size        SEEPROM         die markings
    Golden Tee Golf Home Edition  ?           x16         48          4MB             no              ELAN EU3A14   (developed by FarSight Studios)
    Real Swing Golf               74037       x16         48          4MB             no              ELAN EU3A14   (developed by FarSight Studios)
    Baseball 3                    ?           x16         48          4MB             no              ELAN EU3A14   (developed by FarSight Studios)
    Connectv Football             ?           x16         48          4MB             no              ELAN EU3A14   (developed by Medialink)
    Huntin’3                      ?           x16         48          4MB             no              Elan ?        (developed by V-Tac Technology Co Ltd.)
    Play TV Basketball            75029       x16         48          4MB             no              ELAN EU3A14

    In many ways this is similar to the rad_eu3a05.cpp hardware
    but the video system has changed, here the sprites are more traditional non-tile based, rather
    than coming from 'pages'

    --

    Compared to the XaviXport games camera hookups, Real Swing Golf just has 6 wires, Its camera PCB is the only one with a ceramic resonator.
    Maybe the CU5502 chip offloads some processing from the CPU?

    The Basketball camera also uses an ETOMS CU5502.  It’s different from the others (XaviXport + Real Swing Golf) in that the sensor is on a small PCB with
    a 3.58MHz resonator with 16 wires going to another small PCB that has a glob and a 4MHz resonator.  6 wires go from that PCB to the main game PCB.

    To access hidden test mode in Football hold enter and right during power on.

    Football test mode tests X pos, Y pos, Z pos, direction and speed.  This data must all be coming from the camera in the unit as the shinpads are simply
    reflective objects, they don't contain any electronics.  It could be a useful test case for better understanding these things.

    To access hidden test mode in Golden Tee Home hold back/backspin and left during power on.

    To access hidden test mode in Basketball hold left and Button 1 during power on.

    To access hidden test mode in Real Swing Golf hold left and down during power on.
     - test mode check
     77B6: lda $5041
     77B9: eor #$ed
     77BB: beq $77be

    To access hidden test mode in Baseball 3 hold down during power on.
    - test mode check
    686E: lda $5041
    6871: eor #$f7
    6873: bne $68c8

    It is not clear how to access Huntin'3 Test Mode (if possible) there do appear to be tiles for it tho

    Huntin'3 makes much more extensive use of the video hardware than the other titles, including
     - Table based Rowscroll (most first person views)
     - RAM based tiles (status bar in "Target Range", text descriptions on menus etc.)
     - Windowing effects (to highlight menu items, timer in "Target Range") NOT YET EMULATED / PROPERLY UNDERSTOOD

*/

#include "emu.h"

#include "screen.h"

#include "machine/elan_eu3a14_soc.h"
#include "machine/timer.h"

namespace {

class elan_eu3a14_state : public driver_device
{
public:
	elan_eu3a14_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }


	void radica_eu3a14(machine_config &config);
	void radica_eu3a14p(machine_config &config);

	void radica_eu3a14p_altrambase(machine_config &config);
	void radica_eu3a14_altrambase(machine_config &config);
	void radica_eu3a14_altrambase_adc(machine_config &config);

	void radica_eu3a14_altrambase_bb3(machine_config &config);
	void radica_eu3a14p_altrambase_bb3(machine_config &config);

	void radica_eu3a14_altspritebase(machine_config &config);
	void radica_eu3a14_altspritebase_bat(machine_config &config);

	int tsbuzz_inputs_r();


private:
	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(interrupt);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);

	void external_map_8mb(address_map &map) ATTR_COLD;
	void external_map_4mb(address_map &map) ATTR_COLD;

	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<elan_eu3a14_soc_device> m_maincpu;
	required_device<screen_device> m_screen;
};





uint32_t elan_eu3a14_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return m_maincpu->screen_update(screen, bitmap, cliprect);
}



void elan_eu3a14_state::external_map_8mb(address_map &map)
{
	map(0x000000, 0x7fffff).rom().region("maincpu", 0);
}

void elan_eu3a14_state::external_map_4mb(address_map &map)
{
	map(0x000000, 0x3fffff).mirror(0x400000).rom().region("maincpu", 0);
}

static INPUT_PORTS_START( eu3a14 )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

int elan_eu3a14_state::tsbuzz_inputs_r()
{
	// Test mode shows 8 directions, 2 buttons and a trigger
	// presumably the motion detection from the peripherals gets converted
	// into basic input directions before reaching the SoC

	// TODO: figure out protocol
	return machine().rand();
}

static INPUT_PORTS_START( tsbuzz )
	PORT_INCLUDE( eu3a14 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) // pause(?), hold this on startup for test mode
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) // cheat? skips levels
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(elan_eu3a14_state::tsbuzz_inputs_r)) // all other inputs read through here
INPUT_PORTS_END


static INPUT_PORTS_START( rad_gtg )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) // back / backspin
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) // up and down in the menus should be the trackball
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x08, 0x08, "Track Y test" ) // trackball up/down direction bit? (read in interrupt, increases / decreases a counter)
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x20, 0x20, "Track X test" ) // trackball left / right direction bit? (read in interrupt, increases / decreases a counter)
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( rad_rsg ) // base unit just has 4 directions + enter and a sensor to swing the club over
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) // aiming
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) // select in menus?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) // previous in menus?
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) // next in menus?
	PORT_DIPNAME( 0x20, 0x20, "IN0" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( radica_foot )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) // enter?
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( radica_hnt3 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Menu Previous")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Menu Next")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu Select")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 ) // pause?

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Fire Gun") // maybe
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Safety") PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Fire Gun (alt)") // maybe
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( radica_bask )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x08, 0x08, "IN0" )
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( radica_bb3 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void elan_eu3a14_state::machine_start()
{
}

void elan_eu3a14_state::machine_reset()
{
}


TIMER_DEVICE_CALLBACK_MEMBER(elan_eu3a14_state::scanline_cb)
{
	// these interrupts need to occur based on how fast the trackball is
	// being moved, the direction is read in a port.
	int scanline = param;

	if (scanline == 20)
	{
		// vertical trackball
		m_maincpu->generate_custom_interrupt(12);
	}

	if (scanline == 40)
	{
		// horizontal trackball
		m_maincpu->generate_custom_interrupt(13);

	}
}

INTERRUPT_GEN_MEMBER(elan_eu3a14_state::interrupt)
{
	m_maincpu->generate_custom_interrupt(9);
}


void elan_eu3a14_state::radica_eu3a14(machine_config &config)
{
	ELAN_EU3A14_SOC(config, m_maincpu, XTAL(21'477'272)/2); // marked as 21'477'270
	m_maincpu->set_addrmap(elan_eu3a14_soc_device::AS_EXTERNAL, &elan_eu3a14_state::external_map_4mb);
	m_maincpu->set_vblank_int("screen", FUNC(elan_eu3a14_state::interrupt));
	m_maincpu->set_default_spriteramaddr(0x14);
	m_maincpu->set_tilerambase(0x0000);
	m_maincpu->set_screen("screen");
	m_maincpu->read_callback<0>().set_ioport("IN0");
	m_maincpu->read_callback<1>().set_ioport("IN1");
	m_maincpu->read_callback<2>().set_ioport("IN2");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_screen_update(FUNC(elan_eu3a14_state::screen_update));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 28*8-1);
}

void elan_eu3a14_state::radica_eu3a14_altspritebase(machine_config &config)
{
	radica_eu3a14(config);
	m_maincpu->set_addrmap(elan_eu3a14_soc_device::AS_EXTERNAL, &elan_eu3a14_state::external_map_8mb);
	m_maincpu->set_default_spriteramaddr(0x04); // at 0x800
}

void elan_eu3a14_state::radica_eu3a14_altspritebase_bat(machine_config &config)
{
	radica_eu3a14(config);
	m_maincpu->set_addrmap(elan_eu3a14_soc_device::AS_EXTERNAL, &elan_eu3a14_state::external_map_8mb);
	m_maincpu->set_default_spriteramaddr(0x0c); // at 0x1800
}


void elan_eu3a14_state::radica_eu3a14_altrambase(machine_config &config)
{
	radica_eu3a14(config);
	m_maincpu->set_tilerambase(0x0800);
}

void elan_eu3a14_state::radica_eu3a14_altrambase_bb3(machine_config &config)
{
	radica_eu3a14_altrambase(config);
	m_maincpu->disable_timer_irq();
}

void elan_eu3a14_state::radica_eu3a14_altrambase_adc(machine_config &config)
{
	radica_eu3a14_altrambase(config);

	TIMER(config, "scantimer").configure_scanline(FUNC(elan_eu3a14_state::scanline_cb), "screen", 0, 1);
}


void elan_eu3a14_state::radica_eu3a14p(machine_config &config) // TODO, clocks differ too, what are they on PAL?
{
	radica_eu3a14(config);
	m_maincpu->set_is_pal();
	m_screen->set_refresh_hz(50);
}

void elan_eu3a14_state::radica_eu3a14p_altrambase(machine_config &config)
{
	radica_eu3a14p(config);
	m_maincpu->set_tilerambase(0x0800);
}

void elan_eu3a14_state::radica_eu3a14p_altrambase_bb3(machine_config &config)
{
	radica_eu3a14p_altrambase(config);
	m_maincpu->disable_timer_irq();
}

ROM_START( rad_gtg )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "goldentee.bin", 0x000000, 0x400000, CRC(2d6cdb85) SHA1(ce6ed39d692ff16ea407f39c37b6e731f952b9d5) )
ROM_END

ROM_START( rad_rsg )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "realswinggolf.bin", 0x000000, 0x400000, CRC(89e5b6a6) SHA1(0b14aa84d7e7ae7190cd64e3eb125de2104342bc) )
ROM_END

ROM_START( rad_rsgp )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "realswinggolf.bin", 0x000000, 0x400000, CRC(89e5b6a6) SHA1(0b14aa84d7e7ae7190cd64e3eb125de2104342bc) )
ROM_END

ROM_START( rad_rsgpa )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "realswinggolf_multilanguage.bin", 0x000000, 0x400000, CRC(c03752a6) SHA1(7e9cc804edf0c23a8dedfa0c0a51d1bc811ea5c1) )
ROM_END

ROM_START( rad_foot )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "connectvfootball.bin", 0x000000, 0x400000, CRC(00ac4fc0) SHA1(2b60ae5c6bc7e9ef7cdbd3f6a0a0657ed3ab5afe) )
ROM_END

ROM_START( rad_bb3 )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "baseball3.bin", 0x000000, 0x400000, CRC(af86aab0) SHA1(5fed48a295f045ca839f87b0f9b78ecc51104cdc) )
ROM_END

ROM_START( rad_bb3p )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "baseball3.bin", 0x000000, 0x400000, CRC(af86aab0) SHA1(5fed48a295f045ca839f87b0f9b78ecc51104cdc) )
ROM_END

ROM_START( rad_hnt3 )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "huntin3.bin", 0x000000, 0x400000, CRC(c8e3e40b) SHA1(81eb16ac5ab6d93525fcfadbc6703b2811d7de7f) )
ROM_END

ROM_START( rad_hnt3p )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "huntin3.bin", 0x000000, 0x400000, CRC(c8e3e40b) SHA1(81eb16ac5ab6d93525fcfadbc6703b2811d7de7f) )
ROM_END

ROM_START( rad_bask )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "basketball.bin", 0x000000, 0x400000, CRC(7d6ff53c) SHA1(1c75261d55e0107a3b8e8d4c1eb2854750f2d0e8) )
ROM_END

ROM_START( rad_baskp )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "basketball.bin", 0x000000, 0x400000, CRC(7d6ff53c) SHA1(1c75261d55e0107a3b8e8d4c1eb2854750f2d0e8) )
ROM_END

/*

  The Interactive M.A.G. Motion Activated Gear titles use globtops with an unusual square pinout
  for the main ROM

  10   01
  +------\
11|      |48
  |      |
  |      |
24+------+35
  25    34


01 | A10
02 | A09
03 | A08
04 | A19
05 | A21
06 | A20
07 | A18
08 | A17
09 | A07
10 | A06
11 | A05
12 | A04
13 | A00
14 | A01
15 | A02
16 | A03
17 | /CE
18 | N/C
19 | D08
20 | D00
21 | N/C
22 | N/C
23 | D01
24 | D09
25 | D02
26 | D10
27 | D03
28 | D11
29 | N/C
30 | VCC
31 | VCC
32 | D04
33 | D12
34 | D05
35 | D13
36 | D06
37 | D15
38 | GND
39 | D07
40 | D14
41 | GND
42 | VCC
43 | A13
44 | A14
45 | A16
46 | A15
47 | A12
48 | A11

*/

ROM_START( tsbuzz )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "toystory_buzz.bin", 0x000000, 0x800000, CRC(8d727ed4) SHA1(228e1d788cdbaf251e15dba01b6c71e82197ea28) )
ROM_END

ROM_START( batvgc )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "batvgc.bin", 0x000000, 0x800000, CRC(513a5625) SHA1(d8db60818a4452e665c312b8b93642d8b2b33c8f) )
ROM_END

ROM_START( spidtt )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "mag_spidtt", 0x000000, 0x800000, CRC(05de01de) SHA1(f2891d6e743abdd7bb50d0bb84701b18225a0a7a) )
ROM_END

ROM_START( teentit )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "teentitansmag.bin", 0x000000, 0x800000, CRC(6087c7ca) SHA1(0bf639218c5f25449f4f98d5c3659a2311a28f72) )
ROM_END

} // anonymous namespace


CONS( 2006, rad_gtg,  0,        0, radica_eu3a14_altrambase_adc, rad_gtg,       elan_eu3a14_state, empty_init,  "Radica / FarSight Studios (licensed from Incredible Technologies)", "Golden Tee Golf: Home Edition", MACHINE_NOT_WORKING )

CONS( 2005, rad_rsg,  0,        0, radica_eu3a14_altrambase,     rad_rsg,       elan_eu3a14_state, empty_init,  "Radica / FarSight Studios",                                         "Play TV Real Swing Golf", MACHINE_NOT_WORKING )
CONS( 2005, rad_rsgp, rad_rsg,  0, radica_eu3a14p_altrambase,    rad_rsg,       elan_eu3a14_state, empty_init,  "Radica / FarSight Studios",                                         "Connectv Real Swing Golf (set 1)", MACHINE_NOT_WORKING ) // English only
CONS( 2005, rad_rsgpa,rad_rsg,  0, radica_eu3a14p_altrambase,    rad_rsg,       elan_eu3a14_state, empty_init,  "Radica / FarSight Studios",                                         "Connectv Real Swing Golf (set 2)", MACHINE_NOT_WORKING ) // English, German, French, Spanish, Italian

// also has a Connectv Real Soccer logo in the roms, apparently unused, maybe that was to be the US title (without the logo being changed to Play TV) but Play TV Soccer ended up being a different game licensed from Epoch instead.
CONS( 2006, rad_foot, 0,        0, radica_eu3a14p,               radica_foot,   elan_eu3a14_state, empty_init,  "Radica / Medialink",                                                "Connectv Football", MACHINE_NOT_WORKING )

CONS( 2005, rad_bb3,  0,        0, radica_eu3a14_altrambase_bb3,  radica_bb3,    elan_eu3a14_state, empty_init,  "Radica / FarSight Studios",                                         "Play TV Baseball 3", MACHINE_NOT_WORKING )
CONS( 2005, rad_bb3p, rad_bb3,  0, radica_eu3a14p_altrambase_bb3, radica_bb3,    elan_eu3a14_state, empty_init,  "Radica / FarSight Studios",                                         "Connectv Baseball 3", MACHINE_NOT_WORKING )

CONS( 2005, rad_hnt3, 0,        0, radica_eu3a14,                radica_hnt3,   elan_eu3a14_state, empty_init,  "Radica / V-Tac Technology Co Ltd.",                                 "Play TV Huntin' 3", MACHINE_NOT_WORKING )
CONS( 2005, rad_hnt3p,rad_hnt3, 0, radica_eu3a14p,               radica_hnt3,   elan_eu3a14_state, empty_init,  "Radica / V-Tac Technology Co Ltd.",                                 "Connectv Huntin' 3", MACHINE_NOT_WORKING )

CONS( 2005, rad_bask, 0,        0, radica_eu3a14_altrambase,     radica_bask,   elan_eu3a14_state, empty_init,  "Radica / FarSight Studios",                                         "Play TV Basketball", MACHINE_NOT_WORKING )
CONS( 2005, rad_baskp,rad_bask, 0, radica_eu3a14p_altrambase,    radica_bask,   elan_eu3a14_state, empty_init,  "Radica / FarSight Studios",                                         "Connectv Basketball", MACHINE_NOT_WORKING )

CONS( 200?, tsbuzz,   0,        0, radica_eu3a14_altspritebase,      tsbuzz,    elan_eu3a14_state, empty_init,  "Thinkway Toys",                                                     "Interactive M.A.G. Motion Activated Gear: Toy Story and Beyond! Buzz Lightyear Galactic Adventure", MACHINE_NOT_WORKING )
CONS( 200?, batvgc,   0,        0, radica_eu3a14_altspritebase_bat,  tsbuzz,    elan_eu3a14_state, empty_init,  "Thinkway Toys",                                                     "Interactive M.A.G. Motion Activated Gear: The Batman - Villains of Gotham City", MACHINE_NOT_WORKING )
CONS( 200?, spidtt,   0,        0, radica_eu3a14_altspritebase_bat,  tsbuzz,    elan_eu3a14_state, empty_init,  "Thinkway Toys",                                                     "Interactive M.A.G. Motion Activated Gear: Spider-Man - Triple Threat", MACHINE_NOT_WORKING )
CONS( 200?, teentit,  0,        0, radica_eu3a14_altspritebase_bat,  tsbuzz,    elan_eu3a14_state, empty_init,  "Thinkway Toys",                                                     "Interactive M.A.G. Motion Activated Gear: Teen Titans Arena Showdown", MACHINE_NOT_WORKING )

// the following Thinkway Toys 'MAG' products likely also fit here
// MAG: Superman Fight for Metropolis
// MAG: Disney Pixar Cars I Am Speed
