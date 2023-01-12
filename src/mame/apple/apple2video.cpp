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

WRITE_LINE_MEMBER(a2_video_device::txt_w)
{
	if (m_graphics == state) // avoid flickering from II+ refresh polling
	{
		// select graphics or text mode
		screen().update_now();
		m_graphics = !state;
	}
}

WRITE_LINE_MEMBER(a2_video_device::mix_w)
{
	// select mixed mode or nomix
	screen().update_now();
	m_mix = state;
}

WRITE_LINE_MEMBER(a2_video_device::scr_w)
{
	// select primary or secondary page
	if (!m_80col)
		screen().update_now();
	m_page2 = state;
}

WRITE_LINE_MEMBER(a2_video_device::res_w)
{
	// select lo-res or hi-res
	screen().update_now();
	m_hires = state;
}

WRITE_LINE_MEMBER(a2_video_device::dhires_w)
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

WRITE_LINE_MEMBER(a2_video_device::an2_w)
{
	m_an2 = state;
}

// 4-bit left rotate. Bits 4-6 of n must be a copy of bits 0-2.
static inline unsigned rotl4b(unsigned n, unsigned count) { return (n >> (-count & 3)) & 0x0F; }
// 4-bit left rotate. Bits 4-6 of n must be zero.
static inline unsigned rotl4(unsigned n, unsigned count) { return rotl4b(n * 0x11, count); }

inline bool a2_video_device::use_page_2() const { return m_page2 && !m_80store; }

inline bool a2_video_device::monochrome_monitor() { return (m_vidconfig.read_safe(0) & 4) != 0; }

inline bool a2_video_device::rgb_monitor() { return (m_vidconfig.read_safe(0) & 7) == 3; }

int a2_video_device::monochrome_hue()
{
	switch (m_vidconfig.read_safe(0) & 7)
	{
		case 5: return GREEN;
		case 6: return ORANGE;
		default: return WHITE;
	}
}

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
static uint8_t const artifact_color_lut[1<<7] = {
	0x00,0x00,0x00,0x00,0x88,0x00,0x00,0x00,0x11,0x11,0x55,0x11,0x99,0x99,0xDD,0xFF,
	0x22,0x22,0x66,0x66,0xAA,0xAA,0xEE,0xEE,0x33,0x33,0x33,0x33,0xBB,0xBB,0xFF,0xFF,
	0x00,0x00,0x44,0x44,0xCC,0xCC,0xCC,0xCC,0x55,0x55,0x55,0x55,0x99,0x99,0xDD,0xFF,
	0x00,0x22,0x66,0x66,0xEE,0xAA,0xEE,0xEE,0x77,0x77,0x77,0x77,0xFF,0xFF,0xFF,0xFF,
	0x00,0x00,0x00,0x00,0x88,0x88,0x88,0x88,0x11,0x11,0x55,0x11,0x99,0x99,0xDD,0xFF,
	0x00,0x22,0x66,0x66,0xAA,0xAA,0xAA,0xAA,0x33,0x33,0x33,0x33,0xBB,0xBB,0xFF,0xFF,
	0x00,0x00,0x44,0x44,0xCC,0xCC,0xCC,0xCC,0x11,0x11,0x55,0x55,0x99,0x99,0xDD,0xDD,
	0x00,0x22,0x66,0x66,0xEE,0xAA,0xEE,0xEE,0xFF,0xFF,0xFF,0x77,0xFF,0xFF,0xFF,0xFF
};

// This alternate table colors 0110000 as a permutation of 0110, so that runs of medium colors
// against black or white produce 4n colored pixels instead of 4n-2. It is disabled because it
// makes colorized 40-column text annoying to read. The 0010000 rule causes problems for
// 80-column text, but that's a lost cause anyway.
// static uint8_t const artifact_color_lut[1<<7] = {
// 	0x00,0x00,0x00,0x00,0x88,0x00,0xCC,0x00,0x11,0x11,0x55,0x11,0x99,0x99,0xDD,0xFF,
// 	0x22,0x22,0x66,0x66,0xAA,0xAA,0xEE,0xEE,0x33,0x33,0x33,0x33,0xBB,0xBB,0xFF,0xFF,
// 	0x00,0x00,0x44,0x44,0xCC,0xCC,0xCC,0xCC,0x55,0x55,0x55,0x55,0x99,0x99,0xDD,0xFF,
// 	0x66,0x22,0x66,0x66,0xEE,0xAA,0xEE,0xEE,0x77,0x77,0x77,0x77,0xFF,0xFF,0xFF,0xFF,
// 	0x00,0x00,0x00,0x00,0x88,0x88,0x88,0x88,0x11,0x11,0x55,0x11,0x99,0x99,0xDD,0x99,
// 	0x00,0x22,0x66,0x66,0xAA,0xAA,0xAA,0xAA,0x33,0x33,0x33,0x33,0xBB,0xBB,0xFF,0xFF,
// 	0x00,0x00,0x44,0x44,0xCC,0xCC,0xCC,0xCC,0x11,0x11,0x55,0x55,0x99,0x99,0xDD,0xDD,
// 	0x00,0x22,0x66,0x66,0xEE,0xAA,0xEE,0xEE,0xFF,0x33,0xFF,0x77,0xFF,0xFF,0xFF,0xFF,
// };

template <a2_video_device::model Model, bool Invert, bool Flip>
void a2_video_device::plot_text_character(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, int fg, int bg)
{
	if (Model == model::IIE || Model == model::IIGS)
	{
		if (!m_altcharset)
		{
			if ((code >= 0x40) && (code <= 0x7f))
			{
				code &= 0x3f;

				if (m_flash)
				{
					using std::swap;
					swap(fg, bg);
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
				using std::swap; // and flip the color
				std::swap(fg, bg);
			}
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
				using std::swap;
				swap(fg, bg);
			}
		}
		else if (code < 0x40 && Model != model::IVEL_ULTRA) // inverse: flip FG and BG
		{
			using std::swap;
			swap(fg, bg);
		}

		if (Model == model::II_J_PLUS && m_an2)
		{
			code |= 0x80;
		}
	}

	if (Invert)
	{
		std::swap(fg, bg);
	}

	/* look up the character data */
	uint8_t const *const chardata = &m_char_ptr[(code * 8)];

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 7; x++)
		{
			// Model and Flip are template parameters, so the compiler will boil this down appropriately
			unsigned const bit = (Model == model::IVEL_ULTRA) ? (2 << x) : Flip ? (64 >> x) : (1 << x);
			uint16_t const color = (chardata[y] & bit) ? bg : fg;

			for (int i = 0; i < xscale; i++)
			{
				bitmap.pix(ypos + y, xpos + (x * xscale) + i) = color;
			}
		}
	}
}

void a2_video_device::lores_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	uint32_t const start_address = use_page_2() ? 0x0800 : 0x0400;

	bool const monochrome = monochrome_monitor();
	int const fg = monochrome_hue();

	/* perform adjustments */
	beginrow = (std::max)(beginrow, cliprect.top());
	endrow = (std::min)(endrow, cliprect.bottom());
	const int startrow = (beginrow / 8) * 8;
	const int stoprow = ((endrow / 8) + 1) * 8;
	const int startcol = (cliprect.left() / 14);
	const int stopcol = ((cliprect.right() / 14) + 1);

	//printf("GR: row %d startcol %d stopcol %d left %d right %d\n", beginrow, startcol, stopcol, cliprect.left(), cliprect.right());

	if (!monochrome)
	{
		for (int row = startrow; row <= stoprow; row += 8)
		{
			for (int col = startcol; col < stopcol; col++)
			{
				/* calculate adderss */
				uint32_t const address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));

				/* perform the lookup */
				uint8_t const code = m_ram_ptr[address];

				/* and now draw */
				for (int y = 0; y < 4; y++)
				{
					if (((row + y) >= beginrow) && ((row + y) <= endrow))
					{
						for (int x = 0; x < 14; x++)
						{
							bitmap.pix(row + y, col * 14 + x) = (code >> 0) & 0x0F;
						}
					}
				}
				for (int y = 4; y < 8; y++)
				{
					if (((row + y) >= beginrow) && ((row + y) <= endrow))
					{
						for (int x = 0; x < 14; x++)
						{
							bitmap.pix(row + y, col * 14 + x) = (code >> 4) & 0x0F;
						}
					}
				}
			}
		}
	}
	else
	{
		for (int row = startrow; row <= stoprow; row += 8)
		{
			for (int col = startcol; col < stopcol; col++)
			{
				uint8_t bits;

				/* calculate adderss */
				uint32_t const address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));

				/* perform the lookup */
				uint8_t const code = m_ram_ptr[address];

				bits = (code >> 0) & 0x0F;
				/* and now draw */
				for (int y = 0; y < 4; y++)
				{
					if (((row + y) >= beginrow) && ((row + y) <= endrow))
					{
						for (int x = 0; x < 14; x++)
						{
							if (col & 1)
							{
								bitmap.pix(row + y, col * 14 + x) = bits & (1 << ((x+2) % 4)) ? fg : 0;
							}
							else
							{
								bitmap.pix(row + y, col * 14 + x) = bits & (1 << (x % 4)) ? fg : 0;
							}
						}
					}
				}

				bits = (code >> 4) & 0x0F;
				for (int y = 4; y < 8; y++)
				{
					if (((row + y) >= beginrow) && ((row + y) <= endrow))
					{
						for (int x = 0; x < 14; x++)
						{
							if (col & 1)
							{
								bitmap.pix(row + y, col * 14 + x) = bits & (1 << ((x+2) % 4)) ? fg : 0;
							}
							else
							{
								bitmap.pix(row + y, col * 14 + x) = bits & (1 << (x % 4)) ? fg : 0;
							}
						}
					}
				}
			}
		}
	}
}

void a2_video_device::dlores_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	uint32_t const start_address = use_page_2() ? 0x0800 : 0x0400;
	static const int aux_colors[16] = { 0, 2, 4, 6, 8, 0xa, 0xc, 0xe, 1, 3, 5, 7, 9, 0xb, 0xd, 0xf };

	bool const monochrome = monochrome_monitor();
	int const fg = monochrome_hue();

	/* perform adjustments */
	beginrow = (std::max)(beginrow, cliprect.top() - (cliprect.top() % 8));
	endrow = (std::min)(endrow, cliprect.bottom() - (cliprect.bottom() % 8) + 7);

	const int startrow = (beginrow / 8) * 8;
	const int stoprow = ((endrow / 8) + 1) * 8;
	const int startcol = (cliprect.left() / 14);
	const int stopcol = ((cliprect.right() / 14) + 1);

	if (!monochrome)
	{
		for (int row = startrow; row <= stoprow; row += 8)
		{
			for (int col = startcol; col < stopcol; col++)
			{
				/* calculate adderss */
				uint32_t const address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col));

				/* perform the lookup */
				uint8_t const code = m_ram_ptr[address];
				uint8_t const auxcode = m_aux_ptr[address];

				/* and now draw */
				for (int y = 0; y < 4; y++)
				{
					if (((row + y) >= beginrow) && ((row + y) <= endrow))
					{
						uint16_t *vram = &bitmap.pix(row + y, (col * 14));

						*vram++ = aux_colors[(auxcode >> 0) & 0x0F];
						*vram++ = aux_colors[(auxcode >> 0) & 0x0F];
						*vram++ = aux_colors[(auxcode >> 0) & 0x0F];
						*vram++ = aux_colors[(auxcode >> 0) & 0x0F];
						*vram++ = aux_colors[(auxcode >> 0) & 0x0F];
						*vram++ = aux_colors[(auxcode >> 0) & 0x0F];
						*vram++ = aux_colors[(auxcode >> 0) & 0x0F];
						*vram++ = (code >> 0) & 0x0F;
						*vram++ = (code >> 0) & 0x0F;
						*vram++ = (code >> 0) & 0x0F;
						*vram++ = (code >> 0) & 0x0F;
						*vram++ = (code >> 0) & 0x0F;
						*vram++ = (code >> 0) & 0x0F;
						*vram++ = (code >> 0) & 0x0F;
					}
				}
				for (int y = 4; y < 8; y++)
				{
					if (((row + y) >= beginrow) && ((row + y) <= endrow))
					{
						uint16_t *vram = &bitmap.pix(row + y, (col * 14));

						*vram++ = aux_colors[(auxcode >> 4) & 0x0F];
						*vram++ = aux_colors[(auxcode >> 4) & 0x0F];
						*vram++ = aux_colors[(auxcode >> 4) & 0x0F];
						*vram++ = aux_colors[(auxcode >> 4) & 0x0F];
						*vram++ = aux_colors[(auxcode >> 4) & 0x0F];
						*vram++ = aux_colors[(auxcode >> 4) & 0x0F];
						*vram++ = aux_colors[(auxcode >> 4) & 0x0F];
						*vram++ = (code >> 4) & 0x0F;
						*vram++ = (code >> 4) & 0x0F;
						*vram++ = (code >> 4) & 0x0F;
						*vram++ = (code >> 4) & 0x0F;
						*vram++ = (code >> 4) & 0x0F;
						*vram++ = (code >> 4) & 0x0F;
						*vram++ = (code >> 4) & 0x0F;
					}
				}
			}
		}
	}
	else
	{
		for (int row = startrow; row < stoprow; row += 8)
		{
			/* calculate address */
			uint32_t const address = start_address + ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5));

			uint8_t prev_code = (startcol == 0) ? 0 : m_ram_ptr[address + startcol - 1];

			for (int col = 0; col < 40; col++)
			{
				/* perform the lookup */
				uint8_t const code = m_ram_ptr[address + col];
				uint8_t const auxcode = m_aux_ptr[address + col];

				/* and now draw */
				for (int y = 0; y < 8; y++)
				{
					if (((row + y) >= beginrow) && ((row + y) <= endrow))
					{
						uint16_t *vram = &bitmap.pix(row + y, (col * 14));

						unsigned const color1 = (auxcode >> (y & 4)) & 0x0F;
						unsigned const color2 = (code >> (y & 4)) & 0x0F;
						unsigned allbits = (((color1 * 0x111) >> ((col * 14) & 3)) & 0x007f)
										 + (((color2 * 0x8880) >> ((col * 14) & 3)) & 0x3f80);

						// Workaround for Github issue #10760: shift everything right by one pixel
						// (losing the last pixel of each row) so that the NTSC shader sees what
						// it expects. This should be removed when there is a better fix.
						unsigned const prev_bit = (prev_code >> ((y & 4) + ((col * 14) & 3))) & 1;
						allbits = allbits * 2 + prev_bit;

						for (int x = 0; x < 14; x++)
						{
							*vram++ = allbits & (1 << x) ? fg : 0;
						}
					}
				}

				prev_code = code;
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

	int fg = (Model == model::IIGS) ? m_GSfg : monochrome_hue();
	int bg = (Model == model::IIGS) ? m_GSbg : BLACK;

	for (int row = startrow; row < stoprow; row += 8)
	{
		if ((Model == model::IIE || Model == model::IIGS) && m_80col)
		{
			for (int col = startcol; col < stopcol; col++)
			{
				/* calculate address */
				uint32_t const address = start_address + ((((row / 8) & 0x07) << 7) | (((row / 8) & 0x18) * 5 + col));
				uint32_t const aux_address = (Model == model::IIGS) ? address : (address & m_aux_mask);
				plot_text_character<Model, Invert, Flip>(bitmap, col * 14, row, 1, aux_page[aux_address], fg, bg);
				plot_text_character<Model, Invert, Flip>(bitmap, col * 14 + 7, row, 1, m_ram_ptr[address], fg, bg);
			}
		}
		else
		{
			for (int col = startcol; col < stopcol; col++)
			{
				/* calculate address */
				uint32_t const address = start_address + ((((row / 8) & 0x07) << 7) | (((row / 8) & 0x18) * 5 + col));
				if (Model == model::IIE && rgb_monitor() && m_dhires)
				{
					u8 tmp = aux_page[address];
					fg = tmp >> 4;
					bg = tmp & 0xf;
				}

				plot_text_character<Model, Invert, Flip>(bitmap, col * 14, row, 2, m_ram_ptr[address], fg, bg);
			}
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

static unsigned decode_hires_byte(uint8_t byte, unsigned last_output_bit) {
	// duplicate the bottom 7 bits by bit twiddling
	unsigned word = byte;
	word = (word ^ (word << 4)) & 0x70F;
	word = (word ^ (word << 2)) & 0x1333;
	word = (word ^ (word << 1)) & 0x1555;
	word *= 3;
	// shift right on the screen (left in the word) if the high bit is set
	if (byte & 0x80)
	{
		word = (word * 2 + last_output_bit) & 0x3FFF;
	}
	return word;
}

void a2_video_device::hgr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	/* sanity checks */
	if (beginrow < cliprect.top())
		beginrow = cliprect.top();
	if (endrow > cliprect.bottom())
		endrow = cliprect.bottom();
	if (endrow < beginrow)
		return;

	// B&W/Green/Amber monitor, CEC mono HGR mode, or IIgs $C021 monochrome HGR
	bool const monochrome = monochrome_monitor() || m_monohgr || (m_monochrome & 0x80);

	int const fg = monochrome_hue();
	int const bg = BLACK;

	uint8_t const *const vram = &m_ram_ptr[use_page_2() ? 0x4000 : 0x2000];

	// verified on h/w: setting dhires w/o 80col emulates a rev. 0 Apple ][ with no orange/blue
	uint8_t const bit7_mask = m_dhires ? 0x7f : 0xff;

	for (int row = beginrow; row <= endrow; row++)
	{
		uint8_t const *const vram_row = vram + (((row/8) & 0x07) << 7) + (((row/8) & 0x18) * 5) + ((row & 7) << 10);

		// w holds 3+14+14=31 bits: 3 bits of the previous 14-pixel group and the current and next groups.
		uint32_t w = decode_hires_byte(vram_row[0] & bit7_mask, 0) << (3 + 14);

		uint16_t *p = &bitmap.pix(row);

		for (int col = 0; col < 40; col++)
		{
			w >>= 14;
			if (col + 1 < 40)
			{
				w |= decode_hires_byte(vram_row[col + 1] & bit7_mask, w >> (3 + 13)) << (3 + 14);
			}

			if (!monochrome)
			{
				for (int b = 0; b < 14; b++)
				{
					if (((col * 14 + b) >= cliprect.left()) && ((col * 14 + b) <= cliprect.right()))
					{
						*p = rotl4b(artifact_color_lut[(w >> b) & 0x7f], col * 14 + b);
					}
					p++;
				}
			}
			else
			{
				for (int b = 0; b < 14; b++)
				{
					if (((col * 14 + b) >= cliprect.left()) && ((col * 14 + b) <= cliprect.right()))
					{
						*p = (w & (8 << b)) ? fg : bg;
					}
					p++;
				}
			}
		}
	}
}

void a2_video_device::dhgr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	uint16_t v;
	int const page = use_page_2() ? 0x4000 : 0x2000;
	int const rgbmode = rgb_monitor() ? m_rgbmode : -1;

	// B&W/Green/Amber monitor, IIgs force-monochrome-DHR setting, or IIe RGB card monochrome DHR
	bool const monochrome = monochrome_monitor() || (m_newvideo & 0x20) || rgbmode == 0;
	int const fg = monochrome_hue();

	/* sanity checks */
	if (beginrow < cliprect.top())
		beginrow = cliprect.top();
	if (endrow > cliprect.bottom())
		endrow = cliprect.bottom();
	if (endrow < beginrow)
		return;

	uint8_t const *const vram = &m_ram_ptr[page];
	uint8_t const *const vaux = (m_aux_ptr ? m_aux_ptr : vram) + page;

	uint8_t vram_row[82];
	vram_row[0] = 0;
	vram_row[81] = 0;

	for (int row = beginrow; row <= endrow; row++)
	{
		for (int col = 0; col < 40; col++)
		{
			int const offset = ((((row/8) & 0x07) << 7) | (((row/8) & 0x18) * 5 + col)) | ((row & 7) << 10);
			vram_row[1+(col*2)+0] = vaux[offset];
			vram_row[1+(col*2)+1] = vram[offset];
		}

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
			for (int col = 0; col < 80; col++)
			{
				v = vram_row[col+1];
				*p++ = v & 0xf;
				*p++ = v & 0xf;
				*p++ = v & 0xf;
				v >>= 4;
				*p++ = v & 0xf;
				*p++ = v & 0xf;
				*p++ = v & 0xf;
			}
			for (int b = 0; b < 40; b++)
			{
				*p++ = BLACK;
			}

			continue;
		}

		for (int col = 0; col < 80; col++)
		{
			uint32_t w =    (((uint32_t) vram_row[col+0] & 0x7f) <<  0)
						|   (((uint32_t) vram_row[col+1] & 0x7f) <<  7)
						|   (((uint32_t) vram_row[col+2] & 0x7f) << 14);

			if (monochrome)
			{
				// Shifting by 6 instead of 7 here shifts the entire DHGR screen right one pixel, so the leftmost pixel is
				// always black and the rightmost pixel is not shown. This is to work around a problem with the HLSL NTSC
				// shader. See Github issues #6308 and #10759. This should be changed when there is a better solution.
				w >>= 6;
				for (int b = 0; b < 7; b++)
				{
					v = (w & 1);
					w >>= 1;
					*(p++) = v ? fg : BLACK;
				}
			}
			else if (rgbmode < 0)
			{
				for (int b = 0; b < 7; b++)
				{
					*p++ = rotl4((w >> (b + 7-1)) & 0x0F, col * 7 + b);
				}
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

				if (rgbmode == 1 && !(vram_row[col+1] & 0x80))  // monochrome
				{
					for (int b = 0; b < 7; b++)
					{
						*p++ = ((w >> (b + 7)) & 1) ? WHITE : BLACK;
					}
				}
				else  // color
				{
					// In column 0 get the color from w bits 7-10 or 11-14,
					// in column 1 get the color from w bits 4-7 or 8-11, etc.
					for (int b = 0; b < 7; b++)
					{
						*p++ = rotl4((w >> (b - ((b - col) & 3) + 7)) & 0x0F, 1);
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
				dlores_update(screen, bitmap, cliprect, 0, text_start_row - 1);
			}
			else
			{
				lores_update(screen, bitmap, cliprect, 0, text_start_row - 1);
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
INPUT_PORTS_END

static INPUT_PORTS_START( a2_vidconfig_composite_rgb )
	PORT_START("a2_video_config")
	PORT_CONFNAME(0x07, 0x00, "Monitor type")
	PORT_CONFSETTING(0x00, "Color")
	PORT_CONFSETTING(0x04, "B&W")
	PORT_CONFSETTING(0x05, "Green")
	PORT_CONFSETTING(0x06, "Amber")
	PORT_CONFSETTING(0x03, "Video-7 RGB")
INPUT_PORTS_END

ioport_constructor a2_video_device_composite::device_input_ports() const { return INPUT_PORTS_NAME(a2_vidconfig_composite); }
ioport_constructor a2_video_device_composite_rgb::device_input_ports() const { return INPUT_PORTS_NAME(a2_vidconfig_composite_rgb); }
