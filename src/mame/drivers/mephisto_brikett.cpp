// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

Hegener + Glaser Mephisto (I)/1X/II/ESB II/III
The base device is nicknamed the "Brikett"

Mephisto is the 1st chess computer by H+G, chess engine by Thomas Nitsche & Elmar Henne.
Initially, it didn't support an electronic chessboard. H+G released a dedicated ESB II
in 1981, and in 1982 they released ESB 3000 and ESB 6000 for use with ESB II/III programs.

Hardware notes:

1st model (Mephisto, Mephisto II 1981):
- 2 PCBs: DH 4001-C(computer), DH 4002-C(LCD/keypad)
- CDP1802CE @ 3.579MHz
- CDP1852CE (I/O port chip), external port
- 8*MWS5101L3 (256x4 RAM)
- 4-digit 7seg LCD
- piezo, TTL, 16-button keypad

2nd model (Mephisto II 1982, Mephisto III): (listed differences)
- PCB label: DH 4005-101 01
- CDP1802ACE @ 6.144MHz, later serials also seen with 1806 CPU
- 2 XTALs (3.579MHz and 6.144MHz), lower speed when running on battery
- 2*TC5514P (1KBx4 RAM), unpopulated on some Mephisto III

Mephisto program module:
- PCB label: DF 4003-B
- 6*CDP1833CE (1KB ROM)

Mephisto II program module:
- PCB label: HG 4005 02 301 00
- 3*TC5334P (4KB ROM)
- 2*TC5514P (1KBx4 RAM)
- 2*CDP1859CE (4bit latch)

Mephisto III program module:
- PCB label: HG 4005 02 401 00
- 2*HN4827128G (16KB EPROM), also seen with HN613256P G81 (32KB ROM)
- 2*CDM6116E1 (2KB RAM)
- DM74LS373N (latch)
- HCF4556BE (chip select?)

ESB 6000 chessboard:
- CD4017, 74373, 74374
- 64 reed switches (magnet sensors)
- 64 leds + power led

ESB 3000 hardware is probably same as ESB 6000.
There are no other known external port peripherals.

The Brikett was also used in the 1983 Mephisto Excalibur, but the hardware
is completely different, based on a 68000.

******************************************************************************/

#include "emu.h"
#include "cpu/cosmac/cosmac.h"
#include "machine/cdp1852.h"
#include "machine/sensorboard.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "video/pwm.h"
#include "speaker.h"

// internal artwork
#include "mephisto_1.lh" // clickable
#include "mephisto_3.lh" // clickable


namespace {

class brikett_state : public driver_device
{
public:
	brikett_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_extport(*this, "extport"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_speaker_off(*this, "speaker_off"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(reset_button) { if (newval) machine_reset(); }
	DECLARE_INPUT_CHANGED_MEMBER(switch_cpu_freq) { set_cpu_freq(); }

	// machine drivers
	void mephisto(machine_config &config);
	void mephisto2(machine_config &config);
	void mephisto3(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	// devices/pointers
	required_device<cdp1802_device> m_maincpu;
	required_device<cdp1852_device> m_extport;
	optional_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_device<timer_device> m_speaker_off;
	optional_ioport_array<4+2> m_inputs;

	// address maps
	void mephisto_map(address_map &map);
	void mephisto2_map(address_map &map);
	void mephisto3_map(address_map &map);
	void mephisto_io(address_map &map);

	void set_cpu_freq();

	// I/O handlers
	INTERRUPT_GEN_MEMBER(interrupt);
	DECLARE_READ_LINE_MEMBER(clear_r);
	DECLARE_WRITE_LINE_MEMBER(q_w);
	DECLARE_WRITE8_MEMBER(lcd_w);
	DECLARE_READ8_MEMBER(input_r);
	DECLARE_READ8_MEMBER(sound_r);

	DECLARE_WRITE8_MEMBER(esb_w);
	DECLARE_READ_LINE_MEMBER(esb_r);

	TIMER_DEVICE_CALLBACK_MEMBER(speaker_off) { m_dac->write(0); }

	bool m_reset;
	u8 m_lcd_mask;
	u8 m_digit_idx;
	u8 m_digit_data[4];

	u8 m_esb_led;
	u8 m_esb_row;
	u8 m_esb_select;
};

void brikett_state::machine_start()
{
	// zerofill
	m_reset = false;
	m_lcd_mask = 0;
	m_digit_idx = 0;
	memset(m_digit_data, 0, sizeof(m_digit_data));

	m_esb_led = 0;
	m_esb_row = 0;
	m_esb_select = 0;

	// register for savestates
	save_item(NAME(m_reset));
	save_item(NAME(m_lcd_mask));
	save_item(NAME(m_digit_idx));
	save_item(NAME(m_digit_data));

	save_item(NAME(m_esb_led));
	save_item(NAME(m_esb_row));
	save_item(NAME(m_esb_select));
}

void brikett_state::machine_reset()
{
	m_reset = true;
	m_digit_idx = 0;

	set_cpu_freq();
}

void brikett_state::set_cpu_freq()
{
	// 2nd hardware model has 2 XTALs, it will increase CPU voltage (and speed) when running on mains power,
	// the 3.579545MHz XTAL is still used for IRQ. Mephisto III could be fitted with a 12MHz XTAL instead of 6.144MHz
	// and a newer CDP1805CE CPU by Hobby Computer Centrale on request.
	// (It is unexpected that the 1805 accepts such a high overclock, but tests show that it is indeed twice faster)
	u8 inp = m_inputs[4]->read() >> 4 & m_inputs[4]->read();
	m_maincpu->set_unscaled_clock((inp & 2) ? 12_MHz_XTAL : ((inp & 1) ? 6.144_MHz_XTAL : 3.579545_MHz_XTAL));
}



/******************************************************************************
    I/O
******************************************************************************/

INTERRUPT_GEN_MEMBER(brikett_state::interrupt)
{
	m_maincpu->set_input_line(COSMAC_INPUT_LINE_INT, HOLD_LINE);
}

READ_LINE_MEMBER(brikett_state::clear_r)
{
	// CLEAR low + RESET high resets cpu
	int ret = (m_reset) ? 0 : 1;
	m_reset = false;
	return ret;
}

WRITE_LINE_MEMBER(brikett_state::q_w)
{
	// Q: LCD digit data mask
	// also assume LCD update on rising edge
	if (state && !m_lcd_mask)
	{
		for (int i = 0; i < 4; i++)
			m_display->write_row(i, m_digit_data[i]);
		m_display->update();
	}

	m_lcd_mask = state ? 0xff : 0;
}

WRITE8_MEMBER(brikett_state::lcd_w)
{
	// d0-d7: write/shift LCD digit (4*CD4015)
	// note: last digit "dp" is the colon in the middle
	m_digit_data[m_digit_idx] = data ^ m_lcd_mask;
	m_digit_idx = (m_digit_idx + 1) & 3;
}

READ8_MEMBER(brikett_state::sound_r)
{
	// port 1 read enables the speaker
	if (!machine().side_effects_disabled())
	{
		m_dac->write(1);
		m_speaker_off->adjust(attotime::from_usec(25));
	}

	return 0xff;
}

READ8_MEMBER(brikett_state::input_r)
{
	u8 data = 0;

	// a0-a3,d0-d3: read keypad
	for (int i = 0; i < 4; i++)
		if (BIT(~offset, i))
			data |= m_inputs[i]->read();

	return data;
}

WRITE8_MEMBER(brikett_state::esb_w)
{
	if (!m_inputs[5].read_safe(0))
	{
		// chessboard disabled
		m_display->matrix_partial(4, 8, 0, 0);
		return;
	}

	// CDP1852 SR clocks CD4017, goes to external port together with DO0-DO7
	// 4017 Q0: N/C
	// 4017 Q1 + d0-d7: 74374 to led data
	// 4017 Q2 + d0-d7: 74373 to row select
	// 4017 Q2-Q9: column select
	m_esb_select = (m_esb_select + 1) % 10;

	// d0-d7 ANDed together: 4017 reset
	if (data == 0xff)
		m_esb_select = 0;

	if (m_esb_select == 1)
		m_esb_led = data;

	if (m_esb_select == 2)
		m_esb_row = data;

	// update chessboard leds
	m_display->matrix_partial(4, 8, ~m_esb_row, m_esb_led);
}

READ_LINE_MEMBER(brikett_state::esb_r)
{
	// EF1: read chessboard sensor
	if (m_board && m_inputs[5].read_safe(0))
		return (m_board->read_file(m_esb_select - 2) & ~m_esb_row) ? 0 : 1;
	else
		return 0;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void brikett_state::mephisto_map(address_map &map)
{
	map(0x0000, 0x17ff).rom();
	map(0xf400, 0xf7ff).ram();
	map(0xfb00, 0xfb00).mirror(0x00ff).w(FUNC(brikett_state::lcd_w));
	map(0xfff0, 0xffff).r(FUNC(brikett_state::input_r));
}

void brikett_state::mephisto2_map(address_map &map)
{
	mephisto_map(map);
	map(0x0000, 0x2fff).rom();
	map(0xf000, 0xf3ff).ram();
}

void brikett_state::mephisto3_map(address_map &map)
{
	mephisto_map(map);
	map(0x0000, 0x7fff).rom().nopw(); // dummy write with sound_r
	map(0x8000, 0x8fff).ram();
}

void brikett_state::mephisto_io(address_map &map)
{
	map(0x01, 0x01).r(FUNC(brikett_state::sound_r));
	map(0x02, 0x02).w(m_extport, FUNC(cdp1852_device::write));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( mephisto )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_NAME("CL")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A / 1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("ENT")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B / 2 / Pawn")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("STA") // enter move
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("C / 3 / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("LEV")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("D / 4 / Bishop")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("LIST")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("E / 5 / Rook")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("SW / 9")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("F / 6 / Queen")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("WS / 0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G / 7 / King")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("REV")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("H / 8")

	PORT_START("IN.4") // 2nd model main PCB has 2 XTALs on PCB
	PORT_CONFNAME( 0x03, 0x00, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, brikett_state, switch_cpu_freq, nullptr) PORT_CONDITION("IN.4", 0x30, NOTEQUALS, 0x00)
	PORT_CONFSETTING(    0x00, "3.579MHz (Battery)" )
	PORT_CONFSETTING(    0x01, "6.144MHz (Mains)" )
	PORT_CONFNAME( 0x30, 0x00, "Base Hardware" ) PORT_CHANGED_MEMBER(DEVICE_SELF, brikett_state, switch_cpu_freq, nullptr)
	PORT_CONFSETTING(    0x00, "1st Model (1980)" )
	PORT_CONFSETTING(    0x30, "2nd Model (1982)" )

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("RES") PORT_CHANGED_MEMBER(DEVICE_SELF, brikett_state, reset_button, nullptr)
INPUT_PORTS_END

static INPUT_PORTS_START( mephisto3 )
	PORT_INCLUDE( mephisto )

	PORT_MODIFY("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("INFO")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("POS")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left / Black / 9")

	PORT_MODIFY("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right / White / 0")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("MEM")

	PORT_START("IN.5") // optional
	PORT_CONFNAME( 0x01, 0x01, "ESB 6000" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )

	PORT_MODIFY("IN.4")
	PORT_CONFNAME( 0x03, 0x01, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, brikett_state, switch_cpu_freq, nullptr)
	PORT_CONFSETTING(    0x00, "3.579MHz (Battery)" )
	PORT_CONFSETTING(    0x01, "6.144MHz (Mains)" )
	PORT_CONFSETTING(    0x02, "12MHz (Special)" )
	PORT_BIT(0x30, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void brikett_state::mephisto(machine_config &config)
{
	/* basic machine hardware */
	CDP1802(config, m_maincpu, 3.579545_MHz_XTAL); // see set_cpu_freq
	m_maincpu->set_addrmap(AS_PROGRAM, &brikett_state::mephisto_map);
	m_maincpu->set_addrmap(AS_IO, &brikett_state::mephisto_io);
	m_maincpu->clear_cb().set(FUNC(brikett_state::clear_r));
	m_maincpu->q_cb().set(FUNC(brikett_state::q_w)).invert();
	m_maincpu->tpb_cb().set(m_extport, FUNC(cdp1852_device::clock_w));
	m_maincpu->ef1_cb().set_constant(0); // external port
	m_maincpu->ef3_cb().set_constant(0); // external port, but unused

	const attotime irq_period = attotime::from_hz(3.579545_MHz_XTAL / 0x10000); // through SAJ300T
	m_maincpu->set_periodic_int(FUNC(brikett_state::interrupt), irq_period);

	CDP1852(config, m_extport);
	m_extport->mode_cb().set_constant(1);
	m_extport->do_cb().set_nop();

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(4, 8);
	m_display->set_segmask(0xf, 0x7f);
	config.set_default_layout(layout_mephisto_1);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	TIMER(config, m_speaker_off).configure_generic(FUNC(brikett_state::speaker_off));
}

void brikett_state::mephisto2(machine_config &config)
{
	mephisto(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &brikett_state::mephisto2_map);
}

void brikett_state::mephisto3(machine_config &config)
{
	mephisto2(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &brikett_state::mephisto3_map);
	m_maincpu->q_cb().set(FUNC(brikett_state::q_w));
	m_maincpu->ef1_cb().set(FUNC(brikett_state::esb_r));
	m_extport->do_cb().set(FUNC(brikett_state::esb_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(250));

	m_display->set_size(4+8, 8);
	config.set_default_layout(layout_mephisto_3);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( mephisto )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("89810", 0x0000, 0x0400, CRC(6816be9e) SHA1(f5f1d5084925fe239f5b2ecf4724751e0dc4fc51) ) // CDP1833CE
	ROM_LOAD("89811", 0x0400, 0x0400, CRC(15febc73) SHA1(10353a7f021993f2cf7d509a928425617e1786fb) ) // "
	ROM_LOAD("89812", 0x0800, 0x0400, CRC(5e45eb65) SHA1(9d46e5f405bd48705d1e29826917522595fc9768) ) // "
	ROM_LOAD("89813", 0x0c00, 0x0400, CRC(62da3d89) SHA1(a7f9ada7037e0bd61420358c147b2f57ee47ebcb) ) // "
	ROM_LOAD("89814", 0x1000, 0x0400, CRC(8e212d9c) SHA1(5df221ce8ca4fbb74f34f31738db4c2efee7fb01) ) // "
	ROM_LOAD("89815", 0x1400, 0x0400, CRC(072e0b01) SHA1(5b1074932b3f21ab01392250061c093de4af3624) ) // "
ROM_END


ROM_START( mephisto2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("5619_03_351", 0x0000, 0x1000, CRC(5b13d7bf) SHA1(e1b7dee278a03f75e8a1554715fca4c7fbbc1cb8) ) // TC5334P
	ROM_LOAD("5620_03_352", 0x1000, 0x1000, CRC(e93bf521) SHA1(42f9adce0d5e25b1b9d10217f8e3e0994d7b70d5) ) // "
	ROM_LOAD("5621_03_353", 0x2000, 0x1000, CRC(430dac62) SHA1(a0e23fcb4cfa27778a9398bd4994a7792e4541d0) ) // "
ROM_END


ROM_START( mephisto3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101", 0x0000, 0x4000, CRC(923de04f) SHA1(ca7cb3e29aeb3432a815c9d58bb0ed45e7302581) ) // HN4827128G-45
	ROM_LOAD("201", 0x4000, 0x4000, CRC(0c3cb8fa) SHA1(31449422142c19fc71474a057fc5d6af8a86be7d) ) // "
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME       PARENT CMP MACHINE    INPUT      STATE          INIT        COMPANY, FULLNAME, FLAGS
CONS( 1980, mephisto,  0,      0, mephisto,  mephisto,  brikett_state, empty_init, "Hegener + Glaser", "Mephisto", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1981, mephisto2, 0,      0, mephisto2, mephisto,  brikett_state, empty_init, "Hegener + Glaser", "Mephisto II", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1983, mephisto3, 0,      0, mephisto3, mephisto3, brikett_state, empty_init, "Hegener + Glaser", "Mephisto III", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
