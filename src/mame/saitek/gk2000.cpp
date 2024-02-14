// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Saitek Kasparov GK 2000

TODO:
- verify buttons
- add lcd
- clean up WIP code
- internal artwork

*******************************************************************************/

#include "emu.h"

#include "cpu/h8/h8325.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
//#include "saitek_gk2000.lh"


namespace {

class gk2000_state : public driver_device
{
public:
	gk2000_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_led_pwm(*this, "led_pwm"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void gk2000(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(go_button);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<h8323_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_led_pwm;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<3> m_inputs;

	u16 m_inp_mux = 0;

	void main_map(address_map &map);

	// I/O handlers
	u8 p1_r();
	void p1_w(u8 data);
	u8 p2_r();
	void p2_w(u8 data);
	u8 p3_r();
	void p3_w(u8 data);
	u8 p4_r();
	void p4_w(u8 data);
	u8 p5_r();
	void p5_w(offs_t offset, u8 data, u8 mem_mask);
	void p6_w(u8 data);
	u8 p7_r();
	void p7_w(u8 data);
};

void gk2000_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

INPUT_CHANGED_MEMBER(gk2000_state::go_button)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, newval ? ASSERT_LINE : CLEAR_LINE);
}

//[:maincpu] syscr = f9
//[:maincpu:port1] ddr_w ff
//[:maincpu:port3] ddr_w ff
//[:maincpu:port7] ddr_w ff
//[:maincpu:port6] ddr_w 4f
//[:maincpu:port2] ddr_w 00 ?
//[:maincpu:port5] ddr_w 27 ?
//[:maincpu:port5] ddr_w 1f
//[:maincpu:port2] ddr_w ff

// p4 ddr=0 -> read inputs

u8 gk2000_state::p1_r()
{
	//printf("r1 ");
	return 0xff;
}

void gk2000_state::p1_w(u8 data)
{
	//printf("w1_%X ",data);
}

u8 gk2000_state::p2_r()
{
	//printf("r2 ");
	return 0xff;
}

void gk2000_state::p2_w(u8 data)
{
	//printf("w2_%X ",data);

	// P20-P27: input mux (chessboard), led data
	m_inp_mux = (m_inp_mux & 0x700) | (data ^ 0xff);
	m_led_pwm->write_mx(~data);
}

u8 gk2000_state::p3_r()
{
	//printf("r3 ");
	return 0xff;
}

void gk2000_state::p3_w(u8 data)
{
	//printf("w3_%X ",data);
}

u8 gk2000_state::p4_r()
{
	//printf("r4 ");

	// P40-P47: multiplexed inputs
	u8 data = 0;

	// read buttons
	for (int i = 0; i < 3; i++)
		if (BIT(m_inp_mux, i + 8))
			data |= m_inputs[i]->read();

	// read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_rank(i);

	return ~data;
}

void gk2000_state::p4_w(u8 data)
{
	//printf("w4_%X ",data);
}

u8 gk2000_state::p5_r()
{
	//printf("r5 ");
	return 0xff;
}

void gk2000_state::p5_w(offs_t offset, u8 data, u8 mem_mask)
{
	//printf("w5_%X ",data);
	data |= ~mem_mask;

	// P50: speaker out
	m_dac->write(data & 1);

	// P51,P52: led select
	m_led_pwm->write_my(~data >> 1 & 3);

	// P53-P55: input mux (buttons)
	m_inp_mux = (m_inp_mux & 0xff) | (~data << 5 & 0x700);
}

void gk2000_state::p6_w(u8 data)
{
	//printf("w6_%X ",data);
}

u8 gk2000_state::p7_r()
{
	//printf("r7 ");
	return 0xff;
}

void gk2000_state::p7_w(u8 data)
{
	//printf("w7_%X ",data);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void gk2000_state::main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( gk2000 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) // ng
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) // pos
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) // lev
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) // opt
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) // info
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) // tb
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) // cl
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) // ent

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) // p
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) // n
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) // b
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) // r
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) // q
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) // k
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) // -
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) // +

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K)

	PORT_START("POWER")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_CHANGED_MEMBER(DEVICE_SELF, gk2000_state, go_button, 0) PORT_NAME("Go / Stop")
	PORT_BIT(0xef, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void gk2000_state::gk2000(machine_config &config)
{
	// basic machine hardware
	H8323(config, m_maincpu, 20_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &gk2000_state::main_map);
	//m_maincpu->nvram_enable_backup(true);
	//m_maincpu->standby_cb().set(m_maincpu, FUNC(h8325_device::nvram_set_battery));
	m_maincpu->read_port1().set(FUNC(gk2000_state::p1_r));
	m_maincpu->write_port1().set(FUNC(gk2000_state::p1_w));
	m_maincpu->read_port2().set(FUNC(gk2000_state::p2_r));
	m_maincpu->write_port2().set(FUNC(gk2000_state::p2_w));
	m_maincpu->read_port3().set(FUNC(gk2000_state::p3_r));
	m_maincpu->write_port3().set(FUNC(gk2000_state::p3_w));
	m_maincpu->read_port4().set(FUNC(gk2000_state::p4_r));
	m_maincpu->write_port4().set(FUNC(gk2000_state::p4_w));
	m_maincpu->read_port5().set(FUNC(gk2000_state::p5_r));
	m_maincpu->write_port5().set(FUNC(gk2000_state::p5_w));
	m_maincpu->read_port6().set_ioport("POWER").invert();
	m_maincpu->write_port6().set(FUNC(gk2000_state::p6_w));
	m_maincpu->read_port7().set(FUNC(gk2000_state::p7_r));
	m_maincpu->write_port7().set(FUNC(gk2000_state::p7_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	//m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_led_pwm).set_size(2, 8);
	//config.set_default_layout(layout_saitek_gk2000);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( gk2000 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("92_saitek_86071220x12_3238a13p.u1", 0x0000, 0x4000, CRC(2059399c) SHA1(d99d5f86b80565e6017b19ef3f330112ac1ce685) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1992, gk2000, 0,      0,      gk2000,  gk2000, gk2000_state, empty_init, "Saitek", "Kasparov GK 2000", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
