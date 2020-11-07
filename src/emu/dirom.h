// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    dirom.h

    Interface to a rom, either through a memory map or a region

***************************************************************************/

#pragma once

#ifndef MAME_EMU_DIROM_H
#define MAME_EMU_DIROM_H

// Beware, DataWidth is 0-3
template<int AddrWidth, int DataWidth = 0, int AddrShift = 0, endianness_t Endian = ENDIANNESS_LITTLE> class device_rom_interface : public device_memory_interface
{
public:
	device_rom_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_rom_interface() = default;

	void set_device_rom_tag(const char *tag) { m_rom_tag = tag; }

	inline u8 read_byte(offs_t byteaddress) { return m_rom_cache.read_byte(byteaddress); }
	inline u16 read_word(offs_t byteaddress) { return m_rom_cache.read_word(byteaddress); }
	inline u32 read_dword(offs_t byteaddress) { return m_rom_cache.read_dword(byteaddress); }
	inline u64 read_qword(offs_t byteaddress) { return m_rom_cache.read_qword(byteaddress); }

	void set_rom(const void *base, u32 size);
	void set_rom_bank(int bank);

protected:
	virtual void rom_bank_updated() = 0;
	virtual space_config_vector memory_space_config() const override;

	void override_address_width(u8 width);

private:
	const char *m_rom_tag;
	address_space_config m_rom_config;
	typename memory_access<AddrWidth, DataWidth, AddrShift, Endian>::cache m_rom_cache;

	memory_bank_creator m_bank;
	int m_cur_bank, m_bank_count;

	virtual void interface_pre_start() override;
	virtual void interface_post_load() override;
};

#include "dirom.ipp"

#endif // MAME_EMU_DIROM_H
