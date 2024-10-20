// license:GPL-2.0+
// copyright-holders: Felipe Sanches
/***************************************************************************

    Itautec I7000

    driver by Felipe C. da S. Sanches <juca@members.fsf.org>
    with tech info provided by Alexandre Souza (a.k.a. Tabajara).

    The portuguese Wikipedia article available at
    http://pt.wikipedia.org/wiki/Itautec_I-7000
    also provides a technical overview of this machine:

    The I-7000 was the first computer manufactured by Itautec
    (http://www.itautec.com.br/pt-br/produtos). It was originally an 8 bit CP/M
    computer that became an IBM PC-XT clone in later hardware revisions which
    took the "I-7000 PC-XT" name.

    * Released in 1982
    * Operating System: SIM/M / BASIC
    * CPU: National NSC800 D-4 at 4,00 MHz
    * Memory: 64KB to 128KB
    * keyboards: 80 keys (with a reduced numerical keypad and function keys)
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
     - Up to 4 external floppy drives: 8" (FD/DD, 1,1MB) or 5" 1/4
     - Up to 1 external 10 MB hard-drive

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
#include "tilemap.h"


namespace {

class i7000_state : public driver_device
{
public:
	i7000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_card(*this, "cardslot")
		, m_gfxdecode(*this, "gfxdecode")
		, m_videoram(*this, "videoram")
		, m_kbd(*this, "X%u", 0U)
	{ }

	void i7000(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_card;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_videoram;
	required_ioport_array<8> m_kbd;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint8_t m_row = 0;
	tilemap_t *m_bg_tilemap = nullptr;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_addr);
	void i7000_palette(palette_device &palette) const;
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
	for (int i=0; i<40*25; i++){
		m_bg_tilemap->mark_tile_dirty(i);
	}

	return m_kbd[m_row & 7]->read();
}

/* Input ports */
static INPUT_PORTS_START( i7000 )
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

void i7000_state::i7000_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x33, 0x33, 0x33));
	palette.set_pen_color(1, rgb_t(0xBB, 0xBB, 0xBB));
}

/*FIXME: we still need to figure out the proper memory map
         for the maincpu and where the cartridge slot maps to. */
void i7000_state::i7000_mem(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("boot", 0);
	map(0x2000, 0x2fff).ram().share("videoram");
	map(0x4000, 0xffff).ram();
//  map(0x4000, 0xbfff).rom().region("cardslot", 0);
}

void i7000_state::i7000_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
//  map(0x06, 0x06).w(FUNC(i7000_state::i7000_io_?_w));
//  map(0x08, 0x09).w(FUNC(i7000_state::i7000_io_?_w)); //printer perhaps?
//  map(0x0c, 0x0c).w(FUNC(i7000_state::i7000_io_?_w)); //0x0C and 0x10 may be related to mem page swapping. (self-test "4. PAG")
//  map(0x10, 0x10).w(FUNC(i7000_state::i7000_io_?_w));
//  map(0x14, 0x15).w(FUNC(i7000_state::i7000_io_?_w));

	map(0x18, 0x1b).rw("pit8253", FUNC(pit8253_device::read), FUNC(pit8253_device::write));

//  map(0x1c, 0x1c).w(FUNC(i7000_state::i7000_io_printer_data_w)); //ASCII data
	map(0x1d, 0x1d).portr("DSW");
//  map(0x1e, 0x1e).rw(FUNC(i7000_state::i7000_io_printer_status_r), FUNC(i7000_state::i7000_io_?_w));
//  map(0x1f, 0x1f).w(FUNC(i7000_state::i7000_io_printer_strobe_w)); //self-test routine writes 0x08 and 0x09 (it seems that bit 0 is the strobe and bit 3 is an enable signal)
//  map(0x20, 0x21).rw(FUNC(i7000_state::i7000_io_keyboard_r), FUNC(i7000_state::i7000_io_keyboard_w));

	map(0x20, 0x21).rw("i8279", FUNC(i8279_device::read), FUNC(i8279_device::write));

//  map(0x24, 0x24).r(FUNC(i7000_state::i7000_io_?_r));
//  map(0x25, 0x25).w(FUNC(i7000_state::i7000_io_?_w));

//  map(0x28, 0x2d).rw(FUNC(i7000_state::i7000_io_joystick_r), FUNC(i7000_state::i7000_io_joystick_w));

//  map(0x3b, 0x3b).w(FUNC(i7000_state::i7000_io_?_w));
//  map(0x66, 0x67).w(FUNC(i7000_state::i7000_io_?_w));
//  map(0xbb, 0xbb).w(FUNC(i7000_state::i7000_io_?_w)); //may be related to page-swapping...
}

DEVICE_IMAGE_LOAD_MEMBER(i7000_state::card_load)
{
	uint32_t const size = m_card->common_get_size("rom");

	m_card->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_BIG);
	m_card->common_load_rom(m_card->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

static const gfx_layout i7000_charlayout =
{
	8, 8,                   // 8 x 8 characters
	256,                    // 256 characters
	1,                      // 1 bits per pixel
	{ 0 },                  // no bitplanes
	// x offsets
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	// y offsets
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                     // every char takes 8 bytes
};

static GFXDECODE_START( gfx_i7000 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, i7000_charlayout, 0, 8 )
GFXDECODE_END

/****************************
* Video/Character functions *
****************************/

TILE_GET_INFO_MEMBER(i7000_state::get_bg_tile_info)
{
	tileinfo.set(0, /*code:*/ m_videoram[tile_index], /*color:*/ 0, 0);
}

void i7000_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(i7000_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 40, 25);
}

uint32_t i7000_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/* ROCKWELL 6545 - Transparent Memory Addressing */
MC6845_ON_UPDATE_ADDR_CHANGED(i7000_state::crtc_addr)
{
	/* What is this mandatory function meant to do ? */
}


void i7000_state::i7000(machine_config &config)
{
	/* basic machine hardware */
	NSC800(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &i7000_state::i7000_mem);
	m_maincpu->set_addrmap(AS_IO, &i7000_state::i7000_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(320, 200); /* 40x25 8x8 chars */
	screen.set_visarea(0, 320-1, 0, 200-1);
	screen.set_screen_update(FUNC(i7000_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_i7000);
	PALETTE(config, "palette", FUNC(i7000_state::i7000_palette), 2);

	r6545_1_device &crtc(R6545_1(config, "crtc", XTAL(20'000'000))); /* (?) */
	crtc.set_screen("screen");
	crtc.set_show_border_area(true);
	crtc.set_char_width(8);
	crtc.set_on_update_addr_change_callback(FUNC(i7000_state::crtc_addr));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	/* Programmable timer */
	pit8253_device &pit8253(PIT8253(config, "pit8253", 0));
//  pit8253.set_clk<0>(XTAL(4'000'000) / 2); /* TODO: verify on PCB */
//  pit8253.out_handler<0>().set(FUNC(i7000_state::i7000_pit_out0));
//  pit8253.set_clk<1>(XTAL(4'000'000) / 2); /* TODO: verify on PCB */
//  pit8253.out_handler<1>().set(FUNC(i7000_state::i7000_pit_out1));
	pit8253.set_clk<2>(XTAL(4'000'000) / 2); /* TODO: verify on PCB */
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

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "i7000_chargen.rom", 0x0000, 0x0800, CRC(fb7383e9) SHA1(71a6561bb9ff3cbf74711fa7ab445f9b43f15626) )
		/*
		The character generator ROM originally dumped had
		some corrupt data that was manually fixed:

		ROM address | Originally dumped value | Manually fixed value | Comment
		    0x06A2 |                    0xF7 | 0xFE                 | This is a fix to the upper right portion of a 2x2 tile image of a big filled circle
		    0x06A3 |                    0xF7 | 0xFE                 | This is another fix to the same tile (character value: 0xD4)

		Also, characters 0x05, 0x06, 0x07 and 0x08
		as well as lowercase 'x' (0x78), uppercase 'Y' (0x59)
		may contain corrupt data, but we can't be sure,
		unless we find another Itautec I7000 computer and
		redump it's ROMs to double-check.
	*/

	ROM_REGION( 0x1000, "drive", 0 )
	ROM_LOAD( "i7000_drive_ci01.rom", 0x0000, 0x1000, CRC(d8d6e5c1) SHA1(93e7db42fbfaa8243973321c7fc8c51ed80780be) )

	ROM_REGION( 0x1000, "telex", 0 )
	ROM_LOAD( "i7000_telex_ci09.rom", 0x0000, 0x1000, CRC(c1c8fcc8) SHA1(cbf5fb600e587b998f190a9e3fb398a51d8a5e87) )
ROM_END

} // anonymous namespace


//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY    FULLNAME  FLAGS
COMP( 1982, i7000, 0,      0,      i7000,   i7000, i7000_state, empty_init, "Itautec", "I-7000", MACHINE_NOT_WORKING)
