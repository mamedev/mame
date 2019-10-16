// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Toshiba Pasopia

    Most games can be loaded from cassette, but since the graphics don't
    work they are unplayable, or blank screen.
    Sound uses the beeper, and works. Try SOUND a,b where a=1-82, b=1-255.

    TODO:
    - machine emulation needs merging with Pasopia 7 (video emulation is
      completely different tho)
    - screen resolution switching
    - Centronics printer interface
    - RS-232C serial interface
    - FDC and other I/O expansions
    - fix keyboard (hack is currently in use)
    - colours
    - graphics


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "video/mc6845.h"
#include "imagedev/cassette.h"
#include "sound/spkrdev.h"
#include "speaker.h"
#include "emupal.h"
#include "screen.h"


class pasopia_state : public driver_device
{
public:
	pasopia_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_p_vram(*this, "vram")
		, m_ppi0(*this, "ppi0")
		, m_ppi1(*this, "ppi1")
		, m_ppi2(*this, "ppi2")
		, m_ctc(*this, "ctc")
		, m_pio(*this, "pio")
		, m_crtc(*this, "crtc")
		, m_palette(*this, "palette")
		, m_io_keyboard(*this, "KEY.%d", 0)
		, m_cass(*this, "cassette")
		, m_speaker(*this, "speaker")
	{ }

	void pasopia(machine_config &config);

	void init_pasopia();

private:
	DECLARE_WRITE8_MEMBER(pasopia_ctrl_w);
	DECLARE_WRITE8_MEMBER(vram_addr_lo_w);
	DECLARE_WRITE8_MEMBER(vram_latch_w);
	DECLARE_READ8_MEMBER(vram_latch_r);
	DECLARE_READ8_MEMBER(portb_1_r);
	DECLARE_READ8_MEMBER(portb_2_r);
	DECLARE_WRITE8_MEMBER(porta_2_w);
	DECLARE_WRITE8_MEMBER(vram_addr_hi_w);
	DECLARE_WRITE8_MEMBER(screen_mode_w);
	DECLARE_READ8_MEMBER(rombank_r);
	DECLARE_READ8_MEMBER(keyb_r);
	DECLARE_WRITE8_MEMBER(mux_w);
	DECLARE_WRITE_LINE_MEMBER(speaker_w);
	MC6845_UPDATE_ROW(crtc_update_row);
	TIMER_CALLBACK_MEMBER(pio_timer);

	void pasopia_io(address_map &map);
	void pasopia_map(address_map &map);

	uint8_t m_hblank;
	uint16_t m_vram_addr;
	uint8_t m_vram_latch;
	uint8_t m_attr_latch;
	uint8_t m_gfx_mode;
	uint8_t m_mux_data;
	u8 m_porta_2;
	bool m_video_wl;
	bool m_ram_bank;
	bool m_spr_sw;
	emu_timer *m_pio_timer;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
	required_region_ptr<u8> m_p_vram;
	required_device<i8255_device> m_ppi0;
	required_device<i8255_device> m_ppi1;
	required_device<i8255_device> m_ppi2;
	required_device<z80ctc_device> m_ctc;
	required_device<z80pio_device> m_pio;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_ioport_array<12> m_io_keyboard;
	required_device<cassette_image_device> m_cass;
	required_device<speaker_sound_device> m_speaker;
};

// needed to scan the keyboard, as the pio emulation doesn't do it.
TIMER_CALLBACK_MEMBER( pasopia_state::pio_timer )
{
	m_pio->port_b_write(keyb_r(generic_space(),0,0xff));
}

MC6845_UPDATE_ROW( pasopia_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint8_t chr,gfx,xi,fg=7,bg=0; // colours need to be determined
	uint16_t mem=ma,x;
	uint32_t *p = &bitmap.pix32(y);

	if (m_gfx_mode)
	{
		// To Do
	}
	else
	{
		for (x = 0; x < x_count; x++)
		{
			uint8_t inv = (x == cursor_x) ? 0xff : 0;
			mem = ma + x;
			chr = m_p_vram[mem & 0x7ff];

			/* get pattern of pixels for that character scanline */
			gfx = m_p_chargen[(chr<<3) | ra] ^ inv;

			/* Display a scanline of a character */
			for (xi = 0; xi < 8; xi++)
				*p++ = palette[BIT(gfx, 7-xi) ? fg : bg];
		}
	}
}

WRITE8_MEMBER( pasopia_state::pasopia_ctrl_w )
{
	m_ram_bank = BIT(data, 1);
	membank("bank1")->set_entry(m_ram_bank);
}

void pasopia_state::pasopia_map(address_map &map)
{
	map(0x0000, 0x7fff).bankr("bank1").bankw("bank2");
	map(0x8000, 0xffff).ram();
}


void pasopia_state::pasopia_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_ppi0, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x0b).rw(m_ppi1, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x10, 0x10).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x11, 0x11).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
//  0x18 - 0x1b pac2
//  0x1c - 0x1f something
	map(0x20, 0x23).rw(m_ppi2, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x28, 0x2b).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x30, 0x33).rw(m_pio, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
//  0x38 printer
	map(0x3c, 0x3c).w(FUNC(pasopia_state::pasopia_ctrl_w));
}

/* Input ports */
static INPUT_PORTS_START( pasopia )
	PORT_START("KEY.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GRAPH") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KANA LOCK")
	PORT_BIT( 0xc8, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("KEY.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)
	PORT_START("KEY.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Down")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_NAME("Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_NAME("CLR/Home")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_START("KEY.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("RIGHT")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("BACKSPACE") PORT_CHAR(8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("KEY.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END) PORT_NAME("Label")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Kanji")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Copy")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Stop")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT) PORT_NAME("Ins")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("LEFT")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("SPACE") PORT_CHAR(' ')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("KEY.5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_NAME("F1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_NAME("F2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_NAME("F3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_NAME("F4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_NAME("F5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_NAME("F6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_NAME("F7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_NAME("F8")
	PORT_START("KEY.6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_NAME("1") PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_NAME("0") PORT_CHAR('0') PORT_CHAR('0')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_NAME("4") PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_NAME("R") PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_NAME("Y") PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("_") PORT_CHAR('_')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("^")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Yen")
	PORT_START("KEY.7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_NAME("2") PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_NAME("3") PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_NAME("8") PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_NAME("T") PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_NAME("U") PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_NAME("7") PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_NAME("@") PORT_CHAR('@')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("[ {") PORT_CHAR('[') PORT_CHAR('{')
	PORT_START("KEY.8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("-") PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_NAME("5") PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_NAME("6") PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_NAME("F") PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_NAME("H") PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_NAME("9") PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME(": *") PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("] }") PORT_CHAR(']') PORT_CHAR('}')
	PORT_START("KEY.9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_NAME("Q") PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_NAME("W") PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_NAME("E") PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_NAME("G") PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_NAME("J") PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_NAME("I") PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_NAME("O") PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_NAME("P") PORT_CHAR('P') PORT_CHAR('p')
	PORT_START("KEY.10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_NAME("A") PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_NAME("S") PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_NAME("D") PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_NAME("V") PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_NAME("N") PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_NAME("K") PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_NAME("L") PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_NAME("; +") PORT_CHAR(';') PORT_CHAR('+')
	PORT_START("KEY.11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_NAME("Z") PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_NAME("X") PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_NAME("C") PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_NAME("B") PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_NAME("M") PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_NAME(", <") PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_NAME(". >") PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("/ ?") PORT_CHAR('/') PORT_CHAR('?')
INPUT_PORTS_END

void pasopia_state::machine_start()
{
	m_hblank = 0;
	membank("bank1")->set_entry(0);
	membank("bank2")->set_entry(0);
}

void pasopia_state::machine_reset()
{
	m_porta_2 = 0xFF;
	m_cass->change_state(CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}

WRITE8_MEMBER( pasopia_state::vram_addr_lo_w )
{
	m_vram_addr = (m_vram_addr & 0x3f00) | data;
}

WRITE8_MEMBER( pasopia_state::vram_latch_w )
{
	m_vram_latch = data;
}

READ8_MEMBER( pasopia_state::vram_latch_r )
{
	return m_p_vram[m_vram_addr];
}

READ8_MEMBER( pasopia_state::portb_1_r )
{
	/*
	x--- ---- attribute latch
	-x-- ---- hblank
	--x- ---- vblank
	---x ---- LCD system mode, active low
	*/
	uint8_t grph_latch,lcd_mode;

	m_hblank ^= 0x40; //TODO
	grph_latch = (m_p_vram[m_vram_addr | 0x4000] & 0x80);
	lcd_mode = 0x10;

	return m_hblank | lcd_mode | grph_latch; //bit 4: LCD mode
}

READ8_MEMBER( pasopia_state::portb_2_r )
{
	return (m_cass->input() > +0.04) ? 0x20 : 0;
}

WRITE8_MEMBER( pasopia_state::porta_2_w )
{
	m_cass->output(BIT(data, 4) ? -1.0 : +1.0);
	u8 changed = data ^ m_porta_2;
	m_porta_2 = data;
	if (BIT(changed, 5))
	{
		m_cass->change_state(BIT(data, 5) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
		// nasty hack
		if (!BIT(data, 5))
			m_ctc->write(2,0x23);
	}
}

WRITE_LINE_MEMBER( pasopia_state::speaker_w )
{
	if (state)
	{
		m_spr_sw ^= 1;
		if (BIT(m_mux_data, 7))
			m_speaker->level_w(m_spr_sw);
	}
}

WRITE8_MEMBER( pasopia_state::vram_addr_hi_w )
{
	m_attr_latch = (data & 0x80) | (m_attr_latch & 0x7f);
	if ( BIT(data, 6) && !m_video_wl )
	{
		m_p_vram[m_vram_addr] = m_vram_latch;
		m_p_vram[m_vram_addr | 0x4000] = m_attr_latch;
	}

	m_video_wl = BIT(data, 6);
	m_vram_addr = (m_vram_addr & 0xff) | ((data & 0x3f) << 8);
}

WRITE8_MEMBER( pasopia_state::screen_mode_w )
{
	m_gfx_mode = (data & 0xe0) >> 5;
	m_attr_latch = (m_attr_latch & 0x80) | (data & 7);
	printf("Screen Mode=%02x\n",data);
}

READ8_MEMBER( pasopia_state::rombank_r )
{
	return (m_ram_bank) ? 4 : 0;
}

READ8_MEMBER( pasopia_state::keyb_r )
{
	u8 data = 0xff;
	for (u8 i = 0; i < 3; i++)
		if (BIT(m_mux_data, i+4))
			for (u8 j = 0; j < 4; j++)
				if (BIT(m_mux_data, j))
					data &= m_io_keyboard[i*4+j]->read();

	return data;
}

WRITE8_MEMBER( pasopia_state::mux_w )
{
	m_mux_data = data;
}

static const gfx_layout p7_chars_8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_pasopia )
	GFXDECODE_ENTRY( "chargen", 0x0000, p7_chars_8x8, 0, 4 )
GFXDECODE_END

static const z80_daisy_config pasopia_daisy[] =
{
	{ "ctc" },
	{ "pio" },
//  { "fdc" }, /* TODO */
	{ nullptr }
};



void pasopia_state::init_pasopia()
{
/*
We preset all banks here, so that bankswitching will incur no speed penalty.
0000 indicates ROMs, 8000 indicates RAM.
*/
	uint8_t *ram = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 2, &ram[0x0000], 0x8000);
	membank("bank2")->configure_entry(0, &ram[0x8000]);

	m_pio_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pasopia_state::pio_timer), this));
	m_pio_timer->adjust(attotime::from_hz(50), 0, attotime::from_hz(50));
}

void pasopia_state::pasopia(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 15.9744_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &pasopia_state::pasopia_map);
	m_maincpu->set_addrmap(AS_IO, &pasopia_state::pasopia_io);
	m_maincpu->set_daisy_config(pasopia_daisy);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(14.318181_MHz_XTAL / 2, 456, 0, 296, 262, 0, 192);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	GFXDECODE(config, "gfxdecode", m_palette, gfx_pasopia);
	PALETTE(config, m_palette).set_entries(8);

	/* Devices */
	HD6845S(config, m_crtc, 14.318181_MHz_XTAL / 16);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(pasopia_state::crtc_update_row), this);

	I8255A(config, m_ppi0);
	m_ppi0->out_pa_callback().set(FUNC(pasopia_state::vram_addr_lo_w));
	m_ppi0->out_pb_callback().set(FUNC(pasopia_state::vram_latch_w));
	m_ppi0->in_pc_callback().set(FUNC(pasopia_state::vram_latch_r));

	I8255A(config, m_ppi1);
	m_ppi1->out_pa_callback().set(FUNC(pasopia_state::screen_mode_w));
	m_ppi1->in_pb_callback().set(FUNC(pasopia_state::portb_1_r));
	m_ppi1->out_pc_callback().set(FUNC(pasopia_state::vram_addr_hi_w));

	I8255A(config, m_ppi2);
	m_ppi2->in_pc_callback().set(FUNC(pasopia_state::rombank_r));
	m_ppi2->out_pa_callback().set(FUNC(pasopia_state::porta_2_w));
	m_ppi2->in_pb_callback().set(FUNC(pasopia_state::portb_2_r));

	Z80CTC(config, m_ctc, 15.9744_MHz_XTAL / 4);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<1>().set(FUNC(pasopia_state::speaker_w));
	m_ctc->zc_callback<2>().set(m_ctc, FUNC(z80ctc_device::trg3));

	Z80PIO(config, m_pio, 15.9744_MHz_XTAL / 4);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio->out_pa_callback().set(FUNC(pasopia_state::mux_w));
	m_pio->in_pb_callback().set(FUNC(pasopia_state::keyb_r));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}

/* ROM definition */
ROM_START( pasopia )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tbasic.rom", 0x0000, 0x8000, CRC(f53774ff) SHA1(bbec45a3bad8d184505cc6fe1f6e2e60a7fb53f2))
	ROM_FILL(0x304,2,0) // fixes keyboard but breaks cassette

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "font.rom", 0x0000, 0x0800, BAD_DUMP CRC(a91c45a9) SHA1(a472adf791b9bac3dfa6437662e1a9e94a88b412)) //stolen from pasopia7

	ROM_REGION( 0x8000, "vram", ROMREGION_ERASE00 )
ROM_END

/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT          COMPANY    FULLNAME   FLAGS
COMP( 1982, pasopia, 0,      0,      pasopia, pasopia, pasopia_state, init_pasopia, "Toshiba", "Personal Computer Pasopia PA7010", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
