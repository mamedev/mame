// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Angelo Salese
/***********************************************************************************************************


 PC-Engine Arcade Card emulation

	TODO:
	- Proper Arcade Card Duo support

 ***********************************************************************************************************/


#include "emu.h"
#include "pce_acard.h"



//-------------------------------------------------
//  pce_acard_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(PCE_ROM_ACARD_DUO, pce_acard_duo_device, "pce_acard_duo", "Arcade Card Duo")
DEFINE_DEVICE_TYPE(PCE_ROM_ACARD_PRO, pce_acard_pro_device, "pce_acard_pro", "Arcade Card Pro")


pce_acard_duo_device::pce_acard_duo_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_pce_cart_interface( mconfig, *this )
	, m_ram(*this, "ram", 0x200000, ENDIANNESS_LITTLE)
	, m_shift(0)
	, m_shift_reg(0)
	, m_rotate_reg(0)
{
}

pce_acard_duo_device::pce_acard_duo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pce_acard_duo_device(mconfig, PCE_ROM_ACARD_DUO, tag, owner, clock)
{
}

pce_acard_pro_device::pce_acard_pro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pce_acard_duo_device(mconfig, PCE_ROM_ACARD_PRO, tag, owner, clock)
	, m_scdsys()
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------


void pce_acard_duo_device::device_start()
{
	save_item(STRUCT_MEMBER(m_port, m_ctrl));
	save_item(STRUCT_MEMBER(m_port, m_base_addr));
	save_item(STRUCT_MEMBER(m_port, m_addr_offset));
	save_item(STRUCT_MEMBER(m_port, m_addr_inc));
	save_item(NAME(m_shift));
	save_item(NAME(m_shift_reg));
	save_item(NAME(m_rotate_reg));
}

void pce_acard_pro_device::device_start()
{
	pce_acard_duo_device::device_start();
	
	m_scdsys.init(*this);
	m_scdsys.set_region(false);
}

void pce_acard_duo_device::device_reset()
{
	for (auto &port : m_port)
	{
		port.m_ctrl = 0;
		port.m_base_addr = 0;
		port.m_addr_offset = 0;
		port.m_addr_inc = 0;
	}
	m_shift = 0;
	m_shift_reg = 0;
	m_rotate_reg = 0;
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

void pce_acard_duo_device::install_memory_handlers(address_space &space)
{
	space.install_readwrite_handler(0x080000, 0x087fff, emu::rw_delegate(*this, FUNC(pce_acard_duo_device::ram_r)), emu::rw_delegate(*this, FUNC(pce_acard_duo_device::ram_w)));
	// TODO: mirrored?
	space.install_readwrite_handler(0x1ffa00, 0x1ffaff, 0, 0x100, 0, emu::rw_delegate(*this, FUNC(pce_acard_duo_device::peripheral_r)), emu::rw_delegate(*this, FUNC(pce_acard_duo_device::peripheral_w)));
}

void pce_acard_pro_device::install_memory_handlers(address_space &space)
{
	space.install_rom(0x000000, 0x03ffff, 0x040000, m_rom); // TODO: underdumped or mirrored?
	space.install_ram(0x0d0000, 0x0fffff, m_scdsys.ram());
	space.install_read_handler(0x1ff8c0, 0x1ff8c7, 0, 0x130, 0, emu::rw_delegate(*this, FUNC(pce_acard_pro_device::register_r)));
	pce_acard_duo_device::install_memory_handlers(space);
}

uint8_t pce_acard_duo_device::ram_r(offs_t offset)
{
	return peripheral_r((offset & 0x6000) >> 9);
}

void pce_acard_duo_device::ram_w(offs_t offset, uint8_t data)
{
	peripheral_w((offset & 0x6000) >> 9, data);
}

uint8_t pce_acard_duo_device::peripheral_r(offs_t offset)
{
	if ((offset & 0xe0) == 0xe0)
	{
		switch (offset & 0x1f)
		{
			case 0x00: return (m_shift >> 0)  & 0xff;
			case 0x01: return (m_shift >> 8)  & 0xff;
			case 0x02: return (m_shift >> 16) & 0xff;
			case 0x03: return (m_shift >> 24) & 0xff;
			case 0x04: return m_shift_reg;
			case 0x05: return m_rotate_reg;
			case 0x1c: return 0x00;
			case 0x1d: return 0x00;
			case 0x1e: return 0x10; // Version number (MSB?)
			case 0x1f: return 0x51; // Arcade Card ID
		}

		return 0xff;
	}

	dram_port &port = m_port[(offset & 0x30) >> 4];

	switch (offset & 0x8f)
	{
		case 0x00:
		case 0x01:
		{
			uint8_t const res = m_ram[port.ram_addr()];

			if (!machine().side_effects_disabled())
				port.addr_increment();

			return res;
		}
		case 0x02: return (port.m_base_addr >> 0) & 0xff;
		case 0x03: return (port.m_base_addr >> 8) & 0xff;
		case 0x04: return (port.m_base_addr >> 16) & 0xff;
		case 0x05: return (port.m_addr_offset >> 0) & 0xff;
		case 0x06: return (port.m_addr_offset >> 8) & 0xff;
		case 0x07: return (port.m_addr_inc >> 0) & 0xff;
		case 0x08: return (port.m_addr_inc >> 8) & 0xff;
		case 0x09: return port.m_ctrl;
		default:   return 0xff;
	}
}

void pce_acard_duo_device::peripheral_w(offs_t offset, uint8_t data)
{
	if ((offset & 0xe0) == 0xe0)
	{
		switch (offset & 0x0f)
		{
			case 0: m_shift = (data & 0xff) | (m_shift & 0xffffff00); break;
			case 1: m_shift = (data << 8)   | (m_shift & 0xffff00ff); break;
			case 2: m_shift = (data << 16)  | (m_shift & 0xff00ffff); break;
			case 3: m_shift = (data << 24)  | (m_shift & 0x00ffffff); break;
			case 4:
				m_shift_reg = data & 0x0f;

				if (m_shift_reg != 0)
				{
					m_shift = (m_shift_reg < 8)
							? (m_shift << m_shift_reg)
							: (m_shift >> (16 - m_shift_reg));
				}
				break;
			case 5:
				m_rotate_reg = data & 0x0f;

				if (m_rotate_reg != 0)
				{
					m_shift = (m_rotate_reg < 8)
							? ((m_shift << m_rotate_reg) | (m_shift >> (32 - m_rotate_reg)))
							: ((m_shift >> (16 - m_rotate_reg)) | (m_shift << (32 - (16 - m_rotate_reg))));
				}
				break;
		}
	}
	else
	{
		dram_port &port = m_port[(offset & 0x30) >> 4];

		switch (offset & 0x8f)
		{
			case 0x00:
			case 0x01:
				m_ram[port.ram_addr()] = data;

				port.addr_increment();
				break;

			case 0x02: port.m_base_addr = (data & 0xff) | (port.m_base_addr & 0xffff00); break;
			case 0x03: port.m_base_addr = (data << 8) | (port.m_base_addr & 0xff00ff); break;
			case 0x04: port.m_base_addr = (data << 16) | (port.m_base_addr & 0x00ffff); break;
			case 0x05:
				port.m_addr_offset = (data & 0xff) | (port.m_addr_offset & 0xff00);

				if ((port.m_ctrl & 0x60) == 0x20)
					port.adjust_addr();
				break;
			case 0x06:
				port.m_addr_offset = (data << 8) | (port.m_addr_offset & 0x00ff);

				if ((port.m_ctrl & 0x60) == 0x40)
					port.adjust_addr();
				break;
			case 0x07: port.m_addr_inc = (data & 0xff) | (port.m_addr_inc & 0xff00); break;
			case 0x08: port.m_addr_inc = (data << 8) | (port.m_addr_inc & 0x00ff); break;
			case 0x09: port.m_ctrl = data & 0x7f; break;
			case 0x0a:
				if ((port.m_ctrl & 0x60) == 0x60)
					port.adjust_addr();
				break;
		}
	}
}
