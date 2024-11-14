// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_NCD_BERT_M_H
#define MAME_NCD_BERT_M_H

#pragma once

class bert_device : public device_t
{
public:
	// configuration
	template <typename T> void set_memory(T &&tag, int spacenum) { m_memory_space.set_tag(std::forward<T>(tag), spacenum); }

	bert_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void map(address_map &map) ATTR_COLD;
	void set_qlc_mode(bool state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	u16 read(offs_t offset);
	void write(offs_t offset, u16 data, u16 mem_mask);

private:
	required_address_space m_memory_space;
	memory_access<24, 1, 0, ENDIANNESS_BIG>::cache m_memory;

	u16 m_control;
	u16 m_history;
	u32 m_step;
	bool m_qlc_mode;
	u32 m_qlc_src;
};

DECLARE_DEVICE_TYPE(BERT, bert_device)

#endif // MAME_NCD_BERT_M_H
