// license:GPL-2.0+
// copyright-holders:Lee Hammerton, Dirk Best
/***************************************************************************

    Miles Gordon Technology SAM Coupe

    Note:
    - Early ROMs are buggy. Version 1.0 requires the command CALL 229385
      (256k machine) or CALL 491529 (512k machine) to execute the bootstrap.

    TODO:
    - Better timing, emulate memory contention

    Hardware:
    - Z80 at 6 MHz
    - Custom ASIC handling the system and video
    - SAA1099 sound
    - XTAL X1: 24 MHz (system clock)
    - XTAL X2: 4.433619 MHz (RGB encoder)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/bankdev.h"
#include "machine/ram.h"
#include "sound/saa1099.h"
#include "sound/spkrdev.h"
#include "imagedev/cassette.h"
#include "bus/samcoupe/drive/drive.h"
#include "bus/samcoupe/expansion/expansion.h"
#include "bus/samcoupe/mouse/mouseport.h"
#include "formats/tzx_cas.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define SAM_ENABLE_JOYSTICK (0)

#define SAM_BLOCK           8

#define SAM_TOTAL_WIDTH     SAM_BLOCK*96
#define SAM_TOTAL_HEIGHT    312
#define SAM_SCREEN_WIDTH    SAM_BLOCK*64
#define SAM_SCREEN_HEIGHT   192
#define SAM_BORDER_LEFT     SAM_BLOCK*4
#define SAM_BORDER_RIGHT    SAM_BLOCK*4
#define SAM_BORDER_TOP      37
#define SAM_BORDER_BOTTOM   46


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class samcoupe_state :  public driver_device
{
public:
	samcoupe_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mem(*this, "mem"),
		m_io(*this, "io"),
		m_ram(*this, RAM_TAG),
		m_bios(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_speaker(*this, "speaker"),
		m_cassette(*this, "cassette"),
		m_mouse(*this, "mouseport"),
		m_keyboard(*this, "kbd_%u", 0U),
#if SAM_ENABLE_JOYSTICK
		m_joystick(*this, "joy_%u", 0U)
#endif
		m_drive(*this, "drive%u", 1),
		m_expansion(*this, "exp")
	{
	}

	void samcoupe(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_mem;
	required_device<address_map_bank_device> m_io;
	required_device<ram_device> m_ram;
	required_memory_region m_bios;
	required_device<screen_device> m_screen;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<samcoupe_mouse_port_device> m_mouse;
	required_ioport_array<9> m_keyboard;
#if SAM_ENABLE_JOYSTICK
	required_ioport_array<2> m_joystick;
#endif
	required_device_array<samcoupe_drive_port_device, 2> m_drive;
	required_device<samcoupe_expansion_device> m_expansion;

	enum
	{
		INT_LINE    = 0x01,
		INT_MOUSE   = 0x02,
		INT_MIDIIN  = 0x04,
		INT_FRAME   = 0x08,
		INT_MIDIOUT = 0x10
	};

	void postload();

	// video
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	bitmap_ind16 m_bitmap;

	emu_timer *m_video_update_timer;
	emu_timer *m_irq_off_timer;

	uint8_t m_lmpr, m_hmpr, m_vmpr; /* memory pages */
	uint8_t m_border;           /* border */
	uint8_t m_clut[16];         /* color lookup table, 16 entries */
	uint8_t m_line_int;         /* line interrupt */
	uint8_t m_status;           /* status register */

	/* attribute */
	uint8_t m_attribute;

	uint8_t *m_videoram;

	uint8_t pen_r(offs_t offset);
	void clut_w(offs_t offset, uint8_t data);
	uint8_t status_r(offs_t offset);
	void line_int_w(uint8_t data);
	uint8_t lmpr_r();
	void lmpr_w(uint8_t data);
	uint8_t hmpr_r();
	void hmpr_w(uint8_t data);
	uint8_t vmpr_r();
	void vmpr_w(uint8_t data);
	uint8_t midi_r();
	void midi_w(uint8_t data);
	uint8_t keyboard_r(offs_t offset);
	void border_w(uint8_t data);
	uint8_t attributes_r();

	void samcoupe_palette(palette_device &palette) const;
	INTERRUPT_GEN_MEMBER(samcoupe_frame_interrupt);
	TIMER_CALLBACK_MEMBER(irq_off);
	TIMER_CALLBACK_MEMBER(sam_video_update_callback);

	void draw_mode4_line(int y, int hpos);
	void draw_mode3_line(int y, int hpos);
	void draw_mode12_block(bitmap_ind16 &bitmap, int vpos, int hpos, uint8_t mask);
	void draw_mode2_line(int y, int hpos);
	void draw_mode1_line(int y, int hpos);

	void samcoupe_irq(uint8_t src);

	// memory
	uint8_t mem_r(offs_t offset);
	void mem_w(offs_t offset, uint8_t data);
	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);

	uint8_t mem_page_r(int page, offs_t offset);
	void mem_page_w(int page, offs_t offset, uint8_t data);

	uint8_t mem_block_a_r(offs_t offset);
	void mem_block_a_w(offs_t offset, uint8_t data);
	uint8_t mem_block_b_r(offs_t offset);
	void mem_block_b_w(offs_t offset, uint8_t data);
	uint8_t mem_block_c_r(offs_t offset);
	void mem_block_c_w(offs_t offset, uint8_t data);
	uint8_t mem_block_d_r(offs_t offset);
	void mem_block_d_w(offs_t offset, uint8_t data);

	void mem_map_base(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	void io_map_base(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	int m_pages; // total internal memory pages (16 or 32)
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void samcoupe_state::mem_map_base(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(samcoupe_state::mem_r), FUNC(samcoupe_state::mem_w));
}

void samcoupe_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).ram().rw(FUNC(samcoupe_state::mem_block_a_r), FUNC(samcoupe_state::mem_block_a_w));
	map(0x4000, 0x7fff).ram().rw(FUNC(samcoupe_state::mem_block_b_r), FUNC(samcoupe_state::mem_block_b_w));
	map(0x8000, 0xbfff).ram().rw(FUNC(samcoupe_state::mem_block_c_r), FUNC(samcoupe_state::mem_block_c_w));
	map(0xc000, 0xffff).ram().rw(FUNC(samcoupe_state::mem_block_d_r), FUNC(samcoupe_state::mem_block_d_w));
}

void samcoupe_state::io_map_base(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(samcoupe_state::io_r), FUNC(samcoupe_state::io_w));
}

void samcoupe_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00e0, 0x00e7).mirror(0xff00).rw(m_drive[0], FUNC(samcoupe_drive_port_device::read), FUNC(samcoupe_drive_port_device::write));
	map(0x00e8, 0x00ef).mirror(0xff00).noprw(); // "print" - handled by expansion devices
	map(0x00f0, 0x00f7).mirror(0xff00).rw(m_drive[1], FUNC(samcoupe_drive_port_device::read), FUNC(samcoupe_drive_port_device::write));
	map(0x00f8, 0x00f8).select(0xff00).rw(FUNC(samcoupe_state::pen_r), FUNC(samcoupe_state::clut_w));
	map(0x00f9, 0x00f9).select(0xff00).rw(FUNC(samcoupe_state::status_r), FUNC(samcoupe_state::line_int_w));
	map(0x00fa, 0x00fa).select(0xff00).rw(FUNC(samcoupe_state::lmpr_r), FUNC(samcoupe_state::lmpr_w));
	map(0x00fb, 0x00fb).select(0xff00).rw(FUNC(samcoupe_state::hmpr_r), FUNC(samcoupe_state::hmpr_w));
	map(0x00fc, 0x00fc).select(0xff00).rw(FUNC(samcoupe_state::vmpr_r), FUNC(samcoupe_state::vmpr_w));
	map(0x00fd, 0x00fd).select(0xff00).rw(FUNC(samcoupe_state::midi_r), FUNC(samcoupe_state::midi_w));
	map(0x00fe, 0x00fe).select(0xff00).rw(FUNC(samcoupe_state::keyboard_r), FUNC(samcoupe_state::border_w));
	map(0x00ff, 0x00ff).select(0xff00).r(FUNC(samcoupe_state::attributes_r));
	map(0x00ff, 0x00ff).mirror(0xfe00).w("saa1099", FUNC(saa1099_device::data_w));
	map(0x01ff, 0x01ff).mirror(0xfe00).w("saa1099", FUNC(saa1099_device::control_w));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( samcoupe )
	PORT_START("kbd_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)     PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)     PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)     PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)     PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("kbd_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)     PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)     PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)     PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)     PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)     PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR('}')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(F6))

	PORT_START("kbd_2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)     PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR('<')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)     PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR('>')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)     PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)     PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR('[')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)     PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(']')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(F9))

	PORT_START("kbd_3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)        PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)        PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)        PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)        PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)        PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)      PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)      PORT_CHAR('\t')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("kbd_4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)         PORT_CHAR('0') PORT_CHAR('~')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)         PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)         PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)         PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('-') PORT_CHAR('/')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)    PORT_CHAR('+') PORT_CHAR('*')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DELETE") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)

	PORT_START("kbd_5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('=') PORT_CHAR('_')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('"') PORT_CHAR(0xA9)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(F10))

	PORT_START("kbd_6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)     PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(0xA3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)     PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)     PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)     PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR('^')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EDIT") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START("kbd_7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SYMBOL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)     PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)     PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)     PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)  PORT_CHAR('.')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INV") PORT_CODE(KEYCODE_SLASH) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CHAR('\\')

	PORT_START("kbd_8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)     PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)   PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)   PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)  PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

#if SAM_ENABLE_JOYSTICK
	/* Sam Coupe has single 9-pin ATARI-compatible connector but supports 2 joysticks via a splitter,
	   this works by using a different ground for each stick (pin 8: stick 1 gnd, pin 9: stick 2 gnd.)
	   Joysticks overlay number keys 6-0 for the stick 1 and 1-5 for stick 2 (same scheme as ZX Spectrum) */

	PORT_START("joy_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_8WAY PORT_PLAYER(1)

	PORT_START("joy_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(2)
#endif
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void samcoupe_state::samcoupe_palette(palette_device &palette) const
{
	for (int i = 0; i < 128; i++)
	{
		uint8_t const b = bitswap<3>(i, 4, 0, 3); //  blue: bits 403
		uint8_t const r = bitswap<3>(i, 5, 1, 3); //   red: bits 513
		uint8_t const g = bitswap<3>(i, 6, 2, 3); // green: bits 623

		palette.set_pen_color(i, pal3bit(r), pal3bit(g), pal3bit(b));
	}
}

void samcoupe_state::video_start()
{
	m_screen->register_screen_bitmap(m_bitmap);
}

uint32_t samcoupe_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

void samcoupe_state::draw_mode4_line(int y, int hpos)
{
	/* get start address */
	uint8_t const *vram = m_videoram + ((y - SAM_BORDER_TOP) * 128) + ((hpos - SAM_BORDER_LEFT) / 4);

	for (int i = 0; i < (SAM_BLOCK*2)/4; i++)
	{
		/* draw 2 pixels (doublewidth) */
		m_bitmap.pix(y, hpos + i * 4 + 0) = m_clut[(*vram >> 4) & 0x0f];
		m_bitmap.pix(y, hpos + i * 4 + 1) = m_clut[(*vram >> 4) & 0x0f];
		m_bitmap.pix(y, hpos + i * 4 + 2) = m_clut[(*vram >> 0) & 0x0f];
		m_bitmap.pix(y, hpos + i * 4 + 3) = m_clut[(*vram >> 0) & 0x0f];

		/* move to next address */
		vram++;

		/* attribute register contains the third displayed byte */
		if (i == 2)
			m_attribute = *vram;
	}
}

void samcoupe_state::draw_mode3_line(int y, int hpos)
{
	/* get start address */
	uint8_t const *vram = m_videoram + ((y - SAM_BORDER_TOP) * 128) + ((hpos - SAM_BORDER_LEFT) / 4);

	for (int i = 0; i < (SAM_BLOCK*2)/4; i++)
	{
		/* draw 4 pixels */
		m_bitmap.pix(y, hpos + i * 4 + 0) = m_clut[(*vram >> 6) & 0x03];
		m_bitmap.pix(y, hpos + i * 4 + 1) = m_clut[(*vram >> 4) & 0x03];
		m_bitmap.pix(y, hpos + i * 4 + 2) = m_clut[(*vram >> 2) & 0x03];
		m_bitmap.pix(y, hpos + i * 4 + 3) = m_clut[(*vram >> 0) & 0x03];

		/* move to next address */
		vram++;

		/* attribute register contains the third displayed byte */
		if (i == 2)
			m_attribute = *vram;
	}
}

void samcoupe_state::draw_mode12_block(bitmap_ind16 &bitmap, int vpos, int hpos, uint8_t mask)
{
	/* extract colors from attribute */
	uint8_t ink = m_clut[((m_attribute >> 3) & 0x08) | (m_attribute & 0x07)];
	uint8_t pap = m_clut[(m_attribute >> 3) & 0x0f];

	/* draw block of 8 pixels (doubled to 16) */
	for (int i = 0; i < SAM_BLOCK; i++)
	{
		bitmap.pix(vpos, hpos + i*2 + 0) = BIT(mask, 7 - i) ? ink : pap;
		bitmap.pix(vpos, hpos + i*2 + 1) = BIT(mask, 7 - i) ? ink : pap;
	}
}

void samcoupe_state::draw_mode2_line(int y, int hpos)
{
	int cell = (y - SAM_BORDER_TOP) * 32 + (hpos - SAM_BORDER_LEFT) / SAM_BLOCK / 2;

	uint8_t mask = m_videoram[cell];
	m_attribute = m_videoram[cell + 0x2000];

	draw_mode12_block(m_bitmap, y, hpos, mask);
}

void samcoupe_state::draw_mode1_line(int y, int hpos)
{
	uint8_t mask = m_videoram[((((y - SAM_BORDER_TOP) & 0xc0) << 5) | (((y - SAM_BORDER_TOP) & 0x07) << 8) | (((y - SAM_BORDER_TOP) & 0x38) << 2)) + (hpos - SAM_BORDER_LEFT) / SAM_BLOCK / 2];
	m_attribute = m_videoram[32*192 + (((y - SAM_BORDER_TOP) & 0xf8) << 2) + (hpos - SAM_BORDER_LEFT) / SAM_BLOCK / 2];

	draw_mode12_block(m_bitmap, y, hpos, mask);
}

TIMER_CALLBACK_MEMBER(samcoupe_state::sam_video_update_callback)
{
	int vpos = m_screen->vpos();
	int hpos = m_screen->hpos();

	int next_vpos = vpos;
	int next_hpos = hpos + SAM_BLOCK*2;

	/* next scanline? */
	if (next_hpos >= SAM_BORDER_LEFT + SAM_SCREEN_WIDTH + SAM_BORDER_RIGHT)
	{
		next_vpos = (vpos + 1) % (SAM_BORDER_TOP + SAM_SCREEN_HEIGHT + SAM_BORDER_BOTTOM);
		next_hpos = 0;
	}

	/* display disabled? (only in mode 3 or 4) */
	if (BIT(m_vmpr, 6) && BIT(m_border, 7))
	{
		m_bitmap.plot_box(hpos, vpos, SAM_BLOCK*2, 1, 0);
	}
	else
	{
		/* border area? */
		if (vpos < SAM_BORDER_TOP || vpos >= SAM_BORDER_TOP + SAM_SCREEN_HEIGHT || hpos < SAM_BORDER_LEFT || hpos >= SAM_BORDER_LEFT + SAM_SCREEN_WIDTH)
		{
			m_attribute = 0xff;
			m_bitmap.plot_box(hpos, vpos, SAM_BLOCK*2, 1, m_clut[(m_border & 0x20) >> 2 | (m_border & 0x07)]);
		}
		else
		{
			/* main screen area */
			switch ((m_vmpr & 0x60) >> 5)
			{
			case 0: draw_mode1_line(vpos, hpos); break;
			case 1: draw_mode2_line(vpos, hpos); break;
			case 2: draw_mode3_line(vpos, hpos); break;
			case 3: draw_mode4_line(vpos, hpos); break;
			}
		}
	}

	/* do we need to trigger the scanline interrupt (interrupt happens at the start of the right border before the specified line)? */
	if (m_line_int < SAM_SCREEN_HEIGHT && hpos == SAM_BORDER_LEFT + SAM_SCREEN_WIDTH && vpos == (m_line_int + SAM_BORDER_TOP - 1))
		samcoupe_irq(INT_LINE);

	/* schedule next update */
	m_video_update_timer->adjust(m_screen->time_until_pos(next_vpos, next_hpos));
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

TIMER_CALLBACK_MEMBER(samcoupe_state::irq_off)
{
	/* adjust STATUS register */
	m_status |= param;

	/* clear interrupt */
	if ((m_status & 0x1f) == 0x1f)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

void samcoupe_state::samcoupe_irq(uint8_t src)
{
	/* assert irq and a timer to set it off again */
	m_maincpu->set_input_line(0, ASSERT_LINE);
	m_irq_off_timer->adjust(attotime::from_usec(20), src);

	/* adjust STATUS register */
	m_status &= ~src;
}

INTERRUPT_GEN_MEMBER(samcoupe_state::samcoupe_frame_interrupt)
{
	/* signal frame interrupt */
	samcoupe_irq(INT_FRAME);
}

void samcoupe_state::postload()
{
	// restore videoram pointer
	vmpr_w(m_vmpr);
}

void samcoupe_state::machine_start()
{
	// make sure the ram device is already running
	if (!m_ram->started())
		throw device_missing_dependencies();

	m_pages = m_ram->size() / 0x4000; // 16 or 32

	/* schedule our video updates */
	m_video_update_timer = timer_alloc(FUNC(samcoupe_state::sam_video_update_callback), this);
	m_video_update_timer->adjust(m_screen->time_until_pos(0, 0));

	// register for save states
	save_item(NAME(m_lmpr));
	save_item(NAME(m_hmpr));
	save_item(NAME(m_vmpr));
	save_item(NAME(m_border));
	save_item(NAME(m_clut), 16);
	save_item(NAME(m_line_int));
	save_item(NAME(m_status));
	save_item(NAME(m_attribute));

	machine().save().register_postload(save_prepost_delegate(FUNC(samcoupe_state::postload), this));

	// allocate machine timers
	m_irq_off_timer = timer_alloc(FUNC(samcoupe_state::irq_off), this);
	m_irq_off_timer->adjust(attotime::never);
}

void samcoupe_state::machine_reset()
{
	m_lmpr = 0x0f;      // ROM0 paged in, ROM1 paged out, RAM Banks
	m_hmpr = 0x01;
	vmpr_w(0x81);
	m_line_int = 0xff;  // line interrupts disabled
	m_status = 0x1f;    // no interrupts active
}


//**************************************************************************
//  ASIC
//**************************************************************************

uint8_t samcoupe_state::pen_r(offs_t offset)
{
	uint8_t data;

	if (offset & 0x100)
	{
		// HPEN
		int vpos = m_screen->vpos();

		// return the current screen line or 192 for the border area
		if (vpos < SAM_BORDER_TOP || vpos >= SAM_BORDER_TOP + SAM_SCREEN_HEIGHT)
			data = 192;
		else
			data = vpos - SAM_BORDER_TOP;
	}
	else
	{
		// LPEN

		// 765432--  screen horizontal position
		// ------1-  midi status (currently transferring)
		// -------0  border clut bit 0

		data = (m_screen->hpos() & 0xfc) | (0 << 1) | (m_border & 1);
	}

	return data;
}

void samcoupe_state::clut_w(offs_t offset, uint8_t data)
{
	m_clut[(offset >> 8) & 0x0f] = data & 0x7f;
}

uint8_t samcoupe_state::status_r(offs_t offset)
{
	// 765-----  keyboard matrix k8 to k6
	// ---4----  midi out interrupt
	// ----3---  frame interrupt
	// -----2--  midi in interrupt
	// ------1-  mouse interrupt
	// -------0  line interrupt

	uint8_t data = 0xe0;

	for (int i = 0; i < 8; i++)
		if (BIT(offset, i + 8) == 0)
			data &= m_keyboard[i]->read() & 0xe0;

	data |= m_status;

	return data;
}

void samcoupe_state::line_int_w(uint8_t data)
{
	m_line_int = data;
}

uint8_t samcoupe_state::lmpr_r()
{
	return m_lmpr;
}

void samcoupe_state::lmpr_w(uint8_t data)
{
	m_lmpr = data;
}

uint8_t samcoupe_state::hmpr_r()
{
	return m_hmpr;
}

void samcoupe_state::hmpr_w(uint8_t data)
{
	m_hmpr = data;
}

uint8_t samcoupe_state::vmpr_r()
{
	return m_vmpr;
}

void samcoupe_state::vmpr_w(uint8_t data)
{
	m_vmpr = data;

	if (BIT(m_vmpr, 6))   /* if bit set in 2 bank screen mode */
		m_videoram = &m_ram->pointer()[(m_vmpr & 0x1e) << 14];
	else
		m_videoram = &m_ram->pointer()[(m_vmpr & 0x1f) << 14];
}

uint8_t samcoupe_state::midi_r()
{
	logerror("Read from MIDI port\n");
	return 0xff;
}

void samcoupe_state::midi_w(uint8_t data)
{
	logerror("Write to MIDI port: 0x%02x\n", data);
}

uint8_t samcoupe_state::keyboard_r(offs_t offset)
{
	// 7-------  status bit external memory
	// -6------  cassette input
	// --5-----  light pen input
	// ---43210  keyboard matrix k5 to k1, mouse

	uint8_t data = 0x1f;

	for (int i = 0; i < 8; i++)
		if (BIT(offset, i + 8) == 0)
			data &= m_keyboard[i]->read() & 0x1f;

#if SAM_ENABLE_JOYSTICK
	if (BIT(offset, 12) == 0) data &= m_joystick[0]->read() & 0x1f;
	if (BIT(offset, 11) == 0) data &= m_joystick[1]->read() & 0x1f;
#endif

	if (offset == 0xff00)
	{
		data &= m_keyboard[8]->read() & 0x1f;
		data &= m_mouse->read() & 0x1f;
	}

	data |= 1 << 5;
	data |= (m_cassette->input() > 0 ? 1 : 0) << 6;
	data |= 1 << 7;

	return data;
}

void samcoupe_state::border_w(uint8_t data)
{
	// 7-------  screen off in mode 3/4
	// -6------  midi through
	// --5--210  clut address for border color
	// ---4----  beeper
	// ----3---  cassette output

	m_border = data;

	m_cassette->output(BIT(data, 3) ? -1.0 : +1.0);
	m_speaker->level_w(BIT(data, 4));
}

uint8_t samcoupe_state::attributes_r()
{
	return m_attribute;
}


//**************************************************************************
//  MEMORY
//**************************************************************************

//-------------------------------------------------
//  mem_r/w - program space accesses
//-------------------------------------------------

uint8_t samcoupe_state::mem_r(offs_t offset)
{
	uint8_t data = 0xff;

	int xmem = BIT(m_hmpr, 7) && BIT(offset, 15) ? 1 : 0;

	m_expansion->xmem_w(xmem);

	data &= m_expansion->mreq_r(offset);

	if (xmem == 0)
		data &= m_mem->read8(offset);

	return data;
}

void samcoupe_state::mem_w(offs_t offset, uint8_t data)
{
	int xmem = BIT(m_hmpr, 7) && BIT(offset, 15) ? 1 : 0;

	m_expansion->xmem_w(xmem);

	m_expansion->mreq_w(offset, data);

	if (xmem == 0)
		m_mem->write8(offset, data);
}

//-------------------------------------------------
//  io_r/w - i/o space accesses
//-------------------------------------------------

uint8_t samcoupe_state::io_r(offs_t offset)
{
	uint8_t data = 0xff;

	m_expansion->print_w((offset & 0xf8) == 0xe8); // 0xe8 to 0xef

	data &= m_expansion->iorq_r(offset);
	data &= m_io->read8(offset);

	return data;
}

void samcoupe_state::io_w(offs_t offset, uint8_t data)
{
	m_expansion->print_w((offset & 0xf8) == 0xe8);

	m_expansion->iorq_w(offset, data);
	m_io->write8(offset, data);
}

//-------------------------------------------------
//  mem_page_r/w - read/write memory pages
//-------------------------------------------------

uint8_t samcoupe_state::mem_page_r(int page, offs_t offset)
{
	if (page < m_pages)
		return m_ram->pointer()[(page << 14) | offset];
	else
		return 0xff;
}

void samcoupe_state::mem_page_w(int page, offs_t offset, uint8_t data)
{
	if (page < m_pages)
		m_ram->pointer()[(page << 14) | offset] = data;
}

//-------------------------------------------------
//  mem_block_a/b/c/d_r/w - read/write memory blocks
//-------------------------------------------------

uint8_t samcoupe_state::mem_block_a_r(offs_t offset)
{
	int page = m_lmpr & 0x1f;

	// ram enabled?
	if (BIT(m_lmpr, 5))
		return mem_page_r(page, offset);
	else
		return m_bios->base()[offset];
}

void samcoupe_state::mem_block_a_w(offs_t offset, uint8_t data)
{
	int page = m_lmpr & 0x1f;

	// not write protected?
	if (BIT(m_lmpr, 7) == 0)
		mem_page_w(page, offset, data);
}

uint8_t samcoupe_state::mem_block_b_r(offs_t offset)
{
	int page = ((m_lmpr & 0x1f) + 1) & 0x1f;
	return mem_page_r(page, offset);
}

void samcoupe_state::mem_block_b_w(offs_t offset, uint8_t data)
{
	int page = ((m_lmpr & 0x1f) + 1) & 0x1f;
	mem_page_w(page, offset, data);
}

uint8_t samcoupe_state::mem_block_c_r(offs_t offset)
{
	int page = m_hmpr & 0x1f;
	return mem_page_r(page, offset);
}

void samcoupe_state::mem_block_c_w(offs_t offset, uint8_t data)
{
	int page = m_hmpr & 0x1f;
	mem_page_w(page, offset, data);
}

uint8_t samcoupe_state::mem_block_d_r(offs_t offset)
{
	int page = ((m_hmpr & 0x1f) + 1) & 0x1f;

	// ram enabled?
	if (BIT(m_lmpr, 6) == 0)
		return mem_page_r(page, offset);
	else
		return m_bios->base()[0x4000 + offset];
}

void samcoupe_state::mem_block_d_w(offs_t offset, uint8_t data)
{
	int page = ((m_hmpr & 0x1f) + 1) & 0x1f;
	mem_page_w(page, offset, data);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void samcoupe_state::samcoupe(machine_config &config)
{
	Z80(config, m_maincpu, 24_MHz_XTAL / 4); // 6 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &samcoupe_state::mem_map_base);
	m_maincpu->set_addrmap(AS_IO, &samcoupe_state::io_map_base);
	m_maincpu->set_vblank_int("screen", FUNC(samcoupe_state::samcoupe_frame_interrupt));

	ADDRESS_MAP_BANK(config, m_mem);
	m_mem->set_addrmap(AS_PROGRAM, &samcoupe_state::mem_map);
	m_mem->set_data_width(8);
	m_mem->set_addr_width(16);

	ADDRESS_MAP_BANK(config, m_io);
	m_io->set_addrmap(AS_PROGRAM, &samcoupe_state::io_map);
	m_io->set_data_width(8);
	m_io->set_addr_width(16);

	RAM(config, RAM_TAG).set_default_size("512K").set_extra_options("256K");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(24_MHz_XTAL / 2, SAM_TOTAL_WIDTH,  0, SAM_BORDER_LEFT + SAM_SCREEN_WIDTH + SAM_BORDER_RIGHT,
									   SAM_TOTAL_HEIGHT, 0, SAM_BORDER_TOP + SAM_SCREEN_HEIGHT + SAM_BORDER_BOTTOM);
	m_screen->set_screen_update(FUNC(samcoupe_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(samcoupe_state::samcoupe_palette), 128);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
	SAA1099(config, "saa1099", 24_MHz_XTAL / 3).add_route(ALL_OUTPUTS, "mono", 0.50); // 8 MHz

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(tzx_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("samcoupe_cass");
	SOFTWARE_LIST(config, "cass_list").set_original("samcoupe_cass");

	// expansion ports
	SAMCOUPE_MOUSE_PORT(config, m_mouse, samcoupe_mouse_modules);

	SAMCOUPE_DRIVE_PORT(config, m_drive[0], samcoupe_drive_modules, "floppy");
	SAMCOUPE_DRIVE_PORT(config, m_drive[1], samcoupe_drive_modules, "floppy");

	SOFTWARE_LIST(config, "flop_list").set_original("samcoupe_flop");

	SAMCOUPE_EXPANSION(config, m_expansion, samcoupe_expansion_modules);
}


//**************************************************************************
//  ROM DEFINTIONS
//**************************************************************************

/*
    The bios is actually 32K. This is the combined version of the two old 16K MESS roms.
    It does match the 3.0 one the most, but the first half differs in one byte
    and in the second half, the case of the "plc" in the company string differs.
*/
ROM_START( samcoupe )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0,  "31",  "v3.1" )
	ROMX_LOAD( "rom31.z5",  0x0000, 0x8000, CRC(0b7e3585) SHA1(c86601633fb61a8c517f7657aad9af4e6870f2ee), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1,  "30",  "v3.0" )
	ROMX_LOAD( "rom30.z5",  0x0000, 0x8000, CRC(e535c25d) SHA1(d390f0be420dfb12b1e54a4f528b5055d7d97e2a), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2,  "25",  "v2.5" )
	ROMX_LOAD( "rom25.z5",  0x0000, 0x8000, CRC(ddadd358) SHA1(a25ed85a0f1134ac3a481a3225f24a8bd3a790cf), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3,  "24",  "v2.4" )
	ROMX_LOAD( "rom24.z5",  0x0000, 0x8000, CRC(bb23fee4) SHA1(10cd911ba237dd2cf0c2637be1ad6745b7cc89b9), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4,  "21",  "v2.1" )
	ROMX_LOAD( "rom21.z5",  0x0000, 0x8000, CRC(f6804b46) SHA1(11dcac5fdea782cdac03b4d0d7ac25d88547eefe), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5,  "20",  "v2.0" )
	ROMX_LOAD( "rom20.z5",  0x0000, 0x8000, CRC(eaf32054) SHA1(41736323f0236649f2d5fe111f900def8db93a13), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6,  "181", "v1.81" )
	ROMX_LOAD( "rom181.z5", 0x0000, 0x8000, CRC(d25e1de1) SHA1(cb0fa79e4d5f7df0b57ede08ea7ecc9ae152f534), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 7,  "18",  "v1.8" )
	ROMX_LOAD( "rom18.z5",  0x0000, 0x8000, CRC(f626063f) SHA1(485e7d9e9a4f8a70c0f93cd6e69ff12269438829), ROM_BIOS(7) )
	ROM_SYSTEM_BIOS( 8,  "14",  "v1.4" )
	ROMX_LOAD( "rom14.z5",  0x0000, 0x8000, CRC(08799596) SHA1(b4e596051f2748dee9481ea4af7d15ccddc1e1b5), ROM_BIOS(8) )
	ROM_SYSTEM_BIOS( 9,  "13",  "v1.3" )
	ROMX_LOAD( "rom13.z5",  0x0000, 0x8000, CRC(2093768c) SHA1(af8d348fd080b18a4cbe9ed69d254be7be330146), ROM_BIOS(9) )
	ROM_SYSTEM_BIOS( 10, "12",  "v1.2" )
	ROMX_LOAD( "rom12.z5",  0x0000, 0x8000, CRC(7fe37dd8) SHA1(9339a0c1f72e8512c6f32dec15ab7d6c3bb04151), ROM_BIOS(10) )
	ROM_SYSTEM_BIOS( 11, "10",  "v1.0" )
	ROMX_LOAD( "rom10.z5",  0x0000, 0x8000, CRC(3659d31f) SHA1(d3de7bb74e04d5b4dc7477f70de54d540b1bcc07), ROM_BIOS(11) )
	ROM_SYSTEM_BIOS( 12, "04",  "v0.4" )
	ROMX_LOAD( "rom04.z5",  0x0000, 0x8000, CRC(f439e84e) SHA1(8bc457a5c764b0bb0aa7008c57f28c30248fc6a4), ROM_BIOS(12) )
	ROM_SYSTEM_BIOS( 13, "01",  "v0.1" )
	ROMX_LOAD( "rom01.z5",  0x0000, 0x8000, CRC(c04acfdf) SHA1(8976ed005c14905eec1215f0a5c28aa686a7dda2), ROM_BIOS(13) )
	ROM_SYSTEM_BIOS( 14, "atom",  "ATOM HDD Interface Auto Boot" )
	ROMX_LOAD( "atom.z5",   0x0000, 0x8000, CRC(dec75f58) SHA1(cd342579f066c0863e4f769c2e6757085e21b0a1), ROM_BIOS(14) )
ROM_END

} // anonymous namespace


/***************************************************************************
    GAME DRIVERS
***************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY                        FULLNAME     FLAGS
COMP( 1989, samcoupe, 0,      0,      samcoupe, samcoupe, samcoupe_state, empty_init, "Miles Gordon Technology plc", u8"SAM Coupé", MACHINE_SUPPORTS_SAVE )
