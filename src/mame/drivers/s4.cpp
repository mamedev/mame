// license:BSD-3-Clause
// copyright-holders:Robbbert
/***********************************************************************************

PINBALL
Williams System 4

Differences to system 3:
- Use 4020 instead of NE556 for IRQ circuit
- Use 6802 instead of 6800+6875.
- Soundcard does more sounds, and uses 6808.

Games:
- Flash (#486)
- Trizone (#487)
- Time Warp (#489)
- Stellar Wars (#490)
- Pompei
- Aristocrat
- Topaz
- Taurus
- King Tut
- Omni
- Big Strike
- Triple Strike

The first time run, the pinballs will display will show the model number, while
 the shuffle games show 0200 and 419. Press F3 to clear this.

Phoenix and Pokerino are listed as System 4, but use System 3 ROMs.
 They have been moved to s3.cpp, and are working there.

The "Shuffle" games consist of a flat board with an air-driven puck and 10
 bowling pins. You must push the puck as if it was a bowling ball, and score
 strikes and spares. Since the maximum score is 300, the displays have 4 digits
 and 6 can play. They will most likely be split off to a separate driver.

Pressing NUM-8 will select a different set of sounds.

Status:
- Pinballs are playable.

ToDo:
- Shuffle games: need a layout, and no manuals found. Press 6 to insert coin,
   press 9 to start. Some keys make noises. / key steps to next frame.


************************************************************************************/

#include "emu.h"
#include "machine/genpin.h"

#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/clock.h"
#include "machine/input_merger.h"
#include "machine/ripple_counter.h"
#include "sound/dac.h"
#include "speaker.h"

#include "s4.lh"


namespace {

class s4_state : public genpin_class
{
public:
	s4_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainirq(*this, "mainirq")
		, m_pia22(*this, "pia22")
		, m_pia24(*this, "pia24")
		, m_pia28(*this, "pia28")
		, m_pia30(*this, "pia30")
		, m_4020(*this, "4020")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_dips(*this, "DS%d", 1U)
		, m_digits(*this, "digit%d", 0U)
		, m_leds(*this, "led%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void s4(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void dig0_w(u8 data);
	void dig1_w(u8 data);
	void lamp0_w(u8 data);
	void lamp1_w(u8 data);
	void sol0_w(u8 data);
	u8 dips_r();
	u8 switch_r();
	void switch_w(u8 data);
	void clockcnt_w(u16 data);
	u8 m_strobe = 0;
	u8 m_row = 0;
	bool m_data_ok = 0;
	u8 m_lamp_data = 0;
	bool m_irq_in_progress = 0;
	DECLARE_WRITE_LINE_MEMBER(pia22_ca2_w) { } //ST5
	DECLARE_WRITE_LINE_MEMBER(pia22_cb2_w) { } //ST-solenoids enable
	DECLARE_WRITE_LINE_MEMBER(pia24_ca2_w) { } //ST2
	DECLARE_WRITE_LINE_MEMBER(pia24_cb2_w) { } //ST1
	DECLARE_WRITE_LINE_MEMBER(pia28_ca2_w) { } //diag leds enable
	DECLARE_WRITE_LINE_MEMBER(pia28_cb2_w) { } //ST6
	DECLARE_WRITE_LINE_MEMBER(pia30_ca2_w) { } //ST4
	DECLARE_WRITE_LINE_MEMBER(pia30_cb2_w) { } //ST3
	DECLARE_WRITE_LINE_MEMBER(irq_w);
	void main_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<input_merger_device> m_mainirq;
	required_device<pia6821_device> m_pia22;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia30;
	required_device<ripple_counter_device> m_4020;
	required_ioport_array<8> m_io_keyboard;
	required_ioport_array<2> m_dips;
	output_finder<32> m_digits;
	output_finder<2> m_leds;
	output_finder<80> m_io_outputs; // 16 solenoids + 64 lamps

private:
	void sol1_w(u8 data);
};


class s4a_state : public s4_state
{
public:
	s4a_state(const machine_config &mconfig, device_type type, const char *tag)
		: s4_state(mconfig, type, tag)
		, m_audiocpu(*this, "audiocpu")
		, m_pias(*this, "pias")
		, m_io_snd(*this, "SND")
	{
	}

	void s4a(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);

private:
	virtual void machine_start() override;
	u8 sound_r();
	void s4a_sol1_w(u8 data);
	u8 m_sound_data = 0;
	void audio_map(address_map &map);
	required_device<cpu_device> m_audiocpu;
	required_device<pia6821_device> m_pias;
	required_ioport m_io_snd;
};


void s4_state::main_map(address_map &map)
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

void s4a_state::audio_map(address_map &map)
{
	map.global_mask(0x0fff);
	map(0x0000, 0x00ff).ram();
	map(0x0400, 0x0403).rw(m_pias, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // sounds
	map(0x0800, 0x0fff).rom().region("audiocpu", 0);
}


static INPUT_PORTS_START( s4 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("Plumb Tilt") // 3 touches before it tilts
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("High Score Reset")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
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

	PORT_START("X6")  // used by stlwr
	PORT_START("X7")  // not used

	PORT_START("DIAGS")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_0_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s4_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Advance") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Manual/Auto") PORT_CODE(KEYCODE_6_PAD)
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

static INPUT_PORTS_START( s4a )
	PORT_INCLUDE(s4)

	PORT_START("SND")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Music") PORT_CODE(KEYCODE_8_PAD) PORT_TOGGLE

	PORT_MODIFY("DIAGS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_9_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s4a_state, audio_nmi, 1)
INPUT_PORTS_END

static INPUT_PORTS_START( flash )
	PORT_INCLUDE(s4a)
	PORT_MODIFY("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP09")
	PORT_MODIFY("X5")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) // Playfield tilt
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
INPUT_PORTS_END

static INPUT_PORTS_START( trizn )
	PORT_INCLUDE(s4a)
	PORT_MODIFY("X4")
	PORT_BIT( 0xfa, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X5")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( tmwrp )
	PORT_INCLUDE(s4a)
	PORT_MODIFY("X5")
	PORT_BIT( 0xf4, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( stlwr )
	PORT_INCLUDE(s4a)
	PORT_MODIFY("X1")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X4")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_MODIFY("X6")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP50")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP51")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP52")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP53")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP54")
INPUT_PORTS_END

void s4_state::clockcnt_w(u16 data)
{
	if (data >= 1536)
		m_mainirq->in_set<0>();
	if (BIT(data, 5) && m_irq_in_progress)
		m_4020->reset_w(1);
	else
		m_4020->reset_w(0);
}

void s4_state::machine_start()
{
	genpin_class::machine_start();
	m_io_outputs.resolve();
	m_digits.resolve();
	m_leds.resolve();

	save_item(NAME(m_irq_in_progress));
	save_item(NAME(m_strobe));
	save_item(NAME(m_row));
	save_item(NAME(m_data_ok));
	save_item(NAME(m_lamp_data));
}

void s4a_state::machine_start()
{
	s4_state::machine_start();
	save_item(NAME(m_sound_data));
}

void s4_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_irq_in_progress = 0;
}

INPUT_CHANGED_MEMBER( s4_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INPUT_CHANGED_MEMBER( s4a_state::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void s4_state::sol0_w(u8 data)
{
	if (BIT(data, 0))
		m_samples->start(5, 5); // outhole

	for (u8 i = 0; i < 8; i++)
		m_io_outputs[i] = BIT(data, i);
}

void s4_state::sol1_w(u8 data)
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

void s4a_state::s4a_sol1_w(u8 data)
{
	u8 sound_data = m_io_snd->read() | (data & 0x1f);

	bool cb1 = (sound_data & 0x1f);

	if (cb1)
		m_sound_data = ~sound_data;

	m_pias->cb1_w(cb1);

	if (BIT(data, 5))
		m_samples->start(0, 6); // knocker

	for (u8 i = 0; i < 8; i++)
		m_io_outputs[8U+i] = BIT(data, i);
}

void s4_state::lamp0_w(u8 data)
{
	m_mainirq->in_clear<0>();
	m_lamp_data = data ^ 0xff;
}

void s4_state::lamp1_w(u8 data)
{
	// find out which row is active
	for (u8 i = 0; i < 8; i++)
		if (BIT(data, i))
			for (u8 j = 0; j < 8; j++)
				m_io_outputs[16U+i*8U+j] = BIT(m_lamp_data, j);
}

u8 s4_state::dips_r()
{
	if (BIT(ioport("DIAGS")->read(), 4))
		return m_dips[BIT(~m_strobe, 1)]->read() << (BIT(m_strobe, 0) ? 4 : 0);

	return 0xff;
}

void s4_state::dig0_w(u8 data)
{
	m_strobe = data & 15;
	m_data_ok = true;
	m_leds[0] = !BIT(data, 4);
	m_leds[1] = !BIT(data, 5);
}

void s4_state::dig1_w(u8 data)
{
	static const u8 patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0,0,0,0,0,0 }; // MC14543
	if (m_data_ok)
	{
		m_digits[m_strobe+16] = patterns[data&15];
		m_digits[m_strobe] = patterns[data>>4];
	}
	m_data_ok = false;
}

u8 s4_state::switch_r()
{
	u8 data = 0;
	// there's hardware for 8 rows, but machines use 7 max
	for (u8 i = 0; i < 7; i++)
		if (BIT(m_row, i))
			data |= m_io_keyboard[i]->read();

	return data;
}

void s4_state::switch_w(u8 data)
{
	m_row = data;
}

u8 s4a_state::sound_r()
{
	return m_sound_data;
}

WRITE_LINE_MEMBER( s4_state::irq_w )
{
	m_irq_in_progress = state;
	m_maincpu->set_input_line(M6802_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

void s4_state::s4(machine_config &config)
{
	// basic machine hardware
	M6802(config, m_maincpu, 3580000); // Divided by 4 internally
	m_maincpu->set_addrmap(AS_PROGRAM, &s4_state::main_map);

	// Video
	config.set_default_layout(layout_s4);

	// Sound
	genpin_audio(config);

	// Devices
	PIA6821(config, m_pia22, 0);
	m_pia22->writepa_handler().set(FUNC(s4_state::sol0_w));
	m_pia22->writepb_handler().set(FUNC(s4_state::sol1_w));
	m_pia22->ca2_handler().set(FUNC(s4_state::pia22_ca2_w));
	m_pia22->cb2_handler().set(FUNC(s4_state::pia22_cb2_w));
	m_pia22->irqa_handler().set(m_mainirq, FUNC(input_merger_device::in_w<1>));
	m_pia22->irqb_handler().set(m_mainirq, FUNC(input_merger_device::in_w<2>));

	PIA6821(config, m_pia24, 0);
	m_pia24->writepa_handler().set(FUNC(s4_state::lamp0_w));
	m_pia24->writepb_handler().set(FUNC(s4_state::lamp1_w));
	m_pia24->ca2_handler().set(FUNC(s4_state::pia24_ca2_w));
	m_pia24->cb2_handler().set(FUNC(s4_state::pia24_cb2_w));
	m_pia24->irqa_handler().set(m_mainirq, FUNC(input_merger_device::in_w<3>));
	m_pia24->irqb_handler().set(m_mainirq, FUNC(input_merger_device::in_w<4>));

	PIA6821(config, m_pia28, 0);
	m_pia28->readpa_handler().set(FUNC(s4_state::dips_r));
	m_pia28->set_port_a_input_overrides_output_mask(0xff);
	m_pia28->readca1_handler().set_ioport("DIAGS").bit(2); // advance button
	m_pia28->readcb1_handler().set_ioport("DIAGS").bit(3); // auto/manual switch
	m_pia28->writepa_handler().set(FUNC(s4_state::dig0_w));
	m_pia28->writepb_handler().set(FUNC(s4_state::dig1_w));
	m_pia28->ca2_handler().set(FUNC(s4_state::pia28_ca2_w));
	m_pia28->cb2_handler().set(FUNC(s4_state::pia28_cb2_w));
	m_pia28->irqa_handler().set(m_mainirq, FUNC(input_merger_device::in_w<5>));
	m_pia28->irqb_handler().set(m_mainirq, FUNC(input_merger_device::in_w<6>));

	PIA6821(config, m_pia30, 0);
	m_pia30->readpa_handler().set(FUNC(s4_state::switch_r));
	m_pia30->set_port_a_input_overrides_output_mask(0xff);
	m_pia30->writepb_handler().set(FUNC(s4_state::switch_w));
	m_pia30->ca2_handler().set(FUNC(s4_state::pia30_ca2_w));
	m_pia30->cb2_handler().set(FUNC(s4_state::pia30_cb2_w));
	m_pia30->irqa_handler().set(m_mainirq, FUNC(input_merger_device::in_w<7>));
	m_pia30->irqb_handler().set(m_mainirq, FUNC(input_merger_device::in_w<8>));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	INPUT_MERGER_ANY_HIGH(config, m_mainirq).output_handler().set(FUNC(s4_state::irq_w));

	RIPPLE_COUNTER(config, m_4020);
	m_4020->set_stages(14); // Using Q5,Q8,Q9
	m_4020->count_out_cb().set(FUNC(s4_state::clockcnt_w));

	CLOCK(config, "rclock", 3580000/4).signal_handler().set(m_4020, FUNC(ripple_counter_device::clock_w));
}

void s4a_state::s4a(machine_config &config)
{
	s4(config);

	m_pia22->writepb_handler().set(FUNC(s4a_state::s4a_sol1_w));

	// Add the soundcard
	M6808(config, m_audiocpu, 3580000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &s4a_state::audio_map);

	SPEAKER(config, "speaker").front_center();
	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5);

	PIA6821(config, m_pias, 0);
	m_pias->readpb_handler().set(FUNC(s4a_state::sound_r));
	m_pias->writepa_handler().set("dac", FUNC(dac_byte_interface::data_w));
	m_pias->irqa_handler().set("audioirq", FUNC(input_merger_device::in_w<1>));
	m_pias->irqb_handler().set("audioirq", FUNC(input_merger_device::in_w<2>));

	INPUT_MERGER_ANY_HIGH(config, "audioirq").output_handler().set_inputline(m_audiocpu, M6808_IRQ_LINE);
}


/*--------------------------------
/ Flash - Sys.4 (Game #486)
/-------------------------------*/
ROM_START(flash_l1)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(287f12d6) SHA1(ede0c5b0ea2586d8bdf71ecadbd9cc8552bd6934))
	ROM_LOAD("green1.716",   0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716",   0x1800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x0800, "audiocpu", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

ROM_START(flash_l2)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom2.716", 0x0000, 0x0800, CRC(b7c2e4c7) SHA1(00ea34900af679b1b7e2698f4aa2fc9703d54cf2))
	ROM_LOAD("yellow1.716",  0x1000, 0x0800, CRC(d251738c) SHA1(65ddbf5c36e429243331a4c5d2339df87a8a7f64))
	ROM_LOAD("yellow2.716",  0x1800, 0x0800, CRC(5049326d) SHA1(3b2f4ea054962bf4ba41d46663b7d3d9a77590ef))

	ROM_REGION(0x0800, "audiocpu", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

ROM_START(flash_t1)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(287f12d6) SHA1(ede0c5b0ea2586d8bdf71ecadbd9cc8552bd6934))
	ROM_LOAD("green1.716",   0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2a.716",  0x1800, 0x0800, CRC(16621eec) SHA1(14e1cf5f7227860a3219b2b79fa66dcf252dce98))

	ROM_REGION(0x0800, "audiocpu", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

/*--------------------------------
/ Tri Zone - Sys.4 (Game #487)
/-------------------------------*/
ROM_START(trizn_l1)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(757091c5) SHA1(00dac6c19b08d2528ea293619c4a39499a1a02c2))
	ROM_LOAD("green1.716",   0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716",   0x1800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x0800, "audiocpu", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

ROM_START(trizn_t1)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(757091c5) SHA1(00dac6c19b08d2528ea293619c4a39499a1a02c2))
	ROM_LOAD("green1.716",   0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2a.716",  0x1800, 0x0800, CRC(16621eec) SHA1(14e1cf5f7227860a3219b2b79fa66dcf252dce98))

	ROM_REGION(0x0800, "audiocpu", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

/*--------------------------------
/ Time Warp - Sys.4 (Game #489)
/-------------------------------*/
ROM_START(tmwrp_l3)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("twarp_l3.716",  0x0000, 0x0800, CRC(1234710a) SHA1(a33e9edb79b6ea4c5982d28a55289897f82b7b3c))
	ROM_LOAD("green1.716",   0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716",   0x1800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x0800, "audiocpu", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

ROM_START(tmwrp_l2)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(b168df09) SHA1(d4c97714636ce51be2e5f8cc5af89e10a2f82ac7))
	ROM_LOAD("green1.716",   0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716",   0x1800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x0800, "audiocpu", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

ROM_START(tmwrp_t2)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(b168df09) SHA1(d4c97714636ce51be2e5f8cc5af89e10a2f82ac7))
	ROM_LOAD("green1.716",   0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2a.716",  0x1800, 0x0800, CRC(16621eec) SHA1(14e1cf5f7227860a3219b2b79fa66dcf252dce98))

	ROM_REGION(0x0800, "audiocpu", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

/*--------------------------------
/ Stellar Wars - Sys.4 (Game #490)
/-------------------------------*/
ROM_START(stlwr_l2)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(874e7ef7) SHA1(271aeac2a0e61cb195811ae2e8d908cb1ab45874))
	ROM_LOAD("yellow1.716",  0x1000, 0x0800, CRC(d251738c) SHA1(65ddbf5c36e429243331a4c5d2339df87a8a7f64))
	ROM_LOAD("yellow2.716",  0x1800, 0x0800, CRC(5049326d) SHA1(3b2f4ea054962bf4ba41d46663b7d3d9a77590ef))

	ROM_REGION(0x0800, "audiocpu", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END



/* From here, are NOT pinball machines */

/*----------------------------
/ Pompeii (Shuffle)
/----------------------------*/
ROM_START(pomp_l1)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(0f069ac2) SHA1(d651d49cdb50cf444e420241a1f9ed48c878feee))
	ROM_LOAD("b_ic20.716",   0x1000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716",   0x1800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x0800, "audiocpu", 0)
	ROM_LOAD("soundx.716",   0x0000, 0x0800, CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
ROM_END

/*----------------------------
/ Aristocrat (Shuffle) same roms as Pompeii
/----------------------------*/
ROM_START(arist_l1)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(0f069ac2) SHA1(d651d49cdb50cf444e420241a1f9ed48c878feee))
	ROM_LOAD("b_ic20.716",   0x1000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716",   0x1800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x0800, "audiocpu", 0)
	ROM_LOAD("soundx.716",   0x0000, 0x0800, CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
ROM_END

/*----------------------------
/ Topaz (Shuffle)
/----------------------------*/
ROM_START(topaz_l1)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(cb287b10) SHA1(7fb6b6a26237cf85d5e02cf35271231267f90fc1))
	ROM_LOAD("b_ic20.716",   0x1000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716",   0x1800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x0800, "audiocpu", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

/*----------------------------
/ Taurus (Shuffle)
/----------------------------*/
ROM_START(taurs_l1)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(3246e285) SHA1(4f76784ecb5063a49c24795ae61db043a51e2c89))
	ROM_LOAD("b_ic20.716",   0x1000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716",   0x1800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x0800, "audiocpu", 0)
	ROM_LOAD("soundx.716",   0x0000, 0x0800, CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
ROM_END

/*----------------------------
/ King Tut
/----------------------------*/
ROM_START(kingt_l1)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(54d3280a) SHA1(ca74636e35d2c3e0b3133f89b1ff1233d5d72a5c))
	ROM_LOAD("b_ic20.716",   0x1000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716",   0x1800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x0800, "audiocpu", 0)
	ROM_LOAD("soundx.716",   0x0000, 0x0800, CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
ROM_END

/*----------------------------
/ Omni (Shuffle)
/----------------------------*/
ROM_START(omni_l1)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("omni-1a.u21",  0x0000, 0x0800, CRC(443bd170) SHA1(cc1ebd72d77ec2014cbd84534380e5ea1f12c022))
	ROM_LOAD("b_ic20.716",   0x1000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716",   0x1800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x0800, "audiocpu", 0)
	ROM_LOAD("sound.716",    0x0000, 0x0800, CRC(db085cbb) SHA1(9a57abbad183ba16b3dba16d16923c3bfc46a0c3))
ROM_END

/*----------------------------
/ Big Strike (Shuffle)
/----------------------------*/
ROM_START(bstrk_l1)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(323dbcde) SHA1(a75cbb5de97cb9afc1d36e9b6ff593bb482fcf8b))
	ROM_LOAD("b_ic20.716",   0x1000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716",   0x1800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
ROM_END

/*----------------------------
/ Triple Strike (Shuffle)
/----------------------------*/
ROM_START(tstrk_l1)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(b034c059) SHA1(76b3926b87b3c137fcaf33021a586827e3c030af))
	ROM_LOAD("ic20.716",     0x1000, 0x0800, CRC(f163fc88) SHA1(988b60626f3d4dc8f4a1dbd0c99282418bc53aae))
	ROM_LOAD("b_ic17.716",   0x1800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
ROM_END

} // Anonymous namespace

// Pinball
GAME( 1979, flash_l2, 0,        s4a, flash, s4a_state, empty_init, ROT0, "Williams", "Flash (Williams, L-2)",           MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1979, flash_l1, flash_l2, s4a, flash, s4a_state, empty_init, ROT0, "Williams", "Flash (Williams, L-1)",           MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1979, flash_t1, flash_l2, s4a, flash, s4a_state, empty_init, ROT0, "Williams", "Flash (Williams, T-1) Ted Estes", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1978, trizn_l1, 0,        s4a, trizn, s4a_state, empty_init, ROT0, "Williams", "Tri Zone (L-1)",                  MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1978, trizn_t1, trizn_l1, s4a, trizn, s4a_state, empty_init, ROT0, "Williams", "Tri Zone (T-1)",                  MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1979, tmwrp_l3, 0,        s4a, tmwrp, s4a_state, empty_init, ROT0, "Williams", "Time Warp (Williams, L-3)",       MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1979, tmwrp_l2, tmwrp_l3, s4a, tmwrp, s4a_state, empty_init, ROT0, "Williams", "Time Warp (Williams, L-2)",       MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1979, tmwrp_t2, tmwrp_l3, s4a, tmwrp, s4a_state, empty_init, ROT0, "Williams", "Time Warp (Williams, T-2)",       MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1979, stlwr_l2, 0,        s4a, stlwr, s4a_state, empty_init, ROT0, "Williams", "Stellar Wars (L-2)",              MACHINE_IS_SKELETON_MECHANICAL )

// Shuffle
GAME( 1978, pomp_l1,  0,        s4a, s4a,   s4a_state, empty_init, ROT0, "Williams", "Pompeii (Shuffle) (L-1)",         MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1978, arist_l1, 0,        s4a, s4a,   s4a_state, empty_init, ROT0, "Williams", "Aristocrat (Shuffle) (L-1)",      MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1978, topaz_l1, 0,        s4a, s4a,   s4a_state, empty_init, ROT0, "Williams", "Topaz (Shuffle) (L-1)",           MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1979, taurs_l1, 0,        s4a, s4a,   s4a_state, empty_init, ROT0, "Williams", "Taurus (Shuffle) (L-1)",          MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1979, kingt_l1, 0,        s4a, s4a,   s4a_state, empty_init, ROT0, "Williams", "King Tut (Shuffle) (L-1)",        MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1980, omni_l1,  0,        s4a, s4a,   s4a_state, empty_init, ROT0, "Williams", "Omni (Shuffle) (L-1)",            MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1983, bstrk_l1, 0,        s4,  s4,    s4_state,  empty_init, ROT0, "Williams", "Big Strike (Shuffle) (L-1)",      MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1983, tstrk_l1, 0,        s4,  s4,    s4_state,  empty_init, ROT0, "Williams", "Triple Strike (Shuffle) (L-1)",   MACHINE_IS_SKELETON_MECHANICAL )
