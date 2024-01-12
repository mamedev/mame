// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Saitek Electronic Champion Backgammon

NOTE: Before exiting MAME, change the power switch from GO to STOP. Otherwise,
NVRAM won't save properly.

Hardware notes:
- PCB label: GT4-PE-009
- Hitachi HD6301Y0P @ ~4MHz (no XTAL)
- LCD with custom segments
- 24 LEDs, 13*2 buttons sensor board, piezo

TODO:
- if/when MAME supports an exit callback, hook up power-off switch to that
- sensorboard, lcd, internal artwork

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

	DECLARE_INPUT_CHANGED_MEMBER(power_off);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override { m_power = true; }

private:
	// devices/pointers
	required_device<hd6301y0_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<5> m_inputs;

	bool m_power = false;
	u8 m_inp_mux = 0;

	void init_board(int state);

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
	save_item(NAME(m_inp_mux));
}

void ecbackg_state::init_board(int state)
{
}

INPUT_CHANGED_MEMBER(ecbackg_state::power_off)
{
	if (newval)
		m_power = false;
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
	//printf("r2 ");

	// P27: power switch state
	u8 data = m_power ? 0 : 0x80;

	// P24: P57
	data |= ~p5_r() >> 3 & 0x10;
	return ~data;
}

void ecbackg_state::p2_w(u8 data)
{
	//printf("w2_%X ",data);

	// P20-P22: led select
	m_display->write_my(~data & 7);

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
		if (BIT(bitswap<5>(m_inp_mux,6,5,4,3,0), i))
			data |= m_inputs[i]->read();

	return ~data;
}

void ecbackg_state::p6_w(u8 data)
{
	// P60-P67: input mux, led data
	m_inp_mux = ~data;
	m_display->write_mx(~data);

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

	PORT_START("POWER") // needs to be triggered for nvram to work
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, ecbackg_state, power_off, 0) PORT_NAME("Stop")
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
	m_board->init_cb().set(FUNC(ecbackg_state::init_board));
	m_board->set_size(13, 10);
	m_board->set_spawnpoints(2);
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(3, 8);
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
