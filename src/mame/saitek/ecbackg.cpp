// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Saitek Electronic Champion Backgammon

Hardware notes:
- PCB label: GT4-PE-009
- Hitachi HD6301Y0P @ ~4MHz (no XTAL)
- LCD with custom segments, 24 LEDs, piezo

TODO:
- everything

*******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

//#include "saitek_ecbackg.lh"


namespace {

class ecbackg_state : public driver_device
{
public:
	ecbackg_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void ecbackg(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<hd6301y0_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<5> m_inputs;

	u8 m_but_mux = 0;

	// I/O handlers
	void p1_w(u8 data);
	u8 p2_r();
	void p2_w(u8 data);
	void p3_w(u8 data);
	void p4_w(u8 data);
	u8 p5_r();
	void p6_w(u8 data);
	void p7_w(u8 data);
};

void ecbackg_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_but_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

//[:maincpu] Port 5 Data Direction Register: 00
//[:maincpu] Port 6 Data Direction Register: ff
//[:maincpu] Port 2 Data Direction Register: 2f
//[:maincpu] Port 1 Data Direction Register: ff
//[:maincpu] Port 3 Data Direction Register: ff
//[:maincpu] Port 4 Data Direction Register: ff

void ecbackg_state::p1_w(u8 data)
{
	//printf("w1_%X ",data);
}

u8 ecbackg_state::p2_r()
{
	// P24: P57

	//printf("r2 ");
	return 0xff;
}

void ecbackg_state::p2_w(u8 data)
{
	//printf("w2_%X ",data);

	// P23: speaker out
	m_dac->write(BIT(data, 3));
}

void ecbackg_state::p3_w(u8 data)
{
	//printf("w3_%X ",data);
}

void ecbackg_state::p4_w(u8 data)
{
	//printf("w4_%X ",data);
}

u8 ecbackg_state::p5_r()
{
	// P50-P57: multiplexed inputs
	u8 data = 0;

	// read buttons
	for (int i = 0; i < 5; i++)
		if (BIT(m_but_mux, i))
			data |= m_inputs[i]->read();

	return ~data;
}

void ecbackg_state::p6_w(u8 data)
{
	// P60,P63-P66: button mux
	m_but_mux = (~data >> 2 & 0x1e) | (~data & 1);

	//printf("w6_%X ",data);
}

void ecbackg_state::p7_w(u8 data)
{
	//printf("w7_%X ",data);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( ecbackg )
	PORT_START("IN.0")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) // stats

	PORT_START("IN.1")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) // bear off

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) // double
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) // reject
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) // play
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) // game option
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) // level
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) // take back
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) // verify

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) // roll dice
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) // d1
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) // d2
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) // d3
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) // d4
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) // d5
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) // d6

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) // new game
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void ecbackg_state::ecbackg(machine_config &config)
{
	// basic machine hardware
	HD6301Y0(config, m_maincpu, 4'000'000); // approximation
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(hd6301y0_cpu_device::nvram_set_battery));
	m_maincpu->standby_cb().append([this](int state) { if (state) m_display->clear(); });
	m_maincpu->out_p1_cb().set(FUNC(ecbackg_state::p1_w));
	m_maincpu->in_p2_cb().set(FUNC(ecbackg_state::p2_r));
	m_maincpu->out_p2_cb().set(FUNC(ecbackg_state::p2_w));
	m_maincpu->out_p3_cb().set(FUNC(ecbackg_state::p3_w));
	m_maincpu->out_p4_cb().set(FUNC(ecbackg_state::p4_w));
	m_maincpu->in_p5_cb().set(FUNC(ecbackg_state::p5_r));
	m_maincpu->out_p6_cb().set(FUNC(ecbackg_state::p6_w));
	m_maincpu->out_p7_cb().set(FUNC(ecbackg_state::p7_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->set_delay(attotime::from_msec(150));
	//m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(4, 6);
	//config.set_default_layout(layout_saitek_ecbackg);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( ecbackg )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("saitek_89_gx4b_c12_31y0rm79p.u1", 0x0000, 0x4000, CRC(d7278545) SHA1(9ece31cdb237067aeec480c066e0917752697a4d) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1989, ecbackg, 0,      0,      ecbackg, ecbackg, ecbackg_state, empty_init, "Saitek", "Electronic Champion Backgammon", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_NOT_WORKING )
