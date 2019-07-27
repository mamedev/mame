// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_BERT_H
#define MAME_MACHINE_BERT_H

#pragma once

class bert_device : public device_t
{
public:
	// configuration
	template <typename T> void set_memory(T &&tag, int spacenum) { m_memory_space.set_tag(std::forward<T>(tag), spacenum); }

	bert_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }
	void map(address_map &map);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	u16 read(offs_t offset);
	void write(offs_t offset, u16 data);

private:
	required_address_space m_memory_space;
	memory_access_cache<1, 0, ENDIANNESS_BIG> *m_memory;

	u16 m_control;
	u16 m_shifter;
};

DECLARE_DEVICE_TYPE(BERT, bert_device)

#endif // MAME_MACHINE_BERT_H
