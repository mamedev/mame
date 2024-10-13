// license:BSD-3-Clause
// copyright-holders:Peter Trauner, Wilbert Pol, hap
// thanks-to:Sean Riddle, Berger
/*******************************************************************************

DCS CompuChess / Novag Chess Champion MK I
Initial driver version by PeT mess@utanet.at 2000,2001.
Driver largely rewritten over the years.

To start playing, enter difficulty level (1-6), then when it says "bP", press A
for new game, B for empty board, or C for continue.

TODO:
- cncchess sound is wrong, it should be a long dual-tone alarm sound

BTANB:
- cmpchess/cmpchess2 accepts illegal moves (Conic fixed that)
- digits may flash briefly after entering a command, eg. the "b" or "P" digit
  after setting board preset, this happens on the real device

================================================================================

DataCash Systems's CompuChess released mid-1977. One of the first chess
computers, the first one being Fidelity Chess Challenger (fidelity/cc1.cpp)

Hardware notes:
- PCB label: CompuChess 1, STAID INC, REV A, 7-77
- Fairchild 3850PK, 2MHz XTAL
- Fairchild 3853PK
- 2KB ROM, 256 bytes RAM(2*D2101AL-4)
- 4-digit 7seg led display, no sound

CompuChess 2nd edition is on the same PCB, but with a 3.57MHz XTAL(!), a 3850PC,
and Mostek MK3853N. The MCU speed was also confirmed with move calculation time.

2nd edition added 3 new game modes: E for "Game of Knights", F for "Amazon Queen",
and G for "Survival".

================================================================================

The game underneath CompuChess is better known as Novag's MK I, it was an
unlicensed clone. The ROM is identical. DCS sued JS&A / Novag Industries for
copyright infringement and somehow didn't manage to win the case.
see: https://law.justia.com/cases/federal/district-courts/FSupp/480/1063/1531498/

Unlike CompuChess, MK I was a large success, we can assume that it kickstarted
Novag's chess computer generation. It was also distributed as "Computer Chess"
by JS&A, in the same housing as MK I.

MCU frequency is with an LC circuit. The first version was around 5% faster than
CompuChess, the JS&A version was measured 2.18MHz on average. The 2nd version was
much faster, judging from move calculation time: around 3.5MHz(give or take 0.2).

1st (slow) version:
- Novag MK I with Boris Spassky photo on the box
- JS&A Computer Chess

2nd (fast) version:
- Novag MK I with Karpov photo on the box, later advertised with message
  "Neu: Mit Quick-Electronic für Schnelle Gegenzüge"
- Waddingtons Videomaster Chess Champion

MK I hardware description:
- An F8 3850 CPU accompanied by a 3853 memory interface
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

================================================================================

Conic Computer Chess (aka Master I in Germany) is also based on DCS CompuChess,
this time the 2nd edition. Conic have done a few changes, not enough to hide that
they 'borrowed' code. They also added a piezo circuit.

Hardware notes:
- Fairchild 3850PK CPU @ 2MHz (LC circuit), 3853PK
- 2KB ROM (3216), 256 bytes RAM (2*2111A)
- discrete sound

"bP" buttons are F, G, H (instead of A, B, C)

*******************************************************************************/

#include "emu.h"
#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "video/pwm.h"
#include "speaker.h"

// internal artwork
#include "cmpchess.lh"
#include "novag_mk1.lh"
#include "conic_cchess.lh"


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
	DECLARE_INPUT_CHANGED_MEMBER(change_cpu_freq);

	// machine configs
	void cmpchess(machine_config &config);
	void cmpchess2(machine_config &config);
	void mk1(machine_config &config);
	void cncchess(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	optional_device<timer_device> m_beeper_off;
	optional_device<beep_device> m_beeper;
	required_ioport_array<4> m_inputs;

	u8 m_inp_mux = 0;
	u8 m_digit_select = 0;
	u8 m_digit_data = 0;
	bool m_blink = false;

	// address maps
	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;
	void cncchess_map(address_map &map) ATTR_COLD;
	void cncchess_io(address_map &map) ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(beeper_off) { m_beeper->set_state(0); }
	TIMER_DEVICE_CALLBACK_MEMBER(blink) { m_blink = !m_blink; update_display(); }
	void update_display();
	void update_reset(ioport_value state);

	// I/O handlers
	void input_w(u8 data);
	u8 input_r();
	void digit_select_w(u8 data);
	void digit_data_w(u8 data);
	u8 digit_data_r();
	u8 beeper_r(offs_t offset);

	void input_digit_select_w(u8 data) { input_w(data); digit_select_w(data); }
	void input_digit_data_w(u8 data) { input_w(data); digit_data_w(data); }
};



/*******************************************************************************
    Initialization
*******************************************************************************/

void cmpchess_state::machine_start()
{
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

INPUT_CHANGED_MEMBER(cmpchess_state::change_cpu_freq)
{
	// 2 MK I versions, 2nd one was a lot faster
	const u32 freq = (newval & 1) ? 3'500'000 : 2'250'000;
	m_maincpu->set_unscaled_clock(freq);
	subdevice<f3853_device>("smi")->set_unscaled_clock(freq);
}



/*******************************************************************************
    I/O
*******************************************************************************/

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

void cmpchess_state::digit_data_w(u8 data)
{
	// digit segment data
	m_digit_data = data;
	update_display();
}

u8 cmpchess_state::digit_data_r()
{
	return m_digit_data;
}

void cmpchess_state::digit_select_w(u8 data)
{
	// d0-d3: digit select (active low)
	m_digit_select = ~data & 0xf;
	update_display();
}

void cmpchess_state::input_w(u8 data)
{
	// input matrix is shared with either digit_data_w, or digit_select_w
	m_inp_mux = data;
}

u8 cmpchess_state::input_r()
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

u8 cmpchess_state::beeper_r(offs_t offset)
{
	// cncchess: trigger beeper
	if (!machine().side_effects_disabled())
	{
		m_beeper->set_state(1);
		m_beeper_off->adjust(attotime::from_msec(50)); // wrong, see TODO
	}

	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void cmpchess_state::main_map(address_map &map)
{
	map.global_mask(0x0fff);
	map(0x0000, 0x07ff).rom();
	map(0x0800, 0x08ff).mirror(0x0700).ram();
}

void cmpchess_state::main_io(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(cmpchess_state::input_r), FUNC(cmpchess_state::input_digit_data_w));
	map(0x01, 0x01).w(FUNC(cmpchess_state::digit_select_w));
	map(0x0c, 0x0f).rw("smi", FUNC(f3853_device::read), FUNC(f3853_device::write));
}

void cmpchess_state::cncchess_map(address_map &map)
{
	map(0x0000, 0x07ff).rom();
	map(0x1800, 0x18ff).ram();
	map(0x8000, 0xffff).r(FUNC(cmpchess_state::beeper_r));
}

void cmpchess_state::cncchess_io(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(cmpchess_state::digit_data_r), FUNC(cmpchess_state::digit_data_w));
	map(0x01, 0x01).rw(FUNC(cmpchess_state::input_r), FUNC(cmpchess_state::input_digit_select_w));
	map(0x0c, 0x0f).rw("smi", FUNC(f3853_device::read), FUNC(f3853_device::write));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

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
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_F1) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, cmpchess_state, reset_switch, 0) PORT_NAME("Reset Switch")
INPUT_PORTS_END

static INPUT_PORTS_START( mk1 )
	PORT_INCLUDE( cmpchess )

	PORT_MODIFY("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_F1) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, cmpchess_state, reset_switch, 0) PORT_NAME("L.S. Switch")

	PORT_START("CPU")
	PORT_CONFNAME( 0x01, 0x00, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, cmpchess_state, change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "2.25MHz (Spassky packaging)" )
	PORT_CONFSETTING(    0x01, "3.5MHz (Karpov packaging)" )
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
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, cmpchess_state, reset_switch, 0) PORT_NAME("Reset")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void cmpchess_state::cmpchess(machine_config &config)
{
	// basic machine hardware
	F8(config, m_maincpu, 2_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &cmpchess_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &cmpchess_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("smi", FUNC(f3853_device::int_acknowledge));

	f3853_device &smi(F3853(config, "smi", 2_MHz_XTAL));
	smi.int_req_callback().set_inputline("maincpu", F8_INPUT_LINE_INT_REQ);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(4, 8+1);
	m_display->set_segmask(0xf, 0xff);
	config.set_default_layout(layout_cmpchess);

	const attotime blink_period = attotime::from_msec(200); // approximation
	TIMER(config, "blink_display").configure_periodic(FUNC(cmpchess_state::blink), blink_period);
}

void cmpchess_state::cmpchess2(machine_config &config)
{
	cmpchess(config);

	// basic machine hardware
	m_maincpu->set_clock(3.579545_MHz_XTAL);
	subdevice<f3853_device>("smi")->set_clock(3.579545_MHz_XTAL);
}

void cmpchess_state::mk1(machine_config &config)
{
	cmpchess(config);

	// basic machine hardware
	m_maincpu->set_clock(2'250'000); // see notes
	subdevice<f3853_device>("smi")->set_clock(2'250'000);

	config.set_default_layout(layout_novag_mk1);
}

void cmpchess_state::cncchess(machine_config &config)
{
	cmpchess2(config);

	// basic machine hardware
	m_maincpu->set_clock(2'000'000); // LC circuit, measured 2MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &cmpchess_state::cncchess_map);
	m_maincpu->set_addrmap(AS_IO, &cmpchess_state::cncchess_io);

	subdevice<f3853_device>("smi")->set_clock(2'000'000);

	config.set_default_layout(layout_conic_cchess);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	BEEP(config, m_beeper, 2000); // wrong, see TODO
	m_beeper->add_route(ALL_OUTPUTS, "speaker", 0.25);
	TIMER(config, "beeper_off").configure_generic(FUNC(cmpchess_state::beeper_off));
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( cmpchess )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("32014-4950_cmcsi_staid.u3", 0x0000, 0x0800, CRC(278f7bf3) SHA1(b384c95ba691d52dfdddd35987a71e9746a46170) )
ROM_END

ROM_START( cmpchess2 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("8316a-4952_cmc2_staid.u3", 0x0000, 0x0800, CRC(aee49c5f) SHA1(8a10f74207f2646164cc0deed81bd082143ac38a) )
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



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT    COMPAT  MACHINE    INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1977, cmpchess,  0,        0,      cmpchess,  cmpchess, cmpchess_state, empty_init, "DataCash Systems / Staid", "CompuChess", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE ) // aka CompuChess I
SYST( 1978, cmpchess2, 0,        0,      cmpchess2, cmpchess, cmpchess_state, empty_init, "DataCash Systems / Staid", "CompuChess: The Second Edition", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )

SYST( 1978, ccmk1,     cmpchess, 0,      mk1,       mk1,      cmpchess_state, empty_init, "bootleg (Novag Industries)", "Chess Champion: MK I", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )

SYST( 1979, cncchess,  0,        0,      cncchess,  cncchess, cmpchess_state, empty_init, "Conic", "Computer Chess (Conic, model 7011)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
