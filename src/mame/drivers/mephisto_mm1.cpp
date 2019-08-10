// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

Mephisto MM I, the first H+G slide-in chesscomputer module

The module was included with either the Modular or Modular Exclusive chessboards.
Initially, the module itself didn't have a name. It was only later in retrospect,
after the release of Modul MM II that it became known as the MM I.

Hardware notes:
- CDP1806 @ 8MHz, 6.5V
- 32KB ROM (2*D27128, or HN613256P)
- 4KB RAM (2*HM6116LP-3)
- CDP1853CE, CD4011BE, 3*40373BP, 4556BE
- modular slot, 18-button keypad, beeper

It supports the HG 170 opening book module.
LCD module is assumed to be same as MM II and others.

TODO:
- doesn't work, MAME doesn't emulate 1806 CPU

******************************************************************************/

#include "emu.h"
#include "cpu/cosmac/cosmac.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "video/pwm.h"
#include "speaker.h"

// internal artwork
//#include "mephisto_mm1.lh" // clickable


namespace {

class mm1_state : public driver_device
{
public:
	mm1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine drivers
	void mm1(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<cdp1802_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<4> m_inputs;

	// address maps
	void mm1_map(address_map &map);
	void mm1_io(address_map &map);

};

void mm1_state::machine_start()
{
}



/******************************************************************************
    I/O
******************************************************************************/



/******************************************************************************
    Address Maps
******************************************************************************/

void mm1_state::mm1_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

void mm1_state::mm1_io(address_map &map)
{
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( mm1 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R)

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I)
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void mm1_state::mm1(machine_config &config)
{
	/* basic machine hardware */
	CDP1802(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mm1_state::mm1_map);
	m_maincpu->set_addrmap(AS_IO, &mm1_state::mm1_io);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(250));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(4+8, 8);
	m_display->set_segmask(0xf, 0x7f);
	//config.set_default_layout(layout_mephisto_mm1);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( mm1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("114", 0x0000, 0x4000, CRC(208b4c43) SHA1(48f891d614fa643f47d099f94aff15a44c2efc07) ) // D27128
	ROM_LOAD("214", 0x4000, 0x4000, CRC(93734e49) SHA1(9ad6c191074c4122300f059e2ef9cfeff7b81463) ) // "
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME  PARENT CMP MACHINE INPUT STATE      INIT        COMPANY, FULLNAME, FLAGS
CONS( 1983, mm1,  0,      0, mm1,    mm1,  mm1_state, empty_init, "Hegener + Glaser", "Mephisto MM I", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
