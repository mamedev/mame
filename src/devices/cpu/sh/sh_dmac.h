// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    sh_dmac.h

    SH DMA controller

***************************************************************************/

#ifndef MAME_CPU_SH_SH_DMAC_H
#define MAME_CPU_SH_SH_DMAC_H

#pragma once

// To generalize eventually
class sh7042_device;
class sh_intc_device;

class sh_dmac_device : public device_t {
public:
	sh_dmac_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T> sh_dmac_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu)
		: sh_dmac_device(mconfig, tag, owner)
	{
		set_info(cpu);
	}

	template<typename T> void set_info(T &&cpu) { m_cpu.set_tag(std::forward<T>(cpu)); }

	u16 dmaor_r();
	void dmaor_w(offs_t, u16 data, u16 mem_mask);


protected:
	required_device<sh7042_device> m_cpu;

	u16 m_dmaor;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

class sh_dmac_channel_device : public device_t {
public:
	sh_dmac_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T, typename U> sh_dmac_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu, U &&intc)
		: sh_dmac_channel_device(mconfig, tag, owner)
	{
		set_info(cpu, intc);
	}

	template<typename T, typename U> void set_info(T &&cpu, U &&intc) { m_cpu.set_tag(std::forward<T>(cpu)); m_intc.set_tag(std::forward<U>(intc)); }

	u32 sar_r();
	void sar_w(offs_t, u32 data, u32 mem_mask);
	u32 dar_r();
	void dar_w(offs_t, u32 data, u32 mem_mask);
	u32 dmatcr_r();
	void dmatcr_w(offs_t, u32 data, u32 mem_mask);
	u32 chcr_r();
	void chcr_w(offs_t, u32 data, u32 mem_mask);

protected:
	required_device<sh7042_device> m_cpu;
	required_device<sh_intc_device> m_intc;

	u32 m_sar, m_dar, m_dmatcr, m_chcr;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(SH_DMAC, sh_dmac_device)
DECLARE_DEVICE_TYPE(SH_DMAC_CHANNEL, sh_dmac_channel_device)

#endif
