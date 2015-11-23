// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    VIA VT82C496G "Green PC" system chipset

*/

#include "vt82c496.h"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

const device_type VT82C496 = &device_creator<vt82c496_device>;


vt82c496_device::vt82c496_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, VT82C496, "VIA VT82C496G system chipset", tag, owner, clock, "vt82c496", __FILE__), m_cpu_tag(nullptr), m_region_tag(nullptr), m_space(nullptr), m_ram(nullptr), m_rom(nullptr), m_reg_select(0)
	{
}

void vt82c496_device::device_start()
{
	/* get address space we are working on */
	device_t *cpu = machine().device(m_cpu_tag);
	assert(cpu != NULL);

	m_space = &cpu->memory().space(AS_PROGRAM);

	/* get rom region */
	m_rom = machine().root_device().memregion(m_region_tag)->base();

	save_pointer(m_reg,"Registers",0x100);

	m_ram = machine().device<ram_device>(RAM_TAG);
}

void vt82c496_device::device_reset()
{
	memset(m_reg,0,0x100);

	// set up default ROM banking
	m_space->install_read_bank(0xc0000,0xc3fff,0,0,"bios_c0_r");
	m_space->install_read_bank(0xc4000,0xc7fff,0,0,"bios_c4_r");
	m_space->install_read_bank(0xc8000,0xcbfff,0,0,"bios_c8_r");
	m_space->install_read_bank(0xcc000,0xcffff,0,0,"bios_cc_r");
	m_space->install_read_bank(0xd0000,0xd3fff,0,0,"bios_d0_r");
	m_space->install_read_bank(0xd4000,0xd7fff,0,0,"bios_d4_r");
	m_space->install_read_bank(0xd8000,0xdbfff,0,0,"bios_d8_r");
	m_space->install_read_bank(0xdc000,0xdffff,0,0,"bios_dc_r");
	m_space->install_read_bank(0xe0000,0xeffff,0,0,"bios_e0_r");
	m_space->install_read_bank(0xf0000,0xfffff,0,0,"bios_f0_r");
	m_space->nop_write(0xc0000,0xfffff,0,0);
	machine().root_device().membank("bios_c0_r")->set_base(m_rom);
	machine().root_device().membank("bios_c4_r")->set_base(m_rom+0x4000);
	machine().root_device().membank("bios_c8_r")->set_base(m_rom+0x8000);
	machine().root_device().membank("bios_cc_r")->set_base(m_rom+0xc000);
	machine().root_device().membank("bios_d0_r")->set_base(m_rom+0x10000);
	machine().root_device().membank("bios_d4_r")->set_base(m_rom+0x14000);
	machine().root_device().membank("bios_d8_r")->set_base(m_rom+0x18000);
	machine().root_device().membank("bios_dc_r")->set_base(m_rom+0x1c000);
	machine().root_device().membank("bios_e0_r")->set_base(m_rom+0x20000);
	machine().root_device().membank("bios_f0_r")->set_base(m_rom+0x30000);
}

READ8_MEMBER(vt82c496_device::read)
{
	if(offset == 1)
		return m_reg[m_reg_select];
	return 0x00;
}

WRITE8_MEMBER(vt82c496_device::write)
{
	if(offset == 0)
		m_reg_select = data;
	if(offset == 1)
	{
		m_reg[m_reg_select] = data;
		switch(m_reg_select)
		{
			case 0x30:
				update_mem_c0(data);
				break;
			case 0x31:
				update_mem_d0(data);
				break;
			case 0x32:
				update_mem_e0(data);
				break;
		}
	}
}

void vt82c496_device::update_mem_c0(UINT8 data)
{
	if(data & 0x80)
		machine().root_device().membank("bios_cc_r")->set_base(m_ram->pointer()+0xcc000);
	else
		machine().root_device().membank("bios_cc_r")->set_base(m_rom+0xc000);
	if(data & 0x40)
	{
		m_space->install_write_bank(0xcc000,0xcffff,0,0,"bios_cc_w");
		machine().root_device().membank("bios_cc_w")->set_base(m_ram->pointer()+0xcc000);
	}
	else
		m_space->nop_write(0xcc000,0xcffff,0,0);

	if(data & 0x20)
		machine().root_device().membank("bios_c8_r")->set_base(m_ram->pointer()+0xc8000);
	else
		machine().root_device().membank("bios_c8_r")->set_base(m_rom+0x8000);
	if(data & 0x10)
	{
		m_space->install_write_bank(0xc8000,0xcbfff,0,0,"bios_c8_w");
		machine().root_device().membank("bios_c8_w")->set_base(m_ram->pointer()+0xc8000);
	}
	else
		m_space->nop_write(0xc8000,0xcbfff,0,0);

	if(data & 0x08)
		machine().root_device().membank("bios_c4_r")->set_base(m_ram->pointer()+0xc4000);
	else
		machine().root_device().membank("bios_c4_r")->set_base(m_rom+0x4000);
	if(data & 0x04)
	{
		m_space->install_write_bank(0xc4000,0xc7fff,0,0,"bios_c4_w");
		machine().root_device().membank("bios_c4_w")->set_base(m_ram->pointer()+0xc4000);
	}
	else
		m_space->nop_write(0xc4000,0xc7fff,0,0);

	if(data & 0x02)
		machine().root_device().membank("bios_c0_r")->set_base(m_ram->pointer()+0xc0000);
	else
		machine().root_device().membank("bios_c0_r")->set_base(m_rom+0);
	if(data & 0x01)
	{
		m_space->install_write_bank(0xc0000,0xc3fff,0,0,"bios_c0_w");
		machine().root_device().membank("bios_c0_w")->set_base(m_ram->pointer()+0xc0000);
	}
	else
		m_space->nop_write(0xc0000,0xc3fff,0,0);
}

void vt82c496_device::update_mem_d0(UINT8 data)
{
	if(data & 0x80)
		machine().root_device().membank("bios_dc_r")->set_base(m_ram->pointer()+0xdc000);
	else
		machine().root_device().membank("bios_dc_r")->set_base(m_rom+0x1c000);
	if(data & 0x40)
	{
		m_space->install_write_bank(0xdc000,0xdffff,0,0,"bios_dc_w");
		machine().root_device().membank("bios_dc_w")->set_base(m_ram->pointer()+0xdc000);
	}
	else
		m_space->nop_write(0xdc000,0xdffff,0,0);

	if(data & 0x20)
		machine().root_device().membank("bios_d8_r")->set_base(m_ram->pointer()+0xd8000);
	else
		machine().root_device().membank("bios_d8_r")->set_base(m_rom+0x18000);
	if(data & 0x10)
	{
		m_space->install_write_bank(0xd8000,0xdbfff,0,0,"bios_d8_w");
		machine().root_device().membank("bios_d8_w")->set_base(m_ram->pointer()+0xd8000);
	}
	else
		m_space->nop_write(0xd8000,0xdbfff,0,0);

	if(data & 0x08)
		machine().root_device().membank("bios_d4_r")->set_base(m_ram->pointer()+0xd4000);
	else
		machine().root_device().membank("bios_d4_r")->set_base(m_rom+0x14000);
	if(data & 0x04)
	{
		m_space->install_write_bank(0xd4000,0xd7fff,0,0,"bios_d4_w");
		machine().root_device().membank("bios_d4_w")->set_base(m_ram->pointer()+0xd4000);
	}
	else
		m_space->nop_write(0xd4000,0xd7fff,0,0);

	if(data & 0x02)
		machine().root_device().membank("bios_d0_r")->set_base(m_ram->pointer()+0xd0000);
	else
		machine().root_device().membank("bios_d0_r")->set_base(m_rom+0x10000);
	if(data & 0x01)
	{
		m_space->install_write_bank(0xd0000,0xd3fff,0,0,"bios_d0_w");
		machine().root_device().membank("bios_d0_w")->set_base(m_ram->pointer()+0xd0000);
	}
	else
		m_space->nop_write(0xd0000,0xd3fff,0,0);
}

void vt82c496_device::update_mem_e0(UINT8 data)
{
	if(data & 0x80)
		machine().root_device().membank("bios_e0_r")->set_base(m_ram->pointer()+0xe0000);
	else
		machine().root_device().membank("bios_e0_r")->set_base(m_rom+0x20000);

	if(data & 0x40)
	{
		m_space->install_write_bank(0xe0000,0xeffff,0,0,"bios_e0_w");
		machine().root_device().membank("bios_e0_w")->set_base(m_ram->pointer()+0xe0000);
	}
	else
		m_space->nop_write(0xe0000,0xeffff,0,0);

	if(data & 0x20)
		machine().root_device().membank("bios_f0_r")->set_base(m_ram->pointer()+0xf0000);
	else
		machine().root_device().membank("bios_f0_r")->set_base(m_rom+0x30000);

	if(data & 0x10)
	{
		m_space->install_write_bank(0xf0000,0xfffff,0,0,"bios_f0_w");
		machine().root_device().membank("bios_f0_w")->set_base(m_ram->pointer()+0xf0000);
	}
	else
		m_space->nop_write(0xf0000,0xfffff,0,0);
}
