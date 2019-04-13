// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    SunPlus SPG2xx-series SoC peripheral emulation (System DMA)

**********************************************************************/

#include "emu.h"
#include "spg2xx_sysdma.h"

DEFINE_DEVICE_TYPE(SPG2XX_SYSDMA, spg2xx_sysdma_device, "spg2xx_sysdma", "SPG240-series System-on-a-Chip System DMA")

#define LOG_DMA             (1U << 9)
#define LOG_ALL             (LOG_DMA)

#define VERBOSE             (0)
#include "logmacro.h"



spg2xx_sysdma_device::spg2xx_sysdma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPG2XX_SYSDMA, tag, owner, clock)
	, m_cpu(*this, finder_base::DUMMY_TAG)
{
}

void spg2xx_sysdma_device::device_start()
{
	save_item(NAME(m_dma_regs));
}

void spg2xx_sysdma_device::device_reset()
{
	memset(m_dma_regs, 0, 0x4 * sizeof(uint16_t));
}

/*************************
*    Machine Hardware    *
*************************/

READ16_MEMBER(spg2xx_sysdma_device::dma_r)
{
	offset &= 0x3;

	uint16_t val = m_dma_regs[offset];
	switch (offset)
	{

	case 0x000: // DMA Source (L)
		LOGMASKED(LOG_DMA, "dma_r: DMA Source (lo) = %04x\n", val);
		break;

	case 0x001: // DMA Source (H)
		LOGMASKED(LOG_DMA, "dma_r: DMA Source (hi) = %04x\n", val);
		break;

	case 0x002: // DMA Length
		LOGMASKED(LOG_DMA, "dma_r: DMA Length = %04x\n", 0);
		val = 0;
		break;

	case 0x003: // DMA Destination
		LOGMASKED(LOG_DMA, "dma_r: DMA Dest = %04x\n", val);
		break;
	}

	return val;
}

WRITE16_MEMBER(spg2xx_sysdma_device::dma_w)
{
	offset &= 0x3;

	switch (offset)
	{
	case 0x000: // DMA Source (lo)
		LOGMASKED(LOG_DMA, "dma_w: DMA Source (lo) = %04x\n", data);
		m_dma_regs[offset] = data;
		break;

	case 0x001: // DMA Source (hi)
		LOGMASKED(LOG_DMA, "dma_w: DMA Source (hi) = %04x\n", data);
		m_dma_regs[offset] = data;
		break;

	case 0x002: // DMA Length
		LOGMASKED(LOG_DMA, "dma_w: DMA Length = %04x\n", data);
		if (!(data & 0xc000))  // jak_dora writes 0xffff here which ends up trashing registers etc. why? such writes can't be valid
			do_cpu_dma(data);
		break;

	case 0x003: // DMA Destination
		LOGMASKED(LOG_DMA, "dma_w: DMA Dest = %04x\n", data);
		m_dma_regs[offset] = data;
		break;
	}
}

void spg2xx_sysdma_device::do_cpu_dma(uint32_t len)
{
	address_space &mem = m_cpu->space(AS_PROGRAM);

	uint32_t src = ((m_dma_regs[0x001] & 0x3f) << 16) | m_dma_regs[0x000];
	uint32_t dst = m_dma_regs[0x003] & 0x3fff;

	for (uint32_t j = 0; j < len; j++)
	{
		mem.write_word((dst + j) & 0x3fff, mem.read_word(src + j));
	}

	src += len;
	m_dma_regs[0x000] = (uint16_t)src;
	m_dma_regs[0x001] = (src >> 16) & 0x3f;
	m_dma_regs[0x002] = 0;
	m_dma_regs[0x003] = (dst + len) & 0x3fff;
}
