// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_SINCLAIR_SPECNEXT_DMA_H
#define MAME_SINCLAIR_SPECNEXT_DMA_H

#pragma once

#include "machine/z80dma.h"

class specnext_dma_device : public z80dma_device
{
public:
	specnext_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void dma_mode_w(bool dma_mode) { m_dma_mode = dma_mode; }

	virtual void write(u8 data) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual int is_ready() override;
	virtual void do_write() override;

private:
	bool m_dma_mode; // 0 = zxn dma, 1 = z80 dma
};

DECLARE_DEVICE_TYPE(SPECNEXT_DMA, specnext_dma_device)

#endif // MAME_SINCLAIR_SPECNEXT_DMA_H
