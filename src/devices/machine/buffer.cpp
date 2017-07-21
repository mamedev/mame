// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "buffer.h"

DEFINE_DEVICE_TYPE(INPUT_BUFFER, input_buffer_device, "input_buffer", "Input Buffer")

input_buffer_device::input_buffer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, INPUT_BUFFER, tag, owner, clock)
	, m_input_data(0xff)
{
}

void input_buffer_device::device_start()
{
	save_item(NAME(m_input_data));
}
