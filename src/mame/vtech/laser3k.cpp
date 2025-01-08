// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  laser3k.c
  Driver for VTech Laser 3000 / Dick Smith Electronics "The Cat"

  This machine is somewhat similar to a 48K Apple II if you blur your eyes
  a lot, but it generally fits in poorly with 100% compatible machines
  (no chance of a compatible language card or auxmem) so it gets its own driver.

  An "emulation cartridge" is required to run Apple II software; this appears
  to be just a language card with 16K bytes of RAM that overlay the BASIC
  ROM. The cartridge connector is based on the Apple II peripheral slot, but
  has nothing connected to the /DEVSEL pin and adds several nonstandard features.

  Banking theory:
  - 6502 has 4 banking windows, 0000-3FFF, 4000-7FFF, 8000-BFFF, C000-FFFF
  - Physical address space is 0x00000-0x3FFFF.  ROM and I/O at the top, RAM
    up to 0x2FFFF (192k max).
  - Each window has a bank number register at physical 3C07C/D/E/F

  Technical manual at:
  http://mirrors.apple2.org.za/Apple%20II%20Documentation%20Project/Computers/LASER/LASER%203000/Manuals/The%20Cat%20Technical%20Reference%20Manual.pdf

  TODO:
    - Finish keyboard
    - RGB graphics mode
    - FDC C800 page appears to be inside the FDC cartridge, need a dump :(  (can hack to use IWM in the meantime)
    - Centronics printer port (data at 3c090, read ack at 3c1c0, read busy at 3c1c2)

***************************************************************************/

#include "emu.h"

#include "bus/a2gameio/gameio.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/cassette.h"
#include "machine/bankdev.h"
#include "machine/ram.h"
#include "machine/kb3600.h"
#include "sound/sn76496.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "utf8.h"


namespace {

enum
{
	TEXT = 0,
	HIRES,
	RGB,
	DHIRES
};

#define BLACK   0
#define DKRED   1
#define DKBLUE  2
#define PURPLE  3
#define DKGREEN 4
#define DKGRAY  5
#define BLUE    6
#define LTBLUE  7
#define BROWN   8
#define ORANGE  9
#define GRAY    10
#define PINK    11
#define GREEN   12
#define YELLOW  13
#define AQUA    14
#define WHITE   15

class laser3k_state : public driver_device
{
public:
	laser3k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_ram(*this, "mainram")
		, m_bank(*this, "bank%u", 0U)
		, m_ay3600(*this, "ay3600")
		, m_speaker(*this, "speaker")
		, m_sn(*this, "sn76489")
		, m_cassette(*this, "tape")
		, m_gamepad(*this, "gamepad")
		, m_kbspecial(*this, "keyb_special")
	{ }

	void laser3k(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint8_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint8_t data);
	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);
	uint8_t io2_r(offs_t offset);

	void laser3k_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void text_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void hgr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void dhgr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);

	int ay3600_shift_r();
	int ay3600_control_r();
	void ay3600_data_ready_w(int state);

	void banks_map(address_map &map) ATTR_COLD;
	void laser3k_map(address_map &map) ATTR_COLD;

	void plot_text_character(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, const uint8_t *textgfx_data, uint32_t textgfx_datalen);
	void do_io(int offset);

	required_device<m6502_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<ram_device> m_ram;
	required_device_array<address_map_bank_device, 4> m_bank;
	required_device<ay3600_device> m_ay3600;
	required_device<speaker_sound_device> m_speaker;
	required_device<sn76489_device> m_sn;
	required_device<cassette_image_device> m_cassette;
	required_device<apple2_gameio_device> m_gamepad;
	required_ioport m_kbspecial;

	uint8_t m_bankval[4];
	int m_flash;
	int m_speaker_state;
	int m_cassette_state;
	double m_joystick_x1_time, m_joystick_y1_time, m_joystick_x2_time, m_joystick_y2_time;
	double m_x_calibration, m_y_calibration;
	int m_disp_page;
	int m_bg_color, m_fg_color, m_border_color;
	uint16_t m_lastchar, m_strobe;
	uint8_t m_transchar;
	bool m_80col;
	bool m_mix;
	int m_gfxmode;
	std::unique_ptr<uint16_t[]> m_hires_artifact_map;
	std::unique_ptr<uint16_t[]> m_dhires_artifact_map;
};

/***************************************************************************
    ADDRESS MAP
***************************************************************************/

void laser3k_state::laser3k_map(address_map &map)
{
	map(0x0000, 0x3fff).m(m_bank[0], FUNC(address_map_bank_device::amap8));
	map(0x4000, 0x7fff).m(m_bank[1], FUNC(address_map_bank_device::amap8));
	map(0x8000, 0xbfff).m(m_bank[2], FUNC(address_map_bank_device::amap8));
	map(0xc000, 0xffff).m(m_bank[3], FUNC(address_map_bank_device::amap8));
}

void laser3k_state::banks_map(address_map &map)
{
	map(0x00000, 0x2ffff).rw(FUNC(laser3k_state::ram_r), FUNC(laser3k_state::ram_w));
	map(0x38000, 0x3bfff).rom().region("maincpu", 0);
	map(0x3c000, 0x3c0ff).rw(FUNC(laser3k_state::io_r), FUNC(laser3k_state::io_w));
	map(0x3c100, 0x3c1ff).r(FUNC(laser3k_state::io2_r));
	map(0x3c200, 0x3ffff).rom().region("maincpu", 0x4200);
}

void laser3k_state::machine_start()
{
	static const uint8_t hires_artifact_color_table[] =
	{
		BLACK,  PURPLE, GREEN,  WHITE,
		BLACK,  BLUE,   ORANGE, WHITE
	};

	static const uint8_t dhires_artifact_color_table[] =
	{
		BLACK,      DKGREEN,    BROWN,  GREEN,
		DKRED,      DKGRAY,     ORANGE, YELLOW,
		DKBLUE,     BLUE,       GRAY,   AQUA,
		PURPLE,     LTBLUE,     PINK,   WHITE
	};
	int i, j;
	uint16_t c;

	/* 2^3 dependent pixels * 2 color sets * 2 offsets */
	m_hires_artifact_map = std::make_unique<uint16_t[]>(8 * 2 * 2);

	/* 2^4 dependent pixels */
	m_dhires_artifact_map = std::make_unique<uint16_t[]>(16);

	/* build hires artifact map */
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 2; j++)
		{
			if (i & 0x02)
			{
				if ((i & 0x05) != 0)
					c = 3;
				else
					c = j ? 2 : 1;
			}
			else
			{
				if ((i & 0x05) == 0x05)
					c = j ? 1 : 2;
				else
					c = 0;
			}
			m_hires_artifact_map[ 0 + j*8 + i] = hires_artifact_color_table[(c + 0) % 8];
			m_hires_artifact_map[16 + j*8 + i] = hires_artifact_color_table[(c + 4) % 8];
		}
	}

	/* build double hires artifact map */
	for (i = 0; i < 16; i++)
	{
		m_dhires_artifact_map[i] = dhires_artifact_color_table[i];
	}

	m_cassette_state = 0;
	m_cassette->output(-1.0f);

	m_x_calibration = attotime::from_nsec(10800).as_double();
	m_y_calibration = attotime::from_nsec(10800).as_double();
	m_joystick_x1_time = m_joystick_x2_time = m_joystick_y1_time = m_joystick_y2_time = 0.0;
}

void laser3k_state::machine_reset()
{
	m_bankval[0] = 0;
	m_bankval[1] = 1;
	m_bankval[2] = 2;
	m_bankval[3] = 0xf;
	m_bank[0]->set_bank(m_bankval[0]);
	m_bank[1]->set_bank(m_bankval[1]);
	m_bank[2]->set_bank(m_bankval[2]);
	m_bank[3]->set_bank(m_bankval[3]);

	m_flash = 0;
	m_speaker_state = 0;
	m_disp_page = 0;
	m_bg_color = 0;
	m_fg_color = 15;
	m_border_color = 0;
	m_strobe = 0;
	m_transchar = 0;
	m_lastchar = 0;
	m_80col = 0;
	m_mix = false;
	m_gfxmode = TEXT;

	uint8_t *rom = (uint8_t *)memregion("maincpu")->base();

	// patch out disk controller ID for now so it drops right into BASIC
	rom[0x4607] = 0;
}

uint8_t laser3k_state::ram_r(offs_t offset)
{
	return m_ram->read(offset);
}

void laser3k_state::ram_w(offs_t offset, uint8_t data)
{
	m_ram->write(offset, data);
}

// most softswitches don't care about read vs write, so handle them here
void laser3k_state::do_io(int offset)
{
	switch (offset)
	{
		case 0x08:  // set border color to black
			m_border_color = 0;
			break;
		case 0x09:  // set border color to red
			m_border_color = 1;
			break;
		case 0x0a:  // set border color to green
			m_border_color = 12;
			break;
		case 0x0b:  // set border color to yellow
			m_border_color = 13;
			break;
		case 0x0c:  // set border color to blue
			m_border_color = 6;
			break;
		case 0x0d:  // set border color to magenta
			m_border_color = 3;
			break;
		case 0x0e:  // set border color to cyan
			m_border_color = 14;
			break;
		case 0x0f:  // set border color to white
			m_border_color = 15;
			break;

		case 0x18:  // set bg color to black
			m_bg_color = 0;
			break;
		case 0x19:  // set bg color to red
			m_bg_color = 1;
			break;
		case 0x1a:  // set bg color to green
			m_bg_color = 12;
			break;
		case 0x1b:  // set bg color to yellow
			m_bg_color = 13;
			break;
		case 0x1c:  // set bg color to blue
			m_bg_color = 6;
			break;
		case 0x1d:  // set bg color to magenta
			m_bg_color = 3;
			break;
		case 0x1e:  // set bg color to cyan
			m_bg_color = 14;
			break;
		case 0x1f:  // set bg color to white
			m_bg_color = 15;
			break;

		case 0x20:
			m_cassette_state ^= 1;
			m_cassette->output(m_cassette_state ? 1.0f : -1.0f);
			break;

		case 0x28:  // set fg color to normal
			m_fg_color = 15;
			break;
		case 0x29:  // set fg color to red
			m_fg_color = 1;
			break;
		case 0x2a:  // set fg color to green
			m_fg_color = 12;
			break;
		case 0x2b:  // set fg color to yellow
			m_fg_color = 13;
			break;
		case 0x2c:  // set fg color to blue
			m_fg_color = 6;
			break;
		case 0x2d:  // set fg color to magenta
			m_fg_color = 3;
			break;
		case 0x2e:  // set fg color to cyan
			m_fg_color = 14;
			break;
		case 0x2f:  // set fg color to white
			m_fg_color = 15;
			break;

		case 0x30:
			m_speaker_state ^= 1;
			m_speaker->level_w(m_speaker_state);
			break;

		case 0x4c:  // low resolution (40 column)
			m_80col = false;
			m_maincpu->set_unscaled_clock(1021800);
			break;

		case 0x4d:  // RGB mode
			m_gfxmode = RGB;
			break;

		case 0x4e:  // double hi-res
			m_80col = true;
			m_gfxmode = DHIRES;
			m_maincpu->set_unscaled_clock(1021800*2);
			break;

		case 0x4f:  // high resolution (80 column).  Yes, the CPU clock also doubles when the pixel clock does (!)
			m_80col = true;
			m_maincpu->set_unscaled_clock(1021800*2);
			break;

		case 0x50:  // graphics mode
			m_gfxmode = HIRES;
			break;

		case 0x51:  // text mode
			m_gfxmode = TEXT;
			break;

		case 0x52:  // no mix
			m_mix = false;
			break;

		case 0x53:  // mixed mode
			m_mix = true;
			break;

		case 0x54:  // set page 1
			m_disp_page = 0;
			break;

		case 0x55:  // set page 2
			m_disp_page = 1;
			break;

		case 0x56:  // disable emulation (?)
			break;

		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:  // paddle input reset
			if (machine().time().as_double() >= m_joystick_x1_time)
				m_joystick_x1_time = machine().time().as_double() + m_x_calibration * m_gamepad->pdl0_r();
			if (machine().time().as_double() >= m_joystick_y1_time)
				m_joystick_y1_time = machine().time().as_double() + m_y_calibration * m_gamepad->pdl1_r();
			if (machine().time().as_double() >= m_joystick_x2_time)
				m_joystick_x2_time = machine().time().as_double() + m_x_calibration * m_gamepad->pdl2_r();
			if (machine().time().as_double() >= m_joystick_y2_time)
				m_joystick_y2_time = machine().time().as_double() + m_y_calibration * m_gamepad->pdl3_r();
			break;

		default:
			printf("do_io: unknown softswitch @ %x\n", offset);
			break;
	}
}

uint8_t laser3k_state::io_r(offs_t offset)
{
	switch (offset)
	{
		case 0x00:  // keyboard latch
			return m_transchar | m_strobe;

		case 0x10:  // keyboard strobe
			{
				uint8_t rv = m_transchar | m_strobe;
				if (!machine().side_effects_disabled())
					m_strobe = 0;
				return rv;
			}

		case 0x60: // cassette in
			return m_cassette->input() > 0.0 ? 0x80 : 0;

		case 0x61:  // button 0
			return m_gamepad->sw0_r() ? 0x80 : 0;

		case 0x62:  // button 1
			return m_gamepad->sw1_r() ? 0x80 : 0;

		case 0x63:  // button 2
			return m_gamepad->sw2_r() ? 0x80 : 0;

		case 0x64:  // joy 1 X axis
			if (!m_gamepad->is_device_connected()) return 0x80;
			return (machine().time().as_double() < m_joystick_x1_time) ? 0x80 : 0;

		case 0x65:  // joy 1 Y axis
			if (!m_gamepad->is_device_connected()) return 0x80;
			return (machine().time().as_double() < m_joystick_y1_time) ? 0x80 : 0;

		case 0x66: // joy 2 X axis
			if (!m_gamepad->is_device_connected()) return 0x80;
			return (machine().time().as_double() < m_joystick_x2_time) ? 0x80 : 0;

		case 0x67: // joy 2 Y axis
			if (!m_gamepad->is_device_connected()) return 0x80;
			return (machine().time().as_double() < m_joystick_y2_time) ? 0x80 : 0;

		case 0x7c:
			return m_bankval[0];

		case 0x7d:
			return m_bankval[1];

		case 0x7e:
			return m_bankval[2];

		case 0x7f:
			return m_bankval[3];

		default:
			if (!machine().side_effects_disabled())
				do_io(offset);
			break;
	}

	return 0xff;
}

void laser3k_state::io_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0x10:  // clear keyboard latch
			m_strobe = 0;
			break;

		case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:  // SN76489 sound
			m_sn->write(data);
			break;

		case 0x78:  // called "SYSTEM" in the boot ROM listing, but unsure what it does
			break;

		case 0x7c:  // bank 0
			m_bankval[0] = data & 0xf;
			m_bank[0]->set_bank(m_bankval[0]);
			break;

		case 0x7d:  // bank 1
			m_bankval[1] = data & 0xf;
			m_bank[1]->set_bank(m_bankval[1]);
			break;

		case 0x7e:  // bank 2
			m_bankval[2] = data & 0xf;
			m_bank[2]->set_bank(m_bankval[2]);
			break;

		case 0x7f:  // bank 3
			m_bankval[3] = data & 0xf;
			m_bank[3]->set_bank(m_bankval[3]);
			break;

		default:
			do_io(offset);
			break;
	}
}

uint8_t laser3k_state::io2_r(offs_t offset)
{
	switch (offset)
	{
		case 0xc2:  // h-blank status
			return m_screen->hblank() ? 0x80 : 0x00;

		case 0xc3:  // v-blank status
			return m_screen->vblank() ? 0x80 : 0x00;

		case 0xc5:  // CPU 1/2 MHz status?
			return 0x00;

		default:
			printf("io2_r @ unknown %x\n", offset);
			break;
	}

	return 0xff;
}

void laser3k_state::plot_text_character(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code,
	const uint8_t *textgfx_data, uint32_t textgfx_datalen)
{
	int fg = m_fg_color;
	int bg = m_bg_color;

	/* look up the character data */
	uint8_t const *const chardata = &textgfx_data[(code * 8) % textgfx_datalen];

	if (m_flash && (code >= 0x40) && (code <= 0x7f))
	{
		/* we're flashing; swap */
		using std::swap;
		swap(fg, bg);
	}

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 7; x++)
		{
			uint16_t color = (chardata[y] & (1 << (6-x))) ? fg : bg;

			for (int i = 0; i < xscale; i++)
			{
				bitmap.pix(ypos + y, xpos + (x * xscale) + i) = color;
			}
		}
	}
}

void laser3k_state::text_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col;
	uint32_t start_address;
	uint32_t address;
	uint8_t *m_a2_videoram = m_ram->pointer();

	if (m_80col)
	{
		start_address = (m_disp_page == 0) ? 0x1000 : 0x1800;
	}
	else
	{
		start_address = (m_disp_page == 0) ? 0x400 : 0x800;
	}

	m_flash = ((machine().time() * 4).seconds() & 1) ? 1 : 0;

	beginrow = std::max(beginrow, cliprect.min_y - (cliprect.min_y % 8));
	endrow = std::min(endrow, cliprect.max_y - (cliprect.max_y % 8) + 7);

	for (row = beginrow; row <= endrow; row += 8)
	{
		if (m_80col)
		{
			for (col = 0; col < 40; col++)
			{
				/* calculate address */
				address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));

				plot_text_character(bitmap, col * 7, row, 1, m_a2_videoram[address],
					memregion("gfx1")->base(), memregion("gfx1")->bytes());
				plot_text_character(bitmap, (col + 40) * 7, row, 1, m_a2_videoram[address+0x400],
					memregion("gfx1")->base(), memregion("gfx1")->bytes());
			}
		}
		else
		{
			for (col = 0; col < 40; col++)
			{
				/* calculate address */
				address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));
				plot_text_character(bitmap, col * 14, row, 2, m_a2_videoram[address],
					memregion("gfx1")->base(), memregion("gfx1")->bytes());
			}
		}
	}
}

void laser3k_state::hgr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	/* sanity checks */
	if (beginrow < cliprect.min_y)
		beginrow = cliprect.min_y;
	if (endrow > cliprect.max_y)
		endrow = cliprect.max_y;
	if (endrow < beginrow)
		return;

	uint8_t const *const vram = m_ram->pointer() + (m_disp_page ? 0x4000 : 0x2000);

	uint8_t vram_row[42];
	vram_row[0] = 0;
	vram_row[41] = 0;

	for (int row = beginrow; row <= endrow; row++)
	{
		for (int col = 0; col < 40; col++)
		{
			int offset = ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col)) | ((row & 7) << 10);
			vram_row[1+col] = vram[offset];
		}

		uint16_t *p = &bitmap.pix(row);

		for (int col = 0; col < 40; col++)
		{
			uint32_t w =    (((uint32_t) vram_row[col+0] & 0x7f) <<  0)
						|   (((uint32_t) vram_row[col+1] & 0x7f) <<  7)
						|   (((uint32_t) vram_row[col+2] & 0x7f) << 14);

			uint16_t const *const artifact_map_ptr = &m_hires_artifact_map[((vram_row[col+1] & 0x80) >> 7) * 16];
			for (int b = 0; b < 7; b++)
			{
				uint16_t v = artifact_map_ptr[((w >> (b + 7-1)) & 0x07) | (((b ^ col) & 0x01) << 3)];
				*(p++) = v;
				*(p++) = v;
			}
		}
	}
}

void laser3k_state::dhgr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	/* sanity checks */
	if (beginrow < cliprect.min_y)
		beginrow = cliprect.min_y;
	if (endrow > cliprect.max_y)
		endrow = cliprect.max_y;
	if (endrow < beginrow)
		return;

	uint8_t const *const vram = m_ram->pointer() + (m_disp_page ? 0x8000 : 0x4000);

	uint8_t vram_row[82];
	vram_row[0] = 0;
	vram_row[81] = 0;

	for (int row = beginrow; row <= endrow; row++)
	{
		for (int col = 0; col < 40; col++)
		{
			int offset = ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col)) | ((row & 7) << 10);
			if (col < 40)
			{
				vram_row[1+(col*2)+0] = vram[offset];
				vram_row[1+(col*2)+1] = vram[offset+1];
			}
			else
			{
				// FIXME: this is unreachable - loop bound is checks (col < 40) so these columns can never be copied
				vram_row[1+(col*2)+0] = vram[offset+0x2000];
				vram_row[1+(col*2)+1] = vram[offset+0x2001];
			}
		}

		uint16_t *p = &bitmap.pix(row);

		for (int col = 0; col < 80; col++)
		{
			uint32_t w =    (((uint32_t) vram_row[col+0] & 0x7f) <<  0)
						|   (((uint32_t) vram_row[col+1] & 0x7f) <<  7)
						|   (((uint32_t) vram_row[col+2] & 0x7f) << 14);

			for (int b = 0; b < 7; b++)
			{
				uint16_t v = m_dhires_artifact_map[((((w >> (b + 7-1)) & 0x0F) * 0x11) >> (((2-(col*7+b))) & 0x03)) & 0x0F];
				*(p++) = v;
			}
		}
	}
}

uint32_t laser3k_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	switch (m_gfxmode)
	{
		case TEXT:
			text_update(screen, bitmap, cliprect, 0, 191);
			break;

		case HIRES:
			if (m_mix)
			{
				hgr_update(screen, bitmap, cliprect, 0, 159);
				text_update(screen, bitmap, cliprect, 160, 191);
			}
			else
			{
				hgr_update(screen, bitmap, cliprect, 0, 191);
			}
			break;

		case RGB:
			break;

		case DHIRES:
			if (m_mix)
			{
				dhgr_update(screen, bitmap, cliprect, 0, 159);
				text_update(screen, bitmap, cliprect, 160, 191);
			}
			else
			{
				dhgr_update(screen, bitmap, cliprect, 0, 191);
			}
			break;
	}

	return 0;
}

int laser3k_state::ay3600_shift_r()
{
	// either shift key
	if (m_kbspecial->read() & 0x06)
	{
		return ASSERT_LINE;
	}

	return CLEAR_LINE;
}

int laser3k_state::ay3600_control_r()
{
	if (m_kbspecial->read() & 0x08)
	{
		return ASSERT_LINE;
	}

	return CLEAR_LINE;
}

static const uint8_t key_remap[0x32][4] =
{
/*    norm shft ctrl both */
	{ 0x33,0x23,0x33,0x23 },    /* 3 #     00     */
	{ 0x34,0x24,0x34,0x24 },    /* 4 $     01     */
	{ 0x35,0x25,0x35,0x25 },    /* 5 %     02     */
	{ 0x36,0x5e,0x35,0x53 },    /* 6 ^     03     */
	{ 0x37,0x26,0x37,0x26 },    /* 7 &     04     */
	{ 0x38,0x2a,0x38,0x2a },    /* 8 *     05     */
	{ 0x39,0x28,0x39,0x28 },    /* 9 (     06     */
	{ 0x30,0x29,0x30,0x29 },    /* 0 )     07     */
	{ 0x3b,0x3a,0x3b,0x3a },    /* ; :     08     */
	{ 0x2d,0x5f,0x2d,0x1f },    /* - _     09     */
	{ 0x51,0x51,0x11,0x11 },    /* q Q     0a     */
	{ 0x57,0x57,0x17,0x17 },    /* w W     0b     */
	{ 0x45,0x45,0x05,0x05 },    /* e E     0c     */
	{ 0x52,0x52,0x12,0x12 },    /* r R     0d     */
	{ 0x54,0x54,0x14,0x14 },    /* t T     0e     */
	{ 0x59,0x59,0x19,0x19 },    /* y Y     0f     */
	{ 0x55,0x55,0x15,0x15 },    /* u U     10     */
	{ 0x49,0x49,0x09,0x09 },    /* i I     11     */
	{ 0x4f,0x4f,0x0f,0x0f },    /* o O     12     */
	{ 0x50,0x50,0x10,0x10 },    /* p P     13     */
	{ 0x44,0x44,0x04,0x04 },    /* d D     14     */
	{ 0x46,0x46,0x06,0x06 },    /* f F     15     */
	{ 0x47,0x47,0x07,0x07 },    /* g G     16     */
	{ 0x48,0x48,0x08,0x08 },    /* h H     17     */
	{ 0x4a,0x4a,0x0a,0x0a },    /* j J     18     */
	{ 0x4b,0x4b,0x0b,0x0b },    /* k K     19     */
	{ 0x4c,0x4c,0x0c,0x0c },    /* l L     1a     */
	{ 0x3d,0x2b,0x3d,0x2b },    /* = +     1b     */
	{ 0x08,0x08,0x08,0x08 },    /* Left    1c     */
	{ 0x15,0x15,0x15,0x15 },    /* Right   1d     */
	{ 0x5a,0x5a,0x1a,0x1a },    /* z Z     1e     */
	{ 0x58,0x58,0x18,0x18 },    /* x X     1f     */
	{ 0x43,0x43,0x03,0x03 },    /* c C     20     */
	{ 0x56,0x56,0x16,0x16 },    /* v V     21     */
	{ 0x42,0x42,0x02,0x02 },    /* b B     22     */
	{ 0x4e,0x4e,0x0e,0x0e },    /* n N     23     */
	{ 0x4d,0x4d,0x0d,0x0d },    /* m M     24     */
	{ 0x2c,0x3c,0x2c,0x3c },    /* , <     25     */
	{ 0x2e,0x3e,0x2e,0x3e },    /* . >     26     */
	{ 0x2f,0x3f,0x2f,0x3f },    /* / ?     27     */
	{ 0x53,0x53,0x13,0x13 },    /* s S     28     */
	{ 0x32,0x40,0x32,0x00 },    /* 2 @     29     */
	{ 0x31,0x21,0x31,0x31 },    /* 1 !     2a     */
	{ 0x9b,0x9b,0x9b,0x9b },    /* Escape  2b     */
	{ 0x41,0x41,0x01,0x01 },    /* a A     2c     */
	{ 0x20,0x20,0x20,0x20 },    /* Space   2d     */
	{ 0x27,0x22,0x27,0x22 },    /* ' "     2e     */
	{ 0x00,0x00,0x00,0x00 },    /* 0x2f unused    */
	{ 0x00,0x00,0x00,0x00 },    /* 0x30 unused    */
	{ 0x0d,0x0d,0x0d,0x0d },    /* Enter   31     */
};

void laser3k_state::ay3600_data_ready_w(int state)
{
	if (state == ASSERT_LINE)
	{
		int mod = 0;
		m_lastchar = m_ay3600->b_r();

		mod = (m_kbspecial->read() & 0x06) ? 0x01 : 0x00;
		mod |= (m_kbspecial->read() & 0x08) ? 0x02 : 0x00;

//      printf("lastchar = %02x\n", m_lastchar & 0x3f);

		m_transchar = key_remap[m_lastchar&0x3f][mod];

		if (m_transchar != 0)
		{
			m_strobe = 0x80;
//          printf("new char = %04x (%02x)\n", m_lastchar&0x3f, m_transchar);
		}
	}
}

/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( laser3k )
	PORT_START("X0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)  PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)  PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)  PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)      PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)  PORT_CHAR('-') PORT_CHAR('_')

	PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)  PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)  PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)  PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)  PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)  PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)  PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)  PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)  PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)  PORT_CHAR('P') PORT_CHAR('p')

	PORT_START("X2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)  PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)  PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)  PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)  PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)  PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)  PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)      PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)     PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)  PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)  PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)  PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)  PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)  PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)  PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)      PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")      PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)  PORT_CHAR('\'') PORT_CHAR('\"')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")   PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)

	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X8")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keyb_special")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")  PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET")        PORT_CODE(KEYCODE_F12)
INPUT_PORTS_END

// this is an apple II palette; it seems more likely the
// actual laser3000 has a digital RGB palette...
static constexpr rgb_t laser3k_pens[] =
{
	{ 0x00, 0x00, 0x00 }, // Black
	{ 0xe3, 0x1e, 0x60 }, // Dark Red
	{ 0x60, 0x4e, 0xbd }, // Dark Blue
	{ 0xff, 0x44, 0xfd }, // Purple
	{ 0x00, 0xa3, 0x60 }, // Dark Green
	{ 0x9c, 0x9c, 0x9c }, // Dark Gray
	{ 0x14, 0xcf, 0xfd }, // Medium Blue
	{ 0xd0, 0xc3, 0xff }, // Light Blue
	{ 0x60, 0x72, 0x03 }, // Brown
	{ 0xff, 0x6a, 0x3c }, // Orange
	{ 0x9c, 0x9c, 0x9c }, // Light Grey
	{ 0xff, 0xa0, 0xd0 }, // Pink
	{ 0x14, 0xf5, 0x3c }, // Light Green
	{ 0xd0, 0xdd, 0x8d }, // Yellow
	{ 0x72, 0xff, 0xd0 }, // Aquamarine
	{ 0xff, 0xff, 0xff }  // White
};

/* Initialize the palette */
void laser3k_state::laser3k_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, laser3k_pens);
}

void laser3k_state::laser3k(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 14_MHz_XTAL / 7); // 1 or 2 MHz depending on soft switch
	m_maincpu->set_addrmap(AS_PROGRAM, &laser3k_state::laser3k_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(300*2, 192);
	m_screen->set_visarea(0, (280*2)-1,0,192-1);
	m_screen->set_screen_update(FUNC(laser3k_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(laser3k_state::laser3k_palette), std::size(laser3k_pens));

	/* memory banking */
	for (auto &bank : m_bank)
		ADDRESS_MAP_BANK(config, bank).set_map(&laser3k_state::banks_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x4000);

	RAM(config, "mainram").set_default_size("192K");

	APPLE2_GAMEIO(config, m_gamepad, apple2_gameio_device::joystick_options, nullptr);
	m_gamepad->set_sw_pullups(true); // 3K9 pullups to +5V

	/* the 8048 isn't dumped, so substitute modified real Apple II h/w */
	AY3600(config, m_ay3600, 0);
	m_ay3600->x0().set_ioport("X0");
	m_ay3600->x1().set_ioport("X1");
	m_ay3600->x2().set_ioport("X2");
	m_ay3600->x3().set_ioport("X3");
	m_ay3600->x4().set_ioport("X4");
	m_ay3600->x5().set_ioport("X5");
	m_ay3600->x6().set_ioport("X6");
	m_ay3600->x7().set_ioport("X7");
	m_ay3600->x8().set_ioport("X8");
	m_ay3600->shift().set(FUNC(laser3k_state::ay3600_shift_r));
	m_ay3600->control().set(FUNC(laser3k_state::ay3600_control_r));
	m_ay3600->data_ready().set(FUNC(laser3k_state::ay3600_data_ready_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);
	SN76489(config, m_sn, 14_MHz_XTAL / 7).add_route(ALL_OUTPUTS, "mono", 0.50);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED);
	m_cassette->set_interface("apple2_cass");
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
}

ROM_START(las3000)
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD ( "341-0036.chr", 0x0000, 0x0800, CRC(64f415c6) SHA1(f9d312f128c9557d9d6ac03bfad6c3ddf83e5659))

	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD ( "las3000.rom", 0x0000, 0x8000, CRC(9c7aeb09) SHA1(3302adf41e258cf50210c19736948c8fa65e91de))

	ROM_REGION(0x100, "fdc", 0)
	ROM_LOAD ( "l3kdisk.rom", 0x0000, 0x0100, CRC(2d4b1584) SHA1(989780b77e100598124df7b72663e5a31a3339c0))
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY             FULLNAME      FLAGS
COMP( 1983, las3000, 0,      0,      laser3k, laser3k, laser3k_state, empty_init, "Video Technology", "Laser 3000", MACHINE_NOT_WORKING )
