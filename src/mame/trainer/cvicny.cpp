// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

CVICNY8080 - Practice-z80 - a homebrew from Czechoslavakia.

More data at :
    https://web.archive.org/web/20190202185251/http://www.nostalcomp.cz/cvicny8080.php

2011-10-21 New working driver. [Robbbert]

Keys:
    0-9,A-F : hexadecimal numbers
    ADR : enter an address to work with. After the 4 digits are entered,
          the data at that address shows, and you can modify the data.
    + (inc) : Enter the data into memory, and increment the address by 1.
    GO : execute the program located at the current address.

Pasting:
    0-F : as is
    + (inc) : ^
    ADR : -
    GO : X

Test Paste:
    11^22^33^44^55^66^77^88^99^-0800
    Now press up-arrow to confirm the data has been entered.


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/pwm.h"
#include "cvicny.lh"


namespace {

class cvicny_state : public driver_device
{
public:
	cvicny_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_display(*this, "display")
		, m_io_keyboard(*this, "X%u", 0U)
	{ }

	void cvicny(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void cvicny_mem(address_map &map) ATTR_COLD;
	u8 key_r();
	void digit_w(u8 data);
	void segment_w(u8 data);

	u8 m_digit = 0U;
	u8 m_seg = 0U;
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_ioport_array<8> m_io_keyboard;
};

void cvicny_state::segment_w(u8 data) // output segments on the selected digit
{
	m_seg = data;
	m_display->matrix(1<<m_digit, m_seg);
}

void cvicny_state::digit_w(u8 data) // set keyboard scanning row; set digit to display
{
	m_digit = data & 7;
	m_display->matrix(1<<m_digit, m_seg);
}

u8 cvicny_state::key_r()
{
	u8 data = m_io_keyboard[m_digit]->read();
	return ((data << 4) ^ 0xf0) | data;
}

void cvicny_state::machine_start()
{
	save_item(NAME(m_digit));
	save_item(NAME(m_seg));
}

void cvicny_state::cvicny_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).rom(); // 1 x 2716
	map(0x0800, 0x0bff).ram().mirror(0x400); // 2x 2114 static ram
	map(0x1000, 0x17ff).r(FUNC(cvicny_state::key_r));
	map(0x1800, 0x1fff).w(FUNC(cvicny_state::digit_w));
	map(0x2000, 0x27ff).w(FUNC(cvicny_state::segment_w));
}


/* Input ports */
static INPUT_PORTS_START( cvicny )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("+") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ADR") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GO") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X5")
	PORT_BIT( 0x0F, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X6")
	PORT_BIT( 0x0F, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X7")
	PORT_BIT( 0x0F, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


void cvicny_state::cvicny(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(2'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &cvicny_state::cvicny_mem);

	/* video hardware */
	config.set_default_layout(layout_cvicny);
	PWM_DISPLAY(config, m_display).set_size(8, 8);
	m_display->set_segmask(0xff, 0xff);
}

/* ROM definition */
ROM_START( cvicny )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("cvicny8080.bin", 0x0000, 0x05ea, CRC(e6119052) SHA1(d03c2cbfd047f0d090a787fbbde6353593cc2dd8) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY      FULLNAME        FLAGS
COMP( 1984, cvicny, 0,      0,      cvicny,  cvicny, cvicny_state, empty_init, "<unknown>", "Practice-z80", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
