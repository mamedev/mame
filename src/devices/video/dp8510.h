// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#ifndef MAME_VIDEO_DP8510_H
#define MAME_VIDEO_DP8510_H

#pragma once


class dp8510_device : public device_t
{
public:
	dp8510_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static const char *const BITBLT_OP[];
	static const int FIFO_SIZE = 16;

	void line_drawing(int state) { m_line_drawing = state; }
	void barrel_input_select(int state) { m_barrel_input_select = state; }
	void pixel_data_input(int state) { m_pixel_data_input = state; }
	void data_output_select(int state) { m_data_output_select = state; }
	void left_mask_enable(int state) { m_left_mask = state; }
	void right_mask_enable(int state) { m_right_mask = state; }

	enum control_mask : u16
	{
		CONTROL_RM = 0x000f, // right mask
		CONTROL_LM = 0x00f0, // left mask
		CONTROL_SN = 0x0f00, // shift number
		CONTROL_FS = 0xf000  // function select
	};

	enum control_fs_mask : u16
	{
		FS_0           = 0x1000,
		FS_1           = 0x2000,
		FS_2           = 0x4000,
		FS_3           = 0x8000
	};

	void control_w(const u16 data);

	void source_w(const u16 data, const bool fifo_write = true);
	void destination_w(const u16 data, const bool fifo_write = false);

	u16 output_r() const;
	u8 pixel_r(const offs_t pixel_address) const;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	u16 logic_unit(const u16 src, const u16 dst) const;

	u16 fifo_r();
	void fifo_w(const u16 data);

private:
	// input lines
	int m_line_drawing;
	int m_barrel_input_select;
	int m_pixel_data_input;
	int m_data_output_select;
	int m_left_mask;
	int m_right_mask;

	// registers and latches
	u16 m_control;
	u16 m_barrel_input_latch;
	u16 m_logic_output;
	u16 m_fifo_output_latch;

	// fifo
	u16 m_fifo[FIFO_SIZE];
	u8 m_fifo_head;
	u8 m_fifo_tail;

	// helper internal state
	u16 m_f0;
	u16 m_f1;
	u16 m_f2;
	u16 m_f3;
};

// device type definition
DECLARE_DEVICE_TYPE(DP8510, dp8510_device)

#endif // MAME_VIDEO_DP8510_H
