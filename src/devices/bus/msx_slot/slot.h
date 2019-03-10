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
	void set_start_address(uint32_t start_address) { m_start_address = start_address; m_end_address = m_start_address + m_size; }
	void set_size(uint32_t size) { m_size = size; m_end_address = m_start_address + m_size; }

	virtual uint8_t read(offs_t offset) { return 0xFF; }
	virtual void write(offs_t offset, uint8_t data) { }

	address_space &memory_space() const { return *m_mem_space; }
	address_space &io_space() const { return *m_io_space; }

protected:
	required_address_space m_mem_space;
	required_address_space m_io_space;

	uint32_t m_start_address;
	uint32_t m_size;
	uint32_t m_end_address;
};

#endif // MAME_BUS_MSX_SLOT_SLOT_H
