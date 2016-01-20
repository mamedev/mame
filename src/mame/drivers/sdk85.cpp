// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        Intel SDK-85

        09/12/2009 Skeleton driver.

        22/06/2011 Working [Robbbert]

This is an evaluation kit for the 8085 cpu.

There is no speaker or storage facility in the standard kit.

Download the User Manual to get the operating procedures.

An example is Press SUBST key, enter an address, press NEXT key, enter data,
then press NEXT to increment the address.

ToDo:
- Artwork

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8279.h"
#include "sdk85.lh"


class sdk85_state : public driver_device
{
public:
	sdk85_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu") { }

	DECLARE_WRITE8_MEMBER(scanlines_w);
	DECLARE_WRITE8_MEMBER(digit_w);
	DECLARE_READ8_MEMBER(kbd_r);
	UINT8 m_digit;
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START(sdk85_mem, AS_PROGRAM, 8, sdk85_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_ROM // Monitor rom (A14)
	AM_RANGE(0x0800, 0x0fff) AM_ROM // Expansion rom (A15)
	AM_RANGE(0x1800, 0x1800) AM_DEVREADWRITE("i8279", i8279_device, data_r, data_w )
	AM_RANGE(0x1900, 0x1900) AM_DEVREADWRITE("i8279", i8279_device, status_r, cmd_w)
	//AM_RANGE(0x1800, 0x1fff) AM_RAM // i8279 (A13)
	AM_RANGE(0x2000, 0x27ff) AM_RAM // i8155 (A16)
	AM_RANGE(0x2800, 0x2fff) AM_RAM // i8155 (A17)
ADDRESS_MAP_END

static ADDRESS_MAP_START(sdk85_io, AS_IO, 8, sdk85_state)
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( sdk85 )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EXEC") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NEXT") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GO") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SUBST") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EXAM") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SINGLE") PORT_CODE(KEYCODE_U)
	PORT_BIT(0xC0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


WRITE8_MEMBER( sdk85_state::scanlines_w )
{
	m_digit = data;
}

WRITE8_MEMBER( sdk85_state::digit_w )
{
	if (m_digit < 6)
		output().set_digit_value(m_digit, BITSWAP8(data, 3, 2, 1, 0, 7, 6, 5, 4)^0xff);
}

READ8_MEMBER( sdk85_state::kbd_r )
{
	UINT8 data = 0xff;

	if (m_digit < 3)
	{
		char kbdrow[6];
		sprintf(kbdrow,"X%X",m_digit);
		data = ioport(kbdrow)->read();
	}
	return data;
}

static MACHINE_CONFIG_START( sdk85, sdk85_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8085A, XTAL_2MHz)
	MCFG_CPU_PROGRAM_MAP(sdk85_mem)
	MCFG_CPU_IO_MAP(sdk85_io)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_sdk85)

	/* Devices */
	MCFG_DEVICE_ADD("i8279", I8279, 3100000) // based on divider
	MCFG_I8279_OUT_IRQ_CB(INPUTLINE("maincpu", I8085_RST55_LINE))   // irq
	MCFG_I8279_OUT_SL_CB(WRITE8(sdk85_state, scanlines_w))          // scan SL lines
	MCFG_I8279_OUT_DISP_CB(WRITE8(sdk85_state, digit_w))            // display A&B
	MCFG_I8279_IN_RL_CB(READ8(sdk85_state, kbd_r))                  // kbd RL lines
	MCFG_I8279_IN_SHIFT_CB(VCC)                                     // Shift key
	MCFG_I8279_IN_CTRL_CB(VCC)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( sdk85 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "sdk85.a14", 0x0000, 0x0800, CRC(9d5a983f) SHA1(54e218560fbec009ac3de5cfb64b920241ef2eeb))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT   COMPANY   FULLNAME       FLAGS */
COMP( 1977, sdk85,  0,       0,      sdk85,     sdk85, driver_device,     0,  "Intel",   "SDK-85", MACHINE_NO_SOUND_HW)
