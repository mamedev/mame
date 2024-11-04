// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 EE DMA controller device skeleton
*
*   To Do:
*     Everything
*
*/

#ifndef MAME_MACHINE_PS2DMA_H
#define MAME_MACHINE_PS2DMA_H

#pragma once

#include "cpu/mips/ps2vu.h"
#include "video/ps2gs.h"
#include "ps2sif.h"

class ps2_dmac_device : public device_t, public device_execute_interface
{
public:
	template <typename T, typename U, typename V, typename W, typename X>
	ps2_dmac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&ee_tag, U &&ram_tag, V &&sif_tag, W &&gs_tag, X &&vu1_tag)
		: ps2_dmac_device(mconfig, tag, owner, clock)
	{
		m_ee.set_tag(std::forward<T>(ee_tag));
		m_ram.set_tag(std::forward<U>(ram_tag));
		m_sif.set_tag(std::forward<V>(sif_tag));
		m_gs.set_tag(std::forward<W>(gs_tag));
		m_vu1.set_tag(std::forward<X>(vu1_tag));
	}

	ps2_dmac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~ps2_dmac_device() override;

	enum channel_type : uint32_t
	{
		VIF0 = 0,
		VIF1,
		GIF,
		IPU_OUT,
		IPU_IN,
		SIF0,
		SIF1,
		SIF2,
		SPR_OUT,
		SPR_IN
	};

	uint32_t read(offs_t offset, uint32_t mem_mask = ~0);
	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t channel_r(offs_t offset, uint32_t mem_mask = ~0);
	void channel_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t disable_mask_r(offs_t offset, uint32_t mem_mask = ~0);
	void disable_mask_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void execute_run() override;

	enum tag_id
	{
		ID_REFE = 0,
		ID_CNT,
		ID_NEXT,
		ID_REF,
		ID_REFS,
		ID_CALL,
		ID_RET,
		ID_END
	};

	class channel_t
	{
		friend class ps2_dmac_device;

	public:
		channel_t()
			: m_chcr(0), m_mode(0), m_enabled(false), m_end_tag(false), m_end_irq(false), m_ienable(false), m_addr(0), m_qwc(0)
		{
		}

		void set_chcr(uint32_t data);
		uint32_t chcr() const { return m_chcr; }
		uint32_t mode() const { return m_mode; }
		bool enabled() const { return m_enabled; }
		bool irq_enable() const { return m_ienable; }

		bool end_tag() const { return m_end_tag; }
		void set_end_tag(bool end_tag) { m_end_tag = end_tag; }

		bool end_irq() const { return m_end_irq; }
		void set_end_irq(bool end_irq) { m_end_irq = end_irq; }

		uint32_t addr() const { return m_addr; }
		void set_addr(uint32_t addr) { m_addr = addr; }

		uint32_t quadword_count() const { return m_qwc; }
		void set_quadword_count(uint32_t qwc) { m_qwc = qwc; }

		uint32_t tag_addr() const { return m_tag_addr; }
		void set_tag_addr(uint32_t tag_addr) { m_tag_addr = tag_addr; }

	protected:
		uint32_t m_chcr;
		uint32_t m_mode;
		bool m_enabled;
		bool m_end_tag;
		bool m_end_irq;
		bool m_ienable;

		uint32_t m_addr;
		uint32_t m_qwc;
		uint32_t m_tag_addr;
	};

	void transfer_vif1();
	void transfer_gif();
	void transfer_sif0();
	void transfer_sif1();
	void transfer_finish(uint32_t chan);
	void follow_source_tag(uint32_t channel);
	void update_interrupts();

	required_device<cpu_device> m_ee;
	required_shared_ptr<uint64_t> m_ram;
	required_device<ps2_sif_device> m_sif;
	required_device<ps2_gs_device> m_gs;
	required_device<sonyvu1_device> m_vu1;

	int m_icount;

	uint32_t m_ctrl;
	uint32_t m_mem_drain;
	bool m_enabled;

	uint32_t m_disable_mask;

	uint32_t m_istat;
	uint32_t m_imask;

	uint32_t m_last_serviced;
	channel_t m_channels[10];
};

DECLARE_DEVICE_TYPE(SONYPS2_DMAC, ps2_dmac_device)

#endif // MAME_MACHINE_PS2DMA_H
