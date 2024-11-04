// license:GPL-2.0+
// copyright-holders:smf
#ifndef MAME_BUS_PCCARD_PCCARD_H
#define MAME_BUS_PCCARD_PCCARD_H

#pragma once

class pccard_slot_device;

class device_pccard_interface :
	public device_interface
{
public:
	virtual ~device_pccard_interface() {}

	virtual uint16_t read_memory(offs_t offset, uint16_t mem_mask = ~0);
	virtual uint16_t read_reg(offs_t offset, uint16_t mem_mask = ~0);
	virtual void write_memory(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	virtual void write_reg(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// 16-bit byte swapped trampolines
	uint16_t read_memory_swap(offs_t offset, uint16_t mem_mask = 0xffff) { return swapendian_int16(read_memory(offset, swapendian_int16(mem_mask))); }
	uint16_t read_reg_swap(offs_t offset, uint16_t mem_mask = 0xffff) { return swapendian_int16(read_reg(offset, swapendian_int16(mem_mask))); }
	void write_memory_swap(offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { write_memory(offset, swapendian_int16(data), swapendian_int16(mem_mask)); }
	void write_reg_swap(offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { write_reg(offset, swapendian_int16(data), swapendian_int16(mem_mask)); }

	// 8-bit trampolines
	uint8_t read_memory_byte(offs_t offset) { return read_memory(offset >> 1, 0xff << (BIT(offset, 0) * 8)) >> (BIT(offset, 0) * 8); }
	uint8_t read_reg_byte(offs_t offset) { return read_reg(offset >> 1, 0xff << (BIT(offset, 0) * 8)) >> (BIT(offset, 0) * 8); }
	void write_memory_byte(offs_t offset, uint8_t data) { write_memory(offset >> 1, data << (BIT(offset, 0) * 8), 0xff << (BIT(offset, 0) * 8)); }
	void write_reg_byte(offs_t offset, uint8_t data) { write_reg(offset >> 1, data << (BIT(offset, 0) * 8), 0xff << (BIT(offset, 0) * 8)); }

	auto cd1() { return m_cd1_cb.bind(); }
	auto cd2() { return m_cd2_cb.bind(); }
	auto bvd1() { return m_bvd1_cb.bind(); }
	auto bvd2() { return m_bvd2_cb.bind(); }
	auto wp() { return m_wp_cb.bind(); }

protected:
	device_pccard_interface(const machine_config &mconfig, device_t &device);

	devcb_write_line m_cd1_cb;
	devcb_write_line m_cd2_cb;
	devcb_write_line m_bvd1_cb;
	devcb_write_line m_bvd2_cb;
	devcb_write_line m_wp_cb;
};

class pccard_slot_device :
	public device_t,
	public device_single_card_slot_interface<device_pccard_interface>,
	public device_pccard_interface
{
public:
	pccard_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T>
	pccard_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: pccard_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	virtual uint16_t read_memory(offs_t offset, uint16_t mem_mask = ~0) override;
	virtual uint16_t read_reg(offs_t offset, uint16_t mem_mask = ~0) override;
	virtual void write_memory(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void write_reg(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;

	void update_cd1(int state);
	void update_cd2(int state);
	void update_bvd1(int state);
	void update_bvd2(int state);
	void update_wp(int state);

	device_pccard_interface *m_dev;
};

DECLARE_DEVICE_TYPE(PCCARD_SLOT, pccard_slot_device)

#endif // MAME_BUS_PCCARD_PCCARD_H
