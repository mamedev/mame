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
	device_rom_interface(const machine_config &mconfig, device_t &device, UINT8 addrwidth, endianness_t endian = ENDIANNESS_LITTLE, UINT8 datawidth = 8);
	virtual ~device_rom_interface();

	inline UINT8 read_byte(offs_t byteaddress) { return m_rom_direct->read_byte(byteaddress); }
	inline UINT16 read_word(offs_t byteaddress) { return m_rom_direct->read_word(byteaddress); }
	inline UINT32 read_dword(offs_t byteaddress) { return m_rom_direct->read_dword(byteaddress); }
	inline UINT64 read_qword(offs_t byteaddress) { return m_rom_direct->read_qword(byteaddress); }

	void set_rom(const void *base, UINT32 size);

private:
	const address_space_config m_rom_config;
	direct_read_data *m_rom_direct;

	virtual const address_space_config *memory_space_config(address_spacenum spacenum) const override;
	virtual void interface_pre_start() override;

	DECLARE_READ8_MEMBER(z8_r);
	DECLARE_READ16_MEMBER(z16_r);
	DECLARE_READ32_MEMBER(z32_r);
	DECLARE_READ64_MEMBER(z64_r);
};

#endif
