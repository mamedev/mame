// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
// thanks-to:Sean Riddle
/******************************************************************************

Applied Concepts Boris Diplomat

- F3870 MCU (Motorola SC80265P or Fairchild SL90259)
- 256 bytes RAM(2*2112-1)
- 8-digit 7seg led panel

Two versions exists, a blue one(seen with SC80265P) and a brown one(seen with
either MCU). The one emulated here is from a brown version with the SC80265P.
Motorola SC80265P is a 3870 clone, it's assumed that the program is the same
as SL90259.

******************************************************************************/

#include "emu.h"
#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "machine/timer.h"

// internal artwork
#include "aci_borisdpl.lh" // clickable


namespace {

class borisdpl_state : public driver_device
{
public:
	borisdpl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_keypad(*this, "LINE%u", 1U),
		m_delay_display(*this, "delay_display_%u", 0),
		m_out_digit(*this, "digit%u", 0U)
	{ }

	void borisdpl(machine_config &config);

	// reset button is tied to MCU RESET pin
	DECLARE_INPUT_CHANGED_MEMBER(reset_button) { m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE); }

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_ioport_array<4> m_keypad;
	required_device_array<timer_device, 8> m_delay_display;
	output_finder<8> m_out_digit;

	void main_map(address_map &map);
	void main_io(address_map &map);

	TIMER_DEVICE_CALLBACK_MEMBER(delay_display);

	DECLARE_WRITE8_MEMBER(digit_w);
	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE8_MEMBER(matrix_w);

	// 256 bytes data RAM accessed via I/O ports
	DECLARE_READ8_MEMBER(ram_address_r) { return m_ram_address; }
	DECLARE_WRITE8_MEMBER(ram_address_w) { m_ram_address = data; }
	DECLARE_READ8_MEMBER(ram_data_r) { return m_ram[m_ram_address]; }
	DECLARE_WRITE8_MEMBER(ram_data_w) { m_ram[m_ram_address] = data; }

	std::unique_ptr<u8[]> m_ram;
	u8 m_ram_address;
	u8 m_matrix;
};

void borisdpl_state::machine_start()
{
	// resolve handlers
	m_out_digit.resolve();

	// zerofill
	m_ram = make_unique_clear<u8[]>(0x100);
	m_ram_address = 0;
	m_matrix = 0;

	// register for savestates
	save_pointer(NAME(m_ram), 0x100);
	save_item(NAME(m_ram_address));
	save_item(NAME(m_matrix));
}



/******************************************************************************
    Devices, I/O
******************************************************************************/

// F3870 ports

TIMER_DEVICE_CALLBACK_MEMBER(borisdpl_state::delay_display)
{
	// clear digits if inactive
	if (param != (m_matrix & 7))
		m_out_digit[param] = 0;
}

WRITE8_MEMBER(borisdpl_state::digit_w)
{
	// digit segments, update display here
	m_out_digit[m_matrix & 7] = ~data & 0x7f;
}

WRITE8_MEMBER(borisdpl_state::matrix_w)
{
	// d0-d2: MC14028B to input/digit select
	// digits are strobed, so on falling edge, delay them going off to prevent flicker or stuck display
	if ((data & 7) != (m_matrix & 7))
		m_delay_display[m_matrix & 7]->adjust(attotime::from_msec(20), m_matrix & 7);

	m_matrix = data;
}

READ8_MEMBER(borisdpl_state::input_r)
{
	// d4-d7: multiplexed inputs (only one lane can be selected at the same time)
	u8 data = m_matrix;
	if ((m_matrix & 7) < 4)
		data |= m_keypad[m_matrix & 3]->read() << 4;

	return data;
}



/******************************************************************************
    Address Maps
******************************************************************************/

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



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( borisdpl )
	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_CODE(KEYCODE_MINUS) PORT_NAME("-")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("B/W") // black/white
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A.1 / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B.2 / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("C.3 / Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Rank")

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("D.4 / Rook")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("E.5 / Queen")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("F.6 / King")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Time")

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G.7")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("H.8")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9 / Set")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE") // clear entry

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, borisdpl_state, reset_button, nullptr) PORT_NAME("Reset")
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void borisdpl_state::borisdpl(machine_config &config)
{
	/* basic machine hardware */
	F8(config, m_maincpu, 3000000/2); // frequency approximated from video reference
	m_maincpu->set_addrmap(AS_PROGRAM, &borisdpl_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &borisdpl_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("psu", FUNC(f38t56_device::int_acknowledge));

	f38t56_device &psu(F38T56(config, "psu", 3000000/2));
	psu.set_int_vector(0x5020);
	psu.int_req_callback().set_inputline("maincpu", F8_INPUT_LINE_INT_REQ);
	psu.read_a().set(FUNC(borisdpl_state::ram_data_r));
	psu.write_a().set(FUNC(borisdpl_state::ram_data_w));
	psu.read_b().set(FUNC(borisdpl_state::ram_address_r));
	psu.write_b().set(FUNC(borisdpl_state::ram_address_w));

	/* video hardware */
	for (int i = 0; i < 8; i++)
		TIMER(config, m_delay_display[i]).configure_generic(FUNC(borisdpl_state::delay_display));

	config.set_default_layout(layout_aci_borisdpl);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( borisdpl )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("007-7024-00_7847.u8", 0x0000, 0x0800, CRC(e20bac03) SHA1(9e17b9d90522371fbf7018926356150f70b9a3b6) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT CMP MACHINE   INPUT     STATE           INIT        COMPANY, FULLNAME, FLAGS
CONS( 1979, borisdpl, 0,      0, borisdpl, borisdpl, borisdpl_state, empty_init, "Applied Concepts", "Boris Diplomat", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
