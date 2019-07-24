// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An emulation of the BERT ASIC found in the NCD 16 X terminal.
 *
 * Sources:
 *   - none known
 *
 * TODO:
 *   - shift amounts greater than 16
 *   - other control bits
 */

#include "emu.h"
#include "bert.h"

#define LOG_GENERAL (1U << 0)

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

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
	save_item(NAME(m_shifter));

	m_memory = m_memory_space->cache<1, 0, ENDIANNESS_BIG>();
}

void bert_device::device_reset()
{
	m_control = 0;
	m_shifter = 0;
}

void bert_device::map(address_map &map)
{
	map(0x000000, 0x7fffff).rw(FUNC(bert_device::read), FUNC(bert_device::write));
	map(0x000000, 0x000001).lw16("control", [this](u16 data) { LOG("control 0x%04x\n", data); m_control = data; });
}

u16 bert_device::read(offs_t offset)
{
	u16 const data = m_memory->read_word(offset << 1);
	unsigned const shift = m_control & 0x1f;
	u16 const mask_r = (1 << shift) - 1;

	u16 result = (m_shifter << (16 - shift)) | (data >> shift);
	m_shifter = data & mask_r;

	if (BIT(m_control, 8))
		result = ~result;

	LOG("r 0x%06x 0x%04x 0x%04x 0x%04x (%s)\n", offset << 1, data, result, m_shifter, machine().describe_context());

	return result;
}

void bert_device::write(offs_t offset, u16 data)
{
	unsigned const shift = m_control & 0x1f;
	u16 const mask_r = (1 << shift) - 1;

	u16 result = (data << shift) | (m_shifter & mask_r);
	m_shifter = data >> (16 - shift);

	if (BIT(m_control, 8))
		result = ~result;

	LOG("w 0x%06x 0x%04x 0x%04x 0x%04x (%s)\n", offset << 1, data, result, m_shifter, machine().describe_context());

	m_memory->write_word(offset << 1, result);
}
