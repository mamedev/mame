// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
/*******************************************************************************

VEB Mikroelektronik "Karl Marx" Erfurt Chess-Master (aka Schachcomputer CM)
- Chess-Master (G-5003-500) (10*U505 roms)
- Chess-Master (G-5003-501) (2 roms set)

Unlike SC2, the chess engine was not copied from an existing one. It is an
original creation by Rüdiger Worbs and Dieter Schultze. It competed in
Budapest WMCCC 1983 and ended at a low 16th place.

Hardware notes:
- UB880 Z80 @ ~2.5MHz
- 2*Z80 PIO
- 10KB ROM (10*U505D), 2KB RAM (4*U214D)
- chessboard with 64 hall sensors, 64+15 leds, piezo

A newer version had a 4MHz UA880 and 2 ROM chips (8KB + 2KB).

BTANB:
- corner leds flicker sometimes

TODO:
- chessmsta isn't working, needs a redump of u2616. Program differences are
  minor so it seems to boot fine if you take 064/065 from chessmst, but will
  probably have some problems.

*******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/z80pio.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "chessmst.lh"


namespace {

class chessmst_state : public driver_device
{
public:
	chessmst_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pio(*this, "z80pio%u", 0),
		m_board(*this, "board"),
		m_led_pwm(*this, "led_pwm"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(halt_button);
	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

	void chessmst(machine_config &config);
	void chessmsta(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device_array<z80pio_device, 2> m_pio;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_led_pwm;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<2> m_inputs;

	u16 m_matrix = 0;
	u8 m_led_data[2] = { 0, 0 };

	void chessmst_io(address_map &map) ATTR_COLD;
	void chessmst_mem(address_map &map) ATTR_COLD;

	void pio1_port_a_w(u8 data);
	void pio1_port_b_w(u8 data);
	u8 pio2_port_a_r();
	void pio2_port_b_w(u8 data);

	void update_leds();
};

void chessmst_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_matrix));
	save_item(NAME(m_led_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void chessmst_state::update_leds()
{
	m_led_pwm->matrix(m_matrix, m_led_data[0] | m_led_data[1]);
}

void chessmst_state::pio1_port_a_w(u8 data)
{
	// d0-d7: led data
	m_led_data[0] = ~data;
	update_leds();
}

void chessmst_state::pio1_port_b_w(u8 data)
{
	// d0,d1: input mux/led select high
	m_matrix = (m_matrix & 0xff) | ((data & 0x03) << 8);

	// d2,d3: led data 2nd/3rd rows (duplicate)
	m_led_data[1] = ~data >> 1 & 6;
	update_leds();

	// d6: speaker out
	m_dac->write(BIT(data, 6));
}

u8 chessmst_state::pio2_port_a_r()
{
	u8 data = 0xff;

	// read chessboard sensors
	for (int i = 0; i < 8; i++)
		if (BIT(m_matrix, i))
			data &= ~m_board->read_file(i);

	// read other buttons
	if (m_matrix & 0x100)
		data &= m_inputs[0]->read();

	return data;
}

void chessmst_state::pio2_port_b_w(u8 data)
{
	// d0-d7: input mux/led select
	m_matrix = (data & 0xff) | (m_matrix & ~0xff);
	update_leds();
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void chessmst_state::chessmst_mem(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x7fff); // A15 not connected
	map(0x0000, 0x27ff).rom();
	map(0x3400, 0x3bff).ram();
}

void chessmst_state::chessmst_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x0f);
	//map(0x00, 0x03) read/write to both PIOs, but not used by software
	map(0x04, 0x07).rw(m_pio[0], FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x08, 0x0b).rw(m_pio[1], FUNC(z80pio_device::read), FUNC(z80pio_device::write));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

INPUT_CHANGED_MEMBER(chessmst_state::halt_button)
{
	// halt button goes to PIO(0) ASTB pin
	m_pio[0]->strobe_a(newval);
	reset_button(field, param, oldval, newval);
}

INPUT_CHANGED_MEMBER(chessmst_state::reset_button)
{
	// pressing both halt+reset buttons causes a reset
	const bool reset = (m_inputs[1]->read() & 0x03) == 0x03;
	m_maincpu->set_input_line(INPUT_LINE_RESET, reset ? ASSERT_LINE : CLEAR_LINE);
}

static INPUT_PORTS_START( chessmst )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Hint / 7")              PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Random / 6")            PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Referee / 5 / King")    PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Self Play / 4 / Queen") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board / 3 / Rook")      PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Color / 2 / Bishop")    PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Level / 1 / Knight")    PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_L)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("New Game / 0 / Pawn")   PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_N)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Halt")  PORT_CODE(KEYCODE_F2) PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, halt_button, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, reset_button, 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

static const z80_daisy_config chessmst_daisy_chain[] =
{
	{ "z80pio0" },
	{ nullptr }
};

void chessmst_state::chessmst(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 9.8304_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &chessmst_state::chessmst_mem);
	m_maincpu->set_addrmap(AS_IO, &chessmst_state::chessmst_io);
	m_maincpu->set_daisy_config(chessmst_daisy_chain);

	Z80PIO(config, m_pio[0], 9.8304_MHz_XTAL / 4);
	m_pio[0]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio[0]->out_pa_callback().set(FUNC(chessmst_state::pio1_port_a_w));
	m_pio[0]->out_pb_callback().set(FUNC(chessmst_state::pio1_port_b_w));

	Z80PIO(config, m_pio[1], 9.8304_MHz_XTAL / 4);
	m_pio[1]->in_pa_callback().set(FUNC(chessmst_state::pio2_port_a_r));
	m_pio[1]->out_pb_callback().set(FUNC(chessmst_state::pio2_port_b_w));

	SENSORBOARD(config, m_board);
	m_board->set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_led_pwm).set_size(10, 8);
	config.set_default_layout(layout_chessmst);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void chessmst_state::chessmsta(machine_config &config)
{
	chessmst(config);

	// faster UA880 CPU
	const XTAL clk = 8_MHz_XTAL / 2;
	m_maincpu->set_clock(clk);
	m_pio[0]->set_clock(clk);
	m_pio[1]->set_clock(clk);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( chessmst )
	ROM_REGION( 0x2800, "maincpu", 0 )
	ROM_LOAD("bm056.d208", 0x0000, 0x0400, CRC(2b90e5d3) SHA1(c47445964b2e6cb11bd1f27e395cf980c97af196) ) // U505
	ROM_LOAD("bm057.d209", 0x0400, 0x0400, CRC(e666fc56) SHA1(3fa75b82cead81973bea94191a5c35f0acaaa0e6) ) // "
	ROM_LOAD("bm058.d210", 0x0800, 0x0400, CRC(6a17fbec) SHA1(019051e93a5114477c50eaa87e1ff01b02eb404d) ) // "
	ROM_LOAD("bm059.d211", 0x0c00, 0x0400, CRC(e96e3d07) SHA1(20fab75f206f842231f0414ebc473ce2a7371e7f) ) // "
	ROM_LOAD("bm060.d212", 0x1000, 0x0400, CRC(0e31f000) SHA1(daac924b79957a71a4b276bf2cef44badcbe37d3) ) // "
	ROM_LOAD("bm061.d213", 0x1400, 0x0400, CRC(69ad896d) SHA1(25d999b59d4cc74bd339032c26889af00e64df60) ) // "
	ROM_LOAD("bm062.d214", 0x1800, 0x0400, CRC(c42925fe) SHA1(c42d8d7c30a9b6d91ac994cec0cc2723f41324e9) ) // "
	ROM_LOAD("bm063.d215", 0x1c00, 0x0400, CRC(86be4cdb) SHA1(741f984c15c6841e227a8722ba30cf9e6b86d878) ) // "
	ROM_LOAD("bm064.d216", 0x2000, 0x0400, CRC(e82f5480) SHA1(38a939158052f5e6484ee3725b86e522541fe4aa) ) // "
	ROM_LOAD("bm065.d217", 0x2400, 0x0400, CRC(4ec0e92c) SHA1(0b748231a50777391b04c1778750fbb46c21bee8) ) // "
ROM_END

ROM_START( chessmsta )
	ROM_REGION( 0x2800, "maincpu", 0 )
	ROM_LOAD("bm001.d204", 0x0000, 0x2000, CRC(6be28876) SHA1(fd7d77b471e7792aef3b2b3f7ff1de4cdafc94c9) ) // U2364D45
	ROM_LOAD("bm108.d205", 0x2000, 0x0800, CRC(6e69ace3) SHA1(e099b6b6cc505092f64b8d51ab9c70aa64f58f70) BAD_DUMP ) // U2616D45 - problem with d3
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME        PARENT    COMPAT  MACHINE    INPUT     CLASS           INIT        COMPANY                                     FULLNAME                FLAGS
SYST( 1984, chessmst,   0,        0,      chessmst,  chessmst, chessmst_state, empty_init, "VEB Mikroelektronik \"Karl Marx\" Erfurt", "Chess-Master (set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1984, chessmsta,  chessmst, 0,      chessmsta, chessmst, chessmst_state, empty_init, "VEB Mikroelektronik \"Karl Marx\" Erfurt", "Chess-Master (set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
