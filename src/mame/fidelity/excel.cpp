// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger, yoyo_chessboard
/*******************************************************************************

Fidelity Excellence series hardware
(for Excel 68000, see eag68k.cpp)

TODO:
- verify if Designer 2100 CPU is 5MHz or 6MHz
- granits chessboard buttons seem too sensitive (detects input on falling edge if
  held too long), probably BTANB?

================================================================================

The Excellence (model EP12)
---------------------------

Hardware notes:
- PCB label: 510-1099A01
- GTE G65SC102P-3 @ 3MHz (12MHz XTAL)
- 16KB ROM, 2KB RAM
- piezo, 16 LEDs, 8*8 chessboard buttons

Model 6080 is on the same hardware as EP12, and model 6080B (510.1117A02 PCB) has
more memory (32KB ROM, 8KB RAM).

One interesting clone of The Excellence is the Computerchess Playmate-2. It was
produced in 1989 by SPS(Bulgaria) and RRR(Riga Radio Factory). The chess program
ROM is identical to Excellence EP12 (the 101-1072A01 one). All internal circuitry
is the same, the only difference is the capacitor driving the 555 for IRQ is 10nf
instead of 22nf.

What makes it unique is the addition of a chess clock.

connector pinout from main pcb:
1) 5V
2) GND
3) 74HC259.pin9 (Q4) = Row LED driving
4) 74HC259.pin10 (Q5) = Column LED driving
5) 74HC259.pin12 (Q7) = Bat. Low signal
6) 74HC42.pin4 (Q3) = Col-D/Row-4 -> 'White Move' if D-LED blinks

The extra board has a 7474, a К1016ХЛ1 (RTC, or MCU clock driver), a 4-digit VFD,
and some buttons for controlling the clock. IRQ frequency is doubled presumedly
for using the blinking led as seconds counter. It only tracks player time, not
of the opponent. And it obviously doesn't show chessmove coordinates either.

================================================================================

Par Excellence family
---------------------

Hardware notes:

The Par Excellence (model 6083):
- PCB label: 510-1099A01
- Rockwell R65C02P4, 5MHz XTAL
- 8KB RAM(HM6264AP-10), 32KB ROM(AMI 101.1077A01)
- piezo, 16 LEDs, 8*8 chessboard buttons

Designer 2000 (model 6102):
- PCB label 510.1129A01
- Ricoh RP65C02G, 3MHz XTAL
- rest is same as Par Excellence

Designer 2000 is basically same as (Par) Excellence hardware, reskinned board.
They removed low-voltage detection from the PCB, but low battery signal still
works in software. Designer 2100 (model 6103) is almost the same, with a faster
CPU (5MHz WDC 65C02, same frequency as Par Excellence).

(Designer 1500 is on 80C50 hardware, same ROM as The Classic, see sc6.cpp)

RCS Granit S:
- PCB label: 510-1084B01 (SC12B PCB)
- Rockwell R65C02P4, 8MHz XTAL, ~6.4V overvoltage
- SC12B module slot is not functional
- rest is same as Par Excellence

Granit S is a heavily modified SC12B (see sc12.cpp) by Peter Reckwitz. The CPU
is overclocked a lot, the dynamic clock divider was removed, and wire mods were
added to make the hardware similar to Par Excellence. The program is based on
Par Excellence.

The SC12B labels don't make much sense here, like the ones on the bottom left,
or the check and illegal status LEDs. The PB button is Verify, and the PV button
is for Options. A strip was added at the bottom for the status LED labels.

================================================================================

Voice Excellence (model 6092)
-----------------------------

Hardware notes:

Main PCB:
- PCB label: 510.1117A02, appears to be identical to The Excellence 6080B PCB
- GTE G65SC102P-3 @ 3MHz (12MHz XTAL)
- 32 KB PRG ROM: AMI 101-1080A01(IC5), 8192x8 SRAM SRM2264C10(IC6)
- speaker, 16 LEDs, 8*8 chessboard buttons

Voice PCB:
- PCB label: 510.1117A01
- 32 KB ROM: AMI 101-1081A01(IC2)
- TSI S14001A
- DIP Switches set ROM A13 and ROM A14, on the side of the board

ROM A12 is tied to S14001A's A11 (yuck)
ROM A11 is however tied to the CPU's XYZ

0000_07FF - Spanish 1/4
0800_0FFF - Spanish 3/4
1000_17FF - Spanish 2/4
1800_1FFF - Spanish 4/4

2000_27FF - French 1/4
2800_2FFF - French 3/4
3000_3FFF - French 2/4
3800_3FFF - French 4/4

4000_47FF - German 1/4
4800_4FFF - German 3/4
5000_57FF - German 2/4
5800_5FFF - German 4/4

6000_67FF - English 1/2
6800_6FFF - Bridge Challenger 1/2
7000_77FF - English 2/2
7800_7FFF - Bridge Challenger 2/2

------------------
RE info by hap, based on PCB photos

Memory map:
-----------
0000-3FFF: 8K RAM (SRM2264)
4000-7FFF: control (R/W)
8000-FFFF: 32K ROM (M27256 compatible)

control (W):
------------
CPU A0-A2 to 3*74259, CPU Dx to D (_C unused)

CPU D0:
- Q4,Q5: led commons
- Q6,Q7,Q2,Q1: 7seg panel digit select
- Q0-Q3: 7442 A0-A3
  + 0-7: led data
  + 0-8: keypad mux
  + 9: buzzer out

CPU D1: (model 6093)
- Q0-Q7: 7seg data

CPU D2: (model 6092)
- Q0-Q5: TSI C0-C5
- Q6: TSI START pin
- Q7: TSI ROM A11

A11 from TSI is tied to TSI ROM A12(!)
TSI ROM A13,A14 are hardwired to the 2 language switches.
Sound comes from the Audio out pin, digital out pins are N/C.

control (R):
------------
CPU A0-A2 to 2*74251, CPU Dx to output

CPU D7 to Y:
- D0-D7: keypad row data

CPU D6 to W: (model 6092, tied to VCC otherwise)
- D0,D1: language switches
- D2-D6: VCC
- D7: TSI BUSY

*******************************************************************************/

#include "emu.h"

#include "cpu/m6502/r65c02.h"
#include "cpu/m6502/m65sc02.h"
#include "machine/clock.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "sound/s14001a.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "fidel_ex.lh"
#include "fidel_exb.lh"
#include "fidel_exd.lh"
#include "fidel_exv.lh"
#include "granits.lh"


namespace {

class excel_state : public driver_device
{
public:
	excel_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_speech(*this, "speech"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void init_fexcelv();
	void init_granits() { m_invert = 0xff; }

	// machine configs
	void fexcel(machine_config &config);
	void fexcelb(machine_config &config);
	void fexcel4(machine_config &config);
	void fexceld(machine_config &config);
	void fexcelv(machine_config &config);
	void fexcelp(machine_config &config);
	void granits(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(speech_bankswitch);
	DECLARE_INPUT_CHANGED_MEMBER(fexcelp_change_cpu_freq);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	optional_device<s14001a_device> m_speech;
	optional_ioport_array<3> m_inputs;

	u8 m_invert = 0;
	u8 m_select = 0;
	u8 m_7seg_data = 0;
	u8 m_speech_data = 0;
	u8 m_speech_bank = 0;

	// address maps
	void fexcel_map(address_map &map) ATTR_COLD;
	void fexcelb_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void speech_w(u8 data, u8 mask);
	void ttl_w(offs_t offset, u8 data);
	u8 ttl_r(offs_t offset);
};

void excel_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_select));
	save_item(NAME(m_7seg_data));
	save_item(NAME(m_speech_data));
	save_item(NAME(m_speech_bank));
}

INPUT_CHANGED_MEMBER(excel_state::fexcelp_change_cpu_freq)
{
	m_maincpu->set_unscaled_clock((newval & 1) ? 5_MHz_XTAL : 3_MHz_XTAL);
}



/*******************************************************************************
    I/O
*******************************************************************************/

// TTL

void excel_state::ttl_w(offs_t offset, u8 data)
{
	// a0-a2,d0: 74259(1)
	u8 mask = 1 << offset;
	m_select = (m_select & ~mask) | ((data & 1) ? mask : 0);

	// 74259 Q0-Q3: 7442 a0-a3
	// 7442 0-8: led data, input mux
	u16 sel = 1 << (m_select & 0xf) & 0x3ff;
	u8 led_data = sel & 0xff;

	// 7442 9: speaker out
	m_dac->write(BIT(sel, 9));

	// 74259 Q4-Q7,Q2,Q1: digit/led select (active low)
	u8 led_sel = ~bitswap<8>(m_select,0,3,1,2,7,6,5,4) & 0x3f;

	// a0-a2,d1: digit segment data (model 6093)
	m_7seg_data = (m_7seg_data & ~mask) | ((data & 2) ? mask : 0);
	u8 seg_data = bitswap<8>(m_7seg_data,0,1,3,2,7,5,6,4);

	// update display: 4 7seg leds, 2*8 chessboard leds
	m_display->matrix_partial(0, 2, led_sel, led_data);
	m_display->matrix_partial(2, 4, led_sel >> 2, seg_data); // 6093

	// speech (model 6092)
	if (m_speech != nullptr)
		speech_w(data, mask);
}

u8 excel_state::ttl_r(offs_t offset)
{
	u8 sel = m_select & 0xf;
	u8 d7 = 0x80;
	u8 data = 0;

	// 74259(1) Q7 + 74251 I0: battery status
	if (m_inputs[2] != nullptr && sel == 0 && ~m_select & 0x80)
		d7 = m_inputs[2]->read() & 0x80;

	// a0-a2,d6: from speech board: language switches and S14001A busy pin, otherwise tied to VCC
	u8 d6 = (m_inputs[1].read_safe(0xff) >> offset & 1) ? 0x40 : 0;

	// a0-a2,d7: multiplexed inputs (active low)
	// read chessboard sensors
	if (sel < 8)
		data = m_board->read_file(sel) ^ m_invert;

	// read button panel
	else if (sel == 8)
		data = m_inputs[0]->read();

	return ((data >> offset & 1) ? 0 : d7) | d6 | 0x3f;
}


// speech (fexcelv)

void excel_state::init_fexcelv()
{
	u8 *rom = memregion("speech")->base();
	const u32 len = memregion("speech")->bytes();
	assert(len == 0x8000);

	// program controls A11, user controls A13,A14(language switches)
	std::vector<u8> buf(len);
	memcpy(&buf[0], rom, len);
	for (int i = 0; i < len; i++)
		rom[i] = buf[((i & 0x67ff) | bitswap<2>(i,11,12) << 11) ^ 0x6000];
}

INPUT_CHANGED_MEMBER(excel_state::speech_bankswitch)
{
	// tied to speech ROM highest bits
	m_speech_bank = (m_speech_bank & 1) | (newval << 1 & 6);
	m_speech->set_rom_bank(m_speech_bank);
}

void excel_state::speech_w(u8 data, u8 mask)
{
	// a0-a2,d2 (from ttl_w): 74259(2) to speech board
	m_speech_data = (m_speech_data & ~mask) | ((data & 4) ? mask : 0);

	// 74259 Q6: speech ROM A11
	m_speech_bank = (m_speech_bank & ~1) | BIT(m_speech_data, 6);
	m_speech->set_rom_bank(m_speech_bank);

	// Q0-Q5: S14001A C0-C5
	// Q7: S14001A start pin
	m_speech->data_w(m_speech_data & 0x3f);
	m_speech->start_w(BIT(m_speech_data, 7));
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void excel_state::fexcel_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x3800).ram();
	map(0x4000, 0x4007).mirror(0x3ff8).rw(FUNC(excel_state::ttl_r), FUNC(excel_state::ttl_w));
	//map(0x8000, 0x8000).nopr(); // checks for opening book module, but hw doesn't have a module slot
	map(0xc000, 0xffff).rom();
}

void excel_state::fexcelb_map(address_map &map)
{
	map(0x0000, 0x1fff).mirror(0x2000).ram();
	map(0x4000, 0x4007).mirror(0x3ff8).rw(FUNC(excel_state::ttl_r), FUNC(excel_state::ttl_w));
	map(0x8000, 0xffff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( fexcelb )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Clear")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Move / Pawn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Hint / Knight")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Take Back / Bishop")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Level / Rook")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Options / Queen")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Verify / King")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
INPUT_PORTS_END

static INPUT_PORTS_START( fexcelv )
	PORT_INCLUDE( fexcelb )

	PORT_START("IN.1")
	PORT_CONFNAME( 0x03, 0x00, DEF_STR( Language ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, excel_state, speech_bankswitch, 0)
	PORT_CONFSETTING(    0x00, DEF_STR( English ) )
	PORT_CONFSETTING(    0x01, DEF_STR( German ) )
	PORT_CONFSETTING(    0x02, DEF_STR( French ) )
	PORT_CONFSETTING(    0x03, DEF_STR( Spanish ) )
	PORT_BIT(0x7c, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("speech", s14001a_device, busy_r)
INPUT_PORTS_END

static INPUT_PORTS_START( fexcel )
	PORT_INCLUDE( fexcelb )

	PORT_START("IN.2")
	PORT_CONFNAME( 0x80, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x80, "Low" )
	PORT_CONFSETTING(    0x00, DEF_STR( Normal ) )
INPUT_PORTS_END

static INPUT_PORTS_START( fexcelp )
	PORT_INCLUDE( fexcel )

	PORT_START("CPU")
	PORT_CONFNAME( 0x01, 0x01, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, excel_state, fexcelp_change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "3MHz (Designer 2000)" )
	PORT_CONFSETTING(    0x01, "5MHz (Par Excellence, Designer 2100)" )
INPUT_PORTS_END

static INPUT_PORTS_START( granits )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CL")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("RV / Pawn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("DM / Knight")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("TB / Bishop")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("LV / Rook")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("PV / Queen") // options
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("PB / King") // verify
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_N) PORT_NAME("RE")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void excel_state::fexcel(machine_config &config)
{
	// basic machine hardware
	M65SC02(config, m_maincpu, 12_MHz_XTAL/4); // G65SC102P-3, 12.0M ceramic resonator
	m_maincpu->set_addrmap(AS_PROGRAM, &excel_state::fexcel_map);

	auto &irq_clock(CLOCK(config, "irq_clock", 600)); // from 556 timer (22nF, 102K, 1K), ideal frequency is 600Hz
	irq_clock.set_pulse_width(attotime::from_nsec(15250)); // active for 15.25us
	irq_clock.signal_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(100));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(2+4, 8);
	m_display->set_segmask(0x3c, 0x7f);
	config.set_default_layout(layout_fidel_ex);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void excel_state::fexcel4(machine_config &config)
{
	fexcel(config);

	// basic machine hardware
	R65C02(config.replace(), m_maincpu, 4_MHz_XTAL); // R65C02P4
	m_maincpu->set_addrmap(AS_PROGRAM, &excel_state::fexcel_map);
}

void excel_state::fexcelb(machine_config &config)
{
	fexcel(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &excel_state::fexcelb_map);
	config.set_default_layout(layout_fidel_exb);
}

void excel_state::fexceld(machine_config &config)
{
	fexcelb(config);

	config.set_default_layout(layout_fidel_exd);
}

void excel_state::fexcelv(machine_config &config)
{
	fexcelb(config);

	config.set_default_layout(layout_fidel_exv);

	// sound hardware
	S14001A(config, m_speech, 25000); // R/C circuit, around 25khz
	m_speech->add_route(ALL_OUTPUTS, "speaker", 0.75);
}

void excel_state::fexcelp(machine_config &config)
{
	fexcel(config);

	// basic machine hardware
	R65C02(config.replace(), m_maincpu, 5_MHz_XTAL); // R65C02P4
	m_maincpu->set_addrmap(AS_PROGRAM, &excel_state::fexcelb_map);
}

void excel_state::granits(machine_config &config)
{
	fexcelp(config);

	// basic machine hardware
	m_maincpu->set_clock(8_MHz_XTAL); // R65C02P4
	config.set_default_layout(layout_granits);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( fexcel ) // model 6080(B), PCB label 510.1117A02
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1080a01.ic5", 0x8000, 0x8000, CRC(846f8e40) SHA1(4e1d5b08d5ff3422192b54fa82cb3f505a69a971) )
ROM_END

ROM_START( fexceld ) // model 6093, PCB label 510.1117A02
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1080a01.ic5", 0x8000, 0x8000, CRC(846f8e40) SHA1(4e1d5b08d5ff3422192b54fa82cb3f505a69a971) ) // same rom as fexcel
ROM_END

ROM_START( fexcelv ) // model 6092, PCB label 510.1117A02, sound PCB 510.1117A01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1080a01.ic5", 0x8000, 0x8000, CRC(846f8e40) SHA1(4e1d5b08d5ff3422192b54fa82cb3f505a69a971) ) // PCB1, M27256, same rom as fexcel

	ROM_REGION( 0x8000, "speech", 0 )
	ROM_LOAD("101-1081a01.ic2", 0x0000, 0x8000, CRC(c8ae1607) SHA1(6491ce6be60ed77f3dd931c0ca17616f13af943e) ) // PCB2, M27256
ROM_END

ROM_START( fexcel12 ) // model EP12, PCB label 510-1099A01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1073a01.ic5", 0xc000, 0x4000, CRC(3e221534) SHA1(7516bc6a8aab9d8ac30ac1a9317630a6aa9ac1a0) )
ROM_END

ROM_START( fexcel12a ) // model EP12, PCB label 510-1099A01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1072a01.ic5", 0xc000, 0x4000, CRC(212b006d) SHA1(242ff851b0841cbec66bbada6a730da021010e2c) )
ROM_END

ROM_START( fexcela ) // model 6080, PCB label 510-1099A01(manuf.1985) or 510-1099B01(manuf.1986)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1072b01.ic5", 0xc000, 0x4000, CRC(fd2f6064) SHA1(f84bb98bdb9565a04891eb6820597d7aecc90c21) ) // RCA
ROM_END


ROM_START( fexcelp ) // model 6083, PCB label 510-1099A01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1077a01.ic5", 0x8000, 0x8000, CRC(62006320) SHA1(1d6370973dbae42c54639b261cc81e32cdfc1d5d) ) // only 1 byte difference, assume bugfix in bookrom
ROM_END

ROM_START( fexcelpa ) // model 6083, PCB label 510-1099B01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("white.ic5", 0x8000, 0x8000, CRC(0d17b0f0) SHA1(3a6070fd4718c62b62ff0f08637bb6eb84eb9a1c) ) // GI 27C256, no label
ROM_END


ROM_START( granits ) // modified SC12 board
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("granit_s-4.ic15", 0x8000, 0x8000, CRC(274d6aff) SHA1(c8d943b2f15422ac62f539b568f5509cbce568a3) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT   COMPAT  MACHINE   INPUT    CLASS        INIT          COMPANY, FULLNAME, FLAGS
SYST( 1987, fexcel,    0,       0,      fexcelb,  fexcelb, excel_state, empty_init,   "Fidelity International", "The Excellence (model 6080B)", MACHINE_SUPPORTS_SAVE )
SYST( 1987, fexceld,   fexcel,  0,      fexceld,  fexcelb, excel_state, empty_init,   "Fidelity International", "Excel Display", MACHINE_SUPPORTS_SAVE )
SYST( 1987, fexcelv,   fexcel,  0,      fexcelv,  fexcelv, excel_state, init_fexcelv, "Fidelity International", "Voice Excellence", MACHINE_SUPPORTS_SAVE )
SYST( 1985, fexcel12,  fexcel,  0,      fexcel4,  fexcel,  excel_state, empty_init,   "Fidelity International", "The Excellence (model EP12, set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1985, fexcel12a, fexcel,  0,      fexcel,   fexcel,  excel_state, empty_init,   "Fidelity International", "The Excellence (model EP12, set 2)", MACHINE_SUPPORTS_SAVE ) // 1st version of The Excellence
SYST( 1985, fexcela,   fexcel,  0,      fexcel,   fexcel,  excel_state, empty_init,   "Fidelity International", "The Excellence (model 6080)", MACHINE_SUPPORTS_SAVE )

SYST( 1986, fexcelp,   0,       0,      fexcelp,  fexcelp, excel_state, empty_init,   "Fidelity International", "The Par Excellence (set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1986, fexcelpa,  fexcelp, 0,      fexcelp,  fexcelp, excel_state, empty_init,   "Fidelity International", "The Par Excellence (set 2)", MACHINE_SUPPORTS_SAVE )

SYST( 1986, granits,   fscc12,  0,      granits,  granits, excel_state, init_granits, "hack (Remote Control Systems)", "Granit S", MACHINE_SUPPORTS_SAVE )
