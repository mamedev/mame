// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    sh_dma.h

    SH DMA controller

***************************************************************************/

#ifndef MAME_CPU_SH_SH_DMA_H
#define MAME_CPU_SH_SH_DMA_H

#pragma once

// To generalize eventually
class sh7042_device;

class sh_dma_device : public device_t {
public:
	sh_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T> sh_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu)
		: sh_dma_device(mconfig, tag, owner)
	{
		set_info(cpu);
	}

	template<typename T> void set_info(T &&cpu) { m_cpu.set_tag(std::forward<T>(cpu)); }

	u16 addr_r(offs_t offset);
	u8 dmasr_r();
	u8 dmar_r();
	void dmasr_w(u8 data);
	void dmar_w(u8 data);

protected:
	required_device<sh7042_device> m_cpu;

	uint16_t m_addr[8];
	uint8_t m_dmasr, m_dmar;


	virtual void device_start() override;
	virtual void device_reset() override;
};

DECLARE_DEVICE_TYPE(SH_DMA, sh_dma_device)

#endif
