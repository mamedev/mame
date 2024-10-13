// license:BSD-3-Clause
// copyright-holders:Robbbert
/***********************************************************************************

PINBALL
Williams System 3

Typical of Williams hardware: Motorola 8-bit CPUs, and lots of PIAs.

Schematic and PinMAME used as references.

Written during October 2012 [Robbbert]

Games:
- Hot Tip (#477)
- Lucky Seven (#480)
- World Cup Soccer (#481)
- Contact (#482)
- Disco Fevel (#483)
- Phoenix (#485)
- Pokerino (#488)

When first used, it appears frozen (the score should alternate). Press F3 to fix.

Sound:
- All games have a knocker.
- Hot Tip and Lucky Seven use chimes. There was a noise drum to simulate the sound
      of EM score reels.
- Hot Tip, Lucky Seven, World Cup, Disco Fever have a dedicated tilt line to operate
      a mechanical buzzer.
- World Cup and Disco Fever, being chime-based internally, would activate one sound
      line per sound. There's 4 "chime" lines and the tilt line.
- The later games could send any of 15 codes (but never did), so only needed 4 lines.
- All soundcard games also have an "alternator" line. By activating this, followed by
      activating bit 0, the startup tune would play.

Status:
- All games are playable

ToDo:
- Nothing

************************************************************************************/

#include "emu.h"
#include "genpin.h"

#include "williamssound.h"

#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/input_merger.h"
#include "machine/timer.h"

#include "speaker.h"

#include "s3.lh"


namespace {

class s3_state : public genpin_class
{
public:
	s3_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainirq(*this, "mainirq")
		, m_pia22(*this, "pia22")
		, m_pia24(*this, "pia24")
		, m_pia28(*this, "pia28")
		, m_pia30(*this, "pia30")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_dips(*this, "DS%d", 1U)
		, m_digits(*this, "digit%d", 0U)
		, m_leds(*this, "led%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void s3(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);
	void init_1() { m_game = 1; } // wldcp
	void init_2() { m_game = 2; } // cntct
	void init_3() { m_game = 3; } // disco
	void init_4() { m_game = 4; } // lucky

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;


	void dig0_w(u8 data);
	void dig1_w(u8 data);
	void lamp0_w(u8 data);
	void lamp1_w(u8 data);
	u8 dips_r();
	u8 switch_r();
	void switch_w(u8 data);
	u8 m_t_c = 0;
	u8 m_strobe = 0;
	u8 m_row = 0;
	bool m_data_ok = false;
	u8 m_lamp_data = 0;
	u8 m_game = 0;
	bool m_disco = false;
	void pia22_ca2_w(int state) { m_io_outputs[20] = state; } //ST5
	void pia22_cb2_w(int state) { } //ST-solenoids enable
	void pia24_ca2_w(int state) { m_io_outputs[17] = state; } //ST2
	void pia24_cb2_w(int state) { m_io_outputs[16] = state; } //ST1
	void pia28_ca2_w(int state) { } //diag leds enable
	void pia28_cb2_w(int state) { m_io_outputs[21] = state; } //ST6
	void pia30_ca2_w(int state) { m_io_outputs[19] = state; } //ST4
	void pia30_cb2_w(int state) { m_io_outputs[18] = state; } //ST3
	TIMER_DEVICE_CALLBACK_MEMBER(irq);
	void main_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<input_merger_device> m_mainirq;
	required_device<pia6821_device> m_pia22;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia30;
	required_ioport_array<8> m_io_keyboard;
	required_ioport_array<2> m_dips;
	output_finder<32> m_digits;
	output_finder<2> m_leds;
	output_finder<86> m_io_outputs; // 22 solenoids + 64 lamps

private:
	void sol0_w(u8 data);
	void sol1_w(u8 data);
};


class s3a_state : public s3_state
{
public:
	s3a_state(const machine_config &mconfig, device_type type, const char *tag)
		: s3_state(mconfig, type, tag)
		, m_s4sound(*this, "s4sound")
	{ }

	void s3a(machine_config &config);

private:
	void s3a_sol0_w(u8 data);
	void s3a_sol1_w(u8 data);
	required_device<williams_s4_sound_device> m_s4sound;
};


void s3_state::main_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x00ff).ram();
	map(0x0100, 0x01ff).ram().share("nvram");
	map(0x2200, 0x2203).rw(m_pia22, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // solenoids
	map(0x2400, 0x2403).rw(m_pia24, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // lamps
	map(0x2800, 0x2803).rw(m_pia28, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // display
	map(0x3000, 0x3003).rw(m_pia30, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // inputs
	map(0x6000, 0x7fff).rom().region("maincpu", 0);
}


static INPUT_PORTS_START( s3 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("Plumb Tilt") // 3 touches before it tilts
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("High Score Reset") // only on last few games

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP09")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP10")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP11")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP12")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP13")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP14")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP15")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP16")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP17")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP18")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP19")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP20")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP21")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP22")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP23")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP24")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP25")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP26")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP27")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP28")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP29")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP30")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP31")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP32")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP33")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP34")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP35")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP36")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP37")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP38")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP39")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP40")

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP41")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP42")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP43")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP44")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP45")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP46")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("INP47")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP48")

	PORT_START("X6")  // not used
	PORT_START("X7")  // not used

	PORT_START("DIAGS")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_0_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s3_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Advance") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Manual/Auto") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("DS1")
	PORT_DIPNAME( 0xf0, 0xf0, "Data units" )
	PORT_DIPSETTING(    0xf0, "0" )
	PORT_DIPSETTING(    0x70, "1" )
	PORT_DIPSETTING(    0xb0, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0xd0, "4" )
	PORT_DIPSETTING(    0x50, "5" )
	PORT_DIPSETTING(    0x90, "6" )
	PORT_DIPSETTING(    0x10, "7" )
	PORT_DIPSETTING(    0xe0, "8" )
	PORT_DIPSETTING(    0x60, "9" )
	PORT_DIPNAME( 0x0f, 0x0f, "Data tens" )
	PORT_DIPSETTING(    0x0f, "0" )
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x0b, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x0d, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x09, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x0e, "8" )
	PORT_DIPSETTING(    0x06, "9" )

	PORT_START("DS2")
	PORT_DIPNAME( 0xff, 0xff, "Function" )
	PORT_DIPSETTING(    0xff, "0" )
	PORT_DIPSETTING(    0x7f, "1" )
	PORT_DIPSETTING(    0xbf, "2" )
	PORT_DIPSETTING(    0x3f, "3" )
	PORT_DIPSETTING(    0xdf, "4" )
	PORT_DIPSETTING(    0x5f, "5" )
	PORT_DIPSETTING(    0x9f, "6" )
	PORT_DIPSETTING(    0x1f, "7" )
	PORT_DIPSETTING(    0xef, "8" )
	PORT_DIPSETTING(    0x6f, "9" )
	PORT_DIPSETTING(    0xaf, "10" )
	PORT_DIPSETTING(    0x2f, "11" )
	PORT_DIPSETTING(    0xcf, "12" )
	PORT_DIPSETTING(    0x4f, "13" )
	PORT_DIPSETTING(    0x8f, "14" )
	PORT_DIPSETTING(    0x0f, "15" )
	PORT_DIPSETTING(    0xf7, "16" )
	PORT_DIPSETTING(    0x77, "17" )
	PORT_DIPSETTING(    0xb7, "18" )
	PORT_DIPSETTING(    0x37, "19" )
	PORT_DIPSETTING(    0xd7, "20" )
	PORT_DIPSETTING(    0x57, "21" )
	PORT_DIPSETTING(    0x97, "22" )
	PORT_DIPSETTING(    0x17, "23" )
	PORT_DIPSETTING(    0xe7, "24" )
	PORT_DIPSETTING(    0x67, "25" )
	PORT_DIPSETTING(    0xa7, "26" )
	PORT_DIPSETTING(    0x27, "27" )
	PORT_DIPSETTING(    0xc7, "28" )
	PORT_DIPSETTING(    0x47, "29" )
	PORT_DIPSETTING(    0x87, "30" )
	PORT_DIPSETTING(    0x07, "31" )
INPUT_PORTS_END

// Unassigned inputs will tilt or reset the machine, so remove them
static INPUT_PORTS_START( httip )
	PORT_INCLUDE(s3)
	PORT_MODIFY("X0")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // Playfield tilt
	PORT_MODIFY("X4")
	PORT_BIT( 0xe8, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X5")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( lucky )
	PORT_INCLUDE(s3)
	PORT_MODIFY("X0")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X2")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_MODIFY("X3")
	PORT_BIT( 0x83, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X5")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( wldcp )
	PORT_INCLUDE(s3)
	PORT_MODIFY("X0")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X1")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) // Playfield tilt
	PORT_MODIFY("X4")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X5")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( cntct )
	PORT_INCLUDE(s3)
	PORT_MODIFY("X0")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // Playfield tilt
	PORT_MODIFY("X5")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( disco )
	PORT_INCLUDE(s3)
	PORT_MODIFY("X0")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X2")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_MODIFY("X3")
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X4")
	PORT_BIT( 0x18, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X5")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( phnix )
	PORT_INCLUDE(s3)
	PORT_MODIFY("X2")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) // Playfield tilt
	PORT_MODIFY("X4")
	PORT_BIT( 0x0a, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
INPUT_PORTS_END

static INPUT_PORTS_START( pkrno )
	PORT_INCLUDE(s3)
	PORT_MODIFY("X2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // Playfield tilt
	PORT_MODIFY("X3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_MODIFY("X5")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void s3_state::machine_start()
{
	genpin_class::machine_start();
	m_io_outputs.resolve();
	m_digits.resolve();
	m_leds.resolve();

	save_item(NAME(m_t_c));
	save_item(NAME(m_strobe));
	save_item(NAME(m_row));
	save_item(NAME(m_data_ok));
	save_item(NAME(m_lamp_data));
	save_item(NAME(m_game));
}

void s3_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_t_c = 0;
}

INPUT_CHANGED_MEMBER( s3_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void s3_state::sol0_w(u8 data)
{
	if (m_game == 4) // lucky
	{
		if (BIT(data, 5))
			m_samples->start(5, 5); // outhole
	}
	else // httip
	if (BIT(data, 4))
		m_samples->start(5, 5); // outhole

	for (u8 i = 0; i < 8; i++)
		m_io_outputs[i] = BIT(data, i);
}

void s3a_state::s3a_sol0_w(u8 data)
{
	if ((m_game == 2) || (m_game == 3)) // cntct, disco, phnix
	{
		if (BIT(data, 0))
			m_samples->start(5, 5); // outhole
	}
	else // wldcp, pkrno
	if (BIT(data, 4))
		m_samples->start(5, 5); // outhole

	for (u8 i = 0; i < 8; i++)
		m_io_outputs[i] = BIT(data, i);

	// disco has the sound-alternator in a different place
	if (m_game == 3)
	{
		if (data == 0x10)
		{
			m_disco = true;
			m_s4sound->write(0x30);
		}
		else
		if (m_disco)
		{
			m_disco = false;
			m_s4sound->write(0xff);
		}
	}
}

void s3_state::sol1_w(u8 data)
{
	if (BIT(data, 0))
		m_samples->start(4, 4); // 10 chime

	if (BIT(data, 1))
		m_samples->start(1, 1); // 100 chime

	if (BIT(data, 2))
		m_samples->start(2, 2); // 1000 chime

	if (BIT(data, 3))
		m_samples->start(3, 3); // 10k chime

	if (BIT(data, 5))
		m_samples->start(0, 6); // knocker

	for (u8 i = 0; i < 8; i++)
		m_io_outputs[8U+i] = BIT(data, i);
}

void s3a_state::s3a_sol1_w(u8 data)
{
	u8 sound_data = data & 15;

	// wldcp and disco have a dedicated tilt line
	if (((m_game == 1) || (m_game == 3))&& BIT(data, 6))
		sound_data = 0x10;

	// enable alternator line (all except disco)
	if (BIT(data, 4))
		sound_data |= 0x80;

	m_s4sound->write(~sound_data);

	if (BIT(data, 5))
		m_samples->start(0, 6); // knocker

	for (u8 i = 0; i < 8; i++)
		m_io_outputs[8U+i] = BIT(data, i);
}

void s3_state::lamp0_w(u8 data)
{
	m_mainirq->in_clear<0>();
	m_lamp_data = data ^ 0xff;
}

void s3_state::lamp1_w(u8 data)
{
	// find out which row is active
	for (u8 i = 0; i < 8; i++)
		if (BIT(data, i))
			for (u8 j = 0; j < 8; j++)
				m_io_outputs[22U+i*8U+j] = BIT(m_lamp_data, j);
}

u8 s3_state::dips_r()
{
	if (BIT(ioport("DIAGS")->read(), 4))
		return m_dips[BIT(~m_strobe, 1)]->read() << (BIT(m_strobe, 0) ? 4 : 0);

	return 0xff;
}

void s3_state::dig0_w(u8 data)
{
	m_strobe = data & 15;
	m_data_ok = true;
	m_leds[0] = !BIT(data, 4);
	m_leds[1] = !BIT(data, 5);
}

void s3_state::dig1_w(u8 data)
{
	static const u8 patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0,0,0,0,0,0 }; // MC14543
	if (m_data_ok)
	{
		m_digits[m_strobe+16] = patterns[data&15];
		m_digits[m_strobe] = patterns[data>>4];
	}
	m_data_ok = false;
}

u8 s3_state::switch_r()
{
	u8 data = 0;
	// there's hardware for 8 rows, but machines only use 5 or 6
	for (u8 i = 0; i < 6; i++)
		if (BIT(m_row, i))
			data |= m_io_keyboard[i]->read();

	return data;
}

void s3_state::switch_w(u8 data)
{
	m_row = data;
}

TIMER_DEVICE_CALLBACK_MEMBER( s3_state::irq )
{
	if (m_t_c > 0x70)
		m_mainirq->in_set<0>();
	else
		m_t_c++;
}

void s3_state::s3(machine_config &config)
{
	// basic machine hardware
	M6800(config, m_maincpu, 3580000/4);   // 3.58MHz xtal and mc6875
	m_maincpu->set_addrmap(AS_PROGRAM, &s3_state::main_map);
	TIMER(config, "ne556").configure_periodic(FUNC(s3_state::irq), attotime::from_hz(923));  // NE556, freq from online calculator

	// Video
	config.set_default_layout(layout_s3);

	// Sound
	genpin_audio(config);

	// Devices
	PIA6821(config, m_pia22);
	m_pia22->writepa_handler().set(FUNC(s3_state::sol0_w));
	m_pia22->writepb_handler().set(FUNC(s3_state::sol1_w));
	m_pia22->ca2_handler().set(FUNC(s3_state::pia22_ca2_w));
	m_pia22->cb2_handler().set(FUNC(s3_state::pia22_cb2_w));
	m_pia22->irqa_handler().set(m_mainirq, FUNC(input_merger_device::in_w<1>));
	m_pia22->irqb_handler().set(m_mainirq, FUNC(input_merger_device::in_w<2>));

	PIA6821(config, m_pia24);
	m_pia24->writepa_handler().set(FUNC(s3_state::lamp0_w));
	m_pia24->writepb_handler().set(FUNC(s3_state::lamp1_w));
	m_pia24->ca2_handler().set(FUNC(s3_state::pia24_ca2_w));
	m_pia24->cb2_handler().set(FUNC(s3_state::pia24_cb2_w));
	m_pia24->irqa_handler().set(m_mainirq, FUNC(input_merger_device::in_w<3>));
	m_pia24->irqb_handler().set(m_mainirq, FUNC(input_merger_device::in_w<4>));

	PIA6821(config, m_pia28);
	m_pia28->readpa_handler().set(FUNC(s3_state::dips_r));
	m_pia28->set_port_a_input_overrides_output_mask(0xff);
	m_pia28->readca1_handler().set_ioport("DIAGS").bit(2); // advance button
	m_pia28->readcb1_handler().set_ioport("DIAGS").bit(3); // auto/manual switch
	m_pia28->writepa_handler().set(FUNC(s3_state::dig0_w));
	m_pia28->writepb_handler().set(FUNC(s3_state::dig1_w));
	m_pia28->ca2_handler().set(FUNC(s3_state::pia28_ca2_w));
	m_pia28->cb2_handler().set(FUNC(s3_state::pia28_cb2_w));
	m_pia28->irqa_handler().set(m_mainirq, FUNC(input_merger_device::in_w<5>));
	m_pia28->irqb_handler().set(m_mainirq, FUNC(input_merger_device::in_w<6>));

	PIA6821(config, m_pia30);
	m_pia30->readpa_handler().set(FUNC(s3_state::switch_r));
	m_pia30->set_port_a_input_overrides_output_mask(0xff);
	m_pia30->writepb_handler().set(FUNC(s3_state::switch_w));
	m_pia30->ca2_handler().set(FUNC(s3_state::pia30_ca2_w));
	m_pia30->cb2_handler().set(FUNC(s3_state::pia30_cb2_w));
	m_pia30->irqa_handler().set(m_mainirq, FUNC(input_merger_device::in_w<7>));
	m_pia30->irqb_handler().set(m_mainirq, FUNC(input_merger_device::in_w<8>));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	INPUT_MERGER_ANY_HIGH(config, m_mainirq).output_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);
}

void s3a_state::s3a(machine_config &config)
{
	s3(config);

	m_pia22->writepa_handler().set(FUNC(s3a_state::s3a_sol0_w));
	m_pia22->writepb_handler().set(FUNC(s3a_state::s3a_sol1_w));

	// Add the soundcard
	SPEAKER(config, "mono").front_center();
	WILLIAMS_S4_SOUND(config, m_s4sound, 0).add_route(ALL_OUTPUTS, "mono", 1.0);
}


//***************************************** SYSTEM 3 ******************************************************


/*----------------------------
/ Hot Tip - Sys.3 (Game #477) - No Sound board
/----------------------------*/
ROM_START(httip_l1)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(b1d4fd9b) SHA1(e55ecf1328a55979c4cf8f3fb4e6761747e0abc4))
	ROM_LOAD("white1.716",   0x1000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_LOAD("white2.716",   0x1800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))
ROM_END

/*---------------------------------
/ Lucky Seven - Sys.3 (Game #480) - No Sound board
/---------------------------------*/
ROM_START(lucky_l1)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(7cfbd4c7) SHA1(825e2245fd1615e932973f5e2b5ed5f2da9309e7))
	ROM_LOAD("white1.716",   0x1000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_LOAD("white2.716",   0x1800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))
ROM_END

/*-------------------------------------
/ World Cup Soccer - Sys.3 (Game #481)
/-------------------------------------*/
ROM_START(wldcp_l1)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(c8071956) SHA1(0452aaf2ec1bcc5717fe52a6c541d79402bebb17))
	ROM_LOAD("white1.716",   0x1000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_LOAD("white2wc.716", 0x1800, 0x0800, CRC(618d15b5) SHA1(527387893eeb2cd4aa563a4cfb1948a15d2ed741))

	ROM_REGION(0x0800, "s4sound:audiocpu", 0)
	ROM_LOAD("481_s0_world_cup.716",   0x0000, 0x0800, CRC(cf012812) SHA1(26074f6a44075a94e6f91de1dbf92f8ec3ff8ca4))
ROM_END

/*-------------------------------------
/ Contact - Sys.3 (Game #482)
/-------------------------------------*/
ROM_START(cntct_l1)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(35359b60) SHA1(ab4c3328d93bdb4c952090b327c91b0ded36152c))
	ROM_LOAD("white1.716",   0x1000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_LOAD("white2.716",   0x1800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))

	ROM_REGION(0x0800, "s4sound:audiocpu", 0)
	ROM_LOAD("482_s0_contact.716",   0x0000, 0x0800, CRC(d3c713da) SHA1(1fc4a8fadf472e9a04b3a86f60a9d625d07764e1))
ROM_END

/*-------------------------------------
/ Disco Fever - Sys.3 (Game #483)
/-------------------------------------*/
ROM_START(disco_l1)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(831d8adb) SHA1(99a9c3d5c8cbcdf3bb9c210ad9d05c34905b272e))
	ROM_LOAD("white1.716",   0x1000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_LOAD("white2.716",   0x1800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))

	ROM_REGION(0x0800, "s4sound:audiocpu", 0)
	ROM_LOAD("483_s0_disco_fever.716",   0x0000, 0x0800, CRC(d1cb5047) SHA1(7f36296975df19feecc6456ffb91f4a23bcad037))
ROM_END

/*--------------------------------
/ Phoenix - Sys.4 (Game #485)
/-------------------------------*/
ROM_START(phnix_l1)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(3aba6eac) SHA1(3a9f669216b3214bc42a1501aa2b10cfbcc36315))
	ROM_LOAD("white1.716",   0x1000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_LOAD("white2.716",   0x1800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))

	ROM_REGION(0x0800, "s4sound:audiocpu", 0)
	ROM_LOAD("485_s0_phoenix.716",   0x0000, 0x0800, CRC(1c3dea6e) SHA1(04bfe952be2eab66f023b204c21a1bd461ea572f))
ROM_END

/*--------------------------------
/ Pokerino - Sys.4 (Game #488)
/-------------------------------*/
ROM_START(pkrno_l1)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(9b4d01a8) SHA1(1bd51745f38381ffc66fde4b28b76aab33b573ca))
	ROM_LOAD("white1.716",   0x1000, 0x0800, CRC(9bbbf14f) SHA1(b0542ffdd683fa0ea4a9819576f3789cd5a4b2eb))
	ROM_LOAD("white2.716",   0x1800, 0x0800, CRC(4d4010dd) SHA1(11221124fef3b7bf82d353d65ce851495f6946a7))

	ROM_REGION(0x0800, "s4sound:audiocpu", 0)
	ROM_LOAD("488_s0_pokerino.716",   0x0000, 0x0800, CRC(5de02e62) SHA1(f838439a731511a264e508a576ae7193d9fed1af))
ROM_END

} // Anonymous namespace

// Chimes
GAME( 1977, httip_l1, 0, s3,  httip, s3_state,  empty_init, ROT0, "Williams", "Hot Tip (L-1)",          MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1977, lucky_l1, 0, s3,  lucky, s3_state,  init_4,     ROT0, "Williams", "Lucky Seven (L-1)",      MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )

// Sound board
GAME( 1978, wldcp_l1, 0, s3a, wldcp, s3a_state, init_1,     ROT0, "Williams", "World Cup (L-1)",        MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1978, disco_l1, 0, s3a, disco, s3a_state, init_3,     ROT0, "Williams", "Disco Fever (L-1)",      MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1978, cntct_l1, 0, s3a, cntct, s3a_state, init_2,     ROT0, "Williams", "Contact (L-1)",          MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1978, phnix_l1, 0, s3a, phnix, s3a_state, init_2,     ROT0, "Williams", "Phoenix (L-1)",          MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1978, pkrno_l1, 0, s3a, pkrno, s3a_state, empty_init, ROT0, "Williams", "Pokerino (L-1)",         MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
