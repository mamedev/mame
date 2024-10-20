// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_DEVICES_MACHINE_BANKDEV_H
#define MAME_DEVICES_MACHINE_BANKDEV_H

#pragma once


class address_map_bank_device :
	public device_t,
	public device_memory_interface
{
public:
	// construction/destruction
	address_map_bank_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configuration helpers
	template <typename... T> address_map_bank_device& set_map(T &&... args) { set_addrmap(0, std::forward<T>(args)...); return *this; }
	address_map_bank_device& set_endianness(endianness_t endianness) { m_endianness = endianness; return *this; }
	address_map_bank_device& set_data_width(u8 data_width) { m_data_width = data_width; return *this; }
	address_map_bank_device& set_addr_width(u8 addr_width) { m_addr_width = addr_width; return *this; }
	address_map_bank_device& set_stride(u32 stride) { m_stride = stride; return *this; }
	address_map_bank_device& set_shift(u32 shift) { m_shift = shift; return *this; }
	address_map_bank_device& set_options(endianness_t endianness, u8 data_width, u8 addr_width, u32 stride = 1)
	{
		set_endianness(endianness);
		set_data_width(data_width);
		set_addr_width(addr_width);
		set_stride(stride);
		return *this;
	}

	template <typename... T> address_map_bank_device& map(T &&... args) { set_addrmap(0, std::forward<T>(args)...); return *this; }
	address_map_bank_device& endianness(endianness_t endianness) { m_endianness = endianness; return *this; }
	address_map_bank_device& data_width(u8 data_width) { m_data_width = data_width; return *this; }
	address_map_bank_device& addr_width(u8 addr_width) { m_addr_width = addr_width; return *this; }
	address_map_bank_device& stride(u32 stride) { m_stride = stride; return *this; }
	address_map_bank_device& shift(u32 shift) { m_shift = shift; return *this; }

	void amap8(address_map &map) ATTR_COLD;
	void amap16(address_map &map) ATTR_COLD;
	void amap32(address_map &map) ATTR_COLD;
	void amap64(address_map &map) ATTR_COLD;

	void write8(offs_t offset, u8 data);
	void write16(offs_t offset, u16 data, u16 mem_mask = 0xffff);
	void write32(offs_t offset, u32 data, u32 mem_mask = 0xffffffff);
	void write64(offs_t offset, u64 data, u64 mem_mask = ~u64(0));

	u8 read8(offs_t offset);
	u16 read16(offs_t offset, u16 mem_mask = 0xffff);
	u32 read32(offs_t offset, u32 mem_mask = 0xffffffff);
	u64 read64(offs_t offset, u64 mem_mask = ~u64(0));

	void set_bank(offs_t offset);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_config_complete() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	// internal state
	endianness_t m_endianness;
	u8 m_data_width;
	u8 m_addr_width;
	u32 m_stride;
	address_space_config m_program_config;
	address_space *m_program;
	offs_t m_offset;
	int m_shift;
};


// device type definition
DECLARE_DEVICE_TYPE(ADDRESS_MAP_BANK, address_map_bank_device)

#endif // MAME_DEVICES_MACHINE_BANKDEV_H
