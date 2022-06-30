// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

Saitek Kasparov Renaissance

Saitek's 2nd version modular chesscomputer. It accepts the same modules as
Leonardo/Galileo. "OSA" version for Renaissance is 1.5.

Hardware notes:
- 6301Y0(mode 1) or HD6303YP MCU @ 10MHz
- 8KB RAM, 32KB ROM
- "HELIOS" I/O (NEC gate array)
- Epson SED1502F, LCD screen
- magnet sensors chessboard with 81 leds

The 6301Y0 seen on one of them, was a SX8A 6301Y0G84P, this is in fact the
MCU(+internal maskrom, disabled here) used in Saitek Conquistador.

The LCD screen is fairly large, it's the same one as in Saitek Simultano,
so a chessboard display + 7seg info. It's on a small drawer that can be
pushed in to hide the chessboard display.

TODO:
- fart noise at boot if maestroa module is inserted
- weird beep at boot if sparc module is inserted (related to above?)
- make it a subdriver of leonardo.cpp? or too many differences
- same TODO list as saitek_leonardo.cpp

******************************************************************************/

#include "emu.h"

#include "bus/saitek_osa/expansion.h"
#include "cpu/m6800/m6801.h"
#include "machine/input_merger.h"
#include "machine/sensorboard.h"
#include "sound/spkrdev.h"
#include "video/pwm.h"
#include "video/sed1500.h"

#include "render.h"
#include "screen.h"
#include "speaker.h"

// internal artwork
#include "saitek_renaissance.lh" // clickable


namespace {

class ren_state : public driver_device
{
public:
	ren_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_expansion(*this, "exp"),
		m_stb(*this, "stb"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_lcd(*this, "lcd"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0),
		m_out_lcd(*this, "s%u.%u", 0U, 0U)
	{ }

	template <int N> DECLARE_INPUT_CHANGED_MEMBER(change_view);

	// machine configs
	void ren(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	// devices/pointers
	required_device<hd6303y_cpu_device> m_maincpu;
	required_device<saitekosa_expansion_device> m_expansion;
	required_device<input_merger_device> m_stb;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<sed1502_device> m_lcd;
	optional_device<speaker_sound_device> m_dac;
	required_ioport_array<8+1> m_inputs;
	output_finder<16, 34> m_out_lcd;

	void main_map(address_map &map);

	void lcd_pwm_w(offs_t offset, u8 data);
	void lcd_output_w(offs_t offset, u64 data);

	void update_display();
	void mux_w(u8 data);
	void leds_w(u8 data);
	void control_w(u8 data);
	u8 control_r();
	void exp_rts_w(int state);

	u8 p2_r();
	void p2_w(u8 data);
	u8 p5_r();
	void p5_w(u8 data);
	u8 p6_r();
	void p6_w(u8 data);

	u8 m_inp_mux = 0;
	u8 m_led_data[2] = { };
};

void ren_state::machine_start()
{
	m_out_lcd.resolve();

	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_data));
}

void ren_state::machine_reset()
{
	m_stb->in_clear<0>();
}

template <int N> INPUT_CHANGED_MEMBER(ren_state::change_view)
{
	if (oldval && !newval)
	{
		// meant for changing lcd drawer view
		render_target *target = machine().render().first_target();
		target->set_view(target->view() + N);
	}
}



/******************************************************************************
    I/O
******************************************************************************/

// LCD

void ren_state::lcd_pwm_w(offs_t offset, u8 data)
{
	m_out_lcd[offset & 0x3f][offset >> 6] = data;
}

void ren_state::lcd_output_w(offs_t offset, u64 data)
{
	m_lcd_pwm->write_row(offset, data);
}


// misc

void ren_state::update_display()
{
	m_display->matrix_partial(0, 9, 1 << (m_inp_mux & 0xf), (m_inp_mux << 4 & 0x100) | m_led_data[0]);
	m_display->matrix_partial(9, 1, 1, (m_inp_mux >> 2 & 0x38) | m_led_data[1]);
}

void ren_state::mux_w(u8 data)
{
	// d0-d3 input/chessboard led mux
	// d4: chessboard led data
	// d5: module led
	// d6,d7: mode led
	m_inp_mux = data ^ 0x20;
	update_display();
}

void ren_state::leds_w(u8 data)
{
	// chessboard led data
	m_led_data[0] = data;
	update_display();
}

void ren_state::control_w(u8 data)
{
	// d1: speaker out
	m_dac->level_w(BIT(data, 1));

	// d2: comm led
	m_led_data[1] = (m_led_data[1] & ~0x4) | (~data & 0x4);
	update_display();

	// other: ?
}

u8 ren_state::control_r()
{
	// d5,d6: freq sel?
	// d7: ?
	return 0;
}

void ren_state::exp_rts_w(int state)
{
	// NAND with ACK-P (not used by chesscomputer?)
}


// MCU ports

u8 ren_state::p2_r()
{
	u8 data = 0;

	// d0-d2: multiplexed inputs
	if (~m_inp_mux & 8)
		data = m_inputs[m_inp_mux & 7]->read();

	// d3: ?
	return ~data;
}

void ren_state::p2_w(u8 data)
{
	// d5,d6: b/w leds
	m_led_data[1] = (m_led_data[1] & ~3) | (~data >> 5 & 3);
	update_display();
}

u8 ren_state::p5_r()
{
	// d6: battery status
	u8 b = m_inputs[8]->read() & 0x40;

	// d4: IS strobe (handled with inputline)
	// other: ?
	return b | (0xff ^ 0x50);
}

void ren_state::p5_w(u8 data)
{
	// d1: expansion NMI-P
	m_expansion->nmi_w(BIT(data, 1));

	// d3: NAND with STB-P
	m_stb->in_w<1>(BIT(data, 3));

	// d5: expansion ACK-P
	m_expansion->ack_w(BIT(data, 5));

	// d0: power-off on falling edge
	m_expansion->pw_w(data & 1);

	// other: ?
}

u8 ren_state::p6_r()
{
	// read chessboard sensors and module data
	return ~m_board->read_file(m_inp_mux & 0xf) & m_expansion->data_r();
}

void ren_state::p6_w(u8 data)
{
	// module data
	m_expansion->data_w(data);
}



/******************************************************************************
    Address Maps
******************************************************************************/

void ren_state::main_map(address_map &map)
{
	map(0x0000, 0x0027).m(m_maincpu, FUNC(hd6303y_cpu_device::hd6301y_io));
	map(0x0040, 0x013f).ram(); // internal
	map(0x2000, 0x2000).w(FUNC(ren_state::mux_w));
	map(0x2400, 0x2400).w(FUNC(ren_state::leds_w));
	map(0x2600, 0x2600).rw(FUNC(ren_state::control_r), FUNC(ren_state::control_w));
	map(0x4000, 0x5fff).ram();
	map(0x6000, 0x607f).w("lcd", FUNC(sed1502_device::write));
	map(0x8000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( ren )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Scroll")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Tab / Color")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_EQUALS) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Function")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Sound")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Stop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Library")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Info")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Play")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_L) PORT_NAME("Level")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Normal")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Analysis")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Set Up")

	PORT_START("IN.8")
	PORT_CONFNAME( 0x40, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x40, "Low" )
	PORT_CONFSETTING(    0x00, DEF_STR( Normal ) )

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Go")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F1) PORT_NAME("ACL")

	PORT_START("VIEW")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CHANGED_MEMBER(DEVICE_SELF, ren_state, change_view<+1>, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CHANGED_MEMBER(DEVICE_SELF, ren_state, change_view<-1>, 0)
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void ren_state::ren(machine_config &config)
{
	// basic machine hardware
	HD6303Y(config, m_maincpu, 10_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ren_state::main_map);
	m_maincpu->in_p2_cb().set(FUNC(ren_state::p2_r));
	m_maincpu->out_p2_cb().set(FUNC(ren_state::p2_w));
	m_maincpu->in_p5_cb().set(FUNC(ren_state::p5_r));
	m_maincpu->out_p5_cb().set(FUNC(ren_state::p5_w));
	m_maincpu->in_p6_cb().set(FUNC(ren_state::p6_r));
	m_maincpu->out_p6_cb().set(FUNC(ren_state::p6_w));

	INPUT_MERGER_ANY_LOW(config, m_stb);
	m_stb->output_handler().set_inputline(m_maincpu, M6801_IS_LINE);

	config.set_maximum_quantum(attotime::from_hz(6000));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	SED1502(config, m_lcd, 32768).write_segs().set(FUNC(ren_state::lcd_output_w));
	PWM_DISPLAY(config, m_lcd_pwm).set_size(16, 34);
	m_lcd_pwm->set_refresh(attotime::from_hz(30));
	m_lcd_pwm->output_x().set(FUNC(ren_state::lcd_pwm_w));

	auto &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(873/2, 1080/2);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(9+1, 9);
	config.set_default_layout(layout_saitek_renaissance);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	SPEAKER_SOUND(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);

	// expansion module
	SAITEKOSA_EXPANSION(config, m_expansion, saitekosa_expansion_modules);
	m_expansion->stb_handler().set(m_stb, FUNC(input_merger_device::in_w<0>));
	m_expansion->rts_handler().set(FUNC(ren_state::exp_rts_w));
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( renaissa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("sw7_518d_u3.u3", 0x8000, 0x8000, CRC(21d2405f) SHA1(6ddcf9bdd30aa446fcaeab919a8f950dc3428365) ) // HN27C256AG-10

	ROM_REGION( 795951, "screen", 0 )
	ROM_LOAD("simultano.svg", 0, 795951, CRC(ac9942bb) SHA1(f9252e5bf7b8af698a403c3f8f5ea9e475e0bf0b) )
ROM_END

ROM_START( renaissaa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("sx7_518b.u3", 0x8000, 0x8000, CRC(a0c3ffe8) SHA1(fa170a6d4d54d41de77e0bb72f969219e6f376af) ) // MBM27C256H-10

	ROM_REGION( 795951, "screen", 0 )
	ROM_LOAD("simultano.svg", 0, 795951, CRC(ac9942bb) SHA1(f9252e5bf7b8af698a403c3f8f5ea9e475e0bf0b) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME       PARENT    CMP  MACHINE INPUT CLASS      INIT        COMPANY, FULLNAME, FLAGS
CONS( 1989, renaissa,  0,        0,   ren,    ren,  ren_state, empty_init, "Saitek", "Kasparov Renaissance (set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1989, renaissaa, renaissa, 0,   ren,    ren,  ren_state, empty_init, "Saitek", "Kasparov Renaissance (set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
