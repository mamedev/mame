// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Novag Constellation Forte

Hardware notes:
- R65C02P4 @ 5MHz (10MHz XTAL)
- 2*2KB RAM(NEC D449C-3), 2*32KB ROM(27C256)
- HLCD0538P, 10-digit 7seg LCD display
- TTL, 18 LEDs, 8*8 chessboard buttons
- ports for optional printer and chess clock

I/O is similar to supercon

TODO:
- add power-off NMI? does nothing, it will just go into an infinite loop

*******************************************************************************/

#include "emu.h"

#include "cpu/m6502/r65c02.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/beep.h"
#include "video/hlcd0538.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "novag_cforte.lh"


namespace {

class cforte_state : public driver_device
{
public:
	cforte_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_board(*this, "board"),
		m_lcd(*this, "hlcd0538"),
		m_beeper(*this, "beeper"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine configs
	void cforte(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<sensorboard_device> m_board;
	required_device<hlcd0538_device> m_lcd;
	required_device<beep_device> m_beeper;
	required_ioport_array<8> m_inputs;

	u8 m_inp_mux = 0;
	u8 m_led_select = 0;

	// address maps
	void main_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void update_display();
	void lcd_output_w(u64 data);
	void mux_w(u8 data);
	void control_w(u8 data);
	u8 input1_r();
	u8 input2_r();
};

void cforte_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_select));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// LCD

void cforte_state::lcd_output_w(u64 data)
{
	// 4 rows used
	u32 rowdata[4];
	for (int i = 0; i < 4; i++)
		rowdata[i] = (data >> i & 1) ? u32(data >> 8) : 0;

	// 2 segments per row
	for (int dig = 0; dig < 13; dig++)
	{
		data = 0;
		for (int i = 0; i < 4; i++)
			data |= ((rowdata[i] >> (2*dig) & 3) << (2*i));

		data = bitswap<8>(data,7,2,0,4,6,5,3,1);
		m_display->write_row(dig+3, data);
	}
}


// misc

void cforte_state::update_display()
{
	// 3 led rows
	m_display->matrix_partial(0, 3, m_led_select, m_inp_mux);
}

void cforte_state::mux_w(u8 data)
{
	// d0-d7: input mux, led data
	m_inp_mux = data;
	update_display();
}

void cforte_state::control_w(u8 data)
{
	// d0: HLCD0538 data in
	// d1: HLCD0538 clk
	// d2: HLCD0538 lcd
	m_lcd->data_w(BIT(data, 0));
	m_lcd->clk_w(BIT(data, 1));
	m_lcd->lcd_w(BIT(data, 2));

	// d3: ? (goes high at power-off NMI)

	// d4-d6: select led row
	m_led_select = data >> 4 & 7;
	update_display();

	// d7: enable beeper
	m_beeper->set_state(BIT(data, 7));
}

u8 cforte_state::input1_r()
{
	u8 data = 0;

	// d0-d7: multiplexed inputs (chessboard squares)
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_rank(i ^ 7, true);

	return ~data;
}

u8 cforte_state::input2_r()
{
	u8 data = 0;

	// d6,d7: multiplexed inputs (side panel)
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read() << 6;

	// other: ?
	return ~data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void cforte_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("nvram");
	map(0x1c00, 0x1c00).nopw(); // accessory?
	map(0x1d00, 0x1d00).nopw(); // "
	map(0x1e00, 0x1e00).rw(FUNC(cforte_state::input2_r), FUNC(cforte_state::mux_w));
	map(0x1f00, 0x1f00).rw(FUNC(cforte_state::input1_r), FUNC(cforte_state::control_w));
	map(0x2000, 0xffff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( cforte )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("New Game")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Player/Player / Gambit/Large / King")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Verify/Set Up / Pro-Op")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Random/Tour/Normal / Training Level / Queen")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Change Color / Time Control / Priority")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Sound / Depth Search / Bishop")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Flip Display / Clear Board / Clear Book")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Solve Mate / Infinite / Knight")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Print Moves / Print Evaluations / Print Book")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Print Board / Interface / Rook")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trace Forward / Auto Play / No/End")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Print List / Acc. Time / Pawn")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Hint / Next Best / Yes/Start")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Set Level")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Go / Right")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Take Back / Restore / Left")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void cforte_state::cforte(machine_config &config)
{
	// basic machine hardware
	R65C02(config, m_maincpu, 10_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cforte_state::main_map);

	auto &irq_clock(CLOCK(config, "irq_clock", 32.768_kHz_XTAL/128)); // 256Hz
	irq_clock.set_pulse_width(attotime::from_usec(11)); // active for 11us
	irq_clock.signal_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(200));
	m_board->set_nvram_enable(true);

	// video hardware
	HLCD0538(config, m_lcd).write_cols().set(FUNC(cforte_state::lcd_output_w));
	PWM_DISPLAY(config, m_display).set_size(3+13, 8);
	m_display->set_segmask(0x3ff0, 0xff);
	config.set_default_layout(layout_novag_cforte);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 32.768_kHz_XTAL/32); // 1024Hz
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( cfortea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("a_903_l", 0x0000, 0x8000, CRC(01e7e306) SHA1(6a2b982bb0f412a63f5d3603958dd863e38669d9) ) // NEC D27C256AD-12
	ROM_LOAD("a_903_h", 0x8000, 0x8000, CRC(c5a5573f) SHA1(7e11eb2f3d96bc41386a14a19635427a386ec1ec) ) // "
ROM_END

ROM_START( cforteb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("forte_b_l.bin", 0x0000, 0x8000, CRC(e3d194a1) SHA1(80457580d7c57e07895fd14bfdaf14b30952afca) )
	ROM_LOAD("forte_b_h.bin", 0x8000, 0x8000, CRC(dd824be8) SHA1(cd8666b6b525887f9fc48a730b71ceabcf07f3b9) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1986, cfortea, 0,       0,      cforte,  cforte, cforte_state, empty_init, "Novag Industries / Intelligent Heuristic Programming", "Constellation Forte (version A)", MACHINE_SUPPORTS_SAVE )
SYST( 1986, cforteb, cfortea, 0,      cforte,  cforte, cforte_state, empty_init, "Novag Industries / Intelligent Heuristic Programming", "Constellation Forte (version B)", MACHINE_SUPPORTS_SAVE )
