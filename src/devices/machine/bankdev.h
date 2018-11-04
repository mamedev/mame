// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_DEVICES_MACHINE_BANKDEV_H
#define MAME_DEVICES_MACHINE_BANKDEV_H

#pragma once


#define MCFG_ADDRESS_MAP_BANK_ENDIANNESS(_endianness) \
	address_map_bank_device::set_endianness(*device, _endianness);

#define MCFG_ADDRESS_MAP_BANK_DATA_WIDTH(_data_width) \
	address_map_bank_device::set_data_width(*device, _data_width);

#define MCFG_ADDRESS_MAP_BANK_ADDR_WIDTH(_addr_width) \
	address_map_bank_device::set_addr_width(*device, _addr_width);

#define MCFG_ADDRESS_MAP_BANK_STRIDE(_stride) \
	address_map_bank_device::set_stride(*device, _stride);

class address_map_bank_device :
	public device_t,
	public device_memory_interface
{
public:
	// construction/destruction
	address_map_bank_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	static void set_endianness(device_t &device, endianness_t endianness) { downcast<address_map_bank_device &>(device).m_endianness = endianness; }
	static void set_data_width(device_t &device, uint8_t data_width) { downcast<address_map_bank_device &>(device).m_data_width = data_width; }
	static void set_addr_width(device_t &device, uint8_t addr_width) { downcast<address_map_bank_device &>(device).m_addr_width = addr_width; }
	static void set_stride(device_t &device, uint32_t stride) { downcast<address_map_bank_device &>(device).m_stride = stride; }

	void amap8(address_map &map);
	void amap16(address_map &map);
	void amap32(address_map &map);
	void amap64(address_map &map);

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
	virtual void device_start() override;
	virtual void device_config_complete() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	// internal state
	endianness_t m_endianness;
	uint8_t m_data_width;
	uint8_t m_addr_width;
	uint32_t m_stride;
	address_space_config m_program_config;
	address_space *m_program;
	offs_t m_offset;
};


// device type definition
DECLARE_DEVICE_TYPE(ADDRESS_MAP_BANK, address_map_bank_device)

#endif // MAME_DEVICES_MACHINE_BANKDEV_H
