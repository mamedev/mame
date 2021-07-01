// license:BSD-3-Clause
// copyright-holders:Couriersud
/**********************************************************************

    8 bit latch interface and emulation

    2008/08     couriersud

**********************************************************************/

#include "emu.h"
#include "latch8.h"

void latch8_device::update(uint8_t new_val, uint8_t mask)
{
	const uint8_t old_val = m_value;

	m_value = (m_value & ~mask) | (new_val & mask);

	if (m_has_write)
	{
		uint8_t changed = old_val ^ m_value;
		for (int i = 0; i < 8; i++)
			if (BIT(changed, i) && !m_write_cb[i].isnull())
				m_write_cb[i](BIT(m_value, i));
	}
}

TIMER_CALLBACK_MEMBER( latch8_device::timerproc )
{
	const uint8_t new_val = param & 0xff;
	const uint8_t mask = param >> 8;

	update(new_val, mask);
}

/* ----------------------------------------------------------------------- */

uint8_t latch8_device::read(offs_t offset)
{
	assert(offset == 0);

	uint8_t res = m_value;
	if (m_has_read)
	{
		for (int i = 0; i < 8; i++)
		{
			if (!m_read_cb[i].isnull())
				res = (res & ~(1 << i)) | (m_read_cb[i]() << i);
		}
	}
	return (res & ~m_maskout) ^ m_xorvalue;
}


void latch8_device::write(offs_t offset, uint8_t data)
{
	assert(offset == 0);

	if (m_nosync != 0xff)
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(latch8_device::timerproc),this), (0xff << 8) | data);
	else
		update(data, 0xff);
}


void latch8_device::reset_w(offs_t offset, uint8_t data)
{
	assert(offset == 0);

	m_value = 0;
}

// write bit x from data into bit determined by offset
// latch = (latch & ~(1<<offset)) | (((data >> x) & 0x01) << offset)

template <int Bit> inline void latch8_device::bitx_w(offs_t offset, uint8_t data)
{
	const uint8_t mask = 1 << offset;
	const uint8_t masked_data = BIT(data, Bit) << offset;

	assert(offset < 8);

	/* No need to synchronize ? */
	if (m_nosync & mask)
		update(masked_data, mask);
	else
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(latch8_device::timerproc),this), (mask << 8) | masked_data);
}

void latch8_device::bit0_w(offs_t offset, uint8_t data) { bitx_w<0>(offset, data); }
void latch8_device::bit1_w(offs_t offset, uint8_t data) { bitx_w<1>(offset, data); }
void latch8_device::bit2_w(offs_t offset, uint8_t data) { bitx_w<2>(offset, data); }
void latch8_device::bit3_w(offs_t offset, uint8_t data) { bitx_w<3>(offset, data); }
void latch8_device::bit4_w(offs_t offset, uint8_t data) { bitx_w<4>(offset, data); }
void latch8_device::bit5_w(offs_t offset, uint8_t data) { bitx_w<5>(offset, data); }
void latch8_device::bit6_w(offs_t offset, uint8_t data) { bitx_w<6>(offset, data); }
void latch8_device::bit7_w(offs_t offset, uint8_t data) { bitx_w<7>(offset, data); }

DEFINE_DEVICE_TYPE(LATCH8, latch8_device, "latch8", "8-bit latch")

latch8_device::latch8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, LATCH8, tag, owner, clock)
	, m_write_cb(*this)
	, m_read_cb(*this)
	, m_value(0)
	, m_has_write(false)
	, m_has_read(false)
	, m_maskout(0)
	, m_xorvalue(0)
	, m_nosync(0)
{
}


//-------------------------------------------------
//  device_validity_check - validate device
//  configuration
//-------------------------------------------------

void latch8_device::device_validity_check(validity_checker &valid) const
{
	for (int i = 0; i < 8; i++)
		if (!m_read_cb[i].isnull() && !m_write_cb[i].isnull())
			osd_printf_error("Device %s: Bit %d already has a handler.\n", tag(), i);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void latch8_device::device_start()
{
	// setup nodemap
	for (auto &cb : m_write_cb)
	{
		if (!cb.isnull())
			m_has_write = true;
		cb.resolve();
	}

	// setup device read handlers
	for (auto &cb : m_read_cb)
	{
		if (!cb.isnull())
			m_has_read = true;
		cb.resolve();
	}

	save_item(NAME(m_value));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void latch8_device::device_reset()
{
	m_value = 0;
}
