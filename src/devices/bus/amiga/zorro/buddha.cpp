// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Buddha

    Zorro-II IDE controller

    The 'speed' register is used to select the IDE timing according to
    the following table (bits 7-5 are used):

    0    497ns   7c  to select, IOR/IOW after 172ns  2c
    1    639ns   9c  to select, IOR/IOW after 243ns  3c
    2    781ns  11c  to select, IOR/IOW after 314ns  4c
    3    355ns   5c  to select, IOR/IOW after 101ns  1c
    4    355ns   5c  to select, IOR/IOW after 172ns  2c
    5    355ns   5c  to select, IOR/IOW after 243ns  3c
    6   1065ns  15c  to select, IOR/IOW after 314ns  4c
    7    355ns   5c  to select, IOR/IOW after 101ns  1c

    c = clock cycles. This isn't emulated.

***************************************************************************/

#include "emu.h"
#include "buddha.h"

#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AMIGA_BUDDHA, bus::amiga::zorro::buddha_device, "amiga_buddha", "Buddha IDE controller")

namespace bus::amiga::zorro {

buddha_device::buddha_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_BUDDHA, tag, owner, clock),
	device_zorro2_card_interface(mconfig, *this),
	m_ata_0(*this, "ata_0"),
	m_ata_1(*this, "ata_1"),
	m_bootrom(*this, "bootrom"),
	m_ide_interrupts_enabled(false),
	m_ide_0_interrupt(0),
	m_ide_1_interrupt(0)
{
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void buddha_device::mmio_map(address_map &map)
{
	map(0x07fe, 0x07ff).rw(FUNC(buddha_device::speed_r), FUNC(buddha_device::speed_w));
	map(0x0800, 0x08ff).rw(FUNC(buddha_device::ide_0_cs0_r), FUNC(buddha_device::ide_0_cs0_w));
	map(0x0900, 0x09ff).rw(FUNC(buddha_device::ide_0_cs1_r), FUNC(buddha_device::ide_0_cs1_w));
	map(0x0a00, 0x0aff).rw(FUNC(buddha_device::ide_1_cs0_r), FUNC(buddha_device::ide_1_cs0_w));
	map(0x0b00, 0x0bff).rw(FUNC(buddha_device::ide_1_cs1_r), FUNC(buddha_device::ide_1_cs1_w));
	map(0x0f00, 0x0f3f).r(FUNC(buddha_device::ide_0_interrupt_r));
	map(0x0f40, 0x0f7f).r(FUNC(buddha_device::ide_1_interrupt_r));
	map(0x0fc0, 0x0fff).w(FUNC(buddha_device::ide_interrupt_enable_w));
	map(0x1000, 0xffff).r(FUNC(buddha_device::rom_r)).umask16(0xff00);
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void buddha_device::device_add_mconfig(machine_config &config)
{
	ATA_INTERFACE(config, m_ata_0).options(ata_devices, nullptr, nullptr, false);
	m_ata_0->irq_handler().set(FUNC(buddha_device::ide_0_interrupt_w));

	ATA_INTERFACE(config, m_ata_1).options(ata_devices, nullptr, nullptr, false);
	m_ata_1->irq_handler().set(FUNC(buddha_device::ide_1_interrupt_w));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( buddha )
	ROM_REGION(0x8000, "bootrom", 0)
	ROM_DEFAULT_BIOS("v103-17")
	ROM_SYSTEM_BIOS(0, "v103-8", "Version 103.8")
	ROMX_LOAD("buddha_103-8.rom",  0x0000, 0x8000, CRC(44f81426) SHA1(95555c6690b5c697e1cdca2726e47c1c6c194d7c), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v103-17", "Version 103.17")
	ROMX_LOAD("buddha_103-17.rom", 0x0000, 0x8000, CRC(2b7b24e0) SHA1(ec17a58962c373a2892090ec9b1722d2c326d631), ROM_BIOS(1))
ROM_END

const tiny_rom_entry *buddha_device::device_rom_region() const
{
	return ROM_NAME( buddha );
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void buddha_device::device_start()
{
	save_item(NAME(m_ide_interrupts_enabled));
	save_item(NAME(m_ide_0_interrupt));
	save_item(NAME(m_ide_1_interrupt));
}

void buddha_device::busrst_w(int state)
{
	if (state == 0)
	{
		m_ide_interrupts_enabled = false;
		m_ide_0_interrupt = 0;
		m_ide_1_interrupt = 0;
	}
}

uint8_t buddha_device::rom_r(offs_t offset)
{
	// the first 0x800 bytes cannot be read
	return m_bootrom[0x800 + offset];
}

uint16_t buddha_device::speed_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

	if (!machine().side_effects_disabled())
		LOG("speed_r %04x [mask = %04x]\n", data, mem_mask);

	return data;
}

void buddha_device::speed_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOG("speed_w %04x [mask = %04x]\n", data, mem_mask);
}

void buddha_device::ide_0_interrupt_w(int state)
{
	LOG("ide_0_interrupt_w (%d)\n", state);

	m_ide_0_interrupt = state;

	if (m_ide_interrupts_enabled)
		m_zorro->int2_w(state);
}

void buddha_device::ide_1_interrupt_w(int state)
{
	LOG("ide_1_interrupt_w (%d)\n", state);

	m_ide_1_interrupt = state;

	if (m_ide_interrupts_enabled)
		m_zorro->int2_w(state);
}

uint16_t buddha_device::ide_0_interrupt_r(offs_t offset, uint16_t mem_mask)
{
//  LOG("ide_0_interrupt_r %04x [mask = %04x]\n", m_ide_0_interrupt << 15, mem_mask);
	return m_ide_0_interrupt << 15;
}

uint16_t buddha_device::ide_1_interrupt_r(offs_t offset, uint16_t mem_mask)
{
//  LOG("ide_1_interrupt_r %04x [mask = %04x]\n", m_ide_1_interrupt << 15, mem_mask);
	return m_ide_1_interrupt << 15;
}

void buddha_device::ide_interrupt_enable_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOG("ide_interrupt_enable_w %04x [mask = %04x]\n", data, mem_mask);

	// writing any value here enables ide interrupts to the zorro slot
	m_ide_interrupts_enabled = true;
}

uint16_t buddha_device::ide_0_cs0_r(offs_t offset, uint16_t mem_mask)
{
	return m_ata_0->cs0_swap_r((offset >> 1) & 0x07, mem_mask);
}

void buddha_device::ide_0_cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_ata_0->cs0_swap_w((offset >> 1) & 0x07, data, mem_mask);
}

uint16_t buddha_device::ide_0_cs1_r(offs_t offset, uint16_t mem_mask)
{
	return m_ata_0->cs1_swap_r((offset >> 1) & 0x07, mem_mask);
}

void buddha_device::ide_0_cs1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_ata_0->cs1_swap_w((offset >> 1) & 0x07, data, mem_mask);
}

uint16_t buddha_device::ide_1_cs0_r(offs_t offset, uint16_t mem_mask)
{
	return m_ata_1->cs0_swap_r((offset >> 1) & 0x07, mem_mask);
}

void buddha_device::ide_1_cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_ata_1->cs0_swap_w((offset >> 1) & 0x07, data, mem_mask);
}

uint16_t buddha_device::ide_1_cs1_r(offs_t offset, uint16_t mem_mask)
{
	return m_ata_1->cs1_swap_r((offset >> 1) & 0x07, mem_mask);
}

void buddha_device::ide_1_cs1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_ata_1->cs1_swap_w((offset >> 1) & 0x07, data, mem_mask);
}


//**************************************************************************
//  AUTOCONFIG
//**************************************************************************

void buddha_device::autoconfig_base_address(offs_t address)
{
	LOG("autoconfig_base_address received: 0x%06x\n", address);
	LOG("-> installing buddha\n");

	// stop responding to initial location
	m_zorro->space().unmap_readwrite(0xe80000, 0xe8ffff);

	// install buddha memory access to final location
	m_zorro->space().install_device(address, address + 0xffff, *this, &buddha_device::mmio_map);

	// install autoconfig handler to new location
	m_zorro->space().install_readwrite_handler(address, address + 0x7f,
		read16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_read)),
		write16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_write)), 0xffff);

	// we're done
	m_zorro->cfgout_w(0);
}

void buddha_device::cfgin_w(int state)
{
	LOG("cfgin_w (%d)\n", state);

	if (state == 0)
	{
		// buddha memory is also active at this point
		m_zorro->space().install_device(0xe80000, 0xe8ffff, *this, &buddha_device::mmio_map);

		// setup autoconfig
		autoconfig_board_type(BOARD_TYPE_ZORRO2);
		autoconfig_board_size(BOARD_SIZE_64K);
		autoconfig_link_into_memory(false);
		autoconfig_rom_vector_valid(true);
		autoconfig_multi_device(false);
		autoconfig_8meg_preferred(false);
		autoconfig_can_shutup(true);
		autoconfig_product(0);
		autoconfig_manufacturer(4626);
		autoconfig_serial(0x00000000);
		autoconfig_rom_vector(0x1000);

		// install autoconfig handler
		m_zorro->space().install_readwrite_handler(0xe80000, 0xe8007f,
			read16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_read)),
			write16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_write)), 0xffff);
	}
}

} // namespace bus::amiga::zorro
