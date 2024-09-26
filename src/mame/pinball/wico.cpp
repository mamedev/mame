// license:BSD-3-Clause
// copyright-holders:Robbbert
/**************************************************************************

PINBALL
Schematic and PinMAME used as references.

Machines by this manufacturer: Af-tor, Big Top, Whirlwind Play Booster.

Af-Tor was the first pinball to use alphanumeric displays.
Each display has 12 segments, but are programmed with 8-bit codes.
This makes some of the letters look rather odd, but it is still readable.

         a a
         _ _
       f/_/_/ b    g = 2 middle horizontal segments
      e/_/_/ c     h = 2 middle vertical segments
        d d


Press num-0 to enter service/selftest. Press 1 to step through the tests.
When you reach the audit stages, press 6 to advance and 5 to clear.
In the switch test, it will report any closed dip as a failure. You can
ignore these.
The game has 2 balls, for multiball feature, so the outhole doesn't
work because it thinks the 2nd ball is in play somewhere.

To activate the end-of-ball, hold down X then tap END.


Status:
- Working

***************************************************************************/

#include "emu.h"
#include "genpin.h"

#include "cpu/m6809/m6809.h"
#include "machine/timer.h"
#include "sound/sn76496.h"
#include "speaker.h"

#include "wico.lh"

namespace {

class wico_state : public genpin_class
{
public:
	wico_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_ccpu(*this, "ccpu")
		, m_hcpu(*this, "hcpu")
		, m_shared_ram(*this, "sharedram")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void wico(machine_config &config);

private:
	u8 lampst_r();
	u16 seg8to14(u16 data);
	u8 switch_r(offs_t offset);
	void muxen_w(u8 data);
	void muxld_w(u8 data);
	void csols_w(u8 data);
	void msols_w(u8 data);
	void dled0_w(u8 data);
	void dled1_w(u8 data);
	void zcres_w(u8 data);
	void wdogcl_w(u8 data);
	u8 gentmrcl_r();
	TIMER_DEVICE_CALLBACK_MEMBER(irq_housekeeping);
	TIMER_DEVICE_CALLBACK_MEMBER(firq_housekeeping);
	void ccpu_map(address_map &map) ATTR_COLD;
	void hcpu_map(address_map &map) ATTR_COLD;

	bool m_zcen = false;
	bool m_gten = false;
	bool m_disp_on = false;
	bool m_diag_on = false;
	u8 m_firqtimer = 0U;
	u8 m_diag_segments = 0U;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_ccpu;
	required_device<cpu_device> m_hcpu;
	required_shared_ptr<u8> m_shared_ram;
	required_ioport_array<16> m_io_keyboard;
	output_finder<48> m_digits;
	output_finder<176> m_io_outputs;   // (8+36) solenoids + 4 spares + 128 lamps
};

// housekeeping cpu
void wico_state::hcpu_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("sharedram");
	map(0x1fe0, 0x1fe0).w(FUNC(wico_state::muxld_w));
	//map(0x1fe1, 0x1fe1).w(FUNC(wico_state::store_w));
	map(0x1fe2, 0x1fe2).w(FUNC(wico_state::muxen_w));
	//map(0x1fe3, 0x1fe3).w(FUNC(wico_state::csols_w));
	map(0x1fe4, 0x1fe4).noprw();
	map(0x1fe5, 0x1fe5).w("sn76494", FUNC(sn76494_device::write));
	map(0x1fe6, 0x1fe6).w(FUNC(wico_state::wdogcl_w));
	map(0x1fe7, 0x1fe7).w(FUNC(wico_state::zcres_w));
	map(0x1fe8, 0x1fe8).w(FUNC(wico_state::dled0_w));
	map(0x1fe9, 0x1fe9).w(FUNC(wico_state::dled1_w));
	map(0x1fea, 0x1fea).r(FUNC(wico_state::gentmrcl_r));
	map(0x1feb, 0x1feb).r(FUNC(wico_state::lampst_r));
	//map(0x1fec, 0x1fec).r(FUNC(wico_state::sast_r));
	//map(0x1fed, 0x1fed).r(FUNC(wico_state::solst1_r));
	//map(0x1fee, 0x1fee).r(FUNC(wico_state::solst0_r));
	map(0x1fef, 0x1fef).r(FUNC(wico_state::switch_r));
	map(0xf000, 0xffff).rom().region("hcpu", 0);
}

// command cpu
void wico_state::ccpu_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("sharedram"); // 2128  2k RAM
	//map(0x1fe0, 0x1fe0).w(FUNC(wico_state::muxld_w)); // to display module
	//map(0x1fe1, 0x1fe1).w(FUNC(wico_state::store_w)); // enable save to nvram
	map(0x1fe2, 0x1fe2).w(FUNC(wico_state::muxen_w)); // digit to display on diagnostic LED; d0=L will disable main displays
	map(0x1fe3, 0x1fe3).w(FUNC(wico_state::csols_w)); // continuous solenoids
	map(0x1fe4, 0x1fe4).w(FUNC(wico_state::msols_w)); // matrix solenoids
	map(0x1fe5, 0x1fe5).w("sn76494", FUNC(sn76494_device::write));
	map(0x1fe6, 0x1fe6).w(FUNC(wico_state::wdogcl_w)); // watchdog clear
	map(0x1fe7, 0x1fe7).w(FUNC(wico_state::zcres_w)); // enable IRQ on hcpu
	map(0x1fe8, 0x1fe8).w(FUNC(wico_state::dled0_w)); // turn off diagnostic LED
	map(0x1fe9, 0x1fe9).w(FUNC(wico_state::dled1_w)); // turn on diagnostic LED
	map(0x1fea, 0x1fea).r(FUNC(wico_state::gentmrcl_r)); // enable IRQ on ccpu
	//map(0x1feb, 0x1feb).r(FUNC(wico_state::lampst_r)); // lamps?
	//map(0x1fec, 0x1fec).r(FUNC(wico_state::sast_r)); // a pwron pulse to d0 L->H
	//map(0x1fed, 0x1fed).r(FUNC(wico_state::solst1_r)); // switches
	//map(0x1fee, 0x1fee).r(FUNC(wico_state::solst0_r)); // switches
	//map(0x1fef, 0x1fef).r(FUNC(wico_state::switch_r)); // switches
	map(0x4000, 0x40ff).ram().share("nvram"); // X2212 4bit x 256 NVRAM, stores only when store_w is active
	map(0x8000, 0x9fff).rom().region("ccpu", 0);
	map(0xe000, 0xffff).rom().region("ccpu", 0x2000);
}

static INPUT_PORTS_START( wico )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) // Clear button
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) // Advance button
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Top Lane 4") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Top Lane 3") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Top Lane 2") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Top Lane 1") PORT_CODE(KEYCODE_D)
	PORT_START("X1")
	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Bumper BR") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Bumper BL") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Bumper TR") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Bumper TL") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Target BR") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Target BL") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Target TR") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Target TL") PORT_CODE(KEYCODE_L)
	PORT_START("X3")
	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Oil Pit Release") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Oil Pit Target") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Waterhole Release") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Outhole") PORT_CODE(KEYCODE_X) // press END as well
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("L Spinner") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("R Rollunder") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("R Spinner") PORT_CODE(KEYCODE_R)
	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("L SLingshot") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("R Slingshot") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("R Drop Bank E") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("R Drop Bank P") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("R Drop Bank A") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("L Drop Bank C") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("L Drop Bank S") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("L Drop Bank E") PORT_CODE(KEYCODE_COMMA)
	PORT_START("X6")
	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("R Target Zone E") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("R Target Zone D") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("R Target Zone I") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("R Target Zone R") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("L Target Zone T") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("L Target Zone S") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("L Target Zone A") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("L Target Zone F") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_START("X8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("R Outlane Target") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("L Outlane Target") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("R Rollover R Outlane") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("R Rollover M Outlane") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("R Rollover L Outlane") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("L Rollover R Outlane") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("L Rollover M Outlane") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("L Rollover L Outlane") PORT_CODE(KEYCODE_PGUP)
	PORT_START("X9")
	PORT_START("X10")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Door Slam")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Playfield Tilt")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Pendulum Tilt")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("R Flipper Lane Change") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Ball Feed Middle") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Ball Feed Lower") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("10 points") PORT_CODE(KEYCODE_HOME)
	PORT_START("X11")
	PORT_START("X12")
	PORT_DIPNAME( 0x0f, 0x00, "Chute 1" )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, "1 coin 10 credits" )
	PORT_DIPSETTING(    0x08, "1 coin 14 credits" )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x0c, "2 coins 1 credit then 2 coins for 2 more credits" )
	PORT_DIPSETTING(    0x0d, "1 coin 1 credit up to 3, then 4th coin gives 2 credits" )
	PORT_DIPSETTING(    0x0e, "1 coin 1 credit, next coin gives 2 credits" )
	PORT_DIPSETTING(    0x0f, "1st coin 1 credit, 2nd coin 2 more credits, 3rd coin 1 credit, 4th coin 3 more credits" )
	PORT_DIPNAME( 0x10, 0x00, "Release Targets Spot Wico" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPNAME( 0x20, 0x00, "Saving Top Lane Lights" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPNAME( 0x40, 0x00, "Fast Ride Lights Extra Ball" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPNAME( 0x80, 0x00, "Free Play" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_START("X13")
	PORT_DIPNAME( 0x0f, 0x00, "Chute 2" )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, "1 coin 10 credits" )
	PORT_DIPSETTING(    0x08, "1 coin 14 credits" )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x0c, "2 coins 1 credit then 2 coins for 2 more credits" )
	PORT_DIPSETTING(    0x0d, "1 coin 1 credit up to 3, then 4th coin gives 2 credits" )
	PORT_DIPSETTING(    0x0e, "1 coin 1 credit, next coin gives 2 credits" )
	PORT_DIPSETTING(    0x0f, "1st coin 1 credit, 2nd coin 2 more credits, 3rd coin 1 credit, 4th coin 3 more credits" )
	PORT_DIPNAME( 0x10, 0x00, "Drain eject hole on outhole" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "2x multi during multiball" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPNAME( 0xc0, 0x40, "Credits for new High score" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0xc0, "0" )
	PORT_START("X14")
	PORT_DIPNAME( 0x01, 0x00, "Reset left outlane gate" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x02, "Maximum credits" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPSETTING(    0x02, "15" )
	PORT_DIPSETTING(    0x04, "25" )
	PORT_DIPSETTING(    0x06, "40" )
	PORT_DIPNAME( 0x08, 0x00, "Extra ball allowed" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x00, "Level pass payout" )
	PORT_DIPSETTING(    0x00, "Free Game" )
	PORT_DIPSETTING(    0x10, "Extra Ball" )
	PORT_DIPNAME( 0x20, 0x20, "Capture ball in multiball" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Saves multi-lights" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPNAME( 0x80, 0x80, "One extra ball per ball" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_START("X15")
	PORT_DIPNAME( 0x03, 0x00, "Special bonus" )
	PORT_DIPSETTING(    0x00, "Free Game" )
	PORT_DIPSETTING(    0x01, "Extra Ball" )
	PORT_DIPSETTING(    0x02, "100k points" )
	PORT_DIPSETTING(    0x03, "Nothing" )
	PORT_DIPNAME( 0x04, 0x00, "Flip special lights" )
	PORT_DIPSETTING(    0x00, "Alternate" )
	PORT_DIPSETTING(    0x04, "Both on" )
	PORT_DIPNAME( 0x18, 0x08, "Balls" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPNAME( 0x20, 0x00, "Disable Credits display" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Disable Match display" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	// This is a dip and a pushbutton in parallel.
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Self Test")
INPUT_PORTS_END

// diagnostic display off
void wico_state::dled0_w(u8 data)
{
	m_diag_on = 0;
	m_digits[9] = 0;
}

// diagnostic display on
void wico_state::dled1_w(u8 data)
{
	m_diag_on = 1;
	m_digits[9] = m_diag_segments;
}

// Continuous solenoids (d7 = flipper enable)
// 7 solenoids + flipen
void wico_state::csols_w(u8 data)
{
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[i] = BIT(data, i);
}

// Matrix solenoids
// d0,1,2 = 1 to 6; d3,4,5 = 1 to 6; total 36 solenoids
void wico_state::msols_w(u8 data)
{
	u8 k = BIT(data, 3, 3) - 1;
	if (k < 6)
		for (u8 i = 0; i < 6; i++)
			m_io_outputs[8+k*6+i] = (BIT(data, 0, 3) == (i+1));
	// Outhole
	if (data == 0x1c)
		m_samples->start(0, 5);
}

// write to diagnostic display
void wico_state::muxen_w(u8 data)
{
	static const u8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71 }; // MC14495

	m_diag_segments = patterns[data>>4];

	if (m_diag_on)
		m_digits[9] = m_diag_segments;
	else
		m_digits[9] = 0;

	m_disp_on = BIT(data, 0);
}

// reset digit/scan counter
void wico_state::muxld_w(u8 data)
{
}

// enable zero-crossing interrupt
void wico_state::zcres_w(u8 data)
{
	m_zcen = 1;
}

// enable firq
u8 wico_state::gentmrcl_r()
{
	m_gten = 1;
	return 0xff;
}

// read a switch row
u8 wico_state::switch_r(offs_t offset)
{
	offset = m_shared_ram[0x95];
	u8 data = m_io_keyboard[offset]->read();

	// Reflex solenoids - operated directly by the switches without needing the cpu
	if ((offset==2) && (data & 15))
		m_samples->start(0, 0); // bumpers
	else
	if ((offset==5) && (data & 3))
		m_samples->start(1, 7); // slings

	return data;
}

// write digits in main display
u8 wico_state::lampst_r()
{
	u8 i, j;
	for (i = 0; i < 5; i++)
	{
		if (m_disp_on)
			j = m_shared_ram[0x7f9 + i];
		else
			j = 0;
		m_digits[i * 10 + (m_shared_ram[0x96] & 7)] = seg8to14(j);
	}
	// Lamps
	for (i = 0; i < 16; i++)
	{
		u8 k = m_shared_ram[0x46+i];
		for (j = 0; j < 8; j++)
			m_io_outputs[48+i*8+j] = BIT(k, j);
	}

	return 0xff;
}

// convert custom 8seg digit to MAME 14seg digit
u16 wico_state::seg8to14(u16 data)
{
	return bitswap<10>(data,7,7,6,6,5,4,3,2,1,0);
}

// reset watchdog and enable housekeeping cpu
void wico_state::wdogcl_w(u8 data)
{
	m_hcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}


TIMER_DEVICE_CALLBACK_MEMBER( wico_state::irq_housekeeping )
{
	if (m_zcen)
		m_hcpu->set_input_line(M6809_IRQ_LINE, HOLD_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER( wico_state::firq_housekeeping )
{
	if (m_gten)
		m_hcpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);

	// Gen. timer irq of command CPU kicks in every 4 interrupts of this timer
	m_firqtimer++;
	if (m_firqtimer > 3) // divided by 4 by U2 74LS393.
	{
		m_ccpu->set_input_line(M6809_IRQ_LINE, HOLD_LINE);
		m_firqtimer = 0;
	}
}

void wico_state::machine_start()
{
	genpin_class::machine_start();

	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_zcen));
	save_item(NAME(m_gten));
	save_item(NAME(m_disp_on));
	save_item(NAME(m_diag_on));
	save_item(NAME(m_firqtimer));
	save_item(NAME(m_diag_segments));
}

void wico_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_hcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_zcen = 0;
	m_gten = 0;
	m_firqtimer = 0;
	m_disp_on = 0;
	m_diag_on = 0;
}


void wico_state::wico(machine_config &config)
{
	/* basic machine hardware */
	MC6809E(config, m_ccpu, XTAL(10'000'000) / 8); // MC68A09EP @ U51
	m_ccpu->set_addrmap(AS_PROGRAM, &wico_state::ccpu_map);

	MC6809E(config, m_hcpu, XTAL(10'000'000) / 8); // MC68A09EP @ U24
	m_hcpu->set_addrmap(AS_PROGRAM, &wico_state::hcpu_map);

	TIMER(config, "irq").configure_periodic(FUNC(wico_state::irq_housekeeping), attotime::from_hz(120)); // zero crossing
	TIMER(config, "firq").configure_periodic(FUNC(wico_state::firq_housekeeping), attotime::from_hz(750)); // time generator
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_wico);

	/* Sound */
	genpin_audio(config);
	SPEAKER(config, "mono").front_center();
	SN76494(config, "sn76494", XTAL(10'000'000) / 64).add_route(ALL_OUTPUTS, "mono", 0.75);
}

/*-------------------------------------------------------------------
/ Af-Tor (1984)
/-------------------------------------------------------------------*/
ROM_START(aftor)
	ROM_REGION(0x1000, "hcpu", 0)
	ROM_LOAD("u25.bin", 0x0000, 0x1000, CRC(d66e95ff) SHA1(f7e8c51f1b37e7ef560406f1968c12a2043646c5))

	ROM_REGION(0x4000, "ccpu", 0)
	ROM_LOAD("u52.bin", 0x0000, 0x2000, CRC(8035b446) SHA1(3ec59015e259c315bf09f4e2046f9d98e2d7a732))
	ROM_LOAD("u48.bin", 0x2000, 0x2000, CRC(b4406563) SHA1(6d1a9086eb1f6f947eae3a92ccf7a9b7375d85d3))
ROM_END

/*-------------------------------------------------------------------
/ Big Top  (1977)
/-------------------------------------------------------------------*/

} // Anonymous namespace

GAME(1984,  aftor,  0,  wico,  wico, wico_state, empty_init, ROT0,  "Wico", "Af-Tor", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
