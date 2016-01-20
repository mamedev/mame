// license:GPL2+
// copyright-holders:FelipeSanches
/*
    ZAP - Z80 Applications Processor

    driver by: Felipe Correa da Silva Sanches <juca@members.fsf.org>

    Based on the technical descriptions and
    assembly listings of the book:

        "Build Your Own Z80 Microcomputer"
        by Steve Ciarcia (1981)
        published by BYTE/McGRAW-HILL

    Basic usage instructions:
    SHIFT + 0: view and edit memory values
    SHIFT + 1: view and edit register values
    SHIFT + 2: execution mode

    For more info, please check chapter 6 "Monitor software" in the book
    and/or the assembly listings of the ZAP monitor software
    avaliable at appendix D.

    Currently missing features in this driver:
      * hookup RS232 support
      * maybe also support video terminal described in chapter 9
*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "zapcomputer.lh"

class zapcomp_state : public driver_device
{
public:
	zapcomp_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(display_7seg_w);

private:
	UINT8 decode7seg(UINT8 data);
	virtual void machine_start() override;
	required_device<cpu_device> m_maincpu;
};

UINT8 zapcomp_state::decode7seg(UINT8 data)
{
	//These are bit patterns representing the conversion of 4bit values
	//into the status of the segments of the 7 segment displays
	//controlled by a 82S23 PROM

	UINT8 patterns[16] = {
		0x77, 0x41, 0x6e, 0x6b,
		0x59, 0x3b, 0x3f, 0x61,
		0x7f, 0x79, 0x7d, 0x1f,
		0x36, 0x4f, 0x3e, 0x3c
	};

	// Bit order for the FAIRCHILD FND-70
	//                     7-SEGMENT LCD:  .  g  f  e  d  c  b  a
	return BITSWAP8(patterns[data & 0x0F], 7, 3, 4, 2, 1, 0, 6, 5);
}

WRITE8_MEMBER( zapcomp_state::display_7seg_w )
{
	switch (offset){
		case 0: //Port 0x05 : address HI
			output().set_digit_value(0, decode7seg(data >> 4));
			output().set_digit_value(1, decode7seg(data));
			break;
		case 1: //Port 0x06 : address LOW
			output().set_digit_value(2, decode7seg(data >> 4));
			output().set_digit_value(3, decode7seg(data));
			break;
		case 2: //Port 0x07 : data
			output().set_digit_value(4, decode7seg(data >> 4));
			output().set_digit_value(5, decode7seg(data));
			break;
		default:
			break;
	}
}

READ8_MEMBER( zapcomp_state::keyboard_r )
{
	UINT8 retval = 0x00;
	UINT8 special = ioport("X1")->read();
	UINT16 hex_keys = ioport("X0")->read();

	if BIT(special, 2) /* "SHIFT" key is pressed */
		retval |= 0x40; /* turn on the SHIFT bit but DO NOT turn on the strobe bit */

	if BIT(special, 1) /* "NEXT" key is pressed */
		retval |= 0xA0; /* turn on the strobe & NEXT bits */

	if BIT(special, 0) /* "EXEC" key is pressed */
		retval |= 0x90; /* turn on the strobe & EXEC bit */

	for (int i=0; i<16; i++)
		if (hex_keys & (1 << i))
			retval |= (0x80 | i); /* provide the key index in bits 3-0
                                     as well as turning on the strobe bit */

	return retval;
}

static ADDRESS_MAP_START( zapcomp_mem, AS_PROGRAM, 8, zapcomp_state )
	AM_RANGE(0x0000, 0x03ff) AM_ROM AM_REGION("roms", 0) /* system monitor */
	AM_RANGE(0x0400, 0x07ff) AM_RAM /* mandatory 1 kilobyte bank #0 */
	AM_RANGE(0x0800, 0x0bff) AM_RAM /* extra 1 kilobyte bank #1 (optional) */
	AM_RANGE(0x0c00, 0x0fff) AM_RAM /* extra 1 kilobyte bank #2 (optional) */
	AM_RANGE(0x1000, 0x13ff) AM_RAM /* extra 1 kilobyte bank #3 (optional) */
	AM_RANGE(0x1400, 0x17ff) AM_RAM /* extra 1 kilobyte bank #4 (optional) */
	AM_RANGE(0x1800, 0x1bff) AM_RAM /* extra 1 kilobyte bank #5 (optional) */
	AM_RANGE(0x1c00, 0x1fff) AM_RAM /* extra 1 kilobyte bank #6 (optional) */
	AM_RANGE(0x2000, 0x23ff) AM_RAM /* extra 1 kilobyte bank #7 (optional) */
ADDRESS_MAP_END

static ADDRESS_MAP_START( zapcomp_io, AS_IO, 8, zapcomp_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(keyboard_r)
	AM_RANGE(0x05, 0x07) AM_WRITE(display_7seg_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( zapcomp )
	PORT_START("X0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f')

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("EXEC") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("NEXT") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
INPUT_PORTS_END

void zapcomp_state::machine_start()
{
}

static MACHINE_CONFIG_START( zapcomp, zapcomp_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, XTAL_2MHz)
	MCFG_CPU_PROGRAM_MAP(zapcomp_mem)
	MCFG_CPU_IO_MAP(zapcomp_io)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_zapcomputer)
MACHINE_CONFIG_END

ROM_START( zapcomp )
	ROM_REGION( 0x10000, "roms", 0 )
	ROM_LOAD("zap.rom", 0x0000, 0x0400, CRC(3f4416e9) SHA1(d6493707bfba1a1e1e551f8144194afa5bda3316) )
ROM_END

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT    CLASS         INIT    COMPANY                              FULLNAME                              FLAGS
COMP( 1981, zapcomp,  0,      0,      zapcomp, zapcomp, driver_device,  0,  "Steve Ciarcia / BYTE / McGRAW-HILL",  "ZAP - Z80 Applications Processor", MACHINE_NO_SOUND_HW )
