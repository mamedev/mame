// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        MMD-1 & MMD-2 driver by Miodrag Milanovic

        12/05/2009 Initial version

        2011-JAN-12 MMD2 working {Robbbert]

MMD-1
*****

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

When is keyboard LINE3 scanned? it isn't - it's a reset button.

MMD-2
*****

http://www.cs.unc.edu/~yakowenk/classiccmp/mmd2/
Memory map:

    * 4K RAM addresses $0000..$0FFF
    * ROM addresses $D800..$E7FF
    * 256 bytes of RAM, ($FC00..$FCFF?)

DIP switches:

    * WE 0 - Write enable for addresses $0000..$03FF
    * WE 1 - Write enable for addresses $0400..$07FF
    * WE 2 - Write enable for addresses $0800..$0BFF
    * WE 3 - Write enable for addresses $0C00..$0FFF
    * SPARE - ???
    * HEX OCT - choose display and entry to be in Hexadecimal or Octal
    * PUP RESET - ???
    * EXEC USER - update binary LED's with data entry? Or not?
      (in either setting, outputs to ports 0,1,2 still show)

Operation:

    * Enter bytes on the keypad of hex digits
    * Set MSByte of address by entering it on the keypad & pressing "HIGH".
    * ... LSByte ... "LOW"
    * Change contents of memory at the selected address by entering the new value & pressing "STORE".
    * Look at adjacent memory locations with "NEXT" and "PREV".
    * Execute the program at the selected address by pressing "GO".

AUX functions:

    * BRL HI # - OFF disables BRL LO
    * BRL LO #
    * STEP #
    * SRC HI # - source for COPY/DUMP - OFF disables "DUMP" function
    * DES HI # - destination for COPY/DUMP
    * LEN HI # - length for COPY/DUMP
    * CLR TST ON - test if PROM is empty
    * POP PRM ON - program a PROM
    * DUP TST ON - test if PROM duplicated okay
    * PROM 2708/2716
    * MEM MAP RAM/ROM
    * BAUD 110/150/300/600/1200

The memory map can be rearranged by the system by using IN5, IN6, IN7.
A pair of undumped proms control what goes where on each map.
Each set of ROMs also has its own pair of PROMs.


I/O ports:
IN0: user expansion
IN1: 0-TTYIN, 1-CASSIN, 3-SW8(binary/ports), 4-SW7(reset/pup), 5-SW6(hex/oct), 6-(pup signal)
IN3: 8279 status
IN4: 8279 key
IN5: set MAP1
IN6: set MAP2
IN7: set MAP3
IN8: read eprom (in the eprom programmer)
OUT0: PORT0 LEDs
OUT1: PORT1 LEDs
OUT2: PORT2 LEDs
OUT3: 8279 control
OUT4: 8279 7-segment LED data
OUT5: TTYOUT, CASSOUT
OUT9: turn pup signal off
OUTA: programming pulse on/off (eprom programmer)

Dips:
SW1-4 hardware control of RAM being writable
SW5 not used
SW6 Control if the 7-segment displays show Octal (low) or Hex (high).
SW7 Reset only causes the cpu to perform a warm start. PUP does a cold start (Power-UP).
SW8 Control if the PORT displays echo the 7-segment displays (high), or just act as normal output ports (low).


ToDo
- MMD1 cassette uart ports 0x12/13 (possibly 8251), and circuits to convert to tones
- MMD1 tty uart ports 0x10/11 (possibly 8251), and rs232 interface
- MMD2 Hook up WE0-3
- MMD2 cassette is hooked up but not tested
- MMD2 tty rs232 interface
- Add interrupt module (INTE LED is always on atm)
- Need software
- Lots of other things

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8279.h"
#include "imagedev/cassette.h"
#include "speaker.h"
#include "mmd1.lh"
#include "mmd2.lh"


class mmd1_state : public driver_device
{
public:
	mmd1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
		, m_io_keyboard(*this, "X%u", 0)
		, m_digits(*this, "digit%u", 0U)
		, m_mmd2(false)
		{ }

	void mmd1(machine_config &config);
	void mmd2(machine_config &config);

	void init_mmd2();

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

private:
	DECLARE_WRITE8_MEMBER(mmd1_port0_w);
	DECLARE_WRITE8_MEMBER(mmd1_port1_w);
	DECLARE_WRITE8_MEMBER(mmd1_port2_w);
	DECLARE_READ8_MEMBER(mmd1_keyboard_r);
	DECLARE_WRITE8_MEMBER(cass_w);
	DECLARE_READ8_MEMBER(mmd2_01_r);
	DECLARE_READ8_MEMBER(mmd2_bank_r);
	DECLARE_READ8_MEMBER(mmd2_kbd_r);
	DECLARE_WRITE8_MEMBER(mmd2_scanlines_w);
	DECLARE_WRITE8_MEMBER(mmd2_digit_w);
	DECLARE_WRITE8_MEMBER(mmd2_status_callback);
	DECLARE_WRITE_LINE_MEMBER(mmd2_inte_callback);
	DECLARE_MACHINE_RESET(mmd1);
	DECLARE_MACHINE_RESET(mmd2);
	void mmd1_io(address_map &map);
	void mmd1_mem(address_map &map);
	void mmd2_io(address_map &map);
	void mmd2_mem(address_map &map);
	void reset_banks();

	uint8_t m_return_code;
	uint8_t m_digit;
	virtual void machine_start() override { m_digits.resolve(); }
	required_device<i8080_cpu_device> m_maincpu;
	optional_device<cassette_image_device> m_cass;
	optional_ioport_array<4> m_io_keyboard;
	output_finder<9> m_digits;
	bool m_mmd2;
};


WRITE8_MEMBER( mmd1_state::mmd1_port0_w )
{
	output().set_value("p0_7", BIT(data,7) ? 0 : 1);
	output().set_value("p0_6", BIT(data,6) ? 0 : 1);
	output().set_value("p0_5", BIT(data,5) ? 0 : 1);
	output().set_value("p0_4", BIT(data,4) ? 0 : 1);
	output().set_value("p0_3", BIT(data,3) ? 0 : 1);
	output().set_value("p0_2", BIT(data,2) ? 0 : 1);
	output().set_value("p0_1", BIT(data,1) ? 0 : 1);
	output().set_value("p0_0", BIT(data,0) ? 0 : 1);
}

WRITE8_MEMBER( mmd1_state::mmd1_port1_w )
{
	output().set_value("p1_7", BIT(data,7) ? 0 : 1);
	output().set_value("p1_6", BIT(data,6) ? 0 : 1);
	output().set_value("p1_5", BIT(data,5) ? 0 : 1);
	output().set_value("p1_4", BIT(data,4) ? 0 : 1);
	output().set_value("p1_3", BIT(data,3) ? 0 : 1);
	output().set_value("p1_2", BIT(data,2) ? 0 : 1);
	output().set_value("p1_1", BIT(data,1) ? 0 : 1);
	output().set_value("p1_0", BIT(data,0) ? 0 : 1);
}

WRITE8_MEMBER( mmd1_state::mmd1_port2_w )
{
	output().set_value("p2_7", BIT(data,7) ? 0 : 1);
	output().set_value("p2_6", BIT(data,6) ? 0 : 1);
	output().set_value("p2_5", BIT(data,5) ? 0 : 1);
	output().set_value("p2_4", BIT(data,4) ? 0 : 1);
	output().set_value("p2_3", BIT(data,3) ? 0 : 1);
	output().set_value("p2_2", BIT(data,2) ? 0 : 1);
	output().set_value("p2_1", BIT(data,1) ? 0 : 1);
	output().set_value("p2_0", BIT(data,0) ? 0 : 1);
}

// keyboard has a keydown and a keyup code. Keyup = last keydown + bit 7 set
READ8_MEMBER( mmd1_state::mmd1_keyboard_r )
{
	uint8_t line1 = ioport("LINE1")->read();
	uint8_t line2 = ioport("LINE2")->read();
	uint8_t i, data = 0xff;


	for (i = 0; i < 8; i++)
	{
		if (!BIT(line1, i))
			data = i;
	}

	for (i = 0; i < 8; i++)
	{
		if (!BIT(line2, i))
			data = i+8;
	}

	if (data < 0x10)
	{
		m_return_code = data | 0x80;
		return data;
	}
	else
		return m_return_code;
}

void mmd1_state::mmd1_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).rom(); // Main ROM
	map(0x0100, 0x01ff).rom(); // Expansion slot
	map(0x0200, 0x02ff).ram();
	map(0x0300, 0x03ff).ram();
}

void mmd1_state::mmd1_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x07);
	map(0x00, 0x00).rw(FUNC(mmd1_state::mmd1_keyboard_r), FUNC(mmd1_state::mmd1_port0_w));
	map(0x01, 0x01).w(FUNC(mmd1_state::mmd1_port1_w));
	map(0x02, 0x02).w(FUNC(mmd1_state::mmd1_port2_w));
	//map(0x10, 0x11).rw  TTY UART
	//map(0x12, 0x13).rw  CASS UART
}

void mmd1_state::mmd2_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).bankr("bank1").bankw("bank2");
	map(0x0400, 0x0fff).bankr("bank3").bankw("bank4");
	map(0xd800, 0xe3ff).bankr("bank5").bankw("bank6");
	map(0xe400, 0xe7ff).bankr("bank7").bankw("bank8");
	map(0xfc00, 0xfcff).ram(); // Scratchpad
}

void mmd1_state::mmd2_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x00).w(FUNC(mmd1_state::mmd1_port0_w));
	map(0x01, 0x01).rw(FUNC(mmd1_state::mmd2_01_r), FUNC(mmd1_state::mmd1_port1_w));
	map(0x02, 0x02).w(FUNC(mmd1_state::mmd1_port2_w));
	map(0x03, 0x03).rw("i8279", FUNC(i8279_device::status_r), FUNC(i8279_device::cmd_w));
	map(0x04, 0x04).rw("i8279", FUNC(i8279_device::data_r), FUNC(i8279_device::data_w));
	map(0x05, 0x07).r(FUNC(mmd1_state::mmd2_bank_r));
	map(0x05, 0x05).w(FUNC(mmd1_state::cass_w));
	//map(0x09, 0x09).w  PUP signal
	//map(0x0a, 0x0a).w  Eprom programmer
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
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_LALT) PORT_CHANGED_MEMBER(DEVICE_SELF, mmd1_state, reset_button, nullptr)
INPUT_PORTS_END

static INPUT_PORTS_START( mmd2 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x20, 0x00, "Sw6") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x20, "Hex")
	PORT_DIPSETTING(    0x00, "Octal")
	PORT_DIPNAME( 0x10, 0x10, "Sw7") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x10, "PUP")
	PORT_DIPSETTING(    0x00, "Reset")
	PORT_DIPNAME( 0x08, 0x08, "Sw8") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x08, "Exec")
	PORT_DIPSETTING(    0x00, "User")

	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CANCEL") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("AUX") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("REGS") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MEM") PORT_CODE(KEYCODE_MINUS)
	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COPY") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PROM") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DUMP") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LOAD") PORT_CODE(KEYCODE_O)
	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("OPTION") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GO") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LOW") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("HIGH") PORT_CODE(KEYCODE_H)
	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STEP") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NEXT") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STORE") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PREV") PORT_CODE(KEYCODE_DOWN)
	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET") PORT_CODE(KEYCODE_LALT) PORT_CHANGED_MEMBER(DEVICE_SELF, mmd1_state, reset_button, nullptr)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(mmd1_state::reset_button)
{
	if (newval && m_mmd2)
		reset_banks();
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}

/*
Keyboard
0  1  2  3      PREV  STORE  NEXT  STEP
4  5  6  7      HIGH  LOW  GO  OPTION
8  9  A  B      LOAD  DUMP  PROM  COPY
C  D  E  F      MEM  REGS  AUX  CANCEL

*/

READ8_MEMBER( mmd1_state::mmd2_bank_r )
{
	membank("bank1")->set_entry(offset);
	membank("bank2")->set_entry(offset);
	membank("bank3")->set_entry(offset);
	membank("bank4")->set_entry(offset);
	membank("bank5")->set_entry(offset);
	membank("bank6")->set_entry(offset);
	membank("bank7")->set_entry(offset);
	membank("bank8")->set_entry(offset);
	return 0xff;
}

READ8_MEMBER( mmd1_state::mmd2_01_r )
{
	// need to add ttyin bit 0
	uint8_t data = 0x84;
	data |= ioport("DSW")->read();
	data |= (m_cass->input() < 0.02) ? 0 : 2;
	return data;
}

WRITE8_MEMBER( mmd1_state::cass_w )
{
	// need to add ttyout bit 0
	m_cass->output(BIT(data, 2) ? -1.0 : +1.0);
}

WRITE8_MEMBER( mmd1_state::mmd2_scanlines_w )
{
	m_digit = data;
}

WRITE8_MEMBER( mmd1_state::mmd2_digit_w )
{
	if (m_digit < 9)
		m_digits[m_digit] = data;
}

READ8_MEMBER( mmd1_state::mmd2_kbd_r )
{
	uint8_t data = 0xff;

	if (m_digit < 4)
		data = m_io_keyboard[m_digit]->read();

	return data;
}

WRITE8_MEMBER( mmd1_state::mmd2_status_callback )
{
	// operate the HALT LED
	output().set_value("led_halt", ~data & i8080_cpu_device::STATUS_HLTA);
	// operate the HOLD LED - this should connect to the HLDA pin,
	// but it isn't emulated, using WO instead (whatever that does).
	output().set_value("led_hold", data & i8080_cpu_device::STATUS_WO);
}

WRITE_LINE_MEMBER( mmd1_state::mmd2_inte_callback )
{
	// operate the INTE LED
	output().set_value("led_inte", state);
}

MACHINE_RESET_MEMBER(mmd1_state,mmd1)
{
	m_return_code = 0xff;
}

MACHINE_RESET_MEMBER(mmd1_state,mmd2)
{
	reset_banks();
}

void mmd1_state::reset_banks()
{
	membank("bank1")->set_entry(0);
	membank("bank2")->set_entry(0);
	membank("bank3")->set_entry(0);
	membank("bank4")->set_entry(0);
	membank("bank5")->set_entry(0);
	membank("bank6")->set_entry(0);
	membank("bank7")->set_entry(0);
	membank("bank8")->set_entry(0);
}

void mmd1_state::init_mmd2()
{
/*
We preset all banks here, so that bankswitching will incur no speed penalty.
0000/0400 indicate ROMs, D800/DC00/E400 indicate RAM, 8000 is a dummy write area for ROM banks.
*/
	uint8_t *p_ram = memregion("maincpu")->base();
	membank("bank1")->configure_entry(0, &p_ram[0x0000]);
	membank("bank1")->configure_entry(1, &p_ram[0xd800]);
	membank("bank1")->configure_entry(2, &p_ram[0x0c00]);
	membank("bank2")->configure_entry(0, &p_ram[0x8000]);
	membank("bank2")->configure_entry(1, &p_ram[0xd800]);
	membank("bank2")->configure_entry(2, &p_ram[0x8000]);
	membank("bank3")->configure_entry(0, &p_ram[0x0400]);
	membank("bank3")->configure_entry(1, &p_ram[0xdc00]);
	membank("bank3")->configure_entry(2, &p_ram[0xdc00]);
	membank("bank4")->configure_entry(0, &p_ram[0x8000]);
	membank("bank4")->configure_entry(1, &p_ram[0xdc00]);
	membank("bank4")->configure_entry(2, &p_ram[0xdc00]);
	membank("bank5")->configure_entry(0, &p_ram[0xd800]);
	membank("bank5")->configure_entry(1, &p_ram[0x0000]);
	membank("bank5")->configure_entry(2, &p_ram[0x0000]);
	membank("bank6")->configure_entry(0, &p_ram[0xd800]);
	membank("bank6")->configure_entry(1, &p_ram[0x8000]);
	membank("bank6")->configure_entry(2, &p_ram[0x8000]);
	membank("bank7")->configure_entry(0, &p_ram[0xe400]);
	membank("bank7")->configure_entry(1, &p_ram[0x0c00]);
	membank("bank7")->configure_entry(2, &p_ram[0xd800]);
	membank("bank8")->configure_entry(0, &p_ram[0xe400]);
	membank("bank8")->configure_entry(1, &p_ram[0x8000]);
	membank("bank8")->configure_entry(2, &p_ram[0xd800]);
	m_mmd2 = true;
}

void mmd1_state::mmd1(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, 6750000 / 9);
	m_maincpu->set_addrmap(AS_PROGRAM, &mmd1_state::mmd1_mem);
	m_maincpu->set_addrmap(AS_IO, &mmd1_state::mmd1_io);

	MCFG_MACHINE_RESET_OVERRIDE(mmd1_state,mmd1)

	/* video hardware */
	config.set_default_layout(layout_mmd1);
}

void mmd1_state::mmd2(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, 6750000 / 9);
	m_maincpu->set_addrmap(AS_PROGRAM, &mmd1_state::mmd2_mem);
	m_maincpu->set_addrmap(AS_IO, &mmd1_state::mmd2_io);
	m_maincpu->out_status_func().set(FUNC(mmd1_state::mmd2_status_callback));
	m_maincpu->out_inte_func().set(FUNC(mmd1_state::mmd2_inte_callback));

	MCFG_MACHINE_RESET_OVERRIDE(mmd1_state,mmd2)

	/* video hardware */
	config.set_default_layout(layout_mmd2);

	/* Devices */
	i8279_device &kbdc(I8279(config, "i8279", 400000));             // based on divider
	kbdc.out_sl_callback().set(FUNC(mmd1_state::mmd2_scanlines_w)); // scan SL lines
	kbdc.out_disp_callback().set(FUNC(mmd1_state::mmd2_digit_w));   // display A&B
	kbdc.in_rl_callback().set(FUNC(mmd1_state::mmd2_kbd_r));        // kbd RL lines
	kbdc.in_shift_callback().set_constant(1);                       // Shift key
	kbdc.in_ctrl_callback().set_constant(1);

	SPEAKER(config, "mono").front_center();

	/* Cassette */
	CASSETTE(config, m_cass);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}

/* ROM definition */
ROM_START( mmd1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "kex.ic15", 0x0000, 0x0100, CRC(434f6923) SHA1(a2af60deda54c8d3f175b894b47ff554eb37e9cb))
ROM_END

ROM_START( mmd2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "mmd2330.bin", 0x0000, 0x0800, CRC(69a77199) SHA1(6c83093b2c32a558c969f4fe8474b234023cc348))
	ROM_LOAD( "mmd2340.bin", 0x0800, 0x0800, CRC(70681bd6) SHA1(c37e3cf34a75e8538471030bb49b8aed45d00ec3))
	ROM_LOAD( "mmd2350.bin", 0x1000, 0x0800, CRC(359f577c) SHA1(9405ca0c1977721e4540a4017907c06dab08d398))
	ROM_LOAD( "mmd2360.bin", 0x1800, 0x0800, CRC(967e69b8) SHA1(c21ec8bef955806a2c6e1b1c8e9068662fb88038))
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY                FULLNAME  FLAGS
COMP( 1976, mmd1,  0,      0,      mmd1,    mmd1,  mmd1_state, empty_init, "E&L Instruments Inc", "MMD-1",  MACHINE_NO_SOUND_HW )
COMP( 1976, mmd2,  mmd1,   0,      mmd2,    mmd2,  mmd1_state, init_mmd2,  "E&L Instruments Inc", "MMD-2",  MACHINE_NO_SOUND_HW )
