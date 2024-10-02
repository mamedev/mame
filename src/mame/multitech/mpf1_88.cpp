// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Multitech Micro-Professor I/88

    TODO:
    - IFM-I/88 interrupts, need documentation/schematic.

***************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"
#include "video/hd44780.h"
#include "bus/centronics/ctronics.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "bus/isa/isa.h"
#include "bus/isa/cga.h"
#include "bus/isa/com.h"
#include "bus/isa/mda.h"
//#include "bus/mpf1/slot.h"
#include "imagedev/cassette.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class mpf1_88_state : public driver_device
{
public:
	mpf1_88_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, "ram")
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_key(*this, "KC%u", 0U)
		, m_ctrl(*this, "CTRL")
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
		, m_leds(*this, "led%u", 0U)
		, m_centronics_busy(0)
	{ }

	void mpf1_88(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER( trigger_res );

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_ioport_array<12> m_key;
	required_ioport m_ctrl;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	output_finder<2> m_leds;

	TIMER_DEVICE_CALLBACK_MEMBER(check_halt_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(key_nmi);

	HD44780_PIXEL_UPDATE(lcd_pixel_update);

	void lcd_palette(palette_device &palette) const;

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	uint8_t ipd_port_r();
	void opd_port1_w(uint8_t data);
	void opd_port2_w(uint8_t data);

	uint16_t m_key_col = 0;

	int m_centronics_busy;
	bool m_nmi_enable = false;
};


void mpf1_88_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0xf4000, 0xf7fff).r("rom_2", FUNC(generic_slot_device::read_rom));
	map(0xf8000, 0xfbfff).r("rom_1", FUNC(generic_slot_device::read_rom));
	map(0xfc000, 0xfffff).rom().region("rom_0", 0);
}

void mpf1_88_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0160, 0x0160).w(FUNC(mpf1_88_state::opd_port2_w));
	map(0x0180, 0x0180).w(FUNC(mpf1_88_state::opd_port1_w));
	map(0x01a0, 0x01a1).w("lcdc", FUNC(hd44780_device::write));
	map(0x01a2, 0x01a3).r("lcdc", FUNC(hd44780_device::read));
	map(0x01c0, 0x01c0).r(FUNC(mpf1_88_state::ipd_port_r));
	map(0x01e0, 0x01e0).w(m_cent_data_out, FUNC(output_latch_device::write));
}


INPUT_CHANGED_MEMBER( mpf1_88_state::trigger_res )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
}


static INPUT_PORTS_START( mpf1_88 )
	PORT_START("KC0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR('<')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)          PORT_CHAR('V')  PORT_CHAR('v')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)          PORT_CHAR('B')  PORT_CHAR('b')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KC1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)          PORT_CHAR('K')  PORT_CHAR('k')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)          PORT_CHAR('T')  PORT_CHAR('t')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)          PORT_CHAR('U')  PORT_CHAR('u')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KC2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')  PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)          PORT_CHAR('C')  PORT_CHAR('c')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)          PORT_CHAR('Q')  PORT_CHAR('q')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)          PORT_CHAR('Y')  PORT_CHAR('y')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KC3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)          PORT_CHAR('L')  PORT_CHAR('l')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)          PORT_CHAR('F')  PORT_CHAR('f')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('*')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KC4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')  PORT_CHAR('?')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')  PORT_CHAR('>')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)          PORT_CHAR('X')  PORT_CHAR('x')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)          PORT_CHAR('W')  PORT_CHAR('w')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)          PORT_CHAR('J')  PORT_CHAR('j')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KC5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')  PORT_CHAR('{')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)          PORT_CHAR('S')  PORT_CHAR('s')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)          PORT_CHAR('3')  PORT_CHAR('#')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')  PORT_CHAR('&')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KC6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)          PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)          PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)          PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KC7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')  PORT_CHAR(':')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)          PORT_CHAR('E')  PORT_CHAR('e')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)          PORT_CHAR('2')  PORT_CHAR('@')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)          PORT_CHAR('N')  PORT_CHAR('n')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KC8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')  PORT_CHAR('}')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)          PORT_CHAR('P')  PORT_CHAR('p')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)          PORT_CHAR('4')  PORT_CHAR('$')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)          PORT_CHAR('A')  PORT_CHAR('a')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)          PORT_CHAR('G')  PORT_CHAR('g')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KC9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('`')  PORT_CHAR('~')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)          PORT_CHAR('M')  PORT_CHAR('m')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)          PORT_CHAR('R')  PORT_CHAR('r')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)          PORT_CHAR('1')  PORT_CHAR('!')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)          PORT_CHAR('I')  PORT_CHAR('i')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KC10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)          PORT_CHAR('O')  PORT_CHAR('o')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)          PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_END)        PORT_CHAR(UCHAR_MAMEKEY(END))  PORT_NAME("\xe2\x87\xa5 \xe2\x87\xa4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)          PORT_CHAR('6')  PORT_CHAR('^')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KC11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LALT)       PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("CTRL")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0xdf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("RESET")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3) PORT_NAME("RESET") PORT_CHANGED_MEMBER(DEVICE_SELF, mpf1_88_state, trigger_res, 0)
INPUT_PORTS_END


uint8_t mpf1_88_state::ipd_port_r()
{
	uint8_t data = 0x3f;

	// bit 0 to 4, keyboard array input
	for (int row = 0; row < 12; row++)
		if (!BIT(m_key_col, row))
			data &= m_key[row]->read();

	// bit 5, control key
	data &= m_ctrl->read();

	// bit 6, printer busy
	data |= m_centronics_busy ? 0x40 : 0;

	// bit 7, tape in
	data |= ((m_cassette)->input() > 0) ? 0x80 : 0;

	return data;
}

void mpf1_88_state::opd_port1_w(uint8_t data)
{
	// bit 0-3, keyboard array output
	m_key_col = (m_key_col & 0x00ff) | (data << 8);

	// bit 4, NMI enable */
	m_nmi_enable = BIT(data, 4);

	// bit 6, tape out (beep)
	m_leds[0] = !BIT(data, 6);
	m_speaker->level_w(BIT(data, 6));
	m_cassette->output(BIT(data, 6) ? 1.0 : -1.0);

	// bit 7, printer strobe
	m_centronics->write_strobe(BIT(data, 7));
}

void mpf1_88_state::opd_port2_w(uint8_t data)
{
	// bit 0-7, keyboard array output
	m_key_col = (m_key_col & 0xff00) | data;
}


HD44780_PIXEL_UPDATE(mpf1_88_state::lcd_pixel_update)
{
	if (line < 2 && pos < 20)
		bitmap.pix(6 + line * (8 + 1) + y, 6 + pos * 6 + x) = state ? 1 : 2;
}

void mpf1_88_state::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t( 92,  83,  88)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // lcd pixel off
}


TIMER_DEVICE_CALLBACK_MEMBER(mpf1_88_state::check_halt_callback)
{
	// hold-LED; the red one, is turned on when the processor is halted
	int led_halt = m_maincpu->state_int(I8086_HALT);
	m_leds[1] = led_halt;
}

TIMER_DEVICE_CALLBACK_MEMBER(mpf1_88_state::key_nmi)
{
	if (m_nmi_enable)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void mpf1_88_state::machine_start()
{
	m_leds.resolve();

	m_maincpu->space(AS_PROGRAM).install_ram(0, m_ram->mask(), m_ram->pointer());

	// register for state saving
	save_item(NAME(m_nmi_enable));
}

void mpf1_88_state::machine_reset()
{
	m_nmi_enable = false;

	m_key_col = 0x00;
}


static void mpf1_88_isa8_cards(device_slot_interface &device)
{
	device.option_add("com", ISA8_COM);
	device.option_add("cga", ISA8_CGA);
	device.option_add("mda", ISA8_MDA);
}


void mpf1_88_state::mpf1_88(machine_config &config)
{
	I8088(config, m_maincpu, 14.318181_MHz_XTAL/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &mpf1_88_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &mpf1_88_state::io_map);

	TIMER(config, "halt_timer").configure_periodic(FUNC(mpf1_88_state::check_halt_callback), attotime::from_hz(1));

	TIMER(config, "nmi_timer").configure_periodic(FUNC(mpf1_88_state::key_nmi), attotime::from_msec(15));

	auto &screen = SCREEN(config, "screen", SCREEN_TYPE_LCD);
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(132, 28);
	screen.set_visarea_full();
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(mpf1_88_state::lcd_palette), 3);

	hd44780_device &lcdc(HD44780(config, "lcdc", 270'000)); // TODO: clock not measured, datasheet typical clock used
	lcdc.set_lcd_size(2, 20);
	lcdc.set_pixel_update_cb(FUNC(mpf1_88_state::lcd_pixel_update));
	lcdc.set_function_set_at_any_time(true);

	RAM(config, m_ram).set_default_size("8K").set_extra_options("2K,4K,6K,16K,24K");

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set([this](int state) { m_centronics_busy = state; });
	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(latch);

	GENERIC_SOCKET(config, "rom_1", generic_linear_slot, "mpf1_rom", "bin,rom");
	GENERIC_SOCKET(config, "rom_2", generic_linear_slot, "mpf1_rom", "bin,rom");

	SOFTWARE_LIST(config, "rom_ls").set_original("mpf1_rom").set_filter("I88");

	// IFM-I/88 (Interface Module)
	//mpf1_exp_device &exp(MPF1_EXP(config, "exp", 3.579545_MHz_XTAL/2, mpf1_88_exp_devices, nullptr));
	//exp.set_program_space(m_maincpu, AS_PROGRAM);
	//exp.set_io_space(m_maincpu, AS_IO);
	isa8_device &isa8(ISA8(config, "isa", 3.579545_MHz_XTAL/2));
	isa8.set_memspace(m_maincpu, AS_PROGRAM);
	isa8.set_iospace(m_maincpu, AS_IO);
	ISA8_SLOT(config, "isa1", 0, "isa", mpf1_88_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa2", 0, "isa", mpf1_88_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa3", 0, "isa", mpf1_88_isa8_cards, nullptr, false);
}

} // anonymous namespace


ROM_START( mpf1_88 )
	ROM_REGION(0x4000, "rom_0", 0)
	ROM_LOAD("mon88_v1.11.u18", 0x0000, 0x4000, CRC(caa200f3) SHA1(9dfdd618e4cf94b3d3ed97a41d2fb0bf2842b4de))
ROM_END

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY      FULLNAME                  FLAGS
COMP( 1985, mpf1_88,  0,      0,      mpf1_88, mpf1_88, mpf1_88_state, empty_init, "Multitech", "Micro-Professor I/88",   0 )
