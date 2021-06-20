// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/******************************************************************************

Lexicon LK-3000, a pocket computer.
It was also licensed to Nixdorf.

Hardware notes:
- 3870 MCU (Motorola brand) @ 4MHz(2MHz internal)
- optional extra ROM or RAM
- 4*Litronix DL 1414 (16*17segs total)
- 33-button keypad

The CPU and memory is on the cartridge. In theory, any hardware configuration
is possible, but it looks like they always used a 3870.

cartridge types known:
- CPU + 4KB ROM (ROM may be unpopulated)
- CPU + 2*4KB ROM
- CPU + 1KB battery-backed RAM (2*NEC D444)

******************************************************************************/

#include "emu.h"
#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "video/dl1416.h"

// internal artwork
#include "lk3000.lh"


namespace {

class lk3000_state : public driver_device
{
public:
	lk3000_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dl1414(*this, "dl1414_%u", 0),
		m_inputs(*this, "IN.%u", 0),
		m_digits(*this, "digit%u", 0U)
	{ }

	void lk3000(machine_config &config);

	// CLR button is tied to MCU RESET pin
	DECLARE_INPUT_CHANGED_MEMBER(reset_button) { m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE); }

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device_array<dl1414_device, 4> m_dl1414;
	required_ioport_array<8> m_inputs;
	output_finder<16> m_digits;

	void main_map(address_map &map);
	void main_io(address_map &map);

	template <int N> void update_display(offs_t offset, u16 data);

	u8 p0_r();
	void p0_w(u8 data);
	u8 p1_r();
	void p1_w(u8 data);
	u8 p4_r();
	void p4_w(u8 data);
	u8 p5_r();
	void p5_w(u8 data);

	u8 m_p0 = 0;
	u8 m_p1 = 0;
	u8 m_p4 = 0;
	u8 m_p5 = 0;
};

void lk3000_state::machine_start()
{
	m_digits.resolve();

	// register for savestates
	save_item(NAME(m_p0));
	save_item(NAME(m_p1));
	save_item(NAME(m_p4));
	save_item(NAME(m_p5));
}



/******************************************************************************
    I/O
******************************************************************************/

template <int N>
void lk3000_state::update_display(offs_t offset, u16 data)
{
	m_digits[N * 4 + offset] = data;
}

u8 lk3000_state::p0_r()
{
	return 0;
}

void lk3000_state::p0_w(u8 data)
{
}

u8 lk3000_state::p1_r()
{
	u8 data = 0;

	// P10-P13: multiplexed inputs
	if (m_p4 & 0x10)
		data |= m_inputs[~m_p4 & 7]->read();

	return data;
}

void lk3000_state::p1_w(u8 data)
{
	// P10-P15,_P15: DL1414 data
	for (int i = 0; i < 4; i++)
		m_dl1414[i]->data_w((~data & 0x3f) | (data << 1 & 0x40));

	m_p1 = data;
}

u8 lk3000_state::p4_r()
{
	return 0;
}

void lk3000_state::p4_w(u8 data)
{
	// P40,P41: DL1414 address
	for (int i = 0; i < 4; i++)
		m_dl1414[i]->addr_w(~data & 3);

	// P42,P43,GND,P45: 7442 to DL1414 _WR
	for (int i = 0; i < 4; i++)
		m_dl1414[i]->wr_w(BIT(1 << (~data >> 2 & 0xb), i));

	// P40,P41,P42,P44: 7442 to keypad mux
	m_p4 = data;
}

u8 lk3000_state::p5_r()
{
	return 0;
}

void lk3000_state::p5_w(u8 data)
{
}



/******************************************************************************
    Address Maps
******************************************************************************/

void lk3000_state::main_map(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x0000, 0x07ff).rom();
}

void lk3000_state::main_io(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(lk3000_state::p0_r), FUNC(lk3000_state::p0_w));
	map(0x01, 0x01).rw(FUNC(lk3000_state::p1_r), FUNC(lk3000_state::p1_w));
	map(0x04, 0x07).rw("psu", FUNC(f38t56_device::read), FUNC(f38t56_device::write));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( lk3000 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, lk3000_state, reset_button, 0) PORT_NAME("clr")
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void lk3000_state::lk3000(machine_config &config)
{
	// basic machine hardware
	F8(config, m_maincpu, 4_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &lk3000_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &lk3000_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("psu", FUNC(f38t56_device::int_acknowledge));

	auto &psu(F38T56(config, "psu", 4_MHz_XTAL/2));
	psu.set_int_vector(0x20);
	psu.int_req_callback().set_inputline("maincpu", F8_INPUT_LINE_INT_REQ);
	psu.read_a().set(FUNC(lk3000_state::p4_r));
	psu.write_a().set(FUNC(lk3000_state::p4_w));
	psu.read_b().set(FUNC(lk3000_state::p5_r));
	psu.write_b().set(FUNC(lk3000_state::p5_w));

	// video hardware
	DL1414T(config, m_dl1414[0], u32(0)).update().set(FUNC(lk3000_state::update_display<0>));
	DL1414T(config, m_dl1414[1], u32(0)).update().set(FUNC(lk3000_state::update_display<1>));
	DL1414T(config, m_dl1414[2], u32(0)).update().set(FUNC(lk3000_state::update_display<2>));
	DL1414T(config, m_dl1414[3], u32(0)).update().set(FUNC(lk3000_state::update_display<3>));
	config.set_default_layout(layout_lk3000);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( lk3000 )
	ROM_REGION( 0x0800, "maincpu", ROMREGION_ERASE00 )
	// none here, it's in the module slot
	//ROM_LOAD("sc80249p_lk3900m", 0x0000, 0x0800, CRC(192edfcf) SHA1(527216b6e15b9ab5f83742f437a5a8d8eef14ce2) ) // will be moved to swlist
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME    PARENT  COMP  MACHINE  INPUT   STATE         INIT        COMPANY, FULLNAME, FLAGS
CONS( 1979, lk3000, 0,      0,    lk3000,  lk3000, lk3000_state, empty_init, "Lexicon", "LK-3000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
