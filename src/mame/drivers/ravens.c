// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Ravensburger Selbstbaucomputer

This is a project described in "Ravensburger" magazine. You had to make
the entire thing (including the circuit boards) yourself.

http://petersieg.bplaced.com/?2650_Computer:2650_Selbstbaucomputer

        2013-04-23 Skeleton driver.


No instructions, no schematics - it's all guesswork.

The cassette saves a noise but it returns a bad load.


Version 0.9
-----------
Hardware:
        0000-07FF ROM "MON1"
        0800-1FFF RAM (3x HM6116)
        24 pushbuttons and 6-digit LED display on front panel.
        Other buttons and switches on the main board.

The few photos show the CPU and a number of ordinary 74LSxxx chips.
There is a XTAL of unknown frequency.

The buttons are labelled CMD, RUN, GOTO, RST, F, MON, PC, NXT but at
this time not all buttons are identified.

What is known:
- Press NXT to read memory. Press NXT again to read the next address.
- Press PC and it says PCxxxx
- Press CMD, it says CND=, you can choose one of these:
-- A displays value of a register. Press A again to see more registers.
-- B sets a breakpoint
-- C clears a breakpoint
-- D dumps blocks to tape
-- E examine tape file
-- F fetch (load) from tape

Quickload: Load the program then press Y. There are 6 that work and
           6 that do nothing.

ToDo:
- Cassette

Version V2.0
------------
This used a terminal interface with a few non-standard control codes.
The pushbuttons and LEDs appear to have been done away with.

Commands (must be in uppercase):
A    Examine memory; press C to alter memory
B    Set breakpoint?
C    View breakpoint?
D    Dump to screen and tape (at the same time)
E    Execute
I    ?
L    Load
R    ?
V    Verify?

ToDo:
- Cassette

****************************************************************************/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/terminal.h"
#include "sound/wave.h"
#include "ravens.lh"

#define TERMINAL_TAG "terminal"

class ravens_state : public driver_device
{
public:
	ravens_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG),
		m_cass(*this, "cassette")
	{
	}

	DECLARE_READ8_MEMBER(port07_r);
	DECLARE_READ8_MEMBER(port17_r);
	DECLARE_WRITE8_MEMBER(port1b_w);
	DECLARE_WRITE8_MEMBER(port1c_w);
	DECLARE_WRITE8_MEMBER(display_w);
	DECLARE_WRITE8_MEMBER(leds_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_MACHINE_RESET(ravens2);
	DECLARE_READ8_MEMBER(cass_r);
	DECLARE_WRITE_LINE_MEMBER(cass_w);
	DECLARE_QUICKLOAD_LOAD_MEMBER( ravens );
	UINT8 m_term_char;
	UINT8 m_term_data;
	required_device<cpu_device> m_maincpu;
	optional_device<generic_terminal_device> m_terminal;
	required_device<cassette_image_device> m_cass;
};

WRITE_LINE_MEMBER( ravens_state::cass_w )
{
	m_cass->output(state ? -1.0 : +1.0);
}

READ8_MEMBER( ravens_state::cass_r )
{
	return (m_cass->input() > 0.03) ? 1 : 0;
}

WRITE8_MEMBER( ravens_state::display_w )
{
	output_set_digit_value(offset, data);
}

WRITE8_MEMBER( ravens_state::leds_w )
{
	char ledname[8];
	for (int i = 0; i < 8; i++)
	{
		sprintf(ledname,"led%d",i);
		output_set_value(ledname, !BIT(data, i));
	}
}

READ8_MEMBER( ravens_state::port07_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0x80;
	return ret;
}

READ8_MEMBER( ravens_state::port17_r )
{
	UINT8 keyin, i;

	keyin = ioport("X0")->read();
	if (keyin != 0xff)
		for (i = 0; i < 8; i++)
			if BIT(~keyin, i)
				return i | 0x80;

	keyin = ioport("X1")->read();
	if (keyin != 0xff)
		for (i = 0; i < 8; i++)
			if BIT(~keyin, i)
				return i | 0x88;

	keyin = ioport("X2")->read();
	if (!BIT(keyin, 0))
		m_maincpu->reset();
	if (keyin != 0xff)
		for (i = 0; i < 8; i++)
			if BIT(~keyin, i)
				return (i<<4) | 0x80;

	return 0;
}

WRITE8_MEMBER( ravens_state::port1b_w )
{
	if (BIT(data, 7))
		return;
	else
	if ((data == 0x08 && m_term_char == 0x20))
		data = 0x0c; // FormFeed
	else
	if ((data == 0x0a && m_term_char == 0x20))
		data = 0x0a; // LineFeed
	else
	if ((data == 0x01 && m_term_char == 0xc2))
		data = 0x0d; // CarriageReturn
	else
		data = m_term_char;

	m_terminal->write(space, 0, data);
}

WRITE8_MEMBER( ravens_state::port1c_w )
{
	m_term_char = data;
}

MACHINE_RESET_MEMBER( ravens_state, ravens2 )
{
	m_term_data = 0x80;
	output_set_digit_value(6, 0);
}


static ADDRESS_MAP_START( ravens_mem, AS_PROGRAM, 8, ravens_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x07ff) AM_ROM
	AM_RANGE( 0x0800, 0x1fff) AM_RAM
	AM_RANGE( 0x2000, 0x7FFF) AM_RAM // for quickload, optional
ADDRESS_MAP_END

static ADDRESS_MAP_START( ravens_io, AS_IO, 8, ravens_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x09, 0x09) AM_WRITE(leds_w) // LED output port
	AM_RANGE(0x10, 0x15) AM_WRITE(display_w) // 6-led display
	AM_RANGE(0x17, 0x17) AM_READ(port17_r) // pushbuttons
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ(cass_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ravens2_io, AS_IO, 8, ravens_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x07, 0x07) AM_READ(port07_r)
	AM_RANGE(0x1b, 0x1b) AM_WRITE(port1b_w)
	AM_RANGE(0x1c, 0x1c) AM_WRITE(port1c_w)
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ(cass_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( ravens )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RST") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NXT") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RUN") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PC")  PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("???") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("???") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CMD") PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("???") PORT_CODE(KEYCODE_O) PORT_CHAR('O')
INPUT_PORTS_END

WRITE8_MEMBER( ravens_state::kbd_put )
{
	if (data > 0x60) data -= 0x20; // fold to uppercase
	m_term_data = data;
}

QUICKLOAD_LOAD_MEMBER( ravens_state, ravens )
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int i;
	int quick_addr = 0x900;
	int exec_addr;
	int quick_length;
	dynamic_buffer quick_data;
	int read_;
	int result = IMAGE_INIT_FAIL;

	quick_length = image.length();
	if (quick_length < 0x0900)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "File too short");
		image.message(" File too short");
	}
	else if (quick_length > 0x8000)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "File too long");
		image.message(" File too long");
	}
	else
	{
		quick_data.resize(quick_length);
		read_ = image.fread( &quick_data[0], quick_length);
		if (read_ != quick_length)
		{
			image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Cannot read the file");
			image.message(" Cannot read the file");
		}
		else if (quick_data[0] != 0xc6)
		{
			image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Invalid header");
			image.message(" Invalid header");
		}
		else
		{
			exec_addr = quick_data[2] * 256 + quick_data[3];

			if (exec_addr >= quick_length)
			{
				image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Exec address beyond end of file");
				image.message(" Exec address beyond end of file");
			}
			else
			{
				for (i = quick_addr; i < read_; i++)
					space.write_byte(i, quick_data[i]);

				/* display a message about the loaded quickload */
				image.message(" Quickload: size=%04X : exec=%04X",quick_length,exec_addr);

				// Start the quickload
				m_maincpu->set_state_int(S2650_PC, exec_addr);

				result = IMAGE_INIT_PASS;
			}
		}
	}

	return result;
}

static MACHINE_CONFIG_START( ravens, ravens_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",S2650, XTAL_1MHz) // frequency is unknown
	MCFG_CPU_PROGRAM_MAP(ravens_mem)
	MCFG_CPU_IO_MAP(ravens_io)
	MCFG_S2650_FLAG_HANDLER(WRITELINE(ravens_state, cass_w))

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_ravens)

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", ravens_state, ravens, "pgm", 1)

	/* cassette */
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ravens2, ravens_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",S2650, XTAL_1MHz) // frequency is unknown
	MCFG_CPU_PROGRAM_MAP(ravens_mem)
	MCFG_CPU_IO_MAP(ravens2_io)
	MCFG_S2650_FLAG_HANDLER(WRITELINE(ravens_state, cass_w))

	MCFG_MACHINE_RESET_OVERRIDE(ravens_state, ravens2)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(ravens_state, kbd_put))

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", ravens_state, ravens, "pgm", 1)

	/* cassette */
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ravens )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "v1.0", "V1.0" )
	ROMX_LOAD( "mon_v1.0.bin", 0x0000, 0x0800, CRC(785eb1ad) SHA1(c316b8ac32ab6aa37746af37b9f81a23367fedd8), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v0.9", "V0.9" )
	ROMX_LOAD( "mon_v0_9.bin", 0x0000, 0x07b5, CRC(2f9b9178) SHA1(ec2ebbc80ee9ff2502c1409ab4f99127032ed724), ROM_BIOS(2))
ROM_END

ROM_START( ravens2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "mon_v2.0.bin", 0x0000, 0x0800, CRC(bcd47c58) SHA1(f261a3f128fbedbf59a8b5480758fff4d7f76de1))
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT   COMPAT   MACHINE  INPUT   CLASS        INIT     COMPANY    FULLNAME       FLAGS */
COMP( 1984, ravens,  0,       0,       ravens,  ravens, driver_device, 0, "Joseph Glagla and Dieter Feiler", "Ravensburger Selbstbaucomputer V0.9", MACHINE_NO_SOUND_HW )
COMP( 1985, ravens2, ravens,  0,       ravens2, ravens, driver_device, 0, "Joseph Glagla and Dieter Feiler", "Ravensburger Selbstbaucomputer V2.0", MACHINE_NO_SOUND_HW )
