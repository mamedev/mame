// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple "djMEMC" and "MEMCjr" memory controllers
    Emulation by R. Belmont

    djMEMC and MEMCjr contain the following:
    - A memory controller for up to 640 MiB of RAM (10 banks of 64 MiB for djMEMC or 5 banks of 128 MiB for MEMCjr)
    - The usual Mac ROM/RAM switch so at boot the processor has vectors at 0
    - DAFB II video, minus the "Turbo SCSI" feature, which has moved to IOSB
*/

#include "emu.h"
#include "djmemc.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(DJMEMC, djmemc_device, "djmemc", "Apple djMEMC memory controller")
DEFINE_DEVICE_TYPE(MEMCJR, memcjr_device, "memcjr", "Apple MEMCjr memory controller")

//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------

void djmemc_device::map(address_map &map)
{
	map(0x40000000, 0x400fffff).r(FUNC(djmemc_device::rom_switch_r)).mirror(0x0ff00000).nopw();
	map(0xf9000000, 0xf91fffff).rw(m_video, FUNC(dafb_device::vram_r), FUNC(dafb_device::vram_w));
	map(0xf9800000, 0xf98003ff).m(m_video, FUNC(dafb_device::map));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void djmemc_device::device_add_mconfig(machine_config &config)
{
	DAFB_MEMC(config, m_video);
	m_video->dafb_irq().set(FUNC(djmemc_device::vbl_w));
}

//-------------------------------------------------
//  djmemc_device - constructor
//-------------------------------------------------

djmemc_device::djmemc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_video(*this, "video"),
	m_maincpu(*this, finder_base::DUMMY_TAG),
	m_rom(*this, finder_base::DUMMY_TAG),
	m_irq(*this),
	m_overlay(false)
{
}

djmemc_device::djmemc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	djmemc_device(mconfig, DJMEMC, tag, owner, clock)
{
}

memcjr_device::memcjr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	djmemc_device(mconfig, MEMCJR, tag, owner, clock),
	m_dafb_holding(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void djmemc_device::device_start()
{
	m_rom_ptr = &m_rom[0];
	m_rom_size = m_rom.length() << 2;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void djmemc_device::device_reset()
{
	m_overlay = true;

	// put ROM mirror at 0
	address_space &space = m_maincpu->space(AS_PROGRAM);
	const u32 memory_size = std::min((u32)0x3fffff, m_rom_size);
	const u32 memory_end = memory_size - 1;
	offs_t memory_mirror = memory_end & ~(memory_size - 1);

	space.unmap_write(0x00000000, memory_end);
	space.install_rom(0x00000000, memory_end & ~memory_mirror, memory_mirror, m_rom_ptr);
}

u32 djmemc_device::rom_switch_r(offs_t offset)
{
	// disable the overlay
	if (m_overlay && !machine().side_effects_disabled())
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);
		const u32 memory_end = m_ram_size - 1;
		void *memory_data = m_ram_ptr;
		offs_t memory_mirror = memory_end & ~memory_end;

		space.install_ram(0x00000000, memory_end & ~memory_mirror, memory_mirror, memory_data);
		m_overlay = false;
	}

	return m_rom_ptr[offset & ((m_rom_size - 1) >> 2)];
}

void djmemc_device::set_ram_info(u32 *ram, u32 size)
{
	m_ram_ptr = ram;
	m_ram_size = size;
}

void djmemc_device::vbl_w(int state)
{
	m_irq(state);
}

// memcjr_device -------------------------------------------------------------------

void memcjr_device::device_add_mconfig(machine_config &config)
{
	DAFB_MEMCJR(config, m_video);
	m_video->dafb_irq().set(FUNC(memcjr_device::vbl_w));
}

void memcjr_device::device_start()
{
	djmemc_device::device_start();

	save_item(NAME(m_dafb_holding));
}

void memcjr_device::map(address_map &map)
{
	map(0x40000000, 0x400fffff).r(FUNC(memcjr_device::rom_switch_r)).mirror(0x0ff00000).nopw();
	map(0x5000e000, 0x5000ffff).rw(FUNC(memcjr_device::memcjr_r), FUNC(memcjr_device::memcjr_w)).mirror(0x00fc0000);
	map(0xf9000000, 0xf91fffff).rw(m_video, FUNC(dafb_device::vram_r), FUNC(dafb_device::vram_w));
	map(0xf9800000, 0xf98003ff).m(m_video, FUNC(dafb_device::map));
	map(0xf9800000, 0xf98001ff).rw(FUNC(memcjr_device::dafb_holding_r), FUNC(memcjr_device::dafb_holding_w));
}

u32 memcjr_device::dafb_holding_r(offs_t offset)
{
	u32 result;

	if (offset < (0x100>>2))
	{
		result = m_video->dafb_r(offset);
	}
	else
	{
		result = m_video->swatch_r(offset - (0x100>>2));
	}

	m_dafb_holding = (result >> 6) & 0x3f;

	return result & 0x3f;
}

void memcjr_device::dafb_holding_w(offs_t offset, u32 data)
{
//  printf("%04x (%04x) to DAFB @ %04x\n", (data & 0x3f) | m_dafb_holding, data & 0xfff, offset<<2);
	if (offset < (0x100>>2))
	{
		m_video->dafb_w(offset, (data & 0x3f) | m_dafb_holding);
	}
	else
	{
		m_video->swatch_w(offset - (0x100>>2), (data & 0x3f) | m_dafb_holding);
	}
	m_dafb_holding = 0;
}

u32 memcjr_device::memcjr_r(offs_t offset)
{
	//printf("memcjr_r @ %x\n", offset<<2);
	if (offset == (0x7c>>2))
	{
		return m_dafb_holding >> 6;
	}

	return 0;
}

void memcjr_device::memcjr_w(offs_t offset, u32 data)
{
	//printf("memcjr_w: %08x to %x\n", data, offset<<2);
	if (offset == (0x7c>>2))
	{
		m_dafb_holding = (data & 0x3f) << 6;
	//  printf("holding = %x\n", m_dafb_holding);
	}
}
