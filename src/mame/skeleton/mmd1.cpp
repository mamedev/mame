// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

MMD-1 driver by Miodrag Milanovic

2009-05-12 Initial version


 It appears that you enter an 3 digit octal number and then hit a function key.
 H - puts the number in the H register
 L - puts the number in the L register
 S - puts the number into memory pointed to by HL and then increments HL.
 G - Loads the program counter with the contents of HL

1) There is a 'working byte' which you can enter using the octal digits
(just press them in order), and which is displayed on the port 2 LEDs
when KEX is running.

2) 'R' is a hardware reset

3) 'H' and 'L' are used to load the address (high and low parts, and it
really is the HL register of the 8080). So to enter a particular address,
you type in the high half (in octal), press H. Then type in the low half
and press L. The address is displayed on the port 0 and port 1 LEDs when
KEX is running.

4) 'S' is 'Step' or 'Store'. It stores the working byte at the current
address (in HL), and then increments the address. It's used to enter
bytes into memory

5) 'G' is 'go'. It loads the 8080 PC with the address in HL, and thus
executes a program at that address.

OK, this is what I would try.

1) Press 'R' to reset the 8080 and start KEX running.

2) Type 004 H 000 L  to load the start address of your program. The bytes
should appear on the rightmost 8 LEDs as you enter them and should then
also appear on the left and middle sets of LEDs when you press H and L.

3) Enter the program

076 S 123 S 323 S 000 S 166S

As you type each byte it should appear on the rightmost LEDs. When you
press S, the address on the rest of the LEDs should increment by 1.

4) Re-enter the start address
004 H 000 L

5) Press G to run the program. The left most LEDs should change to
.*.*..** (. = off, * = on), I think. The keys will then do nothing (as
the CPU is halted) until you press R again to re-run KEX.


Cassette:
- The only info available is that an unknown UART is used on ports 12/13.
- Since MMD2 uses Kansas City format, I've used it here too. As there are
  no setup bytes, the UART is assumed to be the AY-3-1015 or equivalent.
  The result works perfectly.
- To save: 001H 025L G (start recording) press 0-7 to indicate number of
           blocks to save (0 = 8 blocks). The start address is always 1800.
           When it's finished, control will return.
- To load: 001H 000L (start playing), press G. When it's finished (cassette
           sound stops), press R. It always loads to 1800-up.

ToDo:
- tty uart ports 0x10/11, and rs232 interface
- Need software

****************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
#include "machine/timer.h"

#include "speaker.h"

#include "mmd1.lh"


namespace {

class mmd1_state : public driver_device
{
public:
	mmd1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
		, m_uart(*this, "uart")
		, m_lines(*this, "LINE%u", 1U)
		, m_digits(*this, "digit%u", 0U)
		, m_p(*this, "p%u_%u", 0U, 0U)
	{ }

	void mmd1(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void round_leds_w(offs_t offset, u8 data);
	u8 keyboard_r();
	u8 port13_r();
	int si();
	void so(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	void kansas_w(int state);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	u8 m_cass_data[4]{};
	bool m_cassinbit = 0, m_cassoutbit = 0, m_cassold = 0;
	u8 m_return_code = 0U;

	required_device<i8080_cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	required_device<ay31015_device> m_uart;
	required_ioport_array<2> m_lines;
	output_finder<9> m_digits;
	output_finder<3, 8> m_p;
};


void mmd1_state::round_leds_w(offs_t offset, u8 data)
{
	for (u8 i = 0; i < 8; i++)
		m_p[offset][i] = BIT(data, i) ? 0 : 1;
}


// keyboard has a keydown and a keyup code. Keyup = last keydown + bit 7 set
u8 mmd1_state::keyboard_r()
{
	const u8 line1 = m_lines[0]->read();
	const u8 line2 = m_lines[1]->read();
	u8 data = 0xff;

	for (unsigned i = 0; i < 8; i++)
	{
		if (!BIT(line1, i))
			data = i;
	}

	for (unsigned i = 0; i < 8; i++)
	{
		if (!BIT(line2, i))
			data = i + 8;
	}

	if (data < 0x10)
	{
		m_return_code = data | 0x80;
		return data;
	}
	else
		return m_return_code;
}

u8 mmd1_state::port13_r()
{
	u8 data = 0xfa;
	data |= m_uart->dav_r() ? 1 : 0;
	data |= m_uart->tbmt_r() ? 4 : 0;
	return data;
}

int mmd1_state::si()
{
	return m_cassinbit;
}

void mmd1_state::so(int state)
{
	m_cassoutbit = state;
}

void mmd1_state::kansas_w(int state)
{
	if ((m_cass->get_state() & CASSETTE_MASK_UISTATE) == CASSETTE_RECORD)
	{
		// incoming @4800Hz
		u8 twobit = m_cass_data[3] & 15;

		if (state)
		{
			if (twobit == 0)
				m_cassold = m_cassoutbit;

			if (m_cassold)
				m_cass->output(BIT(m_cass_data[3], 0) ? -1.0 : +1.0); // 2400Hz
			else
				m_cass->output(BIT(m_cass_data[3], 1) ? -1.0 : +1.0); // 1200Hz

			m_cass_data[3]++;
		}
	}

	m_uart->write_tcp(state);
	m_uart->write_rcp(state);
}

TIMER_DEVICE_CALLBACK_MEMBER( mmd1_state::kansas_r )
{
	// no tape - set to idle
	m_cass_data[1]++;
	if (m_cass_data[1] > 48)
	{
		m_cass_data[1] = 48;
		m_cassinbit = 1;
	}

	if ((m_cass->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_PLAY)
		return;

	/* cassette - turn 1200/2400Hz to a bit */
	u8 cass_ws = (m_cass->input() > +0.04) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_cassinbit = (m_cass_data[1] < 12) ? 1 : 0;
		m_cass_data[1] = 0;
	}
}

void mmd1_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).rom(); // Main ROM
	map(0x0100, 0x01ff).rom(); // Expansion slot
	map(0x0200, 0x02ff).ram();
	map(0x0300, 0x03ff).ram();
	map(0x1800, 0x1fff).ram(); // Area that can be accessed by cassette
}

void mmd1_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x02).w(FUNC(mmd1_state::round_leds_w));
	map(0x00, 0x00).r(FUNC(mmd1_state::keyboard_r));
	//map(0x10, 0x11).rw  TTY UART
	map(0x12, 0x12).rw(m_uart, FUNC(ay51013_device::receive), FUNC(ay51013_device::transmit));
	map(0x13, 0x13).r(FUNC(mmd1_state::port13_r));
}


/* Input ports */
static INPUT_PORTS_START( mmd1 )
	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_LALT) PORT_CHANGED_MEMBER(DEVICE_SELF, mmd1_state, reset_button, 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(mmd1_state::reset_button)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}

void mmd1_state::machine_start()
{
	m_digits.resolve();
	m_p.resolve();
	save_item(NAME(m_cass_data));
	save_item(NAME(m_cassinbit));
	save_item(NAME(m_cassoutbit));
	save_item(NAME(m_cassold));
	save_item(NAME(m_return_code));
}

void mmd1_state::machine_reset()
{
	m_return_code = 0xff;
	// setup uart to 8N2
	m_uart->write_np(1);
	m_uart->write_tsb(1);
	m_uart->write_nb1(1);
	m_uart->write_nb2(1);
	m_uart->write_eps(1);
	m_uart->write_cs(1);
	m_uart->write_cs(0);
}

void mmd1_state::mmd1(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, 6750000 / 9);
	m_maincpu->set_addrmap(AS_PROGRAM, &mmd1_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &mmd1_state::io_map);

	/* video hardware */
	config.set_default_layout(layout_mmd1);

	AY51013(config, m_uart);
	m_uart->read_si_callback().set(FUNC(mmd1_state::si));
	m_uart->write_so_callback().set(FUNC(mmd1_state::so));
	m_uart->set_auto_rdav(true);

	clock_device &uart_clock(CLOCK(config, "uart_clock", 4800));
	uart_clock.signal_handler().set(FUNC(mmd1_state::kansas_w));
	TIMER(config, "kansas_r").configure_periodic(FUNC(mmd1_state::kansas_r), attotime::from_hz(40000));

	// cassette is connected to the uart
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	SPEAKER(config, "mono").front_center();
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}

/* ROM definition */
ROM_START( mmd1 )
	ROM_REGION( 0x0200, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "kex.ic15",    0x0000, 0x0100, CRC(434f6923) SHA1(a2af60deda54c8d3f175b894b47ff554eb37e9cb))
	ROM_LOAD( "prom1.ic16",  0x0100, 0x0100, BAD_DUMP CRC(d23a6ac3) SHA1(469d981b635058dd23e843a3efc555316f87ece4) )     // Typed in by hand from the manual
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY                FULLNAME  FLAGS
COMP( 1976, mmd1,  0,      0,      mmd1,    mmd1,  mmd1_state, empty_init, "E&L Instruments Inc", "MMD-1 Mini-Micro Designer",  MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
