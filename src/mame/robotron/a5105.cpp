// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

A5105

2009-05-12 Skeleton driver.

http://www.robotrontechnik.de/index.htm?/html/computer/a5105.htm
http://www.sax.de/~zander/bic/bic_bw.html

- this looks like "somehow" inspired by the MSX1 machine?

Cassette commands: CSAVE "name" ; CLOAD


ToDo:
- Cassette (coded per schematic, but doesn't work)


****************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "sound/beep.h"
#include "video/upd7220.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "formats/a5105_dsk.h"


namespace {

class a5105_state : public driver_device
{
public:
	a5105_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_hgdc(*this, "upd7220")
		, m_cass(*this, "cassette")
		, m_beep(*this, "beeper")
		, m_fdc(*this, "upd765a")
		, m_floppy(*this, "upd765a:%u", 0U)
		, m_video_ram(*this, "video_ram")
		, m_ram(*this, RAM_TAG)
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{ }

	void a5105(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	uint8_t a5105_memsel_r();
	uint8_t key_r();
	uint8_t key_mux_r();
	uint8_t pio_pb_r();
	void key_mux_w(uint8_t data);
	void a5105_ab_w(uint8_t data);
	void a5105_memsel_w(uint8_t data);
	void a5105_upd765_w(uint8_t data);
	void pcg_addr_w(uint8_t data);
	void pcg_val_w(uint8_t data);
	void a5105_palette(palette_device &palette) const;
	static void floppy_formats(format_registration &fr);
	UPD7220_DISPLAY_PIXELS_MEMBER( hgdc_display_pixels );
	UPD7220_DRAW_TEXT_LINE_MEMBER( hgdc_draw_text );

	void a5105_io(address_map &map) ATTR_COLD;
	void a5105_mem(address_map &map) ATTR_COLD;
	void upd7220_map(address_map &map) ATTR_COLD;

	uint8_t *m_ram_base = 0;
	uint8_t *m_rom_base = 0;
	uint8_t *m_char_ram = 0;
	uint16_t m_pcg_addr = 0U;
	uint16_t m_pcg_internal_addr = 0U;
	uint8_t m_key_mux = 0U;
	uint8_t m_memsel[4]{};
	required_device<z80_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<upd7220_device> m_hgdc;
	required_device<cassette_image_device> m_cass;
	required_device<beep_device> m_beep;
	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	required_shared_ptr<uint16_t> m_video_ram;
	required_device<ram_device> m_ram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

/* TODO */
UPD7220_DISPLAY_PIXELS_MEMBER( a5105_state::hgdc_display_pixels )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();

	int const gfx = m_video_ram[(address & 0xffff)];

	for (int xi = 0; xi < 16; xi++)
	{
		int const pen = ((gfx >> xi) & 1) ? 7 : 0;

		bitmap.pix(y, x + xi) = palette[pen];
	}
}

UPD7220_DRAW_TEXT_LINE_MEMBER( a5105_state::hgdc_draw_text )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();

	for (int x = 0; x < pitch; x++)
	{
		int const tile = (m_video_ram[(((addr+x)*2) & 0x1ffff) >> 1] & 0xff);
		int const color = ((m_video_ram[(((addr+x)*2) & 0x1ffff) >> 1] >> 8) & 0x0f);

		for (int yi = 0; yi < lr; yi++)
		{
			uint8_t tile_data = m_char_ram[(tile*8+yi) & 0x7ff];

			if (cursor_on && cursor_addr == addr+x && m_screen->frame_number() & 0x10)
				tile_data ^= 0xff;

			for (int xi = 0; xi < 8; xi++)
			{
				int pen = (tile_data >> xi) & 1 ? color : 0;

				int const res_x = x * 8 + xi;
				int const res_y = y + yi;

				if (yi >= 8)
					pen = 0;

				if (m_screen->visible_area().contains(res_x+0, res_y))
					bitmap.pix(res_y, res_x) = palette[pen];
			}
		}
	}
}

void a5105_state::a5105_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).bankr("bank1");
	map(0x4000, 0x7fff).bankr("bank2");
	map(0x8000, 0xbfff).bankrw("bank3");
	map(0xc000, 0xffff).bankrw("bank4");
}

uint8_t a5105_state::pio_pb_r()
{
	/*

	    PIO Channel B

	    0  R    PAR12
	    1  W    SER1
	    2  W    SER2
	    3  R    SER3
	    4  R    SER4
	    5  W    JOY2
	    6  W    /JOYEN
	    7  R    Cassette Data

	*/

	uint8_t data = 0x7f;

	// cassette data
	data |= (m_cass->input() > 0) ? 0x80 : 0;

	return data;
}

void  a5105_state::pcg_addr_w(uint8_t data)
{
	m_pcg_addr = data << 3;
	m_pcg_internal_addr = 0;
}

void a5105_state::pcg_val_w(uint8_t data)
{
	m_char_ram[m_pcg_addr | m_pcg_internal_addr] = data;

	m_gfxdecode->gfx(0)->mark_dirty(m_pcg_addr >> 3);

	m_pcg_internal_addr++;
	m_pcg_internal_addr&=7;
}

uint8_t a5105_state::key_r()
{
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3",
											"KEY4", "KEY5", "KEY6", "KEY7",
											"KEY8", "KEY9", "KEYA", "UNUSED",
											"UNUSED", "UNUSED", "UNUSED", "UNUSED" };

	return ioport(keynames[m_key_mux & 0x0f])->read();
}

uint8_t a5105_state::key_mux_r()
{
	return m_key_mux;
}

void a5105_state::key_mux_w(uint8_t data)
{
	/*
	    xxxx ---- unknown
	    ---- xxxx keyboard mux
	*/

	m_key_mux = data;
}

void a5105_state::a5105_ab_w(uint8_t data)
{
/*port $ab
        ---- 100x tape motor, active low
        ---- 101x tape data
        ---- 110x led (color green)
        ---- 111x key click, active high
*/
	switch (data & 6)
	{
	case 0:
		if (BIT(data, 0))
			m_cass->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
		else
			m_cass->change_state(CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);
		break;

	case 2:
		m_cass->output( BIT(data, 0) ? -1.0 : +1.0);
		break;

	case 4:
		//led
		break;

	case 6:
		m_beep->set_state(BIT(data, 0));
		break;
	}
}

uint8_t a5105_state::a5105_memsel_r()
{
	uint8_t res;

	res = (m_memsel[0] & 3) << 0;
	res|= (m_memsel[1] & 3) << 2;
	res|= (m_memsel[2] & 3) << 4;
	res|= (m_memsel[3] & 3) << 6;

	return res;
}

void a5105_state::a5105_memsel_w(uint8_t data)
{
	address_space &prog = m_maincpu->space( AS_PROGRAM );

	if (m_memsel[0] != ((data & 0x03) >> 0))
	{
		m_memsel[0] = (data & 0x03) >> 0;

		switch (m_memsel[0])
		{
		case 0:
			membank("bank1")->set_base(m_rom_base);
			prog.install_read_bank(0x0000, 0x3fff, membank("bank1"));
			prog.unmap_write(0x0000, 0x3fff);
			break;
		case 2:
			membank("bank1")->set_base(m_ram_base);
			prog.install_readwrite_bank(0x0000, 0x3fff, membank("bank1"));
			break;
		default:
			prog.unmap_readwrite(0x0000, 0x3fff);
			break;
		}
	}

	if (m_memsel[1] != ((data & 0x0c) >> 2))
	{
		m_memsel[1] = (data & 0x0c) >> 2;

		switch (m_memsel[1])
		{
		case 0:
			membank("bank2")->set_base(m_rom_base + 0x4000);
			prog.install_read_bank(0x4000, 0x7fff, membank("bank2"));
			prog.unmap_write(0x4000, 0x4000);
			break;
		case 1:
			membank("bank2")->set_base(memregion("k5651")->base());
			prog.install_read_bank(0x4000, 0x7fff, membank("bank2"));
			prog.unmap_write(0x4000, 0x4000);
			break;
		case 2:
			membank("bank2")->set_base(m_ram_base + 0x4000);
			prog.install_readwrite_bank(0x4000, 0x7fff, membank("bank2"));
			break;
		default:
			prog.unmap_readwrite(0x4000, 0x7fff);
			break;
		}
	}

	if (m_memsel[2] != ((data & 0x30) >> 4))
	{
		m_memsel[2] = (data & 0x30) >> 4;

		switch (m_memsel[2])
		{
		case 0:
			membank("bank3")->set_base(m_rom_base + 0x8000);
			prog.install_read_bank(0x8000, 0xbfff, membank("bank3"));
			prog.unmap_write(0x8000, 0xbfff);
			break;
		case 2:
			membank("bank3")->set_base(m_ram_base + 0x8000);
			prog.install_readwrite_bank(0x8000, 0xbfff, membank("bank3"));
			break;
		default:
			prog.unmap_readwrite(0x8000, 0xbfff);
			break;
		}
	}

	if (m_memsel[3] != ((data & 0xc0) >> 6))
	{
		m_memsel[3] = (data & 0xc0) >> 6;

		switch (m_memsel[3])
		{
		case 2:
			membank("bank4")->set_base(m_ram_base + 0xc000);
			prog.install_readwrite_bank(0xc000, 0xffff, membank("bank4"));
			break;
		default:
			prog.unmap_readwrite(0xc000, 0xffff);
			break;
		}
	}

	//printf("Memsel change to %02x %02x %02x %02x\n",m_memsel[0],m_memsel[1],m_memsel[2],m_memsel[3]);
}

void a5105_state::a5105_upd765_w(uint8_t data)
{
	for (int n = 0; n < 4; n++)
		m_floppy[n]->get_device()->mon_w(!BIT(data,n));

	m_fdc->tc_w(BIT(data, 4));
}

void a5105_state::a5105_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x40, 0x41).m(m_fdc, FUNC(upd765a_device::map));
	map(0x48, 0x4f).w(FUNC(a5105_state::a5105_upd765_w));

	map(0x80, 0x83).rw("z80ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x90, 0x93).rw("z80pio", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x98, 0x99).rw(m_hgdc, FUNC(upd7220_device::read), FUNC(upd7220_device::write));

	map(0x9c, 0x9c).w(FUNC(a5105_state::pcg_val_w));
//  map(0x9d, 0x9d) crtc area (ff-based), palette routes here
	map(0x9e, 0x9e).w(FUNC(a5105_state::pcg_addr_w));

//  map(0xa0, 0xa1) ay8910?
	map(0xa8, 0xa8).rw(FUNC(a5105_state::a5105_memsel_r), FUNC(a5105_state::a5105_memsel_w));
	map(0xa9, 0xa9).r(FUNC(a5105_state::key_r));
	map(0xaa, 0xaa).rw(FUNC(a5105_state::key_mux_r), FUNC(a5105_state::key_mux_w));
	map(0xab, 0xab).w(FUNC(a5105_state::a5105_ab_w)); //misc output, see above
}

/* Input ports */
static INPUT_PORTS_START( a5105 )
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('\\')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('+') PORT_CHAR('*')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(U'ö') PORT_CHAR(U'Ö')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(U'ä') PORT_CHAR(U'Ä')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(U'ü') PORT_CHAR(U'Ü')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('#') PORT_CHAR('^')

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('\'') PORT_CHAR('`')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('?') PORT_CHAR(U'ß')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED) // appears to do nothing
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) /* gives keyclick but does nothing */PORT_CODE(KEYCODE_LCONTROL)    PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GRAPH") PORT_CODE(KEYCODE_PGUP) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CODE") PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)      PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)      PORT_CHAR(9)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STOP") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)    PORT_CHAR(8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SELECT") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)        PORT_CHAR(13)

	PORT_START("KEY8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)        PORT_CHAR(' ')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)     PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)       PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)     PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)       PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)     PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)        PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("KEY9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)     PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)     PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)    PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)        PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)        PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)        PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)        PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)        PORT_CHAR(UCHAR_MAMEKEY(4_PAD))

	PORT_START("KEYA")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)        PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)        PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)        PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)        PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)        PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)    PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)    PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)      PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))

	PORT_START("UNUSED")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


void a5105_state::machine_reset()
{
	a5105_ab_w(9); // turn motor off

	m_ram_base = (uint8_t*)m_ram->pointer();
	m_rom_base = (uint8_t*)memregion("maincpu")->base();

	membank("bank1")->set_base(m_rom_base);
	membank("bank2")->set_base(m_rom_base + 0x4000);
	membank("bank3")->set_base(m_ram_base);
	membank("bank4")->set_base(m_ram_base + 0x4000);
}


static const gfx_layout a5105_chars_8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_a5105 )
	GFXDECODE_ENTRY( "pcg", 0x0000, a5105_chars_8x8, 0, 8 )
GFXDECODE_END


void a5105_state::a5105_palette(palette_device &palette) const
{
	for (int i = 0; i < 16; i++)
	{
		int const x = (i & 8) ? 0xaa : 0xff;
		int const r = (i & 4) ? x : 0x00;
		int const g = (i & 2) ? x : 0x00;
		int const b = (i & 1) ? x : 0x00;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void a5105_state::video_start()
{
	// find memory regions
	m_char_ram = memregion("pcg")->base();
}

void a5105_state::upd7220_map(address_map &map)
{
	map.global_mask(0x1ffff);
	map(0x00000, 0x1ffff).ram().share("video_ram");
}

void a5105_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_A5105_FORMAT);
}

static void a5105_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

static const z80_daisy_config a5105_daisy_chain[] =
{
	{ "z80ctc" },
	{ "z80pio" },
	{ nullptr }
};

void a5105_state::a5105(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(15'000'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &a5105_state::a5105_mem);
	m_maincpu->set_addrmap(AS_IO, &a5105_state::a5105_io);
	m_maincpu->set_daisy_config(a5105_daisy_chain);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_screen_update("upd7220", FUNC(upd7220_device::screen_update));
	m_screen->set_size(40*8, 32*8);
	m_screen->set_visarea(0, 40*8-1, 0, 25*8-1);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_a5105);
	PALETTE(config, m_palette, FUNC(a5105_state::a5105_palette), 16);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, "beeper", 500).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* Devices */
	UPD7220(config, m_hgdc, XTAL(15'000'000) / 16); // unk clock
	m_hgdc->set_addrmap(0, &a5105_state::upd7220_map);
	m_hgdc->set_display_pixels(FUNC(a5105_state::hgdc_display_pixels));
	m_hgdc->set_draw_text(FUNC(a5105_state::hgdc_draw_text));

	z80ctc_device& ctc(Z80CTC(config, "z80ctc", XTAL(15'000'000) / 4));
	ctc.intr_callback().set_inputline(m_maincpu, 0);
	ctc.zc_callback<0>().set("z80ctc", FUNC(z80ctc_device::trg2));
	ctc.zc_callback<2>().set("z80ctc", FUNC(z80ctc_device::trg3));

	z80pio_device& pio(Z80PIO(config, "z80pio", XTAL(15'000'000) / 4));
	pio.in_pb_callback().set(FUNC(a5105_state::pio_pb_r));
	pio.out_int_callback().set_inputline(m_maincpu, 0);

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	UPD765A(config, m_fdc, 8'000'000, true, true);
	FLOPPY_CONNECTOR(config, "upd765a:0", a5105_floppies, "525qd", a5105_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "upd765a:1", a5105_floppies, "525qd", a5105_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "upd765a:2", a5105_floppies, "525qd", a5105_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "upd765a:3", a5105_floppies, "525qd", a5105_state::floppy_formats);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K");
}

/* ROM definition */
ROM_START( a5105 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "k1505_00.rom", 0x0000, 0x8000, CRC(0ed5f556) SHA1(5c33139db9f59e50da5c76729752f8e653ae34ae))
	ROM_LOAD( "k1505_80.rom", 0x8000, 0x2000, CRC(350e4958) SHA1(7e5b587c59676e8549561117ea0b70234f439a94))

	ROM_REGION( 0x800, "pcg", ROMREGION_ERASE00 )

	ROM_REGION( 0x4000, "k5651", ROMREGION_ERASEFF )
	ROM_LOAD( "k5651_40.rom", 0x0000, 0x2000, CRC(f4ad4739) SHA1(9a7bbe6f0d180dd513c7854f441cee986c8d9765))
	ROM_LOAD( "k5651_60.rom", 0x2000, 0x2000, CRC(c77dde3f) SHA1(7c16226be6c4c71013e8008fba9d2e9c5640b6a7))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY         FULLNAME     FLAGS
COMP( 1989, a5105, 0,      0,      a5105,   a5105, a5105_state, empty_init, "VEB Robotron", "BIC A5105", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
