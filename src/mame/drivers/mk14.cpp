// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/*********************************************************************************************************************************

Science of Cambridge MK-14

2009-11-20 Skeleton driver.
2016-08-21 Working

Keys:

UP: MEM increments the currently displayed address, (and goes into data entry mode in V1 bios).
= : TERM changes to "data entry" mode. In this mode, entering hex digits will change the byte at the currently displayed address
- : ABORT changes to "address entry" mode. In this mode, entering hex digits will change the address.
X : GO runs the program from the currently displayed address. On exit, the instruction after the program is displayed

Pasting:
        0-F : as is
        MEM : ^
        TERM: =
        AB :  -
        GO :  X

Example program: ("organ" from p82 of the manual)
-F20=C4^0D^35^C4^00^31^C4^08^C8^F6^C5^01^E4^FF^98^08^8F^00^06^E4^07^07^90^EB^B8^E6^9C^EE^90^E5^-F20X
Pressing keys will produce different tones.


TODO:
- VDU optional attachment (we are missing the chargen rom)

*********************************************************************************************************************************/

#include "emu.h"

#include "cpu/scmp/scmp.h"
#include "imagedev/cassette.h"
#include "machine/ins8154.h"
#include "sound/dac.h"
#include "speaker.h"
#include "video/pwm.h"

#include "mk14.lh"

namespace {

class mk14_state : public driver_device
{
public:
	mk14_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
		, m_dac(*this, "dac")
		, m_display(*this, "display")
		, m_io_keyboard(*this, "X%u", 0U)
		{ }

	void mk14(machine_config &config);

private:
	uint8_t keyboard_r(offs_t offset);
	void display_w(offs_t offset, uint8_t data);
	void port_a_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(cass_w);
	DECLARE_READ_LINE_MEMBER(cass_r);
	void mem_map(address_map &map);

	required_device<scmp_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	required_device<dac_bit_interface> m_dac;
	required_device<pwm_display_device> m_display;
	required_ioport_array<8> m_io_keyboard;
};

/*
000-1FF  512 byte SCIOS ROM  Decoded by 0xxx
200-3FF  ROM Shadow / Expansion RAM
400-5FF  ROM Shadow / Expansion RAM
600-7FF  ROM Shadow / Expansion RAM
800-87F  I/O Ports  Decoded by 1xx0
880-8FF  128 bytes I/O chip RAM  Decoded by 1xx0
900-9FF  Keyboard & Display  Decoded by 1x01
A00-AFF  I/O Port & RAM Shadow
B00-BFF  256 bytes RAM (Extended) / VDU RAM  Decoded by 1011
C00-CFF  I/O Port & RAM Shadow
D00-DFF  Keyboard & Display Shadow
E00-EFF  I/O Port & RAM Shadow
F00-FFF  256 bytes RAM (Standard) / VDU RAM  Decoded by 1111

*/


uint8_t mk14_state::keyboard_r(offs_t offset)
{
	if (offset < 8)
		return m_io_keyboard[offset]->read();
	else
		return 0xff;
}

void mk14_state::display_w(offs_t offset, uint8_t data)
{
	if (offset < 8 )
		m_display->matrix(1<<offset, data);
	else
	{
		//logerror("write %02x to %02x\n",data,offset);
	}
}

void mk14_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x0fff);
	map(0x000, 0x1ff).mirror(0x600).rom(); // ROM
	map(0x800, 0x87f).mirror(0x600).rw("ic8", FUNC(ins8154_device::read_io), FUNC(ins8154_device::write_io)); // I/O
	map(0x880, 0x8ff).mirror(0x600).rw("ic8", FUNC(ins8154_device::read_ram), FUNC(ins8154_device::write_ram)); // 128 bytes I/O chip RAM
	map(0x900, 0x9ff).mirror(0x400).rw(FUNC(mk14_state::keyboard_r), FUNC(mk14_state::display_w));
	map(0xb00, 0xbff).ram(); // VDU RAM
	map(0xf00, 0xfff).ram(); // Standard RAM
}


/* Input ports */
static INPUT_PORTS_START( mk14 )
	PORT_START("X0")
		PORT_BIT(0x0F, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")    PORT_CODE(KEYCODE_A)      PORT_CHAR('A')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8")    PORT_CODE(KEYCODE_8)      PORT_CHAR('8')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")    PORT_CODE(KEYCODE_0)      PORT_CHAR('0')
	PORT_START("X1")
		PORT_BIT(0x0F, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B")    PORT_CODE(KEYCODE_B)      PORT_CHAR('B')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9")    PORT_CODE(KEYCODE_9)      PORT_CHAR('9')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1")    PORT_CODE(KEYCODE_1)      PORT_CHAR('1')
	PORT_START("X2")
		PORT_BIT(0x0F, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C")    PORT_CODE(KEYCODE_C)      PORT_CHAR('C')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GO")   PORT_CODE(KEYCODE_X)      PORT_CHAR('X')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2")    PORT_CODE(KEYCODE_2)      PORT_CHAR('2')
	PORT_START("X3")
		PORT_BIT(0x0F, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D")    PORT_CODE(KEYCODE_D)      PORT_CHAR('D')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MEM")  PORT_CODE(KEYCODE_UP)     PORT_CHAR('^')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3")    PORT_CODE(KEYCODE_3)      PORT_CHAR('3')
	PORT_START("X4")
		PORT_BIT(0x0F, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("AB")   PORT_CODE(KEYCODE_MINUS)  PORT_CHAR('-')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")    PORT_CODE(KEYCODE_4)      PORT_CHAR('4')
	PORT_START("X5")
		PORT_BIT(0x0F, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5")    PORT_CODE(KEYCODE_5)      PORT_CHAR('5')
	PORT_START("X6")
		PORT_BIT(0x0F, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E")    PORT_CODE(KEYCODE_E)      PORT_CHAR('E')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6")    PORT_CODE(KEYCODE_6)      PORT_CHAR('6')
	PORT_START("X7")
		PORT_BIT(0x0F, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F")    PORT_CODE(KEYCODE_F)      PORT_CHAR('F')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TERM") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7")    PORT_CODE(KEYCODE_7)      PORT_CHAR('7')
INPUT_PORTS_END

void mk14_state::port_a_w(uint8_t data)
{
}

WRITE_LINE_MEMBER( mk14_state::cass_w )
{
	m_cass->output(state ? -1.0 : +1.0);
	m_dac->write(state);
}

READ_LINE_MEMBER( mk14_state::cass_r )
{
	return (m_cass->input() > 0.03) ? 1 : 0;
}

void mk14_state::mk14(machine_config &config)
{
	/* basic machine hardware */
	// IC1 1SP-8A/600 (8060) SC/MP Microprocessor
	INS8060(config, m_maincpu, 4.433619_MHz_XTAL);
	m_maincpu->flag_out().set(FUNC(mk14_state::cass_w));
	m_maincpu->s_out().set_nop();
	m_maincpu->s_in().set(FUNC(mk14_state::cass_r));
	m_maincpu->sense_a().set_constant(0);
	m_maincpu->sense_b().set(FUNC(mk14_state::cass_r));
	m_maincpu->halt().set_nop();
	m_maincpu->set_addrmap(AS_PROGRAM, &mk14_state::mem_map);

	/* video hardware */
	config.set_default_layout(layout_mk14);
	PWM_DISPLAY(config, m_display).set_size(8, 8);
	m_display->set_segmask(0xff, 0xff);

	// sound
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25);
	ZN425E(config, "dac8", 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // Ferranti ZN425E

	/* devices */
	ins8154_device &ic8(INS8154(config, "ic8"));
	ic8.out_a().set(FUNC(mk14_state::port_a_w));
	ic8.out_b().set("dac8", FUNC(dac_byte_interface::data_w));

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "speaker", 0.05);
}

/* ROM definition */
ROM_START( mk14 )
	ROM_REGION( 0x200, "maincpu", 0 )
	// IC2,3 74S571 512 x 4 bit ROM
	ROM_DEFAULT_BIOS("v2")
	ROM_SYSTEM_BIOS(0, "v2", "SCIOS V2")
	ROMX_LOAD( "scios_v2.bin", 0x0000, 0x0200, CRC(8b667daa) SHA1(802dc637ce5391a2a6627f76f919b12a869b56ef), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v1", "SCIOS V1")
	ROMX_LOAD( "scios_v1.bin", 0x0000, 0x0200, CRC(3d2477e7) SHA1(795829a2025e24d87a413e245d72a284f872e0db), ROM_BIOS(1))
ROM_END

} // Anonymous namespace

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY                 FULLNAME  FLAGS
COMP( 1977, mk14, 0,      0,      mk14,    mk14,  mk14_state, empty_init, "Science of Cambridge", "MK-14", MACHINE_SUPPORTS_SAVE )
