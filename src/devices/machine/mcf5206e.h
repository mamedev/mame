// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    MCF5206E Peripherals

***************************************************************************/

#pragma once

#ifndef __MCF5206E_PERIPHERAL_H__
#define __MCF5206E_PERIPHERAL_H__

#include "emu.h"



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_MCF5206E_PERIPHERAL_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MCF5206E_PERIPHERAL, 0)

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> mcf5206e_peripheral_device

enum
{
	ICR1 = 0,
	ICR2,
	ICR3,
	ICR4,
	ICR5,
	ICR6,
	ICR7,
	ICR8,
	ICR9,
	ICR10,
	ICR11,
	ICR12,
	ICR13,
	MAX_ICR
};

class mcf5206e_peripheral_device :  public device_t,
									public device_memory_interface
{
public:
	// construction/destruction
	mcf5206e_peripheral_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void ICR_info(uint8_t ICR);

	uint32_t dev_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void dev_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t seta2_coldfire_regs_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void seta2_coldfire_regs_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	uint8_t ICR1_ICR2_ICR3_ICR4_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ICR1_ICR2_ICR3_ICR4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t ICR9_ICR10_ICR11_ICR12_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ICR9_ICR10_ICR11_ICR12_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ICR13_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ICR13_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint16_t CSAR_r(int which, int offset, uint16_t mem_mask);
	void CSAR_w(int which, int offset, uint16_t data, uint16_t mem_mask);
	uint32_t CSMR_r(int which, uint32_t mem_mask);
	void CSMR_w(int which, uint32_t data, uint32_t mem_mask);
	uint16_t CSCR_r(int which, int offset, uint16_t mem_mask);
	void CSCR_w(int which, int offset, uint16_t data, uint16_t mem_mask);

	uint16_t CSAR0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void CSAR0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t CSMR0_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void CSMR0_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint16_t CSCR0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void CSCR0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t CSAR1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void CSAR1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t CSMR1_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void CSMR1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint16_t CSCR1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void CSCR1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t CSAR2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void CSAR2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t CSMR2_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void CSMR2_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint16_t CSCR2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void CSCR2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t CSAR3_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void CSAR3_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t CSMR3_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void CSMR3_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint16_t CSCR3_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void CSCR3_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t CSAR4_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void CSAR4_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t CSMR4_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void CSMR4_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint16_t CSCR4_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void CSCR4_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t CSAR5_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void CSAR5_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t CSMR5_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void CSMR5_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint16_t CSCR5_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void CSCR5_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t CSAR6_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void CSAR6_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t CSMR6_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void CSMR6_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint16_t CSCR6_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void CSCR6_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t CSAR7_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void CSAR7_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t CSMR7_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void CSMR7_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint16_t CSCR7_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void CSCR7_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t DMCR_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void DMCR_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t PAR_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void PAR_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t TMR1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void TMR1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t TRR1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void TRR1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t TER1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void TER1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t TCN1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void TCN1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint8_t PPDDR_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void PPDDR_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t PPDAT_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void PPDAT_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint16_t IMR_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void IMR_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);


	uint8_t MBCR_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void MBCR_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t MBSR_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void MBSR_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t MFDR_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void MFDR_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t MBDR_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void MBDR_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);




	cpu_device* m_cpu;

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override { }
	virtual void device_clock_changed() override { }
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;
	address_space_config        m_space_config;


private:

	void init_regs(bool first_init);

	uint8_t m_ICR[MAX_ICR];

	uint16_t m_CSAR[8];
	uint32_t m_CSMR[8];
	uint16_t m_CSCR[8];

	uint16_t m_DMCR;
	uint16_t m_PAR;

	emu_timer *m_timer1;
	uint16_t m_TMR1;
	uint16_t m_TRR1;
	uint8_t m_TER1;
	uint16_t m_TCN1;
	void timer1_callback(void *ptr, int32_t param);


	uint8_t m_PPDDR;
	uint8_t m_PPDAT;

	uint16_t m_IMR;

	uint8_t m_MBCR;
	uint8_t m_MBSR;
	uint8_t m_MFDR;
	uint8_t m_MBDR;

	uint32_t m_coldfire_regs[0x400/4];

private:
};


// device type definition
extern const device_type MCF5206E_PERIPHERAL;

#endif  /* __MCF5206E_PERIPHERAL_H__ */
