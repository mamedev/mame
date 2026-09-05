// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

/*
 * SGS-Thomson STi3400 MPEG-1 video decoder (preliminary)
 *
 * The host interface, start-code detector and MPEG-1 picture reconstruction
 * are implemented sufficiently for Cobra 3 software.
 */

#include "emu.h"
#include "sti3400.h"

#define LOG_START_CODES (1U << 1)
#define LOG_INVALID_DATA (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(STI3400, sti3400_device, "sti3400", "SGS-Thomson STi3400 MPEG-1 Video Decoder")

sti3400_device::sti3400_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, STI3400, tag, owner, clock)
	, m_irq_cb(*this)
{
}

void sti3400_device::device_start()
{
	if (!m_dram_size || !std::has_single_bit(m_dram_size) || (m_dram_size % BIT_BUFFER_LEVEL_UNIT_BYTES))
		fatalerror("%s: picture DRAM size must be a power-of-two multiple of 256 bytes\n", tag());

	m_decode_timer = timer_alloc(FUNC(sti3400_device::decode_tick), this);
	m_fifo = std::make_unique<u8[]>(COMPRESSED_DATA_BUFFER_BYTES);
	m_input = std::make_unique<u8[]>(COMPRESSED_DATA_BUFFER_BYTES);
	m_dram = std::make_unique<u8[]>(m_dram_size);
	m_overwrite_display = std::make_unique<u8[]>(m_dram_size);
	m_picture_valid = std::make_unique<u8[]>(m_dram_size / BIT_BUFFER_LEVEL_UNIT_BYTES);
	m_decoder = std::make_unique<mpeg_video>(m_input.get(), MAX_VIDEO_WIDTH, MAX_VIDEO_HEIGHT);
	m_decoder->register_save_state(*this);

	save_item(NAME(m_registers));
	save_pointer(NAME(m_fifo), COMPRESSED_DATA_BUFFER_BYTES);
	save_item(NAME(m_event_position));
	save_item(NAME(m_event_code));
	save_item(NAME(m_fifo_write));
	save_item(NAME(m_fifo_read));
	save_item(NAME(m_decode_position));
	save_item(NAME(m_start_code_shift));
	save_item(NAME(m_event_head));
	save_item(NAME(m_event_tail));
	save_item(NAME(m_event_count));
	save_item(NAME(m_interrupt_status));
	save_item(NAME(m_event_active));
	save_pointer(NAME(m_input), COMPRESSED_DATA_BUFFER_BYTES);
	save_pointer(NAME(m_dram), m_dram_size);
	save_pointer(NAME(m_overwrite_display), m_dram_size);
	save_pointer(NAME(m_picture_valid), m_dram_size / BIT_BUFFER_LEVEL_UNIT_BYTES);
	save_item(NAME(m_input_bytes));
	save_item(NAME(m_input_bit_position));
	save_item(NAME(m_display_pointer));
	save_item(NAME(m_presented_pointer));
	save_item(NAME(m_reconstructed_pointer));
	save_item(NAME(m_forward_pointer));
	save_item(NAME(m_backward_pointer));
	save_item(NAME(m_width));
	save_item(NAME(m_height));
	save_item(NAME(m_decoded_width));
	save_item(NAME(m_decoded_height));
	save_item(NAME(m_frame_rate));
	save_item(NAME(m_task_active));
	save_item(NAME(m_repeat_pending));
	save_item(NAME(m_overwrite_display_active));
}

void sti3400_device::device_reset()
{
	std::fill(std::begin(m_registers), std::end(m_registers), 0);
	m_interrupt_status = 0;
	m_status = STA_RESET;
	m_irq_state = false;
	reset_decoder();
	m_irq_cb(CLEAR_LINE);
}

void sti3400_device::device_post_load()
{
	m_status = decoder_status();
	m_irq_state = bool(m_interrupt_status & m_registers[REG_ITM]);
	m_irq_cb(m_irq_state ? ASSERT_LINE : CLEAR_LINE);
}

void sti3400_device::reset_decoder()
{
	m_fifo_write = 0;
	m_fifo_read = 0;
	m_decode_position = 0;
	m_start_code_shift = MPEG_START_CODE_SHIFT_RESET;
	m_event_head = 0;
	m_event_tail = 0;
	m_event_count = 0;
	m_event_active = false;
	std::fill_n(m_picture_valid.get(), m_dram_size / BIT_BUFFER_LEVEL_UNIT_BYTES, 0);
	m_input_bytes = 0;
	m_input_bit_position = 0;
	m_display_pointer = 0;
	m_presented_pointer = 0;
	m_reconstructed_pointer = 0;
	m_forward_pointer = 0;
	m_backward_pointer = 0;
	m_width = 0;
	m_height = 0;
	m_decoded_width = 0;
	m_decoded_height = 0;
	m_frame_rate = FALLBACK_FRAME_RATE;
	m_task_active = false;
	m_repeat_pending = false;
	m_overwrite_display_active = false;
	m_decoder->clear();
	m_decode_timer->adjust(attotime::never);
}

void sti3400_device::decoder_soft_reset()
{
	reset_decoder();
	update_status();
}

bool sti3400_device::execute_task()
{
	auto const picture_buffer = [this] (u16 pointer)
	{
		const u32 base = (u32(pointer) * BIT_BUFFER_LEVEL_UNIT_BYTES) & (m_dram_size - 1);
		return mpeg_video::picture_buffer{ m_dram.get() + base, m_dram_size - base };
	};
	const mpeg_video::picture_buffers buffers
	{
		picture_buffer(m_reconstructed_pointer),
		picture_buffer(m_forward_pointer),
		picture_buffer(m_backward_pointer)
	};

	for (;;)
	{
		int position = m_input_bit_position;
		int width = m_decoded_width;
		int height = m_decoded_height;
		double frame_rate = m_frame_rate;
		const u64 stream_position = m_decode_position + (position / 8);
		const mpeg_video::decode_result result = m_decoder->decode_buffer(
				position,
				m_input_bytes * 8,
				buffers,
				width,
				height,
				frame_rate);

		if (result == mpeg_video::decode_result::PICTURE)
		{
			m_decoded_width = width;
			m_decoded_height = height;
			m_frame_rate = frame_rate;
			const u32 picture = m_reconstructed_pointer & ((m_dram_size / BIT_BUFFER_LEVEL_UNIT_BYTES) - 1);
			m_picture_valid[picture] = 1;
		}
		else if (result == mpeg_video::decode_result::INVALID_DATA)
		{
			LOGMASKED(LOG_INVALID_DATA, "%s: skipped invalid MPEG video data at byte %08x\n",
				machine().describe_context(), u32(stream_position));
		}

		const u32 bytes_consumed = position / 8;
		if (bytes_consumed)
		{
			std::move(m_input.get() + bytes_consumed, m_input.get() + m_input_bytes, m_input.get());
			m_input_bytes -= bytes_consumed;
			m_decode_position += bytes_consumed;
		}
		m_input_bit_position = position & 7;

		if (result != mpeg_video::decode_result::INVALID_DATA)
			return result != mpeg_video::decode_result::NEED_DATA;
	}
}

TIMER_CALLBACK_MEMBER(sti3400_device::decode_tick)
{
	if (m_task_active && execute_task())
		m_task_active = false;

	update_status();
	activate_event();
}

void sti3400_device::vblank_w(int state)
{
	if (!state)
		return;

	// DFP is double-buffered by the hardware and becomes active at VSYNC.  The
	// physical pipeline can display a buffer while reconstruction finishes;
	// retain the preceding complete picture until the whole-picture backend
	// finishes an active task targeting the newly selected buffer.
	m_display_pointer = m_registers[REG_DFP] & PICTURE_POINTER_MASK;
	if (!m_task_active || (m_display_pointer != m_reconstructed_pointer))
	{
		m_overwrite_display_active = false;
		m_presented_pointer = m_display_pointer;
	}
	m_width = m_decoded_width;
	m_height = m_decoded_height;

	// A VSYNC-generated DSYNC starts the next task and restarts automatic start-code detection.
	if (m_repeat_pending)
	{
		m_repeat_pending = false;
	}
	else if ((m_registers[REG_CTL] & CTL_EDC) && !(m_registers[REG_CTL] & CTL_DVS) &&
		!(m_registers[REG_INS] & INS_WAIT) && m_event_active && !m_task_active)
	{
		m_reconstructed_pointer = m_registers[REG_RFP] & PICTURE_POINTER_MASK;
		m_forward_pointer = m_registers[REG_FFP] & PICTURE_POINTER_MASK;
		m_backward_pointer = m_registers[REG_BFP] & PICTURE_POINTER_MASK;
		if ((m_registers[REG_INS] & INS_OVW) && (m_reconstructed_pointer == m_display_pointer))
		{
			// The hardware keeps reconstruction behind the display scan in overwrite mode.
			// Preserve the current field because MAME renders it as a single operation.
			std::copy_n(m_dram.get(), m_dram_size, m_overwrite_display.get());
			m_overwrite_display_active = true;
		}
		m_task_active = true;
		m_repeat_pending = bool(m_registers[REG_INS] & INS_RPT);
		finish_event();
		// PSD is a three-clock status pulse at DSYNC.  As primary-clock timing is
		// not modelled, latch its interrupt-visible rising edge directly.
		m_interrupt_status |= STA_PSD;
		update_irq();
		m_decode_timer->adjust(attotime::zero);
	}
}

void sti3400_device::stream_byte_w(u8 data)
{
	if (m_input_bytes == COMPRESSED_DATA_BUFFER_BYTES)
	{
		logerror("%s: compressed-video input overflow\n", machine().describe_context());
		return;
	}
	m_input[m_input_bytes++] = data;

	m_fifo[m_fifo_write & (COMPRESSED_DATA_BUFFER_BYTES - 1)] = data;
	m_fifo_write++;
	m_start_code_shift = (m_start_code_shift << 8) | data;

	if ((m_start_code_shift & MPEG_START_CODE_MASK) == MPEG_START_CODE_PREFIX)
	{
		const u8 code = m_start_code_shift;

		// In MPEG mode the detector recognises every start code except slice codes 01-AF.
		if ((code == MPEG_PICTURE_START_CODE) || (code >= MPEG_NON_SLICE_CODE_MIN))
		{
			LOGMASKED(LOG_START_CODES, "start code %02x at %08x\n", code, u32(m_fifo_write - 1));
			queue_start_code(m_fifo_write - 1, code);
		}
	}

	update_status();
	activate_event();

	// Resume a parked picture task at the modelled picture cadence once CDF input returns.
	if (m_task_active && !m_decode_timer->enabled())
		m_decode_timer->adjust(attotime::from_hz(m_frame_rate));
}

bool sti3400_device::video_valid() const
{
	const u32 picture = m_presented_pointer & ((m_dram_size / BIT_BUFFER_LEVEL_UNIT_BYTES) - 1);
	return m_picture_valid[picture];
}

u16 sti3400_device::video_width() const
{
	return m_width;
}

u16 sti3400_device::video_height() const
{
	return m_height;
}

void sti3400_device::video_line(u32 y, u32 x, u32 width, u32 *destination) const
{
	assert(video_valid());
	assert(y < m_height);
	assert(x <= m_width);
	assert(width <= (m_width - x));

	const u32 mask = m_dram_size - 1;
	const u32 base = (u32(m_presented_pointer) * BIT_BUFFER_LEVEL_UNIT_BYTES) & mask;
	const u32 luma_pitch = (m_width + 15) & ~15;
	const u32 luma_rows = (m_height + 15) & ~15;
	const u32 luma_bytes = luma_pitch * luma_rows;
	const u32 chroma_pitch = luma_pitch / 2;
	const u32 chroma_bytes = chroma_pitch * luma_rows / 2;
	const u32 luma = base + y * luma_pitch + x;
	const u32 cb_plane = base + luma_bytes + (y / 2) * chroma_pitch;
	const u32 cr_plane = cb_plane + chroma_bytes;
	const u8 *const picture = m_overwrite_display_active ? m_overwrite_display.get() : m_dram.get();

	for (u32 column = 0; column != width; column++)
	{
		const int luminance = picture[(luma + column) & mask];
		const u32 chroma_x = (x + column) / 2;
		const int cb = picture[(cb_plane + chroma_x) & mask] - 128;
		const int cr = picture[(cr_plane + chroma_x) & mask] - 128;
		const int scaled_luminance = 298 * (luminance - 16);
		const u8 red = rgb_t::clamp((scaled_luminance + 409 * cr + 128) >> 8);
		const u8 green = rgb_t::clamp((scaled_luminance - 100 * cb - 208 * cr + 128) >> 8);
		const u8 blue = rgb_t::clamp((scaled_luminance + 516 * cb + 128) >> 8);
		destination[column] = rgb_t(red, green, blue);
	}
}

void sti3400_device::queue_start_code(u64 position, u8 code)
{
	if (m_event_count == START_CODE_EVENT_COUNT)
	{
		logerror("%s: start-code event queue overflow\n", machine().describe_context());
		return;
	}

	m_event_position[m_event_tail] = position;
	m_event_code[m_event_tail] = code;
	m_event_tail = (m_event_tail + 1) & (START_CODE_EVENT_COUNT - 1);
	m_event_count++;
	activate_event();
}

void sti3400_device::activate_event()
{
	if (!m_event_active && m_event_count)
	{
		const u64 position = m_event_position[m_event_head];
		const u8 code = m_event_code[m_event_head];
		if (code == MPEG_SEQUENCE_END_CODE)
		{
			if (m_decode_position <= position)
				return;
		}
		else if ((m_fifo_write - position) < HEADER_FIFO_BYTES)
		{
			// The software detector runs on CDF writes; defer the hit until HDF can supply a hardware-sized window.
			return;
		}

		m_event_active = true;
		m_fifo_read = position;
		update_status();
	}
}

void sti3400_device::finish_event()
{
	if (m_event_active)
	{
		m_event_head = (m_event_head + 1) & (START_CODE_EVENT_COUNT - 1);
		m_event_count--;
		m_event_active = false;
	}

	update_status();
	activate_event();
}

u16 sti3400_device::bit_buffer_level() const
{
	const u64 bytes_available = (m_fifo_write > m_decode_position) ? (m_fifo_write - m_decode_position) : 0;

	// BBL excludes the first 64 bytes and reports the remainder in 256-byte units.
	return std::min<u64>((bytes_available > BIT_BUFFER_LEVEL_BIAS_BYTES)
		? ((bytes_available - BIT_BUFFER_LEVEL_BIAS_BYTES) / BIT_BUFFER_LEVEL_UNIT_BYTES)
		: 0, BIT_BUFFER_LEVEL_MASK);
}

u16 sti3400_device::decoder_status() const
{
	const u16 level = bit_buffer_level();
	u16 status = 0;

	if (!m_task_active)
		status |= STA_PID;
	if (!level || (level < (m_registers[REG_BBB] & BIT_BUFFER_LEVEL_MASK)))
		status |= STA_BBE;
	if (level > (m_registers[REG_BBT] & BIT_BUFFER_LEVEL_MASK))
		status |= STA_BBF;
	if (!m_event_active || (m_fifo_read >= m_fifo_write))
		status |= STA_HFE;
	if (m_event_active && ((m_fifo_write - m_fifo_read) >= HEADER_FIFO_BYTES))
		status |= STA_HFF;
	if (m_event_active && (m_fifo_read == m_event_position[m_event_head]))
		status |= STA_SCH;

	return status;
}

void sti3400_device::update_status()
{
	const u16 status = decoder_status();
	m_interrupt_status |= status & ~m_status;
	m_status = status;
	update_irq();
}

void sti3400_device::update_irq()
{
	const u16 mask = m_registers[REG_ITM];
	const bool state = bool(m_interrupt_status & mask);
	if (state != m_irq_state)
	{
		m_irq_state = state;
		m_irq_cb(state ? ASSERT_LINE : CLEAR_LINE);
	}
}

u8 sti3400_device::stream_byte(u64 position) const
{
	if (position >= m_fifo_write)
		return 0;

	return m_fifo[position & (COMPRESSED_DATA_BUFFER_BYTES - 1)];
}

u16 sti3400_device::register_r(offs_t offset)
{
	return m_registers[offset];
}

void sti3400_device::register_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_registers[offset]);
}

void sti3400_device::compressed_data_w(offs_t, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_8_15)
		stream_byte_w(data >> 8);
	if (ACCESSING_BITS_0_7)
		stream_byte_w(data);
}

u16 sti3400_device::header_data_r()
{
	const u16 result = (stream_byte(m_fifo_read) << 8) | stream_byte(m_fifo_read + 1);
	if (!machine().side_effects_disabled())
	{
		m_fifo_read = std::min(m_fifo_read + 2, m_fifo_write);
		update_status();
	}
	return result;
}

u16 sti3400_device::header_position_r()
{
	return 0;
}

u16 sti3400_device::status_r()
{
	return m_status;
}

u16 sti3400_device::interrupt_status_r(offs_t, u16 mem_mask)
{
	const u16 result = m_interrupt_status;
	if (!machine().side_effects_disabled())
	{
		if (ACCESSING_BITS_8_15)
			m_interrupt_status &= 0x00ff;
		if (ACCESSING_BITS_0_7)
			m_interrupt_status &= 0xff00;
		update_irq();
	}
	return result;
}

u16 sti3400_device::display_pointer_r()
{
	return m_display_pointer;
}

u16 sti3400_device::reconstructed_pointer_r()
{
	return m_reconstructed_pointer;
}

u16 sti3400_device::forward_pointer_r()
{
	return m_forward_pointer;
}

u16 sti3400_device::backward_pointer_r()
{
	return m_backward_pointer;
}

u16 sti3400_device::bit_buffer_level_r()
{
	return bit_buffer_level();
}

void sti3400_device::control_w(offs_t, u16 data, u16 mem_mask)
{
	const u16 old_data = m_registers[REG_CTL];
	COMBINE_DATA(&m_registers[REG_CTL]);
	if ((old_data & CTL_SRS) && !(m_registers[REG_CTL] & CTL_SRS))
		decoder_soft_reset();
	else
		update_status();
}

void sti3400_device::interrupt_mask_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_registers[REG_ITM]);
	update_irq();
}

void sti3400_device::header_search_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_registers[REG_HDS]);
	finish_event();
}

void sti3400_device::bit_buffer_bottom_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_registers[REG_BBB]);
	update_status();
}

void sti3400_device::bit_buffer_top_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_registers[REG_BBT]);
	update_status();
}

void sti3400_device::map(address_map &map)
{
	map(0x00, 0x7f).rw(FUNC(sti3400_device::register_r), FUNC(sti3400_device::register_w));
	map(0x00, 0x01).w(FUNC(sti3400_device::compressed_data_w));
	map(0x04, 0x05).r(FUNC(sti3400_device::header_data_r));
	map(0x08, 0x09).r(FUNC(sti3400_device::header_position_r));
	map(0x10, 0x11).r(FUNC(sti3400_device::status_r));
	map(0x14, 0x15).w(FUNC(sti3400_device::control_w));
	map(0x1c, 0x1d).w(FUNC(sti3400_device::interrupt_mask_w));
	map(0x20, 0x21).r(FUNC(sti3400_device::interrupt_status_r));
	map(0x24, 0x25).w(FUNC(sti3400_device::header_search_w));
	map(0x2c, 0x2d).w(FUNC(sti3400_device::bit_buffer_bottom_w));
	map(0x30, 0x31).r(FUNC(sti3400_device::display_pointer_r));
	map(0x34, 0x35).r(FUNC(sti3400_device::reconstructed_pointer_r));
	map(0x38, 0x39).r(FUNC(sti3400_device::forward_pointer_r));
	map(0x3c, 0x3d).r(FUNC(sti3400_device::backward_pointer_r));
	map(0x44, 0x45).r(FUNC(sti3400_device::bit_buffer_level_r));
	map(0x64, 0x65).w(FUNC(sti3400_device::bit_buffer_top_w));
}
