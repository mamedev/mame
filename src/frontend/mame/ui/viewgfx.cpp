// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui/viewgfx.cpp

    Internal graphics viewer.

*********************************************************************/

#include "emu.h"
#include "ui/viewgfx.h"


#include "emupal.h"
#include "render.h"
#include "rendfont.h"
#include "rendutil.h"
#include "screen.h"
#include "tilemap.h"
#include "uiinput.h"

#include <cmath>
#include <vector>


namespace {

class gfx_viewer
{
public:
	gfx_viewer(running_machine &machine) :
		m_machine(machine),
		m_palette(machine),
		m_gfxset(machine),
		m_tilemap(machine)
	{
	}

	// copy constructor needed to make std::any happy
	gfx_viewer(gfx_viewer const &that) :
		gfx_viewer(that.m_machine)
	{
	}

	~gfx_viewer()
	{
		if (m_texture)
			m_machine.render().texture_free(m_texture);
	}

	uint32_t handle(mame_ui_manager &mui, render_container &container, bool uistate)
	{
		// implicitly cancel if there's nothing to display
		if (!is_relevant())
			return cancel(uistate);

		// always mark the bitmap dirty if not paused
		if (!m_machine.paused())
			m_bitmap_dirty = true;

		// try to display the selected view
		while (true)
		{
			switch (m_mode)
			{
			case view::PALETTE:
				if (m_palette.interface())
					return handle_palette(mui, container, uistate);
				m_mode = view::GFXSET;
				break;

			case view::GFXSET:
				if (m_gfxset.has_gfx())
					return handle_gfxset(mui, container, uistate);
				m_mode = view::TILEMAP;
				break;

			case view::TILEMAP:
				if (m_machine.tilemap().count())
					return handle_tilemap(mui, container, uistate);
				m_mode = view::PALETTE;
				break;
			}
		}
	}

private:
	enum class view
	{
		PALETTE = 0,
		GFXSET,
		TILEMAP
	};

	class palette
	{
	public:
		enum class subset
		{
			PENS,
			INDIRECT
		};

		palette(running_machine &machine) :
			m_count(palette_interface_enumerator(machine.root_device()).count())
		{
			if (m_count)
				set_device(machine);
		}

		device_palette_interface *interface() const noexcept
		{
			return m_interface;
		}

		bool indirect() const noexcept
		{
			return subset::INDIRECT == m_which;
		}

		unsigned columns() const noexcept
		{
			return m_columns;
		}

		unsigned index(unsigned x, unsigned y) const noexcept
		{
			return m_offset + (y * m_columns) + x;
		}

		void handle_keys(running_machine &machine);

	private:
		void set_device(running_machine &machine)
		{
			m_interface = palette_interface_enumerator(machine.root_device()).byindex(m_index);
		}

		void next_group(running_machine &machine) noexcept
		{
			if ((subset::PENS == m_which) && m_interface->indirect_entries())
			{
				m_which = subset::INDIRECT;
			}
			else if ((m_count - 1) > m_index)
			{
				++m_index;
				set_device(machine);
				m_which = subset::PENS;
			}
		}

		void prev_group(running_machine &machine) noexcept
		{
			if (subset::INDIRECT == m_which)
			{
				m_which = subset::PENS;
			}
			else if (0 < m_index)
			{
				--m_index;
				set_device(machine);
				m_which = m_interface->indirect_entries() ? subset::INDIRECT : subset::PENS;
			}
		}

		device_palette_interface *m_interface = nullptr;
		unsigned const m_count;
		unsigned m_index = 0U;
		subset m_which = subset::PENS;
		unsigned m_columns = 16U;
		int m_offset = 0;
	};

	class gfxset
	{
	public:
		struct setinfo
		{
			void next_color() noexcept
			{
				if ((m_color_count - 1) > m_color)
					++m_color;
				else
					m_color = 0U;
			}

			void prev_color() noexcept
			{
				if (m_color)
					--m_color;
				else
					m_color = m_color_count - 1;
			}

			device_palette_interface *m_palette = nullptr;
			int m_offset = 0;
			unsigned m_color = 0;
			unsigned m_color_count = 0U;
			uint8_t m_rotate = 0U;
			uint8_t m_columns = 16U;
			bool m_integer_scale = false;
		};

		class devinfo
		{
		public:
			devinfo(device_gfx_interface &interface, device_palette_interface *first_palette, u8 rotate) :
				m_interface(&interface),
				m_setcount(0U)
			{
				for (gfx_element *gfx; (MAX_GFX_ELEMENTS > m_setcount) && ((gfx = interface.gfx(m_setcount)) != nullptr); ++m_setcount)
				{
					auto &set = m_sets[m_setcount];
					if (gfx->has_palette())
					{
						set.m_palette = &gfx->palette();
						set.m_color_count = gfx->colors();
					}
					else
					{
						set.m_palette = first_palette;
						set.m_color_count = first_palette->entries() / gfx->granularity();
						if (!set.m_color_count)
							set.m_color_count = 1U;
					}
					set.m_rotate = rotate;
				}
			}

			device_gfx_interface &interface() const noexcept
			{
				return *m_interface;
			}

			unsigned setcount() const noexcept
			{
				return m_setcount;
			}

			setinfo const &set(unsigned index) const noexcept
			{
				return m_sets[index];
			}

			setinfo &set(unsigned index) noexcept
			{
				return m_sets[index];
			}

		private:
			device_gfx_interface *m_interface;
			unsigned m_setcount;
			setinfo m_sets[MAX_GFX_ELEMENTS];
		};

		gfxset(running_machine &machine)
		{
			// get useful defaults
			uint8_t const rotate = machine.system().flags & machine_flags::MASK_ORIENTATION;
			device_palette_interface *const first_palette = palette_interface_enumerator(machine.root_device()).first();

			// iterate over graphics decoders
			for (device_gfx_interface &interface : gfx_interface_enumerator(machine.root_device()))
			{
				// if there are any exposed graphics sets, add the device
				if (interface.gfx(0U))
					m_devices.emplace_back(interface, first_palette, rotate);
			}
		}

		bool has_gfx() const noexcept
		{
			return !m_devices.empty();
		}

		bool handle_keys(running_machine &machine, int xcells, int ycells);

		std::vector<devinfo> m_devices;
		unsigned m_device = 0U;
		unsigned m_set = 0U;

	private:
		bool next_group() noexcept
		{
			if ((m_devices[m_device].setcount() - 1) > m_set)
			{
				++m_set;
				return true;
			}
			else if ((m_devices.size() - 1) > m_device)
			{
				++m_device;
				m_set = 0U;
				return true;
			}
			else
			{
				return false;
			}
		}

		bool prev_group() noexcept
		{
			if (m_set)
			{
				--m_set;
				return true;
			}
			else if (m_device)
			{
				--m_device;
				m_set = m_devices[m_device].setcount() - 1;
				return true;
			}
			else
			{
				return false;
			}
		}
	};

	class tilemap
	{
	public:
		tilemap(running_machine &machine)
		{
			uint8_t const rotate = machine.system().flags & machine_flags::MASK_ORIENTATION;
			m_info.resize(machine.tilemap().count());
			for (auto &info : m_info)
				info.m_rotate = rotate;
		}

		unsigned index() const noexcept
		{
			return m_index;
		}

		float zoom_scale() const noexcept
		{
			auto const &info = m_info[m_index];
			return info.m_zoom_frac ? (1.0f / float(info.m_zoom)) : float(info.m_zoom);
		}

		bool auto_zoom() const noexcept
		{
			return m_info[m_index].m_auto_zoom;
		}

		uint8_t rotate() const noexcept
		{
			return m_info[m_index].m_rotate;
		}

		uint32_t flags() const noexcept
		{
			return m_info[m_index].m_flags;
		}

		int xoffs() const noexcept
		{
			return m_info[m_index].m_xoffs;
		}

		int yoffs() const noexcept
		{
			return m_info[m_index].m_yoffs;
		}

		bool handle_keys(running_machine &machine, float pixelscale);

	private:
		static constexpr int MAX_ZOOM_LEVEL = 8; // maximum tilemap zoom ratio screen:native
		static constexpr int MIN_ZOOM_LEVEL = 8; // minimum tilemap zoom ratio native:screen

		struct info
		{
			bool zoom_in(float pixelscale) noexcept
			{
				if (m_auto_zoom)
				{
					// auto zoom never uses fractional factors
					m_zoom = std::min<int>(std::lround(pixelscale) + 1, MAX_ZOOM_LEVEL);
					m_zoom_frac = false;
					m_auto_zoom = false;
					return true;
				}
				else if (m_zoom_frac)
				{
					m_zoom--;
					if (m_zoom == 1)
						m_zoom_frac = false; // entering integer zoom range
					return true;
				}
				else if (MAX_ZOOM_LEVEL > m_zoom)
				{
					m_zoom++; // remaining in integer zoom range
					return true;
				}
				else
				{
					return false;
				}
			}

			bool zoom_out(float pixelscale) noexcept
			{
				if (m_auto_zoom)
				{
					// auto zoom never uses fractional factors
					m_zoom = std::lround(pixelscale) - 1;
					m_zoom_frac = !m_zoom;
					if (m_zoom_frac)
						m_zoom = 2;
					m_auto_zoom = false;
					return true;
				}
				else if (!m_zoom_frac)
				{
					if (m_zoom == 1)
					{
						m_zoom++;
						m_zoom_frac = true; // entering fractional zoom range
					}
					else
					{
						m_zoom--; // remaining in integer zoom range
					}
					return true;
				}
				else if (MIN_ZOOM_LEVEL > m_zoom)
				{
					m_zoom++; // remaining in fractional zoom range
					return true;
				}
				else
				{
					return false;
				}
			}

			bool next_category() noexcept
			{
				if (TILEMAP_DRAW_ALL_CATEGORIES == m_flags)
				{
					m_flags = 0U;
					return true;
				}
				else if (TILEMAP_DRAW_CATEGORY_MASK > m_flags)
				{
					++m_flags;
					return true;
				}
				else
				{
					return false;
				}
			}

			bool prev_catagory() noexcept
			{
				if (!m_flags)
				{
					m_flags = TILEMAP_DRAW_ALL_CATEGORIES;
					return true;
				}
				else if (TILEMAP_DRAW_ALL_CATEGORIES != m_flags)
				{
					--m_flags;
					return true;
				}
				else
				{
					return false;
				}
			}

			int m_xoffs = 0;
			int m_yoffs = 0;
			unsigned m_zoom = 1U;
			bool m_zoom_frac = false;
			bool m_auto_zoom = true;
			uint8_t m_rotate = 0U;
			uint32_t m_flags = TILEMAP_DRAW_ALL_CATEGORIES;
		};

		static int scroll_step(running_machine &machine)
		{
			auto &input = machine.input();
			if (input.code_pressed(KEYCODE_LCONTROL) || input.code_pressed(KEYCODE_RCONTROL))
				return 64;
			else if (input.code_pressed(KEYCODE_LSHIFT) || input.code_pressed(KEYCODE_RSHIFT))
				return 1;
			else
				return 8;
		}

		std::vector<info> m_info;
		unsigned m_index = 0U;
	};

	bool is_relevant() const noexcept
	{
		return m_palette.interface() || m_gfxset.has_gfx() || m_machine.tilemap().count();
	}

	uint32_t handle_general_keys(bool uistate)
	{
		auto &input = m_machine.ui_input();

		// UI select cycles through views
		if (input.pressed(IPT_UI_SELECT))
		{
			m_mode = view((int(m_mode) + 1) % 3);
			m_bitmap_dirty = true;
		}

		// pause does what you'd expect
		if (input.pressed(IPT_UI_PAUSE))
		{
			if (m_machine.paused())
				m_machine.resume();
			else
				m_machine.pause();
		}

		// cancel or graphics viewer dismisses the viewer
		if (input.pressed(IPT_UI_CANCEL) || input.pressed(IPT_UI_SHOW_GFX))
			return cancel(uistate);

		return uistate;
	}

	uint32_t cancel(bool uistate)
	{
		if (!uistate)
			m_machine.resume();
		m_bitmap_dirty = true;
		return UI_HANDLER_CANCEL;
	}

	uint32_t handle_palette(mame_ui_manager &mui, render_container &container, bool uistate);
	uint32_t handle_gfxset(mame_ui_manager &mui, render_container &container, bool uistate);
	uint32_t handle_tilemap(mame_ui_manager &mui, render_container &container, bool uistate);

	void update_gfxset_bitmap(int xcells, int ycells, gfx_element &gfx);
	void update_tilemap_bitmap(int width, int height);

	void gfxset_draw_item(gfx_element &gfx, int index, int dstx, int dsty, gfxset::setinfo const &info);

	void resize_bitmap(int32_t width, int32_t height)
	{
		if (!m_bitmap.valid() || !m_texture || (m_bitmap.width() != width) || (m_bitmap.height() != height))
		{
			// free the old stuff
			if (m_texture)
				m_machine.render().texture_free(m_texture);

			// allocate new stuff
			m_bitmap.resize(width, height);
			m_texture = m_machine.render().texture_alloc();
			m_texture->set_bitmap(m_bitmap, m_bitmap.cliprect(), TEXFORMAT_ARGB32);

			// force a redraw
			m_bitmap_dirty = true;
		}
	}

	bool map_mouse(render_container &container, render_bounds const &clip, float &x, float &y) const
	{
		int32_t target_x, target_y;
		bool button;
		render_target *const target = m_machine.ui_input().find_mouse(&target_x, &target_y, &button);
		if (!target)
			return false;
		else if (!target->map_point_container(target_x, target_y, container, x, y))
			return false;
		else
			return clip.includes(x, y);
	}

	running_machine &m_machine;
	view m_mode = view::PALETTE;

	bitmap_rgb32 m_bitmap;
	render_texture *m_texture = nullptr;
	bool m_bitmap_dirty = false;

	palette m_palette;
	gfxset m_gfxset;
	tilemap m_tilemap;
};


void gfx_viewer::palette::handle_keys(running_machine &machine)
{
	auto &input = machine.ui_input();

	// handle zoom (minus,plus)
	if (input.pressed(IPT_UI_ZOOM_OUT))
		m_columns = std::min<unsigned>(m_columns * 2, 64);
	if (input.pressed(IPT_UI_ZOOM_IN))
		m_columns = std::max<unsigned>(m_columns / 2, 4);
	if (input.pressed(IPT_UI_ZOOM_DEFAULT))
		m_columns = 16;

	// handle colormap selection (open bracket,close bracket)
	if (input.pressed(IPT_UI_PREV_GROUP))
		prev_group(machine);
	if (input.pressed(IPT_UI_NEXT_GROUP))
		next_group(machine);

	// cache some info in locals
	int const total = (subset::INDIRECT == m_which) ? m_interface->indirect_entries() : m_interface->entries();

	// determine number of entries per row and total
	int const rowcount = m_columns;
	int const screencount = rowcount * rowcount;

	// handle keyboard navigation
	if (input.pressed_repeat(IPT_UI_UP, 4))
		m_offset -= rowcount;
	if (input.pressed_repeat(IPT_UI_DOWN, 4))
		m_offset += rowcount;
	if (input.pressed_repeat(IPT_UI_PAGE_UP, 6))
		m_offset -= screencount;
	if (input.pressed_repeat(IPT_UI_PAGE_DOWN, 6))
		m_offset += screencount;
	if (input.pressed_repeat(IPT_UI_HOME, 4))
		m_offset = 0;
	if (input.pressed_repeat(IPT_UI_END, 4))
		m_offset = total;

	// clamp within range
	if (m_offset + screencount > ((total + rowcount - 1) / rowcount) * rowcount)
		m_offset = ((total + rowcount - 1) / rowcount) * rowcount - screencount;
	if (m_offset < 0)
		m_offset = 0;
}


bool gfx_viewer::gfxset::handle_keys(running_machine &machine, int xcells, int ycells)
{
	auto &input = machine.ui_input();
	bool const shift_pressed = machine.input().code_pressed(KEYCODE_LSHIFT) || machine.input().code_pressed(KEYCODE_RSHIFT);
	bool result = false;

	// handle previous/next group
	if (input.pressed(IPT_UI_PREV_GROUP) && prev_group())
		result = true;
	if (input.pressed(IPT_UI_NEXT_GROUP) && next_group())
		result = true;

	auto &info = m_devices[m_device];
	auto &set = info.set(m_set);
	auto &gfx = *info.interface().gfx(m_set);

	// handle cells per line (0/-/=)
	if (input.pressed(IPT_UI_ZOOM_OUT) && (xcells < 128))
	{
		set.m_columns = xcells + 1;
		set.m_integer_scale = shift_pressed;
		result = true;
	}
	if (input.pressed(IPT_UI_ZOOM_IN) && (xcells > 2))
	{
		set.m_columns = xcells - 1;
		set.m_integer_scale = shift_pressed;
		result = true;
	}
	if (input.pressed(IPT_UI_ZOOM_DEFAULT) && ((xcells != 16) || (set.m_integer_scale != shift_pressed)))
	{
		set.m_columns = 16;
		set.m_integer_scale = shift_pressed;
		result = true;
	}

	// handle rotation (R)
	if (input.pressed(IPT_UI_ROTATE))
	{
		set.m_rotate = orientation_add(ROT90, set.m_rotate);
		result = true;
	}

	// handle navigation within the cells (up,down,pgup,pgdown)
	if (input.pressed_repeat(IPT_UI_UP, 4))
	{
		set.m_offset -= xcells;
		result = true;
	}
	if (input.pressed_repeat(IPT_UI_DOWN, 4))
	{
		set.m_offset += xcells;
		result = true;
	}
	if (input.pressed_repeat(IPT_UI_PAGE_UP, 6))
	{
		set.m_offset -= xcells * ycells;
		result = true;
	}
	if (input.pressed_repeat(IPT_UI_PAGE_DOWN, 6))
	{
		set.m_offset += xcells * ycells;
		result = true;
	}
	if (input.pressed_repeat(IPT_UI_HOME, 4))
	{
		set.m_offset = 0;
		result = true;
	}
	if (input.pressed_repeat(IPT_UI_END, 4))
	{
		set.m_offset = gfx.elements();
		result = true;
	}

	// clamp within range
	if (set.m_offset + xcells * ycells > ((gfx.elements() + xcells - 1) / xcells) * xcells)
	{
		set.m_offset = ((gfx.elements() + xcells - 1) / xcells) * xcells - xcells * ycells;
		result = true;
	}
	if (set.m_offset < 0)
	{
		set.m_offset = 0;
		result = true;
	}

	// handle color selection (left,right)
	if (input.pressed_repeat(IPT_UI_LEFT, 4))
	{
		set.prev_color();
		result = true;
	}
	if (input.pressed_repeat(IPT_UI_RIGHT, 4))
	{
		set.next_color();
		result = true;
	}

	return result;
}


bool gfx_viewer::tilemap::handle_keys(running_machine &machine, float pixelscale)
{
	auto &input = machine.ui_input();
	bool result = false;

	// handle tilemap selection (open bracket,close bracket)
	if (input.pressed(IPT_UI_PREV_GROUP) && m_index > 0)
	{
		m_index--;
		result = true;
	}
	if (input.pressed(IPT_UI_NEXT_GROUP) && ((m_info.size() - 1) > m_index))
	{
		m_index++;
		result = true;
	}

	auto &info = m_info[m_index];

	// handle zoom (minus,plus)
	if (input.pressed(IPT_UI_ZOOM_OUT) && info.zoom_out(pixelscale))
	{
		result = true;
		machine.popmessage(info.m_zoom_frac ? _("Zoom = 1/%1$d") : _("Zoom = %1$d"), info.m_zoom);
	}
	if (input.pressed(IPT_UI_ZOOM_IN) && info.zoom_in(pixelscale))
	{
		result = true;
		machine.popmessage(info.m_zoom_frac ? _("Zoom = 1/%1$d") : _("Zoom = %1$d"), info.m_zoom);
	}
	if (input.pressed(IPT_UI_ZOOM_DEFAULT) && !info.m_auto_zoom)
	{
		info.m_auto_zoom = true;
		machine.popmessage(_("Expand to fit"));
	}

	// handle rotation (R)
	if (input.pressed(IPT_UI_ROTATE))
	{
		info.m_rotate = orientation_add(ROT90, info.m_rotate);
		result = true;
	}

	// return to (0,0) (HOME)
	if (input.pressed(IPT_UI_HOME))
	{
		info.m_xoffs = 0;
		info.m_yoffs = 0;
		result = true;
	}

	// handle flags (category)
	if (input.pressed(IPT_UI_PAGE_UP) && info.prev_catagory())
	{
		result = true;
		if (TILEMAP_DRAW_ALL_CATEGORIES == info.m_flags)
			machine.popmessage("Category All");
		else
			machine.popmessage("Category = %d", info.m_flags);
	}
	if (input.pressed(IPT_UI_PAGE_DOWN) && info.next_category())
	{
		result = true;
		machine.popmessage("Category = %d", info.m_flags);
	}

	// handle navigation (up,down,left,right), taking orientation into account
	int const step = scroll_step(machine); // this may be applied more than once if multiple directions are pressed
	if (input.pressed_repeat(IPT_UI_UP, 4))
	{
		if (info.m_rotate & ORIENTATION_SWAP_XY)
			info.m_xoffs -= (info.m_rotate & ORIENTATION_FLIP_Y) ? -step : step;
		else
			info.m_yoffs -= (info.m_rotate & ORIENTATION_FLIP_Y) ? -step : step;
		result = true;
	}
	if (input.pressed_repeat(IPT_UI_DOWN, 4))
	{
		if (info.m_rotate & ORIENTATION_SWAP_XY)
			info.m_xoffs += (info.m_rotate & ORIENTATION_FLIP_Y) ? -step : step;
		else
			info.m_yoffs += (info.m_rotate & ORIENTATION_FLIP_Y) ? -step : step;
		result = true;
	}
	if (input.pressed_repeat(IPT_UI_LEFT, 6))
	{
		if (info.m_rotate & ORIENTATION_SWAP_XY)
			info.m_yoffs -= (info.m_rotate & ORIENTATION_FLIP_X) ? -step : step;
		else
			info.m_xoffs -= (info.m_rotate & ORIENTATION_FLIP_X) ? -step : step;
		result = true;
	}
	if (input.pressed_repeat(IPT_UI_RIGHT, 6))
	{
		if (info.m_rotate & ORIENTATION_SWAP_XY)
			info.m_yoffs += (info.m_rotate & ORIENTATION_FLIP_X) ? -step : step;
		else
			info.m_xoffs += (info.m_rotate & ORIENTATION_FLIP_X) ? -step : step;
		result = true;
	}

	// cache some info in locals
	tilemap_t *const tilemap = machine.tilemap().find(m_index);
	uint32_t const mapwidth = tilemap->width();
	uint32_t const mapheight = tilemap->height();

	// clamp within range
	while (info.m_xoffs < 0)
		info.m_xoffs += mapwidth;
	while (info.m_xoffs >= mapwidth)
		info.m_xoffs -= mapwidth;
	while (info.m_yoffs < 0)
		info.m_yoffs += mapheight;
	while (info.m_yoffs >= mapheight)
		info.m_yoffs -= mapheight;

	return result;
}


uint32_t gfx_viewer::handle_palette(mame_ui_manager &mui, render_container &container, bool uistate)
{
	device_palette_interface &palette = *m_palette.interface();
	palette_device *const paldev = dynamic_cast<palette_device *>(&palette.device());

	bool const indirect = m_palette.indirect();
	unsigned const total = indirect ? palette.indirect_entries() : palette.entries();
	rgb_t const *const raw_color = palette.palette()->entry_list_raw();

	// add a half character padding for the box
	render_font *const ui_font = mui.get_font();
	float const aspect = m_machine.render().ui_aspect(&container);
	float const chheight = mui.get_line_height();
	float const chwidth = ui_font->char_width(chheight, aspect, '0');
	render_bounds const boxbounds{
			0.0f + (0.5f * chwidth),
			0.0f + (0.5f * chheight),
			1.0f - (0.5f * chwidth),
			1.0f - (0.5f * chheight) };

	// the character cell box bounds starts a half character in from the box
	render_bounds cellboxbounds = boxbounds;
	cellboxbounds.x0 += 0.5f * chwidth;
	cellboxbounds.y0 += 0.5f * chheight;
	cellboxbounds.x1 -= 0.5f * chwidth;
	cellboxbounds.y1 -= 0.5f * chheight;

	// add space on the left for 5 characters of text, plus a half character of padding
	cellboxbounds.x0 += 5.5f * chwidth;

	// add space on the top for a title, a half line of padding, a header, and another half line
	cellboxbounds.y0 += 3.0f * chheight;

	// compute the cell size
	float const cellwidth = (cellboxbounds.x1 - cellboxbounds.x0) / float(m_palette.columns());
	float const cellheight = (cellboxbounds.y1 - cellboxbounds.y0) / float(m_palette.columns());

	// figure out the title
	std::ostringstream title_buf;
	util::stream_format(title_buf, "'%s'", palette.device().tag());
	if (palette.indirect_entries() > 0)
		title_buf << (indirect ? _(" COLORS") : _(" PENS"));

	// if the mouse pointer is over one of our cells, add some info about the corresponding palette entry
	float mouse_x, mouse_y;
	if (map_mouse(container, cellboxbounds, mouse_x, mouse_y))
	{
		int const index = m_palette.index(int((mouse_x - cellboxbounds.x0) / cellwidth), int((mouse_y - cellboxbounds.y0) / cellheight));
		if (index < total)
		{
			util::stream_format(title_buf, " #%X", index);
			if (palette.indirect_entries() && indirect)
				util::stream_format(title_buf, " => %X", palette.pen_indirect(index));
			else if (paldev && paldev->basemem().base())
				util::stream_format(title_buf, " = %X", paldev->read_entry(index));

			rgb_t const col = indirect ? palette.indirect_color(index) : raw_color[index];
			util::stream_format(title_buf, " (A:%X R:%X G:%X B:%X)", col.a(), col.r(), col.g(), col.b());
		}
	}

	float x0, y0;

	// expand the outer box to fit the title
	std::string const title = std::move(title_buf).str();
	float const titlewidth = ui_font->string_width(chheight, aspect, title);
	x0 = 0.0f;
	if (boxbounds.x1 - boxbounds.x0 < titlewidth + chwidth)
		x0 = boxbounds.x0 - (0.5f - 0.5f * (titlewidth + chwidth));

	// go ahead and draw the outer box now
	mui.draw_outlined_box(container, boxbounds.x0 - x0, boxbounds.y0, boxbounds.x1 + x0, boxbounds.y1, mui.colors().gfxviewer_bg_color());

	// draw the title
	x0 = 0.5f - 0.5f * titlewidth;
	y0 = boxbounds.y0 + 0.5f * chheight;
	for (auto ch : title)
	{
		container.add_char(x0, y0, chheight, aspect, rgb_t::white(), *ui_font, ch);
		x0 += ui_font->char_width(chheight, aspect, ch);
	}

	// draw the top column headers
	int const rowskip = int(chwidth / cellwidth);
	for (int x = 0; x < m_palette.columns(); x += 1 + rowskip)
	{
		x0 = boxbounds.x0 + 6.0f * chwidth + float(x) * cellwidth;
		y0 = boxbounds.y0 + 2.0f * chheight;
		container.add_char(x0 + 0.5f * (cellwidth - chwidth), y0, chheight, aspect, rgb_t::white(), *ui_font, "0123456789ABCDEF"[x & 0xf]);

		// if we're skipping, draw a point between the character and the box to indicate which one it's referring to
		if (rowskip)
			container.add_point(x0 + 0.5f * cellwidth, 0.5f * (y0 + chheight + cellboxbounds.y0), UI_LINE_WIDTH, rgb_t::white(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}

	// draw the side column headers
	int const colskip = int(chheight / cellheight);
	for (int y = 0; y < m_palette.columns(); y += 1 + colskip)
	{
		// only display if there is data to show
		unsigned const index = m_palette.index(0, y);
		if (index < total)
		{
			// if we're skipping, draw a point between the character and the box to indicate which
			// one it's referring to
			x0 = boxbounds.x0 + 5.5f * chwidth;
			y0 = boxbounds.y0 + 3.5f * chheight + float(y) * cellheight;
			if (colskip != 0)
				container.add_point(0.5f * (x0 + cellboxbounds.x0), y0 + 0.5f * cellheight, UI_LINE_WIDTH, rgb_t::white(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

			// draw the row header
			char buffer[10];
			sprintf(buffer, "%5X", index);
			for (int x = 4; x >= 0; x--)
			{
				x0 -= ui_font->char_width(chheight, aspect, buffer[x]);
				container.add_char(x0, y0 + 0.5f * (cellheight - chheight), chheight, aspect, rgb_t::white(), *ui_font, buffer[x]);
			}
		}
	}

	// now add the rectangles for the colors
	for (int y = 0; y < m_palette.columns(); y++)
	{
		for (int x = 0; x < m_palette.columns(); x++)
		{
			int const index = m_palette.index(x, y);
			if (index < total)
			{
				pen_t const pen = indirect ? palette.indirect_color(index) : raw_color[index];
				container.add_rect(
						cellboxbounds.x0 + x * cellwidth, cellboxbounds.y0 + y * cellheight,
						cellboxbounds.x0 + (x + 1) * cellwidth, cellboxbounds.y0 + (y + 1) * cellheight,
						0xff000000 | pen, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			}
		}
	}

	// handle keys
	m_palette.handle_keys(m_machine);
	return handle_general_keys(uistate);
}


uint32_t gfx_viewer::handle_gfxset(mame_ui_manager &mui, render_container &container, bool uistate)
{
	// get graphics info
	auto &info = m_gfxset.m_devices[m_gfxset.m_device];
	auto &set = info.set(m_gfxset.m_set);
	device_gfx_interface &interface = info.interface();
	gfx_element &gfx = *interface.gfx(m_gfxset.m_set);

	// get some UI metrics
	render_font *const ui_font = mui.get_font();
	int const targwidth = m_machine.render().ui_target().width();
	int const targheight = m_machine.render().ui_target().height();
	float const aspect = m_machine.render().ui_aspect(&container);
	float const chheight = mui.get_line_height();
	float const chwidth = ui_font->char_width(chheight, aspect, '0');

	// add a half character padding for the box
	render_bounds boxbounds{
			0.0f + (0.5f * chwidth),
			0.0f + (0.5f * chheight),
			1.0f - (0.5f * chwidth),
			1.0f - (0.5f * chheight) };

	// the character cell box bounds starts a half character in from the box
	render_bounds cellboxbounds = boxbounds;
	cellboxbounds.x0 += 0.5f * chwidth;
	cellboxbounds.y0 += 0.5f * chheight;
	cellboxbounds.x1 -= 0.5f * chwidth;
	cellboxbounds.y1 -= 0.5f * chheight;

	// add space on the left for 5 characters of text, plus a half character of padding
	cellboxbounds.x0 += 5.5f * chwidth;

	// add space on the top for a title, a half line of padding, a header, and another half line
	cellboxbounds.y0 += 3.0f * chheight;

	// convert back to pixels
	float cellboxwidth = cellboxbounds.width() * float(targwidth);
	float cellboxheight = cellboxbounds.height() * float(targheight);

	// compute the number of source pixels in a cell
	int const cellxpix = 1 + ((set.m_rotate & ORIENTATION_SWAP_XY) ? gfx.height() : gfx.width());
	int const cellypix = 1 + ((set.m_rotate & ORIENTATION_SWAP_XY) ? gfx.width() : gfx.height());

	// compute the largest pixel scale factor that still fits
	int xcells = set.m_columns;
	float pixelscale = 0.0f;
	while (xcells > 1)
	{
		pixelscale = cellboxwidth / (xcells * cellxpix);
		if (set.m_integer_scale)
			pixelscale = std::floor(pixelscale);
		if (0.25f <= pixelscale)
			break;
		xcells--;
	}
	if (0.0f == pixelscale)
		pixelscale = cellboxwidth / (xcells * cellxpix);

	// in the Y direction, we just display as many as we can
	int ycells = int(cellboxheight / (pixelscale * cellypix));
	if (!ycells)
	{
		ycells = 1;
		pixelscale = cellboxheight / cellypix;
		xcells = int(cellboxwidth / (pixelscale * cellxpix));
	}

	// now determine the actual cellbox size
	set.m_columns = xcells;
	cellboxwidth = std::min(cellboxwidth, xcells * pixelscale * cellxpix);
	cellboxheight = std::min(cellboxheight, ycells * pixelscale * cellypix);

	// compute the size of a single cell at this pixel scale factor
	float const cellwidth = (cellboxwidth / xcells) / targwidth;
	float const cellheight = (cellboxheight / ycells) / targheight;

	// working from the new width/height, recompute the boxbounds
	float const fullwidth = cellboxwidth / targwidth + 6.5f * chwidth;
	float const fullheight = cellboxheight / targheight + 4.0f * chheight;

	// recompute boxbounds from this
	boxbounds.x0 = (1.0f - fullwidth) * 0.5f;
	boxbounds.y0 = (1.0f - fullheight) * 0.5f;
	boxbounds.x1 = boxbounds.x0 + fullwidth;
	boxbounds.y1 = boxbounds.y0 + fullheight;

	// recompute cellboxbounds
	cellboxbounds.x0 = boxbounds.x0 + 6.0f * chwidth;
	cellboxbounds.y0 = boxbounds.y0 + 3.5f * chheight;
	cellboxbounds.x1 = cellboxbounds.x0 + cellboxwidth / float(targwidth);
	cellboxbounds.y1 = cellboxbounds.y0 + cellboxheight / float(targheight);

	// figure out the title
	std::ostringstream title_buf;
	util::stream_format(title_buf, "'%s' %d/%d", interface.device().tag(), m_gfxset.m_set, info.setcount() - 1);

	// if the mouse pointer is over a pixel in a tile, add some info about the tile and pixel
	bool found_pixel = false;
	float mouse_x, mouse_y;
	if (map_mouse(container, cellboxbounds, mouse_x, mouse_y))
	{
		int const code = set.m_offset + int((mouse_x - cellboxbounds.x0) / cellwidth) + int((mouse_y - cellboxbounds.y0) / cellheight) * xcells;
		int xpixel = int((mouse_x - cellboxbounds.x0) / (cellwidth / cellxpix)) % cellxpix;
		int ypixel = int((mouse_y - cellboxbounds.y0) / (cellheight / cellypix)) % cellypix;
		if ((code < gfx.elements()) && (xpixel < (cellxpix - 1)) && (ypixel < (cellypix - 1)))
		{
			found_pixel = true;
			if (set.m_rotate & ORIENTATION_FLIP_X)
				xpixel = (cellxpix - 2) - xpixel;
			if (set.m_rotate & ORIENTATION_FLIP_Y)
				ypixel = (cellypix - 2) - ypixel;
			if (set.m_rotate & ORIENTATION_SWAP_XY)
				std::swap(xpixel, ypixel);
			uint8_t const pixdata = gfx.get_data(code)[xpixel + ypixel * gfx.rowbytes()];
			util::stream_format(title_buf, " #%X:%X @ %d,%d = %X",
					code, set.m_color,
					xpixel, ypixel,
					gfx.colorbase() + (set.m_color * gfx.granularity()) + pixdata);
		}
	}
	if (!found_pixel)
		util::stream_format(title_buf, " %dx%d COLOR %X/%X", gfx.width(), gfx.height(), set.m_color, set.m_color_count);

	float x0, y0;

	// expand the outer box to fit the title
	std::string const title = std::move(title_buf).str();
	float const titlewidth = ui_font->string_width(chheight, aspect, title);
	x0 = 0.0f;
	if (boxbounds.x1 - boxbounds.x0 < titlewidth + chwidth)
		x0 = boxbounds.x0 - (0.5f - 0.5f * (titlewidth + chwidth));

	// go ahead and draw the outer box now
	mui.draw_outlined_box(container, boxbounds.x0 - x0, boxbounds.y0, boxbounds.x1 + x0, boxbounds.y1, mui.colors().gfxviewer_bg_color());

	// draw the title
	x0 = 0.5f - 0.5f * titlewidth;
	y0 = boxbounds.y0 + 0.5f * chheight;
	for (auto ch : title)
	{
		container.add_char(x0, y0, chheight, aspect, rgb_t::white(), *ui_font, ch);
		x0 += ui_font->char_width(chheight, aspect, ch);
	}

	// draw the top column headers
	int const colskip = int(chwidth / cellwidth);
	for (int x = 0; x < xcells; x += 1 + colskip)
	{
		x0 = boxbounds.x0 + 6.0f * chwidth + float(x) * cellwidth;
		y0 = boxbounds.y0 + 2.0f * chheight;
		container.add_char(x0 + 0.5f * (cellwidth - chwidth), y0, chheight, aspect, rgb_t::white(), *ui_font, "0123456789ABCDEF"[x & 0xf]);

		// if we're skipping, draw a point between the character and the box to indicate which one it's referring to
		if (colskip)
			container.add_point(x0 + 0.5f * cellwidth, 0.5f * (y0 + chheight + boxbounds.y0 + 3.5f * chheight), UI_LINE_WIDTH, rgb_t::white(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}

	// draw the side column headers
	int const rowskip = int(chheight / cellheight);
	for (int y = 0; y < ycells; y += 1 + rowskip)
	{
		// only display if there is data to show
		if (set.m_offset + (y * xcells) < gfx.elements())
		{
			// if we're skipping, draw a point between the character and the box to indicate which one it's referring to
			x0 = boxbounds.x0 + 5.5f * chwidth;
			y0 = boxbounds.y0 + 3.5f * chheight + float(y) * cellheight;
			if (rowskip)
				container.add_point(0.5f * (x0 + boxbounds.x0 + 6.0f * chwidth), y0 + 0.5f * cellheight, UI_LINE_WIDTH, rgb_t::white(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

			// draw the row header
			char buffer[10];
			sprintf(buffer, "%5X", set.m_offset + (y * xcells));
			for (int x = 4; x >= 0; x--)
			{
				x0 -= ui_font->char_width(chheight, aspect, buffer[x]);
				container.add_char(x0, y0 + 0.5f * (cellheight - chheight), chheight, aspect, rgb_t::white(), *ui_font, buffer[x]);
			}
		}
	}

	// update the bitmap
	update_gfxset_bitmap(xcells, ycells, gfx);

	// add the final quad
	container.add_quad(
			cellboxbounds.x0, cellboxbounds.y0, cellboxbounds.x1, cellboxbounds.y1,
			rgb_t::white(), m_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	// handle keyboard navigation before drawing
	if (m_gfxset.handle_keys(m_machine, xcells, ycells))
		m_bitmap_dirty = true;
	return handle_general_keys(uistate);
}


uint32_t gfx_viewer::handle_tilemap(mame_ui_manager &mui, render_container &container, bool uistate)
{
	// get some UI metrics
	render_font *const ui_font = mui.get_font();
	int const targwidth = m_machine.render().ui_target().width();
	int const targheight = m_machine.render().ui_target().height();
	float const aspect = m_machine.render().ui_aspect(&container);
	float const chheight = mui.get_line_height();
	float const chwidth = ui_font->char_width(chheight, aspect, '0');

	// get the size of the tilemap itself
	tilemap_t &tilemap = *m_machine.tilemap().find(m_tilemap.index());
	uint32_t mapwidth = tilemap.width();
	uint32_t mapheight = tilemap.height();
	if (m_tilemap.rotate() & ORIENTATION_SWAP_XY)
		std::swap(mapwidth, mapheight);

	// add a half character padding for the box
	render_bounds boxbounds{
			0.0f + (0.5f * chwidth),
			0.0f + (0.5f * chheight),
			1.0f - (0.5f * chwidth),
			1.0f - (0.5f * chheight) };

	// the tilemap box bounds starts a half character in from the box
	render_bounds mapboxbounds = boxbounds;
	mapboxbounds.x0 += 0.5f * chwidth;
	mapboxbounds.x1 -= 0.5f * chwidth;
	mapboxbounds.y0 += 0.5f * chheight;
	mapboxbounds.y1 -= 0.5f * chheight;

	// add space on the top for a title and a half line of padding
	mapboxbounds.y0 += 1.5f * chheight;

	// convert back to pixels
	int mapboxwidth = mapboxbounds.width() * float(targwidth);
	int mapboxheight = mapboxbounds.height() * float(targheight);

	float pixelscale;
	if (m_tilemap.auto_zoom())
	{
		// determine the maximum integral scaling factor
		pixelscale = std::min(std::floor(mapboxwidth / mapwidth), std::floor(mapboxheight / mapheight));
		pixelscale = std::max(pixelscale, 1.0f);
	}
	else
	{
		pixelscale = m_tilemap.zoom_scale();
	}

	// recompute the final box size
	mapboxwidth = std::min<int>(mapboxwidth, std::lround(mapwidth * pixelscale));
	mapboxheight = std::min<int>(mapboxheight, std::lround(mapheight * pixelscale));

	// recompute the bounds, centered within the existing bounds
	mapboxbounds.x0 += 0.5f * ((mapboxbounds.x1 - mapboxbounds.x0) - float(mapboxwidth) / targwidth);
	mapboxbounds.x1 = mapboxbounds.x0 + float(mapboxwidth) / targwidth;
	mapboxbounds.y0 += 0.5f * ((mapboxbounds.y1 - mapboxbounds.y0) - float(mapboxheight) / targheight);
	mapboxbounds.y1 = mapboxbounds.y0 + float(mapboxheight) / targheight;

	// now recompute the outer box against this new info
	boxbounds.x0 = mapboxbounds.x0 - 0.5f * chwidth;
	boxbounds.x1 = mapboxbounds.x1 + 0.5f * chwidth;
	boxbounds.y0 = mapboxbounds.y0 - 2.0f * chheight;
	boxbounds.y1 = mapboxbounds.y1 + 0.5f * chheight;

	// figure out the title
	std::ostringstream title_buf;
	util::stream_format(title_buf, "TILEMAP %d/%d", m_tilemap.index() + 1, m_machine.tilemap().count());

	// if the mouse pointer is over a tile, add some info about its coordinates and color
	float mouse_x, mouse_y;
	if (map_mouse(container, mapboxbounds, mouse_x, mouse_y))
	{
		int xpixel = (mouse_x - mapboxbounds.x0) * targwidth;
		int ypixel = (mouse_y - mapboxbounds.y0) * targheight;
		if (m_tilemap.rotate() & ORIENTATION_FLIP_X)
			xpixel = (mapboxwidth - 1) - xpixel;
		if (m_tilemap.rotate() & ORIENTATION_FLIP_Y)
			ypixel = (mapboxheight - 1) - ypixel;
		if (m_tilemap.rotate() & ORIENTATION_SWAP_XY)
			std::swap(xpixel, ypixel);
		uint32_t const col = ((std::lround(xpixel / pixelscale) + m_tilemap.xoffs()) / tilemap.tilewidth()) % tilemap.cols();
		uint32_t const row = ((std::lround(ypixel / pixelscale) + m_tilemap.yoffs()) / tilemap.tileheight()) % tilemap.rows();
		uint8_t gfxnum;
		uint32_t code, color;
		tilemap.get_info_debug(col, row, gfxnum, code, color);
		util::stream_format(title_buf, " @ %u,%u = GFX%u #%X:%X",
				col * tilemap.tilewidth(), row * tilemap.tileheight(),
				gfxnum, code, color);
	}
	else
	{
		util::stream_format(title_buf, " %dx%d OFFS %d,%d", tilemap.width(), tilemap.height(), m_tilemap.xoffs(), m_tilemap.yoffs());
	}

	if (m_tilemap.flags() != TILEMAP_DRAW_ALL_CATEGORIES)
		util::stream_format(title_buf, " CAT %u", m_tilemap.flags());

	// expand the outer box to fit the title
	std::string const title = std::move(title_buf).str();
	float const titlewidth = ui_font->string_width(chheight, aspect, title);
	if (boxbounds.x1 - boxbounds.x0 < titlewidth + chwidth)
	{
		boxbounds.x0 = 0.5f - 0.5f * (titlewidth + chwidth);
		boxbounds.x1 = boxbounds.x0 + titlewidth + chwidth;
	}

	// go ahead and draw the outer box now
	mui.draw_outlined_box(container, boxbounds.x0, boxbounds.y0, boxbounds.x1, boxbounds.y1, mui.colors().gfxviewer_bg_color());

	// draw the title
	float x0 = 0.5f - 0.5f * titlewidth;
	float y0 = boxbounds.y0 + 0.5f * chheight;
	for (auto ch : title)
	{
		container.add_char(x0, y0, chheight, aspect, rgb_t::white(), *ui_font, ch);
		x0 += ui_font->char_width(chheight, aspect, ch);
	}

	// update the bitmap
	update_tilemap_bitmap(std::lround(mapboxwidth / pixelscale), std::lround(mapboxheight / pixelscale));

	// add the final quad
	container.add_quad(
			mapboxbounds.x0, mapboxbounds.y0,
			mapboxbounds.x1, mapboxbounds.y1,
			rgb_t::white(), m_texture,
			PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(m_tilemap.rotate()));

	// handle keyboard input
	if (m_tilemap.handle_keys(m_machine, pixelscale))
		m_bitmap_dirty = true;
	return handle_general_keys(uistate);
}


void gfx_viewer::update_gfxset_bitmap(int xcells, int ycells, gfx_element &gfx)
{
	auto const &info = m_gfxset.m_devices[m_gfxset.m_device];
	auto const &set = info.set(m_gfxset.m_set);

	// compute the number of source pixels in a cell
	int const cellxpix = 1 + ((set.m_rotate & ORIENTATION_SWAP_XY) ? gfx.height() : gfx.width());
	int const cellypix = 1 + ((set.m_rotate & ORIENTATION_SWAP_XY) ? gfx.width() : gfx.height());

	// reallocate the bitmap if it is too small
	resize_bitmap(cellxpix * xcells, cellypix * ycells);

	// handle the redraw
	if (m_bitmap_dirty)
	{
		// pre-fill with transparency
		m_bitmap.fill(0);

		// loop over rows
		for (int y = 0, index = set.m_offset; y < ycells; y++)
		{
			// make a rectangle that covers this row
			rectangle cellbounds(0, m_bitmap.width() - 1, y * cellypix, (y + 1) * cellypix - 1);

			// only display if there is data to show
			if (index < gfx.elements())
			{
				// draw the individual cells
				for (int x = 0; x < xcells; x++, index++)
				{
					// update the bounds for this cell
					cellbounds.min_x = x * cellxpix;
					cellbounds.max_x = (x + 1) * cellxpix - 1;

					if (index < gfx.elements()) // only render if there is data
						gfxset_draw_item(gfx, index, cellbounds.min_x, cellbounds.min_y, set);
					else // otherwise, fill with transparency
						m_bitmap.fill(0, cellbounds);
				}
			}
		}

		// reset the texture to force an update
		m_texture->set_bitmap(m_bitmap, m_bitmap.cliprect(), TEXFORMAT_ARGB32);
		m_bitmap_dirty = false;
	}
}


void gfx_viewer::update_tilemap_bitmap(int width, int height)
{
	// swap the coordinates back if they were talking about a rotated surface
	if (m_tilemap.rotate() & ORIENTATION_SWAP_XY)
		std::swap(width, height);

	// reallocate the bitmap if it is too small
	resize_bitmap(width, height);

	// handle the redraw
	if (m_bitmap_dirty)
	{
		m_bitmap.fill(0);
		tilemap_t &tilemap = *m_machine.tilemap().find(m_tilemap.index());
		screen_device *const first_screen = screen_device_enumerator(m_machine.root_device()).first();
		if (first_screen)
			tilemap.draw_debug(*first_screen, m_bitmap, m_tilemap.xoffs(), m_tilemap.yoffs(), m_tilemap.flags());

		// reset the texture to force an update
		m_texture->set_bitmap(m_bitmap, m_bitmap.cliprect(), TEXFORMAT_RGB32);
		m_bitmap_dirty = false;
	}
}


void gfx_viewer::gfxset_draw_item(gfx_element &gfx, int index, int dstx, int dsty, gfxset::setinfo const &info)
{
	int const width = (info.m_rotate & ORIENTATION_SWAP_XY) ? gfx.height() : gfx.width();
	int const height = (info.m_rotate & ORIENTATION_SWAP_XY) ? gfx.width() : gfx.height();
	rgb_t const *const palette = info.m_palette->palette()->entry_list_raw() + gfx.colorbase() + info.m_color * gfx.granularity();
	uint8_t const *const src = gfx.get_data(index);

	// loop over rows in the cell
	for (int y = 0; y < height; y++)
	{
		uint32_t *dest = &m_bitmap.pix(dsty + y, dstx);

		// loop over columns in the cell
		for (int x = 0; x < width; x++)
		{
			// compute effective x,y values after rotation
			int effx = x, effy = y;
			if (!(info.m_rotate & ORIENTATION_SWAP_XY))
			{
				if (info.m_rotate & ORIENTATION_FLIP_X)
					effx = gfx.width() - 1 - effx;
				if (info.m_rotate & ORIENTATION_FLIP_Y)
					effy = gfx.height() - 1 - effy;
			}
			else
			{
				if (info.m_rotate & ORIENTATION_FLIP_X)
					effx = gfx.height() - 1 - effx;
				if (info.m_rotate & ORIENTATION_FLIP_Y)
					effy = gfx.width() - 1 - effy;
				std::swap(effx, effy);
			}

			// get a pointer to the start of this source row
			uint8_t const *const s = src + (effy * gfx.rowbytes());

			// extract the pixel
			*dest++ = 0xff000000 | palette[s[effx]];
		}
	}
}

} // anonymous namespace



/***************************************************************************
    MAIN ENTRY POINT
***************************************************************************/

//-------------------------------------------------
//  ui_gfx_ui_handler - primary UI handler
//
//  NOTE: this must not be called before machine
//  initialization is complete, as some drivers
//  create or modify gfx sets in VIDEO_START
//-------------------------------------------------

uint32_t ui_gfx_ui_handler(render_container &container, mame_ui_manager &mui, bool uistate)
{
	return mui.get_session_data<gfx_viewer, gfx_viewer>(mui.machine()).handle(mui, container, uistate);
}
