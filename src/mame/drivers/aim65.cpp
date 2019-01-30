// license:GPL-2.0+
// copyright-holders:Peter Trauner, Dan Boris, Dirk Best, Robbbert
/******************************************************************************
 PeT mess@utanet.at Nov 2000pia6821_device
Updated by Dan Boris, 3/4/2007
Rewrite in progress, Dirk Best, 2007-07-31

ToDo:
    - Printer. Tried to implement this but it was not working, currently disabled.
    - Dual tape interface (done, but see bugs below)
    - Implement punchtape reader/writer and TTY keyboard
    - Front panel Reset switch (switch S1)
    - Front panel Run/Step switch (switch S2)

Bugs
    - Cassette should output data on PB7, but the bit stays High.
    - At the end of saving, both motors sometimes get turned on!
    - CA2 should switch the cassette circuits between input and output.
      It goes High on Read (correct) but doesn't go Low for Write.
    - Read of CA1 is to check the printer, but it never happens.
    - Write to CB1 should occur to activate printer's Start line, but it
      also never happens.
    - The common factor is the 6522, maybe it has problems..

******************************************************************************/

#include "emu.h"
#include "includes/aim65.h"

#include "softlist.h"
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

/* Note: RAM is mapped dynamically in machine/aim65.c */
void aim65_state::aim65_mem(address_map &map)
{
	map(0x1000, 0x3fff).noprw(); /* User available expansions */
	map(0x4000, 0x7fff).rom(); /* 4 ROM sockets in 16K PROM/ROM module */
	map(0x8000, 0x9fff).noprw(); /* User available expansions */
	map(0xa000, 0xa00f).mirror(0x3f0).rw("via6522_1", FUNC(via6522_device::read), FUNC(via6522_device::write)); // user via
	map(0xa400, 0xa47f).m("riot", FUNC(mos6532_new_device::ram_map));
	map(0xa480, 0xa497).m("riot", FUNC(mos6532_new_device::io_map));
	map(0xa498, 0xa7ff).noprw(); /* Not available */
	map(0xa800, 0xa80f).mirror(0x3f0).rw("via6522_0", FUNC(via6522_device::read), FUNC(via6522_device::write)); // system via
	map(0xac00, 0xac03).rw("pia6821", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xac04, 0xac43).ram(); /* PIA RAM */
	map(0xac44, 0xafff).noprw(); /* Not available */
	map(0xb000, 0xffff).rom(); /* 5 ROM sockets */
}


/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( aim65 )
	PORT_START("keyboard_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space")       PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(32)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".  >")        PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M")           PORT_CODE(KEYCODE_M)          PORT_CHAR('m')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B")           PORT_CODE(KEYCODE_B)          PORT_CHAR('b')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C")           PORT_CODE(KEYCODE_C)          PORT_CHAR('c')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z")           PORT_CODE(KEYCODE_Z)          PORT_CHAR('z')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("keyboard_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LF  @")       PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(10)  PORT_CHAR('@')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L")           PORT_CODE(KEYCODE_L)          PORT_CHAR('l')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J")           PORT_CODE(KEYCODE_J)          PORT_CHAR('j')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G")           PORT_CODE(KEYCODE_G)          PORT_CHAR('g')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D")           PORT_CODE(KEYCODE_D)          PORT_CHAR('d')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")           PORT_CODE(KEYCODE_A)          PORT_CHAR('a')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("keyboard_2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Print")       PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P")           PORT_CODE(KEYCODE_P)          PORT_CHAR('p')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I")           PORT_CODE(KEYCODE_I)          PORT_CHAR('i')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y")           PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R")           PORT_CODE(KEYCODE_R)          PORT_CHAR('r')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W")           PORT_CODE(KEYCODE_W)          PORT_CHAR('w')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc")         PORT_CODE(KEYCODE_TAB)        PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("keyboard_3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return")      PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-  =")        PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O")           PORT_CODE(KEYCODE_O)          PORT_CHAR('o')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U")           PORT_CODE(KEYCODE_U)          PORT_CHAR('u')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T")           PORT_CODE(KEYCODE_T)          PORT_CHAR('t')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E")           PORT_CODE(KEYCODE_E)          PORT_CHAR('e')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q")           PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')

	PORT_START("keyboard_4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl")        PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":  *")        PORT_CODE(KEYCODE_MINUS)      PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9  )")        PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7  '")        PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5  %")        PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3  #")        PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1  !")        PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("keyboard_5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift")  PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")           PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8  (")        PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6  &")        PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4  $")        PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2  \"")       PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3")          PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("keyboard_6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del")         PORT_CODE(KEYCODE_TILDE)      PORT_CHAR(8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";  +")        PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K")           PORT_CODE(KEYCODE_K)          PORT_CHAR('k')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H")           PORT_CODE(KEYCODE_H)          PORT_CHAR('h')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F")           PORT_CODE(KEYCODE_F)          PORT_CHAR('f')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S")           PORT_CODE(KEYCODE_S)          PORT_CHAR('s')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2")          PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START("keyboard_7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/  ?")        PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",  <")        PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N")           PORT_CODE(KEYCODE_N)          PORT_CHAR('n')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V")           PORT_CODE(KEYCODE_V)          PORT_CHAR('v')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X")           PORT_CODE(KEYCODE_X)          PORT_CHAR('x')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1")          PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("switches")
	PORT_DIPNAME(0x08, 0x08, "KB/TTY") PORT_DIPLOCATION("S3:1")
	PORT_DIPSETTING(0x00, "TTY")
	PORT_DIPSETTING(0x08, "KB")
INPUT_PORTS_END


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

image_init_result aim65_state::load_cart(device_image_interface &image, generic_slot_device *slot, const char *slot_tag)
{
	uint32_t size = slot->common_get_size(slot_tag);

	if (size > 0x1000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported ROM size");
		return image_init_result::FAIL;
	}

	if (image.loaded_through_softlist() && image.get_software_region(slot_tag) == nullptr)
	{
		std::string errmsg = string_format(
				"Attempted to load file with wrong extension\nSocket '%s' only accepts files with '.%s' extension",
				slot_tag, slot_tag);
		image.seterror(IMAGE_ERROR_UNSPECIFIED, errmsg.c_str());
		return image_init_result::FAIL;
	}

	slot->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	slot->common_load_rom(slot->get_rom_base(), size, slot_tag);

	return image_init_result::PASS;
}


MACHINE_CONFIG_START(aim65_state::aim65)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", M6502, AIM65_CLOCK) /* 1 MHz */
	MCFG_DEVICE_PROGRAM_MAP(aim65_mem)

	config.set_default_layout(layout_aim65);

	/* alpha-numeric display */
	DL1416T(config, m_ds[0], u32(0));
	m_ds[0]->update().set(FUNC(aim65_state::aim65_update_ds<1>));
	DL1416T(config, m_ds[1], u32(0));
	m_ds[1]->update().set(FUNC(aim65_state::aim65_update_ds<2>));
	DL1416T(config, m_ds[2], u32(0));
	m_ds[2]->update().set(FUNC(aim65_state::aim65_update_ds<3>));
	DL1416T(config, m_ds[3], u32(0));
	m_ds[3]->update().set(FUNC(aim65_state::aim65_update_ds<4>));
	DL1416T(config, m_ds[4], u32(0));
	m_ds[4]->update().set(FUNC(aim65_state::aim65_update_ds<5>));

	/* Sound - wave sound only */
	SPEAKER(config, "mono").front_center();
	WAVE(config, "wave", m_cassette1).add_route(ALL_OUTPUTS, "mono", 0.25);

	/* other devices */
	mos6532_new_device &riot(MOS6532_NEW(config, "riot", AIM65_CLOCK));
	riot.pa_wr_callback().set(FUNC(aim65_state::aim65_riot_a_w));
	riot.pb_rd_callback().set(FUNC(aim65_state::aim65_riot_b_r));
	riot.irq_wr_callback().set_inputline(m_maincpu, M6502_IRQ_LINE);

	via6522_device &via0(VIA6522(config, "via6522_0", AIM65_CLOCK));
	via0.readpb_handler().set(FUNC(aim65_state::aim65_pb_r));
	// in CA1 printer ready?
	via0.writepb_handler().set(FUNC(aim65_state::aim65_pb_w));
	// out CB1 printer start
	// out CA2 cass control (H=in)
	// out CB2 turn printer on
	via0.irq_handler().set_inputline("maincpu", M6502_IRQ_LINE);

	via6522_device &via1(VIA6522(config, "via6522_1", AIM65_CLOCK));
	via1.irq_handler().set_inputline("maincpu", M6502_IRQ_LINE);

	pia6821_device &pia(PIA6821(config, "pia6821", 0));
	pia.writepa_handler().set(FUNC(aim65_state::aim65_pia_a_w));
	pia.writepb_handler().set(FUNC(aim65_state::aim65_pia_b_w));

	// Deck 1 can play and record
	CASSETTE(config, m_cassette1);
	m_cassette1->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);

	// Deck 2 can only record
	CASSETTE(config, m_cassette2);
	m_cassette2->set_default_state(CASSETTE_RECORD | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED);

	MCFG_GENERIC_SOCKET_ADD("z26", generic_plain_slot, "aim65_z26_cart")
	MCFG_GENERIC_EXTENSIONS("z26")
	MCFG_GENERIC_LOAD(aim65_state, z26_load)

	MCFG_GENERIC_SOCKET_ADD("z25", generic_plain_slot, "aim65_z25_cart")
	MCFG_GENERIC_EXTENSIONS("z25")
	MCFG_GENERIC_LOAD(aim65_state, z25_load)

	MCFG_GENERIC_SOCKET_ADD("z24", generic_plain_slot, "aim65_z24_cart")
	MCFG_GENERIC_EXTENSIONS("z24")
	MCFG_GENERIC_LOAD(aim65_state, z24_load)

	/* PROM/ROM module sockets */
	MCFG_GENERIC_SOCKET_ADD("z12", generic_plain_slot, "rm65_z12_cart")
	MCFG_GENERIC_EXTENSIONS("z12")
	MCFG_GENERIC_LOAD(aim65_state, z12_load)

	MCFG_GENERIC_SOCKET_ADD("z13", generic_plain_slot, "rm65_z13_cart")
	MCFG_GENERIC_EXTENSIONS("z13")
	MCFG_GENERIC_LOAD(aim65_state, z13_load)

	MCFG_GENERIC_SOCKET_ADD("z14", generic_plain_slot, "rm65_z14_cart")
	MCFG_GENERIC_EXTENSIONS("z14")
	MCFG_GENERIC_LOAD(aim65_state, z14_load)

	MCFG_GENERIC_SOCKET_ADD("z15", generic_plain_slot, "rm65_z15_cart")
	MCFG_GENERIC_EXTENSIONS("z15")
	MCFG_GENERIC_LOAD(aim65_state, z15_load)

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("4K").set_extra_options("1K,2K,3K");

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("aim65_cart");
MACHINE_CONFIG_END


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( aim65 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "aim65",  "Rockwell AIM-65")
	ROMX_LOAD("aim65mon.z23", 0xe000, 0x1000, CRC(90e44afe) SHA1(78e38601edf6bfc787b58750555a636b0cf74c5c), ROM_BIOS(0))
	ROMX_LOAD("aim65mon.z22", 0xf000, 0x1000, CRC(d01914b0) SHA1(e5b5ddd4cd43cce073a718ee4ba5221f2bc84eaf), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "dynatem",  "Dynatem AIM-65")
	ROMX_LOAD("dynaim65.z23", 0xe000, 0x1000, CRC(90e44afe) SHA1(78e38601edf6bfc787b58750555a636b0cf74c5c), ROM_BIOS(1))
	ROMX_LOAD("dynaim65.z22", 0xf000, 0x1000, CRC(83e1c6e7) SHA1(444134043edd83385bd70434cb100269901c4417), ROM_BIOS(1))
ROM_END

/* Currently dumped and available software:
 *
 * Name             Loc  CRC32     SHA1
 * ------------------------------------------------------------------------
 * Assembler        Z24  0878b399  483e92b57d64be51643a9f6490521a8572aa2f68
 * Basic V1.1       Z25  d7b42d2a  4bbdb28d332429825adea0266ed9192786d9e392
 * Basic V1.1       Z26  36a61f39  f5ce0126cb594a565e730973fd140d03c298cefa
 * Forth V1.3       Z25  0671d019  dd2a1613e435c833634100cf4a22c6cff70c7a26
 * Forth V1.3       Z26  a80ad472  42a2e8c86829a2fe48090e6665ff9fe25b12b070
 * Mathpack         Z24  4889af55  5e9541ddfc06e3802d09b30d1bd89c5da914c76e
 * Monitor          Z22  d01914b0  e5b5ddd4cd43cce073a718ee4ba5221f2bc84eaf
 * Monitor          Z23  90e44afe  78e38601edf6bfc787b58750555a636b0cf74c5c
 * Monitor Dynatem  Z22  83e1c6e7  444134043edd83385bd70434cb100269901c4417
 * PL/65 V1.0       Z25  76dcf864  e937c54ed109401f796640cd45b27dfefb76667e
 * PL/65 V1.0       Z26  2ac71abd  6df5e3125bebefac80d51d9337555f54bdf0d8ea
 *
 */


/***************************************************************************
    GAME DRIVERS
***************************************************************************/

//   YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY     FULLNAME  FLAGS
COMP(1977, aim65, 0,      0,      aim65,   aim65, aim65_state, empty_init, "Rockwell", "AIM 65", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW)
