// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation DMA emulator
 *
 * Copyright 2003-2011 smf
 *
 */

#ifndef MAME_CPU_PSX_DMA_H
#define MAME_CPU_PSX_DMA_H

#pragma once


DECLARE_DEVICE_TYPE(PSX_DMA, psxdma_device)

class psxdma_device : public device_t
{
public:
	typedef delegate<void (uint32_t *, uint32_t, int32_t)> read_delegate;
	typedef delegate<void (uint32_t *, uint32_t, int32_t)> write_delegate;

	psxdma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	//configuration helpers
	auto irq() { return m_irq_handler.bind(); }

	void install_read_handler( int n_channel, read_delegate p_fn_dma_read );
	void install_write_handler( int n_channel, write_delegate p_fn_dma_write );

	DECLARE_WRITE32_MEMBER( write );
	DECLARE_READ32_MEMBER( read );

	uint32_t *m_ram;
	size_t m_ramsize;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	struct psx_dma_channel
	{
		uint32_t n_base;
		uint32_t n_blockcontrol;
		uint32_t n_channelcontrol;
		emu_timer *timer;
		read_delegate fn_read;
		write_delegate fn_write;
		uint32_t n_ticks;
		uint32_t b_running;
	};

	void dma_start_timer( int n_channel, uint32_t n_ticks );
	void dma_stop_timer( int n_channel );
	void dma_timer_adjust( int n_channel );
	void dma_interrupt_update();
	void dma_finished( int n_channel );

	psx_dma_channel m_channel[7];
	uint32_t m_dpcp;
	uint32_t m_dicr;

	devcb_write_line m_irq_handler;
};

#endif
