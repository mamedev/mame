// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Olivier Galibert
/***************************************************************************

    emumem.cpp

    Functions which handle device memory access.
    Memory view specific functions

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

#define VERBOSE 0

#if VERBOSE
template <typename Format, typename... Params> static void VPRINTF(Format &&fmt, Params &&...args)
{
	util::stream_format(std::cerr, std::forward<Format>(fmt), std::forward<Params>(args)...);
}
#else
template <typename Format, typename... Params> static void VPRINTF(Format &&, Params &&...) {}
#endif

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
} // anonymous namespace

address_map_entry &memory_view::memory_view_entry::operator()(offs_t start, offs_t end)
{
	return (*m_map)(start, end);
}

template<int Level, int Width, int AddrShift>
class memory_view_entry_specific : public memory_view::memory_view_entry
{
	using uX = typename emu::detail::handler_entry_size<Width>::uX;
	using NativeType = uX;

	// constants describing the native size
	static constexpr u32 NATIVE_BYTES = 1 << Width;
	static constexpr u32 NATIVE_STEP = AddrShift >= 0 ? NATIVE_BYTES << iabs(AddrShift) : NATIVE_BYTES >> iabs(AddrShift);
	static constexpr u32 NATIVE_MASK = NATIVE_STEP - 1;
	static constexpr u32 NATIVE_BITS = 8 * NATIVE_BYTES;

	static constexpr offs_t offset_to_byte(offs_t offset) { return AddrShift < 0 ? offset << iabs(AddrShift) : offset >> iabs(AddrShift); }

public:
	memory_view_entry_specific(const address_space_config &config, memory_manager &manager, memory_view &view, int id) : memory_view_entry(config, manager, view, id) {
	}

	virtual ~memory_view_entry_specific() = default;

	handler_entry_read <Width, AddrShift> *r() { return static_cast<handler_entry_read <Width, AddrShift> *>(m_view.m_handler_read); }
	handler_entry_write<Width, AddrShift> *w() { return static_cast<handler_entry_write<Width, AddrShift> *>(m_view.m_handler_write); }

	void invalidate_caches(read_or_write readorwrite) { return m_view.m_space->invalidate_caches(readorwrite); }

	virtual void populate_from_map(address_map *map = nullptr) override;

	using address_space_installer::install_read_tap;
	using address_space_installer::install_write_tap;
	using address_space_installer::install_readwrite_tap;

	virtual memory_passthrough_handler install_read_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, uX &data, uX mem_mask)> tap, memory_passthrough_handler *mph) override;
	virtual memory_passthrough_handler install_write_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, uX &data, uX mem_mask)> tap, memory_passthrough_handler *mph) override;
	virtual memory_passthrough_handler install_readwrite_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, uX &data, uX mem_mask)> tapr, std::function<void (offs_t offset, uX &data, uX mem_mask)> tapw, memory_passthrough_handler *mph) override;

	virtual void unmap_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, u16 flags, read_or_write readorwrite, bool quiet) override;
	virtual void install_ram_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, u16 flags, read_or_write readorwrite, void *baseptr) override;
	virtual void install_bank_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, u16 flags, memory_bank *rbank, memory_bank *wbank) override;
	virtual void install_view(offs_t addrstart, offs_t addrend, offs_t addrmirror, memory_view &view) override;
	virtual void install_readwrite_port(offs_t addrstart, offs_t addrend, offs_t addrmirror, u16 flags, std::string rtag, std::string wtag) override;
	virtual void install_device_delegate(offs_t addrstart, offs_t addrend, device_t &device, address_map_constructor &map, u64 unitmask, int cswidth, u16 flags) override;


	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write8_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8_delegate rhandler, write8_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write16_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16_delegate rhandler, write16_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write32_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32_delegate rhandler, write32_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write64_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64_delegate rhandler, write64_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }

	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8m_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write8m_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8m_delegate rhandler, write8m_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16m_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write16m_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16m_delegate rhandler, write16m_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32m_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write32m_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32m_delegate rhandler, write32m_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64m_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write64m_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64m_delegate rhandler, write64m_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }

	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8s_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write8s_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8s_delegate rhandler, write8s_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16s_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write16s_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16s_delegate rhandler, write16s_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32s_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write32s_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32s_delegate rhandler, write32s_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64s_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write64s_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64s_delegate rhandler, write64s_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }

	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8sm_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write8sm_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8sm_delegate rhandler, write8sm_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16sm_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write16sm_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16sm_delegate rhandler, write16sm_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32sm_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write32sm_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32sm_delegate rhandler, write32sm_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64sm_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write64sm_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64sm_delegate rhandler, write64sm_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }

	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8mo_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write8mo_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8mo_delegate rhandler, write8mo_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16mo_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write16mo_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16mo_delegate rhandler, write16mo_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32mo_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write32mo_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32mo_delegate rhandler, write32mo_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64mo_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write64mo_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64mo_delegate rhandler, write64mo_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }

	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8smo_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write8smo_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8smo_delegate rhandler, write8smo_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16smo_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write16smo_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16smo_delegate rhandler, write16smo_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32smo_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write32smo_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32smo_delegate rhandler, write32smo_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64smo_delegate rhandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_read_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler); }
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write64smo_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_write_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, whandler); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64smo_delegate rhandler, write64smo_delegate whandler, u64 unitmask = 0, int cswidth = 0, u16 flags = 0) override
	{ install_readwrite_handler_impl(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, rhandler, whandler); }

	template<typename READ>
	void install_read_handler_impl(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth, u16 flags, READ &handler_r)
	{
		try { handler_r.resolve(); }
		catch (const binding_type_exception &) {
			osd_printf_error("Binding error while installing read handler %s for range 0x%X-0x%X mask 0x%X mirror 0x%X select 0x%X umask 0x%X\n", handler_r.name(), addrstart, addrend, addrmask, addrmirror, addrselect, unitmask);
			throw;
		}
		install_read_handler_helper<handler_width<READ>::value>(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, handler_r);
	}

	template<typename WRITE>
	void install_write_handler_impl(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth, u16 flags, WRITE &handler_w)
	{
		try { handler_w.resolve(); }
		catch (const binding_type_exception &) {
			osd_printf_error("Binding error while installing write handler %s for range 0x%X-0x%X mask 0x%X mirror 0x%X select 0x%X umask 0x%X\n", handler_w.name(), addrstart, addrend, addrmask, addrmirror, addrselect, unitmask);
			throw;
		}
		install_write_handler_helper<handler_width<WRITE>::value>(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, handler_w);
	}

	template<typename READ, typename WRITE>
	void install_readwrite_handler_impl(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth, u16 flags, READ &handler_r, WRITE &handler_w)
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
		install_readwrite_handler_helper<handler_width<READ>::value>(addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, flags, handler_r, handler_w);
	}

	template<int AccessWidth, typename READ>
	void install_read_handler_helper(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth, u16 flags, const READ &handler_r)
	{
		if constexpr (Width < AccessWidth) {
			fatalerror("install_read_handler: cannot install a %d-wide handler in a %d-wide bus", 8 << AccessWidth, 8 << Width);
		} else {
			VPRINTF("memory_view::install_read_handler(%*x-%*x mask=%*x mirror=%*x, space width=%d, handler width=%d, %s, %*x)\n",
					m_addrchars, addrstart, m_addrchars, addrend,
					m_addrchars, addrmask, m_addrchars, addrmirror,
					8 << Width, 8 << AccessWidth,
					handler_r.name(), data_width() / 4, unitmask);

			offs_t nstart, nend, nmask, nmirror;
			u64 nunitmask;
			int ncswidth;
			check_optimize_all("install_read_handler", 8 << AccessWidth, addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, nstart, nend, nmask, nmirror, nunitmask, ncswidth);

			if constexpr (Width == AccessWidth) {
				auto hand_r = new handler_entry_read_delegate<Width, AddrShift, READ>(m_view.m_space, flags, handler_r);
				hand_r->set_address_info(nstart, nmask);
				r()->populate(nstart, nend, nmirror, hand_r);
			} else {
				auto hand_r = new handler_entry_read_delegate<AccessWidth, -AccessWidth, READ>(m_view.m_space, flags, handler_r);
				memory_units_descriptor<Width, AddrShift> descriptor(AccessWidth, endianness(), hand_r, nstart, nend, nmask, nunitmask, ncswidth);
				hand_r->set_address_info(descriptor.get_handler_start(), descriptor.get_handler_mask());
				r()->populate_mismatched(nstart, nend, nmirror, descriptor);
				hand_r->unref();
			}
			invalidate_caches(read_or_write::READ);
		}
	}

	template<int AccessWidth, typename WRITE>
	void install_write_handler_helper(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth, u16 flags,
								 const WRITE &handler_w)
	{
		if constexpr (Width < AccessWidth) {
			fatalerror("install_write_handler: cannot install a %d-wide handler in a %d-wide bus", 8 << AccessWidth, 8 << Width);
		} else {
			VPRINTF("memory_view::install_write_handler(%*x-%*x mask=%*x mirror=%*x, space width=%d, handler width=%d, %s, %*x)\n",
					m_addrchars, addrstart, m_addrchars, addrend,
					m_addrchars, addrmask, m_addrchars, addrmirror,
					8 << Width, 8 << AccessWidth,
					handler_w.name(), data_width() / 4, unitmask);

			offs_t nstart, nend, nmask, nmirror;
			u64 nunitmask;
			int ncswidth;
			check_optimize_all("install_write_handler", 8 << AccessWidth, addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, nstart, nend, nmask, nmirror, nunitmask, ncswidth);

			if constexpr (Width == AccessWidth) {
				auto hand_w = new handler_entry_write_delegate<Width, AddrShift, WRITE>(m_view.m_space, flags, handler_w);
				hand_w->set_address_info(nstart, nmask);
				w()->populate(nstart, nend, nmirror, hand_w);
			} else {
				auto hand_w = new handler_entry_write_delegate<AccessWidth, -AccessWidth, WRITE>(m_view.m_space, flags, handler_w);
				memory_units_descriptor<Width, AddrShift> descriptor(AccessWidth, endianness(), hand_w, nstart, nend, nmask, nunitmask, ncswidth);
				hand_w->set_address_info(descriptor.get_handler_start(), descriptor.get_handler_mask());
				w()->populate_mismatched(nstart, nend, nmirror, descriptor);
				hand_w->unref();
			}
			invalidate_caches(read_or_write::WRITE);
		}
	}

	template<int AccessWidth, typename READ, typename WRITE>
	void install_readwrite_handler_helper(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth, u16 flags,
									 const READ  &handler_r,
									 const WRITE &handler_w)
	{
		if constexpr (Width < AccessWidth) {
			fatalerror("install_readwrite_handler: cannot install a %d-wide handler in a %d-wide bus", 8 << AccessWidth, 8 << Width);
		} else {
			VPRINTF("memory_view::install_readwrite_handler(%*x-%*x mask=%*x mirror=%*x, space width=%d, handler width=%d, %s, %s, %*x)\n",
					m_addrchars, addrstart, m_addrchars, addrend,
					m_addrchars, addrmask, m_addrchars, addrmirror,
					8 << Width, 8 << AccessWidth,
					handler_r.name(), handler_w.name(), data_width() / 4, unitmask);

			offs_t nstart, nend, nmask, nmirror;
			u64 nunitmask;
			int ncswidth;
			check_optimize_all("install_readwrite_handler", 8 << AccessWidth, addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, nstart, nend, nmask, nmirror, nunitmask, ncswidth);

			if constexpr (Width == AccessWidth) {
				auto hand_r = new handler_entry_read_delegate <Width, AddrShift, READ>(m_view.m_space, flags, handler_r);
				hand_r->set_address_info(nstart, nmask);
				r() ->populate(nstart, nend, nmirror, hand_r);

				auto hand_w = new handler_entry_write_delegate<Width, AddrShift, WRITE>(m_view.m_space, flags, handler_w);
				hand_w->set_address_info(nstart, nmask);
				w()->populate(nstart, nend, nmirror, hand_w);
			} else {
				auto hand_r = new handler_entry_read_delegate <AccessWidth, -AccessWidth, READ>(m_view.m_space, flags, handler_r);
				memory_units_descriptor<Width, AddrShift> descriptor(AccessWidth, endianness(), hand_r, nstart, nend, nmask, nunitmask, ncswidth);
				hand_r->set_address_info(descriptor.get_handler_start(), descriptor.get_handler_mask());
				r() ->populate_mismatched(nstart, nend, nmirror, descriptor);
				hand_r->unref();

				auto hand_w = new handler_entry_write_delegate<AccessWidth, -AccessWidth, WRITE>(m_view.m_space, flags, handler_w);
				descriptor.set_subunit_handler(hand_w);
				hand_w->set_address_info(descriptor.get_handler_start(), descriptor.get_handler_mask());
				w()->populate_mismatched(nstart, nend, nmirror, descriptor);
				hand_w->unref();
			}
			invalidate_caches(read_or_write::READWRITE);
		}
	}
};

namespace {
	template<int Level, int Width, int AddrShift> memory_view::memory_view_entry *mve_make_1(const address_space_config &config, memory_manager &manager, memory_view &view, int id) {
		return new memory_view_entry_specific<Level, Width, AddrShift>(config, manager, view, id);
	}

	template<int Width, int AddrShift> memory_view::memory_view_entry *mve_make_2(int Level, const address_space_config &config, memory_manager &manager, memory_view &view, int id) {
		switch(Level) {
		case 0: return mve_make_1<0, Width, AddrShift>(config, manager, view, id);
		case 1: return mve_make_1<1, Width, AddrShift>(config, manager, view, id);
		default: abort();
		}
	}

	memory_view::memory_view_entry *mve_make(int Level, int Width, int AddrShift, const address_space_config &config, memory_manager &manager, memory_view &view, int id) {
		switch (Width | (AddrShift + 4)) {
		case  8|(4+1): return mve_make_2<0,  1>(Level, config, manager, view, id);
		case  8|(4-0): return mve_make_2<0,  0>(Level, config, manager, view, id);
		case 16|(4+3): return mve_make_2<1,  3>(Level, config, manager, view, id);
		case 16|(4-0): return mve_make_2<1,  0>(Level, config, manager, view, id);
		case 16|(4-1): return mve_make_2<1, -1>(Level, config, manager, view, id);
		case 32|(4+3): return mve_make_2<2,  3>(Level, config, manager, view, id);
		case 32|(4-0): return mve_make_2<2,  0>(Level, config, manager, view, id);
		case 32|(4-1): return mve_make_2<2, -1>(Level, config, manager, view, id);
		case 32|(4-2): return mve_make_2<2, -2>(Level, config, manager, view, id);
		case 64|(4-0): return mve_make_2<3,  0>(Level, config, manager, view, id);
		case 64|(4-1): return mve_make_2<3, -1>(Level, config, manager, view, id);
		case 64|(4-2): return mve_make_2<3, -2>(Level, config, manager, view, id);
		case 64|(4-3): return mve_make_2<3, -3>(Level, config, manager, view, id);
		default: abort();
		}
	}
}

memory_view::memory_view_entry &memory_view::operator[](int slot)
{
	if (!m_config)
		fatalerror("A view must be in a map or a space before it can be setup.");

	auto i = m_entry_mapping.find(slot);
	if (i == m_entry_mapping.end()) {
		memory_view_entry *e;
		int id = m_entries.size();
		e = mve_make(emu::detail::handler_entry_dispatch_level(m_config->addr_width()), m_config->data_width(), m_config->addr_shift(),
					 *m_config, m_device.machine().memory(), *this, id);
		m_entries.resize(id+1);
		m_entries[id].reset(e);
		m_entry_mapping[slot] = id;
		if (m_handler_read) {
			m_handler_read->select_u(id);
			m_handler_write->select_u(id);
		}
		return *e;

	} else
		return *m_entries[i->second];
}

memory_view::memory_view_entry::memory_view_entry(const address_space_config &config, memory_manager &manager, memory_view &view, int id) : address_space_installer(config, manager), m_view(view), m_id(id)
{
	m_map = std::make_unique<address_map>(m_view);
}

void memory_view::memory_view_entry::prepare_map_generic(address_map &map, bool allow_alloc)
{
	memory_region *devregion = (m_view.m_space->spacenum() == 0) ? m_view.m_device.memregion(DEVICE_SELF) : nullptr;
	u32 devregionsize = (devregion != nullptr) ? devregion->bytes() : 0;

	// make a pass over the address map, adjusting for the device and getting memory pointers
	for (address_map_entry &entry : map.m_entrylist)
	{
		// computed adjusted addresses first
		adjust_addresses(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror);

		// if we have a share entry, add it to our map
		if (entry.m_share != nullptr)
		{
			// if we can't find it, add it to our map if we're allowed to
			std::string fulltag = entry.m_devbase.subtag(entry.m_share);
			memory_share *share = m_manager.share_find(fulltag);
			if (!share)
			{
				if (!allow_alloc)
					fatalerror("Trying to create share '%s' too late\n", fulltag);
				VPRINTF("Creating share '%s' of length 0x%X\n", fulltag, entry.m_addrend + 1 - entry.m_addrstart);
				share = m_manager.share_alloc(m_view.m_device, fulltag, m_config.data_width(), address_to_byte(entry.m_addrend + 1 - entry.m_addrstart), endianness());
			}
			else
			{
				std::string result = share->compare(m_config.data_width(), address_to_byte(entry.m_addrend + 1 - entry.m_addrstart), endianness());
				if (!result.empty())
					fatalerror("%s\n", result);
			}
			entry.m_memory = share->ptr();
		}

		// if this is a ROM handler without a specified region and not shared, attach it to the implicit region
		if (m_view.m_space->spacenum() == AS_PROGRAM && entry.m_read.m_type == AMH_ROM && entry.m_region == nullptr && entry.m_share == nullptr)
		{
			// make sure it fits within the memory region before doing so, however
			if (entry.m_addrend < devregionsize)
			{
				entry.m_region = m_view.m_device.tag();
				entry.m_rgnoffs = address_to_byte(entry.m_addrstart);
			}
		}

		// validate adjusted addresses against implicit regions
		if (entry.m_region != nullptr)
		{
			// determine full tag
			std::string fulltag = entry.m_devbase.subtag(entry.m_region);

			// find the region
			memory_region *region = m_manager.machine().root_device().memregion(fulltag);
			if (region == nullptr)
				fatalerror("device '%s' %s space memory map entry %X-%X references nonexistent region \"%s\"\n", m_view.m_device.tag(), m_view.m_space->name(), entry.m_addrstart, entry.m_addrend, entry.m_region);

			// validate the region
			if (entry.m_rgnoffs + m_config.addr2byte(entry.m_addrend - entry.m_addrstart + 1) > region->bytes())
				fatalerror("device '%s' %s space memory map entry %X-%X extends beyond region \"%s\" size (%X)\n", m_view.m_device.tag(), m_view.m_space->name(), entry.m_addrstart, entry.m_addrend, entry.m_region, region->bytes());

			if (entry.m_share != nullptr)
				fatalerror("device '%s' %s space memory map entry %X-%X has both .region() and .share()\n", m_view.m_device.tag(), m_view.m_space->name(), entry.m_addrstart, entry.m_addrend);
		}

		// convert any region-relative entries to their memory pointers
		if (entry.m_region != nullptr)
		{
			// determine full tag
			std::string fulltag = entry.m_devbase.subtag(entry.m_region);

			// set the memory address
			entry.m_memory = m_manager.machine().root_device().memregion(fulltag)->base() + entry.m_rgnoffs;
		}

		// allocate anonymous ram when needed
		if (!entry.m_memory && (entry.m_read.m_type == AMH_RAM || entry.m_write.m_type == AMH_RAM))
		{
			if (!allow_alloc)
				fatalerror("Trying to create memory in range %X-%X too late\n", entry.m_addrstart, entry.m_addrend);

			entry.m_memory = m_manager.anonymous_alloc(*m_view.m_space, address_to_byte(entry.m_addrend + 1 - entry.m_addrstart), m_config.data_width(), entry.m_addrstart, entry.m_addrend);
		}
	}
}

void memory_view::memory_view_entry::prepare_device_map(address_map &map)
{
	// Disable the test for now, some cleanup needed before
	if(0) {
		// device maps are not supposed to set global parameters
		if (map.m_unmapval)
			fatalerror("Device maps should not set the unmap value\n");

		if (map.m_globalmask && map.m_globalmask != m_view.m_space->addrmask())
			fatalerror("Device maps should not set the global mask\n");
	}

	prepare_map_generic(map, false);
}


template<int Level, int Width, int AddrShift> void memory_view_entry_specific<Level, Width, AddrShift>::populate_from_map(address_map *map)
{
	// no map specified, use the space-specific one and import the submaps
	if (map == nullptr) {
		map = m_map.get();
		map->import_submaps(m_manager.machine(), m_view.m_device, data_width(), endianness(), addr_shift());
	}

	prepare_map_generic(*map, true);

	// Force the slot to exist, in case the map is empty
	r()->select_u(m_id);
	w()->select_u(m_id);

	// install the handlers, using the original, unadjusted memory map
	for(const address_map_entry &entry : map->m_entrylist) {
		// map both read and write halves
		populate_map_entry(entry, read_or_write::READ);
		populate_map_entry(entry, read_or_write::WRITE);
	}
}

std::string memory_view::memory_view_entry::key() const
{
	std::string key = m_view.m_context;
	if (m_id != -1)
		key += util::string_format("%s[%d].", m_view.m_name, m_view.id_to_slot(m_id));
	return key;
}


memory_view::memory_view(device_t &device, std::string name) : m_device(device), m_name(name), m_config(nullptr), m_addrstart(0), m_addrend(0), m_space(nullptr), m_handler_read(nullptr), m_handler_write(nullptr), m_cur_id(-1), m_cur_slot(-1)
{
	device.view_register(this);
}

memory_view::~memory_view()
{
	if (m_handler_read) {
		m_handler_read->unref();
		m_handler_write->unref();
	}
}

void memory_view::register_state()
{
	m_device.machine().save().save_item(&m_device, "view", m_name.c_str(), 0, NAME(m_cur_slot));
	m_device.machine().save().save_item(&m_device, "view", m_name.c_str(), 0, NAME(m_cur_id));
	m_device.machine().save().register_postload(save_prepost_delegate(NAME([this]() { m_handler_read->select_a(m_cur_id); m_handler_write->select_a(m_cur_id); })));
}

void memory_view::disable()
{
	m_cur_slot = -1;
	m_cur_id = -1;
	m_handler_read->select_a(-1);
	m_handler_write->select_a(-1);

	if(m_space)
		m_space->invalidate_caches(read_or_write::READWRITE);
}

void memory_view::select(int slot)
{
	auto i = m_entry_mapping.find(slot);
	if (i == m_entry_mapping.end())
		fatalerror("memory_view %s: select of unknown slot %d", m_name, slot);

	m_cur_slot = slot;
	m_cur_id = i->second;
	m_handler_read->select_a(m_cur_id);
	m_handler_write->select_a(m_cur_id);

	if(m_space)
		m_space->invalidate_caches(read_or_write::READWRITE);
}

int memory_view::id_to_slot(int id) const
{
	for(const auto &p : m_entry_mapping)
		if (p.second == id)
			return p.first;
	fatalerror("memory_view::id_to_slot on unknown id %d\n", id);
}

void memory_view::initialize_from_address_map(offs_t addrstart, offs_t addrend, const address_space_config &config)
{
	if (m_config)
		fatalerror("A memory_view can be present in only one address map.");

	m_config = &config;
	m_addrstart = addrstart;
	m_addrend = addrend;
}

namespace {
	template<int Width, int AddrShift> void h_make_1(int HighBits, address_space &space, memory_view &view, handler_entry *&r, handler_entry *&w) {
		switch(HighBits) {
		case  0: r = new handler_entry_read_dispatch<std::max(0, Width), Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<std::max(0, Width), Width, AddrShift>(&space, view); break;
		case  1: r = new handler_entry_read_dispatch<std::max(1, Width), Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<std::max(1, Width), Width, AddrShift>(&space, view); break;
		case  2: r = new handler_entry_read_dispatch<std::max(2, Width), Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<std::max(2, Width), Width, AddrShift>(&space, view); break;
		case  3: r = new handler_entry_read_dispatch<std::max(3, Width), Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<std::max(3, Width), Width, AddrShift>(&space, view); break;
		case  4: r = new handler_entry_read_dispatch< 4, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch< 4, Width, AddrShift>(&space, view); break;
		case  5: r = new handler_entry_read_dispatch< 5, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch< 5, Width, AddrShift>(&space, view); break;
		case  6: r = new handler_entry_read_dispatch< 6, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch< 6, Width, AddrShift>(&space, view); break;
		case  7: r = new handler_entry_read_dispatch< 7, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch< 7, Width, AddrShift>(&space, view); break;
		case  8: r = new handler_entry_read_dispatch< 8, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch< 8, Width, AddrShift>(&space, view); break;
		case  9: r = new handler_entry_read_dispatch< 9, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch< 9, Width, AddrShift>(&space, view); break;
		case 10: r = new handler_entry_read_dispatch<10, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<10, Width, AddrShift>(&space, view); break;
		case 11: r = new handler_entry_read_dispatch<11, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<11, Width, AddrShift>(&space, view); break;
		case 12: r = new handler_entry_read_dispatch<12, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<12, Width, AddrShift>(&space, view); break;
		case 13: r = new handler_entry_read_dispatch<13, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<13, Width, AddrShift>(&space, view); break;
		case 14: r = new handler_entry_read_dispatch<14, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<14, Width, AddrShift>(&space, view); break;
		case 15: r = new handler_entry_read_dispatch<15, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<15, Width, AddrShift>(&space, view); break;
		case 16: r = new handler_entry_read_dispatch<16, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<16, Width, AddrShift>(&space, view); break;
		case 17: r = new handler_entry_read_dispatch<17, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<17, Width, AddrShift>(&space, view); break;
		case 18: r = new handler_entry_read_dispatch<18, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<18, Width, AddrShift>(&space, view); break;
		case 19: r = new handler_entry_read_dispatch<19, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<19, Width, AddrShift>(&space, view); break;
		case 20: r = new handler_entry_read_dispatch<20, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<20, Width, AddrShift>(&space, view); break;
		case 21: r = new handler_entry_read_dispatch<21, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<21, Width, AddrShift>(&space, view); break;
		case 22: r = new handler_entry_read_dispatch<22, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<22, Width, AddrShift>(&space, view); break;
		case 23: r = new handler_entry_read_dispatch<23, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<23, Width, AddrShift>(&space, view); break;
		case 24: r = new handler_entry_read_dispatch<24, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<24, Width, AddrShift>(&space, view); break;
		case 25: r = new handler_entry_read_dispatch<25, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<25, Width, AddrShift>(&space, view); break;
		case 26: r = new handler_entry_read_dispatch<26, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<26, Width, AddrShift>(&space, view); break;
		case 27: r = new handler_entry_read_dispatch<27, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<27, Width, AddrShift>(&space, view); break;
		case 28: r = new handler_entry_read_dispatch<28, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<28, Width, AddrShift>(&space, view); break;
		case 29: r = new handler_entry_read_dispatch<29, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<29, Width, AddrShift>(&space, view); break;
		case 30: r = new handler_entry_read_dispatch<30, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<30, Width, AddrShift>(&space, view); break;
		case 31: r = new handler_entry_read_dispatch<31, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<31, Width, AddrShift>(&space, view); break;
		case 32: r = new handler_entry_read_dispatch<32, Width, AddrShift>(&space, view); w = new handler_entry_write_dispatch<32, Width, AddrShift>(&space, view); break;
		default: abort();
		}
	}

	void h_make(int HighBits, int Width, int AddrShift, address_space &space, memory_view &view, handler_entry *&r, handler_entry *&w) {
		switch (Width | (AddrShift + 4)) {
		case  8|(4+1): h_make_1<0,  1>(HighBits, space, view, r, w); break;
		case  8|(4-0): h_make_1<0,  0>(HighBits, space, view, r, w); break;
		case 16|(4+3): h_make_1<1,  3>(HighBits, space, view, r, w); break;
		case 16|(4-0): h_make_1<1,  0>(HighBits, space, view, r, w); break;
		case 16|(4-1): h_make_1<1, -1>(HighBits, space, view, r, w); break;
		case 32|(4+3): h_make_1<2,  3>(HighBits, space, view, r, w); break;
		case 32|(4-0): h_make_1<2,  0>(HighBits, space, view, r, w); break;
		case 32|(4-1): h_make_1<2, -1>(HighBits, space, view, r, w); break;
		case 32|(4-2): h_make_1<2, -2>(HighBits, space, view, r, w); break;
		case 64|(4-0): h_make_1<3,  0>(HighBits, space, view, r, w); break;
		case 64|(4-1): h_make_1<3, -1>(HighBits, space, view, r, w); break;
		case 64|(4-2): h_make_1<3, -2>(HighBits, space, view, r, w); break;
		case 64|(4-3): h_make_1<3, -3>(HighBits, space, view, r, w); break;
		default: abort();
		}
	}
}

std::pair<handler_entry *, handler_entry *> memory_view::make_handlers(address_space &space, offs_t addrstart, offs_t addrend)
{
	if (m_space != &space || m_addrstart != addrstart || m_addrend != addrend) {
		if (m_space)
			fatalerror("A memory_view can be installed only once.");

		if (m_config) {
			if (m_addrstart != addrstart || m_addrend != addrend)
				fatalerror("A memory_view must be installed at its configuration address.");
		} else {
			m_config = &space.space_config();
			m_addrstart = addrstart;
			m_addrend = addrend;
		}

		m_space = &space;

		offs_t span = addrstart ^ addrend;
		u32 awidth = 32 - count_leading_zeros_32(span);

		h_make(awidth, m_config->data_width(), m_config->addr_shift(), space, *this, m_handler_read, m_handler_write);
		m_handler_read->ref();
		m_handler_write->ref();
	}

	return std::make_pair(m_handler_read, m_handler_write);
}

void memory_view::make_subdispatch(std::string context)
{
	m_context = context;
	for(auto &e : m_entries)
		e->populate_from_map();
}

void memory_view::memory_view_entry::check_range_optimize_mirror(const char *function, offs_t addrstart, offs_t addrend, offs_t addrmirror, offs_t &nstart, offs_t &nend, offs_t &nmask, offs_t &nmirror)
{
	check_optimize_mirror(function, addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);
	if (nstart < m_view.m_addrstart || (nend | nmirror) > m_view.m_addrend)
		fatalerror("%s: The range %x-%x mirror %x, exceeds the view window boundaries %x-%x.\n", function, addrstart, addrend, addrmirror, m_view.m_addrstart, m_view.m_addrend);
}

void memory_view::memory_view_entry::check_range_optimize_all(const char *function, int width, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth, offs_t &nstart, offs_t &nend, offs_t &nmask, offs_t &nmirror, u64 &nunitmask, int &ncswidth)
{
	check_optimize_all(function, width, addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, nstart, nend, nmask, nmirror, nunitmask, ncswidth);
	if (nstart < m_view.m_addrstart || (nend | nmirror | addrselect) > m_view.m_addrend)
		fatalerror("%s: The range %x-%x mirror %x select %x, exceeds the view window boundaries %x-%x.\n", function, addrstart, addrend, addrmirror, addrselect, m_view.m_addrstart, m_view.m_addrend);
}

void memory_view::memory_view_entry::check_range_address(const char *function, offs_t addrstart, offs_t addrend)
{
	check_address(function, addrstart, addrend);
	if (addrstart < m_view.m_addrstart || addrend > m_view.m_addrend)
		fatalerror("%s: The range %x-%x exceeds the view window boundaries %x-%x.\n", function, addrstart, addrend, m_view.m_addrstart, m_view.m_addrend);
}

template<int Level, int Width, int AddrShift> void memory_view_entry_specific<Level, Width, AddrShift>::install_ram_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, u16 flags, read_or_write readorwrite, void *baseptr)
{
	VPRINTF("memory_view::install_ram_generic(%s-%s mirror=%s, %s, %p)\n",
			m_addrchars, addrstart, m_addrchars, addrend,
			m_addrchars, addrmirror,
			(readorwrite == read_or_write::READ) ? "read" : (readorwrite == read_or_write::WRITE) ? "write" : (readorwrite == read_or_write::READWRITE) ? "read/write" : "??",
			baseptr);

	offs_t nstart, nend, nmask, nmirror;
	check_range_optimize_mirror("install_ram_generic", addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);

	r()->select_u(m_id);
	w()->select_u(m_id);

	// map for read
	if (readorwrite == read_or_write::READ || readorwrite == read_or_write::READWRITE)
	{
		auto hand_r = new handler_entry_read_memory<Width, AddrShift>(m_view.m_space, flags, baseptr);
		hand_r->set_address_info(nstart, nmask);
		r()->populate(nstart, nend, nmirror, hand_r);
	}

	// map for write
	if (readorwrite == read_or_write::WRITE || readorwrite == read_or_write::READWRITE)
	{
		auto hand_w = new handler_entry_write_memory<Width, AddrShift>(m_view.m_space, flags, baseptr);
		hand_w->set_address_info(nstart, nmask);
		w()->populate(nstart, nend, nmirror, hand_w);
	}

	invalidate_caches(readorwrite);
}

template<int Level, int Width, int AddrShift> void memory_view_entry_specific<Level, Width, AddrShift>::unmap_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, u16 flags, read_or_write readorwrite, bool quiet)
{
	VPRINTF("memory_view::unmap(%*x-%*x mirror=%*x, %s, %s)\n",
			m_addrchars, addrstart, m_addrchars, addrend,
			m_addrchars, addrmirror,
			(readorwrite == read_or_write::READ) ? "read" : (readorwrite == read_or_write::WRITE) ? "write" : (readorwrite == read_or_write::READWRITE) ? "read/write" : "??",
			quiet ? "quiet" : "normal");

	offs_t nstart, nend, nmask, nmirror;
	check_range_optimize_mirror("unmap_generic", addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);

	r()->select_u(m_id);
	w()->select_u(m_id);

	// read space
	if (readorwrite == read_or_write::READ || readorwrite == read_or_write::READWRITE) {
		auto handler = flags ? (quiet ?
								static_cast<handler_entry_read<Width, AddrShift> *>(new handler_entry_read_nop<Width, AddrShift>(m_view.m_space, flags)) :
								static_cast<handler_entry_read<Width, AddrShift> *>(new handler_entry_read_unmapped<Width, AddrShift>(m_view.m_space, flags)))
			: (quiet ?
			   static_cast<handler_entry_read<Width, AddrShift> *>(m_view.m_space->nop_r()) :
			   static_cast<handler_entry_read<Width, AddrShift> *>(m_view.m_space->unmap_r()));
		handler->ref();
		r()->populate(nstart, nend, nmirror, handler);
	}

	// write space
	if (readorwrite == read_or_write::WRITE || readorwrite == read_or_write::READWRITE) {
		auto handler = flags ? (quiet ?
								static_cast<handler_entry_write<Width, AddrShift> *>(new handler_entry_write_nop<Width, AddrShift>(m_view.m_space, flags)) :
								static_cast<handler_entry_write<Width, AddrShift> *>(new handler_entry_write_unmapped<Width, AddrShift>(m_view.m_space, flags)))
			: (quiet ?
			   static_cast<handler_entry_write<Width, AddrShift> *>(m_view.m_space->nop_w()) :
			   static_cast<handler_entry_write<Width, AddrShift> *>(m_view.m_space->unmap_w()));
		handler->ref();
		w()->populate(nstart, nend, nmirror, handler);
	}

	invalidate_caches(readorwrite);
}

template<int Level, int Width, int AddrShift> void memory_view_entry_specific<Level, Width, AddrShift>::install_view(offs_t addrstart, offs_t addrend, offs_t addrmirror, memory_view &view)
{
	offs_t nstart, nend, nmask, nmirror;
	check_range_optimize_mirror("install_view", addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);

	r()->select_u(m_id);
	w()->select_u(m_id);

	auto handlers = view.make_handlers(*m_view.m_space, addrstart, addrend);
	r()->populate(nstart, nend, nmirror, static_cast<handler_entry_read <Width, AddrShift> *>(handlers.first));
	w()->populate(nstart, nend, nmirror, static_cast<handler_entry_write<Width, AddrShift> *>(handlers.second));
	view.make_subdispatch(key()); // Must be called after populate
}

template<int Level, int Width, int AddrShift> memory_passthrough_handler memory_view_entry_specific<Level, Width, AddrShift>::install_read_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, uX &data, uX mem_mask)> tap, memory_passthrough_handler *mph)
{
	offs_t nstart, nend, nmask, nmirror;
	check_range_optimize_mirror("install_read_tap", addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);
	auto impl = m_view.m_space->make_mph(mph);

	r()->select_u(m_id);
	w()->select_u(m_id);

	auto handler = new handler_entry_read_tap<Width, AddrShift>(m_view.m_space, *impl, name, tap);
	r()->populate_passthrough(nstart, nend, nmirror, handler);
	handler->unref();

	invalidate_caches(read_or_write::READ);

	return impl;
}

template<int Level, int Width, int AddrShift> memory_passthrough_handler memory_view_entry_specific<Level, Width, AddrShift>::install_write_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, uX &data, uX mem_mask)> tap, memory_passthrough_handler *mph)
{
	offs_t nstart, nend, nmask, nmirror;
	check_range_optimize_mirror("install_write_tap", addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);
	auto impl = m_view.m_space->make_mph(mph);

	r()->select_u(m_id);
	w()->select_u(m_id);

	auto handler = new handler_entry_write_tap<Width, AddrShift>(m_view.m_space, *impl, name, tap);
	w()->populate_passthrough(nstart, nend, nmirror, handler);
	handler->unref();

	invalidate_caches(read_or_write::WRITE);

	return impl;
}

template<int Level, int Width, int AddrShift> memory_passthrough_handler memory_view_entry_specific<Level, Width, AddrShift>::install_readwrite_tap(offs_t addrstart, offs_t addrend, offs_t addrmirror, std::string name, std::function<void (offs_t offset, uX &data, uX mem_mask)> tapr, std::function<void (offs_t offset, uX &data, uX mem_mask)> tapw, memory_passthrough_handler *mph)
{
	offs_t nstart, nend, nmask, nmirror;
	check_range_optimize_mirror("install_readwrite_tap", addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);
	auto impl = m_view.m_space->make_mph(mph);

	r()->select_u(m_id);
	w()->select_u(m_id);

	auto rhandler = new handler_entry_read_tap <Width, AddrShift>(m_view.m_space, *impl, name, tapr);
	r() ->populate_passthrough(nstart, nend, nmirror, rhandler);
	rhandler->unref();

	auto whandler = new handler_entry_write_tap<Width, AddrShift>(m_view.m_space, *impl, name, tapw);
	w()->populate_passthrough(nstart, nend, nmirror, whandler);
	whandler->unref();

	invalidate_caches(read_or_write::READWRITE);

	return impl;
}

template<int Level, int Width, int AddrShift> void memory_view_entry_specific<Level, Width, AddrShift>::install_device_delegate(offs_t addrstart, offs_t addrend, device_t &device, address_map_constructor &delegate, u64 unitmask, int cswidth, u16 flags)
{
	check_range_address("install_device_delegate", addrstart, addrend);
	address_map map(*m_view.m_space, addrstart, addrend, unitmask, cswidth, flags, m_view.m_device, delegate);
	map.import_submaps(m_manager.machine(), device, data_width(), endianness(), addr_shift());
	prepare_device_map(map);
	populate_from_map(&map);
}

template<int Level, int Width, int AddrShift> void memory_view_entry_specific<Level, Width, AddrShift>::install_readwrite_port(offs_t addrstart, offs_t addrend, offs_t addrmirror, u16 flags, std::string rtag, std::string wtag)
{
	VPRINTF("memory_view::install_readwrite_port(%*x-%*x mirror=%*x, read=\"%s\" / write=\"%s\")\n",
			m_addrchars, addrstart, m_addrchars, addrend,
			m_addrchars, addrmirror,
			rtag.empty() ? "(none)" : rtag, wtag.empty() ? "(none)" : wtag);

	offs_t nstart, nend, nmask, nmirror;
	check_range_optimize_mirror("install_readwrite_port", addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);

	r()->select_u(m_id);
	w()->select_u(m_id);

	// read handler
	if (rtag != "")
	{
		// find the port
		ioport_port *port = m_view.m_device.ioport(rtag);
		if (port == nullptr)
			throw emu_fatalerror("Attempted to map non-existent port '%s' for read in space %s of device '%s'\n", rtag, m_view.m_name, m_view.m_device.tag());

		// map the range and set the ioport
		auto hand_r = new handler_entry_read_ioport<Width, AddrShift>(m_view.m_space, flags, port);
		r()->populate(nstart, nend, nmirror, hand_r);
	}

	if (wtag != "")
	{
		// find the port
		ioport_port *port = m_view.m_device.ioport(wtag);
		if (port == nullptr)
			fatalerror("Attempted to map non-existent port '%s' for write in space %s of device '%s'\n", wtag, m_view.m_name, m_view.m_device.tag());

		// map the range and set the ioport
		auto hand_w = new handler_entry_write_ioport<Width, AddrShift>(m_view.m_space, flags, port);
		w()->populate(nstart, nend, nmirror, hand_w);
	}

	invalidate_caches(rtag != "" ? wtag != "" ? read_or_write::READWRITE : read_or_write::READ : read_or_write::WRITE);
}
template<int Level, int Width, int AddrShift> void memory_view_entry_specific<Level, Width, AddrShift>::install_bank_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, u16 flags, memory_bank *rbank, memory_bank *wbank)
{
	VPRINTF("memory_view::install_readwrite_bank(%*x-%*x mirror=%*x, read=\"%s\" / write=\"%s\")\n",
			m_addrchars, addrstart, m_addrchars, addrend,
			m_addrchars, addrmirror,
			(rbank != nullptr) ? rbank->tag() : "(none)", (wbank != nullptr) ? wbank->tag() : "(none)");

	offs_t nstart, nend, nmask, nmirror;
	check_range_optimize_mirror("install_bank_generic", addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);

	r()->select_u(m_id);
	w()->select_u(m_id);

	// map the read bank
	if (rbank != nullptr)
	{
		auto hand_r = new handler_entry_read_memory_bank<Width, AddrShift>(m_view.m_space, flags, *rbank);
		hand_r->set_address_info(nstart, nmask);
		r()->populate(nstart, nend, nmirror, hand_r);
	}

	// map the write bank
	if (wbank != nullptr)
	{
		auto hand_w = new handler_entry_write_memory_bank<Width, AddrShift>(m_view.m_space, flags, *wbank);
		hand_w->set_address_info(nstart, nmask);
		w()->populate(nstart, nend, nmirror, hand_w);
	}

	invalidate_caches(rbank ? wbank ? read_or_write::READWRITE : read_or_write::READ : read_or_write::WRITE);
}
