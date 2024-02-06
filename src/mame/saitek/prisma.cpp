// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Saitek Kasparov Prisma

Hardware notes:
- PCB label: ST9A-PE-001
- Hitachi H8/325 MCU, 20MHz XTAL
- Epson SED1502F, LCD screen (same as simultano)
- piezo, 16+3 leds, button sensors chessboard

It was also sold by Tandy as Chess Champion 2150L, with a slower CPU (16MHz XTAL).

TODO:
- does not work, it's waiting for a 16-bit timer IRQ?
- everything else

*******************************************************************************/

#include "emu.h"

#include "cpu/h8/h8325.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
//#include "saitek_prisma.lh"


namespace {

class prisma_state : public driver_device
{
public:
	prisma_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void prisma(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<h8325_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<3> m_inputs;

	u8 m_lcd_data = 0;
	u8 m_lcd_address = 0;
	u8 m_inp_mux = 0;

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
	void p5_w(u8 data);
	u8 p6_r();
	void p6_w(u8 data);
	u8 p7_r();
	void p7_w(u8 data);
};

void prisma_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_lcd_data));
	save_item(NAME(m_lcd_address));
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

//[:maincpu:port1] ddr_w ff
//[:maincpu:port2] ddr_w ff
//[:maincpu:port3] ddr_w ff
//[:maincpu:port4] ddr_w 7f
//[:maincpu:port5] ddr_w 30
//[:maincpu:port6] ddr_w ff
//[:maincpu:port7] ddr_w 00

u8 prisma_state::p1_r()
{
	//printf("r1 ");
	return 0xff;
}

void prisma_state::p1_w(u8 data)
{
	//printf("w1_%X ",data);
	// P14: speaker out
	m_dac->write(BIT(data, 4));
}

u8 prisma_state::p2_r()
{
	//printf("r2 ");
	return 0xff;
}

void prisma_state::p2_w(u8 data)
{
	//printf("w2_%X ",data);
	// P20-P27: input mux
	m_inp_mux = bitswap<8>(~data,7,6,5,4,0,3,1,2);
}

u8 prisma_state::p3_r()
{
	//printf("r3 ");
	return 0xff;
}

void prisma_state::p3_w(u8 data)
{
	//printf("w3_%X ",data);
	// P30-P37: LCD data
	m_lcd_data = bitswap<8>(data,3,4,5,6,7,0,1,2);
}

u8 prisma_state::p4_r()
{
	//printf("r4 ");
	return 0xff;
}

void prisma_state::p4_w(u8 data)
{
	//printf("w4_%X ",data);
	// P40: LCD CS
	// P41: LCD RD
	// P42: LCD WR
}

u8 prisma_state::p5_r()
{
	//printf("r5 ");

	u8 data = 0;

	// P50,P52: read buttons
	for (int i = 0; i < 3; i++)
		if (m_inp_mux & m_inputs[i]->read())
			data |= 1 << i;

	return ~data;
}

void prisma_state::p5_w(u8 data)
{
	//printf("w5_%X ",data);
}

u8 prisma_state::p6_r()
{
	//printf("r6 ");
	return 0xff;
}

void prisma_state::p6_w(u8 data)
{
	//printf("w6_%X ",data);
	// P60-P66: LCD address
	m_lcd_address = data & 0x7f;
}

u8 prisma_state::p7_r()
{
	//printf("r7 ");
	// P70-P77: read chessboard
	return 0xff;
}

void prisma_state::p7_w(u8 data)
{
	//printf("w7_%X ",data);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void prisma_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( prisma )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) // k
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) // b
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) // swap
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) // ng
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) // nor
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) // col
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) // n
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) // p
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) // stop
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) // ana
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) // info
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) // fun

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) // level
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) // play
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) // q
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) // r
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) // sound
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) // setup
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) // +
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void prisma_state::prisma(machine_config &config)
{
	// basic machine hardware
	H8325(config, m_maincpu, 20_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &prisma_state::main_map);
	m_maincpu->read_port1().set(FUNC(prisma_state::p1_r));
	m_maincpu->write_port1().set(FUNC(prisma_state::p1_w));
	m_maincpu->read_port2().set(FUNC(prisma_state::p2_r));
	m_maincpu->write_port2().set(FUNC(prisma_state::p2_w));
	m_maincpu->read_port3().set(FUNC(prisma_state::p3_r));
	m_maincpu->write_port3().set(FUNC(prisma_state::p3_w));
	m_maincpu->read_port4().set(FUNC(prisma_state::p4_r));
	m_maincpu->write_port4().set(FUNC(prisma_state::p4_w));
	m_maincpu->read_port5().set(FUNC(prisma_state::p5_r));
	m_maincpu->write_port5().set(FUNC(prisma_state::p5_w));
	m_maincpu->read_port6().set(FUNC(prisma_state::p6_r));
	m_maincpu->write_port6().set(FUNC(prisma_state::p6_w));
	m_maincpu->read_port7().set(FUNC(prisma_state::p7_r));
	m_maincpu->write_port7().set(FUNC(prisma_state::p7_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(2+3, 16);
	//config.set_default_layout(layout_saitek_prisma);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( prisma )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD("90_saitek_86051150st9_3258l02p.u1", 0x0000, 0x8000, CRC(b6f8384f) SHA1(a4e8a4a45009c15bda1778512a87dea756aae6d8) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1990, prisma, 0,      0,      prisma,  prisma, prisma_state, empty_init, "Saitek", "Kasparov Prisma", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
