// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 PC-Engine & Turbografx-16 cart emulation


 ***********************************************************************************************************/


#include "emu.h"
#include "pce_rom.h"



//-------------------------------------------------
//  pce_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(PCE_ROM_STD,      pce_rom_device,      "pce_rom",      "PCE/TG16 Carts")
DEFINE_DEVICE_TYPE(PCE_ROM_CDSYS3,   pce_cdsys3_device,   "pce_cdsys3",   "PCE/TG16 CD-System Cart v3.00")
DEFINE_DEVICE_TYPE(PCE_ROM_POPULOUS, pce_populous_device, "pce_populous", "PCE Populous Cart")
DEFINE_DEVICE_TYPE(PCE_ROM_SF2,      pce_sf2_device,      "pce_sf2",      "PCE Street Fighter 2 CE Cart")
DEFINE_DEVICE_TYPE(PCE_ROM_TENNOKOE, pce_tennokoe_device, "pce_tennokoe", "PCE Tennokoe Bank Cart")


pce_rom_device::pce_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_pce_cart_interface( mconfig, *this )
{
}

pce_rom_device::pce_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pce_rom_device(mconfig, PCE_ROM_STD, tag, owner, clock)
{
}

pce_cdsys3_device::pce_cdsys3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pce_rom_device(mconfig, PCE_ROM_CDSYS3, tag, owner, clock)
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
