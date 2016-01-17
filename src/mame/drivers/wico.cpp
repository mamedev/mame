// license:BSD-3-Clause
// copyright-holders:Robbbert
/**************************************************************************

    PINBALL
    Wico's only game : Af-tor

    Schematic and PinMAME used as references.
    Code for the interrupts/timers was derived from PinMAME.

    Af-Tor was the first pinball to use alphanumeric displays.
    Each display has 12 segments, but are programmed with 8-bit codes.
    This makes some of the letters look rather odd, but it is still
    readable.

         a a
         _ _
       f/_/_/ b    g = 2 middle horizontal segments
      e/_/_/ c     h = 2 middle vertical segments
        d d


    Press 9 to enter service/selftest. Press 1 to step through the tests.
    When you reach the audit stages, press 6 to advance and 5 to clear.
    In the switch test, it will report any closed dip as a failure. You can
    ignore these.
    The game has 2 balls, for multiball feature, so the outhole doesn't
    work because it thinks the 2nd ball is in play somewhere.


ToDo:
- Add outhole/saucer sound




***************************************************************************/

#include "machine/genpin.h"
#include "cpu/m6809/m6809.h"
#include "sound/sn76496.h"
#include "wico.lh"


class wico_state : public genpin_class
{
public:
	wico_state(const machine_config &mconfig, device_type type, std::string tag)
		: genpin_class(mconfig, type, tag)
		, m_ccpu(*this, "ccpu")
		, m_hcpu(*this, "hcpu")
		, m_shared_ram(*this, "sharedram")
	{ }

	DECLARE_READ8_MEMBER(lampst_r);
	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_WRITE8_MEMBER(muxen_w);
	DECLARE_WRITE8_MEMBER(muxld_w);
	DECLARE_WRITE8_MEMBER(csols_w);
	DECLARE_WRITE8_MEMBER(msols_w);
	DECLARE_WRITE8_MEMBER(dled0_w);
	DECLARE_WRITE8_MEMBER(dled1_w);
	DECLARE_WRITE8_MEMBER(zcres_w);
	DECLARE_WRITE8_MEMBER(wdogcl_w);
	DECLARE_READ8_MEMBER(gentmrcl_r);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_housekeeping);
	TIMER_DEVICE_CALLBACK_MEMBER(firq_housekeeping);
private:
	bool m_zcen;
	bool m_gten;
	bool m_disp_on;
	bool m_diag_on;
	UINT8 m_firqtimer;
	UINT8 m_diag_segments;
	virtual void machine_reset() override;
	required_device<cpu_device> m_ccpu;
	required_device<cpu_device> m_hcpu;
	required_shared_ptr<UINT8> m_shared_ram;
};

// housekeeping cpu
static ADDRESS_MAP_START( hcpu_map, AS_PROGRAM, 8, wico_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("sharedram")
	AM_RANGE(0x1fe0, 0x1fe0) AM_WRITE(muxld_w)
	//AM_RANGE(0x1fe1, 0x1fe1) AM_WRITE(store_w)
	AM_RANGE(0x1fe2, 0x1fe2) AM_WRITE(muxen_w)
	//AM_RANGE(0x1fe3, 0x1fe3) AM_WRITE(csols_w)
	AM_RANGE(0x1fe4, 0x1fe4) AM_NOP
	AM_RANGE(0x1fe5, 0x1fe5) AM_DEVWRITE("sn76494", sn76494_device, write)
	AM_RANGE(0x1fe6, 0x1fe6) AM_WRITE(wdogcl_w)
	AM_RANGE(0x1fe7, 0x1fe7) AM_WRITE(zcres_w)
	AM_RANGE(0x1fe8, 0x1fe8) AM_WRITE(dled0_w)
	AM_RANGE(0x1fe9, 0x1fe9) AM_WRITE(dled1_w)
	AM_RANGE(0x1fea, 0x1fea) AM_READ(gentmrcl_r)
	AM_RANGE(0x1feb, 0x1feb) AM_READ(lampst_r)
	//AM_RANGE(0x1fec, 0x1fec) AM_READ(sast_r)
	//AM_RANGE(0x1fed, 0x1fed) AM_READ(solst1_r)
	//AM_RANGE(0x1fee, 0x1fee) AM_READ(solst0_r)
	AM_RANGE(0x1fef, 0x1fef) AM_READ(switch_r)
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

// command cpu
static ADDRESS_MAP_START( ccpu_map, AS_PROGRAM, 8, wico_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("sharedram") // 2128  2k RAM
	//AM_RANGE(0x1fe0, 0x1fe0) AM_WRITE(muxld_w) // to display module
	//AM_RANGE(0x1fe1, 0x1fe1) AM_WRITE(store_w) // enable save to nvram
	AM_RANGE(0x1fe2, 0x1fe2) AM_WRITE(muxen_w) // digit to display on diagnostic LED; d0=L will disable main displays
	AM_RANGE(0x1fe3, 0x1fe3) AM_WRITE(csols_w) // solenoid column
	AM_RANGE(0x1fe4, 0x1fe4) AM_WRITE(msols_w) // solenoid row
	AM_RANGE(0x1fe5, 0x1fe5) AM_DEVWRITE("sn76494", sn76494_device, write)
	AM_RANGE(0x1fe6, 0x1fe6) AM_WRITE(wdogcl_w) // watchdog clear
	AM_RANGE(0x1fe7, 0x1fe7) AM_WRITE(zcres_w) // enable IRQ on hcpu
	AM_RANGE(0x1fe8, 0x1fe8) AM_WRITE(dled0_w) // turn off diagnostic LED
	AM_RANGE(0x1fe9, 0x1fe9) AM_WRITE(dled1_w) // turn on diagnostic LED
	AM_RANGE(0x1fea, 0x1fea) AM_READ(gentmrcl_r) // enable IRQ on ccpu
	//AM_RANGE(0x1feb, 0x1feb) AM_READ(lampst_r) // lamps?
	//AM_RANGE(0x1fec, 0x1fec) AM_READ(sast_r) // a pwron pulse to d0 L->H
	//AM_RANGE(0x1fed, 0x1fed) AM_READ(solst1_r) // switches
	//AM_RANGE(0x1fee, 0x1fee) AM_READ(solst0_r) // switches
	//AM_RANGE(0x1fef, 0x1fef) AM_READ(switch_r) // switches
	AM_RANGE(0x4000, 0x40ff) AM_RAM AM_SHARE("nvram") // X2212 4bit x 256 NVRAM, stores only when store_w is active
	AM_RANGE(0x8000, 0x9fff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( wico )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) // Clear button
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) // Advance button
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Top Lane 4") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Top Lane 3") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Top Lane 2") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Top Lane 1") PORT_CODE(KEYCODE_R)
	PORT_START("X1")
	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bumper BR") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bumper BL") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bumper TR") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bumper TL") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Target BR") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Target BL") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Target TR") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Target TL") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_START("X3")
	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Oil Pit Release") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Oil Pit Target") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Waterhole Release") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Outhole") PORT_CODE(KEYCODE_X) // not working
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L Spinner") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R Rollunder") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R Spinner") PORT_CODE(KEYCODE_M)
	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L SLingshot") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R Slingshot") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R Drop Bank E") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R Drop Bank P") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R Drop Bank A") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L Drop Bank C") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L Drop Bank S") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L Drop Bank E") PORT_CODE(KEYCODE_H)
	PORT_START("X6")
	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R Target Zone E") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R Target Zone D") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R Target Zone I") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R Target Zone R") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L Target Zone T") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L Target Zone S") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L Target Zone A") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L Target Zone F") PORT_CODE(KEYCODE_7_PAD)
	PORT_START("X8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R Outlane Target") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L Outlane Target") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R Rollover R Outlane") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R Rollover M Outlane") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R Rollover L Outlane") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L Rollover R Outlane") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L Rollover M Outlane") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L Rollover L Outlane") PORT_CODE(KEYCODE_SLASH)
	PORT_START("X9")
	PORT_START("XA")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_TILT1) PORT_NAME("Door Slam")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_TILT) PORT_NAME("Playfield Tilt")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_TILT) PORT_NAME("Pendulum Tilt")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R Flipper Lane Change") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ball Feed Middle") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ball Feed Lower") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("10 points") PORT_CODE(KEYCODE_COLON)
	PORT_START("XB")
	PORT_START("XC")
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
	PORT_START("XD")
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
	PORT_START("XE")
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
	PORT_START("XF")
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
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Self Test")
INPUT_PORTS_END

// diagnostic display off
WRITE8_MEMBER( wico_state::dled0_w )
{
	m_diag_on = 0;
	output().set_digit_value(9, 0);
}

// diagnostic display on
WRITE8_MEMBER( wico_state::dled1_w )
{
	m_diag_on = 1;
	output().set_digit_value(9, m_diag_segments);
}

WRITE8_MEMBER( wico_state::csols_w )
{
}

WRITE8_MEMBER( wico_state::msols_w )
{
}

// write to diagnostic display
WRITE8_MEMBER( wico_state::muxen_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71 }; // MC14495

	m_diag_segments = patterns[data>>4];

	if (m_diag_on)
		output().set_digit_value(9, m_diag_segments);
	else
		output().set_digit_value(9, 0);

	m_disp_on = BIT(data, 0);
}

// reset digit/scan counter
WRITE8_MEMBER( wico_state::muxld_w )
{
}

// enable zero-crossing interrupt
WRITE8_MEMBER( wico_state::zcres_w )
{
	m_zcen = 1;
}

// enable firq
READ8_MEMBER( wico_state::gentmrcl_r )
{
	m_gten = 1;
	return 0xff;
}

// read a switch row
READ8_MEMBER( wico_state::switch_r )
{
	char kbdrow[8];
	offset = m_shared_ram[0x95];
	sprintf(kbdrow,"X%X",offset);
	UINT8 data = ioport(kbdrow)->read();

	// Reflex solenoids - operated directly by the switches without needing the cpu
	if ((offset==2) && (data & 15))
		m_samples->start(0, 0); // bumpers
	else
	if ((offset==5) && (data & 3))
		m_samples->start(1, 7); // slings

	return data;
}

// write digits in main display
READ8_MEMBER( wico_state::lampst_r )
{
	int i, j;
	for (i = 0; i < 5; i++)
	{
		if (m_disp_on)
			j = m_shared_ram[0x7f9 + i];
		else
			j = 0;
		output().set_digit_value(i * 10 + (m_shared_ram[0x96] & 7), BITSWAP16(j, 8, 8, 8, 8, 8, 8, 7, 7, 6, 6, 5, 4, 3, 2, 1, 0));
	}
	return 0xff;
}

// reset watchdog and enable housekeeping cpu
WRITE8_MEMBER( wico_state::wdogcl_w )
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

void wico_state::machine_reset()
{
	m_hcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_zcen = 0;
	m_gten = 0;
	m_firqtimer = 0;
	m_disp_on = 0;
	m_diag_on = 0;
}


static MACHINE_CONFIG_START( wico, wico_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("ccpu", M6809, 10000000 / 8)
	MCFG_CPU_PROGRAM_MAP(ccpu_map)
	MCFG_CPU_ADD("hcpu", M6809, 10000000 / 8)
	MCFG_CPU_PROGRAM_MAP(hcpu_map)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq", wico_state, irq_housekeeping, attotime::from_hz(120)) // zero crossing
	MCFG_TIMER_DRIVER_ADD_PERIODIC("firq", wico_state, firq_housekeeping, attotime::from_hz(750)) // time generator
	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_wico)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("sn76494", SN76494, 10000000 / 64)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Af-Tor (1984)
/-------------------------------------------------------------------*/
ROM_START(aftor)
	ROM_REGION(0x10000, "hcpu", 0)
	ROM_LOAD("u25.bin", 0xf000, 0x1000, CRC(d66e95ff) SHA1(f7e8c51f1b37e7ef560406f1968c12a2043646c5))

	ROM_REGION(0x10000, "ccpu", 0)
	ROM_LOAD("u52.bin", 0x8000, 0x2000, CRC(8035b446) SHA1(3ec59015e259c315bf09f4e2046f9d98e2d7a732))
	ROM_LOAD("u48.bin", 0xe000, 0x2000, CRC(b4406563) SHA1(6d1a9086eb1f6f947eae3a92ccf7a9b7375d85d3))
ROM_END

/*-------------------------------------------------------------------
/ Big Top  (1977)
/-------------------------------------------------------------------*/

GAME(1984,  aftor,  0,  wico,  wico, driver_device,  0,  ROT0,  "Wico", "Af-Tor", MACHINE_MECHANICAL)
