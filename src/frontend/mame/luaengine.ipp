// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    luaengine.ipp

    Controls execution of the core MAME system.

***************************************************************************/
#ifndef MAME_FRONTEND_MAME_LUAENGINE_IPP
#define MAME_FRONTEND_MAME_LUAENGINE_IPP

#include "luaengine.h"

#include "options.h"

#include <lua.hpp>

#include <cassert>
#include <system_error>



template <typename T>
struct lua_engine::simple_list_wrapper
{
	simple_list_wrapper(simple_list<T> const &l) : list(l) { }

	simple_list<T> const &list;
};


template <typename T>
struct lua_engine::tag_object_ptr_map
{
	tag_object_ptr_map(T const &m) : map(m) { }

	T const &map;
};


class lua_engine::buffer_helper
{
private:
	class proxy
	{
	private:
		buffer_helper &m_host;
		char *m_space;
		size_t const m_size;

	public:
		proxy(proxy const &) = delete;
		proxy &operator=(proxy const &) = delete;

		proxy(proxy &&that) : m_host(that.m_host), m_space(that.m_space), m_size(that.m_size)
		{
			that.m_space = nullptr;
		}

		proxy(buffer_helper &host, size_t size) : m_host(host), m_space(luaL_prepbuffsize(&host.m_buffer, size)), m_size(size)
		{
			m_host.m_prepared = true;
		}

		~proxy()
		{
			if (m_space)
			{
				assert(m_host.m_prepared);
				luaL_addsize(&m_host.m_buffer, 0U);
				m_host.m_prepared = false;
			}
		}

		char *get()
		{
			return m_space;
		}

		void add(size_t size)
		{
			assert(m_space);
			assert(size <= m_size);
			assert(m_host.m_prepared);
			m_space = nullptr;
			luaL_addsize(&m_host.m_buffer, size);
			m_host.m_prepared = false;
		}
	};

	luaL_Buffer m_buffer;
	bool m_valid;
	bool m_prepared;

public:
	buffer_helper(buffer_helper const &) = delete;
	buffer_helper &operator=(buffer_helper const &) = delete;

	buffer_helper(lua_State *L)
	{
		luaL_buffinit(L, &m_buffer);
		m_valid = true;
		m_prepared = false;
	}

	~buffer_helper()
	{
		assert(!m_prepared);
		if (m_valid)
			luaL_pushresult(&m_buffer);
	}

	void push()
	{
		assert(m_valid);
		assert(!m_prepared);
		luaL_pushresult(&m_buffer);
		m_valid = false;
	}

	proxy prepare(size_t size)
	{
		assert(m_valid);
		assert(!m_prepared);
		return proxy(*this, size);
	}
};


namespace sol {

// don't convert core_options to a table directly
template <> struct is_container<core_options> : std::false_type { };


// these things should be treated as containers
template <typename T> struct is_container<lua_engine::devenum<T> > : std::true_type { };
template <typename T> struct is_container<lua_engine::simple_list_wrapper<T> > : std::true_type { };
template <typename T> struct is_container<lua_engine::tag_object_ptr_map<T> > : std::true_type { };


template <typename T> struct usertype_container<lua_engine::devenum<T> >;


template <typename T>
struct usertype_container<lua_engine::simple_list_wrapper<T> > : lua_engine::immutable_collection_helper<lua_engine::simple_list_wrapper<T>, simple_list<T> const, typename simple_list<T>::auto_iterator>
{
private:
	static int next_pairs(lua_State *L)
	{
		typename usertype_container::indexed_iterator &i(stack::unqualified_get<user<typename usertype_container::indexed_iterator> >(L, 1));
		if (i.src.end() == i.it)
			return stack::push(L, lua_nil);
		int result;
		result = stack::push(L, i.ix + 1);
		result += stack::push_reference(L, *i.it);
		++i;
		return result;
	}

public:
	static int at(lua_State *L)
	{
		lua_engine::simple_list_wrapper<T> &self(usertype_container::get_self(L));
		std::ptrdiff_t const index(stack::unqualified_get<std::ptrdiff_t>(L, 2));
		if ((0 >= index) || (self.list.count() < index))
			return stack::push(L, lua_nil);
		else
			return stack::push_reference(L, *self.list.find(index - 1));
	}

	static int get(lua_State *L) { return at(L); }
	static int index_get(lua_State *L) { return at(L); }

	static int index_of(lua_State *L)
	{
		lua_engine::simple_list_wrapper<T> &self(usertype_container::get_self(L));
		T &target(stack::unqualified_get<T>(L, 2));
		int const found(self.list.indexof(target));
		if (0 > found)
			return stack::push(L, lua_nil);
		else
			return stack::push(L, found + 1);
	}

	static int size(lua_State *L)
	{
		lua_engine::simple_list_wrapper<T> &self(usertype_container::get_self(L));
		return stack::push(L, self.list.count());
	}

	static int empty(lua_State *L)
	{
		lua_engine::simple_list_wrapper<T> &self(usertype_container::get_self(L));
		return stack::push(L, self.list.empty());
	}

	static int next(lua_State *L) { return stack::push(L, next_pairs); }
	static int pairs(lua_State *L) { return ipairs(L); }

	static int ipairs(lua_State *L)
	{
		lua_engine::simple_list_wrapper<T> &self(usertype_container::get_self(L));
		stack::push(L, next_pairs);
		stack::push<user<typename usertype_container::indexed_iterator> >(L, self.list, self.list.begin());
		stack::push(L, lua_nil);
		return 3;
	}
};


template <typename T>
struct usertype_container<lua_engine::tag_object_ptr_map<T> > : lua_engine::immutable_collection_helper<lua_engine::tag_object_ptr_map<T>, T const, typename T::const_iterator>
{
private:
	template <bool Indexed>
	static int next_pairs(lua_State *L)
	{
		typename usertype_container::indexed_iterator &i(stack::unqualified_get<user<typename usertype_container::indexed_iterator> >(L, 1));
		if (i.src.end() == i.it)
			return stack::push(L, lua_nil);
		int result;
		if constexpr (Indexed)
			result = stack::push(L, i.ix + 1);
		else
			result = stack::push(L, i.it->first);
		result += stack::push_reference(L, *i.it->second);
		++i;
		return result;
	}

	template <bool Indexed>
	static int start_pairs(lua_State *L)
	{
		lua_engine::tag_object_ptr_map<T> &self(usertype_container::get_self(L));
		stack::push(L, next_pairs<Indexed>);
		stack::push<user<typename usertype_container::indexed_iterator> >(L, self.map, self.map.begin());
		stack::push(L, lua_nil);
		return 3;
	}

public:
	static int at(lua_State *L)
	{
		lua_engine::tag_object_ptr_map<T> &self(usertype_container::get_self(L));
		std::ptrdiff_t const index(stack::unqualified_get<std::ptrdiff_t>(L, 2));
		if ((0 >= index) || (self.map.size() < index))
			return stack::push(L, lua_nil);
		auto const found(std::next(self.map.begin(), index - 1));
		if (!found->second)
			return stack::push(L, lua_nil);
		else
			return stack::push_reference(L, *found->second);
	}

	static int get(lua_State *L)
	{
		lua_engine::tag_object_ptr_map<T> &self(usertype_container::get_self(L));
		char const *const tag(stack::unqualified_get<char const *>(L));
		auto const found(self.map.find(tag));
		if ((self.map.end() == found) || !found->second)
			return stack::push(L, lua_nil);
		else
			return stack::push_reference(L, *found->second);
	}

	static int index_get(lua_State *L)
	{
		return get(L);
	}

	static int index_of(lua_State *L)
	{
		lua_engine::tag_object_ptr_map<T> &self(usertype_container::get_self(L));
		auto &obj(stack::unqualified_get<decltype(*self.map.begin()->second)>(L, 2));
		auto it(self.map.begin());
		std::ptrdiff_t ix(0);
		while ((self.map.end() != it) && (it->second.get() != &obj))
		{
			++it;
			++ix;
		}
		if (self.map.end() == it)
			return stack::push(L, lua_nil);
		else
			return stack::push(L, ix + 1);
	}

	static int size(lua_State *L)
	{
		lua_engine::tag_object_ptr_map<T> &self(usertype_container::get_self(L));
		return stack::push(L, self.map.size());
	}

	static int empty(lua_State *L)
	{
		lua_engine::tag_object_ptr_map<T> &self(usertype_container::get_self(L));
		return stack::push(L, self.map.empty());
	}

	static int next(lua_State *L) { return stack::push(L, next_pairs<false>); }
	static int pairs(lua_State *L) { return start_pairs<false>(L); }
	static int ipairs(lua_State *L) { return start_pairs<true>(L); }
};

} // namespace sol


// automatically convert std::error_condition to string
int sol_lua_push(sol::types<std::error_condition>, lua_State &L, std::error_condition &&value);

// enums to automatically convert to strings
int sol_lua_push(sol::types<map_handler_type>, lua_State *L, map_handler_type &&value);
int sol_lua_push(sol::types<image_init_result>, lua_State *L, image_init_result &&value);
int sol_lua_push(sol::types<image_verify_result>, lua_State *L, image_verify_result &&value);
int sol_lua_push(sol::types<endianness_t>, lua_State *L, endianness_t &&value);


template <typename T>
struct lua_engine::immutable_container_helper
{
protected:
	static T &get_self(lua_State *L)
	{
		auto p(sol::stack::unqualified_check_get<T *>(L, 1));
		if (!p)
			luaL_error(L, "sol: 'self' is not of type '%s' (pass 'self' as first argument with ':' or call on proper type)", sol::detail::demangle<T>().c_str());
		if (!*p)
			luaL_error(L, "sol: 'self' argument is nil (pass 'self' as first argument with ':' or call on a '%s' type", sol::detail::demangle<T>().c_str());
		return **p;
	}

public:
	static int set(lua_State *L)
	{
		return luaL_error(L, "sol: cannot call 'set(key, value)' on type '%s': container is not modifiable", sol::detail::demangle<T>().c_str());
	}

	static int index_set(lua_State *L)
	{
		return luaL_error(L, "sol: cannot call 'container[key] = value' on type '%s': container is not modifiable", sol::detail::demangle<T>().c_str());
	}

	static int add(lua_State *L)
	{
		return luaL_error(L, "sol: cannot call 'add' on type '%s': container is not modifiable", sol::detail::demangle<T>().c_str());
	}

	static int insert(lua_State *L)
	{
		return luaL_error(L, "sol: cannot call 'insert' on type '%s': container is not modifiable", sol::detail::demangle<T>().c_str());
	}

	static int find(lua_State *L)
	{
		return luaL_error(L, "sol: cannot call 'find' on type '%s': no supported comparison operator for the value type", sol::detail::demangle<T>().c_str());
	}

	static int index_of(lua_State *L)
	{
		return luaL_error(L, "sol: cannot call 'index_of' on type '%s': no supported comparison operator for the value type", sol::detail::demangle<T>().c_str());
	}

	static int clear(lua_State *L)
	{
		return luaL_error(L, "sol: cannot call 'clear' on type '%s': container is not modifiable", sol::detail::demangle<T>().c_str());
	}

	static int erase(lua_State *L)
	{
		return luaL_error(L, "sol: cannot call 'erase' on type '%s': container is not modifiable", sol::detail::demangle<T>().c_str());
	}
};


template <typename T, typename C, typename I>
struct lua_engine::immutable_collection_helper : immutable_container_helper<T>
{
protected:
	using iterator = I;

	struct indexed_iterator
	{
		indexed_iterator(C &s, iterator i) : src(s), it(i), ix(0U) { }

		C &src;
		iterator it;
		std::size_t ix;

		indexed_iterator &operator++()
		{
			++it;
			++ix;
			return *this;
		}
	};
};


template <typename T, typename C, typename I>
struct lua_engine::immutable_sequence_helper : immutable_collection_helper<T, C, I>
{
protected:
	template <bool Indexed>
	static int next_pairs(lua_State *L)
	{
		auto &i(sol::stack::unqualified_get<sol::user<typename immutable_sequence_helper::indexed_iterator> >(L, 1));
		if (i.src.end() == i.it)
			return sol::stack::push(L, sol::lua_nil);
		int result;
		if constexpr (Indexed)
			result = sol::stack::push(L, i.ix + 1);
		else
			result = T::push_key(L, i.it, i.ix);
		result += sol::stack::push_reference(L, T::unwrap(i.it));
		++i;
		return result;
	}

	template <bool Indexed>
	static int start_pairs(lua_State *L)
	{
		T &self(immutable_sequence_helper::get_self(L));
		sol::stack::push(L, next_pairs<Indexed>);
		sol::stack::push<sol::user<typename immutable_sequence_helper::indexed_iterator> >(L, self.items(), self.items().begin());
		sol::stack::push(L, sol::lua_nil);
		return 3;
	}

public:
	static int at(lua_State *L)
	{
		T &self(immutable_sequence_helper::get_self(L));
		std::ptrdiff_t const index(sol::stack::unqualified_get<std::ptrdiff_t>(L, 2));
		if ((0 >= index) || (self.items().size() < index))
			return sol::stack::push(L, sol::lua_nil);
		else
			return sol::stack::push_reference(L, T::unwrap(std::next(self.items().begin(), index - 1)));
	}

	static int index_of(lua_State *L)
	{
		T &self(immutable_sequence_helper::get_self(L));
		auto it(self.items().begin());
		std::ptrdiff_t ix(0);
		auto const &item(sol::stack::unqualified_get<decltype(T::unwrap(it))>(L, 2));
		while ((self.items().end() != it) && (&item != &T::unwrap(it)))
		{
			++it;
			++ix;
		}
		if (self.items().end() == it)
			return sol::stack::push(L, sol::lua_nil);
		else
			return sol::stack::push(L, ix + 1);
	}

	static int size(lua_State *L)
	{
		T &self(immutable_sequence_helper::get_self(L));
		return sol::stack::push(L, self.items().size());
	}

	static int empty(lua_State *L)
	{
		T &self(immutable_sequence_helper::get_self(L));
		return sol::stack::push(L, self.items().empty());
	}

	static int next(lua_State *L) { return sol::stack::push(L, next_pairs<false>); }
	static int pairs(lua_State *L) { return start_pairs<false>(L); }
	static int ipairs(lua_State *L) { return start_pairs<true>(L); }
};



struct lua_engine::addr_space
{
	addr_space(address_space &s, device_memory_interface &d) : space(s), dev(d) { }

	template <typename T> T mem_read(offs_t address);
	template <typename T> void mem_write(offs_t address, T val);
	template <typename T> T log_mem_read(offs_t address);
	template <typename T> void log_mem_write(offs_t address, T val);
	template <typename T> T direct_mem_read(offs_t address);
	template <typename T> void direct_mem_write(offs_t address, T val);

	address_space &space;
	device_memory_interface &dev;
};


template <typename T, size_t Size>
class lua_engine::enum_parser
{
public:
	constexpr enum_parser(std::initializer_list<std::pair<std::string_view, T> > values)
	{
		if (values.size() != Size)
			throw false && "size template argument incorrectly specified";
		std::copy(values.begin(), values.end(), m_map.begin());
	}

	T operator()(std::string_view text) const
	{
		auto iter = std::find_if(
				m_map.begin() + 1,
				m_map.end(),
				[&text] (const auto &x) { return text == x.first; });
		if (iter == m_map.end())
			iter = m_map.begin();
		return iter->second;
	}

private:
	std::array<std::pair<std::string_view, T>, Size> m_map;
};


//-------------------------------------------------
//  make_simple_callback_setter - make a callback
//  setter for simple cases
//-------------------------------------------------

template <typename R, typename T, typename D>
auto lua_engine::make_simple_callback_setter(void (T::*setter)(delegate<R ()> &&), D &&dflt, const char *name, const char *desc)
{
	return
		[setter, dflt, name, desc] (T &self, sol::object cb)
		{
			if (cb == sol::lua_nil)
			{
				(self.*setter)(delegate<R ()>());
			}
			else if (cb.is<sol::protected_function>())
			{
				(self.*setter)(delegate<R ()>(
							[dflt, desc, cbfunc = cb.as<sol::protected_function>()] () -> R
							{
								if constexpr (std::is_same_v<R, void>)
								{
									(void)dflt;
									(void)desc;
									invoke(cbfunc);
								}
								else
								{
									auto result(invoke(cbfunc).get<std::optional<R> >());
									if (result)
									{
										return *result;
									}
									else
									{
										osd_printf_error("[LUA ERROR] invalid return from %s callback\n", desc);
										return dflt();
									}
								}
							}));
			}
			else
			{
				osd_printf_error("[LUA ERROR] must call %s with function or nil\n", name);
			}
		};
}

#endif // MAME_FRONTEND_MAME_LUAENGINE_IPP
