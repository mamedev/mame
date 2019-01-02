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
	address_map_bank_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration helpers
	template <typename... T> address_map_bank_device& set_map(T &&... args) { set_addrmap(0, std::forward<T>(args)...); return *this; }
	address_map_bank_device& set_endianness(endianness_t endianness) { m_endianness = endianness; return *this; }
	address_map_bank_device& set_data_width(uint8_t data_width) { m_data_width = data_width; return *this; }
	address_map_bank_device& set_addr_width(uint8_t addr_width) { m_addr_width = addr_width; return *this; }
	address_map_bank_device& set_stride(uint32_t stride) { m_stride = stride; return *this; }
	address_map_bank_device& set_shift(uint32_t shift) { m_shift = shift; return *this; }
	address_map_bank_device& set_options(endianness_t endianness, uint8_t data_width, uint8_t addr_width, uint32_t stride = 1)
	{
		set_endianness(endianness);
		set_data_width(data_width);
		set_addr_width(addr_width);
		set_stride(stride);
		return *this;
	}

	template <typename... T> address_map_bank_device& map(T &&... args) { set_addrmap(0, std::forward<T>(args)...); return *this; }
	address_map_bank_device& endianness(endianness_t endianness) { m_endianness = endianness; return *this; }
	address_map_bank_device& data_width(uint8_t data_width) { m_data_width = data_width; return *this; }
	address_map_bank_device& addr_width(uint8_t addr_width) { m_addr_width = addr_width; return *this; }
	address_map_bank_device& stride(uint32_t stride) { m_stride = stride; return *this; }
	address_map_bank_device& shift(uint32_t shift) { m_shift = shift; return *this; }

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
	int m_shift;
};


// device type definition
DECLARE_DEVICE_TYPE(ADDRESS_MAP_BANK, address_map_bank_device)

#endif // MAME_DEVICES_MACHINE_BANKDEV_H
