// license:BSD-3-Clause
// copyright-holders:Robbbert
/***********************************************************************************

PINBALL
Williams/United Shuffle-Alley games

Games:
- Topaz (s4)
- Pompei (s4)
- Aristocrat (s4)
- Taurus (s4)
- King Tut (s4)(#912)
- Omni (s4)
- Big Strike (s4)
- Triple Strike (s4)
- Strike Zone (s9)(#916)
- Alley Cats (s11)(#918)
- Tic-Tac-Strike (s11a)(#919)
- Gold Mine (s11b)(#920)
- Top Dawg (s11b)(#921)
- Shuffle Inn (s11b)(#922)

The first time run, the machine will show some numbers. Press F3 to clear this.

Each game consists of a flat board with an air-driven puck and 10 bowling pins.
 You must push the puck as if it was a bowling ball, and score strikes and spares.
 The displays have 4 digits and 6 can play. The board has contactors to detect
 the passage of the puck, and the machine will lift the bowling pins out of the
 way, so in fact the puck never touches the pins at all.

Status:
- All games are playable.
- To play: 5,1,(optional: 3 to select game type), any of keys A-W,Y,Z,comma,period
    to activate puck detectors, X to signal end. Press it twice to end a frame.
- To score a strike, press ABCDFGIX one at a time.

ToDo:
- Only 2 manuals found, and only one schematic, so it's largely guesswork.
- Roms missing

************************************************************************************/

#include "emu.h"
#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "audio/williams.h"
#include "machine/6821pia.h"
#include "machine/clock.h"
#include "machine/input_merger.h"
#include "machine/ripple_counter.h"
#include "speaker.h"

#include "shuffle4.lh"
#include "shuffle9.lh"
#include "shuffle11.lh"


namespace {

class shuffle_state : public genpin_class
{
public:
	shuffle_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainirq(*this, "mainirq")
		, m_s4sound(*this, "s4sound")
		, m_s9sound(*this, "s9sound")
		, m_s11sound(*this, "s11sound")
		, m_pia21(*this, "pia21")
		, m_pia22(*this, "pia22")
		, m_pia24(*this, "pia24")
		, m_pia28(*this, "pia28")
		, m_pia2c(*this, "pia2c")
		, m_pia30(*this, "pia30")
		, m_pia34(*this, "pia34")
		, m_4020(*this, "4020")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_dips(*this, "DS%d", 1U)
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void s4(machine_config &config);
	void s9(machine_config &config);
	void s11(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void dig0_w(u8 data);
	void dig1_w(u8 data);
	void lamp0_w(u8 data);
	void lamp1_w(u8 data);
	void sol0_w(u8 data);
	void sol1_w(u8 data);
	void sol2_w(u8 data);
	void sol3_w(u8 data);
	void pia2c_pa_w(u8 data) { }
	void pia2c_pb_w(u8 data) { }
	void pia34_pa_w(u8 data) { m_data_ok = false; }
	void pia34_pb_w(u8 data) { }
	u8 dips_r();
	u8 switch_r();
	void switch_w(u8 data);
	void clockcnt_w(u16 data);
	u8 m_strobe = 0;
	u8 m_row = 0;
	bool m_data_ok = 0;
	u8 m_lamp_data = 0;
	bool m_irq_in_progress = 0;
	DECLARE_WRITE_LINE_MEMBER(pia21_cb2_w) { } // enable solenoids
	DECLARE_WRITE_LINE_MEMBER(pia22_ca2_w) { } //ST5
	DECLARE_WRITE_LINE_MEMBER(pia22_cb2_w) { } //ST-solenoids enable
	DECLARE_WRITE_LINE_MEMBER(pia24_ca2_w) { } //ST2
	DECLARE_WRITE_LINE_MEMBER(pia24_cb2_w) { } //ST1
	DECLARE_WRITE_LINE_MEMBER(pia28_ca2_w) { } //diag leds enable
	DECLARE_WRITE_LINE_MEMBER(pia28_cb2_w) { } //ST6
	DECLARE_WRITE_LINE_MEMBER(pia30_ca2_w) { } //ST4
	DECLARE_WRITE_LINE_MEMBER(pia30_cb2_w) { } //ST3
	DECLARE_WRITE_LINE_MEMBER(irq_w);
	void s4_map(address_map &map);
	void s9_map(address_map &map);
	void s11_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<input_merger_device> m_mainirq;
	optional_device<williams_s4_sound_device> m_s4sound;
	optional_device<williams_s9_sound_device> m_s9sound;
	optional_device<williams_s11_sound_device> m_s11sound;
	optional_device<pia6821_device> m_pia21;
	optional_device<pia6821_device> m_pia22;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	optional_device<pia6821_device> m_pia2c;
	required_device<pia6821_device> m_pia30;
	optional_device<pia6821_device> m_pia34;
	required_device<ripple_counter_device> m_4020;
	required_ioport_array<8> m_io_keyboard;
	required_ioport_array<2> m_dips;
	output_finder<32> m_digits;
	output_finder<96> m_io_outputs; // 32 solenoids + 64 lamps
};


void shuffle_state::s4_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x00ff).ram();
	map(0x0100, 0x01ff).ram().share("nvram");
	map(0x2200, 0x2203).rw(m_pia22, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // solenoids
	map(0x2400, 0x2403).rw(m_pia24, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // lamps
	map(0x2800, 0x2803).rw(m_pia28, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // display
	map(0x3000, 0x3003).rw(m_pia30, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // inputs
	map(0x4000, 0x7fff).rom().region("maincpu", 0);
}

void shuffle_state::s9_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x2100, 0x2103).rw(m_pia21, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // sound+solenoids
	map(0x2200, 0x2200).w(FUNC(shuffle_state::sol3_w)); // solenoids
	map(0x2400, 0x2403).rw(m_pia24, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // lamps
	map(0x2800, 0x2803).rw(m_pia28, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // display
	map(0x3000, 0x3003).rw(m_pia30, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // inputs
	map(0x4000, 0x7fff).rom().region("maincpu", 0);
}

void shuffle_state::s11_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x2100, 0x2103).rw(m_pia21, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // sound+solenoids
	map(0x2200, 0x2200).w(FUNC(shuffle_state::sol3_w)); // solenoids
	map(0x2400, 0x2403).rw(m_pia24, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // lamps
	map(0x2800, 0x2803).rw(m_pia28, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // display
	map(0x2c00, 0x2c03).rw(m_pia2c, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // alphanumeric display
	map(0x3000, 0x3003).rw(m_pia30, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // inputs
	map(0x3400, 0x3403).rw(m_pia34, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // widget
	map(0x4000, 0x7fff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START( s4 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Select Game")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("High Score Reset")

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
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Back Row")

	PORT_START("X5")  // not used
	PORT_START("X6")  // not used
	PORT_START("X7")  // not used

	PORT_START("DIAGS")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_0_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, shuffle_state, main_nmi, 1)
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

static INPUT_PORTS_START( s9 )
	PORT_INCLUDE(s4)
	PORT_MODIFY("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Select Game")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("High Score Reset")

	PORT_MODIFY("X4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Ticket Dispenser")
INPUT_PORTS_END

void shuffle_state::clockcnt_w(u16 data)
{
	// A wire jumper allows selection of 7,8,9, or 8,9,10
	m_mainirq->in_w<0>(BIT(data, 7, 3)==7);

	if (BIT(data, 5) && m_irq_in_progress)
		m_4020->reset_w(1);
	else
		m_4020->reset_w(0);
}

INPUT_CHANGED_MEMBER( shuffle_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void shuffle_state::sol0_w(u8 data)
{
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[i] = BIT(data, i);
}

void shuffle_state::sol1_w(u8 data)
{
	if (m_s4sound)
	{
		u8 sound_data = BIT(data, 4, 4);
		m_s4sound->write(~sound_data);
	}

	for (u8 i = 0; i < 8; i++)
		m_io_outputs[8U+i] = BIT(data, i);
}

void shuffle_state::sol2_w(u8 data)
{
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[16U+i] = BIT(data, i);
}

void shuffle_state::sol3_w(u8 data)
{
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[24U+i] = BIT(data, i);
}

void shuffle_state::lamp0_w(u8 data)
{
	m_mainirq->in_clear<0>();
	m_lamp_data = data ^ 0xff;
}

void shuffle_state::lamp1_w(u8 data)
{
	// find out which row is active
	for (u8 i = 0; i < 8; i++)
		if (BIT(data, i))
			for (u8 j = 0; j < 8; j++)
				m_io_outputs[32U+i*8U+j] = BIT(m_lamp_data, j);
}

u8 shuffle_state::dips_r()
{
	if (BIT(ioport("DIAGS")->read(), 4))
		return m_dips[BIT(~m_strobe, 1)]->read() << (BIT(m_strobe, 0) ? 4 : 0);

	return 0xff;
}

void shuffle_state::dig0_w(u8 data)
{
	if (data < 0x90)
	{
		m_strobe = data & 15;
		m_data_ok = true;
	}
}

void shuffle_state::dig1_w(u8 data)
{
	static const u8 patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0,0,0,0,0,0 }; // MC14543
	if (m_data_ok)
	{
		m_digits[m_strobe+16] = patterns[data&15];
		m_digits[m_strobe] = patterns[data>>4];
	}
	m_data_ok = false;
}

u8 shuffle_state::switch_r()
{
	u8 data = 0;
	for (u8 i = 0; i < 8; i++)
		if (BIT(m_row, i))
			data |= m_io_keyboard[i]->read();

	return data;
}

void shuffle_state::switch_w(u8 data)
{
	m_row = data;
}

WRITE_LINE_MEMBER( shuffle_state::irq_w )
{
	m_irq_in_progress = state;
	m_maincpu->set_input_line(M6802_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

void shuffle_state::machine_start()
{
	genpin_class::machine_start();
	m_io_outputs.resolve();
	m_digits.resolve();

	save_item(NAME(m_irq_in_progress));
	save_item(NAME(m_strobe));
	save_item(NAME(m_row));
	save_item(NAME(m_data_ok));
	save_item(NAME(m_lamp_data));
}

void shuffle_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_irq_in_progress = 0;
}

void shuffle_state::s4(machine_config &config)
{
	// basic machine hardware
	M6802(config, m_maincpu, 3580000); // Divided by 4 internally
	m_maincpu->set_addrmap(AS_PROGRAM, &shuffle_state::s4_map);

	// Video
	config.set_default_layout(layout_shuffle4);

	// Sound
	genpin_audio(config);

	// Devices
	PIA6821(config, m_pia22, 0);
	m_pia22->writepa_handler().set(FUNC(shuffle_state::sol0_w));
	m_pia22->writepb_handler().set(FUNC(shuffle_state::sol1_w));
	m_pia22->ca2_handler().set(FUNC(shuffle_state::pia22_ca2_w));
	m_pia22->cb2_handler().set(FUNC(shuffle_state::pia22_cb2_w));
	m_pia22->irqa_handler().set(m_mainirq, FUNC(input_merger_device::in_w<1>));
	m_pia22->irqb_handler().set(m_mainirq, FUNC(input_merger_device::in_w<2>));

	PIA6821(config, m_pia24, 0);
	m_pia24->writepa_handler().set(FUNC(shuffle_state::lamp0_w));
	m_pia24->writepb_handler().set(FUNC(shuffle_state::lamp1_w));
	m_pia24->ca2_handler().set(FUNC(shuffle_state::pia24_ca2_w));
	m_pia24->cb2_handler().set(FUNC(shuffle_state::pia24_cb2_w));
	m_pia24->irqa_handler().set(m_mainirq, FUNC(input_merger_device::in_w<3>));
	m_pia24->irqb_handler().set(m_mainirq, FUNC(input_merger_device::in_w<4>));

	PIA6821(config, m_pia28, 0);
	m_pia28->readpa_handler().set(FUNC(shuffle_state::dips_r));
	m_pia28->readca1_handler().set_ioport("DIAGS").bit(2); // advance button
	m_pia28->readcb1_handler().set_ioport("DIAGS").bit(3); // auto/manual switch
	m_pia28->writepa_handler().set(FUNC(shuffle_state::dig0_w));
	m_pia28->writepb_handler().set(FUNC(shuffle_state::dig1_w));
	m_pia28->ca2_handler().set(FUNC(shuffle_state::pia28_ca2_w));
	m_pia28->cb2_handler().set(FUNC(shuffle_state::pia28_cb2_w));
	m_pia28->irqa_handler().set(m_mainirq, FUNC(input_merger_device::in_w<5>));
	m_pia28->irqb_handler().set(m_mainirq, FUNC(input_merger_device::in_w<6>));

	PIA6821(config, m_pia30, 0);
	m_pia30->readpa_handler().set(FUNC(shuffle_state::switch_r));
	m_pia30->set_port_a_input_overrides_output_mask(0xff);
	m_pia30->writepb_handler().set(FUNC(shuffle_state::switch_w));
	m_pia30->ca2_handler().set(FUNC(shuffle_state::pia30_ca2_w));
	m_pia30->cb2_handler().set(FUNC(shuffle_state::pia30_cb2_w));
	m_pia30->irqa_handler().set(m_mainirq, FUNC(input_merger_device::in_w<7>));
	m_pia30->irqb_handler().set(m_mainirq, FUNC(input_merger_device::in_w<8>));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	INPUT_MERGER_ANY_HIGH(config, m_mainirq).output_handler().set(FUNC(shuffle_state::irq_w));

	RIPPLE_COUNTER(config, m_4020);
	m_4020->set_stages(14); // Using Q5,Q8,Q9
	m_4020->count_out_cb().set(FUNC(shuffle_state::clockcnt_w));

	CLOCK(config, "rclock", 3580000/4).signal_handler().set(m_4020, FUNC(ripple_counter_device::clock_w));

	// Add the soundcard
	SPEAKER(config, "mono").front_center();
	WILLIAMS_S4_SOUND(config, m_s4sound, 0).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void shuffle_state::s9(machine_config &config)
{
	s4(config);
	config.device_remove("pia22");
	config.device_remove("s4sound");

	M6802(config.replace(), m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &shuffle_state::s9_map);

	config.set_default_layout(layout_shuffle9);

	PIA6821(config, m_pia21, 0);
	m_pia21->writepa_handler().set("s9sound", FUNC(williams_s9_sound_device::write));
	m_pia21->writepb_handler().set(FUNC(shuffle_state::sol2_w));
	m_pia21->ca2_handler().set("s9sound", FUNC(williams_s9_sound_device::strobe));
	m_pia21->cb2_handler().set(FUNC(shuffle_state::pia21_cb2_w));
	m_pia21->irqa_handler().set(m_mainirq, FUNC(input_merger_device::in_w<9>));
	m_pia21->irqb_handler().set(m_mainirq, FUNC(input_merger_device::in_w<10>));

	WILLIAMS_S9_SOUND(config, m_s9sound, 0).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void shuffle_state::s11(machine_config &config)
{
	s9(config);
	config.device_remove("s9sound");
	m_maincpu->set_addrmap(AS_PROGRAM, &shuffle_state::s11_map);

	m_pia21->writepa_handler().set("s11sound", FUNC(williams_s11_sound_device::write));
	m_pia21->ca2_handler().set("s11sound", FUNC(williams_s11_sound_device::strobe));

	config.set_default_layout(layout_shuffle11);

	PIA6821(config, m_pia2c, 0);
	m_pia2c->writepa_handler().set(FUNC(shuffle_state::pia2c_pa_w));
	m_pia2c->writepb_handler().set(FUNC(shuffle_state::pia2c_pb_w));
	m_pia2c->irqa_handler().set(m_mainirq, FUNC(input_merger_device::in_w<11>));
	m_pia2c->irqb_handler().set(m_mainirq, FUNC(input_merger_device::in_w<12>));

	PIA6821(config, m_pia34, 0);
	m_pia34->writepa_handler().set(FUNC(shuffle_state::pia34_pa_w));
	m_pia34->writepb_handler().set(FUNC(shuffle_state::pia34_pb_w));
	//m_pia34->cb2_handler().set(FUNC(shuffle_state::pia34_cb2_w));
	m_pia34->irqa_handler().set(m_mainirq, FUNC(input_merger_device::in_w<13>));
	m_pia34->irqb_handler().set(m_mainirq, FUNC(input_merger_device::in_w<14>));

	WILLIAMS_S11_SOUND(config, m_s11sound, 0).add_route(ALL_OUTPUTS, "mono", 1.0);
}


/*----------------------------
/ Topaz
/----------------------------*/
ROM_START(topaz_l1)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom.716",  0x2000, 0x0800, CRC(cb287b10) SHA1(7fb6b6a26237cf85d5e02cf35271231267f90fc1))
	ROM_LOAD("b_ic20.716",   0x3000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716",   0x3800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x0800, "s4sound:audiocpu", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

/*----------------------------
/ Pompeii
/----------------------------*/
ROM_START(pomp_l1)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom.716",  0x2000, 0x0800, CRC(0f069ac2) SHA1(d651d49cdb50cf444e420241a1f9ed48c878feee))
	ROM_LOAD("b_ic20.716",   0x3000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716",   0x3800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x0800, "s4sound:audiocpu", 0)
	ROM_LOAD("soundx.716",   0x0000, 0x0800, CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
ROM_END

/*----------------------------------
/ Aristocrat (same roms as Pompeii)
/----------------------------------*/
ROM_START(arist_l1)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom.716",  0x2000, 0x0800, CRC(0f069ac2) SHA1(d651d49cdb50cf444e420241a1f9ed48c878feee))
	ROM_LOAD("b_ic20.716",   0x3000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716",   0x3800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x0800, "s4sound:audiocpu", 0)
	ROM_LOAD("soundx.716",   0x0000, 0x0800, CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
ROM_END

/*----------------------------
/ Taurus
/----------------------------*/
ROM_START(taurs_l1)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom.716",  0x2000, 0x0800, CRC(3246e285) SHA1(4f76784ecb5063a49c24795ae61db043a51e2c89))
	ROM_LOAD("b_ic20.716",   0x3000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716",   0x3800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x0800, "s4sound:audiocpu", 0)
	ROM_LOAD("soundx.716",   0x0000, 0x0800, CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
ROM_END

/*----------------------------
/ King Tut (#912)
/----------------------------*/
ROM_START(kingt_l1)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom.716",  0x2000, 0x0800, CRC(54d3280a) SHA1(ca74636e35d2c3e0b3133f89b1ff1233d5d72a5c))
	ROM_LOAD("b_ic20.716",   0x3000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716",   0x3800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x0800, "s4sound:audiocpu", 0)
	ROM_LOAD("soundx.716",   0x0000, 0x0800, CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
ROM_END

/*----------------------------
/ Omni
/----------------------------*/
ROM_START(omni_l1)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("omni-1a.u21",  0x2000, 0x0800, CRC(443bd170) SHA1(cc1ebd72d77ec2014cbd84534380e5ea1f12c022))
	ROM_LOAD("b_ic20.716",   0x3000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716",   0x3800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x0800, "s4sound:audiocpu", 0)
	ROM_LOAD("sound.716",    0x0000, 0x0800, CRC(db085cbb) SHA1(9a57abbad183ba16b3dba16d16923c3bfc46a0c3))
ROM_END

/*----------------------------
/ Big Strike
/----------------------------*/
ROM_START(bstrk_l1)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom.716",  0x2000, 0x0800, CRC(323dbcde) SHA1(a75cbb5de97cb9afc1d36e9b6ff593bb482fcf8b))
	ROM_LOAD("b_ic20.716",   0x3000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716",   0x3800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x0800, "s4sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("sound.716",    0x0000, 0x0800, NO_DUMP)
	// small program to stop the error log from filling up
	ROM_FILL(0x05dd,1,0x7e)
	ROM_FILL(0x05de,2,0xdd)
	ROM_FILL(0x07fe,2,0xdd)
ROM_END

/*----------------------------
/ Triple Strike
/----------------------------*/
ROM_START(tstrk_l1)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom.716",  0x2000, 0x0800, CRC(b034c059) SHA1(76b3926b87b3c137fcaf33021a586827e3c030af))
	ROM_LOAD("ic20.716",     0x3000, 0x0800, CRC(f163fc88) SHA1(988b60626f3d4dc8f4a1dbd0c99282418bc53aae))
	ROM_LOAD("b_ic17.716",   0x3800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x0800, "s4sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("sound.716",    0x0000, 0x0800, NO_DUMP)
	// small program to stop the error log from filling up
	ROM_FILL(0x05dd,1,0x7e)
	ROM_FILL(0x05de,2,0xdd)
	ROM_FILL(0x07fe,2,0xdd)
ROM_END

/*--------------------------------
/ Strike Zone (#916)
/---------------------------------*/
ROM_START(szone_l5)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("sz_u19r5.732", 0x1000, 0x1000, CRC(c79c46cb) SHA1(422ba74ae67bebbe02f85a9a8df0e3072f3cebc0))
	ROM_LOAD("sz_u20r5.764", 0x2000, 0x2000, CRC(9b5b3be2) SHA1(fce051a60b6eecd9bc07273892b14046b251b372))

	ROM_REGION(0x8000, "s9sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("szs_u49.128",  0x4000, 0x4000, CRC(144c3c07) SHA1(57be6f336f200079cd698b13f8fa4755cf694274))
ROM_END

ROM_START(szone_l2)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("sz_u19r2.732", 0x1000, 0x1000, CRC(c0e4238b) SHA1(eae60ccd5b5001671cd6d2685fd588494d052d1e))
	ROM_LOAD("sz_u20r2.764", 0x2000, 0x2000, CRC(91c08137) SHA1(86da08f346f85810fceceaa7b9824ab76a68da54))

	ROM_REGION(0x8000, "s9sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("szs_u49.128",  0x4000, 0x4000, CRC(144c3c07) SHA1(57be6f336f200079cd698b13f8fa4755cf694274))
ROM_END

/*------------------------------
/ Alley Cats (#918)
/-------------------------------*/
ROM_START(alcat_l7)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("u26_rev7.rom", 0x1000, 0x1000, CRC(4d274dd3) SHA1(80d72bd0f85ce2cac04f6d9f59dc1fcccc86d402))
	ROM_LOAD("u27_rev7.rom", 0x2000, 0x2000, CRC(9c7faf8a) SHA1(dc1a561948b9a303f7924d7bebcd972db766827b))

	ROM_REGION(0x10000, "s11sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("acs_u21.bin", 0x8000, 0x8000, CRC(c54cd329) SHA1(4b86b10e60a30c4de5d97129074f5657447be676))
	ROM_LOAD("acs_u22.bin", 0x0000, 0x8000, CRC(56c1011a) SHA1(c817a3410c643617f3643897b8f529ae78546b0d))
ROM_END

/*-------------------------
/ Tic-Tac-Strike (#919)
/-------------------------*/
ROM_START(tts_l2)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("u27_l2.128", 0x0000, 0x4000, CRC(edbcab92) SHA1(0f6b2dc01874984f9a17ee873f2fa0b6c9bba5be))

	ROM_REGION(0x10000, "s11sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("tts_u21.256", 0x8000, 0x8000, NO_DUMP)
	ROM_LOAD("tts_u22.256", 0x0000, 0x8000, NO_DUMP)
	// small program to stop the error log from filling up
	ROM_FILL(0x9ddd,1,0x7e)
	ROM_FILL(0x9dde,2,0xdd)
	ROM_FILL(0xbffe,2,0xdd)
ROM_END

ROM_START(tts_l1)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("tts_u27.128", 0x0000, 0x4000, CRC(f540c53c) SHA1(1c7a318278ad1afdcbe6aaf81f9b774882b069d6))

	ROM_REGION(0x10000, "s11sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("tts_u21.256", 0x8000, 0x8000, NO_DUMP)
	ROM_LOAD("tts_u22.256", 0x0000, 0x8000, NO_DUMP)
	// small program to stop the error log from filling up
	ROM_FILL(0x9ddd,1,0x7e)
	ROM_FILL(0x9dde,2,0xdd)
	ROM_FILL(0xbffe,2,0xdd)
ROM_END

/*-------------------------------
/ Gold Mine (#920)
/--------------------------------*/
ROM_START(gmine_l2)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("u27.128", 0x0000, 0x4000, CRC(99c6e049) SHA1(356faec0598a54892050a28857e9eb5cdbf35833))

	ROM_REGION(0x10000, "s11sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("u21.256", 0x8000, 0x8000, CRC(3b801570) SHA1(50b50ff826dcb031a30940fa3099bd3a8d773831))
	ROM_LOAD("u22.256", 0x0000, 0x8000, CRC(08352101) SHA1(a7437847a71cf037a80686292f9616b1e08922df))
ROM_END

/*-------------------------
/ Top Dawg (#921)
/--------------------------*/
ROM_START(tdawg_l1)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("tdu27r1.128", 0x0000, 0x4000, CRC(0b4bb586) SHA1(a927ebf7167609cc84b38c22aa35d0c4d259dd8b))

	ROM_REGION(0x10000, "s11sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("tdsu21r1.256", 0x8000, 0x8000, CRC(6a323227) SHA1(7c7263754e5672c654a2ee9582f0b278e637a909))
	ROM_LOAD("tdsu22r1.256", 0x0000, 0x8000, CRC(58407eb4) SHA1(6bd9b304c88d9470eae5afb6621187f4a8313573))
ROM_END

/*----------------------------
/ Shuffle Inn (#922)
/-----------------------------*/
ROM_START(shfin_l1)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("u27rom-1.rv1", 0x0000, 0x4000, CRC(40cfb74a) SHA1(8cee4212ea8bb6b360060391df3208e1e129d7e5))

	ROM_REGION(0x10000, "s11sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("u21snd-2.rv1", 0x8000, 0x8000, CRC(80ddce05) SHA1(9498260e5ccd2fe0eb03ff321dd34eb945b0213a))
	ROM_LOAD("u22snd-2.rv1", 0x0000, 0x8000, CRC(6894abaf) SHA1(2d661765fbfce33a73a20778c41233c0bd9933e9))
ROM_END

} // Anonymous namespace

GAME( 1978, topaz_l1, 0,        s4,  s4, shuffle_state, empty_init, ROT0, "Williams", "Topaz (Shuffle) (L-1)",                  MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1978, pomp_l1,  0,        s4,  s4, shuffle_state, empty_init, ROT0, "Williams/United", "Pompeii (Shuffle) (L-1)",         MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1978, arist_l1, 0,        s4,  s4, shuffle_state, empty_init, ROT0, "Williams/United", "Aristocrat (Shuffle) (L-1)",      MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1979, taurs_l1, 0,        s4,  s4, shuffle_state, empty_init, ROT0, "Williams", "Taurus (Shuffle) (L-1)",                 MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1979, kingt_l1, 0,        s4,  s4, shuffle_state, empty_init, ROT0, "Williams/United", "King Tut (Shuffle) (L-1)",        MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1980, omni_l1,  0,        s4,  s4, shuffle_state, empty_init, ROT0, "Williams/United", "Omni (Shuffle) (L-1)",            MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1983, bstrk_l1, 0,        s4,  s4, shuffle_state, empty_init, ROT0, "Williams/United", "Big Strike (Shuffle) (L-1)",      MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1983, tstrk_l1, 0,        s4,  s4, shuffle_state, empty_init, ROT0, "Williams/United", "Triple Strike (Shuffle) (L-1)",   MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1984, szone_l5, 0,        s9,  s9, shuffle_state, empty_init, ROT0, "Williams/United", "Strike Zone (Shuffle) (L-5)",     MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1984, szone_l2, szone_l5, s9,  s9, shuffle_state, empty_init, ROT0, "Williams/United", "Strike Zone (Shuffle) (L-2)",     MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1985, alcat_l7, 0,        s11, s9, shuffle_state, empty_init, ROT0, "Williams", "Alley Cats (Shuffle) (L-7)",             MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, tts_l2,   0,        s11, s9, shuffle_state, empty_init, ROT0, "Williams", "Tic-Tac-Strike (Shuffle) (L-2)",         MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, tts_l1,   tts_l2,   s11, s9, shuffle_state, empty_init, ROT0, "Williams", "Tic-Tac-Strike (Shuffle) (L-1)",         MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, gmine_l2, 0,        s11, s9, shuffle_state, empty_init, ROT0, "Williams", "Gold Mine (Shuffle) (L-2)",              MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, tdawg_l1, 0,        s11, s9, shuffle_state, empty_init, ROT0, "Williams", "Top Dawg (Shuffle) (L-1)",               MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, shfin_l1, 0,        s11, s9, shuffle_state, empty_init, ROT0, "Williams", "Shuffle Inn (Shuffle) (L-1)",            MACHINE_IS_SKELETON_MECHANICAL )
