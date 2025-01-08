// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

  SH7014 Direct Memory Access Controller

***************************************************************************/

#ifndef MAME_CPU_SH_SH7014_DMAC_H
#define MAME_CPU_SH_SH7014_DMAC_H

#pragma once

#include "sh2.h"
#include "sh7014_intc.h"

DECLARE_DEVICE_TYPE(SH7014_DMAC, sh7014_dmac_device)
DECLARE_DEVICE_TYPE(SH7014_DMAC_CHANNEL, sh7014_dmac_channel_device)

class sh7014_dmac_device;

class sh7014_dmac_channel_device : public device_t
{
public:
	enum {
		RS_EXTERNAL_DUAL_ADDR = 0, // External request, dual address mode
		RS_EXTERNAL_SINGLE_ADDR_TO_DEV = 2, // External request, single address mode. External address space -> external device
		RS_EXTERNAL_SINGLE_DEV_TO_ADDR = 3, // External request, single address mode. External device -> external address space
		RS_AUTO_REQUEST = 4,
		RS_MTU_TGI0A = 6,
		RS_MTU_TGI1A = 7,
		RS_MTU_TGI2A = 8,
		RS_AD = 11,
		RS_SCI_TXI0 = 12,
		RS_SCI_RXI0 = 13,
		RS_SCI_TXI1 = 14,
		RS_SCI_RXI1 = 15,
	};

	sh7014_dmac_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<typename T, typename U, typename V> sh7014_dmac_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&dmac, U &&cpu, V &&intc, int chan_id, int vector)
		: sh7014_dmac_channel_device(mconfig, tag, owner, clock)
	{
		m_dmac.set_tag(std::forward<T>(dmac));
		m_cpu.set_tag(std::forward<U>(cpu));
		m_intc.set_tag(std::forward<V>(intc));
		m_channel_id = chan_id;
		m_vector = vector;
	}

	auto notify_dma_source_callback() { return m_notify_dma_source_cb.bind(); }

	void map(address_map &map) ATTR_COLD;

	void dma_check();

	bool is_dma_activated(int vector);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum {
		RS_TYPE_EXTERNAL_DUAL = 0,
		RS_TYPE_EXTERNAL_SINGLE,
		RS_TYPE_AUTO,
		RS_TYPE_INTERNAL,
		RS_TYPE_PROHIBITED,
	};

	enum {
		DMA_ADDR_MODE_FIXED = 0,
		DMA_ADDR_MODE_INC,
		DMA_ADDR_MODE_DEC,
		DMA_ADDR_MODE_PROHIBITED,
	};

	enum {
		CHCR_DE = 1 << 0,
		CHCR_TE = 1 << 1,
		CHCR_IE = 1 << 2,
		CHCR_TM = 1 << 5,
	};

	bool is_enabled();

	uint32_t sar_r();
	void sar_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t dar_r();
	void dar_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t dmatcr_r();
	void dmatcr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t chcr_r();
	void chcr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	TIMER_CALLBACK_MEMBER( dma_timer_callback );

	required_device<sh7014_dmac_device> m_dmac;
	required_device<sh2_device> m_cpu;
	required_device<sh7014_intc_device> m_intc;

	emu_timer *m_dma_current_active_timer;

	devcb_write32 m_notify_dma_source_cb;

	uint32_t m_channel_id;
	uint32_t m_vector;

	uint32_t m_chcr;
	uint32_t m_sar;
	uint32_t m_dar;
	int32_t m_dmatcr;

	bool m_dma_timer_active;
	uint32_t m_active_dma_addr_mode_source;
	uint32_t m_active_dma_addr_mode_dest;
	uint32_t m_active_dma_unit_size;
	bool m_active_dma_is_burst;

	int32_t m_request_source_type, m_selected_resource;
	int32_t m_expected_irq_vector;
};

class sh7014_dmac_device : public device_t
{
public:
	sh7014_dmac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<typename T, typename U> sh7014_dmac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu, U &&intc)
		: sh7014_dmac_device(mconfig, tag, owner, clock)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
		m_intc.set_tag(std::forward<U>(intc));
	}

	template <typename... T> void set_notify_dma_source_callback(T &&... args) {
		m_chan[0].lookup()->notify_dma_source_callback().set(std::forward<T>(args)...);
		m_chan[1].lookup()->notify_dma_source_callback().set(std::forward<T>(args)...);
	}

	void map(address_map &map) ATTR_COLD;

	int is_dma_activated(int vector);
	bool is_transfer_allowed();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	enum {
		DMAOR_DME = 1 << 0,
		DMAOR_NMIF = 1 << 1,
		DMAOR_AE = 1 << 2,
	};

	void dmaor_w(uint16_t data);
	uint16_t dmaor_r();

	required_device<sh2_device> m_cpu;
	required_device<sh7014_intc_device> m_intc;
	required_device_array<sh7014_dmac_channel_device, 2> m_chan;

	uint16_t m_dmaor;
};

#endif // MAME_CPU_SH_SH7014_DMAC_H
