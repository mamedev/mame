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
#include "chesstrv.lh"
#include "borisdpl.lh"

class chesstrv_state : public driver_device
{
public:
	chesstrv_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu") { }

	virtual void machine_start() override;

	DECLARE_READ8_MEMBER( ram_addr_r );
	DECLARE_WRITE8_MEMBER( ram_addr_w );
	DECLARE_READ8_MEMBER( ram_r );
	DECLARE_WRITE8_MEMBER( ram_w );
	DECLARE_WRITE8_MEMBER( display_w );
	DECLARE_WRITE8_MEMBER( matrix_w );
	DECLARE_READ8_MEMBER( keypad_r );
	DECLARE_WRITE8_MEMBER( diplomat_display_w );
	DECLARE_READ8_MEMBER( diplomat_keypad_r );

	UINT8 m_ram_addr;
	UINT8 *m_ram;
	UINT8 m_matrix;
	//TIMER_DEVICE_CALLBACK_MEMBER(borisdpl_timer_interrupt);
	required_device<cpu_device> m_maincpu;
};

WRITE8_MEMBER( chesstrv_state::ram_addr_w )
{
	m_ram_addr = data;
}

READ8_MEMBER( chesstrv_state::ram_addr_r )
{
	return m_ram_addr;
}

READ8_MEMBER( chesstrv_state::ram_r )
{
	return m_ram[m_ram_addr];
}

WRITE8_MEMBER( chesstrv_state::ram_w )
{
	m_ram[m_ram_addr] = data;
}

WRITE8_MEMBER( chesstrv_state::display_w )
{
	UINT8 seg_data = BITSWAP8(data,0,1,2,3,4,5,6,7);

	if(!(m_matrix & 0x01))
		output().set_digit_value( 3, seg_data );
	if(!(m_matrix & 0x02))
		output().set_digit_value( 2, seg_data );
	if(!(m_matrix & 0x04))
		output().set_digit_value( 1, seg_data );
	if(!(m_matrix & 0x08))
		output().set_digit_value( 0, seg_data );
}

WRITE8_MEMBER( chesstrv_state::matrix_w )
{
	m_matrix = data;
}

READ8_MEMBER( chesstrv_state::keypad_r )
{
	UINT8 data = 0;

	data |= ioport("LINE1")->read();
	data |= ioport("LINE2")->read();
	data |= ioport("LINE3")->read();
	data |= ioport("LINE4")->read();
	data |= (ioport("LINE1")->read() ? 0x10 : 0);
	data |= (ioport("LINE2")->read() ? 0x20 : 0);
	data |= (ioport("LINE3")->read() ? 0x40 : 0);
	data |= (ioport("LINE4")->read() ? 0x80 : 0);

	return data;
}

WRITE8_MEMBER( chesstrv_state::diplomat_display_w )
{
	output().set_digit_value( m_matrix & 7, data ^ 0xff );
}

READ8_MEMBER( chesstrv_state::diplomat_keypad_r )
{
	UINT8 data = m_matrix & 0x07;

	switch (m_matrix & 7)
	{
		case 0:     data |= ioport("LINE1")->read();    break;
		case 1:     data |= ioport("LINE2")->read();    break;
		case 2:     data |= ioport("LINE3")->read();    break;
		case 3:     data |= ioport("LINE4")->read();    break;
	}

	return data;
}


static ADDRESS_MAP_START( chesstrv_mem, AS_PROGRAM, 8, chesstrv_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE( 0x0000, 0x07ff ) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( chesstrv_io, AS_IO, 8, chesstrv_state )
	AM_RANGE( 0x00, 0x00 ) AM_READWRITE( ram_addr_r, ram_addr_w )
	AM_RANGE( 0x01, 0x01 ) AM_WRITE( display_w )
	AM_RANGE( 0x04, 0x04 ) AM_READWRITE( ram_r, ram_w )
	AM_RANGE( 0x05, 0x05 ) AM_READWRITE( keypad_r, matrix_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( borisdpl_io, AS_IO, 8, chesstrv_state )
	AM_RANGE( 0x00, 0x00 ) AM_READWRITE( diplomat_keypad_r, matrix_w )
	AM_RANGE( 0x01, 0x01 ) AM_WRITE( diplomat_display_w )
	AM_RANGE( 0x04, 0x04 ) AM_READWRITE( ram_r, ram_w )
	AM_RANGE( 0x05, 0x05 ) AM_READWRITE( ram_addr_r, ram_addr_w )
ADDRESS_MAP_END

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
TIMER_DEVICE_CALLBACK_MEMBER(chesstrv_state::borisdpl_timer_interrupt)
{
    m_maincpu->set_input_line_and_vector(F8_INPUT_LINE_INT_REQ, HOLD_LINE, 0x20);
}
*/

void chesstrv_state::machine_start()
{
	m_ram = memregion("ram")->base();

	save_item(NAME(m_ram_addr));
	save_item(NAME(m_matrix));
}

static MACHINE_CONFIG_START( chesstrv, chesstrv_state )
	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", F8, 3000000 )      // Fairchild 3870
	MCFG_CPU_PROGRAM_MAP( chesstrv_mem )
	MCFG_CPU_IO_MAP( chesstrv_io )

	/* video hardware */
	MCFG_DEFAULT_LAYOUT( layout_chesstrv )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( borisdpl, chesstrv_state )
	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", F8, 30000000 )     // Motorola SC80265P
	MCFG_CPU_PROGRAM_MAP( chesstrv_mem )
	MCFG_CPU_IO_MAP( borisdpl_io )

	/* video hardware */
	MCFG_DEFAULT_LAYOUT( layout_borisdpl )

	//MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_interrupt", chesstrv_state, borisdpl_timer_interrupt, attotime::from_hz(40))
MACHINE_CONFIG_END


ROM_START( chesstrv )
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("3870-sl90387", 0x0000, 0x0800, CRC(b76214d8) SHA1(7760903a64d9c513eb54c4787f535dabec62eb64))

	ROM_REGION(0x0100, "ram", ROMREGION_ERASE)
ROM_END

ROM_START( borisdpl )
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("007-7024-00_7847.u8", 0x0000, 0x0800, CRC(e20bac03) SHA1(9e17b9d90522371fbf7018926356150f70b9a3b6))

	ROM_REGION(0x0100, "ram", ROMREGION_ERASE)
ROM_END


/*    YEAR   NAME  PARENT  COMPAT  MACHINE INPUT   INIT    COMPANY                 FULLNAME */
CONS( 1980,  chesstrv,  0,      0,      chesstrv,    chesstrv, driver_device,    0,   "Acetronic", "Chess Traveller", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
CONS( 1979,  borisdpl,  0,      0,      borisdpl,    borisdpl, driver_device,    0,   "Applied Concepts",  "Boris Diplomat",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
