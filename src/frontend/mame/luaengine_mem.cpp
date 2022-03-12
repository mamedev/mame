// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic,Luca Bruno
/***************************************************************************

    luaengine_input.cpp

    Controls execution of the core MAME system.

***************************************************************************/

#include "emu.h"
#include "luaengine.ipp"


namespace {

//-------------------------------------------------
//  region_read - templated region readers for <sign>,<size>
//  -> manager:machine():memory().regions[":maincpu"]:read_i8(0xC000)
//-------------------------------------------------

template <typename T>
T region_read(memory_region &region, offs_t address)
{
	T mem_content = 0;
	const offs_t lowmask = region.bytewidth() - 1;
	for (int i = 0; i < sizeof(T); i++)
	{
		int addr = region.endianness() == ENDIANNESS_LITTLE ? address + sizeof(T) - 1 - i : address + i;
		if (addr < region.bytes())
		{
			if constexpr (sizeof(T) > 1)
				mem_content <<= 8;
			if (region.endianness() == ENDIANNESS_BIG)
				mem_content |= region.as_u8((BYTE8_XOR_BE(addr) & lowmask) | (addr & ~lowmask));
			else
				mem_content |= region.as_u8((BYTE8_XOR_LE(addr) & lowmask) | (addr & ~lowmask));
		}
	}

	return mem_content;
}

//-------------------------------------------------
//  region_write - templated region writer for <sign>,<size>
//  -> manager:machine():memory().regions[":maincpu"]:write_u16(0xC000, 0xF00D)
//-------------------------------------------------

template <typename T>
void region_write(memory_region &region, offs_t address, T val)
{
	const offs_t lowmask = region.bytewidth() - 1;
	for (int i = 0; i < sizeof(T); i++)
	{
		int addr = region.endianness() == ENDIANNESS_BIG ? address + sizeof(T) - 1 - i : address + i;
		if (addr < region.bytes())
		{
			if (region.endianness() == ENDIANNESS_BIG)
				region.base()[(BYTE8_XOR_BE(addr) & lowmask) | (addr & ~lowmask)] = val & 0xff;
			else
				region.base()[(BYTE8_XOR_LE(addr) & lowmask) | (addr & ~lowmask)] = val & 0xff;
			if constexpr (sizeof(T) > 1)
				val >>= 8;
		}
	}
}

//-------------------------------------------------
//  share_read - templated share readers for <sign>,<size>
//  -> manager:machine():memory().shares[":maincpu"]:read_i8(0xC000)
//-------------------------------------------------

template <typename T>
T share_read(memory_share &share, offs_t address)
{
	T mem_content = 0;
	const offs_t lowmask = share.bytewidth() - 1;
	u8 *ptr = (u8 *)share.ptr();
	for (int i = 0; i < sizeof(T); i++)
	{
		int addr = share.endianness() == ENDIANNESS_LITTLE ? address + sizeof(T) - 1 - i : address + i;
		if (addr < share.bytes())
		{
			if constexpr (sizeof(T) > 1)
				mem_content <<= 8;
			if (share.endianness() == ENDIANNESS_BIG)
				mem_content |= ptr[(BYTE8_XOR_BE(addr) & lowmask) | (addr & ~lowmask)];
			else
				mem_content |= ptr[(BYTE8_XOR_LE(addr) & lowmask) | (addr & ~lowmask)];
		}
	}

	return mem_content;
}

//-------------------------------------------------
//  share_write - templated share writer for <sign>,<size>
//  -> manager:machine():memory().shares[":maincpu"]:write_u16(0xC000, 0xF00D)
//-------------------------------------------------

template <typename T>
void share_write(memory_share &share, offs_t address, T val)
{
	const offs_t lowmask = share.bytewidth() - 1;
	u8 *ptr = (u8 *)share.ptr();
	for (int i = 0; i < sizeof(T); i++)
	{
		int addr = share.endianness() == ENDIANNESS_BIG ? address + sizeof(T) - 1 - i : address + i;
		if (addr < share.bytes())
		{
			if (share.endianness() == ENDIANNESS_BIG)
				ptr[(BYTE8_XOR_BE(addr) & lowmask) | (addr & ~lowmask)] = val & 0xff;
			else
				ptr[(BYTE8_XOR_LE(addr) & lowmask) | (addr & ~lowmask)] = val & 0xff;
			if constexpr (sizeof(T) > 1)
				val >>= 8;
		}
	}
}

} // anonymous namespace



//-------------------------------------------------
//  sol_lua_push - automatically convert
//  map_handler_type to a string
//-------------------------------------------------

int sol_lua_push(sol::types<map_handler_type>, lua_State *L, map_handler_type &&value)
{
	const char *typestr;
	switch(value)
	{
	case AMH_NONE:
		typestr = "none";
		break;
	case AMH_RAM:
		typestr = "ram";
		break;
	case AMH_ROM:
		typestr = "rom";
		break;
	case AMH_NOP:
		typestr = "nop";
		break;
	case AMH_UNMAP:
		typestr = "unmap";
		break;
	case AMH_DEVICE_DELEGATE:
	case AMH_DEVICE_DELEGATE_M:
	case AMH_DEVICE_DELEGATE_S:
	case AMH_DEVICE_DELEGATE_SM:
	case AMH_DEVICE_DELEGATE_MO:
	case AMH_DEVICE_DELEGATE_SMO:
		typestr = "delegate";
		break;
	case AMH_PORT:
		typestr = "port";
		break;
	case AMH_BANK:
		typestr = "bank";
		break;
	case AMH_DEVICE_SUBMAP:
		typestr = "submap";
		break;
	default:
		typestr = "unknown";
		break;
	}
	return sol::stack::push(L, typestr);
}


//-------------------------------------------------
//  sol_lua_push - automatically convert
//  endianness_t to a string
//-------------------------------------------------

int sol_lua_push(sol::types<endianness_t>, lua_State *L, endianness_t &&value)
{
	return sol::stack::push(L, util::endian_to_string_view(value));
}


//-------------------------------------------------
//  tap_helper - class for managing address space
//  taps
//-------------------------------------------------

class lua_engine::tap_helper
{
public:
	tap_helper(tap_helper const &) = delete;
	tap_helper(tap_helper &&) = delete;

	tap_helper(
			lua_engine &engine,
			address_space &space,
			read_or_write mode,
			offs_t start,
			offs_t end,
			std::string &&name,
			sol::protected_function &&callback)
		: m_callback(std::move(callback))
		, m_engine(engine)
		, m_space(space)
		, m_handler()
		, m_name(std::move(name))
		, m_start(start)
		, m_end(end)
		, m_mode(mode)
		, m_installing(0U)
	{
		reinstall();
	}

	~tap_helper()
	{
		remove();
	}

	offs_t start() const noexcept { return m_start; }
	offs_t end() const noexcept { return m_end; }
	std::string const &name() const noexcept { return m_name; }

	void reinstall()
	{
		switch (m_space.data_width())
		{
		case  8: do_install<u8>();  break;
		case 16: do_install<u16>(); break;
		case 32: do_install<u32>(); break;
		case 64: do_install<u64>(); break;
		}
	}

	void remove()
	{
		++m_installing;
		try
		{
			m_handler.remove();
		}
		catch (...)
		{
			--m_installing;
			throw;
		}
		--m_installing;
	}

private:
	template <typename T>
	void do_install()
	{
		if (m_installing)
			return;
		++m_installing;
		try
		{
			m_handler.remove();

			switch (m_mode)
			{
			case read_or_write::READ:
				m_handler = m_space.install_read_tap(
						m_start,
						m_end,
						m_name,
						[this] (offs_t offset, T &data, T mem_mask)
						{
							auto result = m_engine.invoke(m_callback, offset, data, mem_mask).template get<sol::optional<T> >();
							if (result)
								data = *result;
						},
						&m_handler);
				break;
			case read_or_write::WRITE:
				m_handler = m_space.install_write_tap(
						m_start,
						m_end,
						m_name,
						[this] (offs_t offset, T &data, T mem_mask)
						{
							auto result = m_engine.invoke(m_callback, offset, data, mem_mask).template get<sol::optional<T> >();
							if (result)
								data = *result;
						},
						&m_handler);
				break;
			case read_or_write::READWRITE:
				// won't ever get here, but compilers complain about unhandled enum value
				break;
			}
		}
		catch (...)
		{
			--m_installing;
			throw;
		}
		--m_installing;
	};

	sol::protected_function m_callback;
	lua_engine &m_engine;
	address_space &m_space;
	memory_passthrough_handler m_handler;
	std::string m_name;
	offs_t const m_start;
	offs_t const m_end;
	read_or_write const m_mode;
	unsigned m_installing;
};


//-------------------------------------------------
//  mem_read - templated memory readers for <sign>,<size>
//  -> manager:machine().devices[":maincpu"].spaces["program"]:read_i8(0xC000)
//-------------------------------------------------

template <typename T>
T lua_engine::addr_space::mem_read(offs_t address)
{
	T mem_content = 0;
	switch (sizeof(mem_content) * 8)
	{
	case 8:
		mem_content = space.read_byte(address);
		break;
	case 16:
		if (WORD_ALIGNED(address))
			mem_content = space.read_word(address);
		else
			mem_content = space.read_word_unaligned(address);
		break;
	case 32:
		if (DWORD_ALIGNED(address))
			mem_content = space.read_dword(address);
		else
			mem_content = space.read_dword_unaligned(address);
		break;
	case 64:
		if (QWORD_ALIGNED(address))
			mem_content = space.read_qword(address);
		else
			mem_content = space.read_qword_unaligned(address);
		break;
	default:
		break;
	}

	return mem_content;
}

//-------------------------------------------------
//  mem_write - templated memory writer for <sign>,<size>
//  -> manager:machine().devices[":maincpu"].spaces["program"]:write_u16(0xC000, 0xF00D)
//-------------------------------------------------

template <typename T>
void lua_engine::addr_space::mem_write(offs_t address, T val)
{
	switch (sizeof(val) * 8)
	{
	case 8:
		space.write_byte(address, val);
		break;
	case 16:
		if (WORD_ALIGNED(address))
			space.write_word(address, val);
		else
			space.write_word_unaligned(address, val);
		break;
	case 32:
		if (DWORD_ALIGNED(address))
			space.write_dword(address, val);
		else
			space.write_dword_unaligned(address, val);
		break;
	case 64:
		if (QWORD_ALIGNED(address))
			space.write_qword(address, val);
		else
			space.write_qword_unaligned(address, val);
		break;
	default:
		break;
	}
}

//-------------------------------------------------
//  log_mem_read - templated logical memory readers for <sign>,<size>
//  -> manager:machine().devices[":maincpu"].spaces["program"]:read_log_i8(0xC000)
//-------------------------------------------------

template <typename T>
T lua_engine::addr_space::log_mem_read(offs_t address)
{
	if (!dev.translate(space.spacenum(), TRANSLATE_READ_DEBUG, address))
		return 0;

	T mem_content = 0;
	switch (sizeof(mem_content) * 8)
	{
	case 8:
		mem_content = space.read_byte(address);
		break;
	case 16:
		if (WORD_ALIGNED(address))
			mem_content = space.read_word(address);
		else
			mem_content = space.read_word_unaligned(address);
		break;
	case 32:
		if (DWORD_ALIGNED(address))
			mem_content = space.read_dword(address);
		else
			mem_content = space.read_dword_unaligned(address);
		break;
	case 64:
		if (QWORD_ALIGNED(address))
			mem_content = space.read_qword(address);
		else
			mem_content = space.read_qword_unaligned(address);
		break;
	default:
		break;
	}

	return mem_content;
}

//-------------------------------------------------
//  log_mem_write - templated logical memory writer for <sign>,<size>
//  -> manager:machine().devices[":maincpu"].spaces["program"]:write_log_u16(0xC000, 0xF00D)
//-------------------------------------------------

template <typename T>
void lua_engine::addr_space::log_mem_write(offs_t address, T val)
{
	if (!dev.translate(space.spacenum(), TRANSLATE_WRITE_DEBUG, address))
		return;

	switch (sizeof(val) * 8)
	{
	case 8:
		space.write_byte(address, val);
		break;
	case 16:
		if (WORD_ALIGNED(address))
			space.write_word(address, val);
		else
			space.write_word_unaligned(address, val);
		break;
	case 32:
		if (DWORD_ALIGNED(address))
			space.write_dword(address, val);
		else
			space.write_dword_unaligned(address, val);
		break;
	case 64:
		if (QWORD_ALIGNED(address))
			space.write_qword(address, val);
		else
			space.write_qword_unaligned(address, val);
		break;
	default:
		break;
	}
}

//-------------------------------------------------
//  mem_direct_read - templated direct memory readers for <sign>,<size>
//  -> manager:machine().devices[":maincpu"].spaces["program"]:read_direct_i8(0xC000)
//-------------------------------------------------

template <typename T>
T lua_engine::addr_space::direct_mem_read(offs_t address)
{
	T mem_content = 0;
	const offs_t lowmask = space.data_width() / 8 - 1;
	for (int i = 0; i < sizeof(T); i++)
	{
		int addr = space.endianness() == ENDIANNESS_LITTLE ? address + sizeof(T) - 1 - i : address + i;
		u8 *base = (u8 *)space.get_read_ptr(addr & ~lowmask);
		if (base)
		{
			if constexpr (sizeof(T) > 1)
				mem_content <<= 8;
			if (space.endianness() == ENDIANNESS_BIG)
				mem_content |= base[BYTE8_XOR_BE(addr) & lowmask];
			else
				mem_content |= base[BYTE8_XOR_LE(addr) & lowmask];
		}
	}

	return mem_content;
}

//-------------------------------------------------
//  mem_direct_write - templated memory writer for <sign>,<size>
//  -> manager:machine().devices[":maincpu"].spaces["program"]:write_direct_u16(0xC000, 0xF00D)
//-------------------------------------------------

template <typename T>
void lua_engine::addr_space::direct_mem_write(offs_t address, T val)
{
	const offs_t lowmask = space.data_width() / 8 - 1;
	for (int i = 0; i < sizeof(T); i++)
	{
		int addr = space.endianness() == ENDIANNESS_BIG ? address + sizeof(T) - 1 - i : address + i;
		u8 *base = (u8 *)space.get_read_ptr(addr & ~lowmask);
		if (base)
		{
			if (space.endianness() == ENDIANNESS_BIG)
				base[BYTE8_XOR_BE(addr) & lowmask] = val & 0xff;
			else
				base[BYTE8_XOR_LE(addr) & lowmask] = val & 0xff;
			if constexpr (sizeof(T) > 1)
				val >>= 8;
		}
	}
}

//-------------------------------------------------
//  initialize_memory - register memory user types
//-------------------------------------------------

void lua_engine::initialize_memory(sol::table &emu)
{

	auto addr_space_type = sol().registry().new_usertype<addr_space>("addr_space", sol::no_constructor);
	addr_space_type["read_i8"] = &addr_space::mem_read<s8>;
	addr_space_type["read_u8"] = &addr_space::mem_read<u8>;
	addr_space_type["read_i16"] = &addr_space::mem_read<s16>;
	addr_space_type["read_u16"] = &addr_space::mem_read<u16>;
	addr_space_type["read_i32"] = &addr_space::mem_read<s32>;
	addr_space_type["read_u32"] = &addr_space::mem_read<u32>;
	addr_space_type["read_i64"] = &addr_space::mem_read<s64>;
	addr_space_type["read_u64"] = &addr_space::mem_read<u64>;
	addr_space_type["write_i8"] = &addr_space::mem_write<s8>;
	addr_space_type["write_u8"] = &addr_space::mem_write<u8>;
	addr_space_type["write_i16"] = &addr_space::mem_write<s16>;
	addr_space_type["write_u16"] = &addr_space::mem_write<u16>;
	addr_space_type["write_i32"] = &addr_space::mem_write<s32>;
	addr_space_type["write_u32"] = &addr_space::mem_write<u32>;
	addr_space_type["write_i64"] = &addr_space::mem_write<s64>;
	addr_space_type["write_u64"] = &addr_space::mem_write<u64>;
	addr_space_type["readv_i8"] = &addr_space::log_mem_read<s8>;
	addr_space_type["readv_u8"] = &addr_space::log_mem_read<u8>;
	addr_space_type["readv_i16"] = &addr_space::log_mem_read<s16>;
	addr_space_type["readv_u16"] = &addr_space::log_mem_read<u16>;
	addr_space_type["readv_i32"] = &addr_space::log_mem_read<s32>;
	addr_space_type["readv_u32"] = &addr_space::log_mem_read<u32>;
	addr_space_type["readv_i64"] = &addr_space::log_mem_read<s64>;
	addr_space_type["readv_u64"] = &addr_space::log_mem_read<u64>;
	addr_space_type["writev_i8"] = &addr_space::log_mem_write<s8>;
	addr_space_type["writev_u8"] = &addr_space::log_mem_write<u8>;
	addr_space_type["writev_i16"] = &addr_space::log_mem_write<s16>;
	addr_space_type["writev_u16"] = &addr_space::log_mem_write<u16>;
	addr_space_type["writev_i32"] = &addr_space::log_mem_write<s32>;
	addr_space_type["writev_u32"] = &addr_space::log_mem_write<u32>;
	addr_space_type["writev_i64"] = &addr_space::log_mem_write<s64>;
	addr_space_type["writev_u64"] = &addr_space::log_mem_write<u64>;
	addr_space_type["read_direct_i8"] = &addr_space::direct_mem_read<s8>;
	addr_space_type["read_direct_u8"] = &addr_space::direct_mem_read<u8>;
	addr_space_type["read_direct_i16"] = &addr_space::direct_mem_read<s16>;
	addr_space_type["read_direct_u16"] = &addr_space::direct_mem_read<u16>;
	addr_space_type["read_direct_i32"] = &addr_space::direct_mem_read<s32>;
	addr_space_type["read_direct_u32"] = &addr_space::direct_mem_read<u32>;
	addr_space_type["read_direct_i64"] = &addr_space::direct_mem_read<s64>;
	addr_space_type["read_direct_u64"] = &addr_space::direct_mem_read<u64>;
	addr_space_type["write_direct_i8"] = &addr_space::direct_mem_write<s8>;
	addr_space_type["write_direct_u8"] = &addr_space::direct_mem_write<u8>;
	addr_space_type["write_direct_i16"] = &addr_space::direct_mem_write<s16>;
	addr_space_type["write_direct_u16"] = &addr_space::direct_mem_write<u16>;
	addr_space_type["write_direct_i32"] = &addr_space::direct_mem_write<s32>;
	addr_space_type["write_direct_u32"] = &addr_space::direct_mem_write<u32>;
	addr_space_type["write_direct_i64"] = &addr_space::direct_mem_write<s64>;
	addr_space_type["write_direct_u64"] = &addr_space::direct_mem_write<u64>;
	addr_space_type["read_range"] =
			[] (addr_space &sp, sol::this_state s, u64 first, u64 last, int width, sol::object opt_step) -> sol::object
			{
				lua_State *L = s;
				luaL_Buffer buff;
				offs_t space_size = sp.space.addrmask();
				u64 step = 1;
				if (opt_step.is<u64>())
				{
					step = opt_step.as<u64>();
					if (step < 1 || step > last - first)
					{
						luaL_error(L, "Invalid step");
						return sol::lua_nil;
					}
				}
				if (first > space_size || last > space_size || last < first)
				{
					luaL_error(L, "Invalid offset");
					return sol::lua_nil;
				}
				int byte_count = width / 8 * (last - first + 1) / step;
				switch (width)
				{
				case 8:
					{
						u8 *dest = (u8 *)luaL_buffinitsize(L, &buff, byte_count);
						for ( ; first <= last; first += step)
							*dest++ = sp.mem_read<u8>(first);
						break;
					}
				case 16:
					{
						u16 *dest = (u16 *)luaL_buffinitsize(L, &buff, byte_count);
						for ( ; first <= last; first += step)
							*dest++ = sp.mem_read<u16>(first);
						break;
					}
				case 32:
					{
						u32 *dest = (u32 *)luaL_buffinitsize(L, &buff, byte_count);
						for( ; first <= last; first += step)
							*dest++ = sp.mem_read<u32>(first);
						break;
					}
				case 64:
					{
						u64 *dest = (u64 *)luaL_buffinitsize(L, &buff, byte_count);
						for( ; first <= last; first += step)
							*dest++ = sp.mem_read<u64>(first);
						break;
					}
				default:
					luaL_error(L, "Invalid width. Must be 8/16/32/64");
					return sol::lua_nil;
				}
				luaL_pushresultsize(&buff, byte_count);
				return sol::make_reference(L, sol::stack_reference(L, -1));
			};
	addr_space_type["add_change_notifier"] =
			[this] (addr_space &sp, sol::protected_function &&cb)
			{
				return sp.space.add_change_notifier(
						[this, callback = std::move(cb)] (read_or_write mode)
						{
							char const *modestr = "";
							switch (mode)
							{
							case read_or_write::READ:      modestr = "r";  break;
							case read_or_write::WRITE:     modestr = "w";  break;
							case read_or_write::READWRITE: modestr = "rw"; break;
							}
							invoke(callback, modestr);
						});
			};
	addr_space_type["install_read_tap"] =
			[this] (addr_space &sp, offs_t start, offs_t end, std::string &&name, sol::protected_function &&cb)
			{
				return std::make_unique<tap_helper>(*this, sp.space, read_or_write::READ, start, end, std::move(name), std::move(cb));
			};
	addr_space_type["install_write_tap"] =
			[this] (addr_space &sp, offs_t start, offs_t end, std::string &&name, sol::protected_function &&cb)
			{
				return std::make_unique<tap_helper>(*this, sp.space, read_or_write::WRITE, start, end, std::move(name), std::move(cb));
			};
	addr_space_type["name"] = sol::property([] (addr_space &sp) { return sp.space.name(); });
	addr_space_type["shift"] = sol::property([] (addr_space &sp) { return sp.space.addr_shift(); });
	addr_space_type["index"] = sol::property([] (addr_space &sp) { return sp.space.spacenum(); });
	addr_space_type["address_mask"] = sol::property([] (addr_space &sp) { return sp.space.addrmask(); });
	addr_space_type["data_width"] = sol::property([] (addr_space &sp) { return sp.space.data_width(); });
	addr_space_type["endianness"] = sol::property([] (addr_space &sp) { return sp.space.endianness(); });
	addr_space_type["map"] = sol::property([] (addr_space &sp) { return sp.space.map(); });


	auto tap_type = sol().registry().new_usertype<tap_helper>("mempassthrough", sol::no_constructor);
	tap_type["reinstall"] = &tap_helper::reinstall;
	tap_type["remove"] = &tap_helper::remove;
	tap_type["addrstart"] = sol::property(&tap_helper::start);
	tap_type["addrend"] = sol::property(&tap_helper::end);
	tap_type["name"] = sol::property(&tap_helper::name);


	auto addrmap_type = sol().registry().new_usertype<address_map>("addrmap", sol::no_constructor);
	addrmap_type["spacenum"] = sol::readonly(&address_map::m_spacenum);
	addrmap_type["device"] = sol::readonly(&address_map::m_device);
	addrmap_type["unmap_value"] = sol::readonly(&address_map::m_unmapval);
	addrmap_type["global_mask"] = sol::readonly(&address_map::m_globalmask);
	addrmap_type["entries"] = sol::property([] (address_map &m) { return simple_list_wrapper<address_map_entry>(m.m_entrylist); });


	auto mapentry_type = sol().registry().new_usertype<address_map_entry>("mapentry", sol::no_constructor);
	mapentry_type["address_start"] = sol::readonly(&address_map_entry::m_addrstart);
	mapentry_type["address_end"] = sol::readonly(&address_map_entry::m_addrend);
	mapentry_type["address_mirror"] = sol::readonly(&address_map_entry::m_addrmirror);
	mapentry_type["address_mask"] = sol::readonly(&address_map_entry::m_addrmask);
	mapentry_type["mask"] = sol::readonly(&address_map_entry::m_mask);
	mapentry_type["cswidth"] = sol::readonly(&address_map_entry::m_cswidth);
	mapentry_type["read"] = sol::readonly(&address_map_entry::m_read);
	mapentry_type["write"] = sol::readonly(&address_map_entry::m_write);
	mapentry_type["share"] = sol::readonly(&address_map_entry::m_share);
	mapentry_type["region"] = sol::readonly(&address_map_entry::m_region);
	mapentry_type["region_offset"] = sol::readonly(&address_map_entry::m_rgnoffs);


	auto handler_data_type = sol().registry().new_usertype<map_handler_data>("handlerdata", sol::no_constructor);
	handler_data_type["handlertype"] = sol::property([] (map_handler_data const &hd) { return hd.m_type; }); // can't use member pointer or won't be converted to string
	handler_data_type["bits"] = sol::readonly(&map_handler_data::m_bits);
	handler_data_type["name"] = sol::readonly(&map_handler_data::m_name);
	handler_data_type["tag"] = sol::readonly(&map_handler_data::m_tag);


	auto memory_type = sol().registry().new_usertype<memory_manager>("memory", sol::no_constructor);
	memory_type["banks"] = sol::property([] (memory_manager &mm) { return standard_tag_object_ptr_map<memory_bank>(mm.banks()); });
	memory_type["regions"] = sol::property([] (memory_manager &mm) { return standard_tag_object_ptr_map<memory_region>(mm.regions()); });
	memory_type["shares"] = sol::property([] (memory_manager &mm) { return standard_tag_object_ptr_map<memory_share>(mm.shares()); });


	auto bank_type = sol().registry().new_usertype<memory_bank>("membank", sol::no_constructor);
	bank_type["tag"] = sol::property(&memory_bank::tag);
	bank_type["entry"] = sol::property(&memory_bank::entry, &memory_bank::set_entry);


	auto region_type = sol().registry().new_usertype<memory_region>("region", sol::no_constructor);
	region_type["read_i8"] = &region_read<s8>;
	region_type["read_u8"] = &region_read<u8>;
	region_type["read_i16"] = &region_read<s16>;
	region_type["read_u16"] = &region_read<u16>;
	region_type["read_i32"] = &region_read<s32>;
	region_type["read_u32"] = &region_read<u32>;
	region_type["read_i64"] = &region_read<s64>;
	region_type["read_u64"] = &region_read<u64>;
	region_type["write_i8"] = &region_write<s8>;
	region_type["write_u8"] = &region_write<u8>;
	region_type["write_i16"] = &region_write<s16>;
	region_type["write_u16"] = &region_write<u16>;
	region_type["write_i32"] = &region_write<s32>;
	region_type["write_u32"] = &region_write<u32>;
	region_type["write_i64"] = &region_write<s64>;
	region_type["write_u64"] = &region_write<u64>;
	region_type["tag"] = sol::property(&memory_region::name);
	region_type["size"] = sol::property(&memory_region::bytes);
	region_type["length"] = sol::property([] (memory_region &r) { return r.bytes() / r.bytewidth(); });
	region_type["endianness"] = sol::property(&memory_region::endianness);
	region_type["bitwidth"] = sol::property(&memory_region::bitwidth);
	region_type["bytewidth"] = sol::property(&memory_region::bytewidth);


	auto share_type = sol().registry().new_usertype<memory_share>("share", sol::no_constructor);
	share_type["read_i8"] = &share_read<s8>;
	share_type["read_u8"] = &share_read<u8>;
	share_type["read_i16"] = &share_read<s16>;
	share_type["read_u16"] = &share_read<u16>;
	share_type["read_i32"] = &share_read<s32>;
	share_type["read_u32"] = &share_read<u32>;
	share_type["read_i64"] = &share_read<s64>;
	share_type["read_u64"] = &share_read<u64>;
	share_type["write_i8"] = &share_write<s8>;
	share_type["write_u8"] = &share_write<u8>;
	share_type["write_i16"] = &share_write<s16>;
	share_type["write_u16"] = &share_write<u16>;
	share_type["write_i32"] = &share_write<s32>;
	share_type["write_u32"] = &share_write<u32>;
	share_type["write_i64"] = &share_write<s64>;
	share_type["write_u64"] = &share_write<u64>;
	share_type["tag"] = sol::property(&memory_share::name);
	share_type["size"] = sol::property(&memory_share::bytes);
	share_type["length"] = sol::property([] (memory_share &s) { return s.bytes() / s.bytewidth(); });
	share_type["endianness"] = sol::property(&memory_share::endianness);
	share_type["bitwidth"] = sol::property(&memory_share::bitwidth);
	share_type["bytewidth"] = sol::property(&memory_share::bytewidth);

}
