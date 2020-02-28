// license:BSD-3-Clause
// copyright-holders:hap
/*

Mitel MD4330B / MD4332B LCD Driver

It's a simple shift register CMOS LCD driver.
'30 has 30 segments, '32 has 32. The "C" versions are ceramic chips, the "E" are epoxy.

TODO:
- RST pin (asynchronous on MD4332B)

*/

#include "emu.h"
#include "video/md4330b.h"


DEFINE_DEVICE_TYPE(MD4330B, md4330b_device, "md4330b", "Mitel MD4330B LCD Driver")
DEFINE_DEVICE_TYPE(MD4332B, md4332b_device, "md4332b", "Mitel MD4332B LCD Driver")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

md4330b_device::md4330b_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 qmax) :
	device_t(mconfig, type, tag, owner, clock),
	m_qmax(qmax), m_write_q(*this), m_write_do(*this)
{ }

md4330b_device::md4330b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	md4330b_device(mconfig, MD4330B, tag, owner, clock, 30)
{ }

md4332b_device::md4332b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	md4330b_device(mconfig, MD4332B, tag, owner, clock, 32)
{ }



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void md4330b_device::device_start()
{
	// resolve callbacks
	m_write_q.resolve_safe();
	m_write_do.resolve_safe();

	// zerofill
	m_shift = 0;
	m_clk = 0;
	m_di = 0;
	m_do = 0;
	m_rst = 0;
	m_tc = 0;

	// register for savestates
	save_item(NAME(m_shift));
	save_item(NAME(m_clk));
	save_item(NAME(m_di));
	save_item(NAME(m_do));
	save_item(NAME(m_rst));
	save_item(NAME(m_tc));
}


//-------------------------------------------------
//  handlers
//-------------------------------------------------

void md4330b_device::update_q()
{
	u32 out = m_shift;
	if (m_tc)
		out = ~out;
	out &= (u64(1) << m_qmax) - 1;

	m_write_q(0, out);
}

WRITE_LINE_MEMBER(md4330b_device::clk_w)
{
	state = (state) ? 1 : 0;

	// shift on rising edge
	if (state && !m_clk)
	{
		// DO pin follows carry out
		m_do = (m_rst) ? 0 : BIT(m_shift, m_qmax-1);

		if (m_rst)
			m_shift = 0;
		else
			m_shift = (m_shift << 1) | m_di;

		// output
		update_q();
		m_write_do(m_do);
	}

	m_clk = state;
}
