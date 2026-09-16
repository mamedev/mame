// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MIPS_MIPS_RAMBO_H
#define MAME_MIPS_MIPS_RAMBO_H

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

	template <unsigned Channel> auto dma_r() { return m_channel[Channel].read_cb.bind(); }
	template <unsigned Channel> auto dma_w() { return m_channel[Channel].write_cb.bind(); }

	// input lines
	template <unsigned Channel> void drq_w(int state);

	void map(address_map &map) ATTR_COLD;
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	template <unsigned Channel> u32 load_address_r() { return m_channel[Channel].load_address; }
	template <unsigned Channel> u32 diag_r() { return 0; }
	template <unsigned Channel> u16 fifo_r();
	template <unsigned Channel> u32 mode_r();
	template <unsigned Channel> u16 block_count_r()
	{
		if ((Channel == 0) || !(m_channel[Channel].mode & MODE_CHANNEL_EN))
			return m_channel[Channel].block_count;

		/*
		 * HACK: The RISC/os boot sequence tests the dma channel 1 block count
		 * when configuring the monochrome graphics option. This hack simulates
		 * decrementing the block count for the video dma channel assuming a
		 * transfer of 1152x900 1-bit pixels at 72 Hz. With a device clock of
		 * 6.25 MHz, this is one block (64 bytes) approximately every 43 device
		 * cycles.
		 */
		u64 const block_cycles = attotime_to_clocks(machine().time()) / 43;

		return m_channel[Channel].block_count - (block_cycles % (m_channel[Channel].block_count + 1));
	}
	template <unsigned Channel> u32 current_address_r() { return m_channel[Channel].current_address; }

	u32 tcount_r();
	u32 tbreak_r() { return m_tbreak; }
	u32 error_r() { return 0; }
	u32 control_r() { return 0; }

	template <unsigned Channel> void load_address_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <unsigned Channel> void fifo_w(u16 data);
	template <unsigned Channel> void mode_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <unsigned Channel> void block_count_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void tcount_w(u32 data);
	void tbreak_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void control_w(u32 data);

	TIMER_CALLBACK_MEMBER(timer);
	TIMER_CALLBACK_MEMBER(dma);
	TIMER_CALLBACK_MEMBER(buzzer);

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
		MODE_COUNT_MASK  = 0x000000ff, // fifo queue length

		MODE_WRITE_MASK  = 0xff000000,
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
		u16 reload_count;
		u32 current_address;

		bool drq_asserted;

		devcb_read16 read_cb;
		devcb_write16 write_cb;
	}
	m_channel[2];

	// FIXME: move this into dma_t
	util::fifo<u16, 32> m_fifo[2];

	emu_timer *m_timer;
	emu_timer *m_dma;
	emu_timer *m_buzzer;

	int m_buzzer_out_state;

	attotime m_tcount;
	u32 m_tbreak;
};

DECLARE_DEVICE_TYPE(MIPS_RAMBO, mips_rambo_device)

#endif // MAME_MIPS_MIPS_RAMBO_H
