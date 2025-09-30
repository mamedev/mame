// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "emumem_mud.h"

template<typename T> static u8 mask_to_ukey(T mask);

template<> u8 mask_to_ukey<u16>(u16 mask)
{
	return
		(mask & 0xff00 ? 0x02 : 0x00) |
		(mask & 0x00ff ? 0x01 : 0x00);
}

template<> u8 mask_to_ukey<u32>(u32 mask)
{
	return
		(mask & 0xff000000 ? 0x08 : 0x00) |
		(mask & 0x00ff0000 ? 0x04 : 0x00) |
		(mask & 0x0000ff00 ? 0x02 : 0x00) |
		(mask & 0x000000ff ? 0x01 : 0x00);
}

template<> u8 mask_to_ukey<u64>(u64 mask)
{
	return
		(mask & 0xff00000000000000 ? 0x80 : 0x00) |
		(mask & 0x00ff000000000000 ? 0x40 : 0x00) |
		(mask & 0x0000ff0000000000 ? 0x20 : 0x00) |
		(mask & 0x000000ff00000000 ? 0x10 : 0x00) |
		(mask & 0x00000000ff000000 ? 0x08 : 0x00) |
		(mask & 0x0000000000ff0000 ? 0x04 : 0x00) |
		(mask & 0x000000000000ff00 ? 0x02 : 0x00) |
		(mask & 0x00000000000000ff ? 0x01 : 0x00);
}

template<int Width, int AddrShift> memory_units_descriptor<Width, AddrShift>::memory_units_descriptor(u8 access_width, endianness_t access_endian, handler_entry *handler, offs_t addrstart, offs_t addrend, offs_t mask, emu::detail::handler_entry_size_t<Width> unitmask, int cswidth) : m_handler(handler), m_access_width(access_width), m_access_endian(access_endian)
{
	u32 bits_per_access = 8 << access_width;
	constexpr u32 NATIVE_MASK = Width + AddrShift >= 0 ? make_bitmask<u32>(Width + AddrShift) : 0;

	// Compute the real base addresses
	m_addrstart = addrstart & ~NATIVE_MASK;
	m_addrend = addrend & ~NATIVE_MASK;

	// Compute the masks and the keys
	std::array<uX, 4> umasks;
	umasks.fill(unitmask);

	uX smask, emask;
	if(access_endian == ENDIANNESS_BIG) {
		smask =  make_bitmask<uX>(8*sizeof(uX) - ((addrstart - m_addrstart) << (3 - AddrShift)));
		emask = ~make_bitmask<uX>(8*sizeof(uX) - ((addrend - m_addrend + 1) << (3 - AddrShift)));
	} else {
		smask = ~make_bitmask<uX>((addrstart - m_addrstart) << (3 - AddrShift));
		emask =  make_bitmask<uX>((addrend - m_addrend + 1) << (3 - AddrShift));
	}

	umasks[handler_entry::START]                    &= smask;
	umasks[handler_entry::END]                      &= emask;
	umasks[handler_entry::START|handler_entry::END] &= smask & emask;

	for(u32 i=0; i<4; i++)
		m_keymap[i] = mask_to_ukey<uX>(umasks[i]);

	// Compute the shift
	uX dmask = make_bitmask<uX>(bits_per_access);
	u32 active_count = 0;
	for(u32 i=0; i != 8 << Width; i += bits_per_access)
		if(unitmask & (dmask << i))
			active_count ++;
	u32 active_count_log = active_count == 1 ? 0 : active_count == 2 ? 1 : active_count <= 4 ? 2 : active_count <= 8 ? 3 : 0xff;
	if(active_count_log == 0xff)
		abort();
	s8 base_shift = Width - access_width - active_count_log;
	s8 shift = base_shift + access_width + AddrShift;

	// Build the handler characteristics
	m_handler_start = shift < 0 ? addrstart << -shift : addrstart >> shift;
	m_handler_mask = (shift < 0 ? (mask << -shift) | make_bitmask<offs_t>(-shift) : mask >> shift) | ((1 << active_count_log) - 1);
	//osd_printf_debug("active_count=%d shift=%d addrstart=%X mask=%X handler_start=%X handler_mask=%X\n", active_count, shift, addrstart, mask, m_handler_start, m_handler_mask);

	for(u32 i=0; i<4; i++)
		if(m_entries_for_key.find(m_keymap[i]) == m_entries_for_key.end())
			generate(m_keymap[i], unitmask, umasks[i], cswidth, bits_per_access, base_shift, shift, active_count);
}

template<int Width, int AddrShift> void memory_units_descriptor<Width, AddrShift>::generate(u8 ukey, emu::detail::handler_entry_size_t<Width> gumask, emu::detail::handler_entry_size_t<Width> umask, u32 cswidth, u32 bits_per_access, u8 base_shift, s8 shift, u32 active_count)
{
	auto &entries = m_entries_for_key[ukey];

	// Compute the selection masks
	if(!cswidth)
		cswidth = bits_per_access;

	uX csmask = make_bitmask<uX>(cswidth);
	uX dmask = make_bitmask<uX>(bits_per_access);

	u32 offset = 0;

	for(u32 i=0; i != 8 << Width; i += bits_per_access) {
		uX numask = dmask << i;
		if(umask & numask) {
			uX amask = csmask << (i & ~(cswidth - 1));
			//osd_printf_debug("umask=%X amask=%X bits_per_access=%d cswidth=%d active_count=%d shift=%d i=%d offset=%d\n", umask, amask, bits_per_access, cswidth, active_count, shift, i, offset);
			entries.emplace_back(entry{ amask, numask, shift, u8(i), u8(m_access_endian == ENDIANNESS_BIG ? active_count - 1 - offset : offset) });
		}
		if(gumask & numask)
			offset ++;
	}
}

template class memory_units_descriptor<1,  3>;
template class memory_units_descriptor<1,  0>;
template class memory_units_descriptor<1, -1>;
template class memory_units_descriptor<2,  3>;
template class memory_units_descriptor<2,  0>;
template class memory_units_descriptor<2, -1>;
template class memory_units_descriptor<2, -2>;
template class memory_units_descriptor<3,  0>;
template class memory_units_descriptor<3, -1>;
template class memory_units_descriptor<3, -2>;
template class memory_units_descriptor<3, -3>;
