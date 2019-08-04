// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An emulation of the BERT ASIC found in the NCD 16 X terminal.
 *
 * Sources:
 *   - none known, but there's a full simulation in the firmware to test it
 */

#include "emu.h"
#include "bert.h"

DEFINE_DEVICE_TYPE(BERT, bert_device, "ncd_bert_asic", "NCD BERT ASIC")

bert_device::bert_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, BERT, tag, owner, clock)
	, m_memory_space(*this, finder_base::DUMMY_TAG, 24, 16)
	, m_memory(nullptr)
{
}

void bert_device::device_start()
{
	save_item(NAME(m_control));
	save_item(NAME(m_history));
	save_item(NAME(m_step));
	save_item(NAME(m_qlc_mode));
	save_item(NAME(m_qlc_src));
	m_memory = m_memory_space->cache<1, 0, ENDIANNESS_BIG>();
}

void bert_device::device_reset()
{
	m_control = 0;
	m_history = 0;
	m_step = 0;
	m_qlc_mode = false;
	m_qlc_src = 0;
}

void bert_device::map(address_map &map)
{
	map(0x000000, 0x7fffff).rw(FUNC(bert_device::read), FUNC(bert_device::write));
}

void bert_device::set_qlc_mode(bool state)
{
	m_qlc_mode = state;
}

u16 bert_device::read(offs_t offset)
{
	if(m_qlc_mode) {
		m_qlc_src = offset << 1;
		return 0;
	}

	constexpr u16 type = 0x00ca;
	u16 data = m_memory->read_word(offset << 1);
	u16 res;

	if(type & (1 << (((m_control >> 8) & 0xc) | m_step))) {
		// mask
		res = m_control & 0x200 ? ~data : data;
	} else {
		// data
		u32 tmp = m_control & 0x10 ? (data << 16) | m_history : (m_history << 16) | data;
		res = tmp >> (m_control & 15);
		if(m_control & 0x100)
			res = ~res;
		m_history = m_control & 0x1000 ? 0 : data;
	}

	m_step = (m_step + 1) & 3;

	return res;
}

void bert_device::write(offs_t offset, u16 data, u16 mem_mask)
{
	if(m_qlc_mode) {
		u32 dest = offset << 1;
		for(u32 i=0; i<512; i+=2)
			m_memory->write_word(dest + i, m_memory->read_word(m_qlc_src + i));
		return;
	}

	m_step = 0;
	m_memory->write_word(offset << 1, data, mem_mask);
	if(!offset) {
		COMBINE_DATA(&m_control);
		m_step = 0;
		if(m_control & 0x1000)
			m_history = 0;
	}
}
