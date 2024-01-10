#ifndef MAME_SINCLAIR_SPECNEXT_DMA_H
#define MAME_SINCLAIR_SPECNEXT_DMA_H

#pragma once

#include "machine/z80dma.h"

class specnext_dma_device : public z80dma_device
{

public:
	specnext_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write(u8 data) override;

protected:
	virtual void device_start() override;

private:
};

DECLARE_DEVICE_TYPE(SPECNEXT_DMA, specnext_dma_device)
#endif // MAME_SINCLAIR_SPECNEXT_DMA_H
