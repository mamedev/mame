// license:GPL-2.0+
// copyright-holders:Peter Trauner, Dan Boris, Dirk Best, Robbbert
/******************************************************************************
 PeT mess@utanet.at Nov 2000
Updated by Dan Boris, 2000-04-03
Rewrite in progress, Dirk Best, 2007-07-31
Updated by Robbbert 2019-04-14

Since 0.226, if you want to use the TTY, you must do these things:
- Use the KB/TTY dipswitch to choose TTY
- Use the TAB menu to choose "Keyboard Mode"
- Make sure that both keyboards are "Enabled"
- Quit to save the settings, then restart
- Press DELETE to start using the terminal
- For subsequent runs, just press DELETE to get started.

ToDo:
- Implement punchtape reader/writer
- Front panel Run/Step switch (switch S2)


******************************************************************************/

#include "emu.h"
#include "aim65.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "aim65.lh"


/** R6502 Clock.
 *
 * The R6502 on AIM65 operates at 1 MHz. The frequency reference is a 4 MHz
 * crystal controlled oscillator. Dual D-type flip-flop Z10 divides the 4 MHz
 * signal by four to drive the R6502 phase 0 (O0) input with a 1 MHz clock.
 */
static constexpr XTAL AIM65_CLOCK(4_MHz_XTAL / 4);


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void aim65_state::mem_map(address_map &map)
{
	map(0x1000, 0x3fff).noprw(); // User available expansions
	map(0x4000, 0x7fff).rom(); // 4 ROM sockets in 16K PROM/ROM module
	map(0x8000, 0x9fff).noprw(); // User available expansions
	map(0xa000, 0xa00f).mirror(0x3f0).m(m_via1, FUNC(via6522_device::map)); // user VIA
	map(0xa400, 0xa47f).m(m_riot, FUNC(mos6532_device::ram_map));
	map(0xa480, 0xa497).m(m_riot, FUNC(mos6532_device::io_map));
	map(0xa498, 0xa7ff).noprw(); // Not available
	map(0xa800, 0xa80f).mirror(0x3f0).m(m_via0, FUNC(via6522_device::map)); // system VIA
	map(0xac00, 0xac03).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xac04, 0xac43).ram(); // PIA RAM
	map(0xac44, 0xafff).noprw(); // Not available
	map(0xb000, 0xffff).rom(); // 5 ROM sockets
}


/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( aim65 )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space")       PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(32)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".  >")        PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M")           PORT_CODE(KEYCODE_M)          PORT_CHAR('m')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B")           PORT_CODE(KEYCODE_B)          PORT_CHAR('b')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C")           PORT_CODE(KEYCODE_C)          PORT_CHAR('c')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z")           PORT_CODE(KEYCODE_Z)          PORT_CHAR('z')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LF  @")       PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(10)  PORT_CHAR('@')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L")           PORT_CODE(KEYCODE_L)          PORT_CHAR('l')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J")           PORT_CODE(KEYCODE_J)          PORT_CHAR('j')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G")           PORT_CODE(KEYCODE_G)          PORT_CHAR('g')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D")           PORT_CODE(KEYCODE_D)          PORT_CHAR('d')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")           PORT_CODE(KEYCODE_A)          PORT_CHAR('a')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Print")       PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P")           PORT_CODE(KEYCODE_P)          PORT_CHAR('p')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I")           PORT_CODE(KEYCODE_I)          PORT_CHAR('i')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y")           PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R")           PORT_CODE(KEYCODE_R)          PORT_CHAR('r')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W")           PORT_CODE(KEYCODE_W)          PORT_CHAR('w')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc")         PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("KEY.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return")      PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-  =")        PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O")           PORT_CODE(KEYCODE_O)          PORT_CHAR('o')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U")           PORT_CODE(KEYCODE_U)          PORT_CHAR('u')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T")           PORT_CODE(KEYCODE_T)          PORT_CHAR('t')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E")           PORT_CODE(KEYCODE_E)          PORT_CHAR('e')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q")           PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')

	PORT_START("KEY.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl")        PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":  *")        PORT_CODE(KEYCODE_MINUS)      PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9  )")        PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7  '")        PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5  %")        PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3  #")        PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1  !")        PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("KEY.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift")  PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")           PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8  (")        PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6  &")        PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4  $")        PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2  \"")       PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3")          PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("KEY.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del")         PORT_CODE(KEYCODE_TILDE)      PORT_CHAR(8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";  +")        PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K")           PORT_CODE(KEYCODE_K)          PORT_CHAR('k')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H")           PORT_CODE(KEYCODE_H)          PORT_CHAR('h')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F")           PORT_CODE(KEYCODE_F)          PORT_CHAR('f')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S")           PORT_CODE(KEYCODE_S)          PORT_CHAR('s')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2")          PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START("KEY.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/  ?")        PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",  <")        PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N")           PORT_CODE(KEYCODE_N)          PORT_CHAR('n')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V")           PORT_CODE(KEYCODE_V)          PORT_CHAR('v')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X")           PORT_CODE(KEYCODE_X)          PORT_CHAR('x')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1")          PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("switches")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RST") PORT_CODE(KEYCODE_LALT) PORT_CHANGED_MEMBER(DEVICE_SELF, aim65_state, reset_button, 0)
	PORT_DIPNAME(0x08, 0x08, "KB/TTY") PORT_DIPLOCATION("S3:1")
	PORT_DIPSETTING(0x00, "TTY")
	PORT_DIPSETTING(0x08, "KB")
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(aim65_state::reset_button)
{
	// Reset all devices
	// If you're using TTY, you must press DEL after the reset.
	if (newval)
	{
		m_via0->reset();
		m_via1->reset();
		m_pia->reset();
		m_riot->reset();
	}
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}

void aim65_state::aim65_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x20, 0x02, 0x05));
	palette.set_pen_color(1, rgb_t(0xc0, 0x00, 0x00));
}


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

std::pair<std::error_condition, std::string> aim65_state::load_cart(
		device_image_interface &image,
		generic_slot_device *slot,
		const char *slot_tag)
{
	uint32_t size = slot->common_get_size(slot_tag);

	if (size > 0x1000)
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported ROM size (must be no more than 4K)");

	if (image.loaded_through_softlist() && image.get_software_region(slot_tag) == nullptr)
	{
		// FIXME: error message seems to be outdated - actual error seems to be incorrect region name in software item
		return std::make_pair(
				image_error::UNSUPPORTED,
				util::string_format("Unsupported file name extension (socket '%1$s' only accepts files with '.%1$s' extension", slot_tag));
	}

	slot->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	slot->common_load_rom(slot->get_rom_base(), size, slot_tag);

	return std::make_pair(std::error_condition(), std::string());
}

// TTY terminal settings. To use, turn KB/TTY switch to TTY, reset, press DEL. All input to be in UPPERCASE.
static DEVICE_INPUT_DEFAULTS_START( serial_term )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_ODD )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void aim65_state::aim65(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, AIM65_CLOCK); // 1 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &aim65_state::mem_map);

	config.set_default_layout(layout_aim65);

	// alpha-numeric display
	DL1416T(config, m_ds[0], u32(0));
	m_ds[0]->update().set(FUNC(aim65_state::update_ds<1>));
	DL1416T(config, m_ds[1], u32(0));
	m_ds[1]->update().set(FUNC(aim65_state::update_ds<2>));
	DL1416T(config, m_ds[2], u32(0));
	m_ds[2]->update().set(FUNC(aim65_state::update_ds<3>));
	DL1416T(config, m_ds[3], u32(0));
	m_ds[3]->update().set(FUNC(aim65_state::update_ds<4>));
	DL1416T(config, m_ds[4], u32(0));
	m_ds[4]->update().set(FUNC(aim65_state::update_ds<5>));

	// pseudo-"screen" for the thermal printer. Index 0.
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_screen_update(FUNC(aim65_state::screen_update));
	screen.set_size(160, 200);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, m_palette, FUNC(aim65_state::aim65_palette), 2);

	// Sound - wave sound only
	SPEAKER(config, "mono").front_center();

	// other devices
	MOS6532(config, m_riot, AIM65_CLOCK);
	m_riot->pa_wr_callback().set([this] (u8 data) { m_riot_port_a = data; });
	m_riot->pb_rd_callback().set([this] () { return aim65_state::z33_pb_r(); });
	m_riot->irq_wr_callback().set_inputline(m_maincpu, M6502_IRQ_LINE);

	MOS6522(config, m_via0, AIM65_CLOCK);
	m_via0->readpb_handler().set([this] () { return aim65_state::z32_pb_r(); });
	m_via0->writepa_handler().set([this] (u8 data) { aim65_state::z32_pa_w(data); });
	m_via0->writepb_handler().set([this] (u8 data) { aim65_state::z32_pb_w(data); });
	// in CA1 printer ready?
	// out CA2 cass control (H=in)
	m_via0->ca2_handler().set([this] (bool state) { m_ca2 = state; });
	// out CB1 printer start
	//m_via0->cb1_handler().set(FUNC(aim65_state::z32_cb1_w));
	// out CB2 turn printer on
	m_via0->cb2_handler().set([this] (bool state) { aim65_state::z32_cb2_w(state); });
	m_via0->irq_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	MOS6522(config, m_via1, AIM65_CLOCK);
	m_via1->irq_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	PIA6821(config, m_pia);
	m_pia->writepa_handler().set([this] (u8 data) { aim65_state::u1_pa_w(data); });
	m_pia->writepb_handler().set([this] (u8 data) { aim65_state::u1_pb_w(data); });

	CASSETTE(config, m_cassette1);
	m_cassette1->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette1->add_route(ALL_OUTPUTS, "mono", 0.1);
	CASSETTE(config, m_cassette2);
	m_cassette2->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette2->add_route(ALL_OUTPUTS, "mono", 0.1);

	// Screen for TTY interface. Index 1.
	RS232_PORT(config, m_rs232, default_rs232_devices, "terminal");
	//m_rs232->rxd_handler().set(m_via0, FUNC(via6522_device::write_pb6));  // function disabled in 6522via.cpp
	m_rs232->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(serial_term));

	GENERIC_SOCKET(config, "z26", generic_plain_slot, "aim65_z26_cart", "z26").set_device_load(FUNC(aim65_state::z26_load));
	GENERIC_SOCKET(config, "z25", generic_plain_slot, "aim65_z25_cart", "z25").set_device_load(FUNC(aim65_state::z25_load));
	GENERIC_SOCKET(config, "z24", generic_plain_slot, "aim65_z24_cart", "z24").set_device_load(FUNC(aim65_state::z24_load));

	// PROM/ROM module sockets
	GENERIC_SOCKET(config, "z12", generic_plain_slot, "rm65_z12_cart", "z12").set_device_load(FUNC(aim65_state::z12_load));
	GENERIC_SOCKET(config, "z13", generic_plain_slot, "rm65_z13_cart", "z13").set_device_load(FUNC(aim65_state::z13_load));
	GENERIC_SOCKET(config, "z14", generic_plain_slot, "rm65_z14_cart", "z14").set_device_load(FUNC(aim65_state::z14_load));
	GENERIC_SOCKET(config, "z15", generic_plain_slot, "rm65_z15_cart", "z15").set_device_load(FUNC(aim65_state::z15_load));

	// internal ram
	RAM(config, RAM_TAG).set_default_size("4K").set_extra_options("1K,2K,3K");

	// Software lists
	SOFTWARE_LIST(config, "cart_list").set_original("aim65_cart");
}


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( aim65 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "aim65",   "Rockwell AIM-65")
	ROMX_LOAD("aim65mon.z23", 0xe000, 0x1000, CRC(90e44afe) SHA1(78e38601edf6bfc787b58750555a636b0cf74c5c), ROM_BIOS(0))
	ROMX_LOAD("aim65mon.z22", 0xf000, 0x1000, CRC(d01914b0) SHA1(e5b5ddd4cd43cce073a718ee4ba5221f2bc84eaf), ROM_BIOS(0))

	/* DRAC/DRAC-1 is an industrial control computer from the Spanish company Comelta (more info: https://www.oldcomputers.es/drac-1/).
	   It's based on a standard Rockwell AIM 65 PCB, but can be expanded with several cards and accessories made by Comelta, from CPU and
	   memory modules to control or interface cards (more info and manuals with schematics: https://www.oldcomputers.es/drac-1-placas-cr/).
	*/
	ROM_SYSTEM_BIOS(1, "drac1",   "Comelta DRAC-1")
	ROMX_LOAD("crosaim_v1.3_b_mone_2b_moni_01_e000.z23", 0xe000, 0x1000, CRC(ae83ba08) SHA1(4ee4157fe6cafda6c763547183be18859bdabc36), ROM_BIOS(1))
	ROMX_LOAD("crosaim_v1.3_b_monf_2b_f000.z22",         0xf000, 0x1000, CRC(047c2ca8) SHA1(1877be29f7b725ee4fec7f21aa679d857391514b), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "dynatem", "Dynatem AIM-65")
	ROMX_LOAD("dynaim65.z23", 0xe000, 0x1000, CRC(90e44afe) SHA1(78e38601edf6bfc787b58750555a636b0cf74c5c), ROM_BIOS(2))
	ROMX_LOAD("dynaim65.z22", 0xf000, 0x1000, CRC(83e1c6e7) SHA1(444134043edd83385bd70434cb100269901c4417), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "spc100",  "Siemens PC100")
	ROMX_LOAD("pc100.z23",    0xe000, 0x1000, CRC(90e44afe) SHA1(78e38601edf6bfc787b58750555a636b0cf74c5c), ROM_BIOS(3))
	ROMX_LOAD("pc100.z22",    0xf000, 0x1000, CRC(aa07742a) SHA1(3b9bee24a00cf23b7b50cee97ccc12e3fa9da1ea), ROM_BIOS(3))
ROM_END


/***************************************************************************
    GAME DRIVERS
***************************************************************************/

//   YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY     FULLNAME  FLAGS
COMP(1977, aim65, 0,      0,      aim65,   aim65, aim65_state, empty_init, "Rockwell", "AIM 65", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW)
