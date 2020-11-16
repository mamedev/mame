// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic,Luca Bruno
/***************************************************************************

    luaengine_input.cpp

    Controls execution of the core MAME system.

***************************************************************************/

#include "emu.h"
#include "luaengine.ipp"

#include "render.h"


//-------------------------------------------------
//  initialize_render - register render user types
//-------------------------------------------------

void lua_engine::initialize_render()
{

/* render_target library
 *
 * manager:machine():render().targets[target_index]
 * manager:machine():render():ui_target()
 *
 * target:view_bounds() - get x0, x1, y0, y1 bounds for target
 * target:width() - get target width
 * target:height() - get target height
 * target:pixel_aspect() - get target aspect
 * target:hidden() - is target hidden
 * target:is_ui_target() - is ui render target
 * target:index() - target index
 * target:view_name([opt] index) - current target layout view name
 *
 * target.max_update_rate -
 * target.view - current target layout view
 * target.orientation - current target orientation
 * target.screen_overlay - enable overlays
 * target.zoom - enable zoom
 */

	auto target_type = sol().registry().new_usertype<render_target>("target", "new", sol::no_constructor);
	target_type.set("view_bounds", [](render_target &rt) {
			const render_bounds b = rt.current_view().bounds();
			return std::tuple<float, float, float, float>(b.x0, b.x1, b.y0, b.y1);
		});
	target_type.set("width", &render_target::width);
	target_type.set("height", &render_target::height);
	target_type.set("pixel_aspect", &render_target::pixel_aspect);
	target_type.set("hidden", &render_target::hidden);
	target_type.set("is_ui_target", &render_target::is_ui_target);
	target_type.set("index", &render_target::index);
	target_type.set("view_name", &render_target::view_name);
	target_type.set("max_update_rate", sol::property(&render_target::max_update_rate, &render_target::set_max_update_rate));
	target_type.set("view", sol::property(&render_target::view, &render_target::set_view));
	target_type.set("orientation", sol::property(&render_target::orientation, &render_target::set_orientation));
	target_type.set("screen_overlay", sol::property(&render_target::screen_overlay_enabled, &render_target::set_screen_overlay_enabled));
	target_type.set("zoom", sol::property(&render_target::zoom_to_screen, &render_target::set_zoom_to_screen));


/* render_container library
 *
 * manager:machine():render():ui_container()
 *
 * container:orientation()
 * container:xscale()
 * container:yscale()
 * container:xoffset()
 * container:yoffset()
 * container:is_empty()
 */

	auto render_container_type = sol().registry().new_usertype<render_container>("render_container", "new", sol::no_constructor);
	render_container_type.set("orientation", &render_container::orientation);
	render_container_type.set("xscale", &render_container::xscale);
	render_container_type.set("yscale", &render_container::yscale);
	render_container_type.set("xoffset", &render_container::xoffset);
	render_container_type.set("yoffset", &render_container::yoffset);
	render_container_type.set("is_empty", &render_container::is_empty);


/* render_manager library
 *
 * manager:machine():render()
 *
 * render:max_update_rate() -
 * render:ui_target() - render_target for ui drawing
 * render:ui_container() - render_container for ui drawing
 *
 * render.targets[] - render_target table
 */

	auto render_type = sol().registry().new_usertype<render_manager>("render", "new", sol::no_constructor);
	render_type.set("max_update_rate", &render_manager::max_update_rate);
	render_type.set("ui_target", &render_manager::ui_target);
	render_type.set("ui_container", &render_manager::ui_container);
	render_type.set("targets", sol::property([this](render_manager &r) {
			sol::table target_table = sol().create_table();
			int tc = 0;
			for(render_target &curr_rt : r.targets())
				target_table[tc++] = &curr_rt;
			return target_table;
		}));

}
