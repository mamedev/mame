// license:BSD-3-Clause
// copyright-holders:David Haywood, MetalliC

#ifndef MAME_MACHINE_ATMEL_ARM_AIC_H
#define MAME_MACHINE_ATMEL_ARM_AIC_H

#pragma once

DECLARE_DEVICE_TYPE(ARM_AIC, arm_aic_device)

class arm_aic_device : public device_t
{
public:
	// construction/destruction
	arm_aic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, ARM_AIC, tag, owner, clock),
		m_irq_out(*this)
	{
	}

	auto irq_callback() { return m_irq_out.bind(); }

	void regs_map(address_map &map);

	void set_irq(int line, int state);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint32_t m_irqs_enabled;
	uint32_t m_irqs_pending;
	uint32_t m_current_irq_vector;
	uint32_t m_current_firq_vector;
	uint32_t m_status;
	uint32_t m_core_status;
	uint32_t m_spurious_vector;
	uint32_t m_debug;
	uint32_t m_fast_irqs;
	int m_lvlidx;
	int m_level_stack[9];

	uint32_t m_aic_smr[32];
	uint32_t m_aic_svr[32];

	devcb_write_line    m_irq_out;
	void check_irqs();
	void set_lines() { m_irq_out((m_core_status & ~m_debug & 2) ? ASSERT_LINE : CLEAR_LINE); }; // TODO FIQ

	void push_level(int lvl) { m_level_stack[++m_lvlidx] = lvl; };
	void pop_level() { if (m_lvlidx) --m_lvlidx; };
	int get_level() { return m_level_stack[m_lvlidx]; };

	DECLARE_READ32_MEMBER(irq_vector_r);
	DECLARE_READ32_MEMBER(firq_vector_r);
	DECLARE_READ32_MEMBER(aic_isr_r) { return m_status; };
	DECLARE_READ32_MEMBER(aic_cisr_r) { return m_core_status; };
	DECLARE_READ32_MEMBER(aic_ipr_r) { return m_irqs_pending; };
	DECLARE_READ32_MEMBER(aic_imr_r) { return m_irqs_enabled; };
	DECLARE_READ32_MEMBER(aic_ffsr_r) { return m_fast_irqs; };

	// can't use AM_RAM and AM_SHARE in device submaps
	DECLARE_READ32_MEMBER(aic_smr_r) { return m_aic_smr[offset]; };
	DECLARE_READ32_MEMBER(aic_svr_r) { return m_aic_svr[offset]; };
	DECLARE_WRITE32_MEMBER(aic_smr_w) { COMBINE_DATA(&m_aic_smr[offset]); };
	DECLARE_WRITE32_MEMBER(aic_svr_w) { COMBINE_DATA(&m_aic_svr[offset]); };
	DECLARE_WRITE32_MEMBER(aic_spu_w) { COMBINE_DATA(&m_spurious_vector); };
	DECLARE_WRITE32_MEMBER(aic_dcr_w) { COMBINE_DATA(&m_debug); check_irqs(); };
	DECLARE_WRITE32_MEMBER(aic_ffer_w) { m_fast_irqs |= data & mem_mask; check_irqs(); };
	DECLARE_WRITE32_MEMBER(aic_ffdr_w) { m_fast_irqs &= ~(data & mem_mask) | 1; check_irqs(); };

	DECLARE_WRITE32_MEMBER(aic_iecr_w) { m_irqs_enabled |= data & mem_mask; check_irqs(); };
	DECLARE_WRITE32_MEMBER(aic_idcr_w) { m_irqs_enabled &= ~(data & mem_mask); check_irqs(); };
	DECLARE_WRITE32_MEMBER(aic_iccr_w) { m_irqs_pending &= ~(data & mem_mask); check_irqs(); };
	DECLARE_WRITE32_MEMBER(aic_iscr_w) { m_irqs_pending |= data & mem_mask; check_irqs(); };
	DECLARE_WRITE32_MEMBER(aic_eoicr_w) { m_status = 0; pop_level(); check_irqs(); };
};

#endif
