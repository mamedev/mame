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

	enum
	{
		CISTPL_NULL = 0x00,
		CISTPL_DEVICE = 0x01,
		CISTPL_LONGLINK_CB = 0x02,
		CISTPL_CONFIG_CB = 0x04,
		CISTPL_CFTABLE_ENTRY_CB = 0x05,
		CISTPL_LONGLINK_MFC = 0x06,
		CISTPL_BAR = 0x07,
		CISTPL_CHECKSUM = 0x10,
		CISTPL_LONGLINK_A = 0x11,
		CISTPL_LONGLINK_C = 0x12,
		CISTPL_LINKTARGET = 0x13,
		CISTPL_NO_LINK = 0x14,
		CISTPL_VERS_1 = 0x15,
		CISTPL_ALTSTR = 0x16,
		CISTPL_DEVICE_A = 0x17,
		CISTPL_JEDEC_C = 0x18,
		CISTPL_JEDEC_A = 0x19,
		CISTPL_CONFIG = 0x1a,
		CISTPL_CFTABLE_ENTRY = 0x1b,
		CISTPL_DEVICE_OC = 0x1c,
		CISTPL_DEVICE_OA = 0x1d,
		CISTPL_DEVICE_GEO = 0x1e,
		CISTPL_DEVICE_GEO_A = 0x1f,
		CISTPL_MANFID = 0x20,
		CISTPL_FUNCID = 0x21,
		CISTPL_FUNCE = 0x22,
		CISTPL_SWIL = 0x23,
		CISTPL_END = 0xff,
	};

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
		set_options(std::forward<T>(opts), dflt, false);
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
