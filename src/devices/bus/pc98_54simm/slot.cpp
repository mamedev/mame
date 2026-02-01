// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "slot.h"

/*
 * SLOT DEVICE
 */

DEFINE_DEVICE_TYPE(PC9801_54_SIMM, pc9801_54_simm_device, "pc9801_54_simm", "PC-9801-54 54SIMM Slot")

pc9801_54_simm_device::pc9801_54_simm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_54_SIMM, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_single_card_slot_interface<device_pc9801_54_interface>(mconfig, *this)
	, m_space_mem_config("space_mem", ENDIANNESS_LITTLE, 16, 24, 0, address_map_constructor())
{
}

pc9801_54_simm_device::~pc9801_54_simm_device()
{
}

void pc9801_54_simm_device::device_start()
{
	m_bank = get_card_device();
	m_space_mem = &space(AS_PROGRAM);
}


device_memory_interface::space_config_vector pc9801_54_simm_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_space_mem_config)
	};
}

// host methods
u16 pc9801_54_simm_device::read(offs_t offset, u16 mem_mask)
{
	return m_space_mem->read_word(offset << 1, mem_mask);
}

void pc9801_54_simm_device::write(offs_t offset, u16 data, u16 mem_mask)
{
	m_space_mem->write_word(offset << 1, data, mem_mask);
}

u16 pc9801_54_simm_device::read_ext(offs_t offset, u16 mem_mask)
{
	return m_space_mem->read_word((offset << 1) + 0x100000, mem_mask);
}

void pc9801_54_simm_device::write_ext(offs_t offset, u16 data, u16 mem_mask)
{
	m_space_mem->write_word((offset << 1) + 0x100000, data, mem_mask);
}

// bank -> slot
void pc9801_54_simm_device::install_ram(offs_t addrstart, offs_t addrend, void *baseptr)
{
	m_space_mem->install_ram(addrstart, addrend, baseptr);
}


/*
 * INTERFACE
 */


device_pc9801_54_interface::device_pc9801_54_interface(const machine_config &mconfig, device_t &device)
   : device_interface(device, "pc9801_54")
{
	m_slot = dynamic_cast<pc9801_54_simm_device *>(device.owner());
}

device_pc9801_54_interface::~device_pc9801_54_interface()
{
}

void device_pc9801_54_interface::interface_pre_start()
{
	if (!m_slot->started())
		throw device_missing_dependencies();
}

void device_pc9801_54_interface::interface_post_start()
{
}

