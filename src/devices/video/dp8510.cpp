// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An implementation of the National Semiconductor DP8510 BITBLT Processing Unit.
 *
 * This device is designed to perform shifting, masking and logic operations on
 * source and destination video memory data values. The device relies on
 * external logic to drive the system video memory, and to control input lines
 * on the device to enable it to read or write its inputs and outputs on the
 * data bus. The device has a 16 bit data bus, as well as a 16 slot internal
 * fifo, which enables it to work with up to 16 words containing 16 bits of
 * pixel data at a time. The expected design was that a single BPU would handle
 * each video bitplane, with multiple devices being used in parallel to handle
 * multiple planes.
 *
 * This device is used in the InterPro GT family graphics, which do not make
 * use of the line drawing mode, and is therefore currently unimplemented.
 *
 * Reference: http://bitsavers.org/components/national/_dataBooks/1988_National_Graphics_Handbook.pdf
 *
 * TODO
 *   - line drawing mode
 */

#include "emu.h"
#include "dp8510.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(DP8510, dp8510_device, "dp8510", "National Semiconductor BITBLT Processing Unit")

const char *const dp8510_device::BITBLT_OP[] = {
	"0", "~s & ~d", "~s & d", "~s", "s & ~d", "~d", "s ^ d",  "~s | ~d",
	"s & d", "~s ^ ~d", "d", "~s | d", "s", "s | ~d", "s | d",  "1"
};

dp8510_device::dp8510_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DP8510, tag, owner, clock)
	, m_line_drawing(CLEAR_LINE)
	, m_barrel_input_select(CLEAR_LINE)
	, m_pixel_data_input(CLEAR_LINE)
	, m_data_output_select(CLEAR_LINE)
	, m_left_mask(CLEAR_LINE)
	, m_right_mask(CLEAR_LINE)
{
}

void dp8510_device::device_start()
{
	// input lines
	save_item(NAME(m_line_drawing));
	save_item(NAME(m_barrel_input_select));
	save_item(NAME(m_pixel_data_input));
	save_item(NAME(m_data_output_select));
	save_item(NAME(m_left_mask));
	save_item(NAME(m_right_mask));

	// registers and latches
	save_item(NAME(m_control));
	save_item(NAME(m_barrel_input_latch));
	save_item(NAME(m_logic_output));
	save_item(NAME(m_fifo_output_latch));

	// fifo
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_head));
	save_item(NAME(m_fifo_tail));

	// helper internal state
	save_item(NAME(m_f0));
	save_item(NAME(m_f1));
	save_item(NAME(m_f2));
	save_item(NAME(m_f3));
}

void dp8510_device::device_reset()
{
	m_fifo_head = 0;
	m_fifo_tail = 0;
}

void dp8510_device::control_w(const u16 data)
{
	LOG("control_w function select %d shift number %d left mask %d right mask %d (%s)\n",
		(data & CONTROL_FS) >> 12,
		(data & CONTROL_SN) >> 8,
		(data & CONTROL_LM) >> 4,
		(data & CONTROL_RM) >> 0,
		machine().describe_context());

	m_control = data;

	// expand function select bits to 16 bit size for convenience
	m_f0 = (data & FS_0) ? ~u16(0) : 0;
	m_f1 = (data & FS_1) ? ~u16(0) : 0;
	m_f2 = (data & FS_2) ? ~u16(0) : 0;
	m_f3 = (data & FS_3) ? ~u16(0) : 0;
}

void dp8510_device::source_w(const u16 data, const bool fifo_write)
{
	if (fifo_write)
	{
		// execute barrel shift
		const u8 shift_amount = (m_control & CONTROL_SN) >> 8;
		const u16 shift_result = m_barrel_input_select
			? (data << shift_amount) | (m_barrel_input_latch >> (16 - shift_amount))
			: (m_barrel_input_latch << shift_amount) | (data >> (16 - shift_amount));

		LOG("source_w 0x%04x shifted data 0x%04x written to fifo\n", data, shift_result);

		// write result to fifo
		fifo_w(shift_result);
	}
	else
		LOG("source_w barrel input latch 0x%04x\n", data);

	// save data in input latch
	m_barrel_input_latch = data;
}

u16 dp8510_device::logic_unit(const u16 src, const u16 dst) const
{
	// compute logic unit result
	const u16 result =
		(m_f3 & src & dst) +
		(m_f2 & src & ~dst) +
		(m_f1 & ~src & dst) +
		(m_f0 & ~src & ~dst);

	LOG("logic_unit operation %s source 0x%04x destination 0x%04x result 0x%04x\n",
		BITBLT_OP[(m_control & CONTROL_FS) >> 12], src, dst, result);

	return result;
}

void dp8510_device::destination_w(const u16 data, const bool fifo_write)
{
	// fetch the source from the pixel data input or fifo
	const u16 source = m_line_drawing ? m_pixel_data_input ? 0xffff : 0 : m_fifo_output_latch = fifo_r();

	// compute the result
	u16 result = logic_unit(source, data);

	// apply left mask
	if (m_left_mask)
	{
		const u16 left_mask = 0xffffU >> ((m_control & CONTROL_LM) >> 4);
		if (left_mask != 0xffff)
			LOG("destination_w left mask 0x%04x applied\n", left_mask);

		result = (result & left_mask) | (data & ~left_mask);
	}

	// apply right mask
	if (m_right_mask)
	{
		const u16 right_mask = s16(0x8000) >> (m_control & CONTROL_RM);
		if (right_mask != 0xffff)
			LOG("destination_w right mask 0x%04x applied\n", right_mask);

		result = (result & right_mask) | (data & ~right_mask);
	}

	// latch output
	m_logic_output = result;

	// write the result back to the fifo
	if (fifo_write)
	{
		LOG("destination_w masked data 0x%04x written to fifo\n", m_logic_output);
		fifo_w(m_logic_output);
	}
	else
		LOG("destination_w masked data 0x%04x latched\n", m_logic_output);
}

u16 dp8510_device::output_r() const
{
	// return output from fifo or logic unit
	if (m_data_output_select)
		return m_fifo_output_latch;
	else
		return m_logic_output;
}

u8 dp8510_device::pixel_r(const offs_t pixel_address) const
{
	return (m_logic_output >> (~pixel_address & 0xf)) & 0x1;
}

u16 dp8510_device::fifo_r()
{
	const u16 data = m_fifo[m_fifo_head++];

	m_fifo_head %= FIFO_SIZE;

	return data;
}

void dp8510_device::fifo_w(const u16 data)
{
	m_fifo[m_fifo_tail++] = data;

	m_fifo_tail %= FIFO_SIZE;

	if (m_fifo_tail == m_fifo_head)
		logerror("fifo_w warning fifo full (%s)\n", machine().describe_context());
}
