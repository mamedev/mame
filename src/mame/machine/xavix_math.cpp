// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "xavix_math.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIX_MATH, xavix_math_device, "xavix_math", "XaviX Math Unit")

xavix_math_device::xavix_math_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XAVIX_MATH, tag, owner, clock)
{
}

void xavix_math_device::device_start()
{
	save_item(NAME(m_barrel_params));
	save_item(NAME(m_multparams));
	save_item(NAME(m_multresults));
}

void xavix_math_device::device_reset()
{
	m_barrel_params[0] = 0x00;
	m_barrel_params[1] = 0x00;

	std::fill(std::begin(m_multparams), std::end(m_multparams), 0x00);
	std::fill(std::begin(m_multresults), std::end(m_multresults), 0x00);
}


// epo_guru uses this for ground movement in 3d stages (and other places)
uint8_t xavix_math_device::barrel_r(offs_t offset)
{
	if (offset == 0)
	{
		// or upper bits of result?
		LOG("%s: reading shift trigger?!\n", machine().describe_context());
		return 0x00;
	}
	else
	{
		uint8_t retdata = m_barrel_params[1];
		LOG("%s: reading shift results/data %02x\n", machine().describe_context(), retdata);
		return retdata;
	}
}

// epo_guru 3d stages still flicker a lot with this, but it seems unrelated to the calculations here, possibly a raster timing issue
// the pickup animations however don't seem to play, which indicates this could still be wrong.
void xavix_math_device::barrel_w(offs_t offset, uint8_t data)
{
	LOG("%s: barrel_w %02x\n", machine().describe_context(), data);

	m_barrel_params[offset] = data;

	// offset 0 = trigger
	if (offset == 0)
	{
		int shift_data = m_barrel_params[1];
		int shift_amount = data & 0x0f;
		int shift_param = (data & 0xf0) >> 4;

		if (shift_param == 0x00) // just a shift? (definitely right for 'hammer throw'
		{
			// used in epo_guru for 'hammer throw', 'pre-title screen', 'mini game select', '3d chase game (floor scroll, pickups, misc)', 'toilet roll mini game (when you make an error)'

			if (shift_amount & 0x08)
			{
				m_barrel_params[1] = (shift_data >> (shift_amount & 0x7));
			}
			else
			{
				m_barrel_params[1] = (shift_data << (shift_amount & 0x7));
			}
		}
		else if (shift_param == 0x8) // rotate? (should it rotate through a carry bit of some kind?)
		{
			// used in epo_guru for '3d chase game' (unsure of actual purpose in it)
			switch (shift_amount)
			{
			case 0x0: m_barrel_params[1] = shift_data; break;
			case 0x1: m_barrel_params[1] = (shift_data << 1) | ((shift_data >> 7) & 0x01); break;
			case 0x2: m_barrel_params[1] = (shift_data << 2) | ((shift_data >> 6) & 0x03); break;
			case 0x3: m_barrel_params[1] = (shift_data << 3) | ((shift_data >> 5) & 0x07); break;
			case 0x4: m_barrel_params[1] = (shift_data << 4) | ((shift_data >> 4) & 0x0f); break;
			case 0x5: m_barrel_params[1] = (shift_data << 5) | ((shift_data >> 3) & 0x1f); break;
			case 0x6: m_barrel_params[1] = (shift_data << 6) | ((shift_data >> 2) & 0x2f); break;
			case 0x7: m_barrel_params[1] = (shift_data << 7) | ((shift_data >> 1) & 0x3f); break;
			case 0x8: m_barrel_params[1] = shift_data; break;
			case 0x9: m_barrel_params[1] = (shift_data >> 1) | ((shift_data & 0x01) << 7); break;
			case 0xa: m_barrel_params[1] = (shift_data >> 2) | ((shift_data & 0x03) << 6); break;
			case 0xb: m_barrel_params[1] = (shift_data >> 3) | ((shift_data & 0x07) << 5); break;
			case 0xc: m_barrel_params[1] = (shift_data >> 4) | ((shift_data & 0x0f) << 4); break;
			case 0xd: m_barrel_params[1] = (shift_data >> 5) | ((shift_data & 0x1f) << 3); break;
			case 0xe: m_barrel_params[1] = (shift_data >> 6) | ((shift_data & 0x2f) << 2); break;
			case 0xf: m_barrel_params[1] = (shift_data >> 7) | ((shift_data & 0x3f) << 1); break;
			}
		}
	}
}



uint8_t xavix_math_device::mult_r(offs_t offset)
{
	return m_multresults[offset];
}

void xavix_math_device::mult_w(offs_t offset, uint8_t data)
{
	// rad_madf writes here to set the base value which the multiplication result gets added to
	m_multresults[offset] = data;
}

uint8_t xavix_math_device::mult_param_r(offs_t offset)
{
	return m_multparams[offset];
}

void xavix_math_device::mult_param_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_multparams[offset]);
	// there are NOPs after one of the writes, so presumably the operation is write triggerd and not intstant
	// see test code at 0184a4 in monster truck

	// offset0 is control

	// mm-- --Ss
	// mm = mode, S = sign for param1, s = sign for param2
	// modes 00 = multiply (regular?) 11 = add to previous 01 / 10 unknown (maybe subtract?)

	if (offset == 2)
	{
		// assume 0 is upper bits, might be 'mode' instead, check


		int signmode = (m_multparams[0] & 0x3f);

		uint16_t result = 0;

		// rad_madf uses this mode (add to previous result)
		if ((m_multparams[0] & 0xc0) == 0xc0)
		{
			const int param1 = signmode & 0x2 ? (int8_t)m_multparams[1] : (uint8_t)m_multparams[1];
			const int param2 = signmode & 0x1 ? (int8_t)m_multparams[2] : (uint8_t)m_multparams[2];

			result = param1 * param2;

			uint16_t oldresult = (m_multresults[1] << 8) | m_multresults[0];
			result = oldresult + result;
		}
		else if ((m_multparams[0] & 0xc0) == 0x00)
		{
			const int param1 = signmode & 0x2 ? (int8_t)m_multparams[1] : (uint8_t)m_multparams[1];
			const int param2 = signmode & 0x1 ? (int8_t)m_multparams[2] : (uint8_t)m_multparams[2];

			result = param1 * param2;
		}
		else
		{
			popmessage("unknown multiplier mode %02x", m_multparams[0] & 0xc0);
		}

		m_multresults[1] = (result >> 8) & 0xff;
		m_multresults[0] = result & 0xff;
	}
}

