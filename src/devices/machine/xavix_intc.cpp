// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    XaviX interrupt controller. Combines peripheral IRQ source levels into
    the CPU interrupt line and exposes the source summary and programmable
    IRQ/NMI vector registers used during interrupt entry. Pending sources
    are acknowledged by their owning peripherals.

    TODO:
    - Establish the meaning, if any, of IRQ summary bits 0-1.

***************************************************************************/

#include "emu.h"
#include "xavix_intc.h"

#define LOG_IRQ (1U << 1)
#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIX_INTC, xavix_intc_device, "xavix_intc", "XaviX Interrupt Controller")

xavix_intc_device::xavix_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_INTC, tag, owner, clock)
	, m_irq_cb(*this)
	, m_irq_source(0)
{
}

void xavix_intc_device::map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(xavix_intc_device::vector_enable_r), FUNC(xavix_intc_device::vector_enable_w));
	map(0x01, 0x01).rw(FUNC(xavix_intc_device::nmi_vector_lo_r), FUNC(xavix_intc_device::nmi_vector_lo_w)); // popira needs readback when paused with some serial EEPROM cartridges
	map(0x02, 0x02).rw(FUNC(xavix_intc_device::nmi_vector_hi_r), FUNC(xavix_intc_device::nmi_vector_hi_w));
	map(0x03, 0x03).rw(FUNC(xavix_intc_device::irq_source_r), FUNC(xavix_intc_device::irq_source_w));
	map(0x05, 0x05).rw(FUNC(xavix_intc_device::irq_vector_lo_r), FUNC(xavix_intc_device::irq_vector_lo_w)); // rad_opus needs readback
	map(0x06, 0x06).rw(FUNC(xavix_intc_device::irq_vector_hi_r), FUNC(xavix_intc_device::irq_vector_hi_w));
}

void xavix_intc_device::device_start()
{
	save_item(NAME(m_irq_source));
	save_item(NAME(m_vector_enable));
	save_item(NAME(m_nmi_vector_lo));
	save_item(NAME(m_nmi_vector_hi));
	save_item(NAME(m_irq_vector_lo));
	save_item(NAME(m_irq_vector_hi));
}

void xavix_intc_device::device_reset()
{
	// Source devices retain ownership of their input levels across reset.
	m_vector_enable = 0;
	m_nmi_vector_lo = 0;
	m_nmi_vector_hi = 0;
	m_irq_vector_lo = 0;
	m_irq_vector_hi = 0;
	update_irq_line();
}

void xavix_intc_device::update_irq_line()
{
	m_irq_cb(m_irq_source != 0);
}

u8 xavix_intc_device::irq_source_r()
{
	/* Regular IRQ handlers read this active-high source register before dispatching.
	   rad_mtrk has the broadest observed dispatch, while many handlers give DMA priority.

	   0x80 - Sound tempo-counter IRQ; channel enables and pending flags are in 0x75fe.
	   0x40 - Shared raster-comparator, arena-end and sprite DMA IRQ. Raster status/
	          acknowledge is 0x6ff8 bit 6; arena is 0x6fea bit 7; sprite DMA is 0x6fe0 bit 7.
	   0x20 - DMA completion IRQ.  Main-map DMA is enabled with 0x7980 bit 6 and
	          acknowledged with bit 7.
	   0x10 - General timer terminal-count IRQ, enabled by 0x7c00 bit 6 and acknowledged
	          by writing bit 7 of the timer control register.
	   0x08 - I/O-event IRQ; per-channel enables, status and acknowledgements are bits
	          0-3 of 0x7a80/0x7a81.  rad_mtrk uses channel 3 event cadence and 0x7a00
	          bit 7 to update its wheel accumulator.
	   0x04 - ADC conversion IRQ, enabled and acknowledged through 0x7b81.
	          rad_mtrk checks this source and acknowledges it by reading/writing 0x7b81.
	   0x02, 0x01 - Not identified or observed. */

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_IRQ, "%s: irq_source_r %02x\n", machine().describe_context(), m_irq_source);
	return m_irq_source;
}

void xavix_intc_device::irq_source_w(u8 data)
{
	// rad_mtrk writes zero during startup; no observed code relies on writes changing a pending source.
	LOGMASKED(LOG_IRQ, "%s: irq_source_w %02x\n", machine().describe_context(), data);
}

u8 xavix_intc_device::vector_enable_r()
{
	return m_vector_enable;
}

void xavix_intc_device::vector_enable_w(u8 data)
{
	LOG("%s: vector_enable_w %02x\n", machine().describe_context(), data);
	m_vector_enable = data & 0x01;
}

u8 xavix_intc_device::nmi_vector_lo_r()
{
	return m_nmi_vector_lo;
}

void xavix_intc_device::nmi_vector_lo_w(u8 data)
{
	LOG("%s: nmi_vector_lo_w %02x\n", machine().describe_context(), data);
	m_nmi_vector_lo = data;
}

u8 xavix_intc_device::nmi_vector_hi_r()
{
	return m_nmi_vector_hi;
}

void xavix_intc_device::nmi_vector_hi_w(u8 data)
{
	LOG("%s: nmi_vector_hi_w %02x\n", machine().describe_context(), data);
	m_nmi_vector_hi = data;
}

u8 xavix_intc_device::irq_vector_lo_r()
{
	if (!machine().side_effects_disabled())
		LOG("%s: irq_vector_lo_r\n", machine().describe_context());
	return m_irq_vector_lo;
}

void xavix_intc_device::irq_vector_lo_w(u8 data)
{
	LOG("%s: irq_vector_lo_w %02x\n", machine().describe_context(), data);
	m_irq_vector_lo = data;
}

u8 xavix_intc_device::irq_vector_hi_r()
{
	if (!machine().side_effects_disabled())
		LOG("%s: irq_vector_hi_r\n", machine().describe_context());
	return m_irq_vector_hi;
}

void xavix_intc_device::irq_vector_hi_w(u8 data)
{
	LOG("%s: irq_vector_hi_w %02x\n", machine().describe_context(), data);
	m_irq_vector_hi = data;
}

s16 xavix_intc_device::interrupt_vector_r(int source, int low_byte)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_IRQ, "interrupt_vector_r %d %d\n", source, low_byte);
	if (m_vector_enable == 0)
		return -1;

	if (source == 0) // NMI
	{
		if (low_byte == 0)
			return m_nmi_vector_hi;
		else
			return m_nmi_vector_lo;
	}
	else
	{
		if (low_byte == 0)
			return m_irq_vector_hi;
		else
			return m_irq_vector_lo;
	}
}
