// license:BSD-3-Clause
// copyright-holders:Peter Trauner, Wilbert Pol, hap
// thanks-to:Sean Riddle
/******************************************************************************

DCS CompuChess / Novag Chess Champion MK I
Initial version by PeT mess@utanet.at 2000,2001.
Driver largely rewritten over the years.

TODO:
- cncchess sound is wrong, it should be a long dual-tone alarm sound
- verify CPU speed (cmpchess XTAL is not 100% certain)
- dump/add cmpchess2

BTANB:
- digits may flash briefly after entering a command, eg. the "b" or "P" digit
  after setting board preset, this happens on the real device

*******************************************************************************

Data Cash Systems's CompuChess (1977, one of the first chess computers).
The game underneath is better known as Novag's MK I, it was an unlicensed clone.
The ROM is identical. DCS sued JS&A / Novag Industries for copyright infringement
and somehow didn't manage to win the case.

Unlike CompuChess, MK I was a large success, we can assume that it kickstarted
Novag's chess computer generation. It was also distributed as "Computer Chess"
by JS&A, in the same casing as MK I.

To start playing, enter difficulty level (1-6), then when it says "bP", press A
for new game, B for empty board, or C for continue.

MK I hardware description:
- An F8 3850 CPU accompanied by a 3853 memory interface, approx 2MHz
  Variations seen:
  - MOSTEK MK 3853N 7915 Philippines (static memory interface for F8)
  - MOSTEK MK 3850N-3 7917 Philipines (Fairchild F8 CPU)
  - 3850PK F7901 SINGAPORE (Fairchild F8 CPU)
  - 3853PK F7851 SINGAPORE (static memory interface for F8)
- 2KB 2316 compatible ROM
  Variations seen:
  - Signetics 7916E C48091 82S210-1 COPYRIGHT
  - RO-3-8316A 8316A-4480 7904 TAIWAN
- 2 x 2111 256x4 SRAM to provide 256 bytes of RAM
  Variations seen:
  - AM9111 BPC / P2111A-4 7851
- 16 keys placed in a 4 x 4 matrix
- Power on switch
- L/S switch. This switch is directly tied to the RESET pin of the F8 CPU.
  This allows the user to reset the CPU without destroying the RAM contents.
- A 4 character 11 segment digit display using a 15 pin interface. Of the 15
  pins 3 pins are not connected, so three segments are never used and this
  leaves a standard 7 segments display with a dot in the lower right.
- The digit display is driven by two other components:
  - SN75492N MALAYSIA 7840B
  - ULN2033A 7847
- Hardware addressing is controlled by a HBF4001AE.
- No speaker.

*******************************************************************************

Conic Computer Chess (aka Master I in Germany) is also based on DCS CompuChess,
probably the 2nd edition. Conic have done a few changes, not enough to hide that
they 'borrowed' code from CompuChess. They also added a piezo circuit.

Fairchild 3850PK CPU @ 2MHz (LC circuit), 3853PK
2KB ROM (3216), 256 bytes RAM (2*2111A)
555, 4001, 4011 for display blinking and beeper

"bP" buttons are F, G, H (instead of A, B, C)

******************************************************************************/

#include "emu.h"
#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "video/pwm.h"
#include "speaker.h"

// internal artwork
#include "cmpchess.lh" // clickable
#include "novag_mk1.lh" // clickable
#include "cncchess.lh" // clickable


namespace {

class cmpchess_state : public driver_device
{
public:
	cmpchess_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_beeper_off(*this, "beeper_off"),
		m_beeper(*this, "beeper"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(reset_switch) { update_reset(newval); }

	// machine drivers
	void cmpchess(machine_config &config);
	void mk1(machine_config &config);
	void cnc(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	optional_device<timer_device> m_beeper_off;
	optional_device<beep_device> m_beeper;
	required_ioport_array<4> m_inputs;

	// address maps
	void main_map(address_map &map);
	void main_io(address_map &map);
	void cnc_io(address_map &map);

	TIMER_DEVICE_CALLBACK_MEMBER(beeper_off) { m_beeper->set_state(0); }
	TIMER_DEVICE_CALLBACK_MEMBER(blink) { m_blink = !m_blink; update_display(); }
	void update_display();
	void update_reset(ioport_value state);

	// I/O handlers
	DECLARE_READ8_MEMBER(beeper_r);
	DECLARE_WRITE8_MEMBER(input_w);
	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE8_MEMBER(digit_select_w);
	DECLARE_WRITE8_MEMBER(digit_data_w);
	DECLARE_READ8_MEMBER(digit_data_r);

	DECLARE_WRITE8_MEMBER(input_digit_select_w) { input_w(space, offset, data); digit_select_w(space, offset, data); }
	DECLARE_WRITE8_MEMBER(input_digit_data_w) { input_w(space, offset, data); digit_data_w(space, offset, data); }

	u8 m_inp_mux;
	u8 m_digit_select;
	u8 m_digit_data;
	bool m_blink;
};

void cmpchess_state::machine_start()
{
	// zerofill
	m_inp_mux = 0;
	m_digit_select = 0;
	m_digit_data = 0;
	m_blink = false;

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_digit_select));
	save_item(NAME(m_digit_data));
	save_item(NAME(m_blink));
}

void cmpchess_state::machine_reset()
{
	update_reset(ioport("RESET")->read());
}

void cmpchess_state::update_reset(ioport_value state)
{
	// reset switch is tied to F3850 RESET pin
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);

	// clear display
	if (state)
		m_display->clear();
}



/******************************************************************************
    I/O
******************************************************************************/

READ8_MEMBER(cmpchess_state::beeper_r)
{
	// cncchess: trigger beeper
	if (!machine().side_effects_disabled() && m_beeper != nullptr)
	{
		m_beeper->set_state(1);
		m_beeper_off->adjust(attotime::from_msec(50)); // wrong, see TODO
	}

	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

void cmpchess_state::update_display()
{
	// display panel goes into automated blink mode if DP segment is held high,
	// and DP segment itself by default only appears to be active if no other segments are
	u8 dmask = (m_digit_data == 1) ? 0x80 : 0x7f;
	u8 bmask = (m_blink && m_digit_data & 1) ? 0 : 0xff;
	u8 bstate = m_digit_data & bmask & 1; // DP state for ignoring dmask

	u8 digit_data = bitswap<8>(m_digit_data,0,2,1,3,4,5,6,7) & dmask & bmask;
	m_display->matrix(m_digit_select, bstate << 8 | digit_data);
}

WRITE8_MEMBER(cmpchess_state::digit_data_w)
{
	// digit segment data
	m_digit_data = data;
	update_display();
}

READ8_MEMBER(cmpchess_state::digit_data_r)
{
	return m_digit_data;
}

WRITE8_MEMBER(cmpchess_state::digit_select_w)
{
	// d0-d3: digit select (active low)
	m_digit_select = ~data & 0xf;
	update_display();
}

WRITE8_MEMBER(cmpchess_state::input_w)
{
	// input matrix is shared with either digit_data_w, or digit_select_w
	m_inp_mux = data;
}

READ8_MEMBER(cmpchess_state::input_r)
{
	u8 data = m_inp_mux;

	// d0-d3: multiplexed inputs from d4-d7
	for (int i = 0; i < 4; i++)
		if (m_inp_mux & m_inputs[i]->read() << 4)
			data |= 1 << i;

	// d4-d7: multiplexed inputs from d0-d3
	for (int i = 0; i < 4; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read() << 4;

	return data;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void cmpchess_state::main_map(address_map &map)
{
	map(0x0000, 0x07ff).rom();
	map(0x1800, 0x18ff).ram();
	map(0x8000, 0xffff).r(FUNC(cmpchess_state::beeper_r));
}

void cmpchess_state::main_io(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(cmpchess_state::input_r), FUNC(cmpchess_state::input_digit_data_w));
	map(0x01, 0x01).w(FUNC(cmpchess_state::digit_select_w));
	map(0x0c, 0x0f).rw("smi", FUNC(f3853_device::read), FUNC(f3853_device::write));
}

void cmpchess_state::cnc_io(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(cmpchess_state::digit_data_r), FUNC(cmpchess_state::digit_data_w));
	map(0x01, 0x01).rw(FUNC(cmpchess_state::input_r), FUNC(cmpchess_state::input_digit_select_w));
	map(0x0c, 0x0f).rw("smi", FUNC(f3853_device::read), FUNC(f3853_device::write));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( cmpchess )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("D / Play")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("C / White Bishop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("B / White Queen")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("A / White King")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_M) PORT_NAME("H / md") // more data
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("G / White Pawn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("F / White Rook")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("E / White Knight")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4 / fp") // find piece(position)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3 / Black Bishop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2 / Black Queen")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1 / Black King")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8 / ep") // enter piece(position)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7 / Black Pawn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6 / Black Rook")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5 / Black Knight")

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_F1) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, cmpchess_state, reset_switch, nullptr) PORT_NAME("Reset Switch") // L.S. switch on the MK I
INPUT_PORTS_END

static INPUT_PORTS_START( cncchess )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4 / Black Bishop")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3 / Black Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2 / Black Rook")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1 / Black Pawn")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_I) PORT_NAME("8 / IP") // insert piece (same as ep)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_S) PORT_NAME("7 / SP") // search piece (same as fp)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6 / Black King")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5 / Black Queen")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("D / White Bishop")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("C / White Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("B / White Rook")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("A / White Pawn")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("H / GO")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_M) PORT_NAME("G / MD") // more data
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("F / White King")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("E / White Queen")

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, cmpchess_state, reset_switch, nullptr) PORT_NAME("Reset")
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void cmpchess_state::cmpchess(machine_config &config)
{
	/* basic machine hardware */
	F8(config, m_maincpu, 3.579545_MHz_XTAL/2); // Fairchild 3850PK
	m_maincpu->set_addrmap(AS_PROGRAM, &cmpchess_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &cmpchess_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("smi", FUNC(f3853_device::int_acknowledge));

	f3853_device &smi(F3853(config, "smi", 3.579545_MHz_XTAL/2));
	smi.int_req_callback().set_inputline("maincpu", F8_INPUT_LINE_INT_REQ);

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(4, 8+1);
	m_display->set_segmask(0xf, 0xff);
	config.set_default_layout(layout_cmpchess);

	TIMER(config, "blink_display").configure_periodic(FUNC(cmpchess_state::blink), attotime::from_msec(200)); // approximation
}

void cmpchess_state::mk1(machine_config &config)
{
	cmpchess(config);

	/* basic machine hardware */
	m_maincpu->set_clock(2000000); // JS&A version measured 2.18MHz on average
	subdevice<f3853_device>("smi")->set_clock(2000000);

	config.set_default_layout(layout_novag_mk1);
}

void cmpchess_state::cnc(machine_config &config)
{
	mk1(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &cmpchess_state::cnc_io);

	config.set_default_layout(layout_cncchess);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	BEEP(config, m_beeper, 2000); // wrong, see TODO
	m_beeper->add_route(ALL_OUTPUTS, "speaker", 0.25);
	TIMER(config, "beeper_off").configure_generic(FUNC(cmpchess_state::beeper_off));
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( cmpchess )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("32014-4950", 0x0000, 0x0800, CRC(278f7bf3) SHA1(b384c95ba691d52dfdddd35987a71e9746a46170) )
ROM_END

ROM_START( ccmk1 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("82c210-1", 0x0000, 0x0800, CRC(278f7bf3) SHA1(b384c95ba691d52dfdddd35987a71e9746a46170) )
ROM_END


ROM_START( cncchess )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("c48170.u1", 0x0000, 0x0800, CRC(5beace32) SHA1(9df924037614831a86b73eb3a16bbc80c63257a2) ) // 2316B
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT   CMP MACHINE   INPUT     STATE           INIT        COMPANY, FULLNAME, FLAGS
CONS( 1977, cmpchess, 0,        0, cmpchess, cmpchess, cmpchess_state, empty_init, "Data Cash Systems / Staid", "CompuChess", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1978, ccmk1,    cmpchess, 0, mk1,      cmpchess, cmpchess_state, empty_init, "Novag", "Chess Champion: MK I", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1979, cncchess, 0,        0, cnc,      cncchess, cmpchess_state, empty_init, "Conic", "Computer Chess (Conic)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
