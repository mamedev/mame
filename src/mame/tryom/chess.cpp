// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Tryom Electronic Chess (model CC-700)

Hardware notes:
- PCB label: 170-313-01, JAB, TRYOM
- Fairchild 3870 MCU, label SL90453
- 256x4 RAM (SCM5101E-8)
- 9-digit 7seg led panel (3 unused), piezo

The hardware has similarities with Omar I.

The user interface resembles CompuChess. At the "L" prompt, enter level (1-8).
At "bP", press A for new game, B for empty board, C to continue. At "0", press
1-4 for an opening book, or 5 to select one at random.

BTANB:
- it locks up after pressing CE while it's thinking

*******************************************************************************/

#include "emu.h"

#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

#include "tchess.lh"


namespace {

class chess_state : public driver_device
{
public:
	chess_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_psu(*this, "psu"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void tchess(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<f38t56_device> m_psu;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<5> m_inputs;

	std::unique_ptr<u8[]> m_ram;
	u8 m_ram_address = 0;
	u8 m_ram_data = 0;
	u8 m_inp_mux = 0;
	u8 m_digit_data = 0;

	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	u8 read_inputs();
	void p0_w(u8 data);
	u8 p0_r();
	void p1_w(u8 data);
	u8 p1_r();
	void p4_w(u8 data);
	u8 p4_r();
	void p5_w(u8 data);
	u8 p5_r();
};

void chess_state::machine_start()
{
	m_ram = make_unique_clear<u8[]>(0x100);

	// register for savestates
	save_pointer(NAME(m_ram), 0x100);
	save_item(NAME(m_ram_address));
	save_item(NAME(m_ram_data));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_digit_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

u8 chess_state::read_inputs()
{
	u8 data = 0;

	for (int i = 0; i < 5; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	return data;
}

void chess_state::p0_w(u8 data)
{
	// P00-P05: digit select, input mux
	// P06: RAM CE1
	m_display->write_my(~data & 0x3f);
	m_inp_mux = data;
}

u8 chess_state::p0_r()
{
	// P07: multiplexed inputs (high)
	return (read_inputs() << 3 & 0x80) | m_inp_mux;
}

void chess_state::p1_w(u8 data)
{
	// P10-P17: RAM address
	m_ram_address = data;
}

u8 chess_state::p1_r()
{
	return m_ram_address;
}

void chess_state::p4_w(u8 data)
{
	// P40-P43: RAM DI
	if (m_inp_mux & 0x40)
		m_ram[m_ram_address] = data & 0xf;

	m_ram_data = data;
}

u8 chess_state::p4_r()
{
	// P44-P47: multiplexed inputs (low), RAM DO
	u8 data = read_inputs();
	if (m_inp_mux & 0x40)
		data |= m_ram[m_ram_address];

	return data << 4 | m_ram_data;
}

void chess_state::p5_w(u8 data)
{
	// P50-P56: digit data
	m_display->write_mx(bitswap<7>(~data,0,1,5,4,6,2,3));
	m_digit_data = data;

	// P57: speaker out
	m_dac->write(BIT(data, 7));
}

u8 chess_state::p5_r()
{
	return m_digit_data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void chess_state::main_map(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x0000, 0x07ff).rom();
}

void chess_state::main_io(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(chess_state::p0_r), FUNC(chess_state::p0_w));
	map(0x01, 0x01).rw(FUNC(chess_state::p1_r), FUNC(chess_state::p1_w));
	map(0x04, 0x07).rw(m_psu, FUNC(f38t56_device::read), FUNC(f38t56_device::write));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( tchess )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("XMV")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("FP")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("EP")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("PLY")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("RST")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("A / WK")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("B / WQ")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("C / WB")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("D / WN")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("WSD")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("E / WR")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("F / WP")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("G")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("H")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("BSD")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1 / BK")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2 / BQ")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3 / BB")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4 / BN")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5 / BR")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6 / BP")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void chess_state::tchess(machine_config &config)
{
	// basic machine hardware
	F8(config, m_maincpu, 4500000/2); // approximation
	m_maincpu->set_addrmap(AS_PROGRAM, &chess_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &chess_state::main_io);
	m_maincpu->set_irq_acknowledge_callback(m_psu, FUNC(f38t56_device::int_acknowledge));

	F38T56(config, m_psu, 4500000/2);
	m_psu->set_int_vector(0x20);
	m_psu->int_req_callback().set_inputline(m_maincpu, F8_INPUT_LINE_INT_REQ);
	m_psu->read_a().set(FUNC(chess_state::p4_r));
	m_psu->write_a().set(FUNC(chess_state::p4_w));
	m_psu->read_b().set(FUNC(chess_state::p5_r));
	m_psu->write_b().set(FUNC(chess_state::p5_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(6, 7);
	m_display->set_segmask(0x3f, 0x7f);
	config.set_default_layout(layout_tchess);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( tchess )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("sl90453", 0x0000, 0x0800, CRC(f7fbe9b0) SHA1(d79ae43acfdf733908bc57b1fcca2563a2fdf48e) ) // 3870X-0453
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS        INIT        COMPANY, FULLNAME, FLAGS
SYST( 1981, tchess, 0,      0,      tchess,  tchess, chess_state, empty_init, "Tryom", "Electronic Chess (Tryom)", MACHINE_SUPPORTS_SAVE )
