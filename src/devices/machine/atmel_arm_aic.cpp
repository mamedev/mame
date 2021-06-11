// license:BSD-3-Clause
// copyright-holders:David Haywood, MetalliC

/*
   ARM AIC (Advanced Interrupt Controller) from Atmel
   typically integrated into the AM91SAM series of chips

   current implementation was tested in pgm2 (pgm2.cpp) only, there might be mistakes if more advanced usage.
   if this peripheral is not available as a standalone chip it could also be moved to
   the CPU folder alongside the ARM instead
   TODO:
    low/high input source types
    check if level/edge input source types logic correct
    FIQ output
*/

#include "emu.h"
#include "atmel_arm_aic.h"

DEFINE_DEVICE_TYPE(ARM_AIC, arm_aic_device, "arm_aic", "ARM Advanced Interrupt Controller")


void arm_aic_device::regs_map(address_map &map)
{
	map(0x000, 0x07f).rw(FUNC(arm_aic_device::aic_smr_r), FUNC(arm_aic_device::aic_smr_w)); // AIC_SMR[32] (AIC_SMR)  Source Mode Register
	map(0x080, 0x0ff).rw(FUNC(arm_aic_device::aic_svr_r), FUNC(arm_aic_device::aic_svr_w)); // AIC_SVR[32] (AIC_SVR)  Source Vector Register
	map(0x100, 0x103).r(FUNC(arm_aic_device::irq_vector_r));      // AIC_IVR    IRQ Vector Register
	map(0x104, 0x107).r(FUNC(arm_aic_device::firq_vector_r));     // AIC_FVR    FIQ Vector Register
	map(0x108, 0x10b).r(FUNC(arm_aic_device::aic_isr_r));         // AIC_ISR Interrupt Status Register
	map(0x10c, 0x10f).r(FUNC(arm_aic_device::aic_ipr_r));         // AIC_IPR Interrupt Pending Register
	map(0x110, 0x113).r(FUNC(arm_aic_device::aic_imr_r));         // AIC_IMR Interrupt Mask Register
	map(0x114, 0x117).r(FUNC(arm_aic_device::aic_cisr_r));        // AIC_CISR    Core Interrupt Status Register
	map(0x120, 0x123).w(FUNC(arm_aic_device::aic_iecr_w));       // AIC_IECR    Interrupt Enable Command Register
	map(0x124, 0x127).w(FUNC(arm_aic_device::aic_idcr_w));       // AIC_IDCR    Interrupt Disable Command Register
	map(0x128, 0x12b).w(FUNC(arm_aic_device::aic_iccr_w));       // AIC_ICCR    Interrupt Clear Command Register
	map(0x12c, 0x12f).w(FUNC(arm_aic_device::aic_iscr_w));       // AIC_ISCR    Interrupt Set Command Register
	map(0x130, 0x133).w(FUNC(arm_aic_device::aic_eoicr_w));      // AIC_EOICR   End of Interrupt Command Register
	map(0x134, 0x137).w(FUNC(arm_aic_device::aic_spu_w));        // AIC_SPU     Spurious Vector Register
	map(0x138, 0x13b).w(FUNC(arm_aic_device::aic_dcr_w));        // AIC_DCR     Debug Control Register (Protect)
	map(0x140, 0x143).w(FUNC(arm_aic_device::aic_ffer_w));       // AIC_FFER    Fast Forcing Enable Register
	map(0x144, 0x147).w(FUNC(arm_aic_device::aic_ffdr_w));       // AIC_FFDR    Fast Forcing Disable Register
	map(0x148, 0x14b).r(FUNC(arm_aic_device::aic_ffsr_r));        // AIC_FFSR    Fast Forcing Status Register
}

uint32_t arm_aic_device::irq_vector_r()
{
	uint32_t mask = m_irqs_enabled & m_irqs_pending & ~m_fast_irqs;
	m_current_irq_vector = m_spurious_vector;
	if (mask)
	{
		// looking for highest level pending interrupt, bigger than current
		int pri = get_level();
		int midx = -1;
		do
		{
			uint8_t idx = 31 - count_leading_zeros_32(mask);
			if ((int)(m_aic_smr[idx] & 7) >= pri)
			{
				midx = idx;
				pri = m_aic_smr[idx] & 7;
			}
			mask ^= 1 << idx;
		} while (mask);

		if (midx > 0)
		{
			m_status = midx;
			m_current_irq_vector = m_aic_svr[midx];
			// note: Debug PROTect mode not implemented (new level pushed on stack and pending line clear only when this register writen after read)
			push_level(m_aic_smr[midx] & 7);
			if (m_aic_smr[midx] & 0x20)         // auto clear pending if edge trigger mode
				m_irqs_pending ^= 1 << midx;
		}
	}

	m_core_status &= ~2;
	set_lines();
	return m_current_irq_vector;
}

uint32_t arm_aic_device::firq_vector_r()
{
	m_current_firq_vector = (m_irqs_enabled & m_irqs_pending & m_fast_irqs) ? m_aic_svr[0] : m_spurious_vector;
	return m_current_firq_vector;
}

void arm_aic_device::device_start()
{
	m_irq_out.resolve_safe();

	save_item(NAME(m_irqs_enabled));
	save_item(NAME(m_irqs_pending));
	save_item(NAME(m_current_irq_vector));
	save_item(NAME(m_current_firq_vector));
	save_item(NAME(m_status));
	save_item(NAME(m_core_status));
	save_item(NAME(m_spurious_vector));
	save_item(NAME(m_debug));
	save_item(NAME(m_fast_irqs));
	save_item(NAME(m_lvlidx));

	save_item(NAME(m_aic_smr));
	save_item(NAME(m_aic_svr));
	save_item(NAME(m_level_stack));
}

void arm_aic_device::device_reset()
{
	m_irqs_enabled = 0;
	m_irqs_pending = 0;
	m_current_irq_vector = 0;
	m_current_firq_vector = 0;
	m_status = 0;
	m_core_status = 0;
	m_spurious_vector = 0;
	m_debug = 0;
	m_fast_irqs = 1;
	m_lvlidx = 0;

	for(auto & elem : m_aic_smr) { elem = 0; }
	for(auto & elem : m_aic_svr) { elem = 0; }
	for(auto & elem : m_level_stack) { elem = 0; }
	m_level_stack[0] = -1;
}

void arm_aic_device::set_irq(int line, int state)
{
	// note: might be incorrect if edge triggered mode set
	if (state == ASSERT_LINE)
		m_irqs_pending |= 1 << line;
	else
		if (m_aic_smr[line] & 0x40)
			m_irqs_pending &= ~(1 << line);

	check_irqs();
}

void arm_aic_device::check_irqs()
{
	m_core_status = 0;

	uint32_t mask = m_irqs_enabled & m_irqs_pending & ~m_fast_irqs;
	if (mask)
	{
		// check if we have pending interrupt with level more than current
		int pri = get_level();
		do
		{
			uint8_t idx = 31 - count_leading_zeros_32(mask);
			if ((int)(m_aic_smr[idx] & 7) > pri)
			{
				m_core_status |= 2;
				break;
			}
			mask ^= 1 << idx;
		} while (mask);
	}

	if (m_irqs_enabled & m_irqs_pending & m_fast_irqs)
		m_core_status |= 1;

	set_lines();
}
