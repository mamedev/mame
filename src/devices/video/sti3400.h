// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_VIDEO_STI3400_H
#define MAME_VIDEO_STI3400_H

#pragma once

#include "mpeg_video.h"

class sti3400_device : public device_t
{
public:
	static constexpr feature_type imperfect_features() { return feature::TIMING; }

	sti3400_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0) ATTR_COLD;

	void set_dram_size(u32 bytes) { m_dram_size = bytes; }

	auto irq() { return m_irq_cb.bind(); }

	void map(address_map &map) ATTR_COLD;
	void vblank_w(int state);

	bool video_valid() const;
	u16 video_width() const;
	u16 video_height() const;
	void video_line(u32 y, u32 x, u32 width, u32 *destination) const;

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	void device_post_load() override ATTR_COLD;

private:
	// STi3400 host-interface register addresses from the data sheet.
	static constexpr offs_t REG_CDF = 0x00; // compressed data FIFO
	static constexpr offs_t REG_HDF = 0x02; // header data FIFO
	static constexpr offs_t REG_HDP = 0x04; // header data position
	static constexpr offs_t REG_STA = 0x08; // decoder status
	static constexpr offs_t REG_CTL = 0x0a; // decoder control
	static constexpr offs_t REG_ITM = 0x0e; // interrupt mask
	static constexpr offs_t REG_ITS = 0x10; // interrupt status
	static constexpr offs_t REG_HDS = 0x12; // header data search command
	static constexpr offs_t REG_INS = 0x14; // next decoding instruction
	static constexpr offs_t REG_BBB = 0x16; // bit-buffer bottom threshold
	static constexpr offs_t REG_DFP = 0x18; // displayed picture pointer
	static constexpr offs_t REG_RFP = 0x1a; // reconstructed picture pointer
	static constexpr offs_t REG_FFP = 0x1c; // forward prediction picture pointer
	static constexpr offs_t REG_BFP = 0x1e; // backward prediction picture pointer
	static constexpr offs_t REG_BBL = 0x22; // current bit-buffer level
	static constexpr offs_t REG_BBT = 0x32; // bit-buffer top threshold

	static constexpr u16 STA_SCH = 0x0001; // start-code hit
	static constexpr u16 STA_HFE = 0x0004; // header FIFO empty
	static constexpr u16 STA_BBF = 0x0008; // bit buffer nearly full
	static constexpr u16 STA_BBE = 0x0010; // bit buffer nearly empty
	static constexpr u16 STA_PSD = 0x0040; // pipeline starting to decode
	static constexpr u16 STA_PID = 0x0200; // decoding pipeline idle
	static constexpr u16 STA_HFF = 0x1000; // header FIFO full
	static constexpr u16 STA_RESET = STA_PID | STA_BBE | STA_HFE; // hard-reset status

	static constexpr u16 CTL_EDC = 0x0001; // enable decoding
	static constexpr u16 CTL_SRS = 0x0002; // soft reset
	static constexpr u16 CTL_DVS = 0x0080; // disable VSYNC-triggered task start
	static constexpr u16 INS_RPT = 0x0002; // repeat picture for a second VSYNC period
	static constexpr u16 INS_WAIT = 0x0004; // inhibit DSYNC, decoding and header search
	static constexpr u16 INS_OVW = 0x8000; // reconstruct into the displayed picture buffer

	// Bit-buffer levels and thresholds are 14-bit counts of 256-byte units.
	static constexpr u16 BIT_BUFFER_LEVEL_MASK = 0x3fff;
	static constexpr u32 BIT_BUFFER_LEVEL_UNIT_BYTES = 0x100;
	static constexpr u32 BIT_BUFFER_LEVEL_BIAS_BYTES = 0x40;
	static constexpr u16 PICTURE_POINTER_MASK = 0x3fff;

	// The hardware header FIFO is 256 bits wide.
	static constexpr u32 HEADER_FIFO_BYTES = 0x20;
	// Software buffers cover the maximum amount representable by BBL.
	static constexpr u32 COMPRESSED_DATA_BUFFER_BYTES = 4U * 1024 * 1024;
	// This software queue holds start codes pending host service; 256 covers the
	// observed backlog and permits masked ring wrap.
	static constexpr u32 START_CODE_EVENT_COUNT = 256;
	static_assert(std::has_single_bit(COMPRESSED_DATA_BUFFER_BYTES));
	static_assert(std::has_single_bit(START_CODE_EVENT_COUNT));

	static constexpr u32 MPEG_START_CODE_SHIFT_RESET = ~u32(0);
	static constexpr u32 MPEG_START_CODE_MASK = 0xffffff00U;
	static constexpr u32 MPEG_START_CODE_PREFIX = 0x00000100U;
	static constexpr u8 MPEG_PICTURE_START_CODE = 0x00;
	static constexpr u8 MPEG_NON_SLICE_CODE_MIN = 0xb0;
	static constexpr u8 MPEG_SEQUENCE_END_CODE = 0xb7;
	// Maximum constrained-parameters MPEG-1 dimensions.
	static constexpr u32 MAX_VIDEO_WIDTH = 768;
	static constexpr u32 MAX_VIDEO_HEIGHT = 576;
	// Used until an MPEG sequence header supplies the picture rate.
	static constexpr u32 FALLBACK_FRAME_RATE = 25;

	void reset_decoder();
	void decoder_soft_reset();
	bool execute_task();
	TIMER_CALLBACK_MEMBER(decode_tick);

	void stream_byte_w(u8 data);
	void queue_start_code(u64 position, u8 code);
	void activate_event();
	void finish_event();
	u16 bit_buffer_level() const;
	u16 decoder_status() const;
	void update_status();
	void update_irq();
	u8 stream_byte(u64 position) const;

	u16 register_r(offs_t offset);
	void register_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void compressed_data_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 header_data_r();
	u16 header_position_r();
	u16 status_r();
	u16 interrupt_status_r(offs_t offset, u16 mem_mask = ~0);
	u16 display_pointer_r();
	u16 reconstructed_pointer_r();
	u16 forward_pointer_r();
	u16 backward_pointer_r();
	u16 bit_buffer_level_r();
	void control_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void interrupt_mask_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void header_search_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bit_buffer_bottom_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bit_buffer_top_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	// Configuration and callbacks
	devcb_write_line m_irq_cb;
	u32 m_dram_size = 0;

	// Host interface
	u16 m_registers[0x40];
	u16 m_interrupt_status = 0;
	u16 m_status = 0;
	bool m_irq_state = false;

	// Compressed-data FIFO and start-code queue
	std::unique_ptr<u8[]> m_fifo;
	u64 m_event_position[START_CODE_EVENT_COUNT];
	u8 m_event_code[START_CODE_EVENT_COUNT];
	u64 m_fifo_write = 0;
	u64 m_fifo_read = 0;
	u32 m_start_code_shift = 0;
	u16 m_event_head = 0;
	u16 m_event_tail = 0;
	u16 m_event_count = 0;
	bool m_event_active = false;

	// Decoder input and scheduling
	emu_timer *m_decode_timer = nullptr;
	std::unique_ptr<u8[]> m_input;
	std::unique_ptr<mpeg_video> m_decoder;
	u64 m_decode_position = 0;
	u32 m_input_bytes = 0;
	u32 m_input_bit_position = 0;
	u16 m_decoded_width = 0;
	u16 m_decoded_height = 0;
	double m_frame_rate = FALLBACK_FRAME_RATE;
	bool m_task_active = false;
	bool m_repeat_pending = false;

	// Picture DRAM and video output
	std::unique_ptr<u8[]> m_dram;
	std::unique_ptr<u8[]> m_overwrite_display;
	std::unique_ptr<u8[]> m_picture_valid;
	u16 m_display_pointer = 0;
	u16 m_presented_pointer = 0;
	u16 m_reconstructed_pointer = 0;
	u16 m_forward_pointer = 0;
	u16 m_backward_pointer = 0;
	u16 m_width = 0;
	u16 m_height = 0;
	bool m_overwrite_display_active = false;
};

DECLARE_DEVICE_TYPE(STI3400, sti3400_device)

#endif // MAME_VIDEO_STI3400_H
