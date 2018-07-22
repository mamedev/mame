// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony Playstation 2 GS interface (GIF) device skeleton
*
*   To Do:
*     Everything
*
*/

#include "emu.h"
#include "ps2gif.h"

DEFINE_DEVICE_TYPE(SONYPS2_GIF, ps2_gif_device, "ps2gif", "Playstation 2 GIF")

ps2_gif_device::ps2_gif_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SONYPS2_GIF, tag, owner, clock)
	, device_execute_interface(mconfig, *this)
	, m_gs(*this, finder_base::DUMMY_TAG)
	, m_vu1(*this, finder_base::DUMMY_TAG)
{
}

ps2_gif_device::~ps2_gif_device()
{
}

ps2_gif_device::tag_t::tag_t(const uint64_t hi, const uint64_t lo)
	: m_nloop(lo & 0x7fffULL)
	, m_end((lo >> 15) & 1)
	, m_prim_enable((lo >> 46) & 1)
	, m_prim_output(false)
	, m_prim((lo >> 47) & 0x7ff)
	, m_format((format_t)((lo >> 58) & 3))
	, m_reg_count((uint8_t)(lo >> 60))
	, m_curr_reg(0)
{
	if (!m_reg_count)
		m_reg_count = 16;

	for (uint32_t shift = 0, index = 0; shift < 64; shift += 4, index++)
	{
		m_regs[index] = (uint8_t)(hi >> shift) & 0xf;
	}

	m_words[0] = (uint32_t)(hi >> 32);
	m_words[1] = (uint32_t)hi;
	m_words[2] = (uint32_t)(lo >> 32);
	m_words[3] = (uint32_t)lo;
}

void ps2_gif_device::device_start()
{
	set_icountptr(m_icount);

	save_item(NAME(m_icount));

	save_item(NAME(m_ctrl));
	save_item(NAME(m_mode));
	save_item(NAME(m_stat));
	save_item(NAME(m_cnt));
	save_item(NAME(m_p3cnt));
	save_item(NAME(m_p3tag));
	save_item(NAME(m_p3mask));
	save_item(NAME(m_next_path3_hi));
	save_item(NAME(m_next_path3_lo));
	save_item(NAME(m_path3_available));
	save_item(NAME(m_vu_mem_address));

	// TODO: Save current tag
}

void ps2_gif_device::device_reset()
{
	gif_reset();
}

void ps2_gif_device::gif_reset()
{
	m_ctrl = CTRL_RST;
	m_mode = 0;
	m_stat = 0;
	m_cnt = 0;
	m_p3cnt = 0;
	m_p3tag = 0;
	m_p3mask = false;
	m_current_path1_tag = tag_t();
	m_current_path3_tag = tag_t();
	m_last_tag = tag_t();
	m_path3_available = true;
	m_next_path3_hi = 0;
	m_next_path3_lo = 0;
	m_vu_mem_address = 0;
}

READ32_MEMBER(ps2_gif_device::read)
{
	uint32_t ret = 0;
	switch (offset)
	{
		case 0x00/4: // GIF_CTRL
			if (BIT(m_ctrl, 3))
			{
				ret = m_ctrl;
				logerror("%s: Read: GIF_CTRL (%08x)\n", machine().describe_context(), ret);
			}
			else
			{
				logerror("%s: Read: GIF_CTRL (00000000) (read-only; actual value %08x)\n", machine().describe_context(), m_ctrl);
			}
			break;

		case 0x10/4: // GIF_MODE
			ret = m_mode;
			logerror("%s: Read: GIF_MODE (00000000) (read-only; actual value %08x)\n", machine().describe_context(), ret);
			break;

		case 0x20/4: // GIF_STAT
			ret = m_stat;
			logerror("%s: Read: GIF_STAT (%08x)\n", machine().describe_context(), ret);
			break;

		case 0x40/4: // GIF_TAG0
			ret = m_last_tag.word(0);
			logerror("%s: Read: GIF_TAG0 (%08x)\n", machine().describe_context(), ret);
			break;

		case 0x50/4: // GIF_TAG1
			ret = m_last_tag.word(1);
			logerror("%s: Read: GIF_TAG1 (%08x)\n", machine().describe_context(), ret);
			break;

		case 0x60/4: // GIF_TAG2
			ret = m_last_tag.word(2);
			logerror("%s: Read: GIF_TAG2 (%08x)\n", machine().describe_context(), ret);
			break;

		case 0x70/4: // GIF_TAG3
			ret = m_last_tag.word(0);
			logerror("%s: Read: GIF_TAG3 (%08x)\n", machine().describe_context(), ret);
			break;

		case 0x80/4: // GIF_CNT
			ret = m_cnt;
			logerror("%s: Read: GIF_CNT (%08x)\n", machine().describe_context(), ret);
			break;

		case 0x90/4: // GIF_P3CNT
			ret = m_p3cnt;
			logerror("%s: Read: GIF_P3CNT (%08x)\n", machine().describe_context(), ret);
			break;

		case 0xa0/4: // GIF_P3TAG
			ret = m_p3tag;
			logerror("%s: Read: GIF_P3TAG (%08x)\n", machine().describe_context(), ret);
			break;

		default:
			logerror("%s: Read: Unknown %08x\n", machine().describe_context(), 0x10003000 + (offset << 2));
			break;
	}
	return ret;
}

WRITE32_MEMBER(ps2_gif_device::write)
{
	switch (offset)
	{
		case 0x00/4: // GIF_CTRL
			m_ctrl = data;
			if (BIT(data, 0))
			{
				gif_reset();
			}
			logerror("%s: Write: GIF_CTRL = %08x: STOP=%d, RESET=%d\n", machine().describe_context(), data, BIT(data, 3), BIT(data, 0));
			break;

		case 0x10/4: // GIF_MODE
			m_mode = data;
			logerror("%s: Write: GIF_MODE = %08x: IMT=%s, MASK=%s\n", machine().describe_context(), data, BIT(data, 2) ? "Intermittent" : "Continuous", BIT(data, 0) ? "Yes" : "No");
			break;

		case 0x20/4: // GIF_STAT
			logerror("%s: Write: GIF_STAT = %08x (ignored)\n", machine().describe_context(), data);
			break;

		case 0x40/4: // GIF_TAG0
			logerror("%s: Write: GIF_TAG0 = %08x (ignored)\n", machine().describe_context(), data);
			break;

		case 0x50/4: // GIF_TAG1
			logerror("%s: Write: GIF_TAG1 = %08x (ignored)\n", machine().describe_context(), data);
			break;

		case 0x60/4: // GIF_TAG2
			logerror("%s: Write: GIF_TAG2 = %08x (ignored)\n", machine().describe_context(), data);
			break;

		case 0x70/4: // GIF_TAG3
			logerror("%s: Write: GIF_TAG3 = %08x (ignored)\n", machine().describe_context(), data);
			break;

		case 0x80/4: // GIF_CNT
			logerror("%s: Write: GIF_CNT = %08x (ignored)\n", machine().describe_context(), data);
			break;

		case 0x90/4: // GIF_P3CNT
			logerror("%s: Write: GIF_P3CNT = %08x (ignored)\n", machine().describe_context(), data);
			break;

		case 0xa0/4: // GIF_P3TAG
			logerror("%s: Write: GIF_P3TAG = %08x (ignored)\n", machine().describe_context(), data);
			break;

		default:
			logerror("%s: Write: Unknown %08x = %08x\n", machine().describe_context(), 0x10003000 + (offset << 2), data);
			break;
	}
}

void ps2_gif_device::kick_path1(uint32_t address)
{
	m_vu_mem_address = address <<= 2;

	uint64_t hi, lo;
	fetch_path1(hi, lo);
	write_path1(hi, lo);
}

void ps2_gif_device::fetch_path1(uint64_t &hi, uint64_t &lo)
{
	uint32_t *tag = &m_vu1->vu_mem()[m_vu_mem_address];
	lo = ((uint64_t)tag[0] << 32) | (uint64_t)tag[1];
	hi = ((uint64_t)tag[2] << 32) | (uint64_t)tag[3];
	m_vu_mem_address += 4;
	m_vu_mem_address &= m_vu1->mem_mask() >> 2;
}

void ps2_gif_device::write_path1(uint64_t hi, uint64_t lo)
{
	if (!m_current_path1_tag.valid())
	{
		m_current_path1_tag = tag_t(hi, lo);
		printf("Starting PATH1: %08x%08x%08x%08x\n", (uint32_t)(hi >> 32), (uint32_t)hi, (uint32_t)(lo >> 32), (uint32_t)lo);
		return;
	}

	printf("Processing PATH1: %08x%08x%08x%08x\n", (uint32_t)(hi >> 32), (uint32_t)hi, (uint32_t)(lo >> 32), (uint32_t)lo);
	process_tag(m_current_path1_tag, hi, lo);
}

void ps2_gif_device::write_path3(uint64_t hi, uint64_t lo)
{
	if (m_p3mask)
	{
		return;
	}

	if (!m_current_path3_tag.valid())
	{
		m_current_path3_tag = tag_t(hi, lo);
		return;
	}

	m_next_path3_hi = hi;
	m_next_path3_lo = lo;
	m_path3_available = false;
}

void ps2_gif_device::process_tag(tag_t &tag, uint64_t hi, uint64_t lo)
{
	m_last_tag = tag;
	switch (tag.format())
	{
		case tag_t::format_t::FMT_PACKED:
			if (tag.is_prim() && !tag.is_prim_output())
			{
				tag.set_prim_output();
				logerror("%s: GIF: PACKED: Not yet implemented: PRIM\n", machine().describe_context());
			}
			else if (tag.nloop())
			{
				const uint8_t reg = tag.reg();
				m_gs->write_packed(reg, hi, lo);
				tag.next_reg();
			}
			break;
		case tag_t::format_t::FMT_REGLIST:
			logerror("%s: GIF: Not yet implemented: REGLIST (%08x%08x%08x%08x)\n", machine().describe_context(), (uint32_t)(hi >> 32), (uint32_t)hi, (uint32_t)(lo >> 32), (uint32_t)lo);
			tag.loop();
			break;
		default:
			m_gs->regs_w(machine().dummy_space(), 0x54, lo);
			m_gs->regs_w(machine().dummy_space(), 0x54, hi);
			tag.loop();
			break;
	}
}

void ps2_gif_device::tag_t::next_reg()
{
	m_curr_reg++;
	if (m_curr_reg == m_reg_count)
	{
		m_curr_reg = 0;
		loop();
	}
}

void ps2_gif_device::set_path3_mask(bool masked)
{
	m_p3mask = masked;
}

void ps2_gif_device::execute_run()
{
	while (m_icount > 0)
	{
		if (m_current_path1_tag.valid())
		{
			uint64_t hi, lo;
			fetch_path1(hi, lo);
			write_path1(hi, lo);
		}
		else if (!m_path3_available)
		{
			write_path3(m_next_path3_hi, m_next_path3_lo);
			m_path3_available = false;
		}
		else
		{
			m_icount = 0;
			break;
		}
		m_icount--;
	}
}
