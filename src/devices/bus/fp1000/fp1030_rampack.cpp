// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

FP-1030 RAMPACK

FILES"PACK<x>:" where <x> is the designated slot number 0-7
LOAD"PACKx:<filename>"
RUN
FORMAT"PACK<x>:" to use it in BASIC as a writable buffer

**************************************************************************************************/

#include "emu.h"
#include "fp1030_rampack.h"

DEFINE_DEVICE_TYPE(FP1030_RAMPACK, fp1030_rampack_device, "fp1030_rampack", "FP-1030 RAMPACK")

fp1030_rampack_device::fp1030_rampack_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: fp1060io_exp_device(mconfig, FP1030_RAMPACK, tag, owner, clock)
	, m_nvram(*this, "nvram")
{
}

void fp1030_rampack_device::io_map(address_map &map)
{
	// TODO: verify mirror/unmap
	map(0x0000, 0x3fff).lrw8(
		NAME([this](offs_t offset) { return m_nvram_ptr[offset & 0x3fff]; }),
		NAME([this](offs_t offset, uint8_t data) { m_nvram_ptr[offset & 0x3fff] = data; })
	);
}

void fp1030_rampack_device::device_add_mconfig(machine_config &config)
{
	// C-16K CMOS, 8 banks with 0x800 length
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void fp1030_rampack_device::device_start()
{
	const u32 nvram_size = 0x4000;
	m_nvram_ptr = std::make_unique<uint8_t[]>(nvram_size);
	m_nvram->set_base(m_nvram_ptr.get(), nvram_size);
	save_pointer(NAME(m_nvram_ptr), nvram_size);
}

void fp1030_rampack_device::device_reset()
{
}
