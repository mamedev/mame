// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

	h8500_frt.cpp

	H8/500 family 16-bit free-running timer (FRT).

	TODOs: external clock input, input capture, and the FTOA/FTOB compare outputs.

***************************************************************************/

#include "emu.h"
#include "h8500_frt.h"


DEFINE_DEVICE_TYPE(H8500_FRT, h8500_frt_device, "h8500_frt", "H8/500 16-bit free-running timer channel")

h8500_frt_device::h8500_frt_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: h8_timer16_channel_device(mconfig, H8500_FRT, tag, owner, clock)
	, m_tcsr(0)
	, m_temp(0)
{
}

void h8500_frt_device::device_start()
{
	h8_timer16_channel_device::device_start();

	save_item(NAME(m_tcsr));
	save_item(NAME(m_temp));
}

void h8500_frt_device::device_reset()
{
	h8_timer16_channel_device::device_reset();

	m_tcsr = 0;
	m_tgr[2] = 0;
	m_clock_divider = 2;
}

void h8500_frt_device::tcr_update()
{
	m_ier =
		(m_tcr & 0x10 ? IRQ_V : 0) |
		(m_tcr & 0x20 ? IRQ_A : 0) |
		(m_tcr & 0x40 ? IRQ_B : 0) |
		(m_tcr & 0x80 ? IRQ_C : 0);

	m_clock_type = DIV_1;

	switch (m_tcr & 3)
	{
	case 0: // o/4
		m_clock_divider = 2;
		break;
	case 1: // o/8
		m_clock_divider = 3;
		break;
	case 2: // o/32
		m_clock_divider = 5;
		break;
	case 3: // external clock input, counted on the rising edge
		logerror("unimplemented external clock source selected\n");
		m_clock_type = -1;
		break;
	}
}

// The flag bits (7-4) can be cleared by writing 0 but never set by software.
void h8500_frt_device::isr_update(u8 val)
{
	m_tcsr = val;

	if (val & 0x01)
	{
		m_tgr_clearing = 0;    // CCLRA: clear the FRC on compare-match A
	}
	else
	{
		m_tgr_clearing = TGR_CLEAR_NONE;
	}

	if (!(val & 0x10))
	{
		m_isr &= ~IRQ_V;
	}
	if (!(val & 0x20))
	{
		m_isr &= ~IRQ_A;
	}
	if (!(val & 0x40))
	{
		m_isr &= ~IRQ_B;
	}
	if (!(val & 0x80))
	{
		m_isr &= ~IRQ_C;
	}
}

u8 h8500_frt_device::isr_to_sr() const
{
	return (m_tcsr & 0x0f) |
		(m_isr & IRQ_V ? 0x10 : 0) |
		(m_isr & IRQ_A ? 0x20 : 0) |
		(m_isr & IRQ_B ? 0x40 : 0) |
		(m_isr & IRQ_C ? 0x80 : 0);
}

// 8-bit-bus accessors: a read of the high byte latches the low byte in
// TEMP, a write to the high byte is latched in TEMP and committed when
// the low byte is written (section 10.3)
u8 h8500_frt_device::frc8_r(offs_t offset)
{
	if (!offset)
	{
		const u16 v = tcnt_r();
		if (!machine().side_effects_disabled())
		{
			m_temp = v & 0xff;
		}
		return v >> 8;
	}
	return m_temp;
}

void h8500_frt_device::frc8_w(offs_t offset, u8 data)
{
	if (!offset)
	{
		m_temp = data;
	}
	else
	{
		tcnt_w(0, (u16(m_temp) << 8) | data, 0xffff);
	}
}

void h8500_frt_device::ocra8_w(offs_t offset, u8 data)
{
	if (!offset)
	{
		m_temp = data;
	}
	else
	{
		ocra_w(0, (u16(m_temp) << 8) | data, 0xffff);
	}
}

void h8500_frt_device::ocrb8_w(offs_t offset, u8 data)
{
	if (!offset)
	{
		m_temp = data;
	}
	else
	{
		ocrb_w(0, (u16(m_temp) << 8) | data, 0xffff);
	}
}

u8 h8500_frt_device::icr8_r(offs_t offset)
{
	if (!offset)
	{
		const u16 v = icr_r();
		if (!machine().side_effects_disabled())
		{
			m_temp = v & 0xff;
		}
		return v >> 8;
	}
	return m_temp;
}
