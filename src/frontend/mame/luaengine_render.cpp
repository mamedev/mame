// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    luaengine_render.cpp

    Controls execution of the core MAME system.

***************************************************************************/

#include "emu.h"
#include "luaengine.ipp"

#include "mame.h"
#include "ui/ui.h"

#include "render.h"
#include "rendlay.h"
#include "rendutil.h"

#include "interface/uievents.h"
#include "ioprocs.h"

#include <algorithm>
#include <atomic>
#include <iterator>


namespace {

class render_texture_helper
{
public:
	render_texture_helper(render_texture_helper const &) = delete;

	render_texture_helper(render_texture_helper &&that)
		: texture(that.texture)
		, bitmap(that.bitmap)
		, manager(that.manager)
		, storage_lock_count(that.storage_lock_count)
		, palette_lock_count(that.palette_lock_count)
	{
		that.texture = nullptr;
		that.bitmap.reset();
	}

	template <typename T>
	render_texture_helper(sol::this_state s, render_manager &m, std::shared_ptr<T> const &b, texture_format f)
		: texture(nullptr)
		, bitmap(b)
		, manager(m)
		, storage_lock_count(b->storage_lock_count)
		, palette_lock_count(b->palette_lock_count)
	{
		if (bitmap)
		{
			texture = manager.texture_alloc();
			if (texture)
			{
				++storage_lock_count;
				++palette_lock_count;
				texture->set_bitmap(*bitmap, bitmap->cliprect(), f);
			}
			else
			{
				luaL_error(s, "Error allocating texture");
			}
		}
	}

	~render_texture_helper()
	{
		free();
	}

	bool valid() const
	{
		return texture && bitmap;
	}

	void free()
	{
		if (texture)
		{
			manager.texture_free(texture);
			texture = nullptr;
		}
		if (bitmap)
		{
			assert(storage_lock_count);
			assert(palette_lock_count);
			--storage_lock_count;
			--palette_lock_count;
			bitmap.reset();
		}
	}

	render_texture *texture;
	std::shared_ptr<bitmap_t> bitmap;

private:
	render_manager &manager;
	std::atomic<unsigned> &storage_lock_count;
	std::atomic<unsigned> &palette_lock_count;
};


struct layout_file_elements
{
	layout_file_elements(layout_file &f) : file(f) { }
	layout_file::element_map &items() { return file.elements(); }

	layout_file &file;
};


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

	static layout_view_item &unwrap(layout_view::item_list::iterator const &it) { return *it; }
	static int push_key(lua_State *L, layout_view::item_list::iterator const &it, std::size_t ix) { return sol::stack::push(L, ix + 1); }

	layout_view &view;
};


struct render_target_view_names
{
	render_target_view_names(render_target &t) : target(t), count(-1) { }

	render_target &target;
	int count;
};


template <typename T>
auto get_bitmap_pixels(T const &bitmap, sol::this_state s, rectangle const &bounds)
{
	if (!bitmap.cliprect().contains(bounds))
		luaL_error(s, "Bounds exceed source clipping rectangle");
	luaL_Buffer buff;
	size_t const size(bounds.width() * bounds.height() * sizeof(typename T::pixel_t));
	auto ptr = reinterpret_cast<typename T::pixel_t *>(luaL_buffinitsize(s, &buff, size));
	for (auto y = bounds.top(); bounds.bottom() >= y; ++y, ptr += bounds.width())
		std::copy_n(&bitmap.pix(y, bounds.left()), bounds.width(), ptr);
	luaL_pushresultsize(&buff, size);
	return std::make_tuple(sol::make_reference(s, sol::stack_reference(s, -1)), bounds.width(), bounds.height());
}


template <typename T>
auto make_bitmap_specific_type(sol::table registry, char const *name)
{
	auto result = registry.new_usertype<T>(
			name,
			sol::no_constructor,
			sol::base_classes, sol::bases<bitmap_t>());
	result.set_function("pix", [] (T &bitmap, int32_t x, int32_t y) { return bitmap.pix(y, x); });
	result["pixels"] = sol::overload(
			[] (T const &bitmap, sol::this_state s)
			{
				return get_bitmap_pixels(bitmap, s, bitmap.cliprect());
			},
			[] (T const &bitmap, sol::this_state s, int32_t minx, int32_t miny, int32_t maxx, int32_t maxy)
			{
				return get_bitmap_pixels(bitmap, s, rectangle(minx, maxx, miny, maxy));
			});
	result["fill"] = sol::overload(
			static_cast<void (T::*)(typename T::pixel_t)>(&T::fill),
			[] (T &bitmap, typename T::pixel_t color, int32_t minx, int32_t miny, int32_t maxx, int32_t maxy)
			{
				bitmap.fill(color, rectangle(minx, maxx, miny, maxy));
			});
	result.set_function(
			"plot",
			[] (T &bitmap, int32_t x, int32_t y, typename T::pixel_t color)
			{
				if (bitmap.cliprect().contains(x, y))
					bitmap.pix(y, x) = color;
			});
	result.set_function("plot_box", &T::plot_box);
	result["bpp"] = sol::property(&T::bpp);
	return result;
}

} // anonymous namespace


namespace sol {

template <> struct is_container<layout_file_elements> : std::true_type { };
template <> struct is_container<layout_file_views> : std::true_type { };
template <> struct is_container<layout_view_items> : std::true_type { };
template <> struct is_container<render_target_view_names> : std::true_type { };


template <>
struct usertype_container<layout_file_elements> : lua_engine::immutable_collection_helper<layout_file_elements, layout_file::element_map>
{
private:
	template <bool Indexed>
	static int next_pairs(lua_State *L)
	{
		usertype_container::indexed_iterator &i(stack::unqualified_get<user<usertype_container::indexed_iterator> >(L, 1));
		if (i.src.end() == i.it)
			return stack::push(L, lua_nil);
		int result;
		if constexpr (Indexed)
			result = stack::push(L, i.ix + 1);
		else
			result = stack::push(L, i.it->first);
		result += stack::push_reference(L, i.it->second);
		++i;
		return result;
	}

	template <bool Indexed>
	static int start_pairs(lua_State *L)
	{
		layout_file_elements &self(usertype_container::get_self(L));
		stack::push(L, next_pairs<Indexed>);
		stack::push<user<usertype_container::indexed_iterator> >(L, self.items(), self.items().begin());
		stack::push(L, lua_nil);
		return 3;
	}

public:
	static int at(lua_State *L)
	{
		layout_file_elements &self(usertype_container::get_self(L));
		std::ptrdiff_t const index(stack::unqualified_get<std::ptrdiff_t>(L, 2));
		if ((0 >= index) || (self.items().size() < index))
			return stack::push(L, lua_nil);
		auto const found(std::next(self.items().begin(), index - 1));
		return stack::push_reference(L, found->second);
	}

	static int get(lua_State *L)
	{
		layout_file_elements &self(usertype_container::get_self(L));
		char const *const tag(stack::unqualified_get<char const *>(L));
		auto const found(self.items().find(tag));
		if (self.items().end() == found)
			return stack::push(L, lua_nil);
		else
			return stack::push_reference(L, found->second);
	}

	static int index_get(lua_State *L)
	{
		return get(L);
	}

	static int index_of(lua_State *L)
	{
		layout_file_elements &self(usertype_container::get_self(L));
		auto &obj(stack::unqualified_get<layout_element>(L, 2));
		auto it(self.items().begin());
		std::ptrdiff_t ix(0);
		while ((self.items().end() != it) && (&it->second != &obj))
		{
			++it;
			++ix;
		}
		if (self.items().end() == it)
			return stack::push(L, lua_nil);
		else
			return stack::push(L, ix + 1);
	}

	static int size(lua_State *L)
	{
		layout_file_elements &self(usertype_container::get_self(L));
		return stack::push(L, self.items().size());
	}

	static int empty(lua_State *L)
	{
		layout_file_elements &self(usertype_container::get_self(L));
		return stack::push(L, self.items().empty());
	}

	static int next(lua_State *L) { return stack::push(L, next_pairs<false>); }
	static int pairs(lua_State *L) { return start_pairs<false>(L); }
	static int ipairs(lua_State *L) { return start_pairs<true>(L); }
};


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
		layout_view_item *const item(self.view.get_item(id));
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
//  sol_lua_push - automatically convert
//  osd::ui_event_handler::pointer to a string
//-------------------------------------------------

int sol_lua_push(sol::types<osd::ui_event_handler::pointer>, lua_State *L, osd::ui_event_handler::pointer &&value)
{
	const char *typestr = "invalid";
	switch (value)
	{
	case osd::ui_event_handler::pointer::UNKNOWN:
		typestr = "unknown";
		break;
	case osd::ui_event_handler::pointer::MOUSE:
		typestr = "mouse";
		break;
	case osd::ui_event_handler::pointer::PEN:
		typestr = "pen";
		break;
	case osd::ui_event_handler::pointer::TOUCH:
		typestr = "touch";
		break;
	}
	return sol::stack::push(L, typestr);
}


template <typename T>
class lua_engine::bitmap_helper : public T
{
public:
	using ptr = std::shared_ptr<bitmap_helper>;

	bitmap_helper(bitmap_helper const &) = delete;
	bitmap_helper(bitmap_helper &&) = delete;
	bitmap_helper &operator=(bitmap_helper const &) = delete;
	bitmap_helper &operator=(bitmap_helper &&) = delete;

	bitmap_helper(sol::this_state s, int width, int height, int xslop, int yslop)
		: T(width, height, xslop, yslop)
		, storage_lock_count(0)
		, palette_lock_count(0)
		, storage()
	{
		if ((0 < width) && (0 < height) && !this->valid())
			luaL_error(s, "Error allocating bitmap storage");
	}

	bitmap_helper(ptr const &source, rectangle const &subrect)
		: T(*source, subrect)
		, storage_lock_count(0)
		, palette_lock_count(0)
		, storage(source->storage ? source->storage : source)
	{
		++storage->storage_lock_count;
		this->set_palette(source->palette());
	}

	~bitmap_helper()
	{
		assert(!storage_lock_count);
		assert(!palette_lock_count);
		release_storage();
	}

	void reset(sol::this_state s)
	{
		if (storage_lock_count)
			luaL_error(s, "Cannot reset bitmap while in use");
		palette_t *const p(this->palette());
		if (p)
			p->ref();
		T::reset();
		this->set_palette(p);
		if (p)
			p->deref();
		release_storage();
	}

	void allocate(sol::this_state s, int width, int height, int xslop, int yslop)
	{
		if (storage_lock_count)
			luaL_error(s, "Cannot reallocate bitmap while in use");
		palette_t *const p(this->palette());
		if (p)
			p->ref();
		T::allocate(width, height, xslop, yslop);
		this->set_palette(p);
		if (p)
			p->deref();
		release_storage();
		if ((0 < width) && (0 < height) && !this->valid())
			luaL_error(s, "Error allocating bitmap storage");
	}

	void resize(sol::this_state s, int width, int height, int xslop, int yslop)
	{
		if (storage_lock_count)
			luaL_error(s, "Cannot resize bitmap while in use");
		T::resize(width, height, xslop, yslop);
		release_storage();
		if ((0 < width) && (0 < height) && !this->valid())
			luaL_error(s, "Error allocating bitmap storage");
	}

	void wrap(sol::this_state s, ptr const &source, rectangle const &subrect)
	{
		if (source.get() == this)
			luaL_error(s, "Bitmap cannot wrap itself");
		if (storage_lock_count)
			luaL_error(s, "Cannot free bitmap storage while in use");
		if (!source->cliprect().contains(subrect))
			luaL_error(s, "Bounds exceed source clipping rectangle");
		palette_t *const p(this->palette());
		if (p)
			p->ref();
		T::wrap(*source, subrect);
		this->set_palette(p);
		if (p)
			p->deref();
		release_storage();
		storage = source->storage ? source->storage : source;
		++storage->storage_lock_count;
	}

	std::atomic<unsigned> storage_lock_count;
	std::atomic<unsigned> palette_lock_count;

	template <typename B>
	static auto make_type(sol::table &registry, char const *name)
	{
		auto result = registry.new_usertype<bitmap_helper>(
				name,
				sol::call_constructor, sol::factories(
					[] (sol::this_state s)
					{
						return std::make_shared<bitmap_helper>(s, 0, 0, 0, 0);
					},
					[] (sol::this_state s, int width, int height)
					{
						return std::make_shared<bitmap_helper>(s, width, height, 0, 0);
					},
					[] (sol::this_state s, int width, int height, int xslop, int yslop)
					{
						return std::make_shared<bitmap_helper>(s, width, height, xslop, yslop);
					},
					[] (ptr const &source)
					{
						return std::make_shared<bitmap_helper>(source, source->cliprect());
					},
					[] (sol::this_state s, ptr const &source, int32_t minx, int32_t miny, int32_t maxx, int32_t maxy)
					{
						rectangle const subrect(minx, maxx, miny, maxy);
						if (!source->cliprect().contains(subrect))
							luaL_error(s, "Bounds exceed source clipping rectangle");
						return std::make_shared<bitmap_helper>(source, subrect);
					}),
				sol::base_classes, sol::bases<T, B, bitmap_t>());
		add_bitmap_members(result);
		return result;
	}

	template <typename B>
	static auto make_indexed_type(sol::table &registry, char const *name)
	{
		auto result = registry.new_usertype<bitmap_helper>(
				name,
				sol::call_constructor, sol::factories(
					[] (sol::this_state s, palette_wrapper &p)
					{
						ptr result = std::make_shared<bitmap_helper>(s, 0, 0, 0, 0);
						result->set_palette(&p.palette());
						return result;
					},
					[] (sol::this_state s, palette_wrapper &p, int width, int height)
					{
						ptr result = std::make_shared<bitmap_helper>(s, width, height, 0, 0);
						result->set_palette(&p.palette());
						return result;
					},
					[] (sol::this_state s, palette_wrapper &p, int width, int height, int xslop, int yslop)
					{
						ptr result = std::make_shared<bitmap_helper>(s, width, height, xslop, yslop);
						result->set_palette(&p.palette());
						return result;
					},
					[] (ptr const &source)
					{
						return std::make_shared<bitmap_helper>(source, source->cliprect());
					},
					[] (sol::this_state s, ptr const &source, int32_t minx, int32_t miny, int32_t maxx, int32_t maxy)
					{
						rectangle const subrect(minx, maxx, miny, maxy);
						if (!source->cliprect().contains(subrect))
							luaL_error(s, "Bounds exceed source clipping rectangle");
						return std::make_shared<bitmap_helper>(source, subrect);
					}),
				sol::base_classes, sol::bases<B, bitmap_t>());
		result["palette"] = sol::property(
				[] (bitmap_helper const &b)
				{
					return b.palette()
						? std::optional<palette_wrapper>(std::in_place, *b.palette())
						: std::optional<palette_wrapper>();
				},
				[] (bitmap_helper &b, sol::this_state s, palette_wrapper &p)
				{
					if (b.palette_lock_count)
						luaL_error(s, "Cannot set palette while in use");
					b.set_palette(&p.palette());
				});
		add_bitmap_members(result);
		return result;
	}

private:
	void release_storage()
	{
		if (storage)
		{
			assert(storage->storage_lock_count);
			--storage->storage_lock_count;
			storage.reset();
		}
	}

	template <typename U>
	static void add_bitmap_members(U &type)
	{
		type.set_function("reset", &bitmap_helper::reset);
		type["allocate"] = sol::overload(
				&bitmap_helper::allocate,
				[] (bitmap_helper &bitmap, sol::this_state s, int width, int height) { bitmap.allocate(s, width, height, 0, 0); });
		type["resize"] = sol::overload(
				&bitmap_helper::resize,
				[] (bitmap_helper &bitmap, sol::this_state s, int width, int height) { bitmap.resize(s, width, height, 0, 0); });
		type["wrap"] = sol::overload(
				[] (bitmap_helper &bitmap, sol::this_state s, ptr const &source)
				{
					bitmap.wrap(s, source, source->cliprect());
				},
				[] (bitmap_helper &bitmap, sol::this_state s, ptr const &source, int32_t minx, int32_t miny, int32_t maxx, int32_t maxy)
				{
					bitmap.wrap(s, source, rectangle(minx, maxx, miny, maxy));
				});
		type["locked"] = sol::property([] (bitmap_helper const &b) { return bool(b.storage_lock_count); });
	}

	ptr storage;
};


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
	bounds_type.set_function("includes", &render_bounds::includes);
	bounds_type.set_function("set_xy", &render_bounds::set_xy);
	bounds_type.set_function("set_wh", &render_bounds::set_wh);
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
	color_type.set_function("set", &render_color::set);
	color_type["a"] = &render_color::a;
	color_type["r"] = &render_color::r;
	color_type["g"] = &render_color::g;
	color_type["b"] = &render_color::b;


	auto palette_type = emu.new_usertype<palette_wrapper>(
			"palette",
			sol::call_constructor, sol::initializers(
				[] (palette_wrapper &pal, uint32_t colors) { new (&pal) palette_wrapper(colors, 1); },
				[] (palette_wrapper &pal, uint64_t colors, uint32_t groups) { new (&pal) palette_wrapper(colors, groups); }));
	palette_type.set_function(
			"entry_color",
			[] (palette_wrapper const &pal, uint32_t index) { return uint32_t(pal.palette().entry_color(index)); });
	palette_type.set_function(
			"entry_contrast",
			[] (palette_wrapper const &pal, uint32_t index) { return pal.palette().entry_contrast(index); });
	palette_type.set_function(
			"entry_adjusted_color",
			[] (palette_wrapper const &pal, uint32_t index, std::optional<uint32_t> group)
			{
				if (group)
				{
					if ((pal.palette().num_colors() <= index) || (pal.palette().num_groups() <= *group))
						return uint32_t(rgb_t::black());
					index += *group * pal.palette().num_colors();
				}
				return uint32_t(pal.palette().entry_adjusted_color(index));
			});
	palette_type["entry_set_color"] = sol::overload(
			[] (palette_wrapper &pal, sol::this_state s, uint32_t index, uint32_t color)
			{
				if (pal.palette().num_colors() <= index)
					luaL_error(s, "Color index out of range");
				pal.palette().entry_set_color(index, rgb_t(color));
			},
			[] (palette_wrapper &pal, sol::this_state s, uint32_t index, uint8_t red, uint8_t green, uint8_t blue)
			{
				if (pal.palette().num_colors() <= index)
					luaL_error(s, "Color index out of range");
				pal.palette().entry_set_color(index, rgb_t(red, green, blue));
			});
	palette_type.set_function(
			"entry_set_red_level",
			[] (palette_wrapper &pal, sol::this_state s, uint32_t index, uint8_t level)
			{
				if (pal.palette().num_colors() <= index)
					luaL_error(s, "Color index out of range");
				pal.palette().entry_set_red_level(index, level);
			});
	palette_type.set_function(
			"entry_set_green_level",
			[] (palette_wrapper &pal, sol::this_state s, uint32_t index, uint8_t level)
			{
				if (pal.palette().num_colors() <= index)
					luaL_error(s, "Color index out of range");
				pal.palette().entry_set_green_level(index, level);
			});
	palette_type.set_function(
			"entry_set_blue_level",
			[] (palette_wrapper &pal, sol::this_state s, uint32_t index, uint8_t level)
			{
				if (pal.palette().num_colors() <= index)
					luaL_error(s, "Color index out of range");
				pal.palette().entry_set_blue_level(index, level);
			});
	palette_type.set_function(
			"entry_set_contrast",
			[] (palette_wrapper &pal, sol::this_state s, uint32_t index, float contrast)
			{
				if (pal.palette().num_colors() <= index)
					luaL_error(s, "Color index out of range");
				pal.palette().entry_set_contrast(index, contrast);
			});
	palette_type.set_function(
			"group_set_brightness",
			[] (palette_wrapper &pal, sol::this_state s, uint32_t group, float brightness)
			{
				if (pal.palette().num_colors() <= group)
					luaL_error(s, "Group index out of range");
				pal.palette().group_set_brightness(group, brightness);
			});
	palette_type.set_function(
			"group_set_contrast",
			[] (palette_wrapper &pal, sol::this_state s, uint32_t group, float contrast)
			{
				if (pal.palette().num_colors() <= group)
					luaL_error(s, "Group index out of range");
				pal.palette().group_set_contrast(group, contrast);
			});
	palette_type["colors"] = sol::property([] (palette_wrapper const &pal) { return pal.palette().num_colors(); });
	palette_type["groups"] = sol::property([] (palette_wrapper const &pal) { return pal.palette().num_groups(); });
	palette_type["max_index"] = sol::property([] (palette_wrapper const &pal) { return pal.palette().max_index(); });
	palette_type["black_entry"] = sol::property([] (palette_wrapper const &pal) { return pal.palette().black_entry(); });
	palette_type["white_entry"] = sol::property([] (palette_wrapper const &pal) { return pal.palette().white_entry(); });
	palette_type["brightness"] = sol::property([] (palette_wrapper &pal, float brightness) { pal.palette().set_brightness(brightness); });
	palette_type["contrast"] = sol::property([] (palette_wrapper &pal, float contrast) { pal.palette().set_contrast(contrast); });
	palette_type["gamma"] = sol::property([] (palette_wrapper &pal, float gamma) { pal.palette().set_gamma(gamma); });


	auto bitmap_type = sol().registry().new_usertype<bitmap_t>("bitmap", sol::no_constructor);
	bitmap_type.set_function(
			"cliprect",
			[] (bitmap_t const &bitmap)
			{
				rectangle const &result(bitmap.cliprect());
				return std::make_tuple(result.left(), result.top(), result.right(), result.bottom());
			});
	bitmap_type["fill"] = sol::overload(
			static_cast<void (bitmap_t::*)(uint64_t)>(&bitmap_t::fill),
			[] (bitmap_t &bitmap, uint64_t color, int32_t minx, int32_t miny, int32_t maxx, int32_t maxy)
			{
				bitmap.fill(color, rectangle(minx, maxx, miny, maxy));
			});
	bitmap_type.set_function("plot_box", &bitmap_t::plot_box);
	bitmap_type["width"] = sol::property(&bitmap_t::width);
	bitmap_type["height"] = sol::property(&bitmap_t::height);
	bitmap_type["rowpixels"] = sol::property(&bitmap_t::rowpixels);
	bitmap_type["rowbytes"] = sol::property(&bitmap_t::rowbytes);
	bitmap_type["bpp"] = sol::property(&bitmap_t::bpp);
	bitmap_type["valid"] = sol::property(&bitmap_t::valid);

	make_bitmap_specific_type<bitmap8_t>(sol().registry(), "bitmap8");
	make_bitmap_specific_type<bitmap16_t>(sol().registry(), "bitmap16");
	make_bitmap_specific_type<bitmap32_t>(sol().registry(), "bitmap32");
	make_bitmap_specific_type<bitmap64_t>(sol().registry(), "bitmap64");

	bitmap_helper<bitmap_ind8>::make_indexed_type<bitmap8_t>(emu, "bitmap_ind8");
	bitmap_helper<bitmap_ind16>::make_indexed_type<bitmap16_t>(emu, "bitmap_ind16");
	bitmap_helper<bitmap_ind32>::make_indexed_type<bitmap32_t>(emu, "bitmap_ind32");
	bitmap_helper<bitmap_ind64>::make_indexed_type<bitmap64_t>(emu, "bitmap_ind64");

	bitmap_helper<bitmap_yuy16>::make_type<bitmap16_t>(emu, "bitmap_yuy16");
	bitmap_helper<bitmap_rgb32>::make_type<bitmap32_t>(emu, "bitmap_rgb32");

	// ARGB32 bitmaps get extra functionality
	auto bitmap_argb32_type = emu.new_usertype<bitmap_argb32>(
			"bitmap_argb32_t",
			sol::no_constructor,
			sol::base_classes, sol::bases<bitmap32_t, bitmap_t>());
	bitmap_argb32_type.set_function(
			"resample",
			[] (bitmap_argb32 &bitmap, bitmap_argb32 &dest, std::optional<render_color> color)
			{
				render_resample_argb_bitmap_hq(dest, bitmap, color ? *color : render_color{ 1.0F, 1.0F, 1.0F, 1.0F });
			});


	auto bitmap_argb32_helper_type = bitmap_helper<bitmap_argb32>::make_type<bitmap32_t>(emu, "bitmap_argb32");
	bitmap_argb32_helper_type.set_function(
			"load",
			[] (sol::this_state s, std::string_view data)
			{
				auto stream(util::ram_read(data.data(), data.size()));
				if (!stream)
					luaL_error(s, "Error allocating stream wrapper");
				auto b = std::make_shared<bitmap_helper<bitmap_argb32> >(s, 0, 0, 0, 0);
				switch (render_detect_image(*stream))
				{
				case RENDUTIL_IMGFORMAT_PNG:
					render_load_png(*b, *stream);
					if (!b->valid())
						luaL_error(s, "Invalid or unsupported PNG data");
					break;
				case RENDUTIL_IMGFORMAT_JPEG:
					render_load_jpeg(*b, *stream);
					if (!b->valid())
						luaL_error(s, "Invalid or unsupported PNG data");
					break;
				case RENDUTIL_IMGFORMAT_MSDIB:
					render_load_msdib(*b, *stream);
					if (!b->valid())
						luaL_error(s, "Invalid or unsupported Microsoft DIB data");
					break;
				default:
					luaL_error(s, "Unsupported bitmap data format");
				}
				return b;
			});

	auto render_texture_type = emu.new_usertype<render_texture_helper>("render_texture", sol::no_constructor);
	render_texture_type.set_function("free", &render_texture_helper::free);
	render_texture_type["valid"] = sol::property(&render_texture_helper::valid);


	auto layout_element_type = sol().registry().new_usertype<layout_element>("layout_element", sol::no_constructor);
	layout_element_type.set_function("invalidate", &layout_element::invalidate);
	layout_element_type.set_function(
			"set_draw_callback",
			make_simple_callback_setter(
					&layout_element::set_draw_callback,
					nullptr,
					"set_draw_callback",
					"draw"));
	layout_element_type["default_state"] = sol::property(
			[] (layout_element const &e) -> std::optional<int>
			{
				if (0 <= e.default_state())
					return e.default_state();
				else
					return std::nullopt;
			});


	auto layout_view_type = sol().registry().new_usertype<layout_view>("layout_view", sol::no_constructor);
	layout_view_type.set_function("has_screen", &layout_view::has_screen);
	layout_view_type.set_function(
			"set_prepare_items_callback",
			make_simple_callback_setter(
					&layout_view::set_prepare_items_callback,
					nullptr,
					"set_prepare_items_callback",
					"prepare items"));
	layout_view_type.set_function(
			"set_preload_callback",
			make_simple_callback_setter(
					&layout_view::set_preload_callback,
					nullptr,
					"set_preload_callback",
					"preload"));
	layout_view_type.set_function(
			"set_recomputed_callback",
			make_simple_callback_setter(
					&layout_view::set_recomputed_callback,
					nullptr,
					"set_recomputed_callback",
					"recomputed"));
	layout_view_type.set_function(
			"set_pointer_updated_callback",
			make_simple_callback_setter(
					&layout_view::set_pointer_updated_callback,
					nullptr,
					"set_pointer_updated_callback",
					"pointer updated"));
	layout_view_type.set_function(
			"set_pointer_left_callback",
			make_simple_callback_setter(
					&layout_view::set_pointer_left_callback,
					nullptr,
					"set_pointer_left_callback",
					"pointer left"));
	layout_view_type.set_function(
			"set_pointer_aborted_callback",
			make_simple_callback_setter(
					&layout_view::set_pointer_aborted_callback,
					nullptr,
					"set_pointer_aborted_callback",
					"pointer aborted"));
	layout_view_type.set_function(
			"set_forget_pointers_callback",
			make_simple_callback_setter(
					&layout_view::set_forget_pointers_callback,
					nullptr,
					"set_forget_pointers_callback",
					"forget pointers"));
	layout_view_type["items"] = sol::property([] (layout_view &v) { return layout_view_items(v); });
	layout_view_type["name"] = sol::property(&layout_view::name);
	layout_view_type["unqualified_name"] = sol::property(&layout_view::unqualified_name);
	layout_view_type["visible_screen_count"] = sol::property(&layout_view::visible_screen_count);
	layout_view_type["effective_aspect"] = sol::property(&layout_view::effective_aspect);
	layout_view_type["bounds"] = sol::property(&layout_view::bounds);
	layout_view_type["has_art"] = sol::property(&layout_view::has_art);
	layout_view_type["show_pointers"] = sol::property(&layout_view::show_pointers, &layout_view::set_show_pointers);
	layout_view_type["hide_inactive_pointers"] = sol::property(&layout_view::hide_inactive_pointers, &layout_view::set_hide_inactive_pointers);


	auto layout_view_item_type = sol().registry().new_usertype<layout_view_item>("layout_item", sol::no_constructor);
	layout_view_item_type.set_function("set_state", &layout_view_item::set_state);
	layout_view_item_type.set_function(
			"set_element_state_callback",
			make_simple_callback_setter(
					&layout_view_item::set_element_state_callback,
					[] () { return 0; },
					"set_element_state_callback",
					"element state"));
	layout_view_item_type.set_function(
			"set_animation_state_callback",
			make_simple_callback_setter(
					&layout_view_item::set_animation_state_callback,
					[] () { return 0; },
					"set_animation_state_callback",
					"animation state"));
	layout_view_item_type.set_function(
			"set_bounds_callback",
			make_simple_callback_setter(
					&layout_view_item::set_bounds_callback,
					[] () { return render_bounds{ 0.0f, 0.0f, 1.0f, 1.0f }; },
					"set_bounds_callback",
					"bounds"));
	layout_view_item_type.set_function(
			"set_color_callback",
			make_simple_callback_setter(
					&layout_view_item::set_color_callback,
					[] () { return render_color{ 1.0f, 1.0f, 1.0f, 1.0f }; },
					"set_color_callback",
					"color"));
	layout_view_item_type.set_function(
			"set_scroll_size_x_callback",
			make_simple_callback_setter(
					&layout_view_item::set_scroll_size_x_callback,
					[] () { return 1.0f; },
					"set_scroll_size_x_callback",
					"horizontal scroll window size"));
	layout_view_item_type.set_function(
			"set_scroll_size_y_callback",
			make_simple_callback_setter(
					&layout_view_item::set_scroll_size_y_callback,
					[] () { return 1.0f; },
					"set_scroll_size_y_callback",
					"vertical scroll window size"));
	layout_view_item_type.set_function(
			"set_scroll_pos_x_callback",
			make_simple_callback_setter(
					&layout_view_item::set_scroll_pos_x_callback,
					[] () { return 1.0f; },
					"set_scroll_pos_x_callback",
					"horizontal scroll position"));
	layout_view_item_type.set_function(
			"set_scroll_pos_y_callback",
			make_simple_callback_setter(
					&layout_view_item::set_scroll_pos_y_callback,
					[] () { return 1.0f; },
					"set_scroll_pos_y_callback",
					"vertical scroll position"));
	layout_view_item_type["id"] = sol::property(
			[] (layout_view_item &i, sol::this_state s) -> sol::object
			{
				if (i.id().empty())
					return sol::lua_nil;
				else
					return sol::make_object(s, i.id());
			});
	layout_view_item_type["element"] = sol::property(&layout_view_item::element);
	layout_view_item_type["bounds_animated"] = sol::property(&layout_view_item::bounds_animated);
	layout_view_item_type["color_animated"] = sol::property(&layout_view_item::color_animated);
	layout_view_item_type["bounds"] = sol::property(&layout_view_item::bounds);
	layout_view_item_type["color"] = sol::property(&layout_view_item::color);
	layout_view_item_type["scroll_wrap_x"] = sol::property(&layout_view_item::scroll_wrap_x);
	layout_view_item_type["scroll_wrap_y"] = sol::property(&layout_view_item::scroll_wrap_y);
	layout_view_item_type["scroll_size_x"] = sol::property(
			&layout_view_item::scroll_size_x,
			&layout_view_item::set_scroll_size_x);
	layout_view_item_type["scroll_size_y"] = sol::property(
			&layout_view_item::scroll_size_y,
			&layout_view_item::set_scroll_size_y);
	layout_view_item_type["scroll_pos_x"] = sol::property(
			&layout_view_item::scroll_pos_x,
			&layout_view_item::set_scroll_pos_y);
	layout_view_item_type["scroll_pos_y"] = sol::property(
			&layout_view_item::scroll_pos_y,
			&layout_view_item::set_scroll_pos_y);
	layout_view_item_type["blend_mode"] = sol::property(&layout_view_item::blend_mode);
	layout_view_item_type["orientation"] = sol::property(&layout_view_item::orientation);
	layout_view_item_type["element_state"] = sol::property(&layout_view_item::element_state);
	layout_view_item_type["animation_state"] = sol::property(&layout_view_item::animation_state);


	auto layout_file_type = sol().registry().new_usertype<layout_file>("layout_file", sol::no_constructor);
	layout_file_type["set_resolve_tags_callback"] =
		make_simple_callback_setter(
				&layout_file::set_resolve_tags_callback,
				nullptr,
				"set_resolve_tags_callback",
				"resolve tags");
	layout_file_type["device"] = sol::property(&layout_file::device);
	layout_file_type["elements"] = sol::property([] (layout_file &f) { return layout_file_elements(f); });
	layout_file_type["views"] = sol::property([] (layout_file &f) { return layout_file_views(f); });


	auto target_type = sol().registry().new_usertype<render_target>("target", sol::no_constructor);
	target_type["ui_container"] = sol::property(&render_target::ui_container);
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
	render_container_type.set_function(
			"draw_box",
			[] (render_container &ctnr, float x1, float y1, float x2, float y2, std::optional<uint32_t> fgcolor, std::optional<uint32_t> bgcolor)
			{
				x1 = std::clamp(x1, 0.0f, 1.0f);
				y1 = std::clamp(y1, 0.0f, 1.0f);
				x2 = std::clamp(x2, 0.0f, 1.0f);
				y2 = std::clamp(y2, 0.0f, 1.0f);
				mame_ui_manager &ui(mame_machine_manager::instance()->ui());
				if (!fgcolor)
					fgcolor = ui.colors().text_color();
				if (!bgcolor)
					bgcolor = ui.colors().background_color();
				ui.draw_outlined_box(ctnr, x1, y1, x2, y2, *fgcolor, *bgcolor);
			});
	render_container_type.set_function(
			"draw_line",
			[] (render_container &ctnr, float x1, float y1, float x2, float y2, std::optional<uint32_t> color)
			{
				x1 = std::clamp(x1, 0.0f, 1.0f);
				y1 = std::clamp(y1, 0.0f, 1.0f);
				x2 = std::clamp(x2, 0.0f, 1.0f);
				y2 = std::clamp(y2, 0.0f, 1.0f);
				if (!color)
					color = mame_machine_manager::instance()->ui().colors().text_color();
				ctnr.add_line(x1, y1, x2, y2, UI_LINE_WIDTH, rgb_t(*color), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			});
	render_container_type.set_function(
			"draw_quad",
			[] (render_container &cntr, render_texture_helper const &tex, float x1, float y1, float x2, float y2, std::optional<uint32_t> color)
			{
				cntr.add_quad(x1, y1, x2, y2, color ? *color : uint32_t(0xffffffff), tex.texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			});
	render_container_type.set_function(
			"draw_text",
			[] (render_container &ctnr, sol::this_state s, sol::object xobj, float y, char const *msg, std::optional<uint32_t> fgcolor, std::optional<uint32_t> bgcolor)
			{
				auto justify = ui::text_layout::text_justify::LEFT;
				float x = 0;
				if (xobj.is<float>())
				{
					x = std::clamp(xobj.as<float>(), 0.0f, 1.0f);
				}
				else if (xobj.is<char const *>())
				{
					char const *const justifystr(xobj.as<char const *>());
					if (!strcmp(justifystr, "left"))
						justify = ui::text_layout::text_justify::LEFT;
					else if (!strcmp(justifystr, "right"))
						justify = ui::text_layout::text_justify::RIGHT;
					else if (!strcmp(justifystr, "center"))
						justify = ui::text_layout::text_justify::CENTER;
				}
				else
				{
					luaL_error(s, "Error in param 1 to draw_text");
					return;
				}
				y = std::clamp(y, 0.0f, 1.0f);
				mame_ui_manager &ui(mame_machine_manager::instance()->ui());
				if (!fgcolor)
					fgcolor = ui.colors().text_color();
				if (!bgcolor)
					bgcolor = 0;
				ui.draw_text_full(
						ctnr,
						msg,
						x, y, (1.0f - x),
						justify, ui::text_layout::word_wrapping::WORD,
						mame_ui_manager::OPAQUE_, *fgcolor, *bgcolor);
			});
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
	render_type["texture_alloc"] = sol::overload(
			[] (render_manager &manager, sol::this_state s, bitmap_helper<bitmap_ind16>::ptr const &bitmap)
			{
				return render_texture_helper(s, manager, bitmap, TEXFORMAT_PALETTE16);
			},
			[] (render_manager &manager, sol::this_state s, bitmap_helper<bitmap_yuy16>::ptr const &bitmap)
			{
				return render_texture_helper(s, manager, bitmap, TEXFORMAT_YUY16);
			},
			[] (render_manager &manager, sol::this_state s, bitmap_helper<bitmap_rgb32>::ptr const &bitmap)
			{
				return render_texture_helper(s, manager, bitmap, TEXFORMAT_RGB32);
			},
			[] (render_manager &manager, sol::this_state s, bitmap_helper<bitmap_argb32>::ptr const &bitmap)
			{
				return render_texture_helper(s, manager, bitmap, TEXFORMAT_ARGB32);
			});
	render_type["max_update_rate"] = sol::property(&render_manager::max_update_rate);
	render_type["ui_target"] = sol::property(&render_manager::ui_target);
	render_type["ui_container"] = sol::property(&render_manager::ui_container);
	render_type["targets"] = sol::property([] (render_manager &m) { return simple_list_wrapper<render_target>(m.targets()); });

}
