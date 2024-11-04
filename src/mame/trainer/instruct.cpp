// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Signetics Instructor 50

2010-04-08 Skeleton driver.
2012-05-20 Connected digits, system boots. [Robbbert]
2012-05-20 Connected keyboard, system mostly usable. [Robbbert]
2013-10-15 Fixed various regressions. [Robbbert]

From looking at a blurry picture of it, this is what I can determine:
    - Left side: 8 toggle switches, with a round red led above each one.
    - Below this is the Port Address Switch with choice of 'Non-Extended', 'Extended' or 'Memory'.
    - To the right of this is another toggle switch labelled 'Interrupt', the
      choices are 'Direct' and 'Indirect'.
    - Above this switch are 2 more round red leds: FLAG and RUN.
    - Middle: a 4 down x3 across keypad containing the function keys. The
      labels (from left to right, top to bottom) are:
      SENS, WCAS, BKPT, INT, RCAS, REG, MON, STEP, MEM, RST, RUN, ENT/NXT.
    - Right side: a 4x4 hexadecimal keypad. The keys are:
      C, D, E, F, 8, 9, A, B, 4, 5, 6, 7, 0, 1, 2, 3
    - Above, extending from one side to the other is a metal plate with
      printed mnemonics. At the right edge are sockets to connect up the
      MIC and EAR cords to a cassette player.
    - At the back is a S100 interface.

Quick usage:
    - Look at memory: Press minus key. Enter an address. Press UP key to see the next.
    - Look at registers: Press R. Press 0. Press UP key to see the next.
    - Set PC register: Press R. Press C. Type in new address, Press UP.
    - Load a tape: Press L, enter file number (1 digit), press UP. On
      completion of a successful load, HELLO will be displayed.

Pasting a test program: (page 2-4 of the user manual, modified)
    - Paste this: QRF0^751120F005000620FA7EF97A84011F0003-0P
    - You should see the LEDs flashing as they count upwards.

ToDO:
    - Connect round led for Run.
    - Last Address Register
    - Initial Jump Logic
    - Single-step and Breakpoint don't stop execution because of the above.
    - The "Port Address Switch" which selects which of the 3 sources will
      be used for port_r and port_w. Currently all 3 are selected at once.

****************************************************************************/

#include "emu.h"

#include "cpu/s2650/s2650.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "speaker.h"
#include "video/pwm.h"

#include "instruct.lh"


namespace {

class instruct_state : public driver_device
{
public:
	instruct_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_ram(*this, "mainram")
		, m_p_smiram(*this, "smiram")
		, m_p_extram(*this, "extram")
		, m_cass(*this, "cassette")
		, m_display(*this, "display")
		, m_io_keyboard(*this, "X%u", 0U)
		, m_leds(*this, "led%u", 0U)
	{ }

	void instruct(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	uint8_t port_r();
	uint8_t portfc_r();
	uint8_t portfd_r();
	uint8_t portfe_r();
	int sense_r();
	void flag_w(int state);
	void port_w(uint8_t data);
	void portf8_w(uint8_t data);
	void portf9_w(uint8_t data);
	void portfa_w(uint8_t data);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	INTERRUPT_GEN_MEMBER(t2l_int);
	void data_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	uint16_t m_lar = 0U;
	uint8_t m_digit = 0U;
	u8 m_seg = 0U;
	bool m_cassin = 0;
	bool m_irqstate = 0;
	required_device<s2650_device> m_maincpu;
	required_shared_ptr<uint8_t> m_p_ram;
	required_shared_ptr<uint8_t> m_p_smiram;
	required_shared_ptr<uint8_t> m_p_extram;
	required_device<cassette_image_device> m_cass;
	required_device<pwm_display_device> m_display;
	required_ioport_array<6> m_io_keyboard;
	output_finder<9> m_leds;
};

// flag led
void instruct_state::flag_w(int state)
{
	m_leds[8] = !state;
}

// user port
void instruct_state::port_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_leds[i] = !BIT(data, i);
}

// cassette port
void instruct_state::portf8_w(uint8_t data)
{
	if (BIT(data, 4))
		m_cass->output(BIT(data, 3) ? -1.0 : +1.0);
	else
		m_cass->output(0.0);

	m_cassin = BIT(data, 7);
}

// segment output
void instruct_state::portf9_w(uint8_t data)
{
	m_seg = data;
	m_display->matrix(m_digit, m_seg);
}

// digit & keyrow-scan select
void instruct_state::portfa_w(uint8_t data)
{
	m_digit = data;
	m_display->matrix(m_digit, m_seg);
}

// user switches
uint8_t instruct_state::port_r()
{
	return ioport("USW")->read();
}

// last address register A0-7 copied to 17E9 at boot
uint8_t instruct_state::portfc_r()
{
	return m_lar;
}

// last address register A8-14 copied to 17E8 at boot
uint8_t instruct_state::portfd_r()
{
	return (m_lar >> 8) & 0x7f;
}

// read keyboard
uint8_t instruct_state::portfe_r()
{
	u8 data = 15;

	for (uint8_t i = 0; i < 6; i++)
		if (BIT(m_digit, i))
			data &= m_io_keyboard[i]->read();

	return data;
}


// Read cassette and SENS key
int instruct_state::sense_r()
{
	if (m_cassin)
		return (m_cass->input() > 0.03) ? 1 : 0;
	else
		return BIT(ioport("HW")->read(), 0);
}

INTERRUPT_GEN_MEMBER( instruct_state::t2l_int )
{
	uint8_t hwkeys = ioport("HW")->read();

	// check RST key
	if (BIT(hwkeys, 3))
	{
		m_maincpu->set_state_int(S2650_PC, 0);
		return;
	}
	else
	// check MON key
	if (BIT(hwkeys, 2))
	{
		m_maincpu->set_state_int(S2650_PC, 0x1800);
		return;
	}
	else
	{
		uint8_t switches = ioport("SW")->read();

		// Check INT sw & key
		if (BIT(switches, 1))
			device.execute().set_input_line(0, BIT(hwkeys, 1) ? ASSERT_LINE : CLEAR_LINE);
		else
		// process ac input
		{
			m_irqstate ^= 1;
			device.execute().set_input_line(0, m_irqstate ? ASSERT_LINE : CLEAR_LINE);
		}
	}
}

void instruct_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0ffe).ram().share("mainram");
	map(0x0fff, 0x0fff).rw(FUNC(instruct_state::port_r), FUNC(instruct_state::port_w));
	map(0x1780, 0x17ff).ram().share("smiram");
	map(0x1800, 0x1fff).rom().region("roms", 0);
	map(0x2000, 0x7fff).ram().share("extram");
}

void instruct_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x07, 0x07).rw(FUNC(instruct_state::port_r), FUNC(instruct_state::port_w));
	map(0xf8, 0xf8).w(FUNC(instruct_state::portf8_w));
	map(0xf9, 0xf9).w(FUNC(instruct_state::portf9_w));
	map(0xfa, 0xfa).w(FUNC(instruct_state::portfa_w));
	map(0xfc, 0xfc).r(FUNC(instruct_state::portfc_r));
	map(0xfd, 0xfd).r(FUNC(instruct_state::portfd_r));
	map(0xfe, 0xfe).r(FUNC(instruct_state::portfe_r));
}

void instruct_state::data_map(address_map &map)
{
	map.unmap_value_high();
	map(S2650_DATA_PORT, S2650_DATA_PORT).rw(FUNC(instruct_state::port_r), FUNC(instruct_state::port_w));
}

/* Input ports */
static INPUT_PORTS_START( instruct )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("WCAS") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RCAS") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STEP") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RUN") PORT_CODE(KEYCODE_X) PORT_CHAR('X')

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BKPT") PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("REG") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MEM") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENT/NXT") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')

	PORT_START("HW")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SENS") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INT") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MON") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RST") PORT_CODE(KEYCODE_P) PORT_CHAR('P')

	PORT_START("SW")
	PORT_DIPNAME( 0x01, 0x00, "INT") // Interrupt jumps to 0007 or *0007
	PORT_DIPSETTING(    0x01, "Indirect")
	PORT_DIPSETTING(    0x00, "Direct")
	PORT_DIPNAME( 0x02, 0x00, "AC/INT") // Interrupt comes from INT key or from power supply
	PORT_DIPSETTING(    0x02, "INT")
	PORT_DIPSETTING(    0x00, "AC")

	PORT_START("USW")
	PORT_DIPNAME( 0x01, 0x00, "Switch A") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x02, 0x02, "Switch B") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x04, 0x04, "Switch C") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x08, 0x00, "Switch D") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x10, 0x00, "Switch E") PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x20, 0x20, "Switch F") PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x40, 0x40, "Switch G") PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x80, 0x00, "Switch H") PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
INPUT_PORTS_END


void instruct_state::machine_reset()
{
	m_irqstate = 0;
	m_cassin = 0;
	port_w(0); // turn round leds off
	m_maincpu->set_state_int(S2650_PC, 0x1800);
}

void instruct_state::machine_start()
{
	m_leds.resolve();

	save_item(NAME(m_lar));
	save_item(NAME(m_digit));
	save_item(NAME(m_seg));
	save_item(NAME(m_cassin));
	save_item(NAME(m_irqstate));
}

QUICKLOAD_LOAD_MEMBER(instruct_state::quickload_cb)
{
	int const quick_length = image.length();
	if (quick_length < 0x0100)
		return std::make_pair(image_error::INVALIDLENGTH, "File too short (must be at least 256 bytes)");
	else if (quick_length > 0x8000)
		return std::make_pair(image_error::INVALIDLENGTH, "File too long (must be no more than 32K)");

	std::vector<uint8_t> quick_data(quick_length);
	uint16_t read_ = image.fread(&quick_data[0], quick_length);
	if (read_ != quick_length)
		return std::make_pair(image_error::UNSPECIFIED, "Cannot read file");
	else if (quick_data[0] != 0xc5)
		return std::make_pair(image_error::INVALIDIMAGE, "Invalid header");

	uint16_t const exec_addr = quick_data[1] * 256 + quick_data[2];
	if (exec_addr >= quick_length)
	{
		return std::make_pair(
				image_error::INVALIDIMAGE,
				util::string_format("Exec address %04X beyond end of file %04X", exec_addr, quick_length));
	}

	// load to 0000-0FFE (standard ram + extra)
	read_ = std::min<uint16_t>(quick_length, 0xfff);
	m_p_ram[0] = 0x1f;  // add jump for RST key
	for (uint16_t i = 1; i < read_; i++)
		m_p_ram[i] = quick_data[i];

	// load to 1780-17BF (spare ram inside 2656)
	if (quick_length > 0x1780)
	{
		read_ = std::min<uint16_t>(quick_length, 0x17c0);
		for (uint16_t i = 0x1780; i < read_; i++)
			m_p_smiram[i-0x1780] = quick_data[i];
	}

	// put start address into PC so it can be debugged
	m_p_smiram[0x68] = m_p_ram[1];
	m_p_smiram[0x69] = m_p_ram[2];

	// load to 2000-7FFF (optional extra RAM)
	if (quick_length > 0x2000)
	{
		for (uint16_t i = 0x2000; i < quick_length; i++)
			m_p_extram[i-0x2000] = quick_data[i];
	}

	// display a message about the loaded quickload
	image.message(" Quickload: size=%04X : exec=%04X", quick_length, exec_addr);

	// Start the quickload - JP exec_addr
	m_maincpu->set_state_int(S2650_PC, 0);

	return std::make_pair(std::error_condition(), std::string());
}

void instruct_state::instruct(machine_config &config)
{
	/* basic machine hardware */
	S2650(config, m_maincpu, XTAL(3'579'545) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &instruct_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &instruct_state::io_map);
	m_maincpu->set_addrmap(AS_DATA, &instruct_state::data_map);
	m_maincpu->set_periodic_int(FUNC(instruct_state::t2l_int), attotime::from_hz(120));
	m_maincpu->sense_handler().set(FUNC(instruct_state::sense_r));
	m_maincpu->flag_handler().set(FUNC(instruct_state::flag_w));
	// Set vector from INDIRECT sw
	m_maincpu->intack_handler().set([this]() { return BIT(ioport("SW")->read(), 0) ? 0x87 : 0x07; });

	/* video hardware */
	config.set_default_layout(layout_instruct);
	PWM_DISPLAY(config, m_display).set_size(8, 8);
	m_display->set_segmask(0xff, 0xff);

	/* quickload */
	QUICKLOAD(config, "quickload", "pgm", attotime::from_seconds(1)).set_load_callback(FUNC(instruct_state::quickload_cb));

	SPEAKER(config, "mono").front_center();

	/* cassette */
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}

/* ROM definition */
ROM_START( instruct )
	ROM_REGION( 0x0800, "roms", 0 )
	ROM_LOAD( "instruct.rom", 0x0000, 0x0800, CRC(131715a6) SHA1(4930b87d09046113ab172ba3fb31f5e455068ec7) )

	ROM_REGION( 0x8020, "proms", 0 )
	ROM_LOAD( "82s123.33",    0x0000, 0x0020, CRC(b7aecef0) SHA1(b39fb35e8b6ab67b31f8f310fd5d56304bcd4123) )
	ROM_LOAD( "82s103.20",    0x0020, 0x8000, NO_DUMP )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY      FULLNAME                   FLAGS
COMP( 1978, instruct, 0,      0,      instruct, instruct, instruct_state, empty_init, "Signetics", "Signetics Instructor 50", MACHINE_SUPPORTS_SAVE )
