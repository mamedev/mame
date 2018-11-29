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
 *   - interrupts
 *   - dma
 *   - timer
 *   - buzzer
 */

#include "emu.h"
#include "mips_rambo.h"

#define LOG_GENERAL (1U << 0)
#define LOG_REG     (1U << 1)

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
	, m_irq_out_state(0)
	, m_buzzer_out_state(0)
{
	(void)m_irq_out_state;
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

	m_buzzer_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mips_rambo_device::buzzer_toggle), this));

}

void mips_rambo_device::device_reset()
{
	// TODO: update interrupts

	m_tcount = machine().time();
}

READ32_MEMBER(mips_rambo_device::tcount_r)
{
	return attotime_to_clocks(machine().time() - m_tcount);
}

WRITE32_MEMBER(mips_rambo_device::tcount_w)
{
	LOGMASKED(LOG_REG, "tcount_w 0x%08x (%s)\n", data, machine().describe_context());

	m_tcount = machine().time();
}

WRITE32_MEMBER(mips_rambo_device::tbreak_w)
{
	LOGMASKED(LOG_REG, "tbreak_w 0x%08x (%s)\n", data, machine().describe_context());
}

WRITE32_MEMBER(mips_rambo_device::control_w)
{
	LOGMASKED(LOG_REG, "control_w 0x%08x (%s)\n", data, machine().describe_context());

	// stop the buzzer
	m_buzzer_timer->enable(false);
	m_buzzer_out_state = 0;
	m_buzzer_out_cb(m_buzzer_out_state);

	// start the buzzer if requested
	if (data & CONTROL_BUZZON)
	{
		attotime const period = attotime::from_ticks(1 << ((data & CONTROL_BUZZMASK) >> 4), 1524_Hz_XTAL);

		m_buzzer_timer->adjust(period, 0, period);
	}
}

template <unsigned Channel> WRITE32_MEMBER(mips_rambo_device::load_address_w)
{
	LOGMASKED(LOG_REG, "load_address_w<%d> 0x%08x (%s)\n", Channel, data, machine().describe_context());

	COMBINE_DATA(&m_channel[Channel].load_address);
}

template <unsigned Channel> WRITE16_MEMBER(mips_rambo_device::fifo_w)
{
	LOGMASKED(LOG_REG, "fifo_w<%d> 0x%04x (%s)\n", Channel, data, machine().describe_context());
}

template <unsigned Channel> WRITE32_MEMBER(mips_rambo_device::mode_w)
{
	LOGMASKED(LOG_REG, "mode_w<%d> 0x%08x (%s)\n", Channel, data, machine().describe_context());

	COMBINE_DATA(&m_channel[Channel].mode);
}

template <unsigned Channel> WRITE16_MEMBER(mips_rambo_device::block_count_w)
{
	LOGMASKED(LOG_REG, "block_count_w<%d> 0x%04x (%s)\n", Channel, data, machine().describe_context());

	COMBINE_DATA(&m_channel[Channel].block_count);
}

TIMER_CALLBACK_MEMBER(mips_rambo_device::buzzer_toggle)
{
	m_buzzer_out_state = !m_buzzer_out_state;
	m_buzzer_out_cb(m_buzzer_out_state);
}

u32 mips_rambo_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// check if dma channel is configured
	u32 const blocks_required = (screen.visible_area().height() * screen.visible_area().width()) >> 9;
	if (!(m_channel[1].mode & MODE_CHANNEL_EN) || !(m_channel[1].mode & MODE_AUTO_RELOAD) || (m_channel[1].block_count != blocks_required))
		return 1;

	u32 address = m_channel[1].load_address;
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
