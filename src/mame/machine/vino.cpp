// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************

    vino.cpp

    Silicon Graphics VINO (Video-In, No Out) controller emulation

*********************************************************************/

#include "emu.h"
#include "vino.h"

#define LOG_UNKNOWN     (1 << 0)
#define LOG_READS       (1 << 1)
#define LOG_WRITES      (1 << 2)
#define LOG_DEFAULT     (LOG_READS | LOG_WRITES | LOG_UNKNOWN)

#define VERBOSE         (LOG_DEFAULT)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(VINO, vino_device, "vino", "SGI VINO Controller")

vino_device::vino_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VINO, tag, owner, clock)
	, m_i2c_data_out(*this)
	, m_i2c_data_in(*this)
	, m_i2c_stop(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vino_device::device_start()
{
	save_item(NAME(m_rev_id));
	save_item(NAME(m_control));
	save_item(NAME(m_int_status));
	save_item(NAME(m_i2c_ctrl));
	save_item(NAME(m_i2c_data));

	for (uint32_t i = CHAN_A; i < CHAN_COUNT; i++)
	{
		save_item(NAME(m_channels[i].m_alpha), i);
		save_item(NAME(m_channels[i].m_clip_start), i);
		save_item(NAME(m_channels[i].m_clip_end), i);
		save_item(NAME(m_channels[i].m_frame_rate), i);
		save_item(NAME(m_channels[i].m_field_counter), i);
		save_item(NAME(m_channels[i].m_line_size), i);
		save_item(NAME(m_channels[i].m_line_counter), i);
		save_item(NAME(m_channels[i].m_page_index), i);
		save_item(NAME(m_channels[i].m_next_desc_ptr), i);
		save_item(NAME(m_channels[i].m_start_desc_ptr), i);
		save_item(NAME(m_channels[i].m_descriptors[0]), i);
		save_item(NAME(m_channels[i].m_descriptors[1]), i);
		save_item(NAME(m_channels[i].m_descriptors[2]), i);
		save_item(NAME(m_channels[i].m_descriptors[3]), i);
		save_item(NAME(m_channels[i].m_fifo_threshold), i);
		save_item(NAME(m_channels[i].m_fifo_gio_ptr), i);
		save_item(NAME(m_channels[i].m_fifo_video_ptr), i);
	}

	m_i2c_data_out.resolve_safe();
	m_i2c_data_in.resolve_safe(0x00);
	m_i2c_stop.resolve_safe();
}

void vino_device::device_reset()
{
	m_rev_id = 0xb0;
	m_control = 0;
	m_int_status = 0;

	for (uint32_t i = CHAN_A; i < CHAN_COUNT; i++)
	{
		m_channels[i].m_field_counter = 0;
		m_channels[i].m_line_size = 0;
		m_channels[i].m_line_counter = 0;
		m_channels[i].m_page_index = 0;
		m_channels[i].m_next_desc_ptr = 0;
		m_channels[i].m_start_desc_ptr = 0;
		for (uint32_t j = 0; j < 4; j++)
		{
			m_channels[i].m_descriptors[j] = DESC_VALID_BIT;
		}
	}
}

READ32_MEMBER(vino_device::read)
{
	switch (offset & ~1)
	{
	case 0x0000/4:  // Rev/ID
		LOGMASKED(LOG_READS, "%s: Rev/ID read: %08x & %08x\n", machine().describe_context(), m_rev_id, mem_mask);
		return m_rev_id;
	case 0x0008/4:  // Control
		LOGMASKED(LOG_READS, "%s: Control read: %08x & %08x\n", machine().describe_context(), m_control, mem_mask);
		return m_control;
	case 0x0010/4:  // Interrupt Status
		LOGMASKED(LOG_READS, "%s: Interrupt Status read: %08x & %08x\n", machine().describe_context(), m_int_status, mem_mask);
		return m_int_status;
	case 0x0018/4:  // I2C Control
		LOGMASKED(LOG_READS, "%s: I2C Control read: %08x & %08x\n", machine().describe_context(), m_i2c_ctrl, mem_mask);
		return m_i2c_ctrl;
	case 0x0020/4:  // I2C Data
		LOGMASKED(LOG_READS, "%s: I2C Data read: %08x & %08x\n", machine().describe_context(), m_i2c_data, mem_mask);
		return m_i2c_data;
	case 0x0028/4:  // ChA Alpha
	case 0x00b0/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_READS, "%s: Ch%c Alpha read: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', m_channels[channel].m_alpha, mem_mask);
		return m_channels[channel].m_alpha;
	}
	case 0x0030/4:  // ChA Clipping Start
	case 0x00b8/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_READS, "%s: Ch%c Clipping Start read: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', m_channels[channel].m_clip_start, mem_mask);
		return m_channels[channel].m_clip_start;
	}
	case 0x0038/4:  // ChA Clipping End
	case 0x00c0/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_READS, "%s: Ch%c Clipping End read: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', m_channels[channel].m_clip_end, mem_mask);
		return m_channels[channel].m_clip_end;
	}
	case 0x0040/4:  // ChA Frame Rate
	case 0x00c8/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_READS, "%s: Ch%c Frame Rate read: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', m_channels[channel].m_frame_rate, mem_mask);
		return m_channels[channel].m_frame_rate;
	}
	case 0x0048/4:  // ChA Field Counter
	case 0x00d0/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_READS, "%s: Ch%c Field Counter read: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', m_channels[channel].m_field_counter, mem_mask);
		return m_channels[channel].m_field_counter;
	}
	case 0x0050/4:  // ChA Line Size
	case 0x00d8/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_READS, "%s: Ch%c Line Size read: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', m_channels[channel].m_line_size, mem_mask);
		return m_channels[channel].m_line_size;
	}
	case 0x0058/4:  // ChA Line Counter
	case 0x00e0/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_READS, "%s: Ch%c Line Counter read: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', m_channels[channel].m_line_counter, mem_mask);
		return m_channels[channel].m_line_counter;
	}
	case 0x0060/4:  // ChA Page Index
	case 0x00e8/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_READS, "%s: Ch%c Page Index read: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', m_channels[channel].m_page_index, mem_mask);
		return m_channels[channel].m_page_index;
	}
	case 0x0068/4:  // ChA Pointer to Next Four Descriptors
	case 0x00f0/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_READS, "%s: Ch%c Pointer to Next Four Descriptors read: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', m_channels[channel].m_next_desc_ptr, mem_mask);
		return m_channels[channel].m_next_desc_ptr;
	}
	case 0x0070/4:  // ChA Pointer to Start of Descriptor Table
	case 0x00f8/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_READS, "%s: Ch%c Pointer to Start of Descriptor Table read: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', m_channels[channel].m_start_desc_ptr, mem_mask);
		return m_channels[channel].m_start_desc_ptr;
	}
	case 0x0078/4:  // ChA Descriptor 0
	case 0x0100/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_READS, "%s: Ch%c Descriptor 0 Data read: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', m_channels[channel].m_descriptors[0], mem_mask);
		return m_channels[channel].m_descriptors[0];
	}
	case 0x0080/4:  // ChA Descriptor 1
	case 0x0108/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_READS, "%s: Ch%c Descriptor 1 Data read: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', m_channels[channel].m_descriptors[1], mem_mask);
		return m_channels[channel].m_descriptors[1];
	}
	case 0x0088/4:  // ChA Descriptor 2
	case 0x0110/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_READS, "%s: Ch%c Descriptor 2 Data read: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', m_channels[channel].m_descriptors[2], mem_mask);
		return m_channels[channel].m_descriptors[2];
	}
	case 0x0090/4:  // ChA Descriptor 3
	case 0x0118/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_READS, "%s: Ch%c Descriptor 3 Data read: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', m_channels[channel].m_descriptors[3], mem_mask);
		return m_channels[channel].m_descriptors[3];
	}
	case 0x0098/4:  // ChA FIFO Threshold
	case 0x0120/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_READS, "%s: Ch%c FIFO Threshold read: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', m_channels[channel].m_fifo_threshold, mem_mask);
		return m_channels[channel].m_fifo_threshold;
	}
	case 0x00a0/4:  // ChA FIFO Read Pointer
	case 0x0128/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_READS, "%s: Ch%c FIFO Read Pointer read: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', m_channels[channel].m_fifo_gio_ptr, mem_mask);
		return m_channels[channel].m_fifo_gio_ptr;
	}
	case 0x00a8/4:  // ChA FIFO Write Pointer
	case 0x0130/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_READS, "%s: Ch%c FIFO Write Pointer read: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', m_channels[channel].m_fifo_video_ptr, mem_mask);
		return m_channels[channel].m_fifo_video_ptr;
	}
	default:
		LOGMASKED(LOG_READS | LOG_UNKNOWN, "%s: Unknown VINO read: %08x & %08x\n", machine().describe_context(), 0x00080000 + offset*4, mem_mask);
		return 0;
	}
	return 0;
}

WRITE32_MEMBER(vino_device::write)
{
	switch (offset & ~1)
	{
	case 0x0000/4:  // Rev/ID
		LOGMASKED(LOG_WRITES, "%s: Rev/ID write (ignored): %08x & %08x\n", machine().describe_context(), data, mem_mask);
		break;
	case 0x0008/4:  // Control
		LOGMASKED(LOG_WRITES, "%s: Control write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_control = data & CTRL_MASK;
		break;
	case 0x0010/4:  // Interrupt Status
		LOGMASKED(LOG_WRITES, "%s: Interrupt Status write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		for (uint32_t i = 0; i < 6; i++)
		{
			if (!BIT(data, i))
			{
				m_int_status &= ~(1 << i);
			}
		}
		// TODO: Handle interrupt all-clear
		break;
	case 0x0018/4:  // I2C Control
		LOGMASKED(LOG_WRITES, "%s: I2C Control write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_i2c_ctrl = data & I2C_CTRL_MASK;
		break;
	case 0x0020/4:  // I2C Data
		LOGMASKED(LOG_WRITES, "%s: I2C Data write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		m_i2c_data = data & I2C_DATA_MASK;
		break;
	case 0x0028/4:  // ChA Alpha
	case 0x00b0/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_WRITES, "%s: Ch%c Alpha write: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', data, mem_mask);
		m_channels[channel].m_alpha = data & ALPHA_MASK;
		break;
	}
	case 0x0030/4:  // ChA Clipping Start
	case 0x00b8/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_WRITES, "%s: Ch%c Clipping Start write: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', data, mem_mask);
		m_channels[channel].m_clip_start = data & CLIP_REG_MASK;
		break;
	}
	case 0x0038/4:  // ChA Clipping End
	case 0x00c0/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_WRITES, "%s: Ch%c Clipping End write: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', data, mem_mask);
		m_channels[channel].m_clip_end = data & CLIP_REG_MASK;
		break;
	}
	case 0x0040/4:  // ChA Frame Rate
	case 0x00c8/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_WRITES, "%s: Ch%c Frame Rate write: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', data, mem_mask);
		m_channels[channel].m_frame_rate = data & FRAME_RATE_REG_MASK;
		break;
	}
	case 0x0048/4:  // ChA Field Counter
	case 0x00d0/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_WRITES, "%s: Ch%c Field Counter write (ignored): %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', data, mem_mask);
		break;
	}
	case 0x0050/4:  // ChA Line Size
	case 0x00d8/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_WRITES, "%s: Ch%c Line Size write: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', data, mem_mask);
		m_channels[channel].m_line_size = data & LINE_SIZE_MASK;
		break;
	}
	case 0x0058/4:  // ChA Line Counter
	case 0x00e0/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_WRITES, "%s: Ch%c Line Counter write: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', data, mem_mask);
		m_channels[channel].m_line_counter = data & LINE_COUNTER_MASK;
		break;
	}
	case 0x0060/4:  // ChA Page Index
	case 0x00e8/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_WRITES, "%s: Ch%c Page Index write: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', data, mem_mask);
		m_channels[channel].m_page_index = data & PAGE_INDEX_MASK;
		break;
	}
	case 0x0068/4:  // ChA Pointer to Next Four Descriptors
	case 0x00f0/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_WRITES, "%s: Ch%c Pointer to Next Four Descriptors write: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', data, mem_mask);
		m_channels[channel].m_next_desc_ptr = data & DESC_PTR_MASK;
		break;
	}
	case 0x0070/4:  // ChA Pointer to Start of Descriptor Table
	case 0x00f8/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_WRITES, "%s: Ch%c Pointer to Start of Descriptor Table write: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', data, mem_mask);
		m_channels[channel].m_start_desc_ptr = data & DESC_PTR_MASK;
		break;
	}
	case 0x0078/4:  // ChA Descriptor 0
	case 0x0100/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_WRITES, "%s: Ch%c Descriptor 0 Data write: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', data, mem_mask);
		m_channels[channel].m_descriptors[0] = (data & DESC_PTR_MASK);
		m_channels[channel].m_descriptors[0] |= DESC_VALID_BIT; // TODO: Unsure if this is right
		break;
	}
	case 0x0080/4:  // ChA Descriptor 1
	case 0x0108/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_WRITES, "%s: Ch%c Descriptor 1 Data write: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', data, mem_mask);
		m_channels[channel].m_descriptors[1] = (data & DESC_PTR_MASK);
		m_channels[channel].m_descriptors[1] |= DESC_VALID_BIT; // TODO: Unsure if this is right
		break;
	}
	case 0x0088/4:  // ChA Descriptor 2
	case 0x0110/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_WRITES, "%s: Ch%c Descriptor 2 Data write: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', data, mem_mask);
		m_channels[channel].m_descriptors[2] = (data & DESC_PTR_MASK);
		m_channels[channel].m_descriptors[2] |= DESC_VALID_BIT; // TODO: Unsure if this is right
		break;
	}
	case 0x0090/4:  // ChA Descriptor 3
	case 0x0118/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_WRITES, "%s: Ch%c Descriptor 3 Data write: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', data, mem_mask);
		m_channels[channel].m_descriptors[3] = (data & DESC_PTR_MASK);
		m_channels[channel].m_descriptors[3] |= DESC_VALID_BIT; // TODO: Unsure if this is right
		break;
	}
	case 0x0098/4:  // ChA FIFO Threshold
	case 0x0120/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_WRITES, "%s: Ch%c FIFO Threshold write: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', data, mem_mask);
		m_channels[channel].m_fifo_threshold = data & FIFO_MASK;
		break;
	}
	case 0x00a0/4:  // ChA FIFO Read Pointer
	case 0x0128/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_WRITES, "%s: Ch%c FIFO Read Pointer write (ignored): %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', data, mem_mask);
		break;
	}
	case 0x00a8/4:  // ChA FIFO Write Pointer
	case 0x0130/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_WRITES, "%s: Ch%c FIFO Write Pointer write (ignored): %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', data, mem_mask);
		break;
	}
	default:
		LOGMASKED(LOG_WRITES | LOG_UNKNOWN, "%s: Unknown VINO write: %08x = %08x & %08x\n", machine().describe_context(), 0x00080000 + offset*4, data, mem_mask);
		break;
	}
}
