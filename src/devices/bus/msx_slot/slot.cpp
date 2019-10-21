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

msx_internal_slot_interface::msx_internal_slot_interface(const machine_config &mconfig, device_t &device)
	: m_mem_space(device, finder_base::DUMMY_TAG, -1)
	, m_io_space(device, finder_base::DUMMY_TAG, -1)
	, m_start_address(0)
	, m_size(0)
	, m_end_address(0)
{
	(void)mconfig;
}
