// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

SciSys / Novag Chess Champion: Super System III (aka MK III), distributed by
both SciSys and Novag. Which company was responsible for which part of the
manufacturing chain is unknown. The software is by SciSys (no mention of Novag
in the ROM, it has "COPYRIGHT SCISYS LTD 1979").

This is their 1st original product. MK II was licensed from Commodore, and
MK I was, to put it bluntly, a bootleg. The chess engine is by Mike Johnson,
with support from David Levy.

Hardware notes: (main unit)
- Synertek 6502A @ 2MHz (4MHz XTAL)
- Synertek 6522 VIA
- 8KB ROM (2*Synertek 2332)
- 1KB RAM (2*HM472114P-3)
- MD4332BE + a bunch of TTL for the LCD
- 13 buttons, 4 switches, no leds or sensorboard
- connectors for: PSU, Power Pack, Chess Unit, Printer Unit

Chess Unit:
- PCB label: Radofin XM-2057-0C
- Fairchild F6808P CPU @ ?MHz (M6808 compatible)
- Fairchild F6821P PIA
- C28A97M 4KB ROM(2332), 128x8 RAM(F6810P)
- 2*HLCD0438, chessboard LCD

Printer Unit:
- unknown hardware, assume own CPU like the chess unit

PSU ("permanent storage unit"?) is just a battery-backed 256x4 RAM (TC5501P)
module, not sure why it was so expensive (~180DM).
A chess clock accessory was also announced but unreleased.

SciSys Super System IV is on similar hardware.

TODO:
- 6522 ACR register is initialized with 0xe3. Meaning: PA and PB inputs are set
  to latch mode, but the program then never clocks the latch, it functions as if
  it meant to write 0xe0. Maybe 6522 CA1 pin emulation is wrong? Documentation
  says it's edge-triggered, but here it's tied to VCC. I added a trivial hack to
  work around this, see rom defs.
- 6522 timer runs too fast, currently worked around by clocking it at 1MHz. PB6/PB7
  was measured 997Hz on real device, it's 1989Hz on MAME at 2MHz, both 50% duty cycle.
- 2nd 7474 /2 clock divider on each 4000-7fff access, this also applies to 6522 clock
  (doesn't affect chess calculation speed, only I/O access, eg. beeper pitch).
  Should be doable to add, but 6522 device doesn't support live clock changes.
- LCD TC pin? connects to the display, source is a 50hz timer(from power supply),
  probably to keep refreshing the LCD when inactive, there is no need to emulate it
- dump/add chessboard lcd and printer unit
- dump/add 1980 program revision, were the BTANB fixed?

BTANB:
- If the TIME switch is held up, it will sometimes recognize the wrong input when
  another button is pressed. I assume they noticed this bug too late and tried to
  lessen the chance by adding a spring to the switch.
- Similar to the TIME switch bug, pressing 2 buttons simultaneously can cause it
  to malfunction, eg. press A+CE or C+CE and an "8" appears in the display.

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "video/md4330b.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "saitek_ssystem3.lh" // clickable


namespace {

class ssystem3_state : public driver_device
{
public:
	ssystem3_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_via(*this, "via"),
		m_lcd(*this, "lcd"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_nvram(*this, "nvram"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine drivers
	void ssystem3(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<md4332b_device> m_lcd;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_shared_ptr<u8> m_nvram;
	required_ioport_array<4+2> m_inputs;

	// address maps
	void main_map(address_map &map);

	// I/O handlers
	DECLARE_WRITE32_MEMBER(lcd_q_w) { m_lcd_q = data; }
	DECLARE_WRITE8_MEMBER(nvram_w);
	DECLARE_READ8_MEMBER(nvram_r);
	DECLARE_WRITE8_MEMBER(input_w);
	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_READ8_MEMBER(control_r);

	u8 m_inp_mux;
	u8 m_control;
	u8 m_shift;
	u32 m_lcd_q;
};

void ssystem3_state::machine_start()
{
	// zerofill
	m_inp_mux = 0;
	m_control = 0;
	m_shift = 0;
	m_lcd_q = 0;

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_control));
	save_item(NAME(m_shift));
	save_item(NAME(m_lcd_q));
}



/******************************************************************************
    I/O
******************************************************************************/

WRITE8_MEMBER(ssystem3_state::nvram_w)
{
	// nvram is only d0-d3
	if (m_inputs[5]->read() & 1)
		m_nvram[offset] = data & 0x0f;
}

READ8_MEMBER(ssystem3_state::nvram_r)
{
	return (m_inputs[5]->read() & 1) ? (m_nvram[offset] & 0x0f) : 0;
}

WRITE8_MEMBER(ssystem3_state::input_w)
{
	// PA0-PA7: input mux
	m_inp_mux = ~data;
}

READ8_MEMBER(ssystem3_state::input_r)
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

	return ~data;
}

WRITE8_MEMBER(ssystem3_state::control_w)
{
	// PB0: speaker out
	m_dac->write(~data & m_inputs[4]->read() & 1);

	// PB1: LCD DI
	// PB2: LCD CLK
	m_lcd->di_w(BIT(data, 1));
	m_lcd->clk_w(BIT(data, 2));

	// PB2 also clocks a 4015B
	// DA: LCD DO, DB: Q3A
	if (data & ~m_control & 4)
	{
		m_shift = m_shift << 1 | m_lcd->do_r();
		u64 out2 = m_shift | 0x100;

		// weird TTL maze, I assume it's a hw kludge to fix a bug after the maskroms were already manufactured
		if (BIT(m_shift, 3) & ~(BIT(m_shift, 1) ^ BIT(m_shift, 4)) & ~(BIT(m_lcd_q, 7) & BIT(m_lcd_q, 23)))
			out2 ^= 0x12;

		m_display->matrix(1, out2 << 32 | m_lcd_q);
	}

	// PB3: device serial out

	// PB7: tied to PB6 (pulse timer 2)
	m_via->write_pb6(BIT(data, 7));

	m_control = data;
}

READ8_MEMBER(ssystem3_state::control_r)
{
	// PB4: device busy
	// PB5: device attached?
	return 0xff;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void ssystem3_state::main_map(address_map &map)
{
	map(0x0000, 0x03ff).mirror(0x3c00).ram();
	map(0x4000, 0x40ff).mirror(0x1f00).ram().rw(FUNC(ssystem3_state::nvram_r), FUNC(ssystem3_state::nvram_w)).share("nvram");
	map(0x6000, 0x600f).mirror(0x1ff0).m(m_via, FUNC(via6522_device::map));
	map(0x8000, 0x9fff).mirror(0x6000).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( ssystem3 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9 / EP / C SQ")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0 / MD / C Board")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_F) PORT_NAME("F 6 / Knight / Clock")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_LEFT) PORT_NAME("E 5 / Bishop / Left")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE / Interrupt")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("G 7 / Pawn / Right")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_D) PORT_NAME("D 4 / Rook / #")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_A) PORT_NAME("A 1 / White")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Time") // spring-loaded
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_H) PORT_NAME("H 8 / Black")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_C) PORT_NAME("C 3 / Queen / #50")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_B) PORT_NAME("B 2 / King / FP")

	PORT_START("IN.4") // switches
	PORT_CONFNAME( 0x01, 0x01, "Sound" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_CONFNAME( 0x02, 0x02, "Light" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x02, DEF_STR( On ) )

	PORT_START("IN.5") // accessories
	PORT_CONFNAME( 0x01, 0x01, "Memory Unit" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void ssystem3_state::ssystem3(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 4_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &ssystem3_state::main_map);

	VIA6522(config, m_via, 4_MHz_XTAL / 4); // WRONG! should be 2MHz
	m_via->writepa_handler().set(FUNC(ssystem3_state::input_w));
	m_via->readpa_handler().set(FUNC(ssystem3_state::input_r));
	m_via->writepb_handler().set(FUNC(ssystem3_state::control_w));
	m_via->readpb_handler().set(FUNC(ssystem3_state::control_r));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	MD4332B(config, m_lcd);
	m_lcd->write_q().set(FUNC(ssystem3_state::lcd_q_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/2, 729/2);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(1, 32+8+1);
	m_display->set_bri_levels(0.25);
	config.set_default_layout(layout_saitek_ssystem3);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( ssystem3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("c19081e_ss-3-lrom.u4", 0x8000, 0x1000, CRC(9ea46ed3) SHA1(34eef85b356efbea6ddac1d1705b104fc8e2731a) ) // 2332
	ROM_LOAD("c19082_ss-3-hrom.u5",  0x9000, 0x1000, CRC(52741e0b) SHA1(2a7b950f9810c5a14a1b9d5e6b2bd93da621662e) ) // "

	// HACK! 6522 ACR register setup
	ROM_FILL(0x946d, 1, 0xe0) // was 0xe3

	ROM_REGION( 0x100, "nvram", 0 ) // default settings
	ROM_LOAD( "nvram", 0, 0x100, CRC(b5dddc7b) SHA1(3be9ec8359cc9ef16a04f28dfd24f9ffe1a2fca9) )

	ROM_REGION( 53511, "screen", 0)
	ROM_LOAD( "ssystem3.svg", 0, 53511, CRC(a2820cc8) SHA1(2e922bb2d4a244931c1e7dafa84910dee5ab2623) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT CMP MACHINE   INPUT     STATE           INIT        COMPANY, FULLNAME, FLAGS
CONS( 1979, ssystem3, 0,      0, ssystem3, ssystem3, ssystem3_state, empty_init, "SciSys / Novag", "Chess Champion: Super System III", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
