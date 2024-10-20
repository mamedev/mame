// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    SunPlus SPG2xx-series SoC peripheral emulation (System DMA)

**********************************************************************/

#ifndef MAME_MACHINE_SPG2XX_SYSDMA_H
#define MAME_MACHINE_SPG2XX_SYSDMA_H

#pragma once

#include "cpu/unsp/unsp.h"

DECLARE_DEVICE_TYPE(SPG2XX_SYSDMA, spg2xx_sysdma_device)


class spg2xx_sysdma_device : public device_t
{
public:
	template <typename T>
	spg2xx_sysdma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: spg2xx_sysdma_device(mconfig, tag, owner, clock)
	{
		m_cpu.set_tag(std::forward<T>(cpu_tag));
	}

	spg2xx_sysdma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	uint16_t dma_r(offs_t offset);
	void dma_w(offs_t offset, uint16_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void do_cpu_dma(uint32_t len);

	uint16_t m_dma_regs[0x4];

	required_device<unsp_device> m_cpu;
};


#endif // MAME_MACHINE_SPG2XX_SYSDMA_H
