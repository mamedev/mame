// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_ATMEL_ARM_AIC_H
#define MAME_MACHINE_ATMEL_ARM_AIC_H

#pragma once

DECLARE_DEVICE_TYPE(ARM_AIC, arm_aic_device)

#define MCFG_ARM_AIC_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, ARM_AIC, 0)

#define MCFG_IRQ_LINE_CB(_devcb) \
	devcb = &arm_aic_device::set_line_callback(*device, DEVCB_##_devcb);

class arm_aic_device : public device_t
{
public:
	// construction/destruction
	arm_aic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, ARM_AIC, tag, owner, clock),
		m_irq_out(*this)
	{
	}

	// configuration
	template <class Object> static devcb_base &set_line_callback(device_t &device, Object &&cb) { return downcast<arm_aic_device &>(device).m_irq_out.set_callback(std::forward<Object>(cb)); }

	DECLARE_ADDRESS_MAP(regs_map, 32);

	DECLARE_READ32_MEMBER(irq_vector_r);
	DECLARE_READ32_MEMBER(firq_vector_r);

	// can't use AM_RAM and AM_SHARE in device submaps
	DECLARE_READ32_MEMBER(aic_smr_r) { return m_aic_smr[offset]; };
	DECLARE_READ32_MEMBER(aic_svr_r) { return m_aic_svr[offset]; };
	DECLARE_WRITE32_MEMBER(aic_smr_w) { COMBINE_DATA(&m_aic_smr[offset]); };
	DECLARE_WRITE32_MEMBER(aic_svr_w) { COMBINE_DATA(&m_aic_svr[offset]); };

	DECLARE_WRITE32_MEMBER(aic_iecr_w) { /*logerror("%s: aic_iecr_w  %08x (Interrupt Enable Command Register)\n", machine().describe_context().c_str(), data);*/ COMBINE_DATA(&m_irqs_enabled); };
	DECLARE_WRITE32_MEMBER(aic_idcr_w) { /*logerror("%s: aic_idcr_w  %08x (Interrupt Disable Command Register)\n", machine().describe_context().c_str(), data);*/ };
	DECLARE_WRITE32_MEMBER(aic_iccr_w);
	DECLARE_WRITE32_MEMBER(aic_eoicr_w){ /*logerror("%s: aic_eoicr_w (End of Interrupt Command Register)\n", machine().describe_context().c_str());*/ }; // value doesn't matter

	void set_irq(int identity);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint32_t m_irqs_enabled;
	uint32_t m_current_irq_vector;
	uint32_t m_current_firq_vector;

	uint32_t m_aic_smr[32];
	uint32_t m_aic_svr[32];

	devcb_write_line    m_irq_out;
};

#endif
