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
	uint8_t *ptr = (uint8_t *)share.ptr();
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
	uint8_t *ptr = (uint8_t *)share.ptr();
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
		uint8_t *base = (uint8_t *)space.get_read_ptr(addr & ~lowmask);
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
		uint8_t *base = (uint8_t *)space.get_read_ptr(addr & ~lowmask);
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

void lua_engine::initialize_memory()
{

/* addr_space library
 *
 * manager:machine().devices[device_tag].spaces[space]
 *
 * read/write by signedness u/i and bit-width 8/16/32/64:
 * space:read_*(addr)
 * space:write_*(addr, val)
 * space:read_log_*(addr)
 * space:write_log_*(addr, val)
 * space:read_direct_*(addr)
 * space:write_direct_*(addr, val)
 * space:read_range(first_addr, last_addr, width, [opt] step) - read range of addresses and
 *                                                              return as a binary string
 *
 * space.name - address space name
 * space.shift - address bus shift, bitshift required for a bytewise address
 *               to map onto this space's address resolution (addressing granularity).
 *               positive value means leftshift, negative means rightshift.
 * space.index
 * space.address_mask
 * space.data_width
 * space.endianness
 *
 * space.map[] - table of address map entries (k=index, v=address_map_entry)
 */

	auto addr_space_type = sol().registry().new_usertype<addr_space>("addr_space", sol::call_constructor, sol::constructors<sol::types<address_space &, device_memory_interface &>>());
	addr_space_type.set("read_i8", &addr_space::mem_read<int8_t>);
	addr_space_type.set("read_u8", &addr_space::mem_read<uint8_t>);
	addr_space_type.set("read_i16", &addr_space::mem_read<int16_t>);
	addr_space_type.set("read_u16", &addr_space::mem_read<uint16_t>);
	addr_space_type.set("read_i32", &addr_space::mem_read<int32_t>);
	addr_space_type.set("read_u32", &addr_space::mem_read<uint32_t>);
	addr_space_type.set("read_i64", &addr_space::mem_read<int64_t>);
	addr_space_type.set("read_u64", &addr_space::mem_read<uint64_t>);
	addr_space_type.set("write_i8", &addr_space::mem_write<int8_t>);
	addr_space_type.set("write_u8", &addr_space::mem_write<uint8_t>);
	addr_space_type.set("write_i16", &addr_space::mem_write<int16_t>);
	addr_space_type.set("write_u16", &addr_space::mem_write<uint16_t>);
	addr_space_type.set("write_i32", &addr_space::mem_write<int32_t>);
	addr_space_type.set("write_u32", &addr_space::mem_write<uint32_t>);
	addr_space_type.set("write_i64", &addr_space::mem_write<int64_t>);
	addr_space_type.set("write_u64", &addr_space::mem_write<uint64_t>);
	addr_space_type.set("read_log_i8", &addr_space::log_mem_read<int8_t>);
	addr_space_type.set("read_log_u8", &addr_space::log_mem_read<uint8_t>);
	addr_space_type.set("read_log_i16", &addr_space::log_mem_read<int16_t>);
	addr_space_type.set("read_log_u16", &addr_space::log_mem_read<uint16_t>);
	addr_space_type.set("read_log_i32", &addr_space::log_mem_read<int32_t>);
	addr_space_type.set("read_log_u32", &addr_space::log_mem_read<uint32_t>);
	addr_space_type.set("read_log_i64", &addr_space::log_mem_read<int64_t>);
	addr_space_type.set("read_log_u64", &addr_space::log_mem_read<uint64_t>);
	addr_space_type.set("write_log_i8", &addr_space::log_mem_write<int8_t>);
	addr_space_type.set("write_log_u8", &addr_space::log_mem_write<uint8_t>);
	addr_space_type.set("write_log_i16", &addr_space::log_mem_write<int16_t>);
	addr_space_type.set("write_log_u16", &addr_space::log_mem_write<uint16_t>);
	addr_space_type.set("write_log_i32", &addr_space::log_mem_write<int32_t>);
	addr_space_type.set("write_log_u32", &addr_space::log_mem_write<uint32_t>);
	addr_space_type.set("write_log_i64", &addr_space::log_mem_write<int64_t>);
	addr_space_type.set("write_log_u64", &addr_space::log_mem_write<uint64_t>);
	addr_space_type.set("read_direct_i8", &addr_space::direct_mem_read<int8_t>);
	addr_space_type.set("read_direct_u8", &addr_space::direct_mem_read<uint8_t>);
	addr_space_type.set("read_direct_i16", &addr_space::direct_mem_read<int16_t>);
	addr_space_type.set("read_direct_u16", &addr_space::direct_mem_read<uint16_t>);
	addr_space_type.set("read_direct_i32", &addr_space::direct_mem_read<int32_t>);
	addr_space_type.set("read_direct_u32", &addr_space::direct_mem_read<uint32_t>);
	addr_space_type.set("read_direct_i64", &addr_space::direct_mem_read<int64_t>);
	addr_space_type.set("read_direct_u64", &addr_space::direct_mem_read<uint64_t>);
	addr_space_type.set("write_direct_i8", &addr_space::direct_mem_write<int8_t>);
	addr_space_type.set("write_direct_u8", &addr_space::direct_mem_write<uint8_t>);
	addr_space_type.set("write_direct_i16", &addr_space::direct_mem_write<int16_t>);
	addr_space_type.set("write_direct_u16", &addr_space::direct_mem_write<uint16_t>);
	addr_space_type.set("write_direct_i32", &addr_space::direct_mem_write<int32_t>);
	addr_space_type.set("write_direct_u32", &addr_space::direct_mem_write<uint32_t>);
	addr_space_type.set("write_direct_i64", &addr_space::direct_mem_write<int64_t>);
	addr_space_type.set("write_direct_u64", &addr_space::direct_mem_write<uint64_t>);
	addr_space_type.set("read_range", [](addr_space &sp, sol::this_state s, u64 first, u64 last, int width, sol::object opt_step) {
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
					return sol::make_reference(L, nullptr);
				}
			}
			if (first > space_size || last > space_size || last < first)
			{
				luaL_error(L, "Invalid offset");
				return sol::make_reference(L, nullptr);
			}
			int byte_count = width / 8 * (last - first + 1) / step;
			switch (width)
			{
			case 8:
			{
				u8 *dest = (u8 *)luaL_buffinitsize(L, &buff, byte_count);
				for(; first <= last; first += step)
					*dest++ = sp.mem_read<u8>(first);
				break;
			}
			case 16:
			{
				u16 *dest = (u16 *)luaL_buffinitsize(L, &buff, byte_count);
				for(; first <= last; first += step)
					*dest++ = sp.mem_read<u16>(first);
				break;
			}
			case 32:
			{
				u32 *dest = (u32 *)luaL_buffinitsize(L, &buff, byte_count);
				for(; first <= last; first += step)
					*dest++ = sp.mem_read<u32>(first);
				break;
			}
			case 64:
			{
				u64 *dest = (u64 *)luaL_buffinitsize(L, &buff, byte_count);
				for(; first <= last; first += step)
					*dest++ = sp.mem_read<u64>(first);
				break;
			}
			default:
				luaL_error(L, "Invalid width. Must be 8/16/32/64");
				return sol::make_reference(L, nullptr);
			}
			luaL_pushresultsize(&buff, byte_count);
			return sol::make_reference(L, sol::stack_reference(L, -1));
		});
	addr_space_type.set("name", sol::property([](addr_space &sp) { return sp.space.name(); }));
	addr_space_type.set("shift", sol::property([](addr_space &sp) { return sp.space.addr_shift(); }));
	addr_space_type.set("index", sol::property([](addr_space &sp) { return sp.space.spacenum(); }));
	addr_space_type.set("address_mask", sol::property([](addr_space &sp) { return sp.space.addrmask(); }));
	addr_space_type.set("data_width", sol::property([](addr_space &sp) { return sp.space.data_width(); }));
	addr_space_type.set("endianness", sol::property([](addr_space &sp) {
			std::string endianness;
			switch (sp.space.endianness())
			{
				case endianness_t::ENDIANNESS_BIG:
					endianness = "big";
					break;
				case endianness_t::ENDIANNESS_LITTLE:
					endianness = "little";
					break;
			}
			return endianness;
		}));


/* address_map_entry library
 *
 * manager:machine().devices[device_tag].spaces[space].map[entry_index]
 *
 * mapentry.offset - address start
 * mapentry.endoff - address end
 * mapentry.readtype
 * mapentry.writetype
 */
	addr_space_type.set("map", sol::property([this](addr_space &sp) {
			address_space &space = sp.space;
			sol::table map = sol().create_table();
			for (address_map_entry &entry : space.map()->m_entrylist)
			{
				sol::table mapentry = sol().create_table();
				mapentry["offset"] = entry.m_addrstart & space.addrmask();
				mapentry["endoff"] = entry.m_addrend & space.addrmask();
				mapentry["readtype"] = entry.m_read.m_type;
				mapentry["writetype"] = entry.m_write.m_type;
				map.add(mapentry);
			}
			return map;
		}));


/* memory_manager library
 *
 * manager:machine():memory()
 *
 * memory.banks[] - table of memory banks (k=tag, v=memory_bank)
 * memory.regions[] - table of memory regions (k=tag, v=memory_region)
 * memory.shares[] - table of memory shares (k=tag, v=memory_share)
 */

	auto memory_type = sol().registry().new_usertype<memory_manager>("memory", "new", sol::no_constructor);
	memory_type.set("banks", sol::property([this](memory_manager &mm) {
			sol::table table = sol().create_table();
			for (auto &bank : mm.banks())
				table[bank.second->tag()] = bank.second.get();
			return table;
		}));
	memory_type.set("regions", sol::property([this](memory_manager &mm) {
			sol::table table = sol().create_table();
			for (auto &region : mm.regions())
				table[region.second->name()] = region.second.get();
			return table;
		}));
	memory_type.set("shares", sol::property([this](memory_manager &mm) {
			sol::table table = sol().create_table();
			for (auto &share : mm.shares())
				table[share.first] = share.second.get();
			return table;
		}));


/* memory_region library
 *
 * manager:machine():memory().regions[region_tag]
 *
 * read/write by signedness u/i and bit-width 8/16/32/64:
 * region:read_*(addr)
 * region:write_*(addr, val)
 *
 * region.size
 */

	auto region_type = sol().registry().new_usertype<memory_region>("region", "new", sol::no_constructor);
	region_type.set("read_i8", &region_read<int8_t>);
	region_type.set("read_u8", &region_read<uint8_t>);
	region_type.set("read_i16", &region_read<int16_t>);
	region_type.set("read_u16", &region_read<uint16_t>);
	region_type.set("read_i32", &region_read<int32_t>);
	region_type.set("read_u32", &region_read<uint32_t>);
	region_type.set("read_i64", &region_read<int64_t>);
	region_type.set("read_u64", &region_read<uint64_t>);
	region_type.set("write_i8", &region_write<int8_t>);
	region_type.set("write_u8", &region_write<uint8_t>);
	region_type.set("write_i16", &region_write<int16_t>);
	region_type.set("write_u16", &region_write<uint16_t>);
	region_type.set("write_i32", &region_write<int32_t>);
	region_type.set("write_u32", &region_write<uint32_t>);
	region_type.set("write_i64", &region_write<int64_t>);
	region_type.set("write_u64", &region_write<uint64_t>);
	region_type.set("size", sol::property(&memory_region::bytes));


/* memory_share library
 *
 * manager:machine():memory().shares[share_tag]
 *
 * read/write by signedness u/i and bit-width 8/16/32/64:
 * share:read_*(addr)
 * share:write_*(addr, val)
 *
 * region.size
*/

	auto share_type = sol().registry().new_usertype<memory_share>("share", "new", sol::no_constructor);
	share_type.set("read_i8", &share_read<int8_t>);
	share_type.set("read_u8", &share_read<uint8_t>);
	share_type.set("read_i16", &share_read<int16_t>);
	share_type.set("read_u16", &share_read<uint16_t>);
	share_type.set("read_i32", &share_read<int32_t>);
	share_type.set("read_u32", &share_read<uint32_t>);
	share_type.set("read_i64", &share_read<int64_t>);
	share_type.set("read_u64", &share_read<uint64_t>);
	share_type.set("write_i8", &share_write<int8_t>);
	share_type.set("write_u8", &share_write<uint8_t>);
	share_type.set("write_i16", &share_write<int16_t>);
	share_type.set("write_u16", &share_write<uint16_t>);
	share_type.set("write_i32", &share_write<int32_t>);
	share_type.set("write_u32", &share_write<uint32_t>);
	share_type.set("write_i64", &share_write<int64_t>);
	share_type.set("write_u64", &share_write<uint64_t>);
	share_type.set("size", sol::property(&memory_share::bytes));

}
