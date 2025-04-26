// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Hegener + Glaser Mephisto (I)/1X/II/ESB II/III/Junior
The base device is nicknamed the "Brikett"

Mephisto is the 1st chess computer by H+G, chess engine by Thomas Nitsche & Elmar Henne.
Initially, it didn't support an electronic chessboard. H+G released a dedicated ESB II
in 1981, and in 1982 they released ESB 3000 and ESB 6000 for use with ESB II/III programs.

Hardware notes:

1st model (Mephisto, early Mephisto II):
- 2 PCBs: DH 4001-C(computer), DH 4002-C(LCD/keypad)
- CDP1802CE @ 3.579MHz
- CDP1852CE (I/O port chip), external port
- 8*MWS5101L3 (256x4 RAM)
- 4-digit 7seg LCD
- piezo, TTL, 16-button keypad
- module slot

2nd model (later Mephisto II): (listed differences)
- PCB label: DH 4005-101 00
- CDP1802ACE @ 3.579MHz or 4.194MHz (chess clock runs faster on 4.194MHz)
- 2*MWS5114E or 2*TC5514P (1KBx4 RAM)

3rd model (later Mephisto II, Mephisto III): (listed differences)
- PCB label: DH 4005-101 01
- CDP1802ACE @ 6.144MHz, later serials also seen with 1806 CPU
- 2 XTALs (3.579MHz and 6.144MHz), lower speed when running on battery
- 2*TC5514P may be unpopulated on some Mephisto III

Mephisto Junior: (listed differences)
- 2 PCBs: RAWE 003010 B(computer), RAWE 003010 A(LCD/keypad)
  (RAWE = Rawe Datentechnik GmbH)
- CDP1802ACE @ 4.194MHz
- 1KB RAM (2*MWS5114E1), 8KB ROM (SMM2365C)
- no module slot, I/O chip, or external port

Mephisto program module:
- PCB label: DF 4003-B or DH 4005 20100
- 6*CDP1833CE (1KB ROM)

Mephisto II/ESB II program module:
- PCB label: HG 4005 02 301 00
- 3*TC5334P (4KB ROM), 4*M2532A on ESB II
- 2*TC5514P (1KBx4 RAM)
- 2*CDP1859CE (4bit latch)

Mephisto 1X program module:
- PCB label: DH 4005 02 301 00
- rest is same as Mephisto II, but ROM chips are CM3200-2

Mephisto III program module:
- PCB label: HG 4005 02 401 00
- 2*HN4827128G (16KB EPROM), also seen with HN613256P G81 (32KB ROM)
- 2*CDM6116E1 (2KB RAM)
- DM74LS373N (latch)
- HCF4556BE (chip select?)

ESB II board interface (via module slot):
- PCB label: DH 5000 00 111 01
- CD4001, 3*74373, MDP1603-331G (resistor array)

ESB 6000 board interface (via external port):
- PCB label: DH 5000 00 111 12
- TC4081, TC4082, TC4017, 74373, 74374

ESB II/6000 chessboard:
- 128 reed switches (magnet sensors, 2 per square)
- 64 leds + power led

ESB 3000 hardware is probably same as ESB 6000.
There are no other known external port peripherals.

The Brikett (sans main PCB) was also used in the 1983 Mephisto Excalibur, but
the hardware is completely different, based on a 68000 (see excalibur.cpp).

BTANB:
- bad bug in mephistoj opening library: e4 e6 / d4 d5 / Nd2 c5 / exd5 Qd1xd5,
  in other words: computer makes an illegal move with the WHITE queen

*******************************************************************************/

#include "emu.h"

#include "mmdisplay1.h"

#include "cpu/cosmac/cosmac.h"
#include "machine/cdp1852.h"
#include "machine/clock.h"
#include "machine/sensorboard.h"
#include "sound/spkrdev.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "mephisto_1.lh"
#include "mephisto_esb2.lh"
#include "mephisto_3.lh"
#include "mephisto_junior.lh"


namespace {

class brikett_state : public driver_device
{
public:
	brikett_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_irq_clock(*this, "irq_clock"),
		m_extport(*this, "extport"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_led_pwm(*this, "led_pwm"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(reset_button) { if (newval) machine_reset(); }
	DECLARE_INPUT_CHANGED_MEMBER(change_cpu_freq);

	// machine configs
	void mephisto(machine_config &config);
	void mephistoj(machine_config &config);
	void mephisto2(machine_config &config);
	void mephistoe2(machine_config &config);
	void mephistoe2a(machine_config &config);
	void mephisto3(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD { m_reset = true; }

private:
	// devices/pointers
	required_device<cdp1802_device> m_maincpu;
	required_device<clock_device> m_irq_clock;
	optional_device<cdp1852_device> m_extport;
	optional_device<sensorboard_device> m_board;
	required_device<mephisto_display1_device> m_display;
	optional_device<pwm_display_device> m_led_pwm;
	required_device<speaker_sound_device> m_dac;
	optional_ioport_array<4+2> m_inputs;

	emu_timer *m_speaker_off;
	bool m_reset = false;
	u8 m_esb_led = 0;
	u8 m_esb_row = 0;
	u8 m_esb_select = 0;

	// address maps
	void mephisto_map(address_map &map) ATTR_COLD;
	void mephistoj_map(address_map &map) ATTR_COLD;
	void mephisto2_map(address_map &map) ATTR_COLD;
	void mephistoe2_map(address_map &map) ATTR_COLD;
	void mephistoe2a_map(address_map &map) ATTR_COLD;
	void mephisto3_map(address_map &map) ATTR_COLD;
	void mephisto_io(address_map &map) ATTR_COLD;
	void mephistoj_io(address_map &map) ATTR_COLD;

	// I/O handlers
	int clear_r();
	u8 input_r(offs_t offset);
	u8 sound_r();

	void esb2_w(offs_t offset, u8 data);
	u8 esb2_r(offs_t offset);
	void esb6_w(u8 data);
	int esb6_r();

	TIMER_CALLBACK_MEMBER(speaker_off) { m_dac->level_w(0); }
};



/*******************************************************************************
    Initialization
*******************************************************************************/

void brikett_state::machine_start()
{
	m_speaker_off = timer_alloc(FUNC(brikett_state::speaker_off), this);

	// register for savestates
	save_item(NAME(m_reset));
	save_item(NAME(m_esb_led));
	save_item(NAME(m_esb_row));
	save_item(NAME(m_esb_select));
}

INPUT_CHANGED_MEMBER(brikett_state::change_cpu_freq)
{
	newval = m_inputs[4]->read();

	if (newval & 8)
	{
		/*
		    3rd hardware model has 2 XTALs, it will increase CPU voltage (and speed)
		    when running on mains power, the 3.579545MHz XTAL is still used for IRQ.

		    Mephisto III could be fitted with a 12MHz XTAL instead of 6.144MHz and
		    a newer CDP1805CE CPU by Hobby Computer Centrale on request. (It is
		    unexpected that the 1805 accepts such a high overclock, but tests show
		    that it is indeed twice faster)
		*/
		static const XTAL freq[3] = { 3.579545_MHz_XTAL, 6.144_MHz_XTAL, 12_MHz_XTAL };
		m_maincpu->set_unscaled_clock(freq[(newval & 3) % 3]);
	}
	else
	{
		m_maincpu->set_unscaled_clock((newval & 4) ? 4.194304_MHz_XTAL : 3.579545_MHz_XTAL);
	}

	// also change interrupt frequency
	m_irq_clock->set_period(attotime::from_ticks(0x10000, (newval & 4) ? 4.194304_MHz_XTAL : 3.579545_MHz_XTAL));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// base hardware

int brikett_state::clear_r()
{
	// CLEAR low + WAIT high resets cpu
	int ret = (m_reset) ? 0 : 1;
	m_reset = false;
	return ret;
}

u8 brikett_state::sound_r()
{
	// port 1 read enables the speaker
	if (!machine().side_effects_disabled())
	{
		m_dac->level_w(1);
		m_speaker_off->adjust(attotime::from_ticks(8, m_maincpu->clock()));
	}

	return 0xff;
}

u8 brikett_state::input_r(offs_t offset)
{
	u8 data = 0;

	// a0-a3,d0-d3: read keypad
	for (int i = 0; i < 4; i++)
		if (BIT(~offset, i))
			data |= m_inputs[i]->read();

	return data;
}


// ESB II board

void brikett_state::esb2_w(offs_t offset, u8 data)
{
	// a0-a7,d0-d7: chessboard leds
	m_led_pwm->matrix(~offset, m_inputs[5].read_safe(0) ? data : 0);
}

u8 brikett_state::esb2_r(offs_t offset)
{
	u8 data = 0;

	// a0-a7,d0-d7: read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(~offset, i))
			data |= m_board->read_rank(i);

	return m_inputs[5].read_safe(0) ? ~data : 0;
}


// ESB 6000 board

void brikett_state::esb6_w(u8 data)
{
	// CDP1852 SR + DO0-DO7 goes to external port, to chessboard
	if (!m_inputs[5].read_safe(0))
	{
		// chessboard disabled
		m_led_pwm->clear();
		return;
	}

	// SR clocks TC4017
	// 4017 Q0: N/C
	// 4017 Q1 + d0-d7: 74374 to led data
	// 4017 Q2 + d0-d7: 74373 to row select
	// 4017 Q2-Q9: column select
	m_esb_select = (m_esb_select + 1) % 10;

	// DO0-DO7 ANDed together: 4017 reset
	if (data == 0xff)
		m_esb_select = 0;

	if (m_esb_select == 1)
		m_esb_led = data;

	if (m_esb_select == 2)
		m_esb_row = data;

	// update chessboard leds
	m_led_pwm->matrix(~m_esb_row, m_esb_led);
}

int brikett_state::esb6_r()
{
	// EF1: read chessboard square
	if (m_inputs[5].read_safe(0))
		return (m_board->read_file(m_esb_select - 2) & ~m_esb_row) ? 0 : 1;
	else
		return 0;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void brikett_state::mephisto_map(address_map &map)
{
	map(0x0000, 0x17ff).rom();
	map(0xf400, 0xf7ff).ram();
	map(0xfb00, 0xfb00).mirror(0x00ff).w(m_display, FUNC(mephisto_display1_device::data_w));
	map(0xfff0, 0xffff).r(FUNC(brikett_state::input_r));
}

void brikett_state::mephistoj_map(address_map &map)
{
	mephisto_map(map);
	map(0x0000, 0x1fff).rom();
}

void brikett_state::mephisto2_map(address_map &map)
{
	mephisto_map(map);
	map(0x0000, 0x2fff).rom();
	map(0xf000, 0xf3ff).ram();
}

void brikett_state::mephistoe2_map(address_map &map)
{
	mephisto2_map(map);
	map(0x3000, 0x3fff).rom();
}

void brikett_state::mephistoe2a_map(address_map &map)
{
	mephistoe2_map(map);
	map(0xbf00, 0xbfff).rw(FUNC(brikett_state::esb2_r), FUNC(brikett_state::esb2_w));
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

void brikett_state::mephistoj_io(address_map &map)
{
	map(0x01, 0x01).r(FUNC(brikett_state::sound_r));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( mephisto )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_NAME("CL")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A / 1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("ENT")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B / 2 / Pawn")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("STA") // enter move
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

	PORT_START("IN.4") // 3rd model main PCB has 2 XTALs on PCB
	PORT_CONFNAME( 0x03, 0x01, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(brikett_state::change_cpu_freq), 0) PORT_CONDITION("IN.4", 0x0c, EQUALS, 0x08)
	PORT_CONFSETTING(    0x00, "3.579MHz (Battery)" )
	PORT_CONFSETTING(    0x01, "6.144MHz (Mains)" )
	PORT_CONFNAME( 0x0c, 0x00, "Base Hardware" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(brikett_state::change_cpu_freq), 0)
	PORT_CONFSETTING(    0x00, "1st Model (3.579MHz)" ) // or 2nd model with this XTAL
	PORT_CONFSETTING(    0x04, "2nd Model (4.194MHz)" )
	PORT_CONFSETTING(    0x08, "3rd Model (2 XTALs)" )

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("RES") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(brikett_state::reset_button), 0)
INPUT_PORTS_END

static INPUT_PORTS_START( mephistoj )
	PORT_INCLUDE( mephisto )

	PORT_MODIFY("IN.4") // 1 XTAL
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( mephistoe2 )
	PORT_INCLUDE( mephisto )

	PORT_START("IN.5") // optional
	PORT_CONFNAME( 0x01, 0x01, "ESB 6000 Board" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mephistoe2a )
	PORT_INCLUDE( mephistoe2 )

	PORT_MODIFY("IN.5") // optional
	PORT_CONFNAME( 0x01, 0x01, "ESB II Board" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mephisto3 )
	PORT_INCLUDE( mephistoe2 )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("ENT")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("INFO")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("POS")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left / Black / 9")

	PORT_MODIFY("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right / White / 0")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("MEM")

	PORT_MODIFY("IN.4")
	PORT_CONFNAME( 0x03, 0x01, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(brikett_state::change_cpu_freq), 0)
	PORT_CONFSETTING(    0x00, "3.579MHz (Battery)" )
	PORT_CONFSETTING(    0x01, "6.144MHz (Mains)" )
	PORT_CONFSETTING(    0x02, "12MHz (Special)" )
	PORT_BIT(0x0c, 0x08, IPT_CUSTOM) // 3rd model
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void brikett_state::mephistoj(machine_config &config)
{
	// basic machine hardware
	CDP1802(config, m_maincpu, 4.194304_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &brikett_state::mephistoj_map);
	m_maincpu->set_addrmap(AS_IO, &brikett_state::mephistoj_io);
	m_maincpu->clear_cb().set(FUNC(brikett_state::clear_r));
	m_maincpu->q_cb().set(m_display, FUNC(mephisto_display1_device::strobe_w)).invert();

	const attotime irq_period = attotime::from_ticks(0x10000, 4.194304_MHz_XTAL); // through SAJ300T
	CLOCK(config, m_irq_clock).set_period(irq_period);
	m_irq_clock->signal_handler().set_inputline(m_maincpu, COSMAC_INPUT_LINE_INT, HOLD_LINE);

	// video hardware
	MEPHISTO_DISPLAY_MODULE1(config, m_display); // internal
	config.set_default_layout(layout_mephisto_junior);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	SPEAKER_SOUND(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.5);
}

void brikett_state::mephisto(machine_config &config)
{
	mephistoj(config);

	// basic machine hardware
	m_maincpu->set_clock(3.579545_MHz_XTAL); // see change_cpu_freq
	m_maincpu->set_addrmap(AS_PROGRAM, &brikett_state::mephisto_map);
	m_maincpu->set_addrmap(AS_IO, &brikett_state::mephisto_io);
	m_maincpu->tpb_cb().set(m_extport, FUNC(cdp1852_device::clock_w));
	m_maincpu->ef1_cb().set_constant(0); // external port
	m_maincpu->ef3_cb().set_constant(0); // external port, but unused

	m_irq_clock->set_period(attotime::from_ticks(0x10000, 3.579545_MHz_XTAL));

	CDP1852(config, m_extport);
	m_extport->mode_cb().set_constant(1);
	m_extport->do_cb().set_nop();

	config.set_default_layout(layout_mephisto_1);
}

void brikett_state::mephisto2(machine_config &config)
{
	mephisto(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &brikett_state::mephisto2_map);
}

void brikett_state::mephistoe2a(machine_config &config)
{
	mephisto2(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &brikett_state::mephistoe2a_map);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(250));

	PWM_DISPLAY(config, m_led_pwm).set_size(8, 8);
	config.set_default_layout(layout_mephisto_esb2);
}

void brikett_state::mephistoe2(machine_config &config)
{
	mephistoe2a(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &brikett_state::mephistoe2_map);
	m_maincpu->ef1_cb().set(FUNC(brikett_state::esb6_r));
	m_extport->do_cb().set(FUNC(brikett_state::esb6_w));
}

void brikett_state::mephisto3(machine_config &config)
{
	mephistoe2(config);

	// basic machine hardware
	m_maincpu->set_clock(6.144_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &brikett_state::mephisto3_map);
	m_maincpu->q_cb().set(m_display, FUNC(mephisto_display1_device::strobe_w));

	config.set_default_layout(layout_mephisto_3);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( mephisto ) // module s/n 00226xx (898xx Mask ROMs), 01011xx (911xx Mask ROMs)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("89810", 0x0000, 0x0400, CRC(6816be9e) SHA1(f5f1d5084925fe239f5b2ecf4724751e0dc4fc51) ) // CDP1833CE, also seen with label 91143
	ROM_LOAD("89811", 0x0400, 0x0400, CRC(15febc73) SHA1(10353a7f021993f2cf7d509a928425617e1786fb) ) // " or 91144
	ROM_LOAD("89812", 0x0800, 0x0400, CRC(5e45eb65) SHA1(9d46e5f405bd48705d1e29826917522595fc9768) ) // " or 91145
	ROM_LOAD("89813", 0x0c00, 0x0400, CRC(62da3d89) SHA1(a7f9ada7037e0bd61420358c147b2f57ee47ebcb) ) // " or 91146
	ROM_LOAD("89814", 0x1000, 0x0400, CRC(8e212d9c) SHA1(5df221ce8ca4fbb74f34f31738db4c2efee7fb01) ) // " or 91163
	ROM_LOAD("89815", 0x1400, 0x0400, CRC(072e0b01) SHA1(5b1074932b3f21ab01392250061c093de4af3624) ) // " or 91147
	// 911xx Mask ROMs have the same contents as 898xx Mask ROMs, some modules have both 898xx and 911xx
ROM_END

ROM_START( mephisto1x )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("3-2911_adi.1", 0x0000, 0x1000, CRC(0d62fa67) SHA1(b4bd934fec595f37f99b74eb341d220c511c07a5) ) // CM3200-2
	ROM_LOAD("3-2501_adj.2", 0x1000, 0x1000, CRC(4e1b67ae) SHA1(4fded3ed1a1e168dedc07eea4086fa31805252d9) ) // "
	ROM_LOAD("3-2841_adk.3", 0x2000, 0x1000, CRC(5dd05a5d) SHA1(372ed83a936fb0720b68590ca6ff4a02c80f4bab) ) // "
ROM_END

ROM_START( mephistoj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("smm2365c.7", 0x0000, 0x2000, CRC(f2f46583) SHA1(960b6f781e4c3d98db85a1bb4f90df4a133f06ba) ) // SMM2365C, no label
ROM_END


ROM_START( mephisto2 ) // module s/n 01142xx (HN462532G EPROMs), 00476xx (TC5334P Mask ROMs)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("4005_02_351_02.1", 0x0000, 0x1000, CRC(5b13d7bf) SHA1(e1b7dee278a03f75e8a1554715fca4c7fbbc1cb8) ) // HN462532G
	ROM_LOAD("4005_02_351_02.2", 0x1000, 0x1000, CRC(e93bf521) SHA1(42f9adce0d5e25b1b9d10217f8e3e0994d7b70d5) ) // "
	ROM_LOAD("4005_02_351_02.3", 0x2000, 0x1000, CRC(430dac62) SHA1(a0e23fcb4cfa27778a9398bd4994a7792e4541d0) ) // "
	// TC5334P Mask ROM contents is the same (labels 5619 03 351, 5620 03 351, 5621 03 351)
ROM_END

ROM_START( mephisto2a ) // module s/n 01085xx
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("4005_02_351_01.1", 0x0000, 0x1000, CRC(5b13d7bf) SHA1(e1b7dee278a03f75e8a1554715fca4c7fbbc1cb8) ) // HN462532G
	ROM_LOAD("4005_02_352_01.2", 0x1000, 0x1000, CRC(da88b62f) SHA1(f5e71521ba8ab0b481e4725ffa706b1c157424b5) ) // "
	ROM_LOAD("4005_02_353_01.3", 0x2000, 0x1000, CRC(1f933d33) SHA1(5d5bfd40158354830c434f4c8b4ff1cac8ab4f5c) ) // "
ROM_END

ROM_START( mephisto2b ) // module s/n 01070xx
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("4005_02_351_00.1", 0x0000, 0x1000, CRC(5b13d7bf) SHA1(e1b7dee278a03f75e8a1554715fca4c7fbbc1cb8) ) // HN462532G
	ROM_LOAD("4005_02_352_00.2", 0x1000, 0x1000, CRC(da88b62f) SHA1(f5e71521ba8ab0b481e4725ffa706b1c157424b5) ) // "
	ROM_LOAD("4005_02_353_00.3", 0x2000, 0x1000, CRC(23f9eee4) SHA1(c86d63d3c720ba4ae2a12d3b68356cffacae9592) ) // "
ROM_END


ROM_START( mephistoe2 ) // module s/n (0)0111xx
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("251-11.1", 0x0000, 0x1000, CRC(3c8e2631) SHA1(5960e47f0659b1e5f164107069738e730e3ff255) ) // TMS2532JL-35
	ROM_LOAD("252-10.2", 0x1000, 0x1000, CRC(832b053e) SHA1(b0dfe857c38f13a4b04ac67a8a46f37c962a8629) ) // "
	ROM_LOAD("253-09.3", 0x2000, 0x1000, CRC(00788b63) SHA1(cf94dc19ef85b359989410e7824280c59433fca9) ) // "
	ROM_LOAD("254-09.4", 0x3000, 0x1000, CRC(d6be47a6) SHA1(3d577036111c026292b6c445efcb126cf7a6a472) ) // "
ROM_END

ROM_START( mephistoe2a ) // module s/n 00065xx
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("5000_00_251_01.1", 0x0000, 0x1000, CRC(cebf66be) SHA1(7793ba80159a8ae857a1b35d270bb6972fef0563) ) // HN462532G
	ROM_LOAD("5000_00_252_01.2", 0x1000, 0x1000, CRC(66f38992) SHA1(2f9007985ce350a8ee82797a66f37bb9a4de36a4) ) // "
	ROM_LOAD("5000_00_253_01.3", 0x2000, 0x1000, CRC(1b685892) SHA1(af197e62e8a47a6244f7c2261929847f483775e5) ) // "
	ROM_LOAD("5000_00_254_01.4", 0x3000, 0x1000, CRC(e455845d) SHA1(871e2b2b7e75b6ee09d3e027e1b4e5a24ce4c550) ) // "
ROM_END


ROM_START( mephisto3 ) // module s/n 05072xx
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("g81", 0x0000, 0x8000, CRC(7b49475d) SHA1(30193153f0c259294b47e95d3e33834e40a94821) ) // HN613256P
ROM_END

ROM_START( mephisto3a ) // module s/n 00744xx
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("109", 0x0000, 0x4000, CRC(02f9e37d) SHA1(1911d45c0c8db030d129c4d2b25572678835112a) ) // D27128-4
	ROM_LOAD("209", 0x4000, 0x4000, CRC(0f217caf) SHA1(5aa77157af51a73e0654c344636ea2887bc45d42) ) // "
ROM_END

ROM_START( mephisto3b ) // module s/n 00737xx
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("107", 0x0000, 0x4000, CRC(52c072b8) SHA1(938dfaa18d751f06f42be16dedb7d32d010023b2) ) // HN4827128G-25
	ROM_LOAD("207", 0x4000, 0x4000, CRC(9b45c350) SHA1(96a11f740c657a915a9ce3fa417a59f4e064a10b) ) // "
ROM_END

ROM_START( mephisto3c ) // module s/n 00711xx
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101", 0x0000, 0x4000, CRC(923de04f) SHA1(ca7cb3e29aeb3432a815c9d58bb0ed45e7302581) ) // HN4827128G-45 or D27128-4
	ROM_LOAD("201", 0x4000, 0x4000, CRC(0c3cb8fa) SHA1(31449422142c19fc71474a057fc5d6af8a86be7d) ) // "
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME         PARENT      COMPAT  MACHINE      INPUT        STATE          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1980, mephisto,    0,          0,      mephisto,    mephisto,    brikett_state, empty_init, "Hegener + Glaser", "Mephisto", MACHINE_SUPPORTS_SAVE )

SYST( 1981, mephisto1x,  0,          0,      mephisto2,   mephisto,    brikett_state, empty_init, "Hegener + Glaser", "Mephisto 1X", MACHINE_SUPPORTS_SAVE ) // France
SYST( 1982, mephistoj,   0,          0,      mephistoj,   mephistoj,   brikett_state, empty_init, "Hegener + Glaser", "Mephisto Junior (1982 version)", MACHINE_SUPPORTS_SAVE ) // there's also a "Mephisto Junior" from 1990

SYST( 1981, mephisto2,   0,          0,      mephisto2,   mephisto,    brikett_state, empty_init, "Hegener + Glaser", "Mephisto II (set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1981, mephisto2a,  mephisto2,  0,      mephisto2,   mephisto,    brikett_state, empty_init, "Hegener + Glaser", "Mephisto II (set 2)", MACHINE_SUPPORTS_SAVE )
SYST( 1981, mephisto2b,  mephisto2,  0,      mephisto2,   mephisto,    brikett_state, empty_init, "Hegener + Glaser", "Mephisto II (set 3)", MACHINE_SUPPORTS_SAVE )

SYST( 1981, mephistoe2,  0,          0,      mephistoe2,  mephistoe2,  brikett_state, empty_init, "Hegener + Glaser", "Mephisto ESB II (ESB 6000 board)", MACHINE_SUPPORTS_SAVE )
SYST( 1981, mephistoe2a, mephistoe2, 0,      mephistoe2a, mephistoe2a, brikett_state, empty_init, "Hegener + Glaser", "Mephisto ESB II (ESB II board)", MACHINE_SUPPORTS_SAVE )

SYST( 1983, mephisto3,   0,          0,      mephisto3,   mephisto3,   brikett_state, empty_init, "Hegener + Glaser", "Mephisto III (set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1983, mephisto3a,  mephisto3,  0,      mephisto3,   mephisto3,   brikett_state, empty_init, "Hegener + Glaser", "Mephisto III (set 2)", MACHINE_SUPPORTS_SAVE )
SYST( 1983, mephisto3b,  mephisto3,  0,      mephisto3,   mephisto3,   brikett_state, empty_init, "Hegener + Glaser", "Mephisto III (set 3)", MACHINE_SUPPORTS_SAVE )
SYST( 1983, mephisto3c,  mephisto3,  0,      mephisto3,   mephisto3,   brikett_state, empty_init, "Hegener + Glaser", "Mephisto III (set 4)", MACHINE_SUPPORTS_SAVE )
