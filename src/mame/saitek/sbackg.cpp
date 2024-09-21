// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Saitek Sensory Backgammon

TODO:
- skeleton driver

Hardware notes:
- x

*******************************************************************************/

#include "emu.h"

#include "cpu/hmcs400/hmcs400.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
//#include "saitek_sbackg.lh"


namespace {

class sbackg_state : public driver_device
{
public:
	sbackg_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_led_pwm(*this, "led_pwm"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void sbackg(machine_config &config);
	void granada(machine_config &config);
	void supra(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(granada_change_cpu_freq);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<hmcs400_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_led_pwm;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<1> m_inputs;
};

void sbackg_state::machine_start()
{
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( sbackg )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void sbackg_state::sbackg(machine_config &config)
{
	// basic machine hardware
	HD614085(config, m_maincpu, 5'000'000); // approximation, no XTAL

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->set_delay(attotime::from_msec(150));
	//m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(2, 24);

	PWM_DISPLAY(config, m_led_pwm).set_size(6, 8);
	//config.set_default_layout(layout_saitek_sbackg);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( sbackg )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("1988_gx2_saitek_614085sc72.u1", 0x0000, 0x4000, CRC(0e91d7a1) SHA1(fa147105fa99d01210b53389b8b70031cff6ad66) )

	ROM_REGION( 109526, "screen", 0 )
	ROM_LOAD("sbackg.svg", 0, 109526, CRC(b8149d74) SHA1(0cc6f1a2c50f53f8d2be73379019d275799d0546) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1988, sbackg, 0,      0,      sbackg,  sbackg, sbackg_state, empty_init, "Saitek", "Sensory Backgammon", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
