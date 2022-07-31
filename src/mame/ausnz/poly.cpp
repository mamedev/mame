// license:BSD-3-Clause
// copyright-holders:Robbbert,Nigel Barnes
// thanks-to:Andrew Trotman
/***************************************************************************

    Poly 1/2 (New Zealand)

    10/07/2011 Skeleton driver.

    http://www.cs.otago.ac.nz/homepages/andrew/poly/Poly.htm

    Andrew has supplied the roms for -bios 1

    It uses a 6809 for all main functions. There is a Z80 for CP/M, but all
    of the roms are 6809 code.

    The keyboard controller is one of those custom XR devices.
    Will use the terminal keyboard instead.

    With bios 1, after entering your userid and password, you get a black
    screen. This is normal, because it joins to a network which isn't there.

    TODO:
    - Connect up the device ports & lines
    - Find out about graphics mode and how it is selected
    - Fix Keyboard with KR2376-12 encoder
    - Poly 2 and Poly 1 (early) probably require a different keyboard matrix
    - Find out how to make 2nd teletext screen to display
    - Improve MC6854 emulation for network

****************************************************************************/

#include "emu.h"
#include "poly.h"
#include "formats/flex_dsk.h"
#include "softlist_dev.h"


void poly_state::poly_bank(address_map &map)
{
	map.unmap_value_high();
	/* System mode */
	map(0x00000, 0x0ffff).rw(FUNC(poly_state::logical_mem_r), FUNC(poly_state::logical_mem_w)); // Logical Memory
	map(0x0e000, 0x0e003).rw(m_pia[0], FUNC(pia6821_device::read), FUNC(pia6821_device::write));      // Video control PIA 6821
	map(0x0e004, 0x0e005).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));      // Optional RS232 Interface
	map(0x0e006, 0x0e006).w(FUNC(poly_state::baud_rate_w));                                     // Baud rate controller
	map(0x0e00c, 0x0e00f).rw(m_pia[1], FUNC(pia6821_device::read), FUNC(pia6821_device::write));      // Keyboard PIA 6821
	map(0x0e020, 0x0e027).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));         // Timer 6840
	map(0x0e030, 0x0e036).rw(FUNC(poly_state::network_r), FUNC(poly_state::network_w));         // Data Link Controller 6854
	map(0x0e040, 0x0e040).w(FUNC(poly_state::set_protect_w));                                   // Set protect flip-flop after 1 E-cycle
	map(0x0e050, 0x0e05f).ram().share("dat");                                                         // Dynamic Address Translator
	map(0x0e060, 0x0e060).rw(FUNC(poly_state::select_map_r), FUNC(poly_state::select_map1_w));  // Select Map 1
	map(0x0e070, 0x0e070).rw(FUNC(poly_state::select_map_r), FUNC(poly_state::select_map2_w));  // Select Map 2
	map(0x0e800, 0x0efff).ram().share("videoram");                                                    // Teletext screens and System data
	map(0x0f000, 0x0ffff).rom().region("system", 0);                                                  // System Program ROM
	/* User mode */
	map(0x10000, 0x1ffff).rw(FUNC(poly_state::logical_mem_r), FUNC(poly_state::logical_mem_w)); // Logical Memory
	map(0x1fff0, 0x1ffff).r(FUNC(poly_state::vector_r));                                        // Vector fetch (interrupt and reset)
}

void polydev_state::poly_bank(address_map &map)
{
	poly_state::poly_bank(map);
	map(0x0e014, 0x0e014).rw(FUNC(polydev_state::drive_register_r), FUNC(polydev_state::drive_register_w)); // Drive register
	map(0x0e018, 0x0e01b).rw(FUNC(polydev_state::fdc_inv_r), FUNC(polydev_state::fdc_inv_w));               // Floppy controller
}

void poly_state::poly_mem(address_map &map)
{
	map(0x0000, 0xffff).m(m_bankdev, FUNC(address_map_bank_device::amap8));
}


static INPUT_PORTS_START( poly1 )
	PORT_START("MODIFIERS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl")       PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)             PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift")      PORT_CODE(KEYCODE_LSHIFT)   PORT_CODE(KEYCODE_RSHIFT)               PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps Lock")  PORT_CODE(KEYCODE_CAPSLOCK)                             PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("X0")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)      PORT_NAME("Keypad 1 Red")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)      PORT_NAME("Keypad 0")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)       PORT_NAME("Help Calc")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)    PORT_NAME("Keypad .")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X1")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)      PORT_NAME("Keypad 3 Yellow")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)      PORT_NAME("Keypad 4 Blue")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)      PORT_NAME("Keypad 2 Green")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)     PORT_NAME("Ins Char Line")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)       PORT_NAME("Repeat Next")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X2")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)      PORT_NAME("Keypad 6 Cyan")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)      PORT_NAME("Keypad 7 White")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)      PORT_NAME("Keypad 5 Magenta")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_NAME("Del Char Line")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)       PORT_NAME("Back Exit")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X3")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)      PORT_NAME("Keypad 9")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)      PORT_NAME("Keypad 8 Flash")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('@')   PORT_CHAR(0xa3)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)        PORT_NAME("Pause")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_NAME(UTF8_UP)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_NAME(UTF8_RIGHT)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_NAME(UTF8_DOWN)
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_NAME(UTF8_LEFT)

	PORT_START("X4")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z')   PORT_CHAR('Z')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x')   PORT_CHAR('X')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c')   PORT_CHAR('C')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v')   PORT_CHAR('V')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b')   PORT_CHAR('B')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n')   PORT_CHAR('N')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m')   PORT_CHAR('M')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')   PORT_CHAR('<')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')   PORT_CHAR('>')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')   PORT_CHAR('?')
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X5")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a')   PORT_CHAR('A')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s')   PORT_CHAR('S')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d')   PORT_CHAR('D')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f')   PORT_CHAR('F')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g')   PORT_CHAR('G')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h')   PORT_CHAR('H')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j')   PORT_CHAR('J')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k')   PORT_CHAR('K')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l')   PORT_CHAR('L')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')   PORT_CHAR('+')
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(':')   PORT_CHAR('*')

	PORT_START("X6")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')   PORT_CHAR('Q')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w')   PORT_CHAR('W')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e')   PORT_CHAR('E')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r')   PORT_CHAR('R')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t')   PORT_CHAR('T')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')   PORT_CHAR('Y')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u')   PORT_CHAR('U')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i')   PORT_CHAR('I')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o')   PORT_CHAR('O')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p')   PORT_CHAR('P')
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_NAME("EXP \xE2\x80\x96")

	PORT_START("X7")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1')   PORT_CHAR('!')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2')   PORT_CHAR('"')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3')   PORT_CHAR('#')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4')   PORT_CHAR('$')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5')   PORT_CHAR('%')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6')   PORT_CHAR('&')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')   PORT_CHAR('\'')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')   PORT_CHAR('(')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')   PORT_CHAR(')')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')   PORT_CHAR('=')
INPUT_PORTS_END

static INPUT_PORTS_START( poly1e )
	PORT_INCLUDE(poly1)
	// TODO: unknown matrix, contains extra user definable keys
INPUT_PORTS_END

static INPUT_PORTS_START( poly2 )
	PORT_INCLUDE(poly1)
	// TODO: unknown matrix, seems to differ from poly1
INPUT_PORTS_END


void poly_state::machine_start()
{
	m_dat_bank = 0;
	m_protect_timer = timer_alloc(FUNC(poly_state::set_protect), this);
}


void poly_state::machine_reset()
{
	m_protect_timer->adjust(attotime::never);

	//m_kr2376->set_input_pin(kr2376_device::KR2376_DSII, 0);
	//m_kr2376->set_input_pin(kr2376_device::KR2376_PII, 0);

	/* system mode is entered on Reset */
	m_bankdev->set_bank(0);
}


void poly_state::init_poly()
{
	uint8_t bitswapped[0x4000];
	uint8_t *rom;

	/* basic rom region */
	rom = m_user->base() + 0xc000;

	/* decrypt rom data lines */
	for (int i = 0x0000; i<0x4000; i++)
		bitswapped[i] = bitswap<8>(rom[i], 3, 4, 2, 5, 1, 6, 0, 7);

	/* decrypt rom address lines */
	for (int i = 0x0000; i<0x4000; i++)
		rom[i] = bitswapped[bitswap<16>(i, 15, 14, 13, 12, 10, 8, 4, 2, 0, 1, 3, 5, 6, 7, 9, 11)];

	/* system rom region */
	rom = m_system->base();

	/* decrypt rom data lines */
	for (int i = 0x0000; i<0x1000; i++)
		bitswapped[i] = bitswap<8>(rom[i], 3, 4, 2, 5, 1, 6, 0, 7);

	/* decrypt rom address lines */
	for (int i = 0x0000; i<0x1000; i++)
		rom[i] = bitswapped[bitswap<16>(i, 15, 14, 13, 12, 10, 8, 4, 2, 0, 1, 3, 5, 6, 7, 9, 11)];
}


void polydev_state::floppy_formats(format_registration &fr)
{
	fr.add_fm_containers();
	fr.add(FLOPPY_FLEX_FORMAT);
}

static void poly_floppies(device_slot_interface &device)
{
	device.option_add("8dssd", FLOPPY_8_DSSD);
	device.option_add("525sd", FLOPPY_525_SD);
}


void poly_state::poly(machine_config &config)
{
	/* basic machine hardware */
	MC6809(config, m_maincpu, 12.0576_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &poly_state::poly_mem);

	ADDRESS_MAP_BANK(config, "bankdev").set_map(&poly_state::poly_bank).set_options(ENDIANNESS_LITTLE, 8, 17, 0x10000);

	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(40 * 12, 24 * 20);
	screen.set_visarea(0, 40 * 12 - 1, 0, 24 * 20 - 1);
	screen.set_screen_update(FUNC(poly_state::screen_update));

	SAA5050(config, m_trom[0], 12.0576_MHz_XTAL / 2);
	m_trom[0]->d_cb().set(FUNC(poly_state::videoram_1_r));
	m_trom[0]->set_screen_size(40, 24, 40);

	SAA5050(config, m_trom[1], 12.0576_MHz_XTAL / 2);
	m_trom[1]->d_cb().set(FUNC(poly_state::videoram_2_r));
	m_trom[1]->set_screen_size(40, 24, 40);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* internal ram */
	RAM(config, m_ram).set_default_size("128K").set_extra_options("64K");

	/* network */
	MC6854(config, m_adlc);
	//m_adlc->out_txd_cb().set(NETWORK_TAG, FUNC(poly_network_device::data_w));
	//m_adlc->out_irq_cb().set("irqs", FUNC(input_merger_device::in_w<0>));

	//MCFG_POLY_NETWORK_ADD()
	//MCFG_POLY_NETWORK_CLK_CB(WRITELINE(*this, poly_state, network_clk_w))
	//MCFG_POLY_NETWORK_DATA_CB(WRITELINE("mc6854", mc6854_device, set_rx))
	//MCFG_POLY_NETWORK_SLOT_ADD("netup", poly_network_devices, "proteus")
	//MCFG_POLY_NETWORK_SLOT_ADD("netdown", poly_network_devices, nullptr)

	/* timer */
	PTM6840(config, m_ptm, 12.0576_MHz_XTAL / 3);
	m_ptm->set_external_clocks(0, 0, 0);
	m_ptm->o2_callback().set(FUNC(poly_state::ptm_o2_callback));
	m_ptm->o3_callback().set(FUNC(poly_state::ptm_o3_callback));
	//m_ptm->irq_callback().set("irqs", FUNC(input_merger_device::in_w<1>));

	/* keyboard encoder */
	//KR2376_12(config, m_kr2376, 50000);
	//m_kr2376->x<0>().set_ioport("X0");
	//m_kr2376->x<1>().set_ioport("X1");
	//m_kr2376->x<2>().set_ioport("X2");
	//m_kr2376->x<3>().set_ioport("X3");
	//m_kr2376->x<4>().set_ioport("X4");
	//m_kr2376->x<5>().set_ioport("X5");
	//m_kr2376->x<6>().set_ioport("X6");
	//m_kr2376->x<7>().set_ioport("X7");
	//m_kr2376->shift().set(FUNC(poly_state::kbd_shift_r));
	//m_kr2376->control().set(FUNC(poly_state::kbd_control_r));
	//m_kr2376->strobe().set("pia1", FUNC(pia6821_device::cb1_w));

	/* generic keyboard until ROM in KR2376-12 is known */
	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(poly_state::kbd_put));

	/* video control */
	PIA6821(config, m_pia[0], 0);
	m_pia[0]->writepa_handler().set(FUNC(poly_state::pia0_pa_w));
	m_pia[0]->writepb_handler().set(FUNC(poly_state::pia0_pb_w));
	m_pia[0]->irqa_handler().set("irqs", FUNC(input_merger_device::in_w<2>));
	m_pia[0]->irqb_handler().set("irqs", FUNC(input_merger_device::in_w<3>));

	/* keyboard PIA */
	PIA6821(config, m_pia[1], 0);
	m_pia[1]->readpb_handler().set(FUNC(poly_state::pia1_b_in));
	m_pia[1]->irqa_handler().set("irqs", FUNC(input_merger_device::in_w<4>));
	m_pia[1]->irqb_handler().set("irqs", FUNC(input_merger_device::in_w<5>));

	/* optional rs232 interface */
	ACIA6850(config, m_acia, 0);
	//m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	//m_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_acia->irq_handler().set("irqs", FUNC(input_merger_device::in_w<6>));

	CLOCK(config, m_acia_clock, 153600);
	m_acia_clock->signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
	m_acia_clock->signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));

	/* software lists */
	SOFTWARE_LIST(config, "flop_list").set_original("poly_flop").set_filter("POLY1");
}


void poly_state::poly2(machine_config &config)
{
	poly(config);

	/* internal ram */
	m_ram->set_default_size("128K");

	/* software lists */
	subdevice<software_list_device>("flop_list")->set_filter("POLY2");
}


void polydev_state::polydev(machine_config &config)
{
	poly(config);

	/* fdc */
	FD1771(config, m_fdc, 12.0_MHz_XTAL / 12);
	m_fdc->hld_wr_callback().set(FUNC(polydev_state::motor_w));
	m_fdc->set_force_ready(true);

	FLOPPY_CONNECTOR(config, "fdc:0", poly_floppies, "525sd", polydev_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", poly_floppies, nullptr, polydev_state::floppy_formats).enable_sound(true);

	/* remove devices*/
	//config.device_remove("netup");
	//config.device_remove("netdown");
}


/* ROM definition */
ROM_START( poly1 )
	ROM_REGION( 0x20000, "user", 0 )
	ROM_SYSTEM_BIOS(0, "bas34", "PolyBASIC 3.4")
	ROMX_LOAD( "bas1.u92",       0xc000, 0x1000, CRC(04d96d81) SHA1(8d2d0980afb8c447cf235325e4dc15ed2a200f16), ROM_BIOS(0) )
	ROMX_LOAD( "bas2.u91",       0xd000, 0x1000, CRC(3e12f823) SHA1(0b37f2dfa241fac1bf06ca93b19e44a660c7758e), ROM_BIOS(0) )
	ROMX_LOAD( "bas3.u90",       0xe000, 0x1000, CRC(22759a84) SHA1(d22edea312567596b4ef92290ffe52a25de01487), ROM_BIOS(0) )
	ROMX_LOAD( "bas4.u89",       0xf000, 0x1000, CRC(30650d92) SHA1(4e41ea2ec127b9ed277f5a62d52cb3432d64aa84), ROM_BIOS(0) )

	ROM_REGION( 0x1000, "system", 0 )
	ROMX_LOAD( "bios.u86",       0x0000, 0x1000, CRC(fc97cc6a) SHA1(103dce01a86a47e7e235c9d2f820fa1501ab9800), ROM_BIOS(0) )
ROM_END

ROM_START( poly1e )
	ROM_REGION( 0x20000, "user", 0 )
	ROM_SYSTEM_BIOS(0, "bas23", "PolyBASIC 2.3")
	ROMX_LOAD( "v3bas1.u92",     0xc000, 0x1000, CRC(ee25fe89) SHA1(af1a73434c9f5524c5a1a5e19d06500ad1b643d1), ROM_BIOS(0) )
	ROMX_LOAD( "v3bas2.u91",     0xd000, 0x1000, CRC(6ca4a8b5) SHA1(54e71e34b55a5ee41a9e0da05d9c9cbcb2fb80c2), ROM_BIOS(0) )
	ROMX_LOAD( "v3bas3.u90",     0xe000, 0x1000, CRC(6021fc00) SHA1(214aab19e096ddfd417993d48c11f81ec139fef3), ROM_BIOS(0) )
	ROMX_LOAD( "v3bas4.u89",     0xf000, 0x1000, CRC(df071e52) SHA1(41482517a5dfc64f3728732b3907cee636674de4), ROM_BIOS(0) )

	ROM_REGION( 0x1000, "system", 0 )
	ROMX_LOAD( "plrt16v3e9.u86", 0x0000, 0x1000, CRC(f7e3aa86) SHA1(b642c281e54ad9698cfaec19508ae8b4df50c296), ROM_BIOS(0) )
ROM_END

ROM_START( poly2 )
	ROM_DEFAULT_BIOS("bas31")
	ROM_SYSTEM_BIOS(0, "bas31", "PolyBASIC 3.1")
	ROM_SYSTEM_BIOS(1, "bas30", "PolyBASIC 3.0")

	ROM_REGION( 0x20000, "user", 0 )
	ROMX_LOAD( "bas1.u92",           0xc000, 0x1000, CRC(340b7d75) SHA1(330d31b5c90c82c08c62e2df40669ca62c8fffed), ROM_BIOS(0) )
	ROMX_LOAD( "bas2.u91",           0xd000, 0x1000, CRC(45152d26) SHA1(7da2663f253031a587705b9db9a18e93ba4db2cf), ROM_BIOS(0) )
	ROMX_LOAD( "bas3.u90",           0xe000, 0x1000, CRC(a6f70e62) SHA1(7912a9da29d682bb5922f3adf6136b4efc0494dc), ROM_BIOS(0) )
	ROMX_LOAD( "bas4.u89",           0xf000, 0x1000, CRC(72d21cea) SHA1(d413490d043845ce4a9cf5cc70bd92ddc931c837), ROM_BIOS(0) )

	ROMX_LOAD( "bas1-12-12-84.u92",  0xc000, 0x1000, CRC(a3791342) SHA1(e801db28419eedf6eebdbb5a7eb551ba36c43cd6), ROM_BIOS(1) )
	ROMX_LOAD( "bas2-12-12-84.u91",  0xd000, 0x1000, CRC(3bb5849e) SHA1(71c47a0d3dba096a6a79300edf56ffedce276ac6), ROM_BIOS(1) )
	ROMX_LOAD( "bas3-12-12-84.u90",  0xe000, 0x1000, CRC(a6f70e62) SHA1(7912a9da29d682bb5922f3adf6136b4efc0494dc), ROM_BIOS(1) )
	ROMX_LOAD( "bas4-12-12-84.u89",  0xf000, 0x1000, CRC(8f736cee) SHA1(3ec5cc49a426fc921bbadb2bcc1b86b4768627fa), ROM_BIOS(1) )


	ROM_REGION( 0x1000, "system", 0 )
	ROMX_LOAD( "sys31.u86",          0x0000, 0x1000, CRC(fb54c36e) SHA1(934f84a7a99a76b0a017379a6ecd8a8e444cd085), ROM_BIOS(0) )
	ROMX_LOAD( "plrt17-5-11-84.u86", 0x0000, 0x1000, CRC(896165dd) SHA1(005584310f1c689a9b1bb549989c5fedabead6c4), ROM_BIOS(1) )
ROM_END

ROM_START( polydev )
	ROM_REGION( 0x20000, "user", 0 )
	ROM_SYSTEM_BIOS(0, "bas34", "PolyBASIC 3.4")
	ROMX_LOAD( "v2bas1.bin",         0xc000, 0x1000, CRC(04d96d81) SHA1(8d2d0980afb8c447cf235325e4dc15ed2a200f16), ROM_BIOS(0) )
	ROMX_LOAD( "v2bas2.bin",         0xd000, 0x1000, CRC(3e12f823) SHA1(0b37f2dfa241fac1bf06ca93b19e44a660c7758e), ROM_BIOS(0) )
	ROMX_LOAD( "v2bas3.bin",         0xe000, 0x1000, CRC(22759a84) SHA1(d22edea312567596b4ef92290ffe52a25de01487), ROM_BIOS(0) )
	ROMX_LOAD( "v2bas4.bin",         0xf000, 0x1000, CRC(30650d92) SHA1(4e41ea2ec127b9ed277f5a62d52cb3432d64aa84), ROM_BIOS(0) )

	ROM_REGION( 0x1000, "system", 0 )
	ROMX_LOAD( "slrt15_00f9.bin",    0x0000, 0x1000, CRC(046a9aef) SHA1(c5c74b0f66e8969c12db03899244e18228be38eb), ROM_BIOS(0) )
ROM_END

/* Driver */

/*    YEAR   NAME     PARENT  COMPAT   MACHINE    INPUT   CLASS          INIT       COMPANY     FULLNAME                               FLAGS */
COMP( 1981,  poly1,   0,      0,       poly,      poly1,  poly_state,    init_poly, "Polycorp", "Poly 1 Educational Computer",         MACHINE_NOT_WORKING )
COMP( 1981,  poly1e,  poly1,  0,       poly,      poly1e, poly_state,    init_poly, "Polycorp", "Poly 1 Educational Computer (early)", MACHINE_NOT_WORKING )
COMP( 1984,  poly2,   poly1,  0,       poly2,     poly2,  poly_state,    init_poly, "Polycorp", "Poly 2 Learning System",              MACHINE_NOT_WORKING )
COMP( 1983,  polydev, poly1,  0,       polydev,   poly1,  polydev_state, init_poly, "Polycorp", "Poly Development System",             MACHINE_NOT_WORKING )
