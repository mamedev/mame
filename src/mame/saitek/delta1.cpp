// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

SciSys Delta-1, it was sold by both Novag and SciSys, ROM has "COPY RIGHT WINKLER
HK 1979", Winkler was the founder of SciSys(later renamed to Saitek). It was
released in late-1980, even though it says 1979 in the ROM and manual.

Hardware notes:
- 3850PK CPU at ~2MHz, 3853PK memory interface
- 4KB ROM(2332A), 256 bytes RAM(2*2111A-4)
- 4-digit 7seg panel, no sound, no chessboard

================================================================================

Program origin notes by bataais:

They (some SciSys engineers) took the rom of Boris Master (rev. 1) and tried to
rewrite it for a 4x 7-Segment LED, unfortunately they botched the hack job, the
timer interrupt calls the new display routine much more frequently and so loses
time for chess calculations; and more weird stuff is going on.

No wonder it plays so weak.

In the Delta-1 ROM is even some fragmented code remaining of the message:

0878:BORIS AWAITS YOUR MOVE
01 CD 7E 53 09 37 AE 50 (BC FD 59 C0 86 0)

*******************************************************************************/

#include "emu.h"

#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "machine/timer.h"
#include "video/pwm.h"

// internal artwork
#include "saitek_delta1.lh"


namespace {

class delta1_state : public driver_device
{
public:
	delta1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// New Game button is directly tied to F3850 EXT RES pin
	DECLARE_INPUT_CHANGED_MEMBER(reset_button) { m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE); }

	// machine configs
	void delta1(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_ioport_array<5> m_inputs;

	u8 m_mux_data = 0;
	u8 m_led_select = 0;
	u8 m_inp_mux = 0;
	u8 m_7seg_data = 0;
	bool m_7seg_rc = false;
	bool m_blink = false;

	// address maps
	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(blink) { m_blink = !m_blink; update_display(); }

	// I/O handlers
	void update_display();
	void mux_w(u8 data);
	void digit_w(u8 data);
	u8 input_r();
};

void delta1_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_mux_data));
	save_item(NAME(m_led_select));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_7seg_data));
	save_item(NAME(m_7seg_rc));
	save_item(NAME(m_blink));

	// game reads from uninitialized RAM while it's thinking
	for (int i = 0; i < 0x100; i++)
		m_maincpu->space(AS_PROGRAM).write_byte(i + 0x2000, machine().rand());
}


/*******************************************************************************
    I/O
*******************************************************************************/

void delta1_state::update_display()
{
	m_display->matrix(m_led_select, (m_blink && m_7seg_rc) ? 0 : m_7seg_data);
}

void delta1_state::mux_w(u8 data)
{
	// IO00-02: MC14028B A-C (D to GND)
	// MC14028B Q3-Q7: input mux
	// MC14028B Q4-Q7: digit select through 75492
	u8 sel = 1 << (~data & 7);
	m_inp_mux = sel >> 3 & 0x1f;
	m_led_select = sel >> 4;
	update_display();

	m_mux_data = data;
}

u8 delta1_state::input_r()
{
	u8 data = 0;

	// IO04-07: multiplexed inputs
	// IO03: GND
	for (int i = 0; i < 5; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	return data << 4 | m_mux_data | 8;
}

void delta1_state::digit_w(u8 data)
{
	// IO17: R/C circuit to segment commons (for automated blinking)
	// IO10-16: digit segments A-G
	m_7seg_rc = bool(BIT(data, 7));
	m_7seg_data = bitswap<7>(data,0,1,2,3,4,5,6);
	update_display();
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void delta1_state::main_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x0fff).mirror(0x1000).rom(); // _A13
	map(0x2000, 0x20ff).mirror(0x1f00).ram(); // A13
}

void delta1_state::main_io(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(delta1_state::input_r), FUNC(delta1_state::mux_w));
	map(0x01, 0x01).w(FUNC(delta1_state::digit_w));
	map(0x0c, 0x0f).rw("f3853", FUNC(f3853_device::read), FUNC(f3853_device::write));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( delta1 )
	PORT_START("IN.0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Time Set")
	PORT_BIT(0x0d, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_A) PORT_NAME("A 1 / White King")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_B) PORT_NAME("B 2 / White Queen")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_C) PORT_NAME("C 3 / White Rook")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_D) PORT_NAME("D 4")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_E) PORT_NAME("E 5 / White Bishop")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_F) PORT_NAME("F 6 / White Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_G) PORT_NAME("G 7 / White Pawn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_H) PORT_NAME("H 8")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9 / Black King")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0 / Black Queen")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Time Display / Black Rook")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE") // clear entry

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("FP / Black Bishop") // find position
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("EP / Black Knight") // enter position
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Change Color / Black Pawn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")

	PORT_START("RESET") // not on matrix
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_CHANGED_MEMBER(DEVICE_SELF, delta1_state, reset_button, 0) PORT_NAME("New Game")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void delta1_state::delta1(machine_config &config)
{
	// basic machine hardware
	F8(config, m_maincpu, 2'000'000); // LC circuit, measured 2MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &delta1_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &delta1_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("f3853", FUNC(f3853_device::int_acknowledge));

	f3853_device &f3853(F3853(config, "f3853", 2'000'000));
	f3853.int_req_callback().set_inputline("maincpu", F8_INPUT_LINE_INT_REQ);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(4, 7);
	m_display->set_segmask(0xf, 0x7f);
	config.set_default_layout(layout_saitek_delta1);

	TIMER(config, "display_blink").configure_periodic(FUNC(delta1_state::blink), attotime::from_msec(200)); // approximation
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( ccdelta1 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("ma_winke_y1d", 0x0000, 0x1000, CRC(ddc04aca) SHA1(bbf334c82bc89b2f131f5a50f0a617bc3bc4c329) ) // 2332a
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1980, ccdelta1, 0,      0,      delta1,  delta1, delta1_state, empty_init, "SciSys / Novag Industries", "Chess Champion: Delta-1", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
