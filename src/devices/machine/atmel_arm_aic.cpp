// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
   ARM AIC (Advanced Interrupt Controller) from Atmel
   typically integrated into the AM91SAM series of chips

   see http://sam7-ex256.narod.ru/include/HTML/AT91SAM7X256_AIC.html
   current implementation only handles basics needed for pgm2 (pgm2.cpp)
   if this peripheral is not available as a standalone chip it could also be moved to
   the CPU folder alongside the ARM instead
*/

#include "emu.h"
#include "atmel_arm_aic.h"

DEFINE_DEVICE_TYPE(ARM_AIC, arm_aic_device, "arm_aic", "ARM Advanced Interrupt Controller")


DEVICE_ADDRESS_MAP_START( regs_map, 32, arm_aic_device )
	AM_RANGE(0x000, 0x07f) AM_READWRITE(aic_smr_r, aic_smr_w) // AIC_SMR[32] (AIC_SMR)  Source Mode Register
	AM_RANGE(0x080, 0x0ff) AM_READWRITE(aic_svr_r, aic_svr_w) // AIC_SVR[32] (AIC_SVR)  Source Vector Register
	AM_RANGE(0x100, 0x103) AM_READ(irq_vector_r)      // AIC_IVR    IRQ Vector Register
	AM_RANGE(0x104, 0x107) AM_READ(firq_vector_r)     // AIC_FVR    FIQ Vector Register
// 0x108    AIC_ISR Interrupt Status Register
// 0x10C    AIC_IPR Interrupt Pending Register
// 0x110    AIC_IMR Interrupt Mask Register
// 0x114    AIC_CISR    Core Interrupt Status Register
	AM_RANGE(0x120, 0x123) AM_WRITE(aic_iecr_w) // 0x120    AIC_IECR    Interrupt Enable Command Register
	AM_RANGE(0x124, 0x127) AM_WRITE(aic_idcr_w) // 0x124    AIC_IDCR    Interrupt Disable Command Register
	AM_RANGE(0x128, 0x12b) AM_WRITE(aic_iccr_w) // 0x128    AIC_ICCR    Interrupt Clear Command Register
// 0x12C    AIC_ISCR    Interrupt Set Command Register
	AM_RANGE(0x130, 0x133) AM_WRITE(aic_eoicr_w) // 0x130   AIC_EOICR   End of Interrupt Command Register
// 0x134    AIC_SPU Spurious Vector Register
// 0x138    AIC_DCR Debug Control Register (Protect)
// 0x140    AIC_FFER    Fast Forcing Enable Register
// 0x144    AIC_FFDR    Fast Forcing Disable Register
// 0x148    AIC_FFSR    Fast Forcing Status Register
ADDRESS_MAP_END

READ32_MEMBER(arm_aic_device::irq_vector_r)
{
	return m_current_irq_vector;
}

READ32_MEMBER(arm_aic_device::firq_vector_r)
{
	return m_current_firq_vector;
}

void arm_aic_device::device_start()
{
	m_irq_out.resolve_safe();

	save_item(NAME(m_irqs_enabled));
	save_item(NAME(m_current_irq_vector));
	save_item(NAME(m_current_firq_vector));

	save_item(NAME(m_aic_smr));
	save_item(NAME(m_aic_svr));
}

void arm_aic_device::device_reset()
{
	m_irqs_enabled = 0;
	m_current_irq_vector = 0;
	m_current_firq_vector = 0;

	for(auto & elem : m_aic_smr) { elem = 0; }
	for(auto & elem : m_aic_svr) { elem = 0; }
}

void arm_aic_device::set_irq(int identity)
{
	for (int i = 0;i < 32;i++)
	{
		if (m_aic_smr[i] == identity)
		{
			if ((m_irqs_enabled >> i) & 1)
			{
				m_current_irq_vector = m_aic_svr[i];
				m_irq_out(ASSERT_LINE);
				return;
			}
		}
	}
}

WRITE32_MEMBER(arm_aic_device::aic_iccr_w)
{
	//logerror("%s: aic_iccr_w  %08x (Interrupt Clear Command Register)\n", machine().describe_context().c_str(), data);
	m_irq_out(CLEAR_LINE);
};
