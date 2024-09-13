// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
// thanks-to:Sean Riddle
/*******************************************************************************

Applied Concepts Boris Diplomat

Hardware notes:
- F3870 MCU (Motorola SC80265P or Fairchild SL90259)
- 256 bytes RAM(2*2112-1)
- 8-digit 7seg led panel

Two versions exist, a blue one(seen with SC80265P) and a brown one(seen with
either SC80265P or SL90259). The one emulated here is from a brown version with
the SC80265P. Motorola SC80265P is a 3870 clone, it's assumed that the program
is the same as SL90259.

*******************************************************************************/

#include "emu.h"

#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "video/pwm.h"

// internal artwork
#include "aci_borisdpl.lh"


namespace {

class borisdpl_state : public driver_device
{
public:
	borisdpl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void borisdpl(machine_config &config);

	// reset button is tied to MCU RESET pin
	DECLARE_INPUT_CHANGED_MEMBER(reset_button) { m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE); }

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_ioport_array<4> m_inputs;

	std::unique_ptr<u8[]> m_ram;
	u8 m_ram_address = 0;
	u8 m_matrix = 0;
	u8 m_digit_data = 0;

	void main_map(address_map &map);
	void main_io(address_map &map);

	void update_display();
	void digit_w(u8 data);
	u8 input_r();
	void matrix_w(u8 data);

	// 256 bytes data RAM accessed via I/O ports
	u8 ram_address_r() { return m_ram_address; }
	void ram_address_w(u8 data) { m_ram_address = data; }
	u8 ram_data_r() { return m_ram[m_ram_address]; }
	void ram_data_w(u8 data) { m_ram[m_ram_address] = data; }
};

void borisdpl_state::machine_start()
{
	m_ram = make_unique_clear<u8[]>(0x100);

	// register for savestates
	save_pointer(NAME(m_ram), 0x100);
	save_item(NAME(m_ram_address));
	save_item(NAME(m_matrix));
	save_item(NAME(m_digit_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// F3870 ports

void borisdpl_state::update_display()
{
	m_display->matrix(1 << (m_matrix & 7), m_digit_data);
}

void borisdpl_state::digit_w(u8 data)
{
	// digit segments
	m_digit_data = ~data & 0x7f;
	update_display();
}

void borisdpl_state::matrix_w(u8 data)
{
	// d0-d2: MC14028B to input/digit select
	m_matrix = data;
	update_display();
}

u8 borisdpl_state::input_r()
{
	// d4-d7: multiplexed inputs (only one lane can be selected at the same time)
	u8 data = m_matrix;
	if ((m_matrix & 7) < 4)
		data |= m_inputs[m_matrix & 3]->read() << 4;

	return data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void borisdpl_state::main_map(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x0000, 0x07ff).rom();
}

void borisdpl_state::main_io(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(borisdpl_state::input_r), FUNC(borisdpl_state::matrix_w));
	map(0x01, 0x01).w(FUNC(borisdpl_state::digit_w));
	map(0x04, 0x07).rw("psu", FUNC(f38t56_device::read), FUNC(f38t56_device::write));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( borisdpl )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_CODE(KEYCODE_MINUS) PORT_NAME("-")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("B/W") // black/white
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A.1 / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B.2 / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("C.3 / Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Rank")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("D.4 / Rook")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("E.5 / Queen")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("F.6 / King")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Time")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G.7")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("H.8")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9 / Set")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE") // clear entry

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, borisdpl_state, reset_button, 0) PORT_NAME("Reset")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void borisdpl_state::borisdpl(machine_config &config)
{
	// basic machine hardware
	F8(config, m_maincpu, 3'000'000/2); // frequency approximated from video reference
	m_maincpu->set_addrmap(AS_PROGRAM, &borisdpl_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &borisdpl_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("psu", FUNC(f38t56_device::int_acknowledge));

	f38t56_device &psu(F38T56(config, "psu", 3'000'000/2));
	psu.set_int_vector(0x5020);
	psu.int_req_callback().set_inputline("maincpu", F8_INPUT_LINE_INT_REQ);
	psu.read_a().set(FUNC(borisdpl_state::ram_data_r));
	psu.write_a().set(FUNC(borisdpl_state::ram_data_w));
	psu.read_b().set(FUNC(borisdpl_state::ram_address_r));
	psu.write_b().set(FUNC(borisdpl_state::ram_address_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 7);
	m_display->set_segmask(0xff, 0x7f);
	config.set_default_layout(layout_aci_borisdpl);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( borisdpl )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("007-7024-00_7847.u8", 0x0000, 0x0800, CRC(e20bac03) SHA1(9e17b9d90522371fbf7018926356150f70b9a3b6) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1979, borisdpl, 0,      0,      borisdpl, borisdpl, borisdpl_state, empty_init, "Applied Concepts", "Boris Diplomat", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
