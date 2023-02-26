// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Shared Game Boy cartridge helpers

 ***************************************************************************/
#ifndef MAME_BUS_GAMEBOY_CARTBASE_H
#define MAME_BUS_GAMEBOY_CARTBASE_H

#pragma once

#include "slot.h"

#include <string>


namespace bus::gameboy {

//**************************************************************************
//  16 KiB fixed at 0x0000, 16 KiB switchable at 0x4000
//**************************************************************************

class mbc_device_base : public device_t, public device_gb_cart_interface
{
protected:
	mbc_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;

	void set_bank_bits_rom(unsigned bits)
	{
		m_bank_bits_rom = bits;
	}

	bool check_rom(std::string &message) ATTR_COLD;
	void install_rom() ATTR_COLD;
	void configure_bank_rom() ATTR_COLD;

	void set_bank_rom(u16 entry);

private:
	static inline constexpr unsigned PAGE_ROM_SIZE = 0x4000;

	memory_bank_creator m_bank_rom;

	unsigned m_bank_bits_rom;
	u16 m_bank_mask_rom;
};



//**************************************************************************
//  16 KiB switchable coarse at 0x0000, 16 KiB switchable fine at 0x4000
//**************************************************************************

class mbc_dual_device_base : public device_t, public device_gb_cart_interface
{
protected:
	mbc_dual_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;

	void set_bank_bits_rom(unsigned coarse, unsigned fine)
	{
		m_bank_bits_rom[0] = coarse;
		m_bank_bits_rom[1] = fine;
	}

	bool check_rom(std::string &message) ATTR_COLD;
	void install_rom() ATTR_COLD;
	void configure_bank_rom() ATTR_COLD;

	void set_bank_rom_coarse(u16 entry);
	void set_bank_rom_fine(u16 entry);
	void set_bank_rom_low_coarse_mask(u16 mask);
	void set_bank_rom_high_coarse_mask(u16 mask);

	u16 bank_rom_coarse() const { return m_bank_sel_rom[0]; }
	u16 bank_rom_fine() const { return m_bank_sel_rom[1]; }
	u8 const *bank_rom_low_base() const { return reinterpret_cast<u8 const *>(m_bank_rom[0]->base()); }
	u8 const *bank_rom_high_base() const { return reinterpret_cast<u8 const *>(m_bank_rom[1]->base()); }

private:
	static inline constexpr unsigned PAGE_ROM_SIZE = 0x4000;

	u16 bank_rom_entry_low() const noexcept
	{
		return m_bank_sel_rom[0] & m_bank_rom_coarse_mask[0] & m_bank_mask_rom[0];
	}

	u16 bank_rom_entry_high() const noexcept
	{
		u16 const coarse(m_bank_sel_rom[0] & m_bank_rom_coarse_mask[1]);
		return (m_bank_sel_rom[1] | (coarse << m_bank_bits_rom[1])) & m_bank_mask_rom[1];
	}

	memory_bank_array_creator<2> m_bank_rom;

	unsigned m_bank_bits_rom[2];
	u16 m_bank_mask_rom[2];
	u16 m_bank_sel_rom[2];
	u16 m_bank_rom_coarse_mask[2];
};



//**************************************************************************
//  16 KiB switchable at 0x0000, 16 KiB switchable at 0x4000
//**************************************************************************

class mbc_dual_uniform_device_base : public device_t, public device_gb_cart_interface
{
protected:
	mbc_dual_uniform_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;

	void set_bank_bits_rom(unsigned bits)
	{
		m_bank_bits_rom = bits;
	}

	bool check_rom(std::string &message) ATTR_COLD;
	void install_rom() ATTR_COLD;
	void configure_bank_rom() ATTR_COLD;

	void set_bank_rom_low(u16 entry);
	void set_bank_rom_high(u16 entry);

	u8 const *bank_rom_low_base() const { return reinterpret_cast<u8 const *>(m_bank_rom[0]->base()); }
	u8 const *bank_rom_high_base() const { return reinterpret_cast<u8 const *>(m_bank_rom[1]->base()); }

private:
	static inline constexpr unsigned PAGE_ROM_SIZE = 0x4000;

	memory_bank_array_creator<2> m_bank_rom;

	unsigned m_bank_bits_rom;
	u16 m_bank_mask_rom;
};



//**************************************************************************
//  16 KiB fixed at 0x0000, 8 KiB switchable at 0x4000 and 0x6000
//**************************************************************************

class mbc_8k_device_base : public device_t, public device_gb_cart_interface
{
protected:
	mbc_8k_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;

	void set_bank_bits_rom(unsigned bits) { m_bank_bits_rom = bits; }

	bool check_rom(std::string &message) ATTR_COLD;
	void install_rom(address_space_installer &fixedspace, address_space_installer &lowspace, address_space_installer &highspace) ATTR_COLD;

	void set_bank_rom_low(u16 entry);
	void set_bank_rom_high(u16 entry);

private:
	static inline constexpr unsigned PAGE_ROM_SIZE = 0x2000;

	memory_bank_array_creator<2> m_bank_rom;

	unsigned m_bank_bits_rom;
	u16 m_bank_mask_rom;
};



//**************************************************************************
//  32 KiB switchable at 0x0000
//**************************************************************************

class banked_32k_device_base : public device_t, public device_gb_cart_interface
{
protected:
	banked_32k_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;

	void set_bank_bits_rom(unsigned bits) { m_bank_bits_rom = bits; }

	bool check_rom(std::string &message) ATTR_COLD;
	void install_rom() ATTR_COLD;
	void configure_bank_rom() ATTR_COLD;

	void set_bank_rom(u16 entry);

private:
	static inline constexpr unsigned PAGE_ROM_SIZE = 0x8000;

	memory_bank_creator m_bank_rom;

	unsigned m_bank_bits_rom;
	u16 m_bank_mask_rom;
};



//**************************************************************************
//  8 KiB flat RAM helper
//**************************************************************************

template <typename Base>
class flat_ram_device_base : public Base
{
public:
	virtual void unload() override ATTR_COLD;

protected:
	using Base::Base;

	bool check_ram(std::string &message) ATTR_COLD;
	void install_ram() ATTR_COLD;
};



//**************************************************************************
//  8 KiB switchable RAM helper
//**************************************************************************

template <typename Base>
class mbc_ram_device_base : public Base
{
public:
	virtual void unload() override ATTR_COLD;

protected:
	mbc_ram_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;

	void set_bank_bits_ram(unsigned bits)
	{
		m_bank_bits_ram = bits;
	}

	bool check_ram(std::string &message) ATTR_COLD;
	void install_ram(address_space_installer &space) ATTR_COLD;
	void install_ram(address_space_installer &rospace, address_space_installer &rwspace) ATTR_COLD;
	bool configure_bank_ram(std::string &message) ATTR_COLD;

	void set_bank_ram(u8 entry);
	void set_bank_ram_mask(u8 mask);

	u8 bank_ram() const { return m_bank_sel_ram; }
	u8 *bank_ram_base() const { return reinterpret_cast<u8 *>(m_bank_ram->base()); }

private:
	static inline constexpr unsigned PAGE_RAM_SIZE = 0x2000;

	void install_ram(address_space_installer *rospace, address_space_installer *rwspace) ATTR_COLD;

	u8 bank_ram_entry() const noexcept
	{
		return m_bank_sel_ram & m_bank_ram_mask & m_bank_mask_ram;
	}

	memory_bank_creator m_bank_ram;

	unsigned m_bank_bits_ram;
	u8 m_bank_mask_ram;
	u8 m_bank_sel_ram;
	u8 m_bank_ram_mask;
};

} // namespace bus::gameboy

#endif // MAME_BUS_GAMEBOY_CARTBASE_H
