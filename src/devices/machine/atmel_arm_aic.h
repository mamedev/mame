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
	void set_lines() { m_irq_out((m_core_status & ~m_debug & 2) ? ASSERT_LINE : CLEAR_LINE); } // TODO FIQ

	void push_level(int lvl) { m_level_stack[++m_lvlidx] = lvl; }
	void pop_level() { if (m_lvlidx) --m_lvlidx; }
	int get_level() { return m_level_stack[m_lvlidx]; }

	uint32_t irq_vector_r();
	uint32_t firq_vector_r();
	uint32_t aic_isr_r() { return m_status; }
	uint32_t aic_cisr_r() { return m_core_status; }
	uint32_t aic_ipr_r() { return m_irqs_pending; }
	uint32_t aic_imr_r() { return m_irqs_enabled; }
	uint32_t aic_ffsr_r() { return m_fast_irqs; }

	// can't use ram() and share() in device submaps
	uint32_t aic_smr_r(offs_t offset) { return m_aic_smr[offset]; }
	uint32_t aic_svr_r(offs_t offset) { return m_aic_svr[offset]; }
	void aic_smr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { COMBINE_DATA(&m_aic_smr[offset]); }
	void aic_svr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { COMBINE_DATA(&m_aic_svr[offset]); }
	void aic_spu_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { COMBINE_DATA(&m_spurious_vector); }
	void aic_dcr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { COMBINE_DATA(&m_debug); check_irqs(); }
	void aic_ffer_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { m_fast_irqs |= data & mem_mask; check_irqs(); }
	void aic_ffdr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { m_fast_irqs &= ~(data & mem_mask) | 1; check_irqs(); }

	void aic_iecr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { m_irqs_enabled |= data & mem_mask; check_irqs(); }
	void aic_idcr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { m_irqs_enabled &= ~(data & mem_mask); check_irqs(); }
	void aic_iccr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { m_irqs_pending &= ~(data & mem_mask); check_irqs(); }
	void aic_iscr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { m_irqs_pending |= data & mem_mask; check_irqs(); }
	void aic_eoicr_w(uint32_t data) { m_status = 0; pop_level(); check_irqs(); }
};

#endif
