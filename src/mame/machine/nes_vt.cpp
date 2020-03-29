// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "nes_vt.h"

DEFINE_DEVICE_TYPE(NES_VT_SOC, nes_vt_soc_device, "nes_vt_soc", "VTxx series System on a Chip")

void nes_vt_soc_device::program_map(address_map &map)
{
}

nes_vt_soc_device::nes_vt_soc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NES_VT_SOC, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_program_space_config("program", ENDIANNESS_LITTLE, 8, 25, 0, address_map_constructor(FUNC(nes_vt_soc_device::program_map), this))
{
}

void nes_vt_soc_device::device_start()
{
}

device_memory_interface::space_config_vector nes_vt_soc_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_space_config)
	};
}
