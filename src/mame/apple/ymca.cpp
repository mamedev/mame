// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple "YMCA" memory controller ("Yazdy's Memory Controller and Arbiter")
    Emulation by R. Belmont

    This includes a DRAM controller, bus arbitration for the 4 possible bus
    masters in Cyclone/Tempest (CPU, DSP, NuBus bus master cards, bus master
    DMA in the PSC chip), and a few GPIO pins for CPU ID and video functions.
*/

#include "emu.h"
#include "ymca.h"

DEFINE_DEVICE_TYPE(YMCA, ymca_device, "ymca", "Apple YMCA memory controller")

static constexpr int YMCA_CPUID0            = 0x38;
static constexpr int YMCA_CPUID1            = 0x3c;
static constexpr int YMCA_CPUID2            = 0x40;
static constexpr int YMCA_CPUID3            = 0x44;
static constexpr int YMCA_CLOCKSEL          = 0x48;
static constexpr int YMCA_BYPASS            = 0x4c;

void ymca_device::map(address_map &map)
{
	map(0x40000000, 0x401fffff).r(FUNC(ymca_device::rom_switch_r)).mirror(0x0fe00000).nopw();
	map(0x50f30400, 0x50f307ff).rw(FUNC(ymca_device::regs_r), FUNC(ymca_device::regs_w));
}

void ymca_device::device_add_mconfig(machine_config &config)
{
}

ymca_device::ymca_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_maincpu(*this, finder_base::DUMMY_TAG),
	m_rom(*this, finder_base::DUMMY_TAG),
	m_irq(*this),
	m_ntscpalsel(*this),
	m_rgb_bypass(*this),
	m_overlay(false),
	m_cpu_id(0)
{
}

ymca_device::ymca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymca_device(mconfig, YMCA, tag, owner, clock)
{
}

void ymca_device::device_start()
{
	m_rom_ptr = &m_rom[0];
	m_rom_size = m_rom.length() << 2;
}

void ymca_device::device_reset()
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

u32 ymca_device::rom_switch_r(offs_t offset)
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

void ymca_device::set_ram_info(u32 *ram, u32 size)
{
	m_ram_ptr = ram;
	m_ram_size = size;
}

u32 ymca_device::regs_r(offs_t offset, u32 mem_mask)
{
	switch (offset << 2)
	{
		case YMCA_CPUID0: // ID 0
			return BIT(m_cpu_id, 0) << 31;

		case YMCA_CPUID1: // ID 1
			return BIT(m_cpu_id, 1) << 31;

		case YMCA_CPUID2: // ID 2
			return BIT(m_cpu_id, 2) << 31;

		case YMCA_CPUID3: // ID 3
			return BIT(m_cpu_id, 3) << 31;
	}
	return 0;
}

void ymca_device::regs_w(offs_t offset, u32 data, u32 mem_mask)
{
//  printf("YMCA: %08x @ %x (%08x)\n", data, offset, mem_mask);

	switch (offset << 2)
	{
		case YMCA_CLOCKSEL:
			m_ntscpalsel(data ? ASSERT_LINE : CLEAR_LINE);
			break;

		case YMCA_BYPASS:
			m_rgb_bypass(data ? ASSERT_LINE : CLEAR_LINE);
			break;
	}
}
