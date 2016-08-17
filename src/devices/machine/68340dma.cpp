// license:BSD-3-Clause
// copyright-holders:David Haywood
/* 68340 DMA module */

#include "emu.h"
#include "68340.h"


READ32_MEMBER( m68340cpu_device::m68340_internal_dma_r )
{
	m68340cpu_device *m68k = this;
	m68340_dma* dma = m68k->m68340DMA;
	assert(dma != nullptr);

	if (dma)
	{
		int pc = space.device().safe_pc();
		logerror("%08x m68340_internal_dma_r %08x, (%08x)\n", pc, offset*4,mem_mask);
	}

	return 0x00000000;
}

WRITE32_MEMBER( m68340cpu_device::m68340_internal_dma_w )
{
	m68340cpu_device *m68k = this;
	m68340_dma* dma = m68k->m68340DMA;
	assert(dma != nullptr);

	if (dma)
	{
		int pc = space.device().safe_pc();
		logerror("%08x m68340_internal_dma_w %08x, %08x (%08x)\n", pc, offset*4,data,mem_mask);
	}
}

void m68340_dma::reset(void)
{
}
