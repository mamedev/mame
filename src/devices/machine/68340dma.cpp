// license:BSD-3-Clause
// copyright-holders:David Haywood
/* 68340 DMA module */

#include "emu.h"
#include "68340.h"


READ32_MEMBER( m68340_cpu_device::m68340_internal_dma_r )
{
	assert(m_m68340DMA);
	//m68340_dma &dma = *m_m68340DMA;

	logerror("%08x m68340_internal_dma_r %08x, (%08x)\n", m_ppc, offset*4,mem_mask);

	return 0x00000000;
}

WRITE32_MEMBER( m68340_cpu_device::m68340_internal_dma_w )
{
	assert(m_m68340DMA);
	//m68340_dma &dma = *m_m68340DMA;

	logerror("%08x m68340_internal_dma_w %08x, %08x (%08x)\n", m_ppc, offset*4,data,mem_mask);
}

void m68340_dma::reset()
{
	module_reset();
}

void m68340_dma::module_reset()
{
}
