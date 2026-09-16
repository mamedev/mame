// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************

    vino.h

    Silicon Graphics VINO (Video-In, No Out) controller emulation

*********************************************************************/

#ifndef MAME_SGI_VINO_H
#define MAME_SGI_VINO_H

#pragma once

#include "bitmap.h"
#include "imagedev/picture.h"
#include "imagedev/avivideo.h"

class vino_device : public device_t
{
public:
	vino_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);

	uint32_t read(offs_t offset, uint32_t mem_mask = ~0);
	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	auto i2c_data_out() { return m_i2c_data_out.bind(); }
	auto i2c_data_in() { return m_i2c_data_in.bind(); }
	auto i2c_stop() { return m_i2c_stop.bind(); }
	auto interrupt_cb() { return m_interrupt_cb.bind(); }

	template <typename T> void set_gio64_space(T &&tag, int space) { m_space.set_tag(std::forward<T>(tag), space); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	enum channel_num_t : uint32_t
	{
		CHAN_A,
		CHAN_B,
		CHAN_COUNT
	};

	enum : u64
	{
		CTRL_MASK                   = 0x7fffffff,

		CTRL_ENDIAN_BIG             = (0 << 0),
		CTRL_ENDIAN_LITTLE          = (1 << 0),

		CTRL_CHA_FIELD_INT_EN       = (1 << 1),
		CTRL_CHA_FIFO_INT_EN        = (1 << 2),
		CTRL_CHA_DESC_INT_EN        = (1 << 3),

		CTRL_CHB_FIELD_INT_EN       = (1 << 4),
		CTRL_CHB_FIFO_INT_EN        = (1 << 5),
		CTRL_CHB_DESC_INT_EN        = (1 << 6),

		CTRL_CHA_DMA_EN             = (1 << 7),
		CTRL_CHA_INTERLEAVE_EN      = (1 << 8),
		CTRL_CHA_SYNC_EN            = (1 << 9),
		CTRL_CHA_SELECT_PHILIPS     = (0 << 10),
		CTRL_CHA_SELECT_D1          = (1 << 10),
		CTRL_CHA_COLOR_SPACE_YUV    = (0 << 11),
		CTRL_CHA_COLOR_SPACE_RGB    = (1 << 11),
		CTRL_CHA_LUMA_ONLY          = (1 << 12),
		CTRL_CHA_DECIMATE_EN        = (1 << 13),
		CTRL_CHA_DECIMATION_SHIFT   = 14,
		CTRL_CHA_DECIMATION_MASK    = 0x7,
		CTRL_CHA_DECIMATE_HORIZ_EN  = (1 << 17),
		CTRL_CHA_DITHER_EN          = (1 << 18),

		CTRL_CHB_DMA_EN             = (1 << 19),
		CTRL_CHB_INTERLEAVE_EN      = (1 << 20),
		CTRL_CHB_SYNC_EN            = (1 << 21),
		CTRL_CHB_SELECT_PHILIPS     = (0 << 22),
		CTRL_CHB_SELECT_D1          = (1 << 22),
		CTRL_CHB_COLOR_SPACE_YUV    = (0 << 23),
		CTRL_CHB_COLOR_SPACE_RGB    = (1 << 23),
		CTRL_CHB_LUMA_ONLY          = (1 << 24),
		CTRL_CHB_DECIMATE_EN        = (1 << 25),
		CTRL_CHB_DECIMATION_SHIFT   = 26,
		CTRL_CHB_DECIMATION_MASK    = 0x7,
		CTRL_CHB_DECIMATE_HORIZ_EN  = (1 << 29),
		CTRL_CHB_DITHER_EN          = (1 << 30),

		ISR_CHA_EOF                 = (1 << 0),
		ISR_CHA_FIFO                = (1 << 1),
		ISR_CHA_DESC                = (1 << 2),
		ISR_CHB_EOF                 = (1 << 3),
		ISR_CHB_FIFO                = (1 << 4),
		ISR_CHB_DESC                = (1 << 5),
		ISR_MASK                    = 0x3f,

		ALPHA_MASK                  = 0xff,

		CLIP_X_SHIFT                = 0,
		CLIP_X_MASK                 = 0x03ff,
		CLIP_YODD_SHIFT             = 10,
		CLIP_YODD_MASK              = 0x01ff,
		CLIP_YEVEN_SHIFT            = 19,
		CLIP_YEVEN_MASK             = 0x01ff,
		CLIP_REG_MASK               = (CLIP_X_MASK << CLIP_X_SHIFT) | (CLIP_YODD_MASK << CLIP_YODD_SHIFT) | (CLIP_YEVEN_MASK << CLIP_YEVEN_SHIFT),

		FRAME_RATE_NTSC             = (0 << 0),
		FRAME_RATE_PAL              = (1 << 0),
		FRAME_RATE_SHIFT            = 1,
		FRAME_RATE_MASK             = 0x0fff,
		FRAME_RATE_REG_MASK         = 0x1fff,

		FIELD_COUNTER_MASK          = 0xffff,
		LINE_SIZE_MASK              = 0x0ff8,
		LINE_COUNTER_MASK           = 0x0ff8,
		PAGE_INDEX_MASK             = 0x0ff8,

		DESC_PTR_MASK               = 0xfffffff0,

		DESC_VALID_BIT              = (1ULL << 32),
		DESC_STOP_BIT               = (1ULL << 31),
		DESC_JUMP_BIT               = (1ULL << 30),
		DESC_DATA_MASK              = 0x00000000ffffffffULL,

		FIFO_MASK                   = 0x03f8,

		I2C_CTRL_IDLE               = (0 << 0),
		I2C_CTRL_BUSY               = (1 << 0),
		I2C_CTRL_FORCE_IDLE         = (0 << 0),
		I2C_BUS_DIR_WRITE           = (0 << 1),
		I2C_BUS_DIR_READ            = (1 << 1),
		I2C_LAST_RELEASE            = (0 << 2),
		I2C_LAST_HOLD               = (1 << 2),
		I2C_XFER_DONE               = (0 << 4),
		I2C_XFER_BUSY               = (1 << 4),
		I2C_ACK_RECEIVED            = (0 << 5),
		I2C_ACK_NOT_RECEIVED        = (1 << 5),
		I2C_BUS_ERROR               = (1 << 7),
		I2C_CTRL_MASK               = 0xb7,

		I2C_DATA_MASK               = 0xff,
	};

	enum pixel_format_t : uint8_t
	{
		FORMAT_RGBA32,
		FORMAT_YUV422,
		FORMAT_RGBA8,
		FORMAT_Y8
	};

	struct channel_t
	{
		// Externally-visible state
		uint32_t m_alpha;
		uint32_t m_clip_start;
		uint32_t m_clip_end;
		uint32_t m_frame_rate;
		uint32_t m_field_counter;
		uint32_t m_line_size;
		uint32_t m_line_counter;
		uint32_t m_page_index;
		uint32_t m_next_desc_ptr;
		uint32_t m_start_desc_ptr;
		uint64_t m_descriptors[4];
		uint32_t m_fifo_threshold;
		uint32_t m_fifo_gio_ptr;
		uint32_t m_fifo_video_ptr;

		// Internal state
		uint64_t m_fifo[128];
		uint32_t m_active_alpha;
		uint32_t m_curr_line;
		uint16_t m_frame_mask_shift;
		uint16_t m_frame_mask_shifter;
		uint32_t m_pixel_size;
		uint64_t m_next_fifo_word;
		uint32_t m_word_pixel_counter;
		uint32_t m_decimation;

		// Kludges for picture input
		uint32_t m_field_width;
		uint32_t m_field_height[2];
		uint32_t m_field_x;
		uint32_t m_field_y;

		// Kludges in order to trigger end-of-field
		int32_t m_pixels_per_even_field;
		int32_t m_pixels_per_odd_field;
		int32_t m_field_pixels_remaining[2];
		bool m_end_of_field;

		emu_timer *m_fetch_timer;
	};

	void do_dma_transfer(int channel);

	//bool decimate(int channel);
	bool is_interleaved(int channel);
	bool is_even_field(int channel);
	void end_of_field(int channel);

	void push_fifo(int channel);

	void count_pixel(int channel);
	void argb_to_yuv(uint32_t argb, int32_t &y, int32_t &u, int32_t &v);
	uint32_t yuv_to_abgr(int channel, int32_t y, int32_t u, int32_t v);
	bool merge_pixel(int channel, int32_t y, int32_t u, int32_t v, pixel_format_t format);
	pixel_format_t get_current_format(int channel);
	void process_pixel(int channel, int32_t y, int32_t u, int32_t v);
	uint32_t linear_rgb(uint32_t a, uint32_t b, float f);
	uint32_t bilinear_pixel(float s, float t);
	void input_pixel(int channel, int32_t &y, int32_t &u, int32_t &v);
	template <int Channel> TIMER_CALLBACK_MEMBER(fetch_pixel);
	attotime calculate_field_rate(int channel);
	attotime calculate_fetch_rate(int channel);

	void shift_dma_descriptors(int channel);
	void load_dma_descriptors(int channel, uint32_t addr);
	void invalidate_dma_descriptors(int channel);

	void load_frame_mask_shifter(int channel);

	bool line_count_w(int channel, uint32_t data);
	void frame_rate_w(int channel, uint32_t data);
	bool page_index_w(int channel, uint32_t data);
	void next_desc_w(int channel, uint32_t data);
	void control_w(uint32_t data);
	void interrupts_w(uint32_t new_int);

	uint32_t m_rev_id;
	uint32_t m_control;
	uint32_t m_int_status;
	uint32_t m_i2c_ctrl;
	uint32_t m_i2c_data;
	channel_t m_channels[2];

	devcb_write8 m_i2c_data_out;
	devcb_read8 m_i2c_data_in;
	devcb_write_line m_i2c_stop;
	devcb_write_line m_interrupt_cb;

	required_device<picture_image_device> m_picture;
	required_device<avivideo_image_device> m_avivideo;
	required_address_space m_space;

	bitmap_argb32 *m_input_bitmap;
};

DECLARE_DEVICE_TYPE(VINO, vino_device)

#endif // MAME_SGI_VINO_H
