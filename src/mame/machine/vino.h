// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************

    vino.h

    Silicon Graphics VINO (Video-In, No Out) controller emulation

*********************************************************************/

#ifndef MAME_MACHINE_VINO_H
#define MAME_MACHINE_VINO_H

#pragma once

class vino_device : public device_t
{
public:
	vino_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);

	DECLARE_READ32_MEMBER(read);
	DECLARE_WRITE32_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	enum channel_num_t : uint32_t
	{
		CHAN_A,
		CHAN_B,
		CHAN_COUNT
	};

	enum
	{
		CTRL_MASK					= 0x7fffffff,

		CTRL_ENDIAN_BIG				= (0 << 0),
		CTRL_ENDIAN_LITTLE			= (1 << 0),

		CTRL_CHA_FIELD_INT_EN		= (1 << 1),
		CTRL_CHA_FIFO_INT_EN		= (1 << 2),
		CTRL_CHA_DESC_INT_EN		= (1 << 3),

		CTRL_CHB_FIELD_INT_EN		= (1 << 4),
		CTRL_CHB_FIFO_INT_EN		= (1 << 5),
		CTRL_CHB_DESC_INT_EN		= (1 << 6),

		CTRL_CHA_DMA_EN				= (1 << 7),
		CTRL_CHA_INTERLEAVE_EN		= (1 << 8),
		CTRL_CHA_SYNC_EN			= (1 << 9),
		CTRL_CHA_SELECT_PHILIPS		= (0 << 10),
		CTRL_CHA_SELECT_D1			= (1 << 10),
		CTRL_CHA_COLOR_SPACE_YUV	= (0 << 11),
		CTRL_CHA_COLOR_SPACE_RGB	= (1 << 11),
		CTRL_CHA_LUMA_ONLY			= (1 << 12),
		CTRL_CHA_DECIMATE_EN		= (1 << 13),
		CTRL_CHA_DECIMATION_SHIFT	= 14,
		CTRL_CHA_DECIMATION_MASK	= 0x7,
		CTRL_CHA_DECIMATE_HORIZ_EN	= (1 << 17),
		CTRL_CHA_DITHER_EN			= (1 << 18),

		CTRL_CHB_DMA_EN				= (1 << 19),
		CTRL_CHB_INTERLEAVE_EN		= (1 << 20),
		CTRL_CHB_SYNC_EN			= (1 << 21),
		CTRL_CHB_SELECT_PHILIPS		= (0 << 22),
		CTRL_CHB_SELECT_D1			= (1 << 22),
		CTRL_CHB_COLOR_SPACE_YUV	= (0 << 23),
		CTRL_CHB_COLOR_SPACE_RGB	= (1 << 23),
		CTRL_CHB_LUMA_ONLY			= (1 << 24),
		CTRL_CHB_DECIMATE_EN		= (1 << 25),
		CTRL_CHB_DECIMATION_SHIFT	= 26,
		CTRL_CHB_DECIMATION_MASK	= 0x7,
		CTRL_CHB_DECIMATE_HORIZ_EN	= (1 << 29),
		CTRL_CHB_DITHER_EN			= (1 << 30),

		ISR_CHA_EOF					= (1 << 0),
		ISR_CHA_FIFO				= (1 << 1),
		ISR_CHA_DESC				= (1 << 2),
		ISR_CHB_EOF					= (1 << 3),
		ISR_CHB_FIFO				= (1 << 4),
		ISR_CHB_DESC				= (1 << 5),

		ALPHA_MASK					= 0xff,

		CLIP_X_SHIFT				= 0,
		CLIP_X_MASK					= 0x03ff,
		CLIP_YODD_SHIFT				= 10,
		CLIP_YODD_MASK				= 0x01ff,
		CLIP_YEVEN_SHIFT			= 19,
		CLIP_YEVEN_MASK				= 0x01ff,
		CLIP_REG_MASK				= (CLIP_X_MASK << CLIP_X_SHIFT) | (CLIP_YODD_MASK << CLIP_YODD_SHIFT) | (CLIP_YEVEN_MASK << CLIP_YEVEN_SHIFT),

		FRAME_RATE_NTSC				= (0 << 0),
		FRAME_RATE_PAL				= (1 << 0),
		FRAME_RATE_SHIFT			= 1,
		FRAME_RATE_MASK				= 0x0fff,
		FRAME_RATE_REG_MASK			= 0x1fff,

		FIELD_COUNTER_MASK			= 0xffff,
		LINE_SIZE_MASK				= 0x0ff8,
		LINE_COUNTER_MASK			= 0x0ff8,
		PAGE_INDEX_MASK				= 0x0ff8,

		DESC_PTR_MASK				= 0xfffffff0,

		DESC_VALID_BIT				= (1ULL << 32),
		DESC_STOP_BIT				= (1ULL << 31),
		DESC_JUMP_BIT				= (1ULL << 30),
		DESC_DATA_MASK				= 0x00000000ffffffffULL,

		FIFO_MASK					= 0x03f8,

		I2C_CTRL_IDLE				= (0 << 0),
		I2C_CTRL_BUSY				= (1 << 0),
		I2C_CTRL_FORCE_IDLE			= (0 << 0),
		I2C_BUS_DIR_WRITE			= (0 << 1),
		I2C_BUS_DIR_READ			= (1 << 1),
		I2C_LAST_RELEASE			= (0 << 2),
		I2C_LAST_HOLD				= (1 << 2),
		I2C_XFER_DONE				= (0 << 4),
		I2C_XFER_BUSY				= (1 << 4),
		I2C_ACK_RECEIVED			= (0 << 5),
		I2C_ACK_NOT_RECEIVED		= (1 << 5),
		I2C_BUS_ERROR				= (1 << 7),
		I2C_CTRL_MASK				= 0xb7,

		I2C_DATA_MASK				= 0xff
	};

	struct channel_t
	{
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
	};

	uint32_t m_rev_id;
	uint32_t m_control;
	uint32_t m_int_status;
	uint32_t m_i2c_ctrl;
	uint32_t m_i2c_data;
	channel_t m_channels[2];
};

DECLARE_DEVICE_TYPE(VINO, vino_device)

#endif // MAME_MACHINE_VINO_H
