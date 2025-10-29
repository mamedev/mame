// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "emu.h"
#include "gaelco_ds5002fp.h"

#include "cpu/mcs51/ds5002fp.h"
#include "machine/nvram.h"


DEFINE_DEVICE_TYPE(GAELCO_DS5002FP, gaelco_ds5002fp_device, "gaelco_ds5002fp", "Gaelco DS5002FP")

void gaelco_ds5002fp_device::dallas_rom(address_map &map)
{
	map(0x00000, 0x07fff).readonly().share("sram");
}

void gaelco_ds5002fp_device::dallas_ram(address_map &map)
{
	map(0x00000, 0x0ffff).rw(FUNC(gaelco_ds5002fp_device::hostmem_r), FUNC(gaelco_ds5002fp_device::hostmem_w));
	map(0x10000, 0x17fff).ram().share("sram"); // yes, the games access it as data and use it for temporary storage!!
}

gaelco_ds5002fp_device::gaelco_ds5002fp_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, GAELCO_DS5002FP, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_hostmem_config({ "hostmem", ENDIANNESS_BIG, 8, 16, 0 })
	, m_hostmem(nullptr)
{
}

uint8_t gaelco_ds5002fp_device::hostmem_r(offs_t offset)
{
	return m_hostmem->read_byte(offset);
}

void gaelco_ds5002fp_device::hostmem_w(offs_t offset, uint8_t data)
{
	m_hostmem->write_byte(offset, data);
}

void gaelco_ds5002fp_device::device_add_mconfig(machine_config &config)
{
	ds5002fp_device &mcu(DS5002FP(config, "mcu", DERIVED_CLOCK(1, 1)));
	mcu.set_addrmap(AS_PROGRAM, &gaelco_ds5002fp_device::dallas_rom);
	mcu.set_addrmap(AS_DATA, &gaelco_ds5002fp_device::dallas_ram);

	NVRAM(config, "sram", nvram_device::DEFAULT_ALL_0);
}

void gaelco_ds5002fp_device::device_start()
{
	m_hostmem = &space(0);
}

device_memory_interface::space_config_vector gaelco_ds5002fp_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(0, &m_hostmem_config) };
}
