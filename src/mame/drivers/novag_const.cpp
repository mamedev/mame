// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

Novag Super Sensor IV (model 812)
Novag Constellation (model 831)
Novag Super Constellation (model 844)
Novag Constellation 3.6MHz (model 845)
Novag Constellation Quattro (model 862)

The chess engine is by David Kittinger, Super Sensor IV is his first one
working under Novag. The Constellation engine is completely different.

Hardware notes:

They are all on very similar hardware.
The more expensive chesscomputers have battery-backed RAM and support for
printer and chessclock peripherals.

Constellation:
- MOS MPS6502A @ 2MHz
- 2KB RAM (daughterboard with 4*2114), 2*8KB ROM
- TTL, buzzer, 24 LEDs, 8*8 chessboard buttons

Constellation 3.6MHz:
- PCB label: NOVAG CONSTELLATION Rev E 100037
- G65SC02P-3 or R65C02P3(newer version) @ 3.6MHz (7.2MHz XTAL)
- 2KB RAM (TC5516AP), 16KB ROM (custom label, assumed TMM23128)
- PCB supports "Memory Save", but components aren't installed

Constellation 3.6MHz manufactured in 1986 was fitted with the same ROM as
the Quattro, but PCB remains almost the same (Quatto PCB is very different).
Button labels are the same as the older version.

Constellation Quattro:
- R65C02P3 @ 4MHz (8MHz XTAL)
- 2KB RAM (D449C), 16KB ROM (custom label)

Super Sensor IV:
- MOS MPS6502A @ 2MHz
- 1KB battery-backed RAM (2*TC5514AP-3)
- 8KB ROM (TMM2364P)
- 2 ROM sockets for expansion (blue @ u6, white @ u5)

Known Super Sensor IV expansion ROMs:
- Quartz Chess Clock (came with the clock accessory)

Sensor Dynamic's ROM is identical to Super Sensor IV "1I", the hardware is
basically a low-budget version of it with peripheral ports removed.

Super Constellation:
- UMC UM6502C @ 4 MHz (8MHz XTAL)
- 4KB battery-backed RAM (2*TC5516APL-2)
- 2*32KB ROM custom label

TODO:
- is Dynamic S a program update of ssensor4 or identical?

******************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6502/m65sc02.h"
#include "cpu/m6502/r65c02.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/beep.h"
#include "video/pwm.h"

#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "novag_const.lh" // clickable
#include "novag_constq.lh" // clickable
#include "novag_ssensor4.lh" // clickable
#include "novag_supercon.lh" // clickable


namespace {

class const_state : public driver_device
{
public:
	const_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_beeper(*this, "beeper"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(power) { if (newval && m_power) power_off(); }

	// machine configs
	void nconst(machine_config &config);
	void nconst36(machine_config &config);
	void nconst36a(machine_config &config);
	void nconstq(machine_config &config);
	void ssensor4(machine_config &config);
	void sconst(machine_config &config);

	void init_const();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override { m_power = true; }

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<beep_device> m_beeper;
	required_ioport_array<8> m_inputs;

	// address maps
	void const_map(address_map &map);
	void ssensor4_map(address_map &map);
	void sconst_map(address_map &map);

	// I/O handlers
	void update_display();
	void mux_w(u8 data);
	void control_w(u8 data);
	u8 input1_r();
	u8 input2_r();

	void power_off();
	bool m_power = false;

	u8 m_inp_mux = 0;
	u8 m_led_select = 0;
};

void const_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_power));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_select));
}

void const_state::power_off()
{
	// NMI at power-off (ssensor4 prepares nvram for next power-on)
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	m_power = false;
}

void const_state::init_const()
{
	// game relies on RAM filled with FF at power-on
	for (int i = 0; i < 0x800; i++)
		m_maincpu->space(AS_PROGRAM).write_byte(i, 0xff);
}



/******************************************************************************
    I/O
******************************************************************************/

void const_state::update_display()
{
	m_display->matrix(m_led_select, m_inp_mux);
}

void const_state::mux_w(u8 data)
{
	// d0-d7: input mux, led data
	m_inp_mux = data;
	update_display();
}

void const_state::control_w(u8 data)
{
	// d0-d2: ?
	// d3: ? (goes high at power-off NMI)
	// d4-d6: select led row
	m_led_select = data >> 4 & 7;
	update_display();

	// d7: enable beeper
	m_beeper->set_state(data >> 7 & 1);
}

u8 const_state::input1_r()
{
	u8 data = 0;

	// d0-d7: multiplexed inputs (chessboard squares)
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_rank(i ^ 7, true);

	return ~data;
}

u8 const_state::input2_r()
{
	u8 data = 0;

	// d3: timing related? seems unused (always high)
	// other: ?

	// d6,d7: multiplexed inputs (side panel)
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read() << 6;

	return ~data;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void const_state::ssensor4_map(address_map &map)
{
	map(0x0000, 0x03ff).ram().share("nvram");
	map(0x2000, 0x2000).nopw(); // accessory?
	map(0x4000, 0x4000).nopw(); // "
	map(0x6000, 0x6000).rw(FUNC(const_state::input2_r), FUNC(const_state::mux_w));
	map(0x8000, 0x8000).rw(FUNC(const_state::input1_r), FUNC(const_state::control_w));
	map(0xc000, 0xdfff).r("exrom", FUNC(generic_slot_device::read_rom));
	map(0xe000, 0xffff).rom();
}

void const_state::const_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x6000, 0x6000).rw(FUNC(const_state::input2_r), FUNC(const_state::mux_w));
	map(0x8000, 0x8000).rw(FUNC(const_state::input1_r), FUNC(const_state::control_w));
	map(0xa000, 0xffff).rom();
}

void const_state::sconst_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("nvram");
	map(0x1c00, 0x1c00).nopw(); // accessory?
	map(0x1d00, 0x1d00).nopw(); // "
	map(0x1e00, 0x1e00).rw(FUNC(const_state::input2_r), FUNC(const_state::mux_w));
	map(0x1f00, 0x1f00).rw(FUNC(const_state::input1_r), FUNC(const_state::control_w));
	map(0x2000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( nconst )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("New Game")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Multi Move / Player/Player / King")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Verify / Set Up")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Best Move / Random / Queen")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Change Color")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Sound / Bishop")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Clear Board")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Solve Mate / Knight")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Rook")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Pawn")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Hint / Show Moves")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Set Level")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Go")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Take Back")
INPUT_PORTS_END

static INPUT_PORTS_START( ssensor4 )
	PORT_INCLUDE( nconst )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Accessory / King")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Time Control / Queen")

	PORT_MODIFY("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Print Moves")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Print Board / Rook")

	PORT_MODIFY("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Form Size")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Print List / Pawn")

	PORT_MODIFY("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Hint")

	PORT_START("POWER") // needs to be triggered for nvram to work
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, const_state, power, 0) PORT_NAME("Power Off")
INPUT_PORTS_END

static INPUT_PORTS_START( nconstq )
	PORT_INCLUDE( nconst )

	PORT_MODIFY("IN.1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Best Move/Random / Training Level / Queen")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Sound / Depth Search / Bishop")
INPUT_PORTS_END

static INPUT_PORTS_START( sconst )
	PORT_INCLUDE( nconstq )

	PORT_MODIFY("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Print Moves")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Print Board / Rook")

	PORT_MODIFY("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Form Size")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Print List / Acc. Time / Pawn")

	PORT_START("POWER")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, const_state, power, 0) PORT_NAME("Power Off")
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void const_state::nconst(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 2_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &const_state::const_map);

	auto &irq_clock(CLOCK(config, "irq_clock", 2_MHz_XTAL / 0x2000)); // through 4020 IC, ~244Hz
	irq_clock.set_pulse_width(attotime::from_nsec(17200)); // active for ~17.2us
	irq_clock.signal_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(250));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(3, 8);
	config.set_default_layout(layout_novag_const);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 2_MHz_XTAL / 0x800); // ~976Hz
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void const_state::ssensor4(machine_config &config)
{
	nconst(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &const_state::ssensor4_map);

	subdevice<clock_device>("irq_clock")->set_pulse_width(attotime::from_usec(39)); // irq active for 39us

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);
	m_board->set_nvram_enable(true);

	config.set_default_layout(layout_novag_ssensor4);

	/* expansion */
	GENERIC_SOCKET(config, "exrom", generic_plain_slot, "novag_ssensor4");
	SOFTWARE_LIST(config, "cart_list").set_original("novag_ssensor4");
}

void const_state::nconst36(machine_config &config)
{
	nconst(config);

	/* basic machine hardware */
	M65SC02(config.replace(), m_maincpu, 7.2_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &const_state::const_map);

	subdevice<clock_device>("irq_clock")->set_clock(7.2_MHz_XTAL/2 / 0x2000); // ~439Hz (pulse width same as nconst)

	m_board->set_delay(attotime::from_msec(200));

	/* sound hardware */
	BEEP(config.replace(), m_beeper, 7.2_MHz_XTAL/2 / 0x800); // ~1758Hz
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void const_state::nconst36a(machine_config &config)
{
	nconst36(config);

	/* basic machine hardware */
	R65C02(config.replace(), m_maincpu, 7.2_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &const_state::const_map);

	// 4020 CLK is 7.2_MHz_XTAL/4, but with IRQ on Q11 instead of Q12, result
	// frequency and duty cycle are identical to nconst36

	/* sound hardware */
	BEEP(config.replace(), m_beeper, 7.2_MHz_XTAL/4 / 0x800); // ~879Hz
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void const_state::nconstq(machine_config &config)
{
	nconst36a(config);

	/* basic machine hardware */
	m_maincpu->set_clock(8_MHz_XTAL/2);

	subdevice<clock_device>("irq_clock")->set_clock(8_MHz_XTAL/4 / 0x1000); // ~488Hz (pulse width same as nconst)

	config.set_default_layout(layout_novag_constq);

	/* sound hardware */
	BEEP(config.replace(), m_beeper, 8_MHz_XTAL/4 / 0x800); // ~976Hz
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void const_state::sconst(machine_config &config)
{
	nconstq(config);

	/* basic machine hardware */
	M6502(config.replace(), m_maincpu, 8_MHz_XTAL/2); // UM6502C
	m_maincpu->set_addrmap(AS_PROGRAM, &const_state::sconst_map);

	subdevice<clock_device>("irq_clock")->set_pulse_width(attotime::from_nsec(10200)); // irq active for 10.2us

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);
	m_board->set_nvram_enable(true);

	config.set_default_layout(layout_novag_supercon);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( ssensor4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("5611_1i_orange.u4", 0xe000, 0x2000, CRC(f4ee99d1) SHA1(f44144a26b92c51f4350da85858470e6c3b66fc1) ) // TMM2364P
ROM_END


ROM_START( const )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("8315_white",  0xa000, 0x2000, CRC(76e6c97b) SHA1(55645e08f9f1258366c29a4ea2033bb86d860227) ) // TMM2364P
	ROM_LOAD("8314_orange", 0xe000, 0x2000, CRC(89395a86) SHA1(4807f196fec70abdaabff5bfc479a64d5cf2b0ad) ) // "
ROM_END

ROM_START( const36 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("novag-831a_6133-8316.u2", 0xc000, 0x4000, CRC(7da760f3) SHA1(6172e0fa03377e911141a86747849bf25f20613f) )
ROM_END

ROM_START( const36a )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("novag_854-501.u2", 0xc000, 0x4000, CRC(b083d5c4) SHA1(ecac8a599bd8ea8dd549c742ec45b94fb8b11af4) )
ROM_END

ROM_START( constq )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD("novag_854-501.u2", 0xc000, 0x4000, CRC(b083d5c4) SHA1(ecac8a599bd8ea8dd549c742ec45b94fb8b11af4) )
ROM_END


ROM_START( supercon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("novag_8443", 0x0000, 0x8000, CRC(b853cf6e) SHA1(1a759072a5023b92c07f1fac01b7a21f7b5b45d0) )
	ROM_LOAD("novag_8442", 0x8000, 0x8000, CRC(c8f82331) SHA1(f7fd039f9a3344db9749931490ded9e9e309cfbe) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT CMP MACHINE    INPUT     STATE        INIT        COMPANY, FULLNAME, FLAGS
CONS( 1981, ssensor4, 0,      0, ssensor4,  ssensor4, const_state, empty_init, "Novag", "Super Sensor IV", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1983, const,    0,      0, nconst,    nconst,   const_state, init_const, "Novag", "Constellation", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1984, const36,  const,  0, nconst36,  nconst,   const_state, init_const, "Novag", "Constellation 3.6MHz (set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1986, const36a, const,  0, nconst36a, nconst,   const_state, init_const, "Novag", "Constellation 3.6MHz (set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1986, constq,   const,  0, nconstq,   nconstq,  const_state, init_const, "Novag", "Constellation Quattro", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1984, supercon, 0,      0, sconst,    sconst,   const_state, empty_init, "Novag", "Super Constellation", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
