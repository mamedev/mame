// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    dragon.c

    Dragon

***************************************************************************/

#include "emu.h"
#include "includes/dragon.h"
#include "includes/dgnalpha.h"
#include "imagedev/cassette.h"
#include "formats/coco_cas.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m6809/hd6309.h"
#include "formats/vdk_dsk.h"
#include "formats/dmk_dsk.h"
#include "formats/sdf_dsk.h"
#include "imagedev/floppy.h"

#include "bus/coco/dragon_fdc.h"
#include "bus/coco/dragon_jcbsnd.h"
#include "bus/coco/coco_pak.h"
#include "bus/coco/coco_ssc.h"
#include "bus/coco/coco_orch90.h"
#include "bus/coco/coco_gmc.h"


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( dragon_mem )
//-------------------------------------------------

void dragon_state::dragon_mem(address_map &map)
{
}


//**************************************************************************
//  INPUT PORTS
//**************************************************************************

/* Dragon keyboard

             PB0 PB1 PB2 PB3 PB4 PB5 PB6 PB7
    PA6: Ent Clr Brk N/c N/c N/c N/c Shift
    PA5: X   Y   Z   Up  Dwn Lft Rgt Space
    PA4: P   Q   R   S   T   U   V   W
    PA3: H   I   J   K   L   M   N   O
    PA2: @   A   B   C   D   E   F   G
    PA1: 8   9   :   ;   ,   -   .   /
    PA0: 0   1   2   3   4   5   6   7
 */
static INPUT_PORTS_START( dragon_keyboard )
	PORT_START("row0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("row1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_MINUS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("row2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_G) PORT_CHAR('G')

	PORT_START("row3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_O) PORT_CHAR('O')

	PORT_START("row4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_W) PORT_CHAR('W')

	PORT_START("row5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP), '^')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN), 10)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(UCHAR_MAMEKEY(LEFT), 8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT), 9)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("row6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_NAME("CLEAR") PORT_CODE(KEYCODE_HOME) PORT_CHAR(12)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_NAME("BREAK") PORT_CODE(KEYCODE_END) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x78, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
INPUT_PORTS_END

static INPUT_PORTS_START( dragon200e_keyboard )
	PORT_INCLUDE(dragon_keyboard)

	PORT_MODIFY("row0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_NAME("0 \xC3\x87") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(0xC7)

	PORT_MODIFY("row1")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_NAME("\xC3\x91") PORT_CODE(KEYCODE_COLON) PORT_CHAR(0xD1)

	PORT_MODIFY("row2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(';') PORT_CHAR('+')

	PORT_MODIFY("row5")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_NAME("DOWN \xC2\xA1") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(10) PORT_CHAR(0xA1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_NAME("RIGHT \xC2\xBF") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(9) PORT_CHAR(0xBF)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_CHAR(0xA7)

	PORT_MODIFY("row6")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_NAME("CLEAR @") PORT_CODE(KEYCODE_HOME) PORT_CHAR(12) PORT_CHAR('@')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, nullptr) PORT_NAME("BREAK \xC3\xBC") PORT_CODE(KEYCODE_END) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27) PORT_CHAR(0xFC)
INPUT_PORTS_END

static INPUT_PORTS_START( dragon )
	PORT_INCLUDE(dragon_keyboard)
	PORT_INCLUDE(coco_joystick)
	PORT_INCLUDE(coco_analog_control)
INPUT_PORTS_END

static INPUT_PORTS_START( dragon200e )
	PORT_INCLUDE(dragon200e_keyboard)
	PORT_INCLUDE(coco_joystick)
	PORT_INCLUDE(coco_analog_control)
INPUT_PORTS_END

MC6847_GET_CHARROM_MEMBER( dragon200e_state::char_rom_r )
{
	return m_char_rom->base()[(ch * 12 + line) & 0xfff];
}

void dragon_cart(device_slot_interface &device)
{
	device.option_add("dragon_fdc", DRAGON_FDC);
	device.option_add("premier_fdc", PREMIER_FDC);
	device.option_add("sdtandy_fdc", SDTANDY_FDC);
	device.option_add("jcbsnd", DRAGON_JCBSND);
	device.option_add("ssc", COCO_SSC);
	device.option_add("orch90", COCO_ORCH90);
	device.option_add("gmc", COCO_PAK_GMC);
	device.option_add("pak", COCO_PAK);
}

FLOPPY_FORMATS_MEMBER( dragon_alpha_state::dragon_formats )
	FLOPPY_VDK_FORMAT,
	FLOPPY_DMK_FORMAT,
	FLOPPY_SDF_FORMAT
FLOPPY_FORMATS_END

static void dragon_alpha_floppies(device_slot_interface &device)
{
	device.option_add("dd", FLOPPY_35_DD);
}

void dragon_state::dragon_base(machine_config &config)
{
	this->set_clock(14.218_MHz_XTAL / 16);

	// basic machine hardware
	MC6809E(config, m_maincpu, DERIVED_CLOCK(1, 1));
	m_maincpu->set_addrmap(AS_PROGRAM, &dragon_state::dragon_mem);

	// devices
	pia6821_device &pia0(PIA6821(config, PIA0_TAG, 0));
	pia0.writepa_handler().set(FUNC(coco_state::pia0_pa_w));
	pia0.writepb_handler().set(FUNC(coco_state::pia0_pb_w));
	pia0.ca2_handler().set(FUNC(coco_state::pia0_ca2_w));
	pia0.cb2_handler().set(FUNC(coco_state::pia0_cb2_w));
	pia0.irqa_handler().set(FUNC(coco_state::pia0_irq_a));
	pia0.irqb_handler().set(FUNC(coco_state::pia0_irq_b));

	pia6821_device &pia1(PIA6821(config, PIA1_TAG, 0));
	pia1.readpa_handler().set(FUNC(coco_state::pia1_pa_r));
	pia1.readpb_handler().set(FUNC(coco_state::pia1_pb_r));
	pia1.writepa_handler().set(FUNC(coco_state::pia1_pa_w));
	pia1.writepb_handler().set(FUNC(coco_state::pia1_pb_w));
	pia1.ca2_handler().set(FUNC(coco_state::pia1_ca2_w));
	pia1.cb2_handler().set(FUNC(coco_state::pia1_cb2_w));
	pia1.irqa_handler().set(FUNC(coco_state::pia1_firq_a));
	pia1.irqb_handler().set(FUNC(coco_state::pia1_firq_b));

	SAM6883(config, m_sam, 14.218_MHz_XTAL, m_maincpu);
	m_sam->res_rd_callback().set(FUNC(dragon_state::sam_read));
	CASSETTE(config, m_cassette);
	m_cassette->set_formats(coco_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED);
	m_cassette->set_interface("dragon_cass");

	PRINTER(config, m_printer, 0);

	// video hardware
	SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER);

	MC6847_PAL(config, m_vdg, 4.433619_MHz_XTAL);
	m_vdg->set_screen(SCREEN_TAG);
	m_vdg->hsync_wr_callback().set(FUNC(dragon_state::horizontal_sync));
	m_vdg->fsync_wr_callback().set(FUNC(dragon_state::field_sync));
	m_vdg->input_callback().set(m_sam, FUNC(sam6883_device::display_read));

	// sound hardware
	coco_sound(config);

	// floating space
	coco_floating(config);

	// software lists
	SOFTWARE_LIST(config, "dragon_cart_list").set_original("dragon_cart");
	SOFTWARE_LIST(config, "dragon_cass_list").set_original("dragon_cass");
	SOFTWARE_LIST(config, "dragon_flop_list").set_original("dragon_flop");
	SOFTWARE_LIST(config, "coco_cart_list").set_compatible("coco_cart");
}

void dragon_state::dragon32(machine_config &config)
{
	dragon_base(config);
	// internal ram
	RAM(config, m_ram).set_default_size("32K").set_extra_options("64K");

	// cartridge
	cococart_slot_device &cartslot(COCOCART_SLOT(config, CARTRIDGE_TAG, DERIVED_CLOCK(1, 1), dragon_cart, "dragon_fdc"));
	cartslot.cart_callback().set([this] (int state) { cart_w(state != 0); }); // lambda because name is overloaded
	cartslot.nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	cartslot.halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
}

void dragon64_state::dragon64(machine_config &config)
{
	dragon_base(config);
	// internal ram
	RAM(config, m_ram).set_default_size("64K");

	// cartridge
	cococart_slot_device &cartslot(COCOCART_SLOT(config, CARTRIDGE_TAG, DERIVED_CLOCK(1, 1), dragon_cart, "dragon_fdc"));
	cartslot.cart_callback().set([this] (int state) { cart_w(state != 0); }); // lambda because name is overloaded
	cartslot.nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	cartslot.halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);

	// acia
	mos6551_device &acia(MOS6551(config, "acia", 0));
	acia.set_xtal(1.8432_MHz_XTAL);

	// software lists
	SOFTWARE_LIST(config, "dragon_flex_list").set_original("dragon_flex");
	SOFTWARE_LIST(config, "dragon_os9_list").set_original("dragon_os9");
}

void dragon64_state::dragon64h(machine_config &config)
{
	dragon64(config);
	// Replace M6809 with HD6309
	HD6309E(config.replace(), m_maincpu, DERIVED_CLOCK(1, 1));
	m_maincpu->set_addrmap(AS_PROGRAM, &dragon64_state::dragon_mem);
	m_ram->set_default_size("64K");
}

void dragon200e_state::dragon200e(machine_config &config)
{
	dragon64(config);
	// video hardware
	m_vdg->set_get_char_rom(FUNC(dragon200e_state::char_rom_r));
}

void d64plus_state::d64plus(machine_config &config)
{
	dragon64(config);
	// video hardware
	screen_device &plus_screen(SCREEN(config, "plus_screen", SCREEN_TYPE_RASTER));
	plus_screen.set_refresh_hz(50);
	plus_screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	plus_screen.set_size(640, 264);
	plus_screen.set_visarea_full();
	plus_screen.set_screen_update("crtc", FUNC(hd6845_device::screen_update));
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	// crtc
	HD6845(config, m_crtc, 14.218_MHz_XTAL / 4 / 2);
	m_crtc->set_screen("plus_screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(d64plus_state::crtc_update_row), this);
}

void dragon_alpha_state::dgnalpha(machine_config &config)
{
	dragon_base(config);
	// internal ram
	RAM(config, RAM_TAG).set_default_size("64K");

	// cartridge
	cococart_slot_device &cartslot(COCOCART_SLOT(config, CARTRIDGE_TAG, DERIVED_CLOCK(1, 1), dragon_cart, nullptr));
	cartslot.cart_callback().set([this] (int state) { cart_w(state != 0); }); // lambda because name is overloaded
	cartslot.nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	cartslot.halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);

	// acia
	mos6551_device &acia(MOS6551(config, "acia", 0));
	acia.set_xtal(1.8432_MHz_XTAL);

	// floppy
	WD2797(config, m_fdc, 4_MHz_XTAL/4);
	m_fdc->intrq_wr_callback().set(FUNC(dragon_alpha_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(dragon_alpha_state::fdc_drq_w));

	FLOPPY_CONNECTOR(config, WD2797_TAG ":0", dragon_alpha_floppies, "dd", dragon_alpha_state::dragon_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, WD2797_TAG ":1", dragon_alpha_floppies, "dd", dragon_alpha_state::dragon_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, WD2797_TAG ":2", dragon_alpha_floppies, nullptr, dragon_alpha_state::dragon_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, WD2797_TAG ":3", dragon_alpha_floppies, nullptr, dragon_alpha_state::dragon_formats).enable_sound(true);

	// sound hardware
	ay8912_device &ay8912(AY8912(config, AY8912_TAG, 4_MHz_XTAL/4));
	ay8912.port_a_read_callback().set(FUNC(dragon_alpha_state::psg_porta_read));
	ay8912.port_a_write_callback().set(FUNC(dragon_alpha_state::psg_porta_write));
	ay8912.add_route(ALL_OUTPUTS, "speaker", 0.75);

	// pia 2
	pia6821_device &pia2(PIA6821(config, PIA2_TAG, 0));
	pia2.writepa_handler().set(FUNC(dragon_alpha_state::pia2_pa_w));
	pia2.irqa_handler().set(FUNC(dragon_alpha_state::pia2_firq_a));
	pia2.irqb_handler().set(FUNC(dragon_alpha_state::pia2_firq_b));

	// software lists
	SOFTWARE_LIST(config, "dgnalpha_flop_list").set_original("dgnalpha_flop");
	SOFTWARE_LIST(config, "dragon_flex_list").set_original("dragon_flex");
	SOFTWARE_LIST(config, "dragon_os9_list").set_original("dragon_os9");
}

void dragon64_state::tanodr64(machine_config &config)
{
	dragon64(config);
	this->set_clock(14.318181_MHz_XTAL / 16);

	m_sam->set_clock(14.318181_MHz_XTAL);

	// video hardware
	MC6847_NTSC(config.replace(), m_vdg, 14.318181_MHz_XTAL / 4);
	m_vdg->set_screen(SCREEN_TAG);
	m_vdg->hsync_wr_callback().set(FUNC(dragon_state::horizontal_sync));
	m_vdg->fsync_wr_callback().set(FUNC(dragon_state::field_sync));
	m_vdg->input_callback().set(m_sam, FUNC(sam6883_device::display_read));

	// cartridge
	subdevice<cococart_slot_device>(CARTRIDGE_TAG)->set_default_option("sdtandy_fdc");
}

void dragon64_state::tanodr64h(machine_config &config)
{
	tanodr64(config);
	// Replace M6809 CPU with HD6309 CPU
	HD6309E(config.replace(), m_maincpu, DERIVED_CLOCK(1, 1));
	m_maincpu->set_addrmap(AS_PROGRAM, &dragon64_state::dragon_mem);
	m_ram->set_default_size("64K");
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(dragon32)
	ROM_REGION(0xC000, "maincpu",0)
	ROM_LOAD("d32.rom",      0x0000,  0x4000, CRC(e3879310) SHA1(f2dab125673e653995a83bf6b793e3390ec7f65a))
ROM_END

ROM_START(dragon64)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("d64_1.rom",    0x0000,  0x4000, CRC(60a4634c) SHA1(f119506eaa3b4b70b9aa0dd83761e8cbe043d042))
	ROM_LOAD("d64_2.rom",    0x8000,  0x4000, CRC(17893a42) SHA1(e3c8986bb1d44269c4587b04f1ca27a70b0aaa2e))
ROM_END

ROM_START(dragon200)
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ic18.rom",    0x0000, 0x4000, CRC(84f68bf9) SHA1(1983b4fb398e3dd9668d424c666c5a0b3f1e2b69))
	ROM_LOAD( "ic17.rom",    0x8000, 0x4000, CRC(17893a42) SHA1(e3c8986bb1d44269c4587b04f1ca27a70b0aaa2e))
ROM_END

ROM_START(dragon200e)
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ic18_v1.4e.ic34",    0x0000, 0x4000, CRC(95af0a0a) SHA1(628543ee8b47a56df2b2175cfb763c0051517b90))
	ROM_LOAD( "ic17_v1.4e.ic37",    0x8000, 0x4000, CRC(48b985df) SHA1(c25632f3c2cfd1af3ee26b2f233a1ce1eccc365d))
	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "rom26.ic1",          0x0000, 0x1000, CRC(565724bc) SHA1(da5b756ba2a9c9ecebaa7daa8ba8bfd984d56a6f))
ROM_END

ROM_START(d64plus)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("d64_1.rom",      0x0000, 0x4000, CRC(60a4634c) SHA1(f119506eaa3b4b70b9aa0dd83761e8cbe043d042))
	ROM_LOAD("d64_2.rom",      0x8000, 0x4000, CRC(17893a42) SHA1(e3c8986bb1d44269c4587b04f1ca27a70b0aaa2e))
	ROM_REGION(0x0200, "prom", 0)
	ROM_LOAD("n82s147an.ic12", 0x0000, 0x0200, CRC(92b6728d) SHA1(bcf7c60c4e5608a58587044458d9cacaca4568aa))
	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("chargen.ic22",   0x0000, 0x2000, CRC(514f1450) SHA1(956c99fcca1b52e79bb5d91dbafc817c992e324a))
ROM_END

ROM_START(tanodr64)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("d64_1.rom",    0x0000,  0x4000, CRC(60a4634c) SHA1(f119506eaa3b4b70b9aa0dd83761e8cbe043d042))
	ROM_LOAD("d64_2.rom",    0x8000,  0x4000, CRC(17893a42) SHA1(e3c8986bb1d44269c4587b04f1ca27a70b0aaa2e))
ROM_END

ROM_START(dgnalpha)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_DEFAULT_BIOS("boot10")
	ROM_SYSTEM_BIOS(0, "boot10", "Boot v1.0")
	ROMX_LOAD("alpha_bt_10.rom", 0x2000,  0x2000, CRC(c3dab585) SHA1(4a5851aa66eb426e9bb0bba196f1e02d48156068), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "boot04", "Boot v0.4")
	ROMX_LOAD("alpha_bt_04.rom", 0x2000,  0x2000, CRC(d6172b56) SHA1(69ea376dbc7418f69e9e809b448d22a4de012344), ROM_BIOS(1))
	ROM_LOAD("alpha_ba.rom",    0x8000,  0x4000, CRC(84f68bf9) SHA1(1983b4fb398e3dd9668d424c666c5a0b3f1e2b69))
ROM_END

#define rom_dragon64h rom_dragon64
#define rom_tanodr64h rom_tanodr64

//    YEAR  NAME        PARENT    COMPAT  MACHINE     INPUT       CLASS               INIT        COMPANY                         FULLNAME                       FLAGS
COMP( 1982, dragon32,   0,        0,      dragon32,   dragon,     dragon_state,       empty_init, "Dragon Data Ltd",              "Dragon 32",                      0 )
COMP( 1983, dragon64,   dragon32, 0,      dragon64,   dragon,     dragon64_state,     empty_init, "Dragon Data Ltd",              "Dragon 64",                      0 )
COMP( 19??, dragon64h,  dragon32, 0,      dragon64h,  dragon,     dragon64_state,     empty_init, "Dragon Data Ltd",              "Dragon 64 (HD6309E CPU)",        MACHINE_UNOFFICIAL )
COMP( 1985, dragon200,  dragon32, 0,      dragon64,   dragon,     dragon64_state,     empty_init, "Eurohard S.A.",                "Dragon 200",                     0 )
COMP( 1985, dragon200e, dragon32, 0,      dragon200e, dragon200e, dragon200e_state,   empty_init, "Eurohard S.A.",                "Dragon 200-E",                   MACHINE_NOT_WORKING )
COMP( 1985, d64plus,    dragon32, 0,      d64plus,    dragon,     d64plus_state,      empty_init, "Dragon Data Ltd / Compusense", "Dragon 64 Plus",                 0 )
COMP( 1983, tanodr64,   dragon32, 0,      tanodr64,   dragon,     dragon64_state,     empty_init, "Dragon Data Ltd / Tano Ltd",   "Tano Dragon 64 (NTSC)",          0 )
COMP( 19??, tanodr64h,  dragon32, 0,      tanodr64h,  dragon,     dragon64_state,     empty_init, "Dragon Data Ltd / Tano Ltd",   "Tano Dragon 64 (NTSC; HD6309E)", MACHINE_UNOFFICIAL )
COMP( 1984, dgnalpha,   dragon32, 0,      dgnalpha,   dragon,     dragon_alpha_state, empty_init, "Dragon Data Ltd",              "Dragon Professional (Alpha)",    0 )
