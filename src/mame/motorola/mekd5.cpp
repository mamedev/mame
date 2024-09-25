// license:BSD-3-Clause
// copyright-holders: 68bit
/******************************************************************************

Motorola Evaluation Kit 6802 D5 - MEK6802D5

Memory map

Range    Short  Description

0000-dfff RAM   Either 128 bytes on board, or external

e000-e3ff RAM   Static RAM, 1K.
e400-e47f RAM   System RAM
e480-e483 PIA   User PIA
e484-e487 PIA   System PIA

e700-e701 ACIA  System ACIA.

e800-efff ROM   Optional user ROM
f000-f7ff ROM   D5BUG monitor ROM
f800-ffff ROM   D5BUG (mirror), or optional user ROM.


A 1K or 2K optional user ROM or EPROM can be installed and mapped to either
0xe800-0xefff, or to 0xf800-0xffff, set via jumper 3.
TODO implement this user ROM.

The board has provision for an ACIA, and the documentation mentions that it is
not used when there is a keypad, and the keypad is removable. However the
D5BUG monitor has no support for this ACIA. Was there an alternative official
monitor that used this ACIA?


Keypad commands:

RS (Reset)  Reset, wired to the CPU reset line
EX (Escape) Typically aborts user program.
M (Memory display/change)
  Digits 5 and 6 show the actual data at the address.
  G  - increase the address.
  M  - decreases the address.
  FS - Offset calculation. Enter address, then press 'GO'.
    GO - stores the offset and returns to memory display and increased the address.
    FC - return to memory display, without storing the offset.
    M  - return to memory display, after a BAD offset.
  EX - exits memory display.
RD (Register display/alter)
  G   - advance to next register.
  M   - previous register.
  T/B - trace a single instruction
  EX  - exits register display.
GO to user program.
  If no address if entered then it uses the pseudo PC, it continues.
  Enter the address and press 'Go' to use that entered address.
  It firstly checks that there is RAM at the stack pointer.
FS T/B - Breakpoint editor
  GO - advance to next breakpoing, up to 8, then loops.
  FS - insert a breakpoint
  FC - deactivate breakpoint
  EX - exits breakpoing editor.
P/L (Punch tape)
  At the 'bb' prompt enter the beginning address of the data, then 'GO'.
  At the 'EE' prompt enter the last address of the data.
  Start the tape and press GO. There is a 30 second leader of $ff.
FS P/L (Load from tape)
FS RD (Verify from tape)
FS 0 to F
  One of 16 user defined functions. Press FS then one number key 0 to F.
  A pointer to a table of 16 function addresses should be set at 0xe43f.

******************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/input_merger.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/mc14411.h"
#include "machine/clock.h"
#include "machine/timer.h"
#include "video/pwm.h"
#include "sound/wave.h"
#include "speaker.h"
#include "bus/rs232/rs232.h"
#include "machine/terminal.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "render.h"
#include "mekd5.lh"


namespace {

#define XTAL_MEKD5 3.579545_MHz_XTAL

class mekd5_state : public driver_device
{
public:
	mekd5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_kpd_pia(*this, "kpd_pia")
		, m_user_pia(*this, "user_pia")
		, m_display(*this, "display")
		, m_brg(*this, "brg")
		, m_baud_rate(*this, "BAUD_RATE")
		, m_acia(*this, "acia")
		, m_cass(*this, "cassette")
		, m_keypad_columns(*this, "COL%u", 0)
	{ }

	void mekd5(machine_config &config);

	void reset_key_w(int state);
	DECLARE_INPUT_CHANGED_MEMBER(keypad_changed);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void trace_timer_clear_w(int state);

	int keypad_cb1_r();
	uint8_t keypad_key_r();
	void led_digit_w(uint8_t data);
	void led_segment_w(uint8_t data);
	int kansas_r();

	// Clocks
	void write_f1_clock(int state);
	void write_f3_clock(int state);
	void write_f5_clock(int state);
	void write_f7_clock(int state);
	void write_f9_clock(int state);
	void write_f13_clock(int state);

	void mekd5_mem(address_map &map) ATTR_COLD;

	bool keypad_key_pressed();

	TIMER_CALLBACK_MEMBER(trace_tick);

	emu_timer *m_trace_timer = nullptr;
	uint8_t m_segment;
	uint8_t m_digit;

	required_device<m6802_cpu_device> m_maincpu;
	required_device<pia6821_device> m_kpd_pia;
	required_device<pia6821_device> m_user_pia;
	required_device<pwm_display_device> m_display;
	required_device<mc14411_device> m_brg;
	required_ioport m_baud_rate;
	required_device<acia6850_device> m_acia;
	required_device<cassette_image_device> m_cass;
	required_ioport_array<4> m_keypad_columns;
};



/***********************************************************

    Address Map

************************************************************/

void mekd5_state::mekd5_mem(address_map &map)
{
	map(0x0000, 0xdfff).ram();
	map(0xe000, 0xe3ff).ram();
	map(0xe400, 0xe47f).ram();

	map(0xe480, 0xe483).mirror(0x0378).rw(m_user_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xe484, 0xe487).mirror(0x0378).rw(m_kpd_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));

	map(0xe700, 0xe701).mirror(0x003e).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));

	/* D5BUG ROM */
	map(0xf000, 0xf7ff).rom().mirror(0x0800);
}

/***********************************************************

    Keys

************************************************************/

static INPUT_PORTS_START(mekd5)

	// RESET is not wired to the key matrix.
	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("RS") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, mekd5_state, reset_key_w)

	PORT_START("COL0")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("FS") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("0") PORT_CODE(KEYCODE_0)

	PORT_START("COL1")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("EX") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("FC") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("F") PORT_CODE(KEYCODE_F)

	PORT_START("COL2")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("RD") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("P/L") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("E") PORT_CODE(KEYCODE_E)

	PORT_START("COL3")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("GO") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("T/B") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, mekd5_state, keypad_changed, 0) PORT_NAME("D") PORT_CODE(KEYCODE_D)

	/* RS232 baud rates available via J5. */
	PORT_START("BAUD_RATE")
	PORT_CONFNAME(0x3f, 1, "RS232 Baud Rate")
	PORT_CONFSETTING(0x20, "110")
	PORT_CONFSETTING(0x10, "300")
	PORT_CONFSETTING(0x08, "1200")
	PORT_CONFSETTING(0x04, "2400")
	PORT_CONFSETTING(0x02, "4800")
	PORT_CONFSETTING(0x01, "9600")

INPUT_PORTS_END

/***********************************************************

    Trace timer

************************************************************/

TIMER_CALLBACK_MEMBER(mekd5_state::trace_tick)
{
	// CB2 is programmed to trigger on the falling edge, so after
	// a count of 16. CB2 input comes from a counter, so the duty
	// cycle should be 50/50, but it makes no difference to rise
	// and fall here.
	m_kpd_pia->cb2_w(1);
	m_kpd_pia->cb2_w(0);
}


// Expect a delay of 16 cycles.  However the 6800 cycle model appears to
// account for the store that writes here as occuring at the start of that
// instruction adding 5 cycles to give an effective 21 cycles. TODO adjust
// this back to 16 cycles when the 6800 cycle timing becomes more accurate.
void mekd5_state::trace_timer_clear_w(int state)
{
	if (state)
		m_kpd_pia->cb2_w(0);
	else
		m_trace_timer->adjust(attotime::from_ticks(21, XTAL_MEKD5 / 4));
}

/***********************************************************

    Keypad

************************************************************/

// Keypad input is disable on views with the RS232 input.

void mekd5_state::reset_key_w(int state)
{
	uint8_t view = machine().render().first_target()->view();
	if (view > 1) return;

	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);

	// TODO reset other devices.
}


bool mekd5_state::keypad_key_pressed()
{
	uint8_t view = machine().render().first_target()->view();
	if (view > 1) return 0;

	return (m_keypad_columns[0]->read() & m_digit) ||
		(m_keypad_columns[1]->read() & m_digit) ||
		(m_keypad_columns[2]->read() & m_digit) ||
		(m_keypad_columns[3]->read() & m_digit);
}

INPUT_CHANGED_MEMBER(mekd5_state::keypad_changed)
{
	m_kpd_pia->cb1_w(mekd5_state::keypad_key_pressed());
}

int mekd5_state::keypad_cb1_r()
{
	return mekd5_state::keypad_key_pressed();
}

uint8_t mekd5_state::keypad_key_r()
{
	uint8_t view = machine().render().first_target()->view();
	if (view > 1) return m_segment;

	uint8_t mux = (m_digit & 0xc0) >> 6;
	uint8_t i = (m_keypad_columns[mux]->read() & m_digit) ? 0 : 0x80;

	return i | m_segment;
}

/***********************************************************

    Seven segment LED display, and cassette

************************************************************/

// PA
void mekd5_state::led_segment_w(uint8_t data)
{
	m_segment = data & 0x7f;
	m_display->matrix(m_digit & 0x3f, ~m_segment);
}

// PB
void mekd5_state::led_digit_w(uint8_t data)
{
	m_digit = data;
	m_display->matrix(m_digit & 0x3f, ~m_segment);
	// PB7 also drives the cassette output.
	m_cass->output(BIT(data, 7) ? -1.0 : +1.0);
	// Update the keypad pressed output which depends on m_digit.
	m_kpd_pia->cb1_w(mekd5_state::keypad_key_pressed());
}

int mekd5_state::kansas_r()
{
	uint8_t data = m_cass->input() > +0.0;
	return data;
}


/***********************************************************

  ACIA clocks

************************************************************/

void mekd5_state::write_f1_clock(int state)
{
	if (BIT(m_baud_rate->read(), 0))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void mekd5_state::write_f3_clock(int state)
{
	if (BIT(m_baud_rate->read(), 1))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void mekd5_state::write_f5_clock(int state)
{
	if (BIT(m_baud_rate->read(), 2))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void mekd5_state::write_f7_clock(int state)
{
	if (BIT(m_baud_rate->read(), 3))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void mekd5_state::write_f9_clock(int state)
{
	if (BIT(m_baud_rate->read(), 4))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void mekd5_state::write_f13_clock(int state)
{
	if (BIT(m_baud_rate->read(), 5))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}


/***********************************************************

************************************************************/

void mekd5_state::machine_start()
{
	save_item(NAME(m_segment));
	save_item(NAME(m_digit));

	m_trace_timer = timer_alloc(FUNC(mekd5_state::trace_tick), this);
}

void mekd5_state::machine_reset()
{
	// Trace timer out low.
	m_kpd_pia->cb2_w(0);

	m_brg->rsa_w(CLEAR_LINE);
	m_brg->rsb_w(ASSERT_LINE);

	// /DCD and /CTS are wired low.
	m_acia->write_dcd(CLEAR_LINE);
	m_acia->write_cts(CLEAR_LINE);
}

/***********************************************************

    Machine

************************************************************/

static DEVICE_INPUT_DEFAULTS_START(terminal)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END

void mekd5_state::mekd5(machine_config &config)
{
	M6802(config, m_maincpu, XTAL_MEKD5);        /* 894.8 kHz clock */
	m_maincpu->set_ram_enable(false);
	m_maincpu->set_addrmap(AS_PROGRAM, &mekd5_state::mekd5_mem);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, M6802_IRQ_LINE);
	INPUT_MERGER_ANY_HIGH(config, "mainnmi").output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	// LED display
	PWM_DISPLAY(config, m_display).set_size(6, 7);
	m_display->set_segmask(0x3f, 0x7f);

	config.set_default_layout(layout_mekd5);

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	// Keypad and display PIA (U23). IRQA is NC. CB2 is trace timer input.
	PIA6821(config, m_kpd_pia);
	m_kpd_pia->readpa_handler().set(FUNC(mekd5_state::keypad_key_r));
	m_kpd_pia->writepa_handler().set(FUNC(mekd5_state::led_segment_w));
	m_kpd_pia->writepb_handler().set(FUNC(mekd5_state::led_digit_w));
	m_kpd_pia->readca1_handler().set(FUNC(mekd5_state::kansas_r));
	m_kpd_pia->ca2_handler().set(FUNC(mekd5_state::trace_timer_clear_w));
	m_kpd_pia->readcb1_handler().set(FUNC(mekd5_state::keypad_cb1_r));
	m_kpd_pia->irqb_handler().set("mainnmi", FUNC(input_merger_device::in_w<1>));

	// User PIA (U9).
	// IRQA and IRQB can be independently jumpered to IRQ or NMI via J1.
	// All the I/O lines are available at the User I/O connector.
	PIA6821(config, m_user_pia);

	// IRQ is NC. RX and TX clk are wired together. RTS is available.
	// /DCD and /CTS and wired low.
	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));

	MC14411(config, m_brg, XTAL(1'843'200));
	m_brg->out_f<1>().set(FUNC(mekd5_state::write_f1_clock));
	m_brg->out_f<3>().set(FUNC(mekd5_state::write_f3_clock));
	m_brg->out_f<5>().set(FUNC(mekd5_state::write_f5_clock));
	m_brg->out_f<7>().set(FUNC(mekd5_state::write_f7_clock));
	m_brg->out_f<9>().set(FUNC(mekd5_state::write_f9_clock));
	m_brg->out_f<13>().set(FUNC(mekd5_state::write_f13_clock));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
}

/***********************************************************

    ROMS

************************************************************/

ROM_START(mekd5)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("d5bug.rom", 0xf000, 0x0800, CRC(67c00a2c) SHA1(ae321dbca0baf4b67d62bfec77266d9132b973bf))
ROM_END

} // anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE   INPUT  CLASS        INIT        COMPANY     FULLNAME      FLAGS
COMP( 1980, mekd5,  0,      0,      mekd5,    mekd5, mekd5_state, empty_init, "Motorola", "MEK6802D5" , MACHINE_NO_SOUND )
