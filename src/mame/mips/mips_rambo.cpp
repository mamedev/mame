// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * DMA/timer ASIC used in MIPS Pizazz architecture systems.
 *
 * Sources:
 *
 *   https://github.com/NetBSD/src/blob/trunk/sys/arch/mipsco/obio/rambo.h
 *
 * TODO
 *   - buzzer
 *   - dma reload
 *   - save state
 */

#include "emu.h"
#include "mips_rambo.h"

#define LOG_GENERAL (1U << 0)
#define LOG_REG     (1U << 1)
#define LOG_DMA     (1U << 2)

//#define VERBOSE (LOG_GENERAL|LOG_REG)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(MIPS_RAMBO, mips_rambo_device, "mips_rambo", "MIPS RAMBO")

mips_rambo_device::mips_rambo_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MIPS_RAMBO, tag, owner, clock)
	, m_ram(*this, finder_base::DUMMY_TAG)
	, m_irq_out_cb(*this)
	, m_parity_out_cb(*this)
	, m_timer_out_cb(*this)
	, m_buzzer_out_cb(*this)
	, m_channel{{ 0,0,0,0,0,0, false, *this, *this }, { 0,0,0,0,0,0, false, *this, *this }}
	, m_buzzer_out_state(0)
{
}

void mips_rambo_device::map(address_map &map)
{
	map(0x000, 0x003).rw(FUNC(mips_rambo_device::load_address_r<0>), FUNC(mips_rambo_device::load_address_w<0>));
	map(0x100, 0x103).r(FUNC(mips_rambo_device::diag_r<0>));
	map(0x202, 0x203).rw(FUNC(mips_rambo_device::fifo_r<0>), FUNC(mips_rambo_device::fifo_w<0>));
	map(0x300, 0x303).rw(FUNC(mips_rambo_device::mode_r<0>), FUNC(mips_rambo_device::mode_w<0>));
	map(0x402, 0x403).rw(FUNC(mips_rambo_device::block_count_r<0>), FUNC(mips_rambo_device::block_count_w<0>));
	map(0x500, 0x503).r(FUNC(mips_rambo_device::current_address_r<0>));

	map(0x600, 0x603).rw(FUNC(mips_rambo_device::load_address_r<1>), FUNC(mips_rambo_device::load_address_w<1>));
	map(0x700, 0x703).r(FUNC(mips_rambo_device::diag_r<1>));
	map(0x802, 0x803).rw(FUNC(mips_rambo_device::fifo_r<1>), FUNC(mips_rambo_device::fifo_w<1>));
	map(0x900, 0x903).rw(FUNC(mips_rambo_device::mode_r<1>), FUNC(mips_rambo_device::mode_w<1>));
	map(0xa02, 0xa03).rw(FUNC(mips_rambo_device::block_count_r<1>), FUNC(mips_rambo_device::block_count_w<1>));
	map(0xb00, 0xb03).r(FUNC(mips_rambo_device::current_address_r<1>));

	map(0xc00, 0xc03).rw(FUNC(mips_rambo_device::tcount_r), FUNC(mips_rambo_device::tcount_w));
	map(0xd00, 0xd03).rw(FUNC(mips_rambo_device::tbreak_r), FUNC(mips_rambo_device::tbreak_w));
	map(0xe00, 0xe03).r(FUNC(mips_rambo_device::error_r));
	map(0xf00, 0xf03).rw(FUNC(mips_rambo_device::control_r), FUNC(mips_rambo_device::control_w));
}

void mips_rambo_device::device_start()
{
	m_irq_out_cb.resolve_safe();
	m_parity_out_cb.resolve_safe();
	m_timer_out_cb.resolve_safe();
	m_buzzer_out_cb.resolve_safe();

	for (dma_t &ch : m_channel)
	{
		ch.read_cb.resolve_safe(0);
		ch.write_cb.resolve_safe();
	}

	m_timer = timer_alloc(FUNC(mips_rambo_device::timer), this);
	m_dma = timer_alloc(FUNC(mips_rambo_device::dma), this);
	m_buzzer = timer_alloc(FUNC(mips_rambo_device::buzzer), this);
}

void mips_rambo_device::device_reset()
{
	m_tcount = machine().time();

	for (dma_t &channel : m_channel)
	{
		if (channel.mode & MODE_DMA_INTR)
			m_irq_out_cb(0);

		channel.load_address = 0;
		channel.diag = 0;
		channel.mode = 0;
		channel.block_count = 0;
		channel.reload_count = 0;
		channel.current_address = 0;
	}

	m_buzzer->enable(false);
	if (m_buzzer_out_state)
	{
		m_buzzer_out_state = 0;
		m_buzzer_out_cb(m_buzzer_out_state);
	}
}

template <unsigned Channel> u16 mips_rambo_device::fifo_r()
{
	if (m_fifo[Channel].empty())
		m_dma->adjust(attotime::zero, Channel);

	return m_fifo[Channel].dequeue();
}

template <unsigned Channel> u32 mips_rambo_device::mode_r()
{
	u32 data = m_channel[Channel].mode | m_fifo[Channel].queue_length();

	if (m_fifo[Channel].full())
		data |= MODE_FIFO_FULL;
	else if (m_fifo[Channel].empty())
		data |= MODE_FIFO_EMPTY;

	return data;
}

u32 mips_rambo_device::tcount_r()
{
	u32 const data = attotime_to_clocks(machine().time() - m_tcount);

	return data;
}

void mips_rambo_device::tcount_w(u32 data)
{
	LOGMASKED(LOG_REG, "tcount_w 0x%08x (%s)\n", data, machine().describe_context());

	m_tcount = machine().time();
}

void mips_rambo_device::tbreak_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_REG, "tbreak_w 0x%08x (%s)\n", data, machine().describe_context());

	COMBINE_DATA(&m_tbreak);

	m_timer->adjust(clocks_to_attotime(m_tbreak) - (machine().time() - m_tcount));
}

void mips_rambo_device::control_w(u32 data)
{
	LOGMASKED(LOG_REG, "control_w 0x%08x (%s)\n", data, machine().describe_context());

	// stop the buzzer
	m_buzzer->enable(false);
	m_buzzer_out_state = 0;
	m_buzzer_out_cb(m_buzzer_out_state);

	// start the buzzer if requested
	if (data & CONTROL_BUZZON)
	{
		attotime const period = attotime::from_ticks(1 << ((data & CONTROL_BUZZMASK) >> 4), 1524_Hz_XTAL);

		m_buzzer->adjust(period, 0, period);
	}
}

template <unsigned Channel> void mips_rambo_device::load_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_REG, "load_address_w<%d> 0x%08x (%s)\n", Channel, data, machine().describe_context());

	COMBINE_DATA(&m_channel[Channel].load_address);
}

template <unsigned Channel> void mips_rambo_device::fifo_w(u16 data)
{
	LOGMASKED(LOG_REG, "fifo_w<%d> 0x%04x (%s)\n", Channel, data, machine().describe_context());

	m_fifo[Channel].enqueue(data);

	if (m_fifo[Channel].full())
		m_dma->adjust(attotime::zero, Channel);
}

template <unsigned Channel> void mips_rambo_device::mode_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_REG, "mode_w<%d> 0x%08x (%s)\n", Channel, data, machine().describe_context());

	dma_t &channel = m_channel[Channel];

	if (data & MODE_CHANNEL_EN)
		channel.current_address = channel.load_address;

	if (data & MODE_FLUSH_FIFO)
		m_fifo[Channel].clear();

	mem_mask &= MODE_WRITE_MASK;
	COMBINE_DATA(&channel.mode);

	// schedule dma transfer
	if (channel.drq_asserted && (channel.block_count || !m_fifo[Channel].empty()))
		m_dma->adjust(attotime::zero, Channel);
}

template <unsigned Channel> void mips_rambo_device::block_count_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOGMASKED(LOG_REG, "block_count_w<%d> 0x%04x (%s)\n", Channel, data, machine().describe_context());

	dma_t &channel = m_channel[Channel];

	COMBINE_DATA(&channel.block_count);
	COMBINE_DATA(&channel.reload_count);

	// FIXME: do this here?
	if (channel.mode & MODE_DMA_INTR)
	{
		channel.mode &= ~MODE_DMA_INTR;
		m_irq_out_cb(0);
	}

	// schedule dma transfer
	if (channel.drq_asserted && (channel.block_count || !m_fifo[Channel].empty()))
		m_dma->adjust(attotime::zero, Channel);
}

TIMER_CALLBACK_MEMBER(mips_rambo_device::timer)
{
	// FIXME: clear timer out line here, or on tcount/tbreak write?
	m_timer_out_cb(ASSERT_LINE);
	m_timer_out_cb(CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(mips_rambo_device::dma)
{
	dma_t &channel = m_channel[param];

	// check channel enabled
	if (!(channel.mode & MODE_CHANNEL_EN))
		return;

	if (channel.mode & MODE_TO_MEMORY)
	{
		// fill fifo from device
		while (channel.drq_asserted && !m_fifo[param].full())
		{
			u16 const data = channel.read_cb();

			m_fifo[param].enqueue(data);
		}

		// empty fifo to memory
		if (m_fifo[param].full() && channel.block_count)
		{
			LOGMASKED(LOG_DMA, "dma transfer to memory 0x%08x\n", channel.current_address);
			while (!m_fifo[param].empty())
			{
				u16 const data = m_fifo[param].dequeue();

				m_ram->write(BYTE4_XOR_BE(channel.current_address++), data >> 8);
				m_ram->write(BYTE4_XOR_BE(channel.current_address++), data);
			}

			channel.block_count--;
		}
	}
	else
	{
		if (m_fifo[param].empty() && channel.block_count)
		{
			LOGMASKED(LOG_DMA, "dma transfer from memory 0x%08x\n", channel.current_address);

			// fill fifo from memory
			while (!m_fifo[param].full())
			{
				u8 const hi = m_ram->read(BYTE4_XOR_BE(channel.current_address++));
				u8 const lo = m_ram->read(BYTE4_XOR_BE(channel.current_address++));

				m_fifo[param].enqueue((hi << 8) | lo);
			}

			channel.block_count--;
		}

		// empty fifo to device
		while (channel.drq_asserted && !m_fifo[param].empty())
		{
			u16 const data = m_fifo[param].dequeue();

			channel.write_cb(data);
		}
	}

	if (m_fifo[param].empty())
	{
		if (channel.block_count == 0)
		{
			LOGMASKED(LOG_DMA, "dma transfer complete\n");

			// trigger interrupt
			if ((channel.mode & MODE_INTR_EN) && !(channel.mode & MODE_DMA_INTR))
			{
				channel.mode |= MODE_DMA_INTR;
				m_irq_out_cb(1);
			}

			// TODO: reload
			if (channel.mode & MODE_AUTO_RELOAD)
				logerror("auto reload not supported\n");
		}
		else if (channel.drq_asserted)
			m_dma->adjust(attotime::zero, param);
	}
}

TIMER_CALLBACK_MEMBER(mips_rambo_device::buzzer)
{
	m_buzzer_out_state = !m_buzzer_out_state;
	m_buzzer_out_cb(m_buzzer_out_state);
}

u32 mips_rambo_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	dma_t &channel = m_channel[1];

	// check if dma channel is configured
	u32 const blocks_required = (screen.visible_area().height() * screen.visible_area().width()) >> 9;
	if (!(channel.mode & MODE_CHANNEL_EN) || (channel.reload_count != blocks_required))
		return 1;

	// screen is blanked unless auto reload is enabled
	if (!(channel.mode & MODE_AUTO_RELOAD))
	{
		channel.block_count = 0;

		return 0;
	}
	else
		channel.block_count = channel.reload_count;

	u32 address = channel.load_address;
	for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
		for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x += 8)
		{
			u8 pixel_data = m_ram->read(BYTE4_XOR_BE(address));

			bitmap.pix(y, x + 0) = BIT(pixel_data, 7) ? rgb_t::white() : rgb_t::black();
			bitmap.pix(y, x + 1) = BIT(pixel_data, 6) ? rgb_t::white() : rgb_t::black();
			bitmap.pix(y, x + 2) = BIT(pixel_data, 5) ? rgb_t::white() : rgb_t::black();
			bitmap.pix(y, x + 3) = BIT(pixel_data, 4) ? rgb_t::white() : rgb_t::black();
			bitmap.pix(y, x + 4) = BIT(pixel_data, 3) ? rgb_t::white() : rgb_t::black();
			bitmap.pix(y, x + 5) = BIT(pixel_data, 2) ? rgb_t::white() : rgb_t::black();
			bitmap.pix(y, x + 6) = BIT(pixel_data, 1) ? rgb_t::white() : rgb_t::black();
			bitmap.pix(y, x + 7) = BIT(pixel_data, 0) ? rgb_t::white() : rgb_t::black();

			address++;
		}

	return 0;
}

template WRITE_LINE_MEMBER(mips_rambo_device::drq_w<0>);
template WRITE_LINE_MEMBER(mips_rambo_device::drq_w<1>);

template <unsigned Channel> WRITE_LINE_MEMBER(mips_rambo_device::drq_w)
{
	dma_t &channel = m_channel[Channel];

	channel.drq_asserted = bool(state == ASSERT_LINE);

	// schedule dma transfer
	if (channel.drq_asserted && (channel.block_count || !m_fifo[Channel].empty()))
		m_dma->adjust(attotime::zero, Channel);
}
