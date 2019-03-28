// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************
*
* novag_cforte.cpp, subdriver of machine/novagbase.cpp, machine/chessbase.cpp

TODO:
- RS232 port?

*******************************************************************************

Novag Constellation Forte overview:
- R65C02P4 @ 5MHz (10MHz XTAL)
- 2*2KB RAM(NEC D449C-3), 2*32KB ROM(27C256)
- HLCD0538P, 10-digit 7seg LCD display
- TTL, 18 LEDs, 8*8 chessboard buttons

I/O is similar to supercon

******************************************************************************/

#include "emu.h"
#include "includes/novagbase.h"

#include "cpu/m6502/r65c02.h"
#include "machine/nvram.h"
#include "video/hlcd0538.h"
#include "speaker.h"

// internal artwork
#include "novag_cforte.lh" // clickable


namespace {

class cforte_state : public novagbase_state
{
public:
	cforte_state(const machine_config &mconfig, device_type type, const char *tag) :
		novagbase_state(mconfig, type, tag),
		m_hlcd0538(*this, "hlcd0538")
	{ }

	// machine drivers
	void cforte(machine_config &config);

private:
	// devices/pointers
	optional_device<hlcd0538_device> m_hlcd0538;

	// address maps
	void main_map(address_map &map);

	// I/O handlers
	void prepare_display();
	DECLARE_WRITE64_MEMBER(lcd_output_w);
	DECLARE_WRITE8_MEMBER(mux_w);
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_READ8_MEMBER(input1_r);
	DECLARE_READ8_MEMBER(input2_r);
};


/******************************************************************************
    Devices, I/O
******************************************************************************/

// TTL/generic

void cforte_state::prepare_display()
{
	// 3 led rows
	display_matrix(8, 3, m_led_data, m_led_select, false);

	// lcd panel (mostly handled in lcd_output_w)
	set_display_segmask(0x3ff0, 0xff);
	set_display_size(8, 3+13);
	display_update();
}

WRITE64_MEMBER(cforte_state::lcd_output_w)
{
	// 4 rows used
	u32 rowdata[4];
	for (int i = 0; i < 4; i++)
		rowdata[i] = (data >> i & 1) ? u32(data >> 8) : 0;

	// 2 segments per row
	for (int dig = 0; dig < 13; dig++)
	{
		m_display_state[dig+3] = 0;
		for (int i = 0; i < 4; i++)
			m_display_state[dig+3] |= ((rowdata[i] >> (2*dig) & 3) << (2*i));

		m_display_state[dig+3] = bitswap<8>(m_display_state[dig+3],7,2,0,4,6,5,3,1);
	}

	prepare_display();
}

WRITE8_MEMBER(cforte_state::mux_w)
{
	// d0-d7: input mux, led data
	m_inp_mux = m_led_data = data;
	prepare_display();
}

WRITE8_MEMBER(cforte_state::control_w)
{
	// d0: HLCD0538 data in
	// d1: HLCD0538 clk
	// d2: HLCD0538 lcd
	m_hlcd0538->data_w(data & 1);
	m_hlcd0538->clk_w(data >> 1 & 1);
	m_hlcd0538->lcd_w(data >> 2 & 1);

	// d3: unused?

	// d4-d6: select led row
	m_led_select = data >> 4 & 7;
	prepare_display();

	// d7: enable beeper
	m_beeper->set_state(data >> 7 & 1);
}

READ8_MEMBER(cforte_state::input1_r)
{
	// d0-d7: multiplexed inputs (chessboard squares)
	return ~read_inputs(8) & 0xff;
}

READ8_MEMBER(cforte_state::input2_r)
{
	// d0-d5: ?
	// d6,d7: multiplexed inputs (side panel)
	return (read_inputs(8) >> 2 & 0xc0) ^ 0xff;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void cforte_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("nvram");
	map(0x1c00, 0x1c00).nopw(); // printer?
	map(0x1d00, 0x1d00).nopw(); // printer?
	map(0x1e00, 0x1e00).rw(FUNC(cforte_state::input2_r), FUNC(cforte_state::mux_w));
	map(0x1f00, 0x1f00).rw(FUNC(cforte_state::input1_r), FUNC(cforte_state::control_w));
	map(0x2000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( cforte )
	PORT_INCLUDE( generic_cb_buttons )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("New Game")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Player/Player / Gambit/Large / King")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Verify/Set Up / Pro-Op")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Random/Tour/Normal / Training Level / Queen")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Change Color / Time Control / Priority")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Sound / Depth Search / Bishop")

	PORT_MODIFY("IN.3")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Flip Display / Clear Board / Clear Book")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Solve Mate / Infinite / Knight")

	PORT_MODIFY("IN.4")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Print Moves / Print Evaluations / Print Book")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Print Board / Interface / Rook")

	PORT_MODIFY("IN.5")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trace Forward / Auto Play / No/End")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Print List / Acc. Time / Pawn")

	PORT_MODIFY("IN.6")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Hint / Next Best / Yes/Start")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Set Level")

	PORT_MODIFY("IN.7")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Go / ->")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Take Back / Restore / <-")
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void cforte_state::cforte(machine_config &config)
{
	/* basic machine hardware */
	R65C02(config, m_maincpu, 10_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cforte_state::main_map);

	const attotime irq_period = attotime::from_hz(32.768_kHz_XTAL/128); // 256Hz
	TIMER(config, m_irq_on).configure_periodic(FUNC(cforte_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_usec(11)); // active for 11us
	TIMER(config, "irq_off").configure_periodic(FUNC(cforte_state::irq_off<M6502_IRQ_LINE>), irq_period);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	/* video hardware */
	HLCD0538(config, m_hlcd0538);
	m_hlcd0538->write_cols().set(FUNC(cforte_state::lcd_output_w));

	TIMER(config, "display_decay").configure_periodic(FUNC(cforte_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_novag_cforte);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 32.768_kHz_XTAL/32); // 1024Hz
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

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



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME     PARENT  CMP MACHINE  INPUT   STATE         INIT        COMPANY, FULLNAME, FLAGS
CONS( 1986, cfortea, 0,       0, cforte,  cforte, cforte_state, empty_init, "Novag", "Constellation Forte (version A)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1986, cforteb, cfortea, 0, cforte,  cforte, cforte_state, empty_init, "Novag", "Constellation Forte (version B)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
