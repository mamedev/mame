// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    XaviX general-purpose DMA controller. Transfers bytes between configured
    source and destination buses, updates the address and length registers,
    and reports completion through status and an optional interrupt. A pending
    destination write is retained separately from the source-read phase.

    Transfer timing is approximate: each byte uses four peripheral clocks,
    divided between its read and write phases. Bus bursts and contention
    are not modelled.

    TODO:
    - Model bus bursts and contention.

***************************************************************************/

#include "emu.h"
#include "xavix_dma.h"

#include "util/multibyte.h"

#include <algorithm>

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIX_DMA, xavix_dma_device, "xavix_dma", "XaviX DMA Controller")

xavix_dma_device::xavix_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_DMA, tag, owner, clock)
	, m_source_read_cb(*this, 0)
	, m_destination_write_cb(*this)
	, m_irq_cb(*this)
{
}

void xavix_dma_device::map(address_map &map)
{
	map(0x0, 0x0).rw(FUNC(xavix_dma_device::ctrl_r), FUNC(xavix_dma_device::ctrl_w));
	map(0x1, 0x3).rw(FUNC(xavix_dma_device::src_r), FUNC(xavix_dma_device::src_w));
	map(0x4, 0x5).rw(FUNC(xavix_dma_device::dst_r), FUNC(xavix_dma_device::dst_w));
	map(0x6, 0x7).rw(FUNC(xavix_dma_device::len_r), FUNC(xavix_dma_device::len_w));
	map(0x8, 0x8).rw(FUNC(xavix_dma_device::mode_r), FUNC(xavix_dma_device::mode_w));
}

void xavix_dma_device::device_start()
{
	m_transfer_timer = timer_alloc(FUNC(xavix_dma_device::transfer_tick), this);

	save_item(NAME(m_src));
	save_item(NAME(m_dst));
	save_item(NAME(m_len));
	save_item(NAME(m_ctrl));
	save_item(NAME(m_mode));
	save_item(NAME(m_pending_data));
	save_item(NAME(m_byte_pending));
}

void xavix_dma_device::device_reset()
{
	std::ranges::fill(m_src, 0);
	std::ranges::fill(m_dst, 0);
	std::ranges::fill(m_len, 0);
	m_ctrl = 0;
	m_mode = 0;
	m_pending_data = 0;
	m_byte_pending = false;
	m_in_destination_callback = false;
	m_destination_written = false;
	m_transfer_timer->enable(false);
	update_irq();
}

u8 xavix_dma_device::ctrl_r()
{
	if (!machine().side_effects_disabled())
		LOG("%s: dma_ctrl_r %02x\n", machine().describe_context(), m_ctrl);
	return m_ctrl;
}

void xavix_dma_device::ctrl_w(u8 data)
{
	LOG("%s: dma_ctrl_w %02x\n", machine().describe_context(), data);

	if (BIT(data, 7) || !BIT(data, 6))
		m_ctrl &= ~0x80;

	// Interrupt enable is a level control; Run and completion are status bits.
	m_ctrl = (m_ctrl & 0x81) | (data & 0x40);
	update_irq();

	if (!BIT(data, 0))
		return;

	if (get_u16le(m_len.data()))
	{
		m_ctrl |= 0x01;
		schedule_transfer();
	}
	else if (!m_mode)
		complete_transfer(BIT(data, 7));
}

u8 xavix_dma_device::src_r(offs_t offset)
{
	return m_src[offset];
}

void xavix_dma_device::src_w(offs_t offset, u8 data)
{
	LOG("%s: dma_src_w (%02x) %02x\n", machine().describe_context(), offset, data);
	m_src[offset] = data;
}

u8 xavix_dma_device::dst_r(offs_t offset)
{
	return m_dst[offset];
}

void xavix_dma_device::dst_w(offs_t offset, u8 data)
{
	LOG("%s: dma_dst_w (%02x) %02x\n", machine().describe_context(), offset, data);
	m_dst[offset] = data & (offset ? 0x7f : 0xff);
	if (m_in_destination_callback)
		m_destination_written = true;
}

u8 xavix_dma_device::len_r(offs_t offset)
{
	return m_len[offset];
}

void xavix_dma_device::len_w(offs_t offset, u8 data)
{
	LOG("%s: dma_len_w (%02x) %02x\n", machine().describe_context(), offset, data);
	m_len[offset] = data & (offset ? 0x7f : 0xff);
	schedule_transfer();
}

u8 xavix_dma_device::mode_r()
{
	return m_mode;
}

void xavix_dma_device::mode_w(u8 data)
{
	m_mode = data & 1;
}

void xavix_dma_device::schedule_transfer()
{
	if (!m_transfer_timer->enabled() && (m_byte_pending || (BIT(m_ctrl, 0) && get_u16le(m_len.data()))))
		m_transfer_timer->adjust(clocks_to_attotime(2));
}

void xavix_dma_device::complete_transfer(bool acknowledge)
{
	m_ctrl &= ~0x01;
	if (BIT(m_ctrl, 6) && !acknowledge)
		m_ctrl |= 0x80;
	update_irq();
}

void xavix_dma_device::update_irq()
{
	m_irq_cb((m_ctrl & 0xc0) == 0xc0);
}

TIMER_CALLBACK_MEMBER(xavix_dma_device::transfer_tick)
{
	// Approximate timing: retain four peripheral clocks per byte, divided
	// between its read and write. Bus bursts and contention are not modelled.
	if (m_byte_pending)
	{
		u16 const destination = get_u16le(m_dst.data());
		u8 const data = m_pending_data;
		m_byte_pending = false;
		// The destination may be a DMA register. Complete this phase before
		// the callback, giving destination-register writes priority over the
		// counter increment, including writes that leave its value unchanged.
		assert(!m_in_destination_callback);
		m_in_destination_callback = true;
		m_destination_write_cb(destination, data);
		m_in_destination_callback = false;
		if (!m_destination_written)
			put_u16le(m_dst.data(), (destination + 1) & 0x7fff);
		m_destination_written = false;
	}
	else if (BIT(m_ctrl, 0) && get_u16le(m_len.data()))
	{
		u32 const source = get_u24le(m_src.data());
		u16 const remaining = get_u16le(m_len.data()) - 1;
		m_pending_data = m_source_read_cb(source);
		m_byte_pending = true;
		put_u24le(m_src.data(), (source + 1) & 0xffffff);
		put_u16le(m_len.data(), remaining);
		// Completion reports the last source read; its buffered destination
		// write still occurs even if software acknowledges or starts again.
		if (!remaining)
			complete_transfer();
	}
	schedule_transfer();
}
