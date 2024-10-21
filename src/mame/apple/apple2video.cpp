// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    apple2video.cpp
    Apple II series video emulation

***************************************************************************/

#include "emu.h"
#include "apple2video.h"

#include "machine/ram.h"

#include "screen.h"

/***************************************************************************/

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

DEFINE_DEVICE_TYPE(APPLE2_VIDEO, a2_video_device, "a2video", "Apple II video")
DEFINE_DEVICE_TYPE(APPLE2_VIDEO_COMPOSITE, a2_video_device_composite, "a2video_comp", "Apple II video (composite)")
DEFINE_DEVICE_TYPE(APPLE2_VIDEO_COMPOSITE_RGB, a2_video_device_composite_rgb, "a2video_comprgb", "Apple II video (composite/RGB)")

//-------------------------------------------------
//  a2_video_device - constructor
//-------------------------------------------------

a2_video_device::a2_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_vidconfig(*this, "a2_video_config") {}

a2_video_device::a2_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a2_video_device(mconfig, APPLE2_VIDEO, tag, owner, clock) {}

a2_video_device_composite::a2_video_device_composite(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a2_video_device(mconfig, APPLE2_VIDEO_COMPOSITE, tag, owner, clock) {}

a2_video_device_composite_rgb::a2_video_device_composite_rgb(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a2_video_device(mconfig, APPLE2_VIDEO_COMPOSITE_RGB, tag, owner, clock) {}

void a2_video_device::device_start()
{
	// initialise for device_palette_interface
	init_palette();

	save_item(NAME(m_page2));
	save_item(NAME(m_flash));
	save_item(NAME(m_mix));
	save_item(NAME(m_graphics));
	save_item(NAME(m_hires));
	save_item(NAME(m_dhires));
	save_item(NAME(m_80col));
	save_item(NAME(m_altcharset));
	save_item(NAME(m_an2));
	save_item(NAME(m_80store));
	save_item(NAME(m_monohgr));
	save_item(NAME(m_GSfg));
	save_item(NAME(m_GSbg));
	save_item(NAME(m_GSborder));
	save_item(NAME(m_newvideo));
	save_item(NAME(m_monochrome));
	save_item(NAME(m_rgbmode));
	save_item(NAME(m_shr_palette));
}

void a2_video_device::device_reset()
{
	// Start in fullscreen hires if there is no character ROM. This is used
	// by the superga2 and tk2000 drivers, which support no other modes.
	m_graphics = m_hires = (m_char_ptr == nullptr);
	m_page2 = false;
	m_80col = false;
	m_altcharset = false;
	m_dhires = false;
	m_flash = false;
	m_mix = false;
	m_an2 = false;
	m_80store = false;
	m_monohgr = false;
	m_newvideo = 0x01;
	m_rgbmode = 3;  // default to color DHGR
	m_monochrome = 0; // TODO: never set, but if left uninitialized could cause the emulation to start in monochrome by accident. Default to color for now
}

void a2_video_device::txt_w(int state)
{
	if (m_graphics == state) // avoid flickering from II+ refresh polling
	{
		// select graphics or text mode
		screen().update_now();
		m_graphics = !state;
	}
}

void a2_video_device::mix_w(int state)
{
	// select mixed mode or nomix
	screen().update_now();
	m_mix = state;
}

void a2_video_device::scr_w(int state)
{
	// select primary or secondary page
	if (!m_80col)
		screen().update_now();
	m_page2 = state;
}

void a2_video_device::res_w(int state)
{
	// select lo-res or hi-res
	screen().update_now();
	m_hires = state;
}

void a2_video_device::dhires_w(int state)
{
	// select double hi-res
	screen().update_now();

	// RGB cards shift in a mode bit on the rising edge
	if ((m_dhires) && (state))
	{
		m_rgbmode = (m_rgbmode << 1) & 3;
		m_rgbmode |= m_80col ? 1 : 0;
	}

	m_dhires = !state;
}

void a2_video_device::an2_w(int state)
{
	m_an2 = state;
}

void a2_video_device::a80col_w(bool b80Col)
{
	screen().update_now();
	m_80col = b80Col;
}

// 4-bit left rotate. Bits 4-6 of n must be a copy of bits 0-2.
static constexpr unsigned rotl4b(unsigned n, unsigned count) { return (n >> (-count & 3)) & 0x0f; }
// 4-bit left rotate. Bits 4-6 of n must be zero.
static constexpr unsigned rotl4(unsigned n, unsigned count) { return rotl4b(n * 0x11, count); }

static constexpr class bit_doubler
{
	uint16_t a[128];

public:
	constexpr bit_doubler() : a{}
	{
		for (unsigned i = 1; i < 128; i++)
		{
			a[i] = a[i >> 1] * 4 + (i & 1) * 3;
		}
	}
	unsigned operator[](unsigned i) const { return a[i]; }

} double_7_bits;

static constexpr class bit_reverser
{
	uint8_t a[128];

public:
	constexpr bit_reverser() : a{}
	{
		for (unsigned i = 1; i < 128; i++)
		{
			a[i] = (a[i >> 1] >> 1) + ((i & 1) << 6);
		}
	}
	unsigned operator[](unsigned i) const { return a[i]; }

} reverse_7_bits;


inline bool a2_video_device::use_page_2() const { return m_page2 && !m_80store; }

inline bool a2_video_device::composite_monitor() { return (m_vidconfig.read_safe(0) & 7) == 0; }
inline bool a2_video_device::monochrome_monitor() { return (m_vidconfig.read_safe(0) & 4) != 0; }
inline bool a2_video_device::rgb_monitor() { return (m_vidconfig.read_safe(0) & 7) == 3; }

inline int a2_video_device::composite_color_mode() { return (m_vidconfig.read_safe(0) >> 4) & 7; }
inline bool a2_video_device::composite_lores_artifacts() { return (m_vidconfig.read_safe(0) & 0x80) != 0; }
inline bool a2_video_device::composite_text_color(bool is_80_column) { return (m_vidconfig.read_safe(0) & (is_80_column ? 0x200 : 0x100)) != 0; }
inline bool a2_video_device::monochrome_dhr_shift() { return (m_vidconfig.read_safe(0) & 7) == 7; }

int a2_video_device::monochrome_hue()
{
	switch (m_vidconfig.read_safe(0) & 7)
	{
		case 5: return GREEN;
		case 6: return ORANGE;
		default: return WHITE;
	}
}


static uint8_t const artifact_color_lut[2][1<<7] = {
// This table implements a colorization scheme defined by one's complement and mirror symmetries
// and the following rules:
//
// Every run of matching pixels gets a single color. For a run of off pixels, if the length is 4
// or more then the color is black, if the length is 3 then the color is 0001 (appropriately
// permuted), and if the length is 2 then the color is 1001. If the length is 1 and the 5-pixel
// group centered on the current pixel is a color (the end bits are equal), then that is the
// color. That leaves four hard cases which are resolved somewhat arbitrarily as follows:
// x1[10101], x[1101]00, where x is a don't care bit and the bracketed bits determine the color.
//
// As a special case, 0010000 is colored as a permutation of 0001 instead of black (and the
// counterparts of that rule under the symmetries). This means that runs of dark colors against
// black and light against white produce 4n-1 pixels instead of 4n-3 (4n would be ideal).
//
// A window size of 7 is enough to find the length of the middle pixel's run to a maximum of 4.
// Each color is duplicated in both nibbles to slightly simplify the 4-bit rotation logic.
{
	0x00,0x00,0x00,0x00,0x88,0x00,0x00,0x00,0x11,0x11,0x55,0x11,0x99,0x99,0xdd,0xff,
	0x22,0x22,0x66,0x66,0xaa,0xaa,0xee,0xee,0x33,0x33,0x33,0x33,0xbb,0xbb,0xff,0xff,
	0x00,0x00,0x44,0x44,0xcc,0xcc,0xcc,0xcc,0x55,0x55,0x55,0x55,0x99,0x99,0xdd,0xff,
	0x00,0x22,0x66,0x66,0xee,0xaa,0xee,0xee,0x77,0x77,0x77,0x77,0xff,0xff,0xff,0xff,
	0x00,0x00,0x00,0x00,0x88,0x88,0x88,0x88,0x11,0x11,0x55,0x11,0x99,0x99,0xdd,0xff,
	0x00,0x22,0x66,0x66,0xaa,0xaa,0xaa,0xaa,0x33,0x33,0x33,0x33,0xbb,0xbb,0xff,0xff,
	0x00,0x00,0x44,0x44,0xcc,0xcc,0xcc,0xcc,0x11,0x11,0x55,0x55,0x99,0x99,0xdd,0xdd,
	0x00,0x22,0x66,0x66,0xee,0xaa,0xee,0xee,0xff,0xff,0xff,0x77,0xff,0xff,0xff,0xff
},
// This alternate table colors 0110000 as a permutation of 0110, so that runs of medium colors
// against black or white produce 4n colored pixels instead of 4n-2. The disadvantage is it makes
// colorized 40-column text uglier. The 0010000 rule causes problems for 80-column text, but
// that's a lost cause anyway.
{
	0x00,0x00,0x00,0x00,0x88,0x00,0xcc,0x00,0x11,0x11,0x55,0x11,0x99,0x99,0xdd,0xff,
	0x22,0x22,0x66,0x66,0xaa,0xaa,0xee,0xee,0x33,0x33,0x33,0x33,0xbb,0xbB,0xff,0xff,
	0x00,0x00,0x44,0x44,0xcc,0xcc,0xcc,0xcc,0x55,0x55,0x55,0x55,0x99,0x99,0xdd,0xff,
	0x66,0x22,0x66,0x66,0xee,0xaa,0xee,0xee,0x77,0x77,0x77,0x77,0xff,0xfF,0xff,0xff,
	0x00,0x00,0x00,0x00,0x88,0x88,0x88,0x88,0x11,0x11,0x55,0x11,0x99,0x99,0xdd,0x99,
	0x00,0x22,0x66,0x66,0xaa,0xaa,0xaa,0xaa,0x33,0x33,0x33,0x33,0xbb,0xbB,0xff,0xff,
	0x00,0x00,0x44,0x44,0xcc,0xcc,0xcc,0xcc,0x11,0x11,0x55,0x55,0x99,0x99,0xdd,0xdd,
	0x00,0x22,0x66,0x66,0xee,0xaa,0xee,0xee,0xff,0x33,0xff,0x77,0xff,0xfF,0xff,0xff,
}};

template <int ContextBits, typename F>
static void render_line_pixel_window(uint16_t *out, uint16_t const *in, int startcol, int stopcol, F &&color_function)
{
	static_assert(ContextBits >= 0 && ContextBits <= 4);  // uint32_t has room for 4+14+14 bits

	if (startcol >= stopcol)
		return;  // to avoid OOB read

	// w holds ContextBits bits of the previous 14-pixel group and the current and next groups.
	uint32_t w = (ContextBits && startcol > 0) ? (in[startcol - 1] >> (14 - ContextBits)) : 0;
	w += in[startcol] << ContextBits;

	for (int col = startcol; col < stopcol; col++)
	{
		if (col + 1 < 40)
			w += in[col + 1] << (14 + ContextBits);

		for (int b = 0; b < 14; b++)
		{
			out[col * 14 + b] = color_function(w, col * 14 + b);
			w >>= 1;
		}
	}
}

static void render_line_monochrome(uint16_t *out, uint16_t const *in, int startcol, int stopcol, int fg, int bg)
{
	render_line_pixel_window<0>(
			out,
			in,
			startcol,
			stopcol,
			[fg, bg] (uint32_t w, int x) { return (w & 1) ? fg : bg; });
}

static void render_line_artifact_color(uint16_t *out, uint16_t const *in, int startcol, int stopcol, bool is_80_column, uint8_t const *lut)
{
	render_line_pixel_window<3>(
			out,
			in,
			startcol,
			stopcol,
			[is_80_column, lut] (uint32_t w, int x) { return rotl4b(lut[w & 0x7f], x + is_80_column); });
}

void a2_video_device::render_line(uint16_t *out, uint16_t const *in, int startcol, int stopcol, bool monochrome, bool is_80_column)
{
	if (monochrome)
	{
		int const fg = monochrome_hue();

		if (is_80_column && monochrome_dhr_shift())
		{
			// There is no way to communicate the chroma phase of the first pixel to the NTSC shader, and it
			// differs between Apple II modes. As a workaround, shift some modes right by one pixel (losing
			// the rightmost pixel). This should be removed when there is a way to pass parameters to shaders.
			render_line_pixel_window<1>(
					out,
					in,
					startcol,
					stopcol,
					[fg] (uint32_t w, int x) { return (w & 1) ? fg : BLACK; });
		}
		else
		{
			render_line_monochrome(out, in, startcol, stopcol, fg, BLACK);
		}
	}
	else
	{
		switch (int color_mode = composite_color_mode())
		{
		default:
			color_mode = 0;
			[[fallthrough]];
		case 0: case 1:
			// Pixel-run coloring
			render_line_artifact_color(out, in, startcol, stopcol, is_80_column, artifact_color_lut[color_mode]);
			break;
		case 2:
			// 4-bit square filter
			render_line_pixel_window<1>(
					out,
					in,
					startcol,
					stopcol,
					[is_80_column] (uint32_t w, int x) { return rotl4(w & 0x0f, x + is_80_column - 1); });
			break;
		}
	}
}

// Render with a different color for each 14-pixel group. Used by two RGB card modes.
static void render_line_color_array(uint16_t *out, uint16_t const *in, int startcol, int stopcol, uint8_t const *fg_bg_array)
{
	for (int col = startcol; col < stopcol; ++col)
	{
		unsigned w = in[col] * 4;
		unsigned const fg_bg = fg_bg_array[col];
		for (int b = 0; b < 14; ++b)
		{
			out[col * 14 + b] = (fg_bg >> (w & 4)) & 0x0f;
			w >>= 1;
		}
	}
}


template <a2_video_device::model Model, bool Invert, bool Flip>
unsigned a2_video_device::get_text_character(uint32_t code, int row)
{
	unsigned invert_mask = Invert ? 0 : 0x7f;

	if (Model == model::IIE || Model == model::IIGS)
	{
		if (!m_altcharset)
		{
			if ((code >= 0x40) && (code <= 0x7f))
			{
				code &= 0x3f;

				if (m_flash)
				{
					invert_mask ^= 0x7f;
				}
			}
		}
		else if (Model == model::IIGS)
		{
			code |= 0x100;
		}
		else
		{
			if ((code >= 0x60) && (code <= 0x7f))
			{
				code |= 0x80;    // map to lowercase normal
				invert_mask ^= 0x7f;  // and flip the color
			}
		}
		if (Model == model::IIE)
		{
			code |= get_iie_langsw() * 0x100;
		}
	}
	else    // original II and II Plus
	{
		if ((code >= 0x40) && (code <= 0x7f))
		{
			if (Model == model::II_J_PLUS)
			{
				code &= 0x3f;
			}
			if (m_flash)
			{
				invert_mask ^= 0x7f;
			}
		}
		else if (code < 0x40 && Model != model::IVEL_ULTRA) // inverse: flip FG and BG
		{
			invert_mask ^= 0x7f;
		}

		if (Model == model::II_J_PLUS && m_an2)
		{
			code |= 0x80;
		}
	}

	/* look up the character data */
	unsigned bits = m_char_ptr[code * 8 + row];
	bits = (Model == model::IVEL_ULTRA) ? (bits >> 1) : (bits & 0x7f);
	bits ^= invert_mask;
	return Flip ? reverse_7_bits[bits] : bits;
}

template<bool Double>
void a2_video_device::lores_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	uint32_t const start_address = use_page_2() ? 0x0800 : 0x0400;

	/* perform adjustments */
	beginrow = (std::max)(beginrow, cliprect.top());
	endrow = (std::min)(endrow, cliprect.bottom());
	const int startrow = (beginrow / 8) * 8;
	const int stoprow = ((endrow / 8) + 1) * 8;
	const int startcol = (cliprect.left() / 14);
	const int stopcol = ((cliprect.right() / 14) + 1);

	bool const monochrome = monochrome_monitor();
	bool render_perfect_blocks = !monochrome && (rgb_monitor() || !composite_lores_artifacts());
	if (!Double && m_dhires)
	{
		render_perfect_blocks = false;
	}
	//printf("GR: row %d startcol %d stopcol %d left %d right %d\n", beginrow, startcol, stopcol, cliprect.left(), cliprect.right());

	for (int row = startrow; row <= stoprow; row += 4)
	{
		/* calculate address */
		uint32_t const address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5));
		uint8_t const *const vram = m_ram_ptr + address;
		uint8_t const *const vaux = Double ? (m_aux_ptr + address) : nullptr;
		auto const NIBBLE = [&row] (auto byte) { return ((byte) >> (row & 4)) & 0x0f; };
		if (render_perfect_blocks)
		{
			uint16_t *p = &bitmap.pix(row, startcol * 14);

			for (int col = startcol; col < stopcol; col++)
			{
				if (Double)
				{
					unsigned c = rotl4(NIBBLE(vaux[col]), 1);
					for (int b = 0; b < 7; b++)
					{
						*p++ = c;
					}
				}
				unsigned c = NIBBLE(vram[col]);
				for (int b = 0; b < (Double ? 7 : 14); b++)
				{
					*p++ = c;
				}
			}
		}
		else
		{
			uint16_t words[40];

			// bounds must be even, include at least one extra column on each side, and be in [0,40]
			int const startcol2 = (startcol - 1) / 2 * 2;
			int const stopcol2 = (stopcol < 40) ? ((stopcol | 1) + 1) : 40;

			for (int col = startcol2; col < stopcol2; col += 2)
			{
				if (Double)
				{
					words[col+0] = (((NIBBLE(vaux[col+0]) * 0x111)     ) & 0x007f) + ((NIBBLE(vram[col+0]) * 0x0880) & 0x3f80);
					words[col+1] = (((NIBBLE(vaux[col+1]) * 0x111) >> 2) & 0x007f) + ((NIBBLE(vram[col+1]) * 0x2220) & 0x3f80);
				}
				else
				{
					if (m_dhires)
					{
						// TODO: lo-res in 7M drawing here (should be just black/white/green/purple)
						words[col+0] = (NIBBLE(vram[col+0]) * 0x1111) & 0x3fff;
						words[col+1] = (NIBBLE(vram[col+1]) * 0x1111) >> 2;
					}
					else
					{
						words[col+0] = (NIBBLE(vram[col+0]) * 0x1111) & 0x3fff;
						words[col+1] = (NIBBLE(vram[col+1]) * 0x1111) >> 2;
					}
				}
			}

			render_line(&bitmap.pix(row), words, startcol, stopcol, monochrome, Double);
		}

		if (startcol < stopcol)
		{
			for (int y = 1; y < 4; y++)
			{
				memcpy(&bitmap.pix(row + y, startcol * 14), &bitmap.pix(row, startcol * 14), (stopcol - startcol) * 14 * (bitmap.bpp() / 8));
			}
		}
	}
}

template <a2_video_device::model Model, bool Invert, bool Flip>
void a2_video_device::text_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	uint8_t const *const aux_page = m_aux_ptr ? m_aux_ptr : m_ram_ptr;

	uint32_t const start_address = use_page_2() ? 0x0800 : 0x0400;

	beginrow = (std::max)(beginrow, cliprect.top() - (cliprect.top() % 8));
	endrow = (std::min)(endrow, cliprect.bottom() - (cliprect.bottom() % 8) + 7);

	const int startrow = (beginrow / 8) * 8;
	const int stoprow = ((endrow / 8) + 1) * 8;
	const int startcol = (cliprect.left() / 14);
	const int stopcol = ((cliprect.right() / 14) + 1);

	// printf("TXT: row %d startcol %d stopcol %d left %d right %d\n", beginrow, startcol, stopcol, cliprect.left(), cliprect.right());

	bool const is_80_column = (Model == model::IIE || Model == model::IIGS) && m_80col;
	bool const monochrome = !(m_graphics && composite_monitor() && composite_text_color(is_80_column));
	for (int row = startrow; row < stoprow; row++)
	{
		uint32_t const address = start_address + ((((row / 8) & 0x07) << 7) | (((row / 8) & 0x18) * 5));
		uint32_t const aux_address = (Model == model::IIGS) ? address : (address & m_aux_mask);
		uint16_t words[40];

		for (int col = std::max(0, startcol-1); col < std::min(stopcol+1, 40); col++)
		{
			if (is_80_column)
			{
				words[col] = get_text_character<Model, Invert, Flip>(aux_page[aux_address + col], row & 7)
						  + (get_text_character<Model, Invert, Flip>(m_ram_ptr[address + col], row & 7) << 7);
			}
			else
			{
				words[col] = double_7_bits[get_text_character<Model, Invert, Flip>(m_ram_ptr[address + col], row & 7)];
			}
		}

		if (Model == model::IIE && rgb_monitor() && m_dhires && !m_80col)
		{
			// Video-7 foreground-background mode
			render_line_color_array(&bitmap.pix(row), words, startcol, stopcol, &aux_page[aux_address]);
		}
		else if (Model == model::IIGS)
		{
			render_line_monochrome(&bitmap.pix(row), words, startcol, stopcol, m_GSfg, m_GSbg);
		}
		else
		{
			render_line(&bitmap.pix(row), words, startcol, stopcol, monochrome, is_80_column);
		}
	}
}

template void a2_video_device::text_update<a2_video_device::model::II, true, true>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
template void a2_video_device::text_update<a2_video_device::model::II, true, false>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
template void a2_video_device::text_update<a2_video_device::model::II, false, true>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
template void a2_video_device::text_update<a2_video_device::model::II, false, false>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
template void a2_video_device::text_update<a2_video_device::model::IIE, true, true>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
template void a2_video_device::text_update<a2_video_device::model::IIE, true, false>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
template void a2_video_device::text_update<a2_video_device::model::IIE, false, true>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
template void a2_video_device::text_update<a2_video_device::model::IIE, false, false>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
template void a2_video_device::text_update<a2_video_device::model::IIGS, false, false>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
template void a2_video_device::text_update<a2_video_device::model::II_J_PLUS, true, true>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
template void a2_video_device::text_update<a2_video_device::model::IVEL_ULTRA, true, false>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);

void a2_video_device::hgr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	beginrow = (std::max)(beginrow, cliprect.top());
	endrow = (std::min)(endrow, cliprect.bottom());
	int const startcol = (cliprect.left() / 14);
	int const stopcol = (cliprect.right() / 14) + 1;

	// B&W/Green/Amber monitor, CEC mono HGR mode, or IIgs $C021 monochrome HGR
	bool const monochrome = monochrome_monitor() || m_monohgr || (m_monochrome & 0x80);

	unsigned const start_address = use_page_2() ? 0x4000 : 0x2000;

	// verified on h/w: setting dhires w/o 80col emulates a rev. 0 Apple ][ with no orange/blue
	uint8_t const bit7_mask = m_dhires ? 0 : 0x80;

	for (int row = beginrow; row <= endrow; row++)
	{
		unsigned const address = start_address + (((row/8) & 0x07) << 7) + (((row/8) & 0x18) * 5) + ((row & 7) << 10);
		uint8_t const *const vram_row = &m_ram_ptr[address];

		uint16_t words[40];

		unsigned last_output_bit = 0;

		for (int col = std::max(0, startcol-1); col < std::min(stopcol+1, 40); col++)
		{
			unsigned word = double_7_bits[vram_row[col] & 0x7f];
			if (vram_row[col] & bit7_mask)
				word = (word * 2 + last_output_bit) & 0x3fff;
			words[col] = word;
			last_output_bit = word >> 13;
		}

		if (rgb_monitor())
		{
			if (m_dhires)
			{
				// Video-7 foreground-background mode
				render_line_color_array(&bitmap.pix(row), words, startcol, stopcol, &m_aux_ptr[address]);
			}
			else
			{
				// Use the "color bias" table as that seems to be closest to what this card does (youtu.be/4lDRv2MudvI?t=666)
				render_line_artifact_color(&bitmap.pix(row), words, startcol, stopcol, false, artifact_color_lut[1]);
			}
		}
		else
		{
			render_line(&bitmap.pix(row), words, startcol, stopcol, monochrome, false);
		}
	}
}

void a2_video_device::dhgr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int const page = use_page_2() ? 0x4000 : 0x2000;
	int const rgbmode = rgb_monitor() ? m_rgbmode : -1;

	// B&W/Green/Amber monitor, IIgs force-monochrome-DHR setting, or IIe RGB card monochrome DHR
	bool const monochrome = monochrome_monitor() || (m_newvideo & 0x20) || rgbmode == 0;

	beginrow = (std::max)(beginrow, cliprect.top());
	endrow = (std::min)(endrow, cliprect.bottom());
	int const startcol = (cliprect.left() / 14);
	int const stopcol = (cliprect.right() / 14) + 1;

	uint8_t const *const vram = &m_ram_ptr[page];
	uint8_t const *const vaux = (m_aux_ptr ? m_aux_ptr : vram) + page;

	for (int row = beginrow; row <= endrow; row++)
	{
		int const offset = ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5)) | ((row & 7) << 10);
		uint8_t const *const vram_row = vram + offset;
		uint8_t const *const vaux_row = vaux + offset;

		uint16_t *p = &bitmap.pix(row);

		if (rgbmode == 2)
		{
			// Video-7 RGB 160-wide mode (which has a much simpler VRAM layout).
			// Center a 480-wide image in the 560-wide display.
			// Aspect ratio won't be perfect, but it's in range.

			for (int b = 0; b < 40; b++)
			{
				*p++ = BLACK;
			}
			for (int col = 0; col < 40; col++)
			{
				unsigned v = vaux_row[col] + (vram_row[col] << 8);
				for (int i = 0; i < 4; i++)
				{
					*p++ = v & 0xf;
					*p++ = v & 0xf;
					*p++ = v & 0xf;
					v >>= 4;
				}
			}
			for (int b = 0; b < 40; b++)
			{
				*p++ = BLACK;
			}
		}
		else
		{
			uint16_t words[40];

			for (int col = std::max(0, startcol-1); col < std::min(stopcol+1, 40); col++)
			{
				words[col] = (vaux_row[col] & 0x7f) + ((vram_row[col] & 0x7f) << 7);
			}

			if (rgbmode < 0 || monochrome)
			{
				// Composite or monochrome, use the renderer that supports artifact rendering.
				render_line(p, words, startcol, stopcol, monochrome, true);
			}
			else
			{
				// Video-7 RGB with rgbmode == 1 (mixed) or 3 (color). In color mode, the card
				// seems to produce just 7 wide pixels per 4 bytes:
				//   column & 3 =  0        1        2        3
				//              nBBBAAAA nDDCCCCB nFEEEEDD nGGGGFFF
				//
				// In mixed mode, the Video-7 User's Manual says:
				//
				// "When [the MSB is 0] the hardware displays the remaining seven bits as bit-mapped
				// video; a logic 'one' will instruct the hardware to display the next seven bits
				// as color pixels. [...] color aberrations will occur at the leading and trailing
				// edges of the transitions from one mode to the other. The aberrations may be
				// eliminated by blanking enough bytes at the beginning and end of each transition
				// to guarantee the four bit integrity of the corresponding color pixels."
				//
				// It's impossible to know the nature of the aberrations without hardware to test,
				// but hopefully this warning means software was designed so that it doesn't matter.

				for (int col = 0; col < 40; col += 2)
				{
					unsigned const w = words[col] + (words[col+1] << 14);

					unsigned const color_mask = (rgbmode == 3) ? -1u :
							(vaux_row[col] >> 7) * 0x7f + (vram_row[col] >> 7) * 0x3f80
							+ (vaux_row[col+1] >> 7) * 0x1fc000 + (vram_row[col+1] >> 7) * 0xfe00000;

					for (int b = 0; b < 28; ++b)
					{
						*p++ = (color_mask & (1 << b)) ? rotl4((w >> (b & ~3u)) & 0x0f, 1) : (w & (1 << b)) ? WHITE : BLACK;
					}
				}
			}
		}
	}
}

/*
    Latest Apple II NTSC palette from "Apple II Video Display Theory"
    https://docs.google.com/spreadsheets/d/1rKR6A_bVniSCtIP_rrv8QLWJdj4h6jEU1jJj0AebWwg/
*/
static const rgb_t apple2_palette[] =
{
	rgb_t::black(),
	rgb_t(0xa7, 0x0b, 0x40), /* Dark Red */
	rgb_t(0x40, 0x1c, 0xf7), /* Dark Blue */
	rgb_t(0xe6, 0x28, 0xff), /* Purple */
	rgb_t(0x00, 0x74, 0x40), /* Dark Green */
	rgb_t(0x80, 0x80, 0x80), /* Dark Gray */
	rgb_t(0x19, 0x90, 0xff), /* Medium Blue */
	rgb_t(0xbf, 0x9c, 0xff), /* Light Blue */
	rgb_t(0x40, 0x63, 0x00), /* Brown */
	rgb_t(0xe6, 0x6f, 0x00), /* Orange */
	rgb_t(0x80, 0x80, 0x80), /* Light Grey */
	rgb_t(0xff, 0x8b, 0xbf), /* Pink */
	rgb_t(0x19, 0xd7, 0x00), /* Light Green */
	rgb_t(0xbf, 0xe3, 0x08), /* Yellow */
	rgb_t(0x58, 0xf4, 0xbf), /* Aquamarine */
	rgb_t(0xff, 0xff, 0xff)  /* White */
};

void a2_video_device::init_palette()
{
	for (int i = 0; i < std::size(apple2_palette); i++)
		set_pen_color(i, apple2_palette[i]);
}

uint32_t a2_video_device::palette_entries() const noexcept
{
	return std::size(apple2_palette);
}

uint32_t a2_video_device::screen_update_GS(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int const beamy = cliprect.top();

	if (m_newvideo & 0x80)
	{
		// in top or bottom border?
		if ((beamy < BORDER_TOP) || (beamy >= 200+BORDER_TOP))
		{
			// don't draw past the bottom border
			if (beamy >= 231+BORDER_TOP)
			{
				return 0;
			}

			uint32_t *const scanline = &bitmap.pix(beamy);
			for (int col = 0; col < BORDER_LEFT+BORDER_RIGHT+640; col++)
			{
				scanline[col] = m_GSborder_colors[m_GSborder];
			}
		}
		else    // regular screen area
		{
			int const shrline = beamy - BORDER_TOP;

			uint8_t scb;
			if (shrline & 1)
			{
				scb = m_aux_ptr[0x9e80 + (shrline >> 1)];
			}
			else
			{
				scb = m_aux_ptr[0x5e80 + (shrline >> 1)];
			}
			int const palette = ((scb & 0x0f) << 4);

			uint8_t const *const vram = &m_aux_ptr[0x2000 + (shrline * 80)];
			uint8_t const *const vram2 = &m_aux_ptr[0x6000 + (shrline * 80)];

			uint32_t *const scanline = &bitmap.pix(beamy);

			// draw left and right borders
			for (int col = 0; col < BORDER_LEFT; col++)
			{
				scanline[col] = m_GSborder_colors[m_GSborder];
				scanline[col+BORDER_LEFT+640] = m_GSborder_colors[m_GSborder];
			}

			if (scb & 0x80) // 640 mode
			{
				for (int col = 0; col < 80; col++)
				{
					uint8_t b = vram[col];
					scanline[col * 8 + 0 + BORDER_LEFT] = m_shr_palette[palette +  0 + ((b >> 6) & 0x03)];
					scanline[col * 8 + 1 + BORDER_LEFT] = m_shr_palette[palette +  4 + ((b >> 4) & 0x03)];
					scanline[col * 8 + 2 + BORDER_LEFT] = m_shr_palette[palette +  8 + ((b >> 2) & 0x03)];
					scanline[col * 8 + 3 + BORDER_LEFT] = m_shr_palette[palette + 12 + ((b >> 0) & 0x03)];

					b = vram2[col];
					scanline[col * 8 + 4 + BORDER_LEFT] = m_shr_palette[palette + 0 + ((b >> 6) & 0x03)];
					scanline[col * 8 + 5 + BORDER_LEFT] = m_shr_palette[palette + 4 + ((b >> 4) & 0x03)];
					scanline[col * 8 + 6 + BORDER_LEFT] = m_shr_palette[palette + 8 + ((b >> 2) & 0x03)];
					scanline[col * 8 + 7 + BORDER_LEFT] = m_shr_palette[palette + 12 + ((b >> 0) & 0x03)];
				}
			}
			else        // 320 mode
			{
				// the low 5 bits of the SCB determine the initial fillmode color
				// for the scanline (hardware testing by John Brooks)
				static const uint32_t fillmode_init[32] =
				{
					2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
					0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
				};

				uint32_t last_pixel = fillmode_init[scb & 0x1f];
				for (int col = 0; col < 80; col++)
				{
					uint8_t b;
					uint32_t pixel;

					b = vram[col];
					pixel = (b >> 4) & 0x0f;

					if ((scb & 0x20) && !pixel)
						pixel = last_pixel;
					else
						last_pixel = pixel;
					pixel += palette;
					scanline[col * 8 + 0 + BORDER_LEFT] = m_shr_palette[pixel];
					scanline[col * 8 + 1 + BORDER_LEFT] = m_shr_palette[pixel];

					pixel = (b >> 0) & 0x0f;

					if ((scb & 0x20) && !pixel)
						pixel = last_pixel;
					else
						last_pixel = pixel;
					pixel += palette;
					scanline[col * 8 + 2 + BORDER_LEFT] = m_shr_palette[pixel];
					scanline[col * 8 + 3 + BORDER_LEFT] = m_shr_palette[pixel];

					b = vram2[col];
					pixel = (b >> 4) & 0x0f;

					if ((scb & 0x20) && !pixel)
						pixel = last_pixel;
					else
						last_pixel = pixel;
					pixel += palette;
					scanline[col * 8 + 4 + BORDER_LEFT] = m_shr_palette[pixel];
					scanline[col * 8 + 5 + BORDER_LEFT] = m_shr_palette[pixel];

					pixel = (b >> 0) & 0x0f;

					if ((scb & 0x20) && !pixel)
						pixel = last_pixel;
					else
						last_pixel = pixel;
					pixel += palette;
					scanline[col * 8 + 6 + BORDER_LEFT] = m_shr_palette[pixel];
					scanline[col * 8 + 7 + BORDER_LEFT] = m_shr_palette[pixel];
				}
			}
		}
	}
	else
	{
		if (beamy >= BORDER_TOP)
		{
			rectangle const new_cliprect(0, 559, cliprect.top() - BORDER_TOP, cliprect.bottom() - BORDER_TOP);
			screen_update<model::IIGS, false, false>(screen, *m_8bit_graphics, new_cliprect);
		}

		if ((beamy < (BORDER_TOP+4)) || (beamy >= (192+4+BORDER_TOP)))
		{
			if (beamy >= (231+BORDER_TOP))
			{
				return 0;
			}

			uint32_t *const scanline = &bitmap.pix(beamy);
			for (int col = 0; col < BORDER_LEFT+BORDER_RIGHT+640; col++)
			{
				scanline[col] = m_GSborder_colors[m_GSborder];
			}
		}
		else
		{
			uint32_t *const scanline = &bitmap.pix(beamy);

			// draw left and right borders
			for (int col = 0; col < BORDER_LEFT + 40; col++)
			{
				scanline[col] = m_GSborder_colors[m_GSborder];
				scanline[col+BORDER_LEFT+600] = m_GSborder_colors[m_GSborder];
			}

			uint16_t *a2pixel = &m_8bit_graphics->pix(beamy-(BORDER_TOP+4));
			for (int x = 0; x < 560; x++)
			{
				scanline[40 + BORDER_LEFT + x] = m_GSborder_colors[*a2pixel++];
			}
		}
	}
	return 0;
}

template <a2_video_device::model Model, bool Invert, bool Flip>
uint32_t a2_video_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (Model == model::IIGS && cliprect.bottom() > 191)
	{
		return 0;
	}

	// always update the flash timer here so it's smooth regardless of mode switches
	m_flash = ((machine().time() * 4).seconds() & 1) ? true : false;

	int text_start_row = 0;

	if (m_graphics)
	{
		text_start_row = m_mix ? 160 : 192;

		if (m_hires)
		{
			if ((Model == model::IIE || Model == model::IIGS) && m_dhires && m_80col)
			{
				dhgr_update(screen, bitmap, cliprect, 0, text_start_row - 1);
			}
			else
			{
				hgr_update(screen, bitmap, cliprect, 0, text_start_row - 1);
			}
		}
		else    // lo-res
		{
			if ((Model == model::IIE || Model == model::IIGS) && m_dhires && m_80col)
			{
				lores_update<true>(screen, bitmap, cliprect, 0, text_start_row - 1);
			}
			else
			{
				lores_update<false>(screen, bitmap, cliprect, 0, text_start_row - 1);
			}
		}
	}

	if (text_start_row < 192)
	{
		text_update<Model, Invert, Flip>(screen, bitmap, cliprect, text_start_row, 191);
	}

	return 0;
}

template uint32_t a2_video_device::screen_update<a2_video_device::model::II, true, true>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
template uint32_t a2_video_device::screen_update<a2_video_device::model::II, true, false>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
template uint32_t a2_video_device::screen_update<a2_video_device::model::II, false, true>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
template uint32_t a2_video_device::screen_update<a2_video_device::model::II, false, false>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
template uint32_t a2_video_device::screen_update<a2_video_device::model::IIE, true, true>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
template uint32_t a2_video_device::screen_update<a2_video_device::model::IIE, true, false>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
template uint32_t a2_video_device::screen_update<a2_video_device::model::IIE, false, true>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
template uint32_t a2_video_device::screen_update<a2_video_device::model::IIE, false, false>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
template uint32_t a2_video_device::screen_update<a2_video_device::model::IIGS, false, false>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
template uint32_t a2_video_device::screen_update<a2_video_device::model::II_J_PLUS, true, true>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
template uint32_t a2_video_device::screen_update<a2_video_device::model::IVEL_ULTRA, true, false>(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

static INPUT_PORTS_START( a2_vidconfig_composite );
	PORT_START("a2_video_config")
	PORT_CONFNAME(0x07, 0x00, "Monitor type")
	PORT_CONFSETTING(0x00, "Color")
	PORT_CONFSETTING(0x04, "B&W")
	PORT_CONFSETTING(0x05, "Green")
	PORT_CONFSETTING(0x06, "Amber")
	PORT_CONFSETTING(0x07, "B&W for NTSC shader")

	PORT_CONFNAME(0x70, 0x00, "Color algorithm")
	PORT_CONFSETTING(0x00, "Pixel-run (B&W bias)")
	PORT_CONFSETTING(0x10, "Pixel-run (color bias)")
	PORT_CONFSETTING(0x20, "Box filter")

	PORT_CONFNAME(0x80, 0x00, "Lores artifacts")
	PORT_CONFSETTING(0x00, "Off")
	PORT_CONFSETTING(0x80, "On")

	PORT_CONFNAME(0x300, 0x000, "Text color")
	PORT_CONFSETTING(0x000, "Off")
	PORT_CONFSETTING(0x100, "40-column only")
	PORT_CONFSETTING(0x300, "On")
INPUT_PORTS_END

static INPUT_PORTS_START( a2_vidconfig_composite_rgb )
	PORT_INCLUDE(a2_vidconfig_composite)
	PORT_MODIFY("a2_video_config")
	PORT_CONFNAME(0x07, 0x00, "Monitor type")
	PORT_CONFSETTING(0x00, "Color")
	PORT_CONFSETTING(0x03, "Video-7 RGB")
	PORT_CONFSETTING(0x04, "B&W")
	PORT_CONFSETTING(0x05, "Green")
	PORT_CONFSETTING(0x06, "Amber")
	PORT_CONFSETTING(0x07, "B&W for NTSC shader")
INPUT_PORTS_END

ioport_constructor a2_video_device_composite::device_input_ports() const { return INPUT_PORTS_NAME(a2_vidconfig_composite); }
ioport_constructor a2_video_device_composite_rgb::device_input_ports() const { return INPUT_PORTS_NAME(a2_vidconfig_composite_rgb); }
