// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/viewgfx.h

    Internal graphics viewer.

***************************************************************************/

#ifndef MAME_FRONTEND_MAME_UI_VIEWGFX_H
#define MAME_FRONTEND_MAME_UI_VIEWGFX_H

#pragma once

#include <vector>

#include "emu.h"
#include "ui/viewgfx.h"

#include "emupal.h"
#include "render.h"
#include "rendfont.h"
#include "rendutil.h"
#include "screen.h"
#include "tilemap.h"
#include "uiinput.h"

#include "util/unicode.h"

#include "osdepend.h"
#include "ui/ui.h"

class gfx_viewer
{
private:
	enum class view
	{
		PALETTE = 0,
		GFXSET,
		TILEMAP
	};
private:
	class palette
	{
	public:
		enum class subset
		{
			PENS,
			INDIRECT
		};
	private:
		device_palette_interface* m_interface = nullptr;
		unsigned const m_count;
		unsigned m_index = 0U;
		subset m_which = subset::PENS;
		unsigned m_columns = 16U;
		int m_offset = 0;
	private:
		void set_device(running_machine& machine);
		void next_group(running_machine& machine) noexcept;
		void prev_group(running_machine& machine) noexcept;
	public:
		palette(running_machine& machine);
		device_palette_interface* interface() const noexcept;
		bool indirect() const noexcept;
		unsigned columns() const noexcept;
		unsigned index(unsigned x, unsigned y) const noexcept;
		void handle_keys(running_machine& machine);
	};
	class tilemap
	{
	private:
		class info
		{
		public:
			int m_xoffs = 0;
			int m_yoffs = 0;
			unsigned m_zoom = 1U;
			bool m_zoom_frac = false;
			bool m_auto_zoom = true;
			uint8_t m_rotate = 0U;
			uint32_t m_flags = TILEMAP_DRAW_ALL_CATEGORIES;
		public:
			bool zoom_in(float pixelscale) noexcept;
			bool zoom_out(float pixelscale) noexcept;
			bool next_category() noexcept;
			bool prev_catagory() noexcept;
		};
	private:
		static constexpr int MAX_ZOOM_LEVEL = 8; // maximum tilemap zoom ratio screen:native
		static constexpr int MIN_ZOOM_LEVEL = 8; // minimum tilemap zoom ratio native:screen
		std::vector<info> m_info;
		unsigned m_index = 0U;
	private:
		static int scroll_step(running_machine& machine);
	public:
		tilemap(running_machine& machine);
		unsigned index() const noexcept;
		float zoom_scale() const noexcept;
		bool auto_zoom() const noexcept;
		uint8_t rotate() const noexcept;
		uint32_t flags() const noexcept;
		int xoffs() const noexcept;
		int yoffs() const noexcept;
		bool handle_keys(running_machine& machine, float pixelscale);
	};
public:
	class gfxset
	{
	public:
		class setinfo
		{
		public:
			device_palette_interface* m_palette = nullptr;
			int m_offset = 0;
			unsigned m_color = 0;
			unsigned m_color_count = 0U;
			uint8_t m_rotate = 0U;
			uint8_t m_columns = 16U;
			bool m_integer_scale = false;
		public:
			void next_color() noexcept;
			void prev_color() noexcept;
		};
		class devinfo
		{
		private:
			device_gfx_interface* m_interface;
			unsigned m_setcount;
			setinfo m_sets[MAX_GFX_ELEMENTS];
		public:
			devinfo(device_gfx_interface& interface, device_palette_interface* first_palette, u8 rotate);
			device_gfx_interface& interface() const noexcept;
			unsigned setcount() const noexcept;
			setinfo const& set(unsigned index) const noexcept;
			setinfo& set(unsigned index) noexcept;
		};
	private:
		bool next_group() noexcept;
		bool prev_group() noexcept;
	public:
		std::vector<devinfo> m_devices;
		unsigned m_device = 0U;
		unsigned m_set = 0U;
	public:
		gfxset(running_machine& machine);
		bool has_gfx() const noexcept;
		bool handle_keys(running_machine& machine, int xcells, int ycells);
	};
private:
	running_machine& m_machine;
	view m_mode = view::PALETTE;

	s32 m_current_pointer = -1;
	ui_event::pointer m_pointer_type = ui_event::pointer::UNKNOWN;
	u32 m_pointer_buttons = 0U;
	float m_pointer_x = -1.0F;
	float m_pointer_y = -1.0F;
	bool m_pointer_inside = false;

	bitmap_rgb32 m_bitmap;
	render_texture* m_texture = nullptr;
	bool m_bitmap_dirty = false;

	palette m_palette;
	gfxset m_gfxset;
	tilemap m_tilemap;
private:
	bool is_relevant() const noexcept;
	uint32_t handle_general_keys(bool uistate);
	uint32_t cancel(bool uistate);
	uint32_t handle_palette(mame_ui_manager& mui, render_container& container, bool uistate);
	uint32_t handle_gfxset(mame_ui_manager& mui, render_container& container, bool uistate);
	uint32_t handle_tilemap(mame_ui_manager& mui, render_container& container, bool uistate);
	void update_gfxset_bitmap(int xcells, int ycells, gfx_element& gfx);
	void update_tilemap_bitmap(int width, int height);
	void gfxset_draw_item(gfx_element& gfx, int index, int dstx, int dsty, gfxset::setinfo const& info);
	void draw_text(mame_ui_manager& mui, render_container& container, std::string_view str, float x, float y);
	void resize_bitmap(int32_t width, int32_t height);
	bool map_mouse(render_container& container, render_bounds const& clip, float& x, float& y) const;
public:
	gfx_viewer(running_machine& machine);
	gfx_viewer(gfx_viewer const& that);		// copy constructor needed to make std::any happy
	~gfx_viewer();
	uint32_t handle(mame_ui_manager& mui, render_container& container, bool uistate);
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

// master handler
uint32_t ui_gfx_ui_handler(render_container &container, mame_ui_manager &mui, bool uistate);


#endif // MAME_FRONTEND_MAME_UI_VIEWGFX_H
