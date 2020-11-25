// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    luaengine_render.cpp

    Controls execution of the core MAME system.

***************************************************************************/

#include "emu.h"
#include "luaengine.ipp"

#include "render.h"

#include <iterator>


namespace {

struct layout_file_views
{
	layout_file_views(layout_file &f) : file(f) { }

	layout_file &file;
};


struct layout_view_items
{
	layout_view_items(layout_view &v) : view(v) { }

	layout_view &view;
};


struct render_manager_targets
{
	render_manager_targets(render_manager &m) : targets(m.targets()) { }

	simple_list<render_target> const &targets;
};

} // anonymous namespace

namespace sol {

template <> struct is_container<layout_file_views> : std::true_type { };
template <> struct is_container<layout_view_items> : std::true_type { };
template <> struct is_container<render_manager_targets> : std::true_type { };


template <>
struct usertype_container<layout_file_views> : lua_engine::immutable_container_helper<layout_file_views, layout_file::view_list>
{
private:
	using view_list = layout_file::view_list;

	template <bool Indexed>
	static int next_pairs(lua_State *L)
	{
		indexed_iterator &i(stack::unqualified_get<user<indexed_iterator> >(L, 1));
		if (i.src.end() == i.it)
			return stack::push(L, lua_nil);
		int result;
		if constexpr (Indexed)
			result = stack::push(L, i.ix + 1);
		else
			result = stack::push_reference(L, i.it->unqualified_name());
		result += stack::push_reference(L, *i.it);
		++i;
		return result;
	}

	template <bool Indexed>
	static int start_pairs(lua_State *L)
	{
		layout_file_views &self(get_self(L));
		stack::push(L, next_pairs<Indexed>);
		stack::push<user<indexed_iterator> >(L, self.file.views(), self.file.views().begin());
		stack::push(L, lua_nil);
		return 3;
	}

public:
	static int at(lua_State *L)
	{
		layout_file_views &self(get_self(L));
		std::ptrdiff_t const index(stack::unqualified_get<std::ptrdiff_t>(L, 2));
		if ((0 >= index) || (self.file.views().size() < index))
			return stack::push(L, lua_nil);
		else
			return stack::push_reference(L, *std::next(self.file.views().begin(), index - 1));
	}

	static int get(lua_State *L)
	{
		layout_file_views &self(get_self(L));
		char const *const name(stack::unqualified_get<char const *>(L));
		auto const found(std::find_if(
					self.file.views().begin(),
					self.file.views().end(),
					[&name] (layout_view &v) { return v.unqualified_name() == name; }));
		if (self.file.views().end() != found)
			return stack::push_reference(L, *found);
		else
			return stack::push(L, lua_nil);
	}

	static int index_get(lua_State *L)
	{
		return get(L);
	}

	static int index_of(lua_State *L)
	{
		layout_file_views &self(get_self(L));
		layout_view &view(stack::unqualified_get<layout_view>(L, 2));
		auto it(self.file.views().begin());
		std::ptrdiff_t ix(0);
		while ((self.file.views().end() != it) && (&view != &*it))
		{
			++it;
			++ix;
		}
		if (self.file.views().end() == it)
			return stack::push(L, lua_nil);
		else
			return stack::push(L, ix + 1);
	}

	static int size(lua_State *L)
	{
		layout_file_views &self(get_self(L));
		return stack::push(L, self.file.views().size());
	}

	static int empty(lua_State *L)
	{
		layout_file_views &self(get_self(L));
		return stack::push(L, self.file.views().empty());
	}

	static int next(lua_State *L) { return stack::push(L, next_pairs<false>); }
	static int pairs(lua_State *L) { return start_pairs<false>(L); }
	static int ipairs(lua_State *L) { return start_pairs<true>(L); }
};


template <>
struct usertype_container<layout_view_items> : lua_engine::immutable_container_helper<layout_view_items, layout_view::item_list>
{
private:
	using item_list = layout_view::item_list;

	static int next_pairs(lua_State *L)
	{
		indexed_iterator &i(stack::unqualified_get<user<indexed_iterator> >(L, 1));
		if (i.src.end() == i.it)
			return stack::push(L, lua_nil);
		int result;
		result = stack::push(L, i.ix + 1);
		result += stack::push_reference(L, *i.it);
		++i.it;
		++i.ix;
		return result;
	}

public:
	static int at(lua_State *L)
	{
		layout_view_items &self(get_self(L));
		std::ptrdiff_t const index(stack::unqualified_get<std::ptrdiff_t>(L, 2));
		if ((0 >= index) || (self.view.items().size() < index))
			return stack::push(L, lua_nil);
		else
			return stack::push_reference(L, *std::next(self.view.items().begin(), index - 1));
	}

	static int get(lua_State *L)
	{
		layout_view_items &self(get_self(L));
		char const *const id(stack::unqualified_get<char const *>(L));
		layout_view::item *const item(self.view.get_item(id));
		if (item)
			return stack::push_reference(L, *item);
		else
			return stack::push(L, lua_nil);
	}

	static int index_get(lua_State *L)
	{
		return get(L);
	}

	static int index_of(lua_State *L)
	{
		layout_view_items &self(get_self(L));
		layout_view::item &item(stack::unqualified_get<layout_view::item>(L, 2));
		auto it(self.view.items().begin());
		std::ptrdiff_t ix(0);
		while ((self.view.items().end() != it) && (&item != &*it))
		{
			++it;
			++ix;
		}
		if (self.view.items().end() == it)
			return stack::push(L, lua_nil);
		else
			return stack::push(L, ix + 1);
	}

	static int size(lua_State *L)
	{
		layout_view_items &self(get_self(L));
		return stack::push(L, self.view.items().size());
	}

	static int empty(lua_State *L)
	{
		layout_view_items &self(get_self(L));
		return stack::push(L, self.view.items().empty());
	}

	static int next(lua_State *L) { return stack::push(L, next_pairs); }

	static int pairs(lua_State *L)
	{
		return luaL_error(L, "sol: cannot call 'pairs' on type '%s': not iterable by ID", sol::detail::demangle<layout_view_items>().c_str());
	}

	static int ipairs(lua_State *L)
	{
		layout_view_items &self(get_self(L));
		stack::push(L, next_pairs);
		stack::push<user<indexed_iterator> >(L, self.view.items(), self.view.items().begin());
		stack::push(L, lua_nil);
		return 3;
	}
};


template <>
struct usertype_container<render_manager_targets> : lua_engine::immutable_container_helper<render_manager_targets, simple_list<render_target> const, simple_list<render_target>::auto_iterator>
{
private:
	using target_list = simple_list<render_target>;

	static int next_pairs(lua_State *L)
	{
		indexed_iterator &i(stack::unqualified_get<user<indexed_iterator> >(L, 1));
		if (i.src.end() == i.it)
			return stack::push(L, lua_nil);
		int result;
		result = stack::push(L, i.ix + 1);
		result += stack::push_reference(L, *i.it);
		++i.it;
		++i.ix;
		return result;
	}

public:
	static int at(lua_State *L)
	{
		render_manager_targets &self(get_self(L));
		std::ptrdiff_t const index(stack::unqualified_get<std::ptrdiff_t>(L, 2));
		if ((0 >= index) || (self.targets.count() < index))
			return stack::push(L, lua_nil);
		else
			return stack::push_reference(L, *self.targets.find(index - 1));
	}

	static int get(lua_State *L) { return at(L); }
	static int index_get(lua_State *L) { return at(L); }

	static int index_of(lua_State *L)
	{
		render_manager_targets &self(get_self(L));
		render_target &target(stack::unqualified_get<render_target>(L, 2));
		int const found(self.targets.indexof(target));
		if (0 > found)
			return stack::push(L, lua_nil);
		else
			return stack::push(L, found + 1);
	}

	static int size(lua_State *L)
	{
		render_manager_targets &self(get_self(L));
		return stack::push(L, self.targets.count());
	}

	static int empty(lua_State *L)
	{
		render_manager_targets &self(get_self(L));
		return stack::push(L, self.targets.empty());
	}

	static int next(lua_State *L) { return stack::push(L, next_pairs); }
	static int pairs(lua_State *L) { return ipairs(L); }

	static int ipairs(lua_State *L)
	{
		render_manager_targets &self(get_self(L));
		stack::push(L, next_pairs);
		stack::push<user<indexed_iterator> >(L, self.targets, self.targets.begin());
		stack::push(L, lua_nil);
		return 3;
	}
};

} // namespace sol


//-------------------------------------------------
//  initialize_render - register render user types
//-------------------------------------------------

void lua_engine::initialize_render()
{

/* render_bounds library
 *
 * bounds:includes(x, y) - returns true if point is within bounds
 * bounds:set_xy(left, top, right, bottom) - set bounds
 * bounds:set_wh(left, top, width, height) - set bounds
 *
 * bounds.x0 - leftmost X coordinate
 * bounds.y0 - topmost Y coordinate
 * bounds.x1 - rightmost X coordinate
 * bounds.y1 - bottommost Y coordinate
 * bounds.width - get/set width
 * bounds.height - get/set height
 * bounds.aspect - read-only aspect ratio width:height
 */
	auto bounds_type = sol().registry().new_usertype<render_bounds>("bounds", sol::call_constructor, sol::initializers(
				[] (render_bounds &b) { new (&b) render_bounds{ 0.0F, 0.0F, 1.0F, 1.0F }; },
				[] (render_bounds &b, float x0, float y0, float x1, float y1) { new (&b) render_bounds{ x0, y0, x1, y1 }; }));
	bounds_type["includes"] = &render_bounds::includes;
	bounds_type["set_xy"] = &render_bounds::includes;
	bounds_type["set_wh"] = &render_bounds::includes;
	bounds_type["x0"] = &render_bounds::x0;
	bounds_type["y0"] = &render_bounds::y0;
	bounds_type["x1"] = &render_bounds::x1;
	bounds_type["y1"] = &render_bounds::y1;
	bounds_type["width"] = sol::property(&render_bounds::width, [] (render_bounds &b, float w) { b.x1 = b.x0 + w; });
	bounds_type["height"] = sol::property(&render_bounds::height, [] (render_bounds &b, float h) { b.y1 = b.y0 + h; });
	bounds_type["aspect"] = sol::property(&render_bounds::aspect);


/* render_color library
 *
 * set(a, r, g, b) - set color
 *
 * color.a - alpha channel
 * color.r - red channel
 * color.g - green channel
 * color.b - blue channel
 */
	auto color_type = sol().registry().new_usertype<render_color>("color", sol::call_constructor, sol::initializers(
				[] (render_color &c) { new (&c) render_color{ 1.0F, 1.0F, 1.0F, 1.0F }; },
				[] (render_color &c, float a, float r, float g, float b) { new (&c) render_color{ a, r, g, b }; }));
	color_type["set"] = &render_color::set;
	color_type["a"] = &render_color::a;
	color_type["r"] = &render_color::r;
	color_type["g"] = &render_color::g;
	color_type["b"] = &render_color::b;


/* layout_view library
 *
 * manager:machine():render().targets[target_index]:current_view()
 *
 * view:has_screen(screen) - returns whether a given screen is present in the view (including hidden screens)
 *
 * view.items - get the items in the view (including hidden items)
 * view.name - display name for the view
 * view.unqualified_name - name of the view as specified in the layout file
 * view.visible_screen_count - number of screens items currently enabled
 * view.effective_aspect - effective aspect ratio in current configuration
 * view.bounds - effective bounds in current configuration
 * view.has_art - true if the view has non-screen items
 */

	auto layout_view_type = sol().registry().new_usertype<layout_view>("layout_view", sol::no_constructor);
	layout_view_type["has_screen"] = &layout_view::has_screen;
	layout_view_type["items"] = sol::property([] (layout_view &v) { return layout_view_items(v); });
	layout_view_type["name"] = sol::property(&layout_view::name);
	layout_view_type["unqualified_name"] = sol::property(&layout_view::unqualified_name);
	layout_view_type["visible_screen_count"] = sol::property(&layout_view::visible_screen_count);
	layout_view_type["effective_aspect"] = sol::property(&layout_view::effective_aspect);
	layout_view_type["bounds"] = sol::property(&layout_view::bounds);
	layout_view_type["has_art"] = sol::property(&layout_view::has_art);


/* layout_view::item library
 *
 * item:set_state(state) - set state value used in absence of bindings
 * item.set_element_state_callback(cb) - set callback to obtain element state
 * item.set_animation_state_callback(cb) - set callback to obtain animation state
 * item.set_bounds_callback(cb) - set callback to obtain item bounds
 * item.set_color_callback(cb) - set callback to obtain item color
 *
 * item.id - get optional item identifier
 * item.bounds_animated - true if bounds depend on state
 * item.color_animated - true if color depends on state
 * item.bounds - get bounds for current state
 * item.color - get color for current state
 * item.blend_mode - get blend mode or -1
 * item.orientation - get item orientation
 * item.element_state - get effective element state
 * item.animation_state - get effective animation state
 */

	auto layout_view_item_type = sol().registry().new_usertype<layout_view::item>("layout_item", sol::no_constructor);
	layout_view_item_type["set_state"] = &layout_view::item::set_state;
	layout_view_item_type["set_element_state_callback"] =
		[this] (layout_view::item &i, sol::object cb)
		{
			if (cb == sol::lua_nil)
			{
				i.set_element_state_callback(layout_view::item::state_delegate());
			}
			else if (cb.is<sol::protected_function>())
			{
				i.set_element_state_callback(layout_view::item::state_delegate(
							[this, cbfunc = cb.as<sol::protected_function>()] () -> int
							{
								sol::optional<int> result(invoke(cbfunc));
								if (result)
								{
									return result.value();
								}
								else
								{
									osd_printf_error("[LUA ERROR] invalid return from element state callback\n");
									return 0;
								}
							}));
			}
			else
			{
				osd_printf_error("[LUA ERROR] must call set_element_state_callback with function or nil\n");
			}
		};
	layout_view_item_type["set_animation_state_callback"] =
		[this] (layout_view::item &i, sol::object cb)
		{
			if (cb == sol::lua_nil)
			{
				i.set_animation_state_callback(layout_view::item::state_delegate());
			}
			else if (cb.is<sol::protected_function>())
			{
				i.set_animation_state_callback(layout_view::item::state_delegate(
							[this, cbfunc = cb.as<sol::protected_function>()] () -> int
							{
								sol::optional<int> result(invoke(cbfunc));
								if (result)
								{
									return result.value();
								}
								else
								{
									osd_printf_error("[LUA ERROR] invalid return from animation state callback\n");
									return 0;
								}
							}));
			}
			else
			{
				osd_printf_error("[LUA ERROR] must call set_animation_state_callback with function or nil\n");
			}
		};
	layout_view_item_type["set_bounds_callback"] =
		[this] (layout_view::item &i, sol::object cb)
		{
			if (cb == sol::lua_nil)
			{
				i.set_bounds_callback(layout_view::item::bounds_delegate());
			}
			else if (cb.is<sol::protected_function>())
			{
				i.set_bounds_callback(layout_view::item::bounds_delegate(
							[this, cbfunc = cb.as<sol::protected_function>()] (render_bounds &b)
							{
								sol::optional<render_bounds> result(invoke(cbfunc));
								if (result)
								{
									b = result.value();
								}
								else
								{
									osd_printf_error("[LUA ERROR] invalid return from bounds callback\n");
									b = render_bounds{ 0.0, 0.0, 1.0, 1.0 };
								}
							}));
			}
			else
			{
				osd_printf_error("[LUA ERROR] must call set_bounds_callback with function or nil\n");
			}
		};
	layout_view_item_type["set_color_callback"] =
		[this] (layout_view::item &i, sol::object cb)
		{
			if (cb == sol::lua_nil)
			{
				i.set_color_callback(layout_view::item::color_delegate());
			}
			else if (cb.is<sol::protected_function>())
			{
				i.set_color_callback(layout_view::item::color_delegate(
							[this, cbfunc = cb.as<sol::protected_function>()] (render_color &c)
							{
								sol::optional<render_color> result(invoke(cbfunc));
								if (result)
								{
									c = result.value();
								}
								else
								{
									osd_printf_error("[LUA ERROR] invalid return from color callback\n");
									c = render_color{ 1.0, 1.0, 1.0, 1.0 };
								}
							}));
			}
			else
			{
				osd_printf_error("[LUA ERROR] must call set_bounds_callback with function or nil\n");
			}
		};
	layout_view_item_type["id"] = sol::property(
			[this] (layout_view::item &i) -> sol::object
			{
				if (i.id().empty())
					return sol::make_object(sol(), sol::lua_nil);
				else
					return sol::make_object(sol(), i.id());
			});
	layout_view_item_type["bounds_animated"] = sol::property(&layout_view::item::bounds_animated);
	layout_view_item_type["color_animated"] = sol::property(&layout_view::item::color_animated);
	layout_view_item_type["bounds"] = sol::property(&layout_view::item::bounds);
	layout_view_item_type["color"] = sol::property(&layout_view::item::color);
	layout_view_item_type["blend_mode"] = sol::property(&layout_view::item::blend_mode);
	layout_view_item_type["orientation"] = sol::property(&layout_view::item::orientation);
	layout_view_item_type["element_state"] = sol::property(&layout_view::item::element_state);
	layout_view_item_type["animation_state"] = sol::property(&layout_view::item::animation_state);


/* layout_file library
 *
 * file.set_resolve_tags_callback - set additional tasks after resolving tags
 *
 * file.device - get device that caused the file to be loaded
 * file.views[] - get view table (k=name, v=layout_view)
 */

	auto layout_file_type = sol().registry().new_usertype<layout_file>("layout_file", sol::no_constructor);
	layout_file_type["set_resolve_tags_callback"] =
		[this] (layout_file &f, sol::object cb)
		{
			if (cb == sol::lua_nil)
			{
				f.set_resolve_tags_callback(layout_file::resolve_tags_delegate());
			}
			else if (cb.is<sol::protected_function>())
			{
				f.set_resolve_tags_callback(layout_file::resolve_tags_delegate(
							[this, cbfunc = cb.as<sol::protected_function>()] () { invoke(cbfunc); }));
			}
			else
			{
				osd_printf_error("[LUA ERROR] must call set_resolve_tags_callback with function or nil\n");
			}
		};
	layout_file_type["device"] = sol::property(&layout_file::device);
	layout_file_type["views"] = sol::property([] (layout_file &f) { return layout_file_views(f); });


/* render_target library
 *
 * manager:machine():render().targets[target_index]
 * manager:machine():render():ui_target()
 *
 * target:current_view() - get current view for target
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

	auto target_type = sol().registry().new_usertype<render_target>("target", sol::no_constructor);
	target_type["current_view"] = &render_target::current_view;
	target_type["width"] = &render_target::width;
	target_type["height"] = &render_target::height;
	target_type["pixel_aspect"] = &render_target::pixel_aspect;
	target_type["hidden"] = &render_target::hidden;
	target_type["is_ui_target"] = &render_target::is_ui_target;
	target_type["index"] = &render_target::index;
	target_type["view_name"] = &render_target::view_name;
	target_type["max_update_rate"] = sol::property(&render_target::max_update_rate, &render_target::set_max_update_rate);
	target_type["view"] = sol::property(&render_target::view, &render_target::set_view);
	target_type["orientation"] = sol::property(&render_target::orientation, &render_target::set_orientation);
	target_type["screen_overlay"] = sol::property(&render_target::screen_overlay_enabled, &render_target::set_screen_overlay_enabled);
	target_type["zoom"] = sol::property(&render_target::zoom_to_screen, &render_target::set_zoom_to_screen);


/* render_container library
 *
 * manager:machine():render():ui_container()
 *
 * container.orientation
 * container.xscale
 * container.yscale
 * container.xoffset
 * container.yoffset
 * container.is_empty
 */

	auto render_container_type = sol().registry().new_usertype<render_container>("render_container", sol::no_constructor);
	render_container_type["orientation"] = sol::property(&render_container::orientation);
	render_container_type["xscale"] = sol::property(&render_container::xscale);
	render_container_type["yscale"] = sol::property(&render_container::yscale);
	render_container_type["xoffset"] = sol::property(&render_container::xoffset);
	render_container_type["yoffset"] = sol::property(&render_container::yoffset);
	render_container_type["is_empty"] = sol::property(&render_container::is_empty);


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

	auto render_type = sol().registry().new_usertype<render_manager>("render", sol::no_constructor);
	render_type["max_update_rate"] = &render_manager::max_update_rate;
	render_type["ui_target"] = &render_manager::ui_target;
	render_type["ui_container"] = &render_manager::ui_container;
	render_type["targets"] = sol::property([] (render_manager &m) { return render_manager_targets(m); });

}
