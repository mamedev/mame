// license:BSD-3-Clause
// copyright-holders:Robbbert
/****************************************************************************************

PINBALL
Williams System 9

Differences to system 7:
- PIA at 0x2200 removed
- Dip switches removed
- Crystals changed from 3.58 to 4MHz
- Special solenoids cannot be computer-activated (so not included in io_outputs)

Games:
- Sorcerer (#532)
- Space Shuttle (#535)
- Comet (#540)

The first time run, the display will show the model number. Press F3 to clear this.


Here are the key codes to enable play:

Game              NUM  Start game                End ball
-----------------------------------------------------------------------------------------------
Sorcerer          532  ASD hit 1                 ASD
Space Shuttle     535  ASD hit 1                 ASD
Comet             540  1                         X

Status:
- All machines are playable.

ToDo:
- Mechanical sounds

*****************************************************************************************/

#include "emu.h"
#include "genpin.h"

#include "williamssound.h"

#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "speaker.h"

#include "s9.lh"

namespace {

class s9_state : public genpin_class
{
public:
	s9_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_s9sound(*this, "s9sound")
		, m_pia21(*this, "pia21")
		, m_pia24(*this, "pia24")
		, m_pia28(*this, "pia28")
		, m_pia30(*this, "pia30")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void init_rr() { m_game = 1; }
	void s9(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer);

private:
	void dig0_w(u8 data);
	void dig1_w(u8 data);
	void lamp0_w(u8 data);
	void lamp1_w(u8 data);
	void sol2_w(u8 data); // solenoids 8-15
	void sol3_w(u8 data) { for (u8 i = 0; i < 8; i++) m_io_outputs[i] = BIT(data, i); }; // solenoids 0-7
	u8 switch_r();
	void switch_w(u8 data);
	void pia21_cb2_w(int state) { } // enable solenoids
	void pia24_cb2_w(int state) { } // dummy to stop error log filling up
	void pia28_ca2_w(int state) { m_comma34 = state; } // comma3&4
	void pia28_cb2_w(int state) { m_comma12 = state; } // comma1&2
	void pia_irq(int state);

	void main_map(address_map &map) ATTR_COLD;

	u8 m_strobe = 0U;
	u8 m_row = 0U;
	bool m_game = false;
	bool m_comma12 = false;
	bool m_comma34 = false;
	bool m_data_ok = false;
	u8 m_lamp_data = 0U;
	emu_timer* m_irq_timer = nullptr;

	required_device<cpu_device> m_maincpu;
	required_device<williams_s9_sound_device> m_s9sound;
	required_device<pia6821_device> m_pia21;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia30;
	required_ioport_array<8> m_io_keyboard;
	output_finder<61> m_digits;
	output_finder<86> m_io_outputs; // 22 solenoids + 64 lamps
};

void s9_state::main_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x2100, 0x2103).rw(m_pia21, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // sound+solenoids
	map(0x2200, 0x2200).w(FUNC(s9_state::sol3_w)); // solenoids
	map(0x2400, 0x2403).rw(m_pia24, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // lamps
	map(0x2800, 0x2803).rw(m_pia28, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // display
	map(0x3000, 0x3003).rw(m_pia30, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // inputs
	map(0x4000, 0x7fff).rom().region("maincpu", 0 );
}

static INPUT_PORTS_START( s9 )
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
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP40")

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP41")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP42")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP43")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP44")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP46")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("INP47")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP48")

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP50")

	PORT_START("X7")

	PORT_START("DIAGS")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_0_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s9_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Advance") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Up/Down") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE
INPUT_PORTS_END

static INPUT_PORTS_START( sorcr )
	PORT_INCLUDE(s9)
	PORT_MODIFY("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP09")
	PORT_MODIFY("X3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP27")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP29")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP30")
	PORT_MODIFY("X5")
	PORT_BIT( 0xe1, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP45")
	PORT_MODIFY("X6")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( sshtl )
	PORT_INCLUDE(s9)
	PORT_MODIFY("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP10")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP11")
	PORT_MODIFY("X3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP27")
	PORT_MODIFY("X5")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP45")
	PORT_MODIFY("X6")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( s9_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void s9_state::sol2_w(u8 data)
{
	if (m_game)
	{
		m_comma12 = BIT(data, 7);
		m_comma34 = BIT(data, 6);
	}
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[8U+i] = BIT(data, i);
}

void s9_state::lamp0_w(u8 data)
{
	m_lamp_data = data ^ 0xff;
}

void s9_state::lamp1_w(u8 data)
{
	// find out which row is active
	for (u8 i = 0; i < 8; i++)
		if (BIT(data, i))
			for (u8 j = 0; j < 8; j++)
				m_io_outputs[22U+i*8U+j] = BIT(m_lamp_data, j);
}

void s9_state::dig0_w(u8 data)
{
	static const u8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7447
	data &= 0x7f;
	m_strobe = data & 15;
	m_data_ok = true;
	m_digits[60] = patterns[data>>4]; // diag digit
}

void s9_state::dig1_w(u8 data)
{
	static const u8 patterns[16] = { 0x3f,0x06,0xdb,0xcf,0xe6,0xed,0xfd,0x07,0xff,0xef,0,0,0,0,0,0 }; // MC14543
	if (m_data_ok)
	{
		m_digits[m_strobe+16] = patterns[data & 15] | (m_comma34 ? 0xc000 : 0);
		m_digits[m_strobe] = patterns[data >> 4] | (m_comma12 ? 0xc000 : 0);
	}
	m_data_ok = false;
}

u8 s9_state::switch_r()
{
	u8 data = 0;
	// there's hardware for 8 rows, but machine uses 7 max
	for (u8 i = 0; i < 7; i++)
		if (BIT(m_row, i))
			data |= m_io_keyboard[i]->read();

	return data;
}

void s9_state::switch_w(u8 data)
{
	m_row = data;
}

void s9_state::pia_irq(int state)
{
	if(state == CLEAR_LINE)
	{
		// restart IRQ timer
		m_irq_timer->adjust(attotime::from_ticks(980,1e6),1);
	}
	else
	{
		// disable IRQ timer while other IRQs are being handled
		// (counter is reset every 32 cycles while a PIA IRQ is handled)
		m_irq_timer->adjust(attotime::zero);
	}
}

TIMER_CALLBACK_MEMBER(s9_state::irq_timer)
{
	if(param == 1)
	{
		m_maincpu->set_input_line(M6808_IRQ_LINE, ASSERT_LINE);
		m_irq_timer->adjust(attotime::from_ticks(32,1e6),0);
		m_pia28->ca1_w(BIT(ioport("DIAGS")->read(), 2));  // Advance
		m_pia28->cb1_w(BIT(ioport("DIAGS")->read(), 3));  // Up/Down
	}
	else
	{
		m_maincpu->set_input_line(M6808_IRQ_LINE, CLEAR_LINE);
		m_irq_timer->adjust(attotime::from_ticks(980,1e6),1);
		m_pia28->ca1_w(1);
		m_pia28->cb1_w(1);
	}
}

void s9_state::machine_start()
{
	genpin_class::machine_start();
	m_io_outputs.resolve();
	m_digits.resolve();

	save_item(NAME(m_strobe));
	save_item(NAME(m_row));
	save_item(NAME(m_data_ok));
	save_item(NAME(m_lamp_data));
	save_item(NAME(m_comma12));
	save_item(NAME(m_comma34));

	m_irq_timer = timer_alloc(FUNC(s9_state::irq_timer), this);
	m_irq_timer->adjust(attotime::from_ticks(980,1e6),1);
}

void s9_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;
}

void s9_state::s9(machine_config &config)
{
	/* basic machine hardware */
	M6808(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &s9_state::main_map);

	/* Video */
	config.set_default_layout(layout_s9);

	/* Sound */
	genpin_audio(config);

	/* Devices */
	PIA6821(config, m_pia21);
	m_pia21->set_port_a_input_overrides_output_mask(0xff);
	m_pia21->ca1_w(1); // sound busy
	m_pia21->writepa_handler().set("s9sound", FUNC(williams_s9_sound_device::write));
	m_pia21->writepb_handler().set(FUNC(s9_state::sol2_w));
	m_pia21->ca2_handler().set("s9sound", FUNC(williams_s9_sound_device::strobe));
	m_pia21->cb2_handler().set(FUNC(s9_state::pia21_cb2_w));
	m_pia21->irqa_handler().set(FUNC(s9_state::pia_irq));
	m_pia21->irqb_handler().set(FUNC(s9_state::pia_irq));

	PIA6821(config, m_pia24);
	m_pia24->writepa_handler().set(FUNC(s9_state::lamp0_w));
	m_pia24->writepb_handler().set(FUNC(s9_state::lamp1_w));
	m_pia24->cb2_handler().set(FUNC(s9_state::pia24_cb2_w));
	m_pia24->irqa_handler().set(FUNC(s9_state::pia_irq));
	m_pia24->irqb_handler().set(FUNC(s9_state::pia_irq));

	PIA6821(config, m_pia28);
	m_pia28->writepa_handler().set(FUNC(s9_state::dig0_w));
	m_pia28->writepb_handler().set(FUNC(s9_state::dig1_w));
	m_pia28->ca2_handler().set(FUNC(s9_state::pia28_ca2_w));
	m_pia28->cb2_handler().set(FUNC(s9_state::pia28_cb2_w));
	m_pia28->irqa_handler().set(FUNC(s9_state::pia_irq));
	m_pia28->irqb_handler().set(FUNC(s9_state::pia_irq));

	PIA6821(config, m_pia30);
	m_pia30->readpa_handler().set(FUNC(s9_state::switch_r));
	m_pia30->set_port_a_input_overrides_output_mask(0xff);
	m_pia30->writepb_handler().set(FUNC(s9_state::switch_w));
	m_pia30->irqa_handler().set(FUNC(s9_state::pia_irq));
	m_pia30->irqb_handler().set(FUNC(s9_state::pia_irq));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Add the soundcard */
	SPEAKER(config, "mono").front_center();
	WILLIAMS_S9_SOUND(config, m_s9sound, 0).add_route(ALL_OUTPUTS, "mono", 1.0);
}


/*-----------------------------
/ Sorcerer : 03/85 (#532)
/------------------------------*/
ROM_START(sorcr_l1)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("cpu_u19.732", 0x1000, 0x1000, CRC(88b6837d) SHA1(d26b06342741443406a72ba48a70e82df62bb26e))
	ROM_LOAD("cpu_u20.764", 0x2000, 0x2000, CRC(c235b692) SHA1(d3b97fad2d501c894570601b387933c7644f64e6))

	ROM_REGION(0x8000, "s9sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("spch_u7.732", 0x0000, 0x1000, CRC(bba9ed18) SHA1(8e37ba8cb6bbc1e0afeef230088beda4513adddb))
	ROM_LOAD("spch_u5.732", 0x1000, 0x1000, CRC(d48c68ad) SHA1(b1391b87519ad47be3dcce7f8581f871e6a3669f))
	ROM_LOAD("spch_u6.732", 0x2000, 0x1000, CRC(a5c54d47) SHA1(4e1206412ecf52ae61c9df2055e0715749a6325d))
	ROM_LOAD("spch_u4.732", 0x3000, 0x1000, CRC(0c81902d) SHA1(6d8f703327e5c73a321fc4aa3a67ce68fff82d70))
	ROM_LOAD("cpu_u49.128", 0x4000, 0x4000, CRC(a0bae1e4) SHA1(dc5172aa1d59191d4119da20757cb2c2469f8fe3))
ROM_END

ROM_START(sorcr_l2)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("cpu_u19.l2",  0x1000, 0x1000, CRC(faf738db) SHA1(a3b3f4160dc837ddf5379e1edb0eafeefcc11e3d))
	ROM_LOAD("cpu_u20.l2",  0x2000, 0x2000, CRC(74fc8117) SHA1(c228c76ade670603f77bb324e6794ec6dd358285))

	ROM_REGION(0x8000, "s9sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("spch_u7.732", 0x0000, 0x1000, CRC(bba9ed18) SHA1(8e37ba8cb6bbc1e0afeef230088beda4513adddb))
	ROM_LOAD("spch_u5.732", 0x1000, 0x1000, CRC(d48c68ad) SHA1(b1391b87519ad47be3dcce7f8581f871e6a3669f))
	ROM_LOAD("spch_u6.732", 0x2000, 0x1000, CRC(a5c54d47) SHA1(4e1206412ecf52ae61c9df2055e0715749a6325d))
	ROM_LOAD("spch_u4.732", 0x3000, 0x1000, CRC(0c81902d) SHA1(6d8f703327e5c73a321fc4aa3a67ce68fff82d70))
	ROM_LOAD("cpu_u49.128", 0x4000, 0x4000, CRC(a0bae1e4) SHA1(dc5172aa1d59191d4119da20757cb2c2469f8fe3))
ROM_END

/*---------------------------------
/ Space Shuttle : 12/84 (#535)
/----------------------------------*/
ROM_START(sshtl_l7)
	// Spanish licensed version by Stargame is identical to this set
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("cpu_u20.128", 0x0000, 0x4000, CRC(848ad54c) SHA1(4e4ce5fb970da37706472f94a27fd912e1ecb1a0))

	ROM_REGION(0x8000, "s9sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("spch_u5.732", 0x1000, 0x1000, CRC(13edd4e5) SHA1(46c4052c31ddc20bb87445636f8fe3b6f7bff856))
	ROM_LOAD("spch_u6.732", 0x2000, 0x1000, CRC(cf48b2e7) SHA1(fe55419a5d40b3a4e8c02a92746b25a075b8efd3))
	ROM_LOAD("spch_u4.732", 0x3000, 0x1000, CRC(b0d03c5e) SHA1(46b952f71a7ecc03e22e427875f6e16a9d124067))
	ROM_LOAD("cpu_u49.128", 0x4000, 0x4000, CRC(8050ae27) SHA1(e3f5e9398f61b075620ecd075617a8dac3c07d0e))
ROM_END

ROM_START(sshtl_l3)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("cpu_u20.l3", 0x0000, 0x4000, CRC(dc5f08e0) SHA1(67869c1db4e1f49f38588978d4ed32fe7d62e2d6))

	ROM_REGION(0x8000, "s9sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("spch_u5.732", 0x1000, 0x1000, CRC(13edd4e5) SHA1(46c4052c31ddc20bb87445636f8fe3b6f7bff856))
	ROM_LOAD("spch_u6.732", 0x2000, 0x1000, CRC(cf48b2e7) SHA1(fe55419a5d40b3a4e8c02a92746b25a075b8efd3))
	ROM_LOAD("spch_u4.732", 0x3000, 0x1000, CRC(b0d03c5e) SHA1(46b952f71a7ecc03e22e427875f6e16a9d124067))
	ROM_LOAD("cpu_u49.128", 0x4000, 0x4000, CRC(8050ae27) SHA1(e3f5e9398f61b075620ecd075617a8dac3c07d0e))
ROM_END

/*-------------------------
/ Comet : 06/85 (#540)
/--------------------------*/
ROM_START(comet_l4)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("cpu_u20.128", 0x0000, 0x4000, CRC(36193600) SHA1(efdc44ef26c2def8f860a0296e27b2c3dac55ec8))

	ROM_REGION(0x8000, "s9sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("spch_u7.732", 0x0000, 0x1000, CRC(36545b22) SHA1(f4a026f3fa58dce81b439d76120a6769f4632955))
	ROM_LOAD("spch_u5.732", 0x1000, 0x1000, CRC(89f7ede5) SHA1(bbfbd991c9e005c2fa36d8458803b121f4933618))
	ROM_LOAD("spch_u6.732", 0x2000, 0x1000, CRC(6ba2aba6) SHA1(783b4e9b38db8677d91f86cb4805f0fa1ae8f856))
	ROM_LOAD("spch_u4.732", 0x3000, 0x1000, CRC(d0215c49) SHA1(4f0925a826199b6e8baa5e7fbff5cde9e31d505b))
	ROM_LOAD("cpu_u49.128", 0x4000, 0x4000, CRC(f1db0cbe) SHA1(59b7f36fb2003b90b288abeff56df62ce50f10c6))
ROM_END

ROM_START(comet_l5)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("cpu_u20.l5",  0x0000, 0x4000, CRC(d153d9ab) SHA1(0b97591b8ba35207b1427900486d69078ae122bc))

	ROM_REGION(0x8000, "s9sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("spch_u7.732", 0x0000, 0x1000, CRC(36545b22) SHA1(f4a026f3fa58dce81b439d76120a6769f4632955))
	ROM_LOAD("spch_u5.732", 0x1000, 0x1000, CRC(89f7ede5) SHA1(bbfbd991c9e005c2fa36d8458803b121f4933618))
	ROM_LOAD("spch_u6.732", 0x2000, 0x1000, CRC(6ba2aba6) SHA1(783b4e9b38db8677d91f86cb4805f0fa1ae8f856))
	ROM_LOAD("spch_u4.732", 0x3000, 0x1000, CRC(d0215c49) SHA1(4f0925a826199b6e8baa5e7fbff5cde9e31d505b))
	ROM_LOAD("cpu_u49.128", 0x4000, 0x4000, CRC(f1db0cbe) SHA1(59b7f36fb2003b90b288abeff56df62ce50f10c6))
ROM_END

} // Anonymous namespace

GAME( 1985, sorcr_l1, sorcr_l2, s9, sorcr, s9_state, empty_init, ROT0, "Williams", "Sorcerer (L-1)",              MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1985, sorcr_l2, 0,        s9, sorcr, s9_state, empty_init, ROT0, "Williams", "Sorcerer (L-2)",              MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1984, sshtl_l7, 0,        s9, sshtl, s9_state, empty_init, ROT0, "Williams", "Space Shuttle (L-7)",         MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1984, sshtl_l3, sshtl_l7, s9, sshtl, s9_state, empty_init, ROT0, "Williams", "Space Shuttle (L-3)",         MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1985, comet_l4, comet_l5, s9, s9,    s9_state, empty_init, ROT0, "Williams", "Comet (L-4)",                 MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1985, comet_l5, 0,        s9, s9,    s9_state, empty_init, ROT0, "Williams", "Comet (L-5)",                 MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
