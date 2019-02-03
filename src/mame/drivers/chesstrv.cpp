// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/******************************************************************************

    Acetronic Chess Traveller

    TODO:
    - Add emulation of the 3870 MCU to the F8 core, including timer interrupt
      that is used by the Boris Diplomat.

******************************************************************************/

#include "emu.h"
#include "cpu/f8/f8.h"
#include "machine/timer.h"
#include "chesstrv.lh"
#include "borisdpl.lh"

class chesstrv_base_state : public driver_device
{
protected:
	chesstrv_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	virtual void machine_start() override;

	DECLARE_READ8_MEMBER(ram_addr_r);
	DECLARE_WRITE8_MEMBER(ram_addr_w);
	DECLARE_READ8_MEMBER(ram_r);
	DECLARE_WRITE8_MEMBER(ram_w);
	DECLARE_WRITE8_MEMBER(matrix_w);

	void chesstrv_mem(address_map &map);

	uint8_t m_ram_addr;
	uint8_t *m_ram;
	uint8_t m_matrix;
	required_device<cpu_device> m_maincpu;
};

class chesstrv_state : public chesstrv_base_state
{
public:
	chesstrv_state(const machine_config &mconfig, device_type type, const char *tag)
		: chesstrv_base_state(mconfig, type, tag)
		, m_digits(*this, "digit%u", 0U)
		, m_keypad(*this, "LINE%u", 1U)
	{
	}

	void chesstrv(machine_config &config);
protected:
	virtual void machine_start() override;
private:
	void chesstrv_io(address_map &map);

	DECLARE_WRITE8_MEMBER(display_w);
	DECLARE_READ8_MEMBER(keypad_r);

	output_finder<4> m_digits;
	required_ioport_array<4> m_keypad;
};

class borisdpl_state : public chesstrv_base_state
{
public:
	borisdpl_state(const machine_config &mconfig, device_type type, const char *tag)
		: chesstrv_base_state(mconfig, type, tag)
		, m_digits(*this, "digit%u", 0U)
		, m_keypad(*this, "LINE%u", 1U)
	{
	}

	void borisdpl(machine_config &config);
protected:
	virtual void machine_start() override;
private:
	void borisdpl_io(address_map &map);

	DECLARE_WRITE8_MEMBER(display_w);
	DECLARE_READ8_MEMBER(keypad_r);

	//TIMER_DEVICE_CALLBACK_MEMBER(timer_interrupt);

	output_finder<8> m_digits;
	required_ioport_array<4> m_keypad;
};

WRITE8_MEMBER(chesstrv_base_state::ram_addr_w)
{
	m_ram_addr = data;
}

READ8_MEMBER(chesstrv_base_state::ram_addr_r)
{
	return m_ram_addr;
}

READ8_MEMBER(chesstrv_base_state::ram_r)
{
	return m_ram[m_ram_addr];
}

WRITE8_MEMBER(chesstrv_base_state::ram_w)
{
	m_ram[m_ram_addr] = data;
}

WRITE8_MEMBER(chesstrv_state::display_w)
{
	uint8_t seg_data = bitswap<8>(data,0,1,2,3,4,5,6,7);

	for (int digit = 0; digit < 4; digit++)
		if (!BIT(m_matrix, 3 - digit))
			m_digits[digit] = seg_data;
}

WRITE8_MEMBER(chesstrv_base_state::matrix_w)
{
	m_matrix = data;
}

READ8_MEMBER(chesstrv_state::keypad_r)
{
	uint8_t data = 0;

	data |= m_keypad[0]->read();
	data |= m_keypad[1]->read();
	data |= m_keypad[2]->read();
	data |= m_keypad[3]->read();
	data |= (m_keypad[0]->read() ? 0x10 : 0);
	data |= (m_keypad[1]->read() ? 0x20 : 0);
	data |= (m_keypad[2]->read() ? 0x40 : 0);
	data |= (m_keypad[3]->read() ? 0x80 : 0);

	return data;
}

WRITE8_MEMBER(borisdpl_state::display_w)
{
	m_digits[m_matrix & 7] = data ^ 0xff;
}

READ8_MEMBER(borisdpl_state::keypad_r)
{
	uint8_t data = m_matrix & 0x07;

	switch (m_matrix & 7)
	{
		case 0:     data |= m_keypad[0]->read();    break;
		case 1:     data |= m_keypad[1]->read();    break;
		case 2:     data |= m_keypad[2]->read();    break;
		case 3:     data |= m_keypad[3]->read();    break;
	}

	return data;
}


void chesstrv_base_state::chesstrv_mem(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x0000, 0x07ff).rom();
}


void chesstrv_state::chesstrv_io(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(chesstrv_state::ram_addr_r), FUNC(chesstrv_state::ram_addr_w));
	map(0x01, 0x01).w(FUNC(chesstrv_state::display_w));
	map(0x04, 0x04).rw(FUNC(chesstrv_state::ram_r), FUNC(chesstrv_state::ram_w));
	map(0x05, 0x05).rw(FUNC(chesstrv_state::keypad_r), FUNC(chesstrv_state::matrix_w));
}

void borisdpl_state::borisdpl_io(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(borisdpl_state::keypad_r), FUNC(borisdpl_state::matrix_w));
	map(0x01, 0x01).w(FUNC(borisdpl_state::display_w));
	map(0x04, 0x04).rw(FUNC(borisdpl_state::ram_r), FUNC(borisdpl_state::ram_w));
	map(0x05, 0x05).rw(FUNC(borisdpl_state::ram_addr_r), FUNC(borisdpl_state::ram_addr_w));
}

static INPUT_PORTS_START( chesstrv )
	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A1")  PORT_CODE(KEYCODE_A)    PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B2")  PORT_CODE(KEYCODE_B)    PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C3")  PORT_CODE(KEYCODE_C)    PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D4")  PORT_CODE(KEYCODE_D)    PORT_CODE(KEYCODE_4)
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E5")  PORT_CODE(KEYCODE_E)    PORT_CODE(KEYCODE_5)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F6")  PORT_CODE(KEYCODE_F)    PORT_CODE(KEYCODE_6)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G7")  PORT_CODE(KEYCODE_G)    PORT_CODE(KEYCODE_7)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H8")  PORT_CODE(KEYCODE_H)    PORT_CODE(KEYCODE_8)
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("LV")  PORT_CODE(KEYCODE_L)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("FP")  PORT_CODE(KEYCODE_K)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("EP")  PORT_CODE(KEYCODE_O)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CB")  PORT_CODE(KEYCODE_Q)
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CE")  PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("ENTER")   PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("MM")  PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( borisdpl )
	PORT_START("LINE1")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("0")       PORT_CODE(KEYCODE_0)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("-")       PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B/W")     PORT_CODE(KEYCODE_W)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("ENTER")   PORT_CODE(KEYCODE_ENTER)

	PORT_START("LINE2")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A1")      PORT_CODE(KEYCODE_A)    PORT_CODE(KEYCODE_1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B2")      PORT_CODE(KEYCODE_B)    PORT_CODE(KEYCODE_2)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C3")      PORT_CODE(KEYCODE_C)    PORT_CODE(KEYCODE_3)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RANK")    PORT_CODE(KEYCODE_R)

	PORT_START("LINE3")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D4")      PORT_CODE(KEYCODE_D)    PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E5")      PORT_CODE(KEYCODE_E)    PORT_CODE(KEYCODE_5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F6")      PORT_CODE(KEYCODE_F)    PORT_CODE(KEYCODE_6)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("TIME")    PORT_CODE(KEYCODE_T)

	PORT_START("LINE4")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G7")      PORT_CODE(KEYCODE_G)    PORT_CODE(KEYCODE_7)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H8")      PORT_CODE(KEYCODE_H)    PORT_CODE(KEYCODE_8)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("9/SET")   PORT_CODE(KEYCODE_S)    PORT_CODE(KEYCODE_9)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CE")      PORT_CODE(KEYCODE_DEL)

INPUT_PORTS_END

/*
TIMER_DEVICE_CALLBACK_MEMBER(borisdpl_state::timer_interrupt)
{
    m_maincpu->set_input_line_and_vector(F8_INPUT_LINE_INT_REQ, HOLD_LINE, 0x20);
}
*/

void chesstrv_base_state::machine_start()
{
	m_ram = memregion("ram")->base();

	save_item(NAME(m_ram_addr));
	save_item(NAME(m_matrix));
}

void chesstrv_state::machine_start()
{
	chesstrv_base_state::machine_start();
	m_digits.resolve();
}

void borisdpl_state::machine_start()
{
	chesstrv_base_state::machine_start();
	m_digits.resolve();
}

MACHINE_CONFIG_START(chesstrv_state::chesstrv)
	/* basic machine hardware */
	MCFG_DEVICE_ADD( "maincpu", F8, 3000000 )      // Fairchild 3870
	MCFG_DEVICE_PROGRAM_MAP( chesstrv_mem )
	MCFG_DEVICE_IO_MAP( chesstrv_io )

	/* video hardware */
	config.set_default_layout(layout_chesstrv);
MACHINE_CONFIG_END

MACHINE_CONFIG_START(borisdpl_state::borisdpl)
	/* basic machine hardware */
	MCFG_DEVICE_ADD( "maincpu", F8, 30000000 )     // Motorola SC80265P
	MCFG_DEVICE_PROGRAM_MAP( chesstrv_mem )
	MCFG_DEVICE_IO_MAP( borisdpl_io )

	/* video hardware */
	config.set_default_layout(layout_borisdpl);

	//TIMER(config, "timer_interrupt").configure_periodic(FUNC(borisdpl_state::timer_interrupt), attotime::from_hz(40));
MACHINE_CONFIG_END


ROM_START( chesstrv )
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("3870-sl90387", 0x0000, 0x0800, CRC(b76214d8) SHA1(7760903a64d9c513eb54c4787f535dabec62eb64))

	ROM_REGION(0x0100, "ram", ROMREGION_ERASE)
ROM_END

ROM_START( boris )
	ROM_REGION(0x0c00, "maincpu", 0)
	ROM_LOAD("007-7020-00_c10502_korea.bin", 0x0000, 0x0800, CRC(18182870) SHA1(cb717a4b5269b04b0d7ae61aaf4a8f6a019626a5))
	ROM_LOAD("007-7021-00_c10503_korea.bin", 0x0800, 0x0400, CRC(49b77505) SHA1(474b665ee2955497f6d70878d817f1783ba1a835))

	ROM_REGION(0x0100, "ram", ROMREGION_ERASE)
ROM_END

ROM_START( borisdpl )
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("007-7024-00_7847.u8", 0x0000, 0x0800, CRC(e20bac03) SHA1(9e17b9d90522371fbf7018926356150f70b9a3b6))

	ROM_REGION(0x0100, "ram", ROMREGION_ERASE)
ROM_END


//    YEAR   NAME      PARENT  COMPAT  MACHINE   INPUT     STATE           INIT        COMPANY             FULLNAME           FLAGS
CONS( 1980,  chesstrv, 0,      0,      chesstrv, chesstrv, chesstrv_state, empty_init, "Acetronic",        "Chess Traveller", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
CONS( 1979,  boris,    0,      0,      borisdpl, borisdpl, borisdpl_state, empty_init, "Applied Concepts", "Boris - Electronic Chess Computer",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
CONS( 1979,  borisdpl, 0,      0,      borisdpl, borisdpl, borisdpl_state, empty_init, "Applied Concepts", "Boris Diplomat",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
