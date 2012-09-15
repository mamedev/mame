/*
 * PlayStation DMA emulator
 *
 * Copyright 2003-2011 smf
 *
 */

#pragma once

#ifndef __PSXDMA_H__
#define __PSXDMA_H__

#include "emu.h"

extern const device_type PSX_DMA;

typedef delegate<void (UINT32, INT32)> psx_dma_read_delegate;
typedef delegate<void (UINT32, INT32)> psx_dma_write_delegate;

struct psx_dma_channel
{
	UINT32 n_base;
	UINT32 n_blockcontrol;
	UINT32 n_channelcontrol;
	emu_timer *timer;
	psx_dma_read_delegate fn_read;
	psx_dma_write_delegate fn_write;
	UINT32 n_ticks;
	UINT32 b_running;
};

class psxdma_device : public device_t
{
public:
	psxdma_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void install_read_handler( int n_channel, psx_dma_read_delegate p_fn_dma_read );
	void install_write_handler( int n_channel, psx_dma_read_delegate p_fn_dma_write );

	WRITE32_MEMBER( write );
	READ32_MEMBER( read );

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_post_load();

private:
	void dma_start_timer( int n_channel, UINT32 n_ticks );
	void dma_stop_timer( int n_channel );
	void dma_timer_adjust( int n_channel );
	void dma_interrupt_update();
	void dma_finished( int n_channel );
	void dma_finished_callback(void *ptr, int param);
	void write( offs_t offset, UINT32 data, UINT32 mem_mask );
	UINT32 read( offs_t offset, UINT32 mem_mask );

	psx_dma_channel channel[7];
	UINT32 n_dpcp;
	UINT32 n_dicr;
};

#endif
