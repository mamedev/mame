// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/***************************************************************************

    Itautec I-7000

    driver by Felipe C. da S. Sanches <juca@members.fsf.org>
    with tech info provided by Alexandre Souza (a.k.a. Tabajara).

    The I-7000, released in 1982, was the first computer manufactured by
    Itautec. It was an 8-bit CP/M machine; later hardware revisions turned
    it into an IBM PC-XT clone, the "I-7000 PC-XT".
    (overview: http://pt.wikipedia.org/wiki/Itautec_I-7000)

    * Operating System: SIM/M / BASIC
    * CPU: National NSC800 D-4 at 4.00 MHz
    * Memory: 64KB to 128KB
    * keyboard: 80 keys (with a reduced numerical keypad and function keys)
    * display:
     - 40 X 25 text
     - 80 X 25 text
     - 160 X 100 (8 colors)
     - 640 X 200 (monochrome, with an expansion board)
     - 320 X 200 (16 colors, with an expansion board)
    * Expansion slots:
     - 1 frontal cart slot
     - 4 internal expansion slots
    * Ports:
     - 1 composite video output for a color monitor
     - 2 cassette interfaces
     - 1 RS-232C serial port
     - 1 parallel interface
    * Storage:
     - Cassette recorder
     - Up to 4 external floppy drives: 8" (FD/DD, 1.1MB) or 5" 1/4
     - Up to 1 external 10 MB hard-drive

    TODO:
    - PIT counter 0 drives RSTB, which needs the NSC800 interrupt control
      register (port 0xbb)
    - printer, cassette, joystick ADC, Telex board
    - verify the clocks

****************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/z80/nsc800.h"
#include "machine/i8279.h"
#include "machine/pit8253.h"
#include "sound/spkrdev.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class i7000_state : public driver_device
{
public:
	i7000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_card(*this, "cardslot")
		, m_crtc(*this, "crtc")
		, m_palette(*this, "palette")
		, m_videoram(*this, "videoram")
		, m_chargen(*this, "chargen")
		, m_kbd(*this, "X%u", 0U)
	{ }

	void i7000(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_card;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_videoram;
	required_region_ptr<uint8_t> m_chargen;
	required_ioport_array<8> m_kbd;

	uint8_t m_row = 0;

	void i7000_palette(palette_device &palette) const ATTR_COLD;
	MC6845_UPDATE_ROW(crtc_update_row);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(card_load);

	uint8_t kbd_r();
	void scanlines_w(uint8_t data);

	void i7000_io(address_map &map) ATTR_COLD;
	void i7000_mem(address_map &map) ATTR_COLD;
};

void i7000_state::scanlines_w(uint8_t data)
{
	m_row = data;
}

uint8_t i7000_state::kbd_r()
{
	return m_kbd[m_row & 7]->read();
}

INPUT_PORTS_START( i7000 )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("!") PORT_CHAR('!')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER)

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0x9D")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0x8F")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^R DC2") //0x12

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("$ ^") PORT_CHAR('$') PORT_CHAR('^')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0xA0")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- _") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BACKSPACE") PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("% +") PORT_CHAR('%') PORT_CHAR('+')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0x9C")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@") PORT_CHAR('@')

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 *") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^U NAK") //0x15
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("| <") PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", ;") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^E ENQ") //0x05

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^O SI") //0x0F
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("#") PORT_CHAR('#')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("{") PORT_CHAR('{')

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^L FF") //0x0C
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^T DC4") //0x14

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(">") PORT_CHAR('>')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACEBAR") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("}") PORT_CHAR('}')

	PORT_START("DSW") /* DP01 */
	PORT_DIPNAME( 0x80, 0x80, "1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "2")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x01, 0x01, "8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
INPUT_PORTS_END


void i7000_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	if (m_card->exists())
	{
		// 0x4000 - 0xbfff   32KB ROM
		program.install_read_handler(0x4000, 0xbfff, read8sm_delegate(*m_card, FUNC(generic_slot_device::read_rom)));
	}
}

DEVICE_IMAGE_LOAD_MEMBER(i7000_state::card_load)
{
	uint32_t const size = m_card->common_get_size("rom");
	if (!size || (size > 0x8000) || (size & 0x1fff))
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be 8K, 16K, 24K or 32K)");

	m_card->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_card->common_load_rom(m_card->get_rom_base(), size, "rom");
	return std::make_pair(std::error_condition(), std::string());
}

void i7000_state::i7000_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x33, 0x44, 0x5d));
	palette.set_pen_color(1, rgb_t(0x67, 0xcc, 0xca));
}

MC6845_UPDATE_ROW(i7000_state::crtc_update_row)
{
	uint32_t *p = &bitmap.pix(y);
	for (int col = 0; col < x_count; col++)
	{
		uint8_t const ch = m_videoram[(ma + col) & 0x07ff];
		uint8_t bits = (ra > 7) ? 0 : m_chargen[((ch << 3) | (ra & 0x07)) & 0x07ff];
		if (col == cursor_x)
			bits ^= 0xff;
		for (int cx = 0; cx < 8; cx++)
			*p++ = m_palette->pen(BIT(bits, 7 - cx));
	}
}

/*FIXME: we still need to figure out the proper memory map
         for the maincpu and where the cartridge slot maps to. */
void i7000_state::i7000_mem(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("boot", 0);
	map(0x2000, 0x2fff).ram().share("videoram");
	map(0x4000, 0xffff).ram().share("mainram");
}

void i7000_state::i7000_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x08, 0x08).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x09, 0x09).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x18, 0x1b).rw("pit8253", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x1d, 0x1d).portr("DSW");
	map(0x20, 0x21).rw("i8279", FUNC(i8279_device::read), FUNC(i8279_device::write));
}

void i7000_state::i7000(machine_config &config)
{
	/* basic machine hardware */
	NSC800(config, m_maincpu, 4'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &i7000_state::i7000_mem);
	m_maincpu->set_addrmap(AS_IO, &i7000_state::i7000_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_size(640, 200); /* the CRTC reconfigures it */
	screen.set_visarea(0, 640-1, 0, 200-1);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, FUNC(i7000_state::i7000_palette), 2);

	MC6845(config, m_crtc, 14'318'181 / 8);  /* TODO: verify the dot clock and the 40/80-column divider on the PCB. */
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(i7000_state::crtc_update_row));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	pit8253_device &pit8253(PIT8253(config, "pit8253"));
	pit8253.set_clk<2>(2'000'000);
	pit8253.out_handler<2>().set("speaker", FUNC(speaker_sound_device::level_w));

	/* Keyboard interface */
	i8279_device &kbdc(I8279(config, "i8279", 4000000)); /* guessed value. TODO: verify on PCB */
	kbdc.out_sl_callback().set(FUNC(i7000_state::scanlines_w));         // scan SL lines
	kbdc.in_rl_callback().set(FUNC(i7000_state::kbd_r));                // kbd RL lines
	kbdc.in_shift_callback().set_constant(1);                           // TODO: Shift key
	kbdc.in_ctrl_callback().set_constant(1);                            // TODO: Ctrl key

	/* Cartridge slot */
	GENERIC_CARTSLOT(config, "cardslot", generic_romram_plain_slot, "i7000_card", "rom").set_device_load(FUNC(i7000_state::card_load));

	/* Software lists */
	SOFTWARE_LIST(config, "card_list").set_original("i7000_card");
}

ROM_START( i7000 )
	ROM_REGION( 0x1000, "boot", 0 )
	ROM_LOAD( "i7000_boot_v1_4r02_15_10_85_d52d.rom",  0x0000, 0x1000, CRC(622412e5) SHA1(bf187a095600fd46a739c35132a85b5f39b2f867) )

	ROM_REGION( 0x0800, "chargen", 0 )
	/* Hand-repaired: 0x06a2 and 0x06a3 read 0xf7 and were set to 0xfe
	   (character 0xd4). Characters 0x05-0x08, 'Y' and 'x' may be bad too. */
	ROM_LOAD( "i7000_chargen.rom", 0x0000, 0x0800, BAD_DUMP CRC(fb7383e9) SHA1(71a6561bb9ff3cbf74711fa7ab445f9b43f15626) )

	ROM_REGION( 0x1000, "drive", 0 )
	ROM_LOAD( "i7000_drive_ci01.rom", 0x0000, 0x1000, CRC(d8d6e5c1) SHA1(93e7db42fbfaa8243973321c7fc8c51ed80780be) )

	ROM_REGION( 0x1000, "telex", 0 )
	ROM_LOAD( "i7000_telex_ci09.rom", 0x0000, 0x1000, CRC(c1c8fcc8) SHA1(cbf5fb600e587b998f190a9e3fb398a51d8a5e87) )
ROM_END

} // anonymous namespace


//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY    FULLNAME  FLAGS
COMP( 1982, i7000, 0,      0,      i7000,   i7000, i7000_state, empty_init, "Itautec", "I-7000", MACHINE_NOT_WORKING)
