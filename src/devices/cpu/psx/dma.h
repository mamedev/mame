// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation DMA emulator
 *
 * Copyright 2003-2011 smf
 *
 */

#pragma once

#ifndef __PSXDMA_H__
#define __PSXDMA_H__


extern const device_type PSX_DMA;

#define MCFG_PSX_DMA_IRQ_HANDLER(_devcb) \
	devcb = &psxdma_device::set_irq_handler(*device, DEVCB_##_devcb);

typedef delegate<void (uint32_t *, uint32_t, int32_t)> psx_dma_read_delegate;
typedef delegate<void (uint32_t *, uint32_t, int32_t)> psx_dma_write_delegate;

struct psx_dma_channel
{
	uint32_t n_base;
	uint32_t n_blockcontrol;
	uint32_t n_channelcontrol;
	emu_timer *timer;
	psx_dma_read_delegate fn_read;
	psx_dma_write_delegate fn_write;
	uint32_t n_ticks;
	uint32_t b_running;
};

class psxdma_device : public device_t
{
public:
	psxdma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<psxdma_device &>(device).m_irq_handler.set_callback(object); }

	void install_read_handler( int n_channel, psx_dma_read_delegate p_fn_dma_read );
	void install_write_handler( int n_channel, psx_dma_read_delegate p_fn_dma_write );

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
