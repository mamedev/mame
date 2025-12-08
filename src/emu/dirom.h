// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    dirom.h

    Interface to a rom, either through a memory map or a region

***************************************************************************/
#ifndef MAME_EMU_DIROM_H
#define MAME_EMU_DIROM_H

#pragma once


// Beware, DataWidth is 0-3
template<int AddrWidth, int DataWidth = 0, int AddrShift = 0, endianness_t Endian = ENDIANNESS_LITTLE> class device_rom_interface : public device_memory_interface
{
public:
	device_rom_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_rom_interface() = default;

	template <typename... T> void set_map(T &&... args) { set_addrmap(0, std::forward<T>(args)...); }
	template <typename T> void set_device_rom_tag(T &&tag) { m_rom_region.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_space(T &&tag, int spacenum) { m_rom_space.set_tag(tag, spacenum); }

	u8 read_byte(offs_t addr) { return m_rom_cache.read_byte(addr); }
	u16 read_word(offs_t addr) { return m_rom_cache.read_word(addr); }
	u32 read_dword(offs_t addr) { return m_rom_cache.read_dword(addr); }
	u64 read_qword(offs_t addr) { return m_rom_cache.read_qword(addr); }

	void set_rom(const void *base, u32 size);
	void set_rom_bank(int bank);

protected:
	typename memory_access<AddrWidth, DataWidth, AddrShift, Endian>::cache m_rom_cache;

	virtual void rom_bank_pre_change() { }
	virtual void rom_bank_post_change() { }
	virtual space_config_vector memory_space_config() const override;

	void override_address_width(u8 width);

private:
	optional_memory_region m_rom_region;
	optional_address_space m_rom_space;
	address_space_config m_rom_config;

	memory_bank_creator m_bank;
	u32 m_cur_bank, m_bank_count;

	virtual void interface_validity_check(validity_checker &valid) const override;
	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;
	virtual void interface_post_load() override;
};

#include "dirom.ipp"

#endif // MAME_EMU_DIROM_H
