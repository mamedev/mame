// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic,Luca Bruno
/***************************************************************************

    luaengine_input.cpp

    Controls execution of the core MAME system.

***************************************************************************/

#include "emu.h"
#include "luaengine.ipp"


namespace {


template <typename T>
std::string get_endianness_name(T const &obj)
{
	std::string endianness;
	switch (obj.endianness())
	{
	case endianness_t::ENDIANNESS_BIG:
		endianness = "big";
		break;
	case endianness_t::ENDIANNESS_LITTLE:
		endianness = "little";
		break;
	}
	return endianness;
}


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

	auto addr_space_type = sol().registry().new_usertype<addr_space>(
			"addr_space",
			sol::call_constructor,
			sol::constructors<sol::types<address_space &, device_memory_interface &>>());
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
	addr_space_type["read_log_i8"] = &addr_space::log_mem_read<s8>;
	addr_space_type["read_log_u8"] = &addr_space::log_mem_read<u8>;
	addr_space_type["read_log_i16"] = &addr_space::log_mem_read<s16>;
	addr_space_type["read_log_u16"] = &addr_space::log_mem_read<u16>;
	addr_space_type["read_log_i32"] = &addr_space::log_mem_read<s32>;
	addr_space_type["read_log_u32"] = &addr_space::log_mem_read<u32>;
	addr_space_type["read_log_i64"] = &addr_space::log_mem_read<s64>;
	addr_space_type["read_log_u64"] = &addr_space::log_mem_read<u64>;
	addr_space_type["write_log_i8"] = &addr_space::log_mem_write<s8>;
	addr_space_type["write_log_u8"] = &addr_space::log_mem_write<u8>;
	addr_space_type["write_log_i16"] = &addr_space::log_mem_write<s16>;
	addr_space_type["write_log_u16"] = &addr_space::log_mem_write<u16>;
	addr_space_type["write_log_i32"] = &addr_space::log_mem_write<s32>;
	addr_space_type["write_log_u32"] = &addr_space::log_mem_write<u32>;
	addr_space_type["write_log_i64"] = &addr_space::log_mem_write<s64>;
	addr_space_type["write_log_u64"] = &addr_space::log_mem_write<u64>;
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
		[] (addr_space &sp, sol::this_state s, u64 first, u64 last, int width, sol::object opt_step)
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
		};
	addr_space_type["name"] = sol::property([] (addr_space &sp) { return sp.space.name(); });
	addr_space_type["shift"] = sol::property([] (addr_space &sp) { return sp.space.addr_shift(); });
	addr_space_type["index"] = sol::property([] (addr_space &sp) { return sp.space.spacenum(); });
	addr_space_type["address_mask"] = sol::property([] (addr_space &sp) { return sp.space.addrmask(); });
	addr_space_type["data_width"] = sol::property([] (addr_space &sp) { return sp.space.data_width(); });
	addr_space_type["endianness"] = sol::property([] (addr_space &sp) { return get_endianness_name(sp.space); });


/* address_map_entry library
 *
 * manager:machine().devices[device_tag].spaces[space].map[entry_index]
 *
 * mapentry.offset - address start
 * mapentry.endoff - address end
 * mapentry.readtype
 * mapentry.writetype
 */
	addr_space_type["map"] = sol::property(
			[this] (addr_space &sp)
			{
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
			});


/* memory_manager library
 *
 * manager:machine():memory()
 *
 * memory.banks[] - table of memory banks (k=tag, v=memory_bank)
 * memory.regions[] - table of memory regions (k=tag, v=memory_region)
 * memory.shares[] - table of memory shares (k=tag, v=memory_share)
 */

	auto memory_type = sol().registry().new_usertype<memory_manager>("memory", sol::no_constructor);
	memory_type["banks"] = sol::property([] (memory_manager &mm) { return standard_tag_object_ptr_map<memory_bank>(mm.banks()); });
	memory_type["regions"] = sol::property([] (memory_manager &mm) { return standard_tag_object_ptr_map<memory_region>(mm.regions()); });
	memory_type["shares"] = sol::property([] (memory_manager &mm) { return standard_tag_object_ptr_map<memory_share>(mm.shares()); });


/* memory_bank library
 *
 * manager:machine():memory().banks[bank_tag]
 *
 * region.tag - absolute tag of the bank
 * bank.entry - get/set the selected entry
 */

	auto bank_type = sol().registry().new_usertype<memory_bank>("membank", sol::no_constructor);
	bank_type["tag"] = sol::property(&memory_bank::tag);
	bank_type["entry"] = sol::property(&memory_bank::entry, &memory_bank::set_entry);


/* memory_region library
 *
 * manager:machine():memory().regions[region_tag]
 *
 * read/write by signedness u/i and bit-width 8/16/32/64:
 * region:read_*(addr)
 * region:write_*(addr, val)
 *
 * region.tag - absolute tag of the region
 * region.size - size in bytes
 * region.length - length in items
 * region.endianness - endiannes as string ("big" or "little")
 * region.bitwidth - item width in bits
 * region.bytewidth - item width in bytes
 */

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
	region_type["endianness"] = sol::property(&get_endianness_name<memory_region>);
	region_type["bitwidth"] = sol::property(&memory_region::bitwidth);
	region_type["bytewidth"] = sol::property(&memory_region::bytewidth);


/* memory_share library
 *
 * manager:machine():memory().shares[share_tag]
 *
 * read/write by signedness u/i and bit-width 8/16/32/64:
 * share:read_*(addr)
 * share:write_*(addr, val)
 *
 * share.tag - absolute tag of the share
 * share.size - size in bytes
 * share.length - length in items
 * region.endianness - endiannes as string ("big" or "little")
 * share.bitwidth - item width in bits
 * share.bytewidth - item width in bytes
*/

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
	share_type["endianness"] = sol::property(&get_endianness_name<memory_share>);
	share_type["bitwidth"] = sol::property(&memory_share::bitwidth);
	share_type["bytewidth"] = sol::property(&memory_share::bytewidth);

}
