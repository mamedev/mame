// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "emu.h"
#include "machine/gaelco_ds5002fp.h"

#include "cpu/mcs51/mcs51.h"
#include "machine/nvram.h"


DEFINE_DEVICE_TYPE(GAELCO_DS5002FP, gaelco_ds5002fp_device, "gaelco_ds5002fp", "Gaelco DS5002FP")

ADDRESS_MAP_START(gaelco_ds5002fp_device::dallas_rom)
	AM_RANGE(0x00000, 0x07fff) AM_READONLY AM_SHARE("sram")
ADDRESS_MAP_END

ADDRESS_MAP_START(gaelco_ds5002fp_device::dallas_ram)
	AM_RANGE(0x00000, 0x0ffff) AM_READWRITE(hostmem_r, hostmem_w)
	AM_RANGE(0x10000, 0x17fff) AM_RAM AM_SHARE("sram") // yes, the games access it as data and use it for temporary storage!!
ADDRESS_MAP_END

gaelco_ds5002fp_device::gaelco_ds5002fp_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, GAELCO_DS5002FP, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_hostmem_config({ "hostmem", ENDIANNESS_BIG, 8, 16, 0 })
	, m_hostmem(nullptr)
{
}

READ8_MEMBER(gaelco_ds5002fp_device::hostmem_r)
{
	return m_hostmem->read_byte(offset);
}

WRITE8_MEMBER(gaelco_ds5002fp_device::hostmem_w)
{
	m_hostmem->write_byte(offset, data);
}

MACHINE_CONFIG_START(gaelco_ds5002fp_device::device_add_mconfig)
	MCFG_CPU_ADD("mcu", DS5002FP, DERIVED_CLOCK(1, 1))
	MCFG_CPU_PROGRAM_MAP(dallas_rom)
	MCFG_CPU_IO_MAP(dallas_ram)

	MCFG_QUANTUM_PERFECT_CPU("mcu")

	MCFG_NVRAM_ADD_0FILL("sram")
MACHINE_CONFIG_END

void gaelco_ds5002fp_device::device_start()
{
	m_hostmem = &space(0);
}

device_memory_interface::space_config_vector gaelco_ds5002fp_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(0, &m_hostmem_config) };
}
