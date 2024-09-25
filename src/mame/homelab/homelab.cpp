// license:BSD-3-Clause
// copyright-holders:linuxforum5, Miodrag Milanovic, Robbbert
/***************************************************************************

    Homelab driver by Miodrag Milanovic

    31/08/2008 Preliminary driver.
    15/06/2012 Various updates [Robbbert]
    27/01/2024 homelab2 now works, reorganized to better match current standards. [linuxforum5, R. Belmont]

    The emulator called HoLa! works fine, but it is closed source.
    You can get HoLa! at: http://gaia.atilia.eu

    ToDO:
    - HTP files should be a cassette format, not a quickload.
    - Quickloads cause the emulated machine to hang or reboot.
    - homelab2 - cassette to fix.
                 Note that rom code 0x40-48 is meaningless garbage,
                 had to patch to stop it crashing. Need a new dump.
               - The speaker code is wrong, but it's the only way to
                 get any sound. Needs to be looked at again.
    - homelab3/4 - Need a dump of the TM188 prom.
                 cassette to fix.
                 up to 64k ram can be fitted. schematic only shows 16k.
                 Z80PIO - expansion only, doesn't do anything in the
                 machine. /CE connects to A6, A/B = A0, C/D = A1.
                 The bios never talks to it. Official port numbers
                 are 3C-3F.
    - Brailab4 - Same as homelab3.


TM188 is (it seems) equivalent to 27S19, TBP18S030N, 6331-1, 74S288, 82S123,
MB7051 - fuse programmed prom.

****************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/timer.h"
#include "sound/mea8000.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class homelab_state : public driver_device
{
public:
	homelab_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_bank1(*this, "bank1"),
		m_p_chargen(*this, "chargen"),
		m_speaker(*this, "speaker"),
		m_cass(*this, "cassette"),
		m_io_keyboard(*this, "X%d", 0),
		m_rows(0), m_cols(0)
	{
	}

protected:
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	required_memory_bank m_bank1;
	required_region_ptr<u8> m_p_chargen;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cass;
	required_ioport_array<16> m_io_keyboard;

	std::unique_ptr<u8[]> m_vram;
	u8 m_rows; // Character rows in screen
	u8 m_cols;
};

class homelab2_state : public homelab_state
{
public:
	homelab2_state(const machine_config &mconfig, device_type type, const char *tag) :
		homelab_state(mconfig, type, tag),
		m_nmi(0),
		m_screenshadow_is_text_mode(true),
		m_screenshadowY0(0),
		m_screenshadowX0(0),
		m_spr_bit(0)
	{
		std::fill(std::begin(m_4000shadow), std::end(m_4000shadow), 0);
		std::fill(std::begin(m_screenshadow), std::end(m_screenshadow), 0);
	}

	void homelab2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	u32 screen2_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	INTERRUPT_GEN_MEMBER(homelab_frame);
	void homelab2_mem(address_map &map) ATTR_COLD;
	u8 cass2_r();
	u8 mem3800_r();
	void mem3800_w(offs_t offset, u8 data);
	u8 mem3a00_r(offs_t offset);
	void mem3c00_w(offs_t, u8 data);
	u8 mem3e00_r(offs_t offset);
	void mem3e00_w(offs_t, u8 data);
	u8 mem4000_r(offs_t offset);
	void mem4000_w(offs_t, u8 data);
	u8 memE000_r(offs_t offset);
	void memE000_w(offs_t, u8 data);

	bool m_nmi;

	u8 m_4000shadow[0x4000];            // Shadow for 0x4000-0x7FFF
	u8 m_screenshadow[40 * 265];        // Maximum screen size is 320x255 (1 bit = 1 pixel). CPU controls hardware to generate video signal.

	bool m_screenshadow_is_text_mode;   // If true, the data source for video generator is from C000. If false, the data source is from 0x6000. This memory contetn addressable from 0xE000
	int32_t m_screenshadowY0;           // The current generated screen x byte position
	int32_t m_screenshadowX0;           // Screen row position
	bool m_spr_bit;
};

class homelab3_state : public homelab_state
{
public:
	using homelab_state::homelab_state;

	void homelab3(machine_config &config);
	void brailab4(machine_config &config);

	int cass3_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u8 exxx_r(offs_t offset);
	void port7f_w(u8 data);
	void portff_w(u8 data);
	void homelab3_io(address_map &map) ATTR_COLD;
	void homelab3_mem(address_map &map) ATTR_COLD;
	void brailab4_io(address_map &map) ATTR_COLD;
	void brailab4_mem(address_map &map) ATTR_COLD;

	std::unique_ptr<u8[]> m_ram;
};

INTERRUPT_GEN_MEMBER(homelab2_state::homelab_frame)
{
	if (m_nmi)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
}

u32 homelab2_state::screen2_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!m_cols)
	{
		return 1;
	}

	int screenHeight = (m_screenshadowY0 > 200) ? m_screenshadowY0 : 200;
	screen.set_visarea(0, 319, 0, screenHeight - 1);
	for (int y = 0; y < screenHeight; y++)
	{
		u16 *p = &bitmap.pix(y); // Row first pixel pointer in viewable screen in screen_y. line
		for (int x = 0; x < 40; x++)
		{
			if (y < m_screenshadowY0)
			{
				u8 const gfx = m_screenshadow[y * 40 + x];
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
			else
			{
				*p++ = 0;
				*p++ = 0;
				*p++ = 0;
				*p++ = 0;
				*p++ = 0;
				*p++ = 0;
				*p++ = 0;
				*p++ = 0;
			}
		}
	}
	m_screenshadowY0 = 0;
	m_screenshadowX0 = 0;
	return 0;
}

u8 homelab2_state::mem3800_r()
{
	return m_io_keyboard[15]->read(); // reset key
}

void homelab2_state::mem3800_w(offs_t offset, u8 data)
{ // Set screen generator hardware to graphics mode
	if (offset == 0x3939 - 0x3800)
	{
		m_screenshadow_is_text_mode = false;
	}
}

u8 homelab2_state::mem3a00_r(offs_t offset)
{
	u8 data = 0xff;

	for (u8 i = 0; i < 8; i++)
	{
		if (!BIT(offset, i))
		{
			data &= m_io_keyboard[i]->read();
		}
	}
	return data;
}

u8 homelab2_state::cass2_r()
{
	return (m_cass->input() > 0.03) ? 0xff : 0;
}

void homelab2_state::mem3c00_w(offs_t offset, u8 data)
{
	m_spr_bit ^= 1;
	m_speaker->level_w(m_spr_bit ? -1.0 : +1.0);
	m_cass->output(m_spr_bit ? -1.0 : +1.0);
}

u8 homelab2_state::memE000_r(offs_t offset)
{
	if (m_nmi)
	{ // NMI enabled, screen generator
		u8 gfx;
		if (m_screenshadow_is_text_mode)
		{
			const int vramRelIndex0 = offset % 0x400;     // Character address in video ram First character in 0x001
			const int row8_index0 = (offset - 1) / 0x400; // Row index in char [0-7]
			u8 const chr = m_vram[vramRelIndex0];         // get char in videoram
			gfx = m_p_chargen[chr | (row8_index0 << 8)];  // get dot pattern in chargen
		}
		else
		{
			gfx = m_4000shadow[0x2000 + offset]; // get dot pixels in GRAPH RAM
		}
		m_screenshadow[40 * m_screenshadowY0 + m_screenshadowX0++] = gfx;

		if (m_screenshadowX0 == 40)
		{
			m_screenshadowX0 = 0;
			m_screenshadowY0++;
			return (m_screenshadow_is_text_mode) ? 0xff : 0xf7; // RST38 : RST 30
		}
		else
		{
			return 0x7f; // LD A,A
		}
	}
	else
	{ // NMI disable, serial input
		return cass2_r();
	}
}

u8 homelab2_state::mem4000_r(offs_t offset)
{
	return m_4000shadow[offset];
}

void homelab2_state::mem4000_w(offs_t offset, u8 data)
{
	m_4000shadow[offset] = data;
}

u8 homelab2_state::mem3e00_r(offs_t offset)
{
	return 0;
}

/**
 * NMI on: LD (3E3E), 0
 * NMI off: LD (3F3F), AL értéke
 */
void homelab2_state::mem3e00_w(offs_t offset, u8 data)
{
	if (offset == 0x3f3f - 0x3e00)
	{ // 3F3F
		m_screenshadow_is_text_mode = true;
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		m_nmi = true;
	}
	if (offset == 0x003e)
	{ // 3E3E
		m_nmi = false;
	}
}

void homelab3_state::machine_reset()
{
	m_bank1->set_entry(1);
}

void homelab3_state::port7f_w(u8 data)
{
	m_bank1->set_entry(1);
}

void homelab3_state::portff_w(u8 data)
{
	m_bank1->set_entry(0);
}

int homelab3_state::cass3_r()
{
	return (m_cass->input() > 0.03);
}

u8 homelab3_state::exxx_r(offs_t offset)
{
	// keys E800-E813 but E810-E813 are not connected
	// cassin E883
	// speaker/cass toggle E880, E802
	if (offset == 0x83)
	{
		return (m_cass->input() > 0.03);
	}
	else
	{
		if (offset == 0x80)
		{
			m_speaker->level_w(0);
			m_cass->output(-1.0);
		}
		else
		{
			if (offset == 0x02)
			{
				m_speaker->level_w(1);
				m_cass->output(+1.0);
			}
		}
	}

	u8 data = 0xff;
	if (offset < 0x10)
	{
		data = m_io_keyboard[offset]->read();
	}
	return data;
}

/* Address maps */
void homelab2_state::homelab2_mem(address_map &map)
{
	map(0x0000, 0x07ff).rom(); // ROM 1
	map(0x0800, 0x0fff).rom(); // ROM 2
	map(0x1000, 0x17ff).rom(); // ROM 3
	map(0x1800, 0x1fff).rom(); // ROM 4
	map(0x2000, 0x27ff).rom(); // ROM 5
	map(0x2800, 0x2fff).rom(); // ROM 6
	map(0x3000, 0x37ff).rom(); // Empty
	map(0x3800, 0x39ff).rw(FUNC(homelab2_state::mem3800_r), FUNC(homelab2_state::mem3800_w));
	map(0x3a00, 0x3bff).r(FUNC(homelab2_state::mem3a00_r));
	map(0x3c00, 0x3dff).w(FUNC(homelab2_state::mem3c00_w));
	map(0x3e00, 0x3fff).rw(FUNC(homelab2_state::mem3e00_r), FUNC(homelab2_state::mem3e00_w));

	map(0x4000, 0x7fff).rw(FUNC(homelab2_state::mem4000_r), FUNC(homelab2_state::mem4000_w));

	map(0xc000, 0xc3ff).mirror(0xc00).bankrw(m_bank1);

	map(0xe000, 0xffff).r(FUNC(homelab2_state::memE000_r));
}

void homelab3_state::homelab3_mem(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0xcfff).ram();
	map(0xe800, 0xefff).r(FUNC(homelab3_state::exxx_r));
	map(0xf800, 0xffff).bankrw(m_bank1);
}

void homelab3_state::homelab3_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x7f, 0x7f).w(FUNC(homelab3_state::port7f_w));
	map(0xff, 0xff).w(FUNC(homelab3_state::portff_w));
}

void homelab3_state::brailab4_mem(address_map &map)
{
	homelab3_mem(map);
	map(0xd000, 0xdfff).rom().region("maincpu", 0x4000);
}

void homelab3_state::brailab4_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	homelab3_io(map);
	map(0xf8, 0xf9).rw("mea8000", FUNC(mea8000_device::read), FUNC(mea8000_device::write));
}

/* Input ports */
static INPUT_PORTS_START(homelab2)
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0xDE, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Home") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Run/Brk") PORT_CODE(KEYCODE_RCONTROL)

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('<')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR('<') PORT_CHAR(',')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('=') PORT_CHAR('-')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('>') PORT_CHAR('.')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('?') PORT_CHAR('/')

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G')

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O')

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W')

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('^')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT) PORT_CODE(KEYCODE_LALT) PORT_CHAR('_')

	PORT_START("X8")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X11")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X12")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X13")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X14")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X15")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0xfe, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

	static INPUT_PORTS_START(homelab3) // F4 to F8 are foreign characters
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED) // A
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)                                     // D
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_VBLANK("screen")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ALT") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(homelab3_state, cass3_r)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('=') PORT_CHAR('-')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('?') PORT_CHAR('/')
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X13")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X14")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X15")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START(brailab4) // F4 to F8 are foreign characters
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED) // A
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)                                     // D
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_VBLANK("screen")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ALT") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(homelab3_state, cass3_r)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('=') PORT_CHAR('-')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('?') PORT_CHAR('/')
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X13")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X14")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X15")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

void homelab2_state::machine_start()
{
	save_item(NAME(m_nmi));
	save_item(NAME(m_rows));
	save_item(NAME(m_cols));
	save_item(NAME(m_4000shadow));
	save_item(NAME(m_screenshadow));
	save_item(NAME(m_screenshadow_is_text_mode));
	save_item(NAME(m_screenshadowX0));
	save_item(NAME(m_screenshadowY0));
	save_item(NAME(m_spr_bit));

	m_vram = make_unique_clear<u8[]>(0x800);
	save_pointer(NAME(m_vram), 0x800);
	m_bank1->configure_entry(0, m_vram.get());
	m_bank1->set_entry(0);

	m_rows = 25;
	m_cols = 40;
	m_nmi = 0;
	m_spr_bit = 0;
}

void homelab3_state::machine_start()
{
	save_item(NAME(m_rows));
	save_item(NAME(m_cols));
	m_vram = make_unique_clear<u8[]>(0x800);
	save_pointer(NAME(m_vram), 0x800);
	m_ram = make_unique_clear<u8[]>(0x800);
	save_pointer(NAME(m_ram), 0x800);
	m_bank1->configure_entry(0, m_vram.get());
	m_bank1->configure_entry(1, m_ram.get());
	m_rows = 32;
	m_cols = 64;
}

u32 homelab_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!m_cols)
	{
		return 1;
	}

	u16 sy = 0, ma = 0;
	for (u8 y = 0; y < m_rows; y++)
	{
		for (u8 ra = 0; ra < 8; ra++)
		{
			u16 *p = &bitmap.pix(sy++);
			for (u16 x = ma; x < ma + m_cols; x++)
			{
				u8 const chr = m_vram[x];                    // get char in videoram
				u8 const gfx = m_p_chargen[chr | (ra << 8)]; // get dot pattern in chargen

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma += m_cols;
	}
	return 0;
}

/* F4 Character Displayer */
static const gfx_layout charlayout =
{
	8, 8, /* 8 x 8 characters */
	256,  /* 256 characters */
	1,    /* 1 bits per pixel */
	{0},  /* no bitplanes */
	/* x offsets */
	{0, 1, 2, 3, 4, 5, 6, 7},
	/* y offsets */
	{0, 1 * 256 * 8, 2 * 256 * 8, 3 * 256 * 8, 4 * 256 * 8, 5 * 256 * 8, 6 * 256 * 8, 7 * 256 * 8},
	8 /* every char takes 8 x 1 bytes */
};

static GFXDECODE_START(gfx_homelab)
	GFXDECODE_ENTRY("chargen", 0x0000, charlayout, 0, 1)
GFXDECODE_END

QUICKLOAD_LOAD_MEMBER(homelab_state::quickload_cb)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int block_counter = 0;
	char block_last_character = 1;
	char pgmname[256];
	u16 args[2];

	image.fseek(0, SEEK_SET);

	while (block_last_character != 0)
	{
		u8 ch = 0;
		u32 bytes = 0;

		block_counter++;
		// Read leading zeros
		while (((bytes = image.fread(&ch, 1)) != 0) && (ch == 0))
		{
		}

		if (bytes != 1 || ch != 0xa5)
			return std::make_pair(image_error::INVALIDIMAGE, "Invalid header");

		int i = 0;
		while ((bytes = image.fread(&ch, 1)) != 0 && (ch != 0))
		{
			if (i >= (std::size(pgmname) - 1))
			{
				return std::make_pair(image_error::INVALIDIMAGE, "File name too long");
			}
			// image.message treats characters with bit 7 as nulls, so replace with question mark
			pgmname[i] = BIT(ch, 7) ? 0x3f : ch; // build program description
			i++;
		}
		pgmname[i] = '\0'; /* terminate string with a null */

		if (image.fread(args, sizeof(args)) != sizeof(args))
		{
			return std::make_pair(image_error::UNSPECIFIED, "Unexpected EOF while getting file size");
		}

		const u16 quick_addr = little_endianize_int16(args[0]);
		const u16 quick_length = little_endianize_int16(args[1]);
		const u16 quick_end = quick_addr + quick_length - 1;

		if (quick_end > 0x7fff)
		{
			return std::make_pair(image_error::INVALIDLENGTH, "File too large");
		}

		for (int i = 0; i < quick_length; i++)
		{
			unsigned j = (quick_addr + i);
			if (image.fread(&ch, 1) != 1)
			{
				return std::make_pair(image_error::UNSPECIFIED, util::string_format("%s: Unexpected EOF while writing byte to %04X", pgmname, j));
			}
			space.write_byte(j, ch);
		}
		image.fread(&ch, 1); // Read crc
		image.fread(&block_last_character, 1);
		/* display a message about the loaded quickload */
		image.message(" %s\nsize=%04X : start=%04X : end=%04X : block_counter=%d", pgmname, quick_length, quick_addr, quick_end, block_counter);
	}
	return std::make_pair(std::error_condition(), std::string());
}

/* Machine driver */
void homelab2_state::homelab2(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(8'000'000) / 2); // 4mhz
	m_maincpu->set_addrmap(AS_PROGRAM, &homelab2_state::homelab2_mem);
	m_maincpu->set_vblank_int("screen", FUNC(homelab2_state::homelab_frame));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::white())); // green
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(320, 255);
	screen.set_visarea(0, 40 * 8 - 1, 0, 25 * 8 - 1);
	screen.set_screen_update(FUNC(homelab2_state::screen2_update));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_homelab);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	CASSETTE(config, m_cass);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	QUICKLOAD(config, "quickload", "htp", attotime::from_seconds(2)).set_load_callback(FUNC(homelab2_state::quickload_cb));
}

void homelab3_state::homelab3(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(12'000'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &homelab3_state::homelab3_mem);
	m_maincpu->set_addrmap(AS_IO, &homelab3_state::homelab3_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(64 * 8, 32 * 8);
	screen.set_visarea(0, 64 * 8 - 1, 0, 32 * 8 - 1);
	screen.set_screen_update(FUNC(homelab3_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_homelab);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	CASSETTE(config, m_cass);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	QUICKLOAD(config, "quickload", "htp", attotime::from_seconds(2)).set_load_callback(FUNC(homelab3_state::quickload_cb));
}

void homelab3_state::brailab4(machine_config &config)
{
	homelab3(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &homelab3_state::brailab4_mem);
	m_maincpu->set_addrmap(AS_IO, &homelab3_state::brailab4_io);
	MEA8000(config, "mea8000", 3'840'000).add_route(ALL_OUTPUTS, "mono", 1.0);
}

/* ROM definition */

ROM_START(homelab2)
	ROM_REGION(0x3800, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("hl2_1.ic2", 0x0000, 0x0800, CRC(205365f7) SHA1(da93b65befd83513dc762663b234227ba804124d))
	ROM_LOAD("hl2_2.ic3", 0x0800, 0x0800, CRC(696af3c1) SHA1(b53bc6ae2b75975618fc90e7181fa5d21409fce1))
	ROM_LOAD("hl2_3.ic4", 0x1000, 0x0800, CRC(69e57e8c) SHA1(e98510abb715dbf513e1b29fb6b09ab54e9483b7))
	ROM_LOAD("hl2_4.ic5", 0x1800, 0x0800, CRC(97cbbe74) SHA1(34f0bad41302b059322018abc3d1c2336ecfbea8))
	ROM_LOAD("hl2_m.ic6", 0x2000, 0x0800, CRC(10040235) SHA1(e121dfb97cc8ea99193a9396a9f7af08585e0ff0))

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("hl2.ic33", 0x0000, 0x0800, CRC(2e669d40) SHA1(639dd82ed29985dc69830aca3b904b6acc8fe54a))
ROM_END

ROM_START(homelab3)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("hl3_1.ic1", 0x0000, 0x1000, CRC(6b90a8ea) SHA1(8ac40ca889b8c26cdf74ca309fbafd70dcfdfbec))
	ROM_LOAD("hl3_2.ic2", 0x1000, 0x1000, CRC(bcac3c24) SHA1(aff371d17f61cb60c464998e092f04d5d85c4d52))
	ROM_LOAD("hl3_3.ic3", 0x2000, 0x1000, CRC(ab1b4ab0) SHA1(ad74c7793f5dc22061a88ef31d3407267ad08719))
	ROM_LOAD("hl3_4.ic4", 0x3000, 0x1000, CRC(bf67eff9) SHA1(2ef5d46f359616e7d0e5a124df528de44f0e850b))

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("hl3.ic21", 0x0000, 0x0800, CRC(f58ee39b) SHA1(49399c42d60a11b218a225856da86a9f3975a78a))

	ROM_REGION(0x0040, "proms", 0)
	ROM_LOAD("tm188.ic7", 0x0000, 0x0040, NO_DUMP)
ROM_END

ROM_START(homelab4)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("hl4_1.ic1", 0x0000, 0x1000, CRC(a549b2d4) SHA1(90fc5595da8431616aee56eb5143b9f04281e798))
	ROM_LOAD("hl4_2.ic2", 0x1000, 0x1000, CRC(151d33e8) SHA1(d32004bc1553f802b9d3266709552f7d5315fe44))
	ROM_LOAD("hl4_3.ic3", 0x2000, 0x1000, CRC(39571ab1) SHA1(8470cff2e3442101e6a0bc655358b3a6fc1ef944))
	ROM_LOAD("hl4_4.ic4", 0x3000, 0x1000, CRC(f4b77ca2) SHA1(ffbdb3c1819c7357e2a0fc6317c111a8a7ecfcd5))

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("hl4.ic21", 0x0000, 0x0800, CRC(f58ee39b) SHA1(49399c42d60a11b218a225856da86a9f3975a78a))

	ROM_REGION(0x0040, "proms", 0)
	ROM_LOAD("tm188.ic7", 0x0000, 0x0040, NO_DUMP)
ROM_END

ROM_START(brailab4)
	ROM_REGION(0x5000, "maincpu", 0)
	ROM_LOAD("brl1.ic1", 0x0000, 0x1000, CRC(02323403) SHA1(3a2e853e0a39e05a04a8db58e1a76de1eda579c9))
	ROM_LOAD("brl2.ic2", 0x1000, 0x1000, CRC(36173fbc) SHA1(1c01398e16a1cbe4103e1be769347ceae873e090))
	ROM_LOAD("brl3.ic3", 0x2000, 0x1000, CRC(d3cdd108) SHA1(1a24e6c5f9c370ff6cb25045cb9d95e664467eb5))
	ROM_LOAD("brl4.ic4", 0x3000, 0x1000, CRC(d4047885) SHA1(00fe40c4c2c64a49bb429fb2b27cc7e0d0025a85))
	ROM_LOAD("brl5.rom", 0x4000, 0x1000, CRC(8a76be04) SHA1(4b683b9be23b47117901fe874072eb7aa481e4ff))

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("hl4.ic21", 0x0000, 0x0800, CRC(f58ee39b) SHA1(49399c42d60a11b218a225856da86a9f3975a78a))

	ROM_REGION(0x0040, "proms", 0)
	ROM_LOAD("tm188.ic7", 0x0000, 0x0040, NO_DUMP)

	// these roms were found on the net, to be investigated
	ROM_REGION(0x5020, "user1", 0)
	// brl1 to 5 merged, with small changes
	// 00BF: 28 18 87 -> 30 30 0c
	// 0138: 07 0a 06 0b -> 0c 06 07 0a (keyboard assignments)
	ROM_LOAD_OPTIONAL("brl.rom", 0x0000, 0x5000, CRC(54af5d30) SHA1(d1e7b7f5866acba0503d47f610456f396526240b))
	// a small prom
	ROM_LOAD_OPTIONAL("brlcpm.rom", 0x5000, 0x0020, CRC(b936d568) SHA1(150330eccbc4b664eba4103f051d6e932038e9e8))
ROM_END

} // anonymous namespace

/* Driver */
/*   YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY                    FULLNAME                  FLAGS */
COMP(1982, homelab2, 0,        0,      homelab2, homelab2, homelab2_state, empty_init, "Jozsef and Endre Lukacs", "Homelab 2 / Aircomp 16", MACHINE_SUPPORTS_SAVE)
COMP(1983, homelab3, homelab2, 0,      homelab3, homelab3, homelab3_state, empty_init, "Jozsef and Endre Lukacs", "Homelab 3", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
COMP(1984, homelab4, homelab2, 0,      homelab3, homelab3, homelab3_state, empty_init, "Jozsef and Endre Lukacs", "Homelab 4", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
COMP(1984, brailab4, homelab2, 0,      brailab4, brailab4, homelab3_state, empty_init, "Jozsef and Endre Lukacs", "Brailab 4", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
