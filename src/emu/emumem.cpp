// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Olivier Galibert
/***************************************************************************

    emumem.cpp

    Functions which handle device memory access.

***************************************************************************/

#include "emu.h"
#include <list>
#include <map>
#include "emuopts.h"
#include "debug/debugcpu.h"

#include "emumem_mud.h"
#include "emumem_hea.h"
#include "emumem_hem.h"
#include "emumem_hedp.h"
#include "emumem_heun.h"
#include "emumem_heu.h"
#include "emumem_hedr.h"
#include "emumem_hedw.h"
#include "emumem_hep.h"
#include "emumem_het.h"


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE 0

#if VERBOSE
template <typename Format, typename... Params> static void VPRINTF(Format &&fmt, Params &&...args)
{
	util::stream_format(std::cerr, std::forward<Format>(fmt), std::forward<Params>(args)...);
}
#else
template <typename Format, typename... Params> static void VPRINTF(Format &&, Params &&...) {}
#endif

#define VALIDATE_REFCOUNTS 0

offs_t handler_entry::dispatch_entry(offs_t address) const
{
	fatalerror("dispatch_entry called on non-dispatching class\n");
}

void handler_entry::dump_map(std::vector<memory_entry> &map) const
{
	fatalerror("dump_map called on non-dispatching class\n");
}

void handler_entry::select_a(int slot)
{
	fatalerror("select_a called on non-view\n");
}

void handler_entry::select_u(int slot)
{
	fatalerror("select_u called on non-view\n");
}

void handler_entry::reflist::add(const handler_entry *entry)
{
	refcounts[entry]++;
	if(seen.find(entry) == seen.end()) {
		seen.insert(entry);
		todo.insert(entry);
	}
}

void handler_entry::reflist::propagate()
{
	while(!todo.empty()) {
		const handler_entry *entry = *todo.begin();
		todo.erase(todo.begin());
		entry->enumerate_references(*this);
	}
}

void handler_entry::reflist::check()
{
	bool bad = false;
	for(const auto &i : refcounts) {
		if(i.first->get_refcount() != i.second) {
			fprintf(stderr, "Reference count error on handler \"%s\" stored %u real %u.\n",
					i.first->name().c_str(), i.first->get_refcount(), i.second);
			bad = true;
		}
	}
	if(bad)
		abort();
}


// default handler methods

void handler_entry::enumerate_references(handler_entry::reflist &refs) const
{
}

template<int Width, int AddrShift> const handler_entry_read<Width, AddrShift> *const *handler_entry_read<Width, AddrShift>::get_dispatch() const
{
	fatalerror("get_dispatch called on non-dispatching class\n");
}

template<int Width, int AddrShift> void handler_entry_read<Width, AddrShift>::populate_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_read<Width, AddrShift> *handler)
{
	fatalerror("populate called on non-dispatching class\n");
}

template<int Width, int AddrShift> void handler_entry_read<Width, AddrShift>::populate_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_read<Width, AddrShift> *handler)
{
	fatalerror("populate called on non-dispatching class\n");
}

template<int Width, int AddrShift> void handler_entry_read<Width, AddrShift>::populate_mismatched_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, const memory_units_descriptor<Width, AddrShift> &descriptor, u8 rkey, std::vector<mapping> &mappings)
{
	fatalerror("populate_mismatched called on non-dispatching class\n");
}

template<int Width, int AddrShift> void handler_entry_read<Width, AddrShift>::populate_mismatched_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, const memory_units_descriptor<Width, AddrShift> &descriptor, std::vector<mapping> &mappings)
{
	fatalerror("populate_mismatched called on non-dispatching class\n");
}

template<int Width, int AddrShift> void handler_entry_read<Width, AddrShift>::populate_passthrough_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_read_passthrough<Width, AddrShift> *handler, std::vector<mapping> &mappings)
{
	fatalerror("populate_passthrough called on non-dispatching class\n");
}

template<int Width, int AddrShift> void handler_entry_read<Width, AddrShift>::populate_passthrough_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_read_passthrough<Width, AddrShift> *handler, std::vector<mapping> &mappings)
{
	fatalerror("populate_passthrough called on non-dispatching class\n");
}

template<int Width, int AddrShift> void handler_entry_read<Width, AddrShift>::lookup(offs_t address, offs_t &start, offs_t &end, handler_entry_read<Width, AddrShift> *&handler) const
{
	fatalerror("lookup called on non-dispatching class\n");
}

template<int Width, int AddrShift> void *handler_entry_read<Width, AddrShift>::get_ptr(offs_t offset) const
{
	return nullptr;
}

template<int Width, int AddrShift> handler_entry_read<Width, AddrShift> *handler_entry_read<Width, AddrShift>::dup()
{
	ref();
	return this;
}

template<int Width, int AddrShift> void handler_entry_read<Width, AddrShift>::detach(const std::unordered_set<handler_entry *> &handlers)
{
	fatalerror("detach called on non-dispatching class\n");
}

template<int Width, int AddrShift> void handler_entry_read<Width, AddrShift>::init_handlers(offs_t start_entry, offs_t end_entry, u32 lowbits, offs_t ostart, offs_t oend, handler_entry_read<Width, AddrShift> **dispatch, handler_entry::range *ranges)
{
	fatalerror("init_handlers called on non-view class\n");
}


template<int Width, int AddrShift> const handler_entry_write<Width, AddrShift> *const *handler_entry_write<Width, AddrShift>::get_dispatch() const
{
	fatalerror("get_dispatch called on non-dispatching class\n");
}

template<int Width, int AddrShift> void handler_entry_write<Width, AddrShift>::populate_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_write<Width, AddrShift> *handler)
{
	fatalerror("populate called on non-dispatching class\n");
}

template<int Width, int AddrShift> void handler_entry_write<Width, AddrShift>::populate_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_write<Width, AddrShift> *handler)
{
	fatalerror("populate called on non-dispatching class\n");
}

template<int Width, int AddrShift> void handler_entry_write<Width, AddrShift>::populate_mismatched_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, const memory_units_descriptor<Width, AddrShift> &descriptor, u8 rkey, std::vector<mapping> &mappings)
{
	fatalerror("populate_mismatched called on non-dispatching class\n");
}

template<int Width, int AddrShift> void handler_entry_write<Width, AddrShift>::populate_mismatched_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, const memory_units_descriptor<Width, AddrShift> &descriptor, std::vector<mapping> &mappings)
{
	fatalerror("populate_mismatched called on non-dispatching class\n");
}

template<int Width, int AddrShift> void handler_entry_write<Width, AddrShift>::populate_passthrough_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_write_passthrough<Width, AddrShift> *handler, std::vector<mapping> &mappings)
{
	fatalerror("populate_passthrough called on non-dispatching class\n");
}

template<int Width, int AddrShift> void handler_entry_write<Width, AddrShift>::populate_passthrough_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_write_passthrough<Width, AddrShift> *handler, std::vector<mapping> &mappings)
{
	fatalerror("populate_passthrough called on non-dispatching class\n");
}

template<int Width, int AddrShift> void handler_entry_write<Width, AddrShift>::lookup(offs_t address, offs_t &start, offs_t &end, handler_entry_write<Width, AddrShift> *&handler) const
{
	fatalerror("lookup called on non-dispatching class\n");
}

template<int Width, int AddrShift> void *handler_entry_write<Width, AddrShift>::get_ptr(offs_t offset) const
{
	return nullptr;
}

template<int Width, int AddrShift> handler_entry_write<Width, AddrShift> *handler_entry_write<Width, AddrShift>::dup()
{
	ref();
	return this;
}

template<int Width, int AddrShift> void handler_entry_write<Width, AddrShift>::detach(const std::unordered_set<handler_entry *> &handlers)
{
	fatalerror("detach called on non-dispatching class\n");
}

template<int Width, int AddrShift> void handler_entry_write<Width, AddrShift>::init_handlers(offs_t start_entry, offs_t end_entry, u32 lowbits, offs_t ostart, offs_t oend, handler_entry_write<Width, AddrShift> **dispatch, handler_entry::range *ranges)
{
	fatalerror("init_handlers called on non-view class\n");
}

template class handler_entry_read<0,  1>;
template class handler_entry_read<0,  0>;
template class handler_entry_read<1,  3>;
template class handler_entry_read<1,  0>;
template class handler_entry_read<1, -1>;
template class handler_entry_read<2,  3>;
template class handler_entry_read<2,  0>;
template class handler_entry_read<2, -1>;
template class handler_entry_read<2, -2>;
template class handler_entry_read<3,  0>;
template class handler_entry_read<3, -1>;
template class handler_entry_read<3, -2>;
template class handler_entry_read<3, -3>;

template class handler_entry_write<0,  1>;
template class handler_entry_write<0,  0>;
template class handler_entry_write<1,  3>;
template class handler_entry_write<1,  0>;
template class handler_entry_write<1, -1>;
template class handler_entry_write<2,  3>;
template class handler_entry_write<2,  0>;
template class handler_entry_write<2, -1>;
template class handler_entry_write<2, -2>;
template class handler_entry_write<3,  0>;
template class handler_entry_write<3, -1>;
template class handler_entry_write<3, -2>;
template class handler_entry_write<3, -3>;

//**************************************************************************
//  MEMORY MANAGER
//**************************************************************************

//-------------------------------------------------
//  memory_manager - constructor
//-------------------------------------------------

memory_manager::memory_manager(running_machine &machine)
	: m_machine(machine)
{
}

//-------------------------------------------------
//  ~memory_manager - free the allocated memory banks
//-------------------------------------------------

memory_manager::~memory_manager()
{
}

//-------------------------------------------------
//  initialize - initialize the memory system
//-------------------------------------------------

void memory_manager::initialize()
{
	// loop over devices and spaces within each device
	std::vector<device_memory_interface *> memories;
	for (device_memory_interface &memory : memory_interface_enumerator(machine().root_device()))
	{
		memories.push_back(&memory);
		allocate(memory);
	}

	// construct and preprocess the address_map for each space
	for (auto const memory : memories)
		memory->prepare_maps();

	// create the handlers from the resulting address maps
	for (auto const memory : memories)
		memory->populate_from_maps();

	// disable logging of unmapped access when no one receives it
	if (!machine().options().log() && !machine().options().oslog() && !(machine().debug_flags & DEBUG_FLAG_ENABLED))
		for (auto const memory : memories)
			memory->set_log_unmap(false);
}


//-------------------------------------------------
//  allocate_memory - allocate some ram and register it for saving
//-------------------------------------------------

void *memory_manager::allocate_memory(device_t &dev, int spacenum, std::string name, u8 width, size_t bytes)
{
	void *const ptr = m_datablocks.emplace_back(malloc(bytes)).get();
	memset(ptr, 0, bytes);
	machine().save().save_memory(&dev, "memory", dev.tag(), spacenum, name.c_str(), ptr, width/8, u32(bytes) / (width/8));
	return ptr;
}



//-------------------------------------------------
//  region_alloc - allocates memory for a region
//-------------------------------------------------

memory_region *memory_manager::region_alloc(std::string name, u32 length, u8 width, endianness_t endian)
{
	// make sure we don't have a region of the same name; also find the end of the list
	if (m_regionlist.find(name) != m_regionlist.end())
		fatalerror("region_alloc called with duplicate region name \"%s\"\n", name);

	// allocate the region
	return m_regionlist.emplace(name, std::make_unique<memory_region>(machine(), name, length, width, endian)).first->second.get();
}


//-------------------------------------------------
//  region_find - find a region by name
//-------------------------------------------------

memory_region *memory_manager::region_find(std::string name)
{
	auto i = m_regionlist.find(name);
	return i != m_regionlist.end() ? i->second.get() : nullptr;
}


//-------------------------------------------------
//  region_free - releases memory for a region
//-------------------------------------------------

void memory_manager::region_free(std::string name)
{
	m_regionlist.erase(name);
}


//-------------------------------------------------
//  anonymous_alloc - allocates a anonymous memory zone
//-------------------------------------------------

void *memory_manager::anonymous_alloc(address_space &space, size_t bytes, u8 width, offs_t start, offs_t end, const std::string &key)
{
	std::string name = util::string_format("%s%x-%x", key, start, end);
	return allocate_memory(space.device(), space.spacenum(), name, width, bytes);
}


//-------------------------------------------------
//  share_alloc - allocates a shared memory zone
//-------------------------------------------------

memory_share *memory_manager::share_alloc(device_t &dev, std::string name, u8 width, size_t bytes, endianness_t endianness)
{
	// make sure we don't have a share of the same name; also find the end of the list
	if (m_sharelist.find(name) != m_sharelist.end())
		fatalerror("share_alloc called with duplicate share name \"%s\"\n", name);

	// allocate and register the memory
	void *ptr = allocate_memory(dev, 0, name, width, bytes);

	// allocate the region
	return m_sharelist.emplace(name, std::make_unique<memory_share>(name, width, bytes, endianness, ptr)).first->second.get();
}


//-------------------------------------------------
//  share_find - find a share by name
//-------------------------------------------------

memory_share *memory_manager::share_find(std::string name)
{
	auto i = m_sharelist.find(name);
	return i != m_sharelist.end() ? i->second.get() : nullptr;
}



//-------------------------------------------------
//  share_alloc - allocates a banking zone
//-------------------------------------------------

memory_bank *memory_manager::bank_alloc(device_t &device, std::string name)
{
	// allocate the bank
	auto const ins = m_banklist.emplace(name, std::make_unique<memory_bank>(device, name));

	// make sure we don't have a bank of the same name
	if (!ins.second)
		fatalerror("bank_alloc called with duplicate bank name \"%s\"\n", name);

	return ins.first->second.get();
}


//-------------------------------------------------
//  bank_find - find a bank by name
//-------------------------------------------------

memory_bank *memory_manager::bank_find(std::string name)
{
	auto i = m_banklist.find(name);
	return i != m_banklist.end() ? i->second.get() : nullptr;
}


//**************************************************************************
//  ADDRESS SPACE CONFIG
//**************************************************************************

//-------------------------------------------------
//  address_space_config - constructors
//-------------------------------------------------

address_space_config::address_space_config()
	: m_name("unknown"),
		m_endianness(ENDIANNESS_NATIVE),
		m_data_width(0),
		m_addr_width(0),
		m_addr_shift(0),
		m_logaddr_width(0),
		m_page_shift(0),
		m_is_octal(false),
		m_internal_map(address_map_constructor())
{
}

/*!
 @param name
 @param endian CPU endianness
 @param datawidth CPU parallelism bits
 @param addrwidth address bits
 @param addrshift
 @param internal
 */
address_space_config::address_space_config(const char *name, endianness_t endian, u8 datawidth, u8 addrwidth, s8 addrshift, address_map_constructor internal)
	: m_name(name),
		m_endianness(endian),
		m_data_width(datawidth),
		m_addr_width(addrwidth),
		m_addr_shift(addrshift),
		m_logaddr_width(addrwidth),
		m_page_shift(0),
		m_is_octal(false),
		m_internal_map(internal)
{
}

address_space_config::address_space_config(const char *name, endianness_t endian, u8 datawidth, u8 addrwidth, s8 addrshift, u8 logwidth, u8 pageshift, address_map_constructor internal)
	: m_name(name),
		m_endianness(endian),
		m_data_width(datawidth),
		m_addr_width(addrwidth),
		m_addr_shift(addrshift),
		m_logaddr_width(logwidth),
		m_page_shift(pageshift),
		m_is_octal(false),
		m_internal_map(internal)
{
}


void address_space_installer::check_optimize_all(const char *function, int width, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth, offs_t &nstart, offs_t &nend, offs_t &nmask, offs_t &nmirror, u64 &nunitmask, int &ncswidth)
{
	if (addrstart > addrend)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, start address is after the end address.\n", function, addrstart, addrend, addrmask, addrmirror, addrselect);
	if (addrstart & ~m_addrmask)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, start address is outside of the global address mask %x, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, m_addrmask, addrstart & m_addrmask);
	if (addrend & ~m_addrmask)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, end address is outside of the global address mask %x, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, m_addrmask, addrend & m_addrmask);

	// Check the relative data widths
	if (width > m_config.data_width())
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, cannot install a %d-bits wide handler in a %d-bits wide address space.\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, width, m_config.data_width());

	// Check the validity of the addresses given their intrinsic width
	// We assume that busses with non-zero address shift have a data width matching the shift (reality says yes)
	offs_t default_lowbits_mask = (m_config.data_width() >> (3 - m_config.addr_shift())) - 1;
	offs_t lowbits_mask = width && !m_config.addr_shift() ? (width >> 3) - 1 : default_lowbits_mask;

	if (addrstart & lowbits_mask)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, start address has low bits set, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, addrstart & ~lowbits_mask);
	if ((~addrend) & lowbits_mask)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, end address has low bits unset, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, addrend | lowbits_mask);

	offs_t set_bits = addrstart | addrend;
	offs_t changing_bits = addrstart ^ addrend;
	// Round up to the nearest power-of-two-minus-one
	changing_bits |= changing_bits >> 1;
	changing_bits |= changing_bits >> 2;
	changing_bits |= changing_bits >> 4;
	changing_bits |= changing_bits >> 8;
	changing_bits |= changing_bits >> 16;

	if (addrmask & ~m_addrmask)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, mask is outside of the global address mask %x, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, m_addrmask, addrmask & m_addrmask);
	if (addrselect & ~m_addrmask)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, select is outside of the global address mask %x, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, m_addrmask, addrselect & m_addrmask);
	if (addrmask & ~changing_bits)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, mask is trying to unmask an unchanging address bit, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, addrmask & changing_bits);
	if (addrmirror & changing_bits)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, mirror touches a changing address bit, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, addrmirror & ~changing_bits);
	if (addrselect & changing_bits)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, select touches a changing address bit, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, addrselect & ~changing_bits);
	if (addrmirror & set_bits)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, mirror touches a set address bit, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, addrmirror & ~set_bits);
	if (addrselect & set_bits)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, select touches a set address bit, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, addrselect & ~set_bits);
	if (addrmirror & addrselect)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, mirror touches a select bit, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, addrmirror & ~addrselect);

	// Check the cswidth, if provided
	if (cswidth > m_config.data_width())
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, the cswidth of %d is too large for a %d-bit space.\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, cswidth, m_config.data_width());
	if (width && (cswidth % width) != 0)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, the cswidth of %d is not a multiple of handler size %d.\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, cswidth, width);
	ncswidth = cswidth ? cswidth : width;

	// Check if the unitmask is structurally correct for the width
	// Not sure what we can actually handle regularity-wise, so don't check that yet
	if (width) {
		// Check if the 1-blocks are of appropriate size
		u64 block_mask = 0xffffffffffffffffU >> (64 - width);
		u64 cs_mask = 0xffffffffffffffffU >> (64 - ncswidth);
		for(int pos = 0; pos < 64; pos += ncswidth) {
			u64 cmask = (unitmask >> pos) & cs_mask;
			while (cmask != 0 && (cmask & block_mask) == 0)
				cmask >>= width;
			if (cmask != 0 && cmask != block_mask)
				fatalerror("%s: In range %x-%x mask %x mirror %x select %x, the unitmask of %016x has incorrect granularity for %d-bit chip selection.\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth);
		}
	}

	nunitmask = 0xffffffffffffffffU >> (64 - m_config.data_width());
	if (unitmask)
		nunitmask &= unitmask;

	nstart = addrstart;
	nend = addrend;
	nmask = (addrmask ? addrmask : changing_bits) | addrselect;
	nmirror = (addrmirror & m_addrmask) | addrselect;

	if(nmirror & default_lowbits_mask) {
		// If the mirroring/select "reaches" within the bus
		// granularity we have to adapt it and the unitmask.

		// We're sure start/end are on the same data-width-sized
		// entry, because otherwise the previous tests wouldn't have
		// passed.  So we need to clear the part of the unitmask that
		// not in the range, then replicate it following the mirror.
		// The start/end also need to be adjusted to the bus
		// granularity.

		// 1. Adjusting
		nstart &= ~default_lowbits_mask;
		nend |= default_lowbits_mask;

		// 2. Clearing
		u64 smask, emask;
		if(m_config.endianness() == ENDIANNESS_BIG) {
			smask =  make_bitmask<u64>(m_config.data_width() - ((addrstart - nstart) << (3 - m_config.addr_shift())));
			emask = ~make_bitmask<u64>(m_config.data_width() - ((addrend - nstart + 1) << (3 - m_config.addr_shift())));
		} else {
			smask = ~make_bitmask<u64>((addrstart - nstart) << (3 - m_config.addr_shift()));
			emask =  make_bitmask<u64>((addrend - nstart + 1) << (3 - m_config.addr_shift()));
		}
		nunitmask &= smask & emask;

		// 3. Mirroring
		offs_t to_mirror = nmirror & default_lowbits_mask;
		if(m_config.endianness() == ENDIANNESS_BIG) {
			for(int i=0; to_mirror; i++)
				if((to_mirror >> i) & 1) {
					to_mirror &= ~(1 << i);
					nunitmask |= nunitmask >> (1 << (3 + i - m_config.addr_shift()));
				}
		} else {
			for(int i=0; to_mirror; i++)
				if((to_mirror >> i) & 1) {
					to_mirror &= ~(1 << i);
					nunitmask |= nunitmask << (1 << (3 + i - m_config.addr_shift()));
				}
		}

		// 4. Ajusting the mirror
		nmirror &= ~default_lowbits_mask;

		// 5. Recompute changing_bits, it matters for the next optimization.  No need to round up through
		changing_bits = nstart ^ nend;
	}

	if(nmirror && !(nstart & changing_bits) && !((~nend) & changing_bits)) {
		// If the range covers the a complete power-of-two zone, it is
		// possible to remove 1 bits from the mirror, pushing the end
		// address.  The mask will clamp, and installing the range
		// will be faster.
		while(nmirror & (changing_bits+1)) {
			offs_t bit = nmirror & (changing_bits+1);
			nmirror &= ~bit;
			nend |= bit;
			changing_bits |= bit;
		}
	}
}

void address_space_installer::check_optimize_mirror(const char *function, offs_t addrstart, offs_t addrend, offs_t addrmirror, offs_t &nstart, offs_t &nend, offs_t &nmask, offs_t &nmirror)
{
	if (addrstart > addrend)
		fatalerror("%s: In range %x-%x mirror %x, start address is after the end address.\n", function, addrstart, addrend, addrmirror);
	if (addrstart & ~m_addrmask)
		fatalerror("%s: In range %x-%x mirror %x, start address is outside of the global address mask %x, did you mean %x ?\n", function, addrstart, addrend, addrmirror, m_addrmask, addrstart & m_addrmask);
	if (addrend & ~m_addrmask)
		fatalerror("%s: In range %x-%x mirror %x, end address is outside of the global address mask %x, did you mean %x ?\n", function, addrstart, addrend, addrmirror, m_addrmask, addrend & m_addrmask);

	offs_t lowbits_mask = (m_config.data_width() >> (3 - m_config.addr_shift())) - 1;
	if (addrstart & lowbits_mask)
		fatalerror("%s: In range %x-%x mirror %x, start address has low bits set, did you mean %x ?\n", function, addrstart, addrend, addrmirror, addrstart & ~lowbits_mask);
	if ((~addrend) & lowbits_mask)
		fatalerror("%s: In range %x-%x mirror %x, end address has low bits unset, did you mean %x ?\n", function, addrstart, addrend, addrmirror, addrend | lowbits_mask);

	offs_t set_bits = addrstart | addrend;
	offs_t changing_bits = addrstart ^ addrend;
	// Round up to the nearest power-of-two-minus-one
	changing_bits |= changing_bits >> 1;
	changing_bits |= changing_bits >> 2;
	changing_bits |= changing_bits >> 4;
	changing_bits |= changing_bits >> 8;
	changing_bits |= changing_bits >> 16;

	if (addrmirror & ~m_addrmask)
		fatalerror("%s: In range %x-%x mirror %x, mirror is outside of the global address mask %x, did you mean %x ?\n", function, addrstart, addrend, addrmirror, m_addrmask, addrmirror & m_addrmask);
	if (addrmirror & changing_bits)
		fatalerror("%s: In range %x-%x mirror %x, mirror touches a changing address bit, did you mean %x ?\n", function, addrstart, addrend, addrmirror, addrmirror & ~changing_bits);
	if (addrmirror & set_bits)
		fatalerror("%s: In range %x-%x mirror %x, mirror touches a set address bit, did you mean %x ?\n", function, addrstart, addrend, addrmirror, addrmirror & ~set_bits);

	nstart = addrstart;
	nend = addrend;
	nmask = changing_bits;
	nmirror = addrmirror;

	if(nmirror && !(nstart & changing_bits) && !((~nend) & changing_bits)) {
		// If the range covers the a complete power-of-two zone, it is
		// possible to remove 1 bits from the mirror, pushing the end
		// address.  The mask will clamp, and installing the range
		// will be faster.
		while(nmirror & (changing_bits+1)) {
			offs_t bit = nmirror & (changing_bits+1);
			nmirror &= ~bit;
			nend |= bit;
			changing_bits |= bit;
		}
	}
}

void address_space_installer::check_address(const char *function, offs_t addrstart, offs_t addrend)
{
	if (addrstart > addrend)
		fatalerror("%s: In range %x-%x, start address is after the end address.\n", function, addrstart, addrend);
	if (addrstart & ~m_addrmask)
		fatalerror("%s: In range %x-%x, start address is outside of the global address mask %x, did you mean %x ?\n", function, addrstart, addrend, m_addrmask, addrstart & m_addrmask);
	if (addrend & ~m_addrmask)
		fatalerror("%s: In range %x-%x, end address is outside of the global address mask %x, did you mean %x ?\n", function, addrstart, addrend, m_addrmask, addrend & m_addrmask);

	offs_t lowbits_mask = (m_config.data_width() >> (3 - m_config.addr_shift())) - 1;
	if (addrstart & lowbits_mask)
		fatalerror("%s: In range %x-%x, start address has low bits set, did you mean %x ?\n", function, addrstart, addrend, addrstart & ~lowbits_mask);
	if ((~addrend) & lowbits_mask)
		fatalerror("%s: In range %x-%x, end address has low bits unset, did you mean %x ?\n", function, addrstart, addrend, addrend | lowbits_mask);
}


//-------------------------------------------------
//  populate_map_entry - map a single read or
//  write entry based on information from an
//  address map entry
//-------------------------------------------------

void address_space_installer::populate_map_entry(const address_map_entry &entry, read_or_write readorwrite)
{
	const map_handler_data &data = (readorwrite == read_or_write::READ) ? entry.m_read : entry.m_write;
	// based on the handler type, alter the bits, name, funcptr, and object
	switch (data.m_type)
	{
		case AMH_NONE:
			return;

		case AMH_ROM:
			// writes to ROM are no-ops
			if (readorwrite == read_or_write::WRITE)
				return;
			// fall through to the RAM case otherwise
			[[fallthrough]];
		case AMH_RAM:
			install_ram_generic(entry.m_addrstart, entry.m_addrend, entry.m_addrmirror, entry.m_flags, readorwrite, entry.m_memory);
			break;

		case AMH_NOP:
			unmap_generic(entry.m_addrstart, entry.m_addrend, entry.m_addrmirror, entry.m_flags, readorwrite, true);
			break;

		case AMH_UNMAP:
			unmap_generic(entry.m_addrstart, entry.m_addrend, entry.m_addrmirror, entry.m_flags, readorwrite, false);
			break;

		case AMH_DEVICE_DELEGATE:
			if (readorwrite == read_or_write::READ)
				switch (data.m_bits)
				{
					case 8:     install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto8,  entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 16:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto16, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 32:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto32, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 64:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto64, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
				}
			else
				switch (data.m_bits)
				{
					case 8:     install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto8,  entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 16:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto16, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 32:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto32, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 64:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto64, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
				}
			break;

		case AMH_DEVICE_DELEGATE_M:
			if (readorwrite == read_or_write::READ)
				switch (data.m_bits)
				{
					case 8:     install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto8m,  entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 16:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto16m, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 32:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto32m, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 64:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto64m, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
				}
			else
				switch (data.m_bits)
				{
					case 8:     install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto8m,  entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 16:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto16m, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 32:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto32m, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 64:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto64m, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
				}
			break;

		case AMH_DEVICE_DELEGATE_S:
			if (readorwrite == read_or_write::READ)
				switch (data.m_bits)
				{
					case 8:     install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto8s,  entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 16:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto16s, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 32:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto32s, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 64:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto64s, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
				}
			else
				switch (data.m_bits)
				{
					case 8:     install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto8s,  entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 16:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto16s, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 32:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto32s, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 64:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto64s, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
				}
			break;

		case AMH_DEVICE_DELEGATE_SM:
			if (readorwrite == read_or_write::READ)
				switch (data.m_bits)
				{
					case 8:     install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto8sm,  entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 16:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto16sm, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 32:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto32sm, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 64:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto64sm, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
				}
			else
				switch (data.m_bits)
				{
					case 8:     install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto8sm,  entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 16:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto16sm, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 32:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto32sm, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 64:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto64sm, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
				}
			break;

		case AMH_DEVICE_DELEGATE_MO:
			if (readorwrite == read_or_write::READ)
				switch (data.m_bits)
				{
					case 8:     install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto8mo,  entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 16:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto16mo, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 32:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto32mo, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 64:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto64mo, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
				}
			else
				switch (data.m_bits)
				{
					case 8:     install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto8mo,  entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 16:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto16mo, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 32:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto32mo, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 64:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto64mo, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
				}
			break;

		case AMH_DEVICE_DELEGATE_SMO:
			if (readorwrite == read_or_write::READ)
				switch (data.m_bits)
				{
					case 8:     install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto8smo,  entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 16:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto16smo, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 32:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto32smo, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 64:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto64smo, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
				}
			else
				switch (data.m_bits)
				{
					case 8:     install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto8smo,  entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 16:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto16smo, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 32:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto32smo, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
					case 64:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto64smo, entry.m_mask, entry.m_cswidth, entry.m_flags); break;
				}
			break;

		case AMH_PORT:
			install_readwrite_port(entry.m_addrstart, entry.m_addrend, entry.m_addrmirror, entry.m_flags,
								   (readorwrite == read_or_write::READ) ? entry.m_devbase.subtag(data.m_tag) : "",
								   (readorwrite == read_or_write::WRITE) ? entry.m_devbase.subtag(data.m_tag) : "");
			break;

		case AMH_BANK:
			{
				std::string tag = entry.m_devbase.subtag(data.m_tag);
				memory_bank *bank = m_manager.bank_find(tag);
				if (!bank)
					bank = m_manager.bank_alloc(entry.m_devbase, tag);
				install_bank_generic(entry.m_addrstart, entry.m_addrend, entry.m_addrmirror, entry.m_flags,
									 (readorwrite == read_or_write::READ) ? bank : nullptr,
									 (readorwrite == read_or_write::WRITE) ? bank : nullptr);
			}
			break;

		case AMH_DEVICE_SUBMAP:
			throw emu_fatalerror("Internal mapping error: leftover mapping of '%s'.\n", data.m_tag);

		case AMH_VIEW:
			if (readorwrite == read_or_write::READ)
				install_view(entry.m_addrstart, entry.m_addrend, entry.m_addrmirror, *entry.m_view);
			break;
	}
}



memory_passthrough_handler address_space_installer::install_read_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u8  &data, u8  mem_mask)> tap, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 8-bits wide bus read tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler address_space_installer::install_read_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u16 &data, u16 mem_mask)> tap, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 16-bits wide bus read tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler address_space_installer::install_read_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u32 &data, u32 mem_mask)> tap, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 32-bits wide bus read tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler address_space_installer::install_read_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u64 &data, u64 mem_mask)> tap, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 64-bits wide bus read tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler address_space_installer::install_write_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u8  &data, u8  mem_mask)> tap, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 8-bits wide bus write tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler address_space_installer::install_write_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u16 &data, u16 mem_mask)> tap, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 16-bits wide bus write tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler address_space_installer::install_write_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u32 &data, u32 mem_mask)> tap, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 32-bits wide bus write tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler address_space_installer::install_write_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u64 &data, u64 mem_mask)> tap, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 64-bits wide bus write tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler address_space_installer::install_readwrite_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u8  &data, u8  mem_mask)> tapr, std::function<void (offs_t offset, u8  &data, u8  mem_mask)> tapw, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 8-bits wide bus read/write tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler address_space_installer::install_readwrite_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u16 &data, u16 mem_mask)> tapr, std::function<void (offs_t offset, u16 &data, u16 mem_mask)> tapw, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 16-bits wide bus read/write tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler address_space_installer::install_readwrite_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u32 &data, u32 mem_mask)> tapr, std::function<void (offs_t offset, u32 &data, u32 mem_mask)> tap, memory_passthrough_handler *mph)
{
	fatalerror("Tryingw to install a 32-bits wide bus read/write tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler address_space_installer::install_readwrite_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u64 &data, u64 mem_mask)> tapr, std::function<void (offs_t offset, u64 &data, u64 mem_mask)> tapw, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 64-bits wide bus read/write tap in a %d-bits wide bus\n", data_width());
}


//**************************************************************************
//  MEMORY BANK
//**************************************************************************

//-------------------------------------------------
//  memory_bank - constructor
//-------------------------------------------------

memory_bank::memory_bank(device_t &device, std::string tag)
	: m_machine(device.machine()),
	  m_curentry(0)
{
	m_tag = std::move(tag);
	m_name = string_format("Bank '%s'", m_tag);
	machine().save().save_item(&device, "memory", m_tag.c_str(), 0, NAME(m_curentry));
}


//-------------------------------------------------
//  memory_bank - destructor
//-------------------------------------------------

memory_bank::~memory_bank()
{
}


//-------------------------------------------------
//  set_base - set the bank base explicitly
//-------------------------------------------------

void memory_bank::set_base(void *base)
{
	// nullptr is not an option
	if (base == nullptr)
		throw emu_fatalerror("memory_bank::set_base called nullptr base");

	// set the base
	if(m_entries.empty()) {
		m_entries.resize(1);
		m_curentry = 0;
	}
	m_entries[m_curentry] = reinterpret_cast<u8 *>(base);
}


//-------------------------------------------------
//  set_entry - set the base to a pre-configured
//  entry
//-------------------------------------------------

void memory_bank::set_entry(int entrynum)
{
	if(entrynum == -1 && m_entries.empty())
		return;

	// validate
	if (entrynum < 0 || entrynum >= int(m_entries.size()))
		throw emu_fatalerror("memory_bank::set_entry called with out-of-range entry %d", entrynum);
	if (m_entries[entrynum] == nullptr)
		throw emu_fatalerror("memory_bank::set_entry called for bank '%s' with invalid bank entry %d", m_tag, entrynum);

	m_curentry = entrynum;
}


//-------------------------------------------------
//  configure_entry - configure an entry
//-------------------------------------------------

void memory_bank::configure_entry(int entrynum, void *base)
{
	// must be positive
	if (entrynum < 0)
		throw emu_fatalerror("memory_bank::configure_entry called with out-of-range entry %d", entrynum);

	// if we haven't allocated this many entries yet, expand our array
	if (entrynum >= int(m_entries.size()))
		m_entries.resize(entrynum+1);

	// set the entry
	m_entries[entrynum] = reinterpret_cast<u8 *>(base);
}


//-------------------------------------------------
//  configure_entries - configure multiple entries
//-------------------------------------------------

void memory_bank::configure_entries(int startentry, int numentries, void *base, offs_t stride)
{
	if (startentry + numentries >= int(m_entries.size()))
		m_entries.resize(startentry + numentries+1);

	// fill in the requested bank entries
	for (int entrynum = 0; entrynum < numentries; entrynum ++)
		m_entries[entrynum + startentry] = reinterpret_cast<u8 *>(base) +  entrynum * stride;
}


//**************************************************************************
//  MEMORY REGIONS
//**************************************************************************

//-------------------------------------------------
//  memory_region - constructor
//-------------------------------------------------

memory_region::memory_region(running_machine &machine, std::string name, u32 length, u8 width, endianness_t endian)
	: m_machine(machine),
		m_name(std::move(name)),
		m_buffer(length),
		m_endianness(endian),
		m_bitwidth(width * 8),
		m_bytewidth(width)
{
	assert(width == 1 || width == 2 || width == 4 || width == 8);
}

std::string memory_share::compare(u8 width, size_t bytes, endianness_t endianness) const
{
	if (width != m_bitwidth)
		return util::string_format("share %s found with unexpected width (expected %d, found %d)", m_name, width, m_bitwidth);
	if (bytes != m_bytes)
		return util::string_format("share %s found with unexpected size (expected %x, found %x)", m_name, bytes, m_bytes);
	if (endianness != m_endianness && m_bitwidth != 8)
		return util::string_format("share %s found with unexpected endianness (expected %s, found %s)", m_name,
								   endianness == ENDIANNESS_LITTLE ? "little" : "big",
								   m_endianness == ENDIANNESS_LITTLE ? "little" : "big");
	return "";
}
