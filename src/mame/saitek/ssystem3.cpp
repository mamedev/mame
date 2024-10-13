// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

SciSys / Novag Chess Champion: Super System III (aka MK III)

It was distributed by both SciSys and Novag. Which company was responsible for
which part of the production chain is unknown. The copyright was assigned to SciSys
(no mention of Novag in the ROM, it has "COPYRIGHT SCISYS LTD 1979").

This is their 1st original product. MK II was licensed from Peter Jennings, and
MK I was, to put it bluntly, a bootleg. The chess engine is by Mike Johnson, with
support from David Levy, Philidor Software.

Hardware notes:

Master Unit:
- PCB label: 201041 (Rev.A to Rev.E)
- Synertek 6502A @ 2MHz (4MHz XTAL)
- Synertek 6522 VIA
- 8KB ROM (2*Synertek 2332)
- 1KB RAM (2*HM472114P-3)
- MD4332BE or HLCD0438 + a bunch of TTL for the LCD
- 13 buttons, 4 switches, no leds or sensorboard
- connectors for: PSU, Power Pack, Chess Unit, Printer Unit

Chess Unit:
- PCB label: Radofin XM-2057-0C
- Fairchild F6808P CPU @ ~6.8MHz (M6808 compatible)
- Fairchild F6821P PIA
- 2KB ROM(2316), 128x8 RAM(F6810P)
- 2*HLCD0438, chessboard LCD

Printer Unit:
- unknown hardware, assume own CPU like the chess unit

PSU ("permanent storage unit"?) is just a 256x4 battery-backed RAM (TC5501P)
module, not sure why it was so expensive (~180DM).

A chess clock accessory was also announced but unreleased.

SciSys Super System IV (AKA MK IV) is on similar hardware. It was supposed to
be a modular chesscomputer, not only with accessory hardware like MK III, but
also a module slot for the program. The box mentions other modules, such as a
reversi program called "The Moor". The chesscomputer was discontinued soon after
release, and none of the accessories or other games came out.

TODO:
- 2nd 7474 /2 clock divider on each 4000-7fff access, this also applies to 6522 clock
  (doesn't affect chess calculation speed, only I/O access, eg. beeper pitch).
  Should be doable to add, but 6522 device doesn't support live clock changes.
- LCD TC pin? connects to the display, source is a 50hz timer(from power supply),
  probably to keep refreshing the LCD when inactive, there is no need to emulate it
- dump/add printer unit
- dump/add other ssystem3 program revisions, were the BTANB fixed in the 1980 version?
  known undumped: C19081 + C19082 (instead of C19081E), C45000 + C45012
- ssystem4 softwarelist if a prototype cartridge is ever dumped

BTANB (ssystem3):
- If the TIME switch is held up, it will sometimes recognize the wrong input when
  another button is pressed. I assume they noticed this bug too late and tried to
  lessen the chance by adding a spring to the switch.
- Similar to the TIME switch bug, pressing 2 buttons simultaneously can cause it
  to malfunction, eg. press A+CE or C+CE and an "8" appears in the display.
- chess unit screen briefly flickers at power-on and when the subcpu receives an
  NMI in the middle of updating the LCD, it is mentioned in the manual

*******************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "cpu/m6800/m6800.h"
#include "machine/6522via.h"
#include "machine/6821pia.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "video/hlcd0438.h"
#include "video/md4330b.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "saitek_ssystem3.lh"
#include "saitek_ssystem4.lh"


namespace {

class ssystem3_state : public driver_device
{
public:
	ssystem3_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_via(*this, "via"),
		m_pia(*this, "pia"),
		m_lcd1(*this, "lcd1"),
		m_lcd2(*this, "lcd2_%u", 0),
		m_display(*this, "display%u", 0),
		m_dac(*this, "dac"),
		m_nvram(*this, "nvram"),
		m_inputs(*this, "IN.%u", 0),
		m_out_lcd2(*this, "s%u.%u", 0U, 0U)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(cu_plug);

	// machine configs
	void ssystem3(machine_config &config);
	void ssystem4(machine_config &config);

	void init_ssystem3() { m_xor_kludge = true; }

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;
	required_device<via6522_device> m_via;
	optional_device<pia6821_device> m_pia;
	required_device<md4332b_device> m_lcd1;
	optional_device_array<hlcd0438_device, 2> m_lcd2;
	optional_device_array<pwm_display_device, 2> m_display;
	required_device<dac_1bit_device> m_dac;
	optional_shared_ptr<u8> m_nvram;
	optional_ioport_array<4+3> m_inputs;
	output_finder<8, 48> m_out_lcd2;

	u8 m_inp_mux = 0;
	u8 m_control = 0;
	u8 m_shift = 0;
	u32 m_lcd1_data = 0;
	u64 m_lcd2_data = 0;
	u8 m_lcd2_select = 0;

	bool m_xor_kludge = false;

	// address maps
	void ssystem3_map(address_map &map) ATTR_COLD;
	void ssystem4_map(address_map &map) ATTR_COLD;
	void chessunit_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void lcd1_output_w(u32 data) { m_lcd1_data = data; }
	void input_w(u8 data);
	u8 input_r();
	void control_w(u8 data);
	u8 control_r();

	void nvram_w(offs_t offset, u8 data);
	u8 nvram_r(offs_t offset);

	void lcd2_pwm_w(offs_t offset, u8 data);
	void lcd2_update();
	template<int N> void lcd2_output_w(offs_t offset, u32 data);
	void cu_pia_a_w(u8 data);
	u8 cu_pia_a_r();
	void cu_pia_b_w(u8 data);
	u8 cu_pia_b_r();
};

void ssystem3_state::machine_start()
{
	m_out_lcd2.resolve();

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_control));
	save_item(NAME(m_shift));
	save_item(NAME(m_lcd1_data));
	save_item(NAME(m_lcd2_data));
	save_item(NAME(m_lcd2_select));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// Master Unit

void ssystem3_state::input_w(u8 data)
{
	// PA0-PA7: input mux
	m_inp_mux = ~data;
}

u8 ssystem3_state::input_r()
{
	u8 data = m_inp_mux;

	// PA1-PA3: multiplexed inputs from PA4-PA7
	// PA0: blocked by diodes
	for (int i = 0; i < 4; i++)
		if (BIT(m_inp_mux, i+4))
			data |= m_inputs[i]->read() & 0xe;

	// PA4-PA7: multiplexed inputs from PA0-PA3
	for (int i = 0; i < 4; i++)
		if (m_inp_mux & m_inputs[i]->read())
			data |= 1 << (i+4);

	// PA5-PA7: freq sel from _PA0
	if (~m_inp_mux & 1)
		data |= m_inputs[5]->read() & 0xe0;

	return ~data;
}

void ssystem3_state::control_w(u8 data)
{
	// PB0: speaker out
	m_dac->write(~data & m_inputs[4]->read() & 1);

	// PB1: LCD DI
	// PB2: LCD CLK
	m_lcd1->di_w(BIT(data, 1));
	m_lcd1->clk_w(BIT(data, 2));

	// PB2 also clocks a 4015B
	// DA: LCD DO, DB: Q3A
	if (data & ~m_control & 4)
	{
		m_shift = m_shift << 1 | m_lcd1->do_r();

		// weird TTL maze, I assume it's a hw kludge to fix a bug after the maskroms were already manufactured
		u8 xorval = m_xor_kludge && (BIT(m_shift, 3) & ~(BIT(m_shift, 1) ^ BIT(m_shift, 4)) & ~(BIT(m_lcd1_data, 7) & BIT(m_lcd1_data, 23))) ? 0x12 : 0;

		// update display
		for (int i = 0; i < 4; i++)
			m_display[0]->write_row(i, m_lcd1_data >> (8*i) & 0xff);
		m_display[0]->write_row(4, (m_shift ^ xorval) | 0x100);
	}

	// PB3: device serial out
	if (m_inputs[5]->read() & 2)
		m_pia->ca1_w(BIT(~data, 3));

	// PB7: tied to PB6 (pulse timer 2)
	m_via->write_pb6(BIT(data, 7));

	m_control = data;
}

u8 ssystem3_state::control_r()
{
	u8 data = 0;

	// PB4: device busy (unused on chess unit)
	// PB5: device attached
	if (m_inputs[5]->read() & 2)
		data ^= 0x30;

	return ~data;
}


// PSU

void ssystem3_state::nvram_w(offs_t offset, u8 data)
{
	// nvram is only d0-d3
	if (m_inputs[5]->read() & 1)
		m_nvram[offset] = data & 0x0f;
}

u8 ssystem3_state::nvram_r(offs_t offset)
{
	return (m_inputs[5]->read() & 1) ? (m_nvram[offset] & 0x0f) : 0;
}


// Chess Unit

INPUT_CHANGED_MEMBER(ssystem3_state::cu_plug)
{
	m_subcpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);

	if (newval)
		m_pia->reset();
	else
		m_display[1]->clear();
}

void ssystem3_state::lcd2_pwm_w(offs_t offset, u8 data)
{
	m_out_lcd2[offset & 0x3f][offset >> 6] = data;
}

void ssystem3_state::lcd2_update()
{
	if (m_inputs[5]->read() & 2)
		m_display[1]->matrix(1 << m_lcd2_select, m_lcd2_data);
}

template<int N>
void ssystem3_state::lcd2_output_w(offs_t offset, u32 data)
{
	if (!offset)
		data = 0;

	m_lcd2_data = u64(data) << 32 | m_lcd2_data >> 32;

	if constexpr (N == 1)
		lcd2_update();
}

void ssystem3_state::cu_pia_a_w(u8 data)
{
	// PA0-PA2: CD4051 to LCD column
	m_lcd2_select = data & 7;
	lcd2_update();
}

u8 ssystem3_state::cu_pia_a_r()
{
	// PA7: serial data in
	return ~m_control << 4 & 0x80;
}

void ssystem3_state::cu_pia_b_w(u8 data)
{
	// PB3: LCD LOAD (both)
	// PB4: LCD LCD (both) + CD4051 COM OUT/IN
	// PB6: LCD CLOCK (both)
	// PB7: LCD DATA IN (1st)
	m_lcd2[0]->data_w(BIT(data, 7));

	for (int i = 0; i < 2; i++)
		m_lcd2[i]->clock_w(BIT(data, 6));

	for (int i = 0; i < 2; i++)
		m_lcd2[i]->load_w(BIT(~data, 3));

	for (int i = 0; i < 2; i++)
		m_lcd2[i]->lcd_w(BIT(data, 4));
}

u8 ssystem3_state::cu_pia_b_r()
{
	// PB5: look switch
	return m_inputs[6]->read() << 5 & 0x20;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void ssystem3_state::ssystem3_map(address_map &map)
{
	map(0x0000, 0x03ff).mirror(0x3c00).ram();
	map(0x4000, 0x40ff).mirror(0x1f00).ram().rw(FUNC(ssystem3_state::nvram_r), FUNC(ssystem3_state::nvram_w)).share("nvram");
	map(0x6000, 0x600f).mirror(0x1ff0).m(m_via, FUNC(via6522_device::map));
	map(0x8000, 0x9fff).mirror(0x6000).rom();
}

void ssystem3_state::ssystem4_map(address_map &map)
{
	map(0x0000, 0x03ff).ram();
	map(0x4400, 0x440f).m(m_via, FUNC(via6522_device::map));
	map(0x4800, 0x48ff).noprw(); // no nvram
	map(0xd000, 0xffff).rom();
}

void ssystem3_state::chessunit_map(address_map &map)
{
	map(0x3000, 0x307f).mirror(0x0f80).ram();
	map(0x4000, 0x47ff).mirror(0xb800).rom();
	map(0x8000, 0x8003).mirror(0x3ffc).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( ssystem4 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9 / EP / C.Square")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0 / MD / C.Board")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_F) PORT_NAME("F 6 / Knight / Clock")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_LEFT) PORT_NAME("E 5 / Bishop / Left")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_I) PORT_NAME("CE / Interrupt")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("G 7 / Pawn / Right")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_D) PORT_NAME("D 4 / Rook / #")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_A) PORT_NAME("A 1 / White")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_TOGGLE PORT_CODE(KEYCODE_T) PORT_NAME("Time")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_H) PORT_NAME("H 8 / Black")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_C) PORT_NAME("C 3 / Queen / #50")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_B) PORT_NAME("B 2 / King / FP")

	PORT_START("IN.4") // switches
	PORT_CONFNAME( 0x01, 0x01, "Sound" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_CONFNAME( 0x02, 0x02, "LCD 1 Light" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x02, DEF_STR( On ) )

	PORT_START("IN.5") // accessories/diodes
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( ssystem3 )
	PORT_INCLUDE( ssystem4 )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9 / EP / C SQ")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0 / MD / C Board")

	PORT_MODIFY("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Time") // spring-loaded

	PORT_MODIFY("IN.4") // switches
	PORT_CONFNAME( 0x04, 0x04, "LCD 2 Light" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x04, DEF_STR( On ) )

	PORT_MODIFY("IN.5") // accessories/diodes
	PORT_CONFNAME( 0x01, 0x01, "Memory Unit" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_CONFNAME( 0x02, 0x02, "Chess Unit" ) PORT_CHANGED_MEMBER(DEVICE_SELF, ssystem3_state, cu_plug, 0)
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_CUSTOM)

	PORT_START("IN.6") // chess unit
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_TOGGLE PORT_CODE(KEYCODE_O) PORT_NAME("Look")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void ssystem3_state::ssystem4(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 4_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &ssystem3_state::ssystem4_map);

	MOS6522(config, m_via, 4_MHz_XTAL / 2);
	m_via->writepa_handler().set(FUNC(ssystem3_state::input_w));
	m_via->readpa_handler().set(FUNC(ssystem3_state::input_r));
	m_via->writepb_handler().set(FUNC(ssystem3_state::control_w));
	m_via->readpb_handler().set(FUNC(ssystem3_state::control_r));

	// video hardware
	MD4332B(config, m_lcd1);
	m_lcd1->write_q().set(FUNC(ssystem3_state::lcd1_output_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/2, 729/2);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display[0]).set_size(5, 9);
	m_display[0]->set_bri_levels(0.25);

	config.set_default_layout(layout_saitek_ssystem4);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void ssystem3_state::ssystem3(machine_config &config)
{
	ssystem4(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &ssystem3_state::ssystem3_map);

	M6808(config, m_subcpu, 6'800'000); // LC circuit, measured
	m_subcpu->set_addrmap(AS_PROGRAM, &ssystem3_state::chessunit_map);

	config.set_perfect_quantum(m_maincpu);

	PIA6821(config, m_pia);
	m_pia->irqa_handler().set_inputline(m_subcpu, INPUT_LINE_NMI);
	m_pia->writepa_handler().set(FUNC(ssystem3_state::cu_pia_a_w));
	m_pia->readpa_handler().set(FUNC(ssystem3_state::cu_pia_a_r));
	m_pia->writepb_handler().set(FUNC(ssystem3_state::cu_pia_b_w));
	m_pia->readpb_handler().set(FUNC(ssystem3_state::cu_pia_b_r));
	m_pia->cb2_handler().set(m_pia, FUNC(pia6821_device::cb1_w));
	m_pia->cb2_handler().append(m_pia, FUNC(pia6821_device::ca2_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	HLCD0438(config, m_lcd2[0], 0);
	m_lcd2[0]->write_segs().set(FUNC(ssystem3_state::lcd2_output_w<0>));
	m_lcd2[0]->write_data().set(m_lcd2[1], FUNC(hlcd0438_device::data_w));

	HLCD0438(config, m_lcd2[1], 0);
	m_lcd2[1]->write_segs().set(FUNC(ssystem3_state::lcd2_output_w<1>));

	screen_device &screen(SCREEN(config, "chessunit", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1060/1.5, 1080/1.5);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display[1]).set_size(8, 48);
	m_display[1]->output_x().set(FUNC(ssystem3_state::lcd2_pwm_w));

	m_display[0]->set_segmask(0xf, 0x7f); // 7segs are at expected positions
	config.set_default_layout(layout_saitek_ssystem3);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( ssystem3 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("c19081e_ss-3-lrom.u4", 0x8000, 0x1000, CRC(9ea46ed3) SHA1(34eef85b356efbea6ddac1d1705b104fc8e2731a) ) // 2332
	ROM_LOAD("c19082_ss-3-hrom.u5",  0x9000, 0x1000, CRC(52741e0b) SHA1(2a7b950f9810c5a14a1b9d5e6b2bd93da621662e) ) // "

	ROM_REGION(0x10000, "subcpu", 0)
	ROM_LOAD("c28a97m_ss-3l-rom", 0x4000, 0x0800, CRC(bf0b2a84) SHA1(286f56aca2e50b78ac1fae4a89413659aceb71d9) ) // 2316

	ROM_REGION(0x100, "nvram", 0) // default settings
	ROM_LOAD("nvram", 0, 0x100, CRC(b5dddc7b) SHA1(3be9ec8359cc9ef16a04f28dfd24f9ffe1a2fca9) )

	ROM_REGION(53552, "screen", 0)
	ROM_LOAD("ssystem3.svg", 0, 53552, CRC(6047f88f) SHA1(2ff9cfce01cd3811a3f46f84b47fdc4ea2cf2ba8) )

	ROM_REGION(748939, "chessunit", 0)
	ROM_LOAD("chessunit.svg", 0, 748939, CRC(713a46fd) SHA1(6119162fb7c00f81aeca0dfe274475dc8575dd70) )
ROM_END

ROM_START( ssystem4 )
	ROM_REGION(0x10000, "maincpu", 0) // roms in a cartridge
	ROM_LOAD("c45021_ss4-lrom", 0xd000, 0x1000, CRC(fc86a4fc) SHA1(ee292925165d4bf7b948c60a81d95f7a4064e797) ) // 2332
	ROM_LOAD("c45022_ss4-mrom", 0xe000, 0x1000, CRC(c6110af1) SHA1(4b63454a23b2fe6b5c8f3fa6718eb49770cb6907) ) // "
	ROM_LOAD("c45023_ss4-hrom", 0xf000, 0x1000, CRC(ab4a4343) SHA1(6eeee7168e13dc1115cb5833f1938a8ea8c01d69) ) // "

	ROM_REGION(53552, "screen", 0) // looks same, but different pinout
	ROM_LOAD("ssystem4.svg", 0, 53552, CRC(b69b12e3) SHA1(c2e39d015397d403309f1c23619fe8abc3745d87) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT           COMPANY, FULLNAME, FLAGS
SYST( 1979, ssystem3, 0,      0,      ssystem3, ssystem3, ssystem3_state, init_ssystem3, "SciSys / Novag Industries / Philidor Software", "Chess Champion: Super System III", MACHINE_SUPPORTS_SAVE )

SYST( 1980, ssystem4, 0,      0,      ssystem4, ssystem4, ssystem3_state, empty_init,    "SciSys / Philidor Software", "Chess Champion: Super System IV", MACHINE_SUPPORTS_SAVE )
