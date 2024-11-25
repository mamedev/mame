// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Novag Micro Chess

It's a portable chesscomputer with sensory board. The MCU says "(C) CAL R & O3",
though the program is supposedly by David Kittinger? Unlikely, since his first
work with Novag was Super Sensor IV, a couple of months newer than Micro Chess.

Hardware notes:
- Mostek 3875/42 (4KB ROM, 64 bytes extra RAM)
- buzzer, button sensors chessboard, 16+4 leds

MCU interrupts are unused. MCU embedded extra RAM is battery-backed via MEM
switch tied to pin #4 (VSB: RAM standby power).

*******************************************************************************/

#include "emu.h"

#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "machine/sensorboard.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "novag_micro.lh"


namespace {

class micro_state : public driver_device
{
public:
	micro_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_board(*this, "board"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.0")
	{ }

	void micro(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<sensorboard_device> m_board;
	required_device<dac_1bit_device> m_dac;
	required_ioport m_inputs;

	u8 m_led_data = 0;
	u8 m_control = 0;
	u8 m_inp_mux = 0;

	// address maps
	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	// I/O handlers
	void update_display();
	void input_w(u8 data);
	u8 input_r();
	void control_w(u8 data);
	u8 control_r();
	void led_w(u8 data);
};

void micro_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_led_data));
	save_item(NAME(m_control));
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void micro_state::update_display()
{
	u8 sel = (m_control << 2 & 0x3c) | (m_led_data >> 6 & 3);
	m_display->matrix(sel, m_led_data & 0x3f);
}

void micro_state::input_w(u8 data)
{
	// P00-P01: MK3875 doesn't have these pins
	// P02-P07: input mux part
	m_inp_mux = data >> 2;
}

u8 micro_state::input_r()
{
	// P10-P17: multiplexed inputs
	u8 data = 0;

	// read chessboard
	u8 cb_mux = (m_inp_mux << 2) | (m_control >> 5 & 3);
	cb_mux = bitswap<8>(cb_mux,4,5,6,7,1,0,3,2);

	for (int i = 0; i < 8; i++)
		if (BIT(cb_mux, i))
			data |= bitswap<8>(m_board->read_file(i),4,5,6,7,3,2,1,0);

	// read buttons
	if (m_control & 0x10)
		data |= m_inputs->read();

	return data;
}

void micro_state::control_w(u8 data)
{
	// P40: led select part
	// P41: white led
	// P42: ?
	// P43: black led
	// P44-P46: input mux part
	m_control = data;
	update_display();

	// P47: speaker out
	m_dac->write(BIT(data, 7));
}

u8 micro_state::control_r()
{
	return m_control;
}

void micro_state::led_w(u8 data)
{
	// P50-P55: led data
	// P56,P57: led select part
	m_led_data = data;
	update_display();
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void micro_state::main_map(address_map &map)
{
	map.global_mask(0xfff);
	map(0x0000, 0x0fbf).rom();
	map(0x0fc0, 0x0fff).ram().share("nvram");
}

void micro_state::main_io(address_map &map)
{
	map(0x00, 0x00).w(FUNC(micro_state::input_w));
	map(0x01, 0x01).r(FUNC(micro_state::input_r)).nopw();
	map(0x04, 0x07).rw("psu", FUNC(f38t56_device::read), FUNC(f38t56_device::write));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( micro )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level / Bishop")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_S) PORT_NAME("Sound / Queen")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back / King")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_G) PORT_NAME("Go")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_B) PORT_NAME("B/W")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_V) PORT_NAME("Verify / Pawn")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_U) PORT_NAME("Set Up / Rook")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game / Knight")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void micro_state::micro(machine_config &config)
{
	// basic machine hardware
	F8(config, m_maincpu, 4'500'000/2); // matches video reference
	m_maincpu->set_addrmap(AS_PROGRAM, &micro_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &micro_state::main_io);

	f38t56_device &psu(F38T56(config, "psu", 4'500'000/2));
	psu.read_a().set(FUNC(micro_state::control_r));
	psu.write_a().set(FUNC(micro_state::control_w));
	psu.write_b().set(FUNC(micro_state::led_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(6, 6);
	config.set_default_layout(layout_novag_micro);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( nmicro )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("3875_42_mk17121n", 0x0000, 0x1000, CRC(f21189f7) SHA1(ba346177eaeddc87a03b1103f0299b5bcd4b6c27) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY, FULLNAME, FLAGS
SYST( 1981, nmicro, 0,      0,      micro,   micro, micro_state, empty_init, "Novag Industries", "Micro Chess", MACHINE_SUPPORTS_SAVE )
