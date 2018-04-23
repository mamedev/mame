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

#define MCFG_MSX_INTERNAL_SLOT_ADD(_tag, _type, _startpage, _numpages) \
	MCFG_DEVICE_ADD(_tag, _type, 0) \
	dynamic_cast<msx_internal_slot_interface &>(*device).set_start_address(_startpage * 0x4000); \
	dynamic_cast<msx_internal_slot_interface &>(*device).set_size(_numpages * 0x4000);

class msx_internal_slot_interface
{
public:
	msx_internal_slot_interface();
	virtual ~msx_internal_slot_interface() { }

	// configuration helpers
	void set_start_address(uint32_t start_address) { m_start_address = start_address; m_end_address = m_start_address + m_size; }
	void set_size(uint32_t size) { m_size = size; m_end_address = m_start_address + m_size; }

	virtual DECLARE_READ8_MEMBER(read) { return 0xFF; }
	virtual DECLARE_WRITE8_MEMBER(write) { }

protected:
	uint32_t m_start_address;
	uint32_t m_size;
	uint32_t m_end_address;
};

#endif // MAME_BUS_MSX_SLOT_SLOT_H
