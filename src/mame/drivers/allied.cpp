// license:BSD-3-Clause
// copyright-holders:Robbbert
/**************************************************************************

  PINBALL
  Allied Leisure Cocktail Pinball
  All tables use the same base roms and some playfields even interchange
  between games.

  6504 CPU, 3x R6530 RRIOT, 5x R6520 PIA
  It is assumed that the R6530 is the same as MOS6530, and the R6520 is
  the same as MC6821.

  The schematic is too blurry to make out much detail, so used PinMAME
  for the PIA connections.

  The display units use 74164 serial units, with clock and data lines
  sufficient for all the digits of one player. Data = IC2,PB7, while
  Clock is IC2,CB2. To prevent display garbage, the new data is stored
  while IC4PBx is high, then displayed when the line goes low. IC7portB
  selects which player's display to update.

  There are no dipswitches; instead there are a number of jumper wires
  which can be pushed onto one of up to ten connector pins each. There
  are 15 10-pin connectors, a 5-pin connector and 2 2-pin connectors.

  For some reason the 'rol $46' instruction outputs the original data
  followed by the new result, so I've had to employ a horrible hack.

  Hold down X while inserting a coin.
  At the start of each ball, the display will be flashing. You need to
  hit Z, and then you can get a score. When the ball indicator goes out,
  your game is over.

  Game doesn't have any backup battery, so all info is lost at poweroff.
  If required, a fake nvram could be used at 00-3F (like PinMAME does).
  Mechanical meters are used to store accounting information.


ToDo:
- Coin slot 3 not working. Pressing it a lot kills all the coin slots.


***************************************************************************/

#include "emu.h"
#include "machine/genpin.h"
#include "cpu/m6502/m6504.h"
#include "machine/mos6530.h"
#include "machine/6821pia.h"
#include "machine/timer.h"
#include "allied.lh"

class allied_state : public genpin_class
{
public:
	allied_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ic1(*this, "ic1")
		, m_ic2(*this, "ic2")
		, m_ic4(*this, "ic4")
		, m_ic5(*this, "ic5")
		, m_ic6(*this, "ic6")
		, m_ic7(*this, "ic7")
		, m_ic8(*this, "ic8")
		, m_digits(*this, "digit%u", 0U)
		, m_leds(*this, "led%u", 0U)
	{ }

	void allied(machine_config &config);

private:
	DECLARE_WRITE8_MEMBER(ic1_b_w);
	DECLARE_WRITE8_MEMBER(ic2_b_w);
	DECLARE_WRITE_LINE_MEMBER(ic2_cb2_w);
	DECLARE_WRITE8_MEMBER(ic3_b_w);
	DECLARE_WRITE8_MEMBER(ic4_b_w);
	DECLARE_WRITE_LINE_MEMBER(ic4_cb2_w);
	DECLARE_WRITE8_MEMBER(ic5_b_w);
	DECLARE_WRITE8_MEMBER(ic6_b_w);
	DECLARE_WRITE8_MEMBER(ic7_b_w);
	DECLARE_WRITE8_MEMBER(ic8_a_w);
	DECLARE_WRITE8_MEMBER(ic8_b_w);
	DECLARE_READ8_MEMBER(ic1_a_r);
	DECLARE_READ8_MEMBER(ic2_a_r);
	DECLARE_READ8_MEMBER(ic4_a_r);
	DECLARE_READ8_MEMBER(ic5_a_r);
	DECLARE_READ8_MEMBER(ic6_a_r);
	DECLARE_READ8_MEMBER(ic6_b_r);
	DECLARE_READ8_MEMBER(ic7_a_r);
	DECLARE_WRITE_LINE_MEMBER(ic8_cb2_w);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_a);
	void allied_map(address_map &map);

	uint32_t m_player_score[6];
	uint8_t m_display;
	uint8_t m_bit_counter;
	bool m_disp_data;
	uint8_t m_ic5a;
	uint8_t m_ic6a0;
	uint8_t m_ic6a1;
	uint8_t m_ic6a2;
	uint8_t m_ic6b4;
	uint8_t m_ic6b7;
	virtual void machine_reset() override;
	virtual void machine_start() override;
	required_device<m6504_device> m_maincpu;
	required_device<pia6821_device> m_ic1;
	required_device<pia6821_device> m_ic2;
	required_device<pia6821_device> m_ic4;
	required_device<mos6530_device> m_ic5;
	required_device<mos6530_device> m_ic6;
	required_device<pia6821_device> m_ic7;
	required_device<pia6821_device> m_ic8;
	output_finder<42> m_digits;
	output_finder<7> m_leds;
};


void allied_state::allied_map(address_map &map)
{
	map(0x0000, 0x003f).ram(); // ic6
	map(0x0044, 0x0047).rw(m_ic2, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0048, 0x004b).rw(m_ic1, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0050, 0x0053).rw(m_ic7, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0060, 0x0063).rw(m_ic4, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0080, 0x008f).rw(m_ic5, FUNC(mos6530_device::read), FUNC(mos6530_device::write));
	map(0x0840, 0x084f).rw(m_ic6, FUNC(mos6530_device::read), FUNC(mos6530_device::write));
	map(0x00c0, 0x00c3).rw(m_ic8, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0100, 0x013f).ram(); // ic5
	map(0x1400, 0x1fff).rom();
}

static INPUT_PORTS_START( allied )
	PORT_START("TEST")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Self Test")
	PORT_START("R11")
	PORT_CONFNAME( 0x0f, 0x00, "1st Replay 1000")
	PORT_CONFSETTING(    0x00, "0")
	PORT_CONFSETTING(    0x01, "1")
	PORT_CONFSETTING(    0x02, "2")
	PORT_CONFSETTING(    0x03, "3")
	PORT_CONFSETTING(    0x04, "4")
	PORT_CONFSETTING(    0x05, "5")
	PORT_CONFSETTING(    0x06, "6")
	PORT_CONFSETTING(    0x07, "7")
	PORT_CONFSETTING(    0x08, "8")
	PORT_CONFSETTING(    0x09, "9")
	PORT_START("R12")
	PORT_CONFNAME( 0x0f, 0x05, "1st Replay 10000")
	PORT_CONFSETTING(    0x00, "0")
	PORT_CONFSETTING(    0x01, "1")
	PORT_CONFSETTING(    0x02, "2")
	PORT_CONFSETTING(    0x03, "3")
	PORT_CONFSETTING(    0x04, "4")
	PORT_CONFSETTING(    0x05, "5")
	PORT_CONFSETTING(    0x06, "6")
	PORT_CONFSETTING(    0x07, "7")
	PORT_CONFSETTING(    0x08, "8")
	PORT_CONFSETTING(    0x09, "9")
	PORT_START("R13")
	PORT_CONFNAME( 0x0f, 0x00, "1st Replay 100000")
	PORT_CONFSETTING(    0x00, "0")
	PORT_CONFSETTING(    0x01, "1")
	PORT_CONFSETTING(    0x02, "2")
	PORT_CONFSETTING(    0x03, "3")
	PORT_CONFSETTING(    0x04, "4")
	PORT_CONFSETTING(    0x05, "5")
	PORT_CONFSETTING(    0x06, "6")
	PORT_CONFSETTING(    0x07, "7")
	PORT_CONFSETTING(    0x08, "8")
	PORT_CONFSETTING(    0x09, "9")
	PORT_START("R21")
	PORT_CONFNAME( 0x0f, 0x00, "2nd Replay 1000")
	PORT_CONFSETTING(    0x00, "0")
	PORT_CONFSETTING(    0x01, "1")
	PORT_CONFSETTING(    0x02, "2")
	PORT_CONFSETTING(    0x03, "3")
	PORT_CONFSETTING(    0x04, "4")
	PORT_CONFSETTING(    0x05, "5")
	PORT_CONFSETTING(    0x06, "6")
	PORT_CONFSETTING(    0x07, "7")
	PORT_CONFSETTING(    0x08, "8")
	PORT_CONFSETTING(    0x09, "9")
	PORT_START("R22")
	PORT_CONFNAME( 0x0f, 0x00, "2nd Replay 10000")
	PORT_CONFSETTING(    0x00, "0")
	PORT_CONFSETTING(    0x01, "1")
	PORT_CONFSETTING(    0x02, "2")
	PORT_CONFSETTING(    0x03, "3")
	PORT_CONFSETTING(    0x04, "4")
	PORT_CONFSETTING(    0x05, "5")
	PORT_CONFSETTING(    0x06, "6")
	PORT_CONFSETTING(    0x07, "7")
	PORT_CONFSETTING(    0x08, "8")
	PORT_CONFSETTING(    0x09, "9")
	PORT_START("R23")
	PORT_CONFNAME( 0x0f, 0x01, "2nd Replay 100000")
	PORT_CONFSETTING(    0x00, "0")
	PORT_CONFSETTING(    0x01, "1")
	PORT_CONFSETTING(    0x02, "2")
	PORT_CONFSETTING(    0x03, "3")
	PORT_CONFSETTING(    0x04, "4")
	PORT_CONFSETTING(    0x05, "5")
	PORT_CONFSETTING(    0x06, "6")
	PORT_CONFSETTING(    0x07, "7")
	PORT_CONFSETTING(    0x08, "8")
	PORT_CONFSETTING(    0x09, "9")
	PORT_START("R31")
	PORT_CONFNAME( 0x0f, 0x00, "3rd Replay 1000")
	PORT_CONFSETTING(    0x00, "0")
	PORT_CONFSETTING(    0x01, "1")
	PORT_CONFSETTING(    0x02, "2")
	PORT_CONFSETTING(    0x03, "3")
	PORT_CONFSETTING(    0x04, "4")
	PORT_CONFSETTING(    0x05, "5")
	PORT_CONFSETTING(    0x06, "6")
	PORT_CONFSETTING(    0x07, "7")
	PORT_CONFSETTING(    0x08, "8")
	PORT_CONFSETTING(    0x09, "9")
	PORT_START("R32")
	PORT_CONFNAME( 0x0f, 0x06, "3rd Replay 10000")
	PORT_CONFSETTING(    0x00, "0")
	PORT_CONFSETTING(    0x01, "1")
	PORT_CONFSETTING(    0x02, "2")
	PORT_CONFSETTING(    0x03, "3")
	PORT_CONFSETTING(    0x04, "4")
	PORT_CONFSETTING(    0x05, "5")
	PORT_CONFSETTING(    0x06, "6")
	PORT_CONFSETTING(    0x07, "7")
	PORT_CONFSETTING(    0x08, "8")
	PORT_CONFSETTING(    0x09, "9")
	PORT_START("R33")
	PORT_CONFNAME( 0x0f, 0x01, "3rd Replay 100000")
	PORT_CONFSETTING(    0x00, "0")
	PORT_CONFSETTING(    0x01, "1")
	PORT_CONFSETTING(    0x02, "2")
	PORT_CONFSETTING(    0x03, "3")
	PORT_CONFSETTING(    0x04, "4")
	PORT_CONFSETTING(    0x05, "5")
	PORT_CONFSETTING(    0x06, "6")
	PORT_CONFSETTING(    0x07, "7")
	PORT_CONFSETTING(    0x08, "8")
	PORT_CONFSETTING(    0x09, "9")
	PORT_START("CM1")
	PORT_CONFNAME( 0x0f, 0x05, "Credits Max 1")
	PORT_CONFSETTING(    0x00, "0")
	PORT_CONFSETTING(    0x01, "1")
	PORT_CONFSETTING(    0x02, "2")
	PORT_CONFSETTING(    0x03, "3")
	PORT_CONFSETTING(    0x04, "4")
	PORT_CONFSETTING(    0x05, "5")
	PORT_CONFSETTING(    0x06, "6")
	PORT_CONFSETTING(    0x07, "7")
	PORT_CONFSETTING(    0x08, "8")
	PORT_CONFSETTING(    0x09, "9")
	PORT_START("CM2")
	PORT_CONFNAME( 0x0f, 0x01, "Credits Max 10")
	PORT_CONFSETTING(    0x00, "0")
	PORT_CONFSETTING(    0x01, "1")
	PORT_CONFSETTING(    0x02, "2")
	PORT_CONFSETTING(    0x03, "3")
	PORT_CONFSETTING(    0x04, "4")
	PORT_CONFSETTING(    0x05, "5")
	PORT_CONFSETTING(    0x06, "6")
	PORT_CONFSETTING(    0x07, "7")
	PORT_CONFSETTING(    0x08, "8")
	PORT_CONFSETTING(    0x09, "9")
	PORT_START("CS1")
	PORT_CONFNAME( 0x0f, 0x01, "Coin Slot 1")
	PORT_CONFSETTING(    0x00, "0")
	PORT_CONFSETTING(    0x01, "1")
	PORT_CONFSETTING(    0x02, "2")
	PORT_CONFSETTING(    0x03, "3")
	PORT_CONFSETTING(    0x04, "4")
	PORT_CONFSETTING(    0x05, "5")
	PORT_CONFSETTING(    0x06, "6")
	PORT_CONFSETTING(    0x07, "7")
	PORT_CONFSETTING(    0x08, "8")
	PORT_CONFSETTING(    0x09, "9")
	PORT_START("CS2")
	PORT_CONFNAME( 0x0f, 0x01, "Coin Slot 2")
	PORT_CONFSETTING(    0x00, "0")
	PORT_CONFSETTING(    0x01, "1")
	PORT_CONFSETTING(    0x02, "2")
	PORT_CONFSETTING(    0x03, "3")
	PORT_CONFSETTING(    0x04, "4")
	PORT_CONFSETTING(    0x05, "5")
	PORT_CONFSETTING(    0x06, "6")
	PORT_CONFSETTING(    0x07, "7")
	PORT_CONFSETTING(    0x08, "8")
	PORT_CONFSETTING(    0x09, "9")
	PORT_START("CS3")
	PORT_CONFNAME( 0x0f, 0x01, "Coin Slot 3")
	PORT_CONFSETTING(    0x00, "0")
	PORT_CONFSETTING(    0x01, "1")
	PORT_CONFSETTING(    0x02, "2")
	PORT_CONFSETTING(    0x03, "3")
	PORT_CONFSETTING(    0x04, "4")
	PORT_CONFSETTING(    0x05, "5")
	PORT_CONFSETTING(    0x06, "6")
	PORT_CONFSETTING(    0x07, "7")
	PORT_CONFSETTING(    0x08, "8")
	PORT_CONFSETTING(    0x09, "9")
	PORT_START("CR")
	PORT_CONFNAME( 0x0f, 0x01, "Credits Option")
	PORT_CONFSETTING(    0x01, "1")
	PORT_CONFSETTING(    0x02, "2")
	PORT_START("B")
	PORT_CONFNAME( 0x07, 0x03, "Balls")
	PORT_CONFSETTING(    0x01, "1")
	PORT_CONFSETTING(    0x02, "2")
	PORT_CONFSETTING(    0x03, "3")
	PORT_CONFSETTING(    0x04, "4")
	PORT_CONFSETTING(    0x05, "5")
	PORT_START("A")
	PORT_CONFNAME( 0x04, 0x04, "Award")
	PORT_CONFSETTING(    0x04, "Replay")
	PORT_CONFSETTING(    0x00, "Extra Ball")
	PORT_CONFNAME( 0x08, 0x08, "Line Up Inhibit")
	PORT_CONFSETTING(    0x08, DEF_STR(No))
	PORT_CONFSETTING(    0x00, DEF_STR(Yes))
	PORT_START("N")
	PORT_CONFNAME( 0x01, 0x01, "Number of Players")
	PORT_CONFSETTING(    0x00, "2")
	PORT_CONFSETTING(    0x01, "4")

	PORT_START("X1A") // ic1_a
	PORT_BIT( 0x5f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("Bullseye target")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("Ball in play")

	PORT_START("X2A") // ic2_a
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("L Bumper")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("C Bumper")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("R Bumper")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("L Bullseye")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("R Bullseye")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("L Sling")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("R Sling")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")

	PORT_START("X4A") // ic4_a
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("Target A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("Target B")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("Target C")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("Target D")
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START )

	PORT_START("X6A") // ic6_a
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT1 ) PORT_NAME("Slam Tilt")

	PORT_START("X7A") // ic7_a
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("Raise Target A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M) PORT_NAME("Raise Target D")
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("500 point rollover")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("Raise Target B")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("Raise Target C")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("Extra Ball when Lit")
INPUT_PORTS_END

// 1 target, 1 rollover
READ8_MEMBER( allied_state::ic1_a_r )
{
	return ioport("X1A")->read();
}

// 6 lamps
WRITE8_MEMBER( allied_state::ic1_b_w )
{
}

// 8 switches
READ8_MEMBER( allied_state::ic2_a_r )
{
	return ioport("X2A")->read();
}

WRITE8_MEMBER( allied_state::ic2_b_w )
{
// PB0-4,6 - lamps

	m_disp_data = !BIT(data, 7);
}

WRITE_LINE_MEMBER( allied_state::ic2_cb2_w )
{
	if ((m_display) && (!state))
	{
		m_bit_counter++;
		if (BIT(m_bit_counter, 0))
			m_player_score[m_display-1] = (m_player_score[m_display-1] << 1) | m_disp_data;
		if (m_bit_counter == 15)
			m_bit_counter = 0;
	}
}

WRITE8_MEMBER( allied_state::ic3_b_w )
{
	m_maincpu->set_input_line(M6504_IRQ_LINE, BIT(data, 7) ? CLEAR_LINE : ASSERT_LINE );
}

// 6 switches
READ8_MEMBER( allied_state::ic4_a_r )
{
	return ioport("X4A")->read();
}

WRITE8_MEMBER( allied_state::ic4_b_w )
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7446A
	uint8_t segment, i;
	for (i = 0; i < 4; i++)
	{
		if (!BIT(data, i+4))
		{
			m_digits[i*10] = patterns[0];
			segment = (m_player_score[i] >> 0) & 15;
			m_digits[i*10+1] = patterns[segment];
			segment = (m_player_score[i] >> 4) & 15;
			m_digits[i*10+2] =  patterns[segment];
			segment = (m_player_score[i] >> 8) & 15;
			m_digits[i*10+3] = patterns[segment];
			segment = (m_player_score[i] >> 12) & 15;
			m_digits[i*10+4] = patterns[segment];
			segment = (m_player_score[i] >> 16) & 15;
			m_digits[i*10+5] = patterns[segment];
		}
		else
		{
			m_digits[i*10] = 0;
			m_digits[i*10+1] = 0;
			m_digits[i*10+2] = 0;
			m_digits[i*10+3] = 0;
			m_digits[i*10+4] = 0;
			m_digits[i*10+5] = 0;
		}
	}

	// doesn't seem to be a strobe for the credits display
	segment = (m_player_score[4] >> 0) & 15;
	m_digits[40] = patterns[segment];
	segment = (m_player_score[4] >> 4) & 15;
	m_digits[41] = patterns[segment];

// PB0-3 - player 1-4 LED - to do
}

WRITE_LINE_MEMBER( allied_state::ic4_cb2_w )
{
}

// 8 of the adjustment connectors
READ8_MEMBER( allied_state::ic5_a_r )
{
	return m_ic5a;
}

// cabinet solenoids
WRITE8_MEMBER( allied_state::ic5_b_w )
{
// PB0 - play meter
// PB1 - replay meter

	if (!BIT(data, 2)) // chime C
		m_samples->start(1, 1);

	if (!BIT(data, 3)) // chime B
		m_samples->start(2, 2);

	if (!BIT(data, 4)) // chime A
		m_samples->start(3, 3); // tens have highest tone

	if (!BIT(data, 5)) // knocker
		m_samples->start(0, 6);

	m_maincpu->set_input_line(M6504_IRQ_LINE, BIT(data, 7) ? CLEAR_LINE : ASSERT_LINE );
}

// 4 adjustments, 3 coin slots, slam tilt
READ8_MEMBER( allied_state::ic6_a_r )
{
	return m_ic6a0 | m_ic6a1 | m_ic6a2 | ioport("X6A")->read();
}

// 1 adjustment, test switch
READ8_MEMBER( allied_state::ic6_b_r )
{
	return m_ic6b4 | ioport("TEST")->read() | m_ic6b7 | 0x4f;
}

WRITE8_MEMBER( allied_state::ic6_b_w )
{
// PB0-3 to drop targets

	m_maincpu->set_input_line(M6504_IRQ_LINE, BIT(data, 7) ? CLEAR_LINE : ASSERT_LINE );
}

// 6 inputs
READ8_MEMBER( allied_state::ic7_a_r )
{
	return ioport("X7A")->read();
}

WRITE8_MEMBER( allied_state::ic7_b_w )
{
// PB7 - tilt lamp

	// PB4-6 display select
	m_display = data >> 4;
	if (m_display > 5)
		m_display = 0;
	m_bit_counter = 0;

	// PB0-3 switch matrix
	data &= 15;
	bool res;
	// IC1CA1 = 3rd 100,000 replay
	res = (ioport("R33")->read() == data);
	m_ic1->ca1_w(!res);
	// IC1CB1 = Number of balls
	res = (ioport("B")->read() == data);
	m_ic1->cb1_w(!res);
	// IC2CA1 = Max. credit tens
	res = (ioport("CM2")->read() == data);
	m_ic2->ca1_w(!res);
	// IC2CB1 = Max. credit units
	res = (ioport("CM1")->read() == data);
	m_ic2->cb1_w(!res);
	// IC4CA1 = Credit 1 prog.
	res = (ioport("CS1")->read() == data);
	m_ic4->ca1_w(!res);
	// IC5PA0 = 1st 1,000 replay
	m_ic5a = (ioport("R11")->read() == data) ? 0 : 1;
	// IC5PA1 = 1st 10,000 replay
	m_ic5a |= (ioport("R12")->read() == data) ? 0 : 2;
	// IC5PA2 = 1st 100,000 replay
	m_ic5a |= (ioport("R13")->read() == data) ? 0 : 4;
	// IC5PA3 = 2nd 1,000 replay
	m_ic5a |= (ioport("R21")->read() == data) ? 0 : 8;
	// IC5PA4 = 2nd 10,000 replay
	m_ic5a |= (ioport("R22")->read() == data) ? 0 : 16;
	// IC5PA5 = 2nd 100,000 replay
	m_ic5a |= (ioport("R23")->read() == data) ? 0 : 32;
	// IC5PA6 = 3rd 1,000 replay
	m_ic5a |= (ioport("R31")->read() == data) ? 0 : 64;
	// IC5PA7 = 3rd 10,000 replay
	m_ic5a |= (ioport("R32")->read() == data) ? 0 : 128;
	// IC6PA0 = Credit options
	m_ic6a0 = (ioport("CR")->read() == data) ? 0 : 1;
	// IC6PA1 = Credit 2 prog.
	m_ic6a1 = (ioport("CS2")->read() == data) ? 0 : 2;
	// IC6PA2 = Replay / Add-a-ball / Match inhibit
	m_ic6a2 = ioport("A")->read();
	// IC6PB4 = Credit 3 prog.
	m_ic6b4 = (ioport("CS3")->read() == data) ? 0 : 16;
	// IC7CA1 = 2/4 Players
	res = ((ioport("N")->read() ? 4 : 2) == data);
	m_ic7->ca1_w(!res);
}

// playfield solenoids
WRITE8_MEMBER( allied_state::ic8_a_w )
{
	if ((data & 0x07) < 0x07) // 3 bumpers
		m_samples->start(0, 0);

	if ((data & 0x60) < 0x60) // slings
		m_samples->start(0, 7);

	if (!BIT(data, 7)) // outhole
		m_samples->start(0, 5);
}

// PB0-4 = ball 1-5 LED; PB5 = shoot again lamp
WRITE8_MEMBER( allied_state::ic8_b_w )
{
	for (int i = 0; i < 6; i++)
		m_leds[i+1] = !BIT(data, i);
}

// this line not emulated in PinMAME, maybe it isn't needed
WRITE_LINE_MEMBER( allied_state::ic8_cb2_w )
{
	m_ic6b7 = state ? 128 : 0; // i think it's pb7, hard to tell
	m_ic7->cb1_w(state);
}

TIMER_DEVICE_CALLBACK_MEMBER( allied_state::timer_a )
{
	uint8_t data = ioport("X6A")->read();

	m_ic8->ca1_w(BIT(data, 4));
	m_ic8->cb1_w(BIT(data, 5));
	m_ic8->ca2_w(BIT(data, 6));
}

void allied_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();
}

void allied_state::machine_reset()
{
	m_display = 0;
	m_bit_counter = 0;
	m_disp_data = 0;
	m_ic5a = 0;
	m_ic6a0 = 0;
	m_ic6a1 = 0;
	m_ic6a2 = 0;
	m_ic6b4 = 0;
	m_ic6b7 = 0;
	m_leds[0] = 1;
}

void allied_state::allied(machine_config &config)
{
	/* basic machine hardware */
	M6504(config, m_maincpu, 3572549/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &allied_state::allied_map);

	/* Video */
	config.set_default_layout(layout_allied);

	/* Sound */
	genpin_audio(config);

	/* Devices */
	PIA6821(config, m_ic1, 0);
	m_ic1->readpa_handler().set(FUNC(allied_state::ic1_a_r));
	//m_ic1->writepa_handler().set(FUNC(allied_state::ic1_a_w));
	//m_ic1->readpb_handler().set(FUNC(allied_state::ic1_b_r));
	m_ic1->writepb_handler().set(FUNC(allied_state::ic1_b_w));
	//m_ic1->ca2_handler().set(FUNC(allied_state::ic1_ca2_w));
	//m_ic1->cb2_handler().set(FUNC(allied_state::ic1_cb2_w));
	m_ic1->irqa_handler().set_inputline("maincpu", M6504_IRQ_LINE);
	m_ic1->irqb_handler().set_inputline("maincpu", M6504_IRQ_LINE);

	PIA6821(config, m_ic2, 0);
	m_ic2->readpa_handler().set(FUNC(allied_state::ic2_a_r));
	//m_ic2->writepa_handler().set(FUNC(allied_state::ic2_a_w));
	//m_ic2->readpb_handler().set(FUNC(allied_state::ic2_b_r));
	m_ic2->writepb_handler().set(FUNC(allied_state::ic2_b_w));
	//m_ic2->ca2_handler().set(FUNC(allied_state::ic2_ca2_w));
	m_ic2->cb2_handler().set(FUNC(allied_state::ic2_cb2_w));
	m_ic2->irqa_handler().set_inputline("maincpu", M6504_IRQ_LINE);
	m_ic2->irqb_handler().set_inputline("maincpu", M6504_IRQ_LINE);

	PIA6821(config, m_ic4, 0);
	m_ic4->readpa_handler().set(FUNC(allied_state::ic4_a_r));
	//m_ic4->writepa_handler().set(FUNC(allied_state::ic4_a_w));
	//m_ic4->readpb_handler().set(FUNC(allied_state::ic4_b_r));
	m_ic4->writepb_handler().set(FUNC(allied_state::ic4_b_w));
	//m_ic4->ca2_handler().set(FUNC(allied_state::ic4_ca2_w));
	m_ic4->cb2_handler().set(FUNC(allied_state::ic4_cb2_w));
	m_ic4->irqa_handler().set_inputline("maincpu", M6504_IRQ_LINE);
	m_ic4->irqb_handler().set_inputline("maincpu", M6504_IRQ_LINE);

	PIA6821(config, m_ic7, 0);
	m_ic7->readpa_handler().set(FUNC(allied_state::ic7_a_r));
	//m_ic7->writepa_handler().set(FUNC(allied_state::ic7_a_w));
	//m_ic7->readpb_handler().set(FUNC(allied_state::ic7_b_r));
	m_ic7->writepb_handler().set(FUNC(allied_state::ic7_b_w));
	//m_ic7->ca2_handler().set(FUNC(allied_state::ic7_ca2_w));
	//m_ic7->cb2_handler().set(FUNC(allied_state::ic7_cb2_w));
	m_ic7->irqa_handler().set_inputline("maincpu", M6504_IRQ_LINE);
	m_ic7->irqb_handler().set_inputline("maincpu", M6504_IRQ_LINE);

	PIA6821(config, m_ic8, 0);
	//m_ic8->readpa_handler().set(FUNC(allied_state::ic8_a_r));
	m_ic8->writepa_handler().set(FUNC(allied_state::ic8_a_w));
	//m_ic8->readpb_handler().set(FUNC(allied_state::ic8_b_r));
	m_ic8->writepb_handler().set(FUNC(allied_state::ic8_b_w));
	//m_ic8->ca2_handler().set(FUNC(allied_state::ic8_ca2_w));
	m_ic8->cb2_handler().set(FUNC(allied_state::ic8_cb2_w));
	m_ic8->irqa_handler().set_inputline("maincpu", M6504_IRQ_LINE);
	m_ic8->irqb_handler().set_inputline("maincpu", M6504_IRQ_LINE);

	mos6530_device &ic3(MOS6530(config, "ic3", 3572549/4)); // unknown where the ram and i/o is located
	ic3.out_pb_callback().set(FUNC(allied_state::ic3_b_w));

	MOS6530(config, m_ic5, 3572549/4);
	m_ic5->in_pa_callback().set(FUNC(allied_state::ic5_a_r));
	//m_ic5->out_pa_callback().set(FUNC(allied_state::ic5_a_w));
	//m_ic5->in_pb_callback().set(FUNC(allied_state::ic5_b_r));
	m_ic5->out_pb_callback().set(FUNC(allied_state::ic5_b_w));

	MOS6530(config, m_ic6, 3572549/4);
	m_ic6->in_pa_callback().set(FUNC(allied_state::ic6_a_r));
	//m_ic6->out_pa_callback().set(FUNC(allied_state::ic6_a_w));
	m_ic6->in_pb_callback().set(FUNC(allied_state::ic6_b_r));
	m_ic6->out_pb_callback().set(FUNC(allied_state::ic6_b_w));

	TIMER(config, "timer_a").configure_periodic(FUNC(allied_state::timer_a), attotime::from_hz(50));
}


ROM_START( allied )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "6530-009.u5", 0x1400, 0x0400, CRC(e4fb64fb) SHA1(a3d9de7cbfb42180a860e0bbbeaeba96d8bd1e20))
	ROM_LOAD( "6530-010.u6", 0x1800, 0x0400, CRC(dca980dd) SHA1(3817d75413854d889fc1ce4fd6a51d820d1e0534))
	ROM_LOAD( "6530-011.u3", 0x1c00, 0x0400, CRC(13f42789) SHA1(baa0f73fda08a3c5d6f1423fb329e4febb07ef97))
ROM_END

#define rom_suprpick    rom_allied
#define rom_royclark    rom_allied
#define rom_thndbolt    rom_allied
#define rom_hoedown     rom_allied
#define rom_takefive    rom_allied
#define rom_heartspd    rom_allied
#define rom_foathens    rom_allied
#define rom_disco79     rom_allied
#define rom_erosone     rom_allied
#define rom_circa33     rom_allied
#define rom_starshot    rom_allied


GAME(1977,  allied,   0,      allied, allied, allied_state, empty_init, ROT0, "Allied Leisure",   "Allied System",               MACHINE_IS_BIOS_ROOT | MACHINE_NOT_WORKING )
GAME(1977,  suprpick, allied, allied, allied, allied_state, empty_init, ROT0, "Allied Leisure",   "Super Picker",                MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME(1977,  royclark, allied, allied, allied, allied_state, empty_init, ROT0, "Fascination Int.", "Roy Clark - The Entertainer", MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME(1977,  thndbolt, allied, allied, allied, allied_state, empty_init, ROT0, "Allied Leisure",   "Thunderbolt",                 MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME(1978,  hoedown,  allied, allied, allied, allied_state, empty_init, ROT0, "Allied Leisure",   "Hoe Down",                    MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME(1978,  takefive, allied, allied, allied, allied_state, empty_init, ROT0, "Allied Leisure",   "Take Five",                   MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME(1978,  heartspd, allied, allied, allied, allied_state, empty_init, ROT0, "Allied Leisure",   "Hearts & Spades",             MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME(1978,  foathens, allied, allied, allied, allied_state, empty_init, ROT0, "Allied Leisure",   "Flame of Athens",             MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME(1979,  disco79,  allied, allied, allied, allied_state, empty_init, ROT0, "Allied Leisure",   "Disco '79",                   MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME(1979,  erosone,  allied, allied, allied, allied_state, empty_init, ROT0, "Fascination Int.", "Eros One",                    MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME(1979,  circa33,  allied, allied, allied, allied_state, empty_init, ROT0, "Fascination Int.", "Circa 1933",                  MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME(1979,  starshot, allied, allied, allied, allied_state, empty_init, ROT0, "Allied Leisure",   "Star Shooter",                MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
