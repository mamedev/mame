// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Ravensburger Selbstbaucomputer

This is a project described in "Ravensburger" magazine. You had to make
the entire thing (including the circuit boards) yourself.


https://web.archive.org/web/20160321001634/http://petersieg.bplaced.com/?2650_Computer:2650_Selbstbaucomputer

2013-04-23 Skeleton driver.

No instructions, no schematics - it's all guesswork.

The cassette saves a noise but it returns a bad load. This is why MNW is set.


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
D    Dump to screen and tape (at the same time) D 00 04 dumps pages 0 to 4
E    Execute
I    Registers? (Esc to quit)
L    Load
R    ? (Esc to quit)
V    Verify

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


namespace {

class ravens_base : public driver_device
{
public:
	ravens_base(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
	{ }

protected:
	int cass_r();
	void cass_w(int state);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	void mem_map(address_map &map) ATTR_COLD;
	required_device<s2650_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
};

class ravens_state : public ravens_base
{
public:
	ravens_state(const machine_config &mconfig, device_type type, const char *tag)
		: ravens_base(mconfig, type, tag)
		, m_io_keyboard(*this, "X%u", 0U)
		, m_digits(*this, "digit%u", 0U)
		, m_leds(*this, "led%u", 0U)
	{ }

	void ravens(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void io_map(address_map &map) ATTR_COLD;
	u8 port17_r();
	void display_w(offs_t offset, u8 data);
	void leds_w(u8 data);

	required_ioport_array<3> m_io_keyboard;
	output_finder<7> m_digits;
	output_finder<8> m_leds;
};

class ravens2_state : public ravens_base
{
public:
	ravens2_state(const machine_config &mconfig, device_type type, const char *tag)
		: ravens_base(mconfig, type, tag)
		, m_terminal(*this, "terminal")
	{ }

	void ravens2(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	void io_map(address_map &map) ATTR_COLD;
	void kbd_put(u8 data);
	u8 port07_r();
	void port1b_w(u8 data);
	void port1c_w(u8 data);
	u8 m_term_out = 0U;
	u8 m_term_in = 0U;
	required_device<generic_terminal_device> m_terminal;
};

void ravens_base::cass_w(int state)
{
	m_cass->output(state ? -1.0 : +1.0);
}

int ravens_base::cass_r()
{
	return (m_cass->input() > 0.03) ? 1 : 0;
}

void ravens_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();
}

void ravens_state::display_w(offs_t offset, u8 data)
{
	m_digits[offset] = data;
}

void ravens_state::leds_w(u8 data)
{
	for (int i = 0; i < 8; i++)
		m_leds[i] = !BIT(data, i);
}

u8 ravens2_state::port07_r()
{
	u8 ret = m_term_in;
	if (!machine().side_effects_disabled())
		m_term_in = 0x80;
	return ret;
}

u8 ravens_state::port17_r()
{
	u8 keyin, i;

	keyin = m_io_keyboard[0]->read();
	if (keyin != 0xff)
		for (i = 0; i < 8; i++)
			if (BIT(~keyin, i))
				return i | 0x80;

	keyin = m_io_keyboard[1]->read();
	if (keyin != 0xff)
		for (i = 0; i < 8; i++)
			if (BIT(~keyin, i))
				return i | 0x88;

	keyin = m_io_keyboard[2]->read();
	if (!BIT(keyin, 0))
		m_maincpu->reset();
	if (keyin != 0xff)
		for (i = 0; i < 8; i++)
			if (BIT(~keyin, i))
				return (i<<4) | 0x80;

	return 0;
}

void ravens2_state::port1b_w(u8 data)
{
	if (BIT(data, 7))
		return;
	if (data == 1) // don't send PSU register to terminal
		return;
	else
	if ((data == 0x08 && m_term_out == 0x20))
		data = 0x0c; // FormFeed
	else
	if ((data == 0x0a && m_term_out == 0x20))
		data = 0x0a; // LineFeed
	else
		data = m_term_out;

	m_terminal->write(data);
}

void ravens2_state::port1c_w(u8 data)
{
	m_term_out = data;
}

void ravens2_state::machine_reset()
{
	m_term_in = 0x80;
}

void ravens2_state::machine_start()
{
	save_item(NAME(m_term_out));
	save_item(NAME(m_term_in));
}

void ravens_base::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).rom();
	map(0x0800, 0x1fff).ram();
	map(0x2000, 0x7FFF).ram(); // for quickload, optional
}

void ravens_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x09, 0x09).w(FUNC(ravens_state::leds_w)); // LED output port
	map(0x10, 0x15).w(FUNC(ravens_state::display_w)); // 6-led display
	map(0x17, 0x17).r(FUNC(ravens_state::port17_r)); // pushbuttons
}

void ravens2_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x07, 0x07).r(FUNC(ravens2_state::port07_r));
	map(0x1b, 0x1b).w(FUNC(ravens2_state::port1b_w));
	map(0x1c, 0x1c).w(FUNC(ravens2_state::port1c_w));
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

void ravens2_state::kbd_put(u8 data)
{
	if (data > 0x60) data -= 0x20; // fold to uppercase
	m_term_in = data;
}

QUICKLOAD_LOAD_MEMBER(ravens_base::quickload_cb)
{
	int const quick_length = image.length();
	if (quick_length < 0x0900)
		return std::make_pair(image_error::INVALIDLENGTH, "File too short");
	else if (quick_length > 0x8000)
		return std::make_pair(image_error::INVALIDLENGTH, "File too long (must be no more than 32K)");

	std::vector<u8> quick_data;
	quick_data.resize(quick_length);
	int const read_ = image.fread( &quick_data[0], quick_length);
	if (read_ != quick_length)
		return std::make_pair(image_error::UNSPECIFIED, "Cannot read the file");
	else if (quick_data[0] != 0xc6)
		return std::make_pair(image_error::INVALIDIMAGE, "Invalid header");

	int const exec_addr = quick_data[2] * 256 + quick_data[3];
	if (exec_addr >= quick_length)
	{
		return std::make_pair(
				image_error::INVALIDIMAGE,
				util::string_format("Exec address %04X beyond end of file %04X", exec_addr, quick_length));
	}

	constexpr int QUICK_ADDR = 0x900;
	address_space &space = m_maincpu->space(AS_PROGRAM);
	for (int i = QUICK_ADDR; i < read_; i++)
		space.write_byte(i, quick_data[i]);

	// display a message about the loaded quickload
	image.message(" Quickload: size=%04X : exec=%04X",quick_length,exec_addr);

	// Start the quickload
	m_maincpu->set_state_int(S2650_PC, exec_addr);

	return std::pair(std::error_condition(), std::string());
}

void ravens_state::ravens(machine_config &config)
{
	/* basic machine hardware */
	S2650(config, m_maincpu, XTAL(1'000'000)); // frequency is unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &ravens_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &ravens_state::io_map);
	m_maincpu->sense_handler().set(FUNC(ravens_state::cass_r));
	m_maincpu->flag_handler().set(FUNC(ravens_state::cass_w));

	/* video hardware */
	config.set_default_layout(layout_ravens);

	/* quickload */
	QUICKLOAD(config, "quickload", "pgm", attotime::from_seconds(1)).set_load_callback(FUNC(ravens_state::quickload_cb));

	SPEAKER(config, "mono").front_center();

	/* cassette */
	CASSETTE(config, m_cass);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}

void ravens2_state::ravens2(machine_config &config)
{
	/* basic machine hardware */
	S2650(config, m_maincpu, XTAL(1'000'000)); // frequency is unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &ravens2_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &ravens2_state::io_map);
	m_maincpu->sense_handler().set(FUNC(ravens2_state::cass_r));
	m_maincpu->flag_handler().set(FUNC(ravens2_state::cass_w));

	/* video hardware */
	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(ravens2_state::kbd_put));

	/* quickload */
	QUICKLOAD(config, "quickload", "pgm", attotime::from_seconds(1)).set_load_callback(FUNC(ravens2_state::quickload_cb));

	SPEAKER(config, "mono").front_center();

	/* cassette */
	CASSETTE(config, m_cass);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}

/* ROM definition */
ROM_START( ravens )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "v1.0", "V1.0" )
	ROMX_LOAD( "mon_v1.0.bin", 0x0000, 0x0800, CRC(785eb1ad) SHA1(c316b8ac32ab6aa37746af37b9f81a23367fedd8), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v0.9", "V0.9" )
	ROMX_LOAD( "mon_v0_9.bin", 0x0000, 0x07b5, CRC(2f9b9178) SHA1(ec2ebbc80ee9ff2502c1409ab4f99127032ed724), ROM_BIOS(1))
ROM_END

ROM_START( ravens2 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mon_v2.0.bin", 0x0000, 0x0800, CRC(bcd47c58) SHA1(f261a3f128fbedbf59a8b5480758fff4d7f76de1))
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS          INIT        COMPANY                            FULLNAME                               FLAGS */
COMP( 1984, ravens,  0,      0,      ravens,  ravens, ravens_state,  empty_init, "Joseph Glagla and Dieter Feiler", "Ravensburger Selbstbaucomputer V0.9", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1985, ravens2, ravens, 0,      ravens2, ravens, ravens2_state, empty_init, "Joseph Glagla and Dieter Feiler", "Ravensburger Selbstbaucomputer V2.0", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
