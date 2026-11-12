// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// ARM PrimeCell PL080 DMA controller (ARM DDI 0196G).
// Memory transfers and DMA-controlled peripheral requests, with linked lists.
// Bus arbitration/FIFO timing is approximate.
// TODO: peripheral-controlled lengths, unequal source/destination widths,
// and big-endian AHB masters.

#include "emu.h"
#include "pl080.h"

#include <algorithm>
#include <iterator>

#define LOG_DMA (1U << 1)
#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(PL080, pl080_device, "pl080", "ARM PL080 DMA controller")

pl080_device::pl080_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PL080, tag, owner, clock)
	, m_dma(*this, finder_base::DUMMY_TAG, -1)
	, m_irq(*this)
{
}

void pl080_device::device_start()
{
	m_timer = timer_alloc(FUNC(pl080_device::transfer), this);
	m_requests = 0;
	m_irq_state = false;
	save_item(NAME(m_channels));
	save_item(NAME(m_config));
	save_item(NAME(m_sync));
	save_item(NAME(m_requests));
	save_item(NAME(m_soft));
	save_item(NAME(m_terminal));
	save_item(NAME(m_errors));
	save_item(NAME(m_irq_state));
}

void pl080_device::device_reset()
{
	for (auto &ch : m_channels)
		std::fill(std::begin(ch), std::end(ch), 0);
	std::fill(std::begin(m_soft), std::end(m_soft), 0);
	m_config = m_sync = m_terminal = m_errors = 0;
	m_timer->adjust(attotime::never);
	update_irq();
}

u32 pl080_device::masked_status(bool error) const
{
	u32 mask = 0;
	for (u32 i = 0; i < 8; ++i)
	{
		if (BIT(m_channels[i][4], error ? 14 : 15))
			mask |= 1U << i;
	}
	return (error ? m_errors : m_terminal) & mask;
}

void pl080_device::update_irq()
{
	bool const irq = bool(masked_status(false) | masked_status(true));
	if (m_irq_state != irq)
	{
		m_irq_state = irq;
		m_irq(irq);
	}
}

void pl080_device::map(address_map &map)
{
	map(0x000, 0x003).r(FUNC(pl080_device::status_r));
	map(0x004, 0x007).r(FUNC(pl080_device::terminal_status_r));
	map(0x008, 0x00b).w(FUNC(pl080_device::terminal_clear_w));
	map(0x00c, 0x00f).r(FUNC(pl080_device::error_status_r));
	map(0x010, 0x013).w(FUNC(pl080_device::error_clear_w));
	map(0x014, 0x017).r(FUNC(pl080_device::terminal_raw_r));
	map(0x018, 0x01b).r(FUNC(pl080_device::error_raw_r));
	map(0x01c, 0x01f).r(FUNC(pl080_device::enabled_channels_r));
	map(0x020, 0x02f).rw(FUNC(pl080_device::soft_r), FUNC(pl080_device::soft_w));
	map(0x030, 0x033).rw(FUNC(pl080_device::configuration_r), FUNC(pl080_device::configuration_w));
	map(0x034, 0x037).rw(FUNC(pl080_device::sync_r), FUNC(pl080_device::sync_w));
	for (u32 channel = 0; channel < 8; ++channel)
	{
		map(0x100 + channel * 0x20, 0x113 + channel * 0x20).lrw32(
				[this, channel] (offs_t offset) -> u32 { return m_channels[channel][offset]; }, "channel_r",
				[this, channel] (offs_t offset, u32 data, u32 mem_mask) { channel_w(channel, offset, data, mem_mask); }, "channel_w");
	}
	map(0xfe0, 0xfff).r(FUNC(pl080_device::identification_r));
}

u32 pl080_device::enabled_channels_r()
{
	u32 bits = 0;
	for (u32 i = 0; i < 8; ++i)
		bits |= (m_channels[i][4] & 1) << i;
	return bits;
}

u32 pl080_device::identification_r(offs_t offset)
{
	static constexpr u8 ID[] = { 0x80, 0x10, 0x04, 0x0a, 0x0d, 0xf0, 0x05, 0xb1 };
	return ID[offset];
}

void pl080_device::terminal_clear_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_terminal &= ~(data & mem_mask);
	update_irq();
}

void pl080_device::error_clear_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_errors &= ~(data & mem_mask);
	update_irq();
}

void pl080_device::soft_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_soft[offset] |= data & mem_mask;
	schedule();
}

void pl080_device::configuration_w(offs_t offset, u32 data, u32 mem_mask)
{
	mem_mask &= 7;
	COMBINE_DATA(&m_config);
	schedule();
}

void pl080_device::sync_w(offs_t offset, u32 data, u32 mem_mask)
{
	mem_mask &= 0xffff;
	COMBINE_DATA(&m_sync);
}

void pl080_device::channel_w(u32 channel, offs_t offset, u32 data, u32 mem_mask)
{
	if (offset == 4)
		mem_mask &= 0x5ffff; // Active is read-only; no outstanding FIFO beats.
	COMBINE_DATA(&m_channels[channel][offset]);
	if (offset == 4)
	{
		LOGMASKED(LOG_DMA, "%s: channel %u src=%08x dst=%08x lli=%08x control=%08x config=%08x\n", machine().describe_context(), channel,
				m_channels[channel][0], m_channels[channel][1], m_channels[channel][2], m_channels[channel][3], m_channels[channel][4]);
	}
	update_irq();
	schedule();
}

void pl080_device::set_request(u32 request, int state)
{
	bool const asserted = bool(state);
	if (bool(BIT(m_requests, request)) == asserted)
		return;
	if (asserted)
		m_requests |= 1U << request;
	else
		m_requests &= ~(1U << request);
	schedule();
}

bool pl080_device::ready(u32 ch) const
{
	u32 const cfg = m_channels[ch][4];
	if (!(m_config & 1) || !(cfg & 1) || BIT(cfg, 18))
		return false;
	u32 const flow = (cfg >> 11) & 7;
	u16 const requests = m_requests | m_soft[0] | m_soft[1] | m_soft[2] | m_soft[3];
	return !flow || flow > 3 || (flow == 1 && BIT(requests, (cfg >> 6) & 15)) || (flow == 2 && BIT(requests, (cfg >> 1) & 15)) ||
			(flow == 3 && BIT(requests, (cfg >> 6) & 15) && BIT(requests, (cfg >> 1) & 15));
}

void pl080_device::schedule()
{
	for (u32 i = 0; i < 8; ++i)
	{
		if (ready(i))
		{
			if (!m_timer->enabled() || m_timer->expire().is_never())
				m_timer->adjust(attotime::from_ticks(1, clock()));
			return;
		}
	}
	m_timer->adjust(attotime::never);
}

void pl080_device::complete(u32 ch)
{
	auto &r = m_channels[ch];
	if (BIT(r[3], 31))
		m_terminal |= 1U << ch;
	u32 const next = r[2] & ~3U;
	if (next)
	{
		for (u32 i = 0; i < 4; ++i)
			r[i] = m_dma->read_dword(next + i * 4);
	}
	else
	{
		r[4] &= ~1U;
	}
	update_irq();
}

TIMER_CALLBACK_MEMBER(pl080_device::transfer)
{
	for (u32 ch = 0; ch < 8; ++ch)
	{
		if (!ready(ch))
			continue;
		auto &r = m_channels[ch];
		u32 const sw = (r[3] >> 18) & 7, dw = (r[3] >> 21) & 7, flow = (r[4] >> 11) & 7;
		if (sw > 2 || dw != sw || flow > 3 || (m_config & 6))
		{
			logerror("unsupported channel %u control=%08x config=%08x global=%08x\n", ch, r[3], r[4], m_config);
			m_errors |= 1U << ch;
			r[4] &= ~1U;
			update_irq();
			break;
		}
		if (!(r[3] & 0xfff))
		{
			complete(ch);
			break;
		}
		if (!sw)
			m_dma->write_byte(r[1], m_dma->read_byte(r[0]));
		else if (sw == 1)
			m_dma->write_word(r[1], m_dma->read_word(r[0]));
		else
			m_dma->write_dword(r[1], m_dma->read_dword(r[0]));
		if (BIT(r[3], 26))
			r[0] += 1U << sw;
		if (BIT(r[3], 27))
			r[1] += 1U << dw;
		--r[3];
		for (auto &soft : m_soft)
			soft &= ~((1U << ((r[4] >> 1) & 15)) | (1U << ((r[4] >> 6) & 15)));
		if (!(r[3] & 0xfff))
			complete(ch);
		break; // Fixed channel priority, one AHB beat per timer event.
	}
	schedule();
}
