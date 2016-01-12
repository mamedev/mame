// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

2011-JUL-16 SLC1 skeleton driver [Robbbert]
2011-DEC-29 Working [Robbbert]

http://www.jens-mueller.org/jkcemu/slc1.html

This computer is both a Z80 trainer, and a chess computer.
        The keyboard is different between the two, so
        we redefine it for your convenience.

        There is no chess board attached. You supply your own
        and you sync the pieces and the computer instructions.

        When started, it is in Chess mode. Press 11111 to switch to
        Trainer mode.

Hardware
        4 Kbytes ROM in the address range 0000-0FFF
        1 Kbyte RAM in the address range 5000-53ff (user area starts at 5100)
        6-digit 7-segment display
        Busy LED
        Keyboard with 12 keys

Keys:
        0-7 : hexadecimal numbers
        Shift then 0-7 : Hexadecimal 8-F (decimal points will appear)
        ADR : enter an address to work with. After the 4 digits are entered,
              the data at that address shows, and you can modify the data.
        + (inc) : Enter the data into memory, and increment the address by 1.

Pasting doesn't work, but if it did...

    Pasting:
        0-7 : as is
        8-F : H, then 0-7
        + : ^
        - : H^
        ADR : -

    Test Paste:
        [[[[[-510011^22^33^44^55^66^77^H8H8^H9H9^-5100
        Now press up-arrow to confirm the data has been entered.



***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/speaker.h"
#include "slc1.lh"


class slc1_state : public driver_device
{
public:
	slc1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	DECLARE_READ8_MEMBER( io_r );
	DECLARE_WRITE8_MEMBER( io_w );
	UINT8 m_digit;
	bool m_kbd_type;
	virtual void machine_reset() override;
	virtual void machine_start() override;
};




/***************************************************************************

    Display

***************************************************************************/

WRITE8_MEMBER( slc1_state::io_w )
{
	bool segonoff = BIT(data, 7);
	bool busyled = BIT(data, 4);
	data &= 15;

	if (data < 8)
		m_digit = data;
	else
	if (data < 12)
	{
		m_speaker->level_w(BIT(data, 1));
		return;
	}
	else
	if (offset == 0x2f07)
		return;

	UINT8 segdata = output().get_digit_value(m_digit);
	UINT8 segnum  = offset & 7;
	UINT8 segmask = 1 << segnum;

	if (segonoff)
		segdata |= segmask;
	else
		segdata &= ~segmask;

	output().set_digit_value(m_digit, segdata);

	output().set_value("busyled", busyled);

	if (m_digit == 3)
		m_kbd_type = (segdata);
}


/***************************************************************************

    Keyboard

***************************************************************************/

READ8_MEMBER( slc1_state::io_r )
{
	UINT8 data = 0xff, upper = (offset >> 8) & 7;

	if (m_kbd_type)
	{ // Trainer
		if (upper == 3)
			data &= ioport("Y0")->read();
		else
		if (upper == 4)
			data &= ioport("Y1")->read();
		else
		if (upper == 5)
			data &= ioport("Y2")->read();
	}
	else
	{ // Chess
		if (upper == 3)
			data &= ioport("X0")->read();
		else
		if (upper == 4)
			data &= ioport("X1")->read();
		else
		if (upper == 5)
			data &= ioport("X2")->read();
	}

	return data;
}



/***************************************************************************

    Machine

***************************************************************************/

void slc1_state::machine_start()
{
}

void slc1_state::machine_reset()
{
}



/***************************************************************************

    Address Map

***************************************************************************/

static ADDRESS_MAP_START( slc1_map, AS_PROGRAM, 8, slc1_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x4fff)
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x5000, 0x53ff) AM_RAM AM_MIRROR(0xc00)
ADDRESS_MAP_END

static ADDRESS_MAP_START( slc1_io, AS_IO, 8, slc1_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(io_r,io_w)
ADDRESS_MAP_END


/**************************************************************************

    Keyboard Layout

***************************************************************************/

static INPUT_PORTS_START( slc1 )
// Chess Keyboard
	PORT_START("X0")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D4 T") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C3 L") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B2 S") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A1 B") PORT_CODE(KEYCODE_1) PORT_CHAR('[')

	PORT_START("X1")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E5 D") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F6 K") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G7")   PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H8")   PORT_CODE(KEYCODE_8)

	PORT_START("X2")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C")    PORT_CODE(KEYCODE_C)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")    PORT_CODE(KEYCODE_A)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("St")   PORT_CODE(KEYCODE_S)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z")    PORT_CODE(KEYCODE_Z)

// Trainer Keyboard
	PORT_START("Y0")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 B")  PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 A")  PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 9")  PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 8")  PORT_CODE(KEYCODE_0) PORT_CHAR('0')

	PORT_START("Y1")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 C INS") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 D DEL") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 E BL")  PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 F Go")  PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("Y2")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Fu DP") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR('H')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+-1 SS") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Seq BG") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ADR BP") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
INPUT_PORTS_END


/***************************************************************************

    Machine driver

***************************************************************************/

static MACHINE_CONFIG_START( slc1, slc1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 2500000)
	MCFG_CPU_PROGRAM_MAP(slc1_map)
	MCFG_CPU_IO_MAP(slc1_io)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_slc1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


/***************************************************************************

    Game driver

***************************************************************************/

ROM_START(slc1)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "bios0", "SLC-1")
	ROMX_LOAD("slc1_0000.bin",   0x0000, 0x1000, CRC(06d32967) SHA1(f25eac66a4fca9383964d509c671a7ad2e020e7e), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "bios1", "SC-1 v2")
	ROMX_LOAD("sc1-v2.bin",      0x0000, 0x1000, CRC(1f122a85) SHA1(d60f89f8b59d04f4cecd6e3ecfe0a24152462a36), ROM_BIOS(2) )
ROM_END


/*    YEAR  NAME      PARENT  COMPAT  MACHINE     INPUT    INIT      COMPANY    FULLNAME */
COMP( 1989, slc1,     0,      0,      slc1,       slc1, driver_device,    0,      "Dr. Dieter Scheuschner",  "SLC-1" , 0 )
