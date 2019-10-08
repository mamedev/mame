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
#define LOG_DESCS       (1 << 3)
#define LOG_DMA         (1 << 4)
#define LOG_DMA_DATA    (1 << 5)
#define LOG_FIFO        (1 << 6)
#define LOG_FIELDS      (1 << 7)
#define LOG_COORDS      (1 << 8)
#define LOG_INPUTS      (1 << 9)
#define LOG_INTERRUPTS  (1 << 10)
#define LOG_INDICES     (1 << 11)
#define LOG_DEFAULT     (LOG_WRITES | LOG_FIELDS | LOG_DMA | LOG_DESCS | LOG_READS | LOG_INTERRUPTS | LOG_INDICES | LOG_COORDS)

#define VERBOSE         (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(VINO, vino_device, "vino", "SGI VINO Controller")

vino_device::vino_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VINO, tag, owner, clock)
	, m_i2c_data_out(*this)
	, m_i2c_data_in(*this)
	, m_i2c_stop(*this)
	, m_interrupt_cb(*this)
	, m_picture(*this, "srcimg")
	, m_space(*this, finder_base::DUMMY_TAG, -1)
	, m_input_bitmap(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vino_device::device_start()
{
	m_channels[0].m_fetch_timer = timer_alloc(TIMER_FETCH_CHA);
	m_channels[1].m_fetch_timer = timer_alloc(TIMER_FETCH_CHB);

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
		save_item(NAME(m_channels[i].m_fifo), i);
		save_item(NAME(m_channels[i].m_active_alpha), i);
		save_item(NAME(m_channels[i].m_curr_line), i);
		save_item(NAME(m_channels[i].m_frame_mask_shift), i);
		save_item(NAME(m_channels[i].m_frame_mask_shifter), i);
		save_item(NAME(m_channels[i].m_pixel_size), i);
		save_item(NAME(m_channels[i].m_next_fifo_word), i);
		save_item(NAME(m_channels[i].m_word_pixel_counter), i);
		save_item(NAME(m_channels[i].m_pixels_per_even_field), i);
		save_item(NAME(m_channels[i].m_pixels_per_odd_field), i);
		save_item(NAME(m_channels[i].m_field_pixels_remaining[0]), i);
		save_item(NAME(m_channels[i].m_field_pixels_remaining[1]), i);
		save_item(NAME(m_channels[i].m_end_of_field), i);

		m_channels[i].m_fetch_timer->adjust(attotime::never);
	}

	m_i2c_data_out.resolve_safe();
	m_i2c_data_in.resolve_safe(0x00);
	m_i2c_stop.resolve_safe();
	m_interrupt_cb.resolve_safe();
}

void vino_device::device_reset()
{
	m_rev_id = 0xb0;
	m_control = 0;
	m_int_status = 0;
	m_i2c_ctrl = 0;
	m_i2c_data = 0;

	for (uint32_t i = CHAN_A; i < CHAN_COUNT; i++)
	{
		m_channels[i].m_alpha = 0;
		m_channels[i].m_clip_start = 0;
		m_channels[i].m_clip_end = 0;
		m_channels[i].m_frame_rate = 0;
		m_channels[i].m_field_counter = 0;
		m_channels[i].m_line_size = 0;
		m_channels[i].m_line_counter = 0;
		m_channels[i].m_page_index = 0;
		m_channels[i].m_next_desc_ptr = 0;
		m_channels[i].m_start_desc_ptr = 0;
		m_channels[i].m_fifo_threshold = 0;
		m_channels[i].m_fifo_gio_ptr = 0;
		m_channels[i].m_fifo_video_ptr = 0;
		m_channels[i].m_active_alpha = 0;
		m_channels[i].m_curr_line = 0;
		m_channels[i].m_frame_mask_shift = 0;
		m_channels[i].m_frame_mask_shifter = 0;
		m_channels[i].m_pixel_size = 0;
		m_channels[i].m_next_fifo_word = 0;
		m_channels[i].m_word_pixel_counter = 0;

		for (uint32_t j = 0; j < 4; j++)
		{
			m_channels[i].m_descriptors[j] = 0ULL;
		}

		m_channels[i].m_fetch_timer->adjust(attotime::never);
	}
}

void vino_device::device_add_mconfig(machine_config &config)
{
	IMAGE_PICTURE(config, m_picture);
}

void vino_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_FETCH_CHA || id == TIMER_FETCH_CHB)
		fetch_pixel((int)id);
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
		control_w(data & CTRL_MASK);
		break;
	case 0x0010/4:  // Interrupt Status
		LOGMASKED(LOG_WRITES, "%s: Interrupt Status write: %08x & %08x\n", machine().describe_context(), data, mem_mask);
		for (uint32_t bit = 0; bit < 6; bit++)
		{
			if (!BIT(data, bit))
			{
				m_int_status &= ~(1 << bit);
			}
		}
		interrupts_w(m_int_status);
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
		frame_rate_w(channel, data);
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
		page_index_w(channel, data & PAGE_INDEX_MASK);
		break;
	}
	case 0x0068/4:  // ChA Pointer to Next Four Descriptors
	case 0x00f0/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_WRITES, "%s: Ch%c Pointer to Next Four Descriptors write: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', data, mem_mask);
		next_desc_w(channel, data & DESC_PTR_MASK);
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
		m_channels[channel].m_descriptors[0] |= DESC_VALID_BIT;
		break;
	}
	case 0x0080/4:  // ChA Descriptor 1
	case 0x0108/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_WRITES, "%s: Ch%c Descriptor 1 Data write: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', data, mem_mask);
		m_channels[channel].m_descriptors[1] = (data & DESC_PTR_MASK);
		m_channels[channel].m_descriptors[1] |= DESC_VALID_BIT;
		break;
	}
	case 0x0088/4:  // ChA Descriptor 2
	case 0x0110/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_WRITES, "%s: Ch%c Descriptor 2 Data write: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', data, mem_mask);
		m_channels[channel].m_descriptors[2] = (data & DESC_PTR_MASK);
		m_channels[channel].m_descriptors[2] |= DESC_VALID_BIT;
		break;
	}
	case 0x0090/4:  // ChA Descriptor 3
	case 0x0118/4:  // ChB ...
	{
		const uint32_t channel = (offset < 0x00b0/4) ? 0 : 1;
		LOGMASKED(LOG_WRITES, "%s: Ch%c Descriptor 3 Data write: %08x & %08x\n", machine().describe_context(), channel ? 'B' : 'A', data, mem_mask);
		m_channels[channel].m_descriptors[3] = (data & DESC_PTR_MASK);
		m_channels[channel].m_descriptors[3] |= DESC_VALID_BIT;
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

bool vino_device::is_even_field(int channel)
{
	return BIT(m_channels[channel].m_field_counter, 0);
}

bool vino_device::is_interleaved(int channel)
{
	static const uint32_t s_masks[2] = { CTRL_CHA_INTERLEAVE_EN, CTRL_CHB_INTERLEAVE_EN };
	return (m_control & s_masks[channel]) != 0;
}

void vino_device::end_of_field(int channel)
{
	LOGMASKED(LOG_FIELDS, "End of field for channel %c\n", channel ? 'B' : 'A');

	channel_t &chan = m_channels[channel];
	if (BIT(chan.m_frame_mask_shifter, 0))
	{
		do_dma_transfer(channel);

		if (!is_interleaved(channel))
		{
			page_index_w(channel, 0);
		}
		else if (is_even_field(channel))
		{
			line_count_w(channel, 0);
			page_index_w(channel, chan.m_line_size + 8);
			next_desc_w(channel, chan.m_start_desc_ptr);
		}
		else // odd field
		{
			line_count_w(channel, 0);
			page_index_w(channel, 0);
			chan.m_start_desc_ptr = chan.m_next_desc_ptr;
		}

		static const uint32_t s_eof_masks[2] = { ISR_CHA_EOF, ISR_CHB_EOF };
		interrupts_w(m_int_status | s_eof_masks[channel]);
	}

	chan.m_field_pixels_remaining[0] = chan.m_pixels_per_odd_field;
	chan.m_field_pixels_remaining[1] = chan.m_pixels_per_even_field;
	chan.m_field_x = 0;
	chan.m_field_y = 0;

	chan.m_field_counter++;

	chan.m_frame_mask_shifter >>= 1;
	chan.m_frame_mask_shift--;
	if (chan.m_frame_mask_shift == 0)
		load_frame_mask_shifter(channel);

	chan.m_active_alpha = chan.m_alpha;
}

void vino_device::do_dma_transfer(int channel)
{
	channel_t &chan = m_channels[channel];
	LOGMASKED(LOG_DMA, "Transferring %08x words via DMA\n", chan.m_fifo_video_ptr - chan.m_fifo_gio_ptr);
	while (chan.m_fifo_gio_ptr != chan.m_fifo_video_ptr && !(chan.m_descriptors[0] & DESC_STOP_BIT))
	{
		const uint32_t address = (chan.m_descriptors[0] & 0x3ffff000) | (chan.m_page_index & 0x00000ff8);
		const uint64_t word = chan.m_fifo[chan.m_fifo_gio_ptr >> 3];
		LOGMASKED(LOG_DMA_DATA, "Writing %08x%08x to %08x\n", (uint32_t)(word >> 32), (uint32_t)word, address);
		m_space->write_qword(address, word);
		page_index_w(channel, chan.m_page_index + 8);
		if (is_interleaved(channel))
		{
			line_count_w(channel, chan.m_line_counter + 8);
		}
		chan.m_fifo_gio_ptr += 8;
	}
	chan.m_fifo_gio_ptr = 0;
	chan.m_fifo_video_ptr = 0;
}

void vino_device::push_fifo(int channel)
{
	channel_t &chan = m_channels[channel];
	LOGMASKED(LOG_FIFO, "Pushing %08x%08x onto FIFO, new ptr %08x, remaining (%d/%d)\n",
		(uint32_t)(chan.m_next_fifo_word >> 32),
		(uint32_t)chan.m_next_fifo_word,
		chan.m_fifo_video_ptr + 8,
		chan.m_field_pixels_remaining[0],
		chan.m_field_pixels_remaining[1]);
	if (chan.m_fifo_video_ptr < 0x400)
	{
		chan.m_fifo[chan.m_fifo_video_ptr >> 3] = chan.m_next_fifo_word;
		chan.m_fifo_video_ptr += 8;
		if (chan.m_fifo_video_ptr >= chan.m_fifo_threshold)
		{
			do_dma_transfer(channel);
		}
	}
	chan.m_next_fifo_word = 0;
	chan.m_word_pixel_counter = 0;
}

void vino_device::argb_to_yuv(uint32_t argb, int32_t &y, int32_t &u, int32_t &v)
{
	const int32_t r = (argb >> 16) & 0xff;
	const int32_t g = (argb >>  8) & 0xff;
	const int32_t b = (argb >>  0) & 0xff;
	y = (int32_t)(0.299f * r + 0.587f * g + 0.114f * b);
	u = (int32_t)(0.492f * (b - y));
	v = (int32_t)(0.877f * (r - y));
}

uint32_t vino_device::yuv_to_abgr(int channel, int32_t y, int32_t u, int32_t v)
{
	int32_t r = y + 1.14f * v;
	int32_t g = y - 0.395f * u - 0.581f * v;
	int32_t b = y + 2.032f * u;
	r = (r > 255) ? 255 : (r < 0 ? 0 : r);
	g = (g > 255) ? 255 : (g < 0 ? 0 : g);
	b = (b > 255) ? 255 : (b < 0 ? 0 : b);
	LOGMASKED(LOG_INPUTS, "%d,%d,%d in yuv is %08x in rgb\n", y, u, v, (b << 16) | (g << 8) | r);
	return (m_channels[channel].m_alpha << 24) | (b << 16) | (g << 8) | r;
}

bool vino_device::merge_pixel(int channel, int32_t y, int32_t u, int32_t v, pixel_format_t format)
{
	channel_t &chan = m_channels[channel];
	switch (format)
	{
		case FORMAT_RGBA32:
		{
			const uint32_t shift = 32 - (chan.m_word_pixel_counter << 5);
			chan.m_next_fifo_word &= ~(0xffffffffULL << shift);
			chan.m_next_fifo_word |= (uint64_t)yuv_to_abgr(channel, y, u, v) << shift;
			chan.m_word_pixel_counter++;
			count_pixel(channel);
			return (chan.m_word_pixel_counter == 2);
		}
		case FORMAT_YUV422:
		{
			const uint8_t uy = (uint8_t)y;
			const uint8_t uu = (uint8_t)u;
			const uint8_t uv = (uint8_t)v;
			const uint32_t y_shift = (3 - chan.m_word_pixel_counter) * 16;
			const uint64_t y_mask = 0xffULL << y_shift;
			chan.m_next_fifo_word &= ~y_mask;
			chan.m_next_fifo_word |= (uy << y_shift);
			if (!(chan.m_word_pixel_counter & 1))
			{
				chan.m_next_fifo_word &= 0xffffff << (y_shift - 8);
				chan.m_next_fifo_word |= (uu << (y_shift + 8));
				chan.m_next_fifo_word |= (uv << (y_shift - 8));
			}
			chan.m_word_pixel_counter++;
			count_pixel(channel);
			return (chan.m_word_pixel_counter == 4);
		}
		case FORMAT_RGBA8:
		{
			const uint32_t abgr = yuv_to_abgr(channel, y, u, v);
			const uint8_t b =  (abgr >> 16) & 0xc0;
			const uint8_t g = ((abgr >>  8) & 0xe0) >> 2;
			const uint8_t r = ((abgr >>  0) & 0xe0) >> 5;
			const uint8_t bgr = b | g | r;
			const uint32_t shift = 56 - (chan.m_word_pixel_counter << 3);
			chan.m_next_fifo_word &= ~(0xffULL << shift);
			chan.m_next_fifo_word |= (uint64_t)bgr << shift;
			chan.m_word_pixel_counter++;
			count_pixel(channel);
			return (chan.m_word_pixel_counter == 8);
		}
		case FORMAT_Y8:
		{
			const uint8_t y8 = (uint8_t)y;
			const uint32_t shift = 56 - (chan.m_word_pixel_counter << 3);
			chan.m_next_fifo_word &= ~(0xffULL << shift);
			chan.m_next_fifo_word |= (uint64_t)y8 << shift;
			chan.m_word_pixel_counter++;
			count_pixel(channel);
			return (chan.m_word_pixel_counter == 8);
		}
	}
	return false;
}

void vino_device::count_pixel(int channel)
{
	channel_t &chan = m_channels[channel];

	const uint32_t even_or_odd = BIT(chan.m_field_counter, 0);

	int32_t pixels_consumed = chan.m_decimation;
	chan.m_field_x += chan.m_decimation;
	if (chan.m_field_x >= chan.m_field_width)
	{
		chan.m_field_x = 0;
		chan.m_field_y++;
		if (is_interleaved(channel))
		{
			//pixels_consumed += chan.m_field_width * chan.m_decimation;
		}
	}

	chan.m_field_pixels_remaining[even_or_odd] -= pixels_consumed;
	if (chan.m_field_pixels_remaining[even_or_odd] == 0)
	{
		chan.m_field_x = 0;
		chan.m_field_y = 0;
		chan.m_end_of_field = true;
	}
}

vino_device::pixel_format_t vino_device::get_current_format(int channel)
{
	static const uint32_t rgb_masks[2] = { CTRL_CHA_COLOR_SPACE_RGB, CTRL_CHB_COLOR_SPACE_RGB };
	static const uint32_t luma_masks[2] = { CTRL_CHA_LUMA_ONLY, CTRL_CHB_LUMA_ONLY };
	static const uint32_t dither_masks[2] = { CTRL_CHA_DITHER_EN, CTRL_CHB_DITHER_EN };

	const bool rgb_mode = (m_control & rgb_masks[channel]) != 0;
	const bool luma_mode = (m_control & luma_masks[channel]) != 0;
	const bool dither_mode = (m_control & dither_masks[channel]) != 0;

	if (rgb_mode)
		return dither_mode ? FORMAT_RGBA8 : FORMAT_RGBA32;
	else
		return luma_mode ? FORMAT_Y8 : FORMAT_YUV422;
}

void vino_device::process_pixel(int channel, int32_t y, int32_t u, int32_t v)
{
	if (merge_pixel(channel, y, u, v, get_current_format(channel)))
	{
		push_fifo(channel);
	}
	if (m_channels[channel].m_end_of_field)
	{
		end_of_field(channel);
		m_channels[channel].m_end_of_field = false;
	}
}

uint32_t vino_device::linear_rgb(uint32_t a, uint32_t b, float f)
{
	const int32_t ra = (a >> 16) & 0xff;
	const int32_t ga = (a >>  8) & 0xff;
	const int32_t ba = (a >>  0) & 0xff;
	const int32_t rb = (b >> 16) & 0xff;
	const int32_t gb = (b >>  8) & 0xff;
	const int32_t bb = (b >>  0) & 0xff;
	const float inv_f = 1.0f - f;
	int32_t rc = (int32_t)(ra * inv_f + rb * f);
	int32_t gc = (int32_t)(ga * inv_f + gb * f);
	int32_t bc = (int32_t)(ba * inv_f + bb * f);
	rc = (rc > 255 ? 255 : (rc < 0 ? 0 : rc));
	gc = (gc > 255 ? 255 : (gc < 0 ? 0 : gc));
	bc = (bc > 255 ? 255 : (bc < 0 ? 0 : bc));
	return (rc << 16) | (gc << 8) | bc;
}

uint32_t vino_device::bilinear_pixel(float s, float t)
{
	if (m_input_bitmap == nullptr)
		return 0xff000000;

	const uint32_t width = m_input_bitmap->width() - 1;
	const uint32_t height = m_input_bitmap->height() - 1;

	int32_t s0 = (int32_t)floorf(s * width);
	int32_t s1 = (int32_t)floorf(s * width + 1);
	int32_t t0 = (int32_t)floorf(t * height);
	int32_t t1 = (int32_t)floorf(t * height + 1);

	s0 = (s0 < 0 ? 0 : (s0 > width ? width : s0));
	s1 = (s1 < 0 ? 0 : (s1 > width ? width : s1));
	t0 = (t0 < 0 ? 0 : (t0 > height ? height : t0));
	t1 = (t1 < 0 ? 0 : (t1 > height ? height : t1));

	LOGMASKED(LOG_COORDS, "lerping from %d,%d to %d,%d\n", s0, t0, s1, t1);
	const uint32_t p00 = m_input_bitmap->pix(t0, s0);
	const uint32_t p01 = m_input_bitmap->pix(t0, s1);
	const uint32_t p10 = m_input_bitmap->pix(t1, s0);
	const uint32_t p11 = m_input_bitmap->pix(t1, s1);

	LOGMASKED(LOG_INPUTS, "%08x, %08x, %08x, %08x\n", p00, p01, p10, p11);

	float ip = 0.0f;
	const float sf = modff(s, &ip);
	const float tf = modff(t, &ip);
	const uint32_t top = linear_rgb(p00, p01, sf);
	const uint32_t bot = linear_rgb(p10, p11, sf);
	LOGMASKED(LOG_INPUTS, "%08x, %08x\n", top, bot);
	const uint32_t mixed = linear_rgb(top, bot, tf);
	LOGMASKED(LOG_INPUTS, "%08x\n", mixed);
	return 0xff000000 | mixed;
}

void vino_device::input_pixel(int channel, int32_t &y, int32_t &u, int32_t &v)
{
	m_input_bitmap = &m_picture->get_bitmap();
	if (m_input_bitmap)
	{
		channel_t &chan = m_channels[channel];
		if (is_interleaved(channel))
		{
			const uint32_t even_or_odd = BIT(chan.m_field_counter, 0);
			const uint32_t even_or_odd_offset = (even_or_odd ? 0 : 1);
			float s = (float)chan.m_field_x / chan.m_field_width;
			float t = (float)chan.m_field_y / chan.m_field_height[even_or_odd];
			LOGMASKED(LOG_COORDS, "%d, %d coords: %f, %f\n", chan.m_field_x, chan.m_field_y * 2 + even_or_odd_offset, s, t);
			const uint32_t argb = bilinear_pixel(s, t);
			argb_to_yuv(argb, y, u, v);
			LOGMASKED(LOG_INPUTS, "%08x in yuv is %d,%d,%d\n", argb, y, u, v);
		}
		else
		{
			float s = (float)chan.m_field_x / chan.m_field_width;
			float t = (float)chan.m_field_y / chan.m_field_height[0];
			LOGMASKED(LOG_COORDS, "%d, %d coords: %f, %f\n", chan.m_field_x, chan.m_field_y, s, t);
			const uint32_t argb = bilinear_pixel(s, t);
			argb_to_yuv(argb, y, u, v);
			LOGMASKED(LOG_INPUTS, "%08x in yuv is %d,%d,%d\n", argb, y, u, v);
		}
	}
}

void vino_device::fetch_pixel(int channel)
{
	channel_t &chan = m_channels[channel];
	if (chan.m_decimation > 1 && (chan.m_field_x % chan.m_decimation) != 0)
	{
		count_pixel(channel);
		return;
	}
	if (BIT(chan.m_frame_mask_shifter, 0))
	{
		int32_t y = 0, u = 0, v = 0;
		input_pixel(channel, y, u, v);
		process_pixel(channel, y, u, v);
	}
}

attotime vino_device::calculate_field_rate(int channel)
{
	channel_t &chan = m_channels[channel];
	const uint32_t fields_per_second = (BIT(chan.m_frame_rate, 0) == FRAME_RATE_PAL) ? 50 : 60;
	return attotime::from_hz(fields_per_second);
}

attotime vino_device::calculate_fetch_rate(int channel)
{
	channel_t &chan = m_channels[channel];
	const uint32_t frames_per_second = (BIT(chan.m_frame_rate, 0) == FRAME_RATE_PAL) ? 25 : 30;

	const uint32_t x_end = chan.m_clip_end & CLIP_X_MASK;
	const uint32_t y_end_even = (chan.m_clip_end >> CLIP_YEVEN_SHIFT) & CLIP_YEVEN_MASK;
	const uint32_t y_end_odd  = (chan.m_clip_end >> CLIP_YODD_SHIFT)  & CLIP_YODD_MASK;

	const uint32_t x_start = chan.m_clip_start & CLIP_X_MASK;
	const uint32_t y_start_even = (chan.m_clip_start >> CLIP_YEVEN_SHIFT) & CLIP_YEVEN_MASK;
	const uint32_t y_start_odd  = (chan.m_clip_start >> CLIP_YODD_SHIFT)  & CLIP_YODD_MASK;

	const uint32_t width = (x_end - x_start) + 1;
	const uint32_t height_even = (y_end_even - y_start_even) + 1;
	const uint32_t height_odd  = (y_end_odd  - y_start_odd)  + 1;

	const uint32_t field_size_even = (height_even * width) / chan.m_decimation;
	const uint32_t field_size_odd  = (height_odd  * width) / chan.m_decimation;

	const uint32_t frame_size = field_size_even + field_size_odd;

	LOGMASKED(LOG_DMA, "Frames per second: %d\n", frames_per_second);
	LOGMASKED(LOG_DMA, "Frame width: %d - %d = %d\n", x_end, x_start, width);
	LOGMASKED(LOG_DMA, "Even field height: %d - %d = %d\n", y_end_even, y_start_even, height_even);
	LOGMASKED(LOG_DMA, "Odd field height: %d - %d = %d\n", y_end_odd, y_start_odd, height_odd);
	LOGMASKED(LOG_DMA, "Even field pixels: %d\n", field_size_even);
	LOGMASKED(LOG_DMA, "Odd field pixels: %d\n", field_size_odd);
	chan.m_pixels_per_even_field = (int32_t)field_size_even;
	chan.m_pixels_per_odd_field = (int32_t)field_size_odd;
	chan.m_field_height[1] = height_even / chan.m_decimation;
	chan.m_field_height[0] = height_odd / chan.m_decimation;
	chan.m_field_width = width;
	chan.m_field_x = 0;
	chan.m_field_y = 0;
	return attotime::from_hz(frames_per_second * frame_size);
}

bool vino_device::page_index_w(int channel, uint32_t data)
{
	channel_t &chan = m_channels[channel];
	const uint32_t old = chan.m_page_index;
	chan.m_page_index = data;
	LOGMASKED(LOG_INDICES, "Page Index write: %08x\n", data);
	while (chan.m_page_index >= 0x1000)
	{
		chan.m_page_index -= 0x1000;
	}
	if (chan.m_page_index < old)
	{
		shift_dma_descriptors(channel);
		return true;
	}
	return false;
}

void vino_device::interrupts_w(uint32_t new_int)
{
	const uint32_t old = m_int_status;
	m_int_status = (new_int & (m_control >> 1)) & ISR_MASK;
	const uint32_t raised_ints = (~old & m_int_status);
	if (raised_ints != 0)
	{
		LOGMASKED(LOG_INTERRUPTS, "Interrupt status %08x, raising interrupt\n", m_int_status);
		m_interrupt_cb(1);
	}
	else if (m_int_status == 0)
	{
		LOGMASKED(LOG_INTERRUPTS, "All interrupts clear, lowering interrupt\n");
		m_interrupt_cb(0);
	}
}

void vino_device::shift_dma_descriptors(int channel)
{
	const uint32_t even_or_odd = BIT(m_channels[channel].m_field_counter, 0);
	LOGMASKED(LOG_DESCS, "Shifting descriptors, remaining pixels %d\n", m_channels[channel].m_field_pixels_remaining[even_or_odd]);
	channel_t &chan = m_channels[channel];
	for (int i = 0; i < 3; i++)
	{
		chan.m_descriptors[i] = chan.m_descriptors[i + 1];
	}
	if (!(chan.m_descriptors[0] & DESC_VALID_BIT))
	{
		LOGMASKED(LOG_DESCS, "Shifted in a descriptor without a valid bit; loading a new set\n");
		load_dma_descriptors(channel, chan.m_next_desc_ptr);
		chan.m_next_desc_ptr += 16;
	}
	else if (chan.m_descriptors[0] & DESC_JUMP_BIT)
	{
		LOGMASKED(LOG_DESCS, "Shifted in a descriptor with a jump bit; loading a new set from %08x\n", chan.m_descriptors[0] & 0x3fffffff);
		load_dma_descriptors(channel, chan.m_descriptors[0] & 0x3fffffff);
	}
}

void vino_device::load_dma_descriptors(int channel, uint32_t addr)
{
	channel_t &chan = m_channels[channel];
	for (int i = 0; i < 4; i++)
	{
		chan.m_descriptors[i] = (uint64_t)m_space->read_dword(addr + (i << 2)) | DESC_VALID_BIT;
		LOGMASKED(LOG_DESCS, "Descriptor %d: %08x\n", i, (uint32_t)chan.m_descriptors[i]);
	}

	if (chan.m_descriptors[0] & DESC_STOP_BIT)
	{
		LOGMASKED(LOG_DESCS, "Shifted in a descriptor with a stop bit; stopping DMA and flagging an interrupt\n");
		static const uint32_t s_stop_masks[2] = { ISR_CHA_DESC, ISR_CHB_DESC };
		interrupts_w(m_int_status | s_stop_masks[channel]);

		static const uint32_t s_dma_mask[2] = { CTRL_CHA_DMA_EN, CTRL_CHB_DMA_EN };
		m_control &= ~s_dma_mask[channel];
		chan.m_fetch_timer->adjust(attotime::never);
	}
}

void vino_device::invalidate_dma_descriptors(int channel)
{
	LOGMASKED(LOG_DESCS, "Invalidating descriptors\n");
	for (int i = 0; i < 4; i++)
		m_channels[channel].m_descriptors[i] &= ~DESC_VALID_BIT;
}

void vino_device::next_desc_w(int channel, uint32_t data)
{
	m_channels[channel].m_next_desc_ptr = data;
	invalidate_dma_descriptors(channel);
	load_dma_descriptors(channel, m_channels[channel].m_next_desc_ptr);
}

bool vino_device::line_count_w(int channel, uint32_t data)
{
	channel_t &chan = m_channels[channel];
	chan.m_line_counter = data & LINE_COUNTER_MASK;
	LOGMASKED(LOG_INDICES, "Line Counter write: %08x\n", data);
	if (chan.m_line_counter == chan.m_line_size)
	{
		chan.m_line_counter = 0xff8;
		return page_index_w(channel, chan.m_page_index + (chan.m_line_size + 8));
	}
	return false;
}

void vino_device::frame_rate_w(int channel, uint32_t data)
{
	m_channels[channel].m_frame_rate = data & FRAME_RATE_REG_MASK;
	load_frame_mask_shifter(channel);
}

void vino_device::load_frame_mask_shifter(int channel)
{
	channel_t &chan = m_channels[channel];
	chan.m_frame_mask_shift = (BIT(chan.m_frame_rate, 0) == FRAME_RATE_PAL) ? 10 : 12;
	chan.m_frame_mask_shifter = chan.m_frame_rate >> 1;
}

void vino_device::control_w(uint32_t data)
{
	const uint32_t old = m_control;
	m_control = data;

	if (m_control & CTRL_CHA_DECIMATE_EN)
	{
		m_channels[0].m_decimation = ((m_control >> CTRL_CHA_DECIMATION_SHIFT) & CTRL_CHA_DECIMATION_MASK) + 1;
	}
	else
	{
		m_channels[0].m_decimation = 1;
	}

	if (m_control & CTRL_CHB_DECIMATE_EN)
	{
		m_channels[1].m_decimation = ((m_control >> CTRL_CHB_DECIMATION_SHIFT) & CTRL_CHB_DECIMATION_MASK) + 1;
	}
	else
	{
		m_channels[1].m_decimation = 1;
	}

	const uint32_t changed = old ^ m_control;
	if (changed == 0)
		return;

	interrupts_w(m_int_status);

	static const uint32_t s_dma_mask[2] = { CTRL_CHA_DMA_EN, CTRL_CHB_DMA_EN };
	for (int channel = 0; channel < 2; channel++)
	{
		if (changed & s_dma_mask[channel])
		{
			if (data & s_dma_mask[channel])
			{
				channel_t &chan = m_channels[channel];
				LOGMASKED(LOG_DMA, "Enabling DMA on channel %c\n", channel ? 'B' : 'A');
				chan.m_field_counter = 0;
				attotime fetch_rate = calculate_fetch_rate(channel);
				chan.m_fetch_timer->adjust(fetch_rate, 0, fetch_rate);
				chan.m_field_pixels_remaining[0] = chan.m_pixels_per_odd_field;
				chan.m_field_pixels_remaining[1] = chan.m_pixels_per_even_field;
				chan.m_field_x = 0;
				chan.m_field_y = 0;
			}
			else
			{
				LOGMASKED(LOG_DMA, "Disabling DMA on channel %c\n", channel ? 'B' : 'A');
				m_channels[channel].m_fetch_timer->adjust(attotime::never);
			}
		}
	}
}
