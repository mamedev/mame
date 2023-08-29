// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_CPU_SH_SH7021_H
#define MAME_CPU_SH_SH7021_H

#pragma once

#include "sh2.h"

class sh2a_sh7021_device : public sh2_device
{
public:
	sh2a_sh7021_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void sh7021_map(address_map &map);

	uint32_t dma_sar0_r();
	void dma_sar0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t dma_dar0_r();
	void dma_dar0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t dmaor_r();
	void dmaor_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dma_tcr0_r();
	void dma_tcr0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dma_chcr0_r();
	void dma_chcr0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t sh7021_r(offs_t offset);
	void sh7021_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void sh7021_dma_exec(int ch);

	uint16_t m_sh7021_regs[0x200];
	struct
	{
		uint32_t sar;  /**< Source Address Register */
		uint32_t dar;  /**< Destination Address Register */
		uint16_t tcr;  /**< Transfer Count Register */
		uint16_t chcr; /**< Channel Control Register */
	} m_dma[4];

	uint16_t m_dmaor; /**< DMA Operation Register (status flags) */
};

DECLARE_DEVICE_TYPE(SH2A_SH7021, sh2a_sh7021_device)

#endif // MAME_CPU_SH_SH7021_H
