// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    luaengine_render.cpp

    Controls execution of the core MAME system.

***************************************************************************/

#include "emu.h"
#include "luaengine.ipp"

#include "render.h"
#include "rendlay.h"

#include <iterator>


namespace {

struct layout_file_views
{
	layout_file_views(layout_file &f) : file(f) { }
	layout_file::view_list &items() { return file.views(); }

	static layout_view &unwrap(layout_file::view_list::iterator const &it) { return *it; }
	static int push_key(lua_State *L, layout_file::view_list::iterator const &it, std::size_t ix) { return sol::stack::push_reference(L, it->unqualified_name()); }

	layout_file &file;
};


struct layout_view_items
{
	layout_view_items(layout_view &v) : view(v) { }
	layout_view::item_list &items() { return view.items(); }

	static layout_view::item &unwrap(layout_view::item_list::iterator const &it) { return *it; }
	static int push_key(lua_State *L, layout_view::item_list::iterator const &it, std::size_t ix) { return sol::stack::push(L, ix + 1); }

	layout_view &view;
};


struct render_target_view_names
{
	render_target_view_names(render_target &t) : target(t), count(-1) { }

	render_target &target;
	int count;
};

} // anonymous namespace


namespace sol {

template <> struct is_container<layout_file_views> : std::true_type { };
template <> struct is_container<layout_view_items> : std::true_type { };
template <> struct is_container<render_target_view_names> : std::true_type { };


template <>
struct usertype_container<layout_file_views> : lua_engine::immutable_sequence_helper<layout_file_views, layout_file::view_list>
{
public:
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
};


template <>
struct usertype_container<layout_view_items> : lua_engine::immutable_sequence_helper<layout_view_items, layout_view::item_list>
{
public:
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

	static int pairs(lua_State *L)
	{
		return luaL_error(L, "sol: cannot call 'pairs' on type '%s': not iterable by ID", sol::detail::demangle<layout_view_items>().c_str());
	}
};


template <>
struct usertype_container<render_target_view_names> : lua_engine::immutable_container_helper<render_target_view_names>
{
private:
	struct iterator
	{
		iterator(render_target &t, unsigned i) : target(t), index(i) { }

		render_target &target;
		unsigned index;
	};

	static int next_pairs(lua_State *L)
	{
		iterator &i(stack::unqualified_get<user<iterator> >(L, 1));
		char const *name(i.target.view_name(i.index));
		if (!name)
			return stack::push(L, lua_nil);
		int result = stack::push(L, i.index + 1);
		result += stack::push(L, name);
		++i.index;
		return result;
	}

public:
	static int at(lua_State *L)
	{
		render_target_view_names &self(get_self(L));
		unsigned const index(stack::unqualified_get<unsigned>(L, 2));
		return stack::push(L, self.target.view_name(index - 1));
	}

	static int get(lua_State *L)
	{
		return at(L);
	}

	static int index_get(lua_State *L)
	{
		return at(L);
	}

	static int find(lua_State *L)
	{
		render_target_view_names &self(get_self(L));
		char const *const key(stack::unqualified_get<char const *>(L, 2));
		for (unsigned i = 0; ; ++i)
		{
			char const *const name(self.target.view_name(i));
			if (!name)
				return stack::push(L, lua_nil);
			else if (!std::strcmp(key, name))
				return stack::push(L, i + 1);
		}
	}

	static int index_of(lua_State *L)
	{
		return find(L);
	}

	static int size(lua_State *L)
	{
		render_target_view_names &self(get_self(L));
		if (0 > self.count)
			for (self.count = 0; self.target.view_name(self.count); ++self.count) { }
		return stack::push(L, self.count);
	}

	static int empty(lua_State *L)
	{
		render_target_view_names &self(get_self(L));
		return stack::push(L, !self.target.view_name(0));
	}

	static int next(lua_State *L)
	{
		return stack::push(L, next_pairs);
	}

	static int pairs(lua_State *L)
	{
		render_target_view_names &self(get_self(L));
		stack::push(L, next_pairs);
		stack::push<user<iterator> >(L, self.target, 0);
		stack::push(L, lua_nil);
		return 3;
	}

	static int ipairs(lua_State *L)
	{
		return pairs(L);
	}
};

} // namespace sol


//-------------------------------------------------
//  initialize_render - register render user types
//-------------------------------------------------

void lua_engine::initialize_render(sol::table &emu)
{

	auto bounds_type = emu.new_usertype<render_bounds>(
			"render_bounds",
			sol::call_constructor, sol::initializers(
				[] (render_bounds &b) { new (&b) render_bounds{ 0.0F, 0.0F, 1.0F, 1.0F }; },
				[] (render_bounds &b, float x0, float y0, float x1, float y1) { new (&b) render_bounds{ x0, y0, x1, y1 }; }));
	bounds_type["includes"] = &render_bounds::includes;
	bounds_type["set_xy"] = &render_bounds::set_xy;
	bounds_type["set_wh"] = &render_bounds::set_wh;
	bounds_type["x0"] = &render_bounds::x0;
	bounds_type["y0"] = &render_bounds::y0;
	bounds_type["x1"] = &render_bounds::x1;
	bounds_type["y1"] = &render_bounds::y1;
	bounds_type["width"] = sol::property(&render_bounds::width, [] (render_bounds &b, float w) { b.x1 = b.x0 + w; });
	bounds_type["height"] = sol::property(&render_bounds::height, [] (render_bounds &b, float h) { b.y1 = b.y0 + h; });
	bounds_type["aspect"] = sol::property(&render_bounds::aspect);


	auto color_type = emu.new_usertype<render_color>(
			"render_color",
			sol::call_constructor, sol::initializers(
				[] (render_color &c) { new (&c) render_color{ 1.0F, 1.0F, 1.0F, 1.0F }; },
				[] (render_color &c, float a, float r, float g, float b) { new (&c) render_color{ a, r, g, b }; }));
	color_type["set"] = &render_color::set;
	color_type["a"] = &render_color::a;
	color_type["r"] = &render_color::r;
	color_type["g"] = &render_color::g;
	color_type["b"] = &render_color::b;


	auto layout_view_type = sol().registry().new_usertype<layout_view>("layout_view", sol::no_constructor);
	layout_view_type["has_screen"] = &layout_view::has_screen;
	layout_view_type["set_prepare_items_callback"] =
		make_simple_callback_setter<void>(
				&layout_view::set_prepare_items_callback,
				nullptr,
				"set_prepare_items_callback",
				nullptr);
	layout_view_type["set_preload_callback"] =
		make_simple_callback_setter<void>(
				&layout_view::set_preload_callback,
				nullptr,
				"set_preload_callback",
				nullptr);
	layout_view_type["set_recomputed_callback"] =
		make_simple_callback_setter<void>(
				&layout_view::set_recomputed_callback,
				nullptr,
				"set_recomputed_callback",
				nullptr);
	layout_view_type["items"] = sol::property([] (layout_view &v) { return layout_view_items(v); });
	layout_view_type["name"] = sol::property(&layout_view::name);
	layout_view_type["unqualified_name"] = sol::property(&layout_view::unqualified_name);
	layout_view_type["visible_screen_count"] = sol::property(&layout_view::visible_screen_count);
	layout_view_type["effective_aspect"] = sol::property(&layout_view::effective_aspect);
	layout_view_type["bounds"] = sol::property(&layout_view::bounds);
	layout_view_type["has_art"] = sol::property(&layout_view::has_art);


	auto layout_view_item_type = sol().registry().new_usertype<layout_view::item>("layout_item", sol::no_constructor);
	layout_view_item_type["set_state"] = &layout_view::item::set_state;
	layout_view_item_type["set_element_state_callback"] =
		make_simple_callback_setter<int>(
				&layout_view::item::set_element_state_callback,
				[] () { return 0; },
				"set_element_state_callback",
				"element state");
	layout_view_item_type["set_animation_state_callback"] =
		make_simple_callback_setter<int>(
				&layout_view::item::set_animation_state_callback,
				[] () { return 0; },
				"set_animation_state_callback",
				"animation state");
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
								auto result(invoke(cbfunc).get<sol::optional<render_bounds> >());
								if (result)
								{
									b = *result;
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
								auto result(invoke(cbfunc).get<sol::optional<render_color> >());
								if (result)
								{
									c = *result;
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
			[] (layout_view::item &i, sol::this_state s) -> sol::object
			{
				if (i.id().empty())
					return sol::lua_nil;
				else
					return sol::make_object(s, i.id());
			});
	layout_view_item_type["bounds_animated"] = sol::property(&layout_view::item::bounds_animated);
	layout_view_item_type["color_animated"] = sol::property(&layout_view::item::color_animated);
	layout_view_item_type["bounds"] = sol::property(&layout_view::item::bounds);
	layout_view_item_type["color"] = sol::property(&layout_view::item::color);
	layout_view_item_type["blend_mode"] = sol::property(&layout_view::item::blend_mode);
	layout_view_item_type["orientation"] = sol::property(&layout_view::item::orientation);
	layout_view_item_type["element_state"] = sol::property(&layout_view::item::element_state);
	layout_view_item_type["animation_state"] = sol::property(&layout_view::item::animation_state);


	auto layout_file_type = sol().registry().new_usertype<layout_file>("layout_file", sol::no_constructor);
	layout_file_type["set_resolve_tags_callback"] =
		make_simple_callback_setter<void>(
				&layout_file::set_resolve_tags_callback,
				nullptr,
				"set_resolve_tags_callback",
				nullptr);
	layout_file_type["device"] = sol::property(&layout_file::device);
	layout_file_type["views"] = sol::property([] (layout_file &f) { return layout_file_views(f); });


	auto target_type = sol().registry().new_usertype<render_target>("target", sol::no_constructor);
	target_type["index"] = sol::property([] (render_target const &t) { return t.index() + 1; });
	target_type["width"] = sol::property(&render_target::width);
	target_type["height"] = sol::property(&render_target::height);
	target_type["pixel_aspect"] = sol::property(&render_target::pixel_aspect);
	target_type["hidden"] = sol::property(&render_target::hidden);
	target_type["is_ui_target"] = sol::property(&render_target::is_ui_target);
	target_type["max_update_rate"] = sol::property(&render_target::max_update_rate, &render_target::set_max_update_rate);
	target_type["orientation"] = sol::property(&render_target::orientation, &render_target::set_orientation);
	target_type["view_names"] = sol::property([] (render_target &t) { return render_target_view_names(t); });
	target_type["current_view"] = sol::property(&render_target::current_view);
	target_type["view_index"] = sol::property(
			[] (render_target const &t) { return t.view() + 1; },
			[] (render_target &t, unsigned v) { t.set_view(v - 1); });
	target_type["visibility_mask"] = sol::property(&render_target::visibility_mask);
	target_type["screen_overlay"] = sol::property(&render_target::screen_overlay_enabled, &render_target::set_screen_overlay_enabled);
	target_type["zoom_to_screen"] = sol::property(&render_target::zoom_to_screen, &render_target::set_zoom_to_screen);


	auto render_container_type = sol().registry().new_usertype<render_container>("render_container", sol::no_constructor);
	render_container_type["user_settings"] = sol::property(&render_container::get_user_settings, &render_container::set_user_settings);
	render_container_type["orientation"] = sol::property(
			&render_container::orientation,
			[] (render_container &c, int v)
			{
				render_container::user_settings s(c.get_user_settings());
				s.m_orientation = v;
				c.set_user_settings(s);
			});
	render_container_type["xscale"] = sol::property(
			&render_container::xscale,
			[] (render_container &c, float v)
			{
				render_container::user_settings s(c.get_user_settings());
				s.m_xscale = v;
				c.set_user_settings(s);
			});
	render_container_type["yscale"] = sol::property(
			&render_container::yscale,
			[] (render_container &c, float v)
			{
				render_container::user_settings s(c.get_user_settings());
				s.m_yscale = v;
				c.set_user_settings(s);
			});
	render_container_type["xoffset"] = sol::property(
			&render_container::xoffset,
			[] (render_container &c, float v)
			{
				render_container::user_settings s(c.get_user_settings());
				s.m_xoffset = v;
				c.set_user_settings(s);
			});
	render_container_type["yoffset"] = sol::property(
			&render_container::yoffset,
			[] (render_container &c, float v)
			{
				render_container::user_settings s(c.get_user_settings());
				s.m_yoffset = v;
				c.set_user_settings(s);
			});
	render_container_type["is_empty"] = sol::property(&render_container::is_empty);


	auto user_settings_type = sol().registry().new_usertype<render_container::user_settings>("render_container_settings", sol::no_constructor);
	user_settings_type["orientation"] = &render_container::user_settings::m_orientation;
	user_settings_type["brightness"] = &render_container::user_settings::m_brightness;
	user_settings_type["contrast"] = &render_container::user_settings::m_contrast;
	user_settings_type["gamma"] = &render_container::user_settings::m_gamma;
	user_settings_type["xscale"] = &render_container::user_settings::m_xscale;
	user_settings_type["yscale"] = &render_container::user_settings::m_yscale;
	user_settings_type["xoffset"] = &render_container::user_settings::m_xoffset;
	user_settings_type["yoffset"] = &render_container::user_settings::m_yoffset;


	auto render_type = sol().registry().new_usertype<render_manager>("render", sol::no_constructor);
	render_type["max_update_rate"] = sol::property(&render_manager::max_update_rate);
	render_type["ui_target"] = sol::property(&render_manager::ui_target);
	render_type["ui_container"] = sol::property(&render_manager::ui_container);
	render_type["targets"] = sol::property([] (render_manager &m) { return simple_list_wrapper<render_target>(m.targets()); });

}
