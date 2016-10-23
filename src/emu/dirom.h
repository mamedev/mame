// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    dirom.h

    Interface to a rom, either through a memory map or a region

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DIROM_H__
#define __DIROM_H__

class device_rom_interface : public device_memory_interface
{
public:
	device_rom_interface(const machine_config &mconfig, device_t &device, uint8_t addrwidth, endianness_t endian = ENDIANNESS_LITTLE, uint8_t datawidth = 8);
	virtual ~device_rom_interface();

	inline uint8_t read_byte(offs_t byteaddress) { return m_rom_direct->read_byte(byteaddress); }
	inline uint16_t read_word(offs_t byteaddress) { return m_rom_direct->read_word(byteaddress); }
	inline uint32_t read_dword(offs_t byteaddress) { return m_rom_direct->read_dword(byteaddress); }
	inline uint64_t read_qword(offs_t byteaddress) { return m_rom_direct->read_qword(byteaddress); }

	void set_rom(const void *base, uint32_t size);
	void set_rom_bank(int bank);

protected:
	virtual void rom_bank_updated() = 0;

private:
	const address_space_config m_rom_config;
	direct_read_data *m_rom_direct;

	memory_bank *m_bank;
	int m_cur_bank, m_bank_count;

	virtual const address_space_config *memory_space_config(address_spacenum spacenum) const override;
	virtual void interface_pre_start() override;

	uint8_t z8_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint16_t z16_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint32_t z32_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint64_t z64_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));

	void reset_bank();
};

#endif
