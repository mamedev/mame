// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 PC-Engine & Turbografx-16 HuCard emulation


 ***********************************************************************************************************/


#include "emu.h"
#include "pce_rom.h"



//-------------------------------------------------
//  pce_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(PCE_ROM_STD,       pce_rom_device,       "pce_rom",       "PCE/TG16 HuCards")
DEFINE_DEVICE_TYPE(PCE_ROM_CDSYS3J,   pce_cdsys3j_device,   "pce_cdsys3j",   "PCE CD-System HuCard v3.00")
DEFINE_DEVICE_TYPE(PCE_ROM_CDSYS3U,   pce_cdsys3u_device,   "pce_cdsys3u",   "TG16 CD-System HuCard v3.00")
DEFINE_DEVICE_TYPE(PCE_ROM_POPULOUS,  pce_populous_device,  "pce_populous",  "PCE Populous HuCard")
DEFINE_DEVICE_TYPE(PCE_ROM_SF2,       pce_sf2_device,       "pce_sf2",       "PCE Street Fighter 2 CE HuCard")
DEFINE_DEVICE_TYPE(PCE_ROM_TENNOKOE,  pce_tennokoe_device,  "pce_tennokoe",  "PCE Tennokoe Bank HuCard")
DEFINE_DEVICE_TYPE(PCE_ROM_ACARD_PRO, pce_acard_pro_device, "pce_acard_pro", "PCE Arcade Card Pro HuCard")


pce_rom_device::pce_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_pce_cart_interface( mconfig, *this )
{
}

pce_rom_device::pce_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pce_rom_device(mconfig, PCE_ROM_STD, tag, owner, clock)
{
}

pce_cdsys3_device::pce_cdsys3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool region)
	: pce_rom_device(mconfig, type, tag, owner, clock)
	, m_region(region)
{
}

pce_cdsys3j_device::pce_cdsys3j_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pce_cdsys3_device(mconfig, PCE_ROM_CDSYS3J, tag, owner, clock, false)
{
}

pce_cdsys3u_device::pce_cdsys3u_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pce_cdsys3_device(mconfig, PCE_ROM_CDSYS3U, tag, owner, clock, true)
{
}

pce_populous_device::pce_populous_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pce_rom_device(mconfig, PCE_ROM_POPULOUS, tag, owner, clock)
{
}

pce_sf2_device::pce_sf2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pce_rom_device(mconfig, PCE_ROM_SF2, tag, owner, clock), m_bank_base(0)
{
}

pce_tennokoe_device::pce_tennokoe_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pce_rom_device(mconfig, PCE_ROM_TENNOKOE, tag, owner, clock),
	device_nvram_interface(mconfig, *this)
{
}

pce_acard_pro_device::pce_acard_pro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pce_cdsys3_device(mconfig, PCE_ROM_ACARD_PRO, tag, owner, clock, false)
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------


void pce_sf2_device::device_start()
{
	save_item(NAME(m_bank_base));
}

void pce_sf2_device::device_reset()
{
	m_bank_base = 0;
}

void pce_tennokoe_device::device_start()
{
	save_item(NAME(m_bram));
	save_item(NAME(m_bram_locked));
}

void pce_tennokoe_device::device_reset()
{
	m_bram_locked = 1;
}

// Tennokoe Bank is a special HuCard containing x4 Backup RAM banks,
// the software can transfer
void pce_tennokoe_device::nvram_default()
{
	memset(m_bram, 0xff, m_bram_size);
	// easter egg: load copy of a BRAM (debug leftover?) inside bank 4.
	// Contains 14 save blocks, mostly at the end of the various games.
	// Not entirely correct but not incorrect as well to just unlock these saves for public use,
	// for testing reasons and for a slim chance of actually be supposed as default data in the bank(s)
	// File list:
	// 001 NEUTOPIA1
	// 002 MOTURBO-1
	// 003 MOMODE2-1
	// 004 MOMOKATSU.
	// 005 BOMBERMAN1
	// 006 POPULOUS.
	// 007 ADVENTURE1 (Starts in HuMan form at the village with 983040 gold, basically before the last dungeon);
	// 008 TENGAI1
	// 009 COBRA1
	// 010 YS.DATA.01 (right before the switch between Ys 1 and 2, read book to move on);
	// 011 YS.DATA.02
	// 012 YS3.DAT.01 (right before the end, go to the right bridge for the ending);
	// 013 URUSEI1
	// 014 MITUBATI1
	// TODO: document the other saves.
	memcpy(m_bram + 0x1800, m_rom + 0x8800, 0x800);
}

bool pce_tennokoe_device::nvram_read(util::read_stream &file)
{
	size_t actual_size;
	return !file.read(m_bram, m_bram_size, actual_size) && actual_size == m_bram_size;
}

bool pce_tennokoe_device::nvram_write(util::write_stream &file)
{
	size_t actual_size;
	return !file.write(m_bram, m_bram_size, actual_size) && actual_size == m_bram_size;
}

void pce_acard_pro_device::device_start()
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

uint8_t pce_rom_device::read_cart(offs_t offset)
{
	int bank = offset / 0x20000;
	return m_rom[rom_bank_map[bank] * 0x20000 + (offset & 0x1ffff)];
}


uint8_t pce_cdsys3_device::read_cart(offs_t offset)
{
	int bank = offset / 0x20000;
	if (!m_ram.empty() && offset >= 0xd0000)
		return m_ram[offset - 0xd0000];

	return m_rom[rom_bank_map[bank] * 0x20000 + (offset & 0x1ffff)];
}

void pce_cdsys3_device::write_cart(offs_t offset, uint8_t data)
{
	if (!m_ram.empty() && offset >= 0xd0000)
		m_ram[offset - 0xd0000] = data;
}

uint8_t pce_cdsys3_device::read_ex(offs_t offset)
{
	switch (offset & 0x0f)
	{
		case 0x1: return 0xaa;
		case 0x2: return 0x55;
		case 0x3: return 0x00;
		case 0x5: return (m_region) ? 0x55 : 0xaa;
		case 0x6: return (m_region) ? 0xaa : 0x55;
		case 0x7: return 0x03;
	}
	return 0x00;
}


uint8_t pce_populous_device::read_cart(offs_t offset)
{
	int bank = offset / 0x20000;
	if (!m_ram.empty() && offset >= 0x80000 && offset < 0x88000)
		return m_ram[offset & 0x7fff];

	return m_rom[rom_bank_map[bank] * 0x20000 + (offset & 0x1ffff)];
}

void pce_populous_device::write_cart(offs_t offset, uint8_t data)
{
	if (!m_ram.empty() && offset >= 0x80000 && offset < 0x88000)
		m_ram[offset & 0x7fff] = data;
}


uint8_t pce_sf2_device::read_cart(offs_t offset)
{
	if (offset < 0x80000)
		return m_rom[offset];
	else
		return m_rom[0x80000 + m_bank_base * 0x80000 + (offset & 0x7ffff)];
}

void pce_sf2_device::write_cart(offs_t offset, uint8_t data)
{
	if (offset >= 0x1ff0 && offset < 0x1ff4)
		m_bank_base = offset & 3;
}

uint8_t pce_tennokoe_device::read_cart(offs_t offset)
{
	switch((offset & 0xf0000) >> 16)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			return m_rom[offset];
		case 8:
			if (m_bram_locked)
				return 0xff;
			else
				return m_bram[offset & (m_bram_size-1)];
	}

	logerror("tennokoe: ROM reading at %06x\n",offset);
	return 0xff;
}

void pce_tennokoe_device::write_cart(offs_t offset, uint8_t data)
{
	switch((offset & 0xf0000) >> 16)
	{
		case 8:
			if(!m_bram_locked)
				m_bram[offset & (m_bram_size-1)] = data;
			break;
		case 0xf:
			// TODO: lock/unlock mechanism is a complete guess, needs real HW study
			// (writes to ports $c0000, $d0000, $f0000)
			m_bram_locked = (data == 0);
			[[fallthrough]];
		default:
			logerror("tennokoe: ROM writing at %06x %02x\n",offset,data);
			break;
	}
}

uint8_t pce_acard_pro_device::read_cart(offs_t offset)
{
	if (offset >= 0x80000 && offset < 0x88000)
		return peripheral_r((offset & 0x6000) >> 9);

	return pce_cdsys3_device::read_cart(offset);
}

void pce_acard_pro_device::write_cart(offs_t offset, uint8_t data)
{
	if (offset >= 0x80000 && offset < 0x88000)
		peripheral_w((offset & 0x6000) >> 9, data);
	else
		pce_cdsys3_device::write_cart(offset, data);
}

uint8_t pce_acard_pro_device::peripheral_r(offs_t offset)
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

void pce_acard_pro_device::peripheral_w(offs_t offset, uint8_t data)
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
