// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

    VCS-80

    12/05/2009 Skeleton driver.

    http://hc-ddr.hucki.net/entwicklungssysteme.htm#VCS_80_von_Eckhard_Schiller

This system is heavily based on the tk80. The display and keyboard matrix
are very similar, while the operation is easier in the vcs80. The hardware
is completely different however.

Pasting:
        0-F : as is
        A+ : ^
        A- : V
        MA : -
        GO : X

When booted, the system begins at 0000 which is ROM. You need to change the
address to 0400 before entering a program. Here is a test to paste in:
0400-11^22^33^44^55^66^77^88^99^0400-
Press the up-arrow to confirm data has been entered.

Operation:
4 digits at left is the address; 2 digits at right is the data.
As you increment addresses, the middle 2 digits show the previous byte.
You can enter 4 digits, and pressing 'MA' will transfer this info
to the left, thus setting the address to this value. Press 'A+' to
store new data and increment the address.

One unusual configuration item is that /A0 connects to PIO.B7, and so
whenever it goes high, an interrupt can be triggered.

Whenever a memory request is made (via /MREQ), it triggers a slight pause at
the CPU's WAIT pin.

****************************************************************************/
#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/bankdev.h"
#include "machine/timer.h"
#include "video/pwm.h"
#include "vcs80.lh"


namespace {

class vcs80_state : public driver_device
{
public:
	vcs80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pio(*this, "pio")
		, m_bdmem(*this, "bdmem")
		, m_display(*this, "display")
		, m_io_keyboard(*this, "Y%u", 0U)
	{ }

	void vcs80(machine_config &config);

private:
	required_device<z80_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_device<address_map_bank_device> m_bdmem;
	required_device<pwm_display_device> m_display;
	required_ioport_array<3> m_io_keyboard;

	virtual void machine_start() override ATTR_COLD;

	uint8_t pio_pa_r();
	void pio_pb_w(uint8_t data);

	uint8_t mem_r(offs_t offset)
	{
		m_pio->port_b_write((!BIT(offset, 0)) << 7);
		return m_bdmem->read8(offset);
	}

	void mem_w(offs_t offset, uint8_t data)
	{
		m_pio->port_b_write((!BIT(offset, 0)) << 7);
		m_bdmem->write8(offset, data);
	}

	uint8_t io_r(offs_t offset)
	{
		m_pio->port_b_write((!BIT(offset, 0)) << 7);
		if (BIT(offset, 2))
			return m_pio->read(offset^3);
		return 0xff;
	}

	void io_w(offs_t offset, uint8_t data)
	{
		m_pio->port_b_write((!BIT(offset, 0)) << 7);
		if (BIT(offset, 2))
			m_pio->write(offset^3, data);
	}

	/* keyboard state */
	bool m_keyclk = false;
	u8 m_digit = 0U;
	u8 m_seg = 0U;
	void init_vcs80();
	TIMER_DEVICE_CALLBACK_MEMBER(vcs80_keyboard_tick);

	void bd_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
};



/* Memory Maps */

void vcs80_state::bd_map(address_map &map)
{
	map(0x0000, 0x01ff).rom().region("maincpu", 0);
	map(0x0400, 0x07ff).ram();
}

void vcs80_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(vcs80_state::mem_r), FUNC(vcs80_state::mem_w));
}

void vcs80_state::io_map(address_map &map)
{
	map.global_mask(0x07);
	map(0x00, 0x07).rw(FUNC(vcs80_state::io_r), FUNC(vcs80_state::io_w));
}

/* Input Ports */

static INPUT_PORTS_START( vcs80 )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A+") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A-") PORT_CODE(KEYCODE_DOWN) PORT_CHAR('V')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MA") PORT_CODE(KEYCODE_M) PORT_CHAR('-')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RE") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GO") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TR") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ST") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PE") PORT_CODE(KEYCODE_P)
INPUT_PORTS_END

/* Z80-PIO Interface */

TIMER_DEVICE_CALLBACK_MEMBER(vcs80_state::vcs80_keyboard_tick)
{
	if (m_keyclk)
	{
		m_digit++;
		m_digit &= 7;
		m_display->matrix(1 << m_digit, m_seg);
	}

	m_pio->port_a_write(m_keyclk << 7);

	m_keyclk = !m_keyclk;
}

uint8_t vcs80_state::pio_pa_r()
{
	/*

	    bit     description

	    PA0     keyboard and led latch bit 0
	    PA1     keyboard and led latch bit 1
	    PA2     keyboard and led latch bit 2
	    PA3     GND
	    PA4     keyboard row input 0
	    PA5     keyboard row input 1
	    PA6     keyboard row input 2
	    PA7     demultiplexer clock input

	*/

	/* keyboard and led latch */
	u8 data = m_digit;

	/* keyboard rows */
	data |= BIT(m_io_keyboard[0]->read(), m_digit) << 4;
	data |= BIT(m_io_keyboard[1]->read(), m_digit) << 5;
	data |= BIT(m_io_keyboard[2]->read(), m_digit) << 6;

	/* demultiplexer clock */
	data |= (m_keyclk << 7);

	return data;
}

void vcs80_state::pio_pb_w(uint8_t data)
{
	/*

	    bit     description

	    PB0     VQD30 segment A
	    PB1     VQD30 segment B
	    PB2     VQD30 segment C
	    PB3     VQD30 segment D
	    PB4     VQD30 segment E
	    PB5     VQD30 segment G
	    PB6     VQD30 segment F
	    PB7     _A0

	*/

	m_seg = bitswap<8>(data & 0x7f, 7, 5, 6, 4, 3, 2, 1, 0);
	m_display->matrix(1 << m_digit, m_seg);
}

/* Z80 Daisy Chain */

static const z80_daisy_config daisy_chain[] =
{
	{ "pio" },
	{ nullptr }
};

/* Machine Initialization */

void vcs80_state::machine_start()
{
	m_pio->strobe_a(1);
	m_pio->strobe_b(1);

	/* register for state saving */
	save_item(NAME(m_keyclk));
	save_item(NAME(m_digit));
	save_item(NAME(m_seg));
}

/* Machine Driver */

void vcs80_state::vcs80(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 5'000'000 /2); // U880D - Uses a LC oscillator rather than a crystal
	m_maincpu->set_addrmap(AS_PROGRAM, &vcs80_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &vcs80_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	/* keyboard timer */
	TIMER(config, "keyboard").configure_periodic(FUNC(vcs80_state::vcs80_keyboard_tick), attotime::from_hz(1000));

	/* video hardware */
	config.set_default_layout(layout_vcs80);
	PWM_DISPLAY(config, m_display).set_size(8, 8);
	m_display->set_segmask(0xff, 0xff);

	/* devices */
	Z80PIO(config, m_pio, 5'000'000 /2);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio->in_pa_callback().set(FUNC(vcs80_state::pio_pa_r));
	m_pio->out_pb_callback().set(FUNC(vcs80_state::pio_pb_w));

	/* bankdev */
	ADDRESS_MAP_BANK(config, "bdmem").set_map(&vcs80_state::bd_map).set_options(ENDIANNESS_BIG, 8, 32, 0x10000);
}

/* ROMs */

ROM_START( vcs80 )
	ROM_REGION( 0x0200, "maincpu", 0 )
	ROM_LOAD( "monitor.rom", 0x0000, 0x0200, CRC(44aff4e9) SHA1(3472e5a9357eaba3ed6de65dee2b1c6b29349dd2) )
ROM_END

} // anonymous namespace


/* System Drivers */

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  STATE        INIT        COMPANY             FULLNAME  FLAGS */
COMP( 1983, vcs80, 0,      0,      vcs80,   vcs80, vcs80_state, empty_init, "Eckhard Schiller", "VCS-80", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )

