// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        ET-3400

        12/05/2009 Skeleton driver.


    TODO:
    - Proper artwork

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "et3400.lh"


class et3400_state : public driver_device
{
public:
	et3400_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;
	DECLARE_READ8_MEMBER( et3400_keypad_r );
	DECLARE_WRITE8_MEMBER( et3400_display_w );
};



READ8_MEMBER( et3400_state::et3400_keypad_r )
{
	UINT8 data = 0xff;

	if (~offset & 4)
		data &= ioport("X2")->read();
	if (~offset & 2)
		data &= ioport("X1")->read();
	if (~offset & 1)
		data &= ioport("X0")->read();

	return data;
}

WRITE8_MEMBER( et3400_state::et3400_display_w )
{
/* This computer sets each segment, one at a time. */

	static const UINT8 segments[8]={0x40,0x20,0x10,0x08,0x04,0x02,0x01,0x80};
	UINT8 digit = (offset >> 4) & 7;
	UINT8 segment = segments[offset & 7];
	UINT8 segdata = output_get_digit_value(digit);

	if (data & 1)
		segdata |= segment;
	else
		segdata &= ~segment;

	output_set_digit_value(digit, segdata);
}


static ADDRESS_MAP_START(et3400_mem, AS_PROGRAM, 8, et3400_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x01ff ) AM_RAM
	AM_RANGE( 0xc000, 0xc0ff ) AM_READ(et3400_keypad_r)
	AM_RANGE( 0xc100, 0xc1ff ) AM_WRITE(et3400_display_w)
	AM_RANGE( 0xfc00, 0xffff ) AM_ROM
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( et3400 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D DO") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A AUTO") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7 RTI") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4 INDEX") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1 ACCA") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0xc0, 0xc0, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E EXAM") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B BACK") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8 SS") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5 CC") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2 ACCB") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(".") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')
	PORT_BIT( 0xc0, 0xc0, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F FWD") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C CHAN") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9 BR") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6 SP") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3 PC") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0xe0, 0xe0, IPT_UNUSED )
INPUT_PORTS_END


static MACHINE_CONFIG_START( et3400, et3400_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, XTAL_4MHz / 8 )
	MCFG_CPU_PROGRAM_MAP(et3400_mem)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_et3400)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( et3400 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "et3400.bin", 0xfc00, 0x0400, CRC(2eff1f58) SHA1(38b655de7393d7a92b08276f7c14a99eaa2a4a9f))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1976, et3400,  0,     0,       et3400,    et3400, driver_device,  0,    "Heath Inc", "Heathkit ET-3400", MACHINE_NO_SOUND_HW)
