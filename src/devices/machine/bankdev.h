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
	address_map_bank_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock );

	// static configuration helpers
	static void set_endianness(device_t &device, endianness_t endianness) { downcast<address_map_bank_device &>(device).m_endianness = endianness; }
	static void set_databus_width(device_t &device, uint8_t databus_width) { downcast<address_map_bank_device &>(device).m_databus_width = databus_width; }
	static void set_addrbus_width(device_t &device, uint8_t addrbus_width) { downcast<address_map_bank_device &>(device).m_addrbus_width = addrbus_width; }
	static void set_stride(device_t &device, uint32_t stride) { downcast<address_map_bank_device &>(device).m_stride = stride; }

	DECLARE_ADDRESS_MAP(amap8, 8);
	DECLARE_ADDRESS_MAP(amap16, 16);
	DECLARE_ADDRESS_MAP(amap32, 32);
	DECLARE_ADDRESS_MAP(amap64, 64);

	void write8(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void write16(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void write32(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void write64(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));

	uint8_t read8(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint16_t read16(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint32_t read32(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint64_t read64(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));

	void set_bank(offs_t offset);

protected:
	virtual void device_start() override;
	virtual void device_config_complete() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

private:
	// internal state
	endianness_t m_endianness;
	uint8_t m_databus_width;
	uint8_t m_addrbus_width;
	uint32_t m_stride;
	address_space_config m_program_config;
	address_space *m_program;
	offs_t m_offset;
};


// device type definition
extern const device_type ADDRESS_MAP_BANK;

#endif
