// license:BSD-3-Clause
// copyright-holders:smf
#pragma once

#ifndef __BANKDEV_H__
#define __BANKDEV_H__

#include "emu.h"

#define MCFG_ADDRESS_MAP_BANK_ENDIANNESS(_endianness) \
	address_map_bank_device::set_endianness(*device, _endianness);

#define MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(_databus_width) \
	address_map_bank_device::set_databus_width(*device, _databus_width);

#define MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(_addrbus_width) \
	address_map_bank_device::set_addrbus_width(*device, _addrbus_width);

#define MCFG_ADDRESS_MAP_BANK_STRIDE(_stride) \
	address_map_bank_device::set_stride(*device, _stride);

class address_map_bank_device :
	public device_t,
	public device_memory_interface
{
public:
	// construction/destruction
	address_map_bank_device( const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock );

	// static configuration helpers
	static void set_endianness(device_t &device, endianness_t endianness) { downcast<address_map_bank_device &>(device).m_endianness = endianness; }
	static void set_databus_width(device_t &device, UINT8 databus_width) { downcast<address_map_bank_device &>(device).m_databus_width = databus_width; }
	static void set_addrbus_width(device_t &device, UINT8 addrbus_width) { downcast<address_map_bank_device &>(device).m_addrbus_width = addrbus_width; }
	static void set_stride(device_t &device, UINT32 stride) { downcast<address_map_bank_device &>(device).m_stride = stride; }

	DECLARE_ADDRESS_MAP(amap8, 8);
	DECLARE_ADDRESS_MAP(amap16, 16);
	DECLARE_ADDRESS_MAP(amap32, 32);
	DECLARE_ADDRESS_MAP(amap64, 64);

	DECLARE_WRITE8_MEMBER(write8);
	DECLARE_WRITE16_MEMBER(write16);
	DECLARE_WRITE32_MEMBER(write32);
	DECLARE_WRITE64_MEMBER(write64);

	DECLARE_READ8_MEMBER(read8);
	DECLARE_READ16_MEMBER(read16);
	DECLARE_READ32_MEMBER(read32);
	DECLARE_READ64_MEMBER(read64);

	void set_bank(offs_t offset);

protected:
	virtual void device_start();
	virtual void device_config_complete();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

private:
	// internal state
	endianness_t m_endianness;
	UINT8 m_databus_width;
	UINT8 m_addrbus_width;
	UINT32 m_stride;
	address_space_config m_program_config;
	address_space *m_program;
	offs_t m_offset;
};


// device type definition
extern const device_type ADDRESS_MAP_BANK;

#endif
