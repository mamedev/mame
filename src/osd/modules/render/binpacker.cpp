// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  binpacker.cpp - Simple texture packer for dynamic atlasing
//
//============================================================

#include "binpacker.h"
#include <algorithm>

bool rectangle_packer::pack(const std::vector<packable_rectangle>& rects, std::vector<std::vector<packed_rectangle>>& packs,  int pack_size)
{
	clear();

	m_pack_size = pack_size;

	// Add rects to member array, and check to make sure none is too big
	for (size_t rect = 0; rect < rects.size(); rect++)
	{
		m_rects.push_back(rectangle(0, 0, rects[rect].width(), rects[rect].height(), rects[rect].hash(), rects[rect].format(), rects[rect].rowpixels(), rects[rect].palette(), rects[rect].base()));
	}

	// Sort from greatest to least area
	std::sort(m_rects.rbegin(), m_rects.rend());

	// Pack
	while (m_num_packed < (int)m_rects.size())
	{
		int i = m_packs.size();
		m_packs.push_back(rectangle(m_pack_size));
		m_roots.push_back(i);
		if (!fill(i))
		{
			return false;
		}
	}

	// Write out
	packs.resize(m_roots.size());
	for (size_t i = 0; i < m_roots.size(); ++i)
	{
		packs[i].clear();
		add_pack_to_array(m_roots[i], packs[i]);
	}

	return true;
}

void rectangle_packer::clear()
{
	m_pack_size = 0;
	m_num_packed = 0;
	m_rects.clear();
	m_packs.clear();
	m_roots.clear();
}

bool rectangle_packer::fill(int pack)
{
	// For each rect
	for (size_t rect = 0; rect < m_rects.size(); ++rect)
	{
		// If it's not already packed
		if (!m_rects[rect].packed)
		{
			// If it fits in the current working area
			if (fits(m_rects[rect], m_packs[pack]))
			{
				// Store in lower-left of working area, split, and recurse
				m_num_packed++;
				split(pack, rect);
				fill(m_packs[pack].children[0]);
				fill(m_packs[pack].children[1]);
				return true;
			}
		}
	}
	return false;
}

void rectangle_packer::split(int pack, int rect)
{
	// Split the working area either horizontally or vertically with respect
	// to the rect we're storing, such that we get the largest possible child
	// area.

	rectangle left = m_packs[pack];
	rectangle right = m_packs[pack];
	rectangle bottom = m_packs[pack];
	rectangle top = m_packs[pack];

	left.y += m_rects[rect].h;
	left.w = m_rects[rect].w;
	left.h -= m_rects[rect].h;

	right.x += m_rects[rect].w;
	right.w -= m_rects[rect].w;

	bottom.x += m_rects[rect].w;
	bottom.h = m_rects[rect].h;
	bottom.w -= m_rects[rect].w;

	top.y += m_rects[rect].h;
	top.h -= m_rects[rect].h;

	int max_lr_area = left.get_area();
	if (right.get_area() > max_lr_area)
	{
		max_lr_area = right.get_area();
	}

	int max_bt_area = bottom.get_area();
	if (top.get_area() > max_bt_area)
	{
		max_bt_area = top.get_area();
	}

	if (max_lr_area > max_bt_area)
	{
		if (left.get_area() > right.get_area())
		{
			m_packs.push_back(left);
			m_packs.push_back(right);
		}
		else
		{
			m_packs.push_back(right);
			m_packs.push_back(left);
		}
	}
	else
	{
		if (bottom.get_area() > top.get_area())
		{
			m_packs.push_back(bottom);
			m_packs.push_back(top);
		}
		else
		{
			m_packs.push_back(top);
			m_packs.push_back(bottom);
		}
	}

	// This pack area now represents the rect we've just stored, so save the
	// relevant info to it, and assign children.
	m_packs[pack].w = m_rects[rect].w;
	m_packs[pack].h = m_rects[rect].h;
	m_packs[pack].hash = m_rects[rect].hash;
	m_packs[pack].format = m_rects[rect].format;
	m_packs[pack].rowpixels = m_rects[rect].rowpixels;
	m_packs[pack].palette = m_rects[rect].palette;
	m_packs[pack].base = m_rects[rect].base;
	m_packs[pack].children[0] = m_packs.size() - 2;
	m_packs[pack].children[1] = m_packs.size() - 1;

	// Done with the rect
	m_rects[rect].packed = true;

}

bool rectangle_packer::fits(rectangle& rect1, const rectangle& rect2)
{
	// Check to see if rect1 fits in rect2

	if (rect1.w <= rect2.w && rect1.h <= rect2.h)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void rectangle_packer::add_pack_to_array(int pack, std::vector<packed_rectangle>& array) const
{
	if (m_packs[pack].hash != 0)
	{
		array.push_back(packed_rectangle(m_packs[pack].hash, m_packs[pack].format,
			m_packs[pack].w, m_packs[pack].h, m_packs[pack].x, m_packs[pack].y,
			m_packs[pack].rowpixels, m_packs[pack].palette, m_packs[pack].base));

		if (m_packs[pack].children[0] != -1)
		{
			add_pack_to_array(m_packs[pack].children[0], array);
		}

		if (m_packs[pack].children[1] != -1)
		{
			add_pack_to_array(m_packs[pack].children[1], array);
		}
	}
}
