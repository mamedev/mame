// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    MCF5206E Peripherals

***************************************************************************/

#ifndef MAME_MACHINE_MCF5206E_H
#define MAME_MACHINE_MCF5206E_H

#pragma once


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> mcf5206e_peripheral_device

class mcf5206e_peripheral_device :  public device_t,
									public device_memory_interface
{
public:
	// construction/destruction
	template <typename T>
	mcf5206e_peripheral_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: mcf5206e_peripheral_device(mconfig, tag, owner, clock)
	{
		m_maincpu.set_tag(std::forward<T>(cpu_tag));
	}

	mcf5206e_peripheral_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t dev_r(offs_t offset, uint32_t mem_mask = ~0);
	void dev_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t seta2_coldfire_regs_r(offs_t offset);
	void seta2_coldfire_regs_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override { }
	virtual space_config_vector memory_space_config() const override;

private:
	TIMER_CALLBACK_MEMBER(timer1_callback);

	void init_regs(bool first_init);

	void ICR_info(uint8_t ICR);

	uint16_t CSAR_r(int which, int offset, uint16_t mem_mask);
	void CSAR_w(int which, int offset, uint16_t data, uint16_t mem_mask);
	uint32_t CSMR_r(int which, uint32_t mem_mask);
	void CSMR_w(int which, uint32_t data, uint32_t mem_mask);
	uint16_t CSCR_r(int which, int offset, uint16_t mem_mask);
	void CSCR_w(int which, int offset, uint16_t data, uint16_t mem_mask);

	uint8_t ICR1_ICR2_ICR3_ICR4_r(offs_t offset);
	void ICR1_ICR2_ICR3_ICR4_w(offs_t offset, uint8_t data);

	uint8_t ICR9_ICR10_ICR11_ICR12_r(offs_t offset);
	void ICR9_ICR10_ICR11_ICR12_w(offs_t offset, uint8_t data);
	uint8_t ICR13_r(offs_t offset);
	void ICR13_w(offs_t offset, uint8_t data);

	uint16_t CSAR0_r(offs_t offset, uint16_t mem_mask = ~0);
	void CSAR0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t CSMR0_r(offs_t offset, uint32_t mem_mask = ~0);
	void CSMR0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t CSCR0_r(offs_t offset, uint16_t mem_mask = ~0);
	void CSCR0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t CSAR1_r(offs_t offset, uint16_t mem_mask = ~0);
	void CSAR1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t CSMR1_r(offs_t offset, uint32_t mem_mask = ~0);
	void CSMR1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t CSCR1_r(offs_t offset, uint16_t mem_mask = ~0);
	void CSCR1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t CSAR2_r(offs_t offset, uint16_t mem_mask = ~0);
	void CSAR2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t CSMR2_r(offs_t offset, uint32_t mem_mask = ~0);
	void CSMR2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t CSCR2_r(offs_t offset, uint16_t mem_mask = ~0);
	void CSCR2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t CSAR3_r(offs_t offset, uint16_t mem_mask = ~0);
	void CSAR3_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t CSMR3_r(offs_t offset, uint32_t mem_mask = ~0);
	void CSMR3_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t CSCR3_r(offs_t offset, uint16_t mem_mask = ~0);
	void CSCR3_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t CSAR4_r(offs_t offset, uint16_t mem_mask = ~0);
	void CSAR4_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t CSMR4_r(offs_t offset, uint32_t mem_mask = ~0);
	void CSMR4_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t CSCR4_r(offs_t offset, uint16_t mem_mask = ~0);
	void CSCR4_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t CSAR5_r(offs_t offset, uint16_t mem_mask = ~0);
	void CSAR5_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t CSMR5_r(offs_t offset, uint32_t mem_mask = ~0);
	void CSMR5_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t CSCR5_r(offs_t offset, uint16_t mem_mask = ~0);
	void CSCR5_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t CSAR6_r(offs_t offset, uint16_t mem_mask = ~0);
	void CSAR6_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t CSMR6_r(offs_t offset, uint32_t mem_mask = ~0);
	void CSMR6_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t CSCR6_r(offs_t offset, uint16_t mem_mask = ~0);
	void CSCR6_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t CSAR7_r(offs_t offset, uint16_t mem_mask = ~0);
	void CSAR7_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t CSMR7_r(offs_t offset, uint32_t mem_mask = ~0);
	void CSMR7_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t CSCR7_r(offs_t offset, uint16_t mem_mask = ~0);
	void CSCR7_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t DMCR_r(offs_t offset, uint16_t mem_mask = ~0);
	void DMCR_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t PAR_r(offs_t offset, uint16_t mem_mask = ~0);
	void PAR_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t TMR1_r(offs_t offset, uint16_t mem_mask = ~0);
	void TMR1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t TRR1_r(offs_t offset, uint16_t mem_mask = ~0);
	void TRR1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t TER1_r(offs_t offset);
	void TER1_w(offs_t offset, uint8_t data);
	uint16_t TCN1_r(offs_t offset, uint16_t mem_mask = ~0);
	void TCN1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint8_t PPDDR_r(offs_t offset);
	void PPDDR_w(offs_t offset, uint8_t data);
	uint8_t PPDAT_r(offs_t offset);
	void PPDAT_w(offs_t offset, uint8_t data);

	uint16_t IMR_r(offs_t offset, uint16_t mem_mask = ~0);
	void IMR_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);


	uint8_t MBCR_r(offs_t offset);
	void MBCR_w(offs_t offset, uint8_t data);
	uint8_t MBSR_r(offs_t offset);
	void MBSR_w(offs_t offset, uint8_t data);
	uint8_t MFDR_r(offs_t offset);
	void MFDR_w(offs_t offset, uint8_t data);
	uint8_t MBDR_r(offs_t offset);
	void MBDR_w(offs_t offset, uint8_t data);

	void coldfire_regs_map(address_map &map) ATTR_COLD;

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

	required_device<cpu_device> m_maincpu;

	address_space_config m_space_config;

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


	uint8_t m_PPDDR;
	uint8_t m_PPDAT;

	uint16_t m_IMR;

	uint8_t m_MBCR;
	uint8_t m_MBSR;
	uint8_t m_MFDR;
	uint8_t m_MBDR;

	uint32_t m_coldfire_regs[0x400/4];
};


// device type definition
DECLARE_DEVICE_TYPE(MCF5206E_PERIPHERAL, mcf5206e_peripheral_device)

#endif // MAME_MACHINE_MCF5206E_H
