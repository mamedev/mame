// license:BSD-3-Clause
// copyright-holders: Dirk Best
/****************************************************************************

    Qume QVT-70/QVT-82 terminal

    QVT-70:
    - Z80 (Z8040008VSC)
    - Z80 DART (Z0847006PSC)
    - QUME 303489-01 QFP144
    - DTC 801000-02 QFP100
    - ROM 128k + 64k
    - CXK5864CM-70LL (8k, next to ROMs)
    - W242575-70LL (32k) + 4x CXK5864CM-70LL (8k)
    - DS1231
    - Beeper + Battery
    - 54.2857MHz XTAL

    Features:
    - 65 hz with 16x16 characters
    - 78 hz with 16x13 characters
    - 64 background/foreground colors
    - 80/132 columns

    QVT-82:
    - Z80 (Z0840008PSC)
    - Z80 DART (Z0847006PSC)
    - QUME 303489-01 QFP144
    - ROM 64k * 2
    - RAM 8k UM6264AK-10L (above Z80) + 8k UM6264K-70L * 3 (below Z80)
    - DS1231
    - 54.2857MHz XTAL
    - Battery

    Notes:
    - Everything here is guessed, no technical manual available

    TODO:
    - Line drawing characters in 132-column mode
    - Unknown ASIC registers
    - Better screen raw parameters
    - Other keyboard models (ANSI, ASCII)

****************************************************************************/

#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/at_ssrt.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "machine/z80sio.h"
#include "sound/beep.h"

#include "emupal.h"
#include "multibyte.h"
#include "screen.h"
#include "speaker.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class qvt70_state : public driver_device
{
public:
	qvt70_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dart(*this, "dart"),
		m_brg(*this, "brg%d", 1U),
		m_rs232(*this, "serial%d", 1U),
		m_rombank(*this, "rom"),
		m_rambank(*this, "ram%d", 0U),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_beeper(*this, "beeper"),
		m_ssrt(*this, "ssrt"),
		m_kbdc(*this, "kbdc"),
		m_centronics(*this, "parallel"),
		m_centronics_latch(*this, "centronics_latch")
	{ }

	void qvt70(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	enum : unsigned
	{
		NMI_ROW = 0,
		NMI_VBLANK,
		NMI_KBD_TX,
		NMI_KBD_RX
	};

	required_device<z80_device> m_maincpu;
	required_device<z80dart_device> m_dart;
	required_device_array<clock_device, 2> m_brg;
	required_device_array<rs232_port_device, 2> m_rs232;
	required_memory_bank m_rombank;
	required_memory_bank_array<2> m_rambank;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<beep_device> m_beeper;
	required_device<at_ssrt_device> m_ssrt;
	required_device<pc_kbdc_device> m_kbdc;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_centronics_latch;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint32_t font_base() const { return 0x8000 + m_connection * 0x2000; }

	void update_screenmode();
	void update_row_timer();
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void palette_init(palette_device &palette) const;
	void vblank_w(int state);
	TIMER_CALLBACK_MEMBER(row_interrupt);

	void update_rambanks();

	void character_w(offs_t offset, uint8_t data);
	void auxiliary_w(offs_t offset, uint8_t data);

	void centronics_busy_w(int state);
	void centronics_fault_w(int state);
	void keyboard_rx_complete_w(int state);
	void keyboard_tx_complete_w(int state);
	void update_baudrate(unsigned port);

	uint8_t asic_r(offs_t offset);
	void asic_w(offs_t offset, uint8_t data);

	void rombank_w(uint8_t data);

	std::unique_ptr<uint8_t[]> m_ram;

	emu_timer *m_row_timer = nullptr;

	uint8_t m_asic[0x80];
	uint8_t m_nmi_state = 0;
	uint8_t m_connection = 0;
	uint8_t m_scroll_top = 0;
	uint8_t m_scroll_bottom = 0;
	bool m_centronics_busy = false;
	bool m_centronics_fault = false;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void qvt70_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).bankr(m_rombank);
	map(0x8000, 0x8000).w(FUNC(qvt70_state::rombank_w));
	map(0x9000, 0x9000).w("centronics_latch", FUNC(output_latch_device::write));
	map(0xa000, 0xbfff).ram().share("nvram");
	map(0xc000, 0xdfff).bankr(m_rambank[0]).w(FUNC(qvt70_state::character_w));
	map(0xe000, 0xffff).bankr(m_rambank[1]).w(FUNC(qvt70_state::auxiliary_w));
}

void qvt70_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x7f).rw(FUNC(qvt70_state::asic_r), FUNC(qvt70_state::asic_w));
	map(0x80, 0x83).rw(m_dart, FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void qvt70_state::update_screenmode()
{
	// bank2:768a programs the ASIC for the new screen mode
	bool const mode_80 = BIT(m_asic[0x1e], 6);
	int const columns = m_asic[mode_80 ? 0x10 : 0x11];
	int const char_width = mode_80 ? 16 : 10;
	int const char_height = m_asic[0x19] + 1;
	int const htotal = (m_asic[0x0c] + 1) * char_width;
	int const hvisible = columns * char_width;
	int const vvisible = (m_asic[0x13] + 1) * char_height;

	// TODO: decode vertical timing registers 12/14/16/69
	// the values below are derived from the pixel clock
	int const vtotal = char_height == 13 ? 418 : 502;

	// only try to update if we have valid values
	if (!columns || char_height > 16 || htotal <= hvisible || vtotal <= vvisible)
		return;

	rectangle const visible(0, hvisible - 1, 0, vvisible - 1);
	attotime const period = attotime::from_ticks(uint64_t(htotal) * vtotal, m_screen->clock());

	// only set a new mode if it actually changed
	if (m_screen->width() == htotal && m_screen->height() == vtotal &&
		m_screen->visible_area() == visible && m_screen->frame_period() == period)
		return;

	logerror("Screen mode: %dx%d (visible: %dx%d)\n", htotal, vtotal, hvisible, vvisible);
	m_screen->configure(htotal, vtotal, visible, period);
}

void qvt70_state::update_row_timer()
{
	m_nmi_state &= ~(1 << NMI_ROW);

	int const row = int(m_asic[0x13]) - m_asic[0x1a];
	int const char_height = m_asic[0x19] + 1;

	if (row < 0 || char_height > 16 || row * char_height >= m_screen->height())
		m_row_timer->adjust(attotime::never);
	else
		m_row_timer->adjust(m_screen->time_until_pos(row * char_height));
}

uint32_t qvt70_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pen = m_palette->pens();

	bitmap.fill(pen[0], cliprect);

	// display enable
	if (BIT(m_asic[0x1d], 3) == 0)
		return 0;

	bool const mode_80 = BIT(m_asic[0x1e], 6);
	int const columns = m_asic[mode_80 ? 0x10 : 0x11];

	int const char_height = m_asic[0x19] + 1;

	if (char_height > 16)
		return 0;

	int const scroll_offset = m_asic[0x18] & 0x0f;
	int const scroll_top_line = m_scroll_top * char_height;
	int const scroll_bottom_line = ((m_scroll_bottom + 1) * char_height) - 1;

	bool const field_attributes = BIT(m_asic[0x1e], 3);
	int const page = BIT(m_asic[0x51], 2);

	bool const blink = bool(m_screen->frame_number() & 0x10); // timing?
	bool const reverse_screen = (m_asic[0x38] & 0xc0) == 0xc0;

	uint16_t const cursor_address = ((m_asic[0x0b] << 8) | m_asic[0x0a]) & 0x1fff;
	bool const cursor_visible = BIT(m_asic[0x2a], 7) && (!BIT(m_asic[0x2a], 2) || !blink);

	uint16_t line_addr = 0;
	uint16_t line_data_addr = 0;
	uint8_t line_attr = 0;
	uint16_t line_next_addr = 0;

	int y = 0;
	int chargen_line = 0;

	while (y <= cliprect.bottom())
	{
		for (int i = 0; i < char_height; i++)
		{
			int const py = y + i;

			// on the first line of the character fetch the new line data for this row
			if (chargen_line == 0)
			{
				// line attributes
				// 7-------  unknown
				// -6------  unknown
				// --5-----  unknown
				// ---4----  fixed line (no scrolling)
				// ----3---  unknown
				// -----2--  double height lower/upper half
				// ------1-  double height line
				// -------0  double width line

				uint8_t const *line = &m_ram[page * 0x2000 + line_addr];

				line_data_addr = get_u16le(&line[0]) & 0x1fff;
				line_attr = line[2];
				line_next_addr = get_u16le(&line[3]) & 0x1fff;

				if (0)
					logerror("line %d addr = %04x, next = %04x, data = %04x, control = %02x\n",
						y + i, line_addr, line_next_addr, line_data_addr, line_attr);

				if (line_addr != line_next_addr)
					line_addr = line_next_addr;
			}

			// check for the start of the scrolling region
			if (py == scroll_top_line && (BIT(line_attr, 4) == 0))
				chargen_line = (chargen_line + scroll_offset) % char_height;

			int chargen_line_final = chargen_line;

			// double-width?
			bool const dw = BIT(line_attr, 0);
			int dw_start = 0; // toggles between 0 and 4 for double-width characters

			// double-height?
			if (BIT(line_attr, 1))
			{
				chargen_line_final /= 2;

				// double-height, upper half
				if (BIT(line_attr, 2) == 0)
					chargen_line_final += char_height / 2;
			}

			bool const cursor_line = (chargen_line >= m_asic[0x33]) && (chargen_line <= m_asic[0x34]);

			if (py >= cliprect.top() && py <= cliprect.bottom())
			{
				uint8_t field_attr = 0;
				uint8_t field_fg = 0;
				uint8_t field_bg = 0;

				for (int x = 0; x < columns; x++)
				{
					// fetch character data
					uint16_t const char_addr = (line_data_addr + x) & 0x1fff;

					uint16_t code = m_ram[0x0000 + char_addr];
					uint8_t attr = m_ram[0x2000 + char_addr];
					uint8_t fg = m_ram[0x4000 + char_addr];
					uint8_t bg = m_ram[0x6000 + char_addr];

					if (field_attributes)
					{
						// bit 6 indicates a field boundary
						if (BIT(attr, 6))
						{
							field_attr = attr;
							field_fg = fg;
							field_bg = bg;
						}

						attr = field_attr;
						fg = field_fg;
						bg = field_bg;
					}

					fg &= 0x3f;
					bg &= 0x3f;

					// reverse video?
					if (reverse_screen)
					{
						using std::swap;
						swap(fg, bg);
					}

					// attribute ram
					// 7-------  unknown
					// -6------  char address bit 8 / field boundary in field mode
					// --5-----  unknown
					// ---4----  unknown
					// ----3---  underline
					// -----2--  reverse
					// ------1-  blink
					// -------0  concealed

					if (!field_attributes && BIT(attr, 6))
						code |= 0x100;

					uint8_t data = m_ram[font_base() + (code << 4) + chargen_line_final];

					bool underline = false;

					// underline
					if (BIT(attr, 3) && chargen_line == m_asic[0x3c])
					{
						underline = true;
						data = 0xff;
					}

					// concealed
					if (BIT(attr, 0))
						data = 0x00;

					// blink
					if (blink && BIT(attr, 1))
						data = 0x00;

					// reverse
					if (BIT(attr, 2))
						data ^= 0xff;

					// cursor
					if (((line_data_addr + x) & 0x1fff) == cursor_address && cursor_line && cursor_visible)
						data = 0xff;

					int px = x * (mode_80 ? 16 : 10);

					auto put_pixel = [&](pen_t color)
					{
						if (cliprect.contains(px, py))
							bitmap.pix(py, px) = color;
						px++;
					};

					if (mode_80)
					{
						// 80 columns: double each glyph pixel to make a 16-pixel cell

						for (int p = 0; p < 8; p++)
						{
							int const glyph_bit = dw ? ((p / 2) + dw_start) : p;
							pen_t const color = BIT(data, 7 - glyph_bit) ? pen[fg] : pen[bg];

							put_pixel(color);
							put_pixel(color);
						}
					}
					else
					{
						// 132 columns: eight glyph pixels in a 10-pixel cell
						// TODO: line drawing characters

						pen_t const spacing_color = underline ? pen[fg] : pen[bg];

						// empty pixel before the glyph
						if (!dw || dw_start == 0)
							put_pixel(spacing_color);

						// double-width left half, add an additional empty pixel
						if (dw && dw_start == 0)
							put_pixel(spacing_color);

						// 8 glyph pixels
						for (int p = 0; p < 8; p++)
						{
							int const glyph_bit = dw ? ((p / 2) + dw_start) : p;
							pen_t const color = BIT(data, 7 - glyph_bit) ? pen[fg] : pen[bg];

							put_pixel(color);
						}

						// empty pixel after the glyph
						if (!dw || dw_start == 4)
							put_pixel(spacing_color);

						// double-width right half, add an additional empty pixel
						if (dw && dw_start == 4)
							put_pixel(spacing_color);
					}

					dw_start = 4 - dw_start;
				}
			}

			// check for the end of the scrolling region
			if (py == scroll_bottom_line)
				chargen_line = 0;
			else
				chargen_line = (chargen_line + 1) % char_height;
		}

		y += char_height;
	}

	return 0;
}

void qvt70_state::vblank_w(int state)
{
	m_nmi_state &= ~(1 << NMI_VBLANK);
	m_nmi_state |= state << NMI_VBLANK;

	if (state && BIT(m_asic[0x42], NMI_VBLANK))
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

TIMER_CALLBACK_MEMBER( qvt70_state::row_interrupt )
{
	m_nmi_state |= 1 << NMI_ROW;

	if (BIT(m_asic[0x42], NMI_ROW))
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void qvt70_state::palette_init(palette_device &palette) const
{
	for (unsigned i = 0; i < 64; i++)
	{
		int b = (i >> 4) & 3;
		int g = (i >> 2) & 3;
		int r = (i >> 0) & 3;

		palette.set_pen_color(i, rgb_t(r * 0x55, g * 0x55, b * 0x55));
	}
}

static const gfx_layout char_layout_8x16 =
{
	8, 16,
	512,
	1,
	{ 0 },
	{ STEP8(0, 1) },
	{ STEP16(0, 8) },
	8*16
};

static const gfx_layout char_layout_8x9 =
{
	8, 9,
	512,
	1,
	{ 0 },
	{ STEP8(0, 1) },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8 },
	8*16
};

static GFXDECODE_START(chars)
	GFXDECODE_RAM(nullptr, 0, char_layout_8x16, 0, 1)
	GFXDECODE_RAM(nullptr, 0, char_layout_8x9, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void qvt70_state::update_rambanks()
{
	// currently assumed ram bank layout:
	// 0 0x0000 characters
	// 1 0x2000 attributes
	// 2 0x4000 foreground color
	// 3 0x6000 background color
	// 4 0x8000 fonts (connection 1)
	// 5 0xa000 fonts (connection 2)
	// 6 0xc000 data (connection 1)
	// 7 0xe000 data (connection 2)

	// port 60 switches the bank at 0xc000
	m_rambank[0]->set_entry(BIT(m_asic[0x60], 7) ? 2 : 0);

	// port 1d, 1e and 60 switch the bank at 0xe000
	if (BIT(m_asic[0x1e], 2))
		m_rambank[1]->set_entry((BIT(m_asic[0x1d], 6) ? 6 : 4) + m_connection);
	else
		m_rambank[1]->set_entry(BIT(m_asic[0x60], 7) ? 3 : 1);

	m_gfxdecode->gfx(0)->set_source(&m_ram[font_base()]);
	m_gfxdecode->gfx(1)->set_source(&m_ram[font_base() + 0x1000]);
}

void qvt70_state::character_w(offs_t offset, uint8_t data)
{
	static_cast<uint8_t *>(m_rambank[0]->base())[offset] = data;

	// on write to the character ram forward latched values
	if (!BIT(m_asic[0x60], 7) && BIT(m_asic[0x1d], 2))
	{
		m_ram[0x2000 + offset] = m_asic[0x17]; // attribute
		m_ram[0x4000 + offset] = m_asic[0x63]; // foreground color
		m_ram[0x6000 + offset] = m_asic[0x64]; // background color
	}
}

void qvt70_state::auxiliary_w(offs_t offset, uint8_t data)
{
	static_cast<uint8_t *>(m_rambank[1]->base())[offset] = data;

	if (BIT(m_asic[0x1e], 2) && !BIT(m_asic[0x1d], 6))
	{
		m_gfxdecode->gfx(0)->mark_dirty(offset >> 4);
		if (offset >= 0x1000)
			m_gfxdecode->gfx(1)->mark_dirty((offset - 0x1000) >> 4);
	}
}

void qvt70_state::centronics_busy_w(int state)
{
	m_centronics_busy = bool(state);
}

void qvt70_state::centronics_fault_w(int state)
{
	m_centronics_fault = bool(state);
}

void qvt70_state::keyboard_rx_complete_w(int state)
{
	m_nmi_state &= ~(1 << NMI_KBD_RX);
	m_nmi_state |= state << NMI_KBD_RX;

	if (state && BIT(m_asic[0x42], NMI_KBD_RX))
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void qvt70_state::keyboard_tx_complete_w(int state)
{
	m_nmi_state &= ~(1 << NMI_KBD_TX);
	m_nmi_state |= state << NMI_KBD_TX;

	if (state && BIT(m_asic[0x42], NMI_KBD_TX))
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void qvt70_state::update_baudrate(unsigned port)
{
	uint16_t ticks = 0;

	switch (port)
	{
		case 0:
			ticks = (((m_asic[0x32] & 0x1f) << 8) | m_asic[0x2f]) + 4;
			break;
		case 1:
			ticks = (((m_asic[0x37] & 0x1f) << 8) | m_asic[0x36]) + 4;
			break;
		default:
			fatalerror("Invalid serial port\n");
	}

	if (ticks)
		m_brg[port]->set_period(attotime::from_ticks(ticks, 54.2857_MHz_XTAL / 4));
	else
		m_brg[port]->set_period(attotime::never);
}

uint8_t qvt70_state::asic_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
		case 0x1d:
			// nmi status
			// 7654----  unknown
			// ----3---  keyboard receive complete
			// -----2--  keyboard transmit complete
			// ------1-  vblank
			// -------0  character-row compare match
			data = m_nmi_state;
			break;

		case 0x1e:
			// peripheral status (bits 7/6 both high = printer ready)
			// 7-------  printer present/ready (/ERROR?)
			// -6------  printer ready (/BUSY?)
			// --54----  unknown
			// ----3---  keyboard line input (DATA?)
			// -----2--  unknown
			// ------1-  keyboard transmit inhibit, 1=busy/not ready
			// -------0  unknown
			data |= m_centronics_fault << 7; // could be something else
			data |= !m_centronics_busy << 6; // could be something else
			break;

		case 0x32:
			// keyboard received byte
			data = m_ssrt->data_r();
			break;

		default:
			logerror("asic_r: %02x\n", offset);
			break;
	}

	return data;
}

void qvt70_state::asic_w(offs_t offset, uint8_t data)
{
	// 00
	// 01
	// 02
	// 03
	// 04
	// 05
	// 06
	// 07 = horizontal timing? 0x5d for 80 columns, 0x97 for 132
	// 08
	// 09
	// 0a = cursor address low byte
	// 0b = cursor address high byte
	// 0c = horizontal total minus one (character clocks: 80=0x67, 132=0xa5)
	// 0d = mode timing, 80=0x04, 132=0x07 (encoding unknown)
	// 0e = mode timing, 80=0x07, 132=0x0c (encoding unknown)
	// 0f = horizontal position
	// 10 = displayed columns in 80-column mode (0x50)
	// 11 = displayed columns in 132-column mode (0x84)
	// 12 = mode timing (encoding unknown)
	// 13 = displayed rows minus one (0x19/0x2b, including auxiliary rows)
	// 14 = mode timing (same value as 16: encoding unknown)
	// 15 = vertical position, firmware adjusts in steps of four
	// 16 = mode timing (encoding unknown)
	// 17 = companion attribute-write latch (enabled by 1d bit 2)
	// 18
	//   76------  unknown
	//   --5-----  set for even offsets, including zero
	//   ---4----  unknown
	//   ----3210  smooth-scroll pixel offset, 0 to cell height minus one
	// 19 = cell height minus one (16/13/9 pixels: 0x0f/0x0c/0x08)
	// 1a = descending row compare (physical row = 13 - 1a)
	// 1b
	// 1c
	// 1d = display and ram control
	//   7-------  unknown
	//   -6------  auxiliary ram, 0=font, 1=saved session (when 1e bit 2=1)
	//   --54----  unknown
	//   ----3---  display enable (also gates firmware cursor enable)
	//   -----2--  enable attribute/color companion writes (when 60 bit 7=0)
	//   ------10  unknown
	// 1e = display and ram control
	//   7-------  unknown
	//   -6------  columns, 1=80, 0=132
	//   --5-----  unknown
	//   ---4----  unknown
	//   ----3---  1=field attributes, 0=per-character attributes
	//   -----2--  0xe000 window, 0=attribute/background, 1=auxiliary ram
	//   ------1-  unknown
	//   -------0  unknown
	// 1f = auxiliary session select (mask 0x14: 0x04=session 0, 0x10=session 1)
	// 20
	// 21
	// 22
	// 23
	// 24
	// 25
	// 26
	// 27
	// 28
	// 29
	// 2a
	//   7-------  cursor enable, 1=visible, 0=hidden
	//   -6------  unknown
	//   --5-----  centronics strobe
	//   ---4----  printer probe pulse (reset?)
	//   ----3---  unknown
	//   -----2--  cursor blink: 1=blinking, 0=steady
	//   ------1-  unknown
	//   -------0  mode timing (function unknown)
	// 2b
	//   7-------  unknown
	//   -6------  beeper enable
	//   --543210  unknown
	// 2c
	// 2d
	// 2e
	// 2f = baud rate counter 1 low byte
	// 30
	// 31
	// 32 = baud rate counter 1 high bits (4-0); divisor = counter + 4
	// 33 = cursor first glyph raster (inclusive)
	// 34 = cursor last glyph raster (inclusive)
	// 35
	// 36 = baud rate counter 2 low byte
	// 37 = baud rate counter 2 high bits (4-0); divisor = counter + 4
	// 38
	//   76------  screen reverse, 11=reverse, 00=normal
	//   --543210  unknown
	// 39
	// 3a
	// 3b
	// 3c = underline glyph raster (16/13/9 pixels: 0x0f/0x0c/0x08)
	// 3d = mode timing (16/13/9 pixels: 0x40/0x43/0x47 - encoding unknown)
	// 3e
	//   7-------  unknown
	//   -6------  first scrolling-row strobe
	//   --5-----  last scrolling-row control
	//   ---43210  encoded raster preload? (derived from offset and cell height)
	// 3f = display timing control
	//   7654----  unknown
	//   ----3---  set for odd cell heights
	//   -----210  unknown
	// 40
	// 41
	// 42 = nmi enables (1=enabled, 0=masked: status at read 1d)
	//   7654----  unknown
	//   ----3---  keyboard receive complete
	//   -----2--  keyboard transmit complete
	//   ------1-  vblank
	//   -------0  character-row compare match
	// 43 = mode timing (16/13/9 pixels: 0x78/0x66/0x44 - encoding unknown)
	// 44
	// 45
	// 46
	// 47 = keyboard control (EPC: 0x1f=idle/tx acknowledge, 0x17=start tx)
	//   7654----  protocol/line control (EPC uses 0x1x; exact mask unknown)
	//   ----3---  EPC tx strobe, falling edge sends 4b
	//   -----2--  unknown
	//   ------10  legacy keyboard handshake (software-pulsed)
	// 48
	// 49
	// 4a
	// 4b = keyboard transmit latch (EPC send triggered by 47)
	// 4c
	// 4d
	// 4e
	// 4f
	// 50
	// 51
	//   76543---  unknown
	//   -----2--  descriptor page, 0=ram 0x0000, 1=ram 0x2000
	//   ------10  unknown
	// 52
	// 53
	// 54
	// 55
	// 56
	// 57
	// 58
	// 59
	// 5a
	// 5b
	// 5c
	// 5d
	// 5e
	// 5f
	// 60 = character/color ram windows
	//   7-------  0=character/attribute, 1=foreground/background at c000/e000
	//   -6543210  unknown
	//   1e bit 2 overrides the e000 window with auxiliary ram
	// 61 = border color
	// 62 = color output (function unknown)
	// 63 = companion foreground-color latch
	// 64 = companion background-color latch
	// 65 = default foreground color? (selection rules unknown)
	// 66 = default background color? (selection rules unknown)
	// 67 = write-protected foreground color (selection rules unknown)
	// 68 = write-protected background color (selection rules unknown)
	// 69 = mode timing (encoding unknown)
	// 6a = mode timing, 80=0x02, 132=0x07 (encoding unknown)
	// 6b = mode timing, 80=0x08, 132=0x0a (encoding unknown)

	if (0)
		logerror("ASIC %02x = %02x\n", offset, data);

	uint8_t prev = m_asic[offset];
	m_asic[offset] = data;

	switch (offset)
	{
		case 0x0c:
		case 0x10:
		case 0x11:
		case 0x13:
		case 0x19:
			update_screenmode();
			break;

		case 0x1a:
			update_row_timer();
			break;

		case 0x1d:
			update_rambanks();
			break;

		case 0x1e:
			update_rambanks();
			update_screenmode();
			break;

		case 0x1f:
			if (BIT(data, 2) != BIT(data, 4))
				m_connection = BIT(data, 4);

			update_rambanks();
			break;

		case 0x2a:
			m_centronics->write_strobe(BIT(data, 5));
			break;

		case 0x2b:
			m_beeper->set_state(BIT(data, 6));
			break;

		case 0x32:
			update_baudrate(0);
			break;

		case 0x37:
			update_baudrate(1);
			break;

		case 0x3e:
			if (BIT(data, 6) && !BIT(prev, 6))
				m_scroll_top = m_asic[0x13] - m_asic[0x1a];
			if (BIT(data, 5) && !BIT(prev, 5))
				m_scroll_bottom = m_asic[0x13] - m_asic[0x1a];
			break;

		case 0x47:
			// keyboard handshake
			if ((data & 0xf0) == 0x10)
			{
				if (BIT(data, 3))
				{
					// the tx-complete output stays high until the next send
					// we need to also clear it here
					m_nmi_state &= ~(1 << NMI_KBD_TX);
				}
				else if (BIT(prev, 3))
				{
					// starting a new transmission replaces the current rx-complete status
					m_nmi_state &= ~(1 << NMI_KBD_RX);
					m_ssrt->data_w(m_asic[0x4b]);
				}
			}
			break;

		case 0x60:
			update_rambanks();
			break;
	}
}

void qvt70_state::rombank_w(uint8_t data)
{
	if (data & ~0x19)
		logerror("rombank_w: %02x\n", data);

	// 765-----  unknown
	// ---43---  bankswitching
	// -----21-  unknown
	// -------0  bankswitching

	switch (data & 0x19)
	{
		case 0x00: m_rombank->set_entry(0); break;
		case 0x08: m_rombank->set_entry(1); break;
		case 0x10: m_rombank->set_entry(2); break;
		case 0x18: m_rombank->set_entry(3); break;
		case 0x01: m_rombank->set_entry(4); break;
		case 0x11: m_rombank->set_entry(5); break;

		default:
			logerror("Unknown ROM bank: %02x\n", data);
	}
}

void qvt70_state::machine_start()
{
	// 192k rom, split into 0x8000 sized banks
	m_rombank->configure_entries(0, 6, memregion("maincpu")->base(), 0x8000);

	// 64k ram, split into 0x2000 sized banks
	m_ram = std::make_unique<uint8_t[]>(0x10000);
	m_rambank[0]->configure_entries(0, 8, &m_ram[0], 0x2000);
	m_rambank[1]->configure_entries(0, 8, &m_ram[0], 0x2000);

	// allocate timer to trigger row interrupts
	m_row_timer = timer_alloc(FUNC(qvt70_state::row_interrupt), this);

	memset(m_asic, 0x00, sizeof(m_asic));

	// register for save states
	save_pointer(NAME(m_ram), 0x10000);
	save_item(NAME(m_asic));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_connection));
	save_item(NAME(m_scroll_top));
	save_item(NAME(m_scroll_bottom));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_centronics_fault));
}

void qvt70_state::machine_reset()
{
	m_nmi_state = 0;
	m_asic[0x42] = 0x00; // disable all nmi sources

	update_rambanks();
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

static const z80_daisy_config daisy_chain[] =
{
	{ "dart" },
	{ nullptr }
};

void qvt70_state::qvt70(machine_config &config)
{
	Z80(config, m_maincpu, 54.2857_MHz_XTAL / 8); // divisor guessed
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &qvt70_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &qvt70_state::io_map);

	NVRAM(config, "nvram");

	SCREEN(config, m_screen);
	m_screen->set_raw(54.2857_MHz_XTAL, 1664, 0, 1280, 502, 0, 416);
	m_screen->set_screen_update(FUNC(qvt70_state::screen_update));
	m_screen->screen_vblank().set(FUNC(qvt70_state::vblank_w));

	PALETTE(config, m_palette, FUNC(qvt70_state::palette_init), 64);

	GFXDECODE(config, m_gfxdecode, m_palette, chars);

	SPEAKER(config, "mono").front_center();

	BEEP(config, m_beeper, 1000).add_route(ALL_OUTPUTS, "mono", 1.0); // unknown frequency

	Z80DART(config, m_dart, 4'000'000);
	m_dart->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_dart->out_txda_callback().set(m_rs232[0], FUNC(rs232_port_device::write_txd));
	m_dart->out_dtra_callback().set(m_rs232[0], FUNC(rs232_port_device::write_dtr));
	m_dart->out_rtsa_callback().set(m_rs232[0], FUNC(rs232_port_device::write_rts));
	m_dart->out_txdb_callback().set(m_rs232[1], FUNC(rs232_port_device::write_txd));
	m_dart->out_rtsb_callback().set(m_rs232[1], FUNC(rs232_port_device::write_rts));
	m_dart->out_dtrb_callback().set(m_rs232[1], FUNC(rs232_port_device::write_dtr));

	// 25-pin (dcd, rxd, txd, dtr, dsr, rts, cts and current loop)
	RS232_PORT(config, m_rs232[0], default_rs232_devices, nullptr);
	m_rs232[0]->rxd_handler().set(m_dart, FUNC(z80dart_device::rxa_w));
	m_rs232[0]->dcd_handler().set(m_dart, FUNC(z80dart_device::dcda_w));
	m_rs232[0]->cts_handler().set(m_dart, FUNC(z80dart_device::ctsa_w));

	// 9-pin (dcd, rxd, txd, dtr, dsr, rts, cts)
	RS232_PORT(config, m_rs232[1], default_rs232_devices, nullptr);
	m_rs232[1]->rxd_handler().set(m_dart, FUNC(z80dart_device::rxb_w));
	m_rs232[1]->dcd_handler().set(m_dart, FUNC(z80dart_device::dcdb_w));
	m_rs232[1]->cts_handler().set(m_dart, FUNC(z80dart_device::ctsb_w));

	CLOCK(config, m_brg[0]);
	m_brg[0]->signal_handler().set(m_dart, FUNC(z80sio_device::txca_w));
	m_brg[0]->signal_handler().append(m_dart, FUNC(z80sio_device::rxca_w));

	CLOCK(config, m_brg[1]);
	m_brg[1]->signal_handler().append(m_dart, FUNC(z80sio_device::rxtxcb_w));

	OUTPUT_LATCH(config, m_centronics_latch);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->set_output_latch(*m_centronics_latch);
	m_centronics->busy_handler().set(FUNC(qvt70_state::centronics_busy_w));
	m_centronics->fault_handler().set(FUNC(qvt70_state::centronics_fault_w));

	AT_SSRT(config, m_ssrt);
	m_ssrt->clk().set(m_kbdc, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_ssrt->txd().set(m_kbdc, FUNC(pc_kbdc_device::data_write_from_mb));
	m_ssrt->rx().set(FUNC(qvt70_state::keyboard_rx_complete_w));
	m_ssrt->tx().set(FUNC(qvt70_state::keyboard_tx_complete_w));

	PC_KBDC(config, m_kbdc, pc_at_keyboards, STR_KBD_IBM_PC_AT_101);
	m_kbdc->out_clock_cb().set(m_ssrt, FUNC(at_ssrt_device::clk_w));
	m_kbdc->out_data_cb().set(m_ssrt, FUNC(at_ssrt_device::rxd_w));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( qvt70 )
	ROM_REGION(0x30000, "maincpu", 0)
	// 251513-03  C/S:D1DA  95' REV.J (checksum matches)
	ROM_LOAD("251513-03_revj.u11", 0x00000, 0x20000, CRC(c56796fe) SHA1(afe024ff93d5e75dc18041219d61e1a22fc6d883))
	// 251513-04  C/S:18D0  95' REV.J (checksum matches)
	ROM_LOAD("251513-04_revj.u12", 0x20000, 0x10000, CRC(3960bbd5) SHA1(9db306cef09be21ff43c081ebe11e9b46f617861))
ROM_END

ROM_START( qvt82 )
	ROM_REGION(0x30000, "maincpu", 0)
	// 304229-02D  QVT-82 REV. D  U6 (BF7F) (checksum matches)
	ROM_LOAD("304229-02d_revd.u6", 0x00000, 0x10000, CRC(597431df) SHA1(10c4669b759dd7cfd6746e54dc12807197cf841a))
	// 304229-01D  QVT-82 REV. D  U5 (462B) (checksum matches)
	ROM_LOAD("304229-01d_revd.u5", 0x20000, 0x10000, CRC(9ebd09b6) SHA1(ef9f002016d05b770e7b66d15f05fc286bd022d9))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME  FLAGS
COMP( 1992, qvt70, 0,      0,      qvt70,   0,     qvt70_state, empty_init, "Qume",  "QVT-70", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
COMP( 1993, qvt82, 0,      0,      qvt70,   0,     qvt70_state, empty_init, "Qume",  "QVT-82", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
