// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_MIPS_RAMBO_H
#define MAME_MACHINE_MIPS_RAMBO_H

#pragma once

#include "machine/ram.h"
#include "screen.h"

class mips_rambo_device : public device_t
{
public:
	mips_rambo_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	auto irq_out() { return m_irq_out_cb.bind(); }
	auto parity_out() { return m_parity_out_cb.bind(); }
	auto timer_out() { return m_timer_out_cb.bind(); }
	auto buzzer_out() { return m_buzzer_out_cb.bind(); }
	template <typename T> void set_ram(T &&tag) { m_ram.set_tag(std::forward<T>(tag)); }

	// input lines
	template <unsigned Interrupt> DECLARE_WRITE_LINE_MEMBER(irq_w) {}
	template <unsigned Channel> DECLARE_WRITE_LINE_MEMBER(drq_w) {}

	void map(address_map &map);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	template <unsigned Channel> DECLARE_READ32_MEMBER(load_address_r) { return m_channel[Channel].load_address; }
	template <unsigned Channel> DECLARE_READ32_MEMBER(diag_r) { return 0; }
	template <unsigned Channel> DECLARE_READ16_MEMBER(fifo_r) { return 0; }
	template <unsigned Channel> DECLARE_READ32_MEMBER(mode_r) { return m_channel[Channel].mode; }
	template <unsigned Channel> DECLARE_READ16_MEMBER(block_count_r) { return m_channel[Channel].block_count; }
	template <unsigned Channel> DECLARE_READ32_MEMBER(current_address_r) { return 0; }

	DECLARE_READ32_MEMBER(tcount_r);
	DECLARE_READ32_MEMBER(tbreak_r) { return 0; }
	DECLARE_READ32_MEMBER(error_r) { return 0; }
	DECLARE_READ32_MEMBER(control_r) { return 0; }

	template <unsigned Channel> DECLARE_WRITE32_MEMBER(load_address_w);
	template <unsigned Channel> DECLARE_WRITE16_MEMBER(fifo_w);
	template <unsigned Channel> DECLARE_WRITE32_MEMBER(mode_w);
	template <unsigned Channel> DECLARE_WRITE16_MEMBER(block_count_w);

	DECLARE_WRITE32_MEMBER(tcount_w);
	DECLARE_WRITE32_MEMBER(tbreak_w);
	DECLARE_WRITE32_MEMBER(control_w);

	TIMER_CALLBACK_MEMBER(buzzer_toggle);

private:
	enum mode_mask : u32
	{
		// these bits are read/write
		MODE_FLUSH_FIFO  = 0x80000000, // clears fifo
		MODE_CHANNEL_EN  = 0x40000000, // enable dma channel
		MODE_AUTO_RELOAD = 0x20000000, // reload src/dst address
		MODE_INTR_EN     = 0x10000000, // enable dma interrupt
		MODE_TO_MEMORY   = 0x08000000, // direction (0=>scsi write)
		MODE_CLR_DMA_ERR = 0x04000000, // clear dma error

		// these bits are read only
		MODE_FIFO_FULL   = 0x00000800, // fifo full state
		MODE_FIFO_EMPTY  = 0x00000400, // fifo empty state
		MODE_DMA_ERROR   = 0x00000200, // parity error during transfer
		MODE_DMA_INTR    = 0x00000100, // channel interrupt pending
		MODE_COUNT_MASK  = 0x000000ff, // halfword count bits
	};

	enum control_mask : u32
	{
		CONTROL_CLRERRINT    = 0x01, // clear error interrupt
		CONTROL_RESETERROR   = 0x02, // reset error latch of EReg
		CONTROL_ENABLEPARITY = 0x04, // enable parity checking
		CONTROL_BUZZON       = 0x08, // buzzer enable
		CONTROL_BUZZMASK     = 0x70, // buzzer divider (1 == 1524 Hz)
	};

	required_device<ram_device> m_ram;

	devcb_write_line m_irq_out_cb;
	devcb_write_line m_parity_out_cb;
	devcb_write_line m_timer_out_cb;
	devcb_write_line m_buzzer_out_cb;

	struct dma_t
	{
		u32 load_address;
		u32 diag;
		u32 mode;
		u16 block_count;
		u32 current_address;
	}
	m_channel[2];

	emu_timer *m_buzzer_timer;

	int m_irq_out_state;
	int m_buzzer_out_state;

	attotime m_tcount;
};

DECLARE_DEVICE_TYPE(MIPS_RAMBO, mips_rambo_device)

#endif // MAME_MACHINE_MIPS_RAMBO_H
