// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    ELAN Microelectronics RISC II (RII) Series

***************************************************************************/

#ifndef MAME_CPU_RII_RISCII_H
#define MAME_CPU_RII_RISCII_H 1

#pragma once



class riscii_series_device : public cpu_device
{
public:
	enum
	{
		RII_PC,
		RII_ACC,
		RII_FSR0,
		RII_FSR1,
		RII_BSR,
		RII_BSR1,
		RII_TABPTR,
		RII_STKPTR,
		RII_CPUCON,
		RII_STATUS,
		RII_PROD
	};

	// construction/destruction
	riscii_series_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	riscii_series_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, unsigned prgbits, unsigned bankbits, uint8_t maxbank);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	void regs_map(address_map &map);

	// address spaces
	address_space_config m_program_config;
	address_space_config m_regs_config;
	address_space *m_program;
	address_space *m_regs;

	// model-specific parameters
	const unsigned m_prgbits;
	const uint8_t m_bankmask;
	const uint8_t m_maxbank;

	// internal state
	u32 m_pc;
	u8 m_acc;
	u8 m_fsr0;
	u8 m_bsr;
	PAIR16 m_fsr1;
	u32 m_tabptr;
	u8 m_stkptr;
	u8 m_cpucon;
	u8 m_status;
	PAIR16 m_prod;

	// execution sequencing
	s32 m_icount;
};

DECLARE_DEVICE_TYPE(RISCII, riscii_series_device)

#endif // MAME_CPU_RII_RISCII_H
