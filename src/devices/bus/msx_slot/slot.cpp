// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***********************************************************************************************************

   MSX (logical) internal slot interfacing

The MSX standard uses logically defined slots, subslots, and pages to access rom and optional components
in a system. There are no physical slots inside the system. A piece of rom/component can occur in multiple
pages; and multiple pieces of rom/ram/components can occur in a single slot.

***********************************************************************************************************/

#include "emu.h"
#include "slot.h"

msx_internal_slot_interface::msx_internal_slot_interface()
	: m_start_address(0)
	, m_size(0)
	, m_end_address(0)
{
}

void msx_internal_slot_interface::set_start_address(device_t &device, UINT32 start_address)
{
	msx_internal_slot_interface &dev = dynamic_cast<msx_internal_slot_interface &>(device);
	dev.m_start_address = start_address;
	dev.m_end_address = dev.m_start_address + dev.m_size;
}

void msx_internal_slot_interface::set_size(device_t &device, UINT32 size)
{
	msx_internal_slot_interface &dev = dynamic_cast<msx_internal_slot_interface &>(device);

	dev.m_size = size;
	dev.m_end_address = dev.m_start_address + dev.m_size;
}
