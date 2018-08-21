// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 VU1 interface device skeleton
*
*   To Do:
*     Everything
*
*   Note: STAT mode bit twiddling is a total guess
*
*/

#include "emu.h"
#include "ps2vif1.h"
#include <cmath>

#include "video/ps2gif.h"

DEFINE_DEVICE_TYPE(SONYPS2_VIF1, ps2_vif1_device, "ps2vif1", "PlayStation 2 VIF1")

/*static*/ const size_t ps2_vif1_device::BUFFER_SIZE = 0x40;
/*static*/ const uint32_t ps2_vif1_device::FORMAT_SIZE[] = {
	1, 1, 1, 1,
	2, 1, 1, 1,
	3, 2, 1, 1,
	4, 2, 1, 2
};

ps2_vif1_device::ps2_vif1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SONYPS2_VIF1, tag, owner, clock)
	, device_execute_interface(mconfig, *this)
	, m_gs(*this, finder_base::DUMMY_TAG)
	, m_vu1(*this, finder_base::DUMMY_TAG)
{
}

ps2_vif1_device::~ps2_vif1_device()
{
}

void ps2_vif1_device::device_start()
{
	set_icountptr(m_icount);

	save_item(NAME(m_icount));

	save_item(NAME(m_buffer));
	save_item(NAME(m_curr));
	save_item(NAME(m_end));

	save_item(NAME(m_status));
	save_item(NAME(m_control));
	save_item(NAME(m_err));
	save_item(NAME(m_mark));
	save_item(NAME(m_cycle));
	save_item(NAME(m_mode));
	save_item(NAME(m_num));
	save_item(NAME(m_mask));
	save_item(NAME(m_code));
	save_item(NAME(m_itops));
	save_item(NAME(m_base));
	save_item(NAME(m_offset));
	save_item(NAME(m_tops));
	save_item(NAME(m_itop));
	save_item(NAME(m_top));

	save_item(NAME(m_row_fill));
	save_item(NAME(m_col_fill));

	save_item(NAME(m_data_needed));
	save_item(NAME(m_data_index));
	save_item(NAME(m_command));
	save_item(NAME(m_alignment));

	save_item(NAME(m_mpg_count));
	save_item(NAME(m_mpg_addr));
	save_item(NAME(m_mpg_insn));

	save_item(NAME(m_unpack_count));
	save_item(NAME(m_unpack_addr));
	save_item(NAME(m_unpack_last));
	save_item(NAME(m_unpack_bits_remaining));
	save_item(NAME(m_unpack_signed));
	save_item(NAME(m_unpack_add_tops));
	save_item(NAME(m_unpack_format));
}

void ps2_vif1_device::device_reset()
{
	m_icount = 0;

	memset(m_buffer, 0, sizeof(uint32_t) * BUFFER_SIZE);
	m_curr = 0;
	m_end = 0;

	m_status = 0;
	m_control = 0;
	m_err = 0;
	m_mark = 0;
	m_cycle = 0;
	m_mode = 0;
	m_num = 0;
	m_mask = 0;
	m_code = 0;
	m_itops = 0;
	m_base = 0;
	m_offset = 0;
	m_tops = 0;
	m_itop = 0;
	m_top = 0;

	memset(m_row_fill, 0, sizeof(uint32_t) * 4);
	memset(m_col_fill, 0, sizeof(uint32_t) * 4);

	m_data_needed = 0;
	m_data_index = 0;
	m_command = 0;
	m_alignment = 0;

	m_mpg_count = 0;
	m_mpg_addr = 0;
	m_mpg_insn = 0;

	m_unpack_count = 0;
	m_unpack_addr = 0;
	m_unpack_last = 0;
	m_unpack_bits_remaining = 0;
	m_unpack_signed = false;
	m_unpack_add_tops = false;
	m_unpack_format = FMT_S32;
}

READ32_MEMBER(ps2_vif1_device::regs_r)
{
	uint32_t ret = 0;
	switch (offset)
	{
		case 0x00/4:
			ret = m_status;
			logerror("%s: Read: VIF1_STAT (%08x)\n", machine().describe_context(), ret);
			break;
		case 0x10/4:
			ret = m_control;
			logerror("%s: Read: VIF1_FBRST (%08x)\n", machine().describe_context(), ret);
			break;
		case 0x20/4:
			ret = m_err;
			logerror("%s: Read: VIF1_ERR (%08x)\n", machine().describe_context(), ret);
			break;
		case 0x30/4:
			ret = m_mark;
			logerror("%s: Read: VIF1_MARK (%08x)\n", machine().describe_context(), ret);
			break;
		case 0x40/4:
			ret = m_cycle;
			logerror("%s: Read: VIF1_CYCLE (%08x)\n", machine().describe_context(), ret);
			break;
		case 0x50/4:
			ret = m_mode;
			logerror("%s: Read: VIF1_MODE (%08x)\n", machine().describe_context(), ret);
			break;
		case 0x60/4:
			ret = m_num;
			logerror("%s: Read: VIF1_NUM (%08x)\n", machine().describe_context(), ret);
			break;
		case 0x70/4:
			ret = m_mask;
			logerror("%s: Read: VIF1_MASK (%08x)\n", machine().describe_context(), ret);
			break;
		case 0x80/4:
			ret = m_code;
			logerror("%s: Read: VIF1_CODE (%08x)\n", machine().describe_context(), ret);
			break;
		case 0x90/4:
			ret = m_itops;
			logerror("%s: Read: VIF1_ITOPS (%08x)\n", machine().describe_context(), ret);
			break;
		case 0xa0/4:
			ret = m_base;
			logerror("%s: Read: VIF1_BASE (%08x)\n", machine().describe_context(), ret);
			break;
		case 0xb0/4:
			ret = m_offset;
			logerror("%s: Read: VIF1_OFST (%08x)\n", machine().describe_context(), ret);
			break;
		case 0xc0/4:
			ret = m_tops;
			logerror("%s: Read: VIF1_TOPS (%08x)\n", machine().describe_context(), ret);
			break;
		case 0xd0/4:
			ret = m_itop;
			logerror("%s: Read: VIF1_ITOP (%08x)\n", machine().describe_context(), ret);
			break;
		case 0xe0/4:
			ret = m_top;
			logerror("%s: Read: VIF1_TOP (%08x)\n", machine().describe_context(), ret);
			break;
		case 0x100/4:
			ret = m_row_fill[0];
			logerror("%s: Read: VIF1_R0 (%08x)\n", machine().describe_context(), ret);
			break;
		case 0x110/4:
			ret = m_row_fill[1];
			logerror("%s: Read: VIF1_R1 (%08x)\n", machine().describe_context(), ret);
			break;
		case 0x120/4:
			ret = m_row_fill[2];
			logerror("%s: Read: VIF1_R2 (%08x)\n", machine().describe_context(), ret);
			break;
		case 0x130/4:
			ret = m_row_fill[3];
			logerror("%s: Read: VIF1_R3 (%08x)\n", machine().describe_context(), ret);
			break;
		case 0x140/4:
			ret = m_col_fill[0];
			logerror("%s: Read: VIF1_C0 (%08x)\n", machine().describe_context(), ret);
			break;
		case 0x150/4:
			ret = m_col_fill[1];
			logerror("%s: Read: VIF1_C1 (%08x)\n", machine().describe_context(), ret);
			break;
		case 0x160/4:
			ret = m_col_fill[2];
			logerror("%s: Read: VIF1_C2 (%08x)\n", machine().describe_context(), ret);
			break;
		case 0x170/4:
			ret = m_col_fill[3];
			logerror("%s: Read: VIF1_C3 (%08x)\n", machine().describe_context(), ret);
			break;
		default:
			logerror("%s: Read: Unknown (%08x)\n", machine().describe_context(), 0x10003c00 + (offset << 2));
			break;
	}
	return ret;
}

WRITE32_MEMBER(ps2_vif1_device::regs_w)
{
	logerror("%s: Write: Unknown %08x = %08x\n", machine().describe_context(), 0x10003c00 + (offset << 2), data);
}

READ64_MEMBER(ps2_vif1_device::mmio_r)
{
	uint64_t ret = 0ULL;
	if (offset)
	{
		logerror("%s: mmio_r [127..64]: (%08x%08x)\n", machine().describe_context(), (uint32_t)(ret >> 32), (uint32_t)ret);
	}
	else
	{
		logerror("%s: mmio_r [63..0]: (%08x%08x)\n", machine().describe_context(), (uint32_t)(ret >> 32), (uint32_t)ret);
	}
	return ret;
}

WRITE64_MEMBER(ps2_vif1_device::mmio_w)
{
	if (offset)
	{
		logerror("%s: mmio_w [127..64]: %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data);
		fifo_push((uint32_t)data);
		fifo_push((uint32_t)(data >> 32));
	}
	else
	{
		logerror("%s: mmio_w [63..0]: %08x%08x\n", machine().describe_context(), (uint32_t)(data >> 32), (uint32_t)data);
		fifo_push((uint32_t)data);
		fifo_push((uint32_t)(data >> 32));
	}
}


void ps2_vif1_device::dma_write(const uint64_t hi, const uint64_t lo)
{
	//logerror("%s: dma_write: %08x%08x%08x%08x\n", machine().describe_context(), (uint32_t)(hi >> 32), (uint32_t)hi, (uint32_t)(lo >> 32), (uint32_t)lo);
	fifo_push((uint32_t)(lo >> 32));
	fifo_push((uint32_t)lo);
	fifo_push((uint32_t)(hi >> 32));
	fifo_push((uint32_t)hi);
}

void ps2_vif1_device::tag_write(uint32_t *data)
{
	logerror("%s: tag_write: %08x%08x\n", machine().describe_context(), data[2], data[3]);
	fifo_push(data[2]);
	fifo_push(data[3]);
}

void ps2_vif1_device::fifo_push(uint32_t data)
{
	if (m_end >= BUFFER_SIZE)
	{
		printf("FIFO overflow :(\n");
		return;
	}

	m_buffer[m_end++] = data;

	m_status &= ~0x1f000000;
	m_status |= ((m_end - m_curr) >> 2) << 24;
	logerror("%s: Pushing %08x onto FIFO, depth is now %d, status is now %08x\n", machine().describe_context(), data, m_end - m_curr, m_status);
}

uint32_t ps2_vif1_device::fifo_pop()
{
	m_alignment = (m_alignment + 1) & 3;

	if (m_curr >= m_end)
	{
		return 0;
	}

	const uint32_t ret = m_buffer[m_curr++];
	if (m_curr >= m_end)
	{
		m_curr = 0;
		m_end = 0;
	}

	m_status &= ~0x1f000000;
	m_status |= ((m_end - m_curr) >> 2) << 24;
	logerror("%s: Popping %08x from FIFO, depth is now %d, status is now %08x\n", machine().describe_context(), ret, m_end - m_curr, m_status);

	return ret;
}

void ps2_vif1_device::execute_run()
{
	while (m_icount > 0)
	{
		if (m_status & (STAT_E_WAIT | STAT_GS_WAIT | STAT_STALL_STOP | STAT_STALL_FBRK | STAT_STALL_INT))
		{
			m_icount = 0;
			break;
		}

		switch (m_status & STAT_MODE_MASK)
		{
			case STAT_MODE_IDLE:
				if (fifo_depth())
				{
					m_code = fifo_pop();
					logerror("%s: New VIFcode: %08x\n", machine().describe_context(), m_code);
				}
				else
				{
					//logerror("%s: Nothing in FIFO, idle\n", machine().describe_context());
					m_icount = 0;
					break;
				}
				// Intentional fall-through
			case STAT_MODE_DECODE:
				decode_vifcode();
				break;
			case STAT_MODE_WAIT:
				if (fifo_depth() < m_data_needed)
				{
					m_icount = 0;
					break;
				}
				// Intentional fall-through
			case STAT_MODE_DATA:
				transfer_vifcode_data();
				break;
		}
	}
}

void ps2_vif1_device::transfer_vifcode_data()
{
	m_status &= ~STAT_MODE_MASK;
	if (fifo_depth() < m_data_needed)
	{
		//logerror("%s: transfer: We need %d but only have %d, waiting\n", machine().describe_context(), m_data_needed, fifo_depth());
		m_status |= STAT_MODE_WAIT;
		m_icount = 0;
		return;
	}

	switch (m_command)
	{
		case 0x20: /* STMASK */
			m_mask = fifo_pop();
			logerror("%s: STMASK: %08x\n", machine().describe_context(), m_mask);
			m_data_needed = 0;
			break;
		case 0x30: /* STROW */
			m_row_fill[m_data_index] = fifo_pop();
			logerror("%s: STMASK: %08x\n", machine().describe_context(), m_row_fill[m_data_index]);
			m_data_needed--;
			break;
		case 0x31: /* STCOL */
			m_col_fill[m_data_index] = fifo_pop();
			logerror("%s: STMASK: %08x\n", machine().describe_context(), m_col_fill[m_data_index]);
			m_data_needed--;
			break;
		case 0x4a: /* MPG */
			transfer_mpg();
			break;
		default:
			if ((m_command & 0x60) == 0x60)
			{
				logerror("%s: Unpack: %02x\n", machine().describe_context(), m_command);
				transfer_unpack();
			}
			else
			{
				logerror("%s: Unknown command: %02x\n", machine().describe_context(), m_command);
			}
			break;
	}

	if (m_data_needed)
	{
		if (fifo_depth() == 0)
		{
			m_status |= STAT_MODE_WAIT;
			m_icount = 0;
		}
		else
		{
			m_status |= STAT_MODE_DATA;
			m_icount--;
		}
	}
	else if (fifo_depth() > 0)
	{
		m_code = fifo_pop();
		logerror("%s: New VIFcode: %08x\n", machine().describe_context(), m_code);
		m_status |= STAT_MODE_DECODE;
		m_icount--;
	}
	else
	{
		m_status |= STAT_MODE_IDLE;
		m_icount = 0;
	}
}

void ps2_vif1_device::transfer_unpack()
{
	switch (m_unpack_format)
	{
		case FMT_V4_32:
			//logerror("%s: Unpacking V4-32.\n", machine().describe_context());
			for (int element = 0; element < 4; element++)
			{
				const uint32_t data = fifo_pop();
				m_vu1->write_vu_mem(m_unpack_addr, data);
				m_unpack_addr += 4;
				m_unpack_count--;
			}
			break;
		default:
			logerror("%s: Unsupported unpack format: %02x\n", machine().describe_context(), m_unpack_format);
			break;
	}

	if (m_unpack_count > 0)
	{
		m_data_needed = FORMAT_SIZE[m_unpack_format] - (m_unpack_bits_remaining ? 1 : 0);
	}
	else
	{
		m_data_needed = 0;
	}
}

void ps2_vif1_device::transfer_mpg()
{
	while (m_data_needed > 2)
	{
		fifo_pop();
		m_data_needed--;
	}

	m_mpg_insn = (uint64_t)fifo_pop() << 32;
	m_mpg_insn |= fifo_pop();
	m_mpg_count--;

	m_vu1->write_micro_mem(m_mpg_addr, m_mpg_insn);
	logerror("%s: MPG, VU insn: %08x = %08x%08x, %d remaining\n", machine().describe_context(), m_mpg_addr, (uint32_t)(m_mpg_insn >> 32), (uint32_t)m_mpg_insn, m_mpg_count);

	m_mpg_addr += 8;
	m_data_needed = m_mpg_count ? 2 : 0;
}

void ps2_vif1_device::decode_vifcode()
{
	bool trigger_interrupt = BIT(m_code, 31);
	m_command = (m_code >> 24) & 0x7f;
	m_status &= ~STAT_MODE_MASK;

	switch (m_command)
	{
		case 0x00: /* NOP */
			logerror("%s: NOP\n", machine().describe_context());
			break;
		case 0x01: /* STCYCL */
			m_cycle = (uint16_t)m_code;
			logerror("%s: STCYCL: %04x\n", machine().describe_context(), (uint16_t)m_cycle);
			break;
		case 0x02: /* OFFSET */
			m_offset = m_code & 0x3ff;
			logerror("%s: OFFSET: %03x\n", machine().describe_context(), m_offset);
			break;
		case 0x03: /* BASE */
			m_base = m_code & 0x3ff;
			logerror("%s: BASE: %03x\n", machine().describe_context(), m_base);
			break;
		case 0x04: /* ITOP */
			m_itops = m_code & 0x3ff;
			logerror("%s: ITOP: %03x\n", machine().describe_context(), m_itops);
			break;
		case 0x05: /* STMOD */
			m_mode = m_code & 3;
			logerror("%s: MODE: %03x\n", machine().describe_context(), m_mode);
			break;
		case 0x06: /* MSKPATH3 */
			m_gs->interface()->set_path3_mask(BIT(m_code, 15));
			logerror("%s: MSKPATH3: %d\n", machine().describe_context(), BIT(m_code, 15));
			break;
		case 0x07: /* Oh hi, MARK */
			m_mark = (uint16_t)m_code;
			logerror("%s: MARK: %04x\n", machine().describe_context(), (uint16_t)m_mark);
			break;
		case 0x14: /* MSCAL */
			logerror("%s: MSCAL %04x\n", machine().describe_context(), (uint16_t)m_code);
			if (m_vu1->running())
			{
				m_icount--;
				return;
			}
			else
			{
				m_vu1->start((uint16_t)m_code);
			}
			break;
		case 0x20: /* STMASK */
			m_data_needed = 1;
			m_data_index = 0;
			logerror("%s: STMASK\n", machine().describe_context());
			break;
		case 0x30: /* STROW */
			m_data_needed = 4;
			m_data_index = 0;
			logerror("%s: STROW\n", machine().describe_context());
			break;
		case 0x31: /* STCOL */
			m_data_needed = 4;
			m_data_index = 0;
			logerror("%s: STCOL\n", machine().describe_context());
			break;
		case 0x4a: /* MPG */
			m_data_needed = 2 + (m_alignment & 1);
			m_data_index = 0;
			m_mpg_count = (m_code >> 16) & 0xff;
			if (!m_mpg_count)
				m_mpg_count = 0x100;
			m_mpg_addr = m_code & 0xffff;
			logerror("%s: MPG\n", machine().describe_context());
			break;
		default:
			if ((m_command & 0x60) == 0x60)
			{ /* UNPACK */
				m_unpack_count = calculate_unpack_count();
				m_unpack_signed = BIT(m_code, 14);
				m_unpack_add_tops = BIT(m_code, 15);
				m_unpack_format = (uint8_t)(m_command & 0xf);
				m_data_needed = FORMAT_SIZE[m_unpack_format];
				logerror("%s: UNPACK (%08x), count %d\n", machine().describe_context(), m_code, m_unpack_count);
			}
			else
			{ /* unknown */
				logerror("%s: decode_vifcode: Unknown command %02x\n", machine().describe_context(), m_command);
			}
			break;
	}

	if (m_data_needed > 0)
	{
		if (fifo_depth())
		{
			m_status |= STAT_MODE_DATA;
			m_icount--;
		}
		else
		{
			m_status |= STAT_MODE_WAIT;
			m_icount = 0;
		}
	}
	else if (fifo_depth())
	{
		m_code = fifo_pop();
		m_status |= STAT_MODE_DECODE;
		m_icount--;
	}
	else
	{
		m_status |= STAT_MODE_IDLE;
		m_icount = 0;
	}

	if (trigger_interrupt)
	{
		//printf("******************\n");
	}
}

uint32_t ps2_vif1_device::calculate_unpack_count()
{
	const uint32_t wl = (m_cycle >> 8) & 0xff;
	const uint32_t cl = m_cycle & 0xff;
	const uint32_t vl = m_command & 3;
	const uint32_t vn = (m_command >> 2) & 3;

	uint32_t num = (m_code >> 16) & 0xff;
	if (wl > cl)
	{
		const uint32_t mod = num % wl;
		num = cl * (num / wl) + ((mod > cl) ? cl : mod);
	}

	return (uint32_t)std::ceil(((32 >> vl) * (vn + 1) * num) / 32.0f);
}
