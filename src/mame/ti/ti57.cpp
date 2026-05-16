// license:BSD-3-Clause
// copyright-holders:baco, Antigravity
/***************************************************************************

    Texas Instruments TI-57 Programmable Calculator Driver

    Main CPU: TMC1501 (TMC1500 family)

    Hardware implementation details:
    - 12-digit LED display (multiplexed via DISP instruction).
    - Keyboard matrix scanned using digit strobe lines D0-D7.
    - 5 kHz instruction cycle rate.

***************************************************************************/

#include "emu.h"
#include "cpu/tmc1500/tmc1500.h"

class ti57_state : public driver_device
{
public:
	ti57_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_inputs(*this, "IN%u", 0)
	{ }

	void ti57(machine_config &config);
	void ti57_program_map(address_map &map);

protected:
	virtual void machine_start() override;

	uint8_t read_k(offs_t offset);
	void write_o(offs_t offset, uint16_t data, uint16_t mem_mask);
	void write_r(uint32_t data);

private:
	required_device<tmc1500_base_device> m_maincpu;
	required_ioport_array<8> m_inputs;

	uint16_t m_main_o;
	uint8_t m_scan_idx;
};

void ti57_state::machine_start()
{
	save_item(NAME(m_main_o));
	save_item(NAME(m_scan_idx));
}

uint8_t ti57_state::read_k(offs_t offset)
{
	// CPU scans digits 0-15. TI-57 keyboard strobes map IN0-IN7 to D7-D0.
	// Index mapping: 0 -> D0 -> IN7, 1 -> D1 -> IN6, ..., 7 -> D7 -> IN0.
	if (offset < 8)
		return m_inputs[7 - offset]->read();
	return 0;
}

void ti57_state::write_o(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset == 0x100)
	{
		// End of scan / blank all digits
		for (int i = 0; i < 12; i++)
			output().set_digit_value(i, 0);
		return;
	}

	if (offset < 12)
	{
		static const uint8_t s_7seg_decode[16] = {
			0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 
			0x7f, 0x6f, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00 
		};

		uint8_t segments = s_7seg_decode[data & 0x0f];
		if (data & 0x10) segments |= 0x80; // Decimal point

		output().set_digit_value(offset, segments);
	}
}

void ti57_state::write_r(uint32_t data)
{
	// TI-57 doesn't use external R registers for major logic
}

void ti57_state::ti57_program_map(address_map &map)
{
	map(0x000, 0x7ff).rom();
}

static INPUT_PORTS_START( ti57 )
	PORT_START("IN0") // D7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("2nd")   PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INV")   PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("lnx")   PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("CE")    PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("CLR")   PORT_CODE(KEYCODE_C)

	PORT_START("IN1") // D6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("LRN")   PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("x:y")   PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("x^2")   PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("sqrt")  PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("1/x")   PORT_CODE(KEYCODE_X)

	PORT_START("IN2") // D5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("SST")   PORT_CODE(KEYCODE_PAGEUP)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("STO")   PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("RCL")   PORT_CODE(KEYCODE_P)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("SUM")   PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("y^x")   PORT_CODE(KEYCODE_H)

	PORT_START("IN3") // D4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("BST")   PORT_CODE(KEYCODE_PAGEDOWN)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("EE")    PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("(")     PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME(")")     PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("/")     PORT_CODE(KEYCODE_SLASH_PAD)

	PORT_START("IN4") // D3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("GTO")   PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("7")     PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("8")     PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("9")     PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("X")     PORT_CODE(KEYCODE_ASTERISK)

	PORT_START("IN5") // D2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("SBR")   PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("4")     PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("5")     PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("6")     PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("-")     PORT_CODE(KEYCODE_MINUS_PAD)

	PORT_START("IN6") // D1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("RST")   PORT_CODE(KEYCODE_HOME)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("1")     PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("2")     PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("3")     PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("+")     PORT_CODE(KEYCODE_PLUS_PAD)

	PORT_START("IN7") // D0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("0")     PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME(".")     PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("+/-")   PORT_CODE(KEYCODE_TAB)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("=")     PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("R/S")   PORT_CODE(KEYCODE_ENTER)
INPUT_PORTS_END

void ti57_state::ti57(machine_config &config)
{
	TMC1501(config, m_maincpu, 5000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ti57_state::ti57_program_map);
	m_maincpu->read_k().set(FUNC(ti57_state::read_k));
	m_maincpu->write_o().set(FUNC(ti57_state::write_o));
	m_maincpu->write_r().set(FUNC(ti57_state::write_r));
}

ROM_START( ti57 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ti57_rom.bin", 0x0000, 0x1000, CRC(f6ba51ce) SHA1(fb86bbe765003516d502aaa384ef65bb70743cbb) )
ROM_END

COMP( 1977, ti57, 0, 0, ti57, ti57, ti57_state, empty_init, "Texas Instruments", "TI-57", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
