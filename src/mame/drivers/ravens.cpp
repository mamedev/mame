// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Ravensburger Selbstbaucomputer

This is a project described in "Ravensburger" magazine. You had to make
the entire thing (including the circuit boards) yourself.


https://web.archive.org/web/20160321001634/http://petersieg.bplaced.com/?2650_Computer:2650_Selbstbaucomputer
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
#include "speaker.h"

#include "ravens.lh"


class ravens_state : public driver_device
{
public:
	ravens_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
		, m_cass(*this, "cassette")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void ravens(machine_config &config);
	void ravens2(machine_config &config);

private:
	DECLARE_READ8_MEMBER(port07_r);
	DECLARE_READ8_MEMBER(port17_r);
	DECLARE_WRITE8_MEMBER(port1b_w);
	DECLARE_WRITE8_MEMBER(port1c_w);
	DECLARE_WRITE8_MEMBER(display_w);
	DECLARE_WRITE8_MEMBER(leds_w);
	void kbd_put(u8 data);
	DECLARE_MACHINE_RESET(ravens2);
	DECLARE_READ_LINE_MEMBER(cass_r);
	DECLARE_WRITE_LINE_MEMBER(cass_w);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	void ravens2_io(address_map &map);
	void ravens_io(address_map &map);
	void ravens_mem(address_map &map);

	uint8_t m_term_char;
	uint8_t m_term_data;
	virtual void machine_start() override { m_digits.resolve(); }
	required_device<s2650_device> m_maincpu;
	optional_device<generic_terminal_device> m_terminal;
	required_device<cassette_image_device> m_cass;
	output_finder<7> m_digits;
};

WRITE_LINE_MEMBER( ravens_state::cass_w )
{
	m_cass->output(state ? -1.0 : +1.0);
}

READ_LINE_MEMBER( ravens_state::cass_r )
{
	return (m_cass->input() > 0.03) ? 1 : 0;
}

WRITE8_MEMBER( ravens_state::display_w )
{
	m_digits[offset] = data;
}

WRITE8_MEMBER( ravens_state::leds_w )
{
	char ledname[8];
	for (int i = 0; i < 8; i++)
	{
		sprintf(ledname,"led%d",i);
		output().set_value(ledname, !BIT(data, i));
	}
}

READ8_MEMBER( ravens_state::port07_r )
{
	uint8_t ret = m_term_data;
	m_term_data = 0x80;
	return ret;
}

READ8_MEMBER( ravens_state::port17_r )
{
	uint8_t keyin, i;

	keyin = ioport("X0")->read();
	if (keyin != 0xff)
		for (i = 0; i < 8; i++)
			if (BIT(~keyin, i))
				return i | 0x80;

	keyin = ioport("X1")->read();
	if (keyin != 0xff)
		for (i = 0; i < 8; i++)
			if (BIT(~keyin, i))
				return i | 0x88;

	keyin = ioport("X2")->read();
	if (!BIT(keyin, 0))
		m_maincpu->reset();
	if (keyin != 0xff)
		for (i = 0; i < 8; i++)
			if (BIT(~keyin, i))
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

	m_terminal->write(data);
}

WRITE8_MEMBER( ravens_state::port1c_w )
{
	m_term_char = data;
}

MACHINE_RESET_MEMBER( ravens_state, ravens2 )
{
	m_term_data = 0x80;
	m_digits[6] = 0;
}


void ravens_state::ravens_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).rom();
	map(0x0800, 0x1fff).ram();
	map(0x2000, 0x7FFF).ram(); // for quickload, optional
}

void ravens_state::ravens_io(address_map &map)
{
	map.unmap_value_high();
	map(0x09, 0x09).w(FUNC(ravens_state::leds_w)); // LED output port
	map(0x10, 0x15).w(FUNC(ravens_state::display_w)); // 6-led display
	map(0x17, 0x17).r(FUNC(ravens_state::port17_r)); // pushbuttons
}

void ravens_state::ravens2_io(address_map &map)
{
	map.unmap_value_high();
	map(0x07, 0x07).r(FUNC(ravens_state::port07_r));
	map(0x1b, 0x1b).w(FUNC(ravens_state::port1b_w));
	map(0x1c, 0x1c).w(FUNC(ravens_state::port1c_w));
}

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

void ravens_state::kbd_put(u8 data)
{
	if (data > 0x60) data -= 0x20; // fold to uppercase
	m_term_data = data;
}

QUICKLOAD_LOAD_MEMBER(ravens_state::quickload_cb)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int i;
	int quick_addr = 0x900;
	int exec_addr;
	int quick_length;
	std::vector<uint8_t> quick_data;
	int read_;
	image_init_result result = image_init_result::FAIL;

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

				result = image_init_result::PASS;
			}
		}
	}

	return result;
}

void ravens_state::ravens(machine_config &config)
{
	/* basic machine hardware */
	S2650(config, m_maincpu, XTAL(1'000'000)); // frequency is unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &ravens_state::ravens_mem);
	m_maincpu->set_addrmap(AS_IO, &ravens_state::ravens_io);
	m_maincpu->sense_handler().set(FUNC(ravens_state::cass_r));
	m_maincpu->flag_handler().set(FUNC(ravens_state::cass_w));

	/* video hardware */
	config.set_default_layout(layout_ravens);

	/* quickload */
	QUICKLOAD(config, "quickload", "pgm", attotime::from_seconds(1)).set_load_callback(FUNC(ravens_state::quickload_cb), this);

	SPEAKER(config, "mono").front_center();

	/* cassette */
	CASSETTE(config, m_cass);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}

void ravens_state::ravens2(machine_config &config)
{
	/* basic machine hardware */
	S2650(config, m_maincpu, XTAL(1'000'000)); // frequency is unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &ravens_state::ravens_mem);
	m_maincpu->set_addrmap(AS_IO, &ravens_state::ravens2_io);
	m_maincpu->sense_handler().set(FUNC(ravens_state::cass_r));
	m_maincpu->flag_handler().set(FUNC(ravens_state::cass_w));

	MCFG_MACHINE_RESET_OVERRIDE(ravens_state, ravens2)

	/* video hardware */
	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(ravens_state::kbd_put));

	/* quickload */
	QUICKLOAD(config, "quickload", "pgm", attotime::from_seconds(1)).set_load_callback(FUNC(ravens_state::quickload_cb), this);

	SPEAKER(config, "mono").front_center();

	/* cassette */
	CASSETTE(config, m_cass);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}

/* ROM definition */
ROM_START( ravens )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "v1.0", "V1.0" )
	ROMX_LOAD( "mon_v1.0.bin", 0x0000, 0x0800, CRC(785eb1ad) SHA1(c316b8ac32ab6aa37746af37b9f81a23367fedd8), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v0.9", "V0.9" )
	ROMX_LOAD( "mon_v0_9.bin", 0x0000, 0x07b5, CRC(2f9b9178) SHA1(ec2ebbc80ee9ff2502c1409ab4f99127032ed724), ROM_BIOS(1))
ROM_END

ROM_START( ravens2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "mon_v2.0.bin", 0x0000, 0x0800, CRC(bcd47c58) SHA1(f261a3f128fbedbf59a8b5480758fff4d7f76de1))
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                            FULLNAME                               FLAGS */
COMP( 1984, ravens,  0,      0,      ravens,  ravens, ravens_state, empty_init, "Joseph Glagla and Dieter Feiler", "Ravensburger Selbstbaucomputer V0.9", MACHINE_NO_SOUND_HW )
COMP( 1985, ravens2, ravens, 0,      ravens2, ravens, ravens_state, empty_init, "Joseph Glagla and Dieter Feiler", "Ravensburger Selbstbaucomputer V2.0", MACHINE_NO_SOUND_HW )
