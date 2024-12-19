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
	save_item(NAME(m_mpr));
	save_item(NAME(m_mpd));
	save_item(NAME(m_mad));
	save_item(NAME(m_sgn_mpd));
	save_item(NAME(m_sgn_mpr));
	save_item(NAME(m_multresults));
}

void xavix_math_device::device_reset()
{
	m_mpr = 0;
	m_mpd = 0;
	m_mad = 0;
	m_sgn_mpd = 0;
	m_sgn_mpr = 0;
	std::fill(std::begin(m_multresults), std::end(m_multresults), 0x00);
}

// epo_guru uses this for ground movement in 3d stages (and other places)
uint8_t xavix_math_device::barrel_r(offs_t offset)
{
	uint8_t ret = 0;
	if (offset == 0)
	{
		ret = (m_sgn_mpd ? 0x80 : 0x00) | (m_mpr & 0x40) | (m_mpr & 0x07);
	}
	else
	{
		ret = (m_mpr & 0x40) ?
			m_multresults[1] ^ m_multresults[0] : (m_mpr & 0x08) ?
			m_multresults[1] : m_multresults[0];
	}

	LOG("%s: reading shift results/data %d %02x\n", machine().describe_context(), offset, ret);
	return ret;
}

// epo_guru 3d stages still flicker a lot with this, but it seems unrelated to the calculations here, possibly a raster timing issue
void xavix_math_device::barrel_w(offs_t offset, uint8_t data)
{
	LOG("%s: barrel_w %d %02x\n", machine().describe_context(), offset, data);

	if (offset == 0)
	{
		m_mad = 0;
		m_sgn_mpd = data & 0x80;
		m_sgn_mpr = 0;
		m_mpr = data;
		do_math(true);
	}
	else
	{
		m_mpd = data;
	}
}

uint8_t xavix_math_device::mult_r(offs_t offset)
{
	uint8_t ret = m_multresults[offset];
	LOG("%s: mult_r %d %02x (read result)\n", machine().describe_context(), offset, ret);
	return ret;
}

void xavix_math_device::mult_w(offs_t offset, uint8_t data)
{
	LOG("%s: mult_w %d %02x (write result)\n", machine().describe_context(), offset, data);
	// rad_madf writes here to set the base value which the multiplication result gets added to
	m_multresults[offset] = data;
}

uint8_t xavix_math_device::mult_param_r(offs_t offset)
{
	uint8_t ret = 0;

	if (offset == 0)
	{
		ret = m_mad | (m_sgn_mpd ? 0x02 : 0x00) | m_sgn_mpr;
	}
	else if (offset == 1)
	{
		ret = m_mpd;
	}
	else
	{
		ret = m_mpr;
	}

	LOG("%s: mult_param_r %d %02x (read parameters)\n", machine().describe_context(), offset, ret);
	return ret;
}

void xavix_math_device::do_math(bool mul_shf)
{
	uint16_t result = 0;
	uint8_t mpd = m_mpd;
	uint8_t mulinp;

	if (!mul_shf)
	{
		mulinp = m_mpr;
	}
	else
	{
		mulinp = 1 << (m_mpr & 0x7);
	}

	if (m_sgn_mpd && (mpd & 0x80) && m_sgn_mpr && (mulinp & 0x80))
	{
		result = mpd * mulinp - (mpd << 8) - (mulinp << 8);
	}
	else if (m_sgn_mpd && (mpd & 0x80))
	{
		result = mpd * mulinp - (mulinp << 8);
	}
	else if (m_sgn_mpr && (mulinp & 0x80))
	{
		result = mpd * mulinp - (mpd << 8);
	}
	else
	{
		result = mpd * mulinp;
	}

	if (m_mad)
	{
		uint16_t oldresult = (m_multresults[1] << 8) | m_multresults[0];
		result = oldresult + result;
	}

	m_multresults[1] = (result >> 8) & 0xff;
	m_multresults[0] = result & 0xff;
}

void xavix_math_device::mult_param_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	// there are NOPs after one of the writes, so presumably the operation is write triggerd and not intstant
	// see test code at 0184a4 in monster truck

	// offset0 is control

	// m?-- --Ss
	// m = mode, S = sign for param1, s = sign for param2
	LOG("%s: mult_param_w %d %02x (write param1 and trigger)\n", machine().describe_context(), offset, data);

	if (offset == 0)
	{
		m_mad = data & 0x80;
		// sometimes 0x40 is set along with 0x80, sometimes only 0x80, why? is it important?

		m_sgn_mpd = data & 0x02;
		m_sgn_mpr = data & 0x01;
	}
	else if (offset == 1)
	{
		m_mpd = data;
	}
	else if (offset == 2)
	{
		m_mpr = data;
		do_math(false);
	}
}
