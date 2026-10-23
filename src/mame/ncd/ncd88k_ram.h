// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#ifndef MAME_NCD_NCD88K_RAM_H
#define MAME_NCD_NCD88K_RAM_H

#pragma once

class ncd88k_ram_device
	: public device_t
{
public:
	static constexpr offs_t CODE_BASE = 0x0400'0000;
	static constexpr offs_t DATA_BASE = 0x0800'0000;

	ncd88k_ram_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	template <typename T> void set_code_space(T &&tag, int spacenum) { m_code.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_data_space(T &&tag, int spacenum) { m_data.set_tag(std::forward<T>(tag), spacenum); }
	template <unsigned Slot> auto slot() { return m_slot[Slot].bind(); }

	void ctrl_w(offs_t offset, u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	struct bank
	{
		std::unique_ptr<u32[]> ptr;
		u32 size;

		void alloc(u32 size);
	};

	void map();
	template <bool Code> void map(bank &ram, offs_t base, u32 window_size);

	required_address_space m_code;
	required_address_space m_data;

	devcb_read32::array<5> m_slot;

	u32 m_ctrl[8];
	bank m_bank[10];
};

DECLARE_DEVICE_TYPE(NCD88K_RAM, ncd88k_ram_device)

#endif // MAME_NCD_NCD88K_RAM_H
