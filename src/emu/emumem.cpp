// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Olivier Galibert
/***************************************************************************

    emumem.c

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

#define VERBOSE         (0)

#define VPRINTF(x)  do { if (VERBOSE) printf x; } while (0)

#define VALIDATE_REFCOUNTS 0

void handler_entry::dump_map(std::vector<memory_entry> &map) const
{
	fatalerror("dump_map called on non-dispatching class\n");
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

template<int Width, int AddrShift, int Endian> void handler_entry_read<Width, AddrShift, Endian>::populate_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_read<Width, AddrShift, Endian> *handler)
{
	fatalerror("populate called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_read<Width, AddrShift, Endian>::populate_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_read<Width, AddrShift, Endian> *handler)
{
	fatalerror("populate called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_read<Width, AddrShift, Endian>::populate_mismatched_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, u8 rkey, std::vector<mapping> &mappings)
{
	fatalerror("populate_mismatched called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_read<Width, AddrShift, Endian>::populate_mismatched_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, std::vector<mapping> &mappings)
{
	fatalerror("populate_mismatched called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_read<Width, AddrShift, Endian>::populate_passthrough_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_read_passthrough<Width, AddrShift, Endian> *handler, std::vector<mapping> &mappings)
{
	fatalerror("populate_passthrough called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_read<Width, AddrShift, Endian>::populate_passthrough_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_read_passthrough<Width, AddrShift, Endian> *handler, std::vector<mapping> &mappings)
{
	fatalerror("populate_passthrough called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_read<Width, AddrShift, Endian>::lookup(offs_t address, offs_t &start, offs_t &end, handler_entry_read<Width, AddrShift, Endian> *&handler) const
{
	fatalerror("lookup called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void *handler_entry_read<Width, AddrShift, Endian>::get_ptr(offs_t offset) const
{
	return nullptr;
}

template<int Width, int AddrShift, int Endian> void handler_entry_read<Width, AddrShift, Endian>::detach(const std::unordered_set<handler_entry *> &handlers)
{
	fatalerror("detach called on non-dispatching class\n");
}


template<int Width, int AddrShift, int Endian> void handler_entry_write<Width, AddrShift, Endian>::populate_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_write<Width, AddrShift, Endian> *handler)
{
	fatalerror("populate called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_write<Width, AddrShift, Endian>::populate_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_write<Width, AddrShift, Endian> *handler)
{
	fatalerror("populate called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_write<Width, AddrShift, Endian>::populate_mismatched_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, u8 rkey, std::vector<mapping> &mappings)
{
	fatalerror("populate_mismatched called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_write<Width, AddrShift, Endian>::populate_mismatched_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, std::vector<mapping> &mappings)
{
	fatalerror("populate_mismatched called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_write<Width, AddrShift, Endian>::populate_passthrough_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_write_passthrough<Width, AddrShift, Endian> *handler, std::vector<mapping> &mappings)
{
	fatalerror("populate_passthrough called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_write<Width, AddrShift, Endian>::populate_passthrough_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_write_passthrough<Width, AddrShift, Endian> *handler, std::vector<mapping> &mappings)
{
	fatalerror("populate_passthrough called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_write<Width, AddrShift, Endian>::lookup(offs_t address, offs_t &start, offs_t &end, handler_entry_write<Width, AddrShift, Endian> *&handler) const
{
	fatalerror("lookup called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void *handler_entry_write<Width, AddrShift, Endian>::get_ptr(offs_t offset) const
{
	return nullptr;
}

template<int Width, int AddrShift, int Endian> void handler_entry_write<Width, AddrShift, Endian>::detach(const std::unordered_set<handler_entry *> &handlers)
{
	fatalerror("detach called on non-dispatching class\n");
}



/*-------------------------------------------------
    core_i64_hex_format - i64 format printf helper
-------------------------------------------------*/

static char *core_i64_hex_format(u64 value, u8 mindigits)
{
	static char buffer[16][64];
	// TODO: this can overflow - e.g. when a lot of unmapped writes are logged
	static int index;
	char *bufbase = &buffer[index++ % 16][0];
	char *bufptr = bufbase;
	s8 curdigit;

	for (curdigit = 15; curdigit >= 0; curdigit--)
	{
		int nibble = (value >> (curdigit * 4)) & 0xf;
		if (nibble != 0 || curdigit < mindigits)
		{
			mindigits = curdigit;
			*bufptr++ = "0123456789ABCDEF"[nibble];
		}
	}
	if (bufptr == bufbase)
		*bufptr++ = '0';
	*bufptr = 0;

	return bufbase;
}



//**************************************************************************
//  CONSTANTS
//**************************************************************************

namespace {

template <typename Delegate> struct handler_width;
template <> struct handler_width<read8_delegate> { static constexpr int value = 0; };
template <> struct handler_width<read8m_delegate> { static constexpr int value = 0; };
template <> struct handler_width<read8s_delegate> { static constexpr int value = 0; };
template <> struct handler_width<read8sm_delegate> { static constexpr int value = 0; };
template <> struct handler_width<read8mo_delegate> { static constexpr int value = 0; };
template <> struct handler_width<read8smo_delegate> { static constexpr int value = 0; };
template <> struct handler_width<write8_delegate> { static constexpr int value = 0; };
template <> struct handler_width<write8m_delegate> { static constexpr int value = 0; };
template <> struct handler_width<write8s_delegate> { static constexpr int value = 0; };
template <> struct handler_width<write8sm_delegate> { static constexpr int value = 0; };
template <> struct handler_width<write8mo_delegate> { static constexpr int value = 0; };
template <> struct handler_width<write8smo_delegate> { static constexpr int value = 0; };
template <> struct handler_width<read16_delegate> { static constexpr int value = 1; };
template <> struct handler_width<read16m_delegate> { static constexpr int value = 1; };
template <> struct handler_width<read16s_delegate> { static constexpr int value = 1; };
template <> struct handler_width<read16sm_delegate> { static constexpr int value = 1; };
template <> struct handler_width<read16mo_delegate> { static constexpr int value = 1; };
template <> struct handler_width<read16smo_delegate> { static constexpr int value = 1; };
template <> struct handler_width<write16_delegate> { static constexpr int value = 1; };
template <> struct handler_width<write16m_delegate> { static constexpr int value = 1; };
template <> struct handler_width<write16s_delegate> { static constexpr int value = 1; };
template <> struct handler_width<write16sm_delegate> { static constexpr int value = 1; };
template <> struct handler_width<write16mo_delegate> { static constexpr int value = 1; };
template <> struct handler_width<write16smo_delegate> { static constexpr int value = 1; };
template <> struct handler_width<read32_delegate> { static constexpr int value = 2; };
template <> struct handler_width<read32m_delegate> { static constexpr int value = 2; };
template <> struct handler_width<read32s_delegate> { static constexpr int value = 2; };
template <> struct handler_width<read32sm_delegate> { static constexpr int value = 2; };
template <> struct handler_width<read32mo_delegate> { static constexpr int value = 2; };
template <> struct handler_width<read32smo_delegate> { static constexpr int value = 2; };
template <> struct handler_width<write32_delegate> { static constexpr int value = 2; };
template <> struct handler_width<write32m_delegate> { static constexpr int value = 2; };
template <> struct handler_width<write32s_delegate> { static constexpr int value = 2; };
template <> struct handler_width<write32sm_delegate> { static constexpr int value = 2; };
template <> struct handler_width<write32mo_delegate> { static constexpr int value = 2; };
template <> struct handler_width<write32smo_delegate> { static constexpr int value = 2; };
template <> struct handler_width<read64_delegate> { static constexpr int value = 3; };
template <> struct handler_width<read64m_delegate> { static constexpr int value = 3; };
template <> struct handler_width<read64s_delegate> { static constexpr int value = 3; };
template <> struct handler_width<read64sm_delegate> { static constexpr int value = 3; };
template <> struct handler_width<read64mo_delegate> { static constexpr int value = 3; };
template <> struct handler_width<read64smo_delegate> { static constexpr int value = 3; };
template <> struct handler_width<write64_delegate> { static constexpr int value = 3; };
template <> struct handler_width<write64m_delegate> { static constexpr int value = 3; };
template <> struct handler_width<write64s_delegate> { static constexpr int value = 3; };
template <> struct handler_width<write64sm_delegate> { static constexpr int value = 3; };
template <> struct handler_width<write64mo_delegate> { static constexpr int value = 3; };
template <> struct handler_width<write64smo_delegate> { static constexpr int value = 3; };

// other address map constants
const int MEMORY_BLOCK_CHUNK = 65536;                   // minimum chunk size of allocated memory blocks

} // anonymous namespace


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> address_space_specific

// this is a derived class of address_space with specific width, endianness, and table size
template<int Width, int AddrShift, endianness_t Endian>
class address_space_specific : public address_space
{
	using uX = typename emu::detail::handler_entry_size<Width>::uX;
	using NativeType = uX;
	using this_type = address_space_specific<Width, AddrShift, Endian>;

	// constants describing the native size
	static constexpr u32 NATIVE_BYTES = 1 << Width;
	static constexpr u32 NATIVE_STEP = AddrShift >= 0 ? NATIVE_BYTES << iabs(AddrShift) : NATIVE_BYTES >> iabs(AddrShift);
	static constexpr u32 NATIVE_MASK = NATIVE_STEP - 1;
	static constexpr u32 NATIVE_BITS = 8 * NATIVE_BYTES;

	static constexpr offs_t offset_to_byte(offs_t offset) { return AddrShift < 0 ? offset << iabs(AddrShift) : offset >> iabs(AddrShift); }

public:
	std::string get_handler_string(read_or_write readorwrite, offs_t byteaddress) const override;
	void dump_maps(std::vector<memory_entry> &read_map, std::vector<memory_entry> &write_map) const override;

	void unmap_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, read_or_write readorwrite, bool quiet) override;
	void install_ram_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, read_or_write readorwrite, void *baseptr) override;
	void install_bank_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string rtag, std::string wtag) override;
	void install_bank_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, memory_bank *rbank, memory_bank *wbank) override;
	void install_readwrite_port(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string rtag, std::string wtag) override;
	void install_device_delegate(offs_t addrstart, offs_t addrend, device_t &device, address_map_constructor &map, u64 unitmask = 0, int cswidth = 0) override;

	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write8_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8_delegate rhandler, write8_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write16_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16_delegate rhandler, write16_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write32_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32_delegate rhandler, write32_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write64_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64_delegate rhandler, write64_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }

	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8m_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write8m_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8m_delegate rhandler, write8m_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16m_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write16m_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16m_delegate rhandler, write16m_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32m_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write32m_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32m_delegate rhandler, write32m_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64m_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write64m_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64m_delegate rhandler, write64m_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }

	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8s_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write8s_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8s_delegate rhandler, write8s_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16s_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write16s_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16s_delegate rhandler, write16s_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32s_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write32s_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32s_delegate rhandler, write32s_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64s_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write64s_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64s_delegate rhandler, write64s_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }

	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8sm_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write8sm_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8sm_delegate rhandler, write8sm_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16sm_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write16sm_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16sm_delegate rhandler, write16sm_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32sm_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write32sm_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32sm_delegate rhandler, write32sm_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64sm_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write64sm_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64sm_delegate rhandler, write64sm_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }

	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8mo_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write8mo_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8mo_delegate rhandler, write8mo_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16mo_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write16mo_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16mo_delegate rhandler, write16mo_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32mo_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write32mo_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32mo_delegate rhandler, write32mo_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64mo_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write64mo_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64mo_delegate rhandler, write64mo_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }

	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8smo_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write8smo_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8smo_delegate rhandler, write8smo_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16smo_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write16smo_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16smo_delegate rhandler, write16smo_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32smo_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write32smo_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32smo_delegate rhandler, write32smo_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64smo_delegate rhandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write64smo_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64smo_delegate rhandler, write64smo_delegate whandler, u64 unitmask = 0, int cswidth = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, rhandler, whandler); }

	using address_space::install_read_tap;
	using address_space::install_write_tap;
	using address_space::install_readwrite_tap;

	virtual memory_passthrough_handler *install_read_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, uX &data, uX mem_mask)> tap, memory_passthrough_handler *mph) override;
	virtual memory_passthrough_handler *install_write_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, uX &data, uX mem_mask)> tap, memory_passthrough_handler *mph) override;
	virtual memory_passthrough_handler *install_readwrite_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, uX &data, uX mem_mask)> tapr, std::function<void (offs_t offset, uX &data, uX mem_mask)> tapw, memory_passthrough_handler *mph) override;

	// construction/destruction
	address_space_specific(memory_manager &manager, device_memory_interface &memory, int spacenum, int address_width)
		: address_space(manager, memory, spacenum)
	{
		m_unmap_r = new handler_entry_read_unmapped <Width, AddrShift, Endian>(this);
		m_unmap_w = new handler_entry_write_unmapped<Width, AddrShift, Endian>(this);
		m_nop_r = new handler_entry_read_nop <Width, AddrShift, Endian>(this);
		m_nop_w = new handler_entry_write_nop<Width, AddrShift, Endian>(this);

		handler_entry::range r{ 0, 0xffffffff >> (32 - address_width) };

		switch (address_width) {
			case  1: m_root_read = new handler_entry_read_dispatch< std::max(1, Width), Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch< std::max(1, Width), Width, AddrShift, Endian>(this, r, nullptr); break;
			case  2: m_root_read = new handler_entry_read_dispatch< std::max(2, Width), Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch< std::max(2, Width), Width, AddrShift, Endian>(this, r, nullptr); break;
			case  3: m_root_read = new handler_entry_read_dispatch< std::max(3, Width), Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch< std::max(3, Width), Width, AddrShift, Endian>(this, r, nullptr); break;
			case  4: m_root_read = new handler_entry_read_dispatch< 4, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch< 4, Width, AddrShift, Endian>(this, r, nullptr); break;
			case  5: m_root_read = new handler_entry_read_dispatch< 5, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch< 5, Width, AddrShift, Endian>(this, r, nullptr); break;
			case  6: m_root_read = new handler_entry_read_dispatch< 6, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch< 6, Width, AddrShift, Endian>(this, r, nullptr); break;
			case  7: m_root_read = new handler_entry_read_dispatch< 7, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch< 7, Width, AddrShift, Endian>(this, r, nullptr); break;
			case  8: m_root_read = new handler_entry_read_dispatch< 8, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch< 8, Width, AddrShift, Endian>(this, r, nullptr); break;
			case  9: m_root_read = new handler_entry_read_dispatch< 9, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch< 9, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 10: m_root_read = new handler_entry_read_dispatch<10, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<10, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 11: m_root_read = new handler_entry_read_dispatch<11, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<11, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 12: m_root_read = new handler_entry_read_dispatch<12, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<12, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 13: m_root_read = new handler_entry_read_dispatch<13, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<13, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 14: m_root_read = new handler_entry_read_dispatch<14, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<14, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 15: m_root_read = new handler_entry_read_dispatch<15, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<15, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 16: m_root_read = new handler_entry_read_dispatch<16, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<16, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 17: m_root_read = new handler_entry_read_dispatch<17, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<17, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 18: m_root_read = new handler_entry_read_dispatch<18, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<18, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 19: m_root_read = new handler_entry_read_dispatch<19, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<19, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 20: m_root_read = new handler_entry_read_dispatch<20, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<20, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 21: m_root_read = new handler_entry_read_dispatch<21, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<21, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 22: m_root_read = new handler_entry_read_dispatch<22, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<22, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 23: m_root_read = new handler_entry_read_dispatch<23, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<23, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 24: m_root_read = new handler_entry_read_dispatch<24, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<24, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 25: m_root_read = new handler_entry_read_dispatch<25, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<25, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 26: m_root_read = new handler_entry_read_dispatch<26, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<26, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 27: m_root_read = new handler_entry_read_dispatch<27, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<27, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 28: m_root_read = new handler_entry_read_dispatch<28, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<28, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 29: m_root_read = new handler_entry_read_dispatch<29, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<29, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 30: m_root_read = new handler_entry_read_dispatch<30, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<30, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 31: m_root_read = new handler_entry_read_dispatch<31, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<31, Width, AddrShift, Endian>(this, r, nullptr); break;
			case 32: m_root_read = new handler_entry_read_dispatch<32, Width, AddrShift, Endian>(this, r, nullptr); m_root_write = new handler_entry_write_dispatch<32, Width, AddrShift, Endian>(this, r, nullptr); break;
			default: fatalerror("Unhandled address bus width %d\n", address_width);
		}
	}

	void *create_cache() override {
		return new memory_access_cache<Width, AddrShift, Endian>(*this, m_root_read, m_root_write);
	}

	void delayed_ref(handler_entry *e) {
		e->ref();
		m_delayed_unrefs.insert(e);
	}

	void delayed_unref(handler_entry *e) {
		m_delayed_unrefs.erase(m_delayed_unrefs.find(e));
		e->unref();
	}

	void validate_reference_counts() const override {
		handler_entry::reflist refs;
		refs.add(m_root_read);
		refs.add(m_root_write);
		refs.add(m_unmap_r);
		refs.add(m_unmap_w);
		refs.add(m_nop_r);
		refs.add(m_nop_w);
		for(handler_entry *e : m_delayed_unrefs)
			refs.add(e);
		refs.propagate();
		refs.check();
	}

	virtual void remove_passthrough(std::unordered_set<handler_entry *> &handlers) override {
		invalidate_caches(read_or_write::READWRITE);
		m_root_read->detach(handlers);
		m_root_write->detach(handlers);
	}

	// generate accessor table
	virtual void accessors(data_accessors &accessors) const override
	{
		accessors.read_byte = reinterpret_cast<u8 (*)(address_space &, offs_t)>(&read_byte_static);
		accessors.read_word = reinterpret_cast<u16 (*)(address_space &, offs_t)>(&read_word_static);
		accessors.read_word_masked = reinterpret_cast<u16 (*)(address_space &, offs_t, u16)>(&read_word_masked_static);
		accessors.read_dword = reinterpret_cast<u32 (*)(address_space &, offs_t)>(&read_dword_static);
		accessors.read_dword_masked = reinterpret_cast<u32 (*)(address_space &, offs_t, u32)>(&read_dword_masked_static);
		accessors.read_qword = reinterpret_cast<u64 (*)(address_space &, offs_t)>(&read_qword_static);
		accessors.read_qword_masked = reinterpret_cast<u64 (*)(address_space &, offs_t, u64)>(&read_qword_masked_static);
		accessors.write_byte = reinterpret_cast<void (*)(address_space &, offs_t, u8)>(&write_byte_static);
		accessors.write_word = reinterpret_cast<void (*)(address_space &, offs_t, u16)>(&write_word_static);
		accessors.write_word_masked = reinterpret_cast<void (*)(address_space &, offs_t, u16, u16)>(&write_word_masked_static);
		accessors.write_dword = reinterpret_cast<void (*)(address_space &, offs_t, u32)>(&write_dword_static);
		accessors.write_dword_masked = reinterpret_cast<void (*)(address_space &, offs_t, u32, u32)>(&write_dword_masked_static);
		accessors.write_qword = reinterpret_cast<void (*)(address_space &, offs_t, u64)>(&write_qword_static);
		accessors.write_qword_masked = reinterpret_cast<void (*)(address_space &, offs_t, u64, u64)>(&write_qword_masked_static);
	}

	// return a pointer to the read bank, or nullptr if none
	virtual void *get_read_ptr(offs_t address) const override
	{
		return m_root_read->get_ptr(address);
	}

	// return a pointer to the write bank, or nullptr if none
	virtual void *get_write_ptr(offs_t address) const override
	{
		return m_root_write->get_ptr(address);
	}

	// native read
	NativeType read_native(offs_t offset, NativeType mask)
	{
		g_profiler.start(PROFILER_MEMREAD);

		uX result = m_root_read->read(offset, mask);

		g_profiler.stop();
		return result;
	}

	// mask-less native read
	NativeType read_native(offs_t offset)
	{
		g_profiler.start(PROFILER_MEMREAD);

		uX result = m_root_read->read(offset, uX(0xffffffffffffffffU));

		g_profiler.stop();
		return result;
	}

	// native write
	void write_native(offs_t offset, NativeType data, NativeType mask)
	{
		g_profiler.start(PROFILER_MEMWRITE);

		m_root_write->write(offset, data, mask);

		g_profiler.stop();
	}

	// mask-less native write
	void write_native(offs_t offset, NativeType data)
	{
		g_profiler.start(PROFILER_MEMWRITE);

		m_root_write->write(offset, data, uX(0xffffffffffffffffU));

		g_profiler.stop();
	}

	// virtual access to these functions
	u8 read_byte(offs_t address) override { address &= m_addrmask; return Width == 0 ? read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 0, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xff); }
	u16 read_word(offs_t address) override { address &= m_addrmask; return Width == 1 ? read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 1, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffff); }
	u16 read_word(offs_t address, u16 mask) override { address &= m_addrmask; return memory_read_generic<Width, AddrShift, Endian, 1, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }
	u16 read_word_unaligned(offs_t address) override { address &= m_addrmask; return memory_read_generic<Width, AddrShift, Endian, 1, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffff); }
	u16 read_word_unaligned(offs_t address, u16 mask) override { address &= m_addrmask; return memory_read_generic<Width, AddrShift, Endian, 1, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }
	u32 read_dword(offs_t address) override { address &= m_addrmask; return Width == 2 ? read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 2, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffffffff); }
	u32 read_dword(offs_t address, u32 mask) override { address &= m_addrmask; return memory_read_generic<Width, AddrShift, Endian, 2, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }
	u32 read_dword_unaligned(offs_t address) override { address &= m_addrmask; return memory_read_generic<Width, AddrShift, Endian, 2, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffffffff); }
	u32 read_dword_unaligned(offs_t address, u32 mask) override { address &= m_addrmask; return memory_read_generic<Width, AddrShift, Endian, 2, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }
	u64 read_qword(offs_t address) override { address &= m_addrmask; return Width == 3 ? read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 3, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffffffffffffffffU); }
	u64 read_qword(offs_t address, u64 mask) override { address &= m_addrmask; return memory_read_generic<Width, AddrShift, Endian, 3, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }
	u64 read_qword_unaligned(offs_t address) override { address &= m_addrmask; return memory_read_generic<Width, AddrShift, Endian, 3, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffffffffffffffffU); }
	u64 read_qword_unaligned(offs_t address, u64 mask) override { address &= m_addrmask; return memory_read_generic<Width, AddrShift, Endian, 3, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }

	void write_byte(offs_t address, u8 data) override { address &= m_addrmask; if (Width == 0) write_native(address & ~NATIVE_MASK, data); else memory_write_generic<Width, AddrShift, Endian, 0, true>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, 0xff); }
	void write_word(offs_t address, u16 data) override { address &= m_addrmask; if (Width == 1) write_native(address & ~NATIVE_MASK, data); else memory_write_generic<Width, AddrShift, Endian, 1, true>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, 0xffff); }
	void write_word(offs_t address, u16 data, u16 mask) override { address &= m_addrmask; memory_write_generic<Width, AddrShift, Endian, 1, true>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, mask); }
	void write_word_unaligned(offs_t address, u16 data) override { address &= m_addrmask; memory_write_generic<Width, AddrShift, Endian, 1, false>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, 0xffff); }
	void write_word_unaligned(offs_t address, u16 data, u16 mask) override { address &= m_addrmask; memory_write_generic<Width, AddrShift, Endian, 1, false>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, mask); }
	void write_dword(offs_t address, u32 data) override { address &= m_addrmask; if (Width == 2) write_native(address & ~NATIVE_MASK, data); else memory_write_generic<Width, AddrShift, Endian, 2, true>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, 0xffffffff); }
	void write_dword(offs_t address, u32 data, u32 mask) override { address &= m_addrmask; memory_write_generic<Width, AddrShift, Endian, 2, true>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, mask); }
	void write_dword_unaligned(offs_t address, u32 data) override { address &= m_addrmask; memory_write_generic<Width, AddrShift, Endian, 2, false>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, 0xffffffff); }
	void write_dword_unaligned(offs_t address, u32 data, u32 mask) override { address &= m_addrmask; memory_write_generic<Width, AddrShift, Endian, 2, false>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, mask); }
	void write_qword(offs_t address, u64 data) override { address &= m_addrmask; if (Width == 3) write_native(address & ~NATIVE_MASK, data); else memory_write_generic<Width, AddrShift, Endian, 3, true>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, 0xffffffffffffffffU); }
	void write_qword(offs_t address, u64 data, u64 mask) override { address &= m_addrmask; memory_write_generic<Width, AddrShift, Endian, 3, true>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, mask); }
	void write_qword_unaligned(offs_t address, u64 data) override { address &= m_addrmask; memory_write_generic<Width, AddrShift, Endian, 3, false>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, 0xffffffffffffffffU); }
	void write_qword_unaligned(offs_t address, u64 data, u64 mask) override {address &= m_addrmask;  memory_write_generic<Width, AddrShift, Endian, 3, false>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, mask); }

	// static access to these functions
	static u8 read_byte_static(this_type &space, offs_t address) { address &= space.m_addrmask; return Width == 0 ? space.read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 0, true>([&space](offs_t offset, NativeType mask) -> NativeType { return space.read_native(offset, mask); }, address, 0xff); }
	static u16 read_word_static(this_type &space, offs_t address) { address &= space.m_addrmask; return Width == 1 ? space.read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 1, true>([&space](offs_t offset, NativeType mask) -> NativeType { return space.read_native(offset, mask); }, address, 0xffff); }
	static u16 read_word_masked_static(this_type &space, offs_t address, u16 mask) { address &= space.m_addrmask; return memory_read_generic<Width, AddrShift, Endian, 1, true>([&space](offs_t offset, NativeType mask) -> NativeType { return space.read_native(offset, mask); }, address, mask); }
	static u32 read_dword_static(this_type &space, offs_t address) { address &= space.m_addrmask; return Width == 2 ? space.read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 2, true>([&space](offs_t offset, NativeType mask) -> NativeType { return space.read_native(offset, mask); }, address, 0xffffffff); }
	static u32 read_dword_masked_static(this_type &space, offs_t address, u32 mask) { address &= space.m_addrmask; return memory_read_generic<Width, AddrShift, Endian, 2, true>([&space](offs_t offset, NativeType mask) -> NativeType { return space.read_native(offset, mask); }, address, mask); }
	static u64 read_qword_static(this_type &space, offs_t address) { address &= space.m_addrmask; return Width == 3 ? space.read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 3, true>([&space](offs_t offset, NativeType mask) -> NativeType { return space.read_native(offset, mask); }, address, 0xffffffffffffffffU); }
	static u64 read_qword_masked_static(this_type &space, offs_t address, u64 mask) { address &= space.m_addrmask; return memory_read_generic<Width, AddrShift, Endian, 3, true>([&space](offs_t offset, NativeType mask) -> NativeType { return space.read_native(offset, mask); }, address, mask); }
	static void write_byte_static(this_type &space, offs_t address, u8 data) { address &= space.m_addrmask; if (Width == 0) space.write_native(address & ~NATIVE_MASK, data); else memory_write_generic<Width, AddrShift, Endian, 0, true>([&space](offs_t offset, NativeType data, NativeType mask) { space.write_native(offset, data, mask); }, address, data, 0xff); }
	static void write_word_static(this_type &space, offs_t address, u16 data) { address &= space.m_addrmask; if (Width == 1) space.write_native(address & ~NATIVE_MASK, data); else memory_write_generic<Width, AddrShift, Endian, 1, true>([&space](offs_t offset, NativeType data, NativeType mask) { space.write_native(offset, data, mask); }, address, data, 0xffff); }
	static void write_word_masked_static(this_type &space, offs_t address, u16 data, u16 mask) { address &= space.m_addrmask; memory_write_generic<Width, AddrShift, Endian, 1, true>([&space](offs_t offset, NativeType data, NativeType mask) { space.write_native(offset, data, mask); }, address, data, mask); }
	static void write_dword_static(this_type &space, offs_t address, u32 data) { address &= space.m_addrmask; if (Width == 2) space.write_native(address & ~NATIVE_MASK, data); else memory_write_generic<Width, AddrShift, Endian, 2, true>([&space](offs_t offset, NativeType data, NativeType mask) { space.write_native(offset, data, mask); }, address, data, 0xffffffff); }
	static void write_dword_masked_static(this_type &space, offs_t address, u32 data, u32 mask) { address &= space.m_addrmask; memory_write_generic<Width, AddrShift, Endian, 2, true>([&space](offs_t offset, NativeType data, NativeType mask) { space.write_native(offset, data, mask); }, address, data, mask); }
	static void write_qword_static(this_type &space, offs_t address, u64 data) { address &= space.m_addrmask; if (Width == 3) space.write_native(address & ~NATIVE_MASK, data); else memory_write_generic<Width, AddrShift, Endian, 3, false>([&space](offs_t offset, NativeType data, NativeType mask) { space.write_native(offset, data, mask); }, address, data, 0xffffffffffffffffU); }
	static void write_qword_masked_static(this_type &space, offs_t address, u64 data, u64 mask) { address &= space.m_addrmask; memory_write_generic<Width, AddrShift, Endian, 3, false>([&space](offs_t offset, NativeType data, NativeType mask) { space.write_native(offset, data, mask); }, address, data, mask); }

	handler_entry_read <Width, AddrShift, Endian> *m_root_read;
	handler_entry_write<Width, AddrShift, Endian> *m_root_write;

	std::unordered_set<handler_entry *> m_delayed_unrefs;

private:
	template<typename READ>
	void install_read_handler_impl(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth, READ &handler_r)
	{
		try { handler_r.resolve(); }
		catch (const binding_type_exception &) {
			osd_printf_error("Binding error while installing read handler %s for range 0x%X-0x%X mask 0x%X mirror 0x%X select 0x%X umask 0x%X\n", handler_r.name(), addrstart, addrend, addrmask, addrmirror, addrselect, unitmask);
			throw;
		}
		install_read_handler_helper<handler_width<READ>::value>(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, handler_r);
	}

	template<typename WRITE>
	void install_write_handler_impl(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth, WRITE &handler_w)
	{
		try { handler_w.resolve(); }
		catch (const binding_type_exception &) {
			osd_printf_error("Binding error while installing write handler %s for range 0x%X-0x%X mask 0x%X mirror 0x%X select 0x%X umask 0x%X\n", handler_w.name(), addrstart, addrend, addrmask, addrmirror, addrselect, unitmask);
			throw;
		}
		install_write_handler_helper<handler_width<WRITE>::value>(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, handler_w);
	}

	template<typename READ, typename WRITE>
	void install_readwrite_handler_impl(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth, READ &handler_r, WRITE &handler_w)
	{
		static_assert(handler_width<READ>::value == handler_width<WRITE>::value, "handler widths do not match");
		try { handler_r.resolve(); }
		catch (const binding_type_exception &) {
			osd_printf_error("Binding error while installing read handler %s for range 0x%X-0x%X mask 0x%X mirror 0x%X select 0x%X umask 0x%X\n", handler_r.name(), addrstart, addrend, addrmask, addrmirror, addrselect, unitmask);
			throw;
		}
		try { handler_w.resolve(); }
		catch (const binding_type_exception &) {
			osd_printf_error("Binding error while installing write handler %s for range 0x%X-0x%X mask 0x%X mirror 0x%X select 0x%X umask 0x%X\n", handler_w.name(), addrstart, addrend, addrmask, addrmirror, addrselect, unitmask);
			throw;
		}
		install_readwrite_handler_helper<handler_width<READ>::value>(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, handler_r, handler_w);
	}

	template<int AccessWidth, typename READ> std::enable_if_t<(Width == AccessWidth)>
	install_read_handler_helper(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth, const READ &handler_r)
	{
		VPRINTF(("address_space::install_read_handler(%s-%s mask=%s mirror=%s, space width=%d, handler width=%d, %s, %s)\n",
				 core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
				 core_i64_hex_format(addrmask, m_addrchars), core_i64_hex_format(addrmirror, m_addrchars),
				 8 << Width, 8 << AccessWidth,
				 handler_r.name(), core_i64_hex_format(unitmask, data_width() / 4)));

		offs_t nstart, nend, nmask, nmirror;
		u64 nunitmask;
		int ncswidth;
		check_optimize_all("install_read_handler", 8 << AccessWidth, addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, nstart, nend, nmask, nmirror, nunitmask, ncswidth);

		auto hand_r = new handler_entry_read_delegate<Width, AddrShift, Endian, READ>(this, handler_r);
		hand_r->set_address_info(nstart, nmask);
		m_root_read->populate(nstart, nend, nmirror, hand_r);
		invalidate_caches(read_or_write::READ);
	}

	template<int AccessWidth, typename READ> std::enable_if_t<(Width > AccessWidth)>
	install_read_handler_helper(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth,
								const READ &handler_r)
	{
		VPRINTF(("address_space::install_read_handler(%s-%s mask=%s mirror=%s, space width=%d, handler width=%d, %s, %s)\n",
				 core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
				 core_i64_hex_format(addrmask, m_addrchars), core_i64_hex_format(addrmirror, m_addrchars),
				 8 << Width, 8 << AccessWidth,
				 handler_r.name(), core_i64_hex_format(unitmask, data_width() / 4)));

		offs_t nstart, nend, nmask, nmirror;
		u64 nunitmask;
		int ncswidth;
		check_optimize_all("install_read_handler", 8 << AccessWidth, addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, nstart, nend, nmask, nmirror, nunitmask, ncswidth);

		auto hand_r = new handler_entry_read_delegate<AccessWidth, -AccessWidth, Endian, READ>(this, handler_r);
		memory_units_descriptor<Width, AddrShift, Endian> descriptor(AccessWidth, Endian, hand_r, nstart, nend, nmask, nunitmask, ncswidth);
		hand_r->set_address_info(descriptor.get_handler_start(), descriptor.get_handler_mask());
		m_root_read->populate_mismatched(nstart, nend, nmirror, descriptor);
		hand_r->unref();
		invalidate_caches(read_or_write::READ);
	}


	template<int AccessWidth, typename READ> std::enable_if_t<(Width < AccessWidth)>
	install_read_handler_helper(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth,
								const READ &handler_r)
	{
		fatalerror("install_read_handler: cannot install a %d-wide handler in a %d-wide bus", 8 << AccessWidth, 8 << Width);
	}

	template<int AccessWidth, typename WRITE> std::enable_if_t<(Width == AccessWidth)>
	install_write_handler_helper(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth,
								 const WRITE &handler_w)
	{
		VPRINTF(("address_space::install_write_handler(%s-%s mask=%s mirror=%s, space width=%d, handler width=%d, %s, %s)\n",
				 core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
				 core_i64_hex_format(addrmask, m_addrchars), core_i64_hex_format(addrmirror, m_addrchars),
				 8 << Width, 8 << AccessWidth,
				 handler_w.name(), core_i64_hex_format(unitmask, data_width() / 4)));

		offs_t nstart, nend, nmask, nmirror;
		u64 nunitmask;
		int ncswidth;
		check_optimize_all("install_write_handler", 8 << AccessWidth, addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, nstart, nend, nmask, nmirror, nunitmask, ncswidth);

		auto hand_w = new handler_entry_write_delegate<Width, AddrShift, Endian, WRITE>(this, handler_w);
		hand_w->set_address_info(nstart, nmask);
		m_root_write->populate(nstart, nend, nmirror, hand_w);
		invalidate_caches(read_or_write::WRITE);
	}

	template<int AccessWidth, typename WRITE> std::enable_if_t<(Width > AccessWidth)>
	install_write_handler_helper(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth,
								 const WRITE &handler_w)
	{
		VPRINTF(("address_space::install_write_handler(%s-%s mask=%s mirror=%s, space width=%d, handler width=%d, %s, %s)\n",
				 core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
				 core_i64_hex_format(addrmask, m_addrchars), core_i64_hex_format(addrmirror, m_addrchars),
				 8 << Width, 8 << AccessWidth,
				 handler_w.name(), core_i64_hex_format(unitmask, data_width() / 4)));

		offs_t nstart, nend, nmask, nmirror;
		u64 nunitmask;
		int ncswidth;
		check_optimize_all("install_write_handler", 8 << AccessWidth, addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, nstart, nend, nmask, nmirror, nunitmask, ncswidth);

		auto hand_w = new handler_entry_write_delegate<AccessWidth, -AccessWidth, Endian, WRITE>(this, handler_w);
		memory_units_descriptor<Width, AddrShift, Endian> descriptor(AccessWidth, Endian, hand_w, nstart, nend, nmask, nunitmask, ncswidth);
		hand_w->set_address_info(descriptor.get_handler_start(), descriptor.get_handler_mask());
		m_root_write->populate_mismatched(nstart, nend, nmirror, descriptor);
		hand_w->unref();
		invalidate_caches(read_or_write::WRITE);
	}


	template<int AccessWidth, typename WRITE> std::enable_if_t<(Width < AccessWidth)>
	install_write_handler_helper(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth,
								 const WRITE &handler_w)
	{
		fatalerror("install_write_handler: cannot install a %d-wide handler in a %d-wide bus", 8 << AccessWidth, 8 << Width);
	}



	template<int AccessWidth, typename READ, typename WRITE> std::enable_if_t<(Width == AccessWidth)>
	install_readwrite_handler_helper(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth,
									 const READ  &handler_r,
									 const WRITE &handler_w)
	{
		VPRINTF(("address_space::install_readwrite_handler(%s-%s mask=%s mirror=%s, space width=%d, handler width=%d, %s, %s, %s)\n",
				 core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
				 core_i64_hex_format(addrmask, m_addrchars), core_i64_hex_format(addrmirror, m_addrchars),
				 8 << Width, 8 << AccessWidth,
				 handler_r.name(), handler_w.name(), core_i64_hex_format(unitmask, data_width() / 4)));

		offs_t nstart, nend, nmask, nmirror;
		u64 nunitmask;
		int ncswidth;
		check_optimize_all("install_readwrite_handler", 8 << AccessWidth, addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, nstart, nend, nmask, nmirror, nunitmask, ncswidth);

		auto hand_r = new handler_entry_read_delegate <Width, AddrShift, Endian, READ>(this, handler_r);
		hand_r->set_address_info(nstart, nmask);
		m_root_read ->populate(nstart, nend, nmirror, hand_r);

		auto hand_w = new handler_entry_write_delegate<Width, AddrShift, Endian, WRITE>(this, handler_w);
		hand_w->set_address_info(nstart, nmask);
		m_root_write->populate(nstart, nend, nmirror, hand_w);

		invalidate_caches(read_or_write::READWRITE);
	}

	template<int AccessWidth, typename READ, typename WRITE> std::enable_if_t<(Width > AccessWidth)>
	install_readwrite_handler_helper(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth,
									 const READ  &handler_r,
									 const WRITE &handler_w)
	{
		VPRINTF(("address_space::install_readwrite_handler(%s-%s mask=%s mirror=%s, space width=%d, handler width=%d, %s, %s, %s)\n",
				 core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
				 core_i64_hex_format(addrmask, m_addrchars), core_i64_hex_format(addrmirror, m_addrchars),
				 8 << Width, 8 << AccessWidth,
				 handler_r.name(), handler_w.name(), core_i64_hex_format(unitmask, data_width() / 4)));

		offs_t nstart, nend, nmask, nmirror;
		u64 nunitmask;
		int ncswidth;
		check_optimize_all("install_readwrite_handler", 8 << AccessWidth, addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, nstart, nend, nmask, nmirror, nunitmask, ncswidth);

		auto hand_r = new handler_entry_read_delegate <AccessWidth, -AccessWidth, Endian, READ>(this, handler_r);
		memory_units_descriptor<Width, AddrShift, Endian> descriptor(AccessWidth, Endian, hand_r, nstart, nend, nmask, nunitmask, ncswidth);
		hand_r->set_address_info(descriptor.get_handler_start(), descriptor.get_handler_mask());
		m_root_read ->populate_mismatched(nstart, nend, nmirror, descriptor);
		hand_r->unref();

		auto hand_w = new handler_entry_write_delegate<AccessWidth, -AccessWidth, Endian, WRITE>(this, handler_w);
		descriptor.set_subunit_handler(hand_w);
		hand_w->set_address_info(descriptor.get_handler_start(), descriptor.get_handler_mask());
		m_root_write->populate_mismatched(nstart, nend, nmirror, descriptor);
		hand_w->unref();

		invalidate_caches(read_or_write::READWRITE);
	}


	template<int AccessWidth, typename READ, typename WRITE> std::enable_if_t<(Width < AccessWidth)>
	install_readwrite_handler_helper(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth,
									 const READ  &handler_r,
									 const WRITE &handler_w)
	{
		fatalerror("install_readwrite_handler: cannot install a %d-wide handler in a %d-wide bus", 8 << AccessWidth, 8 << Width);
	}
};



//**************************************************************************
//  MEMORY MANAGER
//**************************************************************************

//-------------------------------------------------
//  memory_manager - constructor
//-------------------------------------------------

memory_manager::memory_manager(running_machine &machine)
	: m_machine(machine),
	  m_initialized(false)
{
}

//-------------------------------------------------
//  allocate - allocate memory spaces
//-------------------------------------------------

void memory_manager::allocate(device_memory_interface &memory)
{
	for (int spacenum = 0; spacenum < memory.max_space_count(); ++spacenum)
	{
		// if there is a configuration for this space, we need an address space
		address_space_config const *const spaceconfig = memory.space_config(spacenum);
		if (spaceconfig)
		{
			// allocate one of the appropriate type
			switch (spaceconfig->data_width() | (spaceconfig->addr_shift() + 4))
			{
				case  8|(4+1):
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
						memory.allocate<address_space_specific<0,  1, ENDIANNESS_LITTLE>>(*this, spacenum);
					else
						memory.allocate<address_space_specific<0,  1, ENDIANNESS_BIG   >>(*this, spacenum);
					break;

				case  8|(4-0):
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
						memory.allocate<address_space_specific<0,  0, ENDIANNESS_LITTLE>>(*this, spacenum);
					else
						memory.allocate<address_space_specific<0,  0, ENDIANNESS_BIG   >>(*this, spacenum);
					break;

				case 16|(4+3):
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
						memory.allocate<address_space_specific<1,  3, ENDIANNESS_LITTLE>>(*this, spacenum);
					else
						memory.allocate<address_space_specific<1,  3, ENDIANNESS_BIG   >>(*this, spacenum);
					break;

				case 16|(4-0):
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
						memory.allocate<address_space_specific<1,  0, ENDIANNESS_LITTLE>>(*this, spacenum);
					else
						memory.allocate<address_space_specific<1,  0, ENDIANNESS_BIG   >>(*this, spacenum);
					break;

				case 16|(4-1):
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
						memory.allocate<address_space_specific<1, -1, ENDIANNESS_LITTLE>>(*this, spacenum);
					else
						memory.allocate<address_space_specific<1, -1, ENDIANNESS_BIG   >>(*this, spacenum);
					break;

				case 32|(4-0):
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
						memory.allocate<address_space_specific<2,  0, ENDIANNESS_LITTLE>>(*this, spacenum);
					else
						memory.allocate<address_space_specific<2,  0, ENDIANNESS_BIG   >>(*this, spacenum);
					break;

				case 32|(4-1):
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
						memory.allocate<address_space_specific<2, -1, ENDIANNESS_LITTLE>>(*this, spacenum);
					else
						memory.allocate<address_space_specific<2, -1, ENDIANNESS_BIG   >>(*this, spacenum);
					break;

				case 32|(4-2):
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
						memory.allocate<address_space_specific<2, -2, ENDIANNESS_LITTLE>>(*this, spacenum);
					else
						memory.allocate<address_space_specific<2, -2, ENDIANNESS_BIG   >>(*this, spacenum);
					break;

				case 64|(4-0):
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
						memory.allocate<address_space_specific<3,  0, ENDIANNESS_LITTLE>>(*this, spacenum);
					else
						memory.allocate<address_space_specific<3,  0, ENDIANNESS_BIG   >>(*this, spacenum);
					break;

				case 64|(4-1):
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
						memory.allocate<address_space_specific<3, -1, ENDIANNESS_LITTLE>>(*this, spacenum);
					else
						memory.allocate<address_space_specific<3, -1, ENDIANNESS_BIG   >>(*this, spacenum);
					break;

				case 64|(4-2):
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
						memory.allocate<address_space_specific<3, -2, ENDIANNESS_LITTLE>>(*this, spacenum);
					else
						memory.allocate<address_space_specific<3, -2, ENDIANNESS_BIG   >>(*this, spacenum);
					break;

				case 64|(4-3):
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
						memory.allocate<address_space_specific<3, -3, ENDIANNESS_LITTLE>>(*this, spacenum);
					else
						memory.allocate<address_space_specific<3, -3, ENDIANNESS_BIG   >>(*this, spacenum);
					break;

				default:
					throw emu_fatalerror("Invalid width %d/shift %d specified for address_space::allocate", spaceconfig->data_width(), spaceconfig->addr_shift());
			}
		}
	}
}

//-------------------------------------------------
//  initialize - initialize the memory system
//-------------------------------------------------

void memory_manager::initialize()
{
	// loop over devices and spaces within each device
	memory_interface_iterator iter(machine().root_device());
	std::vector<device_memory_interface *> memories;
	for (device_memory_interface &memory : iter)
	{
		memories.push_back(&memory);
		allocate(memory);
	}

	allocate(m_machine.m_dummy_space);

	// construct and preprocess the address_map for each space
	for (auto const memory : memories)
		memory->prepare_maps();

	// create the handlers from the resulting address maps
	for (auto const memory : memories)
		memory->populate_from_maps();

	// allocate memory needed to back each address space
	for (auto const memory : memories)
		memory->allocate_memory();

	// find all the allocated pointers
	for (auto const memory : memories)
		memory->locate_memory();

	// disable logging of unmapped access when no one receives it
	if (!machine().options().log() && !machine().options().oslog() && !(machine().debug_flags & DEBUG_FLAG_ENABLED))
		for (auto const memory : memories)
			memory->set_log_unmap(false);

	// we are now initialized
	m_initialized = true;
}


//-------------------------------------------------
//  region_alloc - allocates memory for a region
//-------------------------------------------------

memory_region *memory_manager::region_alloc(const char *name, u32 length, u8 width, endianness_t endian)
{
	osd_printf_verbose("Region '%s' created\n", name);
	// make sure we don't have a region of the same name; also find the end of the list
	if (m_regionlist.find(name) != m_regionlist.end())
		fatalerror("region_alloc called with duplicate region name \"%s\"\n", name);

	// allocate the region
	m_regionlist.emplace(name, std::make_unique<memory_region>(machine(), name, length, width, endian));
	return m_regionlist.find(name)->second.get();
}


//-------------------------------------------------
//  region_free - releases memory for a region
//-------------------------------------------------

void memory_manager::region_free(const char *name)
{
	m_regionlist.erase(name);
}


//-------------------------------------------------
//  region_containing - helper to determine if
//  a block of memory is part of a region
//-------------------------------------------------

memory_region *memory_manager::region_containing(const void *memory, offs_t bytes) const
{
	const u8 *data = reinterpret_cast<const u8 *>(memory);

	// look through the region list and return the first match
	for (auto &region : m_regionlist)
		if (data >= region.second->base() && (data + bytes) <= region.second->end())
			return region.second.get();

	// didn't find one
	return nullptr;
}

memory_bank *memory_manager::find(const char *tag) const
{
	auto bank = m_banklist.find(tag);
	if (bank != m_banklist.end())
		return bank->second.get();
	return nullptr;
}

memory_bank *memory_manager::find(address_space &space, offs_t addrstart, offs_t addrend) const
{
	// try to find an exact match
	for (auto &bank : m_banklist)
		if (bank.second->anonymous() && bank.second->references_space(space, read_or_write::READWRITE) && bank.second->matches_exactly(addrstart, addrend))
			return bank.second.get();

	// not found
	return nullptr;
}

memory_bank *memory_manager::allocate(address_space &space, offs_t addrstart, offs_t addrend, const char *tag)
{
	auto bank = std::make_unique<memory_bank>(space, m_banklist.size(), addrstart, addrend, tag);
	std::string temptag;
	if (tag == nullptr) {
		temptag = string_format("anon_%p", bank.get());
		tag = temptag.c_str();
	}
	m_banklist.emplace(tag, std::move(bank));
	return m_banklist.find(tag)->second.get();
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


//**************************************************************************
//  ADDRESS SPACE
//**************************************************************************

//-------------------------------------------------
//  address_space - constructor
//-------------------------------------------------

address_space::address_space(memory_manager &manager, device_memory_interface &memory, int spacenum)
	: m_config(*memory.space_config(spacenum)),
		m_device(memory.device()),
		m_addrmask(make_bitmask<offs_t>(m_config.addr_width())),
		m_logaddrmask(make_bitmask<offs_t>(m_config.logaddr_width())),
		m_unmap(0),
		m_spacenum(spacenum),
		m_log_unmap(true),
		m_name(memory.space_config(spacenum)->name()),
		m_addrchars((m_config.addr_width() + 3) / 4),
		m_logaddrchars((m_config.logaddr_width() + 3) / 4),
		m_notifier_id(0),
		m_in_notification(0),
		m_manager(manager)
{
}


//-------------------------------------------------
//  ~address_space - destructor
//-------------------------------------------------

address_space::~address_space()
{
	m_unmap_r->unref();
	m_unmap_w->unref();
	m_nop_r->unref();
	m_nop_w->unref();
}

//-------------------------------------------------
//  adjust_addresses - adjust addresses for a
//  given address space in a standard fashion
//-------------------------------------------------

inline void address_space::adjust_addresses(offs_t &start, offs_t &end, offs_t &mask, offs_t &mirror)
{
	// adjust start/end/mask values
	mask &= m_addrmask;
	start &= ~mirror & m_addrmask;
	end &= ~mirror & m_addrmask;
}

void address_space::check_optimize_all(const char *function, int width, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth, offs_t &nstart, offs_t &nend, offs_t &nmask, offs_t &nmirror, u64 &nunitmask, int &ncswidth)
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
				fatalerror("%s: In range %x-%x mask %x mirror %x select %x, the unitmask of %s has incorrect granularity for %d-bit chip selection.\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, core_i64_hex_format(unitmask, 16), cswidth);
		}
	}

	nunitmask = 0xffffffffffffffffU >> (64 - m_config.data_width());
	if (unitmask)
		nunitmask &= unitmask;

	nstart = addrstart;
	nend = addrend;
	nmask = (addrmask ? addrmask : changing_bits) | addrselect;
	nmirror = (addrmirror & m_addrmask) | addrselect;
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

void address_space::check_optimize_mirror(const char *function, offs_t addrstart, offs_t addrend, offs_t addrmirror, offs_t &nstart, offs_t &nend, offs_t &nmask, offs_t &nmirror)
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

void address_space::check_address(const char *function, offs_t addrstart, offs_t addrend)
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
//  prepare_map - allocate the address map and
//  walk through it to find implicit memory regions
//  and identify shared regions
//-------------------------------------------------

void address_space::prepare_map()
{
	memory_region *devregion = (m_spacenum == 0) ? m_device.memregion(DEVICE_SELF) : nullptr;
	u32 devregionsize = (devregion != nullptr) ? devregion->bytes() : 0;

	// allocate the address map
	m_map = std::make_unique<address_map>(m_device, m_spacenum);

	// merge in the submaps
	m_map->import_submaps(m_manager.machine(), m_device.owner() ? *m_device.owner() : m_device, data_width(), endianness(), addr_shift());

	// extract global parameters specified by the map
	m_unmap = (m_map->m_unmapval == 0) ? 0 : ~0;
	if (m_map->m_globalmask != 0)
	{
		if (m_map->m_globalmask & ~m_addrmask)
			fatalerror("Can't set a global address mask of %08x on a %d-bits address width bus.\n", m_map->m_globalmask, addr_width());

		m_addrmask = m_map->m_globalmask;
	}

	// make a pass over the address map, adjusting for the device and getting memory pointers
	for (address_map_entry &entry : m_map->m_entrylist)
	{
		// computed adjusted addresses first
		adjust_addresses(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror);

		// if we have a share entry, add it to our map
		if (entry.m_share != nullptr)
		{
			// if we can't find it, add it to our map
			std::string fulltag = entry.m_devbase.subtag(entry.m_share);
			if (m_manager.m_sharelist.find(fulltag.c_str()) == m_manager.m_sharelist.end())
			{
				VPRINTF(("Creating share '%s' of length 0x%X\n", fulltag.c_str(), entry.m_addrend + 1 - entry.m_addrstart));
				m_manager.m_sharelist.emplace(fulltag.c_str(), std::make_unique<memory_share>(m_config.data_width(), address_to_byte(entry.m_addrend + 1 - entry.m_addrstart), endianness()));
			}
		}

		// if this is a ROM handler without a specified region, attach it to the implicit region
		if (m_spacenum == 0 && entry.m_read.m_type == AMH_ROM && entry.m_region == nullptr)
		{
			// make sure it fits within the memory region before doing so, however
			if (entry.m_addrend < devregionsize)
			{
				entry.m_region = m_device.tag();
				entry.m_rgnoffs = address_to_byte(entry.m_addrstart);
			}
		}

		// validate adjusted addresses against implicit regions
		if (entry.m_region != nullptr && entry.m_share == nullptr)
		{
			// determine full tag
			std::string fulltag = entry.m_devbase.subtag(entry.m_region);

			// find the region
			memory_region *region = m_manager.machine().root_device().memregion(fulltag.c_str());
			if (region == nullptr)
				fatalerror("device '%s' %s space memory map entry %X-%X references nonexistent region \"%s\"\n", m_device.tag(), m_name, entry.m_addrstart, entry.m_addrend, entry.m_region);

			// validate the region
			if (entry.m_rgnoffs + m_config.addr2byte(entry.m_addrend - entry.m_addrstart + 1) > region->bytes())
				fatalerror("device '%s' %s space memory map entry %X-%X extends beyond region \"%s\" size (%X)\n", m_device.tag(), m_name, entry.m_addrstart, entry.m_addrend, entry.m_region, region->bytes());
		}

		// convert any region-relative entries to their memory pointers
		if (entry.m_region != nullptr)
		{
			// determine full tag
			std::string fulltag = entry.m_devbase.subtag(entry.m_region);

			// set the memory address
			entry.m_memory = m_manager.machine().root_device().memregion(fulltag.c_str())->base() + entry.m_rgnoffs;
		}
	}
}


//-------------------------------------------------
//  populate_from_map - walk the map in reverse
//  order and install the appropriate handler for
//  each case
//-------------------------------------------------

void address_space::populate_from_map(address_map *map)
{
	// no map specified, use the space-specific one
	if (map == nullptr)
		map = m_map.get();

	// no map, nothing to do
	if (map == nullptr)
		return;

	// install the handlers, using the original, unadjusted memory map
	for (const address_map_entry &entry : map->m_entrylist) {
		// map both read and write halves
		populate_map_entry(entry, read_or_write::READ);
		populate_map_entry(entry, read_or_write::WRITE);
	}

	if(VALIDATE_REFCOUNTS)
		validate_reference_counts();
}


//-------------------------------------------------
//  populate_map_entry - map a single read or
//  write entry based on information from an
//  address map entry
//-------------------------------------------------

void address_space::populate_map_entry(const address_map_entry &entry, read_or_write readorwrite)
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

		case AMH_RAM:
			install_ram_generic(entry.m_addrstart, entry.m_addrend, entry.m_addrmirror, readorwrite, nullptr);
			break;

		case AMH_NOP:
			unmap_generic(entry.m_addrstart, entry.m_addrend, entry.m_addrmirror, readorwrite, true);
			break;

		case AMH_UNMAP:
			unmap_generic(entry.m_addrstart, entry.m_addrend, entry.m_addrmirror, readorwrite, false);
			break;

		case AMH_DEVICE_DELEGATE:
			if (readorwrite == read_or_write::READ)
				switch (data.m_bits)
				{
					case 8:     install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto8, entry.m_mask, entry.m_cswidth); break;
					case 16:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto16, entry.m_mask, entry.m_cswidth); break;
					case 32:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto32, entry.m_mask, entry.m_cswidth); break;
					case 64:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto64, entry.m_mask, entry.m_cswidth); break;
				}
			else
				switch (data.m_bits)
				{
					case 8:     install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto8, entry.m_mask, entry.m_cswidth); break;
					case 16:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto16, entry.m_mask, entry.m_cswidth); break;
					case 32:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto32, entry.m_mask, entry.m_cswidth); break;
					case 64:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto64, entry.m_mask, entry.m_cswidth); break;
				}
			break;

		case AMH_DEVICE_DELEGATE_M:
			if (readorwrite == read_or_write::READ)
				switch (data.m_bits)
				{
					case 8:     install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto8m, entry.m_mask, entry.m_cswidth); break;
					case 16:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto16m, entry.m_mask, entry.m_cswidth); break;
					case 32:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto32m, entry.m_mask, entry.m_cswidth); break;
					case 64:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto64m, entry.m_mask, entry.m_cswidth); break;
				}
			else
				switch (data.m_bits)
				{
					case 8:     install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto8m, entry.m_mask, entry.m_cswidth); break;
					case 16:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto16m, entry.m_mask, entry.m_cswidth); break;
					case 32:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto32m, entry.m_mask, entry.m_cswidth); break;
					case 64:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto64m, entry.m_mask, entry.m_cswidth); break;
				}
			break;

		case AMH_DEVICE_DELEGATE_S:
			if (readorwrite == read_or_write::READ)
				switch (data.m_bits)
				{
					case 8:     install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto8s, entry.m_mask, entry.m_cswidth); break;
					case 16:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto16s, entry.m_mask, entry.m_cswidth); break;
					case 32:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto32s, entry.m_mask, entry.m_cswidth); break;
					case 64:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto64s, entry.m_mask, entry.m_cswidth); break;
				}
			else
				switch (data.m_bits)
				{
					case 8:     install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto8s, entry.m_mask, entry.m_cswidth); break;
					case 16:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto16s, entry.m_mask, entry.m_cswidth); break;
					case 32:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto32s, entry.m_mask, entry.m_cswidth); break;
					case 64:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto64s, entry.m_mask, entry.m_cswidth); break;
				}
			break;

		case AMH_DEVICE_DELEGATE_SM:
			if (readorwrite == read_or_write::READ)
				switch (data.m_bits)
				{
					case 8:     install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto8sm, entry.m_mask, entry.m_cswidth); break;
					case 16:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto16sm, entry.m_mask, entry.m_cswidth); break;
					case 32:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto32sm, entry.m_mask, entry.m_cswidth); break;
					case 64:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto64sm, entry.m_mask, entry.m_cswidth); break;
				}
			else
				switch (data.m_bits)
				{
					case 8:     install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto8sm, entry.m_mask, entry.m_cswidth); break;
					case 16:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto16sm, entry.m_mask, entry.m_cswidth); break;
					case 32:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto32sm, entry.m_mask, entry.m_cswidth); break;
					case 64:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto64sm, entry.m_mask, entry.m_cswidth); break;
				}
			break;

		case AMH_DEVICE_DELEGATE_MO:
			if (readorwrite == read_or_write::READ)
				switch (data.m_bits)
				{
					case 8:     install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto8mo, entry.m_mask, entry.m_cswidth); break;
					case 16:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto16mo, entry.m_mask, entry.m_cswidth); break;
					case 32:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto32mo, entry.m_mask, entry.m_cswidth); break;
					case 64:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto64mo, entry.m_mask, entry.m_cswidth); break;
				}
			else
				switch (data.m_bits)
				{
					case 8:     install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto8mo, entry.m_mask, entry.m_cswidth); break;
					case 16:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto16mo, entry.m_mask, entry.m_cswidth); break;
					case 32:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto32mo, entry.m_mask, entry.m_cswidth); break;
					case 64:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto64mo, entry.m_mask, entry.m_cswidth); break;
				}
			break;

		case AMH_DEVICE_DELEGATE_SMO:
			if (readorwrite == read_or_write::READ)
				switch (data.m_bits)
				{
					case 8:     install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto8smo, entry.m_mask, entry.m_cswidth); break;
					case 16:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto16smo, entry.m_mask, entry.m_cswidth); break;
					case 32:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto32smo, entry.m_mask, entry.m_cswidth); break;
					case 64:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_rproto64smo, entry.m_mask, entry.m_cswidth); break;
				}
			else
				switch (data.m_bits)
				{
					case 8:     install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto8smo, entry.m_mask, entry.m_cswidth); break;
					case 16:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto16smo, entry.m_mask, entry.m_cswidth); break;
					case 32:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto32smo, entry.m_mask, entry.m_cswidth); break;
					case 64:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, entry.m_wproto64smo, entry.m_mask, entry.m_cswidth); break;
				}
			break;

		case AMH_PORT:
			install_readwrite_port(entry.m_addrstart, entry.m_addrend, entry.m_addrmirror,
								   (readorwrite == read_or_write::READ) ? entry.m_devbase.subtag(data.m_tag) : "",
								   (readorwrite == read_or_write::WRITE) ? entry.m_devbase.subtag(data.m_tag) : "");
			break;

		case AMH_BANK:
			install_bank_generic(entry.m_addrstart, entry.m_addrend, entry.m_addrmirror,
								 (readorwrite == read_or_write::READ) ? entry.m_devbase.subtag(data.m_tag) : "",
								 (readorwrite == read_or_write::WRITE) ? entry.m_devbase.subtag(data.m_tag) : "");
			break;

		case AMH_DEVICE_SUBMAP:
			throw emu_fatalerror("Internal mapping error: leftover mapping of '%s'.\n", data.m_tag);
	}
}

//-------------------------------------------------
//  allocate_memory - determine all neighboring
//  address ranges and allocate memory to back
//  them
//-------------------------------------------------

void address_space::allocate_memory()
{
	auto &blocklist = m_manager.m_blocklist;

	// make a first pass over the memory map and track blocks with hardcoded pointers
	// we do this to make sure they are found by space_find_backing_memory first
	// do it back-to-front so that overrides work correctly
	int tail = blocklist.size();
	for (address_map_entry &entry : m_map->m_entrylist)
		if (entry.m_memory != nullptr)
			blocklist.insert(blocklist.begin() + tail, std::make_unique<memory_block>(*this, entry.m_addrstart, entry.m_addrend, entry.m_memory));

	// loop over all blocks just allocated and assign pointers from them
	address_map_entry *unassigned = nullptr;

	for (auto memblock = blocklist.begin() + tail; memblock != blocklist.end(); ++memblock)
		unassigned = block_assign_intersecting(memblock->get()->addrstart(), memblock->get()->addrend(), memblock->get()->data());

	// if we don't have an unassigned pointer yet, try to find one
	if (unassigned == nullptr)
		unassigned = block_assign_intersecting(~0, 0, nullptr);

	// loop until we've assigned all memory in this space
	while (unassigned != nullptr)
	{
		// work in MEMORY_BLOCK_CHUNK-sized chunks
		offs_t curblockstart = unassigned->m_addrstart / MEMORY_BLOCK_CHUNK;
		offs_t curblockend = unassigned->m_addrend / MEMORY_BLOCK_CHUNK;

		// loop while we keep finding unassigned blocks in neighboring MEMORY_BLOCK_CHUNK chunks
		bool changed;
		do
		{
			changed = false;

			// scan for unmapped blocks in the adjusted map
			for (address_map_entry &entry : m_map->m_entrylist)
				if (entry.m_memory == nullptr && &entry != unassigned && needs_backing_store(entry))
				{
					// get block start/end blocks for this block
					offs_t blockstart = entry.m_addrstart / MEMORY_BLOCK_CHUNK;
					offs_t blockend = entry.m_addrend / MEMORY_BLOCK_CHUNK;

					// if we intersect or are adjacent, adjust the start/end
					if (blockstart <= curblockend + 1 && blockend >= curblockstart - 1)
					{
						if (blockstart < curblockstart)
							curblockstart = blockstart, changed = true;
						if (blockend > curblockend)
							curblockend = blockend, changed = true;
					}
				}
		} while (changed);

		// we now have a block to allocate; do it
		offs_t curaddrstart = curblockstart * MEMORY_BLOCK_CHUNK;
		offs_t curaddrend = curblockend * MEMORY_BLOCK_CHUNK + (MEMORY_BLOCK_CHUNK - 1);
		auto block = std::make_unique<memory_block>(*this, curaddrstart, curaddrend);

		// assign memory that intersected the new block
		unassigned = block_assign_intersecting(curaddrstart, curaddrend, block.get()->data());
		blocklist.push_back(std::move(block));
	}
}


//-------------------------------------------------
//  locate_memory - find all the requested
//  pointers into the final allocated memory
//-------------------------------------------------

void address_space::locate_memory()
{
	// once this is done, find the starting bases for the banks
	for (auto &bank : m_manager.banks())
		if (bank.second->base() == nullptr && bank.second->references_space(*this, read_or_write::READWRITE))
		{
			// set the initial bank pointer
			for (address_map_entry &entry : m_map->m_entrylist)
				if (entry.m_addrstart == bank.second->addrstart() && entry.m_memory != nullptr)
				{
					bank.second->set_base(entry.m_memory);
					VPRINTF(("assigned bank '%s' pointer to memory from range %08X-%08X [%p]\n", bank.second->tag(), entry.m_addrstart, entry.m_addrend, entry.m_memory));
					break;
				}
		}
}




//-------------------------------------------------
//  block_assign_intersecting - find all
//  intersecting blocks and assign their pointers
//-------------------------------------------------

address_map_entry *address_space::block_assign_intersecting(offs_t addrstart, offs_t addrend, u8 *base)
{
	address_map_entry *unassigned = nullptr;

	// loop over the adjusted map and assign memory to any blocks we can
	for (address_map_entry &entry : m_map->m_entrylist)
	{
		// if we haven't assigned this block yet, see if we have a mapped shared pointer for it
		if (entry.m_memory == nullptr && entry.m_share != nullptr)
		{
			std::string fulltag = entry.m_devbase.subtag(entry.m_share);
			auto share = m_manager.shares().find(fulltag.c_str());
			if (share != m_manager.shares().end() && share->second->ptr() != nullptr)
			{
				entry.m_memory = share->second->ptr();
				VPRINTF(("memory range %08X-%08X -> shared_ptr '%s' [%p]\n", entry.m_addrstart, entry.m_addrend, entry.m_share, entry.m_memory));
			}
			else
			{
				VPRINTF(("memory range %08X-%08X -> shared_ptr '%s' but not found\n", entry.m_addrstart, entry.m_addrend, entry.m_share));
			}
		}

		// otherwise, look for a match in this block
		if (entry.m_memory == nullptr && entry.m_addrstart >= addrstart && entry.m_addrend <= addrend)
		{
			entry.m_memory = base + m_config.addr2byte(entry.m_addrstart - addrstart);
			VPRINTF(("memory range %08X-%08X -> found in block from %08X-%08X [%p]\n", entry.m_addrstart, entry.m_addrend, addrstart, addrend, entry.m_memory));
		}

		// if we're the first match on a shared pointer, assign it now
		if (entry.m_memory != nullptr && entry.m_share != nullptr)
		{
			std::string fulltag = entry.m_devbase.subtag(entry.m_share);
			auto share = m_manager.shares().find(fulltag.c_str());
			if (share != m_manager.shares().end() && share->second->ptr() == nullptr)
			{
				share->second->set_ptr(entry.m_memory);
				VPRINTF(("setting shared_ptr '%s' = %p\n", entry.m_share, entry.m_memory));
			}
		}

		// keep track of the first unassigned entry
		if (entry.m_memory == nullptr && unassigned == nullptr && needs_backing_store(entry))
			unassigned = &entry;
	}

	return unassigned;
}


memory_passthrough_handler *address_space::install_read_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u8  &data, u8  mem_mask)> tap, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 8-bits wide bus read tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler *address_space::install_read_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u16 &data, u16 mem_mask)> tap, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 16-bits wide bus read tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler *address_space::install_read_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u32 &data, u32 mem_mask)> tap, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 32-bits wide bus read tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler *address_space::install_read_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u64 &data, u64 mem_mask)> tap, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 64-bits wide bus read tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler *address_space::install_write_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u8  &data, u8  mem_mask)> tap, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 8-bits wide bus write tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler *address_space::install_write_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u16 &data, u16 mem_mask)> tap, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 16-bits wide bus write tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler *address_space::install_write_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u32 &data, u32 mem_mask)> tap, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 32-bits wide bus write tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler *address_space::install_write_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u64 &data, u64 mem_mask)> tap, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 64-bits wide bus write tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler *address_space::install_readwrite_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u8  &data, u8  mem_mask)> tapr, std::function<void (offs_t offset, u8  &data, u8  mem_mask)> tapw, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 8-bits wide bus read/write tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler *address_space::install_readwrite_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u16 &data, u16 mem_mask)> tapr, std::function<void (offs_t offset, u16 &data, u16 mem_mask)> tapw, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 16-bits wide bus read/write tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler *address_space::install_readwrite_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u32 &data, u32 mem_mask)> tapr, std::function<void (offs_t offset, u32 &data, u32 mem_mask)> tap, memory_passthrough_handler *mph)
{
	fatalerror("Tryingw to install a 32-bits wide bus read/write tap in a %d-bits wide bus\n", data_width());
}

memory_passthrough_handler *address_space::install_readwrite_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, u64 &data, u64 mem_mask)> tapr, std::function<void (offs_t offset, u64 &data, u64 mem_mask)> tapw, memory_passthrough_handler *mph)
{
	fatalerror("Trying to install a 64-bits wide bus read/write tap in a %d-bits wide bus\n", data_width());
}



//-------------------------------------------------
//  get_handler_string - return a string
//  describing the handler at a particular offset
//-------------------------------------------------

template<int Width, int AddrShift, endianness_t Endian> std::string address_space_specific<Width, AddrShift, Endian>::get_handler_string(read_or_write readorwrite, offs_t address) const
{
	if (readorwrite == read_or_write::READ) {
		offs_t start, end;
		handler_entry_read<Width, AddrShift, Endian> *handler;
		m_root_read->lookup(address, start, end, handler);
		return handler->name();
	} else {
		offs_t start, end;
		handler_entry_write<Width, AddrShift, Endian> *handler;
		m_root_write->lookup(address, start, end, handler);
		return handler->name();
	}
}

template<int Width, int AddrShift, endianness_t Endian> void address_space_specific<Width, AddrShift, Endian>::dump_maps(std::vector<memory_entry> &read_map, std::vector<memory_entry> &write_map) const
{
	read_map.clear();
	write_map.clear();
	m_root_read->dump_map(read_map);
	m_root_write->dump_map(write_map);
}


//**************************************************************************
//  DYNAMIC ADDRESS SPACE MAPPING
//**************************************************************************

//-------------------------------------------------
//  unmap - unmap a section of address space
//-------------------------------------------------

template<int Width, int AddrShift, endianness_t Endian> void address_space_specific<Width, AddrShift, Endian>::unmap_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, read_or_write readorwrite, bool quiet)
{
	VPRINTF(("address_space::unmap(%s-%s mirror=%s, %s, %s)\n",
				core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
				core_i64_hex_format(addrmirror, m_addrchars),
				(readorwrite == read_or_write::READ) ? "read" : (readorwrite == read_or_write::WRITE) ? "write" : (readorwrite == read_or_write::READWRITE) ? "read/write" : "??",
				quiet ? "quiet" : "normal"));

	offs_t nstart, nend, nmask, nmirror;
	check_optimize_mirror("unmap_generic", addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);

	// read space
	if (readorwrite == read_or_write::READ || readorwrite == read_or_write::READWRITE) {
		auto handler = static_cast<handler_entry_read<Width, AddrShift, Endian> *>(quiet ? m_nop_r : m_unmap_r);
		handler->ref();
		m_root_read->populate(nstart, nend, nmirror, handler);
	}

	// write space
	if (readorwrite == read_or_write::WRITE || readorwrite == read_or_write::READWRITE) {
		auto handler = static_cast<handler_entry_write<Width, AddrShift, Endian> *>(quiet ? m_nop_w : m_unmap_w);
		handler->ref();
		m_root_write->populate(nstart, nend, nmirror, handler);
	}

	invalidate_caches(readorwrite);
}

//-------------------------------------------------
//  install_read_tap - install a read tap on the bus
//-------------------------------------------------

template<int Width, int AddrShift, endianness_t Endian> memory_passthrough_handler *address_space_specific<Width, AddrShift, Endian>::install_read_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, uX &data, uX mem_mask)> tap, memory_passthrough_handler *mph)
{
	offs_t nstart, nend, nmask, nmirror;
	check_optimize_mirror("install_read_tap", addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);
	if(!mph) {
		m_mphs.emplace_back(std::make_unique<memory_passthrough_handler>(*this));
		mph = m_mphs.back().get();
	}

	auto handler = new handler_entry_read_tap<Width, AddrShift, Endian>(this, *mph, name, tap);
	m_root_read->populate_passthrough(nstart, nend, nmirror, handler);
	handler->unref();

	invalidate_caches(read_or_write::READ);

	return mph;
}

//-------------------------------------------------
//  install_write_tap - install a write tap on the bus
//-------------------------------------------------

template<int Width, int AddrShift, endianness_t Endian> memory_passthrough_handler *address_space_specific<Width, AddrShift, Endian>::install_write_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, uX &data, uX mem_mask)> tap, memory_passthrough_handler *mph)
{
	offs_t nstart, nend, nmask, nmirror;
	check_optimize_mirror("install_write_tap", addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);
	if(!mph) {
		m_mphs.emplace_back(std::make_unique<memory_passthrough_handler>(*this));
		mph = m_mphs.back().get();
	}

	auto handler = new handler_entry_write_tap<Width, AddrShift, Endian>(this, *mph, name, tap);
	m_root_write->populate_passthrough(nstart, nend, nmirror, handler);
	handler->unref();

	invalidate_caches(read_or_write::WRITE);

	return mph;
}
//-------------------------------------------------
//  install_write_tap - install a read and a write tap on the bus
//-------------------------------------------------

template<int Width, int AddrShift, endianness_t Endian> memory_passthrough_handler *address_space_specific<Width, AddrShift, Endian>::install_readwrite_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, uX &data, uX mem_mask)> tapr, std::function<void (offs_t offset, uX &data, uX mem_mask)> tapw, memory_passthrough_handler *mph)
{
	offs_t nstart, nend, nmask, nmirror;
	check_optimize_mirror("install_readwrite_tap", addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);
	if(!mph) {
		m_mphs.emplace_back(std::make_unique<memory_passthrough_handler>(*this));
		mph = m_mphs.back().get();
	}

	auto rhandler = new handler_entry_read_tap <Width, AddrShift, Endian>(this, *mph, name, tapr);
	m_root_read ->populate_passthrough(nstart, nend, nmirror, rhandler);
	rhandler->unref();

	auto whandler = new handler_entry_write_tap<Width, AddrShift, Endian>(this, *mph, name, tapw);
	m_root_write->populate_passthrough(nstart, nend, nmirror, whandler);
	whandler->unref();

	invalidate_caches(read_or_write::READWRITE);

	return mph;
}





//-------------------------------------------------
//  install_device_delegate - install the memory map
//  of a live device into this address space
//-------------------------------------------------

template<int Width, int AddrShift, endianness_t Endian> void address_space_specific<Width, AddrShift, Endian>::install_device_delegate(offs_t addrstart, offs_t addrend, device_t &device, address_map_constructor &delegate, u64 unitmask, int cswidth)
{
	check_address("install_device_delegate", addrstart, addrend);
	address_map map(*this, addrstart, addrend, unitmask, cswidth, m_device, delegate);
	map.import_submaps(m_manager.machine(), device, data_width(), endianness(), addr_shift());
	populate_from_map(&map);
}



//-------------------------------------------------
//  install_readwrite_port - install a new I/O port
//  handler into this address space
//-------------------------------------------------

template<int Width, int AddrShift, endianness_t Endian> void address_space_specific<Width, AddrShift, Endian>::install_readwrite_port(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string rtag, std::string wtag)
{
	VPRINTF(("address_space::install_readwrite_port(%s-%s mirror=%s, read=\"%s\" / write=\"%s\")\n",
				core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
				core_i64_hex_format(addrmirror, m_addrchars),
				rtag.empty() ? "(none)" : rtag.c_str(), wtag.empty() ? "(none)" : wtag.c_str()));

	offs_t nstart, nend, nmask, nmirror;
	check_optimize_mirror("install_readwrite_port", addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);

	// read handler
	if (rtag != "")
	{
		// find the port
		ioport_port *port = device().owner()->ioport(rtag);
		if (port == nullptr)
			throw emu_fatalerror("Attempted to map non-existent port '%s' for read in space %s of device '%s'\n", rtag.c_str(), m_name, m_device.tag());

		// map the range and set the ioport
		auto hand_r = new handler_entry_read_ioport<Width, AddrShift, Endian>(this, port);
		m_root_read->populate(nstart, nend, nmirror, hand_r);
	}

	if (wtag != "")
	{
		// find the port
		ioport_port *port = device().owner()->ioport(wtag);
		if (port == nullptr)
			fatalerror("Attempted to map non-existent port '%s' for write in space %s of device '%s'\n", wtag.c_str(), m_name, m_device.tag());

		// map the range and set the ioport
		auto hand_w = new handler_entry_write_ioport<Width, AddrShift, Endian>(this, port);
		m_root_write->populate(nstart, nend, nmirror, hand_w);
	}

	invalidate_caches(rtag != "" ? wtag != "" ? read_or_write::READWRITE : read_or_write::READ : read_or_write::WRITE);
}


//-------------------------------------------------
//  install_bank_generic - install a range as
//  mapping to a particular bank
//-------------------------------------------------

template<int Width, int AddrShift, endianness_t Endian> void address_space_specific<Width, AddrShift, Endian>::install_bank_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string rtag, std::string wtag)
{
	VPRINTF(("address_space::install_readwrite_bank(%s-%s mirror=%s, read=\"%s\" / write=\"%s\")\n",
				core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
				core_i64_hex_format(addrmirror, m_addrchars),
				rtag.empty() ? "(none)" : rtag.c_str(), wtag.empty() ? "(none)" : wtag.c_str()));

	offs_t nstart, nend, nmask, nmirror;
	check_optimize_mirror("install_bank_generic", addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);

	// map the read bank
	if (rtag != "")
	{
		std::string fulltag = device().siblingtag(rtag);
		memory_bank &bank = bank_find_or_allocate(fulltag.c_str(), addrstart, addrend, addrmirror, read_or_write::READ);

		auto hand_r = new handler_entry_read_memory_bank<Width, AddrShift, Endian>(this, bank);
		hand_r->set_address_info(nstart, nmask);
		m_root_read->populate(nstart, nend, nmirror, hand_r);
	}

	// map the write bank
	if (wtag != "")
	{
		std::string fulltag = device().siblingtag(wtag);
		memory_bank &bank = bank_find_or_allocate(fulltag.c_str(), addrstart, addrend, addrmirror, read_or_write::WRITE);

		auto hand_w = new handler_entry_write_memory_bank<Width, AddrShift, Endian>(this, bank);
		hand_w->set_address_info(nstart, nmask);
		m_root_write->populate(nstart, nend, nmirror, hand_w);
	}

	invalidate_caches(rtag != "" ? wtag != "" ? read_or_write::READWRITE : read_or_write::READ : read_or_write::WRITE);
}


template<int Width, int AddrShift, endianness_t Endian> void address_space_specific<Width, AddrShift, Endian>::install_bank_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, memory_bank *rbank, memory_bank *wbank)
{
	VPRINTF(("address_space::install_readwrite_bank(%s-%s mirror=%s, read=\"%s\" / write=\"%s\")\n",
				core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
				core_i64_hex_format(addrmirror, m_addrchars),
				(rbank != nullptr) ? rbank->tag() : "(none)", (wbank != nullptr) ? wbank->tag() : "(none)"));

	offs_t nstart, nend, nmask, nmirror;
	check_optimize_mirror("install_bank_generic", addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);

	// map the read bank
	if (rbank != nullptr)
	{
		auto hand_r = new handler_entry_read_memory_bank<Width, AddrShift, Endian>(this, *rbank);
		hand_r->set_address_info(nstart, nmask);
		m_root_read->populate(nstart, nend, nmirror, hand_r);
	}

	// map the write bank
	if (wbank != nullptr)
	{
		auto hand_w = new handler_entry_write_memory_bank<Width, AddrShift, Endian>(this, *wbank);
		hand_w->set_address_info(nstart, nmask);
		m_root_write->populate(nstart, nend, nmirror, hand_w);
	}

	invalidate_caches(rbank ? wbank ? read_or_write::READWRITE : read_or_write::READ : read_or_write::WRITE);
}


//-------------------------------------------------
//  install_ram_generic - install a simple fixed
//  RAM region into the given address space
//-------------------------------------------------

template<int Width, int AddrShift, endianness_t Endian> void address_space_specific<Width, AddrShift, Endian>::install_ram_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, read_or_write readorwrite, void *baseptr)
{
	VPRINTF(("address_space::install_ram_generic(%s-%s mirror=%s, %s, %p)\n",
				core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
				core_i64_hex_format(addrmirror, m_addrchars),
				(readorwrite == read_or_write::READ) ? "read" : (readorwrite == read_or_write::WRITE) ? "write" : (readorwrite == read_or_write::READWRITE) ? "read/write" : "??",
				baseptr));

	offs_t nstart, nend, nmask, nmirror;
	check_optimize_mirror("install_ram_generic", addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);

	// map for read
	if (readorwrite == read_or_write::READ || readorwrite == read_or_write::READWRITE)
	{
		// find a bank and map it
		memory_bank &bank = bank_find_or_allocate(nullptr, addrstart, addrend, addrmirror, read_or_write::READ);

		// if we are provided a pointer, set it
		if (baseptr != nullptr)
			bank.set_base(baseptr);

		// if we don't have a bank pointer yet, try to find one
		if (bank.base() == nullptr)
		{
			void *backing = find_backing_memory(addrstart, addrend);
			if (backing != nullptr)
				bank.set_base(backing);
		}

		// if we still don't have a pointer, and we're past the initialization phase, allocate a new block
		if (bank.base() == nullptr && m_manager.m_initialized)
		{
			if (m_manager.machine().phase() >= machine_phase::RESET)
				fatalerror("Attempted to call install_ram_generic() after initialization time without a baseptr!\n");
			auto block = std::make_unique<memory_block>(*this, addrstart, addrend);
			bank.set_base(block.get()->data());
			m_manager.m_blocklist.push_back(std::move(block));
		}

		auto hand_r = new handler_entry_read_memory<Width, AddrShift, Endian>(this);
		if (bank.base())
			hand_r->set_base(static_cast<uX *>(bank.base()));
		else {
			delayed_ref(hand_r);
			bank.add_notifier([this, hand_r](void *base) { hand_r->set_base(static_cast<uX *>(base)); delayed_unref(hand_r); });
		}

		hand_r->set_address_info(nstart, nmask);
		m_root_read->populate(nstart, nend, nmirror, hand_r);
	}

	// map for write
	if (readorwrite == read_or_write::WRITE || readorwrite == read_or_write::READWRITE)
	{
		// find a bank and map it
		memory_bank &bank = bank_find_or_allocate(nullptr, addrstart, addrend, addrmirror, read_or_write::WRITE);

		// if we are provided a pointer, set it
		if (baseptr != nullptr)
			bank.set_base(baseptr);

		// if we don't have a bank pointer yet, try to find one
		if (bank.base() == nullptr)
		{
			void *backing = find_backing_memory(addrstart, addrend);
			if (backing != nullptr)
				bank.set_base(backing);
		}

		// if we still don't have a pointer, and we're past the initialization phase, allocate a new block
		if (bank.base() == nullptr && m_manager.m_initialized)
		{
			if (m_manager.machine().phase() >= machine_phase::RESET)
				fatalerror("Attempted to call install_ram_generic() after initialization time without a baseptr!\n");
			auto block = std::make_unique<memory_block>(*this, address_to_byte(addrstart), address_to_byte_end(addrend));
			bank.set_base(block.get()->data());
			m_manager.m_blocklist.push_back(std::move(block));
		}

		auto hand_w = new handler_entry_write_memory<Width, AddrShift, Endian>(this);
		if (bank.base())
			hand_w->set_base(static_cast<uX *>(bank.base()));
		else {
			delayed_ref(hand_w);
			bank.add_notifier([this, hand_w](void *base) { hand_w->set_base(static_cast<uX *>(base)); delayed_unref(hand_w); });
		}

		hand_w->set_address_info(nstart, nmask);
		m_root_write->populate(nstart, nend, nmirror, hand_w);
	}

	invalidate_caches(readorwrite);
}


//**************************************************************************
//  MEMORY MAPPING HELPERS
//**************************************************************************

//-------------------------------------------------
//  find_backing_memory - return a pointer to
//  the base of RAM associated with the given
//  device and offset
//-------------------------------------------------

void *address_space::find_backing_memory(offs_t addrstart, offs_t addrend)
{
	VPRINTF(("address_space::find_backing_memory('%s',%s,%08X-%08X) -> ", m_device.tag(), m_name, addrstart, addrend));

	if (m_map == nullptr)
		return nullptr;

	// look in the address map first, last winning for overrides
	void *result = nullptr;
	for (address_map_entry &entry : m_map->m_entrylist)
	{
		if (entry.m_memory != nullptr && addrstart >= entry.m_addrstart && addrend <= entry.m_addrend)
		{
			VPRINTF(("found in entry %08X-%08X [%p]\n", entry.m_addrstart, entry.m_addrend, (u8 *)entry.m_memory + address_to_byte(addrstart - entry.m_addrstart)));
			result = (u8 *)entry.m_memory + address_to_byte(addrstart - entry.m_addrstart);
		}
	}
	if (result)
		return result;

	// if not found there, look in the allocated blocks
	for (auto &block : m_manager.m_blocklist)
		if (block->contains(*this, addrstart, addrend))
		{
			VPRINTF(("found in allocated memory block %08X-%08X [%p]\n", block->addrstart(), block->addrend(), block->data() + address_to_byte(addrstart - block->addrstart())));
			return block->data() + address_to_byte(addrstart - block->addrstart());
		}

	VPRINTF(("did not find\n"));
	return nullptr;
}


//-------------------------------------------------
//  space_needs_backing_store - return whether a
//  given memory map entry implies the need of
//  allocating and registering memory
//-------------------------------------------------

bool address_space::needs_backing_store(const address_map_entry &entry)
{
	// if we are sharing, and we don't have a pointer yet, create one
	if (entry.m_share != nullptr)
	{
		std::string fulltag = entry.m_devbase.subtag(entry.m_share);
		auto share = m_manager.shares().find(fulltag.c_str());
		if (share != m_manager.shares().end() && share->second->ptr() == nullptr)
			return true;
	}

	// if we're writing to any sort of bank or RAM, then yes, we do need backing
	if (entry.m_write.m_type == AMH_BANK || entry.m_write.m_type == AMH_RAM)
		return true;

	// if we're reading from RAM or from ROM outside of address space 0 or its region, then yes, we do need backing
	memory_region *region = m_manager.machine().root_device().memregion(m_device.tag());
	if (entry.m_read.m_type == AMH_RAM ||
		(entry.m_read.m_type == AMH_ROM && (m_spacenum != 0 || region == nullptr || entry.m_addrstart >= region->bytes())))
		return true;

	// all other cases don't need backing
	return false;
}

int address_space::add_change_notifier(std::function<void (read_or_write)> n)
{
	int id = m_notifier_id++;
	m_notifiers.emplace_back(notifier_t{ n, id });
	return id;
}

void address_space::remove_change_notifier(int id)
{
	for(auto i = m_notifiers.begin(); i != m_notifiers.end(); i++)
		if(i->m_id == id) {
			m_notifiers.erase(i);
			return;
		}
	fatalerror("Unknown notifier id %d, double remove?\n", id);
}


//**************************************************************************
//  BANKING HELPERS
//**************************************************************************

//-------------------------------------------------
//  bank_find_or_allocate - allocate a new
//  bank, or find an existing one, and return the
//  read/write handler
//-------------------------------------------------

memory_bank &address_space::bank_find_or_allocate(const char *tag, offs_t addrstart, offs_t addrend, offs_t addrmirror, read_or_write readorwrite)
{
	// adjust the addresses, handling mirrors and such
	offs_t addrmask = ~addrmirror;
	adjust_addresses(addrstart, addrend, addrmask, addrmirror);

	// look up the bank by name, or else by byte range
	memory_bank *membank = tag ? m_manager.find(tag) : m_manager.find(*this, addrstart, addrend);

	// if we don't have a bank yet, create a new one
	if (!membank)
		membank = m_manager.allocate(*this, addrstart, addrend, tag);

	// add a reference for this space
	membank->add_reference(*this, readorwrite);
	return *membank;
}


//**************************************************************************
//  CACHE MEMORY RANGES
//**************************************************************************

//-------------------------------------------------
//  memory_access_cache - constructor
//-------------------------------------------------

template<int Width, int AddrShift, int Endian> memory_access_cache<Width, AddrShift, Endian>::memory_access_cache(address_space &space,
																												  handler_entry_read <Width, AddrShift, Endian> *root_read,
																												  handler_entry_write<Width, AddrShift, Endian> *root_write)
	: m_space(space),
	  m_addrmask(space.addrmask()),
	  m_addrstart_r(1),
	  m_addrend_r(0),
	  m_addrstart_w(1),
	  m_addrend_w(0),
	  m_cache_r(nullptr),
	  m_cache_w(nullptr),
	  m_root_read(root_read),
	  m_root_write(root_write)
{
	m_notifier_id = space.add_change_notifier([this](read_or_write mode) {
												  if(u32(mode) & u32(read_or_write::READ)) {
													  m_addrend_r = 0;
													  m_addrstart_r = 1;
													  m_cache_r = nullptr;
												  }
												  if(u32(mode) & u32(read_or_write::WRITE)) {
													  m_addrend_w = 0;
													  m_addrstart_w = 1;
													  m_cache_w = nullptr;
												  }
											  });
}


//-------------------------------------------------
//  ~memory_access_cache - destructor
//-------------------------------------------------

template<int Width, int AddrShift, int Endian> memory_access_cache<Width, AddrShift, Endian>::~memory_access_cache()
{
	m_space.remove_change_notifier(m_notifier_id);
}


template class memory_access_cache<0,  1, ENDIANNESS_LITTLE>;
template class memory_access_cache<0,  1, ENDIANNESS_BIG>;
template class memory_access_cache<0,  0, ENDIANNESS_LITTLE>;
template class memory_access_cache<0,  0, ENDIANNESS_BIG>;
template class memory_access_cache<1,  3, ENDIANNESS_LITTLE>;
template class memory_access_cache<1,  3, ENDIANNESS_BIG>;
template class memory_access_cache<1,  0, ENDIANNESS_LITTLE>;
template class memory_access_cache<1,  0, ENDIANNESS_BIG>;
template class memory_access_cache<1, -1, ENDIANNESS_LITTLE>;
template class memory_access_cache<1, -1, ENDIANNESS_BIG>;
template class memory_access_cache<2,  0, ENDIANNESS_LITTLE>;
template class memory_access_cache<2,  0, ENDIANNESS_BIG>;
template class memory_access_cache<2, -1, ENDIANNESS_LITTLE>;
template class memory_access_cache<2, -1, ENDIANNESS_BIG>;
template class memory_access_cache<2, -2, ENDIANNESS_LITTLE>;
template class memory_access_cache<2, -2, ENDIANNESS_BIG>;
template class memory_access_cache<3,  0, ENDIANNESS_LITTLE>;
template class memory_access_cache<3,  0, ENDIANNESS_BIG>;
template class memory_access_cache<3, -1, ENDIANNESS_LITTLE>;
template class memory_access_cache<3, -1, ENDIANNESS_BIG>;
template class memory_access_cache<3, -2, ENDIANNESS_LITTLE>;
template class memory_access_cache<3, -2, ENDIANNESS_BIG>;
template class memory_access_cache<3, -3, ENDIANNESS_LITTLE>;
template class memory_access_cache<3, -3, ENDIANNESS_BIG>;



//**************************************************************************
//  MEMORY BLOCK
//**************************************************************************

//-------------------------------------------------
//  memory_block - constructor
//-------------------------------------------------

memory_block::memory_block(address_space &space, offs_t addrstart, offs_t addrend, void *memory)
	: m_machine(space.m_manager.machine()),
		m_space(space),
		m_addrstart(addrstart),
		m_addrend(addrend),
		m_data(reinterpret_cast<u8 *>(memory))
{
	offs_t const length = space.address_to_byte(addrend + 1 - addrstart);
	VPRINTF(("block_allocate('%s',%s,%08X,%08X,%p)\n", space.device().tag(), space.name(), addrstart, addrend, memory));

	// allocate a block if needed
	if (m_data == nullptr)
	{
		if (length < 4096)
		{
			m_allocated.resize(length);
			memset(&m_allocated[0], 0, length);
			m_data = &m_allocated[0];
		}
		else
		{
			m_allocated.resize(length + 0xfff);
			memset(&m_allocated[0], 0, length + 0xfff);
			m_data = reinterpret_cast<u8 *>((reinterpret_cast<uintptr_t>(&m_allocated[0]) + 0xfff) & ~0xfff);
		}
	}

	// register for saving, but only if we're not part of a memory region
	if (space.m_manager.region_containing(m_data, length) != nullptr)
		VPRINTF(("skipping save of this memory block as it is covered by a memory region\n"));
	else
	{
		int bytes_per_element = space.data_width() / 8;
		std::string name = string_format("%08x-%08x", addrstart, addrend);
		machine().save().save_memory(&space.device(), "memory", space.device().tag(), space.spacenum(), name.c_str(), m_data, bytes_per_element, (u32)length / bytes_per_element);
	}
}


//-------------------------------------------------
//  memory_block - destructor
//-------------------------------------------------

memory_block::~memory_block()
{
}



//**************************************************************************
//  MEMORY BANK
//**************************************************************************

//-------------------------------------------------
//  memory_bank - constructor
//-------------------------------------------------

memory_bank::memory_bank(address_space &space, int index, offs_t addrstart, offs_t addrend, const char *tag)
	: m_machine(space.m_manager.machine()),
	  m_anonymous(tag == nullptr),
	  m_addrstart(addrstart),
	  m_addrend(addrend),
	  m_curentry(0)
{
	// generate an internal tag if we don't have one
	if (tag == nullptr)
	{
		m_tag = string_format("~%d~", index);
		m_name = string_format("Internal bank #%d", index);
	}
	else
	{
		m_tag = tag;
		m_name = string_format("Bank '%s'", tag);
	}

	if (!m_anonymous && machine().save().registration_allowed())
		machine().save().save_item(&space.device(), "memory", m_tag.c_str(), 0, NAME(m_curentry));
}


//-------------------------------------------------
//  memory_bank - destructor
//-------------------------------------------------

memory_bank::~memory_bank()
{
}


//-------------------------------------------------
//  references_space - walk the list of references
//  to find a match against the provided space
//  and read/write
//-------------------------------------------------

bool memory_bank::references_space(const address_space &space, read_or_write readorwrite) const
{
	for (auto &ref : m_reflist)
		if (ref->matches(space, readorwrite))
			return true;
	return false;
}


//-------------------------------------------------
//  add_reference - add a new reference to the
//  given space
//-------------------------------------------------

void memory_bank::add_reference(address_space &space, read_or_write readorwrite)
{
	// if we already have a reference, skip it
	if (references_space(space, readorwrite))
		return;
	m_reflist.push_back(std::make_unique<bank_reference>(space, readorwrite));
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
	for(auto cb : m_alloc_notifier)
		cb(base);
	m_alloc_notifier.clear();
}


//-------------------------------------------------
//  add_notifier - add a function used to notify when the allocation is done
//-------------------------------------------------
void memory_bank::add_notifier(std::function<void (void *)> cb)
{
	m_alloc_notifier.emplace_back(cb);
}

//-------------------------------------------------
//  set_entry - set the base to a pre-configured
//  entry
//-------------------------------------------------

void memory_bank::set_entry(int entrynum)
{
	// validate
	if (m_anonymous)
		throw emu_fatalerror("memory_bank::set_entry called for anonymous bank");
	if (entrynum < 0 || entrynum >= int(m_entries.size()))
		throw emu_fatalerror("memory_bank::set_entry called with out-of-range entry %d", entrynum);
	if (m_entries[entrynum] == nullptr)
		throw emu_fatalerror("memory_bank::set_entry called for bank '%s' with invalid bank entry %d", m_tag.c_str(), entrynum);

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
		m_entries[entrynum + startentry] = reinterpret_cast<u8 *>(base) +  entrynum * stride ;
}


//**************************************************************************
//  MEMORY REGIONS
//**************************************************************************

//-------------------------------------------------
//  memory_region - constructor
//-------------------------------------------------

memory_region::memory_region(running_machine &machine, const char *name, u32 length, u8 width, endianness_t endian)
	: m_machine(machine),
		m_name(name),
		m_buffer(length),
		m_endianness(endian),
		m_bitwidth(width * 8),
		m_bytewidth(width)
{
	assert(width == 1 || width == 2 || width == 4 || width == 8);
}
