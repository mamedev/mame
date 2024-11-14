// license:BSD-3-Clause
// copyright-holders: Mike Coates, Couriersud

// common methods shared by Century CVS and derived hardware

#include "emu.h"

#include "cvs_base.h"


void cvs_base_state::machine_start()
{
	// register state save
	save_item(NAME(m_collision_register));
	save_item(NAME(m_stars_scroll));
}

void cvs_base_state::machine_reset()
{
	m_collision_register = 0;
	m_stars_scroll = 0;
}

void cvs_base_state::write_s2650_flag(int state) // TODO: remove once set_memview is available via devcb
{
	m_ram_view.select(state);
}

uint8_t cvs_base_state::collision_r()
{
	return m_collision_register;
}

uint8_t cvs_base_state::collision_clear()
{
	if (!machine().side_effects_disabled())
		m_collision_register = 0;
	return 0;
}

// cvs stars hardware

void cvs_base_state::scroll_start()
{
	m_stars_scroll++;
}

void cvs_base_state::init_stars()
{
	int generator = 0;

	// precalculate the star background

	m_total_stars = 0;

	for (int y = 255; y >= 0; y--)
	{
		for (int x = 511; x >= 0; x--)
		{
			generator <<= 1;
			int const bit1 = BIT(~generator, 17);
			int const bit2 = BIT(generator, 5);

			if (bit1 ^ bit2)
				generator |= 1;

			if (BIT(~generator, 16) && (generator & 0xfe) == 0xfe)
			{
				if (BIT(~generator, 12) && BIT(~generator, 13))
				{
					if (m_total_stars < CVS_MAX_STARS)
					{
						m_stars[m_total_stars].x = x;
						m_stars[m_total_stars].y = y;
						m_stars[m_total_stars].code = 1;

						m_total_stars++;
					}
				}
			}
		}
	}
}

void cvs_base_state::update_stars(bitmap_ind16 &bitmap, const rectangle &cliprect, const pen_t star_pen, bool update_always)
{
	for (int offs = 0; offs < m_total_stars; offs++)
	{
		uint8_t x = (m_stars[offs].x + m_stars_scroll) >> 1;
		uint8_t y = m_stars[offs].y + ((m_stars_scroll + m_stars[offs].x) >> 9);

		if (BIT(y, 0) ^ BIT(x, 4))
		{
			if (flip_screen_x())
				x = ~x;

			if (flip_screen_y())
				y = ~y;

			if ((y >= cliprect.top()) && (y <= cliprect.bottom()) &&
				(update_always || (m_palette->pen_indirect(bitmap.pix(y, x)) == 0)))
				bitmap.pix(y, x) = star_pen;
		}
	}
}
