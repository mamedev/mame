// license:GPL-2.0+
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
    SHIFT + 0: view and edit memory values (Enter to escape)
    SHIFT + 1: view and edit register values
    SHIFT + 2: execution mode

    For more info, please check chapter 6 "Monitor software" in the book
    and/or the assembly listings of the ZAP monitor software
    available at appendix D.

    Currently missing features in this driver:
      * hookup RS232 support
      * maybe also support video terminal described in chapter 9

    Here is a test to paste in:
    -0400^11^22^33^44^55^66^77^88^99^X-0400
    You can confirm the entries by pressing UP
*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "zapcomputer.lh"


namespace {

class zapcomp_state : public driver_device
{
public:
	zapcomp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void zapcomp(machine_config &config);

private:
	uint8_t keyboard_r();
	void display_7seg_w(offs_t offset, uint8_t data);

	void zapcomp_io(address_map &map) ATTR_COLD;
	void zapcomp_mem(address_map &map) ATTR_COLD;

	uint8_t decode7seg(uint8_t data);
	virtual void machine_start() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	output_finder<6> m_digits;
};

uint8_t zapcomp_state::decode7seg(uint8_t data)
{
	//These are bit patterns representing the conversion of 4bit values
	//into the status of the segments of the 7 segment displays
	//controlled by a 82S23 PROM

	static uint8_t patterns[16] = {
		0x77, 0x41, 0x6e, 0x6b,
		0x59, 0x3b, 0x3f, 0x61,
		0x7f, 0x79, 0x7d, 0x1f,
		0x36, 0x4f, 0x3e, 0x3c
	};

	// Bit order for the FAIRCHILD FND-70
	//                     7-SEGMENT LCD:  .  g  f  e  d  c  b  a
	return bitswap<8>(patterns[data & 0x0F], 7, 3, 4, 2, 1, 0, 6, 5);
}

void zapcomp_state::display_7seg_w(offs_t offset, uint8_t data)
{
	//Port 0x05 : address HI
	//Port 0x06 : address LOW
	//Port 0x07 : data
	m_digits[offset*2] = decode7seg(data >> 4);
	m_digits[offset*2+1] = decode7seg(data);
}

uint8_t zapcomp_state::keyboard_r()
{
	uint8_t retval = 0x00;
	uint8_t special = ioport("X1")->read();
	uint16_t hex_keys = ioport("X0")->read();

	if (BIT(special, 2)) /* "SHIFT" key is pressed */
		retval |= 0x40; /* turn on the SHIFT bit but DO NOT turn on the strobe bit */

	if (BIT(special, 1)) /* "NEXT" key is pressed */
		retval |= 0xA0; /* turn on the strobe & NEXT bits */

	if (BIT(special, 0)) /* "EXEC" key is pressed */
		retval |= 0x90; /* turn on the strobe & EXEC bit */

	for (int i=0; i<16; i++)
		if (hex_keys & (1 << i))
			retval |= (0x80 | i); /* provide the key index in bits 3-0
			                         as well as turning on the strobe bit */

	return retval;
}

void zapcomp_state::zapcomp_mem(address_map &map)
{
	map(0x0000, 0x03ff).rom(); /* system monitor */
	map(0x0400, 0x07ff).ram(); /* mandatory 1 kilobyte bank #0 */
	map(0x0800, 0x0bff).ram(); /* extra 1 kilobyte bank #1 (optional) */
	map(0x0c00, 0x0fff).ram(); /* extra 1 kilobyte bank #2 (optional) */
	map(0x1000, 0x13ff).ram(); /* extra 1 kilobyte bank #3 (optional) */
	map(0x1400, 0x17ff).ram(); /* extra 1 kilobyte bank #4 (optional) */
	map(0x1800, 0x1bff).ram(); /* extra 1 kilobyte bank #5 (optional) */
	map(0x1c00, 0x1fff).ram(); /* extra 1 kilobyte bank #6 (optional) */
	map(0x2000, 0x23ff).ram(); /* extra 1 kilobyte bank #7 (optional) */
}

void zapcomp_state::zapcomp_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(zapcomp_state::keyboard_r));
	map(0x05, 0x07).w(FUNC(zapcomp_state::display_7seg_w));
}

static INPUT_PORTS_START( zapcomp )
	PORT_START("X0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('-')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('R')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('Z')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("EXEC") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('X')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("NEXT") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
INPUT_PORTS_END

void zapcomp_state::machine_start()
{
	m_digits.resolve();
}

void zapcomp_state::zapcomp(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(2'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &zapcomp_state::zapcomp_mem);
	m_maincpu->set_addrmap(AS_IO, &zapcomp_state::zapcomp_io);

	/* video hardware */
	config.set_default_layout(layout_zapcomputer);
}

ROM_START( zapcomp )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD("zap.rom", 0x0000, 0x0400, CRC(3f4416e9) SHA1(d6493707bfba1a1e1e551f8144194afa5bda3316) )
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY                               FULLNAME                            FLAGS
COMP( 1981, zapcomp, 0,      0,      zapcomp, zapcomp, zapcomp_state, empty_init, "Steve Ciarcia / BYTE / McGRAW-HILL", "ZAP - Z80 Applications Processor", MACHINE_NO_SOUND_HW )
