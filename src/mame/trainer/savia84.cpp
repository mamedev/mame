// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Savia 84

More data at :
        http://www.nostalcomp.cz/pdfka/savia84.pdf
        http://www.nostalcomp.cz/savia.php
        (use archive.org)

2011-02-05 Skeleton driver.
2011-10-11 Found a new rom. Working [Robbbert]

I assume all the LEDs are red ones. The LEDs down the
 left side I assume to be bit 0 through 7 in that order.

Pasting:
        0-F : as is
        DA : ^
        AD : -
        GO : X

Here is a test program. Copy the text and Paste into the emulator.
    -1800^3E^55^D3^F9^76^XX1800^

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "video/pwm.h"
#include "savia84.lh"


namespace {

class savia84_state : public driver_device
{
public:
	savia84_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppi8255(*this, "ppi8255")
		, m_display(*this, "display")
		, m_io_keyboard(*this, "X%u", 0U)
		, m_leds(*this, "led%u", 0U)
		{ }

	void savia84(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	uint8_t ppi_portc_r();
	void ppi_porta_w(uint8_t data);
	void ppi_portb_w(uint8_t data);
	void ppi_portc_w(uint8_t data);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	uint8_t m_digit = 0U;
	uint8_t m_seg = 0U;

	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi8255;
	required_device<pwm_display_device> m_display;
	required_ioport_array<9> m_io_keyboard;
	output_finder<8> m_leds;
};

void savia84_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x7fff); // A15 not connected at the CPU
	map(0x0000, 0x07ff).rom();
	map(0x1800, 0x1fff).ram();
}

void savia84_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x07);
	map(0x00, 0x03).rw(m_ppi8255, FUNC(i8255_device::read), FUNC(i8255_device::write)); // ports F8-FB
}

/* Input ports */
static INPUT_PORTS_START( savia84 )
	PORT_START("X0")
	PORT_BIT( 0x9F, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')

	PORT_START("X1")
	PORT_BIT( 0x8F, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')

	PORT_START("X2")
	PORT_BIT( 0x8F, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')

	PORT_START("X3")
	PORT_BIT( 0x9F, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Ex") PORT_CODE(KEYCODE_X) PORT_CHAR('X')

	PORT_START("X4")
	PORT_BIT( 0x8F, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BR") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')

	PORT_START("X5")
	PORT_BIT( 0x8F, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("AD") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')

	PORT_START("X6")
	PORT_BIT( 0x9F, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')

	PORT_START("X7")
	PORT_BIT( 0x9F, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')

	PORT_START("X8")
	PORT_BIT( 0x8F, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DA") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
INPUT_PORTS_END


void savia84_state::ppi_porta_w(uint8_t data) // OUT F8 - output segments on the selected digit
{
	m_seg = ~data;
	m_display->matrix(1 << m_digit, m_seg);
}

void savia84_state::ppi_portb_w(uint8_t data) // OUT F9 - light the 8 leds down the left side
{
	for (int i = 0; i < 8; i++)
		m_leds[i] = !BIT(data, i);
}

void savia84_state::ppi_portc_w(uint8_t data) // OUT FA - set keyboard scanning row; set digit to display
{
	m_digit = data & 15;
	m_display->matrix(1 << m_digit, m_seg);
}

uint8_t savia84_state::ppi_portc_r() // IN FA - read keyboard
{
	if (m_digit < 9)
		return m_io_keyboard[m_digit]->read();
	else
		return 0xff;
}

void savia84_state::machine_start()
{
	m_leds.resolve();

	save_item(NAME(m_digit));
	save_item(NAME(m_seg));
}

void savia84_state::savia84(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &savia84_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &savia84_state::io_map);

	/* video hardware */
	config.set_default_layout(layout_savia84);
	PWM_DISPLAY(config, m_display).set_size(9, 7);
	m_display->set_segmask(0x1fd, 0x7f);

	/* Devices */
	I8255(config, m_ppi8255);
	m_ppi8255->out_pa_callback().set(FUNC(savia84_state::ppi_porta_w));
	m_ppi8255->out_pb_callback().set(FUNC(savia84_state::ppi_portb_w));
	m_ppi8255->in_pc_callback().set(FUNC(savia84_state::ppi_portc_r));
	m_ppi8255->out_pc_callback().set(FUNC(savia84_state::ppi_portc_w));
}

/* ROM definition */
ROM_START( savia84 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("savia84.bin", 0x0000, 0x0800, CRC(fa8f1fcf) SHA1(b08404469ed988d96c0413416b6a66f3e5b997a3))

	// Note - the below is a bad dump and does not work
	//ROM_LOAD("savia84_1kb.bin", 0x0000, 0x0400, CRC(23a5c15e) SHA1(7e769ed8960d8c591a25cfe4effffcca3077c94b))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY      FULLNAME    FLAGS
COMP( 1984, savia84, 0,      0,      savia84, savia84, savia84_state, empty_init, "J.T. Hyan", "Savia 84", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
