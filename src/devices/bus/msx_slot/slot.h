// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***********************************************************************************************************

   MSX (logical) internal slot/page interfacing

The MSX standard uses logically defined slots, subslots, and pages to access rom and optional components
in a system. There are no physical slots inside the system. A piece of rom/component can occur in multiple
pages; and multiple pieces of rom/ram/components can occur in a single slot.

***********************************************************************************************************/

#ifndef MAME_BUS_MSX_SLOT_SLOT_H
#define MAME_BUS_MSX_SLOT_SLOT_H

#pragma once

class msx_internal_slot_interface
{
public:
	msx_internal_slot_interface(const machine_config &mconfig, device_t &device);
	msx_internal_slot_interface(const msx_internal_slot_interface &device) = delete;
	virtual ~msx_internal_slot_interface() { }

	// configuration helpers
	template <typename T> void set_memory_space(T &&tag, int spacenum) { m_mem_space.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_io_space(T &&tag, int spacenum) { m_io_space.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_maincpu(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }
	void set_start_address(u32 start_address) { m_start_address = start_address; m_end_address = m_start_address + m_size; }
	void set_size(u32 size) { m_size = size; m_end_address = m_start_address + m_size; }

	void install(memory_view::memory_view_entry *page0, memory_view::memory_view_entry *page1, memory_view::memory_view_entry *page2, memory_view::memory_view_entry *page3);

	address_space &memory_space() const { return *m_mem_space; }
	address_space &io_space() const { return *m_io_space; }
	cpu_device &maincpu() const { return *m_maincpu; }
	bool page_configured(int i) { return bool(m_page[i]); }
	memory_view::memory_view_entry *page(int i)
	{
		if (!m_page[i])
			fatalerror("page %i view not configured\n", i);
		return m_page[i];
	}

protected:
	required_address_space m_mem_space;
	required_address_space m_io_space;
	required_device<cpu_device> m_maincpu;

	u32 m_start_address;
	u32 m_size;
	u32 m_end_address;

	memory_view::memory_view_entry *m_page[4];
};

#endif // MAME_BUS_MSX_SLOT_SLOT_H
