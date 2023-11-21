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
DEFINE_DEVICE_TYPE(PCE_ROM_POPULOUS,  pce_populous_device,  "pce_populous",  "PCE Populous HuCard")
DEFINE_DEVICE_TYPE(PCE_ROM_SF2,       pce_sf2_device,       "pce_sf2",       "PCE Street Fighter 2 CE HuCard")
DEFINE_DEVICE_TYPE(PCE_ROM_TENNOKOE,  pce_tennokoe_device,  "pce_tennokoe",  "PCE Tennokoe Bank HuCard")


pce_rom_device::pce_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_pce_cart_interface(mconfig, *this)
{
}

pce_rom_device::pce_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pce_rom_device(mconfig, PCE_ROM_STD, tag, owner, clock)
{
}

pce_populous_device::pce_populous_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pce_rom_device(mconfig, PCE_ROM_POPULOUS, tag, owner, clock)
{
}

pce_sf2_device::pce_sf2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pce_rom_device(mconfig, PCE_ROM_SF2, tag, owner, clock)
	, m_rom_bank(*this, "rom_bank")
{
}

pce_tennokoe_device::pce_tennokoe_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pce_rom_device(mconfig, PCE_ROM_TENNOKOE, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------


void pce_sf2_device::device_reset()
{
	m_rom_bank->set_entry(0);
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

uint8_t pce_rom_device::rom_r(offs_t offset)
{
	int bank = offset / 0x20000;
	return m_rom[rom_bank_map[bank] * 0x20000 + (offset & 0x1ffff)];
}

void pce_rom_device::install_memory_handlers(address_space &space)
{
	space.install_read_handler(0x00000, 0xfffff, emu::rw_delegate(*this, FUNC(pce_rom_device::rom_r)));
}


void pce_populous_device::install_memory_handlers(address_space &space)
{
	pce_rom_device::install_memory_handlers(space);
	space.install_ram(0x80000, 0x87fff, &m_ram[0]);
}


void pce_sf2_device::bank_w(offs_t offset, uint8_t data)
{
	m_rom_bank->set_entry(offset & 3);
}


void pce_sf2_device::install_memory_handlers(address_space &space)
{
	m_rom_bank->configure_entries(0, 4, m_rom + 0x80000, 0x80000);
	space.install_rom(0x000000, 0x07ffff, m_rom);
	space.install_read_bank(0x080000, 0x0fffff, m_rom_bank);
	space.install_write_handler(0x001ff0, 0x001ff3, emu::rw_delegate(*this, FUNC(pce_sf2_device::bank_w)));
}


uint8_t pce_tennokoe_device::bram_r(offs_t offset)
{
	if (m_bram_locked)
		return 0xff;
	else
		return m_bram[offset & (m_bram_size-1)];
}

void pce_tennokoe_device::bram_w(offs_t offset, uint8_t data)
{
	if (!m_bram_locked)
		m_bram[offset & (m_bram_size-1)] = data;
}

void pce_tennokoe_device::bram_lock_w(offs_t offset, uint8_t data)
{
	// TODO: lock/unlock mechanism is a complete guess, needs real HW study
	m_bram_locked = (data == 0);
}

void pce_tennokoe_device::install_memory_handlers(address_space &space)
{
	space.install_rom(0x000000, 0x07ffff, m_rom);
	space.install_readwrite_handler(0x080000, 0x081fff, 0, 0x00e000, 0, emu::rw_delegate(*this, FUNC(pce_tennokoe_device::bram_r)), emu::rw_delegate(*this, FUNC(pce_tennokoe_device::bram_w)));
	// (writes to ports $c0000, $d0000, $f0000)
	space.install_write_handler(0x0f0000, 0x0f0000, 0, 0x00ffff, 0, emu::rw_delegate(*this, FUNC(pce_tennokoe_device::bram_lock_w)));
}

