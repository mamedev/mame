// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Seta IOX HLE implementation

TODO:
- hanadojo protection;
- coinage;
- verify if just a MCU everywhere this is used (essentially a sort of I8x42 keyboard wannabe?);
- merge/subclass for the variants;

**************************************************************************************************/

#include "emu.h"
#include "iox_hle.h"

DEFINE_DEVICE_TYPE(IOX_HLE, iox_hle_device, "iox_hle", "Seta IOX HLE")

iox_hle_device::iox_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, IOX_HLE, tag, owner, clock)
	, m_p1_key{{*this, 0xff}, {*this, 0xff}, {*this, 0xff}, {*this, 0xff}}
	, m_p2_key{{*this, 0xff}, {*this, 0xff}, {*this, 0xff}, {*this, 0xff}}
	, m_direct_in{{*this, 0xff}, {*this, 0xff}}
{
}

void iox_hle_device::device_start()
{
	save_item(NAME(m_data));
	save_item(NAME(m_direct_mode));
	save_item(NAME(m_command));
}

void iox_hle_device::device_reset()
{
	m_data = 0;
	m_command = 0;
	m_direct_mode = false;
}

u8 iox_hle_device::get_key_matrix_value(devcb_read8 *key_read, bool is_p2)
{
	// TODO: id, specific to speedatk?
	//u8 p2_flag = is_p2 << 5;

	for (int row = 0; row < 4; row ++)
	{
		for (int t = 0; t < 8; t++)
		{
			if (!(key_read[row]() & ( 1 << t )))
			{
				return ((row << 3) + t); // | p2_side;
			}
		}
	}

	return 0;
}

u8 iox_hle_device::data_r(offs_t offset)
{
	// protection overlay
	// ...

	// coinage
	// ...

	// TODO: non-input commands
	if (m_data != 1 && m_data != 2 && m_data != 4)
		return 0xff;

	// secondary IOX in hanadojo uses normal parsing for DIPB & DIPD banks
	if (m_direct_mode)
	{
		// TODO: to be verified
		if (m_data == 1)
		{
			u8 p1_direct = m_direct_in[0]();
			u8 p2_direct = m_direct_in[1]();
			return p1_direct & p2_direct;
		}

		return m_direct_in[BIT(m_data, 2)]();
	}

	if (m_data == 1)
	{
		u8 p1_side = get_key_matrix_value(m_p1_key, false);
		u8 p2_side = get_key_matrix_value(m_p2_key, true);

		if (p1_side != 0)
			return p1_side;

		return p2_side;
	}

	if (m_data == 2)
		return get_key_matrix_value(m_p1_key, false);

	return get_key_matrix_value(m_p2_key, true);
}

void iox_hle_device::data_w(offs_t offset, u8 data)
{
	logerror("data_w %02x\n", data);
	m_data = data;
}

u8 iox_hle_device::status_r(offs_t offset)
{
	// bit 0: busy flag
	return 0xfe | 1;
}

void iox_hle_device::command_w(offs_t offset, u8 data)
{
	m_command = data;
	logerror("command_w %02x\n", data);

	switch(m_command & 0xf0)
	{
		case 0x50: m_direct_mode = false; break;
		case 0x90: m_direct_mode = true; break;
	}
}
