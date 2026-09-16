// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 IOP DMAC device skeleton
*
*   To Do:
*     Everything
*
*/

#ifndef MAME_MACHINE_IOPDMA_H
#define MAME_MACHINE_IOPDMA_H

#pragma once

#include "ps2sif.h"
#include "iopintc.h"
#include "iopsio2.h"
#include "sound/iopspu.h"

class iop_dma_device : public device_t, public device_execute_interface
{
public:
	template <typename T, typename U, typename V, typename W, typename X>
	iop_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&intc_tag, U &&ram_tag, V &&sif_tag, W &&spu_tag, X &&sio2_tag)
		: iop_dma_device(mconfig, tag, owner, clock)
	{
		m_intc.set_tag(std::forward<T>(intc_tag));
		m_ram.set_tag(std::forward<U>(ram_tag));
		m_sif.set_tag(std::forward<V>(sif_tag));
		m_spu.set_tag(std::forward<W>(spu_tag));
		m_sio2.set_tag(std::forward<X>(sio2_tag));
	}

	iop_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~iop_dma_device() override;

	uint32_t bank0_r(offs_t offset, uint32_t mem_mask = ~0);
	void bank0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t bank1_r(offs_t offset, uint32_t mem_mask = ~0);
	void bank1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	enum channel_type : uint32_t
	{
		MDEC_IN = 0,
		MDEC_OUT,
		GPU,
		CDVD,
		SPU_BANK1,
		PIO,
		OTC,
		UNUSED_BANK0,
		SPU_BANK2,
		UNKNOWN0,
		SIF0,
		SIF1,
		SIO2_IN,
		SIO2_OUT,
		UNKNOWN1,
		UNUSED_BANK1
	};

protected:
	struct intctrl_t
	{
		uint8_t m_mask;
		uint8_t m_status;
		bool m_enabled;
	};

	class channel_t
	{
		friend class iop_dma_device;

	public:
		channel_t()
			: m_priority(0)
			, m_enabled(false)
			, m_busy(false)
			, m_end(false)
			, m_addr(0)
			, m_ctrl(0)
			, m_tag_addr(0)
			, m_block(0)
			, m_block_count(0)
			, m_word_count(0)
			, m_count(0)
		{
		}

		void set_pri_ctrl(uint32_t pri_ctrl);
		void set_addr(uint32_t addr) { m_addr = addr; }
		void set_block(uint32_t block, uint32_t mem_mask);
		void set_block_count(uint32_t block_count);
		void set_word_count(uint32_t word_count);
		void set_count(uint32_t count) { m_count = count; }
		void set_ctrl(uint32_t ctrl);
		void set_tag_addr(uint32_t tag_addr) { m_tag_addr = tag_addr; }

		bool enabled() const { return m_enabled; }
		bool end() const { return m_end; }
		bool busy() const { return m_busy; }

		uint32_t addr() const { return m_addr; }
		uint32_t ctrl() const { return m_ctrl; }
		uint32_t tag_addr() const { return m_tag_addr; }

		uint32_t block() const { return m_block; }
		uint32_t block_count() const { return m_block_count; }
		uint32_t word_count() const { return m_word_count; }
		uint32_t count() const { return m_count; }

	protected:
		uint8_t m_priority;
		bool m_enabled;
		bool m_busy;
		bool m_end;

		uint32_t m_addr;
		uint32_t m_ctrl;
		uint32_t m_tag_addr;

		uint32_t m_block;
		uint32_t m_block_count;
		uint32_t m_word_count;
		uint32_t m_count;
	};

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void execute_run() override;

	void set_dpcr(uint32_t data, uint32_t index);
	void set_dicr(uint32_t data, uint32_t index);
	void update_interrupts();

	void transfer_sif0(uint32_t chan);
	void transfer_sif1(uint32_t chan);
	void transfer_spu(uint32_t chan);
	void transfer_to_sio2(uint32_t chan);
	void transfer_from_sio2(uint32_t chan);
	void transfer_finish(uint32_t chan);

	required_device<iop_intc_device> m_intc;
	required_shared_ptr<uint32_t> m_ram;
	required_device<ps2_sif_device> m_sif;
	required_device<iop_spu_device> m_spu;
	required_device<iop_sio2_device> m_sio2;

	int m_icount;

	uint32_t m_dpcr[2];
	uint32_t m_dicr[2];
	channel_t m_channels[16];
	intctrl_t m_int_ctrl[2];

	uint32_t m_running_mask;
	uint32_t m_last_serviced;
};

DECLARE_DEVICE_TYPE(SONYIOP_DMA, iop_dma_device)

#endif // MAME_MACHINE_IOPDMA_H
