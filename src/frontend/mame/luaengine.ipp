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


namespace sol {

class buffer
{
public:
	// sol does lua_settop(0), save userdata buffer in registry if necessary
	buffer(int size, lua_State *L)
	{
		ptr = luaL_buffinitsize(L, &buff, size);
		len = size;
		if(buff.b != buff.initb)
		{
			lua_pushvalue(L, -1);
			lua_setfield(L, LUA_REGISTRYINDEX, "sol::buffer_temp");
		}
	}
	~buffer()
	{
		lua_State *L = buff.L;
		if(lua_getfield(L, LUA_REGISTRYINDEX, "sol::buffer_temp") != LUA_TNIL)
		{
			lua_pushnil(L);
			lua_setfield(L, LUA_REGISTRYINDEX, "sol::buffer_temp");
		}
		else
			lua_pop(L, -1);

		luaL_pushresultsize(&buff, len);
	}

	void set_len(int size) { len = size; }
	int get_len() { return len; }
	char *get_ptr() { return ptr; }

private:
	luaL_Buffer buff;
	int len;
	char *ptr;
};


// don't convert core_optons to a table directly
template<> struct is_container<core_options> : std::false_type { };

sol::buffer *sol_lua_get(sol::types<buffer *>, lua_State *L, int index, sol::stack::record &tracking);
int sol_lua_push(sol::types<buffer *>, lua_State *L, buffer *value);

} // namespace sol


int sol_lua_push(sol::types<osd_file::error>, lua_State *L, osd_file::error &&value);
template <typename Handler>
bool sol_lua_check(sol::types<osd_file::error>, lua_State *L, int index, Handler &&handler, sol::stack::record &tracking);

int sol_lua_push(sol::types<map_handler_type>, lua_State *L, map_handler_type &&value);


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


template<typename T, size_t SIZE>
class lua_engine::enum_parser
{
public:
	constexpr enum_parser(std::initializer_list<std::pair<const char *, T>> values)
	{
		if (values.size() != SIZE)
			throw false && "size template argument incorrectly specified";
		std::copy(values.begin(), values.end(), m_map.begin());
	}

	T operator()(const char *text) const
	{
		auto iter = std::find_if(
			m_map.begin() + 1,
			m_map.end(),
			[text](const auto &x) { return !strcmp(text, x.first); });
		if (iter == m_map.end())
			iter = m_map.begin();
		return iter->second;
	}

	T operator()(const std::string &text) const
	{
		return (*this)(text.c_str());
	}

private:
	std::array<std::pair<const char *, T>, SIZE> m_map;
};

#endif // MAME_FRONTEND_MAME_LUAENGINE_IPP
