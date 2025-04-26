// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui/widgets.cpp

    Internal MAME widgets for the user interface.

*********************************************************************/

#include "emu.h"

#include "widgets.h"


namespace ui {

/***************************************************************************
    WIDGETS
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

widgets_manager::widgets_manager(running_machine &machine)
	: m_hilight_bitmap(std::make_unique<bitmap_argb32>(512, 1))
	, m_hilight_texture(nullptr, machine.render())
	, m_hilight_main_bitmap(std::make_unique<bitmap_argb32>(1, 128))
	, m_hilight_main_texture(nullptr, machine.render())
	, m_arrow_texture(nullptr, machine.render())
{
	render_manager &render(machine.render());

	// create a texture for hilighting items
	for (unsigned x = 0; x < 512; ++x)
	{
		unsigned const alpha((x < 50) ? ((x + 1) * 5) : (x > (511 - 50)) ? ((512 - x) * 5) : 0xff);
		m_hilight_bitmap->pix(0, x) = rgb_t(alpha, 0xff, 0xff, 0xff);
	}
	m_hilight_texture.reset(render.texture_alloc());
	m_hilight_texture->set_bitmap(*m_hilight_bitmap, m_hilight_bitmap->cliprect(), TEXFORMAT_ARGB32);

	// create a texture for hilighting items in main menu
	for (unsigned y = 0; y < 128; ++y)
	{
		constexpr unsigned r1(0), g1(169), b1 = (255); // any start color
		constexpr unsigned r2(0), g2(39), b2 = (130); // any stop color
		unsigned const r = r1 + (y * (r2 - r1) / 128);
		unsigned const g = g1 + (y * (g2 - g1) / 128);
		unsigned const b = b1 + (y * (b2 - b1) / 128);
		m_hilight_main_bitmap->pix(y, 0) = rgb_t(r, g, b);
	}
	m_hilight_main_texture.reset(render.texture_alloc());
	m_hilight_main_texture->set_bitmap(*m_hilight_main_bitmap, m_hilight_main_bitmap->cliprect(), TEXFORMAT_ARGB32);

	// create a texture for arrow icons
	m_arrow_texture.reset(render.texture_alloc(render_triangle));
}


//-------------------------------------------------
//  render_triangle - render a triangle that
//  is used for up/down arrows and left/right
//  indicators
//-------------------------------------------------

void widgets_manager::render_triangle(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param)
{
	int const halfwidth = dest.width() / 2;
	int const height = dest.height();

	// start with all-transparent
	dest.fill(rgb_t(0x00, 0x00, 0x00, 0x00));

	// render from the tip to the bottom
	for (int y = 0; y < height; y++)
	{
		int linewidth = (y * (halfwidth - 1) + (height / 2)) * 255 * 2 / height;
		uint32_t *const target = &dest.pix(y, halfwidth);

		// don't antialias if height < 12
		if (dest.height() < 12)
		{
			int pixels = (linewidth + 254) / 255;
			if (pixels % 2 == 0) pixels++;
			linewidth = pixels * 255;
		}

		// loop while we still have data to generate
		for (int x = 0; linewidth > 0; x++)
		{
			int dalpha;
			if (x == 0)
			{
				// first column we only consume one pixel
				dalpha = std::min(0xff, linewidth);
				target[x] = rgb_t(dalpha, 0xff, 0xff, 0xff);
			}
			else
			{
				// remaining columns consume two pixels, one on each side
				dalpha = std::min(0x1fe, linewidth);
				target[x] = target[-x] = rgb_t(dalpha / 2, 0xff, 0xff, 0xff);
			}

			// account for the weight we consumed
			linewidth -= dalpha;
		}
	}
}

} // namespace ui
