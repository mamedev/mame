// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Wilbert Pol, Angelo Salese
/***********************************************************************************************************


 PC-Engine Arcade Card emulation

	TODO:
	- Proper Arcade Card Duo support

 ***********************************************************************************************************/


#include "emu.h"
#include "pce_acard.h"



//-------------------------------------------------
//  pce_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(PCE_ROM_ACARD_DUO, pce_acard_duo_device, "pce_acard_duo", "Arcade Card Duo")
DEFINE_DEVICE_TYPE(PCE_ROM_ACARD_PRO, pce_acard_pro_device, "pce_acard_pro", "Arcade Card Pro")


pce_acard_duo_device::pce_acard_duo_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_pce_cart_interface( mconfig, *this )
{
}

pce_acard_duo_device::pce_acard_duo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pce_acard_duo_device(mconfig, PCE_ROM_ACARD_DUO, tag, owner, clock)
{
}

pce_acard_pro_device::pce_acard_pro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pce_acard_duo_device(mconfig, PCE_ROM_ACARD_PRO, tag, owner, clock)
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------


void pce_acard_duo_device::device_start()
{
	/* Set up Arcade Card RAM buffer */
	m_dram = make_unique_clear<uint8_t[]>(0x200000);

	save_pointer(NAME(m_dram), 0x200000);
	save_item(NAME(m_ctrl));
	save_item(NAME(m_base_addr));
	save_item(NAME(m_addr_offset));
	save_item(NAME(m_addr_inc));
	save_item(NAME(m_shift));
	save_item(NAME(m_shift_reg));
	save_item(NAME(m_rotate_reg));
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

uint8_t pce_acard_duo_device::read_ram(offs_t offset)
{
	if (offset >= 0x0000 && offset < 0x8000)
		return peripheral_r((offset & 0x6000) >> 9);
	
	return 0xff;
}

void pce_acard_duo_device::write_ram(offs_t offset, uint8_t data)
{
	if (offset >= 0x0000 && offset < 0x8000)
		peripheral_w((offset & 0x6000) >> 9, data);
}

uint8_t pce_acard_pro_device::read_cart(offs_t offset)
{
	if (!m_ram.empty() && offset >= 0xd0000)
		return m_ram[offset - 0xd0000];

	const int bank = offset / 0x20000;
	return m_rom[rom_bank_map[bank] * 0x20000 + (offset & 0x1ffff)];
}

void pce_acard_pro_device::write_cart(offs_t offset, uint8_t data)
{
	if (!m_ram.empty() && offset >= 0xd0000)
		m_ram[offset - 0xd0000] = data;
}

uint8_t pce_acard_pro_device::read_ex(offs_t offset)
{
	switch (offset & 0x0f)
	{
		case 0x1: return 0xaa;
		case 0x2: return 0x55;
		case 0x3: return 0x00;
		case 0x5: return 0xaa;
		case 0x6: return 0x55;
		case 0x7: return 0x03;
	}
	return 0x00;
}

uint8_t pce_acard_duo_device::peripheral_r(offs_t offset)
{
	uint8_t r_num;

	if ((offset & 0xe0) == 0xe0)
	{
		switch (offset & 0xef)
		{
			case 0xe0: return (m_shift >> 0)  & 0xff;
			case 0xe1: return (m_shift >> 8)  & 0xff;
			case 0xe2: return (m_shift >> 16) & 0xff;
			case 0xe3: return (m_shift >> 24) & 0xff;
			case 0xe4: return m_shift_reg;
			case 0xe5: return m_rotate_reg;
			case 0xee: return 0x10;
			case 0xef: return 0x51;
		}

		return 0;
	}

	r_num = (offset & 0x30) >> 4;

	switch (offset & 0x0f)
	{
		case 0x00:
		case 0x01:
		{
			uint8_t res;
			if (m_ctrl[r_num] & 2)
				res = m_dram[(m_base_addr[r_num] + m_addr_offset[r_num]) & 0x1fffff];
			else
				res = m_dram[m_base_addr[r_num] & 0x1fffff];

			if ((!machine().side_effects_disabled()) && (m_ctrl[r_num] & 0x1))
			{
				if (m_ctrl[r_num] & 0x10)
				{
					m_base_addr[r_num] += m_addr_inc[r_num];
					m_base_addr[r_num] &= 0xffffff;
				}
				else
				{
					m_addr_offset[r_num] += m_addr_inc[r_num];
				}
			}

			return res;
		}
		case 0x02: return (m_base_addr[r_num] >> 0) & 0xff;
		case 0x03: return (m_base_addr[r_num] >> 8) & 0xff;
		case 0x04: return (m_base_addr[r_num] >> 16) & 0xff;
		case 0x05: return (m_addr_offset[r_num] >> 0) & 0xff;
		case 0x06: return (m_addr_offset[r_num] >> 8) & 0xff;
		case 0x07: return (m_addr_inc[r_num] >> 0) & 0xff;
		case 0x08: return (m_addr_inc[r_num] >> 8) & 0xff;
		case 0x09: return m_ctrl[r_num];
		default:   return 0;
	}
}

void pce_acard_duo_device::peripheral_w(offs_t offset, uint8_t data)
{
	uint8_t w_num;

	if ((offset & 0xe0) == 0xe0)
	{
		switch (offset & 0x0f)
		{
			case 0: m_shift = (data & 0xff) | (m_shift & 0xffffff00); break;
			case 1: m_shift = (data << 8)   | (m_shift & 0xffff00ff); break;
			case 2: m_shift = (data << 16)  | (m_shift & 0xff00ffff); break;
			case 3: m_shift = (data << 24)  | (m_shift & 0x00ffffff); break;
			case 4:
			{
				m_shift_reg = data & 0x0f;

				if (m_shift_reg != 0)
				{
					m_shift = (m_shift_reg < 8) ?
					(m_shift << m_shift_reg)
					: (m_shift >> (16 - m_shift_reg));
				}
			}
				break;
			case 5:
			{
				m_rotate_reg = data;

				if (m_rotate_reg != 0)
				{
					m_shift = (m_rotate_reg < 8) ? 
					((m_shift << m_rotate_reg) | (m_shift >> (32 - m_rotate_reg)))
					: ((m_shift >> (16 - m_rotate_reg)) | (m_shift << (32 - (16 - m_rotate_reg))));
				}
			}
				break;
		}
	}
	else
	{
		w_num = (offset & 0x30) >> 4;

		switch (offset & 0x0f)
		{
			case 0x00:
			case 0x01:
				if (m_ctrl[w_num] & 2)
					m_dram[(m_base_addr[w_num] + m_addr_offset[w_num]) & 0x1fffff] = data;
				else
					m_dram[m_base_addr[w_num] & 0x1FFFFF] = data;

				if (m_ctrl[w_num] & 0x1)
				{
					if (m_ctrl[w_num] & 0x10)
					{
						m_base_addr[w_num] += m_addr_inc[w_num];
						m_base_addr[w_num] &= 0xffffff;
					}
					else
					{
						m_addr_offset[w_num] += m_addr_inc[w_num];
					}
				}

				break;

			case 0x02: m_base_addr[w_num] = (data & 0xff) | (m_base_addr[w_num] & 0xffff00);  break;
			case 0x03: m_base_addr[w_num] = (data << 8) | (m_base_addr[w_num] & 0xff00ff);        break;
			case 0x04: m_base_addr[w_num] = (data << 16) | (m_base_addr[w_num] & 0x00ffff);   break;
			case 0x05: m_addr_offset[w_num] = (data & 0xff) | (m_addr_offset[w_num] & 0xff00);    break;
			case 0x06:
				m_addr_offset[w_num] = (data << 8) | (m_addr_offset[w_num] & 0x00ff);

				if ((m_ctrl[w_num] & 0x60) == 0x40)
				{
					m_base_addr[w_num] += m_addr_offset[w_num] + ((m_ctrl[w_num] & 0x08) ? 0xff0000 : 0);
					m_base_addr[w_num] &= 0xffffff;
				}
				break;
			case 0x07: m_addr_inc[w_num] = (data & 0xff) | (m_addr_inc[w_num] & 0xff00);      break;
			case 0x08: m_addr_inc[w_num] = (data << 8) | (m_addr_inc[w_num] & 0x00ff);            break;
			case 0x09: m_ctrl[w_num] = data & 0x7f;                                              break;
			case 0x0a:
				if ((m_ctrl[w_num] & 0x60) == 0x60)
				{
					m_base_addr[w_num] += m_addr_offset[w_num];
					m_base_addr[w_num] &= 0xffffff;
				}
				break;
		}
	}
}
